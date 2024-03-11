#pragma once

#include "PropertyBox.h"

class CPropertyDoubleBox : public CPropertyBox
{
public:
    CPropertyDoubleBox();
    virtual ~CPropertyDoubleBox();

    virtual void DrawSelf(CDC* pDC);
    virtual BOOL IsModified();
    virtual void SetReadOnly(BOOL bReadOnly);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnEditChanged();

    CEdit m_wnd_edit;
};

