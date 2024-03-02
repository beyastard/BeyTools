#include "AFilePackage.h"
#include "AFilePackMan.h"
#include "AFI.h"
#include "AFPI.h"
#include "AAssist.h"
#include "MutexGuard.h"
#include "zlib.h"

#include <algorithm>

extern uint32_t AFPCK_GUARDBYTE0;
extern uint32_t AFPCK_GUARDBYTE1;
extern uint32_t AFPCK_MASKDWORD;
extern uint32_t AFPCK_CHECKMASK;

int32_t _CacheFileNameCompare(const void* arg1, const void* arg2)
{
	AFilePackage::CACHEFILENAME* pFile1 = *(AFilePackage::CACHEFILENAME**)arg1;
	AFilePackage::CACHEFILENAME* pFile2 = *(AFilePackage::CACHEFILENAME**)arg2;

	return (pFile1->dwFileID > pFile2->dwFileID) ?  1 :
		  ((pFile1->dwFileID < pFile2->dwFileID) ? -1 : 0);
}

void* Zlib_User_Alloc(void* opaque, uint32_t items, uint32_t size)
{
	return new char[size * items];
}

void Zlib_User_Free(void* opaque, void* ptr)
{
	delete[] static_cast<char*>(ptr);
}

int32_t Zlib_Compress(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen, int32_t level = Z_BEST_SPEED)
{
	z_stream stream;
	int32_t err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen)
		return Z_BUF_ERROR;

	stream.zalloc = &Zlib_User_Alloc;	//0;
	stream.zfree = &Zlib_User_Free;		//0;
	stream.opaque = (voidpf)0;

	err = deflateInit(&stream, level);
	if (err != Z_OK)
		return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	*destLen = stream.total_out;
	err = deflateEnd(&stream);

	return err;
}

int32_t Zlib_UnCompress(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen)
{
	z_stream stream;
	int32_t err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen)
		return Z_BUF_ERROR;

	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = &Zlib_User_Alloc;  //0;
	stream.zfree = &Zlib_User_Free;   //0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	*destLen = stream.total_out;
	err = inflateEnd(&stream);

	return err;
}

AFilePackage::directory::~directory()
{
	clear();
}

int32_t AFilePackage::directory::clear()
{
	for (size_t i = 0; i < _list.size(); i++)
		delete _list[i];

	_list.clear();

	return 0;
}

AFilePackage::entry* AFilePackage::directory::SearchItem(const std::wstring& item)
{
	int32_t idx = searchItemIndex(item, nullptr);
	return idx < 0 ? nullptr : _list[idx];
}

AFilePackage::entry* AFilePackage::directory::GetItem(int32_t index)
{
	return (index < 0 || index >= static_cast<int32_t>(_list.size()))
		? nullptr
		: _list[index];
}

int32_t AFilePackage::directory::RemoveItem(const std::wstring& item)
{
	int32_t rst;
	rst = searchItemIndex(item, nullptr);
	if (rst < 0)
		return -1;

	delete _list[rst];
	_list.erase(_list.begin() + rst);

	return 0;
}

int32_t AFilePackage::directory::AppendEntry(entry* item)
{
	// Search for the insertion position
	std::vector<AFilePackage::entry*>::iterator it =
		std::lower_bound(_list.begin(), _list.end(), item,
			[](const entry* a, const entry* b) { return a->_name < b->_name; });

	// Check if item already exists
	if (it != _list.end() && (*it)->_name == item->_name)
		return -1;  // Item already exists

	_list.emplace(it, item); // Insert the item at the determined position

	return 0;
}

int32_t AFilePackage::directory::SearchEntry(const std::wstring& filename)
{
	entry* ent = this;

	size_t pos = 0;
	while (pos < filename.size())
	{
		// Find the next delimiter
		size_t next_pos = filename.find(L'\\', pos);
		if (next_pos == std::wstring::npos)
			next_pos = filename.size();

		// Extract the directory component
		std::wstring dir_component = filename.substr(pos, next_pos - pos);
		pos = next_pos + 1;

		// Search for the directory component in the current directory
		entry* tmp = ent->SearchItem(dir_component);
		if (tmp == nullptr)
			return -1; // Component not found

		if (pos < filename.size())
		{
			// Check if the component is a container
			if (!tmp->IsContainer())
				return -1; // Not a container

			ent = tmp; // Move to the next level of the directory hierarchy
		}
		else
			return tmp->GetIndex(); // Last component found, return its index
	}

	return -1; // If we reach here, the entire filename has been processed but not found
}

int32_t AFilePackage::directory::searchItemIndex(const std::wstring& item, int32_t* pos)
{
	std::vector<AFilePackage::entry*>::iterator it = std::lower_bound(_list.begin(), _list.end(), item,
		[](const entry* e, const std::wstring& name)
		{
			return string_to_wstring(e->_name) < name;
		});

	if (it != _list.end() && string_to_wstring((*it)->_name) == item)
	{
		return pos
			? *pos = static_cast<int32_t>(std::distance(_list.begin(), it))
			: static_cast<int32_t>(std::distance(_list.begin(), it));
	}

	return -1;
}

AFilePackage::AFilePackage()
	: m_bHasChanged(false)
	, m_bReadOnly(false)
	, m_bUseShortName(false)
	, m_header()
	, m_mode(OPENEXIST)
	, m_aFileEntries()
	, m_aFileEntryCache()
	, m_fpPackageFile(nullptr)
	, m_szPckFileName(L"")
	, m_szFolder(L"")
	, m_directory()
	, m_dwCacheSize(0)
	, m_dwSharedSize(0)
	, m_bHasSafeHeader(false)
	, m_safeHeader()
{}

AFilePackage::~AFilePackage() {}

bool AFilePackage::Open(const std::wstring& szPckPath, OPENMODE mode, bool bEncrypt)
{
	if (szPckPath.empty())
	{
		AFERRLOG((L"AFilePackage::Open(), cannot open a null or empty file name!"));
		return false;
	}

	// Find the last occurrence of '\\' to extract the folder path
	size_t pos = szPckPath.find_last_of(L'\\');
	if (pos == std::wstring::npos)
	{
		AFERRLOG((L"AFilePackage::Open(), only file with extension can be opened!"));
		return false;
	}

	std::wstring szFolder = szPckPath.substr(0, pos + 1); // Include the '\\' character

	return InnerOpen(szPckPath, szFolder, mode, bEncrypt, false);
}

