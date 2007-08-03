/**************************************************************************
GeoSys-GUI-Class: COutputDlg
Task: OUT dialog
Programing:
11/2004 OK Implementation
**************************************************************************/
// out_dlg.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "out_dlg.h"
// FEMLib
#include "rf_out_new.h"
#include "rf_pcs.h"
// GEOLib
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
#include "gs_project.h"
#include ".\out_dlg.h"

// CDialogOUT dialog

IMPLEMENT_DYNAMIC(CDialogOUT, CDialog)
CDialogOUT::CDialogOUT(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogOUT::IDD, pParent)
    , m_iDATType(FALSE)
{
  m_dTIMSteps = 1.0;
  m_iTIMType = 0;
  m_iDATType = 0;
  m_strDATType = "TECPLOT";
}

CDialogOUT::~CDialogOUT()
{
}

void CDialogOUT::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_COMBO_NOD_VALUES, m_CB_NODType);
    DDX_Control(pDX, IDC_COMBO_ELE_VALUES, m_CB_ELEType);
    DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Control(pDX, IDC_LIST_TIM, m_LB_TIM);
    DDX_Control(pDX, IDC_LIST, m_LC);
    DDX_Radio(pDX, IDC_RADIO_TIM_TYPE1, m_iTIMType);
    DDX_Text(pDX, IDC_EDIT_TIM_VALUE, m_dTIMSteps);
    DDX_Control(pDX, IDC_LIST_VAL, m_LB_VAL_NOD);
    DDX_Control(pDX, IDC_LIST_VAL_ELE, m_LB_VAL_ELE);
    DDX_Radio(pDX, IDC_RADIO_DAT_TYPE1, m_iDATType);
}

BEGIN_MESSAGE_MAP(CDialogOUT, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnCbnSelchangeComboPCSType)
    // NOD
    ON_CBN_SELCHANGE(IDC_COMBO_NOD_VALUES, OnCbnSelchangeComboNODValues)
    ON_LBN_SELCHANGE(IDC_LIST_VAL, OnLbnSelchangeListVAL)
    ON_BN_CLICKED(ID_VAL_CLEAR, OnBnClickedVALClear)
    // ELE
    ON_CBN_SELCHANGE(IDC_COMBO_ELE_VALUES, OnCbnSelchangeComboELEValues)
    ON_LBN_SELCHANGE(IDC_LIST_VAL_ELE, OnLbnSelchangeListValELE)
    ON_BN_CLICKED(ID_VAL_ELE_CLEAR, OnBnClickedValELEClear)
    // GEO
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    // TIM
    ON_BN_CLICKED(ID_ADD_OUT_TIME, OnBnClickedAddOUTime)
    // LIST
    ON_BN_CLICKED(IDC_BUTTON_OUT_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_OUT_REMOVE, OnBnClickedButtonRemove)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnLvnItemchangedList)
    //
    ON_BN_CLICKED(IDC_BUTTON_OUT_WRITE, OnBnClickedButtonWrite)
    //ON_BN_CLICKED(ID_ADD_NOD_VALUE, OnBnClickedAddNodValue)
END_MESSAGE_MAP()

