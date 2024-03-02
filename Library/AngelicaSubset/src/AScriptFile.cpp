#include "AScriptFile.h"
#include "AFileImage.h"
#include "AFPI.h"

#include <algorithm>
#include <sstream>

AScriptFile::AScriptFile()
{
    std::memset(&m_szToken, 0, sizeof(m_szToken));
    std::memset(&m_Script, 0, sizeof(m_Script));
}

AScriptFile::~AScriptFile()
{
    AScriptFile::Close();
}

bool AScriptFile::Open(AFile* pFile)
{
    uint32_t dwFileLen = (uint32_t)pFile->GetFileLength();
    if (!dwFileLen)
    {
        AFERRLOG((L"AScriptFile::Open, Empty ini file"));
        return true;
    }

    uint8_t* pBuf = new uint8_t[dwFileLen];
    if (!pBuf)
    {
        AFERRLOG((L"AScriptFile::Open, Not enough memory"));
        return false;
    }

    uint32_t dwRead;

    //	Read whole file into memory
    if (!pFile->Read(pBuf, dwFileLen, &dwRead) || dwRead != dwFileLen)
    {
        delete[] pBuf;
        pBuf = nullptr;
        AFERRLOG((L"AScriptFile::Open, Failed to read file content"));
        return false;
    }

    m_Script.pStart = pBuf;
    m_Script.pCur = pBuf;
    m_Script.pEnd = pBuf + dwFileLen;
    m_Script.iLine = 0;

    return true;
}

bool AScriptFile::Open(const std::wstring& szFile)
{
    AFileImage File;

    if (!File.Open(L"", szFile, AFILE_OPENEXIST | AFILE_BINARY | AFILE_TEMPMEMORY))
    {
        File.Close();
        return false;
    }

    if (!Open(&File))
    {
        File.Close();
        return false;
    }

    File.Close();
    return true;
}

void AScriptFile::Close()
{
    if (m_Script.pStart)
    {
        delete[] m_Script.pStart;
        m_Script.pStart = nullptr;
    }

    std::memset(&m_Script, 0, sizeof(m_Script));
}

void AScriptFile::ResetScriptFile()
{
    m_Script.pCur = m_Script.pStart;
    m_Script.iLine = 0;
}

// Get next token and move file pointer forward.
// 
// Return true for success, otherwise return false
// 
// bCrossLine: true, search next token until it is found or all buffer has been checked
//             false, only search next token in current line
bool AScriptFile::GetNextToken(bool bCrossLine)
{
    const char* delimiters = "\"() \t\r\n"; // Delimiters for token extraction

    while (true)
    {
        // Skip whitespace characters and handle line breaks
        while (m_Script.pCur < m_Script.pEnd && (*m_Script.pCur <= ' ' || *m_Script.pCur == ';' || *m_Script.pCur == ','))
        {
            if (*m_Script.pCur == '\n')
            {
                if (!bCrossLine)
                    return false; // End of line, return false if not searching cross line

                m_Script.iLine++; // Increment line counter
            }
            m_Script.pCur++; // Move to the next character
        }

        // Check if at the end of the buffer
        if (m_Script.pCur >= m_Script.pEnd)
            return false;

        // Skip comment lines that begin with '//'
        if (m_Script.pCur[0] == '/' && m_Script.pCur[1] == '/')
        {
            m_Script.pCur += 2; // Skip '//'
            while (m_Script.pCur < m_Script.pEnd && *m_Script.pCur != '\n')
                m_Script.pCur++; // Move to the next character until newline or end of buffer

            if (m_Script.pCur >= m_Script.pEnd || !bCrossLine)
                return false; // Return false if not searching cross line or end of buffer

            m_Script.iLine++; // Increment line counter
            m_Script.pCur++; // Move to the next line
        }
        // Skip comment blocks that begin with '/*'
        else if (m_Script.pCur[0] == '/' && m_Script.pCur[1] == '*')
        {
            m_Script.pCur += 2; // Skip '/*'
            while (m_Script.pCur[0] != '*' || m_Script.pCur[1] != '/')
            {
                if (++m_Script.pCur >= m_Script.pEnd || (*m_Script.pCur == '\n' && !bCrossLine))
                    return false; // Return false if at the end of buffer or newline encountered and not searching cross line

                if (*m_Script.pCur == '\n')
                    m_Script.iLine++; // Increment line counter
            }

            m_Script.pCur += 2; // Skip '*/'
        }
        else
            break; // Found the beginning of a token
    }

    // Extract token until reaching a delimiter
    int32_t i = 0;
    while (m_Script.pCur < m_Script.pEnd && std::strchr(delimiters, *m_Script.pCur) == nullptr)
    {
        // Check for token size limit
        if (i >= AFILE_LINEMAXLEN - 1)
            return false; // Token exceeds maximum length, return false

        m_szToken[i++] = *m_Script.pCur++; // Copy character to token buffer and move to the next character
    }

    m_szToken[i] = '\0'; // Null-terminate the token string

    return true; // Successfully parsed token
}

