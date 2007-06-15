// GSPropertyRightImportData.cpp : implementation file
// 12/2006 JG Implementation
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "GSPropertyRightImportData.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_elements_rfi.h"
#include "msh_quality.h"
#include "dtmesh.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"

#include "afxdisp.h"
#include "afxdlgs.h"
#include ".\gspropertyrightimportdata.h"


// CGSPropertyRightTemplate dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightImportData, CViewPropertyPage)
CGSPropertyRightImportData::CGSPropertyRightImportData()
	: CViewPropertyPage(CGSPropertyRightImportData::IDD)
{
}

CGSPropertyRightImportData::~CGSPropertyRightImportData()
{
}

void CGSPropertyRightImportData::DoDataExchange(CDataExchange* pDX)
{
    CViewPropertyPage::DoDataExchange(pDX);
    //---Open File---   
    DDX_Text(pDX, IDC_FILENAME_EDIT, m_fileopen);
    DDX_Control(pDX, IDC_FILENAME_EDIT, m_filename_edit);

    //---Station-Combobox---
    DDX_Control(pDX, IDC_COMBO_STATION, m_combo_station);
    DDX_Text(pDX, IDC_COMBO_STATION, m_strstation);

    DDX_Control(pDX, IDC_LIST_STATIONDATA, m_stationdata_list);

    //---Anzeige des Stationsnamens der angezeigten Daten---
    DDX_Control(pDX, IDC_EDIT_STATIONNAME, m_dipl_stationname);
    DDX_Text (pDX, IDC_EDIT_STATIONNAME, m_actual_stationname);
}


BEGIN_MESSAGE_MAP(CGSPropertyRightImportData, CViewPropertyPage)

    ON_BN_CLICKED(IDC_BUTTON_FILEOPEN, OnFileopen)
    ON_BN_CLICKED(IDC_BUTTON_STATDATA, OnBnClickedButtonStatdata)
    ON_CBN_SELCHANGE(IDC_COMBO_STATION, OnCbnSelchangeComboStation)

END_MESSAGE_MAP()


// CGSPropertyRightTemplate message handlers

