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
  this->m_dDIS_CONSTANT_Value = 0.0;
  this->m_dDIS_GRADIENT_Gradient = 0.0;
  this->m_dDIS_GRADIENT_RefX = 0.0;
  this->m_dDIS_GRADIENT_RefY = 0.0;
}

CDialogInitialConditions::~CDialogInitialConditions()
{
}

void CDialogInitialConditions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
    DDX_Text(pDX, IDC_EDIT_DIS_CONSTANT, this->m_dDIS_CONSTANT_Value);
    DDX_Text(pDX, IDC_EDIT_DIS_GRADIENT_GRADIENT, this->m_dDIS_GRADIENT_Gradient);
    DDX_Text(pDX, IDC_EDIT_DIS_GRADIENT_REF_X, this->m_dDIS_GRADIENT_RefX);
    DDX_Text(pDX, IDC_EDIT_DIS_GRADIENT_REF_Y, this->m_dDIS_GRADIENT_RefY);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Control(pDX, IDC_LIST, m_LC);
    DDX_Control(pDX, IDC_COMBO_PCS_PV_NAME, m_CB_PV);
}

BEGIN_MESSAGE_MAP(CDialogInitialConditions, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnCbnSelchangeComboPCSType)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_GROUP, OnBnClickedButtonCreateGroup)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_READ_IC, OnBnClickedButtonRead)
    ON_BN_CLICKED(IDC_BUTTON_READ_DATA, OnBnClickedButtonReadData)
    ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_TEC, OnBnClickedButtonWriteTEC)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_PV_NAME, OnCbnSelchangeComboPVName)
    ON_BN_CLICKED(IDC_RADIO_DIS_CONSTANT, &CDialogInitialConditions::OnBnClickedRadioDisConstant)
    ON_BN_CLICKED(IDC_RADIO_DIS_GRADIENT, &CDialogInitialConditions::OnBnClickedRadioDisGradient)
    ON_BN_CLICKED(IDC_BUTTON_IC_DLG_CLOSE, &CDialogInitialConditions::OnBnClickedButtonIcDlgClose)
    ON_NOTIFY(NM_CLICK, IDC_LIST, &CDialogInitialConditions::OnNMClickList)
END_MESSAGE_MAP()


// CDialogInitialConditions message handlers

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation based on CDialogBoundaryConditions
12/2008 NW Change DIS types
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
  CButton *pBtn;
  pBtn = (CButton *)GetDlgItem(IDC_RADIO_DIS_CONSTANT);
  pBtn->SetCheck(BST_CHECKED);
  this->OnBnClickedRadioDisConstant();
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
    m_nod_val->node_value = this->m_dDIS_CONSTANT_Value;
    m_obj->node_value_vector.push_back(m_nod_val);
  } else if (m_obj->dis_type_name.compare("GRADIENT")==0) {
    m_obj->gradient_ref_depth_gradient = this->m_dDIS_GRADIENT_Gradient;
    m_obj->gradient_ref_depth = this->m_dDIS_GRADIENT_RefX;
    m_obj->gradient_ref_depth_value = this->m_dDIS_GRADIENT_RefY;
  } else if (m_obj->dis_type_name.compare("DATA_BASE")==0) {
    AfxMessageBox("DIS_TYPE: DATA_BASE has not been implemented yet."); return;
  }
  //......................................................................
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
05/2005 OK Implementation
06/2009 OK Update
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonRead()
{
  CGSProject* m_gsp = GSPGetMember("pcs");
  if(m_gsp)
  {
    ICDelete(); 
    ICRead(m_gsp->path+m_gsp->base);
    GSPAddMember(m_gsp->base + ".ic");
    OnInitDialog();
  }
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
    //if(m_obj->node_value_vector.size()>0)
    //  m_strItem.Format("%g",m_obj->node_value_vector[0]->node_value);
    if(m_obj->dis_type_name.compare("CONSTANT")==0){
      m_strItem.Format("%g",m_obj->node_value_vector[0]->node_value);
    } else if (m_obj->dis_type_name.compare("GRADIENT")==0) {
      m_strItem.Format("F(Z)=%g*(%g-Z)+%g",m_obj->gradient_ref_depth_gradient,m_obj->gradient_ref_depth,m_obj->gradient_ref_depth_value);
    } else if (m_obj->dis_type_name.compare("DATA_BASE")==0) {
    }
    m_LC.SetItemText(listip,4,m_strItem);
    listip++;
  }
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
**************************************************************************/
void CDialogInitialConditions::OnOK()
{
  CDialog::OnOK();
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
12/2008 NW Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedRadioDisConstant()
{
  //Switch ON CONSTANT, OFF others
  this->m_RB_DISType_CONSTANT = BST_CHECKED;
  this->m_RB_DISType_GRADIENT = BST_UNCHECKED;
  this->m_RB_DISType_DATA_BASE = BST_UNCHECKED;
  this->m_strDISTypeName = "CONSTANT";
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_CONSTANT))->EnableWindow(TRUE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_GRADIENT))->EnableWindow(FALSE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_REF_X))->EnableWindow(FALSE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_REF_Y))->EnableWindow(FALSE);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
12/2008 NW Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedRadioDisGradient()
{
  //Switch ON GRADIENT, OFF others
  this->m_RB_DISType_CONSTANT = BST_UNCHECKED;
  this->m_RB_DISType_GRADIENT = BST_CHECKED;
  this->m_RB_DISType_DATA_BASE = BST_UNCHECKED;
  this->m_strDISTypeName = "GRADIENT";
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_CONSTANT))->EnableWindow(FALSE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_GRADIENT))->EnableWindow(TRUE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_REF_X))->EnableWindow(TRUE);
  ((CEdit*)GetDlgItem(IDC_EDIT_DIS_GRADIENT_REF_Y))->EnableWindow(TRUE);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
