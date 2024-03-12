#include "pch.h"
#include "Policy.h"
#include "PolicyType.h"

#include <fstream>

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
	, strName("")
{
	//std::memset(szName, 0, sizeof(szName));
}

CTriggerData::~CTriggerData() {}

bool CTriggerData::Load(std::ifstream& ifs)
{
	char c;
	int32_t count = 0;

	uint32_t dwVersion;
	ifs.read(reinterpret_cast<char*>(&dwVersion), sizeof(uint32_t));

	if (dwVersion == 0)
	{

		ifs.read(reinterpret_cast<char*>(&uID), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&bActive), sizeof(bool));
		ifs.read(reinterpret_cast<char*>(&bRun), sizeof(bool));
		ifs.read(reinterpret_cast<char*>(&runCondition), sizeof(char));
		
		while (count < 128 && ifs.get(c))
		{
			strName.push_back(c);
			count++;
		}

		int32_t n = 0;

		// Reading condition data
		rootCondition = new _s_tree_item;
		ReadConditionTree(ifs, rootCondition);

		// Reading operation data
		ifs.read(reinterpret_cast<char*>(&n), sizeof(int32_t));
		for (int32_t i = 0; i < n; ++i)
		{
			_s_operation* pTemp = new _s_operation;
			ifs.read(reinterpret_cast<char*>(&pTemp->iType), sizeof(int32_t));
			switch ((_e_operation)pTemp->iType)
			{
			case _e_operation::o_attack:
				pTemp->pParam = new O_ATTACK_TYPE;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ATTACK_TYPE));
				break;
			case _e_operation::o_use_skill:
				pTemp->pParam = new O_USE_SKILL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_USE_SKILL));
				break;
			case _e_operation::o_talk:
				pTemp->pParam = new O_TALK_TEXT;
				ifs.read(reinterpret_cast<char*>(&((O_TALK_TEXT*)pTemp->pParam)->uSize), sizeof(uint32_t));
				((O_TALK_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2 + 1];
				ifs.read(reinterpret_cast<char*>(((O_TALK_TEXT*)pTemp->pParam)->szData), ((O_TALK_TEXT*)pTemp->pParam)->uSize);
				break;
			case _e_operation::o_run_trigger:
				pTemp->pParam = new O_RUN_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_RUN_TRIGGER));
				break;
			case _e_operation::o_stop_trigger:
				pTemp->pParam = new O_STOP_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_STOP_TRIGGER));
				break;
			case _e_operation::o_active_trigger:
				pTemp->pParam = new O_ACTIVE_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ACTIVE_TRIGGER));
				break;
			case _e_operation::o_create_timer:
				pTemp->pParam = new O_CREATE_TIMER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_CREATE_TIMER));
				break;
			case _e_operation::o_kill_timer:
				pTemp->pParam = new O_KILL_TIMER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_KILL_TIMER));
				break;
			case _e_operation::o_active_controller:
				pTemp->pParam = new O_ACTIVE_CONTROLLER;
				ifs.read(reinterpret_cast<char*>(&((O_ACTIVE_CONTROLLER*)pTemp->pParam)->uID), sizeof(uint32_t));
				((O_ACTIVE_CONTROLLER*)pTemp->pParam)->bStop = false;
				break;
			case _e_operation::o_summon:
				pTemp->pParam = new O_SUMMON;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_SUMMON));
				break;
			case _e_operation::o_use_range_skill:
				pTemp->pParam = new O_USE_RANGE_SKILL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_USE_RANGE_SKILL));
				break;
			default:
				pTemp->pParam = 0;
			}

			ifs.read(reinterpret_cast<char*>(&pTemp->mTarget.iType), sizeof(int32_t));
			switch ((_e_target)pTemp->mTarget.iType)
			{
			case _e_target::t_occupation_list:
				pTemp->mTarget.pParam = new T_OCCUPATION;
				ifs.read(reinterpret_cast<char*>(pTemp->mTarget.pParam), sizeof(T_OCCUPATION));
				break;
			default:
				pTemp->mTarget.pParam = nullptr;
			}

			listOperation.push_back(pTemp);
		}
	}
	else if (dwVersion >= 1)
	{
		ifs.read(reinterpret_cast<char*>(&uID), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&bActive), sizeof(bool));
		ifs.read(reinterpret_cast<char*>(&bRun), sizeof(bool));
		ifs.read(reinterpret_cast<char*>(&runCondition), sizeof(char));
		
		while (count < 128 && ifs.get(c))
		{
			strName.push_back(c);
			count++;
		}

		int32_t n = 0;

		// Reading condition data
		rootCondition = new _s_tree_item;
		ReadConditionTree(ifs, rootCondition);

		// Reading operation data
		ifs.read(reinterpret_cast<char*>(&n), sizeof(int32_t));
		for (int32_t i = 0; i < n; ++i)
		{
			_s_operation* pTemp = new _s_operation;
			ifs.read(reinterpret_cast<char*>(&pTemp->iType), sizeof(int32_t));
			switch ((_e_operation)pTemp->iType)
			{
			case _e_operation::o_attack:
				pTemp->pParam = new O_ATTACK_TYPE;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ATTACK_TYPE));
				break;
			case _e_operation::o_use_skill:
				pTemp->pParam = new O_USE_SKILL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_USE_SKILL));
				break;
			case _e_operation::o_talk:
				pTemp->pParam = new O_TALK_TEXT;
				ifs.read(reinterpret_cast<char*>(&((O_TALK_TEXT*)pTemp->pParam)->uSize), sizeof(uint32_t));
				((O_TALK_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2 + 1];
				ifs.read(reinterpret_cast<char*>(((O_TALK_TEXT*)pTemp->pParam)->szData), ((O_TALK_TEXT*)pTemp->pParam)->uSize);
				((O_TALK_TEXT*)pTemp->pParam)->szData[((O_TALK_TEXT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			case _e_operation::o_run_trigger:
				pTemp->pParam = new O_RUN_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_RUN_TRIGGER));
				break;
			case _e_operation::o_stop_trigger:
				pTemp->pParam = new O_STOP_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_STOP_TRIGGER));
				break;
			case _e_operation::o_active_trigger:
				pTemp->pParam = new O_ACTIVE_TRIGGER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ACTIVE_TRIGGER));
				break;
			case _e_operation::o_create_timer:
				pTemp->pParam = new O_CREATE_TIMER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_CREATE_TIMER));
				break;
			case _e_operation::o_kill_timer:
				pTemp->pParam = new O_KILL_TIMER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_KILL_TIMER));
				break;
			case _e_operation::o_active_controller:
				pTemp->pParam = new O_ACTIVE_CONTROLLER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ACTIVE_CONTROLLER));
				break;
			case _e_operation::o_summon:
				pTemp->pParam = new O_SUMMON;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_SUMMON));
				break;
			case _e_operation::o_trigger_task:
				pTemp->pParam = new O_TRIGGER_TASK;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_TRIGGER_TASK));
				break;
			case _e_operation::o_change_path:
				pTemp->pParam = new O_CHANGE_PATH;
				if (dwVersion < 9)
				{
					ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_CHANGE_PATH_VERSION8));
					((O_CHANGE_PATH*)(pTemp->pParam))->iType = 0;
				}
				else
					ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_CHANGE_PATH));
				break;
			case _e_operation::o_sneer_monster:
				pTemp->pParam = new O_SNEER_MONSTER;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_SNEER_MONSTER));
				break;
			case _e_operation::o_use_range_skill:
				pTemp->pParam = new O_USE_RANGE_SKILL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_USE_RANGE_SKILL));
				break;
			case _e_operation::o_set_global:
				pTemp->pParam = new O_SET_GLOBAL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_SET_GLOBAL));
				break;
			case _e_operation::o_revise_global:
				pTemp->pParam = new O_REVISE_GLOBAL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_REVISE_GLOBAL));
				break;
			case _e_operation::o_assign_global:
				pTemp->pParam = new O_ASSIGN_GLOBAL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_ASSIGN_GLOBAL));
				break;
			case _e_operation::o_summon_mineral:
				pTemp->pParam = new O_SUMMON_MINERAL;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_SUMMON_MINERAL));
				break;
			case _e_operation::o_drop_item:
				pTemp->pParam = new O_DROP_ITEM;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_DROP_ITEM));
				break;
			case _e_operation::o_change_hate:
				pTemp->pParam = new O_CHANGE_HATE;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_CHANGE_HATE));
				break;
			case _e_operation::o_start_event:
				pTemp->pParam = new O_START_EVENT;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_START_EVENT));
				break;
			case _e_operation::o_stop_event:
				pTemp->pParam = new O_STOP_EVENT;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_STOP_EVENT));
				break;
			case _e_operation::o_drop_item_new:
				pTemp->pParam = new O_DROP_ITEMNEW;
				ifs.read(reinterpret_cast<char*>(pTemp->pParam), sizeof(O_DROP_ITEMNEW));
				break;
			case _e_operation::o_whisper:
				pTemp->pParam = new O_WHISPER_TEXT;
				ifs.read(reinterpret_cast<char*>(&((O_WHISPER_TEXT*)pTemp->pParam)->uSize), sizeof(uint32_t));
				((O_WHISPER_TEXT*)pTemp->pParam)->szData = new uint16_t[((O_WHISPER_TEXT*)pTemp->pParam)->uSize / 2 + 1];
				ifs.read(reinterpret_cast<char*>(((O_WHISPER_TEXT*)pTemp->pParam)->szData), ((O_WHISPER_TEXT*)pTemp->pParam)->uSize);
				((O_WHISPER_TEXT*)pTemp->pParam)->szData[((O_WHISPER_TEXT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			case _e_operation::o_talk_portrait:
				pTemp->pParam = new O_TALK_PORTRAIT;
				ifs.read(reinterpret_cast<char*>(&((O_TALK_PORTRAIT*)pTemp->pParam)->uSize), sizeof(uint32_t));
				((O_TALK_PORTRAIT*)pTemp->pParam)->szData = new uint16_t[((O_TALK_PORTRAIT*)pTemp->pParam)->uSize / 2 + 1];
				ifs.read(reinterpret_cast<char*>(((O_TALK_PORTRAIT*)pTemp->pParam)->szData), ((O_TALK_PORTRAIT*)pTemp->pParam)->uSize);
				((O_TALK_PORTRAIT*)pTemp->pParam)->szData[((O_TALK_PORTRAIT*)pTemp->pParam)->uSize / 2] = 0;
				break;
			default:
				pTemp->pParam = 0;
			}

			ifs.read(reinterpret_cast<char*>(&pTemp->mTarget.iType), sizeof(int32_t));
			switch ((_e_target)pTemp->mTarget.iType)
			{
			case _e_target::t_occupation_list:
				pTemp->mTarget.pParam = new T_OCCUPATION;
				ifs.read(reinterpret_cast<char*>(pTemp->mTarget.pParam), sizeof(T_OCCUPATION));
				break;
			default:
				pTemp->mTarget.pParam = nullptr;
			}

			listOperation.push_back(pTemp);
		}
	}

	return true;
}

