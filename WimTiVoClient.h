// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// WimTiVoClient.h : main header file for the WimTiVoClient application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

// CWimTiVoClientApp:
// See WimTiVoClient.cpp for the implementation of this class
//

class CWimTiVoClientApp : public CWinAppEx
{
public:
	CWimTiVoClientApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

CString GetFileVersion(const CString & filename);
CString GetFileVersionString(const CString & filename, const CString & string);
void DelimitedTextToVector(std::vector<CString> &record, const CString& line, TCHAR delimiter);

extern CWimTiVoClientApp theApp;
