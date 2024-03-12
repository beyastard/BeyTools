
// PolicyDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "PolicyEdit.h"
#include "PolicyDlg.h"
#include "afxdialogex.h"
#include "PolicyType.h"
#include "PolicyTriggerDlg.h"
#include "PolicyOperationDlg.h"
#include "OperationParam.h"
#include "Convert.h"

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
	, m_fHp(100.0f)
	, m_nPolicyID(0)
	, m_pCurrentPolicy(nullptr)
	, m_bReadOnly(false)
	, m_bStartTest(false)
	, isFileOpen(false)
	, m_bSkipKillPlayer(false)
	, m_bSkipRandom(false)
	, m_bSkipStartAttack(false)
	, m_bSkipBorn(false)
	, m_bSkipDied(false)
	, m_bPathEndPoint(false)
	, m_bAttackBySkill(false)
	, m_bTriggerDisable(false)
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

CString CPolicyDlg::GetTriggerName(uint32_t id)
{
	if (m_pCurrentPolicy)
	{
		int32_t n = m_pCurrentPolicy->GetTriggerPtrNum();
		for (int32_t i = 0; i < n; ++i)
		{
			CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
			if (pData->GetID() == id)
				return Convert::GB18030ToCStringW(pData->GetName());
		}
	}
	return L"Wrong Trigger ID";
}

void CPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_POLICY, m_listPolicy);
	DDX_Control(pDX, IDC_LIST_TRIGGER, m_listTrigger);
	DDX_Control(pDX, IDC_EDIT_RUN_LOG, m_editRunLog);
	DDX_Text(pDX, IDC_EDIT_HP, m_fHp);
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
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST_POLICY, &CPolicyDlg::OnSelChangeListPolicy)
	ON_LBN_DBLCLK(IDC_LIST_TRIGGER, &CPolicyDlg::OnDblclkListTrigger)
	ON_BN_CLICKED(IDC_BUTTON_ADD_POLICY, &CPolicyDlg::OnButtonAddPolicy)
	ON_BN_CLICKED(IDC_BUTTON_DEL_POLICY, &CPolicyDlg::OnButtonDelPolicy)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TRIGGER, &CPolicyDlg::OnButtonAddTrigger)
	ON_BN_CLICKED(IDC_BUTTON_DEL_TRIGGER, &CPolicyDlg::OnButtonDelTrigger)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_UP, &CPolicyDlg::OnButtonMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_DOWN, &CPolicyDlg::OnButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_ATTACKSTART, &CPolicyDlg::OnButtonEventAttackStart)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_RANDOM, &CPolicyDlg::OnButtonEventRandom)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_KILLPLAYER, &CPolicyDlg::OnButtonEventKillPlayer)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_DIED, &CPolicyDlg::OnButtonEventDied)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_END_POINT, &CPolicyDlg::OnButtonEventEndPoint)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_BORN, &CPolicyDlg::OnButtonEventBorn)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_ATTACK_SKILL, &CPolicyDlg::OnButtonEventAttackSkill)
	ON_BN_CLICKED(IDC_BUTTON_RUN_TRIGGER, &CPolicyDlg::OnButtonRunTrigger)
	ON_BN_CLICKED(IDC_BUTTON_STOP_TRIGGER, &CPolicyDlg::OnButtonStopTrigger)
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

void CPolicyDlg::FreshTriggerList()
{
	if (m_pCurrentPolicy == nullptr)
		return;

	int32_t count = m_listTrigger.GetCount();
	int32_t sel = m_listTrigger.GetCurSel();

	for (int32_t i = 0; i < count; i++)
		m_listTrigger.DeleteString(0);

	int32_t n = m_pCurrentPolicy->GetTriggerPtrNum();
	for (int32_t i = 0; i < n; ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (!pData->IsRun())
		{
			int32_t index = m_listTrigger.AddString(Convert::GB18030ToCStringW(pData->GetName()));
			m_listTrigger.SetItemData(index, pData->GetID());
		}
	}

	if (sel != -1 && sel < m_listTrigger.GetCount())
		m_listTrigger.SetCurSel(sel);
}

