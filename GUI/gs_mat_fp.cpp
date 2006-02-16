// gs_mat_fp.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_mat_fp.h"

#include "GeoSysDoc.h"

// FEMLib
#include ".\gs_mat_fp.h"

// CMaterialFluidDensity dialog

IMPLEMENT_DYNAMIC(CMaterialFluidDensity, CDialog)
CMaterialFluidDensity::CMaterialFluidDensity(CWnd* pParent /*=NULL*/)
	: CDialog(CMaterialFluidDensity::IDD, pParent)
    , m_dXmin(0)
    , m_dXmax(0)
    , m_dDensityMin(0)
    , m_dDensityMax(0)
    , m_dDensityRef(0)
    , m_dCompressibility(0)
    , m_dExpansivityC(0)
    , m_dExpansivityT(0)
    , m_dYmin(0)
    , m_dYmax(0)
{
  m_iFluidPhase = 1;
  m_iType = 0;
}

CMaterialFluidDensity::~CMaterialFluidDensity()
{
}

void CMaterialFluidDensity::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MAT_FP_DENSITY, m_LBFluidPhases);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_7, m_dXmin);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_8, m_dXmax);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_11, m_dDensityMin);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_12, m_dDensityMax);
    DDX_Radio(pDX, IDC_RADIO_MAT_FP_DENSITY1, m_iType);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_1, m_dDensityRef);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_2, m_dCompressibility);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_3, m_dExpansivityC);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_4, m_dExpansivityT);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_9, m_dYmin);
    DDX_Text(pDX, IDC_EDIT_MAT_FP_DENSITY_10, m_dYmax);
}


BEGIN_MESSAGE_MAP(CMaterialFluidDensity, CDialog)
    ON_BN_CLICKED(ID_BUTTON_MAT_FP_CALC, OnButtonMatFPCalc)
    ON_BN_CLICKED(IDOK, OnOk)
    ON_LBN_SELCHANGE(IDC_LIST_MAT_FP_DENSITY, OnSelchangeListMatFPDensity)
END_MESSAGE_MAP()


