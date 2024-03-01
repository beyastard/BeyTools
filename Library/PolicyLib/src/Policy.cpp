#include "Policy.h"
#include "PolicyType.h"

enum
{
	CONDITION_TREE_NULL,
	CONDITION_LEFT_SUB,
	CONDITION_RIGHT_SUB,
	CONDITION_LEAF,
	CONDITION_NODE_END,
};

// ********* CTriggerData

CTriggerData::CTriggerData()
	: bActive(false)
	, bRun(false)
	, runCondition(_run_condition::run_attack_effect)
	, uID(0)
	, rootCondition(nullptr)
{
	std::memset(szName, 0, sizeof(szName));
}

CTriggerData::~CTriggerData() {}

bool CTriggerData::Load(FILE* pFile)
{
	uint32_t dwVersion;
	fread(&dwVersion, sizeof(uint32_t), 1, pFile);

	if (dwVersion == 0)
	{

		fread(&uID, sizeof(uint32_t), 1, pFile);
		fread(&bActive, sizeof(bool), 1, pFile);
		fread(&bRun, sizeof(bool), 1, pFile);
		fread(&runCondition, sizeof(char), 1, pFile);
		fread(szName, sizeof(char) * 128, 1, pFile);

		int32_t n = 0;

		// Reading condition data
		rootCondition = new _s_tree_item;
		ReadConditionTree(pFile, rootCondition);

		// Reading operation data
		fread(&n, sizeof(int32_t), 1, pFile);
		for (int32_t i = 0; i < n; ++i)
		{
			_s_operation* pTemp = new _s_operation;
			fread(&pTemp->iType, sizeof(int32_t), 1, pFile);
			switch ((_e_operation)pTemp->iType)
			{
			case _e_operation::o_attack:
				pTemp->pParam = new O_ATTACK_TYPE;
				fread(pTemp->pParam, sizeof(O_ATTACK_TYPE), 1, pFile);
				break;
			case _e_operation::o_use_skill:
				pTemp->pParam = new O_USE_SKILL;
				fread(pTemp->pParam, sizeof(O_USE_SKILL), 1, pFile);
				break;
			case _e_operation::o_talk:
				pTemp->pParam = new O_TALK_TEXT;
				fread(&((O_TALK_TEXT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
				((O_TALK_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2];
				fread(((O_TALK_TEXT*)pTemp->pParam)->szData, ((O_TALK_TEXT*)pTemp->pParam)->uSize, 1, pFile);
				break;
			case _e_operation::o_run_trigger:
				pTemp->pParam = new O_RUN_TRIGGER;
				fread(pTemp->pParam, sizeof(O_RUN_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_stop_trigger:
				pTemp->pParam = new O_STOP_TRIGGER;
				fread(pTemp->pParam, sizeof(O_STOP_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_active_trigger:
				pTemp->pParam = new O_ACTIVE_TRIGGER;
				fread(pTemp->pParam, sizeof(O_ACTIVE_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_create_timer:
				pTemp->pParam = new O_CREATE_TIMER;
				fread(pTemp->pParam, sizeof(O_CREATE_TIMER), 1, pFile);
				break;
			case _e_operation::o_kill_timer:
				pTemp->pParam = new O_KILL_TIMER;
				fread(pTemp->pParam, sizeof(O_KILL_TIMER), 1, pFile);
				break;
			case _e_operation::o_active_controller:
				pTemp->pParam = new O_ACTIVE_CONTROLLER;
				fread(&((O_ACTIVE_CONTROLLER*)pTemp->pParam)->uID, sizeof(uint32_t), 1, pFile);
				((O_ACTIVE_CONTROLLER*)pTemp->pParam)->bStop = false;
				break;
			case _e_operation::o_summon:
				pTemp->pParam = new O_SUMMON;
				fread(pTemp->pParam, sizeof(O_SUMMON), 1, pFile);
				break;
			case _e_operation::o_use_range_skill:
				pTemp->pParam = new O_USE_RANGE_SKILL;
				fread(pTemp->pParam, sizeof(O_USE_RANGE_SKILL), 1, pFile);
				break;
			default:
				pTemp->pParam = 0;
			}

			fread(&pTemp->mTarget.iType, sizeof(int32_t), 1, pFile);
			switch ((_e_target)pTemp->mTarget.iType)
			{
			case _e_target::t_occupation_list:
				pTemp->mTarget.pParam = new T_OCCUPATION;
				fread(pTemp->mTarget.pParam, sizeof(T_OCCUPATION), 1, pFile);
				break;
			default:
				pTemp->mTarget.pParam = nullptr;
			}

			listOperation.push_back(pTemp);
		}
	}
	else if (dwVersion >= 1)
	{
		fread(&uID, sizeof(uint32_t), 1, pFile);
		fread(&bActive, sizeof(bool), 1, pFile);
		fread(&bRun, sizeof(bool), 1, pFile);
		fread(&runCondition, sizeof(char), 1, pFile);
		fread(szName, sizeof(char) * 128, 1, pFile);

		int32_t n = 0;

		// Reading condition data
		rootCondition = new _s_tree_item;
		ReadConditionTree(pFile, rootCondition);

		// Reading operation data
		fread(&n, sizeof(int32_t), 1, pFile);
		for (int32_t i = 0; i < n; ++i)
		{
			_s_operation* pTemp = new _s_operation;
			fread(&pTemp->iType, sizeof(int32_t), 1, pFile);
			switch ((_e_operation)pTemp->iType)
			{
			case _e_operation::o_attack:
				pTemp->pParam = new O_ATTACK_TYPE;
				fread(pTemp->pParam, sizeof(O_ATTACK_TYPE), 1, pFile);
				break;
			case _e_operation::o_use_skill:
				pTemp->pParam = new O_USE_SKILL;
				fread(pTemp->pParam, sizeof(O_USE_SKILL), 1, pFile);
				break;
			case _e_operation::o_talk:
				pTemp->pParam = new O_TALK_TEXT;
				fread(&((O_TALK_TEXT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
				((O_TALK_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2 + 1];
				fread(((O_TALK_TEXT*)pTemp->pParam)->szData, ((O_TALK_TEXT*)pTemp->pParam)->uSize, 1, pFile);
				((O_TALK_TEXT*)pTemp->pParam)->szData[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			case _e_operation::o_run_trigger:
				pTemp->pParam = new O_RUN_TRIGGER;
				fread(pTemp->pParam, sizeof(O_RUN_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_stop_trigger:
				pTemp->pParam = new O_STOP_TRIGGER;
				fread(pTemp->pParam, sizeof(O_STOP_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_active_trigger:
				pTemp->pParam = new O_ACTIVE_TRIGGER;
				fread(pTemp->pParam, sizeof(O_ACTIVE_TRIGGER), 1, pFile);
				break;
			case _e_operation::o_create_timer:
				pTemp->pParam = new O_CREATE_TIMER;
				fread(pTemp->pParam, sizeof(O_CREATE_TIMER), 1, pFile);
				break;
			case _e_operation::o_kill_timer:
				pTemp->pParam = new O_KILL_TIMER;
				fread(pTemp->pParam, sizeof(O_KILL_TIMER), 1, pFile);
				break;
			case _e_operation::o_active_controller:
				pTemp->pParam = new O_ACTIVE_CONTROLLER;
				fread(pTemp->pParam, sizeof(O_ACTIVE_CONTROLLER), 1, pFile);
				break;
			case _e_operation::o_summon:
				pTemp->pParam = new O_SUMMON;
				fread(pTemp->pParam, sizeof(O_SUMMON), 1, pFile);
				break;
			case _e_operation::o_trigger_task:
				pTemp->pParam = new O_TRIGGER_TASK;
				fread(pTemp->pParam, sizeof(O_TRIGGER_TASK), 1, pFile);
				break;
			case _e_operation::o_change_path:
				pTemp->pParam = new O_CHANGE_PATH;
				if (dwVersion < 9)
				{
					fread(pTemp->pParam, sizeof(O_CHANGE_PATH_VERSION8), 1, pFile);
					((O_CHANGE_PATH*)(pTemp->pParam))->iType = 0;
				}
				else fread(pTemp->pParam, sizeof(O_CHANGE_PATH), 1, pFile);
				break;
			case _e_operation::o_sneer_monster:
				pTemp->pParam = new O_SNEER_MONSTER;
				fread(pTemp->pParam, sizeof(O_SNEER_MONSTER), 1, pFile);
				break;
			case _e_operation::o_use_range_skill:
				pTemp->pParam = new O_USE_RANGE_SKILL;
				fread(pTemp->pParam, sizeof(O_USE_RANGE_SKILL), 1, pFile);
				break;
			case _e_operation::o_set_global:
				pTemp->pParam = new O_SET_GLOBAL;
				fread(pTemp->pParam, sizeof(O_SET_GLOBAL), 1, pFile);
				break;
			case _e_operation::o_revise_global:
				pTemp->pParam = new O_REVISE_GLOBAL;
				fread(pTemp->pParam, sizeof(O_REVISE_GLOBAL), 1, pFile);
				break;
			case _e_operation::o_assign_global:
				pTemp->pParam = new O_ASSIGN_GLOBAL;
				fread(pTemp->pParam, sizeof(O_ASSIGN_GLOBAL), 1, pFile);
				break;
			case _e_operation::o_summon_mineral:
				pTemp->pParam = new O_SUMMON_MINERAL;
				fread(pTemp->pParam, sizeof(O_SUMMON_MINERAL), 1, pFile);
				break;
			case _e_operation::o_drop_item:
				pTemp->pParam = new O_DROP_ITEM;
				fread(pTemp->pParam, sizeof(O_DROP_ITEM), 1, pFile);
				break;
			case _e_operation::o_change_hate:
				pTemp->pParam = new O_CHANGE_HATE;
				fread(pTemp->pParam, sizeof(O_CHANGE_HATE), 1, pFile);
				break;
			case _e_operation::o_start_event:
				pTemp->pParam = new O_START_EVENT;
				fread(pTemp->pParam, sizeof(O_START_EVENT), 1, pFile);
				break;
			case _e_operation::o_stop_event:
				pTemp->pParam = new O_STOP_EVENT;
				fread(pTemp->pParam, sizeof(O_STOP_EVENT), 1, pFile);
				break;
			case _e_operation::o_drop_item_new:
				pTemp->pParam = new O_DROP_ITEMNEW;
				fread(pTemp->pParam, sizeof(O_DROP_ITEMNEW), 1, pFile);
				break;
			case _e_operation::o_whisper:
				pTemp->pParam = new O_WHISPER_TEXT;
				fread(&((O_WHISPER_TEXT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
				((O_WHISPER_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_WHISPER_TEXT*)pTemp->pParam)->uSize / 2 + 1];
				fread(((O_WHISPER_TEXT*)pTemp->pParam)->szData, ((O_WHISPER_TEXT*)pTemp->pParam)->uSize, 1, pFile);
				((O_WHISPER_TEXT*)pTemp->pParam)->szData[((O_WHISPER_TEXT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			case _e_operation::o_talk_portrait:
				pTemp->pParam = new O_TALK_PORTRAIT;
				fread(&((O_TALK_PORTRAIT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
				((O_TALK_PORTRAIT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_PORTRAIT*)pTemp->pParam)->uSize / 2 + 1];
				fread(((O_TALK_PORTRAIT*)pTemp->pParam)->szData, ((O_TALK_PORTRAIT*)pTemp->pParam)->uSize, 1, pFile);
				((O_TALK_PORTRAIT*)pTemp->pParam)->szData[((O_TALK_PORTRAIT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			default:
				pTemp->pParam = 0;
			}

			fread(&pTemp->mTarget.iType, sizeof(int32_t), 1, pFile);
			switch ((_e_target)pTemp->mTarget.iType)
			{
			case _e_target::t_occupation_list:
				pTemp->mTarget.pParam = new T_OCCUPATION;
				fread(pTemp->mTarget.pParam, sizeof(T_OCCUPATION), 1, pFile);
				break;
			default:
				pTemp->mTarget.pParam = nullptr;
			}

			listOperation.push_back(pTemp);
		}
	}

	return true;
}

bool CTriggerData::Save(FILE* pFile)
{
	uint32_t dwVersion = F_TRIGGER_VERSION;
	fwrite(&dwVersion, sizeof(uint32_t), 1, pFile);
	fwrite(&uID, sizeof(uint32_t), 1, pFile);
	fwrite(&bActive, sizeof(bool), 1, pFile);
	fwrite(&bRun, sizeof(bool), 1, pFile);
	fwrite(&runCondition, sizeof(char), 1, pFile);
	fwrite(szName, sizeof(char) * 128, 1, pFile);

	size_t n;

	// Writing condition data(tree)
	SaveConditionTree(pFile, rootCondition);

	// Writing operation data
	n = listOperation.size();
	fwrite(&n, sizeof(int32_t), 1, pFile);
	for (size_t i = 0; i < n; ++i)
	{
		_s_operation* pTemp = (_s_operation*)listOperation[i];
		fwrite(&pTemp->iType, sizeof(int32_t), 1, pFile);
		switch ((_e_operation)pTemp->iType)
		{
		case _e_operation::o_attack:
			fwrite(pTemp->pParam, sizeof(O_ATTACK_TYPE), 1, pFile);
			break;
		case _e_operation::o_use_skill:
			fwrite(pTemp->pParam, sizeof(O_USE_SKILL), 1, pFile);
			break;
		case _e_operation::o_use_range_skill:
			fwrite(pTemp->pParam, sizeof(O_USE_RANGE_SKILL), 1, pFile);
			break;
		case _e_operation::o_talk:
			fwrite(&((O_TALK_TEXT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
			fwrite(((O_TALK_TEXT*)pTemp->pParam)->szData, ((O_TALK_TEXT*)pTemp->pParam)->uSize, 1, pFile);
			break;
		case _e_operation::o_reset_hate_list:
			break;
		case _e_operation::o_run_trigger:
			fwrite(pTemp->pParam, sizeof(O_RUN_TRIGGER), 1, pFile);
			break;
		case _e_operation::o_stop_trigger:
			fwrite(pTemp->pParam, sizeof(O_STOP_TRIGGER), 1, pFile);
			break;
		case _e_operation::o_active_trigger:
			fwrite(pTemp->pParam, sizeof(O_ACTIVE_TRIGGER), 1, pFile);
			break;
		case _e_operation::o_create_timer:
			fwrite(pTemp->pParam, sizeof(O_CREATE_TIMER), 1, pFile);
			break;
		case _e_operation::o_kill_timer:
			fwrite(pTemp->pParam, sizeof(O_KILL_TIMER), 1, pFile);
			break;
		case _e_operation::o_active_controller:
			fwrite(pTemp->pParam, sizeof(O_ACTIVE_CONTROLLER), 1, pFile);
			break;
		case _e_operation::o_summon:
			fwrite(pTemp->pParam, sizeof(O_SUMMON), 1, pFile);
			break;
		case _e_operation::o_trigger_task:
			fwrite(pTemp->pParam, sizeof(O_TRIGGER_TASK), 1, pFile);
			break;
		case _e_operation::o_change_path:
			fwrite(pTemp->pParam, sizeof(O_CHANGE_PATH), 1, pFile);
			break;
		case _e_operation::o_sneer_monster:
			fwrite(pTemp->pParam, sizeof(O_SNEER_MONSTER), 1, pFile);
			break;
		case _e_operation::o_set_global:
			fwrite(pTemp->pParam, sizeof(O_SET_GLOBAL), 1, pFile);
			break;
		case _e_operation::o_revise_global:
			fwrite(pTemp->pParam, sizeof(O_REVISE_GLOBAL), 1, pFile);
			break;
		case _e_operation::o_assign_global:
			fwrite(pTemp->pParam, sizeof(O_ASSIGN_GLOBAL), 1, pFile);
			break;
		case _e_operation::o_summon_mineral:
			fwrite(pTemp->pParam, sizeof(O_SUMMON_MINERAL), 1, pFile);
			break;
		case _e_operation::o_drop_item:
			fwrite(pTemp->pParam, sizeof(O_DROP_ITEM), 1, pFile);
			break;
		case _e_operation::o_change_hate:
			fwrite(pTemp->pParam, sizeof(O_CHANGE_HATE), 1, pFile);
			break;
		case _e_operation::o_start_event:
			fwrite(pTemp->pParam, sizeof(O_START_EVENT), 1, pFile);
			break;
		case _e_operation::o_stop_event:
			fwrite(pTemp->pParam, sizeof(O_STOP_EVENT), 1, pFile);
			break;
		case _e_operation::o_drop_item_new:
			fwrite(pTemp->pParam, sizeof(O_DROP_ITEMNEW), 1, pFile);
			break;
		case _e_operation::o_whisper:
			fwrite(&((O_WHISPER_TEXT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
			fwrite(((O_WHISPER_TEXT*)pTemp->pParam)->szData, ((O_WHISPER_TEXT*)pTemp->pParam)->uSize, 1, pFile);
			break;
		case _e_operation::o_talk_portrait:
			fwrite(&((O_TALK_PORTRAIT*)pTemp->pParam)->uSize, sizeof(uint32_t), 1, pFile);
			fwrite(((O_TALK_PORTRAIT*)pTemp->pParam)->szData, ((O_TALK_PORTRAIT*)pTemp->pParam)->uSize, 1, pFile);
			break;
		}

		fwrite(&pTemp->mTarget.iType, sizeof(int32_t), 1, pFile);
		switch ((_e_target)pTemp->mTarget.iType)
		{
		case _e_target::t_occupation_list:
			fwrite(pTemp->mTarget.pParam, sizeof(T_OCCUPATION), 1, pFile);
			break;
		}
	}

	return true;
}

void CTriggerData::Release()
{
	ReleaseConditionTree();

	size_t n = listOperation.size();
	for (size_t i = 0; i < n; ++i)
	{
		if (listOperation[i]->pParam)
		{
			if (listOperation[i]->iType == (int32_t)_e_operation::o_talk)
			{
				if (((O_TALK_TEXT*)listOperation[i]->pParam)->szData)
				{
					delete ((O_TALK_TEXT*)listOperation[i]->pParam)->szData;
					((O_TALK_TEXT*)listOperation[i]->pParam)->szData = nullptr;
				}
			}

			if (listOperation[i]->iType == (int32_t)_e_operation::o_whisper)
			{
				if (((O_WHISPER_TEXT*)listOperation[i]->pParam)->szData)
				{
					delete ((O_WHISPER_TEXT*)listOperation[i]->pParam)->szData;
					((O_WHISPER_TEXT*)listOperation[i]->pParam)->szData = nullptr;

				}
			}

			if (listOperation[i]->iType == (int32_t)_e_operation::o_talk_portrait)
			{
				if (((O_TALK_PORTRAIT*)listOperation[i]->pParam)->szData)
				{
					delete ((O_TALK_PORTRAIT*)listOperation[i]->pParam)->szData;
					((O_TALK_PORTRAIT*)listOperation[i]->pParam)->szData = nullptr;
				}
			}

			delete listOperation[i]->pParam;
			listOperation[i]->pParam = nullptr;
		}

		if (listOperation[i]->mTarget.pParam)
		{
			delete listOperation[i]->mTarget.pParam;
			listOperation[i]->mTarget.pParam = nullptr;
		}

		delete listOperation[i];
		listOperation[i] = nullptr;
	}
}

CTriggerData* CTriggerData::CopyObject()
{
	CTriggerData* pNewObject = new CTriggerData;

	if (pNewObject == 0)
		return 0;

	strcpy(pNewObject->szName, szName);

	pNewObject->bActive = bActive;
	pNewObject->bRun = bRun;
	pNewObject->runCondition = runCondition;
	pNewObject->uID = uID;
	pNewObject->rootCondition = CopyConditionTree(rootCondition);

	for (size_t i = 0; i < listOperation.size(); ++i)
	{
		_s_operation* pNewOperation = CopyOperation(listOperation[i]);
		pNewObject->AddOperation(pNewOperation);
	}

	return pNewObject;
}

void CTriggerData::ReleaseConditionTree()
{
	if (rootCondition)
	{
		delete rootCondition;
		rootCondition = nullptr;
	}
}

CTriggerData::_s_tree_item* CTriggerData::CopyConditionTree(_s_tree_item* pRoot)
{
	if (pRoot == nullptr)
		return nullptr;

	CTriggerData::_s_tree_item* pNew = new CTriggerData::_s_tree_item;

	if (pNew == nullptr)
		return nullptr;

	pNew->mCondition.iType = pRoot->mCondition.iType;
	pNew->mCondition.pParam = nullptr;
	pNew->pLeft = nullptr;
	pNew->pRight = nullptr;

	if (pRoot->mCondition.pParam != nullptr)
	{
		switch ((_e_condition)pRoot->mCondition.iType)
		{
		case _e_condition::c_time_come:
			pNew->mCondition.pParam = new C_TIME_COME;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_TIME_COME));
			break;
		case _e_condition::c_hp_less:
			pNew->mCondition.pParam = new C_HP_LESS;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_HP_LESS));
			break;
		case _e_condition::c_random:
			pNew->mCondition.pParam = new C_RANDOM;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_RANDOM));
			break;
		case _e_condition::c_path_end_point:
			pNew->mCondition.pParam = new C_PATH_END_POINT;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_PATH_END_POINT));
			break;
		case _e_condition::c_enmity_reach:
			pNew->mCondition.pParam = new C_ENMITY_REACH;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_ENMITY_REACH));
			break;
		case _e_condition::c_distance_reach:
			pNew->mCondition.pParam = new C_DISTANCE_REACH;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_DISTANCE_REACH));
			break;
		case _e_condition::c_var:
			pNew->mCondition.pParam = new C_VAR;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_VAR));
			break;
		case _e_condition::c_constant:
			pNew->mCondition.pParam = new C_CONSTANT;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_CONSTANT));
			break;
		case _e_condition::c_rank_level:
			pNew->mCondition.pParam = new C_RANK_LEVEL;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_RANK_LEVEL));
			break;
		case _e_condition::c_attack_by_skill:
			pNew->mCondition.pParam = new C_ATTACK_BY_SKILL;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_ATTACK_BY_SKILL));
			break;
		case _e_condition::c_random_selectone:
			pNew->mCondition.pParam = new C_RANDOM_SELECTONE;
			memcpy(pNew->mCondition.pParam, pRoot->mCondition.pParam, sizeof(C_RANDOM_SELECTONE));
			break;
		default:
			break;
		}
	}

	if (pRoot->pLeft)
		pNew->pLeft = CTriggerData::CopyConditionTree(pRoot->pLeft);

	if (pRoot->pRight)
		pNew->pRight = CTriggerData::CopyConditionTree(pRoot->pRight);

	return pNew;
}

CTriggerData::_s_operation* CTriggerData::CopyOperation(_s_operation* pOperation)
{
	uint32_t len;
	_s_operation* pNewObject = new _s_operation;

	if (pNewObject == 0)
		return 0;

	pNewObject->iType = pOperation->iType;
	pNewObject->mTarget.iType = pOperation->mTarget.iType;

	switch ((_e_operation)pNewObject->iType)
	{
	case _e_operation::o_attack:
		pNewObject->pParam = new O_ATTACK_TYPE;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_ATTACK_TYPE));
		break;
	case _e_operation::o_use_skill:
		pNewObject->pParam = new O_USE_SKILL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_USE_SKILL));
		break;
	case _e_operation::o_talk:
		pNewObject->pParam = new O_TALK_TEXT;
		len = ((O_TALK_TEXT*)pOperation->pParam)->uSize;
		((O_TALK_TEXT*)pNewObject->pParam)->uSize = len;
		((O_TALK_TEXT*)pNewObject->pParam)->szData = new uint16_t[len / 2];
		memcpy(((O_TALK_TEXT*)pNewObject->pParam)->szData, ((O_TALK_TEXT*)pOperation->pParam)->szData, len);
		break;
	case _e_operation::o_reset_hate_list:
		pNewObject->pParam = 0;
		break;
	case _e_operation::o_run_trigger:
		pNewObject->pParam = new O_RUN_TRIGGER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_RUN_TRIGGER));
		break;
	case _e_operation::o_stop_trigger:
		pNewObject->pParam = new O_STOP_TRIGGER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_STOP_TRIGGER));
		break;
	case _e_operation::o_active_trigger:
		pNewObject->pParam = new O_ACTIVE_TRIGGER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_ACTIVE_TRIGGER));
		break;
	case _e_operation::o_create_timer:
		pNewObject->pParam = new O_CREATE_TIMER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_CREATE_TIMER));
		break;
	case _e_operation::o_kill_timer:
		pNewObject->pParam = new O_KILL_TIMER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_KILL_TIMER));
		break;
	case _e_operation::o_flee:
	case _e_operation::o_set_hate_to_first:
	case _e_operation::o_set_hate_to_last:
	case _e_operation::o_set_hate_fifty_percent:
	case _e_operation::o_skip_operation:
		pNewObject->pParam = nullptr;
		break;
	case _e_operation::o_active_controller:
		pNewObject->pParam = new O_ACTIVE_CONTROLLER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_ACTIVE_CONTROLLER));
		break;
	case _e_operation::o_summon:
		pNewObject->pParam = new O_SUMMON;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_SUMMON));
		break;
	case _e_operation::o_trigger_task:
		pNewObject->pParam = new O_TRIGGER_TASK;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_TRIGGER_TASK));
		break;
	case _e_operation::o_change_path:
		pNewObject->pParam = new O_CHANGE_PATH;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_CHANGE_PATH));
		break;
	case _e_operation::o_disappear:
		pNewObject->pParam = nullptr;
		break;
	case _e_operation::o_sneer_monster:
		pNewObject->pParam = new O_SNEER_MONSTER;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_SNEER_MONSTER));
		break;
	case _e_operation::o_use_range_skill:
		pNewObject->pParam = new O_USE_RANGE_SKILL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_USE_RANGE_SKILL));
		break;
	case _e_operation::o_reset:
		pNewObject->pParam = nullptr;
		break;
	case _e_operation::o_set_global:
		pNewObject->pParam = new O_SET_GLOBAL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_SET_GLOBAL));
		break;
	case _e_operation::o_revise_global:
		pNewObject->pParam = new O_REVISE_GLOBAL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_REVISE_GLOBAL));
		break;
	case _e_operation::o_assign_global:
		pNewObject->pParam = new O_ASSIGN_GLOBAL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_ASSIGN_GLOBAL));
		break;
	case _e_operation::o_summon_mineral:
		pNewObject->pParam = new O_SUMMON_MINERAL;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_SUMMON_MINERAL));
		break;
	case _e_operation::o_drop_item:
		pNewObject->pParam = new O_DROP_ITEM;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_DROP_ITEM));
		break;
	case _e_operation::o_change_hate:
		pNewObject->pParam = new O_CHANGE_HATE;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_CHANGE_HATE));
		break;
	case _e_operation::o_start_event:
		pNewObject->pParam = new O_START_EVENT;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_START_EVENT));
		break;
	case _e_operation::o_stop_event:
		pNewObject->pParam = new O_STOP_EVENT;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_STOP_EVENT));
		break;
	case _e_operation::o_drop_item_new:
		pNewObject->pParam = new O_DROP_ITEMNEW;
		memcpy(pNewObject->pParam, pOperation->pParam, sizeof(O_DROP_ITEMNEW));
		break;
	case _e_operation::o_whisper:
		pNewObject->pParam = new O_WHISPER_TEXT;
		len = ((O_WHISPER_TEXT*)pOperation->pParam)->uSize;
		((O_WHISPER_TEXT*)pNewObject->pParam)->uSize = len;
		((O_WHISPER_TEXT*)pNewObject->pParam)->szData = new uint16_t[len / 2];
		memcpy(((O_WHISPER_TEXT*)pNewObject->pParam)->szData, ((O_WHISPER_TEXT*)pOperation->pParam)->szData, len);
		break;
	case _e_operation::o_talk_portrait:
		pNewObject->pParam = new O_TALK_PORTRAIT;
		len = ((O_TALK_PORTRAIT*)pOperation->pParam)->uSize;
		((O_TALK_PORTRAIT*)pNewObject->pParam)->uSize = len;
		((O_TALK_PORTRAIT*)pNewObject->pParam)->szData = new uint16_t[len / 2];
		memcpy(((O_TALK_PORTRAIT*)pNewObject->pParam)->szData, ((O_TALK_PORTRAIT*)pOperation->pParam)->szData, len);
		break;
	default:
		pNewObject->pParam = nullptr;
		pNewObject->iType = 0;
		break;
	}

	if (pNewObject->mTarget.iType == (int32_t)_e_target::t_occupation_list)
	{
		pNewObject->mTarget.pParam = new T_OCCUPATION;
		memcpy(pNewObject->mTarget.pParam, pOperation->mTarget.pParam, sizeof(T_OCCUPATION));
	}
	else
		pNewObject->mTarget.pParam = nullptr;

	return pNewObject;
}

