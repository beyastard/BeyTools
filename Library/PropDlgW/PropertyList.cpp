#include "pch.h"
#include "PropertyList.h"
#include "PropertyBox.h"
#include "AObject.h"

#define ITEMLEFTGAP 2
#define PROPERTYBOXIDSTART 103
#define PROPERTYDEFAULTBOXIDSTART 2003
#define DEFAULTITEMHEIGHT 22
#define AUTOSCROLLTIMERID 56644

CPropertyList::CPropertyList()
	: m_ptr_data(nullptr)
	, m_int_namewidth(0)
	, m_int_first(0)
	, m_int_selected(-1)
	, m_int_page(0)
	, m_cur_CheckBox(-1)
	, m_cur_swing(-1)
	, m_bEnableDesc(false)
	, m_nDescHeight(100)
{}

CPropertyList::~CPropertyList() {}

BOOL CPropertyList::Create(LPCWSTR title, DWORD style, const CRect& rect, CWnd* pParent, UINT nID)
{
	LPCTSTR szClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_PARENTDC,
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetSysColorBrush(COLOR_WINDOW), nullptr);

	style |= WS_VSCROLL;
	style |= WS_TABSTOP;

	return CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
		CSafeString(szClass), CSafeString(title), style, rect, pParent, nID, nullptr);
}

CRect CPropertyList::GetItemRect(int64_t item)
{
	ASSERT(m_array_items.GetSize());
	INT_PTR count = m_array_items.GetSize();
	ASSERT(item < count && item >= 0);

	CRect client;
	GetClientRect(client);
	return CRect(0, (int)(item - m_int_first) * DEFAULTITEMHEIGHT, client.Width(), (int)(item - m_int_first + 1) * DEFAULTITEMHEIGHT);
}

int CPropertyList::GetCurSel()
{
	return m_int_selected;
}

BOOL CPropertyList::TallyPropertyBox(int64_t item)
{
	CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(item);
	ASSERT(pBox);
	CRect rc;
	pBox->GetWindowRect(rc);
	ScreenToClient(rc);
	CRect item_rc = GetPropertyRect(item);
	if (item_rc != rc)
	{
		pBox->MoveWindow(item_rc);
		return TRUE;
	}

	return FALSE;
}

void CPropertyList::SetCurSel(int sel)
{
	if (m_int_selected != -1)
	{
		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(m_int_selected);
		if (!pBox->UpdateData(TRUE))
			return;

		if (!pBox->IsReadOnly())
			pBox->ShowWindow(SW_HIDE);
	}

	if (sel >= m_array_items.GetUpperBound())
		sel = (int)m_array_items.GetSize() - 1;

	if (sel != -1)
	{
		if (m_int_selected != sel)
		{
			m_int_selected = sel;

			TallyPropertyBox(sel);
			Invalidate();
		}

		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(sel);
		if (!pBox->IsWindowVisible() && !pBox->IsReadOnly())
			pBox->ShowWindow(SW_SHOW);
	}
}

CRect CPropertyList::GetPropertyRect(int64_t item)
{
	CRect rc = GetItemRect(item);

	rc.top += 1;
	rc.left += m_int_namewidth + 1;
	rc.bottom -= 1;

	return rc;
}

int64_t CPropertyList::GetFirstVisible()
{
	return m_int_first;
}

void CPropertyList::SetFirstVisible(int64_t first)
{
	if (m_array_items.GetSize() <= m_int_page)
		return;

	if (first < 0)
		first = 0;

	if (first > (m_array_items.GetSize() - m_int_page))
		first = m_array_items.GetSize() - m_int_page;

	m_int_first = first;

	SetScrollPos(SB_VERT, (int)m_int_first);

	if (m_int_selected != -1 /*&& TallyPropertyBox(GetCurSel())*/)
	{
		for (int64_t i = 0; i < m_array_items.GetSize(); i++)
			TallyPropertyBox(i);
		
		Invalidate();
	}
}

