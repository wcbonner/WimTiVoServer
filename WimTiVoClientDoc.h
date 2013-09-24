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

// WimTiVoClientDoc.h : interface of the CWimTiVoClientDoc class
//

#pragma once

class CWimTiVoClientDoc : public CDocument
{
protected: // create from serialization only
	CWimTiVoClientDoc();
	DECLARE_DYNCREATE(CWimTiVoClientDoc)

// Attributes
public:
	std::vector<CTiVoContainer> m_TiVoTiVoContainers;
	std::vector<cTiVoFile> m_TiVoFiles;
	CCriticalSection m_ccTiVoServers;
	std::vector<cTiVoServer> m_TiVoServers;	// List of servers, populated either from the registry or listening for TiVo UDP Beacons
	cTiVoServer m_TiVoServer; // The Selected TiVo Server
	//CString m_TiVoServerName;	// Selected TiVo name
	//CString m_csTiVoMAK;	// Media Access Key to log into TiVo and also to decode TiVo transferred files
	CInternetSession m_InternetSession;
	CTimeSpan m_TiVoTotalTime;			// Total Time of files reported by TiVo
	unsigned long long m_TiVoTotalSize;	// Total size of files reported by TiVo
	CString m_csFFMPEGPath;		// Location of the FFMPEG.exe program
	CString m_csTiVoDecodePath;	// Location of the TiVoDecode.exe program
	CString m_csTDCatPath;	// Location of the TiVoDecode.exe program
	bool m_bFFMPEG;	// Do we want to convert mpg to an MP4 container file with metadata?
	bool m_bTiVoDecode;	// Do we want to convert from a .TiVo file to an mpg file with no metadata?
	CString m_csTiVoFileDestination;	// Location that we will download tivo files
	std::queue<cTiVoFile> m_TiVoFilesToTransfer;
	unsigned long long m_TiVoFilesToTransferTotalSize;
	CTimeSpan m_TotalFileEstimatedTimeRemaining;
	CString m_CurrentFileName;
	int m_CurrentFileProgress;
	int m_TotalFileProgress;
	unsigned long long m_CurrentFileSize;
	unsigned long long m_CurrentFileSpeed;
	CTimeSpan m_CurrentFileEstimatedTimeRemaining;
	std::queue<cTiVoFile> m_TiVoFilesToConvert;
	std::ofstream m_LogFile;

// Operations
public:

// Overrides
public:
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CWimTiVoClientDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	bool m_TiVoBeaconListenThreadRunning;
	bool m_TiVoBeaconListenThreadStopRequested;
	static UINT TiVoBeaconListenThread(LPVOID lvp);
	bool m_TiVoTransferFileThreadRunning;
	bool m_TiVoTransferFileThreadStopRequested;
	static UINT TiVoTransferFileThread(LPVOID lvp);
	bool GetTiVoFile(const cTiVoFile & TiVoFile);
	bool m_TiVoConvertFileThreadRunning;
	bool m_TiVoConvertFileThreadStopRequested;
	static UINT TiVoConvertFileThread(LPVOID lvp);

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	bool GetNowPlaying(void);
};
