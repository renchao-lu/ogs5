#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "rf_out_new.h"
// COutputDlg dialog

// CDialogOUT dialog

class CDialogOUT : public CDialog
{
	DECLARE_DYNAMIC(CDialogOUT)

public:
	CDialogOUT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogOUT();

// Dialog Data
	enum { IDD = IDD_OUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_PCSType;
    CComboBox m_CB_NODType;
    CComboBox m_CB_ELEType;
    CComboBox m_CB_GEOType;
    CListBox m_LB_GEO;
    CListBox m_LB_TIM;
    CListCtrl m_LC;
    int m_iTIMType;
    double m_dTIMSteps;
    vector<double> order;
    COutput* m_obj;
    CString m_strPCSType;
    int pcs_vector_number;
    CString m_strGEOType;
    CString m_strTIMType;
    CString	m_strGEOName;
    CString m_strNODName;
    CString m_strELEName;
    CString m_strDATType;
    int nSelRow;
public:
    afx_msg void OnCbnSelchangeComboPCSType();
    afx_msg void OnCbnSelchangeComboGEOType();
    afx_msg void OnCbnSelchangeComboNODValues();
    afx_msg void OnCbnSelchangeComboELEValues();
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnBnClickedButtonWrite();
    afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
    CListBox m_LB_VAL_NOD;
    afx_msg void OnBnClickedAddNodValue();
    afx_msg void OnLbnSelchangeListVAL();
    afx_msg void OnBnClickedVALClear();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnLbnSelchangeListValELE();
    CListBox m_LB_VAL_ELE;
    afx_msg void OnBnClickedValELEClear();
    afx_msg void OnBnClickedAddOUTime();
    BOOL m_iDATType;
    void UpdateList();
};
