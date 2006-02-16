#pragma once

#include "rfsousin.h"
#include "rf_st_new.h"
#include "grid\virtualgridctrl.h"

// CGSSourceTerm dialog

class SourceList  {
public:
	SourceList(int Index = 0, LPCTSTR lpPCSName = _T(""),  
		        LPCTSTR lpGEOName = _T(""), LPCTSTR lpDISName = _T(""))
	{
	    m_strPCSName = lpPCSName;
	    m_strGEOName = lpGEOName;
	    m_strDISName = lpDISName;
		index = Index;

		m_crBkColor = ::GetSysColor(COLOR_WINDOW);
		m_crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

		m_bAcceptCursor = TRUE;
		m_bReadOnly = FALSE;
		m_bChecked = FALSE;
	}

	CString m_strPCSName;
	CString m_strGEOName;
	CString m_strDISName;

	//  index
	int index;

	//  Additional demo data
	COLORREF m_crBkColor;
	COLORREF m_crTextColor;
	BOOL m_bAcceptCursor;
	BOOL m_bReadOnly;

	//  -------  NEW  --------
	BOOL m_bChecked;
};


class CGSSourceTerm : public CDialog
{
	DECLARE_DYNAMIC(CGSSourceTerm)

public:
	CGSSourceTerm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSSourceTerm();

// Dialog Data
	enum { IDD = IDD_SOURCE_TERM };
	CString	    m_strQuantityName;
	CString	    m_strSTName;
	CListBox	m_LBQuantities;
    CString     m_strLBProperties;
	//CListBox 	m_LBProperties;
	CListBox	m_LBPoints;
	CListBox	m_LBPolylines;
	CListBox	m_LBSurfaces;
    char quantity_name[80];
    char value_char[80];
    int m_iGeoType;
    int m_iDisType;
    double m_dValue;
    CString m_strPointName;
    CString m_strPolylineName;
    CString m_strValue;
    SOURCE_SINK *st;
    CSourceTerm *m_st;
    char m_charSTName[80];
    int m_iNumber;
	CString m_strGEOTypeName;
	CString m_strDISTypeName;
	CString m_strGEONameThis;
    afx_msg void OnButtonAdd();
    afx_msg void OnSelchangeListQuantities();
    afx_msg void OnBnClickedRadioStGeoType1();
    afx_msg void OnSelchangeListPoints();
    afx_msg void OnSelchangeListProperties();
    afx_msg void OnButtonUpdate();
    afx_msg void OnBnClickedRemove();
    afx_msg void OnBnClickedRemoveAll();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
    
    CString GeoTypeName(int);
    CString DisTypeName(int);
    void SetData(SOURCE_SINK*);
    void SetPropertyString( void);
    void GetSTName(CString);
    double* DisTypeValues(int);

	//Grid
   	CArray<SourceList, SourceList> m_Data;
	CVirtualGridCtrl m_grid;
	afx_msg void OnGridGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult=NULL);
    afx_msg void OnCursorOnGridRow(NMHDR* pNMHDR, LRESULT* pResult=NULL);


	DECLARE_MESSAGE_MAP()
private:
	void OnInitGrid(); 
    int Count;
	int MarkedRow;

};
