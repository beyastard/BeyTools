#ifndef __AOBJECT_H__
#define __AOBJECT_H__

#include <cassert>
#include <vector>

#include "AProperty.h"

class APropertyObject
{
public:
	AProperty* GetProperty(int32_t index) { return Properties(index); }
	AProperty* GetProperty(const char* name, int32_t* index = nullptr) { return Properties(name, index); }
	AVariant& GetPropVal(int32_t nIndex) { return Impl_GetPropVal(nIndex); }
	void SetPropVal(int32_t nIndex, const AVariant& var) { Impl_SetPropVal(nIndex, var); }
	int32_t GetPropertiesCount() { return PropertiesCount(); }

	bool InitStateTable();
	
	void SetState(int32_t index, int32_t state) { m_StateList[index] = state; }
	int32_t GetState(int32_t index) { return m_StateList[index]; }

protected:
	APropertyObject() :m_version(0) {}
	virtual int32_t PropertiesCount() { return 0; }
	virtual AProperty* Properties(int32_t index) { return nullptr; }
	virtual AProperty* Properties(const char* name, int32_t* index = nullptr) { return nullptr; }

	virtual AVariant& Impl_GetPropVal(int32_t nIndex) = 0;
	virtual void Impl_SetPropVal(int32_t nIndex, const AVariant& var) = 0;

    int32_t m_version;
    std::vector<int32_t> m_StateList;
};

class ADynPropertyObject : public APropertyObject
{
public:
	ADynPropertyObject();
	virtual ~ADynPropertyObject() {}

	void DynAddProperty(const AVariant& varValue,
		const char* szName, ASet* pSet = nullptr, ARange* pRange = nullptr, int32_t way = WAY_DEFAULT,
		const char* szPath = nullptr, bool isVisable = true, int32_t iState = 0, const char* szDesc = 0);

	const char* GetPropDesc(int32_t nIndex);
	void SetPropDesc(int32_t nIndex, const char* szDesc);
	void SetPropName(int32_t nIndex, const char* name);
	void Clear();

protected:
	virtual int32_t PropertiesCount();
	virtual AProperty* Properties(int32_t index);
	virtual AProperty* Properties(const char* name, int32_t* index = nullptr);
	virtual AVariant& Impl_GetPropVal(int32_t nIndex);
	virtual void Impl_SetPropVal(int32_t nIndex, const AVariant& var);
	bool is_valid() const;

	typedef AVariantProperty VarPropTemp;
	std::vector<VarPropTemp> m_PropLst;
	std::vector<AVariant> m_VarLst;
};

struct CUSTOM_FUNCS
{
	virtual ~CUSTOM_FUNCS() {}
	virtual BOOL CALLBACK OnActivate() = 0;
	virtual LPCTSTR CALLBACK OnGetShowString() const = 0;
	virtual AVariant CALLBACK OnGetValue() const = 0;
	virtual void CALLBACK OnSetValue(const AVariant& var) = 0;
};

#endif
