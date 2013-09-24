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

// WimTiVoClientView.h : interface of the CWimTiVoClientView class
//

#pragma once

class CWimTiVoClientView : public CListView
{
protected: // create from serialization only
	CWimTiVoClientView();
	DECLARE_DYNCREATE(CWimTiVoClientView)

// Attributes
public:
	CWimTiVoClientDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CWimTiVoClientView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnTiviNowplaying();
	afx_msg void OnTivoBeacon();
	afx_msg void OnUpdateTivoBeacon(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTivoMak(CCmdUI *pCmdUI);
	afx_msg void OnTivoList();
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTiVoDecode();
	afx_msg void OnUpdateTiVoDecode(CCmdUI *pCmdUI);
	afx_msg void OnFFMPEG();
	afx_msg void OnUpdateFFMPEG(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in WimTiVoClientView.cpp
inline CWimTiVoClientDoc* CWimTiVoClientView::GetDocument() const
   { return reinterpret_cast<CWimTiVoClientDoc*>(m_pDocument); }
#endif
