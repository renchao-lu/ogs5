// pcs_dlg_new.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "pcs_dlg_new.h"

#include "rf_pcs.h"
#include "gs_project.h"
#include "rf_out_new.h"
#include "rf_tim_new.h"
#include "rfmat_cp.h"
// MSHLib
#include "msh_lib.h"

// CDialogPCS dialog

IMPLEMENT_DYNAMIC(CDialogPCS, CDialog)
CDialogPCS::CDialogPCS(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPCS::IDD, pParent)
{
}

CDialogPCS::~CDialogPCS()
{
}

void CDialogPCS::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_H, m_CBProcessH);
    DDX_Control(pDX, IDC_COMBO_PCS_T, m_CBProcessT);
    DDX_Control(pDX, IDC_COMBO_PCS_M, m_CBProcessM);
    DDX_Control(pDX, IDC_COMBO_PCS_C, m_CBProcessC);
    DDX_Text(pDX, IDC_EDIT_PROJECT_NAME, m_strProjectName);
    DDX_Text(pDX, IDC_EDIT_PROBLEM_TYPE, m_strPCSProblemType);
    DDX_Control(pDX, IDC_LIST_PCS, m_LB_PCS);
}

BEGIN_MESSAGE_MAP(CDialogPCS, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_H, OnCbnSelchangeComboH)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_T, OnCbnSelchangeComboT)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_M, OnCbnSelchangeComboM)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_C, OnCbnSelchangeComboC)
    //ON_BN_CLICKED(IDC_BUTTON_PCS_READ, OnBnClickedRead)
    ON_BN_CLICKED(IDC_BUTTON_PCS_WRITE, OnBnClickedWrite)
    ON_BN_CLICKED(IDC_BUTTON_PCS_CREATE, OnBnClickedCreate)
    ON_BN_CLICKED(IDC_BUTTON_PCS, OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
END_MESSAGE_MAP()

// CDialogPCS message handlers

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
last modification: 
**************************************************************************/
BOOL CDialogPCS::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  int i;
  CRFProcess* m_pcs = NULL;
  int no_processes = (int)pcs_vector.size();
//OK  pcs_type_name_vector.clear();
  //----------------------------------------------------------------------
  m_strProjectName = (CString)project_title.c_str();
  //----------------------------------------------------------------------
  // PCS problem type
  string m_stringPCSProblemType = PCSProblemType();
  m_strPCSProblemType = m_stringPCSProblemType.c_str();
  //----------------------------------------------------------------------
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // PCS H types
  m_CBProcessH.ResetContent();
  m_CBProcessH.InitStorage(8,80);
  m_CBProcessH.AddString("NO_FLOW");
  m_CBProcessH.AddString("GROUNDWATER_FLOW");
  m_CBProcessH.AddString("LIQUID_FLOW");
  m_CBProcessH.AddString("AIR_FLOW");
  m_CBProcessH.AddString("TWO_PHASE_FLOW");
  m_CBProcessH.AddString("COMPONENTAL_TWO_PHASE_FLOW");
  m_CBProcessH.AddString("OVERLAND_FLOW");
  m_CBProcessH.AddString("RICHARDS_FLOW");
  m_CBProcessH.SetCurSel(0);
  //----------------------------------------------------------------------
  // PCS T types
  m_CBProcessT.ResetContent();
  m_CBProcessT.InitStorage(1,80);
  m_CBProcessT.AddString("NO_HEAT_TRANSPORT");
  m_CBProcessT.AddString("HEAT_TRANSPORT");
  m_CBProcessT.SetCurSel(0);
  //----------------------------------------------------------------------
  // PCS M types
  m_CBProcessM.ResetContent();
  m_CBProcessM.InitStorage(1,80);
  m_CBProcessM.AddString("NO_DEFORMATION");
  m_CBProcessM.AddString("DEFORMATION");
  m_CBProcessM.SetCurSel(0);
  //----------------------------------------------------------------------
  // PCS C types
  m_CBProcessC.ResetContent();
  m_CBProcessC.InitStorage(2,80);
  m_CBProcessC.AddString("NO_MASS_TRANSPORT");
  m_CBProcessC.AddString("MASS_TRANSPORT");
  m_CBProcessC.AddString("REACTIVE_MASS_TRANSPORT");
  m_CBProcessC.SetCurSel(0);
  //----------------------------------------------------------------------
  m_LB_PCS.ResetContent();
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    m_LB_PCS.AddString(m_pcs->pcs_type_name.c_str());
  }
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogPCS::OnCbnSelchangeComboH()
{
  m_CBProcessH.GetLBText(m_CBProcessH.GetCurSel(),m_strPCSName);
//OK  pcs_type_name_vector.push_back((string)m_strPCSName);
}

void CDialogPCS::OnCbnSelchangeComboT()
{
  m_CBProcessT.GetLBText(m_CBProcessT.GetCurSel(),m_strPCSName);
//OK  pcs_type_name_vector.push_back((string)m_strPCSName);
}

