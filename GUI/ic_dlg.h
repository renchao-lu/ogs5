#pragma once
#include "afxwin.h"

#include "rf_ic_new.h"

// CDialogInitialConditions dialog

class CDialogInitialConditions : public CDialog
{
	DECLARE_DYNAMIC(CDialogInitialConditions)

public:
	CDialogInitialConditions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogInitialConditions();
    virtual void OnOK(); //OK
// Dialog Data
	//enum { IDD = IDD_INITIAL_CONDITIONS };
	enum { IDD = IDD_DIALOG_IC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_PCSType;
    CComboBox m_CB_PV;
    CComboBox m_CB_GEOType;
    CComboBox m_CB_DISType;
    CListBox m_LB_GEO;
    CString m_strPCSTypeName;
    CString m_strPVName;
    CString m_strGEOTypeName;
    CString m_strDISTypeName;
    CString m_strGEOName;
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
    afx_msg void OnCbnSelchangeComboPVName();
    void FillTable();
};
