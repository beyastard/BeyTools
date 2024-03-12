#include "pch.h"
#include "PropertyBox.h"
#include "AProperty.h"
#include "AObject.h"
#include "PropertyList.h"
#include "PropertyBooleanBox.h"
#include "PropertyIntegerBox.h"
#include "PropertyInteger64Box.h"
#include "PropertyFloatBox.h"
#include "PropertyStringBox.h"
#include "PropertyDoubleBox.h"

CPropertyBox::CPropertyBox()
	: m_defaultState(0)
	, m_property_index(-1)
	, m_ptr_property(nullptr)
	, m_ptr_data(nullptr)
	, m_bReadOnly(false)
{}

CPropertyBox::~CPropertyBox() {}

BOOL CPropertyBox::Create(LPCWSTR title, DWORD style, CRect& rect, CWnd* pParent, UINT nID, APropertyObject* pData, AProperty* pProperty)
{
	//ASSERT(pProperty);

	BOOL bReadOnly = (pProperty->GetWay() & WAY_READONLY) != 0;
	LPCTSTR szClass = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_PARENTDC,
		LoadCursor(NULL, IDC_ARROW),
		GetSysColorBrush(COLOR_WINDOW),
		//GetSysColorBrush(bReadOnly ? COLOR_HIGHLIGHT : COLOR_WINDOW),
		nullptr);

	m_ptr_data = pData;
	m_ptr_property = pProperty;
	if (pProperty == nullptr)
		m_defaultState = -1;
	else
	{
		int index = -1;
		if (pData->GetProperty(pProperty->GetName(), &index) == nullptr)
		{
			assert(false);
			pParent->MessageBox(_T("Failed while looking for attribute index"));
			return FALSE;
		}

		m_property_index = index;
		m_defaultState = pData->GetState(index);
	}

	return CWnd::CreateEx(0, CSafeString(szClass), CSafeString(title), style, rect, pParent, nID, nullptr);
}

#define INNER_WAY_DOUBLE 0xffff

CPropertyBox* CPropertyBox::CreatePropertyBox(CWnd* pParent, UINT nID, CRect& rc, APropertyObject* pData, AProperty* pProperty)
{
	// Select a specific editor based on the type of AProperty
	ASSERT(pParent);
	ASSERT(pProperty);

	int way = pProperty->GetWay();

	BOOL bReadOnly = way & WAY_READONLY; // Get read-only identifier
	way = way & ~WAY_READONLY;

	if (way == WAY_DEFAULT)
	{
		switch (pProperty->GetValue(pData).GetType())
		{
		case AVariant::AVT_INVALIDTYPE: ASSERT(FALSE);          break;
		case AVariant::AVT_BOOL:       way = WAY_BOOLEAN;      break;
		case AVariant::AVT_CHAR:       way = WAY_INTEGER;      break;
		case AVariant::AVT_UCHAR:      way = WAY_INTEGER;      break;
		case AVariant::AVT_SHORT:      way = WAY_INTEGER;      break;
		case AVariant::AVT_USHORT:     way = WAY_INTEGER;      break;
		case AVariant::AVT_INT:        way = WAY_INTEGER;      break;
		case AVariant::AVT_UINT:       way = WAY_INTEGER;      break;
		case AVariant::AVT_LONG:       way = WAY_INTEGER;      break;
		case AVariant::AVT_ULONG:      way = WAY_INTEGER;      break;
		case AVariant::AVT_INT64:      way = WAY_INTEGER64;    break;
		case AVariant::AVT_UINT64:     way = WAY_INTEGER64;    break;
		case AVariant::AVT_FLOAT:      way = WAY_FLOAT;        break;
		case AVariant::AVT_DOUBLE:     way = INNER_WAY_DOUBLE; break;
		case AVariant::AVT_POBJECT:    way = WAY_OBJECT;       break;
		case AVariant::AVT_PSCRIPT:    way = WAY_SCRIPT;       break;
		case AVariant::AVT_APointF:    way = WAY_VECTOR2;      break;
		case AVariant::AVT_A3DVECTOR3: way = WAY_VECTOR3;      break;
		case AVariant::AVT_A3DVECTOR4: way = WAY_VECTOR4;      break;
		case AVariant::AVT_A3DMATRIX4:                         break;
		case AVariant::AVT_STRING:     way = WAY_STRING;       break;
		case AVariant::AVT_PABINARY:   way = WAY_BINARY;       break;
		case AVariant::AVT_PACLASSINFO:                        break;
		case AVariant::AVT_PASET:                              break;
		//case AVariant::AVT_FONT:       way = WAY_FONT;         break;
		}
	}

	CPropertyBox* pBox = nullptr;

	switch (way)
	{
	case WAY_BOOLEAN:      pBox = new CPropertyBooleanBox();   break;
	case WAY_INTEGER:      pBox = new CPropertyIntegerBox();   break;
	case WAY_INTEGER64:    pBox = new CPropertyInteger64Box(); break;
	case WAY_FLOAT:        pBox = new CPropertyFloatBox();     break;
	case WAY_STRING:       pBox = new CPropertyStringBox();    break;
	case INNER_WAY_DOUBLE: pBox = new CPropertyDoubleBox();    break;
	default:
		assert(false && "Property not yet supported");
		break;
	}

	if (pBox)
	{
		BOOL bResult = pBox->Create(CSafeString(pProperty->GetName()),
			WS_CHILD | WS_TABSTOP, rc, pParent, nID, pData, pProperty);
		ASSERT(bResult);
		pBox->SetReadOnly(bReadOnly);
	}

	return pBox;
}

BOOL CPropertyBox::IsModified()
{
	return FALSE;
}

void CPropertyBox::DrawSelf(CDC* pDC) {}

BEGIN_MESSAGE_MAP(CPropertyBox, CWnd)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()


void CPropertyBox::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	DrawSelf(&dc);
}


void CPropertyBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		UpdateData(FALSE);
		CWnd* pChild = GetWindow(GW_CHILD);
		if (pChild)
			pChild->SetFocus();
	}
	else
	{
		if (IsModified())
			UpdateData(TRUE);
	}
}


int CPropertyBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont* pFont = GetParent()->GetFont();
	LOGFONT info;
	pFont->GetLogFont(&info);
	m_gdi_font.CreateFontIndirect(&info);
	if (m_defaultState != -1)
		EnableWindow(!m_defaultState);

	return 0;
}


BOOL CPropertyBox::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}

LRESULT CPropertyBox::OnGetFont(WPARAM wParam, LPARAM lParam)
{
	return LRESULT(m_gdi_font.m_hObject);
}
