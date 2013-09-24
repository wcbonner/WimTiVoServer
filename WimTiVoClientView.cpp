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
	ON_COMMAND(ID_TIVI_NOWPLAYING, &CWimTiVoClientView::OnTiviNowplaying)
	ON_COMMAND(ID_TIVO_BEACON, &CWimTiVoClientView::OnTivoBeacon)
	ON_UPDATE_COMMAND_UI(ID_TIVO_BEACON, &CWimTiVoClientView::OnUpdateTivoBeacon)
END_MESSAGE_MAP()

// CWimTiVoClientView construction/destruction

CWimTiVoClientView::CWimTiVoClientView()
{
	TRACE(__FUNCTION__ "\n");
	// TODO: add construction code here
}

CWimTiVoClientView::~CWimTiVoClientView()
{
	TRACE(__FUNCTION__ "\n");
}

BOOL CWimTiVoClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	TRACE(__FUNCTION__ "\n");
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CListView::PreCreateWindow(cs);
}

void CWimTiVoClientView::OnInitialUpdate()
{
	TRACE(__FUNCTION__ "\n");
	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
	CListCtrl& ListCtrl = GetListCtrl();
	ListCtrl.ModifyStyle(NULL, LVS_SORTASCENDING | LVS_REPORT);
	// see http://msdn.microsoft.com/en-us/library/windows/desktop/bb774732(v=vs.85).aspx
	ListCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	// http://www.microsoft.com/mspress/books/sampchap/4110c.aspx
	// insert columns
	ListCtrl.InsertColumn(0, _T("Local File Name"), LVCFMT_LEFT, 300, 0);
	ListCtrl.InsertColumn(1, _T("Title"), LVCFMT_LEFT, 100, 1);
	ListCtrl.InsertColumn(2, _T("EpisodeTitle"), LVCFMT_LEFT, 100, 2);
	ListCtrl.InsertColumn(3, _T("Description"), LVCFMT_LEFT, 100, 3);
	ListCtrl.InsertColumn(4, _T("CaptureDate"), LVCFMT_LEFT, 105, 4);
	ListCtrl.InsertColumn(5, _T("Duration"), LVCFMT_LEFT, 55, 5);

	CListView::OnInitialUpdate(); // This needed to be moved to the end because it call's Update, and that manipulated the ListCtrl.
	TRACE(__FUNCTION__ " Exiting\n");
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

void CWimTiVoClientView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	TRACE(__FUNCTION__ "\n");
	// TODO: Add your specialized code here and/or call the base class
	CListCtrl& ListCtrl = GetListCtrl();
	if (0 != ListCtrl.DeleteAllItems())
	{
		ASSERT(ListCtrl.GetItemCount() == 0);
		CWimTiVoClientDoc * pDoc = GetDocument();
		for(auto TiVoFile = pDoc->FilesToGetFromTiVo.begin(); TiVoFile != pDoc->FilesToGetFromTiVo.end(); TiVoFile++)
		{
			int nItem = ListCtrl.InsertItem(
				LVIF_TEXT | LVIF_STATE,
				TiVoFile-pDoc->FilesToGetFromTiVo.begin(), 
				TiVoFile->GetPathName(), 
				INDEXTOSTATEIMAGEMASK(1), 
				LVIS_STATEIMAGEMASK, 
				0,
				NULL);
			ListCtrl.SetItemText(nItem, 1, TiVoFile->GetTitle());
			ListCtrl.SetItemText(nItem, 2, TiVoFile->GetEpisodeTitle());
			ListCtrl.SetItemText(nItem, 3, TiVoFile->GetDescription());
			ListCtrl.SetItemText(nItem, 4, TiVoFile->GetCaptureDate().Format(_T("%c")));
			ListCtrl.SetItemText(nItem, 5, CTimeSpan(TiVoFile->GetDuration()/1000).Format(_T("%H:%M:%S")));
		}
	}
	CListView::OnUpdate(pSender, lHint, pHint);
	TRACE(__FUNCTION__ " Exiting\n");
}

void CWimTiVoClientView::OnTiviNowplaying()
{
	TRACE(__FUNCTION__ "\n");
	CWimTiVoClientDoc * pDoc = GetDocument();
	pDoc->GetNowPlaying();
	// TODO: Add your specialized code here and/or call the base class
	CListCtrl& ListCtrl = GetListCtrl();
	if (0 != ListCtrl.DeleteAllItems())
	{
		ASSERT(ListCtrl.GetItemCount() == 0);
		for(auto TiVoFile = pDoc->FilesToGetFromTiVo.begin(); TiVoFile != pDoc->FilesToGetFromTiVo.end(); TiVoFile++)
		{
			int nItem = ListCtrl.InsertItem(
				LVIF_TEXT | LVIF_STATE,
				TiVoFile-pDoc->FilesToGetFromTiVo.begin(), 
				TiVoFile->GetPathName(), 
				INDEXTOSTATEIMAGEMASK(1), 
				LVIS_STATEIMAGEMASK, 
				0,
				NULL);
			ListCtrl.SetItemText(nItem, 1, TiVoFile->GetTitle());
			ListCtrl.SetItemText(nItem, 2, TiVoFile->GetEpisodeTitle());
			ListCtrl.SetItemText(nItem, 3, TiVoFile->GetDescription());
			ListCtrl.SetItemText(nItem, 4, TiVoFile->GetCaptureDate().Format(_T("%c")));
			ListCtrl.SetItemText(nItem, 5, CTimeSpan(TiVoFile->GetDuration()/1000).Format(_T("%H:%M:%S")));
		}
	}
	TRACE(__FUNCTION__ " Exiting\n");
}

void CWimTiVoClientView::OnTivoBeacon()
{
	// TODO: Add your command handler code here
}

void CWimTiVoClientView::OnUpdateTivoBeacon(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}
