#include "AFI.h"
#include "ALog.h"

#include <algorithm>
#include <filesystem>

std::wstring g_strBaseDir = L"";

bool af_Initialize()
{
    if (g_pAFErrLog)
        af_Finalize();

    g_strBaseDir = std::filesystem::current_path().wstring();
    if (!g_strBaseDir.empty() && g_strBaseDir.back() == L'\\')
        g_strBaseDir.pop_back();

    return true;
}

bool af_Finalize()
{
    if (g_pAFErrLog)
    {
        g_pAFErrLog->Release();
        delete g_pAFErrLog;
        g_pAFErrLog = nullptr;
    }

    g_strBaseDir.clear();
    return true;
}

bool af_SetBaseDir(const std::wstring& szBaseDir)
{
    g_strBaseDir = szBaseDir;

    if (!g_strBaseDir.empty() && g_strBaseDir.back() == L'\\')
        g_strBaseDir.pop_back();

    if (!g_pAFErrLog)
    {
        g_pAFErrLog = new ALog;
        if (!g_pAFErrLog)
            return false;

        g_pAFErrLog->Init(L"AF.log", L"Angelica File Module Error Log");
    }

    return true;
}

const std::wstring& af_GetBaseDir()
{
    if (g_strBaseDir.empty())
        af_Initialize();

    return g_strBaseDir;
}

void af_GetRelativePathNoBase(const std::wstring& szFullpath, const std::wstring& szParentPath, std::wstring& szRelativepath)
{
    std::filesystem::path fullPath(szFullpath);
    std::filesystem::path parentPath(szParentPath);
    std::filesystem::path relativePath = fullPath.lexically_relative(parentPath);
    szRelativepath = relativePath.native();
}

void af_GetFullPathNoBase(std::wstring& szFullpath, const std::wstring& szBaseDir, const std::wstring& szFilename)
{
    szFullpath.clear();

    if (szFilename.empty())
        return;

    // Check if it is an absolute path
    if (szFilename.size() > 3 && ((szFilename[1] == L':' && szFilename[2] == L'\\') || (szFilename[0] == L'\\' && szFilename[1] == L'\\')))
    {
        szFullpath = szFilename;
        return;
    }

    // Get rid of prefix .\ to make a clean relative file path
    std::wstring_view realfile = szFilename;
    if (szFilename.size() > 2 && szFilename[0] == L'.' && szFilename[1] == L'\\')
        realfile = std::wstring_view(szFilename.data() + 2, szFilename.size() - 2);

    if (szBaseDir.empty())
        szFullpath = realfile;
    else
    {
        szFullpath = (!szBaseDir.empty() && szBaseDir.back() == L'\\')
            ? std::format(L"{}{}", szBaseDir, realfile)
            : std::format(L"{}\\{}", szBaseDir, realfile);
    }
}

void af_GetFullPath(std::wstring& szFullPath, const std::wstring& szFolderName, const std::wstring& szFileName)
{
    std::wstring szBaseDir = std::format(L"{}\\{}", g_strBaseDir, szFolderName);
    af_GetFullPathNoBase(szFullPath, szBaseDir, szFileName);
}

void af_GetFullPath(std::wstring& szFullPath, const std::wstring& szFileName)
{
    af_GetFullPathNoBase(szFullPath, g_strBaseDir, szFileName);
}

void af_GetRelativePath(const std::wstring& szFullPath, const std::wstring& szFolderName, std::wstring& szRelativePath)
{
    std::wstring szBaseDir = std::format(L"{}\\{}", g_strBaseDir, szFolderName);
    af_GetRelativePathNoBase(szFullPath, szBaseDir, szRelativePath);
}

void af_GetRelativePath(const std::wstring& szFullPath, std::wstring& szRelativePath)
{
    af_GetRelativePathNoBase(szFullPath, g_strBaseDir, szRelativePath);
}

bool af_GetFileTitle(const std::wstring& lpszFile, std::wstring& lpszTitle, uint16_t cbBuf)
{
    if (lpszFile.empty() || cbBuf == 0)
        return false;

    size_t lastSlashPos = lpszFile.find_last_of(L"/\\");
    lpszTitle = (lastSlashPos == std::wstring::npos)
        ? lpszFile
        : lpszFile.substr(lastSlashPos + 1);

    return true;
}

bool af_GetFileTitle(const std::wstring& lpszFile, std::wstring& strTitle)
{
    std::wstring wideTitle;
    return af_GetFileTitle(lpszFile, strTitle, MAX_PATH)
        ? (strTitle = wideTitle, true)
        : false;
}

bool af_GetFilePath(const std::wstring& lpszFile, std::wstring& lpszPath, uint16_t cbBuf)
{
    if (lpszFile.empty() || cbBuf == 0)
        return false;

    lpszPath.clear();

    if (lpszFile[0] == L'\0')
        return true;

    lpszPath.assign(lpszFile.begin(), lpszFile.begin() + min(cbBuf - 1, static_cast<uint16_t>(lpszFile.size())));

    size_t lastSeparatorPos = lpszPath.find_last_of(L"/\\");
    if (lastSeparatorPos != std::wstring::npos)
        lpszPath.resize(lastSeparatorPos);

    return true;
}

bool af_GetFilePath(const std::wstring& lpszFile, std::wstring& strPath)
{
    std::wstring szPath;
    return af_GetFilePath(lpszFile, szPath, MAX_PATH)
        ? (strPath = szPath, true)
        : false;
}

bool af_CheckFileExt(const std::wstring& szFileName, const std::wstring& szExt, int32_t iExtLen, int32_t iFileNameLen)
{
    if (iFileNameLen != iExtLen)
        return false;

    // Get reverse iterators for file name and extension
    std::reverse_iterator<std::wstring::const_iterator> fileNameEnd = szFileName.crbegin() + szFileName.size() - iFileNameLen;
    std::reverse_iterator<std::wstring::const_iterator> extEnd = szExt.crbegin() + szExt.size();

    // Perform character-by-character comparison
    while (extEnd != szExt.crend() && fileNameEnd != szFileName.crend())
    {
        wchar_t ch1 = std::tolower(*fileNameEnd++);
        wchar_t ch2 = std::tolower(*extEnd++);

        // If characters don't match, return false
        if (ch1 != ch2)
            return false;
    }

    return true;
}

bool af_ChangeFileExt(std::wstring& szFileNameBuf, int32_t iBufLen, const std::wstring& szNewExt)
{
    std::filesystem::path filePath(szFileNameBuf);
    filePath.replace_extension(szNewExt);
    std::wstring newFileName = filePath.wstring();

    if (newFileName.size() >= static_cast<size_t>(iBufLen))
    {
        assert(false); // Buffer overflow
        return false;
    }

    szFileNameBuf = newFileName;
    return true;
}

bool af_ChangeFileExt(std::wstring& strFileName, const std::wstring& szNewExt)
{
    std::filesystem::path filePath(strFileName);
    filePath.replace_extension(szNewExt);
    strFileName = filePath.wstring();

    return true;
}

bool af_IsFileExist(const std::wstring& szFileName)
{
    return false;
}
