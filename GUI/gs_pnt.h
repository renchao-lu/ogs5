#if !defined(AFX_DPOINT_H__B923099F_F6E3_453E_A7C7_61DF939CCDCB__INCLUDED_)
#define AFX_DPOINT_H__B923099F_F6E3_453E_A7C7_61DF939CCDCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_pnt.h : header file
//

#include "geo_pnt.h"
#include "ListCtrl.h"
#include "EditCell.h"
/////////////////////////////////////////////////////////////////////////////
// CGSPoint dialog

class CGSPoint : public CDialog
{
// Construction
public:
	//void DrawPoint(CDC *dc,CGLPoint *m_point);
	CGSPoint(CWnd* pParent = NULL);   // standard constructor
    

// Dialog Data
	//{{AFX_DATA(CGSPoint)
	enum { IDD = IDD_POINT };
    CListBox	m_ListPoint;
	CString	m_Pointname;
	bool add_point;
	//}}AFX_DATA
    CMyListCtrl m_Listctrlpoint;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSPoint)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGSPoint)
	afx_msg void OnButtonPointRemove();
	afx_msg void OnButtonPointRemoveall();

	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListPoints();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnButtonPointPropertiesUpdate();
	afx_msg void OnBnClickedButtonPointUpdateAll();
    afx_msg void OnBnClickedButtonPointRemoveDoublePoints();
    afx_msg void OnBnClickedOk();
    afx_msg void OnUpdatePointListbox();//CC 06/2005
   
    afx_msg void OnBnClickedButtonPointNoHighlight(); //CC
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized); //CC
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DPOINT_H__B923099F_F6E3_453E_A7C7_61DF939CCDCB__INCLUDED_)