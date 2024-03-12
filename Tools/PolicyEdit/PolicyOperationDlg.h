#pragma once

#include "afxdialogex.h"
#include "Policy.h"

extern CString strOperationList[(int32_t)CTriggerData::_e_operation::o_num];
extern CString strTargetList[(int32_t)CTriggerData::_e_target::t_num];

// forward declarations
class CPropertyList;
class ADynPropertyObject;
class COperationParam;

// CPolicyOperationDlg dialog

class CPolicyOperationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPolicyOperationDlg)

public:
	CPolicyOperationDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPolicyOperationDlg();

	void UpdateProperty(ADynPropertyObject* pProObj);

	COperationParam* GetNewOperation() { return m_pNewOperation; };

	CPolicyData* m_pPolicyData;
	bool m_bModify;
	CTriggerData::_s_operation* m_pOperationData;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_POLICY_OPERATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

	void ChangeProperty(int32_t iOpType);
	void EnableCheckBox(bool bEnable);
	void InitOperation();

	ADynPropertyObject* m_pTemp;
	COperationParam* m_pNewOperation;
	CPropertyList* m_pList;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSelChangeComboOperationType();
	afx_msg void OnSelChangeComboTargetType();

private:
	CComboBox m_cbOperationType;
	CComboBox m_cbTargetType;
	BOOL m_bWuXia;
	BOOL m_bFaShi;
	BOOL m_bSengLu;
	BOOL m_bYaoJing;
	BOOL m_bYaoShou;
	BOOL m_bMeiLing;
	BOOL m_bYuMang;
	BOOL m_bYuLing;
};
