#pragma once

#include "AObject.h"
#include "Policy.h"
#include "PolicyType.h"
#include "Convert.h"

extern bool g_bPolicyModified;

struct TRIGGER_ID_FUNCS : public CUSTOM_FUNCS
{
	virtual BOOL CALLBACK OnActivate(void);
	virtual LPCTSTR CALLBACK OnGetShowString(void) const;
	virtual AVariant CALLBACK OnGetValue(void) const;
	virtual void CALLBACK OnSetValue(const AVariant& var);

	AVariant m_var;
	CString m_strText;
	void* m_pParent;
};

class COperationParam
{
public:
	COperationParam()
	{
		g_bPolicyModified = false;
		m_pPolicyData = nullptr;
		mTarget.iType = 0;
		mTarget.pParam = nullptr;
		uOperationType = 0;
		m_pProperty = new ADynPropertyObject();
	}

	virtual ~COperationParam()
	{
		if (m_pProperty)
		{
			delete m_pProperty;
			m_pProperty = nullptr;
		}
	}

	ADynPropertyObject* m_pProperty;

	virtual void UpdateProperty(bool bGet) = 0;
	uint32_t GetOperationType() { return uOperationType; };

	CTriggerData::_s_target mTarget;
	CPolicyData* m_pPolicyData;
	virtual void BuildProperty() = 0;

protected:
	uint32_t uOperationType;
};

class COperationParam_Attack : public COperationParam
{
public:
	COperationParam_Attack()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_attack;
		m_Data.uType = 0;
	}

	enum _e_attack_type
	{
		a_close_battle = 0,		// physical combat
		a_skill,				// physical bow and arrow
		a_long_distance,        // magic
		a_skill_and_battle,     // melee + ranged
		a_num,
	};
	O_ATTACK_TYPE m_Data;

protected:
	ASetTemplate<int32_t> attack_type;

	virtual void BuildProperty()
	{
		attack_type.AddElement("Melee ", a_close_battle);
		attack_type.AddElement("Ranged ", a_skill);
		attack_type.AddElement("Magic ", a_long_distance);
		attack_type.AddElement("Melee + Ranged ", a_skill_and_battle);

		m_pProperty->DynAddProperty(AVariant(m_Data.uType), "Attack Type ", &attack_type);
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.uType = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.uType);
	}
};

class COperationParam_Use_Skill : public COperationParam
{
public:
	COperationParam_Use_Skill()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_use_skill;
		m_Data.uSkill = 0;
		m_Data.uLevel = 0;
	}

	O_USE_SKILL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uSkill), "Skill ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uLevel), "Level ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.uSkill = m_pProperty->GetPropVal(0);
			m_Data.uLevel = m_pProperty->GetPropVal(1);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uSkill);
			m_pProperty->SetPropVal(1, m_Data.uLevel);
		}
	}
};

class COperationParam_Talk : public COperationParam
{
public:
	COperationParam_Talk()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_talk;
		char szText[MAX_PATH];
		strcpy(szText, "\0");
		m_Data.uSize = OP_TEXT_LENGTH * 2;
		m_Data.szData = new uint16_t[m_Data.uSize / 2];
		MultiByteToWideChar(CODEPAGE, 0, szText, MAX_PATH, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
	}

	virtual ~COperationParam_Talk()
	{
		if (m_Data.szData)
			delete m_Data.szData;
	}

	O_TALK_TEXT m_Data;

protected:
	virtual void BuildProperty()
	{
		char szText[1024];
		WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, m_Data.uSize / 2, szText, 1024, nullptr, FALSE);
		m_pProperty->DynAddProperty(AVariant((szText)), "Text ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[1024];
		strcpy(szText, "\0");
		if (bGet)
		{
			CStringA temp = CStringA(m_pProperty->GetPropVal(0));
			if (!temp.IsEmpty())
			{
				strcpy(szText, temp.GetBuffer(0));
				temp.ReleaseBuffer();
			}
			MultiByteToWideChar(CODEPAGE, 0, szText, 1024, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
		}
		else
		{
			WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH, szText, 1024, nullptr, FALSE);
			m_pProperty->SetPropVal(0, AVariant(CString(szText)));
		}
	}
};

