// gs_mat_mp_dataedit.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "MainFrm.h"
#include "gs_mat_mp_dataedit.h"
// C++
#include <vector>
#include <string>
using namespace std;

//#include "matrix_class.h"//JG matrix wird im Moment nicht verwendet
#include ".\gs_mat_mp_dataedit.h"
//using Math_Group::Matrix;//JG matrix wird im Moment nicht verwendet
//Math_Group::Matrix* p_matrix;//JG matrix wird im Moment nicht verwendet
vector<CString> type_name_vector;
vector<CString> key_word_vector;

// CMATGroupEditorDataEdit dialog

IMPLEMENT_DYNAMIC(CMATGroupEditorDataEdit, CDialog)
CMATGroupEditorDataEdit::CMATGroupEditorDataEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CMATGroupEditorDataEdit::IDD, pParent)
{
  //p_matrix = new Matrix(100,100); //get memory for 100x100 matrix//JG matrix wird im Moment nicht verwendet
}

CMATGroupEditorDataEdit::~CMATGroupEditorDataEdit()
{
}

void CMATGroupEditorDataEdit::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LISTCONTROL_DATA, m_listctrldata);
    DDX_Control(pDX, IDC_TYPENAMEDISPL, m_typenamedispl);
    DDX_Text (pDX, IDC_TYPENAMEDISPL, m_tndisplstr);

}

BEGIN_MESSAGE_MAP(CMATGroupEditorDataEdit, CDialog)
    ON_BN_CLICKED(IDC_OK, OnBnClickedOK)

END_MESSAGE_MAP()

// CMATGroupEditorDataEdit message handlers

BOOL CMATGroupEditorDataEdit::OnInitDialog()
{
  CDialog::OnInitDialog();

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->dataedit = true;

    //--- ListControl for editing data ---
    CRect rect;
    
    GetClientRect(&rect);
    m_listctrldata.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES);
    // Delete the current contents
	m_listctrldata.DeleteAllItems();   
    // create columns
    m_listctrldata.InsertColumn(0, _T("Parameter"),  LVCFMT_LEFT, rect.Width()/2, 0);
    m_listctrldata.InsertColumn(1, _T("Value"),  LVCFMT_LEFT, rect.Width()/2, 0);

    EmptytnkwVectors();
    //Excel2matrix();
    //matrix2ListCtrl();

    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    if(mainframe->m_fileopen.Find(".xls")!= -1 &&  mainframe->dataupdate == false){
    SafeArray2ListCtrl();
    }
    if(mainframe->m_fileopen.Find(".csv")!= -1 &&  mainframe->dataupdate == false){
        CSVtext2ListCtrl();
    }
    if(mainframe->m_fileopen == "" && mainframe->m_iSelectedMMPGroup>=0 &&  mainframe->dataupdate == true){

        // get selected m_mmp pointer
        CMediumProperties *m_mmp = NULL;
        m_mmp = new CMediumProperties();

        if((mainframe->m_iSelectedMMPGroup>-1)&&(mainframe->m_iSelectedMMPGroup<(int)mmp_vector.size())){
            m_mmp = mmp_vector[mainframe->m_iSelectedMMPGroup];
        }
    }
    return TRUE;  // return TRUE unless you set the focus to a control
}
void CMATGroupEditorDataEdit::OnBnClickedOK()
{
  //ListCtrl2matrix();
  ListCtrl2SafeArray();
  
  oExcel.Quit();
  OnOK();
}


