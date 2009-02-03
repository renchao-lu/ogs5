// gs_output.cpp : implementation file
//
/**************************************************************************
GeoSysGUI-File: 
Task: dialog to output
Programing:
11/2003 OK Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "gs_output.h"

// C++ STL
#include <vector>
#include <list>
using namespace std;
// GeoSys-FEM
#include "rf_pcs.h"
#include "rf_out_new.h"
#include "nodes.h"
#include "mathlib.h"
#include "rf_mmp_new.h"
#include "rf_fct.h"
#include "gs_project.h"
// GeoSys-GEO
#include "geo_pnt.h"
#include "geo_ply.h"
// GeoSys-GUI
#include "gs_mat_fp.h"

// COutputObservation dialog

IMPLEMENT_DYNAMIC(COutputObservation, CDialog)
COutputObservation::COutputObservation(CWnd* pParent /*=NULL*/)
	: CDialog(COutputObservation::IDD, pParent)
{
  m_dUmin = 0.0;
  m_dUmax = 0.0;
}

COutputObservation::~COutputObservation()
{
}

void COutputObservation::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_QUANTITIES, m_LBQuantities);
    DDX_Control(pDX, IDC_LIST_POINTS, m_LBPoints);
    DDX_Control(pDX, IDC_LIST_OBSERVATION_POINTS, m_LBObservationPoints);
    DDX_Text(pDX, IDC_EDIT_GRAF_1, m_dXmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_2, m_dXmax);
    DDX_Text(pDX, IDC_EDIT_GRAF_3, m_dYmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_4, m_dYmax);
    DDX_Text(pDX, IDC_EDIT_UMIN, m_dUmin);
    DDX_Text(pDX, IDC_EDIT_UMAX, m_dUmax);
}


BEGIN_MESSAGE_MAP(COutputObservation, CDialog)
    ON_BN_CLICKED(ID_ADD_OBSERVATION_POINT, OnBnAddObservationPoint)
    ON_LBN_SELCHANGE(IDC_LIST_QUANTITIES, OnSelchangeListQuantities)
END_MESSAGE_MAP()


