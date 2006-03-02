#pragma once


// CGSLegend dialog

class CGSLegend : public CDialog
{
	DECLARE_DYNAMIC(CGSLegend)

public:
	CGSLegend(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSLegend();

// Dialog Data
	enum { IDD = IDD_DIALOG_LEGEND };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
};