void CTriggerData::AddOperation(uint32_t iType, void* pParam, const _s_target* pTarget)
{
	_s_operation* pNew = new _s_operation;

	pNew->iType = iType;
	pNew->pParam = pParam;
	pNew->mTarget.iType = pTarget->iType;
	pNew->mTarget.pParam = pTarget->pParam;

	listOperation.push_back(pNew);
}

void CTriggerData::AddOperation(_s_operation* pOperation)
{
	if (pOperation == nullptr)
		return;

	listOperation.push_back(pOperation);
}

uint32_t CTriggerData::GetOperation(uint32_t idx, uint32_t& iType, void** ppData, _s_target& target)
{
	if (idx > listOperation.size())
		return -1;

	iType = listOperation[idx]->iType;
	*ppData = listOperation[idx]->pParam;
	target = listOperation[idx]->mTarget;

	return iType;
}

void CTriggerData::SetOperation(uint32_t idx, uint32_t iType, void* pData, const _s_target* pTarget)
{
	if (idx > listOperation.size())
		return;

	listOperation[idx]->iType = iType;
	listOperation[idx]->pParam = pData;
	listOperation[idx]->mTarget = *pTarget;
}

void CTriggerData::DelOperation(uint32_t idx)
{
	if (idx > listOperation.size())
		return;

	if (listOperation[idx]->pParam)
	{
		delete listOperation[idx]->pParam;
		listOperation[idx]->pParam = nullptr;
	}

	listOperation.erase(listOperation.begin() + idx);
}