void CMATGroupEditorDataEdit::EmptytnkwVectors(void)
{
  //empty the vectors
  if(type_name_vector.size() > 0){
    for(int i=0;i<(int)type_name_vector.size();i++){
      type_name_vector[i].Delete;
  }
            type_name_vector.clear();
        }

        if(key_word_vector.size() > 0){
    for(int i=0;i<(int)key_word_vector.size();i++){
      key_word_vector[i].Delete;
        }
            key_word_vector.clear();
        }
}
void CMATGroupEditorDataEdit::SafeArray2ListCtrl(void)
{
 
    //check EXCEL
	if (oExcel.m_lpDispatch == NULL) {
		oExcel.CreateDispatch("Excel.Application");
        if (oExcel.m_lpDispatch == NULL){
        AfxMessageBox("Excel has not started!");
        }
	}
    CWorksheets oSheets;
	CWorksheet oSheet;
	CRange oRange, oRangeCols;
	COleVariant covOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    m_fileopen = mainframe->m_fileopen;

    oBooks = oExcel.get_Workbooks();
    oBook = oBooks.Open (m_fileopen, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional);
    //open first sheet
    oSheets = oBook.get_Worksheets();
    oSheet = oSheets.get_Item(COleVariant((short)1));//first worksheet!!

    oRange = oSheet.get_UsedRange();//get all data from sheet
    //....................................................................
    //get all data of the first sheet into tn-vector, kw-vector and matrix
    
    COleSafeArray saRet(oRange.get_Value());//saRet wird als 2dim safearray angelegt

    long iRows;
    long iCols;
    saRet.GetUBound(1, &iRows);
    saRet.GetUBound(2, &iCols);

    long index[2];

    //p_matrix->LimitSize(iRows-1, iCols-1);//limit 100x100 matrix
    
  //get data..
  //..tn in type_name_vector..
  for (int colCounter = 2; colCounter <= iCols; colCounter++) {//"colCounter = 2" without keyword-column
    index[0]=1;//typename-row
    index[1]=colCounter;
	COleVariant vData;
	saRet.GetElement(index,vData);
    CString szstrings_type = vData.bstrVal;
    type_name_vector.push_back(szstrings_type);
  }
  //..kw in key_word_vector..
  for (int rowCounter = 2; rowCounter <= iRows; rowCounter++) {//"rowCounter = 2" weil erste zeile (typenames) ausgelassen wird
    index[0]=rowCounter;
    index[1]=1;//keyword-column
    COleVariant vData;
    saRet.GetElement(index,vData);

    CString szstrings_key = vData.bstrVal;
    key_word_vector.push_back(szstrings_key);
  }
 

  //fill listcontrol with data
    LVITEM lvi;
	CString strItem;
    int type_name_vector_index = -1; //index of typename in tnvector

    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

    //find index of typename
    for(int i=0; i< (int)type_name_vector.size(); i++){
        if(type_name_vector.at(i) == mainframe->m_strDBTypeName){
        type_name_vector_index = i;
        }
    }
    
    for (int i=0; i< iRows-1 ; i++){//-2 weil 1basierter safearrayindex und ohne typenamerow

            //Set first item (keywords)
            lvi.mask =  LVIF_TEXT;
		    strItem = key_word_vector.at(i);
	        lvi.iItem = i;
		    lvi.iSubItem = 0;
		    lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		    m_listctrldata.InsertItem(&lvi);
            //Set subitem (values of tn)

            //double uz = (*p_matrix)(i,type_name_vector_index);
            //strItem.Format("%g",uz);

            index[0]=i+2;//1 für 1basierten safearray-index und 1 weil ohne typename-reihe
            index[1]=type_name_vector_index+2;//1 für 1basierten safearray-index und 1 weil ohne keyword-reihe
            COleVariant vData;
            saRet.GetElement(index,vData);

            if(vData.vt == VT_BSTR){

                strItem = vData;           

		        lvi.iSubItem =1;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_listctrldata.SetItem(&lvi);
            }
            else{ 
              if (vData.vt==VT_R8){
                double szdata=vData.dblVal; 
                strItem.Format("%g",szdata);

		        lvi.iSubItem =1;
		        lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		        m_listctrldata.SetItem(&lvi);
              }
            }

    }

    m_tndisplstr = mainframe->m_strDBTypeName;
    UpdateData(FALSE);//display Typename

}

