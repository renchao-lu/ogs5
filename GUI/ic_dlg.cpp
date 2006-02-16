// ic_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "ic_dlg.h"
#include "rf_ic_new.h"
#include "gs_project.h"
#include "MainFrm.h" //OK

// CDialogInitialConditions dialog

IMPLEMENT_DYNAMIC(CDialogInitialConditions, CDialog)
CDialogInitialConditions::CDialogInitialConditions(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogInitialConditions::IDD, pParent)
{
  m_dValue = 0.0;
}

CDialogInitialConditions::~CDialogInitialConditions()
{
}

void CDialogInitialConditions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
    DDX_Control(pDX, IDC_COMBO_DIS_TYPE, m_CB_DISType);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Text(pDX, IDC_EDIT_VALUE, m_dValue);
    DDX_Control(pDX, IDC_LIST, m_LC);
    DDX_Control(pDX, IDC_COMBO_PCS_PV_NAME, m_CB_PV);
}

BEGIN_MESSAGE_MAP(CDialogInitialConditions, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnCbnSelchangeComboPCSType)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_CBN_SELCHANGE(IDC_COMBO_DIS_TYPE, OnCbnSelchangeComboDISType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_GROUP, OnBnClickedButtonCreateGroup)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_READ_IC, OnBnClickedButtonRead)
    ON_BN_CLICKED(IDC_BUTTON_READ_DATA, OnBnClickedButtonReadData)
    ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_TEC, OnBnClickedButtonWriteTEC)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_PV_NAME, OnCbnSelchangeComboPVName)
END_MESSAGE_MAP()


