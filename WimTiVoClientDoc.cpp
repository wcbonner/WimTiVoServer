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

	for (auto index = 0; index < 16; index++)
	{
		std::stringstream ssKey;
		ssKey << "TiVo-" << index;
		CString regTiVo(AfxGetApp()->GetProfileString(_T("TiVo"),CString(ssKey.str().c_str()),_T("")));
		if (!regTiVo.IsEmpty())
		{
			cTiVoServer myServer;
			myServer.ReadTXT(CStringA(regTiVo).GetString());
			m_ccTiVoServers.Lock();
			m_TiVoServers.push_back(myServer);
			m_ccTiVoServers.Unlock();
		}
	}
	CString TiVoServer(AfxGetApp()->GetProfileString(_T("TiVo"), _T("TiVoServerName")));
	if (!TiVoServer.IsEmpty())
	{
		m_TiVoServer.m_machine = CStringA(TiVoServer).GetString();
		// This next bit makes sure that the MAK and any other attributes came from the list, and not the actual TiVoServer entry. Only the MachineName is important in the TiVoServer entry for consistency
		m_ccTiVoServers.Lock();
		auto pServer = std::find(m_TiVoServers.begin(), m_TiVoServers.end(), m_TiVoServer);
		if (pServer != m_TiVoServers.end())
			m_TiVoServer = *pServer;
		m_ccTiVoServers.Unlock();
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
	m_ccTiVoServers.Lock();
	for (auto TiVo = m_TiVoServers.begin(); TiVo != m_TiVoServers.end(); TiVo++)
	{
		std::stringstream ssKey;
		ssKey << "TiVo-" << (TiVo - m_TiVoServers.begin());
		AfxGetApp()->WriteProfileString(_T("TiVo"), CString(ssKey.str().c_str()), CString(TiVo->WriteTXT().c_str()));
	}
	m_ccTiVoServers.Unlock();
	AfxGetApp()->WriteProfileString(_T("TiVo"), _T("TiVoServerName"), CString(m_TiVoServer.m_machine.c_str()));
	AfxGetApp()->WriteProfileString(_T("TiVo"),_T("TiVoFileDestination"),m_csTiVoFileDestination);
	AfxGetApp()->WriteProfileInt(_T("TiVo"), _T("LogFile"), LogFileClose());
}
bool CWimTiVoClientDoc::LogFileOpen(void)
{
	TCHAR szLogFilePath[MAX_PATH] = _T("");
	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szLogFilePath);
	CString csLogFileName(AfxGetAppName());
	PathAppend(szLogFilePath, csLogFileName.GetString());
	PathAddExtension(szLogFilePath, _T(".txt"));
	m_LogFile.open(szLogFilePath, std::ios_base::out | std::ios_base::app | std::ios_base::ate);
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
	CString csURL(_T("https://192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate"));
	InternetCrackUrl(csURL.GetString(), csURL.GetLength(), ICU_DECODE, &crackedURL);

	std::stringstream ss;
	ss << CStringA(crackedURL.lpszScheme).GetString() << "://";
	ss << "tivo:" << m_TiVoServer.m_MAK << "@";
	ss << m_TiVoServer.m_address << ":" << crackedURL.nPort;
	ss << CStringA(crackedURL.lpszUrlPath).GetString() << CStringA(crackedURL.lpszExtraInfo).GetString();
	csURL = CString(ss.str().c_str());

	if (m_LogFile.is_open())
		m_LogFile << "[" << getTimeISO8601() << "] XML_Parse_TiVoNowPlaying: " << CStringA(csURL).GetString() << std::endl;

	m_TiVoFiles.clear();
	XML_Parse_TiVoNowPlaying(csURL, CString(m_TiVoServer.m_MAK.c_str()), m_TiVoFiles, m_TiVoTiVoContainers, m_InternetSession);

	m_TiVoTotalTime = CTimeSpan::CTimeSpan();
	m_TiVoTotalSize = 0;
	for (auto TiVoFile = m_TiVoFiles.begin(); TiVoFile != m_TiVoFiles.end(); TiVoFile++)
	{
		m_TiVoTotalTime += CTimeSpan(TiVoFile->GetDuration()/1000);
		m_TiVoTotalSize += TiVoFile->GetSourceSize();
	}

	if (m_LogFile.is_open())
		m_LogFile << "[" << getTimeISO8601() << "] TiVoNowPlaying Details: Total Time: " << CStringA(m_TiVoTotalTime.Format(_T("%D Days, %H:%M:%S"))).GetString() << " Total Size: " << m_TiVoTotalSize << std::endl;
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
							bool bNewServer = false;
							pDoc->m_ccTiVoServers.Lock();
							if (pDoc->m_TiVoServers.end() == std::find(pDoc->m_TiVoServers.begin(), pDoc->m_TiVoServers.end(), myServer))
							{
								bNewServer = true;
								if (pDoc->m_LogFile.is_open())
									pDoc->m_LogFile << "[                   ] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << std::endl;
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
								TRACE(_T("URL: %s\n"), csURL.GetString());
								TRACE(_T("Container Count: %d\n"),pDoc->m_TiVoTiVoContainers.size());
								XML_Parse_TiVoNowPlaying(csURL, CString(myServer.m_MAK.c_str()), pDoc->m_TiVoFiles, pDoc->m_TiVoTiVoContainers, pDoc->m_InternetSession);
								TRACE(_T("Container Count: %d\n"),pDoc->m_TiVoTiVoContainers.size());
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
			auto TiVoFile = pDoc->m_TiVoFilesToTransfer.front();
			if (pDoc->GetTiVoFile(TiVoFile))
			{
				RetryCount = 0;
				pDoc->m_TiVoFilesToConvert.push(TiVoFile);
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
	strUsername = _T("tivo");
	strPassword = TiVoFile.GetMAK();
	std::unique_ptr<CHttpConnection> serverConnection(m_InternetSession.GetHttpConnection(strServer,nPort,strUsername,strPassword));
	if (NULL != serverConnection)
	{
		DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | SECURITY_IGNORE_ERROR_MASK;
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
			dwFlags |= INTERNET_FLAG_SECURE;
		if (m_LogFile.is_open())
			m_LogFile << "[" << getTimeISO8601() << "] OpenRequest: " << CStringA(strObject).GetString() << std::endl;
		std::unique_ptr<CHttpFile> serverFile(serverConnection->OpenRequest(1, strObject, NULL, 1, NULL, NULL, dwFlags));
		if (serverFile != NULL)
		{
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
				if(dwRet == HTTP_STATUS_OK)
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
						//std::wcout << L"[                   ] Duration: " << TiVoFile
						if (m_LogFile.is_open())
							m_LogFile << "[" << getTimeISO8601() << "] Writing File: " << CStringA(QuoteFileName(TiVoFile.GetPathName())).GetString() << std::endl;
						m_CurrentFileName = m_csTiVoFileDestination;
						m_CurrentFileName.Append(TiVoFile.GetPathName());
						std::ofstream OutputFile(m_CurrentFileName, std::ios_base::binary);
						if (OutputFile.is_open())
						{
							const size_t ReadWriteBufferSize = 1024*10;
							char ReadWriteBuffer[ReadWriteBufferSize];
							m_CurrentFileSize = 0;
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
							}
							OutputFile.close();
							if (m_LogFile.is_open())
								m_LogFile << "[" << getTimeISO8601() << "] Closing File: " << CStringA(QuoteFileName(TiVoFile.GetPathName())).GetString() << " Total Bytes: " << m_CurrentFileSize << " Transfer Time: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Transfer Speed: " << m_CurrentFileSpeed << " B/s" << std::endl;
							m_CurrentFileEstimatedTimeRemaining = 0;
							m_CurrentFileProgress = 100;
							CFileStatus status;
							if (TRUE == CFile::GetStatus(m_CurrentFileName, status))
							{
								status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
								CFile::SetStatus(m_CurrentFileName, status);
							}
						}
					}
					else
					{
						if (m_LogFile.is_open())
							m_LogFile << "[                   ] not text/xml or video/x-tivo-mpeg" << std::endl;
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
		while (!pDoc->m_TiVoConvertFileThreadStopRequested)
		{
			if (!pDoc->m_TiVoFilesToConvert.empty())
			{
				cTiVoFile TiVoFile = pDoc->m_TiVoFilesToConvert.front();
				pDoc->m_TiVoFilesToConvert.pop();
				CString csTiVoFileName(pDoc->m_csTiVoFileDestination);
				csTiVoFileName.Append(TiVoFile.GetPathName());
				CFileStatus status;
				if (TRUE == CFile::GetStatus(csTiVoFileName, status)) // Test to make sure the .TiVo file exists!
				{
					CString csMPEGPathName(csTiVoFileName);
					csMPEGPathName.Replace(_T(".TiVo"), _T(".mpeg"));
					CString csXMLPathName(csTiVoFileName);
					csXMLPathName.Replace(_T(".TiVo"), _T(".xml"));
					CString csMP4PathName(csTiVoFileName);
					csMP4PathName.Replace(_T(".TiVo"), _T(".mp4"));
					if ((pDoc->m_bTiVoDecode) && (TRUE != CFile::GetStatus(csMPEGPathName, status)))
					{
						if (!pDoc->m_csTDCatPath.IsEmpty())
						{
							if (pDoc->m_LogFile.is_open())
								pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csTDCatPath).GetString() << " " << CStringA(pDoc->m_csTDCatPath).GetString() << "  --mak  " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csXMLPathName)).GetString() << " " << CStringA(csXMLPathName).GetString() << " --chunk-2 " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
							if (-1 == _tspawnl(_P_WAIT, pDoc->m_csTDCatPath.GetString(), pDoc->m_csTDCatPath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csXMLPathName).GetString(), _T("--chunk-2"), QuoteFileName(csTiVoFileName).GetString(), NULL))
								if (pDoc->m_LogFile.is_open())
									pDoc->m_LogFile << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
						}
						if (TRUE == CFile::GetStatus(csXMLPathName, status))
						{
							status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
							CFile::SetStatus(csXMLPathName, status);
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
							DeleteFile(csTiVoFileName);
							if ((pDoc->m_bFFMPEG) && (TRUE != CFile::GetStatus(csMP4PathName, status)))
							{
								CString csTitle(TiVoFile.GetTitle()); while(0 < csTitle.Replace(_T("\""), _T("'"))); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
								CString csShow(TiVoFile.GetTitle()); while(0 < csShow.Replace(_T("\""), _T("'"))); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
								CString csDescription(TiVoFile.GetDescription()); while(0 < csDescription.Replace(_T("\""), _T("'"))); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
								CString csEpisodeID(TiVoFile.GetEpisodeTitle()); while(0 < csEpisodeID.Replace(_T("\""), _T("'"))); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
								if (pDoc->m_LogFile.is_open())
									pDoc->m_LogFile << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(pDoc->m_csFFMPEGPath).GetString() << " " << CStringA(pDoc->m_csFFMPEGPath).GetString() << " -i " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " -metadata " << CStringA(csTitle).GetString() << " -metadata " << CStringA(csShow).GetString() << " -metadata " << CStringA(csDescription).GetString() << " -metadata " << CStringA(csEpisodeID).GetString() << " -vcodec copy -acodec copy -y " << CStringA(QuoteFileName(csMP4PathName)).GetString() << std::endl;
								if (-1 == _tspawnlp(_P_WAIT, pDoc->m_csFFMPEGPath.GetString(), pDoc->m_csFFMPEGPath.GetString(), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
									_T("-metadata"), csTitle.GetString(),
									_T("-metadata"), csShow.GetString(),
									_T("-metadata"), csDescription.GetString(),
									_T("-metadata"), csEpisodeID.GetString(),
									_T("-vcodec"), _T("copy"),
									_T("-acodec"), _T("copy"),
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
		pDoc->m_TiVoConvertFileThreadRunning = false;
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
