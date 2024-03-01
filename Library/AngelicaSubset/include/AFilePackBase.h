#ifndef _AFILEPACKBASE_H_
#define _AFILEPACKBASE_H_

#include "ABaseDef.h"
#include "AFile.h"

#include <fstream>

#define MAX_FILE_PACKAGE 0x7fffff00U

//#define AFPCK_VERSION 0x00010001
//#define AFPCK_VERSION 0x00010002  // Add compression
//#define AFPCK_VERSION 0x00010003  // The final release version on June 2002
//#define AFPCK_VERSION 0x00020001  // The version for element before Oct 2005
#define AFPCK_VERSION 0x00020002  // The version with safe header

#define AFPCK_COPYRIGHT_TAG "Angelica File Package, Perfect World Co. Ltd. 2002~2008. All Rights Reserved. "

class AFilePackBase
{
public:
	enum
	{
		PACKFLAG_ENCRYPT = 0x80000000,
	};

	class CPackageFile
	{
	public:
		CPackageFile();
		~CPackageFile();

		bool Open(const std::wstring& szFileName, const std::wstring szMode);
		bool Phase2Open(int64_t dwOffset);
		bool Close();

		int64_t read(void* buffer, int64_t size, int64_t count);
		int64_t write(const void* buffer, int64_t size, int64_t count);
		
		std::streampos tell();
		void seek(int64_t offset, AFILE_SEEK origin);
		void SetPackageFileSize(int64_t dwFileSize);

		int64_t GetPackageFileSize() const { return m_size1 + m_size2; }

	private:
		std::wstring m_szPath;
		std::wstring m_szPath2;
		std::wstring m_szMode;

		std::fstream m_fileStream1;
		std::fstream m_fileStream2;

		int64_t m_size1;
		int64_t m_size2;
		int64_t m_filePos;
	};

public:
	AFilePackBase() {}
	virtual ~AFilePackBase() {}

	virtual bool Close() = 0;
	virtual bool Flush() { return true; }
	virtual const std::wstring& GetFolder() const = 0;
	virtual bool IsFileExist(const std::wstring& szFileName) = 0;

	virtual void* OpenSharedFile(const std::wstring& szFileName, uint8_t** ppFileBuf, int64_t* pdwFileLen, bool bTempMem) = 0;
};

#endif
