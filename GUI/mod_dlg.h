#pragma once
#include "afxwin.h"


// CDialogMOD dialog

class CDialogMOD : public CDialog
{
	DECLARE_DYNAMIC(CDialogMOD)

public:
	CDialogMOD(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogMOD();
    virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_MOD_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonMODRead();
    CComboBox m_CB_MOD_H;
    BOOL m_bCheckMOD_H;
    afx_msg void OnBnClickedButtonMODCreate();
    afx_msg void OnBnClickedButtonMODRun();
    BOOL m_bCheckMSH_H;
    BOOL m_bCheckPCS_H;
    BOOL m_bCheckOBJ_H;
    BOOL m_bCheckEQS_H;
    BOOL m_bCheckNOD_H;
    BOOL m_bCheckELE_H;
    CString m_strPCSTypeName;
    afx_msg void OnBnClickedButtonMSHEditor();
private:
  friend class ::CRFProcess;
public:
    afx_msg void OnBnClickedButtonMODDelete();
    CListBox m_LB_PCS;
    CComboBox m_CB_MOD_M;
    CComboBox m_CB_MOD_T;
    CComboBox m_CB_MOD_C;
    afx_msg void OnLbnSelchangeListPCS();
    void Check();
    BOOL m_bCheckPCS;
    afx_msg void OnBnClickedCheckPCS();
    afx_msg void OnBnClickedButtonMODReadM();
    BOOL m_bCheckMOD_M;
    bool m_bUseExistingMSH;
    afx_msg void OnBnClickedButtonMODReadT();
    BOOL m_bCheckMOD_T;
};
