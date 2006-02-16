// pcs_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "pcs_dlg.h"
#include ".\pcs_dlg.h"
//
#include "loop_pcs.h"
extern void PCSCreate(void);
#include "rf_pcs.h"
#include "pcs_dm.h"
extern void Init_Linear_Elements();
extern void Init_Quadratic_Elements();
extern void ConfigRenumberProperties(void);
extern void Generate_Quadratic_Elements();
extern void RFConfigRenumber(void);
extern void RFPre_Model();
// FEMLib
#include "rf_ic_new.h"
#include "rf_tim_new.h"
#include "rf_out_new.h"
#include "rf_mfp_new.h"
#include "gs_project.h"

// CPCSDlg dialog

IMPLEMENT_DYNAMIC(CPCSDlg, CDialog)
CPCSDlg::CPCSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPCSDlg::IDD, pParent)
    , m_strProjectName(_T(""))
    , m_strPCSProblemType(_T(""))
{
}

CPCSDlg::~CPCSDlg()
{
}

void CPCSDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_H, m_CBProcessH);
    DDX_Control(pDX, IDC_COMBO_PCS_T, m_CBProcessT);
    DDX_Control(pDX, IDC_COMBO_PCS_M, m_CBProcessM);
    DDX_Control(pDX, IDC_COMBO_PCS_C, m_CBProcessC);
    DDX_Text(pDX, IDC_EDIT_PROJECT_NAME, m_strProjectName);
    DDX_Text(pDX, IDC_EDIT_PROBLEM_TYPE, m_strPCSProblemType);
}


BEGIN_MESSAGE_MAP(CPCSDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_PCS_CREATE, OnBnClickedCreate)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_H, OnCbnSelchangeComboH)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_T, OnCbnSelchangeComboT)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_M, OnCbnSelchangeComboM)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_C, OnCbnSelchangeComboC)
    ON_BN_CLICKED(IDC_BUTTON_PCS_READ, OnBnClickedPCSRead)
    ON_BN_CLICKED(IDC_BUTTON_PCS_WRITE, OnBnClickedPCSWrite)
END_MESSAGE_MAP()

