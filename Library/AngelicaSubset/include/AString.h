#ifndef _ASTRING_H_
#define _ASTRING_H_

#include "ABaseDef.h"

class AString
{
public:
	struct s_STRINGDATA
	{
		int32_t iRefs;    // Reference count
		int32_t iDataLen; // Length of data (not including terminator)
		int32_t iMaxLen;  // Maximum length of data (not including terminator)

		char* Data() { return (char*)(this + 1); } // char* to managed data
	};

	struct s_DOUBLE
	{
		uint8_t aDoubleBits[sizeof(double)];
	};

	AString() : m_pStr(m_pEmptyStr) {}
	AString(const AString& s);
	AString(const char* szStr);
	AString(const char* szStr, int32_t iLen);
	AString(char c, int32_t iRepeat);
	~AString();

	const AString& operator = (char c);
	const AString& operator = (const char* szStr);
	const AString& operator = (const AString& s);

	const AString& operator += (char c);
	const AString& operator += (const char* szStr);
	const AString& operator += (const AString& s);

	friend AString operator + (const AString& str1, const AString& str2) { return AString(str1, str2); }
	friend AString operator + (char c, const AString& s) { return AString(c, s); }
	friend AString operator + (const AString& s, char c) { return AString(s, c); }
	friend AString operator + (const char* szStr, const AString& s) { return AString(szStr, s); }
	friend AString operator + (const AString& s, const char* szStr) { return AString(s, szStr); }

	int32_t Compare(const char* szStr) const;
	int32_t CompareNoCase(const char* szStr) const;

	bool operator == (char c) const { return (GetLength() == 1 && m_pStr[0] == c); }
	bool operator == (const char* szStr) const;
	bool operator == (const AString& s) const;

	bool operator != (char c) const { return !(GetLength() == 1 && m_pStr[0] == c); }
	bool operator != (const char* szStr) const { return !((*this) == szStr); }
	bool operator != (const AString& s) const { return !((*this) == s); }

	bool operator > (const char* szStr) const { return Compare(szStr) > 0; }
	bool operator < (const char* szStr) const { return Compare(szStr) < 0; }
	bool operator >= (const char* szStr) const { return !((*this) < szStr); }
	bool operator <= (const char* szStr) const { return !((*this) > szStr); }

	char operator [] (int32_t n) const { assert(n >= 0 && n <= GetLength()); return m_pStr[n]; }
	char& operator [] (int32_t n);

	operator const char* () const { return m_pStr; }
	const char* GetCharString() const { return m_pStr; }

	const std::wstring GetStdWString(AString& s)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(s.GetCharString());
	}

	//	Get string length
	int32_t	GetLength()	const { return GetData()->iDataLen; }
	//	String is empty ?
	bool IsEmpty() const { return m_pStr == m_pEmptyStr ? true : false; }
	//	Empty string
	void Empty() { FreeBuffer(GetData()); m_pStr = m_pEmptyStr; }
	//	Convert to upper case
	void MakeUpper();
	//	Convert to lower case
	void MakeLower();
	//	Format string
	AString& Format(const char* szFormat, ...);

	//	Get buffer
	char* GetBuffer(int32_t iMinSize);
	//	Release buffer gotten through GetBuffer()
	void ReleaseBuffer(int32_t iNewSize = -1);
	//	Lock buffer
	char* LockBuffer();
	//	Unlock buffer
	void UnlockBuffer();

	//	Trim left
	void TrimLeft();
	void TrimLeft(char s);
	void TrimLeft(const char* szChars);

	//	Trim right
	void TrimRight();
	void TrimRight(char s);
	void TrimRight(const char* szChars);

	//	Cut left sub string
	void CutLeft(int32_t n);
	//	Cut right sub string
	void CutRight(int32_t n);

	//	Finds a character or substring inside a larger string. 
	size_t Find(char s, int32_t iStart = 0) const;
	size_t Find(const char* szSub, int32_t iStart = 0) const;
	//	Finds a character inside a larger string; starts from the end. 
	size_t ReverseFind(char s) const;
	//	Finds the first matching character from a set. 
	size_t FindOneOf(const char* szCharSet) const;

	//	Get left string
	inline AString Left(int32_t n) const;
	//	Get right string
	inline AString Right(int32_t n) const;
	//	Get Mid string
	inline AString Mid(int32_t iFrom, int32_t iNum = -1) const;

	//	Convert string to integer
	inline int32_t ToInt32() const;
	//	Convert string to float
	inline float ToFloat() const;
	//	Convert string to __int64
	inline int64_t ToInt64() const;
	//	Convert string to double
	inline double ToDouble() const;

protected:
	//	These constructors are used to optimize operations such as 'operator +'
	AString(const AString& str1, const AString& str2);
	AString(char s, const AString& c);
	AString(const AString& c, char s);
	AString(const char* szStr, const AString& s);
	AString(const AString& s, const char* szStr);

	//	Get string data
	s_STRINGDATA* GetData() const { return ((s_STRINGDATA*)m_pStr) - 1; }

	//	Safed strlen()
	static int32_t SafeStrLen(const char* szStr) { return szStr ? static_cast<int32_t>(std::strlen(szStr)) : 0; }
	//	String copy
	static void StringCopy(char* szDest, const char* szSrc, int32_t iLen);
	//	Alocate string buffer
	static char* AllocBuffer(int32_t iLen);
	//	Free string data buffer
	static void FreeBuffer(s_STRINGDATA* pStrData);
	//	Judge whether two strings are equal
	static bool StringEqual(const char* s1, const char* s2, int32_t iLen);

	//	Allocate memory and copy string
	static char* AllocThenCopy(const char* szSrc, int32_t iLen);
	static char* AllocThenCopy(const char* szSrc, char c, int32_t iLen);
	static char* AllocThenCopy(char c, const char* szSrc, int32_t iLen);
	static char* AllocThenCopy(const char* s1, const char* s2, int32_t iLen1, int32_t iLen2);

	//	Get format string length
	static int32_t GetFormatLen(const char* szFormat, va_list argList);

	char* m_pStr; // String buffer
	static char* m_pEmptyStr;
};

//	Get left string
AString AString::Left(int32_t n) const
{
	assert(n >= 0);
	int32_t iLen = GetLength();
	return AString(m_pStr, iLen < n ? iLen : n);
}

//	Get right string
AString AString::Right(int32_t n) const
{
	assert(n >= 0);
	int32_t iFrom = GetLength() - n;
	return Mid(iFrom < 0 ? 0 : iFrom, n);
}

//	Get Mid string
AString AString::Mid(int32_t iFrom, int32_t iNum/* -1 */) const
{
	int32_t iLen = GetLength() - iFrom;
	if (iLen <= 0 || !iNum)
		return AString();	//	Return empty string

	if (iNum > 0 && iLen > iNum)
		iLen = iNum;

	return AString(m_pStr + iFrom, iLen);
}

//	Convert string to integer
int32_t AString::ToInt32() const
{
	return IsEmpty() ? 0 : std::atoi(m_pStr);
}

//	Convert string to float
float AString::ToFloat() const
{
	return IsEmpty() ? 0.0f : (float)std::atof(m_pStr);
}

//	Convert string to __int64
int64_t AString::ToInt64() const
{
	return IsEmpty() ? 0 : std::atoll(m_pStr);
}

//	Convert string to double
double AString::ToDouble() const
{
	return IsEmpty() ? 0.0 : std::atof(m_pStr);
}

#endif
