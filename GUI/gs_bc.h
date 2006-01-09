#ifndef AFX_SH_BC_H__7C38E993_DDF7_11D2_9133_004005311D24__INCLUDED_
#define AFX_SH_BC_H__7C38E993_DDF7_11D2_9133_004005311D24__INCLUDED_

// sh_bc.h : Header-Datei
//

#include "rf_bc_new.h"
#include "grid/virtualgridctrl.h"
#include "afxcmn.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBoundaryConditions 
class CNodeDataTable {
public:
	CNodeDataTable(LPCTSTR lpPCSTypeName = _T(""), \
                   LPCTSTR lpGEOTypeName = _T(""), LPCTSTR lpGEOName = _T(""), \
                   LPCTSTR lpDISTypeName = _T(""), double lpDISValue = _T(0.0), \
                   LPCTSTR lpTIMName = _T("") )
	{
	    m_strPCSTypeName = lpPCSTypeName;
	    m_strGEOTypeName = lpGEOTypeName;
	    m_strGEOName = lpGEOName;
	    m_strDISTypeName = lpDISTypeName;
	    m_strTIMName = lpTIMName;
        m_dDISValue = lpDISValue;
        char value[20];
        sprintf(value,"%g",lpDISValue);
        m_strDISValue = value;
		m_crBkColor = ::GetSysColor(COLOR_WINDOW);
		m_crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

		m_bAcceptCursor = TRUE;
		m_bReadOnly = FALSE;
		m_bChecked = FALSE;
	}
	int index;
	CString m_strPCSTypeName;
	CString m_strGEOTypeName;
	CString m_strGEOName;
	CString m_strDISTypeName;
	CString m_strDISValue;
	CString m_strTIMName;
	COLORREF m_crBkColor;
	COLORREF m_crTextColor;
	BOOL m_bAcceptCursor;
	BOOL m_bReadOnly;
	BOOL m_bChecked;
    double m_dDISValue;
};


/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBoundaryConditions 
class CBoundaryConditions : public CDialog
{
// Konstruktion
public:
	CBoundaryConditions(CWnd* pParent = NULL);   // standard constructor
// Dialogfelddaten
	//{{AFX_DATA(CBoundaryConditions)
	enum { IDD = IDD_BOUNDARY_CONDITIONS1 };
	//}}AFX_DATA

// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CBoundaryConditions)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CBoundaryConditions)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    CString m_strThis;
    CString m_strPCSTypeName;
    CString m_strPCSName;
    CString m_strPVName;
    CString m_strGEOTypeName;
	CString	m_strGEOName;
	CString	m_strDISTypeName;
	CString	m_strFCTName;
    CBoundaryCondition *m_bc;
    CString LBString(CBoundaryCondition*);
    CListCtrl m_LC_BC;
    CComboBox m_CB_PCSType;
    CComboBox m_CB_TIMType;
    CComboBox m_CB_PV;
	CComboBox	m_CB_GEOType;
	CListBox	m_LB_GEO;
	CComboBox	m_CB_DISType;
	CString	    m_dValue;
public:
    afx_msg void OnCbnSelchangeComboGEOType();
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnCbnSelchangeComboDISType();
    afx_msg void OnBnClickedButtonCreateGroup();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnLbnSelchangeListFCT();
    afx_msg void OnLbnSelchangeListPCSType();
    afx_msg void OnBnClickedButtonWrite();
    afx_msg void OnBnClickedButtonWriteTEC();
    afx_msg void OnCbnSelchangeComboPVName();
    void FillTable();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SH_BC_H__7C38E993_DDF7_11D2_9133_004005311D24__INCLUDED_