class COperationParam_Reset_Hates : public COperationParam
{
public:
	COperationParam_Reset_Hates() { uOperationType = (int32_t)CTriggerData::_e_operation::o_reset_hate_list; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_Run_Trigger : public COperationParam
{
public:
	COperationParam_Run_Trigger()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_run_trigger;
		m_Data.uID = 0;
		m_pTriggerFunc = nullptr;
	}

	virtual ~COperationParam_Run_Trigger()
	{
		if (m_pTriggerFunc)
			delete m_pTriggerFunc;
	}

	O_RUN_TRIGGER m_Data;

protected:
	TRIGGER_ID_FUNCS* m_pTriggerFunc;

	virtual void BuildProperty()
	{
		CString temp;
		m_pTriggerFunc = new TRIGGER_ID_FUNCS;
		m_pTriggerFunc->m_pParent = this;
		m_pProperty->DynAddProperty(AVariant(temp), "Trigger ", (ASet*)m_pTriggerFunc, nullptr, WAY_CUSTOM);
		UpdateProperty(false);
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (!bGet)
			m_pTriggerFunc->OnSetValue(AVariant());
	}
};

class COperationParam_Stop_Trigger : public COperationParam
{
public:
	COperationParam_Stop_Trigger()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_stop_trigger;
		m_Data.uID = 0;
	}

	O_STOP_TRIGGER m_Data;

protected:
	ASetTemplate<int32_t> trigger_list;

	virtual void BuildProperty()
	{
		if (m_pPolicyData)
		{
			int32_t n = m_pPolicyData->GetTriggerPtrNum();
			for (int32_t i = 0; i < n; ++i)
			{
				CTriggerData* pData = m_pPolicyData->GetTriggerPtr(i);
				if (!pData->IsRun())
					trigger_list.AddElement(pData->GetName(), pData->GetID());
			}
		}
		m_pProperty->DynAddProperty(AVariant(m_Data.uID), "Trigger ", &trigger_list);
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.uID = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.uID);
	}
};

class COperationParam_Active_Trigger : public COperationParam
{
public:
	COperationParam_Active_Trigger()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_active_trigger;
		m_Data.uID = 0;
	}

	O_ACTIVE_TRIGGER m_Data;

protected:
	ASetTemplate<int32_t> trigger_list;

	virtual void BuildProperty()
	{
		if (m_pPolicyData)
		{
			int32_t n = m_pPolicyData->GetTriggerPtrNum();
			for (int32_t i = 0; i < n; ++i)
			{
				CTriggerData* pData = m_pPolicyData->GetTriggerPtr(i);
				if (!pData->IsRun())
					trigger_list.AddElement(pData->GetName(), pData->GetID());
			}
		}
		m_pProperty->DynAddProperty(AVariant(m_Data.uID), "Trigger ", &trigger_list);
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.uID = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.uID);
	}
};

class COperationParam_Create_Timer : public COperationParam
{
public:
	COperationParam_Create_Timer()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_create_timer;
		m_Data.uID = 0;
		m_Data.uCounter = 1;
		m_Data.uPeriod = 10;
	}

	O_CREATE_TIMER m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uID), "Timer ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uPeriod), "Time Period ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uCounter), "Frequency ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.uID = m_pProperty->GetPropVal(0);
			m_Data.uPeriod = m_pProperty->GetPropVal(1);
			m_Data.uCounter = m_pProperty->GetPropVal(2);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uID);
			m_pProperty->SetPropVal(1, m_Data.uPeriod);
			m_pProperty->SetPropVal(2, m_Data.uCounter);
		}
	}
};

class COperationParam_Kill_Timer : public COperationParam
{
public:
	COperationParam_Kill_Timer()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_kill_timer;
		m_Data.uID = 0;
	}

	O_KILL_TIMER m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uID), "Timer ID ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.uID = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.uID);
	}
};