bool CTriggerData::ReadConditionTree(FILE* pFile, _s_tree_item* pNode)
{
	if (pNode == nullptr)
		return true;

	int32_t dat_size = 0;
	int32_t flag = 0;
	fread(&pNode->mCondition.iType, sizeof(int32_t), 1, pFile);
	fread(&dat_size, sizeof(int32_t), 1, pFile);

	if (dat_size != 0)
	{
		pNode->mCondition.pParam = new char[dat_size];
		fread(pNode->mCondition.pParam, dat_size, 1, pFile);
	}
	else
		pNode->mCondition.pParam = nullptr;

	while (true)
	{
		fread(&flag, sizeof(int32_t), 1, pFile);
		if (flag == CONDITION_LEAF)
		{
			pNode->pLeft = nullptr;
			pNode->pRight = nullptr;
			break;
		}
		else if (flag == CONDITION_LEFT_SUB)
		{
			pNode->pLeft = new _s_tree_item;
			ReadConditionTree(pFile, pNode->pLeft);
		}
		else if (flag == CONDITION_RIGHT_SUB)
		{
			pNode->pRight = new _s_tree_item;
			ReadConditionTree(pFile, pNode->pRight);
		}
		else if (flag == CONDITION_NODE_END)
			break;
	}

	return true;
}

