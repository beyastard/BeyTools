#ifndef _AWString_H_
#define _AWString_H_

#include "ABaseDef.h"

#include <cwchar>
#include <sstream>

class AWString
{
public:
	struct s_STRINGDATA
	{
		int32_t iRefs;		//	Reference count
		int32_t iDataLen;	//	Length of data (not including terminator)
		int32_t iMaxLen;	//	Maximum length of data (not including terminator)

		wchar_t* Data() { return (wchar_t*)(this + 1); } // wchar_t* to managed data
	};

	struct s_DOUBLE
	{
		uint8_t aDoubleBits[sizeof(double)];
	};

	AWString() : m_pStr(m_pEmptyStr) {}
	AWString(const AWString& s);
	AWString(const wchar_t* szStr);
	AWString(const wchar_t* szStr, int32_t iLen);
	AWString(wchar_t c, int32_t iRepeat);
	~AWString();

	const AWString& operator = (wchar_t c);
	const AWString& operator = (const wchar_t* szStr);
	const AWString& operator = (const AWString& s);

	const AWString& operator += (wchar_t c);
	const AWString& operator += (const wchar_t* szStr);
	const AWString& operator += (const AWString& s);

	friend AWString operator + (const AWString& str1, const AWString& str2) { return AWString(str1, str2); }
	friend AWString operator + (wchar_t c, const AWString& s) { return AWString(c, s); }
	friend AWString operator + (const AWString& s, wchar_t c) { return AWString(s, c); }
	friend AWString operator + (const wchar_t* szStr, const AWString& s) { return AWString(szStr, s); }
	friend AWString operator + (const AWString& s, const wchar_t* szStr) { return AWString(s, szStr); }

	int32_t Compare(const wchar_t* szStr) const;
	int32_t CompareNoCase(const wchar_t* szStr) const;

	bool operator == (wchar_t c) const { return (GetLength() == 1 && m_pStr[0] == c); }
	bool operator == (const wchar_t* szStr) const;
	bool operator == (const AWString& s) const;

	bool operator != (wchar_t c) const { return !(GetLength() == 1 && m_pStr[0] == c); }
	bool operator != (const wchar_t* szStr) const { return !((*this) == szStr); }
	bool operator != (const AWString& s) const { return !((*this) == s); }

	bool operator > (const wchar_t* szStr) const { return Compare(szStr) > 0; }
	bool operator < (const wchar_t* szStr) const { return Compare(szStr) < 0; }
	bool operator >= (const wchar_t* szStr) const { return !((*this) < szStr); }
	bool operator <= (const wchar_t* szStr) const { return !((*this) > szStr); }

	wchar_t operator [] (int32_t n) const { assert(n >= 0 && n <= GetLength()); return m_pStr[n]; }
	wchar_t& operator [] (int32_t n);

	operator const wchar_t* () const { return m_pStr; }
	const wchar_t* GetWCharString() const { return m_pStr; }

	const std::string GetStdString(AWString& s)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(s.GetWCharString());
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
	AWString& Format(const wchar_t* szFormat, ...);

	//	Get buffer
	wchar_t* GetBuffer(int32_t iMinSize);
	//	Release buffer gotten through GetBuffer()
	void ReleaseBuffer(int32_t iNewSize = -1);
	//	Lock buffer
	wchar_t* LockBuffer();
	//	Unlock buffer
	void UnlockBuffer();

	//	Trim left
	void TrimLeft();
	void TrimLeft(wchar_t c);
	void TrimLeft(const wchar_t* szChars);

	//	Trim right
	void TrimRight();
	void TrimRight(wchar_t c);
	void TrimRight(const wchar_t* szChars);

	//	Cut left sub string
	void CutLeft(int32_t n);
	//	Cut right sub string
	void CutRight(int32_t n);

	//	Finds a character or substring inside a larger string. 
	size_t Find(wchar_t c, int32_t iStart = 0) const;
	size_t Find(const wchar_t* szSub, int32_t iStart = 0) const;
	//	Finds a character inside a larger string; starts from the end. 
	size_t ReverseFind(wchar_t c) const;
	//	Finds the first matching character from a set. 
	size_t FindOneOf(const wchar_t* szCharSet) const;

