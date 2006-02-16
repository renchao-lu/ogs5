#pragma once
#include "afxwin.h"
#include "rf_mfp_new.h"
#include "afxcmn.h"

// CDialogMFP dialog

class CDialogMFP : public CDialog
{
	DECLARE_DYNAMIC(CDialogMFP)

public:
	CDialogMFP(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogMFP();

// Dialog Data
	enum { IDD = IDD_MAT_MFP_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_MFP_STD;
    CString m_strMFPName;
    CFluidProperties* m_mfp;

    afx_msg void OnCbnSelchangeComboMFPStd();
    CListCtrl m_LC;
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnBnClickedButtonWrite();
};
