#ifndef _AFILEPACKGAME_H_
#define _AFILEPACKGAME_H_

#include "AFilePackBase.h"

#include <unordered_map>
#include <mutex>

class AFilePackGame : public AFilePackBase
{
public:
	enum OPENMODE
	{
		OPENEXIST = 0,
		CREATENEW = 1
	};

	struct FILEENTRY
	{
		std::wstring szFileName;     // The file name of this entry; this may contain a path;
		uint32_t dwOffset;           // The offset from the beginning of the package file;
		uint32_t dwLength;           // The length of this file;
		uint32_t dwCompressedLength; // The compressed data length;
	};

	struct FILEHEADER
	{
		uint32_t guardByte0;         // 0xABCDEFAB
		uint32_t dwVersion;          // Composed by two word version, major part and minor part;
		uint32_t dwEntryOffset;      // The entry list offset from the beginning;
		uint32_t dwFlags;            // package flags. the highest bit means the encrypt state;
		char szDescription[252];     // Description
		uint32_t guardByte1;         // 0xFFEEFFEE
	};

	struct SHAREDFILE
	{
		uint32_t dwFileID;           // File ID
		bool bCached;                // Cached flag
		bool bTempMem;               // true, use temporary memory alloctor
		int32_t iRefCnt;             // Reference counter
		uint8_t* pFileData;          // File data buffer
		uint32_t dwFileLen;          // File data length

		FILEENTRY* pFileEntry;       // Point to file entry
	};

	struct CACHEFILENAME
	{
		std::wstring strFileName;    // File name
		uint32_t dwFileID;           // File ID
	};

	struct SAFEFILEHEADER
	{
		uint32_t tag1;               // tag1 of safe header, current it is 0x4DCA23EF
		uint32_t offset;             // offset of real entries
		uint32_t tag2;               // tag2 of safe header, current it is 0x56a089b7
	};

	struct NAMEBUFFER
	{
		char* pBuffer;
		uint32_t dwLength;           // Buffer length
		uint32_t dwOffset;           // Current offset
	};

	friend class AFilePackMan;

	//using FileEntryTable = std::unordered_map<int, FILEENTRY*>;

	AFilePackGame();
	virtual ~AFilePackGame();

	bool Open(const std::wstring& szPckPath, OPENMODE mode, bool bEncrypt = false);
	bool Open(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt = false);
	virtual bool Close();

	bool ResortEntries();

	bool ReadFile(const std::wstring& szFileName, LPBYTE pFileBuffer, uint32_t* pdwBufferLen);
	bool ReadFile(FILEENTRY& fileEntry, LPBYTE pFileBuffer, uint32_t* pdwBufferLen);
	bool ReadCompressedFile(FILEENTRY& fileEntry, LPBYTE pCompressedBuffer, size_t* pdwBufferLen);

	FILEENTRY* GetFileEntry(const std::wstring& szFileName);
	const FILEENTRY* GetFileEntryByIndex(int32_t nIndex) const { return &m_aFileEntries[nIndex]; }
	FILEENTRY* FindIDCollisionFile(const std::wstring& szFileName);

	virtual void* OpenSharedFile(const std::wstring& szFileName, uint8_t** ppFileBuf, size_t* pdwFileLen, bool bTempMem);
	virtual void CloseSharedFile(void* dwFileHandle);

	int32_t GetFileNumber() const { return m_iNumEntry; }
	FILEHEADER GetFileHeader() const { return m_header; }
	virtual const std::wstring& GetFolder() const { return m_szFolder; }
	const std::wstring& GetPckFileName() { return m_szPckFileName; }
	virtual bool IsFileExist(const std::wstring& szFileName);

	int64_t GetPackageFileSize() { return m_fpPackageFile->GetPackageFileSize(); }

protected:
	bool NormalizeFileName(std::wstring& szFileName, bool bUseShortName);
	void GetRidOfFolder(const std::wstring& szInName, std::wstring& szOutName);

	bool InnerOpen(const std::wstring& szPckPath, const std::wstring& szFolder, OPENMODE mode, bool bEncrypt, bool bShortName);

	void Encrypt(LPBYTE pBuffer, uint32_t dwLength);
	void Decrypt(LPBYTE pBuffer, uint32_t dwLength);

	bool LoadSafeHeader();

	std::wstring AllocFileName(const std::wstring& szFile, int32_t iEntryCnt, int32_t iEntryTotalNum);

private:
	bool m_bReadOnly;
	bool m_bUseShortName;

	FILEHEADER m_header;
	OPENMODE m_mode;

	FILEENTRY* m_aFileEntries;
	int32_t m_iNumEntry;
	std::unordered_map<int, FILEENTRY*> m_FileQuickSearchTab;
	std::vector<FILEENTRY*> m_aIDCollisionFiles;
	std::vector<NAMEBUFFER> m_aNameBufs;

	CPackageFile* m_fpPackageFile;
	std::wstring m_szPckFileName;
	std::wstring m_szFolder;

	bool m_bHasSafeHeader;
	SAFEFILEHEADER m_safeHeader;
};

#endif
