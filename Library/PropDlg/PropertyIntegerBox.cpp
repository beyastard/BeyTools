#include "pch.h"
#include "PropertyIntegerBox.h"
#include "AProperty.h"
#include "PropertyList.h"
#include "Convert.h"

CMyComboBox::CMyComboBox() {}
CMyComboBox::~CMyComboBox() {}

BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BOOL CMyComboBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return true;
}

CPropertyIntegerBox::CPropertyIntegerBox()
	: m_bool_modified(false)
	, m_int_index(-1)
{}

CPropertyIntegerBox::~CPropertyIntegerBox() {}

BOOL CPropertyIntegerBox::IsModified()
{
	return m_bool_modified;
}

void CPropertyIntegerBox::DrawSelf(CDC* pDC)
{
	// commented by hedi: shouldn't call UpdateData in DrawSelf, that will cause
	// the window restore to current saved data when the application be activated again
	//UpdateData(FALSE);

	CString text;
	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
		m_wnd_edit.GetWindowText(text);

	if (IsWindow(m_wnd_list.GetSafeHwnd()))
		m_wnd_list.GetWindowText(text);

	ASSERT(pDC);

	int edge_cx = GetSystemMetrics(SM_CXDLGFRAME);
	int edge_cy = GetSystemMetrics(SM_CYDLGFRAME);

	CRect rc;
	GetClientRect(rc);

	rc.top += edge_cy;
	rc.left += edge_cx + 1;
	rc.bottom -= edge_cy;

	int iSave = pDC->SaveDC();
	pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));

	pDC->DrawText(text, rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

	pDC->RestoreDC(iSave);
}

void CPropertyIntegerBox::SetReadOnly(BOOL bReadOnly)
{
	if (m_wnd_edit.GetSafeHwnd())
		m_wnd_edit.SetReadOnly(bReadOnly);
	
	if (m_wnd_list.GetSafeHwnd())
		m_wnd_list.EnableWindow(!bReadOnly);
	
	CPropertyBox::SetReadOnly(bReadOnly);
}

int CPropertyIntegerBox::GetCurrentSelIndex()
{
	if (IsWindow(m_wnd_list.GetSafeHwnd()))
		return m_wnd_list.GetCurSel();
	
	return -1;
}


BEGIN_MESSAGE_MAP(CPropertyIntegerBox, CPropertyBox)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL(CBN_SELCHANGE, 101, OnEditChanged)
	ON_CONTROL(EN_CHANGE, 101, OnEditChanged)
END_MESSAGE_MAP()


void CPropertyIntegerBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyBox::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		UpdateData(FALSE);
		ASet* pSet = m_ptr_property->GetPropertySet();
		if (!pSet)
		{
			m_wnd_edit.SetFocus();
			m_wnd_edit.SetSel(0, -1);
		}
		else
		{
			CWnd* pChild = GetWindow(GW_CHILD);
			if (pChild)
				pChild->SetFocus();
		}
	}
	else
	{
		if (IsModified())
			UpdateData(TRUE);
	}
}


int CPropertyIntegerBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(m_ptr_property);
	ASet* pSet = m_ptr_property->GetPropertySet();
	CreateEditWnd(pSet);
	UpdateData(FALSE);

	return 0;
}


void CPropertyIntegerBox::OnSize(UINT nType, int cx, int cy)
{
	CPropertyBox::OnSize(nType, cx, cy);

	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
		m_wnd_edit.MoveWindow(4, 4, cx - 4, cy - 4);
	
	if (IsWindow(m_wnd_list.GetSafeHwnd()))
		m_wnd_list.MoveWindow(0, 0, cx, cy);
}

void CPropertyIntegerBox::OnEditChanged()
{
	m_bool_modified = TRUE;
	CPropertyList* pList = (CPropertyList*)GetParent();
	GetParent()->GetParent()->PostMessage(WM_LIST_DATA_CHANGED, (WPARAM)pList, pList->m_int_selected);
}


void CPropertyIntegerBox::DoDataExchange(CDataExchange* pDX)
{
	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
	{
		if (pDX->m_bSaveAndValidate)
		{
			int value;
			DDX_Text(pDX, 101, value);
			m_bool_modified = FALSE;
			ARange* pRange = m_ptr_property->GetPropertyRange();
			if (pRange)
				DDV_MinMaxInt(pDX, value, pRange->GetMinValue(), pRange->GetMaxValue());
			
			m_ptr_property->SetValue(m_ptr_data, value);
		}
		else
		{
			int value = m_ptr_property->GetValue(m_ptr_data);
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
				DWORD_PTR value = m_wnd_list.GetItemData(m_int_index);
				m_ptr_property->SetValue(m_ptr_data, value);
				m_bool_modified = FALSE;
			}
		}
		else
		{
			ASet* pSet = m_ptr_property->GetPropertySet();
			ASSERT(pSet);
			m_int_index = pSet->FindValue(m_ptr_property->GetValue(m_ptr_data));
			//if(m_int_index == -1)
			//	m_int_index = 0;

			DDX_CBIndex(pDX, 101, m_int_index);
		}
	}

	CPropertyBox::DoDataExchange(pDX);
}

void CPropertyIntegerBox::CreateEditWnd(ASet* pSet)
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
			int value = pSet->GetValueByIndex(a);
			CString name = Convert::GB18030ToCStringW(pSet->GetNameByIndex(a));
			CString text;
			text.Format(L"%d(%s)", value, (LPCTSTR)name);
			int item = m_wnd_list.AddString(text);
			m_wnd_list.SetItemData(item, value);
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
