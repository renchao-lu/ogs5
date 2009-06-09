// tim_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "tim_dlg.h"

#include "rf_pcs.h"
#include "gs_project.h"
#include ".\tim_dlg.h"

// CDialogTimeDiscretization dialog

IMPLEMENT_DYNAMIC(CDialogTimeDiscretization, CDialog)
CDialogTimeDiscretization::CDialogTimeDiscretization(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogTimeDiscretization::IDD, pParent)
    , m_iNT(1)
    , m_dDT(1)
    , m_dTIMStart(0.0)
    , m_dTIMEnd(1.0)
{
  m_strTIMUnitName = "SECOND";
  m_iTIMType = 1;
  m_iTIMStepScheme = 0;
  m_dTIMStep = 1.0;
}

CDialogTimeDiscretization::~CDialogTimeDiscretization()
{
}

void CDialogTimeDiscretization::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_LIST, m_LC);
    DDX_Control(pDX, IDC_LIST_TIM, m_LB_TIM);
    DDX_Radio(pDX, IDC_RADIO2, m_iTIMType);
    DDX_Text(pDX, IDC_EDIT_NT, m_iNT);
    DDX_Text(pDX, IDC_EDIT_DT, m_dDT);
    DDX_Text(pDX, IDC_EDIT_TIME_START, m_dTIMStart);
    DDX_Text(pDX, IDC_EDIT_TIME_END, m_dTIMEnd);
    DDX_Control(pDX, IDC_COMBO_TIM_UNIT, m_CB_TIM_UNIT);
    DDX_Radio(pDX, IDC_RADIO3, m_iTIMStepScheme);
    DDX_Text(pDX, IDC_EDIT_TIME_STEP, m_dTIMStep);
}

BEGIN_MESSAGE_MAP(CDialogTimeDiscretization, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnCbnSelchangeComboPCSType)
    ON_BN_CLICKED(ID_ADD_TIME_STEPS, OnBnClickedAddTimeSteps)
    ON_BN_CLICKED(IDC_BUTTON_TIM_WRITE, OnBnClickedButtonTIMWrite)
    ON_BN_CLICKED(ID_REMOVE_TIME_STEPS, OnBnClickedRemoveTimeSteps)
    ON_BN_CLICKED(IDC_BUTTON_TIM_REMOVE, OnBnClickedButtonTIMRemove)
    ON_BN_CLICKED(IDC_BUTTON_TIM_CREATE, OnBnClickedButtonTIMCreate)
    ON_CBN_SELCHANGE(IDC_COMBO_TIM_UNIT, OnCbnSelchangeComboTimUnit)
END_MESSAGE_MAP()


