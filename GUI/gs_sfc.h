#pragma once

#include "geo_sfc.h"
#include "afxwin.h"
#include "afxcmn.h"
// CGSSurface dialog

class CGSSurface : public CDialog
{
	DECLARE_DYNAMIC(CGSSurface)

public:
	CGSSurface(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSSurface();
// Dialog Data
	enum { IDD = IDD_SURFACE };
	CListBox	m_LBSFCPolylines;
	CListBox	m_LBSurfaces;
	CListBox	m_LBPolylines;
	CString	    m_strNameSurface;
	CString	    m_strPolylineSelected;
  
	double m_dBufferZone;
    CString     m_strFileNameBase;
	CListBox	m_LBSurfaces2;
    CString     m_strMapFile;
    Surface *m_surface;
    CString m_strFileNamePath;
	void OnUpdatePolylineListPicked();
	void OnLbnSelOnlyThePickedSurfaces();	// Added by PCH
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnSurfaceAdd();
    afx_msg void OnBnPolyline2Surface();
    afx_msg void OnLbnSelchangeListSurfaces();
    afx_msg void OnLbnSelchangeListPolylines();
    afx_msg void OnBnSurfaceRemove();
	
	afx_msg void OnBnClickedButtonSurfaceUpdate();
	afx_msg void OnBnClickedButtonSurfaceRemoveall();
    afx_msg void OnBnClickedButtonSurfaceCreateTIN();
    afx_msg void OnBnClickedButtonSurfaceMap();
private:
    CListBox m_LBSurfaces2Map;
public:
    afx_msg void OnLbnSelchangeListSurfaces2();
    afx_msg void OnBnClickedButtonFile();
    afx_msg void OnBnClickedButtonExecute();
    afx_msg void OnBnClickedButtonCreate();
    CProgressCtrl m_progress_TIN;
    afx_msg void OnBnClickedButtonSurfaceWriteTEC();
	afx_msg void OnBnClickedButtonCreateLayerTINs();
    CProgressCtrl m_progress_TINS;
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnEnChangeEditSurfaceName();
	afx_msg void OnLbnSelchangeListSurfacePolylines();
    afx_msg void OnBnClickedUnselect(); //CC
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual void OnOK(); //OK
	virtual void OnCancel(); //OK
    CListCtrl m_LC_SFC; //OK
    void UpdateList(); //OK
    CComboBox m_CB_MSH;
    CString m_strMSHName; //OK
};