bool CPolicyDlg::TraverselTree(void* pTree, CTriggerData* pTrigger)
{
	CTriggerData::_s_tree_item* temp = (CTriggerData::_s_tree_item*)pTree;
	bool bRes = false;
	bool bRes2 = false;
	switch ((CTriggerData::_e_condition)temp->mCondition.iType)
	{
	case CTriggerData::_e_condition::c_time_come:
		bRes = IsTimeCome(((C_TIME_COME*)temp->mCondition.pParam)->uID);
		break;
	case CTriggerData::_e_condition::c_hp_less:
		bRes = HpLess(((C_HP_LESS*)temp->mCondition.pParam)->fPercent);
		break;
	case CTriggerData::_e_condition::c_start_attack:
		return StartAttack();
		break;
	case CTriggerData::_e_condition::c_random:
		bRes = Random(((C_RANDOM*)temp->mCondition.pParam)->fProbability);
		break;
	case CTriggerData::_e_condition::c_kill_player:
		return KillPlayer();
	case CTriggerData::_e_condition::c_not:
		if (temp->pRight)
			bRes = TraverselTree(temp->pRight, pTrigger);
		return !bRes;
	case CTriggerData::_e_condition::c_or:
		if (temp->pLeft)
			bRes = TraverselTree(temp->pLeft, pTrigger);
		if (temp->pRight)
			bRes2 = TraverselTree(temp->pRight, pTrigger);
		return bRes || bRes2;
	case CTriggerData::_e_condition::c_and:
		if (temp->pLeft)
			bRes = TraverselTree(temp->pLeft, pTrigger);
		if (temp->pRight)
			bRes2 = TraverselTree(temp->pRight, pTrigger);
		return bRes && bRes2;
	case CTriggerData::_e_condition::c_died:
		return Died();
	case CTriggerData::_e_condition::c_path_end_point:
		bRes = IsPathEndPoint();
		break;
	case CTriggerData::_e_condition::c_enmity_reach:
		return true;
	case CTriggerData::_e_condition::c_distance_reach:
		return true;
	case CTriggerData::_e_condition::c_born:
		return Born();
	case CTriggerData::_e_condition::c_attack_by_skill:
		bRes = IsAttackBySkill();
		break;
	case CTriggerData::_e_condition::c_random_selectone:
		bRes = Random(((C_RANDOM_SELECTONE*)temp->mCondition.pParam)->fProbability);
		break;
	default:
		return bRes;
	}

	return bRes;
}

void CPolicyDlg::DelRedundancy()
{
	std::vector<unsigned> listId;
	int32_t n = m_pCurrentPolicy->GetTriggerPtrNum();

	for (int32_t i = 0; i < n; ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (pData->IsRun())
		{
			if (!TriggerIsUse(pData->GetID()))
				listId.push_back(pData->GetID());
		}
	}

	n = (int32_t)listId.size();
	for (int32_t i = 0; i < n; ++i)
	{
		int32_t idx = m_pCurrentPolicy->GetIndex(listId[i]);
		if (idx != -1)
			m_pCurrentPolicy->DelTriggerPtr(idx);
	}

	listId.clear();
}

bool CPolicyDlg::TriggerIsUse(uint32_t id)
{
	int32_t n = m_pCurrentPolicy->GetTriggerPtrNum();
	for (int32_t i = 0; i < n; ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (!pData->IsRun())
		{
			if (TraceTrigger(pData, id))
				return true;
		}
	}
	return false;
}

bool CPolicyDlg::TraceTrigger(CTriggerData* pTrigger, uint32_t id)
{
	if (pTrigger->GetID() == id)
		return true;

	int32_t num = pTrigger->GetOperationNum();
	for (int32_t j = 0; j < num; ++j)
	{
		CTriggerData::_s_operation* pOperation = pTrigger->GetOperation(j);
		if (pOperation->iType == (int32_t)CTriggerData::_e_operation::o_run_trigger)
		{
			int32_t idx = m_pCurrentPolicy->GetIndex(((O_RUN_TRIGGER*)pOperation->pParam)->uID);
			if (idx != -1)
			{
				CTriggerData* pNext = m_pCurrentPolicy->GetTriggerPtr(idx);
				if (pNext)
					if (TraceTrigger(pNext, id))
						return true;
			}
		}
	}
	return false;
}