bool AFilePackage::Open(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt)
{
	return InnerOpen(szPckPath, szFolder, mode, bEncrypt, true);
}

bool AFilePackage::Close()
{
	if (!m_fpPackageFile)
	{
		AFERRLOG((L"AFilePackage::Close(), Package file not open!"));
		return false;
	}

	if (m_mode == CREATENEW)
	{
		if (!SaveEntries())
			return false;

		int32_t iNumFile = (int32_t)m_aFileEntries.size();

		// Write file header here;
		m_header.dwEntryOffset ^= AFPCK_MASKDWORD;
		m_fpPackageFile->write(&m_header, sizeof(FILEHEADER), 1);
		m_header.dwEntryOffset ^= AFPCK_MASKDWORD;

		m_fpPackageFile->write(&iNumFile, sizeof(int32_t), 1);
		m_fpPackageFile->write(&m_header.dwVersion, sizeof(uint32_t), 1);

		SaveSafeHeader();
	}
	else if (m_bHasChanged)
	{
		uint32_t dwFileSize = m_header.dwEntryOffset;
		uint32_t dwEntrySize = 0;
		if (!SaveEntries(&dwEntrySize))
			return false;

		dwFileSize += dwEntrySize;

		// Write file header here;
		m_header.dwEntryOffset ^= AFPCK_MASKDWORD;
		m_fpPackageFile->write(&m_header, sizeof(FILEHEADER), 1);
		m_header.dwEntryOffset ^= AFPCK_MASKDWORD;
		dwFileSize += sizeof(FILEHEADER);

		int32_t iNumFile = (int32_t)m_aFileEntries.size();
		m_fpPackageFile->write(&iNumFile, sizeof(int32_t), 1);
		dwFileSize += sizeof(int32_t);
		m_fpPackageFile->write(&m_header.dwVersion, sizeof(uint32_t), 1);
		dwFileSize += sizeof(uint32_t);

		m_fpPackageFile->SetPackageFileSize(dwFileSize);

		SaveSafeHeader();
	}

	m_fpPackageFile->Close();
	delete m_fpPackageFile;
	m_fpPackageFile = nullptr;

	for (FILEENTRY* entry : m_aFileEntries)
		delete entry;

	m_aFileEntries.clear();

	for (FILEENTRYCACHE* cache : m_aFileEntryCache)
	{
		if (cache)
		{
			delete[] cache->pEntryCompressed;
			cache->pEntryCompressed = nullptr;
			delete cache;
			cache = nullptr;
		}
	}

	m_aFileEntryCache.clear();

	return true;
}

bool AFilePackage::AppendFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t dwFileLength, bool bCompress)
{
	if (m_bReadOnly)
	{
		AFERRLOG((L"AFilePackage::AppendFile(), Read only package, can not append!"));
		return false;
	}

	FILEENTRY fileEntry;
	if (GetFileEntry(wstring_to_string(szFileName), &fileEntry))
	{
		AFERRLOG((L"AFilePackage::AppendFile(), file entry [%s] already exist!", szFileName));
		return false;
	}

	uint32_t dwCompressedLength = dwFileLength;
	if (bCompress)
	{
		//	Compress the file
		uint8_t* pBuffer = new uint8_t[dwFileLength];
		if (!pBuffer)
			return false;

		if (0 != Compress(pFileBuffer, dwFileLength, pBuffer, &dwCompressedLength))
			dwCompressedLength = dwFileLength; // compress error, so use uncompressed format

		if (dwCompressedLength < dwFileLength)
		{
			if (!AppendFileCompressed(szFileName, pBuffer, dwFileLength, dwCompressedLength))
			{
				delete[] pBuffer;
				pBuffer = nullptr;
				return false;
			}
		}
		else
		{
			if (!AppendFileCompressed(szFileName, pFileBuffer, dwFileLength, dwFileLength))
			{
				delete[] pBuffer;
				pBuffer = nullptr;
				return false;
			}
		}

		delete[] pBuffer;
		pBuffer = nullptr;
	}
	else
	{
		if (!AppendFileCompressed(szFileName, pFileBuffer, dwFileLength, dwFileLength))
			return false;
	}

	return true;
}

static bool CheckFileEntryValid(AFilePackage::FILEENTRY* pFileEntry)
{
	if (pFileEntry->dwCompressedLength > MAX_FILE_PACKAGE)
	{
		AFERRLOG((L"CheckFileEntryValid, file entry [%s]'s length is not correct!", pFileEntry->szFileName));
		return false;
	}

	return true;
}

bool AFilePackage::AppendFileCompressed(const std::wstring& szFileName, LPBYTE pCompressedBuffer, uint32_t dwFileLength, uint32_t dwCompressedLength)
{
	FILEENTRY* pEntry = new FILEENTRY;
	if (!pEntry)
	{
		AFERRLOG((L"AFilePackage::AppendFile(), Not enough memory!"));
		return false;
	}

	// Normalize file name
	std::wstring szSavedFileName;
	szSavedFileName = szFileName;
	NormalizeFileName(szSavedFileName, m_bUseShortName);

	// Store this file;
	std::strncpy(pEntry->szFileName, wstring_to_string(szSavedFileName).c_str(), sizeof(szFileName) - 1);
	pEntry->szFileName[sizeof(szFileName) - 1] = '\0';
	pEntry->dwOffset = m_header.dwEntryOffset;
	pEntry->dwLength = dwFileLength;
	pEntry->dwCompressedLength = dwCompressedLength;
	pEntry->iAccessCnt = 0;
	if (!CheckFileEntryValid(pEntry))
	{
		delete pEntry;
		pEntry = nullptr;
		AFERRLOG((L"AFilePackage::AppendFile(), Invalid File Entry!"));
		return false;
	}

	m_aFileEntries.push_back(pEntry);

	FILEENTRYCACHE* pEntryCache = new FILEENTRYCACHE;
	if (!pEntryCache)
	{
		AFERRLOG((L"AFilePackage::AppendFile(), Not enough memory!"));
		return false;
	}

	uint32_t dwCompressedSize = sizeof(FILEENTRY);
	uint8_t* pBuffer = new uint8_t[(sizeof(FILEENTRY))];
	int32_t nRet = Compress((LPBYTE)pEntry, sizeof(FILEENTRY), pBuffer, &dwCompressedSize);
	if (nRet != 0 || dwCompressedSize >= sizeof(FILEENTRY))
	{
		dwCompressedSize = sizeof(FILEENTRY);
		std::memcpy(pBuffer, pEntry, sizeof(FILEENTRY));
	}

	pEntryCache->dwCompressedLength = dwCompressedSize;
	pEntryCache->pEntryCompressed = new uint8_t[dwCompressedSize];
	std::memcpy(pEntryCache->pEntryCompressed, pBuffer, dwCompressedSize);
	m_aFileEntryCache.push_back(pEntryCache);
	delete[] pBuffer;

	m_fpPackageFile->seek(m_header.dwEntryOffset, AFILE_SEEK_SET);

	//	We write the compressed buffer into the disk;
	Encrypt(pCompressedBuffer, dwCompressedLength);
	m_fpPackageFile->write(pCompressedBuffer, dwCompressedLength, 1);
	Decrypt(pCompressedBuffer, dwCompressedLength);
	m_header.dwEntryOffset += dwCompressedLength;

	InsertFileToDir(szSavedFileName, (int32_t)(m_aFileEntries.size() - 1));
	m_bHasChanged = true;

	return true;
}

