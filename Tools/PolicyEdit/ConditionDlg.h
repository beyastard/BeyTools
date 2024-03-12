#pragma once
#include "afxdialogex.h"
#include "Policy.h"

extern CString strConditionList[(int32_t)CTriggerData::_e_condition::c_num];
extern CString strConditionSign[(int32_t)CTriggerData::_e_condition::c_num];

// CConditionDlg dialog

class CConditionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConditionDlg)

public:
	CConditionDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CConditionDlg();

	uint32_t m_uConditionFunc;
	CTriggerData::_s_tree_item* m_pRoot;
	bool bModify;
	bool m_bIsRoot;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONDITION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

	void FreshInterface();
	CString TraverselTree(void* pTree);
	bool Check(CTriggerData::_s_tree_item* left, CTriggerData::_s_tree_item* right);

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSelChangeComboFunc();
	afx_msg void OnChangeEditParamInt();
	afx_msg void OnChangeEditParamFloat();
	afx_msg void OnButtonSub1();
	afx_msg void OnButtonSub2();

private:
	CComboBox m_cbFunc;
	CString m_strLeft;
	CString m_strRight;
	int32_t m_iConditionParam;
	float m_fConditionParam;
public:
	
};
