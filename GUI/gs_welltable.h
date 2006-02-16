//********************************************************************
//08/2004 CC implementation  show the table of wells with attributes *
//********************************************************************

#pragma once
#include "afxcmn.h"
#include "ListCtrl.h"
#include "GeoSysDoc.h"
#include "GeoSysOUT2DView.h"


// CListWellTable dialog

class CListWellTable : public CDialog
{
	DECLARE_DYNAMIC(CListWellTable)

public:
   class GeoSysDoc;
   CListWellTable(CWnd* pParent = NULL); 
   CGeoSysDoc* pDoc;
   virtual ~CListWellTable();

// Dialog Data
	enum { IDD = IDD_DIALOG_TABLE_WELLS };
    CMyListCtrl m_listwell;


  
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
  
public:
    afx_msg void OnDeleteAll();
    afx_msg void OnDelete();
    afx_msg void OnBnClickedOK();
    afx_msg void OnBnClickedExport();

};