void CDialogPCS::OnCbnSelchangeComboM()
{
  m_CBProcessM.GetLBText(m_CBProcessM.GetCurSel(),m_strPCSName);
//OK  pcs_type_name_vector.push_back((string)m_strPCSName);
}

void CDialogPCS::OnCbnSelchangeComboC()
{
  m_CBProcessC.GetLBText(m_CBProcessC.GetCurSel(),m_strPCSName);
//OK  pcs_type_name_vector.push_back((string)m_strPCSName);
}

void CDialogPCS::OnBnClickedButtonAdd()
{
  m_LB_PCS.AddString(m_strPCSName);
}

void CDialogPCS::OnBnClickedButtonClear()
{
  m_LB_PCS.ResetContent();
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
05/2005 OK Implementation
06/2005 OK Control Panel
01/2006 OK MCP,MSP
**************************************************************************/
void CDialogPCS::OnBnClickedCreate()
{
  int j;
  //----------------------------------------------------------------------
  // GSP
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(!m_gsp){
    AfxMessageBox("Error: No MSH member");
    return;
  }
  //----------------------------------------------------------------------
  // Delete existing PCS
  PCSDelete();
  //----------------------------------------------------------------------
  // Check MSH
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strPCSName);
  if(!m_msh){
    AfxMessageBox("Error: no MSH data");
    return;
  }
  //----------------------------------------------------------------------
  ConfigSolverProperties();
  //----------------------------------------------------------------------
  // Create PCS
  for(int i=0;i<(int)m_LB_PCS.GetCount();i++){
    m_LB_PCS.GetText(i,m_strPCSName);
    m_pcs = new CRFProcess();
    pcs_vector.push_back(m_pcs);    
    m_pcs->pcs_number = i;
    m_pcs->pcs_type_number = i;
    m_pcs->pcs_type_name = m_strPCSName;
    // MCP ...............................................................
    if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
    {
      MCPDelete();  
      CompProperties* m_mcp = new CompProperties((int)cp_vec.size());
      m_mcp->diffusion_model = 1;
      m_mcp->diffusion_model_values[0] = 1e-9;
      m_mcp->count_of_diffusion_model_values = 1;
      cp_vec.push_back(m_mcp);
      string mcp_base_type = m_gsp->base + ".mcp";
      GSPAddMember(mcp_base_type);
    }
    // MSP ...............................................................
    if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
    {
      MSPDelete();  
      SolidProp::CSolidProperties* m_msp = new SolidProp::CSolidProperties();
      msp_vector.push_back(m_msp);
      string msp_base_type = m_gsp->base + ".msp";
      GSPAddMember(msp_base_type);
    }
    //....................................................................
    m_pcs->Config();
    m_pcs->PCSSetIC_USER = NULL;
    m_pcs->Create();
    m_pcs->num_type_name = "NEW"; //OK
    m_gsp = GSPGetMember("msh");
    string pcs_base_type = m_gsp->base + ".pcs";
    GSPAddMember(pcs_base_type);
    //NUM ................................................................
    CNumerics* m_num = NULL;
    m_num = NUMGet(m_pcs->pcs_type_name);
    if(!m_num){
      m_pcs->m_num = new CNumerics(m_pcs->pcs_type_name);
      m_pcs->m_num->pcs_type_name = m_pcs->pcs_type_name;
      num_vector.push_back(m_pcs->m_num);
      string num_base_type = m_gsp->base + ".num";
      GSPAddMember(num_base_type);
    }
    // TIM ...............................................................
    CTimeDiscretization* m_tim = NULL;
    m_tim = TIMGet(m_pcs->pcs_type_name);
    if(!m_tim){
      m_tim = new CTimeDiscretization();
      m_tim->pcs_type_name = m_pcs->pcs_type_name;
      m_tim->time_step_vector.push_back(1.0);
      time_vector.push_back(m_tim);
      string tim_base_type = m_gsp->base + ".tim";
      GSPAddMember(tim_base_type);
    }
    // OUT ...............................................................
    COutput* m_out = NULL;
    m_out = OUTGet(m_pcs->pcs_type_name);
    if(!m_out){
      m_out = new COutput();
      m_out->pcs_type_name = m_pcs->pcs_type_name; //OK
      for(j=0;j<m_pcs->pcs_number_of_primary_nvals;j++){
        m_out->nod_value_vector.push_back(m_pcs->pcs_primary_function_name[j]);
      }
/*OK
      for(j=0;j<m_pcs->pcs_number_of_secondary_nvals;j++)
      {
        m_out->nod_value_vector.push_back(m_pcs->pcs_secondary_function_name[j]);
      }
*/
      out_vector.push_back(m_out);
      string out_base_type = m_gsp->base + ".out";
      GSPAddMember(out_base_type);
    }
    //....................................................................
  }
  //----------------------------------------------------------------------
  // Display selected processes in PCSView
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->OnInitDialog();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void CDialogPCS::OnBnClickedWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    PCSWrite(m_gsp->base);
  else
    AfxMessageBox("Error: No GSP data");
}
