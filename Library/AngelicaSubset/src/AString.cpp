#include "AString.h"

struct s_EMPTYSTRING
{
	AString::s_STRINGDATA Data;

	char szStr[1];

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
char* AString::m_pEmptyStr = l_EmptyString.szStr;

/*	Alocate string buffer

	iLen: length of data (not including terminator)
*/
char* AString::AllocBuffer(int32_t iLen)
{
	s_STRINGDATA* pData;
	char* pBuf;

	if (iLen < 64)
	{
		pBuf = new char[64 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 63;
	}
	else if (iLen < 128)
	{
		pBuf = new char[128 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 127;
	}
	else if (iLen < 256)
	{
		pBuf = new char[256 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 255;
	}
	else if (iLen < 512)
	{
		pBuf = new char[512 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = 511;
	}
	else
	{
		pBuf = new char[iLen + 1 + sizeof(s_STRINGDATA)];
		pData = (s_STRINGDATA*)pBuf;

		pData->iRefs = 1;
		pData->iDataLen = iLen;
		pData->iMaxLen = iLen;
	}

	return pBuf + sizeof(s_STRINGDATA);
}

//	Free string data buffer
void AString::FreeBuffer(s_STRINGDATA* pStrData)
{
	switch (pStrData->iRefs)
	{
	case 0:
		return;
	case 1:
		delete[] pStrData;
		break;
	default:
		pStrData->iRefs--;
		break;
	}
}

//	Copy iLen characters from szSrc to szDest and add terminator at the tail of szDest
void AString::StringCopy(char* szDest, const char* szSrc, int32_t iLen)
{
	int32_t i;
	int32_t iSpan = sizeof(long);
	const long* p1 = (const long*)szSrc;
	long* p2 = (long*)szDest;

	for (i = 0; i < iLen / iSpan; ++i, ++p1, ++p2)
		*p2 = *p1;

	for (i *= iSpan; i < iLen; ++i)
		szDest[i] = szSrc[i];

	szDest[i] = '\0';
}

//	Judge whether two strings are equal
bool AString::StringEqual(const char* s1, const char* s2, int32_t iLen)
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

	// Handle remaining bytes
	int32_t remainingBytes = iLen % sizeof(uint64_t);
	const char* pChar1 = reinterpret_cast<const char*>(p1);
	const char* pChar2 = reinterpret_cast<const char*>(p2);

	for (int32_t i = 0; i < remainingBytes; ++i)
	{
		if (pChar1[i] != pChar2[i])
			return false;
	}

	return true;
}

//	Allocate memory and copy string
char* AString::AllocThenCopy(const char* szSrc, int32_t iLen)
{
	if (!iLen)
		return m_pEmptyStr;

	char* s = AllocBuffer(iLen);
	StringCopy(s, szSrc, iLen);

	return s;
}

//	Allocate a new string which is merged by szSrc + ch
char* AString::AllocThenCopy(const char* szSrc, char c, int32_t iLen)
{
	if (!c)
		return AllocThenCopy(szSrc, iLen - 1);

	char* s = AllocBuffer(iLen);
	StringCopy(s, szSrc, iLen - 1);

	s[iLen - 1] = c;
	s[iLen] = '\0';

	return s;
}

//	Allocate a new string which is merged by ch + szSrc
char* AString::AllocThenCopy(char c, const char* szSrc, int32_t iLen)
{
	if (!c)
		return l_EmptyString.szStr;

	char* s = AllocBuffer(iLen);

	s[0] = c;
	StringCopy(s + 1, szSrc, iLen - 1);

	return s;
}

//	Allocate a new string which is merged by s1 + s2
char* AString::AllocThenCopy(const char* s1, const char* s2, int32_t iLen1, int32_t iLen2)
{
	if (!iLen2)
		return AllocThenCopy(s1, iLen1);

	int32_t iLen = iLen1 + iLen2;
	char* s = AllocBuffer(iLen);

	StringCopy(s, s1, iLen1);
	StringCopy(s + iLen1, s2, iLen2);

	return s;
}

AString::AString(const AString& s)
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

AString::AString(const char* szStr)
{
	m_pStr = AllocThenCopy(szStr, SafeStrLen(szStr));
}

AString::AString(const char* szStr, int32_t iLen)
{
	m_pStr = AllocThenCopy(szStr, iLen);
}

AString::AString(char c, int32_t iRepeat)
{
	m_pStr = AllocBuffer(iRepeat);
	std::memset(m_pStr, c, iRepeat);
	m_pStr[iRepeat] = '\0';
}

AString::AString(const AString& str1, const AString& str2)
{
	m_pStr = AllocThenCopy(str1.m_pStr, str2.m_pStr, str1.GetLength(), str2.GetLength());
}

AString::AString(char c, const AString& s)
{
	m_pStr = AllocThenCopy(c, s.m_pStr, s.GetLength() + 1);
}

AString::AString(const AString& s, char c)
{
	m_pStr = AllocThenCopy(s.m_pStr, c, s.GetLength() + 1);
}

AString::AString(const char* szStr, const AString& s)
{
	m_pStr = AllocThenCopy(szStr, s.m_pStr, SafeStrLen(szStr), s.GetLength());
}

AString::AString(const AString& s, const char* szStr)
{
	m_pStr = AllocThenCopy(s.m_pStr, szStr, s.GetLength(), SafeStrLen(szStr));
}

AString::~AString()
{
	s_STRINGDATA* pData = GetData();

	if (pData->iRefs == -1)	//	Buffer is being locked
		pData->iRefs = 1;

	FreeBuffer(pData);
	m_pStr = m_pEmptyStr;
}

const AString& AString::operator = (char c)
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

const AString& AString::operator = (const char* szStr)
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

const AString& AString::operator = (const AString& s)
{
	if (m_pStr == s.m_pStr)
		return *this;

	if (s.IsEmpty())
	{
		Empty();
		return *this;
	}

	s_STRINGDATA* pSrcData = s.GetData();

	if (pSrcData->iRefs == -1)	//	Source string is being locked
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

const AString& AString::operator += (char c)
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

const AString& AString::operator += (const char* szStr)
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

const AString& AString::operator += (const AString& s)
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

int32_t AString::Compare(const char* szStr) const
{
	return (m_pStr == szStr) ? 0 : std::strcmp(m_pStr, szStr);
}

int32_t AString::CompareNoCase(const char* szStr) const
{
	if (m_pStr == szStr)
		return 0;

	for (size_t i = 0; m_pStr[i] != '\0' && szStr[i] != '\0'; ++i)
	{
		char c1 = std::tolower(static_cast<uint8_t>(m_pStr[i]));
		char c2 = std::tolower(static_cast<uint8_t>(szStr[i]));

		if (c1 != c2)
			return c1 - c2;
	}

	// Check if both strings ended at the same time
	if (std::strlen(m_pStr) == std::strlen(szStr))
		return 0;

	// Return a positive or negative value indicating the comparison result
	return std::strlen(m_pStr) > std::strlen(szStr) ? 1 : -1;
}

bool AString::operator == (const char* szStr) const
{
	//	Note: szStr's boundary may be crossed when StringEqual() do
	//		  read operation, if szStr is shorter than 'this'. Now, this
	//		  read operation won't cause problem, but in the future,
	//		  should we check the length of szStr at first, and put the 
	//		  shorter one between 'this' and szStr front when we call StringEqual ?
	return StringEqual(m_pStr, szStr, GetLength() + 1);
}

bool AString::operator == (const AString& s) const
{
	if (m_pStr == s.m_pStr)
		return true;

	int32_t iLen = GetLength();
	if (iLen != s.GetLength())
		return false;

	return StringEqual(m_pStr, s.m_pStr, iLen);
}

char& AString::operator [] (int32_t n)
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
void AString::MakeUpper()
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

	_strupr(m_pStr);
}

//	Convert to lower case
void AString::MakeLower()
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

	_strlwr(m_pStr);
}

// Format string
AString& AString::Format(const char* szFormat, ...)
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
		std::vsprintf(m_pStr, szFormat, argList);
		pData->iDataLen = SafeStrLen(m_pStr);
		{
			va_end(argList);
			return *this;
		}
	}
	else	// iMaxLen > pData->iMaxLen
		FreeBuffer(pData);

	m_pStr = AllocBuffer(iMaxLen);
	iNumWritten = std::vsprintf(m_pStr, szFormat, argList);
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
char* AString::GetBuffer(int32_t iMinSize)
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
			char* szOld = m_pStr;
			m_pStr = AllocBuffer(iMinSize);
			StringCopy(m_pStr, szOld, pData->iDataLen);
			GetData()->iDataLen = pData->iDataLen;
		}
	}
	else if (iMinSize > pData->iMaxLen)
	{
		char* szOld = m_pStr;
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
void AString::ReleaseBuffer(int32_t iNewSize/* -1 */)
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

// Lock buffer. Locked buffer disable reference counting
char* AString::LockBuffer()
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

// Unlock buffer
void AString::UnlockBuffer()
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

/*	Make a guess at the maximum length of the resulting string.
	now this function doesn't support UNICODE string.
	I64 modifier used in WIN32's sprintf is now added in 2010.1.10

	Return estimated length of resulting string.
*/

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

int32_t AString::GetFormatLen(const char* szFormat, va_list argList)
{
	if (!szFormat || !szFormat[0])
		return 0;

	char* pszTemp = nullptr;
	int32_t iMaxLen = 0;
	int32_t iTempBufLen = 0;

	for (const char* pch = szFormat; *pch != '\0'; ++pch)
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
			if (*pch == '#')
				iMaxLen += 2;   // for '0x'
			else if (*pch == '*')
				iWidth = va_arg(argList, int32_t);
			else if (*pch == '-' || *pch == '+' || *pch == '0' || *pch == ' ')
				;
			else	// Hit non-flag character
				break;
		}

		// Get width and skip it
		if (iWidth == 0)
		{
			// Width indicated by digit
			iWidth = std::atoi(pch);
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
				iPrecision = std::atoi(pch);
				for (; *pch != '\0' && (*pch >= '0' && *pch <= '9'); ++pch)
					;
			}

			assert(iPrecision >= 0);
		}

		// should be on type modifier or specifier
		int32_t nModifier = 0;
		if (std::strncmp(pch, "I64", 3) == 0) // Note that I64 is Microsoft-specific
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
			va_arg(argList, char);
			break;

		case 's':	// Strings
		{
			const char* pstrNextArg = va_arg(argList, const char*);
			if (!pstrNextArg)
				iItemLen = 6;	// "(null)"
			else
				iItemLen = pstrNextArg[0] == '\0' ? 1 : (int32_t)std::strlen(pstrNextArg);
			break;
		}
		case 'S':
		{
			const wchar_t* pstrNextArg = va_arg(argList, const wchar_t*);
			if (!pstrNextArg)
				iItemLen = 6;	//	"(null)"
			else
				iItemLen = pstrNextArg[0] == '\0' ? 1 : (int32_t)std::wcslen(pstrNextArg);
			break;
		}
		}

		// Adjust iItemLen for strings
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
			case 'd':	// Integers
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
				va_arg(argList, s_DOUBLE);	// For _X86_
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
						? (char*)std::realloc(pszTemp, iSize)
						: (char*)std::malloc(iSize);

					iTempBufLen = iSize;
				}

				double f = va_arg(argList, double);
				assert(pszTemp != nullptr);
				std::sprintf(pszTemp, "%*.*f", iWidth, iPrecision + 6, f);
				iItemLen = (int32_t)std::strlen(pszTemp);

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

		// Adjust iMaxLen for output iItemLen
		iMaxLen += iItemLen;
	}

	if (pszTemp)
		std::free(pszTemp);

	return iMaxLen;
}

