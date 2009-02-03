// sim_dlg.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "sim_dlg.h"
// FEMLib
#include "rf_pcs.h"
#include "gs_project.h"
#include "rf_tim_new.h"
#include "rf_out_new.h"
#include "rf_num_new.h"
#include "rf_ic_new.h"
#include "rf_bc_new.h"
#include "rf_st_new.h"
#include "rf_mfp_new.h"
#include "rf_msp_new.h"
#include "rf_mmp_new.h"
#include "rfmat_cp.h"
// GEOLib
#include "geo_vol.h"
#include ".\sim_dlg.h"
// MSHLib

// CSimulator dialog

IMPLEMENT_DYNAMIC(CSimulator, CDialog)
CSimulator::CSimulator(CWnd* pParent /*=NULL*/)
	: CDialog(CSimulator::IDD, pParent)
    , m_strProjectTitle(_T(""))
    , m_strProblemType(_T(""))
    , m_bCheckGEO(FALSE)
    , m_iNoGEO(0)
    , m_bSimulatorReady(FALSE)
{
  m_bCheckMSH = FALSE;
  m_bCheckPCS = FALSE;
  m_bCheckNUM = FALSE;
  m_bCheckTIM = FALSE;
  m_bCheckOUT = FALSE;
  m_bCheckIC  = FALSE;
  m_bCheckBC  = FALSE;
  m_bCheckST  = FALSE;
  m_bCheckMFP = FALSE;
  m_bCheckMSP = FALSE;
  m_bCheckMMP = FALSE;
  m_bCheckMCP = FALSE;
  m_iNoGEO = 0;
  m_iNoMSH = 0;
  m_iNoPCS = 0;
  m_iNoNUM = 0;
  m_iNoTIM = 0;
  m_iNoOUT = 0;
  m_iNoIC = 0;
  m_iNoBC = 0;
  m_iNoST = 0;
  m_iNoMFP = 0;
  m_iNoMSP = 0;
  m_iNoMMP = 0;
  m_iNoMCP = 0;
}

CSimulator::~CSimulator()
{
}

void CSimulator::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_PROJECT_NAME, m_strProjectTitle);
    DDX_Text(pDX, IDC_EDIT_PROBLEM_TYPE, m_strProblemType);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR1, m_strGEOFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR2, m_strMSHFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR3, m_strPCSFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR4, m_strNUMFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR5, m_strTIMFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR6, m_strOUTFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR7, m_strICFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR8, m_strBCFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR9, m_strSTFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR10, m_strMFPFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR11, m_strMSPFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR12, m_strMMPFile);
    DDX_Text(pDX, IDC_EDIT_SIMULATOR13, m_strMCPFile);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR1, m_bCheckGEO);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR2, m_bCheckMSH);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR3, m_bCheckPCS);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR4, m_bCheckNUM);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR5, m_bCheckTIM);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR6, m_bCheckOUT);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR7, m_bCheckIC);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR8, m_bCheckBC);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR9, m_bCheckST);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR10, m_bCheckMFP);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR11, m_bCheckMSP);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR12, m_bCheckMMP);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR13, m_bCheckMCP);
    DDX_Text(pDX, IDC_EDIT_NO1, m_iNoGEO);
    DDX_Text(pDX, IDC_EDIT_NO2, m_iNoMSH);
    DDX_Text(pDX, IDC_EDIT_NO3, m_iNoPCS);
    DDX_Text(pDX, IDC_EDIT_NO4, m_iNoNUM);
    DDX_Text(pDX, IDC_EDIT_NO5, m_iNoTIM);
    DDX_Text(pDX, IDC_EDIT_NO6, m_iNoOUT);
    DDX_Text(pDX, IDC_EDIT_NO7, m_iNoIC);
    DDX_Text(pDX, IDC_EDIT_NO8, m_iNoBC);
    DDX_Text(pDX, IDC_EDIT_NO9, m_iNoST);
    DDX_Text(pDX, IDC_EDIT_NO10, m_iNoMFP);
    DDX_Text(pDX, IDC_EDIT_NO11, m_iNoMSP);
    DDX_Text(pDX, IDC_EDIT_NO12, m_iNoMMP);
    DDX_Text(pDX, IDC_EDIT_NO13, m_iNoMCP);
    DDX_Check(pDX, IDC_CHECK_SIMULATOR_READY, m_bSimulatorReady);
    DDX_Control(pDX, IDC_LIST_PCS_SIM, m_LB_PCS);
}

BEGIN_MESSAGE_MAP(CSimulator, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_RUN_SIMULATION, OnBnClickedButtonRunSimulation)
    ON_LBN_SELCHANGE(IDC_LIST_PCS_SIM, OnLbnSelchangeListPCS)
END_MESSAGE_MAP()

