// PolicyTriggerDlg.cpp : implementation file
//

#include "pch.h"
#include "PolicyEdit.h"
#include "afxdialogex.h"
#include "PolicyTriggerDlg.h"
#include "PolicyOperationDlg.h"
#include "ConditionDlg.h"
#include "PolicyDlg.h"
#include "OperationParam.h"
#include "Convert.h"

// CPolicyTriggerDlg dialog

IMPLEMENT_DYNAMIC(CPolicyTriggerDlg, CDialogEx)

CPolicyTriggerDlg::CPolicyTriggerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_POLICY_TRIGGER, pParent)
	, m_strTriggerName(_T(""))
	, m_uTriggerID(0)
	, m_bActive(false)
	, m_strProgram(_T(""))
	, m_pTriggerData(nullptr)
	, m_pCurrentPolicy(nullptr)
	, m_bModified(false)
	, m_dwTriggerID(0)
	, m_bIsChanged(false)
	, m_RunCondition(CTriggerData::_run_condition::run_attack_effect)
	, m_pOldTree(nullptr)
{}

CPolicyTriggerDlg::~CPolicyTriggerDlg() {}

CString CPolicyTriggerDlg::TraverselTree(void* pTree)
{
	CTriggerData::_s_tree_item* temp = (CTriggerData::_s_tree_item*)pTree;
	if (temp == nullptr)
		return L"";

	CString str;
	CString sign;
	switch ((CTriggerData::_e_condition)temp->mCondition.iType)
	{
	case CTriggerData::_e_condition::c_time_come:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_TIME_COME*)temp->mCondition.pParam)->uID);
		break;
	case CTriggerData::_e_condition::c_hp_less:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_HP_LESS*)temp->mCondition.pParam)->fPercent);
		break;
	case CTriggerData::_e_condition::c_random:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_RANDOM*)temp->mCondition.pParam)->fProbability);
		break;
	case CTriggerData::_e_condition::c_path_end_point:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_PATH_END_POINT*)temp->mCondition.pParam)->uID);
		break;
	case CTriggerData::_e_condition::c_enmity_reach:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_ENMITY_REACH*)temp->mCondition.pParam)->nPlayer, ((C_ENMITY_REACH*)temp->mCondition.pParam)->nPlayerLevel);
		break;
	case CTriggerData::_e_condition::c_distance_reach:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_DISTANCE_REACH*)temp->mCondition.pParam)->fDistance);
		break;
	case CTriggerData::_e_condition::c_var:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_VAR*)temp->mCondition.pParam)->iID);
		break;
	case CTriggerData::_e_condition::c_constant:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_CONSTANT*)temp->mCondition.pParam)->iValue);
		break;
	case CTriggerData::_e_condition::c_rank_level:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_RANK_LEVEL*)temp->mCondition.pParam)->iValue);
		break;
	case CTriggerData::_e_condition::c_attack_by_skill:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_ATTACK_BY_SKILL*)temp->mCondition.pParam)->uID);
		break;
	case CTriggerData::_e_condition::c_random_selectone:
		sign.Format(strConditionSign[temp->mCondition.iType], ((C_RANDOM_SELECTONE*)temp->mCondition.pParam)->fProbability);
		break;
	default:
		sign = strConditionSign[temp->mCondition.iType];
	}

	str += L"(";

	if (temp->pLeft)
		str += TraverselTree(temp->pLeft);

	str += sign;
	if (temp->pRight)
		str += TraverselTree(temp->pRight);

	str += L")";

	return str;
}

CString CPolicyTriggerDlg::GetTriggerName(uint32_t id)
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

void CPolicyTriggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TRIGGER_NAME, m_strTriggerName);
	DDX_Text(pDX, IDC_EDIT_TRIGGER_ID, m_uTriggerID);
	DDX_Check(pDX, IDC_CHECK_ACTIVE, m_bActive);
	DDX_Text(pDX, IDC_EDIT_PROGRAM, m_strProgram);
	DDX_Control(pDX, IDC_LISTBOX_OPERATION, m_listOperation);
}

void CPolicyTriggerDlg::OnCancel()
{
	if (m_bModified)
	{
		RestoreTrigger(m_pOldTree);
		if (m_pOldTree)
		{
			delete m_pOldTree;
			m_pOldTree = nullptr;
		}
	}
	else
	{
		if (m_pTriggerData)
		{
			m_pTriggerData->Release();
			delete m_pTriggerData;
			m_pTriggerData = nullptr;
		}
	}
	CDialogEx::OnCancel();
}

