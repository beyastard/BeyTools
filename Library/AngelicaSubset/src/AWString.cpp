#include "AWString.h"

#include <cwctype>
#include <algorithm>

struct s_EMPTYSTRING
{
	AWString::s_STRINGDATA	Data;

	wchar_t	szStr[1];

	s_EMPTYSTRING()
	{
		Data.iRefs = 0;
		Data.iDataLen = 0;
		Data.iMaxLen = 0;
		szStr[0] = '\0';
	}
};

//	For an empty string, m_pchData will point here
static s_EMPTYSTRING l_EmptyString;
wchar_t* AWString::m_pEmptyStr = l_EmptyString.szStr;

/*	Alocate string buffer

	iLen: length of data (not including terminator)
*/
wchar_t* AWString::AllocBuffer(int32_t iLen)
{
	s_STRINGDATA* pData;
	char* pBuf;

	if (iLen < 64)
	{
		pBuf = new char[sizeof(wchar_t) * 64 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 63;
	}
	else if (iLen < 128)
	{
		pBuf = new char[sizeof(wchar_t) * 128 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 127;
	}
	else if (iLen < 256)
	{
		pBuf = new char[sizeof(wchar_t) * 256 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 255;
	}
	else if (iLen < 512)
	{
		pBuf = new char[sizeof(wchar_t) * 512 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 511;
	}
	else
	{
		pBuf = new char[sizeof(wchar_t) * (iLen + 1) + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = iLen;
	}

	return reinterpret_cast<wchar_t*>(pBuf + sizeof(s_STRINGDATA));
}

//	Free string data buffer
void AWString::FreeBuffer(s_STRINGDATA* pStrData)
{
	switch (pStrData->iRefs)
	{
	case 0:
		return;
	case 1:
		delete[] reinterpret_cast<char*>(pStrData);
		break;
	default:
		pStrData->iRefs--;
		break;
	}
}

//	Copy iLen characters from szSrc to szDest and add terminator at the tail of szDest
void AWString::StringCopy(wchar_t* szDest, const wchar_t* szSrc, int32_t iLen)
{
	int32_t i = 0;
	for ( ; i < iLen; ++i)
		szDest[i] = szSrc[i];

	szDest[i] = '\0';
}

//	Judge whether two strings are equal
bool AWString::StringEqual(const wchar_t* s1, const wchar_t* s2, int32_t iLen)
{
	// Define the pointer type for faster comparison
	const uint64_t* p1 = reinterpret_cast<const uint64_t*>(s1);
	const uint64_t* p2 = reinterpret_cast<const uint64_t*>(s2);

	// Calculate the number of iterations based on the size of uint64_t
	int32_t numIterations = iLen / sizeof(uint64_t);

	// Perform comparison in blocks of uint64_t
	for (int32_t i = 0; i < numIterations; ++i, ++p1, ++p2)
	{
		if (*p1 != *p2)
			return false;
	}

	// Handle remaining characters
	int32_t remainingBytes = iLen % sizeof(uint64_t);
	const wchar_t* pChar1 = reinterpret_cast<const wchar_t*>(p1);
	const wchar_t* pChar2 = reinterpret_cast<const wchar_t*>(p2);

	for (int32_t i = 0; i < remainingBytes; ++i)
	{
		if (pChar1[i] == L'\0' || pChar2[i] == L'\0') // Check for null terminator
			return pChar1[i] == pChar2[i];            // Check if both strings terminate at the same position

		if (pChar1[i] != pChar2[i])
			return false;
	}

	return true;
}

//	Allocate memory and copy string
wchar_t* AWString::AllocThenCopy(const wchar_t* szSrc, int32_t iLen)
{
	if (!iLen)
		return m_pEmptyStr;

	wchar_t* s = AllocBuffer(iLen);
	StringCopy(s, szSrc, iLen);

	return s;
}

//	Allocate a new string which is merged by szSrc + ch
wchar_t* AWString::AllocThenCopy(const wchar_t* szSrc, wchar_t c, int32_t iLen)
{
	if (!c)
		return AllocThenCopy(szSrc, iLen - 1);

	wchar_t* s = AllocBuffer(iLen);
	StringCopy(s, szSrc, iLen - 1);

	s[iLen - 1] = c;
	s[iLen] = '\0';

	return s;
}

//	Allocate a new string which is merged by ch + szSrc
wchar_t* AWString::AllocThenCopy(wchar_t c, const wchar_t* szSrc, int32_t iLen)
{
	if (!c)
		return l_EmptyString.szStr;

	wchar_t* s = AllocBuffer(iLen);

	s[0] = c;
	StringCopy(s + 1, szSrc, iLen - 1);

	return s;
}

//	Allocate a new string which is merged by s1 + s2
wchar_t* AWString::AllocThenCopy(const wchar_t* s1, const wchar_t* s2, int32_t iLen1, int32_t iLen2)
{
	if (!iLen2)
		return AllocThenCopy(s1, iLen1);

	int32_t iLen = iLen1 + iLen2;
	wchar_t* s = AllocBuffer(iLen);

	StringCopy(s, s1, iLen1);
	StringCopy(s + iLen1, s2, iLen2);

	return s;
}

AWString::AWString(const AWString& s)
{
	if (s.IsEmpty())
	{
		m_pStr = m_pEmptyStr;
		return;
	}

	s_STRINGDATA* pSrcData = s.GetData();

	if (pSrcData->iRefs == -1)	//	Source string is being locked
	{
		s_STRINGDATA* pData = GetData();
		m_pStr = AllocThenCopy(s.m_pStr, pSrcData->iDataLen);
	}
	else
	{
		pSrcData->iRefs++;
		m_pStr = s.m_pStr;
	}
}

AWString::AWString(const wchar_t* szStr)
{
	m_pStr = AllocThenCopy(szStr, SafeStrLen(szStr));
}

AWString::AWString(const wchar_t* szStr, int32_t iLen)
{
	m_pStr = AllocThenCopy(szStr, iLen);
}

AWString::AWString(wchar_t c, int32_t iRepeat)
{
	m_pStr = AllocBuffer(iRepeat);

	for (int32_t i = 0; i < iRepeat; ++i)
		m_pStr[i] = c;

	m_pStr[iRepeat] = '\0';
}

AWString::AWString(const AWString& str1, const AWString& str2)
{
	m_pStr = AllocThenCopy(str1.m_pStr, str2.m_pStr, str1.GetLength(), str2.GetLength());
}

AWString::AWString(wchar_t c, const AWString& s)
{
	m_pStr = AllocThenCopy(c, s.m_pStr, s.GetLength() + 1);
}

AWString::AWString(const AWString& s, wchar_t c)
{
	m_pStr = AllocThenCopy(s.m_pStr, c, s.GetLength() + 1);
}

AWString::AWString(const wchar_t* szStr, const AWString& s)
{
	m_pStr = AllocThenCopy(szStr, s.m_pStr, SafeStrLen(szStr), s.GetLength());
}

AWString::AWString(const AWString& s, const wchar_t* szStr)
{
	m_pStr = AllocThenCopy(s.m_pStr, szStr, s.GetLength(), SafeStrLen(szStr));
}

AWString::~AWString()
{
	s_STRINGDATA* pData = GetData();

	if (pData->iRefs == -1)	//	Buffer is being locked
		pData->iRefs = 1;

	FreeBuffer(pData);
	m_pStr = m_pEmptyStr;
}

const AWString& AWString::operator = (wchar_t c)
{
	if (!c)
	{
		Empty();
		return *this;
	}

	s_STRINGDATA* pData = GetData();

	if (IsEmpty())
		m_pStr = AllocBuffer(1);
	else if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocBuffer(1);
	}
	else
		pData->iRefs = 1;

	m_pStr[0] = c;
	m_pStr[1] = '\0';

	GetData()->iDataLen = 1;

	return *this;
}

const AWString& AWString::operator = (const wchar_t* szStr)
{
	int32_t iLen = SafeStrLen(szStr);
	if (!iLen)
	{
		Empty();
		return *this;
	}

	s_STRINGDATA* pData = GetData();

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(szStr, iLen);
	}
	else
	{
		if (iLen <= pData->iMaxLen)
		{
			StringCopy(m_pStr, szStr, iLen);
			pData->iDataLen = iLen;
		}
		else
		{
			FreeBuffer(pData);
			m_pStr = AllocThenCopy(szStr, iLen);
		}
	}

	return *this;
}

const AWString& AWString::operator = (const AWString& s)
{
	if (m_pStr == s.m_pStr)
		return *this;

	if (s.IsEmpty())
	{
		Empty();
		return *this;
	}

	s_STRINGDATA* pSrcData = s.GetData();

	if (pSrcData->iRefs == -1)	// Source string is being locked
	{
		s_STRINGDATA* pData = GetData();

		if (pData->iRefs > 1)
		{
			pData->iRefs--;
			m_pStr = AllocThenCopy(s.m_pStr, pSrcData->iDataLen);
		}
		else
		{
			if (pSrcData->iDataLen <= pData->iMaxLen)
			{
				StringCopy(m_pStr, s.m_pStr, pSrcData->iDataLen);
				pData->iDataLen = pSrcData->iDataLen;
			}
			else
			{
				FreeBuffer(pData);
				m_pStr = AllocThenCopy(s.m_pStr, pSrcData->iDataLen);
			}
		}
	}
	else
	{
		FreeBuffer(GetData());
		pSrcData->iRefs++;
		m_pStr = s.m_pStr;
	}

	return *this;
}

const AWString& AWString::operator += (wchar_t c)
{
	if (!c)
		return *this;

	s_STRINGDATA* pData = GetData();

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, c, pData->iDataLen + 1);
		return *this;
	}

	int32_t iLen = pData->iDataLen + 1;
	if (iLen <= pData->iMaxLen)
	{
		m_pStr[iLen - 1] = c;
		m_pStr[iLen] = '\0';
		pData->iDataLen++;
	}
	else
	{
		m_pStr = AllocThenCopy(m_pStr, c, iLen);
		FreeBuffer(pData);
	}

	return *this;
}

const AWString& AWString::operator += (const wchar_t* szStr)
{
	int32_t iLen2 = SafeStrLen(szStr);
	if (!iLen2)
		return *this;

	s_STRINGDATA* pData = GetData();

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, szStr, pData->iDataLen, iLen2);
		return *this;
	}

	int32_t iLen = pData->iDataLen + iLen2;
	if (iLen <= pData->iDataLen)
	{
		StringCopy(m_pStr + pData->iDataLen, szStr, iLen2);
		pData->iDataLen = iLen;
	}
	else
	{
		m_pStr = AllocThenCopy(m_pStr, szStr, pData->iDataLen, iLen2);
		FreeBuffer(pData);
	}

	return *this;
}

