

#pragma once
#include "ViewPropertyPage.h"
#include "resource.h"
#include "afxcmn.h"
#include "GeoSysDoc.h"
#include "afxwin.h"

//--EXCEL---
#include "stdafx.h"
#include "gs_mat_mp.h"//CApplication, CWorkbooks, etc. ....
//----------

// CGSPropertyRightTemplate dialog

class CGSPropertyRightImportData : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightImportData)

private:

    CString m_fileopen;//path to EXCEL-file


public:

    CApplication oExcel;
    CWorkbooks oBooks;    
    CWorkbook oBook;

	CGSPropertyRightImportData();
	virtual ~CGSPropertyRightImportData();

    //--EXCEL-Dateiauswahl---
    CEdit m_filename_edit;
    afx_msg void OnFileopen();

    CString m_strstation;
    CString m_strStationName;
    int m_iSiteSelected;//Selected item of Combobox


    void Excel_stationnames2Combo(void);
    void GetTypenamefromComboBox(void);

    //--funktion für standard dialogaufruf--     
    void fileopendialog(void);

    CEdit m_dipl_stationname;
    CString m_actual_stationname;


// Dialog Data
	enum { IDD = IDD_CONTROL_RIGHT_IMPORT_DATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	

    DECLARE_MESSAGE_MAP()

public:

    CListCtrl m_stationdata_list;
    CComboBox m_combo_station;
    afx_msg void OnBnClickedButtonStatdata();
    afx_msg void OnCbnSelchangeComboStation();
};
