
// 07/2004 CC implementation gs_interpolate.h
#pragma once
#include "afxwin.h"


// GSInterpolate dialog

class GSInterpolate : public CDialog
{
	DECLARE_DYNAMIC(GSInterpolate)

public:
	GSInterpolate(CWnd* pParent = NULL);   // standard constructor
	virtual ~GSInterpolate();

// Dialog Data
	enum { IDD = IDD_DIALOG_INTERPOLATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:

    CListBox m_variable;
    int RBFtype;
    int trendtype;
    int variotype;
    double m_ratiovalue;
    double m_anglevalue;
    double m_powervalue;
    double m_smoothingvalue;
    double m_r2value;
    afx_msg void OnBnClickedButtonAdvanced();
   
    CString m_basename;
    afx_msg void OnBnClickedInterpolate();
    CComboBox m_method;
};