class COperationParam_Flee : public COperationParam
{
public:
	COperationParam_Flee() { uOperationType = (int32_t)CTriggerData::_e_operation::o_flee; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_SetHateToFirst : public COperationParam
{
public:
	COperationParam_SetHateToFirst() { uOperationType = (int32_t)CTriggerData::_e_operation::o_set_hate_to_first; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_SetHateToLast : public COperationParam
{
public:
	COperationParam_SetHateToLast() { uOperationType = (int32_t)CTriggerData::_e_operation::o_set_hate_to_last; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_SetHateFiftyPercent : public COperationParam
{
public:
	COperationParam_SetHateFiftyPercent() { uOperationType = (int32_t)CTriggerData::_e_operation::o_set_hate_fifty_percent; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_SkipOperation : public COperationParam
{
public:
	COperationParam_SkipOperation() { uOperationType = (int32_t)CTriggerData::_e_operation::o_skip_operation; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_Active_Controller : public COperationParam
{
public:
	COperationParam_Active_Controller()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_active_controller;
		m_Data.uID = 0;
		m_Data.bStop = 0;
	}

	O_ACTIVE_CONTROLLER m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uID), "Monster Controller ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.bStop), "Deactivate? ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.uID = m_pProperty->GetPropVal(0);
			m_Data.bStop = m_pProperty->GetPropVal(1);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uID);
			m_pProperty->SetPropVal(1, m_Data.bStop);
		}
	}
};

class COperationParam_Summon : public COperationParam
{
public:
	COperationParam_Summon()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_summon;
		m_Data.uMonsterID = 0;
		m_Data.uBodyMonsterID = 0;
		m_Data.uMonsterNum = 1;
		m_Data.uLife = 0;
		memset(m_Data.szName, 0, sizeof(int16_t) * 16);
		m_Data.fRange = 2;
		m_Data.bFollow = true;
		m_Data.bDisappear = true;
	}

	O_SUMMON m_Data;

protected:
	virtual void BuildProperty()
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		WideCharToMultiByte(20932, 0, (LPWSTR)m_Data.szName, 16, szText, MAX_PATH, nullptr, FALSE);
		m_pProperty->DynAddProperty(AVariant(CString(szText)), "Monster Name (9 characters) ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uMonsterID), "Monster ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uBodyMonsterID), "Monster Form ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uMonsterNum), "Number of Monsters ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uLife), "Monster Survival Time (seconds) ");
		m_pProperty->DynAddProperty(AVariant(m_Data.fRange), "Distance from Body ");
		m_pProperty->DynAddProperty(AVariant(m_Data.bFollow), "Whether to follow? ");
		m_pProperty->DynAddProperty(AVariant(m_Data.bDisappear), "Does it disappear? ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
		{
			CStringA temp = CStringA(m_pProperty->GetPropVal(0));
			if (!temp.IsEmpty())
			{
				strcpy(szText, temp.GetBuffer(0));
				temp.ReleaseBuffer();
			}

			MultiByteToWideChar(CODEPAGE, 0, szText, MAX_PATH, (LPWSTR)m_Data.szName, 16);
			szText[15] = 0;
			m_Data.uMonsterID = m_pProperty->GetPropVal(1);
			m_Data.uBodyMonsterID = m_pProperty->GetPropVal(2);
			m_Data.uMonsterNum = m_pProperty->GetPropVal(3);
			m_Data.uLife = m_pProperty->GetPropVal(4);
			m_Data.fRange = m_pProperty->GetPropVal(5);
			m_Data.bFollow = m_pProperty->GetPropVal(6);
			m_Data.bDisappear = m_pProperty->GetPropVal(7);

			if (!m_Data.bFollow)
				m_Data.bDisappear = false;
		}
		else
		{
			WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szName, OP_TEXT_LENGTH, szText, MAX_PATH, nullptr, FALSE);
			m_pProperty->SetPropVal(0, AVariant(CString(szText)));
			m_pProperty->SetPropVal(1, m_Data.uMonsterID);
			m_pProperty->SetPropVal(2, m_Data.uBodyMonsterID);
			m_pProperty->SetPropVal(3, m_Data.uMonsterNum);
			m_pProperty->SetPropVal(4, m_Data.uLife);
			m_pProperty->SetPropVal(5, m_Data.fRange);
			m_pProperty->SetPropVal(6, m_Data.bFollow);
			m_pProperty->SetPropVal(7, m_Data.bDisappear);
		}
	}
};

class COperationParam_Trigger_Task : public COperationParam
{
public:
	COperationParam_Trigger_Task()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_trigger_task;
		m_Data.uTaskID = 0;
	}

