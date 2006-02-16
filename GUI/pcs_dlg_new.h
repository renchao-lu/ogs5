#pragma once
#include "afxwin.h"
#include "rf_pcs.h"

// CDialogPCS dialog

class CDialogPCS : public CDialog
{
	DECLARE_DYNAMIC(CDialogPCS)

public:
	CDialogPCS(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPCS();

// Dialog Data
	enum { IDD = IDD_PCS_EDITOR };

public:
    CComboBox m_CBProcessH;
    CComboBox m_CBProcessC;
    CComboBox m_CBProcessT;
    CComboBox m_CBProcessM;
    CString m_strProjectName;
    CString m_strPCSProblemType;
    CString m_strPCSName;
    CListBox m_LB_PCS;
    CRFProcess* m_pcs;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnCbnSelchangeComboH();
    afx_msg void OnCbnSelchangeComboT();
    afx_msg void OnCbnSelchangeComboM();
    afx_msg void OnCbnSelchangeComboC();
    afx_msg void OnBnClickedCreate();
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonClear();
    afx_msg void OnBnClickedWrite();
};
