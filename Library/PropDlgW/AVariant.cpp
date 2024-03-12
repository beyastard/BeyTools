#include "pch.h"
#include "AVariant.h"
#include "Convert.h"

AVariant::AVariant()
{
    m_iType = AVT_INVALIDTYPE;
    std::memset(&m_avData, 0, sizeof(AVARIANT));
}

AVariant::AVariant(const AVariant& v)
{
    // When it is a string type, the string must also be copied.
    m_string = v.m_string;
    std::memcpy(&m_avData, &v.m_avData, sizeof(v.m_avData));
    m_iType = v.m_iType;
}

AVariant::AVariant(bool bV)
{
	m_iType = AVT_BOOL;
	m_avData.bV = bV;
}

AVariant::AVariant(char cV)
{
	m_iType = AVT_CHAR;
	m_avData.cV = cV;
}

AVariant::AVariant(unsigned char ucV)
{
	m_iType = AVT_UCHAR;
	m_avData.ucV = ucV;
}

AVariant::AVariant(short sV)
{
	m_iType = AVT_SHORT;
	m_avData.sV = sV;
}

AVariant::AVariant(unsigned short usV)
{
	m_iType = AVT_USHORT;
	m_avData.usV = usV;
}

AVariant::AVariant(int iV)
{
	m_iType = AVT_INT;
	m_avData.iV = iV;
}

AVariant::AVariant(unsigned int uiV)
{
	m_iType = AVT_UINT;
	m_avData.uiV = uiV;
}

AVariant::AVariant(long lV)
{
	m_iType = AVT_LONG;
	m_avData.lV = lV;
}

AVariant::AVariant(unsigned long ulV)
{
	m_iType = AVT_ULONG;
	m_avData.ulV = ulV;
}

AVariant::AVariant(__int64 i64V)
{
	m_iType = AVT_INT64;
	m_avData.i64V = i64V;
}

AVariant::AVariant(unsigned __int64 ui64V)
{
	m_iType = AVT_UINT64;
	m_avData.ui64V = ui64V;
}

AVariant::AVariant(float fV)
{
	m_iType = AVT_FLOAT;
	m_avData.fV = fV;
}

AVariant::AVariant(double dV)
{
	m_iType = AVT_DOUBLE;
	m_avData.dV = dV;
}

AVariant::AVariant(AObject& object)
{
	m_iType = AVT_POBJECT;
	m_avData.paV = &object;
}

AVariant::AVariant(AScriptCodeData& object)
{
	m_iType = AVT_PSCRIPT;
	m_avData.pascdV = &object;
}

AVariant::AVariant(APointF& apoint)
{
	m_iType = AVT_APointF;
	std::memcpy(m_avData.a2dvV, &apoint, sizeof(m_avData.a2dvV));
}

AVariant::AVariant(A3DVECTOR3& avec3)
{
	m_iType = AVT_A3DVECTOR3;
	std::memcpy(m_avData.a3dvV, &avec3, sizeof(m_avData.a3dvV));
}

AVariant::AVariant(A3DVECTOR4& avec4)
{
	m_iType = AVT_A3DVECTOR4;
	std::memcpy(m_avData.a4dvV, &avec4, sizeof(m_avData.a4dvV));
}

AVariant::AVariant(A3DMATRIX4& amat4)
{
	m_iType = AVT_A3DMATRIX4;
	std::memcpy(m_avData.a3dmV, &amat4, sizeof(m_avData.a3dmV));
}

AVariant::AVariant(AWString& s)
{
	m_iType = AVT_STRING;
	m_string = s;
}

AVariant::AVariant(const wchar_t* wszStr)
	: m_iType(AVT_STRING)
	, m_string(wszStr)
{}

AVariant::AVariant(AClassInfo * paciV)
{
	m_iType = AVT_PACLASSINFO;
	m_avData.paciV = paciV;
}

static const wchar_t* _format_type = L"Type: %d";
static const wchar_t* _format_int = L"Int: %d";
static const wchar_t* _format_uint = L"Uint: %u";
static const wchar_t* _format_bool = L"Bool: %d";
static const wchar_t* _format_float = L"Float: %g";
static const wchar_t* _format_string = L"String: ";
static const wchar_t* _format_vector2 = L"X = %f, Y = %f";
static const wchar_t* _format_vector3 = L"X = %f, Y = %f, Z = %f";
static const wchar_t* _format_vector4 = L"X = %f, Y = %f, Z = %f, W = %f";