const AWString& AWString::operator += (const AWString& s)
{
	int32_t iLen2 = s.GetLength();
	if (!iLen2)
		return *this;

	s_STRINGDATA* pData = GetData();

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, s.m_pStr, pData->iDataLen, iLen2);
		return *this;
	}

	int32_t iLen = pData->iDataLen + iLen2;
	if (iLen <= pData->iMaxLen)
	{
		StringCopy(m_pStr + pData->iDataLen, s.m_pStr, iLen2);
		pData->iDataLen = iLen;
	}
	else
	{
		m_pStr = AllocThenCopy(m_pStr, s.m_pStr, pData->iDataLen, iLen2);
		FreeBuffer(pData);
	}

	return *this;
}

int32_t AWString::Compare(const wchar_t* szStr) const
{
	return (m_pStr == szStr) ? 0 : std::wcscmp(m_pStr, szStr);
}

int32_t AWString::CompareNoCase(const wchar_t* szStr) const
{
	if (m_pStr == szStr)
		return 0;

	for (size_t i = 0; m_pStr[i] != L'\0' && szStr[i] != L'\0'; ++i)
	{
		wchar_t c1 = std::towlower(m_pStr[i]);
		wchar_t c2 = std::towlower(szStr[i]);

		if (c1 != c2)
			return c1 - c2;
	}

	// Check if both strings ended at the same time
	if (std::wcslen(m_pStr) == std::wcslen(szStr))
		return 0;

	// Return a positive or negative value indicating the comparison result
	return std::wcslen(m_pStr) > std::wcslen(szStr) ? 1 : -1;
}

