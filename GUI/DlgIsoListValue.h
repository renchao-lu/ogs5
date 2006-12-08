
#pragma once
#include "GSPropertyRightResults.h"

// CDlgIsoListValue dialog

class CDlgIsoListValue : public CDialog
{
	DECLARE_DYNAMIC(CDlgIsoListValue)

public:
	CDlgIsoListValue(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgIsoListValue();

// Dialog Data
	enum { IDD = IDD_ISO_VALUE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_max;
	double m_min;
	double m_step;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CView* m_pParent;
protected:
	virtual void PostNcDestroy();
};
