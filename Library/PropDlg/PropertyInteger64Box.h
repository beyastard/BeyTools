#pragma once

#include "PropertyIntegerBox.h"

#include <map>

class CPropertyInteger64Box : public CPropertyIntegerBox
{
public:
    CPropertyInteger64Box();
    virtual ~CPropertyInteger64Box();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void CreateEditWnd(ASet* pSet);

    DECLARE_MESSAGE_MAP()

    std::map<int, __int64> m_mapCbIndexToData;
};

