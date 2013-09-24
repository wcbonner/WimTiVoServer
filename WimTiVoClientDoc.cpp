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

// WimTiVoClientDoc.cpp : implementation of the CWimTiVoClientDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WimTiVoClient.h"
#endif

#include "WimTiVoClientDoc.h"
#include "CTiVo.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWimTiVoClientDoc

IMPLEMENT_DYNCREATE(CWimTiVoClientDoc, CDocument)

BEGIN_MESSAGE_MAP(CWimTiVoClientDoc, CDocument)
END_MESSAGE_MAP()

// CWimTiVoClientDoc construction/destruction

CWimTiVoClientDoc::CWimTiVoClientDoc()
{
	TRACE(__FUNCTION__ "\n");
	// TODO: add one-time construction code here
	m_TiVoBeaconListenThreadRunning = false;
	m_TiVoBeaconListenThreadStopRequested = false;
	m_csTiVoMAK = AfxGetApp()->GetProfileString(_T("TiVo"),_T("MAK"),_T(""));
	for (auto index = 0; index < 16; index++)
	{
		std::stringstream ssKey;
		ssKey << "TiVo-" << index;
		CString regTiVo(AfxGetApp()->GetProfileString(_T("TiVo"),CString(ssKey.str().c_str()),_T("")));
		if (!regTiVo.IsEmpty())
		{
			cTiVoServer myServer;
			int SectPos = 0;
			CString csSect(regTiVo.Tokenize(_T("\t"), SectPos));
			while (csSect != _T(""))
			{
				int KeyPos = 0;
				CString csKey(csSect.Tokenize(_T("="),KeyPos));
				CString csValue(csSect.Tokenize(_T("="),KeyPos));
				if (!csKey.CompareNoCase(_T("address")))
					myServer.m_address = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("identity")))
					myServer.m_identity = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("machine")))
					myServer.m_machine = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("method")))
					myServer.m_method = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("platform")))
					myServer.m_platform = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("services")))
					myServer.m_services = CStringA(csValue);
				else if (!csKey.CompareNoCase(_T("swversion")))
					myServer.m_swversion = CStringA(csValue);
				csSect = regTiVo.Tokenize(_T("\t"), SectPos);
			}
			m_TiVoServers.push_back(myServer);
		}
	}
}

