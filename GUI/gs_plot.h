// 06/2004 CC Implementation gs_plot.h for ploting the time-processing concontration of 
//                                                    observation wells.

#pragma once

#include "rf_out_new.h"
#include "GeoSysDoc.h"
// CGSPlot dialog

class CGSPlot : public CDialog
{
	DECLARE_DYNAMIC(CGSPlot)

public:
	CGSPlot(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSPlot();
    COutput *m_out;
    CGeoSysDoc * m_document;
// Dialog Data
	enum { IDD = IDD_DIALOG_PLOT };
    double MaxConcentration;
protected:
    CBrush m_brush; 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};