#ifndef __APROPERTY_H__
#define __APROPERTY_H__

#include "ABaseDef.h"
#include "AWString.h"
#include "AVariant.h"

#include <vector>

#define EXTERN_DEFINESET(type, name) extern ASetTemplate<type> name;
#define	DEFINE_SETBEGIN(type, name) ASetTemplate<type> name(
#define DEFINE_SETELEMENT(type, name, value) ASetTemplate<type>::ASetElement<type>(name, value),
#define DEFINE_SETEND(type) ASetTemplate<type>::ASetElement<type>());
#define DEFINE_SETFILE(type,name,filename) ASetTemplate<type> name(filename);

enum
{
	WAY_DEFAULT,
	WAY_BOOLEAN,
	WAY_INTEGER,
	WAY_INTEGER64,
	WAY_FLOAT,
	WAY_STRING,
	WAY_FILENAME,
	WAY_COLOR,
	WAY_VECTOR2,
	WAY_VECTOR3,
	WAY_VECTOR4,
	WAY_VECTOR = WAY_VECTOR3,
	WAY_BINARY,
	WAY_OBJECT,
	WAY_IDOBJECT,
	WAY_NAME,
	WAY_SCRIPT,
	WAY_UNITID,
	WAY_PATHID,
	WAY_WAYPOINTID,
	WAY_STRID,
	WAY_SFXID,
	WAY_TRIGGEROBJECT,
	WAY_MEMO,
	WAY_FONT,
	WAY_CUSTOM,
	WAY_SLIDER,
	//WAY_AUDIOEVENT,
	WAY_READONLY = (1 << 31),
};

void StringToValue(AVariant& var, const wchar_t* str);
void StringToValue(int& value, const wchar_t* str);
void StringToValue(float& value, const wchar_t* str);
void StringToValue(bool& value, const wchar_t* str);
void StringToValue(AWString& value, const wchar_t* str);
void StringToValue(A3DVECTOR3& value, const wchar_t* str);
void StringToValue(A3DCOLOR& value, const wchar_t* str);
void StringToValue(AScriptCodeData& value, const wchar_t* str);

void ValueToString(const AVariant& var, AWString& str);
void ValueToString(int value, AWString& str);
void ValueToString(float value, AWString& str);
void ValueToString(bool value, AWString& str);
void ValueToString(const AWString& value, AWString& str);
void ValueToString(const A3DVECTOR3& value, AWString& str);
void ValueToString(const A3DCOLOR& value, AWString& str);
void ValueToString(const AScriptCodeData& value, AWString& str);

class ARange
{
public:
	virtual AVariant GetMaxValue() = 0;
	virtual AVariant GetMinValue() = 0;
	virtual AVariant GetStep() = 0;
	virtual bool IsInRange(AVariant value) = 0;
};

class ASet
{
	friend int32_t InitPropertiesList();
	friend int32_t ReadSetData(const wchar_t* filename, ASet* pSet);

	virtual int32_t AddData(const wchar_t* name, const wchar_t* data) { return -1; }
	virtual int32_t ReadData() { return -1; };

public:
	virtual int32_t GetCount() = 0;
	virtual AWString GetNameByIndex(int32_t index) = 0;
	virtual AVariant GetValueByIndex(int32_t index) = 0;
	virtual AVariant GetValueByName(AWString szName) = 0;
	virtual AWString GetNameByValue(AVariant value) = 0;
	virtual int32_t FindValue(AVariant value) = 0;
	virtual int32_t FindName(AWString szName) = 0;
};

int32_t AddInitElement(ASet* pSet);
int32_t InitPropertiesList();
int32_t ReadSetData(const wchar_t* filename, ASet* pSet);

class APropertyObject;

class AProperty
{
public:
	enum
	{
		HAS_DEFAULT_VALUE = 1,
		DEFAULT_VALUE = 2,
		USE_DEFINE_DEFAULT = 3, // Content used when defining properties
		DO_NOT_SAVE = 4,
	};

	virtual bool IsUserVisable() = 0;
	virtual const wchar_t* GetName() = 0;
	virtual const wchar_t* GetPath() = 0;
	virtual const wchar_t* GetDesc() = 0;
	virtual ASet* GetPropertySet() = 0;
	virtual ARange* GetPropertyRange() = 0;
	virtual int32_t GetWay() = 0;
	virtual void SetWay(int32_t way) = 0;
	virtual int32_t GetState() = 0;
	virtual void SetState(int32_t state) = 0;
	virtual AVariant GetValue(APropertyObject* pBase) = 0;
	virtual void SetValue(APropertyObject* pBase, AVariant value) = 0;
	virtual void SetStringValue(APropertyObject* pBase, const wchar_t* s) = 0;
	virtual void GetStringValue(APropertyObject* pBase, AWString& s) = 0;
	virtual ~AProperty() {};
};

template <class TYPE> class ARangeTemplate : public ARange
{
public:
	ARangeTemplate(TYPE min, TYPE max)
		: m_min(min)
		, m_max(max)
	{}

	AVariant GetMaxValue() { return AVariant(m_max); }
	AVariant GetMinValue() { return AVariant(m_min); }
	AVariant GetStep() { return AVariant(0); }

	bool IsInRange(AVariant value)
	{
		if (TYPE(value) < m_min && TYPE(value) > m_max)
			return false;
		
		return true;
	}

private:
	TYPE m_min;
	TYPE m_max;
};

