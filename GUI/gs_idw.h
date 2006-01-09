
// 07/2004 CC Implementation gs_idw.h 
#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CGSIdw dialog

class CGSIdw : public CDialog
{
	DECLARE_DYNAMIC(CGSIdw)

public:
	CGSIdw(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSIdw();

// Dialog Data
	enum { IDD = IDD_DIALOG_IDW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnOk();
    int m_settings;
    afx_msg void OnBnClickedRadioUserdefined();
    CEdit m_ratio;
    double m_ratiovalue;
    CEdit m_angle;
    double m_anglevalue;
    CEdit m_power;
    double m_powervalue;
    CEdit m_smoothing;
    double m_smoothingvalue;
    afx_msg void OnBnClickedRadioDefault();
  
};