// CPCSDlg message handlers
BOOL CPCSDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  int i;
  CRFProcess* m_pcs = NULL;
  int no_processes = (int)pcs_vector.size();
  pcs_type_name_vector.clear();
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
  m_CBProcessH.InitStorage(4,80);
  m_CBProcessH.AddString("NO_FLOW");
  m_CBProcessH.AddString("LIQUID_FLOW");
  m_CBProcessH.AddString("GAS_FLOW");
  m_CBProcessH.AddString("TWO_PHASE_FLOW");
  m_CBProcessH.AddString("COMPONENTAL_TWO_PHASE_FLOW");
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
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      default:
        AfxMessageBox("Error in LOPPreTimeLoop_PCS: no valid process !");
        break;
      case 'L': // Liquid flow
        m_CBProcessH.SetCurSel(1);
        m_CBProcessH.GetLBText(1,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'U': // Unconfined flow
        break;
      case 'G': // Gas flow
        m_CBProcessH.SetCurSel(2);
        m_CBProcessH.GetLBText(2,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'T': // Two-phase flow
        m_CBProcessH.SetCurSel(3);
        m_CBProcessH.GetLBText(3,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'C': // Componental flow
        m_CBProcessH.SetCurSel(4);
        m_CBProcessH.GetLBText(4,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'H': // heat transport
        m_CBProcessT.SetCurSel(1);
        m_CBProcessT.GetLBText(1,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'M': // Mass transport
        m_CBProcessC.SetCurSel(1);
        m_CBProcessC.GetLBText(1,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'R': // Reactive mass transport
        m_CBProcessC.SetCurSel(2);
        m_CBProcessC.GetLBText(2,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
      case 'D': // deformation
        m_CBProcessM.SetCurSel(1);
        m_CBProcessM.GetLBText(1,pcs_type_name);
        pcs_type_name_vector.push_back((string)pcs_type_name);
        break;
    }
  }
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

void CPCSDlg::OnCbnSelchangeComboH()
{
  m_CBProcessH.GetLBText(m_CBProcessH.GetCurSel(),pcs_type_name);
  pcs_type_name_vector.push_back((string)pcs_type_name);
}

void CPCSDlg::OnCbnSelchangeComboT()
{
  m_CBProcessT.GetLBText(m_CBProcessT.GetCurSel(),pcs_type_name);
  pcs_type_name_vector.push_back((string)pcs_type_name);
}

void CPCSDlg::OnCbnSelchangeComboM()
{
  m_CBProcessM.GetLBText(m_CBProcessM.GetCurSel(),pcs_type_name);
  pcs_type_name_vector.push_back((string)pcs_type_name);
}

void CPCSDlg::OnCbnSelchangeComboC()
{
  m_CBProcessC.GetLBText(m_CBProcessC.GetCurSel(),pcs_type_name);
  pcs_type_name_vector.push_back((string)pcs_type_name);
}

void CPCSDlg::OnBnClickedCreate()
{
  //CNumerics* m_num = NULL;
  CTimeDiscretization* m_tim = NULL;
  COutput* m_out = NULL;
  CInitialCondition* m_ic = NULL;
  CFluidProperties* m_mfp = NULL;
  CGSProject* m_gsp = NULL;
  //----------------------------------------------------------------------
  // Delete existing PCS
  int i;
  string pcs_name_dm;
  no_pcs = (int)pcs_vector.size();
  for(i=0;i<no_pcs;i++){
    m_pcs = pcs_vector[i];
    delete m_pcs;
    m_pcs = NULL;
  }
  pcs_vector.clear();
  //----------------------------------------------------------------------
  // Remove double names
  // pcs_type_name_vector.?
  //----------------------------------------------------------------------
  // Create selected PCS
  int no_dlg_pcs = (int)pcs_type_name_vector.size();
  for(i=0;i<no_dlg_pcs;i++){
    m_pcs = new CRFProcess();
    m_pcs->pcs_number = i;
    m_pcs->pcs_type_name = pcs_type_name_vector[i];
    //OK41
    m_gsp = GSPGetMember("rfi");
    //NUM ................................................................
    //m_num = m_num->Get(m_pcs->pcs_type_name);
    //if(!m_num)
    m_pcs->m_num = new CNumerics();
    m_pcs->m_num->pcs_type_name = m_pcs->pcs_type_name;
    num_vector.push_back(m_pcs->m_num);
    string num_base_type = m_gsp->base + ".num";
    GSPAddMember(num_base_type);
    // TIM ...............................................................
    m_tim = new CTimeDiscretization();
    m_tim->pcs_name = m_pcs->pcs_type_name;
    time_vector.push_back(m_tim);
    string tim_base_type = m_gsp->base + ".tim";
    GSPAddMember(tim_base_type);
    // OUT ...............................................................
    m_out = new COutput();
    m_out->nod_value_vector.push_back("PRESSURE1");
    out_vector.push_back(m_out);
    string out_base_type = m_gsp->base + ".out";
    GSPAddMember(out_base_type);
    // IC  ...............................................................
    m_ic = new CInitialCondition();
    m_ic->pcs_type_name = "PRESSURE1";
    ic_vector.push_back(m_ic);
    string ic_base_type = m_gsp->base + ".ic";
    GSPAddMember(ic_base_type);
    // MFP ...............................................................
    m_mfp = new CFluidProperties();
    mfp_vector.push_back(m_mfp);
    string mfp_base_type = m_gsp->base + ".mfp";
    GSPAddMember(mfp_base_type);
    //OK41
    pcs_vector.push_back(m_pcs);
    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos) 
    {
       RelocateDeformationProcess(m_pcs);
       m_pcs->pcs_number = i;
    }
  }
  //----------------------------------------------------------------------
  // Topological operation
  for(i=0;i<no_dlg_pcs;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos){
      Generate_Quadratic_Elements();
      Init_Quadratic_Elements();
      ConfigRenumberProperties();
      RFConfigRenumber();
      Init_Linear_Elements();
      break;
    }
  }
  //----------------------------------------------------------------------
  RFPre_Model(); //OK41
  PCSCreate();
  //----------------------------------------------------------------------
  // Project title
  UpdateData(TRUE);
  project_title = m_strProjectName;
  //----------------------------------------------------------------------
  // PCS problem type
  string m_stringPCSProblemType = PCSProblemType();
  m_strPCSProblemType = m_stringPCSProblemType.c_str();
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  OnCancel();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void CPCSDlg::OnBnClickedPCSRead()
{
/*
  CFileDialog fileDlg(TRUE,"pcs",NULL,OFN_ENABLESIZING," PCS Files (*.pcs)|*.pcs|| ");
  if (fileDlg.DoModal()==IDOK) {   
    CString m_strFileNamePathBaseExt = fileDlg.GetPathName();
    int pos = m_strFileNamePathBaseExt.ReverseFind('.');
    CString m_strFileNameBase = m_strFileNamePathBaseExt.Left(pos);
    PCSRead((string)m_strFileNameBase);
    OnInitDialog();
  }
  else
    OnCancel();
*/
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void CPCSDlg::OnBnClickedPCSWrite()
{
/*
  CFileDialog fileDlg(TRUE,"pcs",NULL,OFN_ENABLESIZING," PCS Files (*.pcs)|*.pcs|| ");
  if (fileDlg.DoModal()==IDOK) {   
    CString m_strFileNamePathBaseExt = fileDlg.GetPathName();
    int pos = m_strFileNamePathBaseExt.ReverseFind('.');
    CString m_strFileNameBase = m_strFileNamePathBaseExt.Left(pos);
    PCSWrite((string)m_strFileNameBase);
    NUMWrite((string)m_strFileNameBase);
  }
  else
    OnCancel();
*/
}
