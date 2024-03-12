#ifndef __AI_POLICY_H__
#define __AI_POLICY_H__

#include <string>
#include <vector>

constexpr auto F_POLICY_VERSION = 0;
constexpr auto F_TRIGGER_VERSION = 11;
constexpr auto F_POLICY_EXP_VERSION = 0;

class CTriggerData
{
public:
	//0-WuXia, 1-FaShi, 2-SengLu, 3-YaoJing, 4-YaoShou, 5-MeiLing, 6-YuMang, 7-YuLing
	enum class _e_occupation
	{
		o_wuxia = 1,
		o_fashi = 2,
		o_senglu = 4,
		o_yaojing = 8,
		o_yaoshou = 16,
		o_meiling = 32,
		o_yumang = 64,
		o_yuling = 128,
	};

	enum class _e_condition
	{
		c_time_come = 0,
		c_hp_less,
		c_start_attack,
		c_random,
		c_kill_player,
		c_not,
		c_or,
		c_and,
		c_died,
		c_path_end_point,
		c_enmity_reach,
		c_distance_reach,
		c_plus,
		c_minus,
		c_multiply,
		c_divide,
		c_great,
		c_less,
		c_equ,
		c_var,
		c_constant,
		c_rank_level,
		c_born,
		c_attack_by_skill,
		c_random_selectone,
		c_leave_attack,
		c_num,
	};

	enum class _e_target
	{
		t_hate_first = 0,
		t_hate_second,
		t_hate_others,
		t_most_hp,
		t_most_mp,
		t_least_hp,
		t_occupation_list,
		t_self,
		t_num,
	};

	enum class _e_operation
	{
		o_attack = 0,
		o_use_skill,
		o_talk,
		o_reset_hate_list,
		o_run_trigger,
		o_stop_trigger,
		o_active_trigger,
		o_create_timer,
		o_kill_timer,
		o_flee,
		o_set_hate_to_first,
		o_set_hate_to_last,
		o_set_hate_fifty_percent,
		o_skip_operation,
		o_active_controller,
		o_summon,
		o_trigger_task,
		o_change_path,
		o_disappear,
		o_sneer_monster,
		o_use_range_skill,
		o_reset,
		o_set_global,
		o_revise_global,
		o_assign_global,
		o_summon_mineral,
		o_drop_item,
		o_change_hate,
		o_start_event,
		o_stop_event,
		o_drop_item_new,
		o_whisper,
		o_talk_portrait,
		o_num,
	};

	enum class _run_condition
	{
		run_attack_noeffect = 0,
		run_attack_effect,
	};

	struct _s_condition
	{
		int32_t iType;
		void* pParam;

		_s_condition()
			: iType(0)
			, pParam(nullptr)
		{}

		_s_condition(int32_t type, void* param)
			: iType(type)
			, pParam(param)
		{}
	};

	struct _s_target
	{
		int32_t iType;
		void* pParam;

		_s_target()
			: iType(0)
			, pParam(nullptr)
		{}

		_s_target(int32_t type, void* param)
			: iType(type)
			, pParam(param)
		{}
	};

	struct _s_operation
	{
		int32_t iType;
		void* pParam;
		_s_target mTarget;

		_s_operation()
			: iType(0)
			, pParam(nullptr)
			, mTarget()
		{}

		_s_operation(int32_t type, void* param, const _s_target& target)
			: iType(type)
			, pParam(param)
			, mTarget(target)
		{}
	};

	struct _s_tree_item
	{
		_s_tree_item()
			: mCondition()
			, pLeft(nullptr)
			, pRight(nullptr)
		{}

		~_s_tree_item()
		{
			if (mCondition.pParam)
			{
				delete mCondition.pParam;
				mCondition.pParam = nullptr;
			}

			if (pLeft)
			{
				delete pLeft;
				pLeft = nullptr;
			}

			if (pRight)
			{
				delete pRight;
				pRight = nullptr;
			}
		}
		_s_condition mCondition;
		_s_tree_item* pLeft;
		_s_tree_item* pRight;
	};

