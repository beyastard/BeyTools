#pragma once

#include "afxdialogex.h"
#include "Policy.h"

CString GetTargetStr(void* pTarget);

// The tree structure is to ensure the recovery of the trigger
struct TRIGGER_TREE
{
	TRIGGER_TREE() { pTrigger = 0; }
	~TRIGGER_TREE()
	{
		for (size_t i = 0; i < listChild.size(); ++i)
		{
			if (listChild[i])
			{
				delete listChild[i];
				listChild[i] = nullptr;
			}
		}

		if (pTrigger)
		{
			pTrigger->Release();
			delete pTrigger;
			pTrigger = nullptr;
		}

		listChild.clear();
	}

	CTriggerData* pTrigger;
	std::vector<TRIGGER_TREE*> listChild;
};

// CPolicyTriggerDlg dialog

class CPolicyTriggerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPolicyTriggerDlg)

public:
	CPolicyTriggerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPolicyTriggerDlg();

	CString TraverselTree(void* pTree);
	CString GetTriggerName(uint32_t id);

	CTriggerData* m_pTriggerData;
	CPolicyData* m_pCurrentPolicy;
	bool m_bModified;
	uint32_t m_dwTriggerID;
	bool m_bIsChanged;
	CTriggerData::_run_condition m_RunCondition;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_POLICY_TRIGGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK() {};
	virtual void OnCancel();

	void FreshList();
	void SaveOldTree(TRIGGER_TREE* pTreeRoot, CTriggerData* pTrigger);
	void StartRecord();
	void RestoreTrigger(TRIGGER_TREE* pTreeRoot);

	TRIGGER_TREE* m_pOldTree;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangeEditTriggerName();
	afx_msg void OnCheckActive();
	afx_msg void OnRadioAttackEffect();
	afx_msg void OnRadioAttackNoEffect();
	afx_msg void OnAddCondition();
	afx_msg void OnAddOperation();
	afx_msg void OnDelOperation();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnModify();
	afx_msg void OnDblclkListboxOperation();

private:
	CString m_strTriggerName;
	uint32_t m_uTriggerID;
	BOOL m_bActive;
	CString m_strProgram;
	CListBox m_listOperation;
};