// COutputObservation message handlers
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
10/2004 OK LOAD_PATH_ANALYSIS
**************************************************************************/
BOOL COutputObservation::OnInitDialog() 
{
  long i;
  int j;
  //----------------------------------------------------------------------
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // Quantities
  m_LBQuantities.ResetContent();
  CRFProcess* m_process = NULL;
  for(i=0;i<(int)pcs_vector.size();i++){
    m_process = pcs_vector[i];
    for(j=0;j<m_process->number_of_nvals;j++){
      if(m_process->pcs_nval_data[j].timelevel==1){
        m_strQuantityName = (CString)m_process->pcs_nval_data[j].name;
        m_LBQuantities.AddString(m_strQuantityName);
      }
    }
    if(m_process->pcs_type_name.find("DEFORMATION")!=string::npos)
      m_LBQuantities.AddString("LOAD_PATH_ANALYSIS");
  }
  //----------------------------------------------------------------------
  // GEO Points
  long point_vector_size;
  vector<CGLPoint*> gs_points_vector;
  gs_points_vector = GetPointsVector();
  point_vector_size = (long)gs_points_vector.size();
  for(i=0;i<point_vector_size;i++)
  {  
    //m_iType = gs_points_vector[i]->type;  
    m_LBPoints.AddString((CString)gs_points_vector[i]->name.c_str());
  }
  //----------------------------------------------------------------------
  // OUT Points
  COutput *m_out = NULL;
  for(i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
    if(m_out->geo_type_name.find("POINT")!=string::npos)
      m_LBObservationPoints.AddString((CString)m_out->geo_name.c_str());
  }
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void COutputObservation::OnSelchangeListQuantities()
{
  long i;
  double value;
  double m_dMin1 = 1.e+19;
  double m_dMax1 = -1.e+19;
  // Identify quantity name
  if (m_LBQuantities.GetCurSel()>=0) {
    m_LBQuantities.GetText(m_LBQuantities.GetCurSel(),m_strQuantityName);
  }
  // Determine quantity nodal value index
  strcpy(char_string,m_strQuantityName);
  m_iQuantityIndex = PCSGetNODValueIndex(char_string,1);
  if(m_iQuantityIndex<0) return;
  // Calculate min / max values
  for(i=0;i<NodeListLength;i++) {
    value = GetNodeVal(i,m_iQuantityIndex);
    if(value<m_dMin1) m_dMin1 = value;
    if(value>m_dMax1) m_dMax1 = value;
  }
  m_dYmin = m_dMin1;
  m_dYmax = m_dMax1;
  m_dUmin = m_dMin1;
  m_dUmax = m_dMax1;
  // Refresh control elements
  UpdateData(FALSE);

}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void COutputObservation::OnBnAddObservationPoint()
{
  m_LBPoints.GetText(m_LBPoints.GetCurSel(),m_strNamePoint);

  if(m_strNamePoint.IsEmpty()) {
    AfxMessageBox("No point selected !");
    return;
  }
  //m_LBObservationPoints.AddString(m_strNamePoint);
  SetData();
  // OUT Points
  m_LBObservationPoints.ResetContent();
/*OK_MMP
  OUTPUT *op = NULL;
  list_op_init();
  while ((op = (OUTPUT *) get_list_op_next()) != NULL) {
    if(op->type==1) {
      sprintf(char_string,"%g,%g,%g",op->x[0],op->y[0],op->z[0]);
      m_strNamePoint = char_string;
      m_LBObservationPoints.AddString(m_strNamePoint);
    }
  }
*/
}


/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void COutputObservation::SetData() 
{
/*OK_MMP
  OUTPUT *op = NULL;
  CGLPoint *m_point=NULL;
  char char_string[256];
//  CGeoSysDoc* m_pDoc = GetDocument();

  m_LBPoints.GetText(m_LBPoints.GetCurSel(),m_strNamePoint);
  strcpy(char_string,m_strNamePoint);
  m_point = m_point->GetByName(char_string);
  if(m_point) {
      //node=GetNodeNumberClose(m_point->x,m_point->y,m_point->z);
      op = create_output("GeoSys");
      op->type = 1;
      strcpy(char_string,m_strFileNameBase);
      set_op_file_name(op,char_string);
      strcpy(char_string,m_strFilePath);
      set_op_path_name(op,char_string);
      init_output_coor(op, 1);
      set_op_x(op,0,m_point->x);
      set_op_y(op,0,m_point->y);
      set_op_z(op,0,m_point->z);
      insert_output_list(op);
    }
    else {
    }
*/
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void COutputObservation::OnOK() 
{
  UpdateData(TRUE);
  //SetData();
  CDialog::OnOK();
}



// COUTObservationPolylines dialog

IMPLEMENT_DYNAMIC(COUTObservationPolylines, CDialog)
COUTObservationPolylines::COUTObservationPolylines(CWnd* pParent /*=NULL*/)
	: CDialog(COUTObservationPolylines::IDD, pParent)
    , m_dXmin(0)
    , m_dXmax(0)
    , m_dYmin(0)
    , m_dYmax(0)
{
}

COUTObservationPolylines::~COUTObservationPolylines()
{
}

void COUTObservationPolylines::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_POLYLINES, m_LBPolylines);
    DDX_Control(pDX, IDC_LIST_QUANTITIES2, m_LBQuantities);
    DDX_Control(pDX, IDC_LIST_OUT_POLYLINES, m_LBOUTPolylines);
    DDX_Text(pDX, IDC_EDIT_GRAF_1, m_dXmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_2, m_dXmax);
    DDX_Text(pDX, IDC_EDIT_GRAF_3, m_dYmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_4, m_dYmax);
}


BEGIN_MESSAGE_MAP(COUTObservationPolylines, CDialog)
    ON_LBN_SELCHANGE(IDC_LIST_QUANTITIES2, OnSelchangeListQuantities)
    ON_LBN_SELCHANGE(IDC_LIST_OUT_POLYLINES, OnSelchangeListObservationPolylines)
    ON_BN_CLICKED(ID_BUTTON_OUT_PLY, OnBnClickedButtonOUTPolylines)
    ON_LBN_SELCHANGE(IDC_LIST_POLYLINES, OnLbnSelchangeListPolylines)
