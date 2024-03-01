#include "ALog.h"

#include <chrono>
#include <filesystem>
#include <format>

ALog* g_pAFErrLog = nullptr;
std::wstring ALog::m_szLogDir = L"Logs";

std::string WideCharToUTF8(const std::wstring& wideStr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wideStr);
}

//	Default debug output function
static LPFNDEFLOGOUTPUT	l_lpfnDefLogOutput = nullptr;
static HWND l_hDebugWnd = nullptr;

static void _DefLogOutput(const std::wstring& szMsg)
{
    if (l_lpfnDefLogOutput)
    {
        l_lpfnDefLogOutput(szMsg);
        return;
    }
    OutputDebugStringW(std::format(L"{}", szMsg).c_str());
}

// Redirect default log ouput function. This function can be used to
// redirect the destination all ACommon's internal logs.
// 
// Return previous function set by user.
// 
// lpfn: used defined log output function.
LPFNDEFLOGOUTPUT a_RedirectDefLogOutput(LPFNDEFLOGOUTPUT lpfn)
{
    LPFNDEFLOGOUTPUT lpOld = l_lpfnDefLogOutput;
    l_lpfnDefLogOutput = lpfn;
    return lpOld;
}

void a_LogOutput(int32_t iLevel, const std::wstring szMsg, ...)
{
    std::wstring prefix = (iLevel != 0) ? L"<!> " : L"<-> ";

    va_list vaList;
    va_start(vaList, szMsg);
    size_t sz = std::vswprintf(nullptr, 0, szMsg.c_str(), vaList) + 1; // +1 for null terminator
    std::wstring szBuf(sz, L'\0');
    std::vswprintf(szBuf.data(), sz, szMsg.c_str(), vaList);
    va_end(vaList);

    szBuf.push_back(L'\n');
    _DefLogOutput(prefix + szBuf);
}

void a_LogOutputNoReturn(int32_t iLevel, const std::wstring szMsg, ...)
{
    std::wstring prefix = (iLevel != 0) ? L"<!> " : L"<-> ";

    va_list vaList;
    va_start(vaList, szMsg);
    size_t sz = std::vswprintf(nullptr, 0, szMsg.c_str(), vaList) + 1; // +1 for null terminator
    std::wstring szBuf(sz, L'\0');
    std::vswprintf(szBuf.data(), sz, szMsg.c_str(), vaList);
    va_end(vaList);

    _DefLogOutput(prefix + szBuf);
}

void a_SendLogToDebugWnd(int32_t iLevel, const std::wstring szMsg, ...)
{
    if (!(l_hDebugWnd = FindWindowW(nullptr, L"Angelica Debug Window")))
        return;

    va_list vaList;
    va_start(vaList, szMsg);
    size_t sz = std::vswprintf(nullptr, 0, szMsg.c_str(), vaList) + 1; // +1 for null terminator
    std::wstring szBuf(sz, L'\0');
    std::vswprintf(szBuf.data(), sz, szMsg.c_str(), vaList);
    va_end(vaList);

    COPYDATASTRUCT CopyData{};
    CopyData.lpData = const_cast<wchar_t*>(szBuf.c_str());
    CopyData.cbData = static_cast<uint32_t>(szBuf.size() * sizeof(wchar_t));
    CopyData.dwData = static_cast<uint32_t>(iLevel);

    ::SendMessage(l_hDebugWnd, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&CopyData));
}

ALog::ALog() = default;
ALog::~ALog() = default;

bool ALog::Init(const std::wstring& szLogFile, const std::wstring& szHelloMsg, bool bAppend)
{
    std::filesystem::create_directories(m_szLogDir);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t time_t_now = std::chrono::system_clock::to_time_t(now);

    std::wstringstream logMessage;
    logMessage << szHelloMsg << L"\nCreated(or opened) on "
        << std::put_time(std::gmtime(&time_t_now), L"%d/%m/%Y %H:%M:%S")
        << L"\n\n";

    std::wstring logPath = m_szLogDir + L"\\" + szLogFile;
    m_wlogFile.open(logPath, bAppend ? std::ios_base::app : std::ios_base::trunc);
    if (!m_wlogFile.is_open())
        return true;

    m_wlogFile << logMessage.str();

    return true;
}

bool ALog::Release()
{
    LogString(L"Log file closed successfully!");
    return true;
}

bool ALog::Log(const std::wstring fmt, ...)
{
    constexpr size_t bufferSize = 2048;
    wchar_t szErrorMsg[bufferSize];

    va_list args_list;
    va_start(args_list, fmt);
    std::vswprintf(szErrorMsg, bufferSize, fmt.c_str(), args_list);
    va_end(args_list);

    std::string utf8Msg = WideCharToUTF8(szErrorMsg);
    return LogString(utf8Msg);
}

bool ALog::LogString(const std::wstring& szString)
{
    if (!m_wlogFile)
        return true;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t time_t_now = std::chrono::system_clock::to_time_t(now);

    std::wstring logMessage;
    std::wostringstream timeStringStream;
    timeStringStream << std::put_time(std::localtime(&time_t_now), L"%H:%M:%S");
    logMessage += L"[" + timeStringStream.str() + L"." +
        std::to_wstring(std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 1000) + L"] " +
        szString + L"\n";

    m_wlogFile << logMessage;

    return true;
}

bool ALog::LogString(const std::string& szString)
{
    if (!m_logFile)
        return true;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t time_t_now = std::chrono::system_clock::to_time_t(now);

    std::string logMessage;
    std::ostringstream timeStringStream;
    timeStringStream << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    logMessage += "[" + timeStringStream.str() + "." +
        std::to_string(std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 1000) + "] " +
        szString + "\n";

    m_logFile << logMessage;

    return true;
}

void ALog::SetLogDir(const std::wstring& szLogDir)
{
    m_szLogDir = szLogDir;
    if (!m_szLogDir.empty() && m_szLogDir.back() == L'\\')
        m_szLogDir.pop_back();

    std::filesystem::create_directories(m_szLogDir);
}
