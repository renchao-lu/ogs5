// 07/2004 CC Implementation Radial basis function

#pragma once
#include "afxwin.h"


// CGSRbf dialog

class CGSRbf : public CDialog
{
	DECLARE_DYNAMIC(CGSRbf)

public:
	CGSRbf(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSRbf();

// Dialog Data
	enum { IDD = IDD_DIALOG_RBF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    int m_functionvalue;
    int m_settings;
    CEdit m_ratiorbf;
    double m_ratiorbfvalue;
    CEdit m_anglerbf;
    double m_anglerbfvalue;
    CComboBox m_function;
    afx_msg void OnBnClickedOk();
    CEdit m_rsquared;
    double m_rsquaredvalue;
    afx_msg void OnBnClickedRadioDefault2();
    afx_msg void OnBnClickedRadioUserdefined2();
};
