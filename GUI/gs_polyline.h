#if !defined(AFX_GS_POLYLINE_H__FF0AC486_FF3E_4B31_9C7E_75AA49B65EB3__INCLUDED_)
#define AFX_GS_POLYLINE_H__FF0AC486_FF3E_4B31_9C7E_75AA49B65EB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_polyline.h : header file
//

#include "geo_ply.h"
#include "afxcmn.h"
#include "ListCtrl.h"
#include "EditCell.h" // CC 06/2005

/////////////////////////////////////////////////////////////////////////////
// CPolyline dialog

class CPolyline : public CDialog
{
// Construction
public:
	CPolyline(CWnd* pParent = NULL);   // standard constructor
    //void Draw(CClientDC *dc,CGLPolyline *polyline);
    void DrawPoints(CClientDC *dc,CGLPolyline *m_polyline);
    void FillPolygon(CClientDC *dc,CGLPolyline *m_polyline);
	void OnUpdatePointListPicked();
	void OnLbnSelOnlyThePickedPolylines();
    void OnButtonPolylinePropertiesUpdate();
    void OnUpdatePolylinePointList();
// Dialog Data
	//{{AFX_DATA(CPolyline)
	enum { IDD = IDD_POLYLINE };
    double m_dBufferZone;
    double m_PlyMeshDensity;
	CListBox	m_LBPolylines;
    bool    add_polyline;
	CString	m_strNamePolyline;
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPolyline)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
    friend class CGLPolyline; //OK
protected:
	long m_nSelectedPolylineProperties;
	CString m_strPolylineObjectName;
    long i;
    CPen MATPen;
    CPen BCPen;
	// Generated message map functions
	//{{AFX_MSG(CPolyline)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonPolylineAdd();
	afx_msg void OnButtonPolylineUpdate();
	afx_msg void OnButtonPolylineRemove();
	afx_msg void OnButtonPolylineRemoveall();
	afx_msg void OnButtonPolylinePropertiesRemove();
	afx_msg void OnButtonPolylinePropertiesRemoveall();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeListPolylines();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonPolylineAddLayer();

public: // data
    CString m_strFileNameBase; //without extension
    afx_msg void OnBnClickedButtonPolylineWriteTEC();
    afx_msg void OnBnClickedWritegli();
    afx_msg void OnBnClickedDonothing();
    CMyListCtrl m_listctrlPolyline; // CC 06/2005 change list box into List control
    afx_msg void OnBnClickedButtonPolylinePropertiesAdd();
    void FillPropertiesListCtrl(CGLPolyline*); //OK
    afx_msg void OnBnClickedButtonPolylineUnselect(); //CC
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized); //CC
    afx_msg void OnBnClickedButtonUpdateMd(); //CC
    void OnReleaseAllPlyhighlight();
    void OnMarkselectedPoint();//CC
    double m_dPDMin;
    afx_msg void OnBnClickedButtonCalcPdmin();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GS_POLYLINE_H__FF0AC486_FF3E_4B31_9C7E_75AA49B65EB3__INCLUDED_)
