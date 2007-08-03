// num_dlg.cpp : implementation file
//
/**************************************************************************
07/2007 OK Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"
#include "num_dlg.h"
#include ".\num_dlg.h"

#include "gs_project.h"
#include "rf_num_new.h"

// CDialogNUM dialog

IMPLEMENT_DYNAMIC(CDialogNUM, CDialog)
CDialogNUM::CDialogNUM(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogNUM::IDD, pParent)
{
  m_LinearSolverType = 0;
  m_NonLinearSolverType = 0;
  m_WeightingType = 1;
  m_Preconditioner = true;
  m_ErrorTolerance = 1.e-009;
  m_MaxIterationLinear = 10000;
  m_MaxIterationsNonLinear = 10;
}

CDialogNUM::~CDialogNUM()
{
}

void CDialogNUM::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Radio(pDX, IDC_RADIO_METHOD, m_LinearSolverType);
    DDX_Radio(pDX, IDC_RADIO_METHOD_3, m_NonLinearSolverType);
    DDX_Radio(pDX, IDC_RADIO_METHOD_5, m_WeightingType);
    DDX_Control(pDX, IDC_LIST, m_LC);
    DDX_Check(pDX, IDC_CHECK_PRECOND, m_Preconditioner);
    DDX_Text(pDX, IDC_EDIT2, m_ErrorTolerance);
    DDX_Text(pDX, IDC_EDIT3, m_MaxIterationLinear);
    DDX_Text(pDX, IDC_EDIT4, m_MaxIterationsNonLinear);
    //---------------------------------------------------------------------------
    // PCS types
    //---------------------------------------------------------------------------
    CRFProcess* m_pcs = NULL;
    m_CB_PCSType.ResetContent();
    m_CB_PCSType.InitStorage(10,80);
    for(int i=0;i<(int)pcs_vector.size();i++){
        m_pcs = pcs_vector[i];
        m_strPCSTypeName = (CString)m_pcs->pcs_type_name.c_str();
        m_CB_PCSType.AddString(m_strPCSTypeName);
        m_CB_PCSType.SetCurSel(i);
    }
    FillTable();
}


BEGIN_MESSAGE_MAP(CDialogNUM, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
END_MESSAGE_MAP()


// CDialogNUM message handlers

void CDialogNUM::OnBnClickedButtonCreate()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  m_CB_PCSType.GetLBText(m_CB_PCSType.GetCurSel(),m_strPCSTypeName);
  //----------------------------------------------------------------------
  m_num = new CNumerics((string)m_strPCSTypeName); //OK
/*
  //......................................................................
  // Linear solver
  switch(m_LinearSolverType)
  {
    case 0: m_num->ls_method = 2;
      break;
  }
  m_num->ls_error_method = 5;
  m_num->ls_error_tolerance = m_ErrorTolerance;
  m_num->ls_max_iterations = m_MaxIterationLinear;
  if (m_Preconditioner) 
    m_num->ls_precond = 100;
  else 
    m_num->ls_precond = 0;
  m_num->ls_storage_method = 4;
  //......................................................................
  switch(m_WeightingType)
  {
    case 0: m_num->ls_theta = 0.0;
      break;
    case 1: m_num->ls_theta = 1.0;
      break;
    case 2: m_num->ls_theta = 0.5;
      break;
  }
*/
  //----------------------------------------------------------------------
  num_vector.push_back(m_num);
  //----------------------------------------------------------------------
  FillTable();
  UpdateData(FALSE);
  //----------------------------------------------------------------------
}

void CDialogNUM::OnBnClickedButtonWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    NUMWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CDialogNUM::FillTable()
{
  //---------------------------------------------------------------------------
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
  m_LC.InsertColumn(0,"PCS Type",LVCFMT_LEFT,rect.Width()*1/4,0);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,"Solver",LVCFMT_LEFT,rect.Width()*1/4,0);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,"NL Solver",LVCFMT_LEFT,rect.Width()*1/6,0);
  m_LC.DeleteColumn(3);
  m_LC.InsertColumn(3,"Weighting",LVCFMT_LEFT,rect.Width()*1/9,0);
  m_LC.DeleteColumn(4);
  m_LC.InsertColumn(4,"Precond",LVCFMT_LEFT,rect.Width()*1/9,0);
  m_LC.DeleteColumn(5);
  m_LC.InsertColumn(5,"GP",LVCFMT_LEFT,rect.Width()*1/9,0);
  // Insert data
  m_LC.DeleteAllItems(); //OK
  for(int i=0;i<(int)num_vector.size();i++){
    m_num = num_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_num->pcs_type_name.c_str();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_strItem.Format("%i %i %g %i",m_num->ls_method,m_num->ls_error_method,m_num->ls_error_tolerance,m_num->ls_max_iterations);
    m_LC.SetItemText(listip,1,m_strItem);
    m_LC.SetItemText(listip,2,m_num->nls_method_name.c_str());
    m_strItem.Format("%g",m_num->ls_theta);
    m_LC.SetItemText(listip,3,m_strItem);
    m_strItem.Format("%i",m_num->ls_precond);
    m_LC.SetItemText(listip,4,m_strItem);
    m_strItem.Format("%i",m_num->ele_gauss_points);
    m_LC.SetItemText(listip,5,m_strItem);
    listip++;
  }
}

void CDialogNUM::OnBnClickedButtonRemove()
{
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int nSelRow = m_LC.GetNextSelectedItem(p);
  if(nSelRow>=0){
    m_num = num_vector[nSelRow];
    delete m_num;
    num_vector.erase((num_vector.begin()+nSelRow));
  }
  FillTable();
}

void CDialogNUM::OnOK()
{
  CDialog::OnOK();
}