bool AFilePackage::RemoveFile(const std::wstring& szFileName)
{
	if (m_bReadOnly)
	{
		AFERRLOG((L"AFilePackage::RemoveFile(), Read only package, can not remove file!"));
		return false;
	}

	FILEENTRY Entry;
	int32_t	nIndex;

	if (!GetFileEntry(wstring_to_string(szFileName), &Entry, &nIndex))
	{
		AFERRLOG((L"AFilePackage::RemoveFile(), Can not find file %s", szFileName));
		return false;
	}

	FILEENTRY* pEntry = m_aFileEntries[nIndex];
	RemoveFileFromDir(string_to_wstring(pEntry->szFileName));

	//	Added by dyx on 2013.10.14. Now we only delete entry object and leave a nullptr at it's position
	//	in m_aFileEntries, this is in order that the entry indices recoreded in file items of m_directory 
	//	can still be valid and needn't updating.
	delete pEntry;
	pEntry = nullptr;
	m_aFileEntries[nIndex] = nullptr;

	FILEENTRYCACHE* pEntryCache = m_aFileEntryCache[nIndex];
	if (pEntryCache)
	{
		if (pEntryCache->pEntryCompressed)
		{
			delete[] pEntryCache->pEntryCompressed;
			pEntryCache->pEntryCompressed = nullptr;
		}

		delete pEntryCache;
		pEntryCache = nullptr;
		m_aFileEntryCache[nIndex] = nullptr;
	}

	//ResortEntries();

	m_bHasChanged = true;
	return true;
}

bool AFilePackage::ReplaceAFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t dwFileLength, bool bCompress)
{
	uint32_t dwCompressedLength = dwFileLength;

	if (bCompress)
	{
		//	Try to compress the file
		uint8_t* pBuffer = new uint8_t[dwFileLength];
		if (!pBuffer)
			return false;

		if (0 != Compress(pFileBuffer, dwFileLength, pBuffer, &dwCompressedLength))
			dwCompressedLength = dwFileLength; // compress error, so use uncompressed format

		if (dwCompressedLength < dwFileLength)
		{
			if (!ReplaceFileCompressed(szFileName, pBuffer, dwFileLength, dwCompressedLength))
			{
				delete[] pBuffer;
				pBuffer = nullptr;
				return false;
			}
		}
		else
		{
			if (!ReplaceFileCompressed(szFileName, pFileBuffer, dwFileLength, dwFileLength))
			{
				delete[] pBuffer;
				pBuffer = nullptr;
				return false;
			}
		}

		delete[] pBuffer;
		pBuffer = nullptr;
	}
	else
	{
		if (!ReplaceFileCompressed(szFileName, pFileBuffer, dwFileLength, dwFileLength))
			return false;
	}

	return true;
}

bool AFilePackage::ReplaceFileCompressed(const std::wstring& szFileName, LPBYTE pCompressedBuffer, uint32_t dwFileLength, uint32_t dwCompressedLength)
{
	if (m_bReadOnly)
	{
		AFERRLOG((L"AFilePackage::ReplaceFileCompressed(), Read only package, can not replace!"));
		return false;
	}

	FILEENTRY Entry;
	int32_t	nIndex;

	if (!GetFileEntry(wstring_to_string(szFileName), &Entry, &nIndex))
	{
		AFERRLOG((L"AFilePackage::ReplaceFile(), Can not find file %s", szFileName));
		return false;
	}

	Entry.dwOffset = m_header.dwEntryOffset;
	Entry.dwLength = dwFileLength;
	Entry.dwCompressedLength = dwCompressedLength;
	if (!CheckFileEntryValid(&Entry))
	{
		AFERRLOG((L"AFilePackage::ReplaceFile(), Invalid File Entry"));
		return false;
	}

	FILEENTRY* pEntry = m_aFileEntries[nIndex];
	assert(pEntry);

	// modify this file entry to point to the new file body;			
	pEntry->dwOffset = m_header.dwEntryOffset;
	pEntry->dwLength = dwFileLength;
	pEntry->dwCompressedLength = dwCompressedLength;

	FILEENTRYCACHE* pEntryCache = m_aFileEntryCache[nIndex];
	uint32_t dwCompressedSize = sizeof(FILEENTRY);
	uint8_t* pBuffer = new uint8_t[(sizeof(FILEENTRY))];
	int32_t nRet = Compress((LPBYTE)pEntry, sizeof(FILEENTRY), pBuffer, &dwCompressedSize);
	if (nRet != 0 || dwCompressedSize >= sizeof(FILEENTRY))
	{
		dwCompressedSize = sizeof(FILEENTRY);
		std::memcpy(pBuffer, pEntry, sizeof(FILEENTRY));
	}

	pEntryCache->dwCompressedLength = dwCompressedSize;
	uint8_t* pNewBuffer = new uint8_t[dwCompressedSize];
	std::memcpy(pNewBuffer, pBuffer, dwCompressedSize);
	std::memcpy(pEntryCache->pEntryCompressed, pNewBuffer, dwCompressedSize);
	delete[] pNewBuffer;
	delete[] pBuffer;
	pNewBuffer = nullptr;
	pBuffer = nullptr;

	m_fpPackageFile->seek(m_header.dwEntryOffset, AFILE_SEEK_SET);

	// We write the compressed buffer into the disk;
	Encrypt(pCompressedBuffer, dwCompressedLength);
	m_fpPackageFile->write(pCompressedBuffer, dwCompressedLength, 1);
	Decrypt(pCompressedBuffer, dwCompressedLength);
	m_header.dwEntryOffset += dwCompressedLength;

	m_bHasChanged = true;
	return true;
}

