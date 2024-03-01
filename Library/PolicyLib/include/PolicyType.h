#ifndef __AI_POLICY_TYPE_H__
#define __AI_POLICY_TYPE_H__

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

constexpr auto OP_TEXT_LENGTH = 256;

// *********** Condition ***********

struct C_TIME_COME
{
	uint32_t uID;

	C_TIME_COME()
		:uID(0)
	{}

	C_TIME_COME(uint32_t id)
		: uID(id)
	{}
};

struct C_HP_LESS
{
	float fPercent;

	C_HP_LESS()
		:fPercent(0.0f)
	{}

	C_HP_LESS(float percent)
		: fPercent(percent)
	{}
};

struct C_RANDOM
{
	float fProbability;

	C_RANDOM()
		: fProbability(0.0f)
	{}

	C_RANDOM(float probability)
		: fProbability(probability)
	{}
};

struct C_PATH_END_POINT
{
	uint32_t uID;

	C_PATH_END_POINT()
		: uID(0)
	{}

	C_PATH_END_POINT(uint32_t id)
		: uID(id)
	{}
};

struct C_ENMITY_REACH
{
	int32_t nPlayer;
	int32_t nPlayerLevel;

	C_ENMITY_REACH()
		: nPlayer(0)
		, nPlayerLevel(0)
	{}

	C_ENMITY_REACH(int32_t player, int32_t playerLevel)
		: nPlayer(player)
		, nPlayerLevel(playerLevel)
	{}
};

struct C_DISTANCE_REACH
{
	float fDistance;

	C_DISTANCE_REACH()
		: fDistance(0)
	{}

	C_DISTANCE_REACH(float distance)
		: fDistance(distance)
	{}
};

struct C_VAR
{
	int32_t iID;

	C_VAR()
		: iID(0)
	{}

	C_VAR(int32_t id)
		: iID(id)
	{}
};

struct C_CONSTANT
{
	int32_t iValue;

	C_CONSTANT()
		: iValue(0)
	{}

	C_CONSTANT(int32_t value)
		: iValue(value)
	{}
};

struct C_RANK_LEVEL
{
	int32_t iValue;

	C_RANK_LEVEL()
		: iValue(0)
	{}

	C_RANK_LEVEL(int32_t value)
		: iValue(value)
	{}
};

struct C_ATTACK_BY_SKILL
{
	uint32_t uID;

	C_ATTACK_BY_SKILL()
		: uID(0)
	{}

	C_ATTACK_BY_SKILL(uint32_t id)
		: uID(id)
	{}
};

struct C_RANDOM_SELECTONE
{
	float fProbability;

	C_RANDOM_SELECTONE()
		: fProbability(0)
	{}

	C_RANDOM_SELECTONE(float probability)
		: fProbability(probability)
	{}
};

// *********** Operation ***********

struct O_ATTACK_TYPE
{
	uint32_t uType;

	O_ATTACK_TYPE()
		: uType(0)
	{}

	O_ATTACK_TYPE(uint32_t atype)
		: uType(atype)
	{}
};

struct O_USE_SKILL
{
	uint32_t uSkill;
	uint32_t uLevel;

	O_USE_SKILL()
		: uSkill(0)
		, uLevel(0)
	{}

	O_USE_SKILL(uint32_t skill, uint32_t level)
		: uSkill(skill)
		, uLevel(level)
	{}
};

struct O_TALK_TEXT
{
	uint32_t uSize;
	uint16_t* szData;

	O_TALK_TEXT()
		: uSize(0)
		, szData(nullptr)
	{}

	O_TALK_TEXT(uint32_t txtSize, const uint16_t* data)
		: uSize(txtSize)
	{
		szData = nullptr;
		if (txtSize > 0 && data != nullptr)
		{
			szData = static_cast<uint16_t*>(std::malloc(txtSize * sizeof(uint16_t)));
			if (szData != nullptr)
				std::memcpy(szData, data, txtSize * sizeof(uint16_t));
			else
				throw std::runtime_error("Memory allocation failed!");
		}
	}

	~O_TALK_TEXT()
	{
		std::free(szData);
	}
};

struct O_RUN_TRIGGER
{
	uint32_t uID;

	O_RUN_TRIGGER()
		: uID(0)
	{}

	O_RUN_TRIGGER(uint32_t id)
		: uID(id)
	{}
};

struct O_STOP_TRIGGER
{
	uint32_t uID;

	O_STOP_TRIGGER()
		: uID(0)
	{}

