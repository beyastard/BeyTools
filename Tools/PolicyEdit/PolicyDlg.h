
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

	std::wstring g_strWorkDir;
	CPolicyData* m_pCurrentPolicy;

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

// Implementation
protected:
	HICON m_hIcon;
	CPolicyLoader* loader;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CListBox m_listPolicy;
	CListBox m_listTrigger;
	CStatic m_statusMsg;

	bool isFileOpen;
public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	afx_msg void OnHelpAbout();
	afx_msg void OnSelChangeListPolicy();
};
