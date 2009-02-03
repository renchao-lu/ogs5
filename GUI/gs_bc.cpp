// gs_bc.cpp: Implementierungsdatei
/**************************************************************************
GeoSysGUI-File: 
Task: dialog to initial conditions
Programing:
11/2003 OK Implementation
11/2004 OK CListCtrl table based on WellTable by CC
**************************************************************************/
#include "stdafx.h"
#include "GeoSys.h"
#include "gs_bc.h"
#include "MainFrm.h" //OK
// GeoSys-FEM
#include "makros.h"
#include "rf_pcs.h"
#include "rf_fct.h"
#include "gs_project.h"
// GeoLib
#include "geo_ply.h"
#include "geo_sfc.h"
// Tools
#include "geo_strings.h"
#include "tools.h"
// MSHLib
#include "msh_lib.h"
#include ".\gs_bc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CBoundaryConditions 

CBoundaryConditions::CBoundaryConditions(CWnd* pParent /*=NULL*/)
	: CDialog(CBoundaryConditions::IDD, pParent)
{
  m_strValues.Format("%g",0.0); //OK
}

void CBoundaryConditions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Control(pDX, IDC_COMBO_DIS_TYPE, m_CB_DISType);
    DDX_Text(pDX, IDC_EDIT_VALUE, m_strValues);
    DDX_Control(pDX, IDC_LIST_BC_NEW, m_LC_BC);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_COMBO_TIM_TYPE, m_CB_TIMType);
    DDX_Control(pDX, IDC_COMBO_PCS_PV_NAME, m_CB_PV);
}