bool AWString::operator == (const wchar_t* szStr) const
{
	//	Note: szStr's boundary may be crossed when StringEqual() do
	//		  read operation, if szStr is shorter than 'this'. Now, this
	//		  read operation won't cause problem, but in the future,
	//		  should we check the length of szStr at first, and put the 
	//		  shorter one between 'this' and szStr front when we call StringEqual ?
	return StringEqual(m_pStr, szStr, GetLength() + 1);
}

bool AWString::operator == (const AWString& s) const
{
	if (m_pStr == s.m_pStr)
		return true;

	int32_t iLen = GetLength();
	if (iLen != s.GetLength())
		return false;

	return StringEqual(m_pStr, s.m_pStr, iLen);
}

wchar_t& AWString::operator [] (int32_t n)
{
	assert(n >= 0 && n <= GetLength());

	s_STRINGDATA* pData = GetData();
	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, GetLength());
	}

	return m_pStr[n];
}

//	Convert to upper case
void AWString::MakeUpper()
{
	int32_t iLen = GetLength();
	if (!iLen)
		return;

	s_STRINGDATA* pData = GetData();
	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, iLen);
	}

	std::transform(m_pStr, m_pStr + iLen, m_pStr, [](wchar_t c) { return std::towupper(c); });
}

//	Convert to lower case
void AWString::MakeLower()
{
	int32_t iLen = GetLength();
	if (!iLen)
		return;

	s_STRINGDATA* pData = GetData();
	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, iLen);
	}

	std::transform(m_pStr, m_pStr + iLen, m_pStr, [](wchar_t c) { return std::towlower(c); });
}

