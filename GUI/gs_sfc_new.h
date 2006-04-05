/**************************************************************************
GeoSys GUI - Object: New surface dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
#pragma once

#include "geo_sfc.h"
#include "ListCtrl.h"
#include "EditCell.h"
#include "afxcmn.h"
#include "afxwin.h"

// CGSSurfacenew dialog

class CGSSurfacenew : public CDialog
{
	DECLARE_DYNAMIC(CGSSurfacenew)

public:
	CGSSurfacenew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSSurfacenew();
    CMyListCtrl m_Listctrlsurfacenew;
    CString	    m_strNameSurface;
// Dialog Data
	enum { IDD = IDD_SURFACE_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClickedSurfaceAdd();
    afx_msg void OnClickedSurfaceRemove();
    afx_msg void OnClickedSurfaceRemoveall();
    afx_msg void OnClickedUnselectSfc();
    afx_msg void OnClickedSfcRead();
    afx_msg void OnClickedSfcWrite();
    afx_msg void OnClickedSurfaceWriteTec();
    afx_msg void OnClickedCreateSurface();
    afx_msg void OnClickedPly2sfc();
    afx_msg void OnClickedCreateLayerTins();
    void OnButtonSurfacePropertiesUpdate();
    CComboBox m_CB_plys;
    CComboBox m_CB_tin_msh;
    void AddSurfacetoList(long);
    void UpdateSurfaceList();
    CListBox m_sfc_plys;
    CString m_strFileNamePath;
   
       
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};
