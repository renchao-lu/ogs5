#pragma once
#include "afxwin.h"
#include <vector>

//#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF")
#import "C:\Programme\Gemeinsame Dateien\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF")
#include "afxcmn.h"


// CDialogGHDB_Connect dialog

class CDialogGHDB_Connect : public CDialog
{
	DECLARE_DYNAMIC(CDialogGHDB_Connect)

public:
	CDialogGHDB_Connect(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogGHDB_Connect();

// Dialog Data
	enum { IDD = IDD_GHDB_Connect };

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP();
	_ConnectionPtr	m_pConnection;
	_RecordsetPtr  pSet;
	int rec;  
    afx_msg void OnClose();

	afx_msg void OnBnClickedFileGhdb();
	CString filename;
	HRESULT hr; 

	afx_msg void OnEnChangeGhdbFile();
	CEdit edit_file_name;
	CListBox TableList;
	afx_msg void OnBnClickedGhdbConnect();
	afx_msg void OnBnClickedImporting();
	
	CListBox FieldListBox;
	afx_msg void OnBnClickedButtonAdd();
	CListBox PropertiesListBox;
	CListBox ProListBox;
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnLvnItemchangedListProperties(NMHDR *pNMHDR, LRESULT *pResult);
   
    virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonRead();
	afx_msg void OnBnClickedCancel();
	vector<CString> itemhead;
	CListCtrl pro_table;
	CStatic	m_Statistics;	

	int ncolumns;
	CButton GetValue;
	afx_msg void OnBnClickedGV();
	afx_msg void OnLbnSelchangeList2();

	CString tablename;
	CComboBox Sta_ID;
	
	CButton Search;
	afx_msg void OnBnClickedButton3();
		
	
	CListBox ResultList;
	afx_msg void OnCbnSelchangeCombo1();
	
	CString FID;

	CString Date1;
	CString Date2;
	afx_msg void OnEnChangeEdit1();
	CEdit From;
	afx_msg void OnEnChangeEdit2();
	CEdit End;
	afx_msg void OnBnClickedOk();
};