// Cut left sub string
void AString::CutLeft(int32_t n)
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

	for (int32_t i = 0; i < iNewLen; i++)
		m_pStr[i] = m_pStr[n + i];

	m_pStr[iNewLen] = '\0';
	pData->iDataLen = iNewLen;
}

// Cut right sub string
void AString::CutRight(int32_t n)
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
void AString::TrimLeft()
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::strlen(m_pStr), [](uint8_t c) { return c > 32; }) - m_pStr;

	CutLeft(static_cast<int32_t>(i));
}

// Trim left
void AString::TrimLeft(char c)
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::strlen(m_pStr), [c](uint8_t ch) { return ch != c; }) - m_pStr;

	CutLeft(static_cast<int32_t>(i));
}

// Trim left
void AString::TrimLeft(const char* szChars)
{
	if (!GetLength())
		return;

	std::string_view strView(m_pStr);
	std::string_view charsView(szChars);

	int32_t i = 0;
	while (i < (int32_t)strView.size() && std::find(charsView.begin(), charsView.end(), strView[i]) != charsView.end())
		++i;
	
	CutLeft(i);
}

// Trim right
void AString::TrimRight()
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::strlen(m_pStr), [](char c) { return c > 32; }) - m_pStr;

	CutRight(static_cast<int32_t>(std::strlen(m_pStr) - i));
}