BOOL CGSPropertyRightImportData::OnInitDialog()
{

    CDialog::OnInitDialog();

    // ListControl anlegen für Solution Components in OnInitDialog
    CRect rect;
    m_stationdata_list.GetClientRect(&rect);
	m_stationdata_list.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

    //m_stationdata_list.SetExtendedStyle (LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
        int nColInterval = rect.Width()/4;
    // create columns
        m_stationdata_list.InsertColumn(0,_T("ChemComp"),LVCFMT_LEFT, nColInterval);
        m_stationdata_list.InsertColumn(1,_T("max"),LVCFMT_LEFT, nColInterval);
        m_stationdata_list.InsertColumn(2,_T("mean"),LVCFMT_LEFT, nColInterval);
        m_stationdata_list.InsertColumn(3,_T("min"),LVCFMT_LEFT, nColInterval);

	    m_stationdata_list.DeleteAllItems(); // Delete the current contents

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CGSPropertyRightImportData::OnFileopen()
{
    
    if(m_filename_edit.GetModify() == TRUE){
      UpdateData(TRUE);
      if(m_fileopen != "" && m_fileopen.Find(".xls")== -1){
        AfxMessageBox("No Excel File!");
        //return;
            fileopendialog();
      }
      if(m_fileopen == ""){
            fileopendialog();
      }
    }    
    else{
            fileopendialog();
    }
    
    //--Füllen der ComboBox--
    Excel_stationnames2Combo();
    m_combo_station.SetCurSel(0);

 
    GetDlgItem(IDC_COMBO_STATION) -> EnableWindow(TRUE);//--Station-Combo aktivieren
    GetDlgItem(IDC_TEXT_STATNAME) -> EnableWindow(TRUE);//--Überschrift Station-Combo aktivieren

    m_stationdata_list.DeleteAllItems(); // Delete the current contents

    UpdateData(FALSE);//--Daten an Anzeige geben--

}

void CGSPropertyRightImportData::Excel_stationnames2Combo()
{
    m_combo_station.ResetContent();


    CWorksheets oSheets;
	CWorksheet oSheet;
	CRange oRange, oRangeCols;
	COleVariant covOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);



	if (oExcel.m_lpDispatch == NULL) {
      oExcel.CreateDispatch("Excel.Application");
      if (oExcel.m_lpDispatch == NULL){
        AfxMessageBox("Excel has not started!");
      }
	}
	//Open workbook
    oBooks = oExcel.get_Workbooks();
    oBook = oBooks.Open (m_fileopen, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional);
    //Open first sheet
    oSheets = oBook.get_Worksheets();

    long zr;
    zr = oSheets.get_Count();
    CString stationstr;

    oSheet = oSheets.get_Item(COleVariant((short)1));//erstes worksheet
    stationstr = oSheet.get_Name();

    //------stationname sheets------
    if(stationstr != "min" && stationstr != "mean" && stationstr != "max"){
        for(int i=1; i<=zr; i++){
        oSheet = oSheets.get_Item(COleVariant((short)i));//worksheet der Reihe nach
        stationstr = oSheet.get_Name();
        m_combo_station.AddString(stationstr);
        }
    }

    //------max, mean, min sheets------
    if(stationstr == "min" || stationstr == "mean" || stationstr == "max"){

    oSheet = oSheets.get_Item(COleVariant((short)1));//first worksheet!!
    //get all data from sheet
    oRange = oSheet.get_UsedRange();

    COleSafeArray saTN(oRange.get_Value());

    long TNRows;
    long TNCols;
    saTN.GetUBound(1, &TNRows);//number of rows
    saTN.GetUBound(2, &TNCols);//number of columns
    long index[2];//1 based index for retrieving data from saTN
    //stationnames into combobox
    CString tnstr;
    int colCounter = 1;
	    for (int rowCounter = 2; rowCounter <= TNRows; rowCounter++) {
	        index[0]=rowCounter;
	        index[1]=colCounter;
	        COleVariant vData;
	        saTN.GetElement(index,vData);

            if(vData.vt == VT_BSTR){ 
                tnstr = vData.bstrVal;
		        m_combo_station.AddString(tnstr);
            } 
            /*else{ 
            AfxMessageBox("Invalid Typename found!");
            return;
            }*/
	    }
    }

    oExcel.Quit();
      oExcel.ReleaseDispatch();

    /*/--data displayed imediately---
    UpdateData(TRUE);//get Typename from Combobox
    m_iSiteSelected = 1;//erstes feature in der Combobox

    OnBnClickedButtonStatdata();*/


}
void CGSPropertyRightImportData::OnBnClickedButtonStatdata()
{
    m_stationdata_list.DeleteAllItems(); // Delete the current contents

    GetTypenamefromComboBox();
    
    CString stationstr;
    CWorksheets oSheets;
	CWorksheet oSheet;
	CRange oRange, oRangeCols;
	COleVariant covOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);


	if (oExcel.m_lpDispatch == NULL) {
      oExcel.CreateDispatch("Excel.Application");
      if (oExcel.m_lpDispatch == NULL){
        AfxMessageBox("Excel has not started!");
      }
	}
	//Open workbook
    oBooks = oExcel.get_Workbooks();
    oBook = oBooks.Open (m_fileopen, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional);
    //Open first sheet
    oSheets = oBook.get_Worksheets();

    oSheet = oSheets.get_Item(COleVariant((short)1));//erstes worksheet
    stationstr = oSheet.get_Name();

    //------------------------------    
    //------stationname sheets------
    if(stationstr != "min" && stationstr != "mean" && stationstr != "max"){


    oSheet = oSheets.get_Item(COleVariant(m_strStationName));//selected station worksheet
    //get all data from sheet
    oRange = oSheet.get_UsedRange();

    COleSafeArray saTN(oRange.get_Value());

    long TNRows;
    long TNCols;
    saTN.GetUBound(1, &TNRows);//number of rows
    saTN.GetUBound(2, &TNCols);//number of columns
    long index[2];//1 based index for retrieving data from saTN
    
        //Chemcomp, min, mean, max into ListControl

        CString strItem;
        LVITEM lvi;

    for (int colCounter = 1; colCounter <= TNCols; colCounter++){
	    for (int rowCounter=2, /*int*//*HS 15.06.2007*/ i=0; rowCounter <= TNRows; rowCounter++, i++) {
	        index[0]=rowCounter;
	        index[1]=colCounter;
	        COleVariant vData;
	        saTN.GetElement(index,vData);

          if(colCounter ==1){//ChemComp
            if(vData.vt == VT_BSTR){
                strItem = vData.bstrVal;

                lvi.mask =  LVIF_TEXT;
                // 1 column
	            //strItem.Format("%i",i);
	            lvi.iItem = i;
	            lvi.iSubItem = 0;
	            lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
                m_stationdata_list.InsertItem(&lvi);
            }
            if (vData.vt==VT_R8){
                double szdata=vData.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i;
		        lvi.iSubItem =0;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
          }

          if(colCounter ==2){//max
            if(vData.vt == VT_BSTR){
                strItem = vData.bstrVal;

                lvi.mask =  LVIF_TEXT;
                // 1 column
	            //strItem.Format("%i",i);
	            lvi.iItem = i;
	            lvi.iSubItem = 1;
	            lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
                m_stationdata_list.InsertItem(&lvi);
            }
            if (vData.vt==VT_R8){
                double szdata=vData.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i;
		        lvi.iSubItem =1;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
          }

          if(colCounter ==3){//mean
            if(vData.vt == VT_BSTR){
                strItem = vData.bstrVal;

                lvi.mask =  LVIF_TEXT;
                // 1 column
	            //strItem.Format("%i",i);
	            lvi.iItem = i;
	            lvi.iSubItem = 2;
	            lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
                m_stationdata_list.InsertItem(&lvi);
            }
            if (vData.vt==VT_R8){
                double szdata=vData.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i;
		        lvi.iSubItem =2;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
          }

          if(colCounter ==4){//min
            if(vData.vt == VT_BSTR){
                strItem = vData.bstrVal;

                lvi.mask =  LVIF_TEXT;
                // 1 column
	            //strItem.Format("%i",i);
	            lvi.iItem = i;
	            lvi.iSubItem = 3;
	            lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
                m_stationdata_list.InsertItem(&lvi);
            }
            if (vData.vt==VT_R8){
                double szdata=vData.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i;
		        lvi.iSubItem =3;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
          }
        }
    }
    }

    //---------------------------------
    //------max, mean, min sheets------
    if(stationstr == "min" || stationstr == "mean" || stationstr == "max"){

            //AfxMessageBox("HIER!");

        oSheet = oSheets.get_Item(COleVariant("max"));//max sheet
        oRange = oSheet.get_UsedRange();
        COleSafeArray saTNmax(oRange.get_Value());

        oSheet = oSheets.get_Item(COleVariant("mean"));//mean sheet
        oRange = oSheet.get_UsedRange();
        COleSafeArray saTNmean(oRange.get_Value());

        oSheet = oSheets.get_Item(COleVariant("min"));//min sheet
        oRange = oSheet.get_UsedRange();
        COleSafeArray saTNmin(oRange.get_Value());


        long TNRows;
        long TNCols;
        saTNmax.GetUBound(1, &TNRows);//number of rows (max sheet für alle sheets)
        saTNmax.GetUBound(2, &TNCols);//number of columns (max sheet für alle sheets)
        long index[2];//1 based index for retrieving data from saTNmax (max sheet für alle sheets)

        long indexO[2];//1 based index for retrieving data from saTNmax (max sheet für alle sheets)



        CString strItem;
        LVITEM lvi;

    for (int rowCounter = 1; rowCounter <= TNRows; rowCounter++){
	    for (int colCounter = 1, /*int*//*HS 15.06.2007*/ i=0; colCounter <= TNCols; colCounter++, i++) {
	        index[0]=rowCounter;
	        index[1]=colCounter;
	        COleVariant vDataMAX, vDataMEAN, vDataMIN;
	        saTNmax.GetElement(index,vDataMAX);
	        saTNmean.GetElement(index,vDataMEAN);
	        saTNmin.GetElement(index,vDataMIN);

          if(rowCounter == 1 && colCounter >= 2){//ChemComp
            if(vDataMAX.vt == VT_BSTR){
                strItem = vDataMAX.bstrVal;

                lvi.mask =  LVIF_TEXT;
                // 1 column
	            //strItem.Format("%i",i);
	            lvi.iItem = i;
	            lvi.iSubItem = 0;
	            lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
                m_stationdata_list.InsertItem(&lvi);
            }
          }


          //--
            CString rft;
        	COleVariant vDataO;//--um die stationsnamen zum abgleich einzulesen
	        indexO[0]=rowCounter;
	        indexO[1]=1;

	        saTNmax.GetElement(indexO,vDataO);//wird durch indexO[0]=rowCounter hochgezählt
            rft = vDataO.bstrVal;

          if(rowCounter > 1 && rft == m_strstation ){// abgleich für den richtigen Stationsnamen 
            //-----
            if (vDataMAX.vt==VT_R8){
                double szdata=vDataMAX.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i-1;//-1 weil hier schon rowCounter > 1
		        lvi.iSubItem =1;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
            //------
            if (vDataMEAN.vt==VT_R8){
                double szdata=vDataMEAN.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i-1;//-1 weil hier schon rowCounter > 1
		        lvi.iSubItem =2;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
            //--------
            if (vDataMIN.vt==VT_R8){
                double szdata=vDataMIN.dblVal;
                // 1 column 
                strItem.Format("%g",szdata);
	            lvi.iItem = i-1;//-1 weil hier schon rowCounter > 1
		        lvi.iSubItem =3;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_stationdata_list.SetItem(&lvi);
            }
  
          }


//
    }
        }

    }


    //--Stationsnamen für Anzeige übergeben--
    m_actual_stationname = m_strstation;

    GetDlgItem(IDC_TEXT_DATAFROMSTATION) -> EnableWindow(TRUE);//--Überschrift Station-Combo aktivieren

    UpdateData(FALSE);//--Daten an Anzeige geben--

}

void CGSPropertyRightImportData::GetTypenamefromComboBox(void)
{
  UpdateData(TRUE);//get Typename from Combobox

  m_iSiteSelected = m_combo_station.GetCurSel();
  if(m_iSiteSelected<0){return;}
  m_combo_station.GetLBText( m_iSiteSelected, m_strStationName );

}
void CGSPropertyRightImportData::OnCbnSelchangeComboStation()
{
        OnBnClickedButtonStatdata();
}

void CGSPropertyRightImportData::fileopendialog()
{
    char szFilters[]="PhreeQC File (*.xls)|*.xls|All Files (*.*)|*.*||";
      
    CFileDialog m_ldFile(TRUE, "xls", "*.xls", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, this);
	if(m_ldFile.DoModal() == IDOK){
	m_fileopen = m_ldFile.GetPathName();
        if(m_fileopen.Find(".xls")== -1){
        AfxMessageBox("No Excel File!");
        return;
        }
    UpdateData(FALSE);
    }
}