bool AFilePackage::ResortEntries()
{
	m_directory.clear();
	for (size_t i = 0; i < m_aFileEntries.size(); i++)
	{
		if (m_aFileEntries[i])
			InsertFileToDir(string_to_wstring(m_aFileEntries[i]->szFileName), (int32_t)i);
	}
	return true;
}

bool AFilePackage::ReadFile(const std::wstring& szFileName, LPBYTE pFileBuffer, size_t* pdwBufferLen)
{
	FILEENTRY fileEntry;

	if (!GetFileEntry(wstring_to_string(szFileName), &fileEntry))
	{
		AFERRLOG((L"AFilePackage::ReadFile(), Can not find file entry [%s]!", szFileName));
		return false;
	}

	return ReadFile(fileEntry, pFileBuffer, pdwBufferLen);
}

bool AFilePackage::ReadFile(FILEENTRY& fileEntry, LPBYTE pFileBuffer, size_t* pdwBufferLen)
{
	if (*pdwBufferLen < fileEntry.dwLength)
	{
		AFERRLOG((L"AFilePackage::ReadFile(), Buffer is too small!"));
		return false;
	}

	// We can automatically determine whether compression has been used
	if (fileEntry.dwLength > fileEntry.dwCompressedLength)
	{
		uint32_t dwFileLength = fileEntry.dwLength;
		uint8_t* pBuffer = new uint8_t[fileEntry.dwCompressedLength];
		if (!pBuffer)
			return false;

		std::mutex mtx;
		{
			MutexGuard guard(mtx);
			m_fpPackageFile->seek(fileEntry.dwOffset, AFILE_SEEK_SET);
			m_fpPackageFile->read(pBuffer, fileEntry.dwCompressedLength, 1);
			Decrypt(pBuffer, fileEntry.dwCompressedLength);
		}
		
		if (Uncompress(pBuffer, fileEntry.dwCompressedLength, pFileBuffer, &dwFileLength) != 0)
		{
			std::ofstream ofs("logs\\bad.dat", std::ios::binary);
			if (ofs.is_open())
			{
				ofs.write(reinterpret_cast<const char*>(pBuffer), fileEntry.dwCompressedLength);
				ofs.close();
			}

			delete[] pBuffer;
			pBuffer = nullptr;
			return false;
		}

		*pdwBufferLen = dwFileLength;
		delete[] pBuffer;
		pBuffer = nullptr;
	}
	else
	{
		std::mutex mtx;
		{
			MutexGuard guard(mtx);
			m_fpPackageFile->seek(fileEntry.dwOffset, AFILE_SEEK_SET);
			m_fpPackageFile->read(pFileBuffer, fileEntry.dwLength, 1);
			Decrypt(pFileBuffer, fileEntry.dwLength);
		}
		
		*pdwBufferLen = fileEntry.dwLength;
	}

	return true;
}

bool AFilePackage::ReadCompressedFile(const std::wstring& szFileName, LPBYTE pCompressedBuffer, size_t* pdwBufferLen)
{
	FILEENTRY fileEntry;

	if (!GetFileEntry(wstring_to_string(szFileName), &fileEntry))
	{
		AFERRLOG((L"AFilePackage::ReadCompressedFile(), Can not find file entry [%s]!", szFileName));
		return false;
	}

	return ReadCompressedFile(fileEntry, pCompressedBuffer, pdwBufferLen);
}

bool AFilePackage::ReadCompressedFile(FILEENTRY& fileEntry, LPBYTE pCompressedBuffer, size_t* pdwBufferLen)
{
	if (*pdwBufferLen < fileEntry.dwCompressedLength)
	{
		AFERRLOG((L"AFilePackage::ReadCompressedFile(), Buffer is too small!"));
		return false;
	}

	std::mutex mtx;
	{
		MutexGuard guard(mtx);
		m_fpPackageFile->seek(fileEntry.dwOffset, AFILE_SEEK_SET);
		*pdwBufferLen = m_fpPackageFile->read(pCompressedBuffer, 1, (size_t)fileEntry.dwCompressedLength);
		Decrypt(pCompressedBuffer, fileEntry.dwCompressedLength);
	}
	
	return true;
}

bool AFilePackage::GetFileEntry(const std::string& szFileName, FILEENTRY* pFileEntry, int32_t* pnIndex)
{
	std::wstring szFindName = string_to_wstring(szFileName);
	NormalizeFileName(szFindName, m_bUseShortName);

	ZeroMemory(pFileEntry, sizeof(FILEENTRY));

	int32_t iEntry = m_directory.SearchEntry(szFindName);
	if (iEntry < 0)
		return false;

	if (!m_aFileEntries[iEntry])
		return false;

	*pFileEntry = *m_aFileEntries[iEntry];

	if (!CheckFileEntryValid(pFileEntry))
	{
		pFileEntry->dwLength = 0;
		pFileEntry->dwCompressedLength = 0;
	}

	if (pnIndex)
		*pnIndex = iEntry;

	return true;
}

AFilePackage::directory* AFilePackage::GetDirEntry(const std::wstring& szPath)
{
	std::wstring normalizedPath = szPath;
	if (!NormalizeFileName(normalizedPath))
	{
		// Handle normalization failure, if necessary
		return nullptr;
	}

	// Convert to lowercase
	std::wstring lowercasePath = normalizedPath;
	std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), ::tolower);

	size_t pos = 0;
	directory* dir = &m_directory;
	while ((pos = normalizedPath.find(L'\\')) != std::wstring::npos)
	{
		std::wstring token = normalizedPath.substr(0, pos);
		entry* ent = dir->SearchItem(token);
		if (ent == nullptr || !ent->IsContainer())
			return nullptr; // Entry not found or not a directory

		dir = dynamic_cast<directory*>(ent);
		if (dir == nullptr)
			return nullptr; // Failed to cast to directory

		normalizedPath.erase(0, pos + 1);
	}

	return dir;
}

