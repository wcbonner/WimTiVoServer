// TiVoMAKDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WimTiVoClient.h"
#include "TiVoMAKDlg.h"
#include "afxdialogex.h"

// CTiVoMAKDlg dialog

IMPLEMENT_DYNAMIC(CTiVoMAKDlg, CDialogEx)

CTiVoMAKDlg::CTiVoMAKDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTiVoMAKDlg::IDD, pParent)
	, m_csMediaAccessKey(_T(""))
{
}

CTiVoMAKDlg::~CTiVoMAKDlg()
{
}

void CTiVoMAKDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MEDIA_ACCESS_KEY, m_csMediaAccessKey);
}

BEGIN_MESSAGE_MAP(CTiVoMAKDlg, CDialogEx)
END_MESSAGE_MAP()

// CTiVoMAKDlg message handlers
