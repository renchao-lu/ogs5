#pragma once


// CIsoColArrangement dialog

class CIsoColArrangement : public CDialog
{
	DECLARE_DYNAMIC(CIsoColArrangement)

public:
	CIsoColArrangement(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIsoColArrangement();

// Dialog Data
	enum { IDD = IDD_ISO_COL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	COLORREF UniformCol;
	BOOL UsingUniformCol;
	afx_msg void OnBnClickedOk();
};