//	Format string
AWString& AWString::Format(const wchar_t* szFormat, ...)
{
	va_list argList;
	va_start(argList, szFormat);

	int32_t iNumWritten;
	int32_t iMaxLen = GetFormatLen(szFormat, argList) + 1;

	s_STRINGDATA* pData = GetData();

	if (pData->iRefs > 1)
		pData->iRefs--;
	else if (iMaxLen <= pData->iMaxLen)
	{
		std::vswprintf(m_pStr, szFormat, argList);
		pData->iDataLen = SafeStrLen(m_pStr);
		{
			va_end(argList);
			return *this;
		}
	}
	else	//	iMaxLen > pData->iMaxLen
		FreeBuffer(pData);

	m_pStr = AllocBuffer(iMaxLen);
	iNumWritten = std::vswprintf(m_pStr, szFormat, argList);
	assert(iNumWritten < iMaxLen);
	GetData()->iDataLen = SafeStrLen(m_pStr);
	
	va_end(argList);
	return *this;
}

/*	Get buffer. If you have changed content buffer returned by GetBuffer(), you
	must call ReleaseBuffer() later. Otherwise, ReleaseBuffer() isn't necessary.

	Return buffer's address for success, otherwise return nullptr.

	iMinSize: number of bytes in string buffer user can changed.
*/
wchar_t* AWString::GetBuffer(int32_t iMinSize)
{
	if (iMinSize < 0)
	{
		assert(iMinSize >= 0);
		return nullptr;
	}

	//	Ensure we won't allocate an empty string when iMinSize == 1
	if (!iMinSize)
		iMinSize = 1;

	s_STRINGDATA* pData = GetData();

	if (IsEmpty())
	{
		m_pStr = AllocBuffer(iMinSize);
		m_pStr[0] = '\0';
		GetData()->iDataLen = 0;
	}
	else if (pData->iRefs > 1)
	{
		pData->iRefs--;

		if (iMinSize <= pData->iDataLen)
			m_pStr = AllocThenCopy(m_pStr, pData->iDataLen);
		else
		{
			wchar_t* wszOld = m_pStr;
			m_pStr = AllocBuffer(iMinSize);
			StringCopy(m_pStr, wszOld, pData->iDataLen);
			GetData()->iDataLen = pData->iDataLen;
		}
	}
	else if (iMinSize > pData->iMaxLen)
	{
		wchar_t* szOld = m_pStr;
		m_pStr = AllocBuffer(iMinSize);
		StringCopy(m_pStr, szOld, pData->iDataLen);
		GetData()->iDataLen = pData->iDataLen;
		FreeBuffer(pData);
	}

	return m_pStr;
}

