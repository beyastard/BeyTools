// ConditionDlg.cpp : implementation file
//

#include "pch.h"
#include "PolicyEdit.h"
#include "afxdialogex.h"
#include "ConditionDlg.h"
#include "PolicyType.h"

CString strConditionList[(int32_t)CTriggerData::_e_condition::c_num] =
{
	L"Timer expires (only used alone)",
	L"HP less than percentage",
	L"Attack starts (only used alone)",
	L"Random",
	L"Kill player (only used alone)",
	L"Logical NOT",
	L"Logical OR",
	L"Logical AND",
	L"Monster Death",
	L"Reach end of path",
	L"Hate expressed to specific value",
	L"Distance from position",
	L"Add",
	L"Subtract",
	L"Multiply",
	L"Divide",
	L"Greater Than",
	L"Less Than",
	L"Equal To",
	L"Variable",
	L"Constant",
	L"Character level leaderboard",
	L"Monster Born",
	L"Attacked by specified skill",
	L"Randomly select one",
	L"Leave the attack"
};

CString strConditionSign[(int32_t)CTriggerData::_e_condition::c_num] =
{
	L"Timer(%d)",
	L"HP_Less(%f)",
	L"Start_Attack()",
	L"Random(%f)",
	L"Kill_Player()",
	L"!",
	L"||",
	L"&&",
	L"Died()",
	L"Path_End_Point(%d)",
	L"Enmity_Reach(%d,%d)",
	L"Distance_Reach(%f)",
	L"+",
	L"-",
	L"*",
	L"/",
	L">",
	L"<",
	L"==",
	L"Var[%d]",
	L"%d",
	L"Rank_Level(%d)",
	L"Born()",
	L"Attack_Skill[%d]",
	L"Random_Select_One()",
	L"Leave_Attack()"
};

// CConditionDlg dialog

IMPLEMENT_DYNAMIC(CConditionDlg, CDialogEx)

CConditionDlg::CConditionDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CONDITION, pParent)
	, m_strLeft(_T(""))
	, m_strRight(_T(""))
	, m_iConditionParam(0)
	, m_fConditionParam(0)
	, m_uConditionFunc(0)
	, m_pRoot(nullptr)
	, m_bIsRoot(false)
	, bModify(false)
{}

CConditionDlg::~CConditionDlg() {}

void CConditionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FUNC, m_cbFunc);
	DDX_Text(pDX, IDC_EDIT_SUB1, m_strLeft);
	DDX_Text(pDX, IDC_EDIT_SUB2, m_strRight);
	DDX_Text(pDX, IDC_EDIT_PARAM_INT, m_iConditionParam);
	DDX_Text(pDX, IDC_EDIT_PARAM_FLOAT, m_fConditionParam);
}