CWimTiVoClientDoc::~CWimTiVoClientDoc()
{
	TRACE(__FUNCTION__ "\n");
	AfxGetApp()->WriteProfileString(_T("TiVo"),_T("MAK"),m_csTiVoMAK);
	for (auto TiVo = m_TiVoServers.begin(); TiVo != m_TiVoServers.end(); TiVo++)
	{
		std::stringstream ssKey;
		ssKey << "TiVo-" << (TiVo - m_TiVoServers.begin());
		std::stringstream ssValue;
		ssValue << "address=" << TiVo->m_address;
		ssValue << "\ttivoconnect=1";
		ssValue << "\tmachine=" << TiVo->m_machine;
		ssValue << "\tidentity=" << TiVo->m_identity;
		ssValue << "\tmethod=" << TiVo->m_method;
		ssValue << "\tplatform=" << TiVo->m_platform;
		ssValue << "\tservices=" << TiVo->m_services;
		ssValue << "\tswversion=" << TiVo->m_swversion;
		AfxGetApp()->WriteProfileString(_T("TiVo"), CString(ssKey.str().c_str()), CString(ssValue.str().c_str()));
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CWimTiVoClientDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CWimTiVoClientDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CWimTiVoClientDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CWimTiVoClientDoc diagnostics

#ifdef _DEBUG
void CWimTiVoClientDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWimTiVoClientDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CWimTiVoClientDoc commands

bool CWimTiVoClientDoc::GetNowPlaying(void)
{
	//if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED))) // COINIT_APARTMENTTHREADED
	//{
		//XML_Test_FileReformat(_T("D:\\Videos\\chunk-01-0001.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).1.xml"));
		//XML_Test_FileReformat(_T("D:\\Videos\\chunk-02-0002.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).2.xml"));
		//XML_Test_Read_ElementsOnly();
		//XML_Test_Read();
		//XML_Test_Write();
		//XML_Test_Write_InMemory();
	CString csURL(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate"));
		TCHAR szScheme[_MAX_PATH];
		DWORD dwSchemeLength = sizeof(szScheme) / sizeof(TCHAR);
		TCHAR szHostName[_MAX_PATH];
		DWORD dwHostNameLength = sizeof(szHostName) / sizeof(TCHAR);
		TCHAR szUserName[_MAX_PATH];
		DWORD dwUserNameLength = sizeof(szUserName) / sizeof(TCHAR);
		TCHAR szPassword[_MAX_PATH];
		DWORD dwPasswordLength = sizeof(szPassword) / sizeof(TCHAR);
		TCHAR szUrlPath[_MAX_PATH];
		DWORD dwUrlPathLength = sizeof(szUrlPath) / sizeof(TCHAR);
		TCHAR szExtraInfo[_MAX_PATH];
		DWORD dwExtraInfoLength = sizeof(szExtraInfo) / sizeof(TCHAR);		
		URL_COMPONENTS crackedURL;
		crackedURL.dwStructSize = sizeof(URL_COMPONENTS);
		crackedURL.lpszScheme = szScheme;					// pointer to scheme name
		crackedURL.dwSchemeLength = dwSchemeLength;			// length of scheme name
		crackedURL.nScheme;									// enumerated scheme type (if known)
		crackedURL.lpszHostName = szHostName;				// pointer to host name
		crackedURL.dwHostNameLength = dwHostNameLength;		// length of host name
		crackedURL.nPort;									// converted port number
		crackedURL.lpszUserName = szUserName;				// pointer to user name
		crackedURL.dwUserNameLength = dwUserNameLength;		// length of user name
		crackedURL.lpszPassword = szPassword;				// pointer to password
		crackedURL.dwPasswordLength = dwPasswordLength;		// length of password
		crackedURL.lpszUrlPath = szUrlPath;					// pointer to URL-path
		crackedURL.dwUrlPathLength = dwUrlPathLength;		// length of URL-path
		crackedURL.lpszExtraInfo = szExtraInfo;				// pointer to extra information (e.g. ?foo or #foo)
		crackedURL.dwExtraInfoLength = dwExtraInfoLength;	// length of extra information
		InternetCrackUrl(csURL.GetString(), csURL.GetLength(), ICU_DECODE, &crackedURL);

		std::stringstream ss;
		ss << CStringA(crackedURL.lpszScheme).GetString() << "://";
		ss << CStringA(crackedURL.lpszUserName).GetString() << ":" << CStringA(crackedURL.lpszPassword).GetString() << "@";
		ss << CStringA(crackedURL.lpszHostName).GetString() << ":" << crackedURL.nPort;
		ss << CStringA(crackedURL.lpszUrlPath).GetString() << CStringA(crackedURL.lpszExtraInfo).GetString();
		csURL = CString(ss.str().c_str());

		CInternetSession serverSession0;
		XML_Parse_TiVoNowPlaying(csURL, m_FilesToGetFromTiVo, serverSession0);
	//}
	//CoUninitialize();
	return false;
}

UINT CWimTiVoClientDoc::TiVoBeaconListenThread(LPVOID lvp)
{
	HWND hWnd = reinterpret_cast<HWND>(lvp);
	POSITION DocTemplatePosition = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* DocTemplate = AfxGetApp()->GetNextDocTemplate(DocTemplatePosition);
	POSITION DocumentPosition = DocTemplate->GetFirstDocPosition();
	CWimTiVoClientDoc* pDoc = static_cast<CWimTiVoClientDoc*>(DocTemplate->GetNextDoc(DocumentPosition));
	if (pDoc != 0)
	{
		pDoc->m_TiVoBeaconListenThreadRunning = true;
		do {
			SOCKET theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (theSocket == INVALID_SOCKET)
				pDoc->m_TiVoBeaconListenThreadStopRequested = true;
			else
			{
				SOCKADDR_IN saClient;
				saClient.sin_family = AF_INET;
				saClient.sin_addr.s_addr = INADDR_ANY;
				saClient.sin_port = htons(2190);
				int nRet = bind(theSocket, (LPSOCKADDR)&saClient, sizeof(SOCKADDR_IN));
				if (nRet != SOCKET_ERROR)
				{
					char szBuf[2048];
					SOCKADDR_IN saServer;
					int nLen = sizeof(SOCKADDR_IN);
					nRet = recvfrom(theSocket, szBuf, sizeof(szBuf), 0, (LPSOCKADDR)&saServer, &nLen);
					if (nRet != INVALID_SOCKET)
					{
						// here's where I should look at what I recieve.
						CStringA csServerBroadcast(szBuf, nRet);
						cTiVoServer myServer;
						myServer.m_address = inet_ntoa(saServer.sin_addr);
						int curLine = 0;
						CStringA csLine(csServerBroadcast.Tokenize("\n",curLine));
						while (csLine != "")
						{
							int curPos = 0;
							CStringA Key(csLine.Tokenize("=",curPos));
							CStringA Value(csLine.Tokenize("=",curPos));
							if (!Key.CompareNoCase("identity"))
								myServer.m_identity = Value.GetString();
							else if (!Key.CompareNoCase("swversion"))
								myServer.m_swversion = Value.GetString();
							else if (!Key.CompareNoCase("method"))
								myServer.m_method = Value.GetString();
							else if (!Key.CompareNoCase("machine"))
								myServer.m_machine = Value.GetString();
							else if (!Key.CompareNoCase("platform"))
								myServer.m_platform = Value.GetString();
							else if (!Key.CompareNoCase("services"))
								myServer.m_services = Value.GetString();
							csLine = csServerBroadcast.Tokenize("\n",curLine);
						}
						csServerBroadcast.Replace("\n", " ");
						csServerBroadcast.Trim();
						std::stringstream ss;
						ss << "[                   ] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << std::endl;
						TRACE(ss.str().c_str());
						if (myServer.m_services.find("TiVoMediaServer") == 0)
						{
							if (pDoc->m_TiVoServers.end() == std::find(pDoc->m_TiVoServers.begin(), pDoc->m_TiVoServers.end(), myServer))
								pDoc->m_TiVoServers.push_back(myServer);
						}
					}
				}
				closesocket(theSocket);
			}
		} while (pDoc->m_TiVoBeaconListenThreadStopRequested == false);
		pDoc->m_TiVoBeaconListenThreadRunning = false;
	}
	return(0);
}
