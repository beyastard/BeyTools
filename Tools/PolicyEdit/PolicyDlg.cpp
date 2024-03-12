
// PolicyDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "PolicyEdit.h"
#include "PolicyDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	void SetHelpInfo(CString version, CString copyright)
	{
		m_strVersion = version;
		m_strCopyright = copyright;
	}

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

private:
	CString m_strVersion;
	CString m_strCopyright;
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
, m_strVersion(_T(""))
, m_strCopyright(_T(""))
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_STATIC_COPYRIGHT, m_strCopyright);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPolicyDlg dialog



CPolicyDlg::CPolicyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POLICYEDIT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	loader = new CPolicyLoader();
}

CPolicyDlg::~CPolicyDlg()
{
	if (loader)
		loader->Release();

	delete loader;
	loader = nullptr;
}

void CPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_POLICY, m_listPolicy);
	DDX_Control(pDX, IDC_LIST_TRIGGER, m_listTrigger);
	DDX_Control(pDX, IDC_STATIC_STATUS_MSG, m_statusMsg);
}

BOOL CPolicyDlg::PreTranslateMessage(MSG* pMsg)
{
	// Ignore the <ESC> keypress
	return (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) ? TRUE : CDialogEx::PreTranslateMessage(pMsg);
}

void CPolicyDlg::OnCancel()
{
	CDialogEx::OnCancel();
}

bool CPolicyDlg::InitApp()
{
	std::unique_ptr<wchar_t[]> szAppPath = GetFullCurrentDirectory();

	if (!szAppPath)
		return false;

	CString strAppPath(szAppPath.get());
	strAppPath.Replace(L"\\\\", L"\\");
	g_strWorkDir = szAppPath.get();

	return true;
}

std::unique_ptr<wchar_t[]> CPolicyDlg::GetFullCurrentDirectory()
{
	DWORD reqBufSize = GetCurrentDirectory(0, nullptr);

	// Use MAX_PATH for the initial allocation
	std::unique_ptr<wchar_t[]> szAppPath(new wchar_t[MAX_PATH]);

	// Check if the buffer size was sufficient
	if (reqBufSize == 0)
		return nullptr;

	// If the buffer wasn't large enough, reallocate and retry
	if (reqBufSize > MAX_PATH)
	{
		szAppPath.reset(new wchar_t[reqBufSize]);
		GetCurrentDirectory(reqBufSize, szAppPath.get());
	}
	else
		GetCurrentDirectory(MAX_PATH, szAppPath.get());

	return szAppPath;
}

BEGIN_MESSAGE_MAP(CPolicyDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_OPEN, &CPolicyDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVEAS, &CPolicyDlg::OnFileSaveAs)
	ON_COMMAND(ID_HELP_ABOUT, &CPolicyDlg::OnHelpAbout)
	ON_LBN_SELCHANGE(IDC_LIST_POLICY, &CPolicyDlg::OnSelChangeListPolicy)
END_MESSAGE_MAP()


// CPolicyDlg message handlers

CString CPolicyDlg::GetAppData(ProgramInfo info)
{
	CString data;

	// Get various information from your application's resources
	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (hResource)
	{
		HGLOBAL hGlobal = LoadResource(AfxGetResourceHandle(), hResource);
		if (hGlobal)
		{
			LPVOID pData = LockResource(hGlobal);
			if (pData)
			{
				DWORD* pTranslation;
				UINT translationLength;
				if (VerQueryValue(pData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pTranslation, &translationLength))
				{
					CString subBlock;
					switch (info)
					{
					case Version:
						subBlock.Format(_T("\\StringFileInfo\\%04x%04x\\ProductVersion"), LOWORD(*pTranslation), HIWORD(*pTranslation));
						break;
					case Copyright:
						subBlock.Format(_T("\\StringFileInfo\\%04x%04x\\LegalCopyright"), LOWORD(*pTranslation), HIWORD(*pTranslation));
						break;
					case Company:
						subBlock.Format(_T("\\StringFileInfo\\%04x%04x\\CompanyName"), LOWORD(*pTranslation), HIWORD(*pTranslation));
						break;
					case Product:
						subBlock.Format(_T("\\StringFileInfo\\%04x%04x\\ProductName"), LOWORD(*pTranslation), HIWORD(*pTranslation));
						break;
					}

					if (VerQueryValue(pData, subBlock, (LPVOID*)&pData, &translationLength))
						data = (LPCTSTR)pData;
				}
			}
		}
	}
	return data;
}

BOOL CPolicyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	if (!InitApp())
		return FALSE;

	CString title = GetAppData(Product) + " | " + GetAppData(Company) + " | version " + GetAppData(Version);
	SetWindowText(title);
	isFileOpen = false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPolicyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPolicyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPolicyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPolicyDlg::OnFileOpen()
{
	m_statusMsg.SetWindowText(L"Open file for input...");
	CFileDialog fileDialog(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		L"Policy Files (*.data)|*.data|All Files (*.*)|*.*||", this);

	if (fileDialog.DoModal() == IDOK)
	{
		CString filePath = fileDialog.GetPathName();
		if (loader)
		{
			loader->Release();
			delete loader;

			loader = new CPolicyLoader();
			m_listPolicy.ResetContent();
			m_listTrigger.ResetContent();
		}
		loader->Load(filePath);

		// populate the listbox		
		int32_t count = (int32_t)loader->GetPolicyManager()->GetPolicyNum();
		for (int32_t i = 0; i < count; ++i)
		{
			CString cstr;
			cstr.Format(L"[%d]  %d", i, loader->GetPolicyManager()->GetPolicy(i)->GetID());
			m_listPolicy.AddString(cstr);
		}

		isFileOpen = true;
		m_statusMsg.SetWindowText(L"Policy data file opened successfully!");
	}
	else
		m_statusMsg.SetWindowText(L"User cancelled file open!");
}


void CPolicyDlg::OnFileSaveAs()
{
	// TODO: Add your command handler code here
}


void CPolicyDlg::OnHelpAbout()
{
	CAboutDlg dlg;
	CString version = GetAppData(Product) + ", " + "version " + GetAppData(Version);
	CString copyright = GetAppData(Copyright);
	dlg.SetHelpInfo(version, copyright);
	dlg.DoModal();
}


void CPolicyDlg::OnSelChangeListPolicy()
{
	m_listTrigger.ResetContent();

	int32_t sel = m_listPolicy.GetCurSel();
	int32_t id = loader->GetPolicyManager()->GetPolicy(sel)->GetID();

	m_pCurrentPolicy = loader->GetPolicyManager()->GetPolicy(sel);

	CString cstr;
	CString pId;

	cstr.Format(L"Policy selected: [%d] %d", sel, id);
	m_statusMsg.SetWindowText(cstr);
	pId.Format(L"%d", id);

	int32_t count = loader->GetPolicyManager()->GetPolicy(sel)->GetTriggerPtrNum();
	for (int32_t i = 0; i < count; ++i)
	{
		CString triggerStr;
		uint32_t id = loader->GetPolicyManager()->GetPolicy(sel)->GetTriggerPtr(i)->GetID();
		CString triggerName(Convert::GB18030ToCStringW(
			loader->GetPolicyManager()->GetPolicy(sel)->GetTriggerPtr(i)->GetName().c_str()));
		triggerStr.Format(L"[%d] %s", id, triggerName.GetString());
		m_listTrigger.AddString(triggerStr);
	}
}
