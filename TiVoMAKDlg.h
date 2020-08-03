#pragma once

// CTiVoMAKDlg dialog

class CTiVoMAKDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTiVoMAKDlg)

public:
	CTiVoMAKDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTiVoMAKDlg();

// Dialog Data
	enum { IDD = IDD_TIVO_MAK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_csMediaAccessKey;
};
