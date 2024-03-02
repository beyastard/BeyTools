#ifndef _AFILE_H_
#define _AFILE_H_

#include <fstream>

#define AFILE_TYPE_BINARY 0x42584f4d // MOXT
#define AFILE_TYPE_TEXT   0x54584f4d // MOXB

#define AFILE_OPENEXIST   0x00000001
#define AFILE_CREATENEW   0x00000002
#define AFILE_OPENAPPEND  0x00000004
#define AFILE_TEXT        0x00000008
#define AFILE_BINARY      0x00000010
#define AFILE_NOHEAD      0x00000020
#define AFILE_TEMPMEMORY  0x00000040

#define AFILE_LINEMAXLEN  2048

enum AFILE_SEEK
{
    AFILE_SEEK_SET = std::ios_base::beg,
    AFILE_SEEK_CUR = std::ios_base::cur,
    AFILE_SEEK_END = std::ios_base::end
};

class AFile
{
public:
    AFile();

    AFile(const std::wstring& filename)
        : m_fileStream(filename, std::ios::binary)
        , m_flags(0)
        , m_timeStamp(0)
        , m_bHasOpened(false)
    {}

    virtual ~AFile();

    virtual bool Open(const std::wstring& filepath, std::ios_base::openmode mode);
    virtual bool Open(const std::wstring& filepath, uint32_t dwFlags);
    virtual bool Open(const std::wstring& folderName, const std::wstring& fileName, uint32_t dwFlags);
    virtual bool Close();
    
    virtual bool Read(void* pBuffer, uint32_t dwBufferLength, uint32_t* pReadLength);
    virtual bool Write(const void* pBuffer, uint32_t dwBufferLength, uint32_t* pWriteLength);

    virtual bool ReadLine(char* szLineBuffer, uint32_t dwBufferLength, uint32_t* pdwReadLength);
    virtual bool WriteLine(const char* szLineBuffer);

    virtual bool ReadString(char* szLineBuffer, uint32_t dwBufferLength, uint32_t* pdwReadLength);
    virtual bool ReadString(std::string& s);
    virtual bool WriteString(const std::string& s);

    virtual std::streampos Tell();
    virtual bool Seek(int64_t iOffset, AFILE_SEEK origin);
    virtual bool ResetFilePointer();
    virtual std::streampos GetFileLength();

    bool Flush();

    uint32_t GetFileTimeStamp(const std::wstring& filepath);

    std::wstring changeFileExtension(const std::wstring& filePath, const std::wstring& ext);

    std::fstream& GetFileStream() { return m_fileStream; }

    std::wstring& GetFileName() { return m_strFilename; }
    std::wstring& GetRelativePath() { return m_strRelativePath; }
    std::wstring& GetFullPath() { return m_strFullPath; }

    uint32_t GetFlags() const { return m_flags; }
    bool IsText() const { return (m_flags & AFILE_TEXT) ? true : false; }
    bool IsBinary() const { return !IsText(); }

protected:
    std::wstring m_strFilename;
    std::wstring m_strRelativePath;
    std::wstring m_strFullPath;

    uint32_t m_flags;
    uint32_t m_timeStamp;
    bool m_bHasOpened;

private:
    std::fstream m_fileStream;
};

#endif