// CDialogInitialConditions message handlers

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation based on CDialogBoundaryConditions
**************************************************************************/
BOOL CDialogInitialConditions::OnInitDialog() 
{
  int i;
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // PCS types
  //---------------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_CB_PCSType.ResetContent();
  m_CB_PCSType.InitStorage(10,80);
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSTypeName = (CString)m_pcs->pcs_type_name.c_str();
    m_CB_PCSType.AddString(m_strPCSTypeName);
    m_CB_PCSType.SetCurSel(i);
  }
  //......................................................................
  // Primary variables
  m_CB_PV.ResetContent();
  m_CB_PV.InitStorage(10,80);
  m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs){
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_strPVName = (CString)m_pcs->pcs_primary_function_name[i];
      m_CB_PV.AddString(m_strPVName);
      m_CB_PV.SetCurSel(i);
    }
  }
  //---------------------------------------------------------------------------
  // GEO types
  //---------------------------------------------------------------------------
  m_CB_GEOType.ResetContent();
  m_CB_GEOType.InitStorage(5,80);
  m_CB_GEOType.AddString("POINT");
  m_CB_GEOType.AddString("POLYLINE");
  m_CB_GEOType.AddString("SURFACE");
  m_CB_GEOType.AddString("VOLUME");
  m_strGEOTypeName = "DOMAIN";
  m_CB_GEOType.AddString(m_strGEOTypeName);
  m_CB_GEOType.SetCurSel(4);
  //---------------------------------------------------------------------------
  // DIS types
  //---------------------------------------------------------------------------
  m_CB_DISType.ResetContent();
  m_CB_DISType.InitStorage(3,80);
  m_strDISTypeName = "CONSTANT";
  m_CB_DISType.AddString(m_strDISTypeName);
  m_CB_DISType.AddString("LINEAR");
  m_CB_DISType.AddString("DATA_BASE");
  m_CB_DISType.SetCurSel(0);
  //---------------------------------------------------------------------------
  // Table
  //---------------------------------------------------------------------------
  FillTable();
  //---------------------------------------------------------------------------
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  //---------------------------------------------------------------------------
  return TRUE;
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnCbnSelchangeComboPCSType()
{
  m_CB_PCSType.GetLBText(m_CB_PCSType.GetCurSel(),m_strPCSTypeName);
  OnInitDialog();
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnCbnSelchangeComboPVName()
{
  m_CB_PV.GetLBText(m_CB_PV.GetCurSel(),m_strPVName);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnCbnSelchangeComboGEOType()
{
  m_CB_GEOType.GetLBText(m_CB_GEOType.GetCurSel(),m_strGEOTypeName);
  m_LB_GEO.ResetContent();
  //...........................................................................
  // Points
  if(m_strGEOTypeName.Compare("POINT")==0)
  {
    long i;
    for(i=0;i<(long)gli_points_vector.size();i++) {
      m_LB_GEO.AddString((CString)gli_points_vector[i]->name.c_str());
    }
  }
  //...........................................................................
  // Polylines
  if(m_strGEOTypeName.Compare("POLYLINE")==0) {
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
  if(m_strGEOTypeName.Compare("SURFACE")==0) {
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
  if(m_strGEOTypeName.Compare("VOLUME")==0) {
     vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC
    CGLVolume* m_vol = NULL;
    while(p_vol!=volume_vector.end()) {
      m_vol = *p_vol;
      m_LB_GEO.AddString((CString)m_vol->name.data());
      ++p_vol;
    }
  }
  //...........................................................................
  // Domain
  if(m_strGEOTypeName.Compare("DOMAIN")==0) {
    //m_strGEOName = "DOMAIN";
  }
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnLbnSelchangeListGEO()
{
  m_LB_GEO.GetText(m_LB_GEO.GetCurSel(),m_strGEOName);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnCbnSelchangeComboDISType()
{
  m_CB_DISType.GetLBText(m_CB_DISType.GetCurSel(),m_strDISTypeName);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonCreate()
{
  CString m_str;
  CNodeValue* m_nod_val = NULL;
  UpdateData(true);
  //----------------------------------------------------------------------
  if(m_strPCSTypeName.IsEmpty()) {
    AfxMessageBox("No PCS Type and PV selected"); return; 
  }
  if(m_strGEOName.IsEmpty()&&(m_strGEOTypeName.Compare("DOMAIN"))) {
    AfxMessageBox("No GEO Name/Type selected"); return; 
  }
  if(m_strDISTypeName.IsEmpty()) {
    AfxMessageBox("No DIS Type selected"); return;
  }
  //----------------------------------------------------------------------
  m_obj = new CInitialCondition();
  m_obj->pcs_type_name = m_strPCSTypeName;
  m_obj->pcs_pv_name = m_strPVName;
  m_obj->geo_type_name = m_strGEOTypeName;
  m_obj->geo_name = m_strGEOName;
  m_obj->dis_type_name = m_strDISTypeName;
  //......................................................................
  if(m_obj->dis_type_name.compare("CONSTANT")==0){
    m_nod_val = new CNodeValue();
    m_nod_val->node_value = m_dValue;
    m_obj->node_value_vector.push_back(m_nod_val);
  }
  //......................................................................
  if(m_obj->geo_type_name.compare("POINT")==0){
    m_str = m_strGEOName.GetAt(5); //OK
    m_nod_val->geo_node_number = strtol(m_str,NULL,0);
  }
  else if(m_obj->geo_type_name.compare("DOMAIN")==0){
    m_nod_val = new CNodeValue();
    m_nod_val->node_value = m_dValue;
    m_obj->node_value_vector.push_back(m_nod_val);
  }
  ic_vector.push_back(m_obj);
  //----------------------------------------------------------------------
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    GSPAddMember(m_gsp->base + ".ic");
  //----------------------------------------------------------------------
  FillTable();
  //----------------------------------------------------------------------
  // Draw IC data
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    if(ic_vector.size()>0){
      theApp->g_graphics_modeless_dlg->m_bDisplayIC = true;
      theApp->g_graphics_modeless_dlg->m_iDisplayIsolines = true;
      theApp->g_graphics_modeless_dlg->m_iDisplayIsosurfaces = true;
    }
    else{
      theApp->g_graphics_modeless_dlg->m_bDisplayIC = false;
      theApp->g_graphics_modeless_dlg->m_iDisplayIsolines = false;
      theApp->g_graphics_modeless_dlg->m_iDisplayIsosurfaces = false;
    }
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonRemove()
{
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int nSelRow = m_LC.GetNextSelectedItem(p);
  if(nSelRow>=0){
    m_obj = ic_vector[nSelRow];
    delete m_obj;
    ic_vector.erase((ic_vector.begin()+nSelRow));
  }
  FillTable();
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonCreateGroup()
{
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs){
    m_pcs->SetIC();
  }
  else{
    AfxMessageBox("Select Process");
  }
  aktuelle_zeit = 0.0; //OK
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonRead()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonReadData()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    ICWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonWriteTEC()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditions::FillTable()
{
  CRect rect;
  long listip = 0;
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
  m_LC.InsertColumn(0,"PCS Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,"GEO Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,"GEO Object",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(3);
  m_LC.InsertColumn(3,"DIS Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(4);
  m_LC.InsertColumn(4,"DIS Object",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(5);
  m_LC.InsertColumn(5,"FCT",LVCFMT_LEFT,rect.Width()*1/6,0);
  // Insert data
  m_LC.DeleteAllItems(); //OK
  for(int i=0;i<(int)ic_vector.size();i++){
    m_obj = ic_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_obj->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_LC.SetItemText(listip,1,m_obj->geo_type_name.c_str());
    m_LC.SetItemText(listip,2,m_obj->geo_name.c_str());
    m_LC.SetItemText(listip,3,m_obj->dis_type_name.c_str());
    if(m_obj->node_value_vector.size()>0)
      m_strItem.Format("%g",m_obj->node_value_vector[0]->node_value);
    m_LC.SetItemText(listip,4,m_strItem);
    listip++;
  }
}