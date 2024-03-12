#ifndef __STRING_CONVERT_CLASS_H__
#define __STRING_CONVERT_CLASS_H__

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <codecvt>
#include <optional>
#include <ranges>
#include <sstream>
#include <vector>

#include <stringapiset.h>
#include <atlstr.h>

#define CODEPAGE 54936 // Chinese GB18030

// static std::string wstring_to_string(const std::wstring& wstr)
// static std::string wstring_to_string(const std::wstring& wstr)
// static std::wstring string_to_wstring(const std::string& str)
// static std::optional<std::unique_ptr<wchar_t[]>> char_to_wchar(const char* charText)
// static std::optional<std::unique_ptr<char[]>> wchar_to_char(const wchar_t* wideText)
// static std::optional<std::unique_ptr<char[]>> GB18030ToUTF8(const char* gb18030Text)
// static std::optional<std::unique_ptr<char[]>> UTF8ToGB18030(const char* utf8Text)
// static std::optional<std::unique_ptr<wchar_t[]>> GB18030ToWideChar(const char* gb18030Text)
// static std::optional<std::unique_ptr<char[]>> WideCharToGB18030(const wchar_t* wideText)
// static std::optional<std::wstring> gb18030_to_wstring(const char* gb18030Text)
// static std::optional<std::unique_ptr<char[]>> wstring_to_gb18030(const std::wstring& wstr)
// static char* GB18030ToUTF8_2(const char* gb18030Text)
// static std::unique_ptr<wchar_t[]> GB18030ToWideChar_2(const char* gb18030Text)
// static CStringW GB18030ToCStringW(const char* gb18030Text)
// static char* UTF8ToGB18030_2(const char* utf8Text)
// static char* WideCharToGB18030_2(const wchar_t* wideText)
// static std::wstring GB18030ToWString(const char* gb18030Text)
// static std::wstring UTF8ToWString(const char* utf8Text)
// static std::unique_ptr<char[]> WStringToGB18030(const wchar_t* wideText)
// static std::unique_ptr<char[]> WStringToUTF8(const wchar_t* wideText)
// static CStringW WStringToCStringW(const wchar_t* wideText)
// static std::wstring CStringWToWString(const CStringW& cstringW)
// static std::unique_ptr<char[]> CStringWToUTF8(const CStringW& cstringW)
// static CStringW UTF8ToCStringW(const char* utf8Text)
// static CStringA GB18030ToCStringA(const char* gb18030Text)
// static char* CStringAToGB18030(const CStringA& ansiText)
// static char* CStringWToGB18030(const CStringW& wideText)
// static char* CStringAToUTF8(const CStringA& ansiText)
// static std::unique_ptr<wchar_t[]> CStringAToWideChar(const CStringA& ansiText)
// static std::wstring CStringAToWString(const CStringA& ansiText)
// static CStringW CStringAToCStringW(const CStringA& ansiText)
// static CStringA CStringWToCStringA(const CStringW& wideText)
// static std::wstring StringToWString(const std::string& ansiText)
// static std::string WStringToString(const std::wstring& wideText)
// static wchar_t* CharToWChar(const char* ansiText)
// static char* WCharToChar(const wchar_t* wideText)
// static std::wstring CStringToWString(const CString& cstring)
//
// CharType* unique_ptr_to_raw_ptr(std::unique_ptr<CharType[]>& ptr)


// EXAMPLE:
// const wchar_t* wstr = L"Hello, World!"; // Wide character string
// 
// // Convert wide char to multibyte char
// auto maybeUtf8Text = Convert::wchar_to_char(wstr);
// if (maybeUtf8Text.has_value()) {
//     std::unique_ptr<char[]> utf8Text = std::move(maybeUtf8Text.value());
// 
//     // Get raw pointer from unique_ptr
//     char* rawPtr = unique_ptr_to_raw_ptr(utf8Text);
// 
//     // Print converted string
//     std::cout << "UTF-8 String: " << rawPtr << std::endl;
// }
// else {
//     std::cerr << "Conversion from wide character string to multibyte string failed." << std::endl;
// }

