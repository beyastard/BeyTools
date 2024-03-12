// PolicyOperationDlg.cpp : implementation file
//

#include "pch.h"
#include "PolicyEdit.h"
#include "afxdialogex.h"
#include "PolicyOperationDlg.h"
#include "PropertyList.h"
#include "AObject.h"
#include "Policy.h"
#include "OperationParam.h"

CString strOperationList[(int32_t)CTriggerData::_e_operation::o_num] =
{
	L"Attack",
	L"Use_Skill",
	L"Talk",
	L"Reset_Hate_List",
	L"Run_Trigger",
	L"Stop_Trigger",
	L"Active_Trigger",
	L"Create_Timer",
	L"Kill_Timer",
	L"Flee",
	L"Set_Hate_To_First",
	L"Set_Hate_To_Last",
	L"Set_Hate_To_Half",
	L"Skip_Operation",
	L"Active_Controller",
	L"Summon",
	L"Trigger_Task",
	L"Change_Path",
	L"Disappear",
	L"Sneer_Monster",
	L"Use_Range_Skill",
	L"Reset",
	L"Set_Global",
	L"Revise_Global",
	L"Assign_Global",
	L"Summon_Mineral",
	L"Drop_Item",
	L"Change_Hate",
	L"Start_Event",
	L"Stop_Event",
	L"Drop_Item_New",
	L"Whisper",
	L"Talk_Portrait",
};

CString strTargetList[(int32_t)CTriggerData::_e_target::t_num] =
{
	L"AGGRO_FIRST",
	L"AGGRO_SECOND",
	L"AGGRO_SECOND_RAND",
	L"MOST_HP",
	L"MOST_MP",
	L"LEAST_HP",
	L"CLASS_COMBO",
	L"SELF",
};

// CPolicyOperationDlg dialog

IMPLEMENT_DYNAMIC(CPolicyOperationDlg, CDialogEx)

CPolicyOperationDlg::CPolicyOperationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_POLICY_OPERATION, pParent)
	, m_bWuXia(false)
	, m_bFaShi(false)
	, m_bSengLu(false)
	, m_bYaoJing(false)
	, m_bYaoShou(false)
	, m_bMeiLing(false)
	, m_bYuMang(false)
	, m_bYuLing(false)
	, m_pPolicyData(nullptr)
	, m_bModify(false)
	, m_pOperationData(nullptr)
	, m_pTemp(nullptr)
	, m_pNewOperation(nullptr)
	, m_pList(nullptr)
{}

CPolicyOperationDlg::~CPolicyOperationDlg() {}

void CPolicyOperationDlg::UpdateProperty(ADynPropertyObject* pProObj)
{
	if (m_pList && pProObj)
		m_pList->AttachDataObject(pProObj);
}

void CPolicyOperationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_OPERATION_TYPE, m_cbOperationType);
	DDX_Control(pDX, IDC_COMBO_TARGET_TYPE, m_cbTargetType);
	DDX_Check(pDX, IDC_CHECK_WUXIA, m_bWuXia);
	DDX_Check(pDX, IDC_CHECK_FASHI, m_bFaShi);
	DDX_Check(pDX, IDC_CHECK_SENGLU, m_bSengLu);
	DDX_Check(pDX, IDC_CHECK_YAOJING, m_bYaoJing);
	DDX_Check(pDX, IDC_CHECK_YAOSHOU, m_bYaoShou);
	DDX_Check(pDX, IDC_CHECK_MEILING, m_bMeiLing);
	DDX_Check(pDX, IDC_CHECK_YUMANG, m_bYuMang);
	DDX_Check(pDX, IDC_CHECK_YULING, m_bYuLing);
}

void CPolicyOperationDlg::OnOK()
{
	int32_t sel = m_cbTargetType.GetCurSel();
	if (sel == -1)
		return;

	UpdateData(true);
	if (m_pNewOperation == nullptr)
		return;

	m_pNewOperation->mTarget.iType = sel;
	if (m_pNewOperation->mTarget.pParam)
		free(m_pNewOperation->mTarget.pParam);

	if (sel == (int32_t)CTriggerData::_e_target::t_occupation_list)
	{
		T_OCCUPATION* pNewData = (T_OCCUPATION*)malloc(sizeof(T_OCCUPATION));
		pNewData->uBit = 0;
		if (m_bWuXia)   pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_wuxia;
		if (m_bFaShi)   pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_fashi;
		if (m_bSengLu)  pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_senglu;
		if (m_bYaoJing) pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_yaojing;
		if (m_bYaoShou) pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_yaoshou;
		if (m_bMeiLing) pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_meiling;
		if (m_bYuMang)  pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_yumang;
		if (m_bYuLing)  pNewData->uBit |= (int32_t)CTriggerData::_e_occupation::o_yuling;

		m_pNewOperation->mTarget.pParam = pNewData;
	}
	else
		m_pNewOperation->mTarget.pParam = 0;

	if (m_pList)
		m_pList->UpdateData(true);

	if (m_pNewOperation)
		m_pNewOperation->UpdateProperty(true);

	CDialogEx::OnOK();
}

