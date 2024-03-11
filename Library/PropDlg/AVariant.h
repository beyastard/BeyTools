#ifndef _AVARIANT_H__
#define _AVARIANT_H__

#include "A3DTypes.h"
#include "AFile.h"
#include "AString.h"

#include <cassert>

#define DEFINEOPERATOR(type) operator type() {return *(type*)(&m_avData);}

// forward declarations
class ASet;
class AObject;
class AClassInfo;
class AScriptCodeData;

class AVariant
{
public:
    enum AVARTYPE
    {
        AVT_INVALIDTYPE,
        AVT_BOOL,
        AVT_CHAR,
        AVT_UCHAR,
        AVT_SHORT,
        AVT_USHORT,
        AVT_INT,
        AVT_UINT,
        AVT_LONG,
        AVT_ULONG,
        AVT_INT64,
        AVT_UINT64,
        AVT_FLOAT,
        AVT_DOUBLE,
        AVT_POBJECT,
        AVT_PSCRIPT,
        AVT_A3DVECTOR3,
        AVT_A3DMATRIX4,
        AVT_STRING,
        AVT_PABINARY,
        AVT_PACLASSINFO,
        AVT_PASET,
        AVT_APointF,
        AVT_A3DVECTOR4,
    };

    AVariant();
    AVariant(const AVariant& v);
    AVariant(bool bV);
    AVariant(char cV);
    AVariant(unsigned char ucV);
    AVariant(short sV);
    AVariant(unsigned short usV);
    AVariant(int iV);
    AVariant(unsigned int uiV);
    AVariant(long lV);
    AVariant(unsigned long ulV);
    AVariant(__int64 i64V);
    AVariant(unsigned __int64 ui64V);
    AVariant(float fV);
    AVariant(double dV);
    AVariant(AObject& object);
    AVariant(AScriptCodeData& object);
    AVariant(APointF& apoint);
    AVariant(A3DVECTOR3& avec3);
    AVariant(A3DVECTOR4& avec4);
    AVariant(A3DMATRIX4& amat4);
    AVariant(AString& s);
    AVariant(const char* szStr);
    AVariant(AClassInfo* paciV);

    int32_t GetType() const { return m_iType; }

    operator bool() { return *(bool*)(&m_avData); }
    operator char() { return *(char*)(&m_avData); }
    operator unsigned char() { return *(unsigned char*)(&m_avData); }
    operator short() { return *(short*)(&m_avData); }
    operator unsigned short() { return *(unsigned short*)(&m_avData); }
    operator int() { return *(int*)(&m_avData); }
    operator unsigned int() { return *(unsigned int*)(&m_avData); }
    operator long() { return *(long*)(&m_avData); }
    operator unsigned long() { return *(unsigned long*)(&m_avData); }
    operator __int64() { return *(__int64*)(&m_avData); }
    operator unsigned __int64() { return *(unsigned __int64*)(&m_avData); }

    operator float()
    {
        assert(m_iType != AVT_INVALIDTYPE);
        if (m_iType == AVT_DOUBLE)
            return (float)m_avData.dV;

        if (m_iType == AVT_FLOAT)
            return m_avData.fV;

        return (float)(*(__int64*)(&m_avData));
    }

    operator double()
    {
        assert(m_iType != AVT_INVALIDTYPE);
        if (m_iType == AVT_DOUBLE)
            return m_avData.dV;

        if (m_iType == AVT_FLOAT)
            return (double)m_avData.fV;

        return (double)(*(__int64*)(&m_avData));
    }

    operator AString()
    {
        return m_string;
    }

    AVariant& operator= (const AVariant& src)
    {
        m_string = src.m_string;
        memcpy(&m_avData, &src.m_avData, sizeof(src.m_avData));
        m_iType = src.m_iType;
        return *this;
    }

    operator AObject* () { return *(AObject**)(&m_avData); }
    operator APointF() { return *(APointF*)(&m_avData); }
    operator A3DVECTOR3() { return *(A3DVECTOR3*)(&m_avData); }
    operator A3DVECTOR4() { return *(A3DVECTOR4*)(&m_avData); }
    operator A3DMATRIX4() { return *(A3DMATRIX4*)(&m_avData); }
    operator AClassInfo* () { return *(AClassInfo**)(&m_avData); }
    operator AScriptCodeData* () { return *(AScriptCodeData**)(&m_avData); }

