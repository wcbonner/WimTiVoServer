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
	//m_csTiVoMAK = AfxGetApp()->GetProfileString(_T("TiVo"),_T("MAK"),_T("1760168186"));
	m_csTiVoMAK = AfxGetApp()->GetProfileString(_T("TiVo"),_T("MAK"),_T(""));
}

CWimTiVoClientDoc::~CWimTiVoClientDoc()
{
	TRACE(__FUNCTION__ "\n");
	AfxGetApp()->WriteProfileString(_T("TiVo"),_T("MAK"),m_csTiVoMAK);
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
		CInternetSession serverSession0;
		XML_Parse_TiVoNowPlaying(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate")), m_FilesToGetFromTiVo, serverSession0);
		std::sort(m_FilesToGetFromTiVo.begin(),m_FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);
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
		//while ((pDoc->TiVoBeaconListenThreadStopRequested == false) && (TiVoBeaconListen(saServer) == true))
		//	std::cout << "[" << getTimeISO8601() << "] \r";

		do {
			SOCKET theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (theSocket != INVALID_SOCKET)
			{
				SOCKADDR_IN saClient;
				saClient.sin_family = AF_INET;
				saClient.sin_addr.s_addr = INADDR_ANY;
				saClient.sin_port = htons(2190);
				int nRet = bind(theSocket, (LPSOCKADDR)&saClient, sizeof(SOCKADDR_IN));
				if (nRet == SOCKET_ERROR)
				{
					TRACE("%s: %d\n","bind()",WSAGetLastError());
					closesocket(theSocket);
				}
				else
				{
					char szBuf[2048];
					SOCKADDR_IN saServer;
					int nLen = sizeof(SOCKADDR_IN);
					nRet = recvfrom(theSocket, szBuf, sizeof(szBuf), 0, (LPSOCKADDR)&saServer, &nLen);
					if (nRet != INVALID_SOCKET)
					{
						// here's where I should look at what I recieve.
						CStringA csServerBroadcast(szBuf, nRet);
						csServerBroadcast.Replace("\n", " ");
						csServerBroadcast.Trim();
						std::cout << "[                   ] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << std::endl;
						std::cerr << "[                   ] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << std::endl;
					}
					closesocket(theSocket);
				}
			}
		} while (pDoc->m_TiVoBeaconListenThreadStopRequested == false);
		pDoc->m_TiVoBeaconListenThreadRunning = false;
	}
	return(0);
}
