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
static std::string timeToISO8601(const time_t & TheTime)
{
	std::ostringstream ISOTime;
	struct tm UTC;
	gmtime_s(&UTC, &TheTime);
	ISOTime.fill('0');
	ISOTime << UTC.tm_year+1900 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mon+1 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mday << "T";
	ISOTime.width(2);
	ISOTime << UTC.tm_hour << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_min << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_sec;
	return(ISOTime.str());
}
static std::string getTimeISO8601(void)
{
	time_t timer;
	time(&timer);
	return(timeToISO8601(timer));
}
CString FindEXEFromPath(const CString & csEXE)
{
	CString csFullPath;
	//PathFindOnPath();
	CFileFind finder;
	if (finder.FindFile(csEXE))
	{
		finder.FindNextFile();
		csFullPath = finder.GetFilePath();
		finder.Close();
	}
	else
	{
		TCHAR filename[MAX_PATH];
		unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
		// Get the file name that we are running from.
		GetModuleFileName(AfxGetResourceHandle(), filename, buffersize );
		PathRemoveFileSpec(filename);
		PathAppend(filename, csEXE);
		if (finder.FindFile(filename))
		{
			finder.FindNextFile();
			csFullPath = finder.GetFilePath();
			finder.Close();
		}
		else
		{
			CString csPATH;
			csPATH.GetEnvironmentVariable(_T("PATH"));
			int iStart = 0;
			CString csToken(csPATH.Tokenize(_T(";"), iStart));
			while (csToken != _T(""))
			{
				if (csToken.Right(1) != _T("\\"))
					csToken.AppendChar(_T('\\'));
				csToken.Append(csEXE);
				if (finder.FindFile(csToken))
				{
					finder.FindNextFile();
					csFullPath = finder.GetFilePath();
					finder.Close();
					break;
				}
				csToken = csPATH.Tokenize(_T(";"), iStart);
			}
		}
	}
	return(csFullPath);
}
/////////////////////////////////////////////////////////////////////////////
static const CString QuoteFileName(const CString & Original)
{
	CString csQuotedString(Original);
	if (csQuotedString.Find(_T(" ")) >= 0)
	{
		csQuotedString.Insert(0,_T('"'));
		csQuotedString.AppendChar(_T('"'));
	}
	return(csQuotedString);
}
/////////////////////////////////////////////////////////////////////////////
CWimTiVoClientDoc::CWimTiVoClientDoc()
{
	TRACE(__FUNCTION__ "\n");
	// TODO: add one-time construction code here
	m_TiVoBeaconListenThreadRunning = false;
	m_TiVoBeaconListenThreadStopRequested = false;
	m_TiVoTransferFileThreadRunning = false;
	m_TiVoTransferFileThreadStopRequested = false;
	m_TiVoConvertFileThreadRunning = false;
	m_TiVoConvertFileThreadStopRequested = false;
	m_CurrentFileProgress = 0;
	m_TotalFileProgress = 0;
	m_CurrentFileSize = 0;
	m_CurrentFileSpeed = 0;

	//m_TiVoServerName = AfxGetApp()->GetProfileString(_T("TiVo"),_T("TiVoServerName"),_T(""));
	m_TiVoTotalSize = 0;
	m_csFFMPEGPath = FindEXEFromPath(_T("ffmpeg.exe"));
	if (m_csFFMPEGPath.IsEmpty())
		m_bFFMPEG = false;
	else
		m_bFFMPEG = AfxGetApp()->GetProfileInt(_T("TiVo"), _T("FFMPEG"), true);
	m_csTiVoDecodePath = FindEXEFromPath(_T("tivodecode.exe"));
	if (m_csTiVoDecodePath.IsEmpty())
		m_bTiVoDecode = false;
	else
		m_bTiVoDecode = AfxGetApp()->GetProfileInt(_T("TiVo"), _T("TiVoDecode"), true);

	m_csTDCatPath = FindEXEFromPath(_T("tdcat.exe"));

	m_csCCExtractorPath = FindEXEFromPath(_T("ccextractorwinfull.exe"));

	for (auto index = 0; index < 16; index++)
	{
		std::stringstream ssKey;
		ssKey << "Container-" << index;
		CString regTiVo(AfxGetApp()->GetProfileString(_T("TiVo"), CString(ssKey.str().c_str())));
		if (!regTiVo.IsEmpty())
		{
			CTiVoContainer myContainer;
			myContainer.ReadTXT(CStringA(regTiVo).GetString());
			m_ccTiVoContainers.Lock();
			m_TiVoContainers.push_back(myContainer);
			m_ccTiVoContainers.Unlock();
		}
	}
	CString TiVoContainer(AfxGetApp()->GetProfileString(_T("TiVo"), _T("ContainerName")));
	if (!TiVoContainer.IsEmpty())
	{
		m_TiVoContainer.m_title = CStringA(TiVoContainer).GetString();
		// This next bit makes sure that the MAK and any other attributes came from the list, and not the actual TiVoServer entry. Only the MachineName is important in the TiVoServer entry for consistency
		m_ccTiVoContainers.Lock();
		auto pServer = std::find(m_TiVoContainers.begin(), m_TiVoContainers.end(), m_TiVoContainer);
		if (pServer != m_TiVoContainers.end())
			m_TiVoContainer = *pServer;
		m_ccTiVoContainers.Unlock();
	}

	//PWSTR pszPath[MAX_PATH];
	//SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, pszPath); // This is the new format of this command.
	TCHAR szPath[MAX_PATH] = _T("");
	SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, 0, szPath);
	PathAddBackslash(szPath);
	m_csTiVoFileDestination = AfxGetApp()->GetProfileString(_T("TiVo"), _T("TiVoFileDestination"), szPath);
	if (m_csTiVoFileDestination.IsEmpty()) //HACK: This is temporary to fix a minor thing that Fred may have run into.
		m_csTiVoFileDestination = szPath;

	if (AfxGetApp()->GetProfileInt(_T("TiVo"), _T("LogFile"), true)) // I should probably switch this to default to false at some point
		LogFileOpen();
}