void CConditionDlg::OnOK()
{
	UpdateData(true);

	switch ((CTriggerData::_e_condition)m_uConditionFunc)
	{
	case CTriggerData::_e_condition::c_time_come:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_TIME_COME));
		if (m_pRoot->mCondition.pParam)
			((C_TIME_COME*)m_pRoot->mCondition.pParam)->uID = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_hp_less:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_HP_LESS));
		if (m_pRoot->mCondition.pParam)
			((C_HP_LESS*)m_pRoot->mCondition.pParam)->fPercent = m_fConditionParam;
		break;
	case CTriggerData::_e_condition::c_start_attack:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		break;
	case CTriggerData::_e_condition::c_random:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_RANDOM));
		if (m_pRoot->mCondition.pParam)
			((C_RANDOM*)m_pRoot->mCondition.pParam)->fProbability = m_fConditionParam;
		break;
	case CTriggerData::_e_condition::c_kill_player:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		break;
	case CTriggerData::_e_condition::c_not:
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = 0;
		if (m_pRoot->pRight == nullptr)
		{
			MessageBox(L"Condition 2 cannot be empty!");
			return;
		}

		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		break;
	case CTriggerData::_e_condition::c_or:
	case CTriggerData::_e_condition::c_and:
		if (m_pRoot->pLeft == nullptr)
		{
			MessageBox(L"Condition 1 cannot be empty!");
			return;
		}

		if (m_pRoot->pRight == nullptr)
		{
			MessageBox(L"Condition 2 cannot be empty!");
			return;
		}

		if (!Check(m_pRoot->pLeft, m_pRoot->pRight))
		{
			MessageBox(L"Logical error!");
			return;
		}

		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		break;
	case CTriggerData::_e_condition::c_died:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		break;
	case CTriggerData::_e_condition::c_path_end_point:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_PATH_END_POINT));
		if (m_pRoot->mCondition.pParam)
			((C_PATH_END_POINT*)m_pRoot->mCondition.pParam)->uID = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_enmity_reach:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_ENMITY_REACH));
		((C_ENMITY_REACH*)m_pRoot->mCondition.pParam)->nPlayer = m_iConditionParam;
		((C_ENMITY_REACH*)m_pRoot->mCondition.pParam)->nPlayerLevel = (int32_t)m_fConditionParam;
		break;
	case CTriggerData::_e_condition::c_distance_reach:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_DISTANCE_REACH));
		((C_DISTANCE_REACH*)m_pRoot->mCondition.pParam)->fDistance = m_fConditionParam;
		break;
	case CTriggerData::_e_condition::c_plus:
	case CTriggerData::_e_condition::c_minus:
	case CTriggerData::_e_condition::c_multiply:
	case CTriggerData::_e_condition::c_divide:
	case CTriggerData::_e_condition::c_great:
	case CTriggerData::_e_condition::c_less:
	case CTriggerData::_e_condition::c_equ:
		if (m_pRoot->pLeft == nullptr)
		{
			MessageBox(L"Condition 1 cannot be empty!");
			return;
		}

		if (m_pRoot->pRight == nullptr)
		{
			MessageBox(L"Condition 2 cannot be empty!");
			return;
		}

		if (!Check(m_pRoot->pLeft, m_pRoot->pRight))
		{
			MessageBox(L"Logical error!");
			return;
		}

		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		break;
	case CTriggerData::_e_condition::c_var:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_VAR));
		((C_VAR*)m_pRoot->mCondition.pParam)->iID = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_constant:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_RANK_LEVEL));
		((C_RANK_LEVEL*)m_pRoot->mCondition.pParam)->iValue = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_rank_level:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_CONSTANT));
		((C_CONSTANT*)m_pRoot->mCondition.pParam)->iValue = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_born:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		break;
	case CTriggerData::_e_condition::c_attack_by_skill:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_ATTACK_BY_SKILL));
		((C_ATTACK_BY_SKILL*)m_pRoot->mCondition.pParam)->uID = m_iConditionParam;
		break;
	case CTriggerData::_e_condition::c_random_selectone:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		m_pRoot->mCondition.pParam = malloc(sizeof(C_RANDOM_SELECTONE));
		((C_RANDOM_SELECTONE*)m_pRoot->mCondition.pParam)->fProbability = m_fConditionParam;
		break;
	case CTriggerData::_e_condition::c_leave_attack:
	default:
		if (m_pRoot->mCondition.pParam)
			free(m_pRoot->mCondition.pParam);

		m_pRoot->mCondition.pParam = nullptr;
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = nullptr;
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = nullptr;
		break;
	}

	if (m_bIsRoot)
	{
		if (m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_plus ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_minus ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_multiply ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_divide ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_var ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_constant ||
			m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_rank_level)
		{
			MessageBox(L"Logical error!");
			return;
		}
	}

	m_pRoot->mCondition.iType = m_uConditionFunc;

	CDialogEx::OnOK();
}

void CConditionDlg::OnCancel()
{
	CDialogEx::OnCancel();
}