bool CTriggerData::Save(std::ofstream& ofs)
{
	uint32_t dwVersion = F_TRIGGER_VERSION;
	ofs.write(reinterpret_cast<const char*>(&dwVersion), sizeof(uint32_t));
	ofs.write(reinterpret_cast<const char*>(&uID), sizeof(uint32_t));
	ofs.write(reinterpret_cast<const char*>(&bActive), sizeof(bool));
	ofs.write(reinterpret_cast<const char*>(&bRun), sizeof(bool));
	ofs.write(reinterpret_cast<const char*>(&runCondition), sizeof(char));

	ofs.write(strName.c_str(),
		(static_cast<size_t>(128) < strName.size()
			? static_cast<size_t>(128)
			: strName.size()));

	size_t n;

	// Writing condition data(tree)
	SaveConditionTree(ofs, rootCondition);

	// Writing operation data
	n = listOperation.size();
	ofs.write(reinterpret_cast<const char*>(&n), sizeof(int32_t));
	for (size_t i = 0; i < n; ++i)
	{
		uint32_t dataSize = 0;
		_s_operation* pTemp = (_s_operation*)listOperation[i];
		ofs.write(reinterpret_cast<const char*>(&pTemp->iType), sizeof(int32_t));
		switch ((_e_operation)pTemp->iType)
		{
		case _e_operation::o_attack:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_ATTACK_TYPE));
			break;
		case _e_operation::o_use_skill:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_USE_SKILL));
			break;
		case _e_operation::o_talk:
			dataSize = ((O_TALK_TEXT*)pTemp->pParam)->uSize;
			ofs.write(reinterpret_cast<const char*>(&((O_TALK_TEXT*)pTemp->pParam)->uSize), sizeof(uint32_t));
			ofs.write(reinterpret_cast<const char*>(((O_TALK_TEXT*)pTemp->pParam)->szData), ((O_TALK_TEXT*)pTemp->pParam)->uSize);
			break;
		case _e_operation::o_reset_hate_list:
			break;
		case _e_operation::o_run_trigger:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_RUN_TRIGGER));
			break;
		case _e_operation::o_stop_trigger:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_STOP_TRIGGER));
			break;
		case _e_operation::o_active_trigger:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_ACTIVE_TRIGGER));
			break;
		case _e_operation::o_create_timer:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_CREATE_TIMER));
			break;
		case _e_operation::o_kill_timer:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_KILL_TIMER));
			break;
		case _e_operation::o_flee:
		case _e_operation::o_set_hate_to_first:
		case _e_operation::o_set_hate_to_last:
		case _e_operation::o_set_hate_fifty_percent:
		case _e_operation::o_skip_operation:
			break;
		case _e_operation::o_active_controller:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_ACTIVE_CONTROLLER));
			break;
		case _e_operation::o_summon:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_SUMMON));
			break;
		case _e_operation::o_trigger_task:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_TRIGGER_TASK));
			break;
		case _e_operation::o_change_path:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_CHANGE_PATH));
			break;
		case _e_operation::o_disappear:
			break;
		case _e_operation::o_sneer_monster:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_SNEER_MONSTER));
			break;
		case _e_operation::o_use_range_skill:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_USE_RANGE_SKILL));
			break;
		case _e_operation::o_reset:
			break;
		case _e_operation::o_set_global:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_SET_GLOBAL));
			break;
		case _e_operation::o_revise_global:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_REVISE_GLOBAL));
			break;
		case _e_operation::o_assign_global:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_ASSIGN_GLOBAL));
			break;
		case _e_operation::o_summon_mineral:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_SUMMON_MINERAL));
			break;
		case _e_operation::o_drop_item:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_DROP_ITEM));
			break;
		case _e_operation::o_change_hate:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_CHANGE_HATE));
			break;
		case _e_operation::o_start_event:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_START_EVENT));
			break;
		case _e_operation::o_stop_event:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_STOP_EVENT));
			break;
		case _e_operation::o_drop_item_new:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(O_DROP_ITEMNEW));
			break;
		case _e_operation::o_whisper:
			dataSize = ((O_WHISPER_TEXT*)pTemp->pParam)->uSize;
			ofs.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
			ofs.write(reinterpret_cast<const char*>(((O_WHISPER_TEXT*)pTemp->pParam)->szData), dataSize * sizeof(uint16_t));
			break;
		case _e_operation::o_talk_portrait:
			dataSize = ((O_TALK_PORTRAIT*)pTemp->pParam)->uSize;
			ofs.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
			ofs.write(reinterpret_cast<const char*>(((O_TALK_PORTRAIT*)pTemp->pParam)->szData), dataSize * sizeof(uint16_t));
			break;
		}

		ofs.write(reinterpret_cast<const char*>(&pTemp->iType), sizeof(int32_t));
		switch ((_e_target)pTemp->mTarget.iType)
		{
		case _e_target::t_occupation_list:
			ofs.write(reinterpret_cast<const char*>(pTemp->pParam), sizeof(T_OCCUPATION));
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

	pNewObject->strName = strName;
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

bool CTriggerData::ReadConditionTree(std::ifstream& ifs, _s_tree_item* pNode)
{
	if (pNode == nullptr)
		return true;

	int32_t dat_size = 0;
	int32_t flag = 0;
	ifs.read(reinterpret_cast<char*>(&pNode->mCondition.iType), sizeof(int32_t));
	ifs.read(reinterpret_cast<char*>(&dat_size), sizeof(int32_t));

	if (dat_size != 0)
	{
		pNode->mCondition.pParam = new char[dat_size];
		ifs.read(reinterpret_cast<char*>(pNode->mCondition.pParam), dat_size);
	}
	else
		pNode->mCondition.pParam = nullptr;

	while (true)
	{
		ifs.read(reinterpret_cast<char*>(&flag), sizeof(int32_t));
		if (flag == CONDITION_LEAF)
		{
			pNode->pLeft = nullptr;
			pNode->pRight = nullptr;
			break;
		}
		else if (flag == CONDITION_LEFT_SUB)
		{
			pNode->pLeft = new _s_tree_item;
			ReadConditionTree(ifs, pNode->pLeft);
		}
		else if (flag == CONDITION_RIGHT_SUB)
		{
			pNode->pRight = new _s_tree_item;
			ReadConditionTree(ifs, pNode->pRight);
		}
		else if (flag == CONDITION_NODE_END)
			break;
	}

	return true;
}

bool CTriggerData::SaveConditionTree(std::ofstream& ofs, const _s_tree_item* pNode)
{
	if (pNode == nullptr)
		return true;

	int32_t dat_size = 0;
	int32_t flag = 0;
	ofs.write(reinterpret_cast<const char*>(&pNode->mCondition.iType), sizeof(int32_t));

	if (pNode->mCondition.pParam == nullptr)
		ofs.write(reinterpret_cast<const char*>(&dat_size), sizeof(int32_t));
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

		ofs.write(reinterpret_cast<const char*>(&dat_size), sizeof(int32_t));
		ofs.write(reinterpret_cast<const char*>(pNode->mCondition.pParam), dat_size);
	}

	if (pNode->pLeft == nullptr && pNode->pRight == nullptr)
	{
		flag = CONDITION_LEAF;
		ofs.write(reinterpret_cast<const char*>(&flag), sizeof(int32_t));
		return true;
	}

	if (pNode->pLeft)
	{
		flag = CONDITION_LEFT_SUB;
		ofs.write(reinterpret_cast<const char*>(&flag), sizeof(int32_t));
		SaveConditionTree(ofs, pNode->pLeft);
	}

	if (pNode->pRight)
	{
		flag = CONDITION_RIGHT_SUB;
		ofs.write(reinterpret_cast<const char*>(&flag), sizeof(int32_t));
		SaveConditionTree(ofs, pNode->pRight);
	}

	flag = CONDITION_NODE_END;
	ofs.write(reinterpret_cast<const char*>(&flag), sizeof(int32_t));

	return true;
}