	O_TRIGGER_TASK m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uTaskID), "Task ID ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.uTaskID = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.uTaskID);
	}
};

class COperationParam_Change_Path : public COperationParam
{
public:
	COperationParam_Change_Path()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_change_path;
		m_Data.uPathID = 0;
		m_Data.iType = 0;
	}

	O_CHANGE_PATH m_Data;

protected:
	ASetTemplate<int32_t> path_type;

	virtual void BuildProperty()
	{
		path_type.AddElement("Terminal Stop ", 0);
		path_type.AddElement("Backtrack ", 1);
		path_type.AddElement("Head-to-Tail Loop ", 2);

		m_pProperty->DynAddProperty(AVariant(m_Data.uPathID), "Path ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.iType), "Type ", &path_type);
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.uPathID = m_pProperty->GetPropVal(0);
			m_Data.iType = m_pProperty->GetPropVal(1);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uPathID);
			m_pProperty->SetPropVal(1, m_Data.iType);
		}
	}
};

class COperationParam_Disappear : public COperationParam
{
public:
	COperationParam_Disappear() { uOperationType = (int32_t)CTriggerData::_e_operation::o_disappear; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_Sneer_Monster : public COperationParam
{
public:
	COperationParam_Sneer_Monster()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_sneer_monster;
		m_Data.fRange = 5.0f;
	}

	O_SNEER_MONSTER m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.fRange), "Range(m) ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
			m_Data.fRange = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.fRange);
	}
};

class COperationParam_Use_Range_Skill : public COperationParam
{
public:
	COperationParam_Use_Range_Skill()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_use_range_skill;
		m_Data.uSkill = 0;
		m_Data.uLevel = 0;
		m_Data.fRange = 1;
	}

	O_USE_RANGE_SKILL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uSkill), "Skill ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uLevel), "Level ");
		m_pProperty->DynAddProperty(AVariant(m_Data.fRange), "Range(1-512) ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.uSkill = m_pProperty->GetPropVal(0);
			m_Data.uLevel = m_pProperty->GetPropVal(1);
			m_Data.fRange = m_pProperty->GetPropVal(2);

			if (m_Data.fRange < 1.0f)
				m_Data.fRange = 1.0f;

			if (m_Data.fRange > 512.0f)
				m_Data.fRange = 512.0f;
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uSkill);
			m_pProperty->SetPropVal(1, m_Data.uLevel);
			m_pProperty->SetPropVal(2, m_Data.fRange);
		}
	}
};

class COperationParam_Reset : public COperationParam
{
public:
	COperationParam_Reset() { uOperationType = (int32_t)CTriggerData::_e_operation::o_reset; }

protected:
	virtual void BuildProperty() {}
	virtual void UpdateProperty(bool bGet) {}
};

class COperationParam_Set_Global : public COperationParam
{
public:
	COperationParam_Set_Global()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_set_global;
		m_Data.iID = 0;
		m_Data.iValue = 0;
	}

	O_SET_GLOBAL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iID), "Variable ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.iValue), "Value ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.iID = m_pProperty->GetPropVal(0);
			m_Data.iValue = m_pProperty->GetPropVal(1);

		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.iID);
			m_pProperty->SetPropVal(1, m_Data.iValue);
		}
	}
};

class COperationParam_Revise_Global : public COperationParam
{
public:
	COperationParam_Revise_Global()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_revise_global;
		m_Data.iID = 0;
		m_Data.iValue = 0;
	}

	O_REVISE_GLOBAL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iID), "Variable ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.iValue), "Value ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.iID = m_pProperty->GetPropVal(0);
			m_Data.iValue = m_pProperty->GetPropVal(1);

		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.iID);
			m_pProperty->SetPropVal(1, m_Data.iValue);
		}
	}
};

