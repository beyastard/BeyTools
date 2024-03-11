#include "pch.h"
#include "PropertyStringBox.h"
#include "AProperty.h"
#include "PropertyList.h"
#include "AString.h"
#include "Convert.h"

CPropertyStringBox::CPropertyStringBox()
	: m_bool_modified(false)
{}

CPropertyStringBox::~CPropertyStringBox() {}

BOOL CPropertyStringBox::IsModified()
{
	return m_bool_modified;
}

void CPropertyStringBox::DrawSelf(CDC* pDC)
{
	ASSERT(pDC);

	CString text;
	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
		m_wnd_edit.GetWindowText(text);

	if (IsWindow(m_wnd_list.GetSafeHwnd()))
		m_wnd_list.GetWindowText(text);

	int edge_cx = GetSystemMetrics(SM_CXDLGFRAME);
	int edge_cy = GetSystemMetrics(SM_CYDLGFRAME);

	CRect rc;
	GetClientRect(rc);

	rc.top += edge_cy + 1;
	rc.left += edge_cx + 1;
	rc.bottom -= edge_cy;
	rc.right -= 5;

	int iSave = pDC->SaveDC();
	pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));

	pDC->DrawText(text, rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

	pDC->RestoreDC(iSave);
}

void CPropertyStringBox::SetReadOnly(BOOL bReadOnly)
{
	if (m_wnd_edit.GetSafeHwnd())
		m_wnd_edit.SetReadOnly(bReadOnly);
	
	if (m_wnd_list.GetSafeHwnd())
		m_wnd_list.EnableWindow(!bReadOnly);
	
	CPropertyBox::SetReadOnly(bReadOnly);
}

BEGIN_MESSAGE_MAP(CPropertyStringBox, CPropertyBox)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL(CBN_SELCHANGE, 101, OnEditChanged)
	ON_CONTROL(EN_CHANGE, 101, OnEditChanged)
END_MESSAGE_MAP()


void CPropertyStringBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

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


int CPropertyStringBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(m_ptr_property);
	ASet* pSet = m_ptr_property->GetPropertySet();

	if (pSet)
	{
		// Use list controls
		int height = (pSet->GetCount() + 1) * 20;
		BOOL bResult = m_wnd_list.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
			CRect(0, 0, 0, height > 250 ? 250 : height), this, 101);
		ASSERT(bResult);

		m_wnd_list.SetFont(GetFont());

		int count = pSet->GetCount();
		for (int a = 0; a < count; a++)
		{
			//AString value = pSet->GetValueByIndex(a);
			CString name = Convert::CharToWChar(pSet->GetNameByIndex(a).GetCharString());
			CString text;
			//text.Format("%d(%s)", value, name);
			text.Format(_T("%s"), (LPCTSTR)name);
			int item = m_wnd_list.AddString(text);
			m_wnd_list.SetItemData(item, a);
		}
	}
	else
	{
		// Use edit controls
		BOOL bResult = m_wnd_edit.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 101);
		ASSERT(bResult);
		m_wnd_edit.SetFont(GetFont());
	}

	UpdateData(FALSE);

	return 0;
}


void CPropertyStringBox::OnSize(UINT nType, int cx, int cy)
{
	CPropertyBox::OnSize(nType, cx, cy);

	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
		m_wnd_edit.MoveWindow(4, 4, cx - 4, cy - 4);
	
	if (IsWindow(m_wnd_list.GetSafeHwnd()))
		m_wnd_list.MoveWindow(0, 0, cx, cy);
}

void CPropertyStringBox::OnEditChanged()
{
	m_bool_modified = TRUE;
	CPropertyList* pList = (CPropertyList*)GetParent();
	GetParent()->GetParent()->PostMessage(WM_LIST_DATA_CHANGED, (WPARAM)pList, pList->m_int_selected);
}


void CPropertyStringBox::DoDataExchange(CDataExchange* pDX)
{
	if (IsWindow(m_wnd_edit.GetSafeHwnd()))
	{
		if (pDX->m_bSaveAndValidate)
		{
			DDX_Text(pDX, 101, m_str_value);
			m_ptr_property->SetValue(m_ptr_data,
				AString(CSafeString(Convert::WCharToChar(m_str_value)).GetAnsi()).GetCharString());
			m_wnd_edit.SetModify(FALSE);
		}
		else
		{
			m_str_value = (AString)m_ptr_property->GetValue(m_ptr_data);
			DDX_Text(pDX, 101, m_str_value);
		}
	}

	if (IsWindow(m_wnd_list.GetSafeHwnd()))
	{
		if (pDX->m_bSaveAndValidate)
		{
			// Convert index to value
			int index;
			DDX_CBIndex(pDX, 101, index);
			if (index != -1)
			{
				index = (int)m_wnd_list.GetItemData(index);
				ASet* pSet = m_ptr_property->GetPropertySet();
				AVariant var = pSet->GetValueByIndex(index);
				AString value = var;
				m_ptr_property->SetValue(m_ptr_data, value);
				m_bool_modified = FALSE;
			}
		}
		else
		{
			ASet* pSet = m_ptr_property->GetPropertySet();
			ASSERT(pSet);
			int index = pSet->FindValue(m_ptr_property->GetValue(m_ptr_data));
			DDX_CBIndex(pDX, 101, index);
		}
	}

	CPropertyBox::DoDataExchange(pDX);
}
