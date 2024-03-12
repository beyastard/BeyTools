#include "pch.h"
#include "PropertyInteger64Box.h"
#include "AProperty.h"
#include "PropertyList.h"

CPropertyInteger64Box::CPropertyInteger64Box() {}
CPropertyInteger64Box::~CPropertyInteger64Box() {}

BEGIN_MESSAGE_MAP(CPropertyInteger64Box, CPropertyIntegerBox)
END_MESSAGE_MAP()

void CPropertyInteger64Box::DoDataExchange(CDataExchange* pDX)
{
	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
	{
		if (pDX->m_bSaveAndValidate)
		{
			__int64 value;
			DDX_Text(pDX, 101, value);
			m_bool_modified = FALSE;
			ARange* pRange = m_ptr_property->GetPropertyRange();
			if (pRange)
				DDV_MinMaxLongLong(pDX, value, pRange->GetMinValue(), pRange->GetMaxValue());

			m_ptr_property->SetValue(m_ptr_data, value);
		}
		else
		{
			__int64 value = m_ptr_property->GetValue(m_ptr_data);
			DDX_Text(pDX, 101, value);
		}
	}

	if (IsWindow(m_wnd_list.GetSafeHwnd()))
	{
		if (pDX->m_bSaveAndValidate)
		{
			// Convert index to value
			DDX_CBIndex(pDX, 101, m_int_index);
			if (m_int_index != -1)
			{
				__int64 value = m_mapCbIndexToData[m_int_index];
				m_ptr_property->SetValue(m_ptr_data, value);
				m_bool_modified = FALSE;
			}
		}
		else
		{
			ASet* pSet = m_ptr_property->GetPropertySet();
			ASSERT(pSet);
			m_int_index = pSet->FindValue(m_ptr_property->GetValue(m_ptr_data));
			DDX_CBIndex(pDX, 101, m_int_index);
		}
	}

	CPropertyIntegerBox::DoDataExchange(pDX);
}

void CPropertyInteger64Box::CreateEditWnd(ASet* pSet)
{
	if (pSet)
	{
		// Use list controls
		int height = (pSet->GetCount() + 1) * 25;
		BOOL bResult = m_wnd_list.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
			CRect(0, 0, 0, height > 250 ? 250 : height), this, 101);
		ASSERT(bResult);

		m_wnd_list.SetFont(GetFont());

		int count = pSet->GetCount();
		for (int a = 0; a < count; a++)
		{
			__int64 value = pSet->GetValueByIndex(a);
			CString name =pSet->GetNameByIndex(a).GetWCharString();
			CString text;
			text.Format(_T("%I64d(%s)"), value, (LPCTSTR)name);
			int item = m_wnd_list.AddString(text);
			m_mapCbIndexToData[item] = value;
			//m_wnd_list.SetItemData(item, value);
		}
	}
	else
	{
		// Use edit controls
		BOOL bResult = m_wnd_edit.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 101);
		ASSERT(bResult);
		m_wnd_edit.SetFont(GetFont());
	}
}