void CPropertyList::CleanItems()
{
	m_int_first = -1;
	m_int_selected = -1;

	for (int a = 0; a < m_array_items.GetSize(); a++)
	{
		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(a);
		pBox->DestroyWindow();
		delete pBox;
	}

	m_array_items.RemoveAll();

	if (::IsWindow(m_hWnd))
		ShowScrollBar(SB_VERT, FALSE);

	if (::IsWindow(m_hWnd))
		SetScrollRange(SB_VERT, 0, 0);
}

void CPropertyList::AttachDataObject(APropertyObject* pData)
{
	ASSERT(pData);
	CleanItems();
	m_ptr_data = pData;

	int maxnamewidth = 0;
	CollectItems(m_array_items, maxnamewidth);
	m_int_namewidth = maxnamewidth + ITEMLEFTGAP * 2;

	// Decide whether to show scroll bars
	if (m_int_page < m_array_items.GetSize())
	{
		ShowScrollBar(SB_VERT, TRUE);
		SetScrollRange(SB_VERT, 0, (int)(m_array_items.GetSize() - m_int_page));
	}
	else
		ShowScrollBar(SB_VERT, FALSE);
	
	SetFirstVisible(0);

	if (m_array_items.GetSize())
	{
		for (int i = 0; i < m_array_items.GetSize(); i++)
		{
			CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(i);
			if (pBox && pBox->IsReadOnly())
				pBox->ShowWindow(SW_SHOW);
		}

		m_int_first = 0;
		SetCurSel(0);

		for (int i = 0; i < m_array_items.GetSize(); i++)
			TallyPropertyBox(i);
	}

	Invalidate();
}

void CPropertyList::CollectItems(CPtrArray& array, int& maxnamewidth)
{
	CDC* pDC = GetDC();
	pDC->SelectObject(GetFont());

	APropertyObject* pData = m_ptr_data;
	ASSERT(pData);

	CSize size;
	int count = pData->GetPropertiesCount();
	int cy = 0; // DEFAULTITEMHEIGHT * 2;
	for (int a = 0; a < count; a++)
	{
		AProperty* pProperty = pData->GetProperty(a);
		ASSERT(pProperty);
		if (!pProperty->IsUserVisable())
			continue;

		size = pDC->GetTextExtent(CString(pProperty->GetName()));
		if (size.cx + 16 > maxnamewidth)
			maxnamewidth = size.cx + 16;

		CRect rc(0, cy + 2, 16, cy + 18);
		cy += DEFAULTITEMHEIGHT;

		CRect rect(0, 0, 0, 0);
		CPropertyBox* pBox = CPropertyBox::CreatePropertyBox(this, PROPERTYBOXIDSTART + a, rect, pData, pProperty);
		if (pBox)
			m_array_items.InsertAt(a, pBox);
	}

	ReleaseDC(pDC);
}

void CPropertyList::EnsureVisible(int64_t item)
{
	if (m_int_page > m_array_items.GetUpperBound())
		return;

	int64_t first = GetFirstVisible();

	if ((item - first) >= m_int_page)
		first = item - m_int_page + 1;
	
	if (item < first)
		first = item;
	
	if (first != GetFirstVisible())
		SetFirstVisible(first);
}