void CMATGroupEditorDataEdit::ListCtrl2SafeArray(void)
{
    //COleSafeArray saEdt;//wurde in mainframe deklariert
        CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
        DWORD numElements[] = {(int)key_word_vector.size(), 1}; // größe rows x cols
        mainframe->saEdt.Create(VT_VARIANT, 2, numElements);

    //editierte Daten in saEdt füllen
    long index[2];
    for(index[0]=0; index[0]<(int)key_word_vector.size(); index[0]++){
        //for(index[1]=0; index[1]<10; index[1]++){
        index[1]=0;
        CString strText = m_listctrldata.GetItemText(index[0],1);//.GetItemText(row, col)
        
            if(strText.FindOneOf( "0123456789" ) == 0){//wenn index für ersten treffer aus "0123456789" 0 ist, also erster buchstabe
                
                if(strText.Find(",", 0)){
                strText.Replace(",", ".");
                }

                double strval = atof(strText);//typumwandlung

                VARIANT vDouble;
                vDouble.vt = VT_R8;
                vDouble.dblVal = strval;

                mainframe->saEdt.PutElement(index, &vDouble);
            }
            else{
                BSTR rs = strText.AllocSysString();

                VARIANT vString;
                vString.vt = VT_BSTR;
                vString.bstrVal = rs;

                mainframe->saEdt.PutElement(index, &vString);
            }
        //}
    }
}

void CMATGroupEditorDataEdit::ExcelDirect2SafeArray(void)
{
    int type_name_vector_index = -1; //OK
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

    EmptytnkwVectors();

    //check EXCEL
	if (oExcel.m_lpDispatch == NULL) {
		oExcel.CreateDispatch("Excel.Application");
        if (oExcel.m_lpDispatch == NULL){
        AfxMessageBox("Excel has not started!");
        }
	}
    CWorksheets oSheets;
	CWorksheet oSheet;
	CRange oRange, oRangeCols;
	COleVariant covOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);

    m_fileopen = mainframe->m_fileopen;

    oBooks = oExcel.get_Workbooks();
    oBook = oBooks.Open (m_fileopen, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional);
    //open first sheet
    oSheets = oBook.get_Worksheets();
    oSheet = oSheets.get_Item(COleVariant((short)1));//first worksheet!!

    oRange = oSheet.get_UsedRange();//get all data from sheet
    //....................................................................
    //get all data of the first sheet into tn-vector, kw-vector and matrix
    
    //double szdata;
    //CString szstrings;

    COleSafeArray saRet(oRange.get_Value());//

    long iRows;
    long iCols;
    saRet.GetUBound(1, &iRows);
    saRet.GetUBound(2, &iCols);

    long index[2];

  //get data..
  //..tn in type_name_vector..
  for (int colCounter = 2; colCounter <= iCols; colCounter++) {//"colCounter = 2" without keyword-column
    index[0]=1;//typename-row
    index[1]=colCounter;
	COleVariant vData;
	saRet.GetElement(index,vData);
    CString szstrings_type = vData.bstrVal;
    type_name_vector.push_back(szstrings_type);
  }
  //..kw in key_word_vector..
  for (int rowCounter = 2; rowCounter <= iRows; rowCounter++) {//"rowCounter = 2" weil erste zeile (typenames) ausgelassen wird
    index[0]=rowCounter;
    index[1]=1;//keyword-column
    COleVariant vData;
    saRet.GetElement(index,vData);

    CString szstrings_key = vData.bstrVal;
    key_word_vector.push_back(szstrings_key);
  }
  //..values in SafeArray saEdt
  //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  DWORD numElements[] = {(int)key_word_vector.size(), 1};//rows, cols
  mainframe->saEdt.Create(VT_VARIANT, 2, numElements);

  long indexEdt[2];

  int type_name_vector_size = (int)type_name_vector.size();
  for(int i=0; i<type_name_vector_size; i++){
  if(type_name_vector[i] == mainframe->m_strDBTypeName){
      type_name_vector_index = i;
    }
  }

  for (int rowCounter = 1; rowCounter < iRows; rowCounter++) {

    //for (int colCounter = 1; colCounter < iCols; colCounter++) {
      index[0]=rowCounter+1;//index for saRet
      index[1]=type_name_vector_index+2;//index for saRet: selected typename
      indexEdt[0]=rowCounter-1;//index for saEdt
      indexEdt[1]=0;///index for saEdt
      COleVariant vData;
      saRet.GetElement(index,vData);

      mainframe->saEdt.PutElement(indexEdt,vData);

    //}
  }
   oExcel.Quit();
}
void CMATGroupEditorDataEdit::CSVtext2ListCtrl()
{
  //get data..
  //..tn in type_name_vector..
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  CSV_typenames2typenamevector((string)mainframe->m_fileopen);
  //..kw in key_word_vector..
  //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  CSV_keywords2keywordvector((string)mainframe->m_fileopen);

  //fill listcontrol with data
    LVITEM lvi;
	CString strItem;
    int type_name_vector_index; //index of typename in tnvector
    string line;
    string in;
    string z_rest;
    string type_name;
    string type_name_tmp("TYPE_NAME");
    char line_char[MAX_ZEILE];
    int countGetline = 0;//Zähler für zeilen while-schleife

    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    string fileopen = mainframe->m_fileopen;

    //find index of typename
    for(int i=0; i< (int)type_name_vector.size(); i++){
        if(type_name_vector.at(i) == mainframe->m_strDBTypeName){//.c_str()
        type_name_vector_index = i;
        }
    }

    //Datei zum Lesen aufmachen
    ifstream eingabe(fileopen.data(),ios::in);
    if (eingabe.good()) {
      eingabe.seekg(0L,ios::beg);//rewind

      
      while(eingabe.getline(line_char, MAX_ZEILE)){
       line = line_char;
       in = line.substr(0);
       type_name_tmp = "TYPE_NAME";//vorbelegen wegen if(!type_name_tmp.empty()){....}

       if(countGetline>0){
       //Set first item in ListControl (keywords)
       lvi.mask =  LVIF_TEXT;
       strItem = key_word_vector.at(countGetline-1);//.c_str()
	   lvi.iItem = countGetline-1;//null basierte Listeneinträge
       lvi.iSubItem = 0;
       lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	   m_listctrldata.InsertItem(&lvi);
       
         int ee = 0;
         while(!type_name_tmp.empty()) {
          readCSV_type_name(in,&z_rest);
            type_name_tmp = readCSV_type_name(in,&z_rest);
           if(ee == type_name_vector_index+1){
             //Set subitem (values of tn)
             strItem = type_name_tmp.c_str();
		     lvi.iSubItem =1;
		     lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		     m_listctrldata.SetItem(&lvi);
           }
         in = z_rest;
         ee++;
         }
       }
       countGetline++;
     }
    }
  m_tndisplstr = mainframe->m_strDBTypeName;
  UpdateData(FALSE);//display Typename
}