bool CPolicyDlg::IsTimeCome(uint32_t iId)
{
	for (size_t i = 0; i < listTimer.size(); ++i)
	{
		if (iId == listTimer[i].id)
		{
			if (listTimer[i].counter != 0)
			{
				if (listTimer[i].skipCounter >= listTimer[i].counter)
				{
					m_bTriggerDisable = true;
					return false;
				}

				if (listTimer[i].last_time > listTimer[i].period)
				{
					listTimer[i].skipCounter++;
					listTimer[i].last_time = 0;
					return true;
				}
			}
			else
			{
				if (listTimer[i].last_time > listTimer[i].period)
				{
					listTimer[i].last_time = 0;
					return true;
				}
			}
		}
	}

	return false;
}

bool CPolicyDlg::HpLess(float fLess)
{
	UpdateData(true);
	if (m_fHp < fLess)
	{
		m_bTriggerDisable = true;
		return true;
	}
	else
		return false;
}

bool CPolicyDlg::KillPlayer()
{
	UpdateData(true);
	return m_bSkipKillPlayer;
}

bool CPolicyDlg::Born()
{
	UpdateData(true);
	return m_bSkipBorn;
}

bool CPolicyDlg::Died()
{
	UpdateData(true);
	return m_bSkipDied;
}

bool CPolicyDlg::Random(float fProbability)
{
	UpdateData(true);
	return m_bSkipRandom;
}

bool CPolicyDlg::StartAttack()
{
	UpdateData(true);
	return m_bSkipStartAttack;
}

bool CPolicyDlg::IsPathEndPoint()
{
	UpdateData(true);
	return m_bPathEndPoint;
}

bool CPolicyDlg::IsAttackBySkill()
{
	UpdateData(true);
	return m_bAttackBySkill;
}

void CPolicyDlg::AddLogMsg(const wchar_t* szMsg)
{
	wchar_t szBuf[128];
	_swprintf(szBuf, L"%s\n", szMsg);
	CString cstr(szBuf);

	// Select all text at first then to cancel selection. This is the way to
	// ensure caret is at the end of text when new text is added !!
	m_editRunLog.SetSel(0, -1);
	m_editRunLog.SetSel(-1, -1);
	m_editRunLog.ReplaceSel(cstr);

	SetFocus();
}

