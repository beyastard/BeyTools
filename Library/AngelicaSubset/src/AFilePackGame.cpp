#include "AFilePackGame.h"
#include "AFilePackage.h"
#include "AFI.h"
#include "AFPI.h"
#include "AAssist.h"
#include "MutexGuard.h"
#include "zlib.h"

#include <algorithm>
#include <codecvt>

extern uint32_t AFPCK_GUARDBYTE0;
extern uint32_t AFPCK_GUARDBYTE1;
extern uint32_t AFPCK_MASKDWORD;
extern uint32_t AFPCK_CHECKMASK;

struct FILEENTRY_INFILE
{
	std::wstring szFileName;        // The file name of this entry; this may contain a path;
	uint32_t dwOffset;               // The offset from the beginning of the package file;
	uint32_t dwLength;               // The length of this file;
	uint32_t dwCompressedLength;     // The compressed data length;
	int32_t iAccessCnt;              // Access counter used by OpenSharedFile

	FILEENTRY_INFILE()
		: szFileName(L"")
		, dwOffset(0)
		, dwLength(0)
		, dwCompressedLength(0)
		, iAccessCnt(0)
	{}

	FILEENTRY_INFILE(const std::wstring& fileName, uint32_t offset, uint32_t length, uint32_t compressedLength, int32_t accessCount)
		: szFileName(fileName)
		, dwOffset(offset)
		, dwLength(length)
		, dwCompressedLength(compressedLength)
		, iAccessCnt(accessCount)
	{}
};

extern int32_t _CacheFileNameCompare(const void* arg1, const void* arg2);
extern void* Zlib_User_Alloc(void* opaque, uint32_t items, uint32_t size);
extern void Zlib_User_Free(void* opaque, void* ptr);
extern int32_t Zlib_Compress(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen, int32_t level);
extern int32_t Zlib_UnCompress(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen);

AFilePackGame::AFilePackGame()
	: m_bReadOnly(false)
	, m_bUseShortName(false)
	, m_mode(OPENEXIST)
	, m_aFileEntries(nullptr)
	, m_iNumEntry(0)
	, m_FileQuickSearchTab(2048)
	, m_fpPackageFile(nullptr)
	, m_bHasSafeHeader(false)
{
	std::memset(&m_header, 0, sizeof(FILEHEADER));
	std::memset(&m_safeHeader, 0, sizeof(SAFEFILEHEADER));
}

AFilePackGame::~AFilePackGame() {}

bool AFilePackGame::Open(const std::wstring& szPckPath, OPENMODE mode, bool bEncrypt)
{
	std::wstring szFolder = szPckPath;

	if (szFolder.empty())
	{
		AFERRLOG((L"AFilePackGame::Open(), can not open a null or empty file name!"));
		return false;
	}

	// Find the last occurrence of '\' to determine the folder path
	size_t pos = szFolder.find_last_of(L'\\');
	if (pos == std::wstring::npos)
	{
		AFERRLOG((L"AFilePackGame::Open(), only file with extension can be opened!"));
		return false;
	}

	// Extract the folder path
	szFolder = szFolder.substr(0, pos + 1);

	return InnerOpen(szPckPath, szFolder, mode, bEncrypt, false);
}

bool AFilePackGame::Open(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt)
{
	return InnerOpen(szPckPath, szFolder, mode, bEncrypt, true);
}

bool AFilePackGame::Close()
{
	// Close and delete the package file object
	if (m_fpPackageFile)
	{
		delete m_fpPackageFile;
		m_fpPackageFile = nullptr;
	}

	// Release memory for file entries
	delete[] m_aFileEntries;
	m_aFileEntries = nullptr;

	// Clear quick search table and ID collision files vector
	m_FileQuickSearchTab.clear();
	m_aIDCollisionFiles.clear();

	// Release memory for file names
	for (auto& info : m_aNameBufs)
	{
		delete[] info.pBuffer;
		info.pBuffer = nullptr;
	}

	m_aNameBufs.clear();

	return true;
}

