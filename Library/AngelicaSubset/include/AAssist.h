#ifndef _AASSIST_H_
#define _AASSIST_H_

#include "ABaseDef.h"

uint32_t a_MakeIDFromString(const std::wstring& szStr);
uint32_t a_MakeIDFromLowString(const std::wstring& szStr);
bool a_GetStringAfter(const wchar_t* wszBuffer, const char* szTag, char* szResult);
bool a_GetStringAfter(const std::wstring& wstrBuffer, const std::string& strTag, std::string& strResult);
bool a_GetStringAfter(const std::wstring& wstrBuffer, const std::wstring& wstrTag, std::wstring& wstrResult);

#endif