// Get next token and move file pointer forward.
// 
// Return true for success, otherwise return false
// 
// bCrossLine: true, search next token until it is found or all buffer has been checked
//             false, only search next token in current line
bool AScriptFile::PeekNextToken(bool bCrossLine)
{
    // Record current pointer and line
    uint8_t* pCur = m_Script.pCur;
    int32_t iLine = m_Script.iLine;

    bool bRet = GetNextToken(bCrossLine);

    // Restore pointer and line
    m_Script.pCur = pCur;
    m_Script.iLine = iLine;

    return bRet;
}

// Skip current line and ignore all rest content in it.
// 
// Return true for success, otherwise return false
bool AScriptFile::SkipLine()
{
    // Find the next newline character or reach the end of the buffer
    while (m_Script.pCur < m_Script.pEnd && *m_Script.pCur != '\n')
        m_Script.pCur++;

    m_Script.iLine++;

    // Return true if a newline character was found before reaching the end of the buffer
    return m_Script.pCur < m_Script.pEnd;
}

// Search specified token. This function get next token and check whether it match
// specified string, if match, then stop and return true, otherwise get next token
// again until all file is checked or token is found.
// 
// Note: This will be crossing-line search.
// 
// Return true for success, otherwise return false.
// 
// szToken: specified token will be searched
// bCaseSensitive: true, case sensitive (has no purpose...)
bool AScriptFile::MatchToken(char* szToken, bool bCaseSensitive)
{
    // Convert wszToken to lowercase (or leave unchanged if case-sensitive)
    std::string tokenLower(szToken);
    if (!bCaseSensitive)
    {
        std::transform(tokenLower.begin(), tokenLower.end(), tokenLower.begin(),
            [](char c) { return std::tolower(c, std::locale()); });
    }

    // Iterate over tokens
    while (GetNextToken(true))
    {
        // Convert m_szToken to lowercase (or leave unchanged if case-sensitive)
        std::string curTokenLower(m_szToken);
        if (!bCaseSensitive)
        {
            std::transform(curTokenLower.begin(), curTokenLower.end(), curTokenLower.begin(),
                [](char c) { return std::tolower(c, std::locale()); });
        }

        // Perform case-insensitive comparison
        if (curTokenLower == tokenLower)
            return true;
    }

    return false;
}

bool AScriptFile::MatchToken(char* szToken)
{
    return MatchToken(szToken, true);
}

float AScriptFile::GetNextTokenAsFloat(bool bCrossLine)
{
    GetNextToken(bCrossLine);
    std::stringstream ss(m_szToken);
    float floatValue;
    ss >> floatValue;
    return floatValue;
}

int32_t AScriptFile::GetNextTokenAsInt(bool bCrossLine)
{
    GetNextToken(bCrossLine);
    std::stringstream ss(m_szToken);
    int intValue;
    ss >> intValue;
    return intValue;
}
