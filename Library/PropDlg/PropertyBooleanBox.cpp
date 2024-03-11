#include "pch.h"
#include "PropertyBooleanBox.h"
#include "AProperty.h"
#include "PropertyList.h"

CPropertyBooleanBox::CPropertyBooleanBox()
	: m_int_sel(-1)
	, m_bool_modified(false)
{}

CPropertyBooleanBox::~CPropertyBooleanBox() {}

void CPropertyBooleanBox::DrawSelf(CDC* pDC)
{
	ASSERT(pDC);

	int edge_cx = GetSystemMetrics(SM_CXDLGFRAME);
	int edge_cy = GetSystemMetrics(SM_CYDLGFRAME);

	CString text;
	m_wnd_combobox.GetLBText(m_wnd_combobox.GetCurSel(), text);

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

void CPropertyBooleanBox::SetReadOnly(BOOL bReadOnly)
{
	if (m_wnd_combobox.GetSafeHwnd())
		m_wnd_combobox.EnableWindow(!bReadOnly);
	
	CPropertyBox::SetReadOnly(bReadOnly);
}

BOOL CPropertyBooleanBox::IsModified()
{
	return m_bool_modified;
}

BEGIN_MESSAGE_MAP(CPropertyBooleanBox, CPropertyBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL(CBN_SELCHANGE, 101, OnSelChanged)
END_MESSAGE_MAP()


int CPropertyBooleanBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertyBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bResult = m_wnd_combobox.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, CRect(0, 0, 0, 100), this, 101);

	m_wnd_combobox.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
	m_wnd_combobox.AddString(_T("True"));
	m_wnd_combobox.AddString(_T("False"));

	ASSERT(bResult);
	ASSERT(m_ptr_property);
	UpdateData(FALSE);

	return 0;
}


void CPropertyBooleanBox::OnSize(UINT nType, int cx, int cy)
{
	CPropertyBox::OnSize(nType, cx, cy);

	m_wnd_combobox.MoveWindow(0, 0, cx, cy);
}

void CPropertyBooleanBox::OnSelChanged()
{
	m_bool_modified = TRUE;
	CPropertyList* pList = (CPropertyList*)GetParent();
	GetParent()->GetParent()->PostMessage(WM_LIST_DATA_CHANGED, (WPARAM)pList, pList->m_int_selected);
}


void CPropertyBooleanBox::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		DDX_CBIndex(pDX, 101, m_int_sel);
		m_ptr_property->SetValue(m_ptr_data, m_int_sel == 0 ? true : false);
		m_bool_modified = FALSE;
	}
	else
	{
		bool bProperty = m_ptr_property->GetValue(m_ptr_data);
		m_int_sel = bProperty ? 0 : 1;
		DDX_CBIndex(pDX, 101, m_int_sel);
	}

	CPropertyBox::DoDataExchange(pDX);
}
