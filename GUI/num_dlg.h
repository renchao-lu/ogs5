#pragma once
#include "afxwin.h"

#include "rf_num_new.h"

// CDialogNUM dialog

class CDialogNUM : public CDialog
{
	DECLARE_DYNAMIC(CDialogNUM)

public:
	CDialogNUM(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogNUM();
    virtual void OnOK(); //OK
// Dialog Data
	enum { IDD = IDD_NUM_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_PCSType;
    CString m_strPCSTypeName;
    int m_LinearSolverType;
    int m_NonLinearSolverType;
    int m_WeightingType;
    BOOL m_Preconditioner;
    double m_ErrorTolerance;
    int m_MaxIterationLinear;
    int m_MaxIterationsNonLinear;
    afx_msg void OnBnClickedButtonCreate();
    CNumerics* m_num;
    afx_msg void OnBnClickedButtonWrite();
    CListCtrl m_LC;
    void FillTable();
    afx_msg void OnBnClickedButtonRemove();
};