void CPolicyTriggerDlg::FreshList()
{
	int32_t count = m_listOperation.GetCount();
	for (int32_t i = 0; i < count; i++)
		m_listOperation.DeleteString(0);

	if (m_pTriggerData == nullptr)
		return;

	int32_t n = m_pTriggerData->GetOperationNum();
	uint32_t iType;
	CString strCommand;
	CString strParam;
	CTriggerData::_s_operation* pOperation = 0;
	void* pData;
	for (int32_t i = 0; i < n; i++)
	{
		pOperation = m_pTriggerData->GetOperation(i);
		pData = pOperation->pParam;
		iType = pOperation->iType;
		strCommand = strOperationList[iType];
		strCommand += L" ";
		strParam = L"";
		switch ((CTriggerData::_e_operation)iType)
		{
		case CTriggerData::_e_operation::o_attack:
			switch (((O_ATTACK_TYPE*)pData)->uType)
			{
			case COperationParam_Attack::a_close_battle:
				strParam = L"[Physical Melee]";
				break;
			case COperationParam_Attack::a_skill:
				strParam = L"[Physical Ranged]";
				break;
			case COperationParam_Attack::a_long_distance:
				strParam = L"[magic]";
				break;
			case COperationParam_Attack::a_skill_and_battle:
				strParam = L"[Melee + Ranged]";
				break;
			}
			break;
		case CTriggerData::_e_operation::o_use_skill:
			strParam.Format(L"[Skill %d] [Level %d]", ((O_USE_SKILL*)pData)->uSkill, ((O_USE_SKILL*)pData)->uLevel);
			break;
		case CTriggerData::_e_operation::o_talk:
			//((O_TALK_TEXT *)pTemp)->szBuffer
			strParam.Format(L"[content %s]", L"...");
			break;
		case CTriggerData::_e_operation::o_reset_hate_list:
			break;
		case CTriggerData::_e_operation::o_run_trigger:
			strParam.Format(L"[Trigger (%s)]", GetTriggerName(((O_RUN_TRIGGER*)pData)->uID).GetString());
			break;
		case CTriggerData::_e_operation::o_stop_trigger:
			strParam.Format(L"[Trigger (%s)]", GetTriggerName(((O_STOP_TRIGGER*)pData)->uID).GetString());
			break;
		case CTriggerData::_e_operation::o_active_trigger:
			strParam.Format(L"[Trigger (%s)]", GetTriggerName(((O_ACTIVE_TRIGGER*)pData)->uID).GetString());
			break;
		case CTriggerData::_e_operation::o_create_timer:
			strParam.Format(L"[Timer (%d)] [Interval %d] [Time %d]", ((O_CREATE_TIMER*)pData)->uID, ((O_CREATE_TIMER*)pData)->uPeriod, ((O_CREATE_TIMER*)pData)->uCounter);
			break;
		case CTriggerData::_e_operation::o_kill_timer:
			strParam.Format(L"[Timer (%d)]", ((O_KILL_TIMER*)pData)->uID);
			break;
		case CTriggerData::_e_operation::o_flee:
		case CTriggerData::_e_operation::o_set_hate_to_first:
		case CTriggerData::_e_operation::o_set_hate_to_last:
		case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		case CTriggerData::_e_operation::o_skip_operation:
			break;
		case CTriggerData::_e_operation::o_active_controller:
			strParam.Format(L"[Spawn Monster Controller (%d)] [Stop Activation (%d)]", ((O_ACTIVE_CONTROLLER*)pData)->uID, ((O_ACTIVE_CONTROLLER*)pData)->bStop);
			break;
		case CTriggerData::_e_operation::o_summon:
			char szText[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, (WCHAR*)((O_SUMMON*)pData)->szName, 16, szText, MAX_PATH, nullptr, FALSE);
			strParam.Format(L"[(%s)]", Convert::GB18030ToWString(szText));
			break;
		case CTriggerData::_e_operation::o_trigger_task:
			strParam.Format(L"[Task ID (%d)]", ((O_TRIGGER_TASK*)pData)->uTaskID);
			break;
		case CTriggerData::_e_operation::o_change_path:
			strParam.Format(L"[Path ID(%d)[Type ID(%d)]", ((O_CHANGE_PATH*)pData)->uPathID, ((O_CHANGE_PATH*)pData)->iType);
			break;
		case CTriggerData::_e_operation::o_disappear:
			break;
		case CTriggerData::_e_operation::o_sneer_monster:
			strParam.Format(L"[Range (%4.2f)]", ((O_SNEER_MONSTER*)pData)->fRange);
			break;
		case CTriggerData::_e_operation::o_use_range_skill:
			strParam.Format(L"[Skill %d] [Level %d] [Range %g]", ((O_USE_RANGE_SKILL*)pData)->uSkill, ((O_USE_RANGE_SKILL*)pData)->uLevel, ((O_USE_RANGE_SKILL*)pData)->fRange);
			break;
		case CTriggerData::_e_operation::o_reset:
			break;
		case CTriggerData::_e_operation::o_set_global:
			strParam.Format(L"[(id=%d)(value=%d)]", ((O_SET_GLOBAL*)pData)->iID, ((O_SET_GLOBAL*)pData)->iValue);
			break;
		case CTriggerData::_e_operation::o_revise_global:
			strParam.Format(L"[(id=%d)(value=%d)]", ((O_REVISE_GLOBAL*)pData)->iID, ((O_REVISE_GLOBAL*)pData)->iValue);
			break;
		case CTriggerData::_e_operation::o_assign_global:
			strParam.Format(L"[(iddst=%d)(idsrc=%d)]", ((O_ASSIGN_GLOBAL*)pData)->iIDDst, ((O_ASSIGN_GLOBAL*)pData)->iIDSrc);
			break;
		case CTriggerData::_e_operation::o_summon_mineral:
			strParam.Format(L"[(id=%d)]", ((O_SUMMON_MINERAL*)pData)->uMineralID);
			break;
		case CTriggerData::_e_operation::o_drop_item:
			strParam.Format(L"[(id=%d)]", ((O_DROP_ITEM*)pData)->uItemID);
			break;
		case CTriggerData::_e_operation::o_change_hate:
			strParam.Format(L"[(Change hatred value=%d)]", ((O_CHANGE_HATE*)pData)->iHateValue);
			break;
		case CTriggerData::_e_operation::o_start_event:
			strParam.Format(L"[(id=%d)]", ((O_START_EVENT*)pData)->iId);
			break;
		case CTriggerData::_e_operation::o_stop_event:
			strParam.Format(L"[(id=%d)]", ((O_STOP_EVENT*)pData)->iId);
			break;
		case CTriggerData::_e_operation::o_drop_item_new:
			strParam.Format(L"[(id=%d)]", ((O_DROP_ITEMNEW*)pData)->uItemID);
			break;
		case CTriggerData::_e_operation::o_whisper:
			//((O_WHISPER_TEXT *)pTemp)->szBuffer
			strParam.Format(L"[content %s]", L"...");
			break;
		case CTriggerData::_e_operation::o_talk_portrait:
			//((O_TALK_PORTRAIT *)pTemp)->szBuffer
			strParam.Format(L"[content %s]", L"...");
			break;
		default:
			MessageBox(L"Unrecognized operation type!");
		}

		strParam += GetTargetStr(&pOperation->mTarget);
		m_listOperation.AddString(strCommand + strParam);
	}
}