bool AFilePackGame::ResortEntries()
{
	m_FileQuickSearchTab.clear();

	for (int32_t i = 0; i < m_iNumEntry; i++)
	{
		FILEENTRY* pFileEntry = &m_aFileEntries[i];
		uint32_t idFile = a_MakeIDFromLowString(pFileEntry->szFileName);

		std::unordered_map<int, FILEENTRY*>::const_iterator itr = m_FileQuickSearchTab.find((int32_t)idFile);
		if (itr == m_FileQuickSearchTab.end())
			m_FileQuickSearchTab[(int32_t)idFile] = pFileEntry;
		else
		{
			// ID already exist, is there a ID collision ?
			FILEENTRY* pCheckEntry = itr->second;
			if (pCheckEntry->szFileName.compare(pFileEntry->szFileName) != 0)
			{
				// id collision, add file to candidate array
				//ASSERT(0 && "ID collision");
				m_aIDCollisionFiles.push_back(pFileEntry);
			}
			else
			{
				// Same file was added twice ?!! Shouldn't happen !!
				assert(0 && "Same file was added twice !!");
				return false;
			}
		}
	}

	return true;
}

bool AFilePackGame::ReadFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t* pdwBufferLen)
{
	FILEENTRY* pFileEntry = GetFileEntry(szFileName);
	if (!pFileEntry)
	{
		AFERRLOG((L"AFilePackage::ReadFile(), Can not find file entry [%s]!", szFileName));
		return false;
	}

	return ReadFile(*pFileEntry, pFileBuffer, pdwBufferLen);
}