// ********* CPolicyData

CPolicyData::CPolicyData() :uID(0) {}
CPolicyData::~CPolicyData() {}

bool CPolicyData::Load(const char* path)
{
	std::ifstream ifs(path, std::ios::binary);
	return ifs.is_open() ? Load(ifs) : false;
}

bool CPolicyData::Load(const wchar_t* path)
{
	std::ifstream ifs(path, std::ios::binary);
	return ifs.is_open() ? Load(ifs) : false;
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
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs.is_open())
		return false;

	return Save(ofs);
}

bool CPolicyData::Save(const wchar_t* path)
{
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs.is_open())
		return false;

	return Save(ofs);
}

bool CPolicyData::Save(const std::string& path)
{
	return Save(path.c_str());
}

bool CPolicyData::Save(const std::wstring& path)
{
	return Save(path.c_str());
}

bool CPolicyData::Load(std::ifstream& ifs)
{
	uint32_t nVersion;
	ifs.read(reinterpret_cast<char*>(&nVersion), sizeof(uint32_t));

	if (nVersion == F_POLICY_VERSION)
	{
		int32_t n = 0;
		ifs.read(reinterpret_cast<char*>(&uID), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&n), sizeof(int32_t));

		for (int32_t i = 0; i < n; i++)
		{
			CTriggerData* pNew = new CTriggerData;
			pNew->Load(ifs);
			listTriggerPtr.push_back(pNew);
		}
	}
	else
		return false;

	return true;
}