void AFilePackage::ClearFileCache()
{
	std::unordered_map<SHAREDFILE*, int>::iterator it = m_SharedFileTab.begin();
	for (; it != m_SharedFileTab.end(); )
	{
		SHAREDFILE* pFileItem = it->first;

		// Don't release file which is still referenced
		if (!pFileItem->iRefCnt)
		{
			delete[] pFileItem->pFileData;
			pFileItem->pFileData = nullptr;
			delete pFileItem;
			pFileItem = nullptr;

			it = m_SharedFileTab.erase(it);
		}
		else
			++it;
	}
}

void* AFilePackage::OpenSharedFile(const std::wstring& szFileName, uint8_t** ppFileBuf, size_t* pdwFileLen, bool bTempMem)
{
	FILEENTRY FileEntry;
	int32_t iEntryIndex;
	if (!GetFileEntry(wstring_to_string(szFileName), &FileEntry, &iEntryIndex))
	{
		if (!std::wcsstr(szFileName.c_str(), L"Textures") && !std::wcsstr(szFileName.c_str(), L"Tex_"))
		{
			AFERRLOG((L"AFilePackage::OpenSharedFile, Failed to find file [%s] in package !", szFileName));
		}
		return nullptr;
	}

	assert(m_aFileEntries[iEntryIndex]);

	//	Allocate file data buffer
	uint8_t* pFileData = nullptr;
	if (bTempMem)
		pFileData = new uint8_t[FileEntry.dwLength];

	if (!pFileData)
	{
		AFERRLOG((L"AFilePackage::OpenSharedFile, Not enough memory!"));
		return nullptr;
	}

	//	Read file data
	size_t dwFileLen = FileEntry.dwLength;
	if (!ReadFile(FileEntry, pFileData, &dwFileLen))
	{
		if (bTempMem)
		{
			delete[] pFileData;
			pFileData = nullptr;
		}

		AFERRLOG((L"AFilePackage::OpenSharedFile, Failed to read file data [%s] !", szFileName));
		return nullptr;
	}

	//	Add it to shared file arrey
	SHAREDFILE* pFileItem = new SHAREDFILE;
	if (!pFileItem)
	{
		if (bTempMem)
		{
			delete[] pFileData;
			pFileData = nullptr;
		}

		AFERRLOG((L"AFilePackage::OpenSharedFile, Not enough memory!"));
		return nullptr;
	}

	pFileItem->bCached = false;
	pFileItem->bTempMem = bTempMem;
	pFileItem->dwFileID = 0;
	pFileItem->dwFileLen = (uint32_t)dwFileLen;
	pFileItem->iRefCnt = 1;
	pFileItem->pFileData = pFileData;
	pFileItem->pFileEntry = m_aFileEntries[iEntryIndex];

	//pFileItem->pFileEntry->iAccessCnt++;

	*ppFileBuf = pFileData;
	*pdwFileLen = dwFileLen;

	return pFileItem;
}

void AFilePackage::CloseSharedFile(void* dwFileHandle)
{
	SHAREDFILE* pFileItem = (SHAREDFILE*)dwFileHandle;
	assert(pFileItem && pFileItem->iRefCnt > 0);

	//	No cache file, release it
	if (pFileItem->bTempMem)
	{
		delete[] pFileItem->pFileData;
		pFileItem->pFileData = nullptr;
	}

	delete pFileItem;
	pFileItem = nullptr;
}

bool AFilePackage::IsFileExist(const std::wstring& szFileName)
{
	FILEENTRY FileEntry;
	int32_t iEntryIndex;
	return GetFileEntry(wstring_to_string(szFileName), &FileEntry, &iEntryIndex);
}

// Compress a data buffer
// pFileBuffer         IN      buffer contains data to be compressed
// dwFileLength        IN      the bytes in buffer to be compressed
// pCompressedBuffer   OUT     the buffer to hold the compressed data
// pdwCompressedLength IN/OUT  the compressed buffer size when used as input
//                             when out, it contains the real compressed length
// 
// RETURN:  0,  ok
//         -1,  dest buffer is too small
//         -2,  unknown error
int32_t AFilePackage::Compress(LPBYTE pFileBuffer, uint32_t dwFileLength, LPBYTE pCompressedBuffer, uint32_t* pdwCompressedLength)
{
	int32_t nRet = compress2(pCompressedBuffer, (uLongf*)pdwCompressedLength, pFileBuffer, dwFileLength, 1);
	switch (nRet)
	{
	case Z_OK:
		return 0;
	case Z_BUF_ERROR:
		return -1;
	default:
		return -2;
	}
}

// Uncompress a data buffer
// pCompressedBuffer   IN      buffer contains compressed data to be uncompressed
// dwCompressedLength  IN      the compressed data size
// pFileBuffer         OUT     the uncompressed data buffer
// pdwFileLength       IN/OUT  the uncompressed data buffer size as input
//                             when out, it is the real uncompressed data length
// 
// RETURN:  0,  ok
//         -1,  dest buffer is too small
//         -2,  unknown error
int32_t AFilePackage::Uncompress(LPBYTE pCompressedBuffer, uint32_t dwCompressedLength, LPBYTE pFileBuffer, uint32_t* pdwFileLength)
{
	int32_t nRet = Zlib_UnCompress(pFileBuffer, (uLongf*)pdwFileLength, pCompressedBuffer, dwCompressedLength);
	switch (nRet)
	{
	case Z_OK:
		return 0;
	case Z_BUF_ERROR:
		return -1;
	default:
		return -2;
	}
}

bool AFilePackage::AddCacheFileNameList(const std::wstring& szDescFile)
{
	return true;
}

bool AFilePackage::AddCacheFileName(const std::wstring& szFile)
{
	CACHEFILENAME* pCacheFile = new CACHEFILENAME;
	if (!pCacheFile)
	{
		AFERRLOG((L"AFilePackage::ReadCacheFileNameList, Not enough memory !"));
		return false;
	}

	pCacheFile->strFileName = szFile;
	pCacheFile->dwFileID = a_MakeIDFromLowString(szFile);

	//if (!m_CachedFileTab.put((int32_t)pCacheFile->dwFileID, pCacheFile))
	//{
	//	//	Failed to put item into table, this maybe caused by file name collision
	//	delete pCacheFile;
	//	return false;
	//}
	CACHEFILENAME* key = new CACHEFILENAME(pCacheFile->strFileName, pCacheFile->dwFileID);
	auto result = m_CachedFileTab.insert(std::make_pair(key, 0));
	if (!result.second)
	{
		// Failed to put item into table, this maybe caused by file name collision
		delete key;
		delete pCacheFile;
		key = nullptr;
		pCacheFile = nullptr;
		return false;
	}
	else
		result.first->second++; // Increment the reference count for the key in the map

	return true;
}

