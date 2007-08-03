#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "rf_tim_new.h"

// CDialogTimeDiscretization dialog

class CDialogTimeDiscretization : public CDialog
{
	DECLARE_DYNAMIC(CDialogTimeDiscretization)

public:
	CDialogTimeDiscretization(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogTimeDiscretization();
    virtual void OnOK(); //OK
// Dialog Data
	enum { IDD = IDD_TIM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_PCSType;
    CListCtrl m_LC;
    CTimeDiscretization* m_obj;
    afx_msg void OnCbnSelchangeComboPCSType();
    CListBox m_LB_TIM;
    BOOL m_iTIMType;
    CString m_strPCSTypeName;
    int m_iNT;
    double m_dDT;
    afx_msg void OnBnClickedAddTimeSteps();
    afx_msg void OnBnClickedButtonTIMWrite();
    afx_msg void OnBnClickedRemoveTimeSteps();
    double m_dTIMStart;
    double m_dTIMEnd;
    afx_msg void OnBnClickedButtonTIMRemove();
    afx_msg void OnBnClickedButtonTIMCreate();
    CTimeDiscretization* m_tim;
    void UpdateList();
    CComboBox m_CB_TIM_UNIT;
    CString m_strTIMUnitName;
    afx_msg void OnCbnSelchangeComboTimUnit();
};