bool AFilePackGame::ReadFile(FILEENTRY& fileEntry, LPBYTE pFileBuffer, uint32_t* pdwBufferLen)
{
	if (*pdwBufferLen < fileEntry.dwLength)
	{
		AFERRLOG((L"AFilePackGame::ReadFile(), Buffer is too small!"));
		return false;
	}

	// We can automaticly determine whether compression has been used;
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
		
		if (AFilePackage::Uncompress(pBuffer, fileEntry.dwCompressedLength, pFileBuffer, &dwFileLength) != 0)
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

bool AFilePackGame::ReadCompressedFile(FILEENTRY& fileEntry, LPBYTE pCompressedBuffer, size_t* pdwBufferLen)
{
	if (*pdwBufferLen < fileEntry.dwCompressedLength)
	{
		AFERRLOG((L"AFilePackGame::ReadCompressedFile(), Buffer is too small!"));
		return false;
	}

	std::mutex mtx;
	{
		MutexGuard guard(mtx);
		m_fpPackageFile->seek(fileEntry.dwOffset, AFILE_SEEK_SET);
		*pdwBufferLen = m_fpPackageFile->read(pCompressedBuffer, 1, (uint32_t)fileEntry.dwCompressedLength);
		Decrypt(pCompressedBuffer, fileEntry.dwCompressedLength);
	}
	
	return true;
}

AFilePackGame::FILEENTRY* AFilePackGame::GetFileEntry(const std::wstring& szFileName)
{
	assert(static_cast<int32_t>(m_FileQuickSearchTab.size() + m_aIDCollisionFiles.size()) == m_iNumEntry);

	// Normalize file name
	std::wstring szFindName = szFileName;
	NormalizeFileName(szFindName, m_bUseShortName);

	uint32_t idFile = a_MakeIDFromLowString(szFindName);
	if (idFile == 0)
	{
		assert(false && "Failed to generate file id");
		AFERRLOG((L"AFilePackGame::GetFileEntry, failed to generate file id for [%s%s] !", m_szFolder.c_str(), szFindName.c_str()));
		return nullptr;
	}

	auto itr = m_FileQuickSearchTab.find(idFile);
	if (itr != m_FileQuickSearchTab.end())
	{
		FILEENTRY* pFileEntry = itr->second;

		// Check if filenames match
		if (szFindName == pFileEntry->szFileName)
			return pFileEntry;

		// If filenames don't match, perform additional checks before searching for collisions
		if (szFindName.length() != pFileEntry->szFileName.length())
			return nullptr; // Lengths don't match, so they are definitely different files

		// Lengths match, compare filenames character by character
		for (size_t i = 0; i < szFindName.length(); ++i)
		{
			if (szFindName[i] != pFileEntry->szFileName[i])
			{
				// Filenames differ at this position, indicating a collision
				pFileEntry = FindIDCollisionFile(szFindName);
				return pFileEntry;
			}
		}

		// All characters match, which should not happen for different files
		assert(false && "Unexpected filename match");
		return nullptr;
	}
	else
		return nullptr; // Entry not found in the quick search table
}

AFilePackGame::FILEENTRY* AFilePackGame::FindIDCollisionFile(const std::wstring& szFileName)
{
	for (size_t i = 0; i < m_aIDCollisionFiles.size(); i++)
	{
		FILEENTRY* pFileEntry = m_aIDCollisionFiles[i];
		if (szFileName == pFileEntry->szFileName)
			return pFileEntry;
	}

	return nullptr;
}

void* AFilePackGame::OpenSharedFile(const std::wstring& szFileName, uint8_t** ppFileBuf, size_t* pdwFileLen, bool bTempMem)
{
	FILEENTRY* pFileEntry = GetFileEntry(szFileName);
	if (!pFileEntry)
	{
		if (szFileName.find(L"Textures") == std::wstring::npos && szFileName.find(L"Tex_") == std::wstring::npos)
		{
			AFERRLOG((L"AFilePackGame::OpenSharedFile, Failed to find file [%s] in package !", szFileName));
		}

		return nullptr;
	}

	// Read file data
	uint32_t dwFileLen = pFileEntry->dwLength;
	uint8_t* pFileData = nullptr;
	if (bTempMem)
	{
		pFileData = new uint8_t[dwFileLen];
		if (!ReadFile(*pFileEntry, pFileData, &dwFileLen))
		{
			delete[] pFileData;
			pFileData = nullptr;
			AFERRLOG((L"AFilePackGame::OpenSharedFile, Failed to read file data [%s] !", szFileName.c_str()));
			return nullptr;
		}
	}
	else
	{
		// Just retrieve the file length
		if (!ReadFile(*pFileEntry, nullptr, &dwFileLen))
		{
			AFERRLOG((L"AFilePackGame::OpenSharedFile, Failed to retrieve file length for [%s] !", szFileName.c_str()));
			return nullptr;
		}
	}

	// Create shared file item
	SHAREDFILE* pFileItem = new SHAREDFILE;
	if (!pFileItem)
	{
		if (bTempMem)
		{
			delete[] pFileData;
			pFileData = nullptr;
		}

		AFERRLOG((L"AFilePackGame::OpenSharedFile, Not enough memory!"));
		return nullptr;
	}

	pFileItem->bCached = false;
	pFileItem->bTempMem = bTempMem;
	pFileItem->dwFileID = 0;
	pFileItem->dwFileLen = dwFileLen;
	pFileItem->iRefCnt = 1;
	pFileItem->pFileData = pFileData;
	pFileItem->pFileEntry = pFileEntry;

	*ppFileBuf = pFileData;
	*pdwFileLen = dwFileLen;

	return pFileItem;
}

void AFilePackGame::CloseSharedFile(void* dwFileHandle)
{
	SHAREDFILE* pFileItem = (SHAREDFILE*)dwFileHandle;
	assert(pFileItem && pFileItem->iRefCnt > 0);

	// No cache file, release it
	if (pFileItem->bTempMem)
	{
		delete[] pFileItem->pFileData;
		pFileItem->pFileData = nullptr;
	}

	delete pFileItem;
	pFileItem = nullptr;
}

bool AFilePackGame::IsFileExist(const std::wstring& szFileName)
{
	return GetFileEntry(szFileName) ? true : false;
}

bool AFilePackGame::NormalizeFileName(std::wstring& szFileName, bool bUseShortName)
{
	if (!AFilePackage::NormalizeFileName(szFileName))
		return false;

	if (bUseShortName)
	{
		std::wstring strFileName(szFileName.begin(), szFileName.end());
		std::wstring strFullName;

		GetRidOfFolder(strFileName, strFullName);
		szFileName = std::wstring(strFullName.begin(), strFullName.end());
	}

	return true;
}

void AFilePackGame::GetRidOfFolder(const std::wstring& szInName, std::wstring& szOutName)
{
	af_GetRelativePathNoBase(szInName, m_szFolder, szOutName);
}

bool AFilePackGame::InnerOpen(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt, bool bShortName)
{
	std::wstring szFullPckPath;
	af_GetFullPath(szFullPckPath, szPckPath);

	m_bUseShortName = bShortName;

	// Save folder name
	assert(!szFolder.empty());
	std::wstring normalizedFolder = szFolder;
	std::transform(normalizedFolder.begin(), normalizedFolder.end(), normalizedFolder.begin(), ::towlower);
	AFilePackage::NormalizeFileName(normalizedFolder);

	// Add '\\' at folder tail if needed
	if (!normalizedFolder.empty() && normalizedFolder.back() != L'\\')
		normalizedFolder.push_back(L'\\');

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

				AFERRLOG((L"AFilePackGame::Open(), Can not open file [%s]", szFullPckPath.c_str()));
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
			nOffset = static_cast<int32_t>(m_safeHeader.offset);

		// Now analyze the file entries of the package;
		uint32_t dwVersion;

		// First version;
		m_fpPackageFile->seek(nOffset - sizeof(uint32_t), AFILE_SEEK_SET);
		m_fpPackageFile->read(&dwVersion, sizeof(uint32_t), 1);

		if (dwVersion == 0x00020002 || dwVersion == 0x00020001)
		{
			int32_t i, iNumFile;

			// Now read file number;
			m_fpPackageFile->seek(nOffset - (sizeof(int32_t) + sizeof(uint32_t)), AFILE_SEEK_SET);
			m_fpPackageFile->read(&iNumFile, sizeof(int32_t), 1);
			m_fpPackageFile->seek(nOffset - (sizeof(FILEHEADER) + sizeof(uint32_t) + sizeof(int)), AFILE_SEEK_SET);
			m_fpPackageFile->read(&m_header, sizeof(FILEHEADER), 1);
			if (std::strstr(m_header.szDescription, "lica File Package") == nullptr)
				return false;

			std::strncpy(m_header.szDescription, AFPCK_COPYRIGHT_TAG, sizeof(m_header.szDescription));

			// if we don't expect one encrypted package, we will let the error come out.
			// make sure the encrypt flag is correct
			bool bPackIsEncrypt = (m_header.dwFlags & PACKFLAG_ENCRYPT) != 0;
			if (bEncrypt != bPackIsEncrypt)
			{
				AFERRLOG((L"AFilePackage::Open(), wrong encrypt flag"));
				return false;
			}

			m_header.dwEntryOffset ^= AFPCK_MASKDWORD;

			if (m_header.guardByte0 != AFPCK_GUARDBYTE0 || m_header.guardByte1 != AFPCK_GUARDBYTE1)
			{
				// corrupt file
				AFERRLOG((L"AFilePackGame::Open(), GuardBytes corrupted [%s]", szPckPath.c_str()));
				return false;
			}

			// Seek to entry list;
			m_fpPackageFile->seek(m_header.dwEntryOffset, AFILE_SEEK_SET);

			// Create entries
			m_aFileEntries = new FILEENTRY[iNumFile];
			if (!m_aFileEntries)
			{
				AFERRLOG((L"AFilePackGame::Open(), Not enough memory for entries [%s]", szPckPath.c_str()));
				return false;
			}

			std::memset(m_aFileEntries, 0, sizeof(FILEENTRY) * iNumFile);

			m_iNumEntry = iNumFile;

			for (i = 0; i < iNumFile; i++)
			{
				FILEENTRY* pEntry = &m_aFileEntries[i];

				FILEENTRY_INFILE tempEntry;

				// first read the entry size after compressed
				int32_t nCompressedSize;
				m_fpPackageFile->read(&nCompressedSize, sizeof(int32_t), 1);
				nCompressedSize ^= AFPCK_MASKDWORD;

				int32_t nCheckSize;
				m_fpPackageFile->read(&nCheckSize, sizeof(int32_t), 1);
				nCheckSize = nCheckSize ^ AFPCK_CHECKMASK ^ AFPCK_MASKDWORD;

				if (nCompressedSize != nCheckSize)
				{
					AFERRLOG((L"AFilePackGame::Open(), Check Byte Error!"));
					return false;
				}

				uint8_t* pEntryCompressed = new uint8_t[nCompressedSize];
				if (!pEntryCompressed)
				{
					AFERRLOG((L"AFilePackGame::Open(), Not enough memory !"));
					return false;
				}

				m_fpPackageFile->read(pEntryCompressed, nCompressedSize, 1);
				uint32_t dwEntrySize = sizeof(FILEENTRY_INFILE);

				if (dwEntrySize == nCompressedSize)
					std::memcpy(&tempEntry, pEntryCompressed, sizeof(FILEENTRY_INFILE));
				else
				{
					if (0 != AFilePackage::Uncompress(pEntryCompressed, nCompressedSize, (LPBYTE)&tempEntry, &dwEntrySize))
					{
						delete pEntryCompressed;
						pEntryCompressed = nullptr;
						AFERRLOG((L"AFilePackGame::Open(), decode file entry fail!"));
						return false;
					}

					assert(dwEntrySize == sizeof(FILEENTRY_INFILE));
				}

				delete pEntryCompressed;
				pEntryCompressed = nullptr;

				// Note: A bug existed in AppendFileCompressed() after m_bUseShortName was introduced. The bug
				// didn't normalize file name when a new file is added to the package, so that the szFileName of
				// FILEENTRY may contain '/' character. The bug wasn't fixed until 2013.3.18, many 'new' files
				// have been added to the package, so NormalizeFileName is inserted here to ensure all szFileName
				// of FILEENTRY uses '\' instead of '/', at least in memory.
				NormalizeFileName(tempEntry.szFileName, false);

				// Duplicate entry info
				pEntry->szFileName = AllocFileName(tempEntry.szFileName, i, iNumFile);
				pEntry->dwLength = tempEntry.dwLength;
				pEntry->dwCompressedLength = tempEntry.dwCompressedLength;
				pEntry->dwOffset = tempEntry.dwOffset;
			}

			ResortEntries();

			// now we move entry point to the end of the file so to keep old entries here
			if (m_bHasSafeHeader)
				m_header.dwEntryOffset = nOffset;
		}
		else
		{
			AFERRLOG((L"AFilePackGame::Open(), Incorrect version!"));
			return false;
		}
		break;

	default:
		AFERRLOG((L"AFilePackGame::Open(), Unknown open mode [%d]!", mode));
		return false;
	}

	m_mode = mode;

	return true;
}

