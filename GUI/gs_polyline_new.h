/**************************************************************************
GeoSys GUI - Object: New polyline dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
#pragma once

#include "geo_ply.h"
#include "ListCtrl.h"
#include "EditCell.h"
#include "afxcmn.h"
#include "afxwin.h"
// CGSPolylinenew dialog

class CGSPolylinenew : public CDialog
{
	DECLARE_DYNAMIC(CGSPolylinenew)

public:
	CGSPolylinenew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSPolylinenew();

// Dialog Data
	enum { IDD = IDD_POLYLINE_NEW };
    CMyListCtrl m_Listctrlpolylinenew;
   	CString	m_strNamePolyline;
    CString m_strPolylineObjectName;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
   
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClickedPolylineAdd();
    afx_msg void OnClickedPolylineRemove();
    afx_msg void OnClickedPolylineRemoveall();
    afx_msg void OnClickedPolylineUnselect();
    afx_msg void OnClickedButtonPolylineRead();
    afx_msg void OnClickedButtonPolylineWrite();
    afx_msg void OnClickedButtonPolylineWritetec();
    afx_msg void OnClickedPolylineAddLayer();
    CComboBox m_ply_mshcombo;
    bool add_polyline;
    void UpdatePolylineList();
    void AddPolylinetoList(long);
    void OnButtonPolylinePropertiesUpdate();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};