void CPolicyTriggerDlg::SaveOldTree(TRIGGER_TREE* pTreeRoot, CTriggerData* pTrigger)
{
	if (pTreeRoot == 0 || pTrigger == 0)
		return;

	pTreeRoot->pTrigger = pTrigger;
	int32_t n = pTrigger->GetOperationNum();
	for (int32_t i = 0; i < n; ++i)
	{
		CTriggerData::_s_operation* pOperation = pTrigger->GetOperation(i);
		if (pOperation->iType == (int32_t)CTriggerData::_e_operation::o_run_trigger)
		{
			int32_t index = m_pCurrentPolicy->GetIndex(((O_RUN_TRIGGER*)pOperation->pParam)->uID);
			if (index != -1)
			{
				CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(index);
				CTriggerData* pNewData = pData->CopyObject();
				ASSERT(pNewData);
				TRIGGER_TREE* pNewTree = new TRIGGER_TREE;
				ASSERT(pNewTree);
				pTreeRoot->listChild.push_back(pNewTree);
				SaveOldTree(pNewTree, pNewData);
			}
			else
				pTreeRoot->listChild.push_back(0);
		}
	}
}

void CPolicyTriggerDlg::StartRecord()
{
	m_pOldTree = new TRIGGER_TREE;
	if (m_pOldTree == nullptr)
		return;

	CTriggerData* pNewData = m_pTriggerData->CopyObject();
	ASSERT(pNewData);
	SaveOldTree(m_pOldTree, pNewData);
}

void CPolicyTriggerDlg::RestoreTrigger(TRIGGER_TREE* pTreeRoot)
{
	if (pTreeRoot == 0)
		return;

	int32_t index = m_pCurrentPolicy->GetIndex(pTreeRoot->pTrigger->GetID());
	if (index != -1)
	{
		CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(index);
		if (pData)
		{
			pData->Release();
			delete pData;
			pData = nullptr;
		}
		m_pCurrentPolicy->SetTriggerPtr(index, pTreeRoot->pTrigger->CopyObject());
	}

	int32_t n = pTreeRoot->pTrigger->GetOperationNum();
	int32_t k = 0;
	for (int32_t i = 0; i < n; ++i)
	{
		CTriggerData::_s_operation* pOperation = pTreeRoot->pTrigger->GetOperation(i);
		if (pOperation->iType == (int32_t)CTriggerData::_e_operation::o_run_trigger)
			RestoreTrigger(pTreeRoot->listChild[k++]);
	}
}


BEGIN_MESSAGE_MAP(CPolicyTriggerDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_TRIGGER_NAME, &CPolicyTriggerDlg::OnChangeEditTriggerName)
	ON_BN_CLICKED(IDC_CHECK_ACTIVE, &CPolicyTriggerDlg::OnCheckActive)
	ON_BN_CLICKED(IDC_RADIO_ATTACK_EFFECT, &CPolicyTriggerDlg::OnRadioAttackEffect)
	ON_BN_CLICKED(IDC_RADIO_ATTACK_NOEFFECT, &CPolicyTriggerDlg::OnRadioAttackNoEffect)
	ON_BN_CLICKED(IDC_ADD_CONDITION, &CPolicyTriggerDlg::OnAddCondition)
	ON_BN_CLICKED(IDC_ADD_OPERATION, &CPolicyTriggerDlg::OnAddOperation)
	ON_BN_CLICKED(IDC_DEL_OPERATION, &CPolicyTriggerDlg::OnDelOperation)
	ON_BN_CLICKED(IDC_MOVE_UP, &CPolicyTriggerDlg::OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, &CPolicyTriggerDlg::OnMoveDown)
	ON_BN_CLICKED(ID_MODIFY, &CPolicyTriggerDlg::OnModify)
	ON_LBN_DBLCLK(IDC_LISTBOX_OPERATION, &CPolicyTriggerDlg::OnDblclkListboxOperation)
END_MESSAGE_MAP()


// CPolicyTriggerDlg message handlers