// CSimulator message handlers

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
last modification: 
12/2008 NW Add .msh
**************************************************************************/
BOOL CSimulator::OnInitDialog() 
{
  m_strProjectTitle = (CString)project_title.c_str();
  string problem_type = PCSProblemType();
  m_strProblemType = problem_type.c_str();
  CGSProject* m_gsp = NULL;
  string path_base_type("No file");
  int i;
  int no_gsp_member = (int)gsp_vector.size();
  for(i=0;i<no_gsp_member;i++){
    m_gsp = gsp_vector[i];
    path_base_type = m_gsp->path + m_gsp->base + "." + m_gsp->type; 
    if(m_gsp->type=="gli"){
      m_bCheckGEO = TRUE;
      m_strGEOFile = path_base_type.c_str();
      m_iNoGEO = (int)volume_vector.size();
    }
    if(m_gsp->type=="rfi"){
      m_bCheckMSH = TRUE;
      m_strMSHFile = path_base_type.c_str();
      m_iNoMSH = (long)ElListSize();
    } else if(m_gsp->type=="msh"){
      m_bCheckMSH = TRUE;
      m_strMSHFile = path_base_type.c_str();
      m_iNoMSH = (int)fem_msh_vector.size(); // Number of mesh
    }

    if(m_gsp->type=="pcs"){
      m_bCheckPCS = TRUE;
      m_strPCSFile = path_base_type.c_str();
      m_iNoPCS = (int)pcs_vector.size();
    }
    if(m_gsp->type=="num"){
      m_bCheckNUM = TRUE;
      m_strNUMFile = path_base_type.c_str();
      m_iNoNUM = (int)num_vector.size();
    }
    if(m_gsp->type=="tim"){
      m_bCheckTIM = TRUE;
      m_strTIMFile = path_base_type.c_str();
      m_iNoTIM = (int)time_vector.size();
    }
    if(m_gsp->type=="out"){
      m_bCheckOUT = TRUE;
      m_strOUTFile = path_base_type.c_str();
      m_iNoOUT = (int)out_vector.size();
    }
    if(m_gsp->type=="ic"){
      m_bCheckIC  = TRUE;
      m_strICFile = path_base_type.c_str();
      m_iNoIC = (int)ic_vector.size();
    }
    if(m_gsp->type=="bc"){
      m_bCheckBC  = TRUE;
      m_strBCFile = path_base_type.c_str();
      m_iNoBC = (int)bc_list.size();
    }
    if(m_gsp->type=="st"){
      m_bCheckST  = TRUE;
      m_strSTFile = path_base_type.c_str();
      m_iNoST = (int)st_vector.size();
    }
    if(m_gsp->type=="mfp"){
      m_bCheckMFP = TRUE;
      m_strMFPFile = path_base_type.c_str();
      m_iNoMFP = (int)mfp_vector.size();
    }
    if(m_gsp->type=="msp"){
      m_bCheckMSP = TRUE;
      m_strMSPFile = path_base_type.c_str();
      m_iNoMSP = (int)msp_vector.size();
    }
    if(m_gsp->type=="mmp"){
      m_bCheckMMP = TRUE;
      m_strMMPFile = path_base_type.c_str();
      m_iNoMMP = (int)mmp_vector.size();
    }
    if(m_gsp->type=="mcp"){
      m_bCheckMCP = TRUE;
      m_strMCPFile = path_base_type.c_str();
      m_iNoMCP = (int)cp_vec.size();
    }
  }
  //----------------------------------------------------------------------
  GetDlgItem(IDC_BUTTON_RUN_SIMULATION)->EnableWindow(FALSE);
  m_bSimulatorReady = GSPSimulatorReady();
  if(m_bSimulatorReady)
    GetDlgItem(IDC_BUTTON_RUN_SIMULATION)->EnableWindow(TRUE);
  //----------------------------------------------------------------------
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  m_LB_PCS.ResetContent();
  CRFProcess* m_pcs = NULL;
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_LB_PCS.AddString(m_pcs->pcs_type_name.c_str());
    if(m_pcs->selected)
      m_LB_PCS.SetSel(i,TRUE);
  }
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2005 OK Implementation
last modification: 
**************************************************************************/
void CSimulator::OnLbnSelchangeListPCS()
{
  int i = m_LB_PCS.GetSel(m_LB_PCS.GetCurSel());
  CString m_strPCSName;
  m_LB_PCS.GetText(m_LB_PCS.GetCurSel(),m_strPCSName);
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSName);
  if(m_pcs){
    if(i==0)
      m_pcs->selected = false;
    else if(i==1)
      m_pcs->selected = true;
  }
/*
  CString m_strPCSName;
  m_LB_PCS.GetText(m_LB_PCS.GetCurSel(),m_strPCSName);
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSName);
  if(m_pcs){
    if(m_pcs->selected){
      m_pcs->selected = false;
      //m_LB_PCS.SetSel(nSel,FALSE);
    }
    else{
      m_pcs->selected = true;
      //m_LB_PCS.SetSel(nSel,TRUE);
    }
  }
  OnInitDialog();
*/
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2005 OK Implementation
last modification: 
**************************************************************************/
void CSimulator::OnBnClickedButtonRunSimulation()
{
}
