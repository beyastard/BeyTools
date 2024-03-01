#include "AFilePackBase.h"
#include "AFI.h"
#include "AFPI.h"

#include <filesystem>
#include <io.h>

extern uint32_t AFPCK_GUARDBYTE0;
extern uint32_t AFPCK_GUARDBYTE1;
extern uint32_t AFPCK_MASKDWORD;
extern uint32_t AFPCK_CHECKMASK;

static const uint64_t NET_DISK_RW_MAX_SIZE = 1024 * 1024;
static const uint64_t IO_TIMEOUT_ERROR_COUNT = 120000;

int64_t _FileWrite(const void* buffer, const int64_t num_byte, std::fstream& stream)
{
    if (!buffer || !stream)
        return 0;
    
    std::streampos beginOffset = stream.tellg();
    if (beginOffset == -1)
    {
        AFERRLOG((L"stream.tellg ERROR, check whether devices support file seeking!!"));
        return 0;
    }

    const char* pBuf = static_cast<const char*>(buffer);
    int64_t sizeMaxOnceWrite = NET_DISK_RW_MAX_SIZE;
    int64_t startTimeCnt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    int64_t dwOffset = 0;
    while (dwOffset < num_byte)
    {
        int64_t bytesToWrite = min(num_byte - dwOffset, sizeMaxOnceWrite);
        stream.write(pBuf + dwOffset, bytesToWrite);

        if (!stream.good())
        {
            if ((std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() - startTimeCnt) > IO_TIMEOUT_ERROR_COUNT)
            {
                AFERRLOG((L"Write ERROR: SIZE:" + std::to_wstring(num_byte) +
                    L", OFFSET:" + std::to_wstring(dwOffset) + L", TRY_WRITE:" + std::to_wstring(bytesToWrite)));
                return dwOffset;
            }

            if (sizeMaxOnceWrite >= 2)
                sizeMaxOnceWrite >>= 1;

            stream.clear();
            stream.seekp(beginOffset + static_cast<std::streamoff>(dwOffset));
        }
        else
        {
            dwOffset += bytesToWrite;
            startTimeCnt = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }

    return dwOffset;
}

int64_t _FileRead(void* buffer, const int64_t num_byte, std::fstream& stream)
{
    if (!buffer || !stream)
        return 0;
    
    std::streampos beginOffset = stream.tellg();
    if (beginOffset == -1)
    {
        AFERRLOG((L"ftell ERROR, check whether devices support file seeking!!"));
        return 0;
    }

    LPBYTE pBuf = static_cast<LPBYTE>(buffer);
    int64_t sizeMaxOnceRead = NET_DISK_RW_MAX_SIZE;
    int64_t startTimeCnt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    int64_t dwOffset = 0;
    while (dwOffset < num_byte)
    {
        int64_t bytesToRead = min(num_byte - dwOffset, sizeMaxOnceRead);
        stream.read(reinterpret_cast<char*>(pBuf + dwOffset), bytesToRead);

        if (!stream.good())
        {
            if ((std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() - startTimeCnt) > IO_TIMEOUT_ERROR_COUNT)
            {
                AFERRLOG((L"Read ERROR: SIZE:" + std::to_wstring(num_byte) + L", OFFSET:" +
                    std::to_wstring(dwOffset) + L", TRY_READ:" + std::to_wstring(bytesToRead)));
                return dwOffset;
            }

            if (sizeMaxOnceRead >= 2)
                sizeMaxOnceRead >>= 1;

            stream.clear();
            stream.seekg(beginOffset + static_cast<std::streamoff>(dwOffset));
        }
        else
        {
            dwOffset += bytesToRead;
            startTimeCnt = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }

    return dwOffset;
}

AFilePackBase::CPackageFile::CPackageFile()
    : m_size1(0)
    , m_size2(0)
    , m_filePos(0)
{}

AFilePackBase::CPackageFile::~CPackageFile() = default;

bool AFilePackBase::CPackageFile::Open(const std::wstring& szFileName, const std::wstring szMode)
{
    Close();

    m_fileStream1.open(szFileName, std::ios_base::binary | std::ios_base::in);
    if (!m_fileStream1.is_open())
        return false;

    m_fileStream1.seekg(0, std::ios_base::end);
    m_size1 = m_fileStream1.tellg();
    m_fileStream1.seekg(0, std::ios_base::beg);

    m_filePos = 0;
    m_szPath = szFileName;
    m_szMode = szMode;

    m_szPath2 = m_szPath;
    af_ChangeFileExt(m_szPath2, MAX_PATH, L".pkx");

    return true;
}

bool AFilePackBase::CPackageFile::Phase2Open(int64_t dwOffset)
{
    if (dwOffset >= MAX_FILE_PACKAGE)
    {
        m_fileStream2.open(m_szPath2, std::ios_base::binary | std::ios_base::in);
        if (!m_fileStream2.is_open())
        {
            if (m_szMode == L"r+b" && !af_IsFileExist(m_szPath2))
            {
                m_fileStream2.open(m_szPath2, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                if (!m_fileStream2.is_open())
                    return false;
            }
            else
                return false;
        }

        m_fileStream2.seekg(0, std::ios_base::end);
        m_size2 = m_fileStream2.tellg();
        m_fileStream2.seekg(0, std::ios_base::beg);
    }

    return true;
}

bool AFilePackBase::CPackageFile::Close()
{
    m_fileStream1.close();
    m_fileStream2.close();

    m_size1 = 0;
    m_size2 = 0;
    m_filePos = 0;

    return true;
}

int64_t AFilePackBase::CPackageFile::read(void* buffer, int64_t size, int64_t count)
{
    int64_t size_to_read = size * count;
    int64_t new_pos = m_filePos + size_to_read;

    if (new_pos <= MAX_FILE_PACKAGE)
    {
        // Completely in file 1
        m_fileStream1.seekg(m_filePos);
        m_fileStream1.read(static_cast<char*>(buffer), size_to_read);
        int64_t readsize = m_fileStream1.gcount();
        m_filePos += readsize;

        // Reset m_fileStream2's file pointer if necessary
        if (m_filePos == MAX_FILE_PACKAGE && m_fileStream2.is_open())
            m_fileStream2.seekg(0, std::ios_base::beg);

        return readsize;
    }
    else if (m_filePos < MAX_FILE_PACKAGE)
    {
        // Partial in file 1 and partial in file 2
        int64_t size_to_read1 = min(size_to_read, MAX_FILE_PACKAGE - m_filePos);
        int64_t size_to_read2 = size_to_read - size_to_read1;

        // Read from file 1
        m_fileStream1.seekg(m_filePos);
        m_fileStream1.read(static_cast<char*>(buffer), size_to_read1);
        int64_t readsize = m_fileStream1.gcount();
        m_filePos += readsize;

        // Read from file 2 if necessary
        if (m_fileStream2.is_open())
        {
            m_fileStream2.read(static_cast<char*>(buffer) + size_to_read1, size_to_read2);
            readsize += m_fileStream2.gcount();
        }

        m_filePos += size_to_read2; // Increment file position by size_to_read2
        return readsize;
    }
    else
    {
        // Completely in file 2
        m_fileStream2.seekg(m_filePos - MAX_FILE_PACKAGE);
        m_fileStream2.read(static_cast<char*>(buffer), size_to_read);
        int64_t readsize = m_fileStream2.gcount();
        m_filePos += readsize;
        return readsize;
    }

    return 0;
}

int64_t AFilePackBase::CPackageFile::write(const void* buffer, int64_t size, int64_t count)
{
    int64_t size_to_write = size * count;
    int64_t new_size = m_filePos + size_to_write;

    if (new_size <= MAX_FILE_PACKAGE)
    {
        // Completely in file 1
        m_fileStream1.seekp(m_filePos);
        m_fileStream1.write(static_cast<const char*>(buffer), size_to_write);
        int64_t writesize = m_fileStream1.tellp() - m_filePos;
        m_filePos += writesize;
        if (m_filePos > m_size1)
            m_size1 = m_filePos;
        return writesize;
    }
    else if (m_filePos < MAX_FILE_PACKAGE)
    {
        // Partial in file 1 and partial in file 2
        int64_t size_to_write1 = min(size_to_write, MAX_FILE_PACKAGE - m_filePos);
        int64_t size_to_write2 = size_to_write - size_to_write1;

        // Write to file 1
        m_fileStream1.seekp(m_filePos);
        m_fileStream1.write(static_cast<const char*>(buffer), size_to_write1);
        int64_t writesize1 = m_fileStream1.tellp() - m_filePos;
        m_filePos += writesize1;
        if (m_filePos > m_size1)
            m_size1 = m_filePos;

        if (writesize1 != size_to_write1)
        {
            m_fileStream1.seekp(m_filePos);
            return writesize1;
        }

        m_size1 = MAX_FILE_PACKAGE;

        // Open file 2 if not already opened
        if (!m_fileStream2.is_open())
            Phase2Open(MAX_FILE_PACKAGE);

        // Write to file 2
        m_fileStream2.seekp(0, std::ios_base::end);
        m_fileStream2.write(static_cast<const char*>(buffer) + size_to_write1, size_to_write2);
        int64_t writesize2 = m_fileStream2.tellp() - m_filePos;
        m_filePos += writesize2;
        if (m_filePos > m_size1 + m_size2)
            m_size2 = m_filePos - m_size1;
        return writesize1 + writesize2;
    }
    else
    {
        // Completely in file 2
        if (!m_fileStream2.is_open())
        {
            Phase2Open(MAX_FILE_PACKAGE);
            m_fileStream2.seekp(0, std::ios_base::end);
        }

        m_fileStream2.write(static_cast<const char*>(buffer), size_to_write);
        int64_t writesize = m_fileStream2.tellp() - m_filePos;
        m_filePos += writesize;
        if (m_filePos > m_size1 + m_size2)
            m_size2 = m_filePos - m_size1;
        return writesize;
    }

    return 0;
}

std::streampos AFilePackBase::CPackageFile::tell()
{
    return m_filePos;
}

void AFilePackBase::CPackageFile::seek(int64_t offset, AFILE_SEEK origin)
{
    int64_t newpos = m_filePos;

    if (m_fileStream2.is_open())
    {
        switch (origin)
        {
        case SEEK_SET:
            newpos = offset;
            break;
        case SEEK_CUR:
            newpos += offset;
            break;
        case SEEK_END:
            newpos = m_size1 + m_size2 + offset;
            break;
        }

        newpos = std::max<int64_t>(0, min(newpos, m_size1 + m_size2));

        if (newpos < m_size1)
        {
            m_fileStream1.seekg(newpos, std::ios_base::beg);
            m_filePos = m_fileStream1.tellg();
        }
        else
        {
            m_fileStream2.seekg(newpos - m_size1, std::ios_base::beg);
            m_filePos = m_fileStream2.tellg();
        }
    }
    else
    {
        m_fileStream1.seekg(offset, origin);
        m_filePos = m_fileStream1.tellg();
    }
}

void AFilePackBase::CPackageFile::SetPackageFileSize(int64_t dwFileSize)
{
    if (m_fileStream2)
    {
        if (dwFileSize <= MAX_FILE_PACKAGE)
        {
            std::filesystem::resize_file(m_szPath.c_str(), dwFileSize);
            m_size1 = dwFileSize;

            m_fileStream2.close();
            std::filesystem::remove(m_szPath2.c_str());
            m_size2 = 0;
        }
        else
        {
            m_size2 = dwFileSize - MAX_FILE_PACKAGE;
            std::filesystem::resize_file(m_szPath2.c_str(), m_size2);
        }
    }
    else
    {
        std::filesystem::resize_file(m_szPath.c_str(), dwFileSize);
        m_size1 = dwFileSize;
    }
}