END_MESSAGE_MAP()


// COUTObservationPolylines message handlers

BOOL COUTObservationPolylines::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // Quantities
  m_LBQuantities.ResetContent();
  CRFProcess* m_process = NULL;
  int i,j;
  for(i=0;i<(int)pcs_vector.size();i++){
    m_process = pcs_vector[i];
    for(j=0;j<m_process->number_of_nvals;j++){
      if(m_process->pcs_nval_data[j].timelevel==1){
        m_LBQuantities.AddString((CString)m_process->pcs_nval_data[j].name);
      }
    }
  }
  //----------------------------------------------------------------------
  // GEO Polylines
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();// CC 08/2005
  CGLPolyline *gs_polyline = NULL;
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
  //----------------------------------------------------------------------
  // OUT Polylines
  COutput *m_out = NULL;
  for(i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
    if(m_out->geo_type==1)
      m_LBOUTPolylines.AddString((CString)m_out->geo_name.c_str());
  }
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void COUTObservationPolylines::OnSelchangeListQuantities()
{
  long i;
  double value;
  double m_dXMin1 = 1.e+19;
  double m_dXMax1 = -1.e+19;
  double m_dYMin1 = 1.e+19;
  double m_dYMax1 = -1.e+19;
  // Identify quantity name
  if (m_LBQuantities.GetCurSel()>=0) {
    m_LBQuantities.GetText(m_LBQuantities.GetCurSel(),m_strQuantityName);
  }
  // Determine quantity nodal value index
  strcpy(c_string,m_strQuantityName);
  m_iQuantityIndex = PCSGetNODValueIndex(c_string,1);
  if(m_iQuantityIndex<0) return;
  // Calculate min / max values
  for(i=0;i<NodeListLength;i++) {
    value = GetNodeVal(i,m_iQuantityIndex);
    if(value<m_dYMin1) m_dYMin1 = value;
    if(value>m_dYMax1) m_dYMax1 = value;
    value = GetNodeX(i);
    if(value<m_dXMin1) m_dXMin1 = value;
    if(value>m_dXMax1) m_dXMax1 = value;
  }
  m_dXmin = m_dXMin1;
  m_dXmax = m_dXMax1;
  m_dYmin = m_dYMin1;
  m_dYmax = m_dYMax1;
  // Refresh control elements
  UpdateData(FALSE);
}

void COUTObservationPolylines::OnLbnSelchangeListPolylines()
{
  m_LBPolylines.GetText(m_LBPolylines.GetCurSel(),m_strPolylineName);
}