bool CTriggerData::SaveConditionTree(FILE* pFile, const _s_tree_item* pNode)
{
	if (pNode == nullptr)
		return true;

	int32_t dat_size = 0;
	int32_t flag = 0;
	fwrite(&pNode->mCondition.iType, sizeof(int32_t), 1, pFile);

	if (pNode->mCondition.pParam == nullptr)
		fwrite(&dat_size, sizeof(int32_t), 1, pFile);
	else
	{
		switch ((_e_condition)pNode->mCondition.iType)
		{
		case _e_condition::c_time_come:
			dat_size = sizeof(C_TIME_COME);
			break;
		case _e_condition::c_hp_less:
			dat_size = sizeof(C_HP_LESS);
			break;
		case _e_condition::c_random:
			dat_size = sizeof(C_RANDOM);
			break;
		case _e_condition::c_path_end_point:
			dat_size = sizeof(C_PATH_END_POINT);
			break;
		case _e_condition::c_enmity_reach:
			dat_size = sizeof(C_ENMITY_REACH);
			break;
		case _e_condition::c_distance_reach:
			dat_size = sizeof(C_DISTANCE_REACH);
			break;
		case _e_condition::c_var:
			dat_size = sizeof(C_VAR);
			break;
		case _e_condition::c_constant:
			dat_size = sizeof(C_CONSTANT);
			break;
		case _e_condition::c_rank_level:
			dat_size = sizeof(C_RANK_LEVEL);
			break;
		case _e_condition::c_attack_by_skill:
			dat_size = sizeof(C_ATTACK_BY_SKILL);
			break;
		case _e_condition::c_random_selectone:
			dat_size = sizeof(C_RANDOM_SELECTONE);
			break;
		default:
			break;
		}

		fwrite(&dat_size, sizeof(int32_t), 1, pFile);
		fwrite(pNode->mCondition.pParam, dat_size, 1, pFile);
	}

	if (pNode->pLeft == nullptr && pNode->pRight == nullptr)
	{
		flag = CONDITION_LEAF;
		fwrite(&flag, sizeof(int32_t), 1, pFile);
		return true;
	}

	if (pNode->pLeft)
	{
		flag = CONDITION_LEFT_SUB;
		fwrite(&flag, sizeof(int32_t), 1, pFile);
		SaveConditionTree(pFile, pNode->pLeft);
	}

	if (pNode->pRight)
	{
		flag = CONDITION_RIGHT_SUB;
		fwrite(&flag, sizeof(int32_t), 1, pFile);
		SaveConditionTree(pFile, pNode->pRight);
	}

	flag = CONDITION_NODE_END;
	fwrite(&flag, sizeof(int32_t), 1, pFile);

	return true;
}

