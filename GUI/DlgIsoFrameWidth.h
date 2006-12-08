#pragma once


// CDlgIsoFrameWidth dialog

class CDlgIsoFrameWidth : public CDialog
{
	DECLARE_DYNAMIC(CDlgIsoFrameWidth)

public:
	CDlgIsoFrameWidth(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgIsoFrameWidth();

// Dialog Data
	enum { IDD = IDD_ISO_FRAMEWIDTH};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double Iso_Frame_Width;
	afx_msg void OnBnClickedOk();
};
