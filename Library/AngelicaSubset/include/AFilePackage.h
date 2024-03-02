#ifndef _AFILEPACKAGE_H_
#define _AFILEPACKAGE_H_

#include "AFilePackBase.h"
#include "AFilePackMan.h"
#include "StringConvert.h"

#include <unordered_map>
#include <vector>
#include <mutex>

class AFilePackage : public AFilePackBase
{
public:
	class entry
	{
	public:
		std::string _name;

		entry() : _name() {}
		entry(const std::string& name) : _name(name) {}
		virtual ~entry() {}

		virtual bool IsContainer() = 0;
		virtual int32_t GetIndex() = 0;
		virtual entry* SearchItem(const std::wstring&) = 0;
	};

	class directory : public entry
	{
	public:
		directory(const std::string& name) : entry(name) {}
		directory() {}
		~directory();

		int32_t clear();
		virtual bool IsContainer() { return true; }
		virtual int32_t GetIndex() { return -1; }
		virtual entry* SearchItem(const std::wstring& item);

		entry* GetItem(int32_t index);
		size_t GetEntryCount() { return _list.size(); }
		int32_t RemoveItem(const std::wstring& item);
		int32_t AppendEntry(entry* item);
		int32_t SearchEntry(const std::wstring& filename);

	private:
		std::vector<entry*> _list;
		int32_t searchItemIndex(const std::wstring& item, int32_t* pos);
	};

	class file : public entry
	{
	public:
		file(const std::string& name, int32_t index) :entry(name), _index(index) {}
		virtual bool IsContainer() { return false; }
		virtual entry* SearchItem(const std::wstring& entry) { return nullptr; }
		virtual int32_t GetIndex() { return _index; }
		void SetIndex(int32_t index) { _index = index; }

	private:
		int32_t _index;
	};

	enum OPENMODE
	{
		OPENEXIST = 0,
		CREATENEW = 1
	};

	struct FILEENTRYCACHE
	{
		uint32_t dwCompressedLength; // The compressed file entry length
		uint8_t* pEntryCompressed;   // The compressed file entry data

		FILEENTRYCACHE() : dwCompressedLength(0), pEntryCompressed(nullptr) {}

		FILEENTRYCACHE(uint32_t compressedLength, uint8_t* entryCompressed)
			: dwCompressedLength(compressedLength)
			, pEntryCompressed(entryCompressed)
		{}
	};

	struct FILEENTRY
	{
		char szFileName[MAX_PATH];   // The file name of this entry; this may contain a path;
		uint32_t dwOffset;           // The offset from the beginning of the package file;
		uint32_t dwLength;           // The length of this file;
		uint32_t dwCompressedLength; // The compressed data length;

		int32_t iAccessCnt;          // Access counter used by OpenSharedFile

		FILEENTRY() : dwOffset(0), dwLength(0), dwCompressedLength(0), iAccessCnt(0)
		{
			std::memset(szFileName, 0, sizeof(szFileName));
		}

		FILEENTRY(const std::wstring& fileName, uint32_t offset, uint32_t length, uint32_t compressedLength, int32_t accessCnt)
			: dwOffset(offset)
			, dwLength(length)
			, dwCompressedLength(compressedLength)
			, iAccessCnt(accessCnt)
		{
			std::strncpy(szFileName, wstring_to_string(fileName).c_str(), sizeof(szFileName) - 1);
			szFileName[sizeof(szFileName) - 1] = '\0';
		}
	};

	struct FILEHEADER
	{
		uint32_t guardByte0;       // 0xabcdefab
		uint32_t dwVersion;        // Composed by two word version, major part and minor part;
		uint32_t dwEntryOffset;    // The entry list offset from the beginning;
		uint32_t dwFlags;          // package flags. the highest bit means the encrypt state;
		char szDescription[252];   // Description
		uint32_t guardByte1;       // 0xffeeffee

		FILEHEADER()
			: guardByte0(0xabcdefab)
			, dwVersion(0)
			, dwEntryOffset(0)
			, dwFlags(0)
			, guardByte1(0xffeeffee)
		{
			std::memset(szDescription, 0, sizeof(szDescription));
		}

		FILEHEADER(uint32_t guard0, uint32_t version, uint32_t entryOffset, uint32_t flags, const char* description, uint32_t guard1)
			: guardByte0(guard0)
			, dwVersion(version)
			, dwEntryOffset(entryOffset)
			, dwFlags(flags)
			, guardByte1(guard1)
		{
			std::strncpy(szDescription, description, sizeof(szDescription) - 1);
			szDescription[sizeof(szDescription) - 1] = '\0'; // Ensure null-termination
		}
	};

	struct SHAREDFILE
	{
		uint32_t dwFileID;         // File ID
		bool bCached;              // Cached flag
		bool bTempMem;             // true, use temporary memory alloctor
		int32_t iRefCnt;           // Reference counter
		uint8_t* pFileData;        // File data buffer
		uint32_t dwFileLen;        // File data length

		FILEENTRY* pFileEntry;     // Point to file entry

		SHAREDFILE()
			: dwFileID(0)
			, bCached(false)
			, bTempMem(false)
			, iRefCnt(0)
			, pFileData(nullptr)
			, dwFileLen(0)
			, pFileEntry(nullptr)
		{}

		SHAREDFILE(uint32_t fileId, bool cached, bool tempMem, int32_t refCount, uint8_t* fileData, uint32_t fileLen, FILEENTRY* fileEntry)
			: dwFileID(fileId)
			, bCached(cached)
			, bTempMem(tempMem)
			, iRefCnt(refCount)
			, pFileData(fileData)
			, dwFileLen(fileLen)
			, pFileEntry(fileEntry)
		{}
	};

	struct CACHEFILENAME
	{
		std::wstring strFileName;  // File name
		uint32_t dwFileID;         // File ID