void CConditionDlg::FreshInterface()
{
	CWnd* pParamI = GetDlgItem(IDC_EDIT_PARAM_INT);
	CWnd* pParamF = GetDlgItem(IDC_EDIT_PARAM_FLOAT);
	CWnd* pButton1 = GetDlgItem(IDC_BUTTON_SUB1);
	CWnd* pButton2 = GetDlgItem(IDC_BUTTON_SUB2);

	pButton1->EnableWindow(false);
	pButton2->EnableWindow(false);

	CConditionDlg dlg;

	int32_t sel = m_cbFunc.GetCurSel();
	if (sel == -1)
	{
		pParamI->ShowWindow(SW_HIDE);
		pParamF->ShowWindow(SW_HIDE);
	}
	else
	{
		m_uConditionFunc = sel;
		switch ((CTriggerData::_e_condition)sel)
		{
		case CTriggerData::_e_condition::c_time_come:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			if (m_pRoot->mCondition.pParam)
				m_iConditionParam = ((C_TIME_COME*)m_pRoot->mCondition.pParam)->uID;
			break;
		case CTriggerData::_e_condition::c_hp_less:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_SHOW);
			if (m_pRoot->mCondition.pParam)
				m_fConditionParam = ((C_HP_LESS*)m_pRoot->mCondition.pParam)->fPercent;
			break;
		case CTriggerData::_e_condition::c_start_attack:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			break;
		case CTriggerData::_e_condition::c_random:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_SHOW);
			if (m_pRoot->mCondition.pParam)
				m_fConditionParam = ((C_RANDOM*)m_pRoot->mCondition.pParam)->fProbability;
			break;
		case CTriggerData::_e_condition::c_kill_player:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			break;
		case CTriggerData::_e_condition::c_not:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_or:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_and:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_died:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			break;
		case CTriggerData::_e_condition::c_path_end_point:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			if (m_pRoot->mCondition.pParam)
				m_iConditionParam = ((C_PATH_END_POINT*)m_pRoot->mCondition.pParam)->uID;
			break;
		case CTriggerData::_e_condition::c_enmity_reach:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_SHOW);
			if (m_pRoot->mCondition.pParam)
			{
				m_iConditionParam = ((C_ENMITY_REACH*)m_pRoot->mCondition.pParam)->nPlayer;
				m_fConditionParam = (float)((C_ENMITY_REACH*)m_pRoot->mCondition.pParam)->nPlayerLevel;
			}
			break;
		case CTriggerData::_e_condition::c_distance_reach:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_SHOW);
			if (m_pRoot->mCondition.pParam)
				m_fConditionParam = ((C_DISTANCE_REACH*)m_pRoot->mCondition.pParam)->fDistance;
			break;
		case CTriggerData::_e_condition::c_plus:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_minus:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_multiply:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_divide:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_great:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_less:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_equ:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(true);
			pButton2->EnableWindow(true);
			break;
		case CTriggerData::_e_condition::c_var:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(false);
			pButton2->EnableWindow(false);
			break;
		case CTriggerData::_e_condition::c_constant:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(false);
			pButton2->EnableWindow(false);
			break;
		case CTriggerData::_e_condition::c_rank_level:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			pButton1->EnableWindow(false);
			pButton2->EnableWindow(false);
			break;
		case CTriggerData::_e_condition::c_born:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			break;
		case CTriggerData::_e_condition::c_attack_by_skill:
			pParamI->ShowWindow(SW_SHOW);
			pParamF->ShowWindow(SW_HIDE);
			if (m_pRoot->mCondition.pParam)
				m_iConditionParam = ((C_ATTACK_BY_SKILL*)m_pRoot->mCondition.pParam)->uID;
			break;
		case CTriggerData::_e_condition::c_random_selectone:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_SHOW);
			if (m_pRoot->mCondition.pParam)
				m_fConditionParam = ((C_RANDOM_SELECTONE*)m_pRoot->mCondition.pParam)->fProbability;
			break;
		case CTriggerData::_e_condition::c_leave_attack:
			pParamI->ShowWindow(SW_HIDE);
			pParamF->ShowWindow(SW_HIDE);
			break;
		}

		UpdateData(false);
	}
}