bool CPolicyDlg::RunOperation(void* pTriggerPtr, void* pOperation)
{
	CString msg;
	CTriggerData* pTrigger = (CTriggerData*)pTriggerPtr;
	CTriggerData::_s_operation* pTempOperation = (CTriggerData::_s_operation*)pOperation;

	void* pData = 0;
	char szTalk[MAX_PATH];

	switch ((CTriggerData::_e_operation)pTempOperation->iType)
	{
	case CTriggerData::_e_operation::o_attack:
		pData = (O_ATTACK_TYPE*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [ID(%d)]", strOperationList[pTempOperation->iType], ((O_ATTACK_TYPE*)pData)->uType);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_use_skill:
		pData = (O_USE_SKILL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [skill (%d), level (%d)]", strOperationList[pTempOperation->iType], ((O_USE_SKILL*)pData)->uSkill, ((O_USE_SKILL*)pData)->uLevel);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_talk:
		pData = (O_TALK_TEXT*)pTempOperation->pParam;
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)((O_TALK_TEXT*)pData)->szData, ((O_TALK_TEXT*)pData)->uSize / 2, szTalk, MAX_PATH, nullptr, FALSE);
		msg.Format(L"**Operation [%s] [Content (%s)]", strOperationList[pTempOperation->iType], szTalk);
		break;
	case CTriggerData::_e_operation::o_reset_hate_list:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		break;
	case CTriggerData::_e_operation::o_run_trigger:
		pData = (O_RUN_TRIGGER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [%s]", strOperationList[pTempOperation->iType], GetTriggerName(((O_RUN_TRIGGER*)pData)->uID));
		SetTriggerRunStatus(((O_ACTIVE_TRIGGER*)pData)->uID, true);
		break;
	case CTriggerData::_e_operation::o_stop_trigger:
		pData = (O_STOP_TRIGGER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [%s]", strOperationList[pTempOperation->iType], GetTriggerName(((O_STOP_TRIGGER*)pData)->uID));
		SetTriggerActiveStatus(((O_ACTIVE_TRIGGER*)pData)->uID, false);
		break;
	case CTriggerData::_e_operation::o_active_trigger:
		pData = (O_ACTIVE_TRIGGER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [%s]", strOperationList[pTempOperation->iType], GetTriggerName(((O_ACTIVE_TRIGGER*)pData)->uID));
		SetTriggerActiveStatus(((O_ACTIVE_TRIGGER*)pData)->uID, true);
		break;
	case CTriggerData::_e_operation::o_create_timer:
		pData = (O_CREATE_TIMER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [ID(%d) interval(%d), times(%d)]", strOperationList[pTempOperation->iType], ((O_CREATE_TIMER*)pData)->uID, ((O_CREATE_TIMER*)pData)->uPeriod, ((O_CREATE_TIMER*)pData)->uCounter);
		TIMER_TEST test;
		test.counter = 0;
		test.id = ((O_CREATE_TIMER*)pData)->uID;
		test.last_time = 0;
		test.skipCounter = 0;
		test.period = ((O_CREATE_TIMER*)pData)->uPeriod;
		test.counter = ((O_CREATE_TIMER*)pData)->uCounter;
		listTimer.push_back(test);
		break;
	case CTriggerData::_e_operation::o_kill_timer:
		pData = (O_KILL_TIMER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [ID(%d)]", strOperationList[pTempOperation->iType], ((O_KILL_TIMER*)pData)->uID);
		break;
	case CTriggerData::_e_operation::o_flee:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		break;
	case CTriggerData::_e_operation::o_set_hate_to_first:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_set_hate_to_last:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_skip_operation:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		AddLogMsg(msg);
		return false;
	case CTriggerData::_e_operation::o_active_controller:
		pData = (O_ACTIVE_CONTROLLER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [ID(%d)] [Deactivate(%d)]", strOperationList[pTempOperation->iType], ((O_ACTIVE_CONTROLLER*)pData)->uID, ((O_ACTIVE_CONTROLLER*)pData)->bStop);
		break;
	case CTriggerData::_e_operation::o_summon:
		pData = (O_SUMMON*)pTempOperation->pParam;
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)((O_SUMMON*)pData)->szName, 16, szTalk, MAX_PATH, nullptr, FALSE);
		msg.Format(L"**Operation [%s] [%s]", strOperationList[pTempOperation->iType], szTalk);
		break;
	case CTriggerData::_e_operation::o_trigger_task:
		pData = (O_TRIGGER_TASK*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [Task ID (%d)]", strOperationList[pTempOperation->iType], ((O_TRIGGER_TASK*)pData)->uTaskID);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_change_path:
		pData = (O_CHANGE_PATH*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [Path ID (%d) [Type (%d)]", strOperationList[pTempOperation->iType], ((O_CHANGE_PATH*)pData)->uPathID, ((O_CHANGE_PATH*)pData)->iType);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_disappear:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		break;
	case CTriggerData::_e_operation::o_sneer_monster:
		pData = (O_SNEER_MONSTER*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [Range (%4.2f)]", strOperationList[pTempOperation->iType], ((O_SNEER_MONSTER*)pData)->fRange);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_use_range_skill:
		pData = (O_USE_RANGE_SKILL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [skill (%d), level (%d), range (%g)]", strOperationList[pTempOperation->iType], ((O_USE_RANGE_SKILL*)pData)->uSkill, ((O_USE_RANGE_SKILL*)pData)->uLevel, ((O_USE_RANGE_SKILL*)pData)->fRange);
		msg += GetTargetStr(&pTempOperation->mTarget);
		break;
	case CTriggerData::_e_operation::o_reset:
		msg.Format(L"**Operation [%s]", strOperationList[pTempOperation->iType]);
		break;
	case CTriggerData::_e_operation::o_set_global:
		pData = (O_SET_GLOBAL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d, value=%d]", strOperationList[pTempOperation->iType], ((O_SET_GLOBAL*)pData)->iID, ((O_SET_GLOBAL*)pData)->iValue);
		break;
	case CTriggerData::_e_operation::o_revise_global:
		pData = (O_REVISE_GLOBAL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d, value=%d]", strOperationList[pTempOperation->iType], ((O_REVISE_GLOBAL*)pData)->iID, ((O_REVISE_GLOBAL*)pData)->iValue);
		break;
	case CTriggerData::_e_operation::o_assign_global:
		pData = (O_ASSIGN_GLOBAL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [iddst=%d, idsrc=%d]", strOperationList[pTempOperation->iType], ((O_ASSIGN_GLOBAL*)pData)->iIDDst, ((O_ASSIGN_GLOBAL*)pData)->iIDSrc);
		break;
	case CTriggerData::_e_operation::o_summon_mineral:
		pData = (O_SUMMON_MINERAL*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d]", strOperationList[pTempOperation->iType], ((O_SUMMON_MINERAL*)pData)->uMineralID);
		break;
	case CTriggerData::_e_operation::o_drop_item:
		pData = (O_DROP_ITEM*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d]", strOperationList[pTempOperation->iType], ((O_DROP_ITEM*)pData)->uItemID);
		break;
	case CTriggerData::_e_operation::o_change_hate:
		pData = (O_CHANGE_HATE*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [Change hatred value=%d]", strOperationList[pTempOperation->iType], ((O_CHANGE_HATE*)pData)->iHateValue);
		break;
	case CTriggerData::_e_operation::o_start_event:
		pData = (O_START_EVENT*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d]", strOperationList[pTempOperation->iType], ((O_START_EVENT*)pData)->iId);
		break;
	case CTriggerData::_e_operation::o_stop_event:
		pData = (O_STOP_EVENT*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d]", strOperationList[pTempOperation->iType], ((O_STOP_EVENT*)pData)->iId);
		break;
	case CTriggerData::_e_operation::o_drop_item_new:
		pData = (O_DROP_ITEMNEW*)pTempOperation->pParam;
		msg.Format(L"**Operation [%s] [id=%d]", strOperationList[pTempOperation->iType], ((O_DROP_ITEMNEW*)pData)->uItemID);
		break;
	case CTriggerData::_e_operation::o_whisper:
		pData = (O_WHISPER_TEXT*)pTempOperation->pParam;
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)((O_WHISPER_TEXT*)pData)->szData, ((O_WHISPER_TEXT*)pData)->uSize / 2, szTalk, MAX_PATH, nullptr, FALSE);
		msg.Format(L"**Operation [%s] [Content (%s)]", strOperationList[pTempOperation->iType], szTalk);
		break;
	case CTriggerData::_e_operation::o_talk_portrait:
		pData = (O_TALK_PORTRAIT*)pTempOperation->pParam;
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)((O_TALK_PORTRAIT*)pData)->szData, ((O_TALK_PORTRAIT*)pData)->uSize / 2, szTalk, MAX_PATH, nullptr, FALSE);
		msg.Format(L"**Operation [%s] [Content (%s)]", strOperationList[pTempOperation->iType], szTalk);
		break;
	default:
		break;
	}

	AddLogMsg(msg);

	return true;
}

void CPolicyDlg::RunTrigger(uint32_t id)
{
	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pTrigger = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id == pTrigger->GetID())
		{
			if (pTrigger->IsRun())
			{
				if (TraverselTree(pTrigger->GetConditionRoot(), pTrigger))
				{	//has been triggered
					CString msg;
					CString name = Convert::GB18030ToCStringW(pTrigger->GetName());
					msg.Format(L"==Run trigger [%s], perform the following operations: ", name.GetString());
					AddLogMsg((LPCTSTR)msg);
					for (int32_t k = 0; k < pTrigger->GetOperationNum(); k++)
						RunOperation(pTrigger, pTrigger->GetOperation(k)); // perform an operation
				}
				break;
			}
		}
	}
}

void CPolicyDlg::SetTriggerActiveStatus(uint32_t id, bool bActive)
{
	int32_t n = (int32_t)listTriggerStatus.size();
	for (int32_t i = 0; i < n; ++i)
	{
		if (listTriggerStatus[i].id == id)
			listTriggerStatus[i].bActive = bActive;
	}
}

void CPolicyDlg::SetTriggerRunStatus(uint32_t id, bool bRun)
{
	int32_t n = (int32_t)listTriggerStatus.size();
	for (int32_t i = 0; i < n; ++i)
	{
		if (listTriggerStatus[i].id == id)
			listTriggerStatus[i].bRun = bRun;
	}
}

bool CPolicyDlg::GetTriggerActiveStatus(uint32_t id)
{
	int32_t n = (int32_t)listTriggerStatus.size();
	for (int32_t i = 0; i < n; ++i)
	{
		if (listTriggerStatus[i].id == id)
			return listTriggerStatus[i].bActive;
	}
	return false;
}

bool CPolicyDlg::GetTriggerRunStatus(uint32_t id)
{
	int32_t n = (int32_t)listTriggerStatus.size();
	for (int32_t i = 0; i < n; ++i)
	{
		if (listTriggerStatus[i].id == id)
			return listTriggerStatus[i].bRun;
	}
	return false;
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
	SetTimer(0, 1000, NULL);

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


void CPolicyDlg::OnTimer(UINT_PTR nIDEvent)
{
	int32_t m = (int32_t)listTimer.size();
	for (int32_t h = 0; h < m; ++h)
	{
		if (listTimer[h].skipCounter <= listTimer[h].counter)
			listTimer[h].last_time += 1;
	}

	CString msg;
	if (m_pCurrentPolicy && m_bStartTest)
	{
		for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
		{
			m_bTriggerDisable = false;
			CTriggerData* pTrigger = m_pCurrentPolicy->GetTriggerPtr(i);
			if (!pTrigger->IsRun())
			{//Simulation event trigger table
				if (GetTriggerActiveStatus(pTrigger->GetID()))
				{
					if (TraverselTree(pTrigger->GetConditionRoot(), pTrigger))
					{
						if (m_bTriggerDisable) //has been triggered
							SetTriggerActiveStatus(pTrigger->GetID(), false);

						CString name = Convert::GB18030ToCStringW(pTrigger->GetName());
						msg.Format(L"==Run trigger [%s], perform the following operations: ", name.GetString());
						AddLogMsg((LPCTSTR)msg);

						for (int32_t k = 0; k < pTrigger->GetOperationNum(); k++)
						{
							if (!RunOperation(pTrigger, pTrigger->GetOperation(k))) //perform an operation
								break;
						}
					}
				}
			}
			else
			{//Simulation execution trigger table,
				if (GetTriggerActiveStatus(pTrigger->GetID()) && GetTriggerRunStatus(pTrigger->GetID()))
				{
					if (TraverselTree(pTrigger->GetConditionRoot(), pTrigger))
					{
						if (m_bTriggerDisable) //has been triggered
							SetTriggerActiveStatus(pTrigger->GetID(), false);

						CString name = Convert::GB18030ToCStringW(pTrigger->GetName());
						msg.Format(L"==Run trigger [%s], perform the following operations: ", name.GetString());
						AddLogMsg((LPCTSTR)msg);

						for (int32_t k = 0; k < pTrigger->GetOperationNum(); k++)
						{
							if (!RunOperation(pTrigger, pTrigger->GetOperation(k))) //perform an operation
								break;
						}
					}
				}
			}
		}
	}

	//reset event flag
	m_bSkipKillPlayer = false;
	m_bSkipRandom = false;
	m_bSkipStartAttack = false;
	m_bSkipBorn = false;
	m_bSkipDied = false;
	m_bPathEndPoint = false;
	m_bAttackBySkill = false;

	CDialogEx::OnTimer(nIDEvent);
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
		CString triggerName(Convert::GB18030ToCStringW(loader->GetPolicyManager()->GetPolicy(sel)->GetTriggerPtr(i)->GetName()));
		triggerStr.Format(L"[%d] %s", id, triggerName.GetString());
		m_listTrigger.AddString(triggerStr);
	}
}


void CPolicyDlg::OnDblclkListTrigger()
{
	OnButtonStopTrigger();

	if (m_pCurrentPolicy == nullptr || m_bStartTest)
		return;

	int32_t sel = m_listTrigger.GetCurSel();
	if (sel == -1)
		return;

	DWORD_PTR id = m_listTrigger.GetItemData(sel);
	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id == pData->GetID())
		{
			CPolicyTriggerDlg dlg;
			dlg.m_pCurrentPolicy = m_pCurrentPolicy;
			dlg.m_bModified = true;
			dlg.m_pTriggerData = pData;
			if (IDOK == dlg.DoModal())
			{
				if (dlg.m_bIsChanged)
					g_bPolicyModified = true;

				FreshTriggerList();
			}

			break;
		}
	}

	DelRedundancy();
}


void CPolicyDlg::OnButtonAddPolicy()
{
	// TODO: Add your control notification handler code here
}


void CPolicyDlg::OnButtonDelPolicy()
{
	// TODO: Add your control notification handler code here
}


void CPolicyDlg::OnButtonAddTrigger()
{
	OnButtonStopTrigger();

	if (m_pCurrentPolicy == nullptr || m_bStartTest)
		return;

	CPolicyTriggerDlg dlg;
	dlg.m_pCurrentPolicy = m_pCurrentPolicy;
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_pTriggerData)
		{
			dlg.m_pTriggerData->SetRunStatus(false);
			m_pCurrentPolicy->AddTriggerPtr(dlg.m_pTriggerData);
		}

		FreshTriggerList();
		g_bPolicyModified = true;
	}

	DelRedundancy();
}


void CPolicyDlg::OnButtonDelTrigger()
{
	OnButtonStopTrigger();

	if (m_pCurrentPolicy == nullptr || m_bStartTest)
		return;

	int32_t sel = m_listTrigger.GetCurSel();
	if (sel == -1)
		return;

	DWORD_PTR id = m_listTrigger.GetItemData(sel);
	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id == pData->GetID())
		{
			if (IDYES == AfxMessageBox(L"Are you sure you want to delete the current trigger?", MB_YESNO | MB_ICONQUESTION))
			{
				m_pCurrentPolicy->DelTriggerPtr(i);
				DelRedundancy();
			}

			break;
		}
	}

	FreshTriggerList();
	g_bPolicyModified = true;
}


