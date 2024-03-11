#include "pch.h"
#include "PropertyDoubleBox.h"
#include "AProperty.h"
#include "PropertyList.h"

CPropertyDoubleBox::CPropertyDoubleBox() {}
CPropertyDoubleBox::~CPropertyDoubleBox() {}

void CPropertyDoubleBox::DrawSelf(CDC* pDC)
{
	// commented by hedi: shouldn't call UpdateData in DrawSelf, that will cause
	// the window restore to current saved data when the application be activated again
	//UpdateData(FALSE);

	CString text;
	m_wnd_edit.GetWindowText(text);
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

BOOL CPropertyDoubleBox::IsModified()
{
	return m_wnd_edit.GetModify();
}

void CPropertyDoubleBox::SetReadOnly(BOOL bReadOnly)
{
	if (m_wnd_edit.GetSafeHwnd())
		m_wnd_edit.SetReadOnly(bReadOnly);
	
	CPropertyBox::SetReadOnly(bReadOnly);
}

BEGIN_MESSAGE_MAP(CPropertyDoubleBox, CPropertyBox)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL(EN_CHANGE, 101, OnEditChanged)
END_MESSAGE_MAP()


void CPropertyDoubleBox::OnShowWindow(BOOL bShow, UINT nStatus)
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


int CPropertyDoubleBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bResult = m_wnd_edit.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 101);

	ASSERT(bResult);
	m_wnd_edit.SetFont(GetFont());
	UpdateData(FALSE);

	return 0;
}


void CPropertyDoubleBox::OnSize(UINT nType, int cx, int cy)
{
	CPropertyBox::OnSize(nType, cx, cy);

	m_wnd_edit.MoveWindow(4, 4, cx - 4, cy - 4);
}

void CPropertyDoubleBox::OnEditChanged()
{
	CString csText;
	m_wnd_edit.GetWindowText(csText);
	if (csText.GetLength() == 0)
		return;

	m_wnd_edit.SetModify(TRUE);
	CPropertyList* pList = (CPropertyList*)GetParent();
	GetParent()->GetParent()->PostMessage(WM_LIST_DATA_CHANGED, (WPARAM)pList, pList->m_int_selected);
}


void CPropertyDoubleBox::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		double value;
		DDX_Text(pDX, 101, value);
		m_wnd_edit.SetModify(FALSE);
		ARange* pRange = m_ptr_property->GetPropertyRange();
		if (pRange)
			DDV_MinMaxDouble(pDX, value, pRange->GetMinValue(), pRange->GetMaxValue());
		
		m_ptr_property->SetValue(m_ptr_data, value);
	}
	else
	{
		double value = m_ptr_property->GetValue(m_ptr_data);
		DDX_Text(pDX, 101, value);
	}

	CPropertyBox::DoDataExchange(pDX);
}