bool CPolicyData::Save(std::ofstream& ofs)
{
	uint32_t nVersion = F_POLICY_VERSION;
	ofs.write(reinterpret_cast<const char*>(&nVersion), sizeof(uint32_t));
	ofs.write(reinterpret_cast<const char*>(&uID), sizeof(uint32_t));
	size_t n = listTriggerPtr.size();
	ofs.write(reinterpret_cast<const char*>(&n), sizeof(int32_t));

	for (size_t i = 0; i < n; i++)
		listTriggerPtr[i]->Save(ofs);

	ofs.close();
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
	std::ifstream ifs(path, std::ios::binary);

	if (!ifs.is_open())
		return false;

	uint32_t version;
	int32_t count;
	ifs.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

	if (version != F_POLICY_EXP_VERSION)
	{
		ifs.close();
		return false;
	}

	ifs.read(reinterpret_cast<char*>(&count), sizeof(int32_t));
	for (int32_t i = 0; i < count; ++i)
	{
		CPolicyData* pNew = new CPolicyData;

		if (pNew == 0 || !pNew->Load(ifs))
		{
			ifs.close();
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

	ifs.close();
	return true;
}

bool CPolicyDataManager::Load(const wchar_t* path)
{
	std::ifstream ifs(path, std::ios::binary);

	if (!ifs.is_open())
		return false;

	uint32_t version;
	int32_t count;
	ifs.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

	if (version != F_POLICY_EXP_VERSION)
	{
		ifs.close();
		return false;
	}

	ifs.read(reinterpret_cast<char*>(&count), sizeof(int32_t));
	for (int32_t i = 0; i < count; ++i)
	{
		CPolicyData* pNew = new CPolicyData;

		if (pNew == 0 || !pNew->Load(ifs))
		{
			ifs.close();
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

	ifs.close();
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
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs.is_open())
		return false;

	uint32_t uVersion = F_POLICY_EXP_VERSION;
	size_t count = listPolicy.size();
	ofs.write(reinterpret_cast<const char*>(&uVersion), sizeof(uint32_t));
	ofs.write(reinterpret_cast<const char*>(&count), sizeof(int32_t));

	for (size_t i = 0; i < count; ++i)
	{
		CPolicyData* pPolicy = listPolicy[i];

		if (!pPolicy->Save(ofs))
		{
			ofs.close();
			return false;
		}
	}

	ofs.close();
	return true;
}

bool CPolicyDataManager::Save(const wchar_t* path)
{
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs.is_open())
		return false;

	uint32_t uVersion = F_POLICY_EXP_VERSION;
	size_t count = listPolicy.size();
	ofs.write(reinterpret_cast<const char*>(&uVersion), sizeof(uint32_t));
	ofs.write(reinterpret_cast<const char*>(&count), sizeof(int32_t));

	for (size_t i = 0; i < count; ++i)
	{
		CPolicyData* pPolicy = listPolicy[i];

		if (!pPolicy->Save(ofs))
		{
			ofs.close();
			return false;
		}
	}

	ofs.close();
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

CPolicyData* CPolicyDataManager::GetPolicy(size_t ndx)
{
	if (ndx >= 0 && ndx < listPolicy.size())
		return listPolicy[ndx];
	else
		return nullptr;
}
