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

// WimTiVoClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "WimTiVoClient.h"
#include "MainFrm.h"

#include "WimTiVoClientDoc.h"
#include "WimTiVoClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "version")
CString GetFileVersion(const CString & filename)
{
	CString rval;
	// get The Version number of the file
	DWORD dwVerHnd = 0;
	DWORD nVersionInfoSize = ::GetFileVersionInfoSize((LPTSTR)filename.GetString(), &dwVerHnd);
	if (nVersionInfoSize > 0)
	{
		UINT *puVersionLen = new UINT;
		LPVOID pVersionInfo = new char[nVersionInfoSize];
		BOOL bTest = ::GetFileVersionInfo((LPTSTR)filename.GetString(), dwVerHnd, nVersionInfoSize, pVersionInfo);
		// Pull out the version number
		if (bTest)
		{
			LPVOID pVersionNum = NULL;
			bTest = ::VerQueryValue(pVersionInfo, _T("\\"), &pVersionNum, puVersionLen);
			if (bTest)
			{
				DWORD dwFileVersionMS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionMS;
				DWORD dwFileVersionLS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionLS;
				rval.Format(_T("%d.%d.%d.%d"), HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));
			}
		}
		delete puVersionLen;
		delete [] pVersionInfo;	
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
CString GetFileVersionString(const CString & filename, const CString & string)
{
	CString rval;
	// get The Version number of the file
	DWORD dwVerHnd = 0;
	DWORD nVersionInfoSize = ::GetFileVersionInfoSize((LPTSTR)filename.GetString(), &dwVerHnd);
	if (nVersionInfoSize > 0)
	{
		UINT *puVersionLen = new UINT;
		LPVOID pVersionInfo = new char[nVersionInfoSize];
		BOOL bTest = ::GetFileVersionInfo((LPTSTR)filename.GetString(), dwVerHnd, nVersionInfoSize, pVersionInfo);
		if (bTest)
		{
			// Structure used to store enumerated languages and code pages.
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			}	*lpTranslate = NULL;
			unsigned int cbTranslate;
			// Read the list of languages and code pages.
			::VerQueryValue(pVersionInfo, _T("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);
			// Read the file description for each language and code page.
			LPVOID lpSubBlockValue = NULL;
			unsigned int SubBlockValueSize = 1;
			for(unsigned int i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
			{
				CString SubBlockName;
				SubBlockName.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),lpTranslate[i].wLanguage,lpTranslate[i].wCodePage,string.GetString());
				// Retrieve file description for language and code page "i". 
				::VerQueryValue(pVersionInfo, (LPTSTR)SubBlockName.GetString(), &lpSubBlockValue, &SubBlockValueSize); 
			}
			if (SubBlockValueSize > 0)
				rval = CString((LPTSTR)lpSubBlockValue,SubBlockValueSize-1);
		}
		delete puVersionLen;
		delete [] pVersionInfo;	
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
void DelimitedTextToVector(std::vector<CString> &record, const CString& line, TCHAR delimiter)
{
    int linepos = 0;
    bool inquotes = false;
    int linemax = line.GetLength();
    CString curstring;
    record.clear();
       
    while(line[linepos] != 0 && linepos < linemax)
    {
        TCHAR c = line[linepos];
        if (!inquotes && curstring.GetLength() == 0 && c == _T('"'))
        {
            //beginquotechar
            inquotes = true;
        }
        else if (inquotes && c == _T('"'))
        {
            //quotechar
            if ( (linepos + 1 < linemax) && (line[linepos + 1] == _T('"'))) 
            {
                //encountered 2 double quotes in a row (resolves to 1 double quote)
                curstring.AppendChar(c);
                linepos++;
            }
            else
            {
                //endquotechar
                inquotes = false;
            }
        }
        else if (!inquotes && c == delimiter)
        {
            //end of field
            record.push_back( curstring );
            curstring = "";
        }
        else if (!inquotes && (c == _T('\r') || c == _T('\n')))
        {
            record.push_back( curstring );
            return;
        }
        else
        {
            curstring.AppendChar(c);
        }
        linepos++;
    }
    record.push_back( curstring );
    return;
}
/////////////////////////////////////////////////////////////////////////////
// CWimTiVoClientApp

BEGIN_MESSAGE_MAP(CWimTiVoClientApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CWimTiVoClientApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

// CWimTiVoClientApp construction

CWimTiVoClientApp::CWimTiVoClientApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("WimsWorld.WimTiVoClient.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CWimTiVoClientApp object

CWimTiVoClientApp theApp;

// CWimTiVoClientApp initialization

BOOL CWimTiVoClientApp::InitInstance()
{
	CWinAppEx::InitInstance();
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("WimsWorld"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWimTiVoClientDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWimTiVoClientView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CWimTiVoClientApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CWimTiVoClientApp message handlers

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	CString m_csProgramInfo;

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_CheckForUpdate;
	CMFCLinkCtrl m_LinkToUpdateCtrl;
	CString m_LinkToUpdate;
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROGRAM_INFO, m_csProgramInfo);
	DDX_Check(pDX, IDC_CHECKFORUPDATE, m_CheckForUpdate);
	DDX_Control(pDX, IDC_LINKTOUPDATE, m_LinkToUpdateCtrl);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (m_CheckForUpdate == TRUE)
	{
		CString csAppName(AfxGetAppName());
		TCHAR filename[1024];
		unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
		GetModuleFileName(AfxGetResourceHandle(), filename, buffersize );
		CString csAppVersion(GetFileVersion(filename));
		unsigned long long fv1,fv2,fv3,fv4;
		if (4 == _stscanf_s(csAppVersion,_T("%lld.%lld.%lld.%lld"),&fv1,&fv2,&fv3,&fv4))
		{
			unsigned long long FileVersion = (fv1 << 48) | (fv2 << 32) | (fv3 << 16) | fv4;
			CInternetSession serverSession;
			CHttpFile* serverFile = NULL;
			try 
			{
				serverFile = (CHttpFile*) serverSession.OpenURL(_T("http://www.WimsWorld.com/Documents/SoftwareUpdateURLs.txt"));
				if (serverFile != NULL)
				{
					serverFile->SendRequest();
					DWORD dwRet;
					serverFile->QueryInfoStatusCode(dwRet);
					if(dwRet == HTTP_STATUS_OK)
					{
						CString csLine;
						do
						{
							csLine.Empty();
							char ittybittybuffer;
							while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
							{
								if (ittybittybuffer == '\n')
									break;
								if (ittybittybuffer != '\r')
								csLine += ittybittybuffer;
							}
							std::vector<CString> Element;
							DelimitedTextToVector(Element, csLine, _T('\t'));
							if (Element.size() >= 3)
							{
								if (csAppName.CompareNoCase(Element[0]) == 0)
								{
									unsigned long long uv1,uv2,uv3,uv4;
									if (4 == _stscanf_s(Element[1],_T("%lld.%lld.%lld.%lld"),&uv1,&uv2,&uv3,&uv4))
									{
										unsigned long long UpdateVersion = (uv1 << 48) | (uv2 << 32) | (uv3 << 16) | uv4;
										if (UpdateVersion > FileVersion)
											m_LinkToUpdate = Element[2];
									}
								}
							}
							csLine.Trim(); // makes sure that there's not a line with just whitespace throwing us off
						} while (!csLine.IsEmpty());
					}
				}
				serverFile->Close();
			}
			catch(CInternetException *e)
			{
				TCHAR   szCause[255];
				e->GetErrorMessage(szCause,sizeof(szCause)/sizeof(TCHAR));
				CString csErrorMessage(szCause);
				csErrorMessage.Trim();
				_tprintf(_T("[%s] InternetException: %s(%d)\n"), CTime::GetCurrentTime().Format(_T("%Y-%m-%dT%H:%M:%S")),csErrorMessage.GetString(), e->m_dwError);
			}
			if (serverFile != NULL)
				delete serverFile;
		}
	}
	if (m_LinkToUpdate.IsEmpty())
	{
		m_LinkToUpdateCtrl.SetWindowText(_T("Update not available or not checked"));
		m_LinkToUpdateCtrl.SizeToContent();
		m_LinkToUpdateCtrl.EnableWindow(FALSE);
	}
	else
	{
		m_LinkToUpdateCtrl.SetWindowText(_T("Get available update from RFSAW"));
		m_LinkToUpdateCtrl.SetURL(m_LinkToUpdate);
		m_LinkToUpdateCtrl.SetTooltip(m_LinkToUpdate);
		m_LinkToUpdateCtrl.SizeToContent();
		m_LinkToUpdateCtrl.EnableWindow(TRUE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}

// App command to run the dialog
void CWimTiVoClientApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	TCHAR filename[1024];
	unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
	// Get the file name that we are running from.
	GetModuleFileName(AfxGetResourceHandle(), filename, buffersize );
	aboutDlg.m_csProgramInfo = m_pszAppName + CString(_T(" by ")) + GetFileVersionString(filename,CString(_T("CompanyName")));	// Company that produced the file—for example, "Microsoft Corporation" or "Standard Microsystems Corporation, Inc." This string is required.
	aboutDlg.m_csProgramInfo.Append(_T("\r\nVersion "));
	aboutDlg.m_csProgramInfo.Append(GetFileVersion(filename));
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("LegalCopyright"))));	// Copyright notices that apply to the file. This should include the full text of all notices, legal symbols, copyright dates, and so on. This string is optional.
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	aboutDlg.m_csProgramInfo.Append(filename);
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	aboutDlg.m_csProgramInfo.Append(_T("\r\nBuilt on "));
	aboutDlg.m_csProgramInfo.Append(_T(__DATE__));
	aboutDlg.m_csProgramInfo.Append(_T(" at "));
	aboutDlg.m_csProgramInfo.Append(_T(__TIME__));
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("ProductName"))));	// Name of the product with which the file is distributed. This string is required.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("FileDescription"))));	// File description to be presented to users. This string may be displayed in a list box when the user is choosing files to install—for example, "Keyboard Driver for AT-Style Keyboards". This string is required.
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("InternalName"))));	// Internal name of the file, if one exists—for example, a module name if the file is a dynamic-link library. If the file has no internal name, this string should be the original filename, without extension. This string is required.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("OriginalFilename"))));	// Original name of the file, not including a path. This information enables an application to determine whether a file has been renamed by a user. The format of the name depends on the file system for which the file was created. This string is required.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("Comments"))));	// Additional information that should be displayed for diagnostic purposes.
	aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("LegalTrademarks"))));	// Trademarks and registered trademarks that apply to the file. This should include the full text of all notices, legal symbols, trademark numbers, and so on. This string is optional.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("PrivateBuild"))));	// 	Information about a private version of the file—for example, "Built by TESTER1 on \TESTBED". This string should be present only if VS_FF_PRIVATEBUILD is specified in the fileflags parameter of the root block.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	//aboutDlg.m_csProgramInfo.Append(GetFileVersionString(filename,CString(_T("SpecialBuild"))));	// Text that indicates how this version of the file differs from the standard version—for example, "Private build for TESTER1 solving mouse problems on M250 and M250E computers". This string should be present only if VS_FF_SPECIALBUILD is specified in the fileflags parameter of the root block.
	//aboutDlg.m_csProgramInfo.Append(_T("\r\n"));
	while(aboutDlg.m_csProgramInfo.Replace(_T("\r\n\r\n"),_T("\r\n")));
	aboutDlg.m_csProgramInfo.Trim();
	aboutDlg.m_CheckForUpdate = GetProfileInt(_T("Settings"),_T("CheckForUpdate"), true);
	aboutDlg.DoModal();
	WriteProfileInt(_T("Settings"),_T("CheckForUpdate"), aboutDlg.m_CheckForUpdate);
}

// CWimTiVoClientApp customization load/save methods

void CWimTiVoClientApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CWimTiVoClientApp::LoadCustomState()
{
}

void CWimTiVoClientApp::SaveCustomState()
{
}

// CWimTiVoClientApp message handlers

