/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/

#pragma once
#include "afxwin.h"
#include "ogrsf_frmts.h"//CC2008
#include "geos_c.h"
#include "listctrl.h"

//#include "geos_c.h"

// CDialogDatabase dialog

class CDialogDatabase : public CDialog
{
	DECLARE_DYNAMIC(CDialogDatabase)

public:
	CDialogDatabase(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogDatabase();

// Dialog Data
	enum { IDD = IDD_DATABASE };
    CMyListCtrl m_listctrlattdata;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CString m_db_Filename;
	CString m_strDBPathBase;
	CString m_strGSPFilePathBase;
    CString m_strGSPFileBase;
	OGRDataSource  *poDS;
	OGRLayer  *poLayer;
	int index;
    CString featurename;
	CString m_strGHDBPolyline;
	CString m_strGHDBPoint;
	vector<CString> mat_name;
public:
	afx_msg void OnBnClickedOpenDb();
public:
	CListBox m_TableList;
public:
	afx_msg void OnBnClickedButtonImport();
public:
	CListBox m_AttributeList;
public:
	afx_msg void OnLbnSelchangeList();
public:
	CComboBox m_db_pnt;
public:
	CComboBox m_db_ply;
public:
	CComboBox m_db_sfc;
public:
	afx_msg void OnBnClickedButtonAttribute();
public:
	//CListBox m_attribute_value;
public:
	afx_msg void OnBnClickedButtonEle();

public:
	afx_msg void OnBnClickedOk();


public:
	afx_msg void OnBnClickedButtonExpdb();
public:
	CString m_newtbname_db;
public:
	afx_msg void OnBnClickedButtonMat();
};
