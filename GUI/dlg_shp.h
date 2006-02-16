#pragma once
#include "afxwin.h"


// CDialogSHP dialog

class CDialogSHP : public CDialog
{
	DECLARE_DYNAMIC(CDialogSHP)

public:
	CDialogSHP(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSHP();
    BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_SHP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedFileSHP();
    CString m_strSHPFile;
    void ReadInfo();
    CComboBox m_CB_SHP_PNT;
    CComboBox m_CB_SHP_PLY;
    CString m_strSHPPoint;
    CString m_strSHPPolyline;
    CString m_strGEOBaseName;
    afx_msg void OnBnClickedButtonCreatePNT();
    afx_msg void OnBnClickedButtonCreatePLY();
    afx_msg void OnBnClickedButtonCreateSFC();
    afx_msg void OnBnClickedButtonCreateVOL();
    CComboBox m_CB_GEO_PNT;
    CComboBox m_CB_GEO_PLY;
    CComboBox m_CB_GEO_SFC;
    CComboBox m_CB_GEO_VOL;
    CString m_strGEONumber;
    afx_msg void OnBnClickedButtonGeoWrite();
    CString m_strPathBase;
    CString m_strGSPFilePathBase;
    CString m_strGSPFileBase;
};
