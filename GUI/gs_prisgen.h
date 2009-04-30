#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_PRISGEN_H__3FF566D0_36CB_4DE8_AF57_DB22E737D723__INCLUDED_)
#define AFX_PRISGEN_H__3FF566D0_36CB_4DE8_AF57_DB22E737D723__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fem_ele.h"

class CPRISGEN : public CDialog
{
public:
	CPRISGEN(CWnd* pParent = NULL);   // Standardkonstruktor
// Dialogfelddaten
	enum { IDD = IDD_PRISGEN};
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPRISGEN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL
// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CPRISGEN)
    afx_msg void OnButtonCreatePrisms();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    BOOL OnInitDialog();
    afx_msg void OnLbnSelchangeListSurfaces();
    afx_msg void OnBnClickedButtonDelete();
    afx_msg void OnBnClickedButtonCreatePrismElements();
	virtual void OnCancel();
private:
    CListBox m_LBSurfaces;
    CString m_strSurface;
    CString m_strLayer;
    CString m_strMSHName;
    CFEMesh* m_msh;
    CComboBox m_CB_MSH;
public:
	int	m_numberofprismlayers;
	double	m_thicknessofprismlayer;
    CString m_strMSHType;
    afx_msg void OnCbnSelchangeComboMSH();
    int m_iMATGroup;
    afx_msg void OnBnClickedButtonAppendElements();
    CComboBox m_CB_Layer;
    int m_iSubDivision;
    long m_lNoElements;
    afx_msg void OnBnClickedButtonPrismRefine();
    afx_msg void OnCbnSelchangeComboLayer();
    int m_iLayer;
};

class CPrisGenMap : public CDialog
{
public:
	CPrisGenMap(CWnd* pParent = NULL);   // Standardkonstruktor
 
// Dialogfelddaten
	enum { IDD = IDD_MAPPING};
	int	m_iNumberOfLayers;
	int	m_iRowNumber;

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPRISGEN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
   	afx_msg void OnMapRow();	
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonMapRow();
    afx_msg void OnBnClickedButtonAddFile();
private:
    CListBox m_LBFileNames;
public:
    afx_msg void OnBnClickedButtonExecute();
private:
    CString m_file_number;
	UINT    m_uiRangeFrom;
	UINT    m_uiRangeTo;
public:
    CProgressCtrl m_progess_mapping;
    CString m_strFileNameBase;
    afx_msg void OnBnClickedButtonClearList();
    CComboBox m_CB_MSH;
    afx_msg void OnCbnSelchangeComboMSH();
    CString m_strMSHName;
    CFEMesh* m_msh;
    CComboBox m_CB_LAY;
    afx_msg void OnCbnSelchangeComboLayer();
    // WW
    CButton integ_flag;
    };


class CPrisGenVertDisc : public CDialog
{
public:
	CPrisGenVertDisc(CWnd* pParent = NULL);   // Standardkonstruktor
 
// Dialogfelddaten
	enum { IDD = IDD_VerticalDiscretization};
	int	m_iNumberOfLayers;
	int m_iLayerToBeDiscretized;
    int m_iNumberOfSubLayers;

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPRISGEN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
   	afx_msg void OnMapRow();	

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonRefineLayer();
};

// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.
#endif // AFX_PRISGEN_H__3FF566D0_36CB_4DE8_AF57_DB22E737D723__INCLUDED_
#pragma once