template <class TYPE> class ASliderRangeTemplate : public ARange
{
public:
	ASliderRangeTemplate(TYPE min, TYPE max, TYPE step)
		: m_min(min)
		, m_max(max)
		, m_step(step)
	{}

	AVariant GetMaxValue() { return AVariant(m_max); }
	AVariant GetMinValue() { return AVariant(m_min); }
	AVariant GetStep() { return AVariant(m_step); }

	bool IsInRange(AVariant value)
	{
		if (TYPE(value) < m_min && TYPE(value) > m_max)
			return false;
		
		return true;
	}

private:
	TYPE m_min;
	TYPE m_max;
	TYPE m_step;
};

template <class TYPE> class ASetTemplate : public ASet
{
	virtual int ReadData()
	{
		if (!m_FileName.IsEmpty())
			return ReadSetData(m_FileName, this);
		else
		{
			assert(false);
			return -1;
		}
	}

	virtual int AddData(const wchar_t* name, const wchar_t* value)
	{
		TYPE data;
		StringToValue(data, value);
		ASetElement<TYPE> element(name, data);
		m_List.push_back(element);
		return 0;
	}

public:
	template <class TYPE> class ASetElement
	{
	public:
		ASetElement()
		{
			m_szName.Empty();
		}

		ASetElement(AWString szName, TYPE data)
		{
			m_szName = szName;
			m_data = data;
		}

		ASetElement(const ASetElement& e)
		{
			m_szName = e.m_szName;
			m_data = e.m_data;
		}

		AWString GetName() { return m_szName; }
		TYPE GetData() { return m_data; }

	private:
		AWString m_szName;
		TYPE m_data;
	};

	ASetTemplate() {}
	~ASetTemplate() {}

	ASetTemplate(const wchar_t* filename) :m_FileName(filename)
	{
		AddInitElement(this);
	}

	ASetTemplate(ASetElement<TYPE> first, ...)
	{
		va_list vlist;
		ASetElement<TYPE> element = first;
		va_start(vlist, first);
		while (!element.GetName().IsEmpty())
		{
			m_List.push_back(element);
			element = va_arg(vlist, ASetElement<TYPE>);
		}
		va_end(vlist);
	}

	void AddElement(AWString szName, TYPE val)
	{
		m_List.push_back(ASetElement<TYPE>(szName, val));
	}

	void AddElement(ASetElement<TYPE>& ele)
	{
		m_List.push_back(ele);
	}

	int GetCount()
	{
		return static_cast<int32_t>(m_List.size());
	}

	AWString GetNameByIndex(int32_t index)
	{
		return m_List[index].GetName();
	}

	AVariant GetValueByIndex(int32_t index)
	{
		return AVariant(m_List[index].GetData());
	}

	AVariant GetValueByName(AWString szName)
	{
		for (size_t index = 0; index < m_List.size(); index++)
		{
			if (wcscmp(szName, m_List[index].GetName()) == 0)
				return AVariant(m_List[index].GetData());
		}

		return AVariant();
	}

	AWString GetNameByValue(AVariant value)
	{
		for (size_t index = 0; index < m_List.size(); index++)
		{
			if (m_List[index].GetData() == (TYPE)value)
				return m_List[index].GetName();
		}

		return AWString();
	}

	int FindValue(AVariant value)
	{
		for (size_t index = 0; index < m_List.size(); index++)
		{
			if (m_List[index].GetData() == (TYPE)value)
				return static_cast<int32_t>(index);
		}

		return -1;
	}

	int FindName(AWString szName)
	{
		for (size_t index = 0; index < m_List.size(); index++)
		{
			if (m_List[index].GetName() == szName)
				return static_cast<int32_t>(index);
		}

		return -1;
	}

protected:
	std::vector<ASetElement<TYPE> > m_List;
	AWString m_FileName;
};

class AVariantProperty : public AProperty
{
public:
	AVariantProperty(
		const wchar_t* clsName,
		const wchar_t* szName,
		size_t nIndex,
		ASet* pSet = nullptr,
		ARange* pRange = nullptr,
		int way = WAY_DEFAULT,
		const wchar_t* szPath = nullptr,
		bool isVisable = true,
		int  iState = 0,
		const wchar_t* szDesc = nullptr)
	{
		assert(szName);
		m_szName = szName;
		m_nIndex = nIndex;
		m_pRange = pRange;
		m_pSet = pSet;
		m_iWay = way;
		m_bUserVisable = isVisable;
		m_State = iState;

		if (szPath)
			m_szPath = szPath;

		if (szDesc)
			m_szDesc = szDesc;
	}

	bool IsUserVisable() { return m_bUserVisable; }
	int32_t GetState() { return m_State; }
	void SetState(int32_t state) { m_State = state; }

	AVariant GetValue(APropertyObject* pBase);
	void SetValue(APropertyObject* pBase, AVariant value);
	void SetStringValue(APropertyObject* pBase, const wchar_t* str);
	void GetStringValue(APropertyObject* pBase, AWString& s);

	const wchar_t* GetName() { return m_szName; }
	void SetName(const wchar_t* name) { m_szName = name; }
	const wchar_t* GetPath() { return m_szPath; }
	const wchar_t* GetDesc() { return m_szDesc; }
	void SetDesc(const wchar_t* szDesc) { m_szDesc = szDesc; }
	ASet* GetPropertySet() { return m_pSet; }
	ARange* GetPropertyRange() { return m_pRange; }
	int32_t GetWay() { return m_iWay; }
	void SetWay(int32_t way) { m_iWay = way; }

private:
	AWString m_szName;
	AWString m_szPath;
	AWString m_szDesc;
	size_t m_nIndex;
	int32_t m_iWay;
	ARange* m_pRange;
	ASet* m_pSet;
	bool m_bUserVisable;
	int32_t m_State;
};

#endif