// ********* CPolicyData

CPolicyData::CPolicyData() :uID(0) {}
CPolicyData::~CPolicyData() {}

bool CPolicyData::Load(const char* path)
{
	FILE* pFile = fopen(path, "rb");
	if (pFile == nullptr)
		return false;

	return Load(pFile);
}

bool CPolicyData::Load(const wchar_t* path)
{
	FILE* pFile = _wfopen(path, L"rb");
	if (pFile == nullptr)
		return false;

	return Load(pFile);
}

bool CPolicyData::Load(const std::string& path)
{
	return Load(path.c_str());
}

bool CPolicyData::Load(const std::wstring& path)
{
	return Load(path.c_str());
}

bool CPolicyData::Save(const char* path)
{
	FILE* pFile = fopen(path, "wb");
	if (pFile == nullptr)
		return false;

	return Save(pFile);
}

bool CPolicyData::Save(const wchar_t* path)
{
	FILE* pFile = _wfopen(path, L"wb");
	if (pFile == nullptr)
		return false;

	return Save(pFile);
}

bool CPolicyData::Save(const std::string& path)
{
	return Save(path.c_str());
}

bool CPolicyData::Save(const std::wstring& path)
{
	return Save(path.c_str());
}

bool CPolicyData::Load(FILE* pFile)
{
	uint32_t nVersion;
	fread(&nVersion, sizeof(uint32_t), 1, pFile);

	if (nVersion == F_POLICY_VERSION)
	{
		int32_t n = 0;
		fread(&uID, sizeof(uint32_t), 1, pFile);
		fread(&n, sizeof(int32_t), 1, pFile);

		for (int32_t i = 0; i < n; i++)
		{
			CTriggerData* pNew = new CTriggerData;
			pNew->Load(pFile);
			listTriggerPtr.push_back(pNew);
		}
	}
	else
		return false;

	return true;
}

