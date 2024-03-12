#pragma once

#include "PropertyBox.h"

class CPropertyStringBox : public CPropertyBox
{
public:
    CPropertyStringBox();
    virtual ~CPropertyStringBox();

    virtual BOOL IsModified();
    virtual void DrawSelf(CDC* pDC);
    virtual void SetReadOnly(BOOL bReadOnly);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnEditChanged();

    CString m_str_value;
    CComboBox m_wnd_list;
    CEdit m_wnd_edit;
    bool m_bool_modified;
};