BOOL CPolicyTriggerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (!m_bModified)
	{
		CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_ATTACK_EFFECT);
		pButton->SetCheck(true);
		m_RunCondition = CTriggerData::_run_condition::run_attack_effect;
		m_pTriggerData = new CTriggerData;
	}
	else
	{
		CWnd* pWnd = GetDlgItem(IDC_EDIT_TRIGGER_NAME);
		m_strTriggerName = Convert::GB18030ToCStringW(m_pTriggerData->GetName());
		//pWnd->EnableWindow(false);
		FreshList();
		m_bActive = m_pTriggerData->IsActive();
		m_RunCondition = m_pTriggerData->GetRunCondition();

		CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_ATTACK_EFFECT);
		if (m_RunCondition == CTriggerData::_run_condition::run_attack_effect)
			pButton->SetCheck(true);
		else
			pButton->SetCheck(false);

		pButton = (CButton*)GetDlgItem(IDC_RADIO_ATTACK_NOEFFECT);
		if (m_RunCondition == CTriggerData::_run_condition::run_attack_noeffect)
			pButton->SetCheck(true);
		else
			pButton->SetCheck(false);

		CTriggerData::_s_tree_item* root = m_pTriggerData->GetConditionRoot();
		m_strProgram = TraverselTree(root);
		m_uTriggerID = m_pTriggerData->GetID();

		//Save the original trigger data, which is easy to cancel and restore later
		StartRecord();

		UpdateData(false);
	}

	return true;
}

void CPolicyTriggerDlg::OnChangeEditTriggerName()
{
	UpdateData(true);
}


void CPolicyTriggerDlg::OnCheckActive()
{
	m_bIsChanged = true;
	UpdateData(TRUE);
}


void CPolicyTriggerDlg::OnRadioAttackEffect()
{
	m_RunCondition = CTriggerData::_run_condition::run_attack_effect;
}


void CPolicyTriggerDlg::OnRadioAttackNoEffect()
{
	m_RunCondition = CTriggerData::_run_condition::run_attack_noeffect;
}


void CPolicyTriggerDlg::OnAddCondition()
{
	CTriggerData::_s_tree_item* root = m_pTriggerData->GetConditionRoot();
	CTriggerData::_s_tree_item* old = nullptr;
	if (root == nullptr)
		root = new CTriggerData::_s_tree_item;
	else
		old = CTriggerData::CopyConditionTree(root);

	CConditionDlg dlg;
	dlg.m_pRoot = root;
	dlg.m_bIsRoot = true;

	if (dlg.DoModal() == IDOK)
	{
		m_pTriggerData->SetConditionRoot(root);
		m_strProgram = TraverselTree(root);
		m_bIsChanged = true;
		if (old)
		{
			delete old;
			old = nullptr;
		}
	}
	else
	{
		delete root;
		root = nullptr;

		m_pTriggerData->SetConditionRoot(old);
		m_strProgram = TraverselTree(old);
	}

	UpdateData(false);
}


