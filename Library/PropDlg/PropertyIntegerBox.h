#pragma once

#include "PropertyBox.h"

class ASet;

class CMyComboBox : public CComboBox
{
public:
    CMyComboBox();
    virtual ~CMyComboBox();

protected:
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    DECLARE_MESSAGE_MAP()
};

class CPropertyIntegerBox : public CPropertyBox
{
public:
    CPropertyIntegerBox();
    virtual ~CPropertyIntegerBox();

    virtual BOOL IsModified();
    virtual void DrawSelf(CDC* pDC);
    virtual void SetReadOnly(BOOL bReadOnly);

    int GetCurrentSelIndex();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void CreateEditWnd(ASet* pSet);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnEditChanged();

    BOOL m_bool_modified;
    int	m_int_index;
    CMyComboBox m_wnd_list;
    CEdit m_wnd_edit;
};

