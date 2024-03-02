#ifndef _AFILEIMAGE_H_
#define _AFILEIMAGE_H_

#include "AFile.h"
#include "AFilePackBase.h"

class AFileImage : public AFile
{
public:
	AFileImage();
	virtual ~AFileImage();

	virtual bool Open(const std::wstring& szFullPath, uint32_t dwFlags);
	virtual bool Open(const std::wstring& szFolderName, const std::wstring& szFileName, uint32_t dwFlags);
	virtual bool Close();

	virtual bool Read(LPVOID pBuffer, uint32_t dwBufferLength, uint32_t* pReadLength);
	virtual bool Write(LPVOID pBuffer, uint32_t dwBufferLength, uint32_t* pWriteLength);

	virtual bool ReadLine(char* szLineBuffer, uint32_t dwBufferLength, size_t* pdwReadLength);
	virtual bool WriteLine(const char* szLineBuffer);

	virtual bool ReadString(char* szLineBuffer, uint32_t dwBufferLength, uint32_t* pdwReadLength);
	virtual bool WriteString(const std::string& s);
	virtual bool ReadString(std::string& s);

	virtual uint32_t Tell() const;
	virtual bool Seek(int32_t iOffset, AFILE_SEEK origin);
	virtual bool ResetFilePointer();

	virtual uint32_t GetFileLength() const { return (uint32_t)m_nFileLength; }

	LPBYTE GetFileBuffer() { return m_pFileImage; }

protected:
	bool Init(const std::wstring& szFullName, bool bTempMem);
	bool Release();

private:
	AFilePackBase* m_pPackage; // package object this file image open with
	LPBYTE m_pFileImage;       // Memory pointer of the file image in memory;
	int32_t m_nCurPtr;         // In index into the file image buffer;
	int32_t m_nFileLength;     // File length;
	void* m_dwHandle;          // Handle in file package
	bool m_bTempMem;           // true, use temporary memory alloctor

	bool fimg_read(LPBYTE pBuffer, int32_t nSize, int32_t* pReadSize);
	bool fimg_read_line(char* szLineBuffer, int32_t nMaxLength, size_t* pReadSize);
	bool fimg_seek(int32_t nOffset, int32_t startPos);
};

#endif
