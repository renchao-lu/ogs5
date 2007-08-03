

#pragma once
#include "ViewPropertyPage.h"
#include "resource.h"
#include "afxcmn.h"
#include "GeoSysDoc.h"
#include "afxwin.h"

// CGSPropertyRightPCS dialog

class CGSPropertyRightPCS : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightPCS)

public:
	CGSPropertyRightPCS();
	virtual ~CGSPropertyRightPCS();
    virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_CONTROL_RIGHT_PCS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonPCS();
    afx_msg void OnBnClickedButtonTIM();
    afx_msg void OnBnClickedButtonNUM();
    afx_msg void OnBnClickedButtonOUT();
    afx_msg void OnBnClickedButtonIC();
    afx_msg void OnBnClickedButtonBC();
    afx_msg void OnBnClickedButtonST();
    afx_msg void OnBnClickedButtonMFP();
    afx_msg void OnBnClickedButtonMSP();
    afx_msg void OnBnClickedButtonMCP();
    afx_msg void OnBnClickedButtonMMP();
    BOOL m_check_pcs;
    BOOL m_check_tim;
    BOOL m_check_num;
    BOOL m_check_out;
    BOOL m_check_ic;
    BOOL m_check_bc;
    BOOL m_check_st;
    BOOL m_check_mfp;
    BOOL m_check_msp;
    BOOL m_check_mcp;
    BOOL m_check_mmp;
    BOOL m_check_mod;
    afx_msg void OnBnClickedButtonMOD();
    afx_msg void OnBnClickedButtonMODCheck();
    afx_msg void OnBnClickedButtonUpdateTree();
    CComboBox m_CB_PCS;
    afx_msg void OnCbnSelchangeComboPCS();
    CString m_strPCSTypeName;
    afx_msg void OnBnClickedButtonMODRead();
    afx_msg void OnBnClickedButtonMODEditor();
};
