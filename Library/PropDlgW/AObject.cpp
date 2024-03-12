#include "pch.h"
#include "AObject.h"

bool APropertyObject::InitStateTable()
{
	int32_t count;
	count = PropertiesCount();
	for (int32_t i = 0; i < count; i++)
	{
		int32_t state = Properties(i)->GetState();
		m_StateList.push_back(state & AProperty::DEFAULT_VALUE ? 1 : 0);
	}

	return true;
}

ADynPropertyObject::ADynPropertyObject()
{
	m_PropLst.reserve(16);
	m_VarLst.reserve(16);
}

void ADynPropertyObject::DynAddProperty(const AVariant& varValue,
	const wchar_t* szName, ASet* pSet, ARange* pRange, int32_t way,
	const wchar_t* szPath, bool isVisable, int32_t iState, const wchar_t* szDesc)
{
	assert(is_valid());
	m_VarLst.push_back(varValue);
	m_PropLst.push_back(VarPropTemp(L"", szName, m_VarLst.size() - 1,
		pSet, pRange, way, szPath, isVisable, iState, szDesc));
	m_StateList.push_back(m_PropLst.back().GetState() & AProperty::DEFAULT_VALUE ? 1 : 0);
}

const wchar_t* ADynPropertyObject::GetPropDesc(int32_t nIndex)
{
	assert(is_valid());
	return m_PropLst[nIndex].GetDesc();
}

void ADynPropertyObject::SetPropDesc(int32_t nIndex, const wchar_t* szDesc)
{
	assert(is_valid());
	m_PropLst.at(nIndex).SetDesc(szDesc);
}

void ADynPropertyObject::SetPropName(int32_t nIndex, const wchar_t* name)
{
	assert(is_valid());
	m_PropLst.at(nIndex).SetName(name);
}

void ADynPropertyObject::Clear()
{
	assert(is_valid());
	m_PropLst.clear();
	m_VarLst.clear();
}

int32_t ADynPropertyObject::PropertiesCount()
{
	assert(is_valid());
	return (int32_t)m_PropLst.size();
}

AProperty* ADynPropertyObject::Properties(int32_t index)
{
	assert(is_valid());
	return &m_PropLst[index];
}

AProperty* ADynPropertyObject::Properties(const wchar_t* name, int32_t* index)
{
	assert(is_valid());
	assert(name);
	for (int32_t i = 0; i < (int32_t)m_PropLst.size(); i++)
	{
		if (wcscmp(m_PropLst[i].GetName(), name) == 0)
		{
			if (index)
				*index = i;

			return &m_PropLst[i];
		}
	}
	return nullptr;
}

AVariant& ADynPropertyObject::Impl_GetPropVal(int32_t nIndex)
{
	assert(is_valid());
	assert(nIndex >= 0 && nIndex < (int32_t)m_VarLst.size());
	return m_VarLst[nIndex];
}

void ADynPropertyObject::Impl_SetPropVal(int32_t nIndex, const AVariant& var)
{
	assert(is_valid());
	m_VarLst[nIndex] = var;
}

bool ADynPropertyObject::is_valid() const
{
	size_t pl = m_PropLst.size();
	size_t vl = m_VarLst.size();
	return pl == vl;
}