bool CPolicyData::Save(FILE* pFile)
{
	uint32_t nVersion = F_POLICY_VERSION;
	fwrite(&nVersion, sizeof(uint32_t), 1, pFile);
	fwrite(&uID, sizeof(uint32_t), 1, pFile);
	size_t n = listTriggerPtr.size();
	fwrite(&n, sizeof(int32_t), 1, pFile);

	for (size_t i = 0; i < n; i++)
		listTriggerPtr[i]->Save(pFile);

	fclose(pFile);
	return true;
}

void CPolicyData::Release()
{
	size_t n = listTriggerPtr.size();
	for (size_t i = 0; i < n; ++i)
	{
		listTriggerPtr[i]->Release();
		delete listTriggerPtr[i];
		listTriggerPtr[i] = nullptr;
	}
}

int32_t CPolicyData::GetIndex(uint32_t id)
{
	int32_t n = (int32_t)listTriggerPtr.size();
	for (int32_t i = 0; i < n; ++i)
	{
		if (listTriggerPtr[i]->GetID() == id)
			return i;
	}

	return -1;
}

void CPolicyData::DelTriggerPtr(int32_t ndx)
{
	if ((size_t)ndx > listTriggerPtr.size())
		return;

	if (listTriggerPtr[ndx])
	{
		listTriggerPtr[ndx]->Release();
		CTriggerData** temp = &listTriggerPtr[ndx];
		delete listTriggerPtr[ndx];
		listTriggerPtr[ndx] = nullptr;
		listTriggerPtr.erase(listTriggerPtr.begin() + ndx);
	}
}