class COperationParam_Assign_Global : public COperationParam
{
public:
	COperationParam_Assign_Global()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_assign_global;
		m_Data.iIDSrc = 0;
		m_Data.iIDDst = 0;
	}

	O_ASSIGN_GLOBAL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iIDDst), "Assign global variable ID dst ");
		m_pProperty->DynAddProperty(AVariant(m_Data.iIDSrc), "Assign global variable ID src ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		if (bGet)
		{
			m_Data.iIDSrc = m_pProperty->GetPropVal(0);
			m_Data.iIDDst = m_pProperty->GetPropVal(1);

		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.iIDSrc);
			m_pProperty->SetPropVal(1, m_Data.iIDDst);
		}
	}
};

class COperationParam_Summon_Mineral : public COperationParam
{
public:
	COperationParam_Summon_Mineral()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_summon_mineral;
		m_Data.uMineralID = 0;
		m_Data.uMineralNum = 1;
		m_Data.uHP = 0;
		m_Data.fRange = 10;
		m_Data.bBind = false;
	}

	O_SUMMON_MINERAL m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uMineralID), "Mineral ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uMineralNum), "Mineral Quantity ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uHP), "Mineral Life ");
		m_pProperty->DynAddProperty(AVariant(m_Data.fRange), "Distance from Body ");
		m_pProperty->DynAddProperty(AVariant(m_Data.bBind), "Whether to bind Target ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
		{
			m_Data.uMineralID = m_pProperty->GetPropVal(0);
			m_Data.uMineralNum = m_pProperty->GetPropVal(1);
			m_Data.uHP = m_pProperty->GetPropVal(2);
			m_Data.fRange = m_pProperty->GetPropVal(3);
			m_Data.bBind = m_pProperty->GetPropVal(4);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uMineralID);
			m_pProperty->SetPropVal(1, m_Data.uMineralNum);
			m_pProperty->SetPropVal(2, m_Data.uHP);
			m_pProperty->SetPropVal(3, m_Data.fRange);
			m_pProperty->SetPropVal(4, m_Data.bBind);
		}
	}
};

class COperationParam_Drop_Item : public COperationParam
{
public:
	COperationParam_Drop_Item()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_drop_item;
		m_Data.uItemID = 0;
		m_Data.uItemNum = 1;
		m_Data.uExpireDate = 0;
	}

	O_DROP_ITEM m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uItemID), "Item ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uItemNum), "Item Amount ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uExpireDate), "Item Life ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
		{
			m_Data.uItemID = m_pProperty->GetPropVal(0);
			m_Data.uItemNum = m_pProperty->GetPropVal(1);
			m_Data.uExpireDate = m_pProperty->GetPropVal(2);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uItemID);
			m_pProperty->SetPropVal(1, m_Data.uItemNum);
			m_pProperty->SetPropVal(2, m_Data.uExpireDate);
		}
	}
};

class COperationParam_Change_Hate : public COperationParam
{
public:
	COperationParam_Change_Hate()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_change_hate;
		m_Data.iHateValue = 0;
	}

	O_CHANGE_HATE	m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iHateValue), "Hate Value ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
			m_Data.iHateValue = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.iHateValue);
	}
};

class COperationParam_Start_Event : public COperationParam
{
public:
	COperationParam_Start_Event()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_start_event;
		m_Data.iId = 0;
	}

	O_START_EVENT m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iId), "Event ID ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
			m_Data.iId = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.iId);
	}
};

class COperationParam_Stop_Event : public COperationParam
{
public:
	COperationParam_Stop_Event()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_stop_event;
		m_Data.iId = 0;
	}

	O_STOP_EVENT m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.iId), "Event ID ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
			m_Data.iId = m_pProperty->GetPropVal(0);
		else
			m_pProperty->SetPropVal(0, m_Data.iId);
	}
};

class COperationParam_Drop_ItemNew : public COperationParam
{
public:
	COperationParam_Drop_ItemNew()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_drop_item_new;
		m_Data.uItemID = 0;
		m_Data.uItemNum = 1;
		m_Data.uExpireDate = 0;
	}

	O_DROP_ITEMNEW m_Data;

