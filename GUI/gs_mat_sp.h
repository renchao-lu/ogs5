#pragma once

// MAT_Mech_dlg.h : header file
//
#include "afxcmn.h"
#include "grid/virtualgridctrl.h"
#include "afxwin.h"


/*
    Dialog for mechanical material properties.
	Designed and programmed: WW
	Last modification: WW  01-2004
*/
namespace SolidProp{ class CSolidProperties; };
using SolidProp::CSolidProperties;
class Mat_Mech_Grid  {
public:
	Mat_Mech_Grid(LPCTSTR lpParaName = _T(""), double paraVal = 0, 
		        LPCTSTR lpUnitName = _T(""))
	{
		m_strParaName = lpParaName;
		m_strUnitName = lpUnitName;
		ParaVal = paraVal;

		m_crBkColor = ::GetSysColor(COLOR_WINDOW);
		m_crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

		m_bAcceptCursor = TRUE;
		m_bReadOnly = FALSE;
		m_bChecked = FALSE;
	}

	CString m_strParaName;
	CString m_strUnitName;

	//  Value
	double ParaVal;

	//  Additional demo data
	COLORREF m_crBkColor;
	COLORREF m_crTextColor;
	BOOL m_bAcceptCursor;
	BOOL m_bReadOnly;

	//  -------  NEW  --------
	BOOL m_bChecked;
};


/////////////////////////////////////////////////////////////////////////////
// MAT_Mech_dlg dialog

class MAT_Mech_dlg : public CDialog
{
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MAT_Mech_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGridGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnGridSetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheckThermal();
	afx_msg void OnCheckElasticity();
	afx_msg void OnSelchangeComboPlasticity();
	afx_msg void OnCheckPlasticity();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    int NumRows;
	// Three items: Thermal, Elasticity and Plasticity
	bool Type[4];
	int  ParaNum[4];
	////
    CSolidProperties *m_msp;    
	int PlastModel; 
	int creep_model; 

	double Thermal[2];
	double Elast[3];
	double *plastic_data;
	double *creep_data;
    
	void AddThermal(); 
    void AddElasticity(); 
    void AddPL_DP();
    void AddPL_CM();
    void AddPL_RH();
    void AddCRP_Norton();
    void AddCRP_BGRa();
    void Refresh(); 

// Dialog Data
	//{{AFX_DATA(MAT_Mech_dlg)
	enum { IDD = IDD_MAT_SP };
	CComboBox	Combo_plastity;
	// // Select material group
	CComboBox mat_group;
	// For creep data
	CComboBox combox_creep;
	//}}AFX_DATA
public:
	MAT_Mech_dlg(CWnd* pParent = NULL);   // standard constructor
	~MAT_Mech_dlg();  


	CVirtualGridCtrl m_grid;
	CArray<Mat_Mech_Grid, Mat_Mech_Grid> m_Data;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MAT_Mech_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	afx_msg void OnCbnSelchangeComboMatGroup();
    //
	afx_msg void OnBnClickedMspUpdate();
	afx_msg void OnBnClickedMspNew();
	afx_msg void OnCbnSelchangeComcreep();
	afx_msg void OnBnClickedCreep();
public:
	afx_msg void OnCbnSelchangeCreep1();
};