void CMATGroupEditorDataEdit::CSV_typenames2typenamevector(string csv_file_name)
{
  string in;
  string line;
  string z_rest;
  CString type_name;
  string type_name_tmp("TYPE_NAME");
  char line_char[MAX_ZEILE];
  string delimiter(";");

  //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //m_fileopen = mainframe->m_fileopen;

ifstream eingabe(csv_file_name.data(),ios::in);
  if (eingabe.good()) {
    eingabe.seekg(0L,ios::beg);//rewind um materialgruppen auszulesen
	eingabe.getline(line_char, MAX_ZEILE);
     line = line_char;
    if(line.find(delimiter)!=string::npos) {
        in = line.substr(1);

      while(!type_name_tmp.empty()) {
          //readCSV_type_name(in,&z_rest);
            type_name_tmp = readCSV_type_name(in,&z_rest);
        if(type_name_tmp.empty()) 
          break; 
        else {
          type_name = type_name_tmp.c_str();
          type_name_vector.push_back(type_name);
          in = z_rest;
        }
      }
    }

  }
}

string CMATGroupEditorDataEdit::readCSV_type_name(string in, string *z_rest_out)//
{
  string mat_name;
  string z_restR;
  string delimiter(";");

  delimiter = ";";


  if(in.find_first_not_of(delimiter)!=string::npos)//
  {
    z_restR = in.substr(in.find_first_not_of(delimiter));
    mat_name = z_restR.substr(0,z_restR.find_first_of(delimiter)); //string für die erste (1) material group
    *z_rest_out = z_restR.substr(mat_name.length());
	return mat_name;
  }
  else
    return "";
}

