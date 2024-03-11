#pragma once

#include "PropertyBox.h"

class CPropertyBooleanBox : public CPropertyBox
{
public:
    CPropertyBooleanBox();
    virtual ~CPropertyBooleanBox();

    virtual void DrawSelf(CDC* pDC);
    virtual void SetReadOnly(BOOL bReadOnly);
    virtual BOOL IsModified();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSelChanged();

    int m_int_sel;
    BOOL m_bool_modified;
    CComboBox m_wnd_combobox;
};

