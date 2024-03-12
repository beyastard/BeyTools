
// PolicyDlg.h : header file
//

#pragma once

#include "Policy.h"
#include "PolicyLoader.h"

#include <memory>

// CPolicyDlg dialog
class CPolicyDlg : public CDialogEx
{
// Construction
public:
	CPolicyDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CPolicyDlg();

	CString GetTriggerName(uint32_t id);

	struct TIMER_TEST
	{
		uint32_t id;

		uint32_t counter;
		uint32_t skipCounter;

		uint32_t period;
		uint32_t last_time;
};

	struct TRIGGER_STATUS
	{
		uint32_t id;
		bool bActive;
		bool bRun;

		TRIGGER_STATUS()
			: id(0)
			, bActive(false)
			, bRun(false)
		{}

		TRIGGER_STATUS(uint32_t _id, bool _active, bool _run)
			: id(_id)
			, bActive(_active)
			, bRun(_run)
		{}
	};

	std::wstring g_strWorkDir;
	uint32_t m_nPolicyID;
	CPolicyData* m_pCurrentPolicy;
	CString m_strCurrentPathName;
	bool m_bReadOnly;
	bool m_bStartTest;

	// event trigger
	bool m_bSkipKillPlayer;
	bool m_bSkipRandom;
	bool m_bSkipStartAttack;
	bool m_bSkipBorn;
	bool m_bSkipDied;
	bool m_bPathEndPoint;
	bool m_bAttackBySkill;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POLICYEDIT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnCancel();

	bool InitApp();
	std::unique_ptr<wchar_t[]> GetFullCurrentDirectory();

	enum ProgramInfo
	{
		Version,
		Copyright,
		Company,
		Product
	};
	CString GetAppData(ProgramInfo info);

	void FreshTriggerList();
	bool TraverselTree(void* pTree, CTriggerData* pTrigger);

	void DelRedundancy();
	bool TriggerIsUse(uint32_t id);
	bool TraceTrigger(CTriggerData* pTrigger, uint32_t id);

	// simulation methods
	bool IsTimeCome(uint32_t iId);
	bool HpLess(float fLess);
	bool KillPlayer();
	bool Born();
	bool Died();
	bool Random(float fProbability);
	bool StartAttack();
	bool IsPathEndPoint();
	bool IsAttackBySkill();

	void AddLogMsg(const wchar_t* szMsg);

	bool RunOperation(void* pTriggerPtr, void* pOperation);
	void RunTrigger(uint32_t id);

	void SetTriggerActiveStatus(uint32_t id, bool bActive);
	void SetTriggerRunStatus(uint32_t id, bool bRun);

	bool GetTriggerActiveStatus(uint32_t id);
	bool GetTriggerRunStatus(uint32_t id);

	std::vector<TIMER_TEST> listTimer;
	std::vector<TRIGGER_STATUS> listTriggerStatus;
	bool m_bTriggerDisable;

// Implementation
protected:
	HICON m_hIcon;
	CPolicyLoader* loader;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	afx_msg void OnHelpAbout();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelChangeListPolicy();
	afx_msg void OnDblclkListTrigger();
	afx_msg void OnButtonAddPolicy();
	afx_msg void OnButtonDelPolicy();
	afx_msg void OnButtonAddTrigger();
	afx_msg void OnButtonDelTrigger();
	afx_msg void OnButtonMoveUp();
	afx_msg void OnButtonMoveDown();
	afx_msg void OnButtonEventAttackStart();
	afx_msg void OnButtonEventRandom();
	afx_msg void OnButtonEventKillPlayer();
	afx_msg void OnButtonEventDied();
	afx_msg void OnButtonEventEndPoint();
	afx_msg void OnButtonEventBorn();
	afx_msg void OnButtonEventAttackSkill();
	afx_msg void OnButtonRunTrigger();
	afx_msg void OnButtonStopTrigger();

private:
	bool isFileOpen;
	CListBox m_listPolicy;
	CListBox m_listTrigger;
	CEdit m_editRunLog;
	float m_fHp;
	CStatic m_statusMsg;
};