void CPolicyTriggerDlg::OnAddOperation()
{
	CPolicyOperationDlg dlg;
	dlg.m_pPolicyData = m_pCurrentPolicy;
	if (dlg.DoModal() == IDOK)
	{
		void* pTemp = nullptr;
		COperationParam* pParam = dlg.GetNewOperation();
		switch ((CTriggerData::_e_operation)pParam->GetOperationType())
		{
		case CTriggerData::_e_operation::o_attack:
			pTemp = malloc(sizeof(O_ATTACK_TYPE));
			assert(pTemp != nullptr);
			((O_ATTACK_TYPE*)pTemp)->uType = ((COperationParam_Attack*)pParam)->m_Data.uType;
			break;
		case CTriggerData::_e_operation::o_use_skill:
			pTemp = malloc(sizeof(O_USE_SKILL));
			assert(pTemp != nullptr);
			((O_USE_SKILL*)pTemp)->uSkill = ((COperationParam_Use_Skill*)pParam)->m_Data.uSkill;
			((O_USE_SKILL*)pTemp)->uLevel = ((COperationParam_Use_Skill*)pParam)->m_Data.uLevel;
			break;
		case CTriggerData::_e_operation::o_talk:
			pTemp = malloc(sizeof(O_TALK_TEXT));
			assert(pTemp != nullptr);
			((O_TALK_TEXT*)pTemp)->uSize = (int32_t)wcslen((WCHAR*)((COperationParam_Talk*)pParam)->m_Data.szData) * 2 + 2;
			((O_TALK_TEXT*)pTemp)->szData = new uint16_t[((O_TALK_TEXT*)pTemp)->uSize];
			wcscpy((WCHAR*)((O_TALK_TEXT*)pTemp)->szData, (WCHAR*)((COperationParam_Talk*)pParam)->m_Data.szData);
			break;
		case CTriggerData::_e_operation::o_reset_hate_list:
			break;
		case CTriggerData::_e_operation::o_run_trigger:
			pTemp = malloc(sizeof(O_RUN_TRIGGER));
			((O_RUN_TRIGGER*)pTemp)->uID = ((COperationParam_Run_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_stop_trigger:
			pTemp = malloc(sizeof(O_STOP_TRIGGER));
			((O_STOP_TRIGGER*)pTemp)->uID = ((COperationParam_Stop_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_active_trigger:
			pTemp = malloc(sizeof(O_ACTIVE_TRIGGER));
			((O_ACTIVE_TRIGGER*)pTemp)->uID = ((COperationParam_Active_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_create_timer:
			pTemp = malloc(sizeof(O_CREATE_TIMER));
			*((O_CREATE_TIMER*)pTemp) = ((COperationParam_Create_Timer*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_kill_timer:
			pTemp = malloc(sizeof(O_KILL_TIMER));
			((O_KILL_TIMER*)pTemp)->uID = ((COperationParam_Kill_Timer*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_flee:
		case CTriggerData::_e_operation::o_set_hate_to_first:
		case CTriggerData::_e_operation::o_set_hate_to_last:
		case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		case CTriggerData::_e_operation::o_skip_operation:
			break;
		case CTriggerData::_e_operation::o_active_controller:
			pTemp = malloc(sizeof(O_ACTIVE_CONTROLLER));
			((O_ACTIVE_CONTROLLER*)pTemp)->uID = ((COperationParam_Active_Controller*)pParam)->m_Data.uID;
			((O_ACTIVE_CONTROLLER*)pTemp)->bStop = ((COperationParam_Active_Controller*)pParam)->m_Data.bStop;
			break;
		case CTriggerData::_e_operation::o_summon:
			pTemp = malloc(sizeof(O_SUMMON));
			*((O_SUMMON*)pTemp) = ((COperationParam_Summon*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_trigger_task:
			pTemp = malloc(sizeof(O_TRIGGER_TASK));
			((O_TRIGGER_TASK*)pTemp)->uTaskID = ((COperationParam_Trigger_Task*)pParam)->m_Data.uTaskID;
			break;
		case CTriggerData::_e_operation::o_change_path:
			pTemp = malloc(sizeof(O_CHANGE_PATH));
			((O_CHANGE_PATH*)pTemp)->uPathID = ((COperationParam_Change_Path*)pParam)->m_Data.uPathID;
			((O_CHANGE_PATH*)pTemp)->iType = ((COperationParam_Change_Path*)pParam)->m_Data.iType;
			break;
		case CTriggerData::_e_operation::o_disappear:
			break;
		case CTriggerData::_e_operation::o_sneer_monster:
			pTemp = malloc(sizeof(O_SNEER_MONSTER));
			((O_SNEER_MONSTER*)pTemp)->fRange = ((COperationParam_Sneer_Monster*)pParam)->m_Data.fRange;
			break;
		case CTriggerData::_e_operation::o_use_range_skill:
			pTemp = malloc(sizeof(O_USE_RANGE_SKILL));
			((O_USE_RANGE_SKILL*)pTemp)->uSkill = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.uSkill;
			((O_USE_RANGE_SKILL*)pTemp)->uLevel = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.uLevel;
			((O_USE_RANGE_SKILL*)pTemp)->fRange = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.fRange;
			break;
		case CTriggerData::_e_operation::o_reset:
			break;
		case CTriggerData::_e_operation::o_set_global:
			pTemp = malloc(sizeof(O_SET_GLOBAL));
			*((O_SET_GLOBAL*)pTemp) = ((COperationParam_Set_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_revise_global:
			pTemp = malloc(sizeof(O_REVISE_GLOBAL));
			*((O_REVISE_GLOBAL*)pTemp) = ((COperationParam_Revise_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_assign_global:
			pTemp = malloc(sizeof(O_ASSIGN_GLOBAL));
			*((O_ASSIGN_GLOBAL*)pTemp) = ((COperationParam_Assign_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_summon_mineral:
			pTemp = malloc(sizeof(O_SUMMON_MINERAL));
			*((O_SUMMON_MINERAL*)pTemp) = ((COperationParam_Summon_Mineral*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_drop_item:
			pTemp = malloc(sizeof(O_DROP_ITEM));
			*((O_DROP_ITEM*)pTemp) = ((COperationParam_Drop_Item*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_change_hate:
			pTemp = malloc(sizeof(O_CHANGE_HATE));
			*((O_CHANGE_HATE*)pTemp) = ((COperationParam_Change_Hate*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_start_event:
			pTemp = malloc(sizeof(O_START_EVENT));
			*((O_START_EVENT*)pTemp) = ((COperationParam_Start_Event*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_stop_event:
			pTemp = malloc(sizeof(O_STOP_EVENT));
			*((O_STOP_EVENT*)pTemp) = ((COperationParam_Stop_Event*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_drop_item_new:
			pTemp = malloc(sizeof(O_DROP_ITEMNEW));
			*((O_DROP_ITEMNEW*)pTemp) = ((COperationParam_Drop_ItemNew*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_whisper:
			pTemp = malloc(sizeof(O_WHISPER_TEXT));
			assert(pTemp != nullptr);
			((O_WHISPER_TEXT*)pTemp)->uSize = (int32_t)wcslen((WCHAR*)((COperationParam_Whisper*)pParam)->m_Data.szData) * 2 + 2;
			((O_WHISPER_TEXT*)pTemp)->szData = new uint16_t[((O_WHISPER_TEXT*)pTemp)->uSize];
			wcscpy((WCHAR*)((O_WHISPER_TEXT*)pTemp)->szData, (WCHAR*)((COperationParam_Whisper*)pParam)->m_Data.szData);
			break;
		default:
			MessageBox(L"Unrecognized operation type!");
		}

		if (m_pTriggerData)
			m_pTriggerData->AddOperation(pParam->GetOperationType(), pTemp, &pParam->mTarget);

		m_bIsChanged = true;
		FreshList();
		if (dlg.GetNewOperation())
			delete dlg.GetNewOperation();
	}
}


void CPolicyTriggerDlg::OnDelOperation()
{
	int32_t sel = m_listOperation.GetCurSel();
	if (sel != -1)
	{
		if (IDYES == AfxMessageBox(L"Are you sure you want to delete the currently selected operation?",
			MB_YESNO | MB_ICONQUESTION))
		{
			m_pTriggerData->DelOperation(sel);
			FreshList();
			m_bIsChanged = true;
		}
	}
}


void CPolicyTriggerDlg::OnMoveUp()
{
	int32_t sel = m_listOperation.GetCurSel();
	if (sel == -1 || sel == 0)
		return;

	uint32_t iType1;
	uint32_t iType2;
	void* pData1;
	void* pData2;

	CTriggerData::_s_target target1, target2;
	m_pTriggerData->GetOperation(sel, iType1, &pData1, target1);
	m_pTriggerData->GetOperation(sel - 1, iType2, &pData2, target2);

	m_pTriggerData->SetOperation(sel, iType2, pData2, &target2);
	m_pTriggerData->SetOperation(sel - 1, iType1, pData1, &target1);
	FreshList();

	m_listOperation.SetCurSel(sel - 1);
	m_bIsChanged = true;
}


void CPolicyTriggerDlg::OnMoveDown()
{
	int32_t sel = m_listOperation.GetCurSel();
	if (sel == -1 || sel > m_listOperation.GetCount() - 2)
		return;

	uint32_t iType1;
	uint32_t iType2;
	void* pData1;
	void* pData2;
	CTriggerData::_s_target target1;
	CTriggerData::_s_target target2;

	m_pTriggerData->GetOperation(sel, iType1, &pData1, target1);
	m_pTriggerData->GetOperation(sel + 1, iType2, &pData2, target2);

	m_pTriggerData->SetOperation(sel, iType2, pData2, &target2);
	m_pTriggerData->SetOperation(sel + 1, iType1, pData1, &target1);
	FreshList();

	m_listOperation.SetCurSel(sel + 1);
	m_bIsChanged = true;
}


void CPolicyTriggerDlg::OnModify()
{
	if (m_pTriggerData == nullptr || m_pCurrentPolicy == nullptr)
		return;

	UpdateData(true);

	if (m_pTriggerData->GetConditionRoot() == 0)
	{
		MessageBox(L"Triggers must have conditional expressions!");
		return;
	}

	if (m_pTriggerData->GetOperationNum() <= 0)
	{
		MessageBox(L"There is at least one action in the trigger!");
		return;
	}

	if (m_bModified)
	{
		if (m_strTriggerName.IsEmpty())
		{
			MessageBox(L"Trigger name cannot be empty!");
			return;
		}

		if (m_pTriggerData)
			m_pTriggerData->SetName(Convert::CStringWToGB18030(m_strTriggerName));

		if (m_pOldTree)
			delete m_pOldTree;
	}
	else
	{
		CString strName;
		uint32_t tempId = 0;
		strName = m_strTriggerName;
		if (m_strTriggerName.IsEmpty())
		{
			MessageBox(L"Trigger name cannot be empty!");
			return;
		}

		for (int32_t i = 0; i < (int32_t)m_pCurrentPolicy->GetTriggerPtrNum(); ++i)
		{
			CTriggerData* pData = m_pCurrentPolicy->GetTriggerPtr(i);
			if (tempId <= pData->GetID())
				tempId = pData->GetID() + 1;
		}

		m_dwTriggerID = tempId;
		m_pTriggerData->SetName(Convert::CStringWToGB18030(strName));
		m_pTriggerData->SetID(m_dwTriggerID);
	}

	if (m_bActive)
		m_pTriggerData->ActiveTrigger();
	else
		m_pTriggerData->ToggleTrigger();

	m_pTriggerData->SetRunCondition(m_RunCondition);
	m_bIsChanged = true;

	CDialogEx::OnOK();
}


void CPolicyTriggerDlg::OnDblclkListboxOperation()
{
	int32_t sel = m_listOperation.GetCurSel();
	if (sel == -1)
		return;

	CPolicyOperationDlg dlg;
	dlg.m_pPolicyData = m_pCurrentPolicy;
	dlg.m_bModify = true;
	dlg.m_pOperationData = m_pTriggerData->GetOperation(sel);

	if (IDOK == dlg.DoModal())
	{
		void* pTemp = nullptr;
		COperationParam* pParam = dlg.GetNewOperation();
		switch ((CTriggerData::_e_operation)pParam->GetOperationType())
		{
		case CTriggerData::_e_operation::o_attack:
			pTemp = malloc(sizeof(O_ATTACK_TYPE));
			assert(pTemp != nullptr);
			((O_ATTACK_TYPE*)pTemp)->uType = ((COperationParam_Attack*)pParam)->m_Data.uType;
			break;
		case CTriggerData::_e_operation::o_use_skill:
			pTemp = malloc(sizeof(O_USE_SKILL));
			assert(pTemp != nullptr);
			((O_USE_SKILL*)pTemp)->uSkill = ((COperationParam_Use_Skill*)pParam)->m_Data.uSkill;
			((O_USE_SKILL*)pTemp)->uLevel = ((COperationParam_Use_Skill*)pParam)->m_Data.uLevel;
			break;
		case CTriggerData::_e_operation::o_talk:
			pTemp = malloc(sizeof(O_TALK_TEXT));
			assert(pTemp != nullptr);
			((O_TALK_TEXT*)pTemp)->szData = 0;
			((O_TALK_TEXT*)pTemp)->uSize = (int32_t)wcslen((WCHAR*)((COperationParam_Talk*)pParam)->m_Data.szData) * 2 + 2;
			((O_TALK_TEXT*)pTemp)->szData = new uint16_t[((O_TALK_TEXT*)pTemp)->uSize];
			wcscpy((WCHAR*)((O_TALK_TEXT*)pTemp)->szData, (WCHAR*)((COperationParam_Talk*)pParam)->m_Data.szData);
			break;
		case CTriggerData::_e_operation::o_reset_hate_list:
			break;
		case CTriggerData::_e_operation::o_run_trigger:
			pTemp = malloc(sizeof(O_RUN_TRIGGER));
			assert(pTemp != nullptr);
			((O_RUN_TRIGGER*)pTemp)->uID = ((COperationParam_Run_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_stop_trigger:
			pTemp = malloc(sizeof(O_STOP_TRIGGER));
			assert(pTemp != nullptr);
			((O_STOP_TRIGGER*)pTemp)->uID = ((COperationParam_Stop_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_active_trigger:
			pTemp = malloc(sizeof(O_ACTIVE_TRIGGER));
			assert(pTemp != nullptr);
			((O_ACTIVE_TRIGGER*)pTemp)->uID = ((COperationParam_Active_Trigger*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_create_timer:
			pTemp = malloc(sizeof(O_CREATE_TIMER));
			assert(pTemp != nullptr);
			*((O_CREATE_TIMER*)pTemp) = ((COperationParam_Create_Timer*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_kill_timer:
			pTemp = malloc(sizeof(O_KILL_TIMER));
			assert(pTemp != nullptr);
			((O_KILL_TIMER*)pTemp)->uID = ((COperationParam_Kill_Timer*)pParam)->m_Data.uID;
			break;
		case CTriggerData::_e_operation::o_flee:
		case CTriggerData::_e_operation::o_set_hate_to_first:
		case CTriggerData::_e_operation::o_set_hate_to_last:
		case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		case CTriggerData::_e_operation::o_skip_operation:
			break;
		case CTriggerData::_e_operation::o_active_controller:
			pTemp = malloc(sizeof(O_ACTIVE_CONTROLLER));
			assert(pTemp != nullptr);
			((O_ACTIVE_CONTROLLER*)pTemp)->uID = ((COperationParam_Active_Controller*)pParam)->m_Data.uID;
			((O_ACTIVE_CONTROLLER*)pTemp)->bStop = ((COperationParam_Active_Controller*)pParam)->m_Data.bStop;
			break;
		case CTriggerData::_e_operation::o_summon:
			pTemp = malloc(sizeof(O_SUMMON));
			assert(pTemp != nullptr);
			*((O_SUMMON*)pTemp) = ((COperationParam_Summon*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_trigger_task:
			pTemp = malloc(sizeof(O_TRIGGER_TASK));
			assert(pTemp != nullptr);
			((O_TRIGGER_TASK*)pTemp)->uTaskID = ((COperationParam_Trigger_Task*)pParam)->m_Data.uTaskID;
			break;
		case CTriggerData::_e_operation::o_change_path:
			pTemp = malloc(sizeof(O_CHANGE_PATH));
			((O_CHANGE_PATH*)pTemp)->uPathID = ((COperationParam_Change_Path*)pParam)->m_Data.uPathID;
			((O_CHANGE_PATH*)pTemp)->iType = ((COperationParam_Change_Path*)pParam)->m_Data.iType;
			break;
		case CTriggerData::_e_operation::o_disappear:
			break;
		case CTriggerData::_e_operation::o_sneer_monster:
			pTemp = malloc(sizeof(O_SNEER_MONSTER));
			assert(pTemp != nullptr);
			((O_SNEER_MONSTER*)pTemp)->fRange = ((COperationParam_Sneer_Monster*)pParam)->m_Data.fRange;
			break;
		case CTriggerData::_e_operation::o_use_range_skill:
			pTemp = malloc(sizeof(O_USE_RANGE_SKILL));
			assert(pTemp != nullptr);
			((O_USE_RANGE_SKILL*)pTemp)->uSkill = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.uSkill;
			((O_USE_RANGE_SKILL*)pTemp)->uLevel = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.uLevel;
			((O_USE_RANGE_SKILL*)pTemp)->fRange = ((COperationParam_Use_Range_Skill*)pParam)->m_Data.fRange;
			break;
		case CTriggerData::_e_operation::o_reset:
			break;
		case CTriggerData::_e_operation::o_set_global:
			pTemp = malloc(sizeof(O_SET_GLOBAL));
			assert(pTemp != nullptr);
			*((O_SET_GLOBAL*)pTemp) = ((COperationParam_Set_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_revise_global:
			pTemp = malloc(sizeof(O_REVISE_GLOBAL));
			assert(pTemp != nullptr);
			*((O_REVISE_GLOBAL*)pTemp) = ((COperationParam_Revise_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_assign_global:
			pTemp = malloc(sizeof(O_ASSIGN_GLOBAL));
			assert(pTemp != nullptr);
			*((O_ASSIGN_GLOBAL*)pTemp) = ((COperationParam_Assign_Global*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_summon_mineral:
			pTemp = malloc(sizeof(O_SUMMON_MINERAL));
			assert(pTemp != nullptr);
			*((O_SUMMON_MINERAL*)pTemp) = ((COperationParam_Summon_Mineral*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_drop_item:
			pTemp = malloc(sizeof(O_DROP_ITEM));
			assert(pTemp != nullptr);
			*((O_DROP_ITEM*)pTemp) = ((COperationParam_Drop_Item*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_change_hate:
			pTemp = malloc(sizeof(O_CHANGE_HATE));
			assert(pTemp != nullptr);
			*((O_CHANGE_HATE*)pTemp) = ((COperationParam_Change_Hate*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_start_event:
			pTemp = malloc(sizeof(O_START_EVENT));
			assert(pTemp != nullptr);
			((O_START_EVENT*)pTemp)->iId = ((COperationParam_Start_Event*)pParam)->m_Data.iId;
			break;
		case CTriggerData::_e_operation::o_stop_event:
			pTemp = malloc(sizeof(O_STOP_EVENT));
			assert(pTemp != nullptr);
			((O_STOP_EVENT*)pTemp)->iId = ((COperationParam_Stop_Event*)pParam)->m_Data.iId;
			break;
		case CTriggerData::_e_operation::o_drop_item_new:
			pTemp = malloc(sizeof(O_DROP_ITEMNEW));
			assert(pTemp != nullptr);
			*((O_DROP_ITEMNEW*)pTemp) = ((COperationParam_Drop_ItemNew*)pParam)->m_Data;
			break;
		case CTriggerData::_e_operation::o_whisper:
			pTemp = malloc(sizeof(O_WHISPER_TEXT));
			assert(pTemp != nullptr);
			((O_WHISPER_TEXT*)pTemp)->szData = 0;
			((O_WHISPER_TEXT*)pTemp)->uSize = (int32_t)wcslen((WCHAR*)((COperationParam_Whisper*)pParam)->m_Data.szData) * 2 + 2;
			((O_WHISPER_TEXT*)pTemp)->szData = new uint16_t[((O_WHISPER_TEXT*)pTemp)->uSize];
			wcscpy((WCHAR*)((O_WHISPER_TEXT*)pTemp)->szData, (WCHAR*)((COperationParam_Whisper*)pParam)->m_Data.szData);
			break;
		case CTriggerData::_e_operation::o_talk_portrait:
			pTemp = malloc(sizeof(O_TALK_PORTRAIT));
			assert(pTemp != nullptr);
			((O_TALK_PORTRAIT*)pTemp)->szData = 0;
			((O_TALK_PORTRAIT*)pTemp)->uSize = (int32_t)wcslen((WCHAR*)((COperationParam_Talk_Portrait*)pParam)->m_Data.szData) * 2 + 2;
			((O_TALK_PORTRAIT*)pTemp)->szData = new uint16_t[((O_TALK_PORTRAIT*)pTemp)->uSize];
			wcscpy((WCHAR*)((O_TALK_PORTRAIT*)pTemp)->szData, (WCHAR*)((COperationParam_Talk_Portrait*)pParam)->m_Data.szData);
			break;
		default:
			MessageBox(L"Unrecognized operation type!");
		}

		//Release old operation data
		if (dlg.m_pOperationData->mTarget.pParam)
		{
			free(dlg.m_pOperationData->mTarget.pParam);
			dlg.m_pOperationData->mTarget.pParam = 0;
		}

		if (dlg.m_pOperationData->pParam)
		{
			if (dlg.m_pOperationData->iType == (int32_t)CTriggerData::_e_operation::o_talk)
			{
				if (((O_TALK_TEXT*)dlg.m_pOperationData->pParam)->szData)
					delete ((O_TALK_TEXT*)dlg.m_pOperationData->pParam)->szData;
			}
			else if (dlg.m_pOperationData->iType == (int32_t)CTriggerData::_e_operation::o_whisper)
			{
				if (((O_WHISPER_TEXT*)dlg.m_pOperationData->pParam)->szData)
					delete ((O_WHISPER_TEXT*)dlg.m_pOperationData->pParam)->szData;
			}
			else if (dlg.m_pOperationData->iType == (int32_t)CTriggerData::_e_operation::o_talk_portrait)
			{
				if (((O_TALK_PORTRAIT*)dlg.m_pOperationData->pParam)->szData)
					delete ((O_TALK_PORTRAIT*)dlg.m_pOperationData->pParam)->szData;
			}

			free(dlg.m_pOperationData->pParam);
			dlg.m_pOperationData->pParam = 0;
		}

		//Set to new operation data
		if (m_pTriggerData)
			m_pTriggerData->SetOperation(sel, pParam->GetOperationType(), pTemp, &pParam->mTarget);

		m_bIsChanged = true;
		FreshList();

		if (dlg.GetNewOperation())
			delete dlg.GetNewOperation();
	}
}

CString GetTargetStr(void* pTarget)
{
	if (pTarget == nullptr)
		return L"No Target";

	CString msg;
	CTriggerData::_s_target* pTempTarget = (CTriggerData::_s_target*)pTarget;
	if (pTempTarget == 0)
		return msg;

	msg += L" [";

	CString mTargetList[(int32_t)CTriggerData::_e_target::t_num] =
	{
		L"AGGRO_FIRST",
		L"AGGRO_SECOND",
		L"AGGRO_SECOND_RAND",
		L"MOST_HP",
		L"MOST_MP",
		L"LEAST_HP",
		L"CLASS_COMBO",
		L"SELF"
	};

	msg += mTargetList[pTempTarget->iType];
	switch ((CTriggerData::_e_target)pTempTarget->iType)
	{
	case CTriggerData::_e_target::t_occupation_list:
		break;
	}

	msg += L"]";
	return msg;
}
