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
	std::vector<cTiVoFile> m_FilesToGetFromTiVo;
	std::vector<cTiVoServer> m_TiVoServers;
	CString m_TiVoServerName;
	CString m_csTiVoMAK;

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
