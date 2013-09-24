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
#include "TiVoMAKDlg.h"

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
	ON_COMMAND(ID_TIVO_LIST, &CWimTiVoClientView::OnTivoList)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &CWimTiVoClientView::OnLvnColumnclick)
	ON_UPDATE_COMMAND_UI(ID_TIVODECODE, &CWimTiVoClientView::OnUpdateTiVoDecode)
	ON_COMMAND(ID_TIVODECODE, &CWimTiVoClientView::OnTiVoDecode)
	ON_COMMAND(ID_FFMPEG, &CWimTiVoClientView::OnFFMPEG)
	ON_UPDATE_COMMAND_UI(ID_FFMPEG, &CWimTiVoClientView::OnUpdateFFMPEG)
	ON_COMMAND(ID_TIVO_GET_FILES, &CWimTiVoClientView::OnTivoGetFiles)
	ON_UPDATE_COMMAND_UI(ID_TIVO_GET_FILES, &CWimTiVoClientView::OnUpdateTivoGetFiles)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CWimTiVoClientView construction/destruction

CWimTiVoClientView::CWimTiVoClientView()
{
	TRACE(__FUNCTION__ "\n");
	// TODO: add construction code here
	m_nWindowTimer = 666;
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
			for (auto TiVo = pDoc->m_TiVoServers.begin(); TiVo != pDoc->m_TiVoServers.end(); TiVo++)
				TiVoList->AddItem(CString(TiVo->m_machine.c_str()));
			TiVoList->SelectItem(pDoc->m_TiVoServerName);
			// Make sure that something is selected, in case that m_PortName didn't match anything in the list.
			if (TiVoList->GetCurSel() == LB_ERR)
				if (TiVoList->GetCount() > 0)
					TiVoList->SelectItem(0);
			pDoc->m_TiVoServerName = TiVoList->GetItem(TiVoList->GetCurSel());
		}
		// I really want to right align most of the numbers in the edit bozes, but when I use the following code it causes the desriptive label to disapear.
		//CMFCRibbonEdit* pEditTotalSize = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TIVO_TOTAL_SIZE));
		//if (pEditTotalSize)
		//{
		//	int TextAlignment = pEditTotalSize->GetTextAlign();
		//	pEditTotalSize->SetTextAlign(ES_RIGHT);
		//}
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

	if (pDoc)
	{
		pDoc->m_TiVoBeaconListenThreadStopRequested = false;
		AfxBeginThread(pDoc->TiVoBeaconListenThread, (LPVOID) GetSafeHwnd());
	}
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
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		CListCtrl& ListCtrl = GetListCtrl();
		if (0 != ListCtrl.DeleteAllItems())
		{
			ASSERT(ListCtrl.GetItemCount() == 0);
			for(auto TiVoFile = pDoc->m_TiVoFiles.begin(); TiVoFile != pDoc->m_TiVoFiles.end(); TiVoFile++)
			{
				int nItem = ListCtrl.InsertItem(
					LVIF_TEXT | LVIF_STATE,
					TiVoFile-pDoc->m_TiVoFiles.begin(), 
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
		CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
		if (pRibbon)
		{
			CMFCRibbonComboBox * TiVoList = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, pRibbon->FindByID(ID_TIVO_LIST));
			if (TiVoList)
			{
				for (auto TiVo = pDoc->m_TiVoServers.begin(); TiVo != pDoc->m_TiVoServers.end(); TiVo++)
					TiVoList->AddItem(CString(TiVo->m_machine.c_str()));
				TiVoList->SelectItem(pDoc->m_TiVoServerName);
				// Make sure that something is selected, in case that m_PortName didn't match anything in the list.
				if (TiVoList->GetCurSel() == LB_ERR)
					if (TiVoList->GetCount() > 0)
						TiVoList->SelectItem(0);
				pDoc->m_TiVoServerName = TiVoList->GetItem(TiVoList->GetCurSel());
			}
			CMFCRibbonEdit* pEditTotalTime = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TIVO_TOTAL_TIME));
			if (pEditTotalTime)
				pEditTotalTime->SetEditText(pDoc->m_TiVoTotalTime.Format(_T("%D Days, %H:%M:%S")));
			CMFCRibbonEdit* pEditTotalSize = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TIVO_TOTAL_SIZE));
			if (pEditTotalSize)
			{
				std::stringstream  junk;
				std::locale mylocale("");   // get global locale
				junk.imbue(mylocale);  // imbue global locale
				junk << pDoc->m_TiVoTotalSize << std::endl;
				pEditTotalSize->SetEditText(CString(junk.str().c_str()));
				//pEditTotalSize->SetTextAlign(ES_RIGHT);
				//pEditTotalSize->SetTextAlwaysOnRight();
			}
			CMFCRibbonEdit* pEditTiVoFileDestination = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TIVO_FILE_LOCATION));
			if (pEditTiVoFileDestination)
				pEditTiVoFileDestination->SetEditText(pDoc->m_csTiVoFileDestination);
		}
	}
	CListView::OnUpdate(pSender, lHint, pHint);
	TRACE(__FUNCTION__ " Exiting\n");
}
void CWimTiVoClientView::OnTiviNowplaying()
{
	TRACE(__FUNCTION__ "\n");
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		cTiVoServer SearchServer;
		SearchServer.m_machine = CStringA(pDoc->m_TiVoServerName);
		auto pServer = std::find(pDoc->m_TiVoServers.begin(), pDoc->m_TiVoServers.end(), SearchServer);
		if (pServer != pDoc->m_TiVoServers.end())
		{
			if (pServer->m_MAK.empty())
			{
				CTiVoMAKDlg myDlg;
				if (IDOK == myDlg.DoModal())
					pServer->m_MAK = CStringA(myDlg.m_csMediaAccessKey).GetString();
			}
			CWaitCursor wait;
			pDoc->GetNowPlaying();
		}
		pDoc->UpdateAllViews(NULL);
	}
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
				if (pDoc->m_TiVoServerName.Compare(csNewItem) != 0)
					pDoc->m_TiVoServerName = csNewItem;
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
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileComparePath);
			else
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileComparePathReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
		else if (pNMLV->iSubItem == 4)
		{
			static bool forward = true;
			if (forward)
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileCompareDate);
			else
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileCompareDateReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
		else if (pNMLV->iSubItem == 6)
		{
			static bool forward = true;
			if (forward)
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileCompareSize);
			else
				std::sort(pDoc->m_TiVoFiles.begin(),pDoc->m_TiVoFiles.end(),cTiVoFileCompareSizeReverse);
			forward = !forward;
			pDoc->UpdateAllViews(NULL);
		}
	}
	*pResult = 0;
	TRACE(__FUNCTION__ " Exiting\n");
}
void CWimTiVoClientView::OnTiVoDecode()
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
		pDoc->m_bTiVoDecode = !pDoc->m_bTiVoDecode;
}
void CWimTiVoClientView::OnUpdateTiVoDecode(CCmdUI *pCmdUI)
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		pCmdUI->Enable(!pDoc->m_csTiVoDecodePath.IsEmpty());
		pCmdUI->SetCheck(pDoc->m_bTiVoDecode);
	}
}
void CWimTiVoClientView::OnFFMPEG()
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
		pDoc->m_bFFMPEG = !pDoc->m_bFFMPEG;
}
void CWimTiVoClientView::OnUpdateFFMPEG(CCmdUI *pCmdUI)
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		pCmdUI->Enable(!pDoc->m_csFFMPEGPath.IsEmpty());
		pCmdUI->SetCheck(pDoc->m_bFFMPEG);
	}
}
void CWimTiVoClientView::OnTivoGetFiles()
{
	TRACE(__FUNCTION__ "\n");
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
	{
		while (!pDoc->m_TiVoFilesToTransfer.empty())
			pDoc->m_TiVoFilesToTransfer.pop();
		pDoc->m_TiVoFilesToTransferTotalSize = 0;
		CListCtrl& ListCtrl = GetListCtrl();
		int nItem = -1;
		while (-1 < (nItem = ListCtrl.GetNextItem(nItem, LVNI_ALL)))
		{
			if (ListCtrl.GetCheck(nItem))
			{
				CString csItem(ListCtrl.GetItemText(nItem,0));
				TRACE(_T("Item %d was Checked: %s\n"), nItem, csItem.GetString());
				CString csPathToCheckExistence(csItem);
				csPathToCheckExistence.Insert(0,pDoc->m_csTiVoFileDestination);
				CFileStatus status;
				if (TRUE != CFile::GetStatus(csPathToCheckExistence, status)) // Only do a bunch of this stuff if the TiVo file doesn't already exist
				{
					csPathToCheckExistence.Replace(_T(".TiVo"), _T(".mpeg"));
					if (TRUE != CFile::GetStatus(csPathToCheckExistence, status)) // Only do a bunch of this stuff if the mpeg file doesn't already exist
					{
						csPathToCheckExistence.Replace(_T(".mpeg"), _T(".mp4"));
						if (TRUE != CFile::GetStatus(csPathToCheckExistence, status)) // Only do a bunch of this stuff if the mpeg file doesn't already exist
						{
							cTiVoFile CrapFile;
							CrapFile.SetPathName(csItem);
							auto TiVoFile = std::find(pDoc->m_TiVoFiles.begin(), pDoc->m_TiVoFiles.end(), CrapFile);
							if (pDoc->m_TiVoFiles.end() != TiVoFile)
							{
								pDoc->m_TiVoFilesToTransferTotalSize += TiVoFile->GetSourceSize();
								pDoc->m_TiVoFilesToTransfer.push(*TiVoFile);
							}
						}
						#ifdef _DEBUG
						else TRACE(_T("File Exists: %s\n"), status.m_szFullName);
						#endif
					}
					#ifdef _DEBUG
					else TRACE(_T("File Exists: %s\n"), status.m_szFullName);
					#endif
				}
				#ifdef _DEBUG
				else TRACE(_T("File Exists: %s\n"), status.m_szFullName);
				#endif
			}
		}
		CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
		if (pRibbon)
		{
			CMFCRibbonEdit* pEditTransferCount = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_COUNT));
			if (pEditTransferCount)
			{
				std::stringstream  junk;
				std::locale mylocale("");   // get global locale
				junk.imbue(mylocale);  // imbue global locale
				junk << pDoc->m_TiVoFilesToTransfer.size();
				pEditTransferCount->SetEditText(CString(junk.str().c_str()));
			}
			CMFCRibbonEdit* pEditTransferSize = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_SIZE));
			if (pEditTransferSize)
			{
				std::stringstream  junk;
				std::locale mylocale("");   // get global locale
				junk.imbue(mylocale);  // imbue global locale
				junk << pDoc->m_TiVoFilesToTransferTotalSize;
				pEditTransferSize->SetEditText(CString(junk.str().c_str()));
			}
		}
		//CFrameWndEx* mainFrm = dynamic_cast<CFrameWndEx*>(GetTopLevelFrame());
		//if (mainFrm)
		//{
		//	mainFrm->SetProgressBarState(TBPF_NORMAL);
		//	mainFrm->SetProgressBarRange(0, 100);
		//	mainFrm->SetProgressBarPosition(pDoc->m_CurrentFileProgress);
		//}
		pDoc->m_TiVoTransferFileThreadStopRequested = false;
		AfxBeginThread(pDoc->TiVoTransferFileThread, (LPVOID) GetSafeHwnd());
		AfxBeginThread(pDoc->TiVoConvertFileThread, (LPVOID) GetSafeHwnd());
		m_nWindowTimer = SetTimer(m_nWindowTimer, 1000, NULL);
	}
	TRACE(__FUNCTION__ " Exiting\n");
}
void CWimTiVoClientView::OnUpdateTivoGetFiles(CCmdUI *pCmdUI)
{
	CWimTiVoClientDoc * pDoc = GetDocument();
	if (pDoc)
		pCmdUI->Enable(!pDoc->m_TiVoTransferFileThreadRunning);
}
void CWimTiVoClientView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (m_nWindowTimer == nIDEvent)
	{
		CWimTiVoClientDoc * pDoc = GetDocument();
		if (pDoc)
		{
			if (!pDoc->m_TiVoTransferFileThreadRunning)
			{
				KillTimer(m_nWindowTimer);
				//CFrameWndEx* mainFrm = dynamic_cast<CFrameWndEx*>(GetTopLevelFrame());
				////CFrameWndEx* mainFrm = dynamic_cast<CFrameWndEx*>(AfxGetApp()->GetMainWnd());
				//if (mainFrm)
				//	mainFrm->SetProgressBarState(TBPF_NOPROGRESS);
			}
			//else
			//{
			//	//CFrameWndEx* mainFrm = dynamic_cast<CFrameWndEx*>(AfxGetApp()->GetMainWnd());
			//	CFrameWndEx* mainFrm = dynamic_cast<CFrameWndEx*>(GetTopLevelFrame());
			//	if (mainFrm)
			//		mainFrm->SetProgressBarPosition(pDoc->m_CurrentFileProgress);
			//}
			CMFCRibbonBar* pRibbon = ((CFrameWndEx*) GetTopLevelFrame())->GetRibbonBar();
			if (pRibbon)
			{
				CMFCRibbonEdit* pEditCurrentFileName = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_CURRENT_FILE_NAME));
				if (pEditCurrentFileName)
					pEditCurrentFileName->SetEditText(pDoc->m_CurrentFileName);
				CMFCRibbonEdit* pEditCurrentFileSize = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_CURRENT_FILE_SIZE));
				if (pEditCurrentFileSize)
				{
					std::stringstream  junk;
					std::locale mylocale("");   // get global locale
					junk.imbue(mylocale);  // imbue global locale
					junk << pDoc->m_CurrentFileSize;
					pEditCurrentFileSize->SetEditText(CString(junk.str().c_str()));
				}
				CMFCRibbonEdit* pEditCurrentSpeed = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_SPEED));
				if (pEditCurrentSpeed)
				{
					std::stringstream  junk;
					std::locale mylocale("");   // get global locale
					junk.imbue(mylocale);  // imbue global locale
					junk << pDoc->m_CurrentFileSpeed << " B/s";
					pEditCurrentSpeed->SetEditText(CString(junk.str().c_str()));
				}
				CMFCRibbonEdit* pEditCurrentETR = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_ETR));
				if (pEditCurrentETR)
					pEditCurrentETR->SetEditText(pDoc->m_CurrentFileEstimatedTimeRemaining.Format(_T("%H:%M:%S")));
				CMFCRibbonProgressBar* pEditCurrentFileProgress = DYNAMIC_DOWNCAST(CMFCRibbonProgressBar, pRibbon->FindByID(ID_TRANSFER_CURRENT_FILE_PROGRESS));
				if (pEditCurrentFileProgress)
					pEditCurrentFileProgress->SetPos(pDoc->m_CurrentFileProgress);
				CMFCRibbonEdit* pEditTransferCount = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_COUNT));
				if (pEditTransferCount)
				{
					std::stringstream  junk;
					std::locale mylocale("");   // get global locale
					junk.imbue(mylocale);  // imbue global locale
					junk << pDoc->m_TiVoFilesToTransfer.size();
					pEditTransferCount->SetEditText(CString(junk.str().c_str()));
				}
				CMFCRibbonEdit* pEditTransferSize = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_SIZE));
				if (pEditTransferSize)
				{
					std::stringstream  junk;
					std::locale mylocale("");   // get global locale
					junk.imbue(mylocale);  // imbue global locale
					junk << pDoc->m_TiVoFilesToTransferTotalSize;
					pEditTransferSize->SetEditText(CString(junk.str().c_str()));
				}
				CMFCRibbonEdit* pEditCurrentTTR = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_TRANSFER_TTR));
				if (pEditCurrentTTR)
					pEditCurrentTTR->SetEditText(pDoc->m_TotalFileEstimatedTimeRemaining.Format(_T("%H:%M:%S")));
			}
		}
	}
	else
		CListView::OnTimer(nIDEvent); // If you pass your own timer event to the parent class it will kill off the timer.
}