void COUTObservationPolylines::OnSelchangeListObservationPolylines()
{
  long i;
  double x1,y1,z1;
  double d12[3],dist12;
  long *nodes;
  long number_of_nodes;
  double dist_max;
  dist_max = -1e19;
  // Identify polyline name
  if (m_LBOUTPolylines.GetCurSel()>=0) {
    m_LBOUTPolylines.GetText(m_LBOUTPolylines.GetCurSel(),m_strPolylineName);
  }
  CGLPolyline *m_polyline=NULL;
  m_polyline = GEOGetPLYByName((string)m_strPolylineName);//CC
  strcpy(c_string,m_strPolylineName);
  nodes = MSHGetNodesClose(&number_of_nodes,m_polyline);//CC
  // distance from first polyline point
  for(i=0;i<number_of_nodes;i++) {
    x1 = GetNodeX(nodes[i]);
    y1 = GetNodeY(nodes[i]);
    z1 = GetNodeZ(nodes[i]);
    d12[0]=m_polyline->point_vector[0]->x-x1;
    d12[1]=m_polyline->point_vector[0]->y-y1;
    d12[2]=m_polyline->point_vector[0]->z-z1;
    dist12=MBtrgVec(d12,3);
    if(dist12>dist_max) dist_max = dist12;
  }
  m_dXmin = 0.0;
  m_dXmax = dist_max;
  // Refresh control elements
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
**************************************************************************/
void COUTObservationPolylines::OnBnClickedButtonOUTPolylines()
{
//  m_LBOUTPolylines.GetText(m_LBOUTPolylines.GetCurSel(),m_strPolylineName);
  if(m_strPolylineName.IsEmpty()) {
    AfxMessageBox("No point selected !");
    return;
  }
  //
  m_polyline = GEOGetPLYByName((string)m_strPolylineName);//CC
/*OK_MMP
  if(m_polyline) {
    op = create_output("GeoSys");
    //op->type = 1;
    //set_op_file_name(op,m_strFileNameBase);
    //set_op_path_name(op,char_string);
    strcpy(op->polyline_name,m_strPolylineName);
    insert_output_list(op);
  }
  //
  list_op_init();
  m_LBOUTPolylines.ResetContent();
  while ((op = (OUTPUT *) get_list_op_next()) != NULL) {
    m_LBOUTPolylines.AddString((CString)op->polyline_name);
  }
*/
  // Refresh data elements
  UpdateData(TRUE);
}



// F:\GeoSys3908OK\GUI\gs_output.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_output.h"
#include ".\gs_output.h"


// COUTMaterialProperties dialog

IMPLEMENT_DYNAMIC(COUTMaterialProperties, CDialog)
COUTMaterialProperties::COUTMaterialProperties(CWnd* pParent /*=NULL*/)
	: CDialog(COUTMaterialProperties::IDD, pParent)
{
  m_mmp = NULL;
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_dUmin = 0.0;
  m_dUmax = 1.0;
  m_iMATType = -1; //
}

COUTMaterialProperties::~COUTMaterialProperties()
{
}

void COUTMaterialProperties::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_GRAF_1, m_dXmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_2, m_dXmax);
    DDX_Text(pDX, IDC_EDIT_GRAF_3, m_dYmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_4, m_dYmax);
    DDX_Text(pDX, IDC_EDIT_UMIN, m_dUmin);
    DDX_Text(pDX, IDC_EDIT_UMAX, m_dUmax);
    DDX_Control(pDX, IDC_COMBO_MMP, m_CB_MMP);
    DDX_Control(pDX, IDC_COMBO_MMP_GROUP, m_CB_MMP_Group);
    DDX_Control(pDX, IDC_COMBO_MFP, m_CB_MFP); //OK
    DDX_Control(pDX, IDC_COMBO_MFP_PROPERTY, m_CB_MFP_property); //OK
}

BEGIN_MESSAGE_MAP(COUTMaterialProperties, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_MMP, OnCbnSelchangeComboMMP)
    ON_BN_CLICKED(IDC_BUTTON_FCT_CREATE, OnBnClickedButtonFCTCreate)
    ON_BN_CLICKED(IDC_BUTTON_FCT_WRITE, OnBnClickedButtonFCTWrite)
    ON_CBN_SELCHANGE(IDC_COMBO_MMP_GROUP, OnCbnSelchangeComboMMPGroup)
    ON_BN_CLICKED(IDC_BUTTON_FCT_REFRESH, OnBnClickedButtonRefresh)
    ON_EN_CHANGE(IDC_EDIT_UMIN, OnEnChangeEditUmin)
    ON_EN_CHANGE(IDC_EDIT_UMAX, OnEnChangeEditUmax)
    ON_CBN_SELCHANGE(IDC_COMBO_MFP, OnCbnSelchangeComboMFP)
    ON_CBN_SELCHANGE(IDC_COMBO_MFP_PROPERTY, OnCbnSelchangeComboMFPProperty)
END_MESSAGE_MAP()

