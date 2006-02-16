#pragma once

// CDialogVolume dialog
// GEOLib
#include "geo_sfc.h"
#include "geo_vol.h"
#include "afxwin.h"
#include "afxcmn.h"

class CDialogVolume : public CDialog
{
	DECLARE_DYNAMIC(CDialogVolume)

public:
	CDialogVolume(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogVolume();

	void OnUpdateSurfaceListPicked();
	void OnUpdateSurfaceList(CListBox* list);
// Dialog Data
	enum { IDD = IDD_VOLUME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonCreateLayerVolumes();
    Surface*    m_sfc;
    CGLVolume*  m_vol;
    CListBox	m_LBVOLSurfaces;
	CListBox	m_LBSurfaces;
	CListBox	m_LBVolumes;
    CString	    m_strNameVolume;
    CString	    m_strSurfaceSelected;
    CString m_strFileNamePath;
    afx_msg void OnLbnSelchangeListSurfaces();
    afx_msg void OnBnClickedButtonVolumesWriteTec();
    afx_msg void OnBnClickedButtonSurfaceAdd();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnLbnSelchangeListVolumes();
    afx_msg void OnEnChangeEditVolumeName();
    afx_msg void OnBnClickedButtonCreate();
    afx_msg void OnBnClickedButtonSurface2Volume();
	afx_msg void OnLbnSelchangeListSurfacePolylines();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual void OnOK(); //OK
	virtual void OnCancel(); //OK
};
#pragma once


// CDialogVolumeNew dialog

class CDialogVolumeNew : public CDialog
{
	DECLARE_DYNAMIC(CDialogVolumeNew)

public:
	CDialogVolumeNew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogVolumeNew();

// Dialog Data
	enum { IDD = IDD_VOLUME_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_CB_Layer;
    CComboBox m_CB_MSH;
    CComboBox m_CB_SFC;
    CListCtrl m_LC;
    afx_msg void OnBnClickedButtonCreate();
    CGLVolume* m_vol;
    void UpdateList();
    afx_msg void OnCbnSelchangeComboVolMsh();
    CString m_strMSHName;
    CString m_strLAYName;
    CString m_strSFCName;
    afx_msg void OnCbnSelchangeComboVolLayer();
    afx_msg void OnCbnSelchangeComboVolLayerSfc();
    BOOL m_iType;
    afx_msg void OnBnClickedButtonVolWriteTec();
    CComboBox m_CB_VOL2MAT;
    CString m_strMATName;
    afx_msg void OnCbnSelchangeComboVolMat();
    afx_msg void OnBnClickedButtonVolMat();
    afx_msg void OnBnClickedButtonVolWrite();
    afx_msg void OnLvnItemchangedListVol(NMHDR *pNMHDR, LRESULT *pResult);
};
