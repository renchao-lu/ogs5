#pragma once


// CDialogPETREL dialog

class CDialogPETREL : public CDialog
{
	DECLARE_DYNAMIC(CDialogPETREL)

public:
	CDialogPETREL(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPETREL();

// Dialog Data
	enum { IDD = IDD_PETREL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonCreatePly();
    afx_msg void OnBnClickedButtonCreateEle();
};