// CDialogOUT message handlers

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
BOOL CDialogOUT::OnInitDialog() 
{
  int i;
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // PCS types
  CRFProcess* m_pcs = NULL;
  m_CB_PCSType.ResetContent();
  m_CB_PCSType.InitStorage(10,80);
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSType = m_pcs->pcs_type_name.c_str();
    m_CB_PCSType.AddString(m_strPCSType);
  }
  m_CB_PCSType.SetCurSel(0);
  //---------------------------------------------------------------------------
  // NOD variables
  m_CB_NODType.ResetContent();
  m_CB_NODType.InitStorage(10,80);
  m_pcs = PCSGet((string)m_strPCSType);
  if(m_pcs){
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_strNODName = (CString)m_pcs->pcs_primary_function_name[i];
      m_CB_NODType.AddString(m_strNODName);
      m_CB_NODType.SetCurSel(i);
    }
    for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
      m_CB_NODType.AddString((CString)m_pcs->pcs_secondary_function_name[i]);
    }
  }
  //......................................................................
  m_LB_VAL_NOD.ResetContent();
  if(m_pcs){
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_strNODName = (CString)m_pcs->pcs_primary_function_name[i];
      m_LB_VAL_NOD.AddString(m_strNODName);
    }
    for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
      m_LB_VAL_NOD.AddString((CString)m_pcs->pcs_secondary_function_name[i]);
    }
  }
  //---------------------------------------------------------------------------
  // ELE variables
  m_CB_ELEType.ResetContent();
  m_CB_ELEType.InitStorage(10,80);
  if(m_pcs){
    for(i=0;i<m_pcs->pcs_number_of_evals;i++){
      m_strELEName = (CString)m_pcs->pcs_eval_name[i];
      m_CB_ELEType.AddString(m_strELEName);
      m_CB_ELEType.SetCurSel(0);
    }
  }
  //......................................................................
  m_LB_VAL_ELE.ResetContent();
  if(m_pcs){
    for(i=0;i<m_pcs->pcs_number_of_evals;i++){
      m_strELEName = (CString)m_pcs->pcs_eval_name[i];
      m_LB_VAL_ELE.AddString(m_strELEName);
    }
  }
  //---------------------------------------------------------------------------
  // GEO types
  m_CB_GEOType.ResetContent();
  m_CB_GEOType.InitStorage(5,80);
  m_CB_GEOType.AddString("POINT");
  m_CB_GEOType.AddString("POLYLINE");
  m_CB_GEOType.AddString("SURFACE");
  m_CB_GEOType.AddString("VOLUME");
  m_strGEOType = "DOMAIN";
  m_CB_GEOType.AddString(m_strGEOType);
  m_CB_GEOType.SetCurSel(4);
  //======================================================================
  // Table
  CRect rect;
  CString m_strItem;
  // Table configuration
  m_LC.GetWindowRect(&rect);
  m_LC.SetTextBkColor(RGB(153,153,255));
  m_LC.SetTextColor(RGB(0,0,255));
   // pause the mouse with highlight or you can select it
   // so this will be the hot item.
  m_LC.EnableTrackingToolTips();
  m_LC.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES);
  LV_ITEM lvitem;
  lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
  // Insert head line
  m_LC.DeleteColumn(0);
  m_LC.InsertColumn(0,"PCS Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,"NOD Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,"ELE Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(3);
  m_LC.InsertColumn(3,"GEO Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(4);
  m_LC.InsertColumn(4,"GEO Object",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(5);
  m_LC.InsertColumn(5,"TIM Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteColumn(6);
  m_LC.InsertColumn(6,"DAT Type",LVCFMT_LEFT,rect.Width()*1/7,0);
  m_LC.DeleteAllItems(); //OK
  //......................................................................
  // Insert data
  UpdateList();
/*
  long listip = 0;
  for(i=0;i<(int)out_vector.size();i++){
    m_obj = out_vector[i];
    if(m_obj->time_vector.size()>0){
      m_strTIMType = "TIMES";
    }
    else{
      m_strTIMType = "STEPS";
    }
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_obj->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_LC.SetItemText(listip,1,"ALL");
    m_LC.SetItemText(listip,2,"ALL");
    m_LC.SetItemText(listip,3,m_obj->geo_type_name.c_str());
    m_LC.SetItemText(listip,4,m_obj->geo_name.c_str());
    m_LC.SetItemText(listip,5,m_strTIMType);
    m_LC.SetItemText(listip,6,"TECPLOT");
    listip++;
  }
*/
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  //GetDlgItem(IDC_BUTTON_OUT_CREATE)->EnableWindow(FALSE);
  //GetDlgItem(IDC_BUTTON_OUT_REMOVE)->EnableWindow(FALSE);
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  return TRUE;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
02/2007 CC Modification
**************************************************************************/
void CDialogOUT::OnCbnSelchangeComboPCSType()
{
  int i;
  m_CB_NODType.ResetContent();
  m_CB_NODType.InitStorage(10,80);
  m_LB_VAL_NOD.ResetContent();
  m_CB_ELEType.ResetContent();
  m_CB_ELEType.InitStorage(10,80);
  m_LB_VAL_ELE.ResetContent();
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_CB_PCSType.GetLBText(m_CB_PCSType.GetCurSel(),m_strPCSType);
  m_pcs = PCSGet((string)m_strPCSType);
  m_pcs = pcs_vector[m_CB_PCSType.GetCurSel()];//CC
  pcs_vector_number = m_CB_PCSType.GetCurSel();
  pcs_vector_number = m_CB_PCSType.GetCurSel();//CC
  if(m_pcs){
    //--------------------------------------------------------------------
    // NOD variables
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_strNODName = (CString)m_pcs->pcs_primary_function_name[i];
      m_CB_NODType.AddString(m_strNODName);
      m_CB_NODType.SetCurSel(i);
    }
    for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
      m_CB_NODType.AddString((CString)m_pcs->pcs_secondary_function_name[i]);
    }
    //....................................................................
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_strNODName = (CString)m_pcs->pcs_primary_function_name[i];
      m_LB_VAL_NOD.AddString(m_strNODName);
    }
    for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
      m_LB_VAL_NOD.AddString((CString)m_pcs->pcs_secondary_function_name[i]);
    }
    //--------------------------------------------------------------------
    // ELE variables
    for(i=0;i<m_pcs->pcs_number_of_evals;i++){
      m_strELEName = (CString)m_pcs->pcs_eval_name[i];
      m_CB_ELEType.AddString(m_strELEName);
      m_CB_ELEType.SetCurSel(i);
    }
    //....................................................................
    for(i=0;i<m_pcs->pcs_number_of_evals;i++){
      m_strELEName = (CString)m_pcs->pcs_eval_name[i];
      m_LB_VAL_ELE.AddString(m_strELEName);
    }
  }
  UpdateData(FALSE);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnCbnSelchangeComboNODValues()
{
  m_CB_NODType.GetLBText(m_CB_NODType.GetCurSel(),m_strNODName);
  m_LB_VAL_NOD.AddString(m_strNODName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnBnClickedVALClear()
{
  m_LB_VAL_NOD.ResetContent();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnCbnSelchangeComboELEValues()
{
  m_CB_ELEType.GetLBText(m_CB_ELEType.GetCurSel(),m_strELEName);
  int i;
  CRFProcess* m_pcs = NULL;
  if(m_strELEName.Compare("ALL")==0){
    m_LB_VAL_NOD.ResetContent();
    m_pcs = PCSGet((string)m_strPCSType);
    if(m_pcs){
      for(i=0;i<m_pcs->pcs_number_of_evals;i++){
        m_LB_VAL_ELE.AddString(m_pcs->pcs_eval_name[i]);
      }
    }
  }
  else{
    m_LB_VAL_ELE.AddString(m_strELEName);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnLbnSelchangeListValELE()
{
  m_CB_ELEType.GetLBText(m_CB_ELEType.GetCurSel(),m_strELEName);
  m_LB_VAL_ELE.AddString(m_strELEName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnBnClickedValELEClear()
{
  m_LB_VAL_ELE.ResetContent();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnCbnSelchangeComboGEOType()
{
  m_CB_GEOType.GetLBText(m_CB_GEOType.GetCurSel(),m_strGEOType);
  m_LB_GEO.ResetContent();
  //...........................................................................
  // Points
  if(m_strGEOType.Compare("POINT")==0)
  {
    for(long i=0;i<(long)gli_points_vector.size();i++) {
      m_LB_GEO.AddString((CString)gli_points_vector[i]->name.c_str());
    }
  }
  //...........................................................................
  // Polylines
  if(m_strGEOType.Compare("POLYLINE")==0) {
    vector<CGLPolyline*>::iterator p_ply = polyline_vector.begin();//CC
    CGLPolyline *gs_polyline = NULL;
    while(p_ply!=polyline_vector.end()) {
      gs_polyline = *p_ply;
      m_LB_GEO.AddString((CString) gs_polyline->name.c_str());
      ++p_ply;
    }
  }
  //...........................................................................
  // Surfaces
  if(m_strGEOType.Compare("SURFACE")==0) {
    vector<Surface*>::iterator p_sfc = surface_vector.begin();//CC
    Surface *gs_surface = NULL;
    while(p_sfc!=surface_vector.end()) {
      gs_surface = *p_sfc;
      m_LB_GEO.AddString((CString)gs_surface->name.data());
      ++p_sfc;
    }
  }
  //...........................................................................
  // Volumes
  if(m_strGEOType.Compare("VOLUME")==0) {
  }
  //...........................................................................
  // Domain
  if(m_strGEOType.Compare("DOMAIN")==0) {
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnLbnSelchangeListGEO()
{
  m_LB_GEO.GetText(m_LB_GEO.GetCurSel(),m_strGEOName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2005 OK Implementation
10/2005 YD sorting by values
**************************************************************************/
void CDialogOUT::OnBnClickedAddOUTime()
{
  CString m_strTIME;
  UpdateData(TRUE);
  int i, swap = 0;
  CString m_str;
  if(m_iTIMType==1){
//Empty list in box
	  while(m_LB_TIM.GetCount() > 0){
		  if((int)order.size() == 0){
          for(i=0;i<m_LB_TIM.GetCount();i++){
            m_LB_TIM.GetText(i,m_str);
            order.push_back(strtod(m_str,NULL));
           }
		  }
		for(i=0;i<m_LB_TIM.GetCount();i++)
	       m_LB_TIM.DeleteString(i);
	  }
//----------
      for(i=0;i<(int)order.size()-1;i++){   
		  if(m_dTIMSteps == order.at(i))
			  swap ++;
	  }
	  if(swap == 0)
      order.push_back(m_dTIMSteps);
//Sorting by value 
  do{
     swap = 0;
     for(i=0;i<(int)order.size()-1;i++){     
	   if(order.at(i) > order.at(i+1)){
		 double buffer = order.at(i);
         order.at(i) = order.at(i+1);
         order.at(i+1) = buffer;
		 swap++;
	   }
     }
   }while(swap != 0);
//Add string in list box
	 for(i=0;i <(int)order.size();i++){
       m_strTIME.Format("%g",order.at(i));
	   m_LB_TIM.AddString(m_strTIME);
    }
  }
  else{
    AfxMessageBox("Select OUT times");
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
02/2007 CC Modification
**************************************************************************/
void CDialogOUT::OnBnClickedButtonCreate()
{
  int i;
  double time;
  CString m_str;
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  // Tests
  if(m_strPCSType.IsEmpty()) {
    AfxMessageBox("No PCS Type and PV selected"); return; 
  }
  if(m_strGEOName.IsEmpty()&&(m_strGEOType.Compare("DOMAIN"))) {
    AfxMessageBox("No GEO Name/Type selected"); return; 
  }
  //----------------------------------------------------------------------
  m_obj = new COutput();
  m_obj->pcs_vector_number = pcs_vector_number;//CC
  m_obj->pcs_type_name = m_strPCSType;
  m_obj->geo_type_name = m_strGEOType;
  m_obj->geo_name = m_strGEOName;
  //......................................................................
  for(i=0;i<m_LB_VAL_NOD.GetCount();i++){
    m_LB_VAL_NOD.GetText(i,m_str);
    m_obj->nod_value_vector.push_back((string)m_str);
  }
  //......................................................................
  for(i=0;i<m_LB_VAL_ELE.GetCount();i++){
    m_LB_VAL_ELE.GetText(i,m_str);
    m_obj->ele_value_vector.push_back((string)m_str);
  }
  //......................................................................
  switch(m_iTIMType){
    case 0:
      m_obj->tim_type_name = "STEPS";
      m_obj->nSteps = (int)m_dTIMSteps;
      break;
    case 1:
      m_obj->tim_type_name = "TIMES";
      for(i=0;i<m_LB_TIM.GetCount();i++){
        m_LB_TIM.GetText(i,m_str);
        time = strtod(m_str,NULL);
        m_obj->time_vector.push_back(time);
      }
      break;
  }
  //......................................................................
  switch(m_iDATType){
    case 0:
      m_obj->dat_type_name = "TECPLOT";
      break;
    case 1:
      m_obj->dat_type_name = "ROCKFLOW";
      break;
    default:
      AfxMessageBox("No DAT type");
  }
  //......................................................................
  out_vector.push_back(m_obj);
  //----------------------------------------------------------------------
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    GSPAddMember(m_gsp->base + ".out");
  //----------------------------------------------------------------------
  UpdateList();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnBnClickedButtonRemove()
{
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  nSelRow = m_LC.GetNextSelectedItem(p);
  if(nSelRow>=0){
    m_obj = out_vector[nSelRow];
    delete m_obj;
    out_vector.erase((out_vector.begin()+nSelRow));
  }
  UpdateList();
  //OnInitDialog();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogOUT::UpdateList()
{
  CString m_str;
  int listip = 0;
  m_LC.DeleteAllItems(); //OK
  //......................................................................
  // Insert data
  for(int i=0;i<(int)out_vector.size();i++){
    m_obj = out_vector[i];
    if(m_obj->tim_type_name.compare("STEPS")==0){
      m_strTIMType = "STEPS";
    }
    else{
      m_strTIMType = "TIMES";
    }
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_obj->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    if(m_obj->nod_value_vector.size()==1){
      m_LC.SetItemText(listip,1,m_obj->nod_value_vector[0].c_str());
    }
    else
      m_LC.SetItemText(listip,1,"LIST");
    if(m_obj->ele_value_vector.size()==1){
      m_LC.SetItemText(listip,2,m_obj->ele_value_vector[0].c_str());
    }
    else
      m_LC.SetItemText(listip,2,"LIST");
    m_LC.SetItemText(listip,3,m_obj->geo_type_name.c_str());
    m_LC.SetItemText(listip,4,m_obj->geo_name.c_str());
    m_LC.SetItemText(listip,5,m_obj->tim_type_name.c_str());
    m_LC.SetItemText(listip,6,m_obj->dat_type_name.c_str());
    listip++;
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
  int i;
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  pNMLV=pNMLV;//TK
  // TODO: Add your control notification handler code here
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  nSelRow = m_LC.GetNextSelectedItem(p);
  m_strPCSType = m_LC.GetItemText(nSelRow,0);
  m_strNODName = m_LC.GetItemText(nSelRow,1);
  m_strELEName = m_LC.GetItemText(nSelRow,2);
  m_strGEOType = m_LC.GetItemText(nSelRow,3);
  m_strGEOName = m_LC.GetItemText(nSelRow,4);
  m_strTIMType = m_LC.GetItemText(nSelRow,5);
  m_strDATType = m_LC.GetItemText(nSelRow,6);
  //----------------------------------------------------------------------
  for(i=0;i<(int)out_vector.size();i++){
    m_obj = out_vector[i];
    m_obj->selected = false;
  }
  if((nSelRow>-1)&&(nSelRow<(int)out_vector.size())){
    m_obj = out_vector[nSelRow];
    m_obj->selected = true;
    m_obj->nod_value_name = m_strNODName; //OK
  }
  //
  //---------------------------------------------------------------------------
  // TIMES
  m_LB_TIM.ResetContent();
  CString m_str;
  if(m_obj&&((int)m_obj->time_vector.size()>0)){
    m_iTIMType = 1;
    for(i=0;i<(int)m_obj->time_vector.size();i++){
      m_str.Format("%g",m_obj->time_vector[i]);
      m_LB_TIM.AddString(m_str);
    }
  }
  //---------------------------------------------------------------------------
  *pResult = 0;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnLbnSelchangeListVAL()
{
  m_LB_VAL_NOD.GetText(m_LB_VAL_NOD.GetCurSel(),m_strNODName);
  m_LC.SetItemText(nSelRow,1,m_strNODName); //ToDo
  m_obj->nod_value_name = m_strNODName;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnBnClickedButtonWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    OUTWrite(m_gsp->path + m_gsp->base);
}


/*
    //....................................................................
    m_obj = OUTGet(m_pcs->pcs_type_name);
    m_LB_VAL_NOD.ResetContent();
    for(i=0;i<(int)m_obj->nod_value_vector.size();i++){
      m_LB_VAL_NOD.AddString((CString)m_obj->nod_value_vector[i].c_str());
    }
    */


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogOUT::OnBnClickedAddNodValue()
{
  int i;
  CRFProcess* m_pcs = NULL;
  if(m_strNODName.Compare("ALL")==0){
    m_LB_VAL_NOD.ResetContent();
    m_pcs = PCSGet((string)m_strPCSType);
    if(m_pcs){
      for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
        m_LB_VAL_NOD.AddString(m_pcs->pcs_primary_function_name[i]);
      }
      for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
        m_LB_VAL_NOD.AddString(m_pcs->pcs_secondary_function_name[i]);
      }
    }
  }
  else{
    m_LB_VAL_NOD.AddString(m_strNODName);
  }
}

void CDialogOUT::OnOK()
{
  CDialog::OnOK();
}
