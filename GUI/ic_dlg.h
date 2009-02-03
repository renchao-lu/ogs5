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
    CListBox m_LB_GEO;
    CString m_strPCSTypeName;
    CString m_strPVName;
    CString m_strGEOTypeName;
    CString m_strDISTypeName;
    CString m_strGEOName;
    CListCtrl m_LC;
    CInitialCondition* m_obj;
    int m_RB_DISType_CONSTANT;
    int m_RB_DISType_GRADIENT;
    int m_RB_DISType_DATA_BASE;
    double m_dDIS_CONSTANT_Value;
    double m_dDIS_GRADIENT_Gradient;
    double m_dDIS_GRADIENT_RefX;
    double m_dDIS_GRADIENT_RefY;

public:
    afx_msg void OnCbnSelchangeComboPCSType();
    afx_msg void OnCbnSelchangeComboGEOType();
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnBnClickedButtonCreateGroup();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnBnClickedButtonRead();
    afx_msg void OnBnClickedButtonReadData();
    afx_msg void OnBnClickedButtonWrite();
    afx_msg void OnBnClickedButtonWriteTEC();
    afx_msg void OnCbnSelchangeComboPVName();
    afx_msg void OnBnClickedRadioDisConstant();
    afx_msg void OnBnClickedRadioDisGradient();

    void FillTable();
    afx_msg void OnBnClickedButtonIcDlgClose();
    afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
};
