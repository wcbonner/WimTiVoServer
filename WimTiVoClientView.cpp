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

#define NUM_COLUMNS 6
#define NUM_ITEMS   7
static _TCHAR *_gszColumnLabel[NUM_COLUMNS] =
{
	_T("Local PathName"), 
	_T("Title"), 
	_T("EpisodeTitle"), 
	_T("Description"),
	_T("CaptureDate"), 
	_T("Duration")
};
static int _gnColumnFmt[NUM_COLUMNS] =
{
	LVCFMT_LEFT, 
	LVCFMT_RIGHT, 
	LVCFMT_RIGHT, 
	LVCFMT_RIGHT,
	LVCFMT_RIGHT, 
	LVCFMT_CENTER
};
static int _gnColumnWidth[NUM_COLUMNS] =
{
	150, 
	50, 
	100, 
	50, 
	50, 
	150
};
static _TCHAR *_gszItem[NUM_ITEMS][NUM_COLUMNS] =
{
	_T("Yellow"),  _T("255"), _T("255"), _T("0"),   _T("120"), _T("Neutral"),
	_T("Red"),     _T("255"), _T("0"),   _T("0"),   _T("120"), _T("Warm"),
	_T("Green"),   _T("0"),   _T("255"), _T("0"),   _T("120"), _T("Cool"),
	_T("Magenta"), _T("255"), _T("0"),   _T("255"), _T("120"), _T("Warm"),
	_T("Cyan"),    _T("0"),   _T("255"), _T("255"), _T("120"), _T("Cool"),
	_T("Blue"),    _T("0"),   _T("0"),   _T("255"), _T("120"), _T("Cool"),
	_T("Gray"),    _T("192"), _T("192"), _T("192"), _T("181"), _T("Neutral")
};

void CWimTiVoClientView::OnInitialUpdate()
{
	TRACE(__FUNCTION__ "\n");
	CListView::OnInitialUpdate();
	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
	CListCtrl& ListCtrl = GetListCtrl();
	ListCtrl.ModifyStyle(NULL, LVS_REPORT); // LVS_SORTASCENDING | 
	// see http://msdn.microsoft.com/en-us/library/windows/desktop/bb774732(v=vs.85).aspx
	ListCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	// http://www.microsoft.com/mspress/books/sampchap/4110c.aspx
	// insert columns
	for(auto i = 0; i<NUM_COLUMNS; i++)
	{
		//LV_COLUMN lvc;
		//lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; // | LVCF_SUBITEM;
		//lvc.iSubItem = i;
		//lvc.pszText = _gszColumnLabel[i];
		//lvc.cx = _gnColumnWidth[i];
		//lvc.fmt = _gnColumnFmt[i];
		//ListCtrl.InsertColumn(i,&lvc);
		ListCtrl.InsertColumn(
			i, 
			_gszColumnLabel[i], 
			_gnColumnFmt[i], 
			_gnColumnWidth[i], 
			i);
	}

	// insert items
	for(auto i = 0; i < NUM_ITEMS; i++)
	{
		int nItem = ListCtrl.InsertItem(
			LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, 
			i, 
			_gszItem[i][0], 
			INDEXTOSTATEIMAGEMASK(1), 
			LVIS_STATEIMAGEMASK, 
			i, 
			NULL);
		for(auto j = 1; j < NUM_COLUMNS; j++)
			ListCtrl.SetItemText(nItem, j, _gszItem[i][j]);
	}

	// set item text for additional columns
	//for(auto i = 0; i < NUM_ITEMS; i++)
	//	for(auto j = 1; j < NUM_COLUMNS; j++)
	//		ListCtrl.SetItemText(i, j, _gszItem[i][j]);
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

void CWimTiVoClientView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	TRACE(__FUNCTION__ "\n");
	// TODO: Add your specialized code here and/or call the base class
	CListCtrl& ListCtrl = GetListCtrl();
	BOOL rval = ListCtrl.DeleteAllItems();
	ASSERT(ListCtrl.GetItemCount() == 0);
	CWimTiVoClientDoc * pDoc = GetDocument();
	for(auto TiVoFile = pDoc->FilesToGetFromTiVo.begin(); TiVoFile != pDoc->FilesToGetFromTiVo.end(); TiVoFile++)
	{
		int nItem = ListCtrl.InsertItem(
			LVIF_TEXT | LVIF_STATE | LVCF_SUBITEM, // LVIF_IMAGE | 
			TiVoFile-pDoc->FilesToGetFromTiVo.begin(), 
			TiVoFile->GetPathName(), 
			INDEXTOSTATEIMAGEMASK(1), 
			LVIS_STATEIMAGEMASK, 
			TiVoFile-pDoc->FilesToGetFromTiVo.begin(), 
			NULL);
		ListCtrl.SetItemText(nItem, 1, TiVoFile->GetTitle());
		ListCtrl.SetItemText(nItem, 2, TiVoFile->GetEpisodeTitle());
		ListCtrl.SetItemText(nItem, 3, TiVoFile->GetDescription());
		ListCtrl.SetItemText(nItem, 4, TiVoFile->GetCaptureDate().Format(_T("%c")));
		ListCtrl.SetItemText(nItem, 5, CTimeSpan(TiVoFile->GetDuration()).Format(_T("%H:%M:%S")));
	}
}
