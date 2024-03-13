#include "pch.h"
#include "OperationParam.h"
#include "PolicyTriggerDlg.h"
#include "Convert.h"

bool g_bPolicyModified;

BOOL TRIGGER_ID_FUNCS::OnActivate(void)
{
	COperationParam_Run_Trigger* pTrigger = (COperationParam_Run_Trigger*)m_pParent;
	if (pTrigger->m_Data.uID == 0)
	{	// new trigger
		CPolicyTriggerDlg dlg;
		dlg.m_pCurrentPolicy = pTrigger->m_pPolicyData;
		if (dlg.DoModal() == IDOK)
		{
			if (dlg.m_pTriggerData)
			{
				dlg.m_pTriggerData->SetRunStatus(true);
				pTrigger->m_pPolicyData->AddTriggerPtr(dlg.m_pTriggerData);
				pTrigger->m_Data.uID = dlg.m_pTriggerData->GetID();
				m_strText = Convert::GB18030ToCStringW(dlg.m_pTriggerData->GetName().c_str());
				g_bPolicyModified = true;
			}
		}
	}
	else
	{	// open an old trigger
		for (int32_t i = 0; i < (int32_t)pTrigger->m_pPolicyData->GetTriggerPtrNum(); i++)
		{
			CTriggerData* pData = pTrigger->m_pPolicyData->GetTriggerPtr(i);
			if (pData->IsRun())
			{
				if (pData->GetID() == pTrigger->m_Data.uID)
				{
					CPolicyTriggerDlg dlg;
					dlg.m_bModified = true;
					dlg.m_pTriggerData = pData;
					dlg.m_pCurrentPolicy = pTrigger->m_pPolicyData;
					if (dlg.DoModal() == IDOK)
						g_bPolicyModified = true;

					m_strText = Convert::GB18030ToCStringW(pData->GetName().c_str());
					break;
				}
			}
		}
	}
	return TRUE;
}

LPCTSTR TRIGGER_ID_FUNCS::OnGetShowString(void) const
{
	return CString(m_strText);
}

AVariant TRIGGER_ID_FUNCS::OnGetValue(void) const
{
	return m_var;
}

void TRIGGER_ID_FUNCS::OnSetValue(const AVariant& var)
{
	COperationParam_Run_Trigger* pTrigger = (COperationParam_Run_Trigger*)m_pParent;

	// open an old trigger
	for (int32_t i = 0; i < (int32_t)pTrigger->m_pPolicyData->GetTriggerPtrNum(); i++)
	{
		CTriggerData* pData = pTrigger->m_pPolicyData->GetTriggerPtr(i);
		if (pData->IsRun())
		{
			if (pData->GetID() == pTrigger->m_Data.uID)
			{
				m_strText = Convert::GB18030ToCStringW(pData->GetName().c_str());
				break;
			}
		}
	}
}
