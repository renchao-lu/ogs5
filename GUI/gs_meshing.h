#pragma once
#include "afxwin.h"

// C++ STL
#include <vector>
#include "afxcmn.h"
using namespace std;

#include "fem_ele.h"

////////////////////////////////////////////////////////////////////////////////
// CMSHRefine dialog

class CMSHRefine : public CDialog
{
	DECLARE_DYNAMIC(CMSHRefine)

public:
	CMSHRefine(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMSHRefine();

// Dialog Data
	enum { IDD = IDD_MSH_REFINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
#pragma once


////////////////////////////////////////////////////////////////////////////////
// CMSHPolylines dialog

class CMSHPolylines : public CDialog
{
	DECLARE_DYNAMIC(CMSHPolylines)

public:
	CMSHPolylines(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMSHPolylines();
// Dialog Data
	enum { IDD = IDD_MSH_POLYLINES };
public:
    int type;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
    CListBox m_LBPolylinesGEO;
    CListBox m_LBPolylinesMSH;
    CString m_strNamePolyline;
	int m_iMeshingType;
    CFEMesh* m_msh;
    CGLPolyline* m_ply;
public:
    afx_msg void OnButtonGEO2MSH();
    afx_msg void OnSelchangeListPolylinesGeo();
    afx_msg void OnButtonMSHExecute();
    afx_msg void OnLbnSelchangeListPolylinesMSH();
    CProgressCtrl m_progress_msh; // CC 04/2005
    afx_msg void OnBnClickedDeleteDouble();
    afx_msg void OnCbnSelchangeComboMSH();
    CComboBox m_CB_MSH;
    int m_iNr;
    afx_msg void OnBnClickedButtonMSHSubdivisionExecute();
};
#pragma once

////////////////////////////////////////////////////////////////////////////////
// CMSHEditor dialog

class CMSHEditor : public CDialog
{
	DECLARE_DYNAMIC(CMSHEditor)

public:
	CMSHEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMSHEditor();

// Dialog Data
	enum { IDD = IDD_MSH_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    BOOL m_bMSHActivateLine;
    BOOL m_bMSHActivateTri;
    BOOL m_bMSHActivateQuad;
    BOOL m_bMSHActivateTet;
    BOOL m_bMSHActivatePris;
    BOOL m_bMSHActivateHex;
    long m_lNumberOfLine;
    long m_lNumberOfTris;
    long m_lNumberOfQuad;
    long m_lNumberOfTets;
    long m_lNumberOfPris;
    long m_lNumberOfHexs;
    long m_lNumberOfElements;
    long m_lNumberOfNodes;
    afx_msg void OnBnClickedButtonMSHActivate();
    afx_msg void OnBnClickedButtonMATConnectAll();
    afx_msg void OnBnClickedButtonMATWriteTecplot();
    afx_msg void OnBnClickedButtonMATConnectLine();
    afx_msg void OnBnClickedButtonMATConnectTris();
    afx_msg void OnBnClickedButtonMATConnectQuad();
    afx_msg void OnBnClickedButtonMATConnectTets();
    afx_msg void OnBnClickedButtonMATConnectPris();
    afx_msg void OnBnClickedButtonMATConnectHexs();
    CListBox m_LBMATGroups;
    afx_msg void OnLbnSelchangeListMATGroups();
    CComboBox m_CB_MSH;
    CComboBox m_CB_GEO;
    afx_msg void OnCbnSelchangeComboGEO();
    CListBox m_LB_GEO;
    afx_msg void OnBnClickedButtonGEONodes();
    CString m_strGEOName;
    CString m_strMSHName;
    int m_iGeoType;
    afx_msg void OnLbnSelchangeListGEO();
    afx_msg void OnCbnSelchangeComboMSH();
    long m_lNoMSHNodes;
    CString m_strMSHNameNew;
    CFEMesh* m_msh;
    afx_msg void OnBnClickedButtonMSHName();
    afx_msg void OnBnClickedButtonMSHWrite();
    afx_msg void OnBnClickedButtonMSHDelete();
    CComboBox m_CB_MSH_NEW;
    afx_msg void OnCbnSelchangeComboMSHNew();
    afx_msg void OnBnClickedButtonMatConnectThis();
    int m_iMMPGroup;
    afx_msg void OnBnClickedButtonMatReset();
    afx_msg void OnBnClickedButtonMatWriteTecplot2();
    afx_msg void OnBnClickedButtonMatWriteTec();
    CString m_strGEOTypeName; //OK
    afx_msg void OnBnClickedButtonMatConnectLayer();
    CComboBox m_CB_GEO_LAYER;
    afx_msg void OnCbnSelchangeComboGeoLayer();
    CString m_strLAYName;
    afx_msg void OnBnClickedButtonLayerPolylines();
    afx_msg void OnBnClickedButtonLayerSurfaces();
    afx_msg void OnBnClickedButtonMatConnectFailed();
    long m_lNoMAT;
    double m_dMinEdgeLength;
    afx_msg void OnBnClickedButtonEdgeLength();
};
#pragma once


////////////////////////////////////////////////////////////////////////////////
// CMSHSurfaces dialog

class CMSHSurfaces : public CDialog
{
	DECLARE_DYNAMIC(CMSHSurfaces)

public:
	CMSHSurfaces(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMSHSurfaces();

// Dialog Data
	enum { IDD = IDD_MSH_SURFACES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CListBox m_LB_SFC;
    CComboBox m_CB_MSH;
    afx_msg void OnBnClickedButtonGEO2MSH();
    CListBox m_LB_GEO2MSH;
    CString m_strNameSFC;
    afx_msg void OnLbnSelchangeListSFC();
    BOOL m_iMSHType;
    afx_msg void OnBnClickedButtonMSHExecute();
    int m_iNr;
    int m_iNs;
    afx_msg void OnEnChangeEditMshDensityI();
    afx_msg void OnEnChangeEditMshDensityJ();
};
