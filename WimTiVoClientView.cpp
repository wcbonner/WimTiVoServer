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

// WimTiVoClientView.cpp : implementation of the CWimTiVoClientView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WimTiVoClient.h"
#endif

#include "WimTiVoClientDoc.h"
#include "WimTiVoClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWimTiVoClientView

IMPLEMENT_DYNCREATE(CWimTiVoClientView, CListView)

BEGIN_MESSAGE_MAP(CWimTiVoClientView, CListView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CWimTiVoClientView construction/destruction

CWimTiVoClientView::CWimTiVoClientView()
{
	// TODO: add construction code here
}

CWimTiVoClientView::~CWimTiVoClientView()
{
}

BOOL CWimTiVoClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CListView::PreCreateWindow(cs);
}

void CWimTiVoClientView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

void CWimTiVoClientView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWimTiVoClientView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// CWimTiVoClientView diagnostics

#ifdef _DEBUG
void CWimTiVoClientView::AssertValid() const
{
	CListView::AssertValid();
}

void CWimTiVoClientView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CWimTiVoClientDoc* CWimTiVoClientView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWimTiVoClientDoc)));
	return (CWimTiVoClientDoc*)m_pDocument;
}
#endif //_DEBUG

// CWimTiVoClientView message handlers