CWimTiVoClientDoc::~CWimTiVoClientDoc()
{
	TRACE(__FUNCTION__ "\n");
	while (m_TiVoBeaconListenThreadRunning || m_TiVoTransferFileThreadRunning || m_TiVoConvertFileThreadRunning)
	{
		m_TiVoBeaconListenThreadStopRequested = true;
		m_TiVoTransferFileThreadStopRequested = true;
		m_TiVoConvertFileThreadStopRequested = true;
		if (m_TiVoBeaconListenThreadRunning)
		{
			// Create a UDP/IP datagram socket
			SOCKET theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (theSocket != INVALID_SOCKET)
			{
				BOOL bBroadcastSocket = TRUE;
				int nRet = setsockopt(theSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&bBroadcastSocket, sizeof(bBroadcastSocket));
				if (nRet != SOCKET_ERROR) 
				{
					CStringA csServerBroadcast("tivoconnect=0\n"); // Garbage message that will make the thread accept a packet and check to see if it shoudl continue running.
					SOCKADDR_IN saBroadCast;
					saBroadCast.sin_family = AF_INET;
					saBroadCast.sin_addr.S_un.S_addr = INADDR_BROADCAST;
					saBroadCast.sin_port = htons(2190);	// Port number
					nRet = sendto(theSocket, csServerBroadcast.GetString(), csServerBroadcast.GetLength(), 0, (LPSOCKADDR)&saBroadCast, sizeof(struct sockaddr));
				}
				closesocket(theSocket);
			}
		}
		Sleep(500);
	}
	AfxGetApp()->WriteProfileInt(_T("TiVo"), _T("FFMPEG"), m_bFFMPEG);
	AfxGetApp()->WriteProfileInt(_T("TiVo"), _T("TiVoDecode"), m_bTiVoDecode);
	m_ccTiVoContainers.Lock();
	for (auto Container = m_TiVoContainers.begin(); Container != m_TiVoContainers.end(); Container++)
	{
		std::stringstream ssKey;
		ssKey << "Container-" << (Container - m_TiVoContainers.begin());
		if (!Container->m_MAK.empty())
		{
			CString csURL(Container->m_url.c_str());
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
			if (TRUE == InternetCrackUrl(csURL.GetString(), csURL.GetLength(), ICU_DECODE, &crackedURL))
			{
				std::stringstream ss;
				ss << CStringA(crackedURL.lpszScheme).GetString() << "://";
				ss << "tivo:" << Container->m_MAK << "@";
				ss << CStringA(crackedURL.lpszHostName).GetString() << ":" << crackedURL.nPort;
				ss << CStringA(crackedURL.lpszUrlPath).GetString() << CStringA(crackedURL.lpszExtraInfo).GetString(); // << "&Recurse=Yes" "&SortOrder=!CaptureDate";
				Container->m_url = ss.str();
			}
		}
		AfxGetApp()->WriteProfileString(_T("TiVo"), CString(ssKey.str().c_str()), CString(Container->WriteTXT().c_str()));
	}
	m_ccTiVoContainers.Unlock();
	if (!m_TiVoContainer.m_title.empty()) AfxGetApp()->WriteProfileString(_T("TiVo"), _T("ContainerName"), CString(m_TiVoContainer.m_title.c_str()));
	if (!m_csTiVoFileDestination.IsEmpty()) AfxGetApp()->WriteProfileString(_T("TiVo"),_T("TiVoFileDestination"),m_csTiVoFileDestination);
	AfxGetApp()->WriteProfileInt(_T("TiVo"), _T("LogFile"), LogFileClose());
}
/////////////////////////////////////////////////////////////////////////////
const CString GetLogFileName()
{
	static CString csLogFileName;
	if (csLogFileName.IsEmpty())
	{
		TCHAR szLogFilePath[MAX_PATH] = _T("");
		SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szLogFilePath);
		std::wostringstream woss;
		woss << AfxGetAppName();
		time_t timer;
		time(&timer);
		struct tm UTC;
		if (!gmtime_s(&UTC, &timer))
		{
			woss << "-";
			woss.fill('0');
			woss << UTC.tm_year + 1900 << "-";
			woss.width(2);
			woss << UTC.tm_mon + 1;
		}
		PathAppend(szLogFilePath, woss.str().c_str());
		PathAddExtension(szLogFilePath, _T(".txt"));
		csLogFileName = CString(szLogFilePath);
	}
	return(csLogFileName);
}
/////////////////////////////////////////////////////////////////////////////
bool CWimTiVoClientDoc::LogFileOpen(void)
{
	m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
	if (m_LogFile.is_open())
	{
		std::locale mylocale("");   // get global locale
		m_LogFile.imbue(mylocale);  // imbue global locale
		m_LogFile << "[" << getTimeISO8601() << "] LogFile Opened" << std::endl;
		TCHAR filename[1024];
		unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
		// Get the file name that we are running from.
		GetModuleFileName(AfxGetResourceHandle(), filename, buffersize );
		m_LogFile << "[                   ] Program: " << CStringA(filename).GetString() << std::endl;
		m_LogFile << "[                   ] Version: " << CStringA(GetFileVersion(filename)).GetString() << " Built: " __DATE__ " at " __TIME__ << std::endl;
	}
	return(m_LogFile.is_open());
}
bool CWimTiVoClientDoc::LogFileClose(void)
{
	bool rval = false;
	if (m_LogFile.is_open())
	{
		m_LogFile << "[" << getTimeISO8601() << "] LogFile Closed" << std::endl;
		m_LogFile.close();
		rval = true;
	}
	return(rval);
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
	TRACE(__FUNCTION__ "\n");
	CString csURL(m_TiVoContainer.m_url.c_str());

	if (m_LogFile.is_open())
		m_LogFile << "[" << getTimeISO8601() << "] XML_Parse_TiVoNowPlaying: " << CStringA(csURL).GetString() << " ContainerCount: " << m_TiVoContainers.size() << std::endl;

	m_TiVoFiles.clear();
	std::vector<CTiVoContainer> TiVoContainers;
	if (!XML_Parse_TiVoNowPlaying(csURL, m_TiVoFiles, TiVoContainers, m_InternetSession))
	{
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
		ss << "tivo:" << m_TiVoContainer.m_MAK << "@";
		ss << CStringA(crackedURL.lpszHostName).GetString() << ":" << crackedURL.nPort;
		ss << CStringA(crackedURL.lpszUrlPath).GetString() << CStringA(crackedURL.lpszExtraInfo).GetString() << "&Recurse=Yes"; // "&SortOrder=!CaptureDate";
		XML_Parse_TiVoNowPlaying(CString(ss.str().c_str()), m_TiVoFiles, TiVoContainers, m_InternetSession);
	}
	m_ccTiVoContainers.Lock();
	for (auto Container = TiVoContainers.begin(); Container != TiVoContainers.end(); Container++)
	{
		Container->m_url = DereferenceURL(Container->m_url, CStringA(csURL).GetString());
		if (m_TiVoContainers.end() == std::find(m_TiVoContainers.begin(), m_TiVoContainers.end(), *Container))
			m_TiVoContainers.push_back(*Container);
	}
	m_ccTiVoContainers.Unlock();

	m_TiVoTotalTime = CTimeSpan::CTimeSpan();
	m_TiVoTotalSize = 0;
	for (auto TiVoFile = m_TiVoFiles.begin(); TiVoFile != m_TiVoFiles.end(); TiVoFile++)
	{
		m_TiVoTotalTime += CTimeSpan(TiVoFile->GetDuration()/1000);
		m_TiVoTotalSize += TiVoFile->GetSourceSize();
	}

	if (m_LogFile.is_open())
		m_LogFile << "[" << getTimeISO8601() << "] TiVoNowPlaying Details: Total Time: " << CStringA(m_TiVoTotalTime.Format(_T("%D Days, %H:%M:%S"))).GetString() << " Total Size: " << m_TiVoTotalSize << " ContainerCount: " << m_TiVoContainers.size() << std::endl;
	TRACE(__FUNCTION__ " Exiting\n");
	return false;
}
UINT CWimTiVoClientDoc::TiVoBeaconListenThread(LPVOID lvp)
{
	TRACE(__FUNCTION__ "\n");
	HWND hWnd = reinterpret_cast<HWND>(lvp);
	POSITION DocTemplatePosition = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* DocTemplate = AfxGetApp()->GetNextDocTemplate(DocTemplatePosition);
	POSITION DocumentPosition = DocTemplate->GetFirstDocPosition();
	CWimTiVoClientDoc* pDoc = static_cast<CWimTiVoClientDoc*>(DocTemplate->GetNextDoc(DocumentPosition));
	if (pDoc != 0)
	{
		pDoc->m_TiVoBeaconListenThreadRunning = true;
		#ifdef _DEBUG // This is temporary for debuging the dereference function
		DereferenceURL("/TiVoConnect?Command=QueryContainer&Container=%2F", "http://tivo:password@192.168.0.8/TiVoConnect?Command=QueryContainer&Container=%2F&Recurse=Yes&SortOrder=!CaptureDate");
		#endif
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
				if (nRet == SOCKET_ERROR)
				{
					auto TheError = WSAGetLastError();
					if (WSAEADDRINUSE == TheError)
						pDoc->m_TiVoBeaconListenThreadStopRequested = true;
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
						cTiVoServer myServer;
						myServer.ReadTXT(csServerBroadcast.GetString(), '\n');
						myServer.m_address = inet_ntoa(saServer.sin_addr);
						#ifdef _DEBUG
						std::stringstream ss;
						ss << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << myServer.WriteTXT() << std::endl;
						TRACE(ss.str().c_str());
						#endif
						if (myServer.m_services.find("TiVoMediaServer") == 0)
						{
							bool bNewServer = false;
							pDoc->m_ccTiVoServers.Lock();
							if (pDoc->m_TiVoServers.end() == std::find(pDoc->m_TiVoServers.begin(), pDoc->m_TiVoServers.end(), myServer))
							{
								bNewServer = true;
								pDoc->m_TiVoServers.push_back(myServer);
							}
							pDoc->m_ccTiVoServers.Unlock();
							if (bNewServer)
							{
								// This is where I should get and parse the top level document from the server, populating the containers.
								int curPos = 0;
								CString csServices(myServer.m_services.c_str());
								CString csMediaServer(csServices.Tokenize(_T(":"),curPos));
								CString csMediaServerValue(csServices.Tokenize(_T(":"),curPos));
								curPos = 0;
								CString csMediaServerPort(csMediaServerValue.Tokenize(_T("/"),curPos));
								CString csMediaServerProtocol(csMediaServerValue.Tokenize(_T("/"),curPos));
								CString csURL(csMediaServerProtocol);
								csURL.Append(_T("://"));
								csURL.Append(CString(myServer.m_address.c_str()));
								csURL.Append(_T(":"));
								csURL.Append(csMediaServerPort);
								csURL.Append(_T("/TiVoConnect?Command=QueryContainer&Container=%2F"));
								if (pDoc->m_LogFile.is_open())
								{
									pDoc->m_LogFile << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << myServer.WriteTXT() << std::endl;
									pDoc->m_LogFile << "[                   ] " << CStringA(csURL).GetString() << std::endl;
								}
								TRACE(_T("[                   ] URL: %s\n"), csURL.GetString());
								std::vector<cTiVoFile> TiVoFiles; // Local vector just to reduce multitasking issues
								std::vector<CTiVoContainer> TiVoContainers;
								XML_Parse_TiVoNowPlaying(csURL, TiVoFiles, TiVoContainers, pDoc->m_InternetSession);
								if (!TiVoContainers.empty())
								{
									// This is where I should dereference any URLS in the containers!
									for (auto Container = TiVoContainers.begin(); Container != TiVoContainers.end(); Container++)
										Container->m_url = DereferenceURL(Container->m_url, CStringA(csURL).GetString());
									pDoc->m_ccTiVoContainers.Lock();
									auto OldContainerCount = pDoc->m_TiVoContainers.size();
									TRACE(_T("[                   ] Container Count: %d\n"), OldContainerCount);
									for (auto Container = TiVoContainers.begin(); Container != TiVoContainers.end(); Container++)
									{
										if (pDoc->m_TiVoContainers.end() == std::find(pDoc->m_TiVoContainers.begin(), pDoc->m_TiVoContainers.end(), *Container))
											pDoc->m_TiVoContainers.push_back(*Container);
									}
									if (pDoc->m_TiVoContainers.size() != OldContainerCount)
									{
										TRACE(_T("[                   ] Container Count: %d\n"),pDoc->m_TiVoContainers.size());
										if (pDoc->m_LogFile.is_open())
											for (auto Container = pDoc->m_TiVoContainers.begin(); Container != pDoc->m_TiVoContainers.end(); Container++)
												pDoc->m_LogFile << "[                   ] " << Container->WriteTXT().c_str() << std::endl;
									}
									pDoc->m_ccTiVoContainers.Unlock();
								}
							}
						}
					}
				}
				closesocket(theSocket);
			}
		} while (pDoc->m_TiVoBeaconListenThreadStopRequested == false);
		pDoc->m_TiVoBeaconListenThreadRunning = false;
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
UINT CWimTiVoClientDoc::TiVoTransferFileThread(LPVOID lvp)
{
	TRACE(__FUNCTION__ "\n");
	HWND hWnd = reinterpret_cast<HWND>(lvp);
	POSITION DocTemplatePosition = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* DocTemplate = AfxGetApp()->GetNextDocTemplate(DocTemplatePosition);
	POSITION DocumentPosition = DocTemplate->GetFirstDocPosition();
	CWimTiVoClientDoc* pDoc = static_cast<CWimTiVoClientDoc*>(DocTemplate->GetNextDoc(DocumentPosition));
	if (pDoc != 0)
	{
		pDoc->m_TiVoTransferFileThreadRunning = true;
		int RetryCount = 0;
		while (
			(!pDoc->m_TiVoTransferFileThreadStopRequested) && 
			(!pDoc->m_TiVoFilesToTransfer.empty())
			)
		{
			TRACE(_T("Transfer: %s\n"), pDoc->m_TiVoFilesToTransfer.front().GetPathName().GetString());
			pDoc->m_ccTiVoFilesToConvert.Lock();
			auto TiVoFile = pDoc->m_TiVoFilesToTransfer.front();
			pDoc->m_ccTiVoFilesToConvert.Unlock();
			if (pDoc->GetTiVoFile(TiVoFile))
			{
				RetryCount = 0;
				pDoc->m_ccTiVoFilesToConvert.Lock();
				pDoc->m_TiVoFilesToConvert.push(TiVoFile);
				pDoc->m_ccTiVoFilesToConvert.Unlock();
				pDoc->m_TiVoFilesToTransferTotalSize -= TiVoFile.GetSourceSize();
				TRACE(_T("Pop: %s\n"), TiVoFile.GetPathName().GetString());
				pDoc->m_ccTiVoFilesToTransfer.Lock();
				pDoc->m_TiVoFilesToTransfer.pop();
				pDoc->m_ccTiVoFilesToTransfer.Unlock();
			}
			else
			{
				RetryCount++;
				TRACE(_T("Retry: %d Transfer Failed: %s \n"), RetryCount, TiVoFile.GetPathName().GetString());
				if (RetryCount > 2)
				{
					TRACE(_T("Pop: %s\n"), TiVoFile.GetPathName().GetString());
					pDoc->m_TiVoFilesToTransferTotalSize -= TiVoFile.GetSourceSize();
					pDoc->m_ccTiVoFilesToTransfer.Lock();
					pDoc->m_TiVoFilesToTransfer.pop();
					pDoc->m_ccTiVoFilesToTransfer.Unlock();
				}
			}
		}
		pDoc->m_TiVoTransferFileThreadRunning = false;
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
bool CWimTiVoClientDoc::GetTiVoFile(const cTiVoFile & TiVoFile) //, CInternetSession & serverSession, const CString & csTiVoMAK, const CString & csFileLocation)
{
	bool rval = true;
//	std::cout << "[" << getTimeISO8601() << "] Attempting: " << CStringA(TiVoFile.GetURL()).GetString() << endl;
	DWORD dwServiceType;
	CString strServer;
	CString strObject; 
	INTERNET_PORT nPort; 
	CString strUsername; 
	CString strPassword; 
	AfxParseURLEx(TiVoFile.GetURL().GetString(), dwServiceType, strServer, strObject, nPort, strUsername, strPassword);
	if (!TiVoFile.GetMAK().IsEmpty())
	{
		strUsername = _T("tivo");
		strPassword = TiVoFile.GetMAK();
	}
	std::unique_ptr<CHttpConnection> serverConnection(m_InternetSession.GetHttpConnection(strServer,nPort,strUsername,strPassword));
	if (NULL != serverConnection)
	{
		DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | SECURITY_IGNORE_ERROR_MASK;
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
			dwFlags |= INTERNET_FLAG_SECURE;
		if (m_LogFile.is_open())
			m_LogFile << "[" << getTimeISO8601() << "] OpenRequest: " << CStringA(strObject).GetString() << std::endl;
		LPCTSTR pstrVersion = _T("HTTP/1.1");
		std::unique_ptr<CHttpFile> serverFile(serverConnection->OpenRequest(1, strObject, NULL, 1, NULL, pstrVersion, dwFlags));
		if (serverFile != NULL)
		{
			//Host: 192.168.0.5:64321
			//Range: bytes=0-
			//User-Agent: TvHttpClient
			//tsn: 6520001802E00C3
			//Connection: close
			m_CurrentFileSize = 0;
			std::wstringstream ssRangeHeader;
			ssRangeHeader << "Range: bytes=" << m_CurrentFileSize << "-\r\n";
			serverFile->AddRequestHeaders(ssRangeHeader.str().c_str(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
			serverFile->AddRequestHeaders(_T("User-Agent: TvHttpClient\r\n"));
			serverFile->AddRequestHeaders(_T("tsn: 6520001802E00C3\r\n"));
			int BadCertErrorCount = 0;
			AGAIN:
			try 
			{
				//std::unique_ptr<CHttpFile> serverFile((CHttpFile*) serverSession.OpenURL(*csURL, 1, INTERNET_FLAG_TRANSFER_BINARY));
				serverFile->SendRequest();
				DWORD dwRet;
				serverFile->QueryInfoStatusCode(dwRet);
				#ifdef _DEBUG
				//std::wcout << L"[                   ] Server Status Code: " << dwRet << endl;
				CString headers;
				serverFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF,headers);
				headers.Trim();
				headers.Replace(_T("\r\n"),_T("\r\n[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: "));
				if (m_LogFile.is_open())
					m_LogFile << "[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: " << CStringA(headers).GetString() << std::endl;
				#endif
				if((dwRet == HTTP_STATUS_OK) || (dwRet == HTTP_STATUS_PARTIAL_CONTENT))
				{
					CString csCookie;
					serverFile->QueryInfo(HTTP_QUERY_SET_COOKIE, csCookie);
					if (!csCookie.IsEmpty())
					{
						if (m_LogFile.is_open())
							m_LogFile  << "[                   ] HTTP_QUERY_SET_COOKIE: " << CStringA(csCookie).GetString() << std::endl;
						// sid=1BFA53E13BDF178B; path=/; expires="Saturday, 16-Feb-2013 00:00:00 GMT";
						CString csCookieName(csCookie.Left(csCookie.Find(_T("="))));
						CString csCookieData(csCookie.Left(csCookie.Find(_T(";"))));;
						csCookieData.Delete(0,csCookieData.Find(_T("="))+1);
						CString csCookiePath(csCookie);
						csCookiePath.Delete(0,csCookiePath.Find(_T("path="))+5);
						csCookiePath.Delete(csCookiePath.Find(_T(";")),csCookiePath.GetLength());
						CString csCookieURL;
						if (dwServiceType == AFX_INET_SERVICE_HTTPS)
							csCookieURL = _T("https://");
						else
							csCookieURL = _T("https://");
						csCookieURL.Append(strServer);
						csCookieURL.AppendFormat(_T(":%d"), nPort);
						csCookieURL.Append(csCookiePath);
						if (m_LogFile.is_open())
						{
							m_LogFile << "[                   ] csCookieURL: " << CStringA(csCookieURL).GetString() << std::endl;
							m_LogFile << "[                   ] csCookieName: " << CStringA(csCookieName).GetString() << std::endl;
							m_LogFile << "[                   ] csCookieData: " << CStringA(csCookieData).GetString() << std::endl;
						}
						m_InternetSession.SetCookie(csCookieURL,csCookieName,csCookieData);
					}									
					CString csContentType;
					serverFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, csContentType);
					if (0 < csContentType.Find(_T(";"))) // Fix issue of text/xml; charset=UTF-8
						csContentType.Delete(csContentType.Find(_T(";")),csContentType.GetLength());									
					if (!csContentType.CompareNoCase(_T("video/x-tivo-mpeg")))
					{
						// How can I preallocate disk space for a file without it being reported as readable?
						// https://blogs.msdn.microsoft.com/oldnewthing/20160714-00/?p=93875
						//auto h = CreateFile(L"test.txt", GENERIC_ALL,
						//	FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
						//	FILE_ATTRIBUTE_NORMAL, nullptr);
						//FILE_ALLOCATION_INFO info;
						//info.AllocationSize.QuadPart =
						//	1024LL * 1024LL * 1024LL * 100; // 100GB
						//SetFileInformationByHandle(h, FileAllocationInfo,
						//	&info, sizeof(info));
						//for (int i = 0; i < 10; i++) {
						//	DWORD written;
						//	WriteFile(h, "hello\r\n", 7, &written, nullptr);
						//	Sleep(5000);
						//}
						//CloseHandle(h);
						//std::wcout << L"[                   ] Duration: " << TiVoFile
						if (m_LogFile.is_open())
							m_LogFile << "[" << getTimeISO8601() << "] Writing File: " << CStringA(QuoteFileName(TiVoFile.GetPathName())).GetString() << std::endl;
						m_CurrentFileName = m_csTiVoFileDestination;
						m_CurrentFileName.Append(TiVoFile.GetPathName());
						std::ofstream OutputFile(m_CurrentFileName, std::ios_base::binary);
						if (OutputFile.is_open())
						{
							const size_t ReadWriteBufferSize = 1024*100;
							char ReadWriteBuffer[ReadWriteBufferSize];
							UINT uiRead;
							CTime ctStart(CTime::GetCurrentTime());
							CTimeSpan ctsTotal = CTime::GetCurrentTime() - ctStart;
							while (0 < (uiRead = serverFile->Read(ReadWriteBuffer, ReadWriteBufferSize)))
							{
								OutputFile.write(ReadWriteBuffer,uiRead);
								m_CurrentFileSize += uiRead;
								ctsTotal = CTime::GetCurrentTime() - ctStart;
								m_CurrentFileProgress = 100.0 * (double(m_CurrentFileSize) / double(TiVoFile.GetSourceSize()));
								m_TotalFileProgress = 100.0 * (double(m_CurrentFileSize) / double(m_TiVoFilesToTransferTotalSize));
								auto TotalSeconds = ctsTotal.GetTotalSeconds();
								if (TotalSeconds > 0)
								{
									m_CurrentFileSpeed = (m_CurrentFileSize / TotalSeconds);
									m_CurrentFileEstimatedTimeRemaining = CTimeSpan((TiVoFile.GetSourceSize() - m_CurrentFileSize) / m_CurrentFileSpeed);
									m_TotalFileEstimatedTimeRemaining = CTimeSpan((m_TiVoFilesToTransferTotalSize - m_CurrentFileSize) / m_CurrentFileSpeed);
								}
								if (m_CurrentFileSize > TiVoFile.GetSourceSize())
								{
									m_CurrentFileProgress = 100;
									m_CurrentFileEstimatedTimeRemaining = 0;
								}
							}
							OutputFile.close();
							if (m_LogFile.is_open())
								m_LogFile << "[" << getTimeISO8601() << "] Closing File: " << CStringA(QuoteFileName(TiVoFile.GetPathName())).GetString() << " Total Bytes: " << m_CurrentFileSize << " Transfer Time: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Transfer Speed: " << m_CurrentFileSpeed << " B/s" << std::endl;
							m_CurrentFileEstimatedTimeRemaining = 0;
							m_CurrentFileProgress = 100;
							try
							{
								CFileStatus status;
								if (TRUE == CFile::GetStatus(m_CurrentFileName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
									CFile::SetStatus(m_CurrentFileName, status);
								}
							}
							catch(CFileException *e)
							{
								TCHAR   szCause[512];
								e->GetErrorMessage(szCause,sizeof(szCause)/sizeof(TCHAR));
								CStringA csErrorMessage(szCause);
								csErrorMessage.Trim();
								std::stringstream ss;
								ss << "[" << getTimeISO8601() << "] CFileException: (" << e->m_lOsError << ") " << csErrorMessage.GetString() << std::endl;
								TRACE(ss.str().c_str());
								if (m_LogFile.is_open())
									m_LogFile << ss.str();
							}
						}
						else if (m_LogFile.is_open())
							m_LogFile << "[" << getTimeISO8601() << "] Could not open file: " << CStringA(QuoteFileName(TiVoFile.GetPathName())).GetString() << std::endl;
					}
					else
					{
						if (m_LogFile.is_open())
							m_LogFile << "[" << getTimeISO8601() << "] not text/xml or video/x-tivo-mpeg" << std::endl;
						char ittybittybuffer;
						while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
							std::cout << ittybittybuffer;
						std::cout << std::endl;
					}
				}
				else if (dwRet == HTTP_STATUS_SERVICE_UNAVAIL)
				{
					CString csRetry;
					if (0 < serverFile->QueryInfo(HTTP_QUERY_RETRY_AFTER, csRetry))
					{
						int iRetry = _ttoi(csRetry.GetString());
						if (m_LogFile.is_open())
							m_LogFile << "[" << getTimeISO8601() << "] Sleeping for " << iRetry << " Seconds" << std::endl;
						Sleep(iRetry * 1000);
						rval = false;
					}
				}
				else if (serverFile->GetLength() > 0)
				{
					char ittybittybuffer;
					std::string ss;
					while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
						ss += ittybittybuffer;
					std::cout << "[                   ] Returned File: " << ss << std::endl;
				}
				serverFile->Close();
			}
			catch(CInternetException *e)
			{
				TCHAR   szCause[255];
				e->GetErrorMessage(szCause,sizeof(szCause)/sizeof(TCHAR));
				CStringA csErrorMessage(szCause);
				csErrorMessage.Trim();
				if (m_LogFile.is_open())
					m_LogFile << "[                   ] InternetException: " <<  csErrorMessage.GetString() << " (" << e->m_dwError << ") " << std::endl;
				if ((e->m_dwError == ERROR_INTERNET_INVALID_CA) || 
					(e->m_dwError == ERROR_INTERNET_SEC_CERT_CN_INVALID) ||
					(e->m_dwError == ERROR_INTERNET_SEC_CERT_DATE_INVALID) ||
					(e->m_dwError == ERROR_INTERNET_SEC_INVALID_CERT) )
				{
					serverFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, SECURITY_SET_MASK);
					if (BadCertErrorCount++ < 2)
						goto AGAIN;
					rval = false;
				}
			}
		}
	}
	return(rval);
}
UINT CWimTiVoClientDoc::TiVoConvertFileThread(LPVOID lvp)
{
	TRACE(__FUNCTION__ "\n");
	HWND hWnd = reinterpret_cast<HWND>(lvp);
	POSITION DocTemplatePosition = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* DocTemplate = AfxGetApp()->GetNextDocTemplate(DocTemplatePosition);
	POSITION DocumentPosition = DocTemplate->GetFirstDocPosition();
	CWimTiVoClientDoc* pDoc = static_cast<CWimTiVoClientDoc*>(DocTemplate->GetNextDoc(DocumentPosition));
	if (pDoc != 0)
	{
		pDoc->m_TiVoConvertFileThreadRunning = true;
		if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED))) // COINIT_APARTMENTTHREADED
		{
			while (!pDoc->m_TiVoConvertFileThreadStopRequested)
			{
				if (!pDoc->m_TiVoFilesToConvert.empty())
				{
					pDoc->m_ccTiVoFilesToConvert.Lock();
					cTiVoFile TiVoFile = pDoc->m_TiVoFilesToConvert.front();
					pDoc->m_TiVoFilesToConvert.pop();
					pDoc->m_ccTiVoFilesToConvert.Unlock();
					CString csTiVoFileName(pDoc->m_csTiVoFileDestination);
					csTiVoFileName.Append(TiVoFile.GetPathName());
					CFileStatus status;
					if (TRUE == CFile::GetStatus(csTiVoFileName, status)) // Test to make sure the .TiVo file exists!
					{
						CString csMPEGPathName(csTiVoFileName);
						csMPEGPathName.Replace(_T(".TiVo"), _T(".mpeg"));
						CString csMP4PathName(csTiVoFileName);
						csMP4PathName.Replace(_T(".TiVo"), _T(".mp4"));
						if ((pDoc->m_bTiVoDecode) && (TRUE != CFile::GetStatus(csMPEGPathName, status)))
						{
							if (!pDoc->m_csTDCatPath.IsEmpty())
							{
								CString csXMLPathName(csTiVoFileName);
								csXMLPathName.Replace(_T(".TiVo"), _T(".xml"));
								TCHAR lpTempPathBuffer[MAX_PATH];
								DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
								if (dwRetVal > MAX_PATH || (dwRetVal == 0))
									_tcscpy_s(lpTempPathBuffer, MAX_PATH, _T("."));
								//  Generates a temporary file name. 
								TCHAR szTempFileName[MAX_PATH];  
								UINT uRetVal = GetTempFileName(lpTempPathBuffer, AfxGetAppName(), 0, szTempFileName);
								if (uRetVal == 0)
									std::cout << "[" << getTimeISO8601() << "] GetTempFileName failed" << std::endl;
								if (pDoc->m_LogFile.is_open())
									pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csTDCatPath).GetString() << " " << CStringA(pDoc->m_csTDCatPath).GetString() << "  --mak  " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(szTempFileName)).GetString() << " " << " --chunk-2 " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
								if (-1 == _tspawnl(_P_WAIT, pDoc->m_csTDCatPath.GetString(), pDoc->m_csTDCatPath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), szTempFileName, _T("--chunk-2"), QuoteFileName(csTiVoFileName).GetString(), NULL))
									if (pDoc->m_LogFile.is_open())
										pDoc->m_LogFile << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
								// crap to clean up a file...  
								HRESULT hr = S_OK;
								CComPtr<IXmlReader> pReader; 
								if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
								{
									if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
									{
										CComPtr<IStream> spFileStreamOne;
										if (SUCCEEDED(hr = SHCreateStreamOnFile(szTempFileName, STGM_READ, &spFileStreamOne)))
										{
											if (SUCCEEDED(hr = pReader->SetInput(spFileStreamOne))) 
											{
												CComPtr<IXmlWriter> pWriter;
												if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
												{
													CComPtr<IStream> spFileStreamTwo;
													if (SUCCEEDED(hr = SHCreateStreamOnFile(csXMLPathName.GetString(), STGM_CREATE | STGM_WRITE, &spFileStreamTwo)))
													{
														if (SUCCEEDED(hr = pWriter->SetOutput(spFileStreamTwo)))
														{
															pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
															while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
															pWriter->Flush();
														}
													}
												}
											}
										}
									}
								}
								DeleteFile(szTempFileName);	// I must delete this file because the zero in the unique field up above causes a file to be created.
								if (TRUE == CFile::GetStatus(csXMLPathName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
									CFile::SetStatus(csXMLPathName, status);
								}
								#ifdef _DEBUG
								{
									csXMLPathName.Replace(_T(".xml"), _T(".chunk-1.xml"));
									uRetVal = GetTempFileName(lpTempPathBuffer, AfxGetAppName(), 0, szTempFileName);
									if (uRetVal == 0)
										std::cout << "[" << getTimeISO8601() << "] GetTempFileName failed" << std::endl;
									if (pDoc->m_LogFile.is_open())
										pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csTDCatPath).GetString() << " " << CStringA(pDoc->m_csTDCatPath).GetString() << "  --mak  " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(szTempFileName)).GetString() << " " << " --chunk-1 " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
									if (-1 == _tspawnl(_P_WAIT, pDoc->m_csTDCatPath.GetString(), pDoc->m_csTDCatPath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), szTempFileName, _T("--chunk-1"), QuoteFileName(csTiVoFileName).GetString(), NULL))
										if (pDoc->m_LogFile.is_open())
											pDoc->m_LogFile << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
									// crap to clean up a file...  
									HRESULT hr = S_OK;
									CComPtr<IXmlReader> pReader; 
									if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
									{
										if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
										{
											CComPtr<IStream> spFileStreamOne;
											if (SUCCEEDED(hr = SHCreateStreamOnFile(szTempFileName, STGM_READ, &spFileStreamOne)))
											{
												if (SUCCEEDED(hr = pReader->SetInput(spFileStreamOne))) 
												{
													CComPtr<IXmlWriter> pWriter;
													if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
													{
														CComPtr<IStream> spFileStreamTwo;
														if (SUCCEEDED(hr = SHCreateStreamOnFile(csXMLPathName.GetString(), STGM_CREATE | STGM_WRITE, &spFileStreamTwo)))
														{
															if (SUCCEEDED(hr = pWriter->SetOutput(spFileStreamTwo)))
															{
																pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
																while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
																pWriter->Flush();
															}
														}
													}
												}
											}
										}
									}
									DeleteFile(szTempFileName);	// I must delete this file because the zero in the unique field up above causes a file to be created.
								}
								if (TRUE == CFile::GetStatus(csXMLPathName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
									CFile::SetStatus(csXMLPathName, status);
								}
								#endif
							}

							if (pDoc->m_LogFile.is_open())
								pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csTiVoDecodePath).GetString() << " " << CStringA(pDoc->m_csTiVoDecodePath).GetString() << " --mak " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
							if (-1 == _tspawnl(_P_WAIT, pDoc->m_csTiVoDecodePath.GetString(), pDoc->m_csTiVoDecodePath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csMPEGPathName).GetString(), QuoteFileName(csTiVoFileName).GetString(), NULL))
								if (pDoc->m_LogFile.is_open())
									pDoc->m_LogFile << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
							if (TRUE == CFile::GetStatus(csMPEGPathName, status))
							{
								status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
								CFile::SetStatus(csMPEGPathName, status);
								if (!pDoc->m_csCCExtractorPath.IsEmpty())
								{
									_tspawnl(_P_WAIT, pDoc->m_csCCExtractorPath.GetString(), pDoc->m_csCCExtractorPath.GetString(), QuoteFileName(csTiVoFileName).GetString(), NULL);
									CString csSRTPathName(csTiVoFileName);
									csSRTPathName.Replace(_T(".TiVo"), _T(".srt"));
									if (TRUE == CFile::GetStatus(csSRTPathName, status))
									{
										status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
										CFile::SetStatus(csSRTPathName, status);
									}
								}
								DeleteFile(csTiVoFileName);
								if ((pDoc->m_bFFMPEG) && (TRUE != CFile::GetStatus(csMP4PathName, status)))
								{
									CString csTitle(TiVoFile.GetTitle()); while(0 < csTitle.Replace(_T("\""), _T("'"))); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
									CString csShow(TiVoFile.GetTitle()); while(0 < csShow.Replace(_T("\""), _T("'"))); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
									CString csDescription(TiVoFile.GetDescription()); while(0 < csDescription.Replace(_T("\""), _T("'"))); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
									CString csEpisodeID(TiVoFile.GetEpisodeTitle()); while(0 < csEpisodeID.Replace(_T("\""), _T("'"))); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
									if (pDoc->m_LogFile.is_open())
									{ 
										pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csFFMPEGPath).GetString() << " " << CStringA(pDoc->m_csFFMPEGPath).GetString();
										pDoc->m_LogFile << " -i " << CStringA(QuoteFileName(csMPEGPathName)).GetString();
										pDoc->m_LogFile << " -metadata " << CStringA(csTitle).GetString();
										pDoc->m_LogFile << " -metadata " << CStringA(csShow).GetString();
										pDoc->m_LogFile << " -metadata " << CStringA(csDescription).GetString();
										pDoc->m_LogFile << " -metadata " << CStringA(csEpisodeID).GetString();
										//pDoc->m_LogFile << " -vcodec copy";
										//pDoc->m_LogFile << " -acodec copy ";
										pDoc->m_LogFile << " -profile:v high";
										pDoc->m_LogFile << " -level 4.1";
										pDoc->m_LogFile << "-y " << CStringA(QuoteFileName(csMP4PathName)).GetString() << std::endl;
									}
									if (-1 == _tspawnlp(_P_WAIT, pDoc->m_csFFMPEGPath.GetString(), pDoc->m_csFFMPEGPath.GetString(), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
										_T("-metadata"), csTitle.GetString(),
										_T("-metadata"), csShow.GetString(),
										_T("-metadata"), csDescription.GetString(),
										_T("-metadata"), csEpisodeID.GetString(),
										//_T("-vcodec"), _T("copy"),
										//_T("-acodec"), _T("copy"),
										_T("-profile:v"), _T("high"), // This came from https://trac.ffmpeg.org/wiki/Encode/H.264 settings for iPad 2 and above.
										_T("-level"), _T("4.1"),
										_T("-y"), // Cause it to overwrite exiting output files
										QuoteFileName(csMP4PathName).GetString(), NULL))
										if (pDoc->m_LogFile.is_open())
											pDoc->m_LogFile << "[                   ]  _tspawnlp failed: " /* << _sys_errlist[errno] */ << std::endl;
								}
								if (TRUE == CFile::GetStatus(csMP4PathName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
									CFile::SetStatus(csMP4PathName, status);
									DeleteFile(csMPEGPathName);
								}
							}
						}
					}
				}
				else
					Sleep(1000);
			}
			CoUninitialize();
		}
		pDoc->m_TiVoConvertFileThreadRunning = false;
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