    operator AScriptCodeData& ()
    {
        assert(m_iType != AVT_INVALIDTYPE);
        return *(AScriptCodeData*)(&m_avData);
    }

    void Load(AFile* pFile);
    void Save(AFile* pFile);

private:
    AString m_string;

    union AVARIANT
    {
        bool bV;
        char cV;
        unsigned char ucV;
        short sV;
        unsigned short usV;
        int iV;
        unsigned int uiV;
        long lV;
        unsigned long ulV;
        __int64 i64V;
        unsigned __int64 ui64V;
        float fV;
        double dV;
        AObject* paV;
        char a2dvV[sizeof(APointF)];
        char a3dvV[sizeof(A3DVECTOR3)];
        char a4dvV[sizeof(A3DVECTOR4)];
        char a3dmV[sizeof(A3DMATRIX4)];
        AClassInfo* paciV;
        ASet* pasetV;
        AScriptCodeData* pascdV;
    } m_avData;
    int32_t m_iType;
};

class CSafeString
{
    LPSTR m_szBuf;
    LPWSTR m_wszBuf;

public:
    CSafeString(LPCSTR lpsz, int nLen)
        : m_wszBuf(nullptr)
    {
        assert(lpsz);
        if (nLen < 0)
            nLen = (int)strlen(lpsz);

        m_szBuf = new char[nLen + 1];
        strncpy(m_szBuf, lpsz, nLen);
        m_szBuf[nLen] = '\0';
    }

    CSafeString(LPCSTR lpsz)
        : m_wszBuf(nullptr)
    {
        assert(lpsz);
        int n = (int)strlen(lpsz);
        m_szBuf = new char[n + 1];
        strcpy(m_szBuf, lpsz);
    }

    CSafeString(LPCWSTR lpwsz, int nLen)
        : m_szBuf(nullptr)
    {
        assert(lpwsz);
        if (nLen < 0)
            nLen = (int)wcslen(lpwsz);

        m_wszBuf = new wchar_t[nLen + 1];
        wcsncpy(m_wszBuf, lpwsz, nLen);
        m_wszBuf[nLen] = L'\0';
    }

    CSafeString(LPCWSTR lpwsz)
        : m_szBuf(nullptr)
    {
        assert(lpwsz);
        int n = (int)wcslen(lpwsz);
        m_wszBuf = new wchar_t[n + 1];
        wcscpy(m_wszBuf, lpwsz);
    }

    operator LPCSTR() { return GetAnsi(); }
    operator LPCWSTR() { return GetUnicode(); }

    CSafeString& operator= (const CSafeString& str)
    {
        if (this == &str)
            return *this;

        delete[] m_szBuf;
        delete[] m_wszBuf;

        m_szBuf = nullptr;
        m_wszBuf = nullptr;

        if (str.m_szBuf)
        {
            m_szBuf = new char[strlen(str.m_szBuf) + 1];
            strcpy(m_szBuf, str.m_szBuf);
        }

        if (str.m_wszBuf)
        {
            m_wszBuf = new wchar_t[wcslen(str.m_wszBuf) + 1];
            wcscpy(m_wszBuf, str.m_wszBuf);
        }

        return *this;
    }

    CSafeString(const CSafeString& obj)
    {
        operator = (obj);
    };

    LPCSTR GetAnsi()
    {
        if (m_szBuf)
            return m_szBuf;

        assert(m_wszBuf);

        int nCount = WideCharToMultiByte(CODE_PAGE, 0, m_wszBuf, -1, nullptr, 0, nullptr, nullptr);
        m_szBuf = new char[nCount];
        WideCharToMultiByte(CODE_PAGE, 0, m_wszBuf, -1, m_szBuf, nCount, nullptr, nullptr);

        return m_szBuf;
    }

    LPCWSTR GetUnicode()
    {
        if (m_wszBuf)
            return m_wszBuf;

        assert(m_szBuf);

        int nCount = MultiByteToWideChar(CODE_PAGE, 0, m_szBuf, -1, nullptr, 0);
        m_wszBuf = new wchar_t[nCount];
        MultiByteToWideChar(CODE_PAGE, 0, m_szBuf, -1, m_wszBuf, nCount);

        return m_wszBuf;
    }

    virtual ~CSafeString()
    {
        delete[] m_szBuf;
        delete[] m_wszBuf;
    }
};

#endif