CString CConditionDlg::TraverselTree(void* pTree)
{
	if (pTree == 0)
		return L"";

	CTriggerData::_s_tree_item* temp = (CTriggerData::_s_tree_item*)pTree;
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

bool CConditionDlg::Check(CTriggerData::_s_tree_item* left, CTriggerData::_s_tree_item* right)
{
	if (m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_plus ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_minus ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_multiply ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_divide ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_great ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_less ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_equ)
	{
		if (left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_plus ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_minus ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_multiply ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_divide ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_var ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_constant ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_rank_level)
		{
			if (right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_plus ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_minus ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_multiply ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_divide ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_var ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_constant ||
				right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_rank_level)
				return true;
		}
	}
	else if (
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_or ||
		m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_and)
	{
		if (left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_plus ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_minus ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_multiply ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_divide ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_var ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_constant ||
			left->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_rank_level)
			return false;

		if (right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_plus ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_minus ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_multiply ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_divide ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_var ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_constant ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_rank_level)
			return false;

		return true;
	}
	else if (m_uConditionFunc == (int32_t)CTriggerData::_e_condition::c_not)
	{
		if (right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_plus ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_minus ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_multiply ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_divide ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_var ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_constant ||
			right->mCondition.iType == (int32_t)CTriggerData::_e_condition::c_rank_level)
			return false;

		return true;
	}

	return false;
}

BOOL CConditionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int32_t i = 0; i < (int32_t)CTriggerData::_e_condition::c_num; i++)
		m_cbFunc.InsertString(i, strConditionList[i]);

	m_cbFunc.SetCurSel(m_pRoot->mCondition.iType);
	FreshInterface();

	if (m_pRoot)
	{
		m_strLeft = TraverselTree(m_pRoot->pLeft);
		m_strRight = TraverselTree(m_pRoot->pRight);
	}

	CRect rc;
	GetWindowRect(&rc);
	rc.left += 20;
	rc.top += 20;
	rc.right += 20;
	rc.bottom += 20;
	MoveWindow(rc);

	OnSelChangeComboFunc();
	UpdateData(false);

	return true;
}


BEGIN_MESSAGE_MAP(CConditionDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_FUNC, &CConditionDlg::OnSelChangeComboFunc)
	ON_EN_CHANGE(IDC_EDIT_PARAM_INT, &CConditionDlg::OnChangeEditParamInt)
	ON_EN_CHANGE(IDC_EDIT_PARAM_FLOAT, &CConditionDlg::OnChangeEditParamFloat)
	ON_BN_CLICKED(IDC_BUTTON_SUB1, &CConditionDlg::OnButtonSub1)
	ON_BN_CLICKED(IDC_BUTTON_SUB2, &CConditionDlg::OnButtonSub2)
END_MESSAGE_MAP()


// CConditionDlg message handlers


void CConditionDlg::OnSelChangeComboFunc()
{
	FreshInterface();
}


void CConditionDlg::OnChangeEditParamInt()
{
	UpdateData(true);
}


void CConditionDlg::OnChangeEditParamFloat()
{
	UpdateData(true);
}


void CConditionDlg::OnButtonSub1()
{
	CConditionDlg dlg;
	CTriggerData::_s_tree_item* pOld = CTriggerData::CopyConditionTree(m_pRoot->pLeft);
	if (m_pRoot->pLeft == nullptr)
		m_pRoot->pLeft = new CTriggerData::_s_tree_item;

	dlg.m_pRoot = m_pRoot->pLeft;
	if (IDOK == dlg.DoModal())
	{
		if (pOld)
		{
			delete pOld;
			pOld = nullptr;
		}
	}
	else
	{
		if (m_pRoot->pLeft)
			delete m_pRoot->pLeft;

		m_pRoot->pLeft = pOld;
	}

	m_strLeft = TraverselTree(m_pRoot->pLeft);
	m_strRight = TraverselTree(m_pRoot->pRight);

	UpdateData(false);
}


void CConditionDlg::OnButtonSub2()
{
	CConditionDlg dlg;
	CTriggerData::_s_tree_item* pOld = CTriggerData::CopyConditionTree(m_pRoot->pRight);
	if (m_pRoot->pRight == nullptr)
		m_pRoot->pRight = new CTriggerData::_s_tree_item;

	dlg.m_pRoot = m_pRoot->pRight;

	dlg.bModify = bModify;
	if (IDOK == dlg.DoModal())
	{
		if (pOld)
		{
			delete pOld;
			pOld = nullptr;
		}
	}
	else
	{
		if (m_pRoot->pRight)
			delete m_pRoot->pRight;

		m_pRoot->pRight = pOld;
	}

	m_strLeft = TraverselTree(m_pRoot->pLeft);
	m_strRight = TraverselTree(m_pRoot->pRight);

	UpdateData(false);
}