void CPolicyDlg::OnButtonMoveUp()
{
	OnButtonStopTrigger();

	if (m_pCurrentPolicy == nullptr || m_bStartTest)
		return;

	int32_t sel = m_listTrigger.GetCurSel();
	if (sel == -1 || sel == 0)
		return;

	void* pData1 = nullptr;
	void* pData2 = nullptr;
	DWORD_PTR id1 = m_listTrigger.GetItemData(sel);
	DWORD_PTR id2 = m_listTrigger.GetItemData(sel - 1);
	int32_t idx1 = 0;
	int32_t idx2 = 0;

	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id1 == pData->GetID())
		{
			pData1 = m_pCurrentPolicy->GetTriggerPtr(i);
			idx1 = i;
			break;
		}
	}

	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id2 == pData->GetID())
		{
			pData2 = m_pCurrentPolicy->GetTriggerPtr(i);
			idx2 = i;
			break;
		}
	}

	m_pCurrentPolicy->SetTriggerPtr(idx1, (CTriggerData*)pData2);
	m_pCurrentPolicy->SetTriggerPtr(idx2, (CTriggerData*)pData1);
	FreshTriggerList();
	m_listTrigger.SetCurSel(sel - 1);
	g_bPolicyModified = true;
}


void CPolicyDlg::OnButtonMoveDown()
{
	OnButtonStopTrigger();

	if (m_pCurrentPolicy == nullptr || m_bStartTest)
		return;

	int32_t sel = m_listTrigger.GetCurSel();
	if (sel == -1 || sel > m_listTrigger.GetCount() - 2)
		return;

	void* pData1 = nullptr;
	void* pData2 = nullptr;
	DWORD_PTR id1 = m_listTrigger.GetItemData(sel);
	DWORD_PTR id2 = m_listTrigger.GetItemData(sel + 1);
	int32_t idx1 = 0;
	int32_t idx2 = 0;

	for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id1 == pData->GetID())
		{
			pData1 = m_pCurrentPolicy->GetTriggerPtr(i);
			idx1 = i;
			break;
		}
	}

	for (int32_t i = 0; m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
		if (id2 == pData->GetID())
		{
			pData2 = m_pCurrentPolicy->GetTriggerPtr(i);
			idx2 = i;
			break;
		}
	}

	m_pCurrentPolicy->SetTriggerPtr(idx1, (CTriggerData*)pData2);
	m_pCurrentPolicy->SetTriggerPtr(idx2, (CTriggerData*)pData1);
	FreshTriggerList();
	m_listTrigger.SetCurSel(sel + 1);
	g_bPolicyModified = true;
}


