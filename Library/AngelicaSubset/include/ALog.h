#ifndef _ALOG_H_
#define _ALOG_H_

#include "ABaseDef.h"

#include <fstream>

typedef void (*LPFNDEFLOGOUTPUT)(const std::wstring& szMsg);

LPFNDEFLOGOUTPUT a_RedirectDefLogOutput(LPFNDEFLOGOUTPUT lpfn);

void a_LogOutput(int32_t iLevel, const std::wstring szMsg, ...);
void a_LogOutputNoReturn(int32_t iLevel, const std::wstring szMsg, ...);
void a_SendLogToDebugWnd(int32_t iLevel, const std::wstring szMsg, ...);

class ALog
{
public:
    ALog();
    virtual ~ALog();

    // Init a log file
    //   szLogFile   will be the logs path
    //   szHelloMsg  is the hello message in the log
    //   bAppend     is the flag to append at the end of the log file
    bool Init(const std::wstring& szLogFile, const std::wstring& szHelloMsg, bool bAppend = false);

    // Release the log file
    //    this call will close the log file pointer and write a finished message
    bool Release();

    // Output a variable arguments log message;
    bool Log(const std::wstring fmt, ...);

    // Output a string as a log message;
    bool LogString(const std::wstring& szString);
    bool LogString(const std::string& szString);

    static void SetLogDir(const std::wstring& szLogDir);
    static const std::wstring& GetLogDir() { return m_szLogDir; }

private:
    std::wofstream m_wlogFile;
    std::ofstream m_logFile;
    static std::wstring m_szLogDir;
};

extern ALog* g_pAFErrLog;

#endif
