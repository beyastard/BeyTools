
// PolicyEdit.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPolicyApp:
// See PolicyEdit.cpp for the implementation of this class
//

class CPolicyApp : public CWinApp
{
public:
	CPolicyApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	void ExitApp();

// Implementation

	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileExit();

private:
	int m_nExitCode;
};

extern CPolicyApp theApp;
