#pragma once

#include "AObject.h"

#define WM_LIST_DATA_CHANGED  0x0444
#define WM_LIST_DATA_CHANGING 0x0445

class CPropertyList : public CWnd
{
public:
    CPropertyList();
    virtual ~CPropertyList();

    BOOL Create(LPCWSTR title, DWORD style, const CRect& rect, CWnd* pParent, UINT nID);

    CRect GetItemRect(int64_t item);
	int GetCurSel();
	BOOL TallyPropertyBox(int64_t item);
	void SetCurSel(int sel);
	CRect GetPropertyRect(int64_t item);
    int64_t GetFirstVisible();
	void SetFirstVisible(int64_t first);
	void CleanItems();
	void AttachDataObject(APropertyObject *pData);
	void CollectItems(CPtrArray &array, int& maxnamewidth);
	void EnsureVisible(int64_t item);
	void DrawItems(CDC * pDC);
	int ItemFromPoint(CPoint pt);
	void EnableDesc(bool bEnable = true, int nHeight = 100);
	void SetDesc(const TCHAR* szDesc);
    
    APropertyObject* m_ptr_data;
    int m_int_namewidth;
    CDC m_dc_memdc;
    CPtrArray m_array_items;
    int64_t m_int_first;
    int m_int_selected;
    int64_t m_int_page;

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);

    CFont m_gdi_font;
    int m_cur_CheckBox;
    int m_cur_swing;
    bool m_bEnableDesc;
    int m_nDescHeight;
    CEdit m_wndDesc;
    CFont m_fontDesc;
};

