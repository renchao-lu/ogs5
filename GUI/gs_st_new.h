#pragma once

#include "rf_st_new.h"
#include "grid\virtualgridctrl.h"
#include "gs_bc.h"

// CSourceTerms dialog
class CSourceTerms : public CDialog
{
	DECLARE_DYNAMIC(CSourceTerms)

public:
	CSourceTerms(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSourceTerms();

// Dialog Data
	enum { IDD = IDD_SOURCE_TERMS };
	CListBox	m_LB_PCSType;
	CComboBox	m_CB_GEOType;
	CListBox	m_LB_GEO;
	CComboBox	m_CB_DISType;
	CString	    m_dValue;
	CListBox	m_LB_TIM;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
    afx_msg void OnLbnSelchangeListPCSType();
    afx_msg void OnCbnSelchangeComboPCSNumber();
    afx_msg void OnCbnSelchangeComboGEOType();
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnCbnSelchangeComboDISType();
    afx_msg void OnBnClickedButtonCreateGroup();
    afx_msg void OnBnClickedButtonWrite();
	DECLARE_MESSAGE_MAP()

private:
    CSourceTerm *m_st;
    CString m_strPCSType;
    CString m_strPCSName;
    CString m_strGEOType;
	CString	m_strGEOName;
	CString	m_strDISType;
    CString m_strLB;

    //---------------------------------------------------------------------
	//Grid
private:
	CVirtualGridCtrl m_table;
    CGridColumn *pColumn;
    CGridHeaderSections *pSections;
    CGridHeaderSection *pUpperSection;
	CArray<CNodeDataTable,CNodeDataTable>m_table_data;
    afx_msg void OnGridRowRemove();
	afx_msg void OnGridGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnGridSetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCursorOnGridRow(NMHDR* pNMHDR, LRESULT* pResult);
    int m_iMarkedRow;
	void ConfigNodeDataTable(); 
    void FillNodeDataTable();

public:
    CString m_strFileNameBase;
};