// CMaterialFluidDensity message handlers

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
BOOL CMaterialFluidDensity::OnInitDialog() 
{
/*OK
  fp=NULL;
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // FP Phases
  int i;
  m_LBFluidPhases.ResetContent();
  for(i=0;i<GetRFProcessNumPhases();i++) {
    sprintf(char_string,"%i",i+1);
    m_strPhaseNumber = char_string;
    m_LBFluidPhases.AddString(m_strPhaseNumber);
    sprintf(name_fluid_properties_phase,"%s%i",name_fluid_properties,i+1);
    fp=NULL;
    Get(name_fluid_properties_phase);
  }
*/
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CMaterialFluidDensity::OnSelchangeListMatFPDensity()
{
/*OK
  int nSel = m_LBFluidPhases.GetCurSel();
  m_iFluidPhase = nSel+1;
  sprintf(name_fluid_properties_phase,"%s%i",name_fluid_properties,m_iFluidPhase);
  Get(name_fluid_properties_phase);
*/
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CMaterialFluidDensity::OnButtonMatFPCalc()
{
/*OK
  UpdateData(TRUE);
  sprintf(name_fluid_properties_phase,"%s%i",name_fluid_properties,m_iFluidPhase);
  Set(name_fluid_properties_phase);
  Calc();
*/
/*
  fp=NULL;
  fp = GetFluidPropertiesObject(name_fluid_properties_phase,fp);
  if (fp) {
    values[0] = m_dXmin;
    values[1] = m_dYmin;
    m_dDensityMin  = MATCalcFluidDensity(m_iFluidPhase-1,0,gp,0.0,1,values);
    values[0] = m_dXmax;
    values[1] = m_dYmax;
    m_dDensityMax  = MATCalcFluidDensity(m_iFluidPhase-1,0,gp,0.0,1,values);
  }  
*/
  UpdateData(FALSE);
}

void CMaterialFluidDensity::Calc()
{
/*OK
  fp=NULL;
  double values[2];
  double gp[3];
  gp[0]=0.0, gp[1]=0.0, gp[2]=0.0;

  sprintf(name_fluid_properties_phase,"%s%i",name_fluid_properties,m_iFluidPhase);
  fp=NULL;
  fp = GetFluidPropertiesObject(name_fluid_properties_phase,fp);
  if (fp) {
    values[0] = m_dXmin;
    values[1] = m_dYmin;
    m_dDensityMin  = MATCalcFluidDensity(m_iFluidPhase-1,0,gp,0.0,1,values);
    values[0] = m_dXmax;
    values[1] = m_dYmax;
    m_dDensityMax  = MATCalcFluidDensity(m_iFluidPhase-1,0,gp,0.0,1,values);
  }  
*/
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CMaterialFluidDensity::OnOk()
{
/*OK
  sprintf(name_fluid_properties_phase,"%s%i",name_fluid_properties,m_iFluidPhase);
  Set(name_fluid_properties_phase);
  OnOK();
*/
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CMaterialFluidDensity::Set(char* name)
{
    name=name;//TK
/*OK
  fp=NULL;
  UpdateData(TRUE);
  fp = GetFluidPropertiesObject(name,fp);
  if (fp) {
    switch (m_iType) {
      case 0:
        fp->density_model = 0;
        fp->rho_0 = m_dDensityRef;
        break;
      case 1:
        fp->density_model = 1;
        fp->rho_0 = m_dDensityRef;
        fp->drho_dp = m_dCompressibility;
        break;
      case 2:
        fp->density_model = 10;
        fp->rho_0 = m_dDensityRef;
        fp->drho_dC = m_dExpansivityC;
        break;
      case 3:
        fp->density_model = 12;
        fp->rho_0 = m_dDensityRef;
        fp->drho_dT = m_dExpansivityT;
        fp->T_0 = 293.0; //m_dTemperatureRef;
        break;
      case 4:
        fp->density_model = 11;
        fp->rho_0 = m_dDensityRef;
        fp->drho_dC = m_dExpansivityC;
        fp->drho_dT = m_dExpansivityT;
        break;
      case 5:
        fp->density_model = 13;
        break;
      case 7:
        fp->density_model = 3;
        fp->rho_0 = m_dDensityRef;
        fp->drho_dC = m_dExpansivityC;
        break;
      case 8:
        fp->density_model = 14;
        fp->rho_0 = m_dDensityRef;
        fp->T_0 = 293.0; //m_dTemperatureRef;
        //fp->p_0 = 101325; //m_dPressureRef;
        fp->drho_dC = m_dExpansivityC;
        fp->drho_dT = m_dExpansivityT;
        break;
      default:
        fp->density_model = 0;
        fp->rho_0 = m_dDensityRef;
        break;
    }
  }  
  else 
    AfxMessageBox("No MAT-FP data");
*/
  UpdateData(FALSE);
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CMaterialFluidDensity::Get(char* name)
{
    name=name;//TK
/*OK
  fp=NULL;
  fp = GetFluidPropertiesObject(name,fp);
  if (fp) {
    switch (fp->density_model) {
      case 0:
        m_iType = 0;
        m_dDensityRef = fp->rho_0;
        break;
      case 1:
        m_iType = 1;
        m_dDensityRef = fp->rho_0;
        m_dCompressibility = fp->drho_dp;
        break;
      case 10:
        m_iType = 2;
        m_dDensityRef = fp->rho_0;
        m_dExpansivityC = fp->drho_dC;
        break;
      case 12:
        m_iType = 3;
        m_dDensityRef = fp->rho_0;
        m_dExpansivityT = fp->drho_dT;
        //m_dTemperatureRef = 293.0;
        break;
      case 11:
        m_iType = 4;
        m_dDensityRef = fp->rho_0;
        m_dExpansivityC = fp->drho_dC;
        m_dExpansivityT = fp->drho_dT;
        break;
      case 13:
        m_iType = 5;
        break;
      case 3:
        m_iType = 7;
        m_dDensityRef = fp->rho_0;
        m_dExpansivityC = fp->drho_dC;
        break;
      case 14:
        m_iType = 8;
        m_dDensityRef = fp->rho_0;
        //m_dTemperatureRef = 293.0;
        //m_dPressureRef = 101325.0;
        m_dExpansivityC = fp->drho_dC;
        m_dExpansivityT = fp->drho_dT;
        break;
      default:
        m_iType = 0;
        m_dDensityRef = fp->rho_0;
        break;
    }
  }  
  else 
    AfxMessageBox("No MAT-FP data");
*/
  UpdateData(FALSE);
}