/*	If you have changed content of buffer returned by GetBuffer(), you must call
	ReleaseBuffer() later. Otherwise, ReleaseBuffer() isn't necessary.

	iNewSize: new size in bytes of string. -1 means string is zero ended and it's
			  length can be got by strlen().
*/
void AWString::ReleaseBuffer(int32_t iNewSize/* -1 */)
{
	s_STRINGDATA* pData = GetData();
	if (pData->iRefs != 1)
	{
		assert(pData->iRefs == 1);	//	Ensure GetBuffer has been called.
		return;
	}

	if (iNewSize == -1)
		iNewSize = SafeStrLen(m_pStr);

	if (iNewSize > pData->iMaxLen)
	{
		assert(iNewSize <= pData->iMaxLen);
		return;
	}

	if (iNewSize == 0)
		Empty();
	else
	{
		pData->iDataLen = iNewSize;
		m_pStr[iNewSize] = '\0';
	}
}

//	Lock buffer. Locked buffer disable reference counting
wchar_t* AWString::LockBuffer()
{
	if (IsEmpty())
	{
		assert(!IsEmpty());
		return nullptr;
	}

	s_STRINGDATA* pData = GetData();
	if (pData->iRefs <= 0)
	{
		assert(pData->iRefs > 0);	// Buffer has been locked ?
		return nullptr;
	}

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, pData->iDataLen);
		pData = GetData();
	}

	pData->iRefs = -1;

	return m_pStr;
}

//	Unlock buffer
void AWString::UnlockBuffer()
{
	s_STRINGDATA* pData = GetData();
	if (pData->iRefs >= 0)
	{
		assert(pData->iRefs < 0);	// Buffer must has been locked.
		return;
	}

	pData->iDataLen = SafeStrLen(m_pStr);
	pData->iRefs = 1;
}

// Make a guess at the maximum length of the resulting string.
// Now this function doesn't support UNICODE string.
// I64 modifier used in WIN32's sprintf is added in 2010.1.10
// 
// Return estimated length of resulting string.


constexpr auto FORCE_ANSI    = 0x10000;
constexpr auto FORCE_UNICODE = 0x20000;
constexpr auto FORCE_INT64   = 0x40000;