void CPolicyOperationDlg::OnCancel()
{
	if (m_pList)
	{
		m_pList->CleanItems();
		m_pList->DestroyWindow();

		delete m_pList;

		if (m_pTemp)
			delete m_pTemp;

		m_pList = nullptr;
		m_pTemp = nullptr;
	}

	if (m_pNewOperation)
	{
		if (m_pNewOperation->mTarget.pParam)
			free(m_pNewOperation->mTarget.pParam);

		delete m_pNewOperation;
		m_pNewOperation = nullptr;
	}

	CDialogEx::OnCancel();
}

void CPolicyOperationDlg::ChangeProperty(int32_t iOpType)
{
	UpdateProperty(m_pTemp);

	if (m_pNewOperation)
	{
		if (m_pNewOperation->mTarget.pParam)
			free(m_pNewOperation->mTarget.pParam);

		delete m_pNewOperation;
	}

	m_pNewOperation = nullptr;
	switch ((CTriggerData::_e_operation)iOpType)
	{
	case CTriggerData::_e_operation::o_attack:
		m_pNewOperation = new COperationParam_Attack;
		break;
	case CTriggerData::_e_operation::o_use_skill:
		m_pNewOperation = new COperationParam_Use_Skill;
		break;
	case CTriggerData::_e_operation::o_talk:
		m_pNewOperation = new COperationParam_Talk;
		break;
	case CTriggerData::_e_operation::o_reset_hate_list:
		m_pNewOperation = new COperationParam_Reset_Hates;
		break;
	case CTriggerData::_e_operation::o_run_trigger:
		m_pNewOperation = new COperationParam_Run_Trigger;
		break;
	case CTriggerData::_e_operation::o_stop_trigger:
		m_pNewOperation = new COperationParam_Stop_Trigger;
		break;
	case CTriggerData::_e_operation::o_active_trigger:
		m_pNewOperation = new COperationParam_Active_Trigger;
		break;
	case CTriggerData::_e_operation::o_create_timer:
		m_pNewOperation = new COperationParam_Create_Timer;
		break;
	case CTriggerData::_e_operation::o_kill_timer:
		m_pNewOperation = new COperationParam_Kill_Timer;
		break;
	case CTriggerData::_e_operation::o_flee:
		m_pNewOperation = new COperationParam_Flee;
		break;
	case CTriggerData::_e_operation::o_set_hate_to_first:
		m_pNewOperation = new COperationParam_SetHateToFirst;
		break;
	case CTriggerData::_e_operation::o_set_hate_to_last:
		m_pNewOperation = new COperationParam_SetHateToLast;
		break;
	case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		m_pNewOperation = new COperationParam_SetHateFiftyPercent;
		break;
	case CTriggerData::_e_operation::o_skip_operation:
		m_pNewOperation = new COperationParam_SkipOperation;
		break;
	case CTriggerData::_e_operation::o_active_controller:
		m_pNewOperation = new COperationParam_Active_Controller;
		break;
	case CTriggerData::_e_operation::o_summon:
		m_pNewOperation = new COperationParam_Summon;
		break;
	case CTriggerData::_e_operation::o_trigger_task:
		m_pNewOperation = new COperationParam_Trigger_Task;
		break;
	case CTriggerData::_e_operation::o_change_path:
		m_pNewOperation = new COperationParam_Change_Path;
		break;
	case CTriggerData::_e_operation::o_disappear:
		m_pNewOperation = new COperationParam_Disappear;
		break;
	case CTriggerData::_e_operation::o_sneer_monster:
		m_pNewOperation = new COperationParam_Sneer_Monster;
		break;
	case CTriggerData::_e_operation::o_use_range_skill:
		m_pNewOperation = new COperationParam_Use_Range_Skill;
		break;
	case CTriggerData::_e_operation::o_reset:
		m_pNewOperation = new COperationParam_Reset;
		break;
	case CTriggerData::_e_operation::o_set_global:
		m_pNewOperation = new COperationParam_Set_Global;
		break;
	case CTriggerData::_e_operation::o_revise_global:
		m_pNewOperation = new COperationParam_Revise_Global;
		break;
	case CTriggerData::_e_operation::o_assign_global:
		m_pNewOperation = new COperationParam_Assign_Global;
		break;
	case CTriggerData::_e_operation::o_summon_mineral:
		m_pNewOperation = new COperationParam_Summon_Mineral;
		break;
	case CTriggerData::_e_operation::o_drop_item:
		m_pNewOperation = new COperationParam_Drop_Item;
		break;
	case CTriggerData::_e_operation::o_change_hate:
		m_pNewOperation = new COperationParam_Change_Hate;
		break;
	case CTriggerData::_e_operation::o_start_event:
		m_pNewOperation = new COperationParam_Start_Event;
		break;
	case CTriggerData::_e_operation::o_stop_event:
		m_pNewOperation = new COperationParam_Stop_Event;
		break;
	case CTriggerData::_e_operation::o_drop_item_new:
		m_pNewOperation = new COperationParam_Drop_ItemNew;
		break;
	case CTriggerData::_e_operation::o_whisper:
		m_pNewOperation = new COperationParam_Whisper;
		break;
	case CTriggerData::_e_operation::o_talk_portrait:
		m_pNewOperation = new COperationParam_Talk_Portrait;
		break;
	default:
		MessageBox(L"Unrecognized operation type!");
	}

	if (m_pNewOperation)
	{
		m_pNewOperation->m_pPolicyData = m_pPolicyData;
		m_pNewOperation->BuildProperty();
		UpdateProperty(m_pNewOperation->m_pProperty);
	}
}