	O_STOP_TRIGGER(uint32_t id)
		: uID(id)
	{}
};

struct O_ACTIVE_TRIGGER
{
	uint32_t uID;

	O_ACTIVE_TRIGGER()
		: uID(0)
	{}

	O_ACTIVE_TRIGGER(uint32_t id)
		: uID(id)
	{}
};

struct O_CREATE_TIMER
{
	uint32_t uID;
	uint32_t uPeriod;
	uint32_t uCounter;

	O_CREATE_TIMER()
		: uID(0)
		, uPeriod(0)
		, uCounter(0)
	{}

	O_CREATE_TIMER(uint32_t id, uint32_t period, uint32_t counter)
		: uID(id)
		, uPeriod(period)
		, uCounter(counter)
	{}
};

struct O_KILL_TIMER
{
	uint32_t uID;

	O_KILL_TIMER()
		: uID(0)
	{}

	O_KILL_TIMER(uint32_t id)
		: uID(id)
	{}
};

struct O_ACTIVE_CONTROLLER
{
	uint32_t uID;
	bool bStop;

	O_ACTIVE_CONTROLLER()
		: uID(0)
		, bStop(0)
	{}

	O_ACTIVE_CONTROLLER(uint32_t id, bool stop)
		: uID(id)
		, bStop(stop)
	{}
};

struct O_SUMMON
{
	uint32_t uMonsterID;
	uint32_t uBodyMonsterID;
	uint32_t uMonsterNum;
	uint32_t uLife;
	uint16_t szName[16];
	float fRange;
	bool bFollow;
	bool bDisappear;

	O_SUMMON()
		: uMonsterID(0)
		, uBodyMonsterID(0)
		, uMonsterNum(0)
		, uLife(0)
		, fRange(0.0f)
		, bFollow(false)
		, bDisappear(false)
	{
		std::memset(szName, 0, sizeof(uint16_t) * 16);
	}

	O_SUMMON(uint32_t monsterId, uint32_t bodyMonsterId, uint32_t monsterNum, uint32_t life,
		const uint16_t aName[16], float range, bool follow, bool disappear)
		: uMonsterID(monsterId)
		, uBodyMonsterID(bodyMonsterId)
		, uMonsterNum(monsterNum)
		, uLife(life)
		, fRange(range)
		, bFollow(follow)
		, bDisappear(disappear)
	{
		std::memcpy(szName, aName, sizeof(uint16_t) * 16);
	}
};

struct O_TRIGGER_TASK
{
	uint32_t uTaskID;

	O_TRIGGER_TASK()
		: uTaskID(0)
	{}

	O_TRIGGER_TASK(uint32_t taskId)
		: uTaskID(taskId)
	{}
};

struct O_CHANGE_PATH_VERSION8
{
	uint32_t uPathID;

	O_CHANGE_PATH_VERSION8()
		: uPathID(0)
	{}

	O_CHANGE_PATH_VERSION8(uint32_t pathId)
		: uPathID(pathId)
	{}
};

struct O_CHANGE_PATH
{
	uint32_t uPathID;
	int32_t iType;

	O_CHANGE_PATH()
		: uPathID(0)
		, iType(0)
	{}

	O_CHANGE_PATH(uint32_t pathId, int32_t pathType)
		: uPathID(pathId)
		, iType(pathType)
	{}
};

struct O_SNEER_MONSTER
{
	float fRange;

	O_SNEER_MONSTER()
		: fRange(0)
	{}

	O_SNEER_MONSTER(float range)
		: fRange(range)
	{}
};

struct O_USE_RANGE_SKILL
{
	uint32_t uSkill;
	uint32_t uLevel;
	float fRange;

	O_USE_RANGE_SKILL()
		: uSkill(0)
		, uLevel(0)
		, fRange(0)
	{}

	O_USE_RANGE_SKILL(uint32_t skill, uint32_t level, float range)
		: uSkill(skill)
		, uLevel(level)
		, fRange(range)
	{}
};

struct O_SET_GLOBAL
{
	int32_t iID;
	int32_t iValue;

	O_SET_GLOBAL()
		: iID(0)
		, iValue(0)
	{}

	O_SET_GLOBAL(int32_t id, int32_t value)
		: iID(id)
		, iValue(value)
	{}
};

struct O_REVISE_GLOBAL
{
	int32_t iID;
	int32_t iValue;

	O_REVISE_GLOBAL()
		: iID(0)
		, iValue(0)
	{}

	O_REVISE_GLOBAL(int32_t id, int32_t value)
		: iID(id)
		, iValue(value)
	{}
};

