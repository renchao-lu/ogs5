#pragma once

#include "rf_ic_new.h"

// CDialogInitialConditionsNew dialog

class CDialogInitialConditionsNew : public CDialog
{
	DECLARE_DYNAMIC(CDialogInitialConditionsNew)

public:
	CDialogInitialConditionsNew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogInitialConditionsNew();
    virtual void OnOK(); //OK
// Dialog Data
	enum { IDD = IDD_DIALOG_IC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_PCSType;
    CComboBox m_CB_GEOType;
    CComboBox m_CB_DISType;
    CListBox m_LB_GEO;
    CString m_strPCSName;
    CString m_strGEOName;
    CString m_strDISName;
    CListCtrl m_LC;
    double m_dValue;
    CInitialCondition* m_obj;
public:
    afx_msg void OnCbnSelchangeComboPCSType();
    afx_msg void OnCbnSelchangeComboGEOType();
    afx_msg void OnCbnSelchangeComboDISType();
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnBnClickedButtonCreateGroup();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnBnClickedButtonRead();
    afx_msg void OnBnClickedButtonReadData();
    afx_msg void OnBnClickedButtonWrite();
    afx_msg void OnBnClickedButtonWriteTEC();
};