// ********* CPolicyDataManager

CPolicyDataManager::CPolicyDataManager() {}
CPolicyDataManager::~CPolicyDataManager() {}

bool CPolicyDataManager::Load(const char* path)
{
	FILE* pFile = fopen(path, "rb");

	if (pFile == nullptr)
		return false;

	uint32_t version;
	int32_t count;
	fread(&version, sizeof(uint32_t), 1, pFile);

	if (version != F_POLICY_EXP_VERSION)
	{
		fclose(pFile);
		return false;
	}

	fread(&count, sizeof(int32_t), 1, pFile);
	for (int32_t i = 0; i < count; ++i)
	{
		CPolicyData* pNew = new CPolicyData;

		if (pNew == 0 || !pNew->Load(pFile))
		{
			fclose(pFile);
			count = (int32_t)listPolicy.size();

			for (int32_t k = 0; k < count; ++k)
			{
				listPolicy[k]->Release();
				delete listPolicy[k];
				listPolicy[k] = nullptr;
			}

			listPolicy.clear();
			return false;
		}

		listPolicy.push_back(pNew);
	}

	fclose(pFile);
	return true;
}

bool CPolicyDataManager::Load(const wchar_t* path)
{
	FILE* pFile = _wfopen(path, L"rb");

	if (pFile == nullptr)
		return false;

	uint32_t version;
	int32_t count;
	fread(&version, sizeof(uint32_t), 1, pFile);

	if (version != F_POLICY_EXP_VERSION)
	{
		fclose(pFile);
		return false;
	}

	fread(&count, sizeof(int32_t), 1, pFile);
	for (int32_t i = 0; i < count; ++i)
	{
		CPolicyData* pNew = new CPolicyData;

		if (pNew == 0 || !pNew->Load(pFile))
		{
			fclose(pFile);
			count = (int32_t)listPolicy.size();

			for (int32_t k = 0; k < count; ++k)
			{
				listPolicy[k]->Release();
				delete listPolicy[k];
				listPolicy[k] = nullptr;
			}

			listPolicy.clear();
			return false;
		}

		listPolicy.push_back(pNew);
	}

	fclose(pFile);
	return true;
}

