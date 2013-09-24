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
	ON_UPDATE_COMMAND_UI(ID_TIVO_MAK, &CWimTiVoClientView::OnUpdateTivoMak)
	ON_COMMAND(ID_TIVO_LIST, &CWimTiVoClientView::OnTivoList)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &CWimTiVoClientView::OnLvnColumnclick)
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

	CWimTiVoClientDoc * pDoc = GetDocument();
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
	if ((pRibbon) && (pDoc))
	{
		CMFCRibbonComboBox * TiVoList = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, pRibbon->FindByID(ID_TIVO_LIST));
		if (TiVoList)
		{
			TiVoList->AddItem(_T("Test Value 1"));
			TiVoList->AddItem(_T("Test Value 2"));
			//CUIntArray ports;
			//if (CEnumerateSerial::UsingQueryDosDevice(ports))
			//	for (int i = 0; i < ports.GetSize(); i++)
			//	{
			//		text.Format(_T("\\\\.\\COM%d"), ports[i]);
			//		Model501PortList->AddItem( text );
			//	}
			//Model501PortList->SelectItem(pDoc->m_Model501PortName);
			// Make sure that something is selected, in case that m_PortName didn't match anything in the list.
			if (TiVoList->GetCurSel() == LB_ERR)
				if (TiVoList->GetCount() > 0)
					TiVoList->SelectItem(0);
			//pDoc->m_Model501PortName = Model501PortList->GetItem(Model501PortList->GetCurSel());
		}
	}


	//  its list control through a call to GetListCtrl().
	CListCtrl& ListCtrl = GetListCtrl();
	ListCtrl.ModifyStyle(NULL, LVS_REPORT); // LVS_SORTASCENDING | 
	// see http://msdn.microsoft.com/en-us/library/windows/desktop/bb774732(v=vs.85).aspx
	ListCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// http://www.microsoft.com/mspress/books/sampchap/4110c.aspx
	// insert columns
	ListCtrl.InsertColumn(0, _T("Local File Name"), LVCFMT_LEFT, 300, 0);
	ListCtrl.InsertColumn(1, _T("Title"), LVCFMT_LEFT, 100, 1);
	ListCtrl.InsertColumn(2, _T("EpisodeTitle"), LVCFMT_LEFT, 100, 2);
	ListCtrl.InsertColumn(3, _T("Description"), LVCFMT_LEFT, 100, 3);
	ListCtrl.InsertColumn(4, _T("CaptureDate"), LVCFMT_LEFT, 105, 4);
	ListCtrl.InsertColumn(5, _T("Duration"), LVCFMT_LEFT, 55, 5);
	ListCtrl.InsertColumn(6, _T("Estimated Size"), LVCFMT_RIGHT, 90, 6);

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
		for(auto TiVoFile = pDoc->m_FilesToGetFromTiVo.begin(); TiVoFile != pDoc->m_FilesToGetFromTiVo.end(); TiVoFile++)
		{
			int nItem = ListCtrl.InsertItem(
				LVIF_TEXT | LVIF_STATE,
				TiVoFile-pDoc->m_FilesToGetFromTiVo.begin(), 
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
			std::stringstream ss;
			std::locale mylocale("");   // get global locale
			ss.imbue(mylocale);  // imbue global locale
			ss << TiVoFile->GetSourceSize();
			ListCtrl.SetItemText(nItem, 6, CString(ss.str().c_str()));
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
	pDoc->UpdateAllViews(NULL);
	TRACE(__FUNCTION__ " Exiting\n");
}

void CWimTiVoClientView::OnTivoBeacon()
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		if (pDoc->m_TiVoBeaconListenThreadRunning)
		{
			pDoc->m_TiVoBeaconListenThreadStopRequested = true;
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
		else
		{
			pDoc->m_TiVoBeaconListenThreadStopRequested = false;
			AfxBeginThread(pDoc->TiVoBeaconListenThread, (LPVOID) GetSafeHwnd());
		}
	}
}
void CWimTiVoClientView::OnUpdateTivoBeacon(CCmdUI *pCmdUI)
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
		pCmdUI->SetCheck(pDoc->m_TiVoBeaconListenThreadRunning);
}

void CWimTiVoClientView::OnUpdateTivoMak(CCmdUI *pCmdUI)
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
		if (pRibbon)
		{
			CMFCRibbonEdit* pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(pCmdUI->m_nID));
			if (pEdit)
				pEdit->SetEditText(pDoc->m_csTiVoMAK);
		}
	}
}

void CWimTiVoClientView::OnTivoList()
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
		if (pRibbon)
		{
			CMFCRibbonComboBox * PortList = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, pRibbon->FindByID(ID_TIVO_LIST));
			if (PortList)
			{
				CString csNewItem(PortList->GetItem(PortList->GetCurSel()));
				//if (pDoc->m_Model501PortName.Compare(csNewItem) != 0)
				//	pDoc->m_Model501PortName = csNewItem;
			}
		}
	}
}


void CWimTiVoClientView::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	TRACE(__FUNCTION__ "\n");
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		if (pNMLV->iSubItem == 0)
		{
			static bool forward = true;
			if (forward)
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileComparePath);
			else
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileComparePathReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
		else if (pNMLV->iSubItem == 4)
		{
			static bool forward = true;
			if (forward)
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileCompareDate);
			else
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
		else if (pNMLV->iSubItem == 6)
		{
			static bool forward = true;
			if (forward)
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileCompareSize);
			else
				std::sort(pDoc->m_FilesToGetFromTiVo.begin(),pDoc->m_FilesToGetFromTiVo.end(),cTiVoFileCompareSizeReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
	}
	*pResult = 0;
	TRACE(__FUNCTION__ " Exiting\n");
}