bool AFilePackage::NormalizeFileName(std::wstring& szFileName)
{
	// First, replace '/' with '\\'
	std::replace(szFileName.begin(), szFileName.end(), L'/', L'\\');

	// Remove consecutive '\\' characters
	szFileName.erase(std::unique(szFileName.begin(), szFileName.end(), [](wchar_t a, wchar_t b) { return a == '\\' && b == '\\'; }), szFileName.end());

	// Remove the preceding ".\\" string
	if (szFileName.size() > 2 && szFileName.substr(0, 2) == L".\\")
		szFileName.erase(0, 2);

	// Remove extra space at the end of the string
	szFileName.erase(szFileName.find_last_not_of(L' ') + 1);

	return true;
}

bool AFilePackage::NormalizeFileName(std::wstring& szFileName, bool bUseShortName)
{
	if (!NormalizeFileName(szFileName))
		return false;

	// Get rid of folder from file name
	if (bUseShortName)
	{
		std::wstring szFullName = szFileName;
		GetRidOfFolder(szFullName, szFileName);
	}

	return true;
}

void AFilePackage::GetRidOfFolder(const std::wstring& szInName, std::wstring& szOutName)
{
	af_GetRelativePathNoBase(szInName, m_szFolder, szOutName);
}

bool AFilePackage::InnerOpen(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt, bool bShortName)
{
	std::wstring szFullPckPath;
	af_GetFullPath(szFullPckPath, szPckPath);

	m_bUseShortName = bShortName;

	//	Save folder name
	size_t iFolderLen = szFolder.size();
	m_szFolder.clear();

	if (iFolderLen > 0)
	{
		m_szFolder = szFolder;
		std::transform(m_szFolder.begin(), m_szFolder.end(), m_szFolder.begin(), towlower);
		NormalizeFileName(m_szFolder);

		//	Add '\' at folder tail
		if (m_szFolder[iFolderLen - 1] != L'\\')
			m_szFolder += L'\\';
	}

	switch (mode)
	{
	case OPENEXIST:
		m_bReadOnly = false;
		m_fpPackageFile = new CPackageFile();

		if (!m_fpPackageFile->Open(szFullPckPath, L"r+b"))
		{
			if (!m_fpPackageFile->Open(szFullPckPath, L"rb"))
			{
				delete m_fpPackageFile;
				m_fpPackageFile = nullptr;

				AFERRLOG((L"AFilePackage::Open(), Can not open file [%s]", szFullPckPath));
				return false;
			}
			m_bReadOnly = true;
		}

		m_szPckFileName = szPckPath;

		LoadSafeHeader();

		int32_t nOffset;
		m_fpPackageFile->seek(0, AFILE_SEEK_END);
		nOffset = (int32_t)m_fpPackageFile->tell();
		m_fpPackageFile->seek(0, AFILE_SEEK_SET);

		if (m_bHasSafeHeader)
			nOffset = (int32_t)m_safeHeader.offset;

		// Now analyse the file entries of the package;
		uint32_t dwVersion;

		// First version;
		m_fpPackageFile->seek(nOffset - sizeof(int32_t), AFILE_SEEK_SET);
		m_fpPackageFile->read(&dwVersion, sizeof(uint32_t), 1);

		if (dwVersion == 0x00020002 || dwVersion == 0x00020001)
		{
			int32_t i, iNumFile;

			// Now read file number;
			m_fpPackageFile->seek(nOffset - (sizeof(int32_t) + sizeof(uint32_t)), AFILE_SEEK_SET);
			m_fpPackageFile->read(&iNumFile, sizeof(int32_t), 1);
			m_fpPackageFile->seek(nOffset - (sizeof(FILEHEADER) + sizeof(uint32_t) + sizeof(int32_t)), AFILE_SEEK_SET);
			m_fpPackageFile->read(&m_header, sizeof(FILEHEADER), 1);
			if (std::strstr(m_header.szDescription, "lica File Package") == nullptr)
				return false;

			std::strncpy(m_header.szDescription, AFPCK_COPYRIGHT_TAG, sizeof(m_header.szDescription));

			// if we don't expect one encrypt package, we will let the error come out.
			// make sure the encrypt flag is correct
			bool bPackIsEncrypt = (m_header.dwFlags & PACKFLAG_ENCRYPT) != 0;
			if (bEncrypt != bPackIsEncrypt)
			{
				AFERRLOG((L"AFilePackage::Open(), wrong encrypt flag"));
				return false;
			}

			m_header.dwEntryOffset ^= AFPCK_MASKDWORD;

			if (m_header.guardByte0 != AFPCK_GUARDBYTE0 ||
				m_header.guardByte1 != AFPCK_GUARDBYTE1)
			{
				// corrupt file
				AFERRLOG((L"AFilePackage::Open(), GuardBytes corrupted [%s]", szPckPath));
				return false;
			}

			//	Seek to entry list;
			m_fpPackageFile->seek(m_header.dwEntryOffset, AFILE_SEEK_SET);

			//	Create entries
			m_aFileEntries.resize(iNumFile);
			m_aFileEntryCache.resize(iNumFile);

			for (i = 0; i < iNumFile; i++)
			{
				FILEENTRY* pEntry = new FILEENTRY;
				if (!pEntry)
				{
					AFERRLOG((L"AFilePackage::Open(), Not enough memory!"));
					return false;
				}
				pEntry->iAccessCnt = 0;

				FILEENTRYCACHE* pEntryCache = new FILEENTRYCACHE;
				if (!pEntryCache)
				{
					AFERRLOG((L"AFilePackage::Open(), Not enough memory!"));
					return false;
				}
				std::memset(pEntryCache, 0, sizeof(FILEENTRYCACHE));

				// first read the entry size after compressed
				int32_t nCompressedSize;
				m_fpPackageFile->read(&nCompressedSize, sizeof(int32_t), 1);
				nCompressedSize ^= AFPCK_MASKDWORD;

				int32_t nCheckSize;
				m_fpPackageFile->read(&nCheckSize, sizeof(int32_t), 1);
				nCheckSize = nCheckSize ^ AFPCK_CHECKMASK ^ AFPCK_MASKDWORD;

				if (nCompressedSize != nCheckSize)
				{
					AFERRLOG((L"AFilePackage::Open(), Check Byte Error!"));
					return false;
				}

				pEntryCache->dwCompressedLength = nCompressedSize;
				pEntryCache->pEntryCompressed = new uint8_t[nCompressedSize];
				if (!pEntryCache->pEntryCompressed)
				{
					AFERRLOG((L"AFilePackage::Open(), Not enough memory !"));
					return false;
				}

				m_fpPackageFile->read(pEntryCache->pEntryCompressed, nCompressedSize, 1);
				uint32_t dwEntrySize = sizeof(FILEENTRY);

				if (dwEntrySize == nCompressedSize)
				{
					std::memcpy(pEntry, pEntryCache->pEntryCompressed, sizeof(FILEENTRY));

					// maybe the original package fileentry has not been compressed
					uint32_t dwCompressedSize = sizeof(FILEENTRY);
					uint8_t* pBuffer = new uint8_t[(sizeof(FILEENTRY))];
					int32_t nRet = Compress((LPBYTE)pEntry, sizeof(FILEENTRY), pBuffer, &dwCompressedSize);
					if (nRet != 0 || dwCompressedSize >= sizeof(FILEENTRY))
					{
						dwCompressedSize = sizeof(FILEENTRY);
						std::memcpy(pBuffer, pEntry, sizeof(FILEENTRY));
					}

					pEntryCache->dwCompressedLength = dwCompressedSize;
					pEntryCache->pEntryCompressed = (uint8_t*)realloc(pEntryCache->pEntryCompressed, dwCompressedSize);
					uint8_t* pNewBuffer = new uint8_t[dwCompressedSize];
					std::memcpy(pNewBuffer, pBuffer, dwCompressedSize);
					std::memcpy(pEntryCache->pEntryCompressed, pNewBuffer, dwCompressedSize);
					delete[] pNewBuffer;
					delete[] pBuffer;
					pNewBuffer = nullptr;					
					pBuffer = nullptr;
				}
				else
				{
					if (0 != Uncompress(pEntryCache->pEntryCompressed, nCompressedSize, (LPBYTE)pEntry, &dwEntrySize))
					{
						AFERRLOG((L"AFilePackage::Open(), decode file entry fail!"));
						return false;
					}

					assert(dwEntrySize == sizeof(FILEENTRY));
				}

				//	Note: A bug existed in AppendFileCompressed() after m_bUseShortName was introduced. The bug
				//		didn't normalize file name when new file is added to package, so that the szFileName of
				//		FILEENTRY may contain '/' character. The bug wasn't fixed until 2013.3.18, many 'new' files
				//		have been added to package, so NormalizeFileName is inserted here to ensure all szFileName
				//		of FILEENTRY uses '\' instead of '/', at least in memory.
				NormalizeFileName((std::wstring&)string_to_wstring(pEntry->szFileName), false);

				m_aFileEntries[i] = pEntry;
				m_aFileEntryCache[i] = pEntryCache;
			}

			ResortEntries();

			// now we move entry point to the end of the file so to keep old entries here
			if (m_bHasSafeHeader)
				m_header.dwEntryOffset = nOffset;
		}
		else
		{
			AFERRLOG((L"AFilePackage::Open(), Incorrect version!"));
			return false;
		}
		break;

	case CREATENEW:
		m_bReadOnly = false;
		m_fpPackageFile = new CPackageFile();

		if (!m_fpPackageFile->Open(szFullPckPath, L"wb"))
		{
			delete m_fpPackageFile;
			m_fpPackageFile = nullptr;

			AFERRLOG((L"AFilePackage::Open(), Can not create file [%s]", szFullPckPath));
			return false;
		}

		m_szPckFileName = szPckPath;

		CreateSafeHeader();

		// Init header
		ZeroMemory(&m_header, sizeof(FILEHEADER));
		m_header.guardByte0 = AFPCK_GUARDBYTE0;
		m_header.dwEntryOffset = sizeof(SAFEFILEHEADER);
		m_header.dwVersion = AFPCK_VERSION;
		m_header.dwFlags = bEncrypt ? PACKFLAG_ENCRYPT : 0;
		m_header.guardByte1 = AFPCK_GUARDBYTE1;
		std::strncpy(m_header.szDescription, AFPCK_COPYRIGHT_TAG, sizeof(m_header.szDescription));

		m_aFileEntries.clear();
		m_aFileEntryCache.clear();
		break;

	default:
		AFERRLOG((L"AFilePackage::Open(), Unknown open mode [%d]!", mode));
		return false;
	}

	m_mode = mode;
	m_bHasChanged = false;
	m_dwSharedSize = 0;
	m_dwCacheSize = 0;

	return true;
}

