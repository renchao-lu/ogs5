#pragma once
#include "afxwin.h"
#include "GeoSysDoc.h"
#include "rf_mmp_new.h"
// COutputObservation dialog

class COutputObservation : public CDialog
{
	DECLARE_DYNAMIC(COutputObservation)

  friend class CGeoSysView;
  friend class CGeoSysDoc;

public:
	COutputObservation(CWnd* pParent = NULL);   // standard constructor
	virtual ~COutputObservation();

// Dialog Data
	enum { IDD = IDD_OUT_OBSERVATION_POINTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
private:
    CListBox m_LBQuantities;
    CListBox m_LBPoints;
    CListBox m_LBObservationPoints;
    CString m_strNamePoint;
    void SetData(void);
    char char_string[80];
    int m_iQuantityIndex;
public:
    afx_msg void OnBnAddObservationPoint();
    CString m_strFilePath;
    CString m_strFileNameBase;
    double m_dXmin;
    double m_dXmax;
    double m_dYmin;
    double m_dYmax;
    double m_dUmin;
    double m_dUmax;
    CString m_strQuantityName;
    afx_msg void OnSelchangeListQuantities();
};
#pragma once


// COUTObservationPolylines dialog
#include "geo_ply.h"
#include "rf_out_new.h"

class COUTObservationPolylines : public CDialog
{
	DECLARE_DYNAMIC(COUTObservationPolylines)

public:
	COUTObservationPolylines(CWnd* pParent = NULL);   // standard constructor
	virtual ~COUTObservationPolylines();

// Dialog Data
	enum { IDD = IDD_OUT_OBSERVATION_POLYLINES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
    CListBox m_LBQuantities;
    CListBox m_LBPolylines;
    CListBox m_LBOUTPolylines;
    int m_iQuantityIndex;
    char c_string[80];
    CGLPolyline *m_polyline;
public:
    double m_dXmin;
    double m_dXmax;
    double m_dYmin;
    double m_dYmax;
    CString m_strQuantityName;
    CString m_strPolylineName;
public:
    afx_msg void OnSelchangeListQuantities();
    afx_msg void OnSelchangeListObservationPolylines();
    afx_msg void OnBnClickedButtonOUTPolylines();
    afx_msg void OnLbnSelchangeListPolylines();
};
#pragma once


// COUTMaterialProperties dialog

class COUTMaterialProperties : public CDialog
{
	DECLARE_DYNAMIC(COUTMaterialProperties)

public:
	COUTMaterialProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~COUTMaterialProperties();

// Dialog Data
	enum { IDD = IDD_OUT_MAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
public:
    double m_dXmin;
    double m_dXmax;
    double m_dYmin;
    double m_dYmax;
    double m_dUmin;
    double m_dUmax;
    CString m_strMATPropertyName;
    afx_msg void OnSelchangeListMatProperties();
    int m_iDisplayMATPropertiesType;
    int m_iFluidPhase;
    CComboBox m_CB_MMP;
    afx_msg void OnCbnSelchangeComboMMP();
    afx_msg void OnBnClickedButtonFCTCreate();
    afx_msg void OnBnClickedButtonFCTWrite();
    CComboBox m_CB_MMP_Group;
    afx_msg void OnCbnSelchangeComboMMPGroup();
    CString m_strMATGroupName;
    afx_msg void OnBnClickedButtonRefresh();
    CMediumProperties* m_mmp;
    afx_msg void OnEnChangeEditUmin();
    afx_msg void OnEnChangeEditUmax();
};