struct O_ASSIGN_GLOBAL
{
	int32_t iIDSrc;
	int32_t iIDDst;

	O_ASSIGN_GLOBAL()
		: iIDSrc(0)
		, iIDDst(0)
	{}

	O_ASSIGN_GLOBAL(int32_t id, int32_t value)
		: iIDSrc(id)
		, iIDDst(value)
	{}
};

struct O_SUMMON_MINERAL
{
	uint32_t uMineralID;
	uint32_t uMineralNum;
	uint32_t uHP;
	float fRange;
	bool bBind;

	O_SUMMON_MINERAL()
		: uMineralID(0)
		, uMineralNum(0)
		, uHP(0)
		, fRange(0)
		, bBind(false)
	{}

	O_SUMMON_MINERAL(uint32_t mineralId, uint32_t mineralNum, uint32_t hp, float range, bool _bind)
		: uMineralID(mineralId)
		, uMineralNum(mineralNum)
		, uHP(hp)
		, fRange(range)
		, bBind(_bind)
	{}
};

struct O_DROP_ITEM
{
	uint32_t uItemID;
	uint32_t uItemNum;
	uint32_t uExpireDate;

	O_DROP_ITEM()
		: uItemID(0)
		, uItemNum(0)
		, uExpireDate(0)
	{}

	O_DROP_ITEM(uint32_t itemId, uint32_t itemNum, uint32_t expireDate)
		: uItemID(itemId)
		, uItemNum(itemNum)
		, uExpireDate(expireDate)
	{}
};

struct O_CHANGE_HATE
{
	int32_t iHateValue;

	O_CHANGE_HATE()
		: iHateValue(0)
	{}

	O_CHANGE_HATE(int32_t hateValue)
		: iHateValue(hateValue)
	{}
};

struct O_START_EVENT
{
	int32_t iId;

	O_START_EVENT()
		: iId(0)
	{}

	O_START_EVENT(int32_t id)
		: iId(id)
	{}
};

struct O_STOP_EVENT
{
	int32_t iId;

	O_STOP_EVENT()
		: iId(0)
	{}

	O_STOP_EVENT(int32_t id)
		: iId(id)
	{}
};

struct O_DROP_ITEMNEW
{
	uint32_t uItemID;
	uint32_t uItemNum;
	uint32_t uExpireDate;

	O_DROP_ITEMNEW()
		: uItemID(0)
		, uItemNum(0)
		, uExpireDate(0)
	{}

	O_DROP_ITEMNEW(uint32_t itemId, uint32_t itemNum, uint32_t expireDate)
		: uItemID(itemId)
		, uItemNum(itemNum)
		, uExpireDate(expireDate)
	{}
};

struct O_WHISPER_TEXT
{
	uint32_t uSize;
	uint16_t* szData;

	O_WHISPER_TEXT()
		: uSize(0)
		, szData(nullptr)
	{}

	O_WHISPER_TEXT(uint32_t txtSize, const uint16_t* data)
		: uSize(txtSize)
	{
		szData = nullptr;
		if (txtSize > 0 && data != nullptr)
		{
			szData = static_cast<uint16_t*>(std::malloc(txtSize * sizeof(uint16_t)));
			if (szData != nullptr)
				std::memcpy(szData, data, txtSize * sizeof(uint16_t));
			else
				throw std::runtime_error("Memory allocation failed!");
		}
	}

	~O_WHISPER_TEXT()
	{
		std::free(szData);
	}
};

struct O_TALK_PORTRAIT
{
	uint32_t uSize;
	uint16_t* szData;

	O_TALK_PORTRAIT()
		: uSize(0)
		, szData(nullptr)
	{}

	O_TALK_PORTRAIT(uint32_t txtSize, const uint16_t* data)
		: uSize(txtSize)
	{
		szData = nullptr;
		if (txtSize > 0 && data != nullptr)
		{
			szData = static_cast<uint16_t*>(std::malloc(txtSize * sizeof(uint16_t)));
			if (szData != nullptr)
				std::memcpy(szData, data, txtSize * sizeof(uint16_t));
			else
				throw std::runtime_error("Memory allocation failed!");
		}
	}

	~O_TALK_PORTRAIT()
	{
		std::free(szData);
	}
};

// *********** Target Occupation ***********

struct T_OCCUPATION
{
	uint32_t uBit;

	T_OCCUPATION()
		: uBit(0)
	{}

	T_OCCUPATION(uint32_t bit)
		: uBit(bit)
	{}
};

#endif
