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

#endif