class Convert
{
public:
    static std::string wstring_to_string(const std::wstring& wstr)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    static std::wstring string_to_wstring(const std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }

    static std::optional<std::unique_ptr<wchar_t[]>> char_to_wchar(const char* charText)
    {
        if (charText == nullptr)
            return std::nullopt; // Input is null

        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, charText, -1, nullptr, 0);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CP_UTF8, 0, charText, -1, wideText.get(), requiredSize) == 0)
            return std::nullopt; // Conversion failed

        return std::move(wideText);
    }

    static std::optional<std::unique_ptr<char[]>> wchar_to_char(const wchar_t* wideText)
    {
        if (wideText == nullptr)
            return std::nullopt; // Input is null

        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<char[]> charText(new char[requiredSize]);
        if (WideCharToMultiByte(CP_UTF8, 0, wideText, -1, charText.get(), requiredSize, nullptr, nullptr) == 0)
            return std::nullopt; // Conversion failed

        return std::move(charText);
    }

    static std::optional<std::unique_ptr<char[]>> GB18030ToUTF8(const char* gb18030Text)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, wideText.get(), requiredSize) == 0)
            return std::nullopt; // Conversion failed

        requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideText.get(), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<char[]> utf8Text(new char[requiredSize]);
        if (WideCharToMultiByte(CP_UTF8, 0, wideText.get(), -1, utf8Text.get(), requiredSize, nullptr, nullptr) == 0)
            return std::nullopt; // Conversion failed

        return std::move(utf8Text);
    }

    static std::optional<std::unique_ptr<char[]>> UTF8ToGB18030(const char* utf8Text)
    {
        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, wideText.get(), requiredSize) == 0)
            return std::nullopt; // Conversion failed

        requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText.get(), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<char[]> gb18030Text(new char[requiredSize]);
        if (WideCharToMultiByte(CODEPAGE, 0, wideText.get(), -1, gb18030Text.get(), requiredSize, nullptr, nullptr) == 0)
            return std::nullopt; // Conversion failed

        return std::move(gb18030Text);
    }

    static std::optional<std::unique_ptr<wchar_t[]>> GB18030ToWideChar(const char* gb18030Text)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, wideText.get(), requiredSize) == 0)
            return std::nullopt; // Conversion failed

        return std::move(wideText);
    }

    static std::optional<std::unique_ptr<char[]>> WideCharToGB18030(const wchar_t* wideText)
    {
        int requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return std::nullopt; // Conversion failed

        std::unique_ptr<char[]> gb18030Text(new char[requiredSize]);
        if (WideCharToMultiByte(CODEPAGE, 0, wideText, -1, gb18030Text.get(), requiredSize, nullptr, nullptr) == 0)
            return std::nullopt; // Conversion failed

        return std::move(gb18030Text);
    }

    // USAGE:
    // auto result = gb18030_to_wstring(gb18030Text);
    // if (result.has_value())
    // {
    //     // Conversion succeeded, do something with result.value()
    //     std::wstring convertedText = result.value();
    // }
    // else
    // {
    //     // Conversion failed
    //     // Handle the error appropriately
    // }
    static std::optional<std::wstring> gb18030_to_wstring(const char* gb18030Text)
    {
        std::optional<std::unique_ptr<char[]>> utf8Text(Convert::GB18030ToUTF8(gb18030Text));
        if (!utf8Text)
            return std::nullopt;

        return Convert::string_to_wstring(utf8Text.value().get());
    }

    static std::optional<std::unique_ptr<char[]>> wstring_to_gb18030(const std::wstring& wstr)
    {
        std::string utf8Text = Convert::wstring_to_string(wstr);
        std::optional<std::unique_ptr<char[]>> gb18030Text(Convert::UTF8ToGB18030(utf8Text.c_str()));
        return gb18030Text;
    }

    static char* GB18030ToUTF8_2(const char* gb18030Text)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        wchar_t* wideText = new wchar_t[requiredSize];
        if (MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, wideText, requiredSize) == 0)
        {
            delete[] wideText;
            return nullptr; // Conversion failed
        }

        requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
        {
            delete[] wideText;
            return nullptr; // Conversion failed
        }

        char* utf8Text = new char[requiredSize];
        if (WideCharToMultiByte(CP_UTF8, 0, wideText, -1, utf8Text, requiredSize, nullptr, nullptr) == 0)
        {
            delete[] wideText;
            delete[] utf8Text;
            return nullptr; // Conversion failed
        }

        delete[] wideText;
        return utf8Text;
    }

    static std::unique_ptr<wchar_t[]> GB18030ToWideChar_2(const char* gb18030Text)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, wideText.get(), requiredSize) == 0)
        {
            return nullptr; // Conversion failed
        }

        return wideText;
    }

    static CStringW GB18030ToCStringW(const char* gb18030Text)
    {
        CStringW wideText;
        wideText.SetString(CA2W(gb18030Text, CODEPAGE)); // CODEPAGE: GB18030 code page
        return wideText;
    }

    static char* UTF8ToGB18030_2(const char* utf8Text)
    {
        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, nullptr, 0);
        if (requiredSize == 0)
        {
            return nullptr; // Conversion failed
        }

        wchar_t* wideText = new wchar_t[requiredSize];
        if (MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, wideText, requiredSize) == 0)
        {
            delete[] wideText;
            return nullptr; // Conversion failed
        }

        requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
        {
            delete[] wideText;
            return nullptr; // Conversion failed
        }

        char* gb18030Text = new char[requiredSize];
        if (WideCharToMultiByte(CODEPAGE, 0, wideText, -1, gb18030Text, requiredSize, nullptr, nullptr) == 0)
        {
            delete[] wideText;
            delete[] gb18030Text;
            return nullptr; // Conversion failed
        }

        delete[] wideText;
        return gb18030Text;
    }

    static char* WideCharToGB18030_2(const wchar_t* wideText)
    {
        int requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        char* gb18030Text = new char[requiredSize];
        if (WideCharToMultiByte(CODEPAGE, 0, wideText, -1, gb18030Text, requiredSize, nullptr, nullptr) == 0)
        {
            delete[] gb18030Text;
            return nullptr; // Conversion failed
        }

        return gb18030Text;
    }

    static std::wstring GB18030ToWString(const char* gb18030Text)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return L""; // Conversion failed

        std::vector<wchar_t> wideText(requiredSize);
        if (MultiByteToWideChar(CODEPAGE, 0, gb18030Text, -1, wideText.data(), requiredSize) == 0)
            return L""; // Conversion failed

        return std::wstring(wideText.data());
    }

    static std::wstring UTF8ToWString(const char* utf8Text)
    {
        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, nullptr, 0);
        if (requiredSize == 0)
            return L""; // Conversion failed

        std::vector<wchar_t> wideText(requiredSize);
        if (MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, wideText.data(), requiredSize) == 0)
            return L""; // Conversion failed

        return std::wstring(wideText.data());
    }

    static std::unique_ptr<char[]> WStringToGB18030(const wchar_t* wideText)
    {
        int requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<char[]> gb18030Text(new char[requiredSize]);
        if (WideCharToMultiByte(CODEPAGE, 0, wideText, -1, gb18030Text.get(), requiredSize, nullptr, nullptr) == 0)
            return nullptr; // Conversion failed

        return gb18030Text;
    }

    static std::unique_ptr<char[]> WStringToUTF8(const wchar_t* wideText)
    {
        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideText, -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<char[]> utf8Text(new char[requiredSize]);
        if (WideCharToMultiByte(CP_UTF8, 0, wideText, -1, utf8Text.get(), requiredSize, nullptr, nullptr) == 0)
            return nullptr; // Conversion failed

        return utf8Text;
    }

    static CStringW WStringToCStringW(const wchar_t* wideText)
    {
        CStringW cstringW(wideText);
        return cstringW;
    }

    static std::wstring CStringWToWString(const CStringW& cstringW)
    {
        return static_cast<const wchar_t*>(cstringW);
    }

    static std::unique_ptr<char[]> CStringWToUTF8(const CStringW& cstringW)
    {
        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, static_cast<const wchar_t*>(cstringW), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<char[]> utf8Text(new char[requiredSize]);
        if (WideCharToMultiByte(CP_UTF8, 0, static_cast<const wchar_t*>(cstringW), -1, utf8Text.get(), requiredSize, nullptr, nullptr) == 0)
            return nullptr; // Conversion failed

        return utf8Text;
    }

    static CStringW UTF8ToCStringW(const char* utf8Text)
    {
        CStringW cstringW;
        cstringW.SetString(CA2W(utf8Text, CP_UTF8)); // CP_UTF8: UTF-8 code page
        return cstringW;
    }

    static CStringA GB18030ToCStringA(const char* gb18030Text)
    {
        CStringA ansiText;
        ansiText.SetString(CA2A(gb18030Text, CODEPAGE)); // CODEPAGE: GB18030 code page
        return ansiText;
    }

    static char* CStringAToGB18030(const CStringA& ansiText)
    {
        int requiredSize = MultiByteToWideChar(CODEPAGE, 0, static_cast<LPCSTR>(ansiText), -1, nullptr, 0);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CODEPAGE, 0, static_cast<LPCSTR>(ansiText), -1, wideText.get(), requiredSize) == 0)
            return nullptr; // Conversion failed

        requiredSize = WideCharToMultiByte(CODEPAGE, 0, wideText.get(), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        char* gb18030Text = new char[requiredSize];
        if (WideCharToMultiByte(CODEPAGE, 0, wideText.get(), -1, gb18030Text, requiredSize, nullptr, nullptr) == 0)
        {
            delete[] gb18030Text;
            return nullptr; // Conversion failed
        }

        return gb18030Text;
    }

    static char* CStringWToGB18030(const CStringW& wideText)
    {
        int requiredSize = WideCharToMultiByte(CODEPAGE, 0, static_cast<LPCWSTR>(wideText), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<char[]> gb18030Text(new char[requiredSize]);
        if (WideCharToMultiByte(CODEPAGE, 0, static_cast<LPCWSTR>(wideText), -1, gb18030Text.get(), requiredSize, nullptr, nullptr) == 0)
            return nullptr; // Conversion failed

        return gb18030Text.release();
    }

    static char* CStringAToUTF8(const CStringA& ansiText)
    {
        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, static_cast<LPCSTR>(ansiText), -1, nullptr, 0);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CP_UTF8, 0, static_cast<LPCSTR>(ansiText), -1, wideText.get(), requiredSize) == 0)
            return nullptr; // Conversion failed

        requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideText.get(), -1, nullptr, 0, nullptr, nullptr);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        char* utf8Text = new char[requiredSize];
        if (WideCharToMultiByte(CP_UTF8, 0, wideText.get(), -1, utf8Text, requiredSize, nullptr, nullptr) == 0)
        {
            delete[] utf8Text;
            return nullptr; // Conversion failed
        }

        return utf8Text;
    }

    static std::unique_ptr<wchar_t[]> CStringAToWideChar(const CStringA& ansiText)
    {
        int requiredSize = MultiByteToWideChar(CP_ACP, 0, static_cast<LPCSTR>(ansiText), -1, nullptr, 0);
        if (requiredSize == 0)
            return nullptr; // Conversion failed

        std::unique_ptr<wchar_t[]> wideText(new wchar_t[requiredSize]);
        if (MultiByteToWideChar(CP_ACP, 0, static_cast<LPCSTR>(ansiText), -1, wideText.get(), requiredSize) == 0)
            return nullptr; // Conversion failed

        return wideText;
    }

    static std::wstring CStringAToWString(const CStringA& ansiText)
    {
        int requiredSize = MultiByteToWideChar(CP_ACP, 0, static_cast<LPCSTR>(ansiText), -1, nullptr, 0);
        if (requiredSize == 0)
            return L""; // Conversion failed

        std::vector<wchar_t> wideText(requiredSize);
        if (MultiByteToWideChar(CP_ACP, 0, static_cast<LPCSTR>(ansiText), -1, wideText.data(), requiredSize) == 0)
            return L""; // Conversion failed

        return std::wstring(wideText.data());
    }

    static CStringW CStringAToCStringW(const CStringA& ansiText)
    {
        CStringW wideText;
        wideText.SetString(CA2W(static_cast<LPCSTR>(ansiText), CP_ACP)); // CP_ACP: ANSI code page
        return wideText;
    }

    static CStringA CStringWToCStringA(const CStringW& wideText)
    {
        CStringA ansiText;
        ansiText.SetString(CW2A(static_cast<LPCWSTR>(wideText), CP_ACP)); // CP_ACP: ANSI code page
        return ansiText;
    }

    static std::wstring StringToWString(const std::string& ansiText)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(ansiText);
    }

    static std::string WStringToString(const std::wstring& wideText)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wideText);
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

    static std::wstring CStringToWString(const CString& cstring)
    {
        CT2W converter(cstring); // CT2W is a helper class for converting CString to std::wstring
        return std::wstring(converter);
    }
};