bool CPolicyDataManager::Load(const std::string& path)
{
	return Load(path.c_str());
}

bool CPolicyDataManager::Load(const std::wstring& path)
{
	return Load(path.c_str());
}

bool CPolicyDataManager::Save(const char* path)
{
	FILE* pFile = fopen(path, "wb");

	if (pFile == nullptr)
		return false;

	uint32_t uVersion = F_POLICY_EXP_VERSION;
	size_t count = listPolicy.size();
	fwrite(&uVersion, sizeof(uint32_t), 1, pFile);
	fwrite(&count, sizeof(int32_t), 1, pFile);

	for (size_t i = 0; i < count; ++i)
	{
		CPolicyData* pPolicy = listPolicy[i];

		if (!pPolicy->Save(pFile))
		{
			fclose(pFile);
			return false;
		}
	}

	fclose(pFile);
	return true;
}

bool CPolicyDataManager::Save(const wchar_t* path)
{
	FILE* pFile = _wfopen(path, L"wb");

	if (pFile == nullptr)
		return false;

	uint32_t uVersion = F_POLICY_EXP_VERSION;
	size_t count = listPolicy.size();
	fwrite(&uVersion, sizeof(uint32_t), 1, pFile);
	fwrite(&count, sizeof(int32_t), 1, pFile);

	for (size_t i = 0; i < count; ++i)
	{
		CPolicyData* pPolicy = listPolicy[i];

		if (!pPolicy->Save(pFile))
		{
			fclose(pFile);
			return false;
		}
	}

	fclose(pFile);
	return true;
}

bool CPolicyDataManager::Save(const std::string& path)
{
	return Save(path.c_str());
}

bool CPolicyDataManager::Save(const std::wstring& path)
{
	return Save(path.c_str());
}

void CPolicyDataManager::Release()
{
	size_t n = listPolicy.size();
	for (size_t i = 0; i < n; ++i)
	{
		listPolicy[i]->Release();
		delete listPolicy[i];
		listPolicy[i] = nullptr;
	}
}

size_t CPolicyDataManager::GetPolicyNum() const
{
	return listPolicy.size();
}

CPolicyData* CPolicyDataManager::GetPolicy(int32_t ndx)
{
	if (ndx >= 0 && ndx < (int32_t)listPolicy.size())
		return listPolicy[ndx];
	else
		return nullptr;
}