void CMATGroupEditorDataEdit::CSV_keywords2keywordvector(string csv_file_name)
{
  //string in;//is declared within the loop
  string line;
  string z_rest;
  string type_name;
  string type_name_tmp("TYPE_NAME");
  char line_char[MAX_ZEILE];
  string delimiter(";");
  

  //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //m_fileopen = mainframe->m_fileopen;

  ifstream eingabe(csv_file_name.data(),ios::in);
  if (eingabe.good()) {
    eingabe.seekg(0L,ios::beg);//rewind

	while(eingabe.getline(line_char, MAX_ZEILE)){
      CString in; 
      line = line_char;
      in = (line.substr(0,line.find_first_of(delimiter))).c_str();

      string *t = new string(in);//.c_str()
      
      if(in != ""){//in.size()>0
       key_word_vector.push_back(in);
      }
    }
  }
}
void CMATGroupEditorDataEdit::CSVtextDirect2SafeArray(void)
{
    int type_name_vector_index; //index of typename in tnvector
    string line;
    string in;
    string z_rest;
    string type_name;
    string type_name_tmp; 
    char line_char[MAX_ZEILE];
    int countGetline = 0;//Zähler für zeilen beim durchlauf while-schleife

    CString strText;
    

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

    EmptytnkwVectors();

    //get data..
    //..tn in type_name_vector..
    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    CSV_typenames2typenamevector((string)mainframe->m_fileopen);
    //..kw in key_word_vector..
    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    CSV_keywords2keywordvector((string)mainframe->m_fileopen);

    //..values in SafeArray saEdt
    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    DWORD numElements[] = {(int)key_word_vector.size(), 1};//rows, cols
    mainframe->saEdt.Create(VT_VARIANT, 2, numElements);
    COleVariant vData;

    long indexEdt[2];//index for saEdt

    int type_name_vector_size = (int)type_name_vector.size();
    for(int i=0; i<type_name_vector_size; i++){
    if(type_name_vector[i] == mainframe->m_strDBTypeName){//.c_str()
        type_name_vector_index = i;
      }
    }

    string fileopen = mainframe->m_fileopen;

    //Datei zum Lesen aufmachen
    ifstream eingabe(fileopen.data(),ios::in);
    if (eingabe.good()) {
      eingabe.seekg(0L,ios::beg);//rewind


      while(eingabe.getline(line_char, MAX_ZEILE)){
       if(countGetline>0){//countGetline>0 means: read without first line (typename-line)
         line = line_char;
         in = line.substr(0);
         type_name_tmp = "TYPE_NAME";//initialize for if(!type_name_tmp.empty()){...}

         indexEdt[0]=countGetline-1;//index for saEdt
         indexEdt[1]=0;///index for saEdt

         int ee = 0;

         //CString strText;
         while(!type_name_tmp.empty()) {
          type_name_tmp = readCSV_type_name(in,&z_rest);


           //if(type_name_tmp.empty()) 
           //  break;

           strText = type_name_tmp.c_str();          
 
           if(ee == type_name_vector_index+1){

                if(strText.FindOneOf( "0123456789" ) == 0){//wenn index für ersten treffer aus "0123456789" 0 ist, also erster buchstabe
                
                    if(strText.Find(",", 0)){
                    strText.Replace(",", ".");
                    }

                    double strval = atof(strText);//typumwandlung

                    VARIANT vDouble;
                    vDouble.vt = VT_R8;
                    vDouble.dblVal = strval;

                    mainframe->saEdt.PutElement(indexEdt, &vDouble);
                }
                else{
                    BSTR rs = strText.AllocSysString();

                    VARIANT vString;
                    vString.vt = VT_BSTR;
                    vString.bstrVal = rs;

                    mainframe->saEdt.PutElement(indexEdt, &vString);

                }
           }
         in = z_rest;
         ee++;
         }
       }//if(countGetline>0...
       countGetline++;
      }//while(eingabe.getline...
    }//if (eingabe.good()
}
