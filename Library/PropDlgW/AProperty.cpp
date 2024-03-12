#include "pch.h"
#include "AProperty.h"
#include "AObject.h"
#include "A3DMacros.h"
#include "AWScriptFile.h"
#include "Convert.h"

void StringToValue(AVariant& var, const wchar_t* s)
{
    assert(false);
}

void StringToValue(bool& value, const wchar_t* s)
{
	AWString tmp = s;
	tmp.MakeLower();
	if (tmp == L"true")
		value = true;
	else
		value = false;
}

void StringToValue(int& value, const wchar_t* s)
{
	int _ = swscanf(s, L"%d", &value);
}

void StringToValue(float& value, const wchar_t* s)
{
	int _ = swscanf(s, L"%f", &value);
}

void StringToValue(AWString& value, const wchar_t* s)
{
	value = s;
}

void StringToValue(A3DVECTOR3& value, const wchar_t* s)
{
	float x = 0, y = 0, z = 0;
	int32_t _ = swscanf(s, L"%f,%f,%f", &x, &y, &z);
	value.x = x;
	value.y = y;
	value.z = z;
}

void StringToValue(A3DCOLOR& value, const wchar_t* s)
{
	int32_t r = 255, g = 255, b = 255, a = 0;
	int _ = swscanf(s, L"%d,%d,%d,%d", &r, &g, &b, &a);
	value = A3DCOLORRGBA(r, g, b, a);
}

void StringToValue(AScriptCodeData& value, const wchar_t* s)
{
	assert(false);
}

void ValueToString(const AVariant& var, AWString& s)
{
	assert(false);
}

void ValueToString(bool value, AWString& s)
{
	s = value ? L"true" : L"false";
}

void ValueToString(int value, AWString& s)
{
	s.Format(L"%d", value);
}

void ValueToString(float value, AWString& s)
{
	s.Format(L"%g", value);
}

void ValueToString(const AWString& value, AWString& s)
{
	s = value;
}

void ValueToString(const A3DVECTOR3& value, AWString& s)
{
	s.Format(L"%g,%g,%g", value.x, value.y, value.z);
}

void ValueToString(const A3DCOLOR& value, AWString& s)
{
	int32_t r = A3DCOLOR_GETRED(value);
	int32_t g = A3DCOLOR_GETGREEN(value);
	int32_t b = A3DCOLOR_GETBLUE(value);
	int32_t a = A3DCOLOR_GETALPHA(value);
	s.Format(L"%d,%d,%d,%d", r, g, b, a);
}

void ValueToString(const AScriptCodeData& value, AWString& s)
{
	assert(false);
}

namespace __CLOAK__
{
	struct ASetNode
	{
		ASet* pSet;
		ASetNode* pNext;
	};
}

static __CLOAK__::ASetNode* initList = nullptr;

int32_t AddInitElement(ASet* pSet)
{
	__CLOAK__::ASetNode* pNode = new __CLOAK__::ASetNode();
	pNode->pSet = pSet;
	pNode->pNext = initList;
	initList = pNode;
	return 0;
}

int32_t InitPropertiesList()
{
	int32_t rst = 0;
	while (initList)
	{
		__CLOAK__::ASetNode* pNode = initList;
		initList = initList->pNext;
		rst += pNode->pSet->ReadData();
		delete pNode;
	}

	return rst;
}

int32_t ReadSetData(const wchar_t* filename, ASet* pSet)
{
	AWScriptFile sFile;
	if (!sFile.Open(filename))
	{
		assert(false && "Unable to open configuration file");
		return -1;
	}

	while (sFile.GetNextToken(true))
	{
		AWString token1 = sFile.m_szToken;
		if (!sFile.GetNextToken(true))
		{
			assert(false && "Configuration file format is incorrect");
			sFile.Close();
			return -1;
		}

		pSet->AddData(token1, sFile.m_szToken);
	}
	sFile.Close();
	return 0;
}

AVariant AVariantProperty::GetValue(APropertyObject* pBase)
{
	assert(pBase);
	return pBase->GetPropVal((int32_t)m_nIndex);
}

void AVariantProperty::SetValue(APropertyObject* pBase, AVariant value)
{
	assert(pBase);
	pBase->SetPropVal((int32_t)m_nIndex, value);
}

void AVariantProperty::SetStringValue(APropertyObject* pBase, const wchar_t* s)
{
	AWString tmpStr(s);
	pBase->SetPropVal((int32_t)m_nIndex, AVariant(tmpStr));
}

void AVariantProperty::GetStringValue(APropertyObject* pBase, AWString& s)
{
	s = (AWString)pBase->GetPropVal((int32_t)m_nIndex);
}