// USAGE:
// std::unique_ptr<char[]> utf8Text(Convert::GB18030ToUTF8(gb18030Text));
// char* rawPtr = unique_ptr_to_raw_ptr(utf8Text);
//
// std::unique_ptr<wchar_t[]> wideText(Convert::GB18030ToWideChar(gb18030Text));
// wchar_t* rawWidePtr = unique_ptr_to_raw_ptr(wideText);
template<typename CharType>
CharType* unique_ptr_to_raw_ptr(std::unique_ptr<CharType[]>& ptr)
{
    return ptr.get();
}

template<typename StringType>
StringType trim(const StringType& str)
{
    if (str.empty()) return str;

    size_t start = 0;
    size_t end = str.size() - 1;

    // Trim leading whitespace
    while (start <= end && std::isspace(str[start]))
        ++start;

    // Trim trailing whitespace
    while (end > start && std::isspace(str[end]))
        --end;

    return str.substr(start, end - start + 1);
}

template<typename StringType>
std::vector<StringType> split(const StringType& str, typename StringType::value_type delimiter)
{
    std::vector<StringType> tokens;
    std::basic_istringstream<typename StringType::value_type> ss(str);
    StringType token;
    while (std::getline(ss, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

template<typename StringType>
StringType replace(const StringType& str, const StringType& oldValue, const StringType& newValue)
{
    StringType result = str;
    size_t pos = 0;
    while ((pos = result.find(oldValue, pos)) != StringType::npos)
    {
        result.replace(pos, oldValue.length(), newValue);
        pos += newValue.length();
    }
    return result;
}

template<typename StringType>
StringType strip(const StringType& str)
{
    if (str.empty()) return str;

    size_t end = str.size() - 1;

    // Trim trailing '\0' characters
    while (end > 0 && str[end] == '\0')
        --end;

    return str.substr(0, end + 1);
}

//template<typename StringType>
//std::basic_string<typename StringType::value_type> toLower(const StringType& str)
//{
//    auto transformFunc = [](typename StringType::value_type c)
//    {
//        if constexpr (std::is_same_v<typename StringType::value_type, char>)
//            return std::tolower(static_cast<unsigned char>(c));
//        else if constexpr (std::is_same_v<typename StringType::value_type, wchar_t>)
//            return std::towlower(c);
//    };
//
//    return std::ranges::to<std::basic_string<typename StringType::value_type>>(str | std::views::transform(transformFunc));
//}
//
//template<typename StringType>
//StringType toUpper(const StringType& str)
//{
//    auto transformFunc = [](typename StringType::value_type c)
//    {
//        if constexpr (std::is_same_v<typename StringType::value_type, char>)
//            return std::toupper(static_cast<unsigned char>(c));
//        else if constexpr (std::is_same_v<typename StringType::value_type, wchar_t>)
//            return std::towupper(c);
//    };
//
//    return std::ranges::to<StringType>(str | std::views::transform(transformFunc));
//}

#endif