12/2008 NW Implementation
**************************************************************************/
void CDialogInitialConditions::OnBnClickedButtonIcDlgClose()
{
  this->EndDialog(0);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
12/2008 NW Implementation
**************************************************************************/
void CDialogInitialConditions::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
  *pResult = 0;

  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int nSelRow = m_LC.GetNextSelectedItem(p);
  if (nSelRow>=0) {
    m_obj = ic_vector[nSelRow];
    CString tmp_str;
    //PCS NAME
    for (int i = 0; i < m_CB_PCSType.GetCount(); i++) {
      m_CB_PCSType.GetLBText(i,tmp_str);
      if ((string)tmp_str == m_obj->pcs_type_name)
        m_CB_PCSType.SetCurSel(i);
    }
    //PRIMARY VARIABLE
    for (int i = 0; i < m_CB_PV.GetCount(); i++) {
      m_CB_PV.GetLBText(i,tmp_str);
      if ((string)tmp_str == m_obj->pcs_pv_name)
        m_CB_PV.SetCurSel(i);
    }
    //GEO TYPE
    for (int i = 0; i < m_CB_GEOType.GetCount(); i++) {
      m_CB_GEOType.GetLBText(i,tmp_str);
      if ((string)tmp_str == m_obj->geo_type_name)
        m_CB_GEOType.SetCurSel(i);
    }
    //GEO NAME
    for (int i = 0; i < m_LB_GEO.GetCount(); i++) {
      m_LB_GEO.GetText(i,tmp_str);
      if ((string)tmp_str == m_obj->geo_name)
        m_LB_GEO.SetCurSel(i);
    }
    //DIS TYPE
    if (m_obj->dis_type_name == "CONSTANT") {
      OnBnClickedRadioDisConstant();
      ((CButton*)GetDlgItem(IDC_RADIO_DIS_CONSTANT))->SetCheck(BST_CHECKED);
      ((CButton*)GetDlgItem(IDC_RADIO_DIS_GRADIENT))->SetCheck(BST_UNCHECKED);
      this->m_dDIS_CONSTANT_Value = m_obj->node_value_vector[0]->node_value;
    } else if (m_obj->dis_type_name == "GRADIENT") {
      OnBnClickedRadioDisGradient();
      ((CButton*)GetDlgItem(IDC_RADIO_DIS_CONSTANT))->SetCheck(BST_UNCHECKED);
      ((CButton*)GetDlgItem(IDC_RADIO_DIS_GRADIENT))->SetCheck(BST_CHECKED);
      this->m_dDIS_GRADIENT_Gradient = m_obj->gradient_ref_depth_gradient;
      this->m_dDIS_GRADIENT_RefX = m_obj->gradient_ref_depth;
      this->m_dDIS_GRADIENT_RefY = m_obj->gradient_ref_depth_value;
    }
  }

  UpdateData(FALSE);
  this->Invalidate();
}