bool AFilePackage::RemoveFileFromDir(const std::wstring& filename)
{
	std::wstring normalizedFilename = filename;
	NormalizeFileName(normalizedFilename);

	size_t pos = normalizedFilename.find('\\');
	std::wstring tok = normalizedFilename.substr(0, pos);
	directory* dir = &m_directory;

	while (!tok.empty())
	{
		entry* ent = dir->SearchItem(tok);
		if (ent == nullptr)
			return false; // Entry not found

		size_t nextPos = normalizedFilename.find('\\', pos + 1);
		std::wstring next;
		if (nextPos != std::wstring::npos)
			next = normalizedFilename.substr(pos + 1, nextPos - pos - 1);
		else
			next = normalizedFilename.substr(pos + 1);

		if (next.empty())
		{
			if (!ent->IsContainer())
			{
				dir->RemoveItem(tok);
				return true;
			}
			return false;
		}
		else
		{
			if (ent->IsContainer())
				dir = static_cast<directory*>(ent);
			else
				return false;
		}

		tok = next;
		pos = nextPos;
	}

	return false;
}

bool AFilePackage::InsertFileToDir(const std::wstring& filename, int32_t index)
{
	std::wstring szFindName = filename;
	std::transform(szFindName.begin(), szFindName.end(), szFindName.begin(), ::towlower);

	std::replace(szFindName.begin(), szFindName.end(), L'/', L'\\');

	std::wstring token;
	std::wstring::size_type pos = 0;
	directory* dir = &m_directory;

	while ((pos = szFindName.find_first_of(L'\\')) != std::wstring::npos)
	{
		token = szFindName.substr(0, pos);
		szFindName.erase(0, pos + 1);

		entry* ent = dir->SearchItem(token.c_str());

		if (!ent)
		{
			directory* tmp = new directory(wstring_to_string(token));
			dir->AppendEntry(tmp);
			dir = tmp;
		}
		else
		{
			if (!ent->IsContainer())
			{
				AFERRLOG((L"AFilePackage::InsertFileToDir(), Directory conflict:%s", filename.c_str()));
				return false;
			}
			dir = dynamic_cast<directory*>(ent);
		}
	}

	if (!szFindName.empty())
	{
		entry* ent = dir->SearchItem(szFindName.c_str());
		if (!ent)
			dir->AppendEntry(new file(wstring_to_string(szFindName), index));
		else
		{
			if (ent->IsContainer())
				return false;
			else
				dynamic_cast<file*>(ent)->SetIndex(index);
		}
	}

	return true;
}