// COUTMaterialProperties message handlers

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
05/2005 OK MMP start
01/2009 OK MFP
**************************************************************************/
BOOL COUTMaterialProperties::OnInitDialog() 
{
  int i;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // MFP
  m_CB_MFP.ResetContent();
  for(i=0;i<(int)mfp_vector.size();i++){
    m_mfp = mfp_vector[i];
    m_CB_MFP.AddString(m_mfp->name.c_str());
  }
  m_CB_MFP.SetCurSel(0);
  //......................................................................
  m_CB_MFP_property.ResetContent();
  m_CB_MFP_property.AddString("DENSITY");
  m_CB_MFP_property.AddString("VISCOSITY");
  m_CB_MFP_property.AddString("HEAT_CAPACITY");
  m_CB_MFP_property.AddString("THERMAL_CONDUCTIVITY");
  m_CB_MFP_property.SetCurSel(0);
  //----------------------------------------------------------------------
  // MMP
  m_CB_MMP_Group.ResetContent();
  for(i=0;i<(int)mmp_vector.size();i++){
    m_mmp = mmp_vector[i];
    m_CB_MMP_Group.AddString(m_mmp->name.c_str());
  }
  m_CB_MMP_Group.SetCurSel(0);
  //......................................................................
  m_CB_MMP.ResetContent();
  m_CB_MMP.AddString("CAPILLARY_PRESSURE");
  m_CB_MMP.AddString("PERMEABILITY_SATURATION");
  m_CB_MMP.AddString("SATURATION_CAPILLARY_PRESSURE");
  m_CB_MMP.AddString("D_SATURATION_D_CAPILLARY_PRESSURE");
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnSelchangeListMatProperties()
{
  UpdateData(TRUE);
  // Identify MAT property name

  if(m_strMATPropertyName=="FLUID_DENSITY: PHASE 1")
  {
    m_iDisplayMATPropertiesType = 0;
    m_iFluidPhase = 1;
    CMaterialFluidDensity m_fluid_density;
    m_fluid_density.m_dXmin = m_dXmin;
    m_fluid_density.m_dXmax = m_dXmax;
    m_fluid_density.m_dYmin = m_dYmin;
    m_fluid_density.m_dYmax = m_dYmax;
    m_fluid_density.m_iFluidPhase = m_iFluidPhase;
    m_fluid_density.Calc();
    m_dUmin = m_fluid_density.m_dDensityMin;
    m_dUmax = m_fluid_density.m_dDensityMax;
    m_dYmin = m_dUmin;
    m_dYmax = m_dUmax;
    delete(m_fluid_density);
  }
  if(m_strMATPropertyName=="FLUID_DENSITY: PHASE 2")
  {
    m_iDisplayMATPropertiesType = 0;
    m_iFluidPhase = 2;
  }
  UpdateData(FALSE);
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnCbnSelchangeComboMMPGroup()
{
  m_CB_MMP_Group.GetLBText(m_CB_MMP_Group.GetCurSel(),m_strMATGroupName);
  m_mmp = MMPGet((string)m_strMATGroupName);
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
04/2007 NW Compilation
**************************************************************************/
void COUTMaterialProperties::OnCbnSelchangeComboMMP()
{
  int phase = 0;
  UpdateData(TRUE);
  m_CB_MMP.GetLBText(m_CB_MMP.GetCurSel(),m_strMATPropertyName);
  if(m_mmp){
    m_mmp->mode = 2;
    if(m_strMATPropertyName.Compare("CAPILLARY_PRESSURE")==0){
      m_dUmin = m_mmp->CapillaryPressureFunction(m_mmp->number,NULL,0.0,1,m_dXmax);
      m_dUmax = m_mmp->CapillaryPressureFunction(m_mmp->number,NULL,0.0,1,m_dXmin);
    }
    else if(m_strMATPropertyName.Compare("PERMEABILITY_SATURATION")==0){
      m_mmp->argument = m_dXmin;
      m_dUmin = m_mmp->PermeabilitySaturationFunction(m_mmp->number,NULL,0.0,phase);
      m_mmp->argument = m_dXmax;
      m_dUmax = m_mmp->PermeabilitySaturationFunction(m_mmp->number,NULL,0.0,phase);
    }
    else if(m_strMATPropertyName.Compare("SATURATION_CAPILLARY_PRESSURE")==0){
      m_mmp->argument = m_dXmin;
      m_dUmin = m_mmp->SaturationCapillaryPressureFunction(m_mmp->number,NULL,0.0,phase);
      m_mmp->argument = m_dXmax;
      m_dUmax = m_mmp->SaturationCapillaryPressureFunction(m_mmp->number,NULL,0.0,phase);
    }
    else if(m_strMATPropertyName.Compare("D_SATURATION_D_CAPILLARY_PRESSURE")==0){
	//04/2007 NW compilation
/*
   	  m_mmp->argument = m_dXmin;
      m_dUmin = m_mmp->SaturationPressureDependency(m_mmp->number,NULL,0.0);
      m_mmp->argument = m_dXmax;
      m_dUmax = m_mmp->SaturationPressureDependency(m_mmp->number,NULL,0.0);
*/
    }
  }
  m_iMATType = 0; //OK
  UpdateData(FALSE);
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnBnClickedButtonRefresh()
{
  switch(m_iMATType)
  {
    //--------------------------------------------------------------------
    case 0: //MMP
      OnCbnSelchangeComboMMP();
      break;
    //--------------------------------------------------------------------
    case 1: //MFP
      if(!m_mfp)
      {
        AfxMessageBox("Warning in COUTMaterialProperties: no MFP data");
        return;
      }
      // MFP property type
      switch(m_strMATGroupName[0])
      {
        case 'D': // Density
          break;
      }
      // MFP data type
      switch(m_mfp->density_model)
      {
        case 10: // FCT
          m_fct = FCTGet((string)m_strMATPropertyName);
          if(!m_fct)
            return;
          m_dXmin = m_fct->variable_data_vector[0][0];
          m_dXmax = m_fct->variable_data_vector[m_fct->matrix_dimension[0]-1][0];
          m_dUmin = m_fct->variable_data_vector[m_fct->matrix_dimension[0]+m_fct->matrix_dimension[1]][0];
          m_dUmax = m_fct->variable_data_vector[m_fct->matrix_dimension[0]+m_fct->matrix_dimension[1]+m_fct->matrix_dimension[0]-1][0];
      }
    //--------------------------------------------------------------------
      break;
    default:
      AfxMessageBox("Warning in COUTMaterialProperties: no MAT type selected");
  }
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnBnClickedButtonFCTCreate()
{
  int i;
  int ni = 100;
  double m_dDX = (m_dXmax-m_dXmin)/(double)ni;
  double* variable_data = NULL;
  CMediumProperties* m_mmp = NULL;
  CFunction* m_fct = NULL;
  if(m_strMATPropertyName=="CAPILLARY_PRESSURE"){
    m_fct = new CFunction();
    m_fct->type_name = "MATERIAL_FUNCTION";
    m_fct->geo_type_name = "POINT";
    m_fct->geo_name = "CAPILLARY_PRESSURE";
    m_fct->geo_name += "_MAT_GROUP_0";
    m_fct->variable_names_vector.push_back("SATURATION1");
    m_fct->variable_names_vector.push_back("CAPILLARY_PRESSURE");
    m_mmp = mmp_vector[0]; //ToDo
    m_mmp->mode = 2;
    for(i=0;i<ni;i++){
      variable_data = new double[2];
      variable_data[0] = m_dXmin + i*m_dDX;
      variable_data[1] = m_mmp->CapillaryPressureFunction(0,NULL,0.0,1,variable_data[0]);
      m_fct->variable_data_vector.push_back(variable_data);
    }
    fct_vector.push_back(m_fct);
  }
}

/**************************************************************************
GUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnBnClickedButtonFCTWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    FCTWrite(m_gsp->path + m_gsp->base);
}


void COUTMaterialProperties::OnEnChangeEditUmin()
{
  UpdateData(TRUE);
}

void COUTMaterialProperties::OnEnChangeEditUmax()
{
  UpdateData(TRUE);
}

/**************************************************************************
GUI-Method: 
01/2009 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnCbnSelchangeComboMFP()
{
  m_CB_MFP.GetLBText(m_CB_MFP.GetCurSel(),m_strMATPropertyName);
  m_mfp = MFPGet((string)m_strMATPropertyName);
  if(!m_mfp)
    AfxMessageBox("Warning in COUTMaterialProperties: no MFP data");
  m_iMATType = 1;
}

/**************************************************************************
GUI-Method: 
01/2009 OK Implementation
**************************************************************************/
void COUTMaterialProperties::OnCbnSelchangeComboMFPProperty()
{
  m_CB_MFP_property.GetLBText(m_CB_MFP_property.GetCurSel(),m_strMATGroupName);
  m_iMATType = 1;
}