int32_t AWString::GetFormatLen(const wchar_t* szFormat, va_list argList)
{
	if (!szFormat || !szFormat[0])
		return 0;

	wchar_t* pszTemp = nullptr;
	int32_t iMaxLen = 0;
	int32_t iTempBufLen = 0;

	for (const wchar_t* pch = szFormat; *pch != '\0'; ++pch)
	{
		// Handle '%' character, but watch out for '%%'
		if (*pch != '%' || *(++pch) == '%')
		{
			iMaxLen++;
			continue;
		}

		int32_t iItemLen = 0;
		int32_t iWidth = 0;

		// Handle '%' character with format
		for (; *pch != '\0'; ++pch)
		{
			// Check for valid flags
			switch (*pch)
			{
			case '#':
				iMaxLen += 2;   // for '0x'
				break;
			case '*':
				iWidth = va_arg(argList, int32_t);
				break;
			case '-':
			case '+':
			case '0':
			case ' ':
			default:
				break;
			}
		}

		// Get width and skip it
		if (iWidth == 0)
		{
			// Width indicated by digit
			iWidth = std::wcstol(pch, nullptr, 10);
			for (; *pch != '\0' && (*pch >= '0' && *pch <= '9'); ++pch)
				;
		}

		assert(iWidth >= 0);
		int32_t iPrecision = 0;

		if (*pch == '.')
		{
			// Skip past '.' separator (width.precision)
			pch++;

			// Get precision and skip it
			if (*pch == '*')
			{
				iPrecision = va_arg(argList, int32_t);
				pch++;
			}
			else
			{
				iPrecision = std::wcstol(pch, nullptr, 10);
				for (; *pch != '\0' && (*pch >= '0' && *pch <= '9'); pch++)
					;
			}

			assert(iPrecision >= 0);
		}

		// should be on type modifier or specifier
		int32_t nModifier = 0;
		if (std::wcsncmp(pch, L"I64", 3) == 0) // Note that I64 is Microsoft-specific
		{
			pch += 3;
			nModifier = FORCE_INT64;
			// __int64 is only available on X86 and ALPHA platforms
		}
		else
		{
			switch (*pch)
			{
			case 'h':
			case 'l':
			case 'F':
			case 'N':
			case 'L':
				pch++;
				break;
			}
		}

		switch (*pch)
		{
		case 'c':	// Single characters
		case 'C':
			iItemLen = 2;
			va_arg(argList, wchar_t);
			break;

		case 's':	// Strings
		{
			const wchar_t* pstrNextArg = va_arg(argList, const wchar_t*);
			if (!pstrNextArg)
				iItemLen = 6;	//	"(null)"
			else
				iItemLen = pstrNextArg[0] == '\0' ? 1 : (int32_t)std::wcslen(pstrNextArg);

			break;
		}
		case 'S':
		{
#ifdef UNICODE
			const char* pstrNextArg = va_arg(argList, const char*);
			if (pstrNextArg == nullptr)
				iItemLen = 6;	//	"(null)"
			else
				iItemLen = pstrNextArg[0] == '\0' ? 1 : (int32_t)std::strlen(pstrNextArg);

#else	//	ANSI
			const wchar_t* pstrNextArg = va_arg(argList, const wchar_t*);
			if (!pstrNextArg)
				iItemLen = 6;	//	"(null)"
			else
				iItemLen = pstrNextArg[0] == '\0' ? 1 : (int32_t)std::wcslen(pstrNextArg);
#endif
			break;
		}
		}

		//	Adjust iItemLen for strings
		if (iItemLen != 0)
		{
			if (iPrecision != 0 && iPrecision < iItemLen)
				iItemLen = iPrecision;

			if (iWidth > iItemLen)
				iItemLen = iWidth;
		}
		else
		{
			switch (*pch)
			{
			case 'd':	//	Integers
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					va_arg(argList, int64_t);
				else
					va_arg(argList, int32_t);

				iItemLen = iWidth + iPrecision > 32 ? iWidth + iPrecision : 32;
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, s_DOUBLE);	//	For _X86_
				iItemLen = iWidth + iPrecision > 128 ? iWidth + iPrecision : 128;
				break;

			case 'f':
			{
				//	312 == strlen("-1+(309 zeroes).")
				//	309 zeroes == max precision of a double
				//	6 == adjustment in case precision is not specified,
				//		 which means that the precision defaults to 6
				int32_t iSize = 312 + iPrecision + 6;
				if (iWidth > iSize)
					iSize = iWidth;

				if (iTempBufLen < iSize)
				{
					pszTemp = pszTemp
						? (wchar_t*)std::realloc(pszTemp, iSize)
						: (wchar_t*)std::malloc(iSize);

					iTempBufLen = iSize;
				}

				double f = va_arg(argList, double);
				assert(pszTemp != nullptr);
				std::swprintf(pszTemp, L"%*.*f", iWidth, iPrecision + 6, f);
				iItemLen = (int32_t)std::wcslen(pszTemp);

				break;
			}
			case 'p':
				va_arg(argList, void*);
				iItemLen = iWidth + iPrecision > 32 ? iWidth + iPrecision : 32;
				break;

				//	No output
			case 'n':
				va_arg(argList, int32_t*);
				break;

			default:
				assert(0);  //	Unknown formatting option
				break;
			}
		}

		//	Adjust iMaxLen for output iItemLen
		iMaxLen += iItemLen;
	}

	if (pszTemp)
		std::free(pszTemp);

	return iMaxLen;
}

