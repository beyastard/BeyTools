#ifndef __STRING_CONVERT_H__
#define __STRING_CONVERT_H__

#include "ABaseDef.h"

// convert std::wstring to std::string
static std::string wstring_to_string(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// convert std::string to std::wstring
static std::wstring string_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

static wchar_t* CharToWChar(const char* ansiText)
{
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, ansiText, -1, nullptr, 0);
    if (requiredSize == 0)
        return nullptr; // Conversion failed

    wchar_t* wideText = new wchar_t[requiredSize];
    if (MultiByteToWideChar(CP_ACP, 0, ansiText, -1, wideText, requiredSize) == 0)
    {
        delete[] wideText;
        return nullptr; // Conversion failed
    }

    return wideText;
}

static char* WCharToChar(const wchar_t* wideText)
{
    int requiredSize = WideCharToMultiByte(CP_ACP, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
    if (requiredSize == 0)
        return nullptr; // Conversion failed

    char* ansiText = new char[requiredSize];
    if (WideCharToMultiByte(CP_ACP, 0, wideText, -1, ansiText, requiredSize, nullptr, nullptr) == 0)
    {
        delete[] ansiText;
        return nullptr; // Conversion failed
    }

    return ansiText;
}

#endif
