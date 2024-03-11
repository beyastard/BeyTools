#pragma once

class APropertyObject;
class AProperty;
class APropertyInterface;

class CPropertyBox : public CWnd
{
public:
    CPropertyBox();
    virtual ~CPropertyBox();

    BOOL Create(LPCSTR title, DWORD style, CRect& rect, CWnd* pParent, UINT nID, APropertyObject* pData, AProperty* pProperty);

    static CPropertyBox* CreatePropertyBox(CWnd* pParent, UINT nID, CRect& rc, APropertyObject* pData, AProperty* pProperty);

    virtual BOOL IsModified();
    virtual void DrawSelf(CDC* pDC);

    BOOL IsReadOnly() const { return m_bReadOnly; }
    virtual void SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }

    int m_defaultState;
    int m_property_index;
    AProperty* m_ptr_property;
    APropertyObject* m_ptr_data;

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);

    CFont m_gdi_font;
    BOOL m_bReadOnly;
};