// Trim right
void AString::TrimRight(char c)
{
	if (!GetLength())
		return;

	ptrdiff_t i = std::find_if(m_pStr, m_pStr + std::strlen(m_pStr), [c](char ch) { return ch != c; }) - m_pStr;

	CutRight(static_cast<int32_t>(std::strlen(m_pStr) - i));
}

// Trim right
void AString::TrimRight(const char* szChars)
{
	if (!GetLength())
		return;

	std::string_view strView(m_pStr);
	std::string_view charsView(szChars);

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

// Finds a character inside a larger string. 
// Return -1 for failure.
size_t AString::Find(char c, int32_t iStart/* 0 */) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || iStart < 0 || iStart >= iLen)
		? -1
		: std::find(m_pStr + iStart, m_pStr + iLen, c) - m_pStr;
}

// Finds a substring inside a larger string. 
// Return -1 for failure.
size_t AString::Find(const char* szSub, int32_t iStart/* 0 */) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || iStart < 0 || iStart >= iLen)
		? -1
		: std::strstr(m_pStr + iStart, szSub) - m_pStr;
}

// Finds a character inside a larger string; starts from the end. 
// Return -1 for failure.
size_t AString::ReverseFind(char c) const
{
	int32_t iLen = GetLength();
	return (iLen == 0)
		? -1
		: std::strrchr(m_pStr, c) - m_pStr;
}

// Finds the first matching character from a set. 
// Return -1 for failure.
size_t AString::FindOneOf(const char* szCharSet) const
{
	int32_t iLen = GetLength();
	return (iLen == 0 || std::strcspn(m_pStr, szCharSet) == iLen) ? -1 : 0;
}