void CPolicyOperationDlg::EnableCheckBox(bool bEnable)
{
	CWnd* pWnd = GetDlgItem(IDC_CHECK_WUXIA);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_FASHI);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_SENGLU);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_YAOJING);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_YAOSHOU);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_MEILING);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_YUMANG);
	pWnd->EnableWindow(bEnable);
	pWnd = GetDlgItem(IDC_CHECK_YULING);
	pWnd->EnableWindow(bEnable);
}

void CPolicyOperationDlg::InitOperation()
{
	if (m_pOperationData == nullptr)
		return;

	if (m_pNewOperation)
	{
		if (m_pNewOperation->mTarget.pParam)
			free(m_pNewOperation->mTarget.pParam);

		delete m_pNewOperation;
		m_pNewOperation = nullptr;
	}

	m_cbOperationType.SetCurSel(m_pOperationData->iType);
	m_cbTargetType.SetCurSel(m_pOperationData->mTarget.iType);
	m_pNewOperation = nullptr;

	switch ((CTriggerData::_e_operation)m_pOperationData->iType)
	{
	case CTriggerData::_e_operation::o_attack:
		m_pNewOperation = new COperationParam_Attack;
		((COperationParam_Attack*)m_pNewOperation)->m_Data = *((O_ATTACK_TYPE*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_use_skill:
		m_pNewOperation = new COperationParam_Use_Skill;
		((COperationParam_Use_Skill*)m_pNewOperation)->m_Data = *((O_USE_SKILL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_talk:
		m_pNewOperation = new COperationParam_Talk;
		//memcpy(((COperationParam_Talk*)m_pNewOperation)->m_Data.szData,((O_TALK_TEXT*)m_pOperationData->pParam)->szData,sizeof(unsigned short)*((O_TALK_TEXT*)m_pOperationData->pParam)->uSize);
		wcscpy((WCHAR*)((COperationParam_Talk*)m_pNewOperation)->m_Data.szData, (WCHAR*)((O_TALK_TEXT*)m_pOperationData->pParam)->szData);
		break;
	case CTriggerData::_e_operation::o_reset_hate_list:
		m_pNewOperation = new COperationParam_Reset_Hates;
		break;
	case CTriggerData::_e_operation::o_run_trigger:
		m_pNewOperation = new COperationParam_Run_Trigger;
		((COperationParam_Run_Trigger*)m_pNewOperation)->m_Data = *((O_RUN_TRIGGER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_stop_trigger:
		m_pNewOperation = new COperationParam_Stop_Trigger;
		((COperationParam_Stop_Trigger*)m_pNewOperation)->m_Data = *((O_STOP_TRIGGER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_active_trigger:
		m_pNewOperation = new COperationParam_Active_Trigger;
		((COperationParam_Active_Trigger*)m_pNewOperation)->m_Data = *((O_ACTIVE_TRIGGER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_create_timer:
		m_pNewOperation = new COperationParam_Create_Timer;
		((COperationParam_Create_Timer*)m_pNewOperation)->m_Data = *((O_CREATE_TIMER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_kill_timer:
		m_pNewOperation = new COperationParam_Kill_Timer;
		((COperationParam_Kill_Timer*)m_pNewOperation)->m_Data = *((O_KILL_TIMER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_flee:
		m_pNewOperation = new COperationParam_Flee;
		break;
	case CTriggerData::_e_operation::o_set_hate_to_first:
		m_pNewOperation = new COperationParam_SetHateToFirst;
		break;
	case CTriggerData::_e_operation::o_set_hate_to_last:
		m_pNewOperation = new COperationParam_SetHateToLast;
		break;
	case CTriggerData::_e_operation::o_set_hate_fifty_percent:
		m_pNewOperation = new COperationParam_SetHateFiftyPercent;
		break;
	case CTriggerData::_e_operation::o_skip_operation:
		m_pNewOperation = new COperationParam_SkipOperation;
		break;
	case CTriggerData::_e_operation::o_active_controller:
		m_pNewOperation = new COperationParam_Active_Controller;
		((COperationParam_Active_Controller*)m_pNewOperation)->m_Data = *((O_ACTIVE_CONTROLLER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_summon:
		m_pNewOperation = new COperationParam_Summon;
		((COperationParam_Summon*)m_pNewOperation)->m_Data = *((O_SUMMON*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_trigger_task:
		m_pNewOperation = new COperationParam_Trigger_Task;
		((COperationParam_Trigger_Task*)m_pNewOperation)->m_Data = *((O_TRIGGER_TASK*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_change_path:
		m_pNewOperation = new COperationParam_Change_Path;
		((COperationParam_Change_Path*)m_pNewOperation)->m_Data = *((O_CHANGE_PATH*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_disappear:
		m_pNewOperation = new COperationParam_Disappear;
		break;
	case CTriggerData::_e_operation::o_sneer_monster:
		m_pNewOperation = new COperationParam_Sneer_Monster;
		((COperationParam_Sneer_Monster*)m_pNewOperation)->m_Data = *((O_SNEER_MONSTER*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_use_range_skill:
		m_pNewOperation = new COperationParam_Use_Range_Skill;
		((COperationParam_Use_Range_Skill*)m_pNewOperation)->m_Data = *((O_USE_RANGE_SKILL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_reset:
		m_pNewOperation = new COperationParam_Reset;
		break;
	case CTriggerData::_e_operation::o_set_global:
		m_pNewOperation = new COperationParam_Set_Global;
		((COperationParam_Set_Global*)m_pNewOperation)->m_Data = *((O_SET_GLOBAL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_revise_global:
		m_pNewOperation = new COperationParam_Revise_Global;
		((COperationParam_Revise_Global*)m_pNewOperation)->m_Data = *((O_REVISE_GLOBAL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_assign_global:
		m_pNewOperation = new COperationParam_Assign_Global;
		((COperationParam_Assign_Global*)m_pNewOperation)->m_Data = *((O_ASSIGN_GLOBAL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_summon_mineral:
		m_pNewOperation = new COperationParam_Summon_Mineral;
		((COperationParam_Summon_Mineral*)m_pNewOperation)->m_Data = *((O_SUMMON_MINERAL*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_drop_item:
		m_pNewOperation = new COperationParam_Drop_Item;
		((COperationParam_Drop_Item*)m_pNewOperation)->m_Data = *((O_DROP_ITEM*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_change_hate:
		m_pNewOperation = new COperationParam_Change_Hate;
		((COperationParam_Change_Hate*)m_pNewOperation)->m_Data = *((O_CHANGE_HATE*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_start_event:
		m_pNewOperation = new COperationParam_Start_Event;
		((COperationParam_Start_Event*)m_pNewOperation)->m_Data = *((O_START_EVENT*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_stop_event:
		m_pNewOperation = new COperationParam_Stop_Event;
		((COperationParam_Stop_Event*)m_pNewOperation)->m_Data = *((O_STOP_EVENT*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_drop_item_new:
		m_pNewOperation = new COperationParam_Drop_Item;
		((COperationParam_Drop_ItemNew*)m_pNewOperation)->m_Data = *((O_DROP_ITEMNEW*)m_pOperationData->pParam);
		break;
	case CTriggerData::_e_operation::o_whisper:
		m_pNewOperation = new COperationParam_Whisper;
		wcscpy((WCHAR*)((COperationParam_Whisper*)m_pNewOperation)->m_Data.szData, (WCHAR*)((O_WHISPER_TEXT*)m_pOperationData->pParam)->szData);
		break;
	case CTriggerData::_e_operation::o_talk_portrait:
		m_pNewOperation = new COperationParam_Talk_Portrait;
		wcscpy((WCHAR*)((COperationParam_Talk_Portrait*)m_pNewOperation)->m_Data.szData, (WCHAR*)((O_TALK_PORTRAIT*)m_pOperationData->pParam)->szData);
		break;
	default:
		MessageBox(L"Unrecognized operation type!");
	}

	if (m_pNewOperation)
	{
		m_pNewOperation->mTarget.iType = m_pOperationData->mTarget.iType;
		m_pNewOperation->m_pPolicyData = m_pPolicyData;
		m_pNewOperation->BuildProperty();
		UpdateProperty(m_pNewOperation->m_pProperty);
	}

	if (m_pOperationData->mTarget.iType == (uint32_t)CTriggerData::_e_target::t_occupation_list)
	{
		uint32_t uBit = ((T_OCCUPATION*)m_pOperationData->mTarget.pParam)->uBit;

		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_wuxia)   m_bWuXia   = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_fashi)   m_bFaShi   = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_senglu)  m_bSengLu  = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_yaojing) m_bYaoJing = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_yaoshou) m_bYaoShou = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_meiling) m_bMeiLing = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_yumang)  m_bYuMang  = true;
		if (uBit & (uint32_t)CTriggerData::_e_occupation::o_yuling)  m_bYuLing  = true;

		UpdateData(false);
	}

	OnSelChangeComboTargetType();
}


BEGIN_MESSAGE_MAP(CPolicyOperationDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_OPERATION_TYPE, &CPolicyOperationDlg::OnSelChangeComboOperationType)
	ON_CBN_SELCHANGE(IDC_COMBO_TARGET_TYPE, &CPolicyOperationDlg::OnSelChangeComboTargetType)
END_MESSAGE_MAP()


// CPolicyOperationDlg message handlers


BOOL CPolicyOperationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_pList = new CPropertyList();
	m_pTemp = new ADynPropertyObject;
	CWnd* pWnd = GetDlgItem(IDC_LIST_OPERATION);

	CRect rc, rc1;
	pWnd->GetClientRect(&rc);

	int32_t width = rc.Width();
	int32_t length = rc.Height();

	pWnd->GetWindowRect(&rc);
	GetWindowRect(&rc1);
	rc.left = rc.left - rc1.left;
	rc.top = rc.top - rc1.top - 20;
	rc.right = rc.left + width;
	rc.bottom = rc.top + length;

	BOOL bResult = m_pList->Create("Operation Table", WS_CHILD | WS_VISIBLE, rc, this, 111);

	// Populate the list of operation types
	for (int32_t i = 0; i < (int32_t)CTriggerData::_e_operation::o_num; i++)
		m_cbOperationType.InsertString(i, strOperationList[i]);

	if (m_cbOperationType.GetCount() > 0)
		m_cbOperationType.SetCurSel(0);

	for (int32_t i = 0; i < (int32_t)CTriggerData::_e_target::t_num; i++)
		m_cbTargetType.InsertString(i, strTargetList[i]);

	if (m_cbTargetType.GetCount() > 0)
		m_cbTargetType.SetCurSel(0);

	EnableCheckBox(false);
	if (m_bModify)
		InitOperation();
	else
		ChangeProperty(0);

	return true;
}

void CPolicyOperationDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_pList)
	{
		m_pList->CleanItems();
		m_pList->DestroyWindow();
		delete m_pList;

		if (m_pTemp)
			delete m_pTemp;

		m_pList = nullptr;
		m_pTemp = nullptr;
	}
}


void CPolicyOperationDlg::OnSelChangeComboOperationType()
{
	int32_t sel = m_cbOperationType.GetCurSel();
	if (sel != -1)
	{
		CString szOp;
		m_cbOperationType.GetLBText(sel, szOp);
		ChangeProperty(sel);
	}
}


void CPolicyOperationDlg::OnSelChangeComboTargetType()
{
	int32_t sel = m_cbTargetType.GetCurSel();
	if (sel != -1)
	{
		if (sel == (int32_t)CTriggerData::_e_target::t_occupation_list)
		{
			EnableCheckBox(true);
			return;
		}
	}

	EnableCheckBox(false);
}