//	Cut left sub string
void AWString::CutLeft(int32_t n)
{
	if (!GetLength() || n <= 0)
		return;

	s_STRINGDATA* pData = GetData();

	if (n >= pData->iDataLen)
	{
		Empty();
		return;
	}

	int32_t iNewLen = pData->iDataLen - n;

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr + n, iNewLen);
		return;
	}

	for (int32_t i = 0; i < iNewLen; ++i)
		m_pStr[i] = m_pStr[n + i];

	m_pStr[iNewLen] = '\0';
	pData->iDataLen = iNewLen;
}

//	Cut right sub string
void AWString::CutRight(int32_t n)
{
	if (!GetLength() || n <= 0)
		return;

	s_STRINGDATA* pData = GetData();

	if (n >= pData->iDataLen)
	{
		Empty();
		return;
	}

	int32_t iNewLen = pData->iDataLen - n;

	if (pData->iRefs > 1)
	{
		pData->iRefs--;
		m_pStr = AllocThenCopy(m_pStr, iNewLen);
		return;
	}

	m_pStr[iNewLen] = '\0';
	pData->iDataLen = iNewLen;
}

// Trim left
void AWString::TrimLeft()
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::wcslen(m_pStr), [](wchar_t c) { return c > 32; }) - m_pStr;

	CutLeft(static_cast<int32_t>(i));
}

// Trim left
void AWString::TrimLeft(wchar_t c)
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::wcslen(m_pStr), [c](wchar_t ch) { return ch != c; }) - m_pStr;

	CutLeft(static_cast<int32_t>(i));
}

// Trim left
void AWString::TrimLeft(const wchar_t* szChars)
{
	if (!GetLength())
		return;

	std::wstring_view strView(m_pStr);
	std::wstring_view charsView(szChars);

	int32_t i = 0;
	while (i < static_cast<int32_t>(strView.size()) && std::find(charsView.begin(), charsView.end(), strView[i]) != charsView.end())
		++i;

	CutLeft(i);
}

// Trim right
void AWString::TrimRight()
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::wcslen(m_pStr), [](wchar_t c) { return c > 32; }) - m_pStr;

	CutRight(static_cast<int32_t>(std::wcslen(m_pStr) - i));
}

// Trim right
void AWString::TrimRight(wchar_t c)
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::wcslen(m_pStr), [c](wchar_t ch) { return ch != c; }) - m_pStr;

	CutRight(static_cast<int32_t>(std::wcslen(m_pStr) - i));
}

// Trim right
void AWString::TrimRight(const wchar_t* szChars)
{
	if (!GetLength())
		return;

	std::wstring_view strView(m_pStr);
	std::wstring_view charsView(szChars);

	int32_t i = static_cast<int32_t>(strView.size()) - 1;
	int32_t j = 0;

	for (; i >= 0; --i)
	{
		for (; szChars[j]; ++j)
		{
			if (m_pStr[i] == szChars[j])
				break;
		}

		if (!szChars[j])
			break;
	}

	CutRight(static_cast<int32_t>(strView.size()) - i - 1);
}

//	Finds a character inside a larger string. 
//	Return -1 for failure.
size_t AWString::Find(wchar_t c, int32_t iStart/* 0 */) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || iStart < 0 || iStart >= iLen)
		? -1
		: std::find(m_pStr + iStart, m_pStr + iLen, c) - m_pStr;
}

//	Finds a substring inside a larger string. 
//	Return -1 for failure.
size_t AWString::Find(const wchar_t* szSub, int32_t iStart/* 0 */) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || iStart < 0 || iStart >= iLen)
		? -1
		: std::wcsstr(m_pStr + iStart, szSub) - m_pStr;
}

//	Finds a character inside a larger string; starts from the end. 
//	Return -1 for failure.
size_t AWString::ReverseFind(wchar_t c) const
{
	int32_t iLen = GetLength();
	return (iLen == 0)
		? -1
		: std::wcsrchr(m_pStr, c) - m_pStr;
}

//	Finds the first matching character from a set. 
//	Return -1 for failure.
size_t AWString::FindOneOf(const wchar_t* szCharSet) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || std::wcscspn(m_pStr, szCharSet) == iLen) ? -1 : 0;
}