// Note that the format string used for printf is different from the format string used for sscanf
// The format used for sscanf is: %[*] [width] [{h | l | ll | I64 | L}]type
// The format used for printf is: %[flags] [width] [.precision] [{h | l | ll | I | I32 | I64}]type
// There is no .precision when reading, but there is when writing
// .14lg is used here so that values with 14 digits after the decimal point can be stored or read.
// By default, %lf and %lg only have 6 digits after the decimal point.
static const wchar_t* _format_double_w = L"Double: %.14lg";
static const wchar_t* _format_double_r = L"Double: %lg";
static const wchar_t* _format_int64 = L"Int64: %lld";
static const wchar_t* _format_uint64 = L"UInt64: %llu";

void AVariant::Load(AFile* pFile)
{
	wchar_t szLine[AFILE_LINEMAXLEN];
	wchar_t szBuf[AFILE_LINEMAXLEN];
	int32_t nBool;
	uint32_t dwRead;

	pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
	(void)std::swscanf(szLine, _format_type, &m_iType);

	switch (m_iType)
	{
	case AVT_INVALIDTYPE:
		break;
	case AVT_BOOL:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)swscanf(szLine, _format_bool, &nBool);
		m_avData.bV = (nBool != 0);
		break;
	case AVT_INT:
	case AVT_LONG:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_int, &m_avData.iV);
		break;
	case AVT_UINT:
	case AVT_ULONG:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_uint, &m_avData.uiV);
		break;
	case AVT_INT64:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_int64, &m_avData.i64V);
		break;
	case AVT_UINT64:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_uint64, &m_avData.ui64V);
		break;
	case AVT_FLOAT:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_float, &m_avData.fV);
		break;
	case AVT_DOUBLE:
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_double_r, &m_avData.dV);
		break;
	case AVT_STRING:
		szBuf[0] = '\0';
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		if (std::wcsncmp(szLine, _format_string, std::wcslen(_format_string)) == 0)
			std::wcscpy(szBuf, szLine + std::wcslen(_format_string));
		m_string = szBuf;
		break;
	case AVT_APointF: {
		APointF point;
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_vector2, &point.x, &point.y);
		*this = point;
		break;
	}
	case AVT_A3DVECTOR3: {
		A3DVECTOR3 vec3;
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_vector3, &vec3.x, &vec3.y, &vec3.z);
		*this = vec3;
		break;
	}
	case AVT_A3DVECTOR4: {
		A3DVECTOR4 vec4;
		pFile->ReadLine(Convert::WCharToChar(szLine), AFILE_LINEMAXLEN, &dwRead);
		(void)std::swscanf(szLine, _format_vector4, &vec4.x, &vec4.y, &vec4.z, &vec4.w);
		*this = vec4;
		break;
	}
	default:
		ASSERT(FALSE);
	}
}

void AVariant::Save(AFile* pFile)
{
	wchar_t szLine[AFILE_LINEMAXLEN];

	std::swprintf(szLine, _format_type, m_iType);
	pFile->WriteLine(Convert::WCharToChar(szLine));

	switch (m_iType)
	{
	case AVT_INVALIDTYPE:
		break;
	case AVT_BOOL:
		std::swprintf(szLine, _format_bool, m_avData.bV);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_INT:
	case AVT_LONG:
		std::swprintf(szLine, _format_int, m_avData.iV);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_UINT:
	case AVT_ULONG:
		std::swprintf(szLine, _format_uint, m_avData.uiV);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_INT64:
		std::swprintf(szLine, _format_int64, m_avData.i64V);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_UINT64:
		std::swprintf(szLine, _format_uint64, m_avData.ui64V);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_FLOAT:
		std::swprintf(szLine, _format_float, m_avData.fV);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_DOUBLE:
		std::swprintf(szLine, _format_double_w, m_avData.dV);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_STRING:
		std::wcscpy(szLine, _format_string);
		std::wcscat(szLine, m_string);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	case AVT_APointF: {
		APointF point = *this;
		std::swprintf(szLine, _format_vector2, point.x, point.y);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	}
	case AVT_A3DVECTOR3: {
		A3DVECTOR3 vec3 = *this;
		std::swprintf(szLine, _format_vector3, vec3.x, vec3.y, vec3.z);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	}
	case AVT_A3DVECTOR4: {
		A3DVECTOR4 vec4 = *this;
		std::swprintf(szLine, _format_vector4, vec4.x, vec4.y, vec4.z, vec4.w);
		pFile->WriteLine(Convert::WCharToChar(szLine));
		break;
	}
	default:
		ASSERT(false);
	}
}
