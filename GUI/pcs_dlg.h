#pragma once
#include "afxwin.h"
// FEMLib
#include "rf_pcs.h"

// CPCSDlg dialog

class CPCSDlg : public CDialog
{
	DECLARE_DYNAMIC(CPCSDlg)

public:
	CPCSDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPCSDlg();
    friend class CRFProcess;
// Dialog Data
	enum { IDD = IDD_PCS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
//------------------------------------------------------------------------
// Functions
public: 
    afx_msg void OnBnClickedCreate();
//------------------------------------------------------------------------
// Data
protected: 
    CRFProcess *m_pcs;
    int no_pcs;
    vector<string>pcs_type_name_vector;
    CString pcs_type_name; // friend
public:
    afx_msg void OnCbnSelchangeComboH();
    CComboBox m_CBProcessH;
    CComboBox m_CBProcessC;
    CComboBox m_CBProcessT;
    CComboBox m_CBProcessM;
    afx_msg void OnCbnSelchangeComboT();
    afx_msg void OnCbnSelchangeComboM();
    afx_msg void OnCbnSelchangeComboC();
    CString m_strProjectName;
    CString m_strPCSProblemType;
    afx_msg void OnBnClickedPCSRead();
    afx_msg void OnBnClickedPCSWrite();
};
