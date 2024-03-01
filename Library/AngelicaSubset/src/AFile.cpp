#include "AFile.h"
#include "AFI.h"
#include "ALog.h"

#include <codecvt>
#include <filesystem>
#include <iostream>

AFile::AFile()
    : m_strFilename()
    , m_strRelativePath()
    , m_strFullPath()
    , m_flags(0)
    , m_timeStamp(0)
    , m_bHasOpened(false)
{}

AFile::~AFile()
{
    AFile::Close();
}

bool AFile::Open(const std::wstring& filepath, std::ios_base::openmode mode)
{
    if (m_bHasOpened)
        Close();

    m_fileStream.open(filepath, mode);
    if (m_fileStream.is_open())
    {
        m_strFilename = std::filesystem::path(filepath).filename().wstring();
        m_strRelativePath = std::filesystem::path(filepath).parent_path().wstring();
        m_strFullPath = std::filesystem::absolute(filepath).wstring();
    }
    else
        return false;

    m_bHasOpened = true;
    return true;
}

bool AFile::Open(const std::wstring& filepath, uint32_t dwFlags)
{
    if (m_bHasOpened)
        Close();

    std::ios_base::openmode mode = std::ios_base::binary; // Default mode is binary
    if (dwFlags & AFILE_OPENEXIST)
        mode |= std::ios_base::in;

    if (dwFlags & AFILE_CREATENEW)
        mode |= std::ios_base::out;

    if (dwFlags & AFILE_OPENAPPEND)
        mode |= std::ios_base::app;

    if (dwFlags & AFILE_BINARY)
        mode |= std::ios_base::binary;

    m_fileStream.open(filepath, mode);
    if (m_fileStream.is_open())
    {
        m_strFilename = std::filesystem::path(filepath).filename().wstring();
        m_strRelativePath = std::filesystem::path(filepath).parent_path().wstring();
        m_strFullPath = std::filesystem::absolute(filepath).wstring();
    }
    else
        return false;

    if (dwFlags & AFILE_CREATENEW)
    {
        m_flags = dwFlags;
        uint32_t dwFOURCC;
        if (m_flags & AFILE_TEXT)
        {
            dwFOURCC = 0x54584f4d; // MOXT
            if (!(m_flags & AFILE_NOHEAD))
                m_fileStream.write(reinterpret_cast<const char*>(&dwFOURCC), sizeof(dwFOURCC));
        }
        else
        {
            dwFOURCC = 0x42584f4d; // MOXB
            if (!(m_flags & AFILE_NOHEAD))
                m_fileStream.write(reinterpret_cast<const char*>(&dwFOURCC), sizeof(dwFOURCC));
        }
    }
    else
    {
        m_flags = dwFlags & (~(AFILE_BINARY | AFILE_TEXT));

        uint32_t dwFOURCC;
        m_fileStream.read(reinterpret_cast<char*>(&dwFOURCC), sizeof(dwFOURCC));
        if (dwFOURCC == 0x42584f4d)      // MOXB
            m_flags |= AFILE_BINARY;
        else if (dwFOURCC == 0x54584f4d) // MOXT
            m_flags |= AFILE_TEXT;
        else
        {
            // Default to text mode
            m_flags |= AFILE_TEXT;
            m_fileStream.seekg(0, std::ios_base::beg);
        }
    }

    m_timeStamp = GetFileTimeStamp(GetFullPath());
    m_bHasOpened = true;

    return true;
}

bool AFile::Open(const std::wstring& folderName, const std::wstring& fileName, uint32_t dwFlags)
{
    std::wstring szFullPath;
    if (fileName.empty())
        return false;

    if (std::filesystem::path(fileName).is_absolute())
        szFullPath = fileName;
    else
    {
        std::wstring realfile = fileName;
        if (realfile.size() > 2 && realfile[0] == L'.' && realfile[1] == L'\\')
            realfile = realfile.substr(2);

        std::filesystem::path baseDir = g_strBaseDir;
        szFullPath = baseDir / realfile;
    }

    return Open(szFullPath, dwFlags);
}

bool AFile::Close()
{
    if (m_fileStream.is_open())
        m_fileStream.close();
    
    m_bHasOpened = false;
    return true;
}

bool AFile::Read(void* pBuffer, uint32_t dwBufferLength, uint32_t* pReadLength)
{
    m_fileStream.read(reinterpret_cast<char*>(pBuffer), dwBufferLength);
    if (m_fileStream.fail())
    {
        *pReadLength = 0;
        return false;
    }

    *pReadLength = static_cast<uint32_t>(m_fileStream.gcount());
    return true;
}