void CPolicyDlg::OnButtonEventAttackStart()
{
	m_bSkipStartAttack = true;
}


void CPolicyDlg::OnButtonEventRandom()
{
	m_bSkipRandom = true;
}


void CPolicyDlg::OnButtonEventKillPlayer()
{
	m_bSkipKillPlayer = true;
}


void CPolicyDlg::OnButtonEventDied()
{
	m_bSkipDied = true;
}


void CPolicyDlg::OnButtonEventEndPoint()
{
	m_bPathEndPoint = true;
}


void CPolicyDlg::OnButtonEventBorn()
{
	m_bSkipBorn = true;
}


void CPolicyDlg::OnButtonEventAttackSkill()
{
	m_bAttackBySkill = true;
}


void CPolicyDlg::OnButtonRunTrigger()
{
	CWnd* pWnd = nullptr;
	if (m_pCurrentPolicy && !m_bStartTest)
	{
		m_bStartTest = true;
		m_editRunLog.SetSel(0, -1);
		m_editRunLog.ReplaceSel(L"");
		AddLogMsg(L"The test run for the current strategy has started ...");

		CWnd* pWnd = GetDlgItem(IDC_BUTTON_RUN_TRIGGER);
		if (pWnd)
			pWnd->EnableWindow(false);

		pWnd = GetDlgItem(IDC_BUTTON_STOP_TRIGGER);
		if (pWnd)
			pWnd->EnableWindow(true);

		listTriggerStatus.clear();

		//Get the state save of the departure trigger
		for (int32_t i = 0; i < m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
		{
			CTriggerData* pTrigger = m_pCurrentPolicy->GetTriggerPtr(i);
			TRIGGER_STATUS status;
			status.bActive = pTrigger->IsActive();
			status.id = pTrigger->GetID();
			status.bRun = false;
			listTriggerStatus.push_back(status);
		}
	}
}


void CPolicyDlg::OnButtonStopTrigger()
{
	if (m_pCurrentPolicy && m_bStartTest)
	{
		m_bStartTest = false;
		listTimer.clear();

		CWnd* pWnd = GetDlgItem(IDC_BUTTON_RUN_TRIGGER);
		if (pWnd)
			pWnd->EnableWindow(true);

		pWnd = GetDlgItem(IDC_BUTTON_STOP_TRIGGER);
		if (pWnd)
			pWnd->EnableWindow(false);
	}
}
