
// 07/2004 CC CGSKriging interpolation : kriging
#pragma once
#include "afxwin.h"


// CGSKriging dialog

class CGSKriging : public CDialog
{
	DECLARE_DYNAMIC(CGSKriging)

public:
	CGSKriging(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSKriging();

// Dialog Data
	enum { IDD = IDD_DIALOG_KRIGING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    int m_settings;
    CEdit m_ratio;
    double m_ratiovalue;
    CEdit m_angle;
    double m_anglevalue;
   
    CEdit m_nuggetedit;
    double m_nuggetvalue;
    CEdit m_slope;
    double m_slopevalue;
    CEdit m_sill;
    double m_sillvalue;
    CEdit m_range;
    double m_rangevalue;
    int trendtype;
    int variotype;
    
    afx_msg void OnBnClickedOk();
    CComboBox m_variogram;
    CComboBox m_trendbox;
    BOOL m_check;
    BOOL m_nugget;
    afx_msg void OnBnClickedRadioDefaultKriging();
  
    afx_msg void OnBnClickedCheckNugget();
    afx_msg void OnBnClickedCheckTrend();
    afx_msg void OnBnClickedRadioUserdefined3();
    afx_msg void OnCbnSelchangeComboVario();
    afx_msg void OnCbnSelchangeComboTrend();
};