	//	Get left string
	inline AWString Left(int32_t n) const;
	//	Get right string
	inline AWString Right(int32_t n) const;
	//	Get Mid string
	inline AWString Mid(int32_t iFrom, int32_t iNum = -1) const;

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
	AWString(const AWString& str1, const AWString& str2);
	AWString(wchar_t c, const AWString& s);
	AWString(const AWString& s, wchar_t ch);
	AWString(const wchar_t* szStr, const AWString& s);
	AWString(const AWString& s, const wchar_t* szStr);

	//	Get string data
	s_STRINGDATA* GetData() const { return ((s_STRINGDATA*)m_pStr) - 1; }

	//	Safed strlen()
	static int32_t SafeStrLen(const wchar_t* szStr) { return szStr ? static_cast<int32_t>(std::wcslen(szStr)) : 0; }
	//	String copy
	static void StringCopy(wchar_t* szDest, const wchar_t* szSrc, int32_t iLen);
	//	Alocate string buffer
	static wchar_t* AllocBuffer(int32_t iLen);
	//	Free string data buffer
	static void FreeBuffer(s_STRINGDATA* pStrData);
	//	Judge whether two strings are equal
	static bool StringEqual(const wchar_t* s1, const wchar_t* s2, int32_t iLen);

	//	Allocate memory and copy string
	static wchar_t* AllocThenCopy(const wchar_t* szSrc, int32_t iLen);
	static wchar_t* AllocThenCopy(const wchar_t* szSrc, wchar_t ch, int32_t iLen);
	static wchar_t* AllocThenCopy(wchar_t ch, const wchar_t* szSrc, int32_t iLen);
	static wchar_t* AllocThenCopy(const wchar_t* s1, const wchar_t* s2, int32_t iLen1, int32_t iLen2);

	//	Get format string length
	static int32_t GetFormatLen(const wchar_t* szFormat, va_list argList);

	wchar_t* m_pStr; // String buffer
	static wchar_t* m_pEmptyStr;
};

//	Get left string
AWString AWString::Left(int32_t n) const
{
	assert(n >= 0);
	int32_t iLen = GetLength();
	return AWString(m_pStr, iLen < n ? iLen : n);
}

//	Get right string
AWString AWString::Right(int32_t n) const
{
	assert(n >= 0);
	int32_t iFrom = GetLength() - n;
	return Mid(iFrom < 0 ? 0 : iFrom, n);
}

//	Get Mid string
AWString AWString::Mid(int32_t iFrom, int32_t iNum/* -1 */) const
{
	int32_t iLen = GetLength() - iFrom;
	if (iLen <= 0 || !iNum)
		return AWString();	//	Return empty string

	if (iNum > 0 && iLen > iNum)
		iLen = iNum;

	return AWString(m_pStr + iFrom, iLen);
}

//	Convert string to integer
int32_t AWString::ToInt32() const
{
	if (IsEmpty())
		return 0;

	wchar_t* endPtr;
	long result = std::wcstol(m_pStr, &endPtr, 10);

	if (endPtr == m_pStr || *endPtr != L'\0' || result > INT32_MAX || result < INT32_MIN)
		return 0;

	return static_cast<int32_t>(result);
}

//	Convert string to float
float AWString::ToFloat() const
{
	if (IsEmpty())
		return 0.0f;

	return std::wcstof(m_pStr, nullptr);
}

//	Convert string to __int64
int64_t AWString::ToInt64() const
{
	if (IsEmpty())
		return 0;

	wchar_t* endPtr;
	return std::wcstoll(m_pStr, &endPtr, 10);
}

//	Convert string to double
double AWString::ToDouble() const
{
	if (IsEmpty())
		return 0.0;

	wchar_t* endptr;
	double result = std::wcstod(m_pStr, &endptr);

	if (endptr == m_pStr || *endptr != L'\0')
		return 0.0;
	
	return result;
}

#endif