void CPropertyList::DrawItems(CDC* pDC)
{
	if (m_array_items.GetSize() == 0) return;

	ASSERT(pDC);
	int iSave = pDC->SaveDC();
	pDC->SelectObject(GetFont());
	CPen graypen(PS_SOLID, 1, RGB(128, 128, 128));
	CPen darkpen(PS_SOLID, 2, RGB(0, 0, 0));
	//CPen darkpen2(PS_SOLID, 1, RGB(0,0,0));

	CRect rc;
	GetClientRect(rc);
	int cy = 0;
	for (int64_t a = (m_int_first != -1 ? m_int_first : 0); a < m_array_items.GetSize(); a++)
	{
		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(a);
		ASSERT(pBox);

		// Draw attribute name
		CRect name_rc = CRect(0, cy, m_int_namewidth, cy + DEFAULTITEMHEIGHT);

		if (a == m_int_selected)
		{
			CRect rc = name_rc;
			rc.left += 18;
			FillRect(pDC->m_hDC, rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		}

		pDC->SetWindowOrg(-(m_int_namewidth + 1), -(cy + 1));
		pBox->DrawSelf(pDC);
		pDC->SetWindowOrg(0, 0);

		pDC->SelectObject(&graypen);
		pDC->MoveTo(name_rc.left, name_rc.bottom);
		pDC->LineTo(rc.right, name_rc.bottom);
		pDC->MoveTo(name_rc.right, name_rc.top);
		pDC->LineTo(name_rc.right, name_rc.bottom);

		if (pBox->m_defaultState != -1 && pBox->m_ptr_property->GetState() & AProperty::HAS_DEFAULT_VALUE)
		{
			pDC->SelectObject(&darkpen);
			CRect box_rc(name_rc.left + 4, name_rc.top + 5, name_rc.left + 15, name_rc.top + 16);
			pDC->MoveTo(box_rc.left, box_rc.top);
			pDC->LineTo(box_rc.right, box_rc.top);
			pDC->LineTo(box_rc.right, box_rc.bottom);
			pDC->LineTo(box_rc.left, box_rc.bottom);
			pDC->LineTo(box_rc.left, box_rc.top - 1);
			CRect rc(box_rc.left + 1, box_rc.top + 1, box_rc.right - 1, box_rc.bottom - 1);
			int drawColor = (m_cur_swing == a) ? COLOR_3DLIGHT : COLOR_WINDOW;
			FillRect(pDC->m_hDC, rc, GetSysColorBrush(drawColor));
			if (pBox->m_defaultState == 1)
			{
				pDC->SelectObject(&darkpen);
				pDC->MoveTo(box_rc.left + 2, box_rc.top + 2);
				pDC->LineTo(box_rc.left + 8, box_rc.top + 8);
				pDC->MoveTo(box_rc.left + 8, box_rc.top + 2);
				pDC->LineTo(box_rc.left + 2, box_rc.top + 8);
			}
		}

		CRect text_rc = name_rc;
		text_rc.left += ITEMLEFTGAP + 18;
		CString title;
		pBox->GetWindowText(title);
		pDC->DrawText(title, text_rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		cy += DEFAULTITEMHEIGHT;
		if (cy > rc.Height())
			break;
	}

	pDC->MoveTo(0, 0);
	pDC->LineTo(rc.Width(), 0);

	pDC->RestoreDC(iSave);
}

int CPropertyList::ItemFromPoint(CPoint pt)
{
	if (m_array_items.GetSize() == 0)
		return -1;

	for (int a = 0; a < m_array_items.GetSize(); a++)
	{
		CRect rect = GetItemRect(a);
		if (rect.PtInRect(pt))
			return a;
	}

	return -1;
}

void CPropertyList::EnableDesc(bool bEnable, int nHeight)
{
	m_nDescHeight = nHeight;
	if (m_bEnableDesc)
		return;

	m_bEnableDesc = bEnable;
	CRect rectClient;
	GetWindowRect(&rectClient);
	rectClient.top = rectClient.bottom - m_nDescHeight;
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_WANTRETURN;
	
	if (!m_wndDesc.Create(dwStyle, rectClient, this, 1))
		return;

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 16;
	_tcscpy(lf.lfFaceName, _T("SimSun"));
	VERIFY(m_fontDesc.CreateFontIndirect(&lf));
	m_wndDesc.SetFont(&m_fontDesc);
}

void CPropertyList::SetDesc(const TCHAR* szDesc)
{
	m_wndDesc.SetWindowText(szDesc);
	m_wndDesc.UpdateWindow();
}

BEGIN_MESSAGE_MAP(CPropertyList, CWnd)
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEACTIVATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()


BOOL CPropertyList::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		SHORT s = GetKeyState(VK_SHIFT);
		if (pMsg->wParam == VK_TAB)
		{
			if (s >= 0)
			{
				int item = GetCurSel();
				INT_PTR count = m_array_items.GetSize();
				if (item + 1 < count)
					SetCurSel(item + 1);

				EnsureVisible(GetCurSel());
			}
			else
			{
				int item = GetCurSel();
				INT_PTR count = m_array_items.GetSize();
				if (item - 1 >= 0)
					SetCurSel(item - 1);

				EnsureVisible(GetCurSel());
			}

			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CPropertyList::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		for (int a = 0; a < m_array_items.GetSize(); a++)
		{
			CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(a);
			if (!pBox->UpdateData(TRUE))
			{
				pDX->Fail();
				return;
			}
		}
	}
	else
	{
		INT_PTR count = m_array_items.GetSize();
		for (int a = 0; a < count; a++)
		{
			CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(a);
			if (pBox)
				pBox->UpdateData(FALSE);
		}

		Invalidate();
	}

	CWnd::DoDataExchange(pDX);
}

void CPropertyList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (!pScrollBar)
	{
		SCROLLINFO info;
		info.cbSize = sizeof(info);
		GetScrollInfo(SB_VERT, &info);

		switch (nSBCode)
		{
		case SB_BOTTOM:     SetFirstVisible(m_array_items.GetUpperBound()); break;
		case SB_LINEUP:     SetFirstVisible(info.nPos - 1);                      break;
		case SB_PAGEUP:     SetFirstVisible(info.nPos - m_int_page);             break;
		case SB_LINEDOWN:   SetFirstVisible(info.nPos + 1);                      break;
		case SB_PAGEDOWN:   SetFirstVisible(info.nPos + m_int_page);             break;
		case SB_THUMBTRACK: SetFirstVisible(nPos);                               break;
		case SB_TOP:        SetFirstVisible(0);                                  break;
		case SB_ENDSCROLL:  SetFirstVisible(info.nPos);                          break;
		}
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CPropertyList::OnSize(UINT nType, int cx, int cy)
{
	if (m_bEnableDesc && m_hWnd)
		cy -= m_nDescHeight;

	CWnd::OnSize(nType, cx, cy);

	CBitmap bmp;
	CDC* pDC = GetDC();
	bmp.CreateCompatibleBitmap(pDC, cx, cy);
	m_dc_memdc.SelectObject(&bmp);
	m_int_page = cy / DEFAULTITEMHEIGHT;
	ReleaseDC(pDC);

	if (m_int_page < m_array_items.GetSize())
	{
		ShowScrollBar(SB_VERT, TRUE);
		SetScrollRange(SB_VERT, 0, (int)(m_array_items.GetSize() - m_int_page));
	}
	else
		ShowScrollBar(SB_VERT, FALSE);
	
	UpdateWindow();

	if (!m_wndDesc.m_hWnd)
		return;

	CRect rectDesc(0, cy, cx, cy + m_nDescHeight);
	m_wndDesc.MoveWindow(&rectDesc);
}


int CPropertyList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont* pFont = GetParent()->GetFont();
	LOGFONT info;
	pFont->GetLogFont(&info);
	m_gdi_font.CreateFontIndirect(&info);
	CDC* pDC = GetDC();
	m_dc_memdc.CreateCompatibleDC(pDC);
	ReleaseDC(pDC);

	return 0;
}


void CPropertyList::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	DrawItems(&m_dc_memdc);
	CRect rc;
	GetClientRect(rc);
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &m_dc_memdc, 0, 0, SRCCOPY);
}