AFilePackage::CACHEFILENAME* AFilePackage::SearchCacheFileName(const std::wstring& szFileName)
{
	return nullptr;
}

AFilePackage::CACHEFILENAME* AFilePackage::SearchCacheFileName(uint32_t dwFileID)
{
	return nullptr;
}

#define ENTRY_BUFFER_SIZE (1024 * 1024)

bool AFilePackage::SaveEntries(uint32_t* pdwEntrySize)
{
	uint32_t dwTotalSize = 0;
	const uint32_t dwEntryHeaderSize = sizeof(uint32_t) * 2; // Size of compressed length and check size

	// Reserve space for the entry buffer
	std::vector<uint8_t> entryBuffer;
	entryBuffer.reserve(ENTRY_BUFFER_SIZE);

	// Rewrite file entries and file header
	m_fpPackageFile->seek(m_header.dwEntryOffset, AFILE_SEEK_SET);

	for (const auto& entryCache : m_aFileEntryCache)
	{
		// Ensure entry cache is valid
		if (!entryCache)
			continue;

		uint32_t dwCompressedSize = entryCache->dwCompressedLength;
		dwCompressedSize ^= AFPCK_MASKDWORD;
		entryBuffer.insert(entryBuffer.end(), reinterpret_cast<uint8_t*>(&dwCompressedSize),
			reinterpret_cast<uint8_t*>(&dwCompressedSize) + sizeof(uint32_t));

		dwCompressedSize ^= AFPCK_CHECKMASK;
		entryBuffer.insert(entryBuffer.end(), reinterpret_cast<uint8_t*>(&dwCompressedSize),
			reinterpret_cast<uint8_t*>(&dwCompressedSize) + sizeof(uint32_t));

		entryBuffer.insert(entryBuffer.end(), entryCache->pEntryCompressed,
			entryCache->pEntryCompressed + entryCache->dwCompressedLength);

		dwTotalSize += dwEntryHeaderSize + entryCache->dwCompressedLength;
	}

	// Write the entry buffer directly to the file
	m_fpPackageFile->write(entryBuffer.data(), entryBuffer.size(), 1);

	if (pdwEntrySize)
		*pdwEntrySize = dwTotalSize;

	return true;
}

void AFilePackage::Encrypt(LPBYTE pBuffer, uint32_t dwLength)
{	
	if ((m_header.dwFlags & PACKFLAG_ENCRYPT) == 0)
		return;

	uint32_t dwMask = dwLength + 0x739802ab;

	for (uint32_t i = 0; i < dwLength; i += 4)
	{
		if (i + 3 < dwLength)
		{
			uint32_t data = (pBuffer[i] << 24) | (pBuffer[i + 1] << 16) | (pBuffer[i + 2] << 8) | pBuffer[i + 3];
			data ^= dwMask;
			data = (data << 16) | ((data >> 16) & 0xffff);
			pBuffer[i] = (data >> 24) & 0xff;
			pBuffer[i + 1] = (data >> 16) & 0xff;
			pBuffer[i + 2] = (data >> 8) & 0xff;
			pBuffer[i + 3] = data & 0xff;
		}
	}
}

void AFilePackage::Decrypt(LPBYTE pBuffer, uint32_t dwLength)
{
	if ((m_header.dwFlags & PACKFLAG_ENCRYPT) == 0)
		return;

	uint32_t dwMask = dwLength + 0x739802ab;

	for (uint32_t i = 0; i < dwLength; i += 4)
	{
		if (i + 3 < dwLength)
		{
			uint32_t data = (pBuffer[i] << 24) | (pBuffer[i + 1] << 16) | (pBuffer[i + 2] << 8) | pBuffer[i + 3];
			data = (data << 16) | ((data >> 16) & 0xffff);
			data ^= dwMask;
			pBuffer[i] = (data >> 24) & 0xff;
			pBuffer[i + 1] = (data >> 16) & 0xff;
			pBuffer[i + 2] = (data >> 8) & 0xff;
			pBuffer[i + 3] = data & 0xff;
		}
	}
}

bool AFilePackage::LoadSafeHeader()
{
	m_fpPackageFile->seek(0, AFILE_SEEK_SET);
	m_fpPackageFile->read(&m_safeHeader, sizeof(SAFEFILEHEADER), 1);
	if (m_safeHeader.tag1 == 0x4DCA23EF && m_safeHeader.tag2 == 0x56A089B7)
	{
		m_bHasSafeHeader = true;
		m_fpPackageFile->Phase2Open(m_safeHeader.offset);
	}
	else
		m_bHasSafeHeader = false;

	m_fpPackageFile->seek(0, AFILE_SEEK_SET);

	return true;
}

bool AFilePackage::SaveSafeHeader()
{
	if (m_bHasSafeHeader)
	{
		m_fpPackageFile->seek(0, AFILE_SEEK_END);
		m_safeHeader.offset = (int32_t)m_fpPackageFile->tell();
		m_fpPackageFile->seek(0, AFILE_SEEK_SET);
		m_fpPackageFile->write(&m_safeHeader, sizeof(SAFEFILEHEADER), 1);
		m_fpPackageFile->seek(0, AFILE_SEEK_SET);
	}

	return true;
}

bool AFilePackage::CreateSafeHeader()
{
	m_bHasSafeHeader = true;
	m_safeHeader.tag1 = 0x4DCA23EF;
	m_safeHeader.tag2 = 0x56A089B7;
	m_safeHeader.offset = 0;

	return true;
}