// CDialogTimeDiscretization message handlers
/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation based on CDialogBoundaryConditions
**************************************************************************/
BOOL CDialogTimeDiscretization::OnInitDialog() 
{
  int i,j;
  CString m_str;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // PCS types
  CRFProcess* m_pcs = NULL;
  m_CB_PCSType.ResetContent();
  m_LB_TIM.ResetContent();
  m_CB_PCSType.InitStorage(10,80);
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSTypeName = (CString)m_pcs->pcs_type_name.c_str();
    m_CB_PCSType.AddString(m_strPCSTypeName);
    m_CB_PCSType.SetCurSel(i);
    //....................................................................
    m_tim = TIMGet(m_pcs->pcs_type_name);
    if(m_tim)
    {
      m_dTIMStart = m_tim->time_start;
      m_dTIMEnd = m_tim->time_end;
      m_dTIMStep = m_tim->this_stepsize;
      m_LB_TIM.ResetContent();
      for(j=0;j<(int)m_tim->time_step_vector.size();j++)
      {
        m_str.Format("%g",m_tim->time_step_vector[j]);
        m_LB_TIM.AddString(m_str);
      }
    }
  }
  //----------------------------------------------------------------------
  // TIM units
  m_CB_TIM_UNIT.ResetContent();
  m_CB_TIM_UNIT.InitStorage(10,80);
  m_CB_TIM_UNIT.AddString("SECOND");
  m_CB_TIM_UNIT.AddString("MINUTE");
  m_CB_TIM_UNIT.AddString("HOUR");
  m_CB_TIM_UNIT.AddString("DAY");
  m_CB_TIM_UNIT.AddString("YEAR");
  m_CB_TIM_UNIT.SetCurSel(0);
  //----------------------------------------------------------------------
  // Table
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
  m_LC.InsertColumn(0,"PCS Type",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,"TIM Unit",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,"TIM Start",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(3);
  m_LC.InsertColumn(3,"TIM Type",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(4);
  m_LC.InsertColumn(4,"TIM Criteria",LVCFMT_LEFT,rect.Width()*1/5,0);
  // Insert data
  m_LC.DeleteAllItems(); //OK
  for(i=0;i<(int)time_vector.size();i++){
    m_obj = time_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_obj->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_LC.SetItemText(listip,1,m_obj->time_unit.c_str());
    m_str.Format("%e",m_obj->time_start);
    m_LC.SetItemText(listip,2,m_str);
    m_str.Format("%e",m_obj->time_end);
    m_LC.SetItemText(listip,3,m_str);
    m_LC.SetItemText(listip,4,m_obj->time_control_name.c_str());
    listip++;
  }
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  return TRUE;
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnCbnSelchangeComboPCSType()
{
  CString m_str;
  m_CB_PCSType.GetLBText(m_CB_PCSType.GetCurSel(),m_strPCSTypeName);
  m_obj = TIMGet((string)m_strPCSTypeName);
  m_LB_TIM.ResetContent();
  if(m_obj){
    for(int i=0;i<(int)m_obj->time_step_vector.size();i++){
      m_str.Format("%e",m_obj->time_step_vector[i]);
      m_LB_TIM.AddString(m_str);
    }
    m_dTIMStart = m_obj->time_start;
    m_dTIMEnd = m_obj->time_end;
  }
  UpdateData(FALSE);
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnBnClickedAddTimeSteps()
{
  UpdateData(TRUE);
  CString m_str;
  m_str.Format("%g",m_dDT);
  m_LB_TIM.AddString(m_str);
/*
  m_obj = TIMGet((string)m_strPCSTypeName);
  if(m_obj){
    for(int i=0;i<m_iNT;i++){
      m_obj->time_step_vector.push_back(m_dDT);
    }
  }
  else{
    AfxMessageBox("No TIM selected");
  }
  OnCbnSelchangeComboPCSType();
*/
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnBnClickedRemoveTimeSteps()
{
  int m_iSel = m_LB_TIM.GetCurSel();
  if(m_obj&&(m_iSel>-1)){
    m_obj->time_step_vector.erase((m_obj->time_step_vector.begin()+m_iSel));
  }
  else{
    AfxMessageBox("No TIM selected");
  }
  OnCbnSelchangeComboPCSType();
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnBnClickedButtonTIMWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    TIMWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnBnClickedButtonTIMCreate()
{
  double time,time_sum;
  CString m_str;
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  // Tests
  if(m_strPCSTypeName.IsEmpty()) {
    AfxMessageBox("No PCS type selected"); return; 
  }
  //----------------------------------------------------------------------
  TIMDelete((string)m_strPCSTypeName);
  m_obj = new CTimeDiscretization();
  m_obj->pcs_type_name = m_strPCSTypeName;
  m_obj->time_start = m_dTIMStart;
  m_obj->time_end = m_dTIMEnd;
  m_obj->time_unit = m_strTIMUnitName;
  //......................................................................
  switch(m_iTIMType){
    case 1:
      m_obj->time_control_name = "CRITERIA";
      break;
    case 0:
      m_obj->time_control_name = "FIXED";
      break;
  }
  //......................................................................
  if(m_iTIMType==0)
  {
    time_sum = 0.0;
    if(m_LB_TIM.GetCount()==0) //OK
    {
      m_str.Format("%g",1.0);
      m_LB_TIM.AddString(m_str);
    }
    for(int i=0;i<m_LB_TIM.GetCount();i++)
    {
      m_LB_TIM.GetText(i,m_str);
      time = strtod(m_str,NULL);
      while(time_sum<m_obj->time_end){
        m_obj->time_step_vector.push_back(time);
        time_sum += time;
      }
    }
  }
  //......................................................................
  switch(m_iTIMStepScheme)
  {
    case 0:
      m_obj->time_control_name = "PI_AUTO_STEP_SIZE";
      m_obj->tsize_ctrl_type = 2;
      m_obj->relative_error = 1.0;
      m_obj->absolute_error = 1e-12;
      m_obj->this_stepsize = m_dTIMStep;
      break;
    case 1:
      m_obj->time_control_name = "COURANT_MANIPULATE";
      break;
      m_obj->time_control_name = "NEUMANN";
    case 2:
      break;
  }
/*
time_control_name("STEP_SIZE_RESTRICTION")
time_control_name("ERROR_CONTROL_ADAPTIVE")
time_control_name("SELF_ADAPTIVE")
*/
  //......................................................................
  time_vector.push_back(m_obj);
  //----------------------------------------------------------------------
  UpdateList();
  OnInitDialog();
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::OnBnClickedButtonTIMRemove()
{
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int nSelRow = m_LC.GetNextSelectedItem(p);
  if(nSelRow>=0){
    m_obj = time_vector[nSelRow];
    delete m_obj;
    time_vector.erase((time_vector.begin()+nSelRow));
  }
  UpdateList();
  OnInitDialog();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CDialogTimeDiscretization::UpdateList()
{
  CString m_str;
  int listip = 0;
  m_LC.DeleteAllItems(); //OK
  //......................................................................
  // Insert data
  for(int i=0;i<(int)time_vector.size();i++){
    m_obj = time_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_obj->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_LC.SetItemText(listip,0,m_obj->pcs_type_name.c_str());
    m_LC.SetItemText(listip,1,m_obj->time_type_name.c_str());
    m_str.Format("%e",m_obj->time_start);
    m_LC.SetItemText(listip,2,m_str);
    m_str.Format("%e",m_obj->time_end);
    m_LC.SetItemText(listip,3,m_str);
    m_LC.SetItemText(listip,4,m_obj->time_control_name.c_str());
    listip++;
  }
}

void CDialogTimeDiscretization::OnCbnSelchangeComboTimUnit()
{
  m_CB_TIM_UNIT.GetLBText(m_CB_TIM_UNIT.GetCurSel(),m_strTIMUnitName);
}

void CDialogTimeDiscretization::OnOK()
{
  CDialog::OnOK();
}