BOOL CPropertyList::OnEraseBkgnd(CDC* pDC)
{
	CRect client;
	GetClientRect(client);
	m_dc_memdc.SetBkColor(GetSysColor(COLOR_WINDOW));
	FillRect(m_dc_memdc.m_hDC, client, GetSysColorBrush(COLOR_WINDOW));
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}


void CPropertyList::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	int item = ItemFromPoint(point);
	if (item != -1 && item != GetCurSel())
		SetCurSel(item);
	
	if (item != -1)
	{
		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(item);
		if (m_bEnableDesc)
			m_wndDesc.SetWindowText(((ADynPropertyObject*)(pBox->m_ptr_data))->GetPropDesc(item));

		if (pBox->m_defaultState != -1 && pBox->m_ptr_property->GetState() & AProperty::HAS_DEFAULT_VALUE)
		{
			CRect rect = GetItemRect(item);
			rect.right = rect.left + 15;
			rect.left = rect.left + 2;
			if (rect.PtInRect(point))
				m_cur_CheckBox = item;
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}


void CPropertyList::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags & MK_LBUTTON)
	{
		CRect rc;
		GetClientRect(rc);

		if (rc.PtInRect(point))
		{
			KillTimer(AUTOSCROLLTIMERID);
			int item = ItemFromPoint(point);
			if (item != -1 && item != GetCurSel())
				SetCurSel(item);
		}
		else
			SetTimer(AUTOSCROLLTIMERID, 55, NULL);
	}

	int item = ItemFromPoint(point);
	int tmp = -1;
	if (item != -1)
	{
		CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(item);
		if (pBox->m_defaultState != -1 && pBox->m_ptr_property->GetState() & AProperty::HAS_DEFAULT_VALUE)
		{
			CRect rect = GetItemRect(item);
			rect.right = rect.left + 15;
			rect.left = rect.left + 2;
			if (rect.PtInRect(point))
				tmp = item;
		}
	}

	if (tmp != m_cur_swing)
	{
		m_cur_swing = tmp;
		Invalidate();
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CPropertyList::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		KillTimer(AUTOSCROLLTIMERID);
		ReleaseCapture();
		EnsureVisible(GetCurSel());

		// This is the code that handles the previous checkBox
		if (m_cur_CheckBox != -1)
		{
			CPropertyBox* pBox = (CPropertyBox*)m_array_items.GetAt(m_cur_CheckBox);
			if (pBox->m_defaultState != -1 && pBox->m_ptr_property->GetState() & AProperty::HAS_DEFAULT_VALUE)
			{
				CRect rect = GetItemRect(m_cur_CheckBox);
				rect.right = rect.left + 15;
				rect.left = rect.left + 2;
				if (rect.PtInRect(point))
				{
					pBox->m_defaultState = 1 - pBox->m_defaultState;
					pBox->m_ptr_data->SetState(pBox->m_property_index, pBox->m_defaultState);
					pBox->EnableWindow(!pBox->m_defaultState);
					Invalidate();
				}
			}

			m_cur_CheckBox = -1;
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}


BOOL CPropertyList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta > 0)
	{
		// scroll forward
		OnVScroll(SB_LINEUP, 0, NULL);
		OnVScroll(SB_ENDSCROLL, 0, NULL);
	}
	else
	{
		// scroll back
		OnVScroll(SB_LINEDOWN, 0, NULL);
		OnVScroll(SB_ENDSCROLL, 0, NULL);
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


int CPropertyList::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	SetFocus();

	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


void CPropertyList::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == AUTOSCROLLTIMERID)
	{
		CPoint pt;
		GetCursorPos(&pt);
		CRect rc;
		GetClientRect(rc);
		ScreenToClient(&pt);

		if (pt.y < rc.top)
		{
			int sel = GetCurSel();
			sel--;
			if (sel < 0)
				sel = 0;

			SetCurSel(sel);
		}
		else if (pt.y > rc.bottom)
		{
			int sel = GetCurSel();
			sel++;
			SetCurSel(sel);
		}

		EnsureVisible(GetCurSel());
	}

	CWnd::OnTimer(nIDEvent);
}


void CPropertyList::OnDestroy()
{
	CleanItems();
	CWnd::OnDestroy();
}

LRESULT CPropertyList::OnGetFont(WPARAM wParam, LPARAM lParam)
{
	return LRESULT(m_gdi_font.m_hObject);
}