void AFilePackGame::Encrypt(LPBYTE pBuffer, uint32_t dwLength)
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

void AFilePackGame::Decrypt(LPBYTE pBuffer, uint32_t dwLength)
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

bool AFilePackGame::LoadSafeHeader()
{
	m_fpPackageFile->seek(0, AFILE_SEEK_SET);
	m_bHasSafeHeader = false; // Assume no safe header initially

	m_fpPackageFile->read(&m_safeHeader, sizeof(m_safeHeader), 1);

	if (m_safeHeader.tag1 == 0x4DCA23EF && m_safeHeader.tag2 == 0x56A089B7)
	{
		m_bHasSafeHeader = true;
		m_fpPackageFile->Phase2Open(m_safeHeader.offset);
	}

	return true;
}

std::wstring AFilePackGame::AllocFileName(const std::wstring& szFile, int32_t iEntryCnt, int32_t iEntryTotalNum)
{
	assert(!szFile.empty());

	// Convert std::wstring to UTF-8
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string utf8FileName = converter.to_bytes(szFile);

	size_t nameLen = utf8FileName.length() + 1; // Including null terminator
	assert(nameLen < MAX_PATH);

	bool bAllocNewBuffer = false;
	NAMEBUFFER* pBufInfo = nullptr;

	if (m_aNameBufs.empty())
		bAllocNewBuffer = true;
	else
	{
		pBufInfo = &m_aNameBufs.back();
		if (pBufInfo->dwOffset + nameLen > pBufInfo->dwLength)
			bAllocNewBuffer = true;
	}

	if (bAllocNewBuffer)
	{
		size_t bufferSize = (iEntryTotalNum - iEntryCnt) * 32;
		if (bufferSize < nameLen)
			bufferSize = nameLen * 10;

		char* pBuffer = new char[bufferSize];
		if (!pBuffer)
			throw std::bad_alloc(); // or handle the memory allocation failure appropriately

		NAMEBUFFER info;
		info.dwLength = uint32_t(bufferSize);
		info.pBuffer = pBuffer;
		info.dwOffset = 0;
		m_aNameBufs.push_back(info);

		pBufInfo = &m_aNameBufs.back();
	}

	assert(pBufInfo);

	char* pCurPos = pBufInfo->pBuffer + pBufInfo->dwOffset;
	std::strncpy(pCurPos, utf8FileName.c_str(), nameLen);
	pCurPos[nameLen - 1] = '\0';
	pBufInfo->dwOffset += uint32_t(nameLen);

	// Convert back to std::wstring
	std::wstring convertedFileName = converter.from_bytes(pCurPos);
	return convertedFileName;
}