bool AFile::Write(const void* pBuffer, uint32_t dwBufferLength, uint32_t* pWriteLength)
{
    m_fileStream.write(reinterpret_cast<const char*>(pBuffer), dwBufferLength);
    if (m_fileStream.fail())
    {
        *pWriteLength = 0;
        return false;
    }

    *pWriteLength = dwBufferLength;
    return true;
}

bool AFile::ReadLine(char* szLineBuffer, uint32_t dwBufferLength, uint32_t* pdwReadLength)
{
    if (!m_fileStream.getline(szLineBuffer, dwBufferLength))
        return false;
    
    // Calculate the length of the line
    size_t lineLength = std::strlen(szLineBuffer);

    // Remove trailing newline characters
    while (lineLength > 0 && (szLineBuffer[lineLength - 1] == '\n' || szLineBuffer[lineLength - 1] == '\r')) {
        szLineBuffer[lineLength - 1] = '\0';
        lineLength--;
    }

    *pdwReadLength = static_cast<uint32_t>(lineLength);

    return true;
}

bool AFile::WriteLine(const char* szLineBuffer)
{
    m_fileStream << szLineBuffer << '\n';
    return !m_fileStream.fail();
}

bool AFile::ReadString(char* szLineBuffer, uint32_t dwBufferLength, uint32_t* pdwReadLength)
{
    if (!m_fileStream.getline(szLineBuffer, dwBufferLength))
        return false;
    
    // Calculate the length of the string
    *pdwReadLength = static_cast<uint32_t>(std::strlen(szLineBuffer)) + 1;

    return true;
}

bool AFile::ReadString(std::string& s)
{
    int32_t iLen;
    if (!m_fileStream.read(reinterpret_cast<char*>(&iLen), sizeof(int32_t)))
        return false;
    
    // Read string data
    if (iLen > 0) {
        s.resize(iLen);
        if (!m_fileStream.read(reinterpret_cast<char*>(s.data()), iLen))
            return false;
    }
    else
        s.clear();
    
    return true;
}

bool AFile::WriteString(const std::string& s)
{
    m_fileStream << s << '\n';
    return !m_fileStream.fail();
}

std::streampos AFile::Tell()
{
    return m_fileStream.tellg();
}

bool AFile::Seek(int64_t iOffset, AFILE_SEEK origin)
{
    std::ios_base::seekdir dir;
    if (origin == AFILE_SEEK_SET)
        dir = std::ios_base::beg;
    else if (origin == AFILE_SEEK_CUR)
        dir = std::ios_base::cur;
    else if (origin == AFILE_SEEK_END)
        dir = std::ios_base::end;
    else
        return false; // Invalid origin

    m_fileStream.seekg(iOffset, dir);
    return !m_fileStream.fail();
}

bool AFile::ResetFilePointer()
{
    m_fileStream.seekg(0, std::ios_base::beg);
    return !m_fileStream.fail();
}

std::streampos AFile::GetFileLength()
{
    std::streampos currentPosition = m_fileStream.tellg();
    m_fileStream.seekg(0, std::ios_base::end);
    
    std::streampos endPosition = m_fileStream.tellg();
    m_fileStream.seekg(currentPosition, std::ios_base::beg);
    
    return endPosition;
}

bool AFile::Flush()
{
    m_fileStream.flush();
    return !m_fileStream.fail();
}

uint32_t AFile::GetFileTimeStamp(const std::wstring& filepath)
{
    std::filesystem::path filePath(filepath);
    std::error_code ec;
    std::filesystem::file_time_type fileTime = std::filesystem::last_write_time(filePath, ec);

    if (ec)
    {
        // Convert wide character filepath to multibyte string
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string filepath_mb = converter.to_bytes(filepath);

        // Print an error message indicating the cause of the error
        std::cerr << "Error accessing file: " << filepath_mb << " - " << ec.message() << std::endl;

        return -1;
    }

    auto fileTimePoint = std::chrono::time_point_cast<std::chrono::system_clock::time_point::duration>(fileTime);
    auto time_t_stamp = std::chrono::duration_cast<std::chrono::seconds>(fileTimePoint.time_since_epoch()).count();

    m_timeStamp = static_cast<DWORD>(time_t_stamp);
    return m_timeStamp;
}

std::wstring AFile::changeFileExtension(const std::wstring& filePath, const std::wstring& ext)
{
    size_t dotPosition = filePath.find_last_of(L'.');
    return (dotPosition != std::wstring::npos)
        ? filePath.substr(0, dotPosition) + ext
        : filePath + ext;
}