BEGIN_MESSAGE_MAP(CBoundaryConditions, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_CBN_SELCHANGE(IDC_COMBO_DIS_TYPE, OnCbnSelchangeComboDISType)
    ON_BN_CLICKED(IDC_BUTTON_BC_CREATE_GROUP, OnBnClickedButtonCreateGroup)
    ON_BN_CLICKED(IDC_BUTTON_BC_REMOVE, OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_BC_ADD2, OnBnClickedButtonCreate)
    ON_LBN_SELCHANGE(IDC_LIST_TIM, OnLbnSelchangeListFCT)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnLbnSelchangeListPCSType)
    ON_BN_CLICKED(IDC_BUTTON_BC_WRITE, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_TEC, OnBnClickedButtonWriteTEC)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_PV_NAME, OnCbnSelchangeComboPVName)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Dialog

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
11/2004 OK new table
**************************************************************************/
BOOL CBoundaryConditions::OnInitDialog() 
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
  m_strGEOTypeName = "POINT";
  m_CB_GEOType.AddString(m_strGEOTypeName);
  m_CB_GEOType.AddString("POLYLINE");
  m_CB_GEOType.AddString("SURFACE");
  m_CB_GEOType.SetCurSel(0);
  //---------------------------------------------------------------------------
  // DIS types
  //----------------------------------------------------------------------
  GetDlgItem(IDC_BUTTON_BC_READ_CSV)->EnableWindow(FALSE);
  m_CB_DISType.ResetContent();
  m_CB_DISType.InitStorage(3,80);
  m_strDISTypeName = "CONSTANT";
  m_CB_DISType.AddString(m_strDISTypeName);
  m_CB_DISType.AddString("LINEAR");
  m_CB_DISType.AddString("DATA_BASE");
  m_CB_DISType.SetCurSel(0);
  //----------------------------------------------------------------------
  // FCT types
  //----------------------------------------------------------------------
  m_CB_TIMType.ResetContent();
  //OK CFunction* m_fct = NULL;
  int no_functions = anz_kurven; //OK fct_vector.size();
  CString m_strFunctionNumber;
  char m_charLB[5];
  for(i=0;i<no_functions;i++){
    //OK m_fct = fct_vector[i];
    //OK m_LB_TIM.AddString((CString)m_fct->name.c_str());
    m_strFunctionNumber.Format("%i",i);
    sprintf(m_charLB,"%i",i);
    m_CB_TIMType.AddString((CString)m_charLB);
  }
  m_CB_TIMType.SetCurSel(0);
  //----------------------------------------------------------------------
  // Table
  FillTable();
  //----------------------------------------------------------------------
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  //----------------------------------------------------------------------
  return TRUE;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnLbnSelchangeListPCSType()
{
  m_CB_PCSType.GetLBText(m_CB_PCSType.GetCurSel(),m_strPCSTypeName);
  //OK OnInitDialog();
/*
  m_LC_BC.DeleteAllItems();
  long listip = 0;
  CString m_strItem;
  list<CBoundaryCondition*>::const_iterator p_bc;
  p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if((m_bc->pcs_type_name.compare(m_strPCSType)==0)||\
       (m_strPCSType=="ALL")){
      LV_ITEM lvitem;
      lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
      lvitem.pszText = (LPSTR)m_bc->pcs_type_name.c_str();
      lvitem.iItem = listip; //size of list
      lvitem.iSubItem = 0;
      m_LC_BC.InsertItem(&lvitem);
      m_LC_BC.SetItemText(listip,1,m_bc->geo_type_name.c_str());
      m_LC_BC.SetItemText(listip,2,m_bc->geo_name.c_str());
      m_LC_BC.SetItemText(listip,3,m_bc->dis_type_name.c_str());
      m_strItem.Format("%g",m_bc->geo_node_value);
      m_LC_BC.SetItemText(listip,4,m_strItem);
      m_LC_BC.SetItemText(listip,5,m_bc->fct_name.c_str());
      ++p_bc;
      listip++;
    }
    else
      ++p_bc;
  }
*/
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnCbnSelchangeComboPVName()
{
  m_CB_PV.GetLBText(m_CB_PV.GetCurSel(),m_strPVName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnCbnSelchangeComboGEOType()
{
  m_CB_GEOType.GetLBText(m_CB_GEOType.GetCurSel(),m_strGEOTypeName);

  m_LB_GEO.ResetContent();
  //...........................................................................
  // Points
  if(m_strGEOTypeName.Compare("POINT")==0)
  {
    long gli_points_vector_length = (long)gli_points_vector.size();
    long i;
    for(i=0;i<gli_points_vector_length;i++) {
      m_LB_GEO.AddString((CString)gli_points_vector[i]->name.c_str());
    }
  }
  //...........................................................................
  // Polylines
  if(m_strGEOTypeName.Compare("POLYLINE")==0) {
    vector<CGLPolyline*>::iterator p_ply = polyline_vector.begin();//CC 08/2005
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
    while(p_sfc!=surface_vector.end()) {//CC
      gs_surface = *p_sfc;
      m_LB_GEO.AddString((CString)gs_surface->name.data());
      ++p_sfc;
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnLbnSelchangeListGEO()
{
  m_LB_GEO.GetText(m_LB_GEO.GetCurSel(),m_strGEOName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnCbnSelchangeComboDISType()
{
  GetDlgItem(IDC_BUTTON_BC_READ_CSV)->EnableWindow(FALSE);
  m_CB_DISType.GetLBText(m_CB_DISType.GetCurSel(),m_strDISTypeName);
  if(m_strDISTypeName=="DATA_BASE")
    GetDlgItem(IDC_BUTTON_BC_READ_CSV)->EnableWindow(TRUE);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnLbnSelchangeListFCT()
{
  m_CB_TIMType.GetLBText(m_CB_TIMType.GetCurSel(),m_strFCTName);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2004 OK Implementation
11/2004 OK new Table
07/2007 OK LINEAR
last modified:
**************************************************************************/
void CBoundaryConditions::OnBnClickedButtonCreate()
{
  CString m_str;
  std::stringstream in;
  UpdateData(true);
  if(m_strPCSTypeName.IsEmpty()) {
    AfxMessageBox("No PCS Type selected"); return; 
  }
  if(m_strGEOName.IsEmpty()) {
    AfxMessageBox("No GEO Name selected"); return; 
  }
  if(m_strDISTypeName.IsEmpty()) {
    AfxMessageBox("No DIS Type selected"); return;
  }
  //----------------------------------------------------------------------
  m_bc = new CBoundaryCondition();
  m_bc->pcs_type_name = m_strPCSTypeName;
  m_bc->pcs_pv_name = m_strPVName;
  //----------------------------------------------------------------------
  // GEO type
  m_bc->geo_type_name = m_strGEOTypeName;
  m_bc->SetGEOType();
  m_bc->geo_name = m_strGEOName;
  if(m_bc->geo_type_name.compare("POINT")==0)
  {
    m_str = m_strGEOName.GetAt(5); //OK
    CGLPoint *m_point  = NULL; //CC
    m_point = GEOGetPointByName(m_bc->geo_name);//CC
    if(m_point)
      m_bc->geo_node_number = m_point->id;//CC
  }
  //----------------------------------------------------------------------
  // DIS type
  m_bc->dis_type_name = m_strDISTypeName;
  m_bc->SetDISType();
  //......................................................................
  if(m_bc->dis_type_name.compare("CONSTANT")==0)
  {
    m_bc->geo_node_value = atof(m_strValues);
  }
  //......................................................................
  if(m_bc->dis_type_name.compare("LINEAR")==0)
  {
    m_bc->dis_type = 2;
    long ldummy=0; double ddummy=0.0;
    in.str((string)m_strValues);
    in >> ldummy >> ddummy;
    m_bc->PointsHaveDistribedBC.push_back(ldummy);
    m_bc->DistribedBC.push_back(ddummy);
    in >> ldummy >> ddummy;
    m_bc->PointsHaveDistribedBC.push_back(ldummy);
    m_bc->DistribedBC.push_back(ddummy);
    in.clear(); 
  }
  m_bc->fct_name = m_strFCTName;
  //----------------------------------------------------------------------
  bc_list.push_back(m_bc);
  //----------------------------------------------------------------------
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    GSPAddMember(m_gsp->base + ".bc");
  //----------------------------------------------------------------------
  FillTable();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2004 OK Implementation
11/2004 OK new Table
last modified:
**************************************************************************/
void CBoundaryConditions::OnBnClickedButtonRemove()
{
  // Determine selected BC properties
  POSITION p = m_LC_BC.GetFirstSelectedItemPosition();
  int nSelRow = m_LC_BC.GetNextSelectedItem(p);
  CString l_strPCSType = m_LC_BC.GetItemText(nSelRow,0);
  CString l_strGEOType = m_LC_BC.GetItemText(nSelRow,1);
  CString l_strGEOName = m_LC_BC.GetItemText(nSelRow,2);
  // Remove selected BC
  m_bc = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if((m_bc->pcs_type_name.compare(l_strPCSType)==0)\
     &&(m_bc->geo_type_name.compare(l_strGEOType)==0)\
     &&(m_bc->geo_name.compare(l_strGEOName)==0))
    {
      break;
    }
    ++p_bc;
  }
  if(m_bc) {
    delete m_bc;
    bc_list.remove(m_bc);
  }
  // Refill table
  FillTable();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
07/2005 OK 2nd version
**************************************************************************/
void CBoundaryConditions::OnBnClickedButtonCreateGroup()
{
  if(m_strPCSTypeName.IsEmpty()) {
    AfxMessageBox("No PCS type selected");
    return;
  }
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = PCSGet((string)m_strPCSTypeName);
  CFEMesh* m_msh = FEMGet((string)m_strPCSTypeName);
  //----------------------------------------------------------------------
  // Delete BC group
  CBoundaryConditionsGroup *m_bc_group = NULL;
  BCGroupDelete((string)m_strPCSTypeName,(string)m_strPVName);
  //----------------------------------------------------------------------
/*
  CBoundaryConditionsGroup *m_bc_group = NULL;
  CBoundaryConditionsGroup *this_bc_group = NULL;
  this_bc_group = this_bc_group->Get((string)m_strPCSTypeName);
  list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
  while(p_bc_group!=bc_group_list.end()) {
    m_bc_group = *p_bc_group;
    if(m_bc_group==this_bc_group) 
      delete  m_bc_group;
      bc_group_list.remove(m_bc_group);
    if(bc_group_list.size()==0)
      break;
    ++p_bc_group;
  }
*/
  //----------------------------------------------------------------------
  // Create BC group
  if(m_pcs&&m_msh){
    m_bc_group = new CBoundaryConditionsGroup();
    m_bc_group->pcs_type_name = (string)m_strPCSTypeName;
    m_bc_group->pcs_pv_name = (string)m_strPVName;
    m_bc_group->m_msh = FEMGet((string)m_strPCSTypeName);
    m_bc_group->Set(m_pcs,0,m_bc_group->pcs_pv_name);
    bc_group_list.push_back(m_bc_group);
  }
  //----------------------------------------------------------------------
/*
  CRFProcess* m_pcs = PCSGet((string)m_strPCSTypeName); //OK
  if(m_pcs){
    m_bc_group_new->pcs_type_name = m_pcs->pcs_type_name; //OK
    m_bc_group_new->pcs_pv_name = m_pcs->pcs_primary_function_name[0]; //OK ToDo
    m_bc_group_new->m_msh = m_pcs->m_msh;
    m_bc_group_new->Set(0);
    bc_group_list.push_back(m_bc_group_new);
  }
*/
  else{
    AfxMessageBox("No PCS selected");
  }
  //----------------------------------------------------------------------
  // Draw BC data
CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
{
//WW  if(m_bc_group->group_vector.size()>0){
  if(m_pcs->bc_node_value.size()>0)
    theApp->g_graphics_modeless_dlg->m_iDisplayBC = true;
  
  else{
    theApp->g_graphics_modeless_dlg->m_iDisplayBC = false;
  }
  theApp->g_graphics_modeless_dlg->OnInitDialog();
}
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
01/2005 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnBnClickedButtonWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    BCWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
01/2005 OK Implementation
**************************************************************************/
void CBoundaryConditions::OnBnClickedButtonWriteTEC()
{
  CBoundaryConditionsGroup* m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
  while(p_bc_group!=bc_group_list.end()) {
    m_bc_group = *p_bc_group;
    m_bc_group->WriteTecplot(); //OK41
    ++p_bc_group;
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
01/2005 OK Implementation
10/2008 OK node type
**************************************************************************/
void CBoundaryConditions::FillTable()
{
  CRect rect;
  long listip = 0;
  CString m_strItem;
  // Table configuration
  m_LC_BC.GetWindowRect(&rect);
  m_LC_BC.SetTextBkColor(RGB(153,153,255));
  m_LC_BC.SetTextColor(RGB(0,0,255));
   // pause the mouse with highlight or you can select it
   // so this will be the hot item.
  m_LC_BC.EnableTrackingToolTips();
  m_LC_BC.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES);
  LV_ITEM lvitem;
  lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
  // Insert head line
  m_LC_BC.DeleteColumn(0);
  m_LC_BC.InsertColumn(0,"PCS Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC_BC.DeleteColumn(1);
  m_LC_BC.InsertColumn(1,"GEO Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC_BC.DeleteColumn(2);
  m_LC_BC.InsertColumn(2,"GEO Object",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC_BC.DeleteColumn(3);
  m_LC_BC.InsertColumn(3,"DIS Type",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC_BC.DeleteColumn(4);
  m_LC_BC.InsertColumn(4,"DIS Object",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC_BC.DeleteColumn(5);
  m_LC_BC.InsertColumn(5,"FCT",LVCFMT_LEFT,rect.Width()*1/6,0);
  // Insert data
  m_LC_BC.DeleteAllItems();
  list<CBoundaryCondition*>::const_iterator p_bc;
  p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_bc->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC_BC.InsertItem(&lvitem);
    m_LC_BC.SetItemText(listip,1,m_bc->geo_type_name.c_str());
    m_LC_BC.SetItemText(listip,2,m_bc->geo_name.c_str());
    m_LC_BC.SetItemText(listip,3,m_bc->dis_type_name.c_str());
    switch(m_bc->dis_type_name[0]) 
    {
      case 'C': // Constant
        m_strItem.Format("%g",m_bc->geo_node_value);
        break;
      case 'L': // Linear
        m_strItem.Format("%i",(int)m_bc->PointsHaveDistribedBC.size());
        break;
      case 'N': //OK4801 Node
        m_strItem.Format("%g",m_bc->geo_node_value);
        break;
    }
    m_LC_BC.SetItemText(listip,4,m_strItem);
    m_LC_BC.SetItemText(listip,5,m_bc->fct_name.c_str());
    ++p_bc;
    listip++;
  }
}

void CBoundaryConditions::OnOK()
{
  CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// Table
// untill 4.1.01

////////////////////////////////////////////////////////////////////////////
// Remove