protected:
	virtual void BuildProperty()
	{
		m_pProperty->DynAddProperty(AVariant(m_Data.uItemID), "Item ID ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uItemNum), "Item Amount ");
		m_pProperty->DynAddProperty(AVariant(m_Data.uExpireDate), "Item Life ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[MAX_PATH];
		memset(szText, 0, MAX_PATH);
		if (bGet)
		{
			m_Data.uItemID = m_pProperty->GetPropVal(0);
			m_Data.uItemNum = m_pProperty->GetPropVal(1);
			m_Data.uExpireDate = m_pProperty->GetPropVal(2);
		}
		else
		{
			m_pProperty->SetPropVal(0, m_Data.uItemID);
			m_pProperty->SetPropVal(1, m_Data.uItemNum);
			m_pProperty->SetPropVal(2, m_Data.uExpireDate);
		}
	}
};

class COperationParam_Whisper : public COperationParam
{
public:
	COperationParam_Whisper()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_whisper;
		char szText[MAX_PATH];
		strcpy(szText, "\0");
		m_Data.uSize = OP_TEXT_LENGTH * 2;
		m_Data.szData = new uint16_t[m_Data.uSize / 2];
		MultiByteToWideChar(CODEPAGE, 0, szText, MAX_PATH, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
	}

	virtual ~COperationParam_Whisper()
	{
		if (m_Data.szData)
			delete m_Data.szData;
	}

	O_WHISPER_TEXT m_Data;

protected:
	virtual void BuildProperty()
	{
		char szText[1024];
		WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, m_Data.uSize / 2, szText, 1024, nullptr, FALSE);
		m_pProperty->DynAddProperty(AVariant(CString(szText)), "Text ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[1024];
		strcpy(szText, "\0");
		if (bGet)
		{
			CStringA temp = CStringA(m_pProperty->GetPropVal(0));
			if (!temp.IsEmpty())
			{
				strcpy(szText, temp.GetBuffer(0));
				temp.ReleaseBuffer();
			}
			MultiByteToWideChar(CODEPAGE, 0, szText, 1024, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
		}
		else
		{
			WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH, szText, 1024, nullptr, FALSE);
			m_pProperty->SetPropVal(0, AVariant(CString(szText)));
		}
	}
};

class COperationParam_Talk_Portrait : public COperationParam
{
public:
	COperationParam_Talk_Portrait()
	{
		uOperationType = (int32_t)CTriggerData::_e_operation::o_talk_portrait;
		char szText[MAX_PATH];
		strcpy(szText, "\0");
		m_Data.uSize = OP_TEXT_LENGTH * 2;
		m_Data.szData = new uint16_t[m_Data.uSize / 2];
		MultiByteToWideChar(CODEPAGE, 0, szText, MAX_PATH, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
	}

	virtual ~COperationParam_Talk_Portrait()
	{
		if (m_Data.szData)
			delete m_Data.szData;
	}

	O_TALK_PORTRAIT m_Data;

protected:
	virtual void BuildProperty()
	{
		char szText[1024];
		WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, m_Data.uSize / 2, szText, 1024, nullptr, FALSE);
		m_pProperty->DynAddProperty(AVariant(CString(szText)), "Text ");
	}

	virtual void UpdateProperty(bool bGet)
	{
		char szText[1024];
		strcpy(szText, "\0");
		if (bGet)
		{
			CStringA temp = CStringA(m_pProperty->GetPropVal(0));
			if (!temp.IsEmpty())
			{
				strcpy(szText, temp.GetBuffer(0));
				temp.ReleaseBuffer();
			}
			MultiByteToWideChar(CODEPAGE, 0, szText, 1024, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH);
		}
		else
		{
			WideCharToMultiByte(CODEPAGE, 0, (LPWSTR)m_Data.szData, OP_TEXT_LENGTH, szText, 1024, nullptr, FALSE);
			m_pProperty->SetPropVal(0, AVariant(CString(szText)));
		}
	}
};
