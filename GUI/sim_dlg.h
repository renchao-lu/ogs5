#pragma once
#include "afxwin.h"

// CSimulator dialog

class CSimulator : public CDialog
{
	DECLARE_DYNAMIC(CSimulator)
public:
	CSimulator(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSimulator();
// Dialog Data
	enum { IDD = IDD_SIMULATOR };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CString m_strProjectTitle;
    CString m_strProblemType;
    CString m_strGEOFile;
    CString m_strMSHFile;
    CString m_strPCSFile;
    CString m_strNUMFile;
    CString m_strTIMFile;
    CString m_strOUTFile;
    CString m_strICFile;
    CString m_strBCFile;
    CString m_strSTFile;
    CString m_strMFPFile;
    CString m_strMSPFile;
    CString m_strMMPFile;
    CString m_strMCPFile;
    afx_msg void OnBnClickedButtonRunSimulation();
    BOOL m_bCheckGEO;
    BOOL m_bCheckMSH;
    BOOL m_bCheckPCS;
    BOOL m_bCheckNUM;
    BOOL m_bCheckTIM;
    BOOL m_bCheckOUT;
    BOOL m_bCheckIC;
    BOOL m_bCheckBC;
    BOOL m_bCheckST;
    BOOL m_bCheckMFP;
    BOOL m_bCheckMSP;
    BOOL m_bCheckMMP;
    BOOL m_bCheckMCP;
    int m_iNoGEO;
    long m_iNoMSH;
    int m_iNoPCS;
    int m_iNoNUM;
    int m_iNoTIM;
    int m_iNoOUT;
    int m_iNoIC;
    int m_iNoBC;
    int m_iNoST;
    int m_iNoMFP;
    int m_iNoMSP;
    int m_iNoMMP;
    int m_iNoMCP;
    BOOL m_bSimulatorReady;
    // Select by left mouse clicks
    CListBox m_LB_PCS;
    afx_msg void OnLbnSelchangeListPCS();
};