		CACHEFILENAME() : dwFileID(0) {}

		CACHEFILENAME(std::wstring fileName, uint32_t fileId)
			: strFileName(fileName)
			, dwFileID(fileId)
		{}
	};

	struct SAFEFILEHEADER
	{
		uint32_t tag1;              // tag1 of safe header, current it is 0x4DCA23EF
		uint32_t offset;            // offset of real entries
		uint32_t tag2;              // tag2 of safe header, current it is 0x56A089B7

		SAFEFILEHEADER()
			: tag1(0x4DCA23EF)
			, offset(0)
			, tag2(0x56A089B7)
		{}

		SAFEFILEHEADER(uint32_t t1, uint32_t off, uint32_t t2)
			: tag1(t1)
			, offset(off)
			, tag2(t2)
		{}
	};

	typedef std::unordered_map<SHAREDFILE*, int32_t> SharedTable;
	typedef std::unordered_map<CACHEFILENAME*, int32_t> CachedTable;

	friend class AFilePackMan;
	friend class AFilePackGame;

	AFilePackage();
	virtual ~AFilePackage();

	bool Open(const std::wstring& szPckPath, OPENMODE mode, bool bEncrypt = false);
	bool Open(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt = false);
	virtual bool Close();

	bool AppendFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t dwFileLength, bool bCompress);
	bool AppendFileCompressed(const std::wstring& szFileName, LPBYTE pCompressedBuffer, uint32_t dwFileLength, uint32_t dwCompressedLength);

	bool RemoveFile(const std::wstring& szFileName);

	bool ReplaceAFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t dwFileLength, bool bCompress);
	bool ReplaceFileCompressed(const std::wstring& szFileName, LPBYTE pCompressedBuffer, uint32_t dwFileLength, uint32_t dwCompressedLength);

	bool ResortEntries();

	bool ReadFile(const std::wstring& szFileName, LPBYTE pFileBuffer, size_t* pdwBufferLen);
	bool ReadFile(FILEENTRY& fileEntry, LPBYTE pFileBuffer, size_t* pdwBufferLen);

	bool ReadCompressedFile(const std::wstring& szFileName, LPBYTE pCompressedBuffer, size_t* pdwBufferLen);
	bool ReadCompressedFile(FILEENTRY& fileEntry, LPBYTE pCompressedBuffer, size_t* pdwBufferLen);

	bool GetFileEntry(const std::string& szFileName, FILEENTRY* pFileEntry, int32_t* pnIndex = nullptr);
	const FILEENTRY* GetFileEntryByIndex(int nIndex) const { return m_aFileEntries[nIndex]; }

	directory* GetDirEntry(const std::wstring& szPath);

	void ClearFileCache();
	virtual void* OpenSharedFile(const std::wstring& szFileName, uint8_t** ppFileBuf, size_t* pdwFileLen, bool bTempMem);
	virtual void CloseSharedFile(void* dwFileHandle);

	uint32_t GetCachedFileSize() const { return m_dwCacheSize; }
	uint32_t GetSharedFileSize() const { return m_dwSharedSize; }

	size_t GetFileNumber() const { return m_aFileEntries.size(); }
	FILEHEADER GetFileHeader() const { return m_header; }
	virtual const std::wstring& GetFolder() const { return m_szFolder; }
	const std::wstring& GetPckFileName() { return m_szPckFileName; }
	virtual bool IsFileExist(const std::wstring& szFileName);

	int64_t GetPackageFileSize() { return m_fpPackageFile->GetPackageFileSize(); }

	static int32_t Compress(LPBYTE pFileBuffer, uint32_t dwFileLength, LPBYTE pCompressedBuffer, uint32_t* pdwCompressedLength);
	static int32_t Uncompress(LPBYTE pCompressedBuffer, uint32_t dwCompressedLength, LPBYTE pFileBuffer, uint32_t* pdwFileLength);

	bool AddCacheFileNameList(const std::wstring& szDescFile);
	bool AddCacheFileName(const std::wstring& szFile);

protected:
	static bool NormalizeFileName(std::wstring& szFileName);
	bool NormalizeFileName(std::wstring& szFileName, bool bUseShortName);
	void GetRidOfFolder(const std::wstring& szInName, std::wstring& szOutName);

	bool InnerOpen(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt, bool bShortName);

	bool RemoveFileFromDir(const std::wstring& filename);
	bool InsertFileToDir(const std::wstring& filename, int32_t index);

	CACHEFILENAME* SearchCacheFileName(const std::wstring& szFileName);
	CACHEFILENAME* SearchCacheFileName(uint32_t dwFileID);

	bool SaveEntries(uint32_t* pdwEntrySize = nullptr);
	void Encrypt(LPBYTE pBuffer, uint32_t dwLength);
	void Decrypt(LPBYTE pBuffer, uint32_t dwLength);

	bool LoadSafeHeader();
	bool SaveSafeHeader();
	bool CreateSafeHeader();

private:
	bool m_bHasChanged;
	bool m_bReadOnly;
	bool m_bUseShortName;

	FILEHEADER m_header;
	OPENMODE m_mode;

	std::vector<FILEENTRY*>	m_aFileEntries;
	std::vector<FILEENTRYCACHE*> m_aFileEntryCache;
	CachedTable m_CachedFileTab;
	SharedTable m_SharedFileTab;

	CPackageFile* m_fpPackageFile;
	std::wstring m_szPckFileName;
	std::wstring m_szFolder;

	directory m_directory;
	uint32_t m_dwCacheSize;
	uint32_t m_dwSharedSize;

	bool m_bHasSafeHeader;
	SAFEFILEHEADER m_safeHeader;
};

extern AFilePackMan g_AFilePackMan;

#endif