	CTriggerData();
	~CTriggerData();

	bool Load(std::ifstream& ifs);
	bool Save(std::ofstream& ofs);
	void Release();

	char* GetName() { return szName; }
	void SetName(const char* name) { strcpy(szName, name); }

	bool IsActive() const { return bActive; }
	void ActiveTrigger() { bActive = true; }
	void ToggleTrigger() { bActive = false; }

	bool IsRun() const { return bRun; }
	void SetRunStatus(bool br) { bRun = br; }

	_run_condition GetRunCondition() { return (_run_condition)runCondition; }
	void SetRunCondition(_run_condition condition) { runCondition = condition; }

	uint32_t GetID() const { return uID; }
	void SetID(uint32_t id) { uID = id; }

	_s_tree_item* GetConditionRoot() const { return rootCondition; }
	void SetConditionRoot(_s_tree_item* pRoot) { rootCondition = pRoot; }

	CTriggerData* CopyObject();
	void ReleaseConditionTree();

	static _s_tree_item* CopyConditionTree(_s_tree_item* pRoot);
	static _s_operation* CopyOperation(_s_operation* pOperation);

	void AddOperation(uint32_t iType, void* pParam, const _s_target* pTarget);
	void AddOperation(_s_operation* pOperation);

	int32_t GetOperationNum() const { return (int32_t)listOperation.size(); }

	uint32_t GetOperation(uint32_t idx, uint32_t& iType, void** ppData, _s_target& target);

	_s_operation* GetOperation(uint32_t idx) { return listOperation[idx]; }

	void SetOperation(uint32_t idx, uint32_t iType, void* pData, const _s_target* pTarget);
	void DelOperation(uint32_t idx);

protected:
	static bool ReadConditionTree(std::ifstream& ifs, _s_tree_item* pNode);
	static bool SaveConditionTree(std::ofstream& ofs, const _s_tree_item* pNode);

private:
	char szName[128];
	bool bActive;
	bool bRun;
	_run_condition runCondition;
	uint32_t uID;
	_s_tree_item* rootCondition;
	std::vector<_s_operation*> listOperation;
};

class CPolicyData
{
public:
	CPolicyData();
	~CPolicyData();

	bool Load(const char* path);
	bool Load(const wchar_t* path);
	bool Load(const std::string& path);
	bool Load(const std::wstring& path);

	bool Save(const char* path);
	bool Save(const wchar_t* path);
	bool Save(const std::string& path);
	bool Save(const std::wstring& path);

	bool Load(std::ifstream& ifs);
	bool Save(std::ofstream& ofs);

	void Release();

	uint32_t GetID() const { return uID; }
	void SetID(uint32_t id) { uID = id; }

	int32_t GetTriggerPtrNum() const { return (int32_t)listTriggerPtr.size(); }
	CTriggerData* GetTriggerPtr(int32_t ndx) const { return listTriggerPtr[ndx]; }

	int32_t GetIndex(uint32_t id); // Returns -1 if the search fails

	void SetTriggerPtr(int32_t ndx, CTriggerData* pTrigger) { listTriggerPtr[ndx] = pTrigger; }
	void AddTriggerPtr(CTriggerData* pTrigger) { listTriggerPtr.push_back(pTrigger); }

	void DelTriggerPtr(int32_t ndx);

private:
	uint32_t uID;
	std::vector<CTriggerData*> listTriggerPtr;
};

class CPolicyDataManager
{
public:
	CPolicyDataManager();
	~CPolicyDataManager();

	bool Load(const char* path);
	bool Load(const wchar_t* path);
	bool Load(const std::string& path);
	bool Load(const std::wstring& path);

	bool Save(const char* path);
	bool Save(const wchar_t* path);
	bool Save(const std::string& path);
	bool Save(const std::wstring& path);

	void Release();

	size_t GetPolicyNum() const;
	CPolicyData* GetPolicy(size_t ndx);

private:
	std::vector<CPolicyData*> listPolicy;
};

#endif
