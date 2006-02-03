// gs_graphics.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "geosys.h"
#include "MainFrm.h" //OK
#include "gs_graphics.h"
// MSHLib
#include "msh_nodes_rfi.h"
#include "fem_ele.h"
#include "msh_lib.h"
// GeoSys-FEM
#include "nodes.h"
#include "elements.h"
#include "rf_pcs.h"
#include "rfadt.h"
#include "rf_out_new.h"
extern long GetNodeNumberClose (double xcoor, double ycoor, double zcoor); // et al. to GeoLib
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "par_ddc.h"
// GeoSys-GUI
#include "GeoSysOUT2DView.h"
#include "gs_polyline.h"
// GeoSys-GeoLib
#include "geo_sfc.h"
#include "geo_ply.h"
#include "geo_pnt.h"

#include "gs_project.h"

#include "mathlib.h"
#include <math.h> // amod
#include ".\gs_graphics.h"
extern double sign (double a1, double a2);
extern double dabs (double a);
extern int md1(int a,int b);
static double time_last;

#include <gdiplus.h>
using namespace Gdiplus;

// For stess path visualization
#include "rf_msp_new.h"
#include "pcs_dm.h"

using SolidProp::CSolidProperties;

//field quantity
vector<CQuantity*> quantity_vector;

//global variable
 CGLPoint *gl_point=NULL;
 CGLPolyline *gl_polyline=NULL;
 Surface *gl_surface=NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_ELE_DRAW 10000
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CGraphics 

CGraphics::CGraphics(CWnd* pParent /*=NULL*/)
	: CDialog(CGraphics::IDD, pParent)
    , m_bDisplayTINs(FALSE)
    , m_bDisplayVOL(false)
    , m_bDisplayPLYPoints(FALSE)
    , m_bDisplayPLYLines(FALSE)
    , m_bDisplayNOD(FALSE)
    , m_bDisplayMSHDoubleElements(FALSE)
    , m_bDisplayMaterialGroups(FALSE)
    , m_bDisplayST(FALSE)
    , m_showaxis(FALSE)
    , m_levels(25)
    , m_Displaylegend(FALSE)
    , m_bDisplayPLYLabels(FALSE)
    , m_bDisplaySFCLabels(FALSE)
    , m_bDisplayVOLLabels(FALSE)
    , m_bDisplayMSHLabels(FALSE)
    , m_bDisplayMSHHex(FALSE)
    , m_bDisplayBMP(FALSE)
    , m_lNoElements2Draw(0)
    , m_bDisplayMATGroupsMSH(FALSE)
    , m_bDisplayDOM(FALSE)
    , m_bDisplaySFC_MAT(FALSE)
    , m_bDisplaySFCValues(FALSE)
{
	//{{AFX_DATA_INIT(CGraphics)
	m_dXmin = 0.0;
	m_dXmax = 1.0;
	m_dYmin = 0.0;
	m_dYmax = 1.0;
    m_dUmin = 0.;
    m_dUmax = 1.;
	//}}AFX_DATA_INIT
    // Pens
     // solid
	BlackSolidPen.CreatePen(PS_SOLID, 0, RGB(0,0,0));
	RedSolidPen.CreatePen(PS_SOLID, 0, RGB(255,0,0));
	GreenSolidPen.CreatePen(PS_SOLID, 0, RGB(0,255,0));
	BlueSolidPen.CreatePen(PS_SOLID, 0, RGB(0,0,255));
	BlueVeloPen.CreatePen(PS_SOLID | PS_ENDCAP_ROUND, 0, RGB(0,0,255));
	ZyanSolidPen.CreatePen(PS_SOLID, 0, RGB(0,255,255));
	MagentaSolidPen.CreatePen(PS_SOLID, 0, RGB(255,0,255));
	YellowSolidPen.CreatePen(PS_SOLID, 0, RGB(255,255,0));
	DarkYellowSolidPen.CreatePen(PS_SOLID, 0, RGB(128,128,0));
 
     // bold
	RedBoldPen.CreatePen(PS_SOLID, 0, RGB(255,0,0));
	GreenBoldPen.CreatePen(PS_SOLID, 0, RGB(0,255,0));
	BlueBoldPen.CreatePen(PS_SOLID, 0, RGB(0,0,255));
	BlackBoldPen.CreatePen(PS_SOLID, 0, RGB(0,0,0));
	BlackSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(0,0,0));
	HighlightingPen.CreatePen(PS_SOLID,3,RGB(255,204,0));
     // more bold
	RedSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(255,0,0));
	GreenSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(0,255,0));
	BlueSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(0,0,255));
	DarkYellowSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(128,128,0));
	DarkPinkSolidBoldPen.CreatePen(PS_SOLID, 3, RGB(193,21,189));
     // dashed
    BlackDashedPen.CreatePen(PS_DOT, 0, RGB(0,0,0));
	BlueDashedPen.CreatePen(PS_DOT, 0, RGB(0,0,255));
    // Fonts
	AxisFont.CreateFont (12,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		                 OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,
						 "Arial");
	NumberFont.CreateFont (12,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,
						  "Arial");
	LargeNumberFont.CreateFont (24,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,
						  "Arial");
	FontArial14.CreateFont (14,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,
						  "Arial");
	IsolineFont.CreateFont (10,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		                 OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,
						 "Arial");
    BlackBrush.CreateSolidBrush(RGB(0,0,0));
    GreenBrush.CreateSolidBrush(RGB(0,255,0));
    HellBlauBrush.CreateSolidBrush(RGB(29,189,211));
    BlueBrush.CreateSolidBrush(RGB(0,0,255));
    RedBrush.CreateSolidBrush(RGB(255,0,0));
    ZyanBrush.CreateSolidBrush(RGB(0,255,255));
    MagentaBrush.CreateSolidBrush(RGB(255,0,255));
    DarkGreenBrush.CreateSolidBrush(RGB(0,128,0));
    DarkBlueBrush.CreateSolidBrush(RGB(0,0,128));
    DarkRedBrush.CreateSolidBrush(RGB(128,0,0));
    DarkZyanBrush.CreateSolidBrush(RGB(0,128,128));
    DarkMagentaBrush.CreateSolidBrush(RGB(128,0,128));
    DarkYellowBrush.CreateSolidBrush(RGB(128,128,0));
    HighlightingBrush.CreateSolidBrush(RGB(255,204,0));
/*
//GDI+
SolidBrush solidBrush(Color(255, 255, 0, 0));
Graphics graphics;
graphics.FillEllipse(&solidBrush, 0, 0, 100, 60);
*/
    // Coordinates
    m_dXStep = 0.05;
    m_dYStep = 0.05;
    m_iQuantityIndex = 0;
    m_iNx = 20;
    m_iNy = 20;
    m_bDataRFO = false;
    m_iTimeSelected = -1;
    m_bTimeSelectedAll = false;
    label_elements = 250; //ElListSize()
    m_lNoElements2Draw = 0;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd(); //OK
  mainframe->m_bIsControlPanelOpen = true; //OK
}

CGraphics::~CGraphics()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd(); //OK
  mainframe->m_bIsControlPanelOpen = false; //OK
}

void CGraphics::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_TIMESTEP, m_LBTime);
    DDX_Text(pDX, IDC_EDIT_GRAF_1, m_dXmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_2, m_dXmax);
    DDX_Text(pDX, IDC_EDIT_GRAF_3, m_dYmin);
    DDX_Text(pDX, IDC_EDIT_GRAF_4, m_dYmax);
    DDX_Text(pDX, IDC_EDIT_UMIN, m_dUmin);
    DDX_Text(pDX, IDC_EDIT_UMAX, m_dUmax);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_IC,  m_bDisplayIC); //OK
    DDX_Check(pDX, IDC_CHECK_DISPLAY_BC,  m_iDisplayBC);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_PLY, m_iDisplayPLY);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH, m_iDisplayMSH);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_PNT, m_iDisplayPNT);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_ISOLINES, m_iDisplayIsolines);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_ISOSURFACES, m_iDisplayIsosurfaces);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_SUF, m_iDisplaySUF);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_LINE, m_iDisplayMSHLine);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_TRI, m_iDisplayMSHTri);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_TINs, m_bDisplayTINs);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_VOL, m_bDisplayVOL);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_QUAD, m_bDisplayMSHQuad);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_PRIS, m_bDisplayMSHPris);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_TET, m_bDisplayMSHTet);
    //DDX_Check(pDX, IDC_CHECK_LABELS, m_bShowLabels);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_PLY_PNT, m_bDisplayPLYPoints);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_PLY_LIN, m_bDisplayPLYLines);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_NOD, m_bDisplayNOD);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_ELE_DOUBLE, m_bDisplayMSHDoubleElements);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MAT_GROUPS, m_bDisplayMaterialGroups);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_ST, m_bDisplayST);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_ObS_WELLS, m_iDisplayObservaWells);
    DDX_Check(pDX, IDC_CHECK_AXIS, m_showaxis);
    DDX_Text(pDX, IDC_EDIT_LEVELS, m_levels);
    DDX_Check(pDX, IDC_CHECK_LEGEND, m_Displaylegend);
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Control(pDX, IDC_COMBO_PCS, m_CB_PCS);
    DDX_Control(pDX, IDC_LIST_FCT, m_LB_FCT);
    DDX_Check(pDX, IDC_SHOW_PNT_LABELS, m_bDisplayPNTLabels);
    DDX_Check(pDX, IDC_SHOW_PLY_LABELS, m_bDisplayPLYLabels);
    DDX_Check(pDX, IDC_SHOW_SFC_LABELS, m_bDisplaySFCLabels);
    DDX_Check(pDX, IDC_SHOW_VOL_LABELS, m_bDisplayVOLLabels);
    DDX_Control(pDX, IDC_COMBO_QUANTITIES, m_CB_Quantities);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_LABELS, m_bDisplayMSHLabels); // Who ???
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_PRIS2, m_bDisplayMSHHex);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_BMP, m_bDisplayBMP);
    DDX_Text(pDX, IDC_EDIT_MSH_ELEMENTS_DRAW, m_lNoElements2Draw);
    DDX_Control(pDX, IDC_LIST_DOM, m_LB_DOM);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_MSH_MAT, m_bDisplayMATGroupsMSH);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_DOM, m_bDisplayDOM);
    DDX_Control(pDX, IDC_COMBO_MAT, m_CB_MAT);
    DDX_Check(pDX, IDC_SHOW_SFC_MAT, m_bDisplaySFC_MAT);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_SFC_VALUES, m_bDisplaySFCValues);
}


BEGIN_MESSAGE_MAP(CGraphics, CDialog)
    ON_LBN_SELCHANGE(IDC_LIST_TIMESTEP, OnLbnSelchangeListTimes)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL, OnBnClickedButtonSelectAll)
    ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnBnClickedButtonUpdate)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH, OnCbnSelchangeComboMSH)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS, OnCbnSelchangeComboPCS)
    ON_LBN_SELCHANGE(IDC_LIST_FCT, OnLbnSelchangeListFCT)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL2, OnBnClickedButtonSelectAllFCT)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_NONE2, OnBnClickedButtonSelectNoneFCT)
    ON_BN_CLICKED(IDC_BUTTON_FCT_INTERPOLATE, OnBnClickedButtonFctInterpolate)
    ON_CBN_SELCHANGE(IDC_COMBO_QUANTITIES, OnCbnSelchangeComboQuantities)
    ON_BN_CLICKED(IDC_BUTTON_CALC_MIN_MAX, OnBnClickedButtonCalcMinMax)
    ON_EN_CHANGE(IDC_EDIT_GRAF_1, OnEnChangeEditXMin)
    ON_EN_CHANGE(IDC_EDIT_GRAF_2, OnEnChangeEditXMax)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_MSH, OnBnClickedCheckDisplayMSH)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_MSH_LABELS, OnBnClickedCheckDisplayMSHLabels)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_SUF, OnBnClickedCheckDisplaySFC)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_PLY, OnBnClickedCheckDisplayPLY)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_PNT, OnBnClickedCheckDisplayPNT)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_IC, OnBnClickedCheckDisplayIC)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_BC, OnBnClickedCheckDisplayBC)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_ST, OnBnClickedCheckDisplayST)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_ISOLINES, OnBnClickedCheckDisplayIsolines)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_ISOSURFACES, OnBnClickedCheckDisplayIsosurfaces)
    ON_EN_CHANGE(IDC_EDIT_UMAX, OnEnChangeEditUmax)
    ON_EN_CHANGE(IDC_EDIT_UMIN, OnEnChangeEditUmin)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_BMP, OnBnClickedCheckDisplayBMP)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_MAT_GROUPS, OnBnClickedCheckDisplayMATGroups)
    
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_PLY_PNT, OnBnClickedCheckDisplayPlyPnt)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_PLY_LIN, OnBnClickedCheckDisplayPlyLin)
    ON_BN_CLICKED(IDC_SHOW_PLY_LABELS, OnBnClickedShowPlyLabels)
    ON_BN_CLICKED(IDC_SHOW_PNT_LABELS, OnBnClickedShowPntLabels)
    ON_BN_CLICKED(IDC_SHOW_SFC_LABELS, OnBnClickedShowSfcLabels)
    ON_BN_CLICKED(IDC_BUTTON_DRAW_MSH, OnBnClickedButtonDrawMSHAll)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_MSH_MAT, OnBnClickedCheckDisplayMshMat)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_VOL, OnBnClickedCheckDisplayVOL)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_NOD, OnBnClickedCheckDisplayNod)
    ON_EN_CHANGE(IDC_EDIT_MSH_ELEMENTS_DRAW, OnEnChangeEditMshElementsDraw)
    ON_LBN_SELCHANGE(IDC_LIST_DOM, OnLbnSelchangeListDOM)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_DOM, OnBnClickedCheckDisplayDOM)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_DOM_TEC, OnBnClickedButtonWriteDomTec)
    ON_CBN_SELCHANGE(IDC_COMBO_MAT, OnCbnSelchangeComboMat)
    ON_BN_CLICKED(IDC_SHOW_SFC_MAT, OnBnClickedShowSfcMat)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_SFC_VALUES, OnBnClickedCheckDisplaySFCValues)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f¸r Nachrichten CGraphics 

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/1999 OK Implementation
07/2004 CC Modification quantity_vector
08/2004 OK Extensions for PCS2
04/2005 OK Ordering
05/2005 OK FCT
06/2005 OK Quantities
07/2005 OK Check boxes
10/2005 OK DOM
10/2005 OK MAT
**************************************************************************/
BOOL CGraphics::OnInitDialog() 
{
  int j;
  long i;
  CRFProcess* m_pcs = NULL;
  CFEMesh* m_msh = NULL;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // GEO
  //----------------------------------------------------------------------
  if(gli_points_vector.size()>0) m_iDisplayPNT = true;
  else m_iDisplayPNT = false;
  if(polyline_vector.size()>0) m_iDisplayPLY = true;//CC 08/2005
  else m_iDisplayPLY = false;
  if(surface_vector.size()>0) m_iDisplaySUF = true;//CC
  else m_iDisplaySUF = false;
  //----------------------------------------------------------------------
  // MSH
  //----------------------------------------------------------------------
  m_CB_MSH.ResetContent();
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_msh->selected = false;
    m_strMSHName = m_msh->pcs_name.c_str();
    m_CB_MSH.AddString(m_strMSHName);
    m_CB_MSH.SetCurSel(i);
    if((long)m_msh->ele_vector.size()>MAX_ELE_DRAW)
      m_lNoElements2Draw = MAX_ELE_DRAW; //OK
    else
      m_lNoElements2Draw = (long)m_msh->ele_vector.size(); //OK
  }
  //----------------------------------------------------------------------
  // PCS
  //----------------------------------------------------------------------
  m_CB_PCS.ResetContent();
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSName = m_pcs->pcs_type_name.c_str();
    m_CB_PCS.AddString(m_strPCSName);
    m_CB_PCS.SetCurSel(i);
  }
  //-------------------------------------------------------------------
  // Quantities
  //----------------------------------------------------------------------
  m_CB_Quantities.ResetContent();
  quantity_vector.clear ();
  CQuantity *m_quantity = NULL;
  if(m_bDataRFO){
    int no_node_quantities = (int)m_nodes_elements->nodevariablenames.size();
    for(i=0;i<no_node_quantities;i++){
      m_quantity = new CQuantity;
      m_quantity->m_strQuantityName  = (CString)m_nodes_elements->nodevariablenames[i].c_str();
      m_quantity->index = i;
      m_CB_Quantities.AddString((CString)m_nodes_elements->nodevariablenames[i].c_str());
      quantity_vector.push_back(m_quantity);
    }
  }
  else{
    if(m_pcs){
      for(j=0;j<(int)m_pcs->nod_val_name_vector.size();j++){ //OKMB
        m_CB_Quantities.AddString((CString)m_pcs->nod_val_name_vector[j].data());
        m_CB_Quantities.SetCurSel(0);
      }
      OnCbnSelchangeComboQuantities();
    }
/*OK
      if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)       
	  {
		 m_CB_Quantities.AddString("LOAD_PATH_ANALYSIS");
	     m_quantity = new CQuantity;
		 m_quantity->m_strQuantityName = "LOAD_PATH_ANALYSIS";
         quantity_vector.push_back(m_quantity);
	  } 
	}
*/
  }
  //----------------------------------------------------------------------
  // TIM
  //-------------------------------------------------------------------
  // Time steps
  if(m_bDataRFO){
    int no_times = m_nodes_elements->number_of_times;
    CString m_str_time;
    char c_string[10];
    for(i=0;i<no_times;i++){
      sprintf(c_string, "%ld",i);
      m_str_time = c_string;
      m_str_time += ": ";
      //m_nodes_elements->
      m_LBTime.AddString(m_str_time);
    }
  }
  else {
  }
  //----------------------------------------------------------------------
  // FCT
  //----------------------------------------------------------------------
  CFunction* m_fct = NULL;
  m_LB_FCT.ResetContent();
  for(i=0;i<(int)fct_vector.size();i++){
    m_fct = fct_vector[i];
    m_LB_FCT.AddString((CString)m_fct->geo_name.c_str());
  }
  //----------------------------------------------------------------------
  // DOM
  //----------------------------------------------------------------------
  CPARDomain* m_dom = NULL;
  m_LB_DOM.ResetContent();
  CString m_str;
  for(i=0;i<(int)dom_vector.size();i++){
    m_dom = dom_vector[i];
    m_str.Format("DOMAIN%i | %i | %i | %i",i+1,\
      (long)m_dom->elements.size(),(long)m_dom->nodes_inner.size(),(long)m_dom->nodes_halo.size());
    m_LB_DOM.AddString(m_str);
  }
  m_LB_DOM.AddString("ALL");
  //----------------------------------------------------------------------
  // MAT
  //----------------------------------------------------------------------
  CMediumProperties* m_mmp = NULL;
  m_CB_MAT.ResetContent();
  m_CB_MAT.AddString("ALL");
  for(i=0;i<(int)mmp_vector.size();i++){
    m_mmp = mmp_vector[i];
    m_CB_MAT.AddString(m_mmp->name.data());
  }
  m_CB_MAT.SetCurSel(0);
  //-------------------------------------------------------------------
  UpdateData(FALSE);
  //-------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/1999 OK Implementation
07/2004 CC Modification CQuantity for name, min and max value
10/2004 OK LOAD_PATH_ANALYSIS
**************************************************************************/
/*OK
void CGraphics::OnSelchangeListQuantities() 
{
  long i;
  double value;
  double m_dMin1 = 1.e+19;
  double m_dMax1 = -1.e+19;
  CQuantity *m_quantity = NULL;
  //-------------------------
  // Identify quantity name
  // Determine quantity nodal value index
  //----------------------------------------------------------------------
  if(m_bDataRFO){
    m_LBQuantities.GetText(m_LBQuantities.GetCurSel(),m_strQuantityName);
    m_iQuantityIndex = m_LBQuantities.GetCurSel();
    if(quantity_vector[m_iQuantityIndex]->selected == false)
      quantity_vector[m_iQuantityIndex]->selected = true;
    else
      quantity_vector[m_iQuantityIndex]->selected = false;
    for(i=0;i<m_nodes_elements->number_of_nodes;i++) {
      value = m_nodes_elements->nodevalues[i][m_iQuantityIndex][1];
      if(value<m_dMin1) m_dMin1 = value;
      if(value>m_dMax1) m_dMax1 = value;
    }
    quantity_vector[m_iQuantityIndex]->min = m_dMin1;
    quantity_vector[m_iQuantityIndex]->max = m_dMax1;
  }
  //----------------------------------------------------------------------
  // NOD values
  else{
    m_LBQuantities.GetText(m_LBQuantities.GetCurSel(),m_strQuantityName);
    if(m_strQuantityName=="LOAD_PATH_ANALYSIS"){
      return;
    }
    m_quantity = quantity_vector[m_LBQuantities.GetCurSel()]; // OK sorting
//    int testwert = m_LBQuantities.GetCurSel(); 
	if(quantity_vector[m_LBQuantities.GetCurSel()]->selected == false)
       quantity_vector[m_LBQuantities.GetCurSel()]->selected = true;
    else
      quantity_vector[m_LBQuantities.GetCurSel()]->selected = false;
    m_iQuantityIndex = PCSGetNODValueIndex((string)m_strQuantityName,1);
    m_quantity->index = m_iQuantityIndex;
    for(i=0;i<NodeListLength;i++) {
      value = GetNodeVal(i,m_iQuantityIndex);
      if(value<m_dMin1) m_dMin1 = value;
      if(value>m_dMax1) m_dMax1 = value;
    }
    m_quantity->min = m_dMin1;
    m_quantity->max = m_dMax1;
  } 
  //----------------------------------------------------------------------
  m_strUmin.Format("%g",m_dMin1);
  m_strUmax.Format("%g",m_dMax1);
  //m_strUmin.Format("%g",quantity_vector[m_iQuantityIndex]->min);
  //m_strUmax.Format("%g",quantity_vector[m_iQuantityIndex]->max);
  m_dUmin = m_dMin1;
  m_dUmax = m_dMax1;
  // Refresh control elements
  UpdateData(FALSE);
}
*/
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2004 OK Implementation
**************************************************************************/
void CGraphics::OnLbnSelchangeListTimes()
{
  m_iTimeSelected = m_LBTime.GetCurSel();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2004 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonSelectAll()
{
  m_bTimeSelectedAll = true;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/1999 OK Implementation
**************************************************************************/
void CGraphics::OnOK() 
{
  OnCancel();
}

//===========================================================================
// Draw Obj functions

void CGraphics::DrawLinePixel(int x1, int y1, int x2, int y2, CDC *dc)
{ 
  dc->MoveTo(x1,y1);
  dc->LineTo(x2,y2);
  return;
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGraphics::DrawLinePixelOffset(int x1, int y1, int x2, int y2, CDC *dc)
{ 
  static double dx1_off,dy1_off,dx2_off,dy2_off;
  static long x1_off,y1_off,x2_off,y2_off;
  long dx = (long)(m_dDX*(m_dXmax-m_dXmin)*(double)width);
  long dy = (long)(m_dDY*(m_dYmax-m_dYmin)*(double)height);

  double scale_x = (((double)width-(2.*(double)dx))/(double)width);
  double scale_y = (((double)height-(2.*(double)dy))/(double)height);

  dx1_off = (double)dx + (double)x1*scale_x;
  x1_off = (long)dx1_off;
  dx2_off = (double)dx + (double)x2*scale_x;
  x2_off = (long)dx2_off;

  dy1_off = (double)dy + (double)y1*scale_y;
  y1_off =  (long)dy1_off; //(y1-dy)*(height/(height-(2*dy)));
  dy2_off = (double)dy + (double)y2*scale_y;
  y2_off =  (long)dy2_off; //(y1-dy)*(height/(height-(2*dy)));

  scale_x = (m_dXmax-m_dXmin-2.*m_dDX)/(m_dXmax-m_dXmin);
  scale_y = (m_dYmax-m_dYmin-2.*m_dDY)/(m_dYmax-m_dYmin);

  dx1_off = (double)dx + (double)x1*scale_x;
  x1_off = (long)dx1_off;
  dx2_off = (double)dx + (double)x2*scale_x;
  x2_off = (long)dx2_off;

/*
      //values[0] = (x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
	  //y_0 = (y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
      //values[1] = (x1-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
	  //y_1 = (y1-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
*/
/*
  rf_point->x = (x_real(point.x,Width) - m_graphics.m_dDX) \
              * (m_dXmax-m_dXmin)/ \
                (m_dXmax-m_dXmin-2.*m_graphics.m_dDX) \
              + m_dXmin;
  rf_point->y = (y_real(point.y,Height) - m_graphics.m_dDY - m_dYmin) \
              * (m_dYmax-m_dYmin)/ \
                (m_dYmax-m_dYmin-2.*m_graphics.m_dDY) \
              + m_dYmin;
*/
  dc->MoveTo(x1_off,y1_off);
  dc->LineTo(x2_off,y2_off);
  return;
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGraphics::DrawLineOffset(double x1, double y1, double x2, double y2, CDC *dc)
{ 
  double scale_x = (m_dXmax-m_dXmin-2.*m_dDX)/(m_dXmax-m_dXmin);
  double scale_y = (m_dYmax-m_dYmin-2.*m_dDY)/(m_dYmax-m_dYmin);

  double x1_off = (x1-m_dXmin)*scale_x + m_dXmin + m_dDX;
  double y1_off = (y1-m_dYmin)*scale_y + m_dYmin + m_dDY;
  double x2_off = (x2-m_dXmin)*scale_x + m_dXmin + m_dDX;
  double y2_off = (y2-m_dYmin)*scale_y + m_dYmin + m_dDY;
/*
  rf_point->x = (x_real(point.x,Width) - m_graphics.m_dDX) \
              * (m_dXmax-m_dXmin)/ \
                (m_dXmax-m_dXmin-2.*m_graphics.m_dDX) \
              + m_dXmin;
  rf_point->y = (y_real(point.y,Height) - m_graphics.m_dDY - m_dYmin) \
              * (m_dYmax-m_dYmin)/ \
                (m_dYmax-m_dYmin-2.*m_graphics.m_dDY) \
              + m_dYmin;
*/
  dc->MoveTo(xpixel(x1_off),ypixel(y1_off));
  dc->LineTo(xpixel(x2_off),ypixel(y2_off));
  return;
}



int CGraphics::xpixel(double x)
{
//  GetWindowAttributes(this->m_hWnd, &width, &height);
//  RECT rect;
//  GetClientRect(&rect);   
  //GetClientRect(this->m_hWnd,&rect);
//  width = rect.right;
//  height= rect.bottom;

  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);

  return (int)(xminp + x * skalex);
}      	  	                                     

int CGraphics::ypixel(double y)
{
//  GetWindowAttributes(this->m_hWnd, &width, &height);
//  RECT rect;
//  GetClientRect(&rect);   
  //GetClientRect(this->m_hWnd,&rect);
//  width = rect.right;
//  height= rect.bottom;

  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);

  return (int)( yminp - y * skaley);
}      	  	                                     


//OKmsh
/**************************************************************************/
/* ROCKFLOW - Function: CGraphics::DrawElement
                                                                          */
/* Task:
   Draw mesh for all element types
                                                                          */
/* Programming:
   xx/1999   OK   First version
   xx/2001   MK   Extension
   01/2002   OK   Bugfix for triangles
   04/2002   OK   von 3.8.03 ¸bernommen
   04/2004 OK Labels
   04/2005 CC draw double elements                                                                       */
/**************************************************************************/
void CGraphics::DrawElement(long index,CDC *dc)
{
  CGLPoint gs_point1;
  CGLPoint gs_point2;

  if(ElGetElement(index)==NULL) return;

  int j;
  long *element_nodes = ElGetElementNodes(index);
  int nn = ElGetElementNodesNumber(index);
  if(nn==9) nn=4; // 9-Knoten-Elemente
  if(nn==6) nn=3; // Prismatic elements
  char char_label[10];
  double x_mid_pix, y_mid_pix;

  switch(ElGetElementType(index)) {
    case 1: // Line element
        if(ElGetElement(index)->doubled && m_bDisplayMSHDoubleElements)
      dc->SelectObject(&DarkPinkSolidBoldPen);
        else
            dc->SelectObject(&BlackBoldPen);
      break;
    case 2: // Quad element
         if(ElGetElement(index)->doubled && m_bDisplayMSHDoubleElements)
      dc->SelectObject(&DarkPinkSolidBoldPen);
        else
      dc->SelectObject(&RedBoldPen);
      break;
    case 4: // TRi element
         if(ElGetElement(index)->doubled && m_bDisplayMSHDoubleElements)
      dc->SelectObject(&DarkPinkSolidBoldPen);
        else
      dc->SelectObject(&BlueSolidPen);
      break;
    case 6: // Pris element
         if(ElGetElement(index)->doubled && m_bDisplayMSHDoubleElements)
      dc->SelectObject(&DarkPinkSolidBoldPen);
        else
      dc->SelectObject(&GreenSolidPen);
      break;
    default:
      dc->SelectObject(&BlackSolidPen);
  }

//OKnew
  if(element_nodes) {
    for(j=0;j<nn;j++) {
      gs_point1.x = GetNode(element_nodes[j])->x;
      gs_point1.y = GetNode(element_nodes[j])->y;
      gs_point1.z = GetNode(element_nodes[j])->z;
      if(j==nn-1) { // besser modula
        gs_point2.x = GetNode(element_nodes[0])->x;
        gs_point2.y = GetNode(element_nodes[0])->y;
        gs_point2.z = GetNode(element_nodes[0])->z;
      } 
      else {
        gs_point2.x = GetNode(element_nodes[j+1])->x;
        gs_point2.y = GetNode(element_nodes[j+1])->y;
        gs_point2.z = GetNode(element_nodes[j+1])->z;
      } 
      DrawLinePixel(xpixel(gs_point1.x),ypixel(gs_point1.y),xpixel(gs_point2.x),ypixel(gs_point2.y),dc);
    }
    //.....................................................................
    // Labels
    if(m_bShowLabels) {
      x_mid_pix = 0;
      y_mid_pix = 0;
      for(j=0;j<nn;j++) {
        x_mid_pix += GetNode(element_nodes[j])->x;
        y_mid_pix += GetNode(element_nodes[j])->y;
      }
      x_mid_pix /= nn;
      y_mid_pix /= nn;
      sprintf(char_label,"%d",index);
      dc->TextOut(xpixel(x_mid_pix),ypixel(y_mid_pix),char_label);
    }
    //.....................................................................
  }

}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/1999 OK Implementation
06/2004 OK extensions for RFO data
**************************************************************************/
void CGraphics::DrawIsosurfaces(long index,CDC *dc)
{
  int j;
  int nn;
  double x0,y0;
  double u_mean,u_rel;
  double u_int;
  double px[4],py[4];
  double rstep;
  //----------------------------------------------------------------------
  // Test
  m_iQuantityIndex = PCSGetNODValueIndex((string)m_strQuantityName,1);
  if(m_iQuantityIndex<0){
    return;
  }
  //----------------------------------------------------------------------
  nn = ElGetElementNodesNumber(index);
  if(nn==9) nn=4; // 9-Knoten-Elemente
  if(nn==6) nn=3; // prisms
  if(nn==8) nn=4; //OK hex
  //----------------------------------------------------------------------
  // Element corner nodes
  long *element_nodes = ElGetElementNodes(index);
  for(j=0;j<nn;j++) {
    x0 = GetNode(element_nodes[j])->x;
    y0 = GetNode(element_nodes[j])->y;
    //y0 = GetNode(element_nodes[j])->z;
    px[j] = x0; //(x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
    py[j] = y0; //(y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
    //SetPixel(pDC->m_hDC,xpixel(px[j]),ypixel(py[j]),RGB(0,0,0)); Fehler beim Rechnen
  }
  //----------------------------------------------------------------------
  // Element value
  //int m_iLevels = 25;
  u_mean = 0.0;
  //......................................................................
  if(m_bDataRFO){
    for(j=0;j<nn;j++) {
      u_mean += 1./(double)nn * m_nodes_elements->nodevalues[element_nodes[j]][m_iQuantityIndex][m_iTimeSelected]; 
    }
  }
  //......................................................................
  else{
    m_iQuantityIndex = PCSGetNODValueIndex((string)m_strQuantityName,1);
    for(j=0;j<nn;j++) {
      u_mean += 1./(double)nn * GetNodeVal(element_nodes[j],m_iQuantityIndex);
    }
  }
  //----------------------------------------------------------------------
  // Color calculation
  if(m_dUmax-m_dUmin>MKleinsteZahl)
    u_rel = (u_mean-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
  else
    u_rel = MKleinsteZahl;
  modf(u_rel*m_levels,&u_int);
  rstep = u_int * 255.0/m_levels;
  CGraphics::ChooseContourColours(rstep); // Neue Farbtabelle MK_3.8.03 
  //......................................................................
  MFC_FillElement1(nn,px,py,dc,ContourColourRed,ContourColourGreen,ContourColourBlue);
/*
  switch(ElGetElementType(index)) {
//OK3906
    case 2:
      u_mean = 0.0;
      if(0) {}
      //evals u_mean += 1./(double)nn * ElGetElementVal(index,m_iQuantityIndex); 
	  else {
        for(j=0;j<nn;j++) {
          u_mean += 1./(double)nn * GetNodeVal(element_nodes[j],m_iQuantityIndex);
		}    
      }                                // MK_000928_3422_end 
      if(m_dUmax-m_dUmin>MKleinsteZahl)
        u_rel = (u_mean-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
      else
        u_rel = MKleinsteZahl;

      modf(u_rel*m_iLevels,&u_int);
      rstep = u_int * 255.0/m_iLevels;
      // Berechnung der RGB-Farbwerte und Ausgabe
      CGraphics::ChooseContourColours(rstep);
      MFC_FillElement1(nn,px,py,dc,ContourColourRed,ContourColourGreen,ContourColourBlue);
      break;

    case 4: // Dreiecke
      u_mean = 0.0;
      for(j=0;j<nn;j++) {
        u_mean += 1./(double)nn * GetNodeVal(element_nodes[j],m_iQuantityIndex);
      }
      if(m_dUmax-m_dUmin>MKleinsteZahl)
        u_rel = (u_mean-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
      else
        u_rel = MKleinsteZahl;

      modf(u_rel*m_iLevels,&u_int);
      rstep = u_int * 255.0/m_iLevels;
      CGraphics::ChooseContourColours(rstep);
      MFC_FillElement1(nn,px,py,dc,ContourColourRed,ContourColourGreen,ContourColourBlue);
      break;
  }
*/
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/1999 OK Implementation
06/2004 OK extensions for RFO data
04/2005 OK MSH
**************************************************************************/
void CGraphics::DrawIsosurfacesNew(CElem*m_ele,CRFProcess*m_pcs,CDC*dc)
{
  int j;
  int nn;
  double x0,y0;
  double u_mean,u_rel;
  double u_int;
  double px[4],py[4];
  double rstep;
  CNode* m_nod = NULL;
  if(!m_pcs)
    return;
  //----------------------------------------------------------------------
  nn = m_ele->GetVertexNumber();
  if(nn==9) nn=4; // 9-Knoten-Elemente
  if(nn==6) nn=3; // prisms
  //----------------------------------------------------------------------
  // Element corner nodes
  for(j=0;j<nn;j++) {
    m_nod = m_ele->GetNode(j); //OKWW
    x0 = m_nod->X();
    y0 = m_nod->Y();
    px[j] = x0; //(x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
    py[j] = y0; //(y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
  }
  if(m_ele->GetVertexNumber()==2){
    m_nod = m_ele->GetNode(1); //OKWW
    px[2] = m_nod->X() + 100.;
    py[2] = m_nod->Y();
    m_nod = m_ele->GetNode(0); //OKWW
    px[3] = m_nod->X() + 100.;
    py[3] = m_nod->Y();
  }
  //----------------------------------------------------------------------
  // Element value
  u_mean = 0.0;
  //......................................................................
  if(m_bDataRFO){
    for(j=0;j<nn;j++) {
      u_mean += 1./(double)nn * m_nodes_elements->nodevalues[m_ele->nodes_index[j]][m_iQuantityIndex][m_iTimeSelected]; 
    }
  }
  //......................................................................
  else{
    m_iQuantityIndex = m_pcs->GetNodeValueIndex((string)m_strQuantityName) + 1; //new time
    //OK m_iQuantityIndex = m_pcs->GetNODValueIndex((string)m_strQuantityName,1); //new time
    for(j=0;j<nn;j++) {
      u_mean += 1./(double)nn * m_pcs->GetNodeValue(m_ele->nodes_index[j],m_iQuantityIndex);
    }
  }
  //----------------------------------------------------------------------
  // Color calculation
  if(m_dUmax-m_dUmin>MKleinsteZahl)
    u_rel = (u_mean-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
  else
    u_rel = MKleinsteZahl;
  modf(u_rel*m_levels,&u_int);
  rstep = u_int * 255.0/m_levels;
  ChooseContourColours(rstep); // Neue Farbtabelle MK_3.8.03 
  //......................................................................
  if(m_ele->GetVertexNumber()==2)
    nn = 4;
  MFC_FillElement1(nn,px,py,dc,ContourColourRed,ContourColourGreen,ContourColourBlue);
}

void CGraphics::MFC_FillElement1(int nn,double *nx,double *ny,CDC *dc,int r,int g,int b)
{
    CRgn quadrilateral;
    POINT m_arrPoint[9];

    for (int i=0;i<nn;i++) {
	    m_arrPoint[i].x = xpixel(nx[i]);
        m_arrPoint[i].y = ypixel(ny[i]);
    }

    if(nn==4) quadrilateral.CreatePolygonRgn(&m_arrPoint[0],4,WINDING);
    else if(nn==3) quadrilateral.CreatePolygonRgn(&m_arrPoint[0],3,WINDING);
    CBrush VariableBrush(RGB(r,g,b));
    //GreenBrush.CreateSolidBrush(RGB(0,255,0));
    dc->FillRgn(&quadrilateral,&VariableBrush);
    DeleteObject(VariableBrush);
    DeleteObject(quadrilateral);
}

void CGraphics::ChooseContourColours(double farbwert)
{
  if (farbwert <=127) {
  /* Dunkelblau bis Dunkelgr¸n */
    ContourColourRed  =0;
    ContourColourGreen=(int)(farbwert/127.0*170);
    ContourColourBlue =(int)((127.0-farbwert)/127.0*180.0); 
  }  
//   MFC_FillElement1(nn,px,py,dc,0.0,(rstep/127.0*170),((127.0-rstep)/127.0*180.0));
  else {
  /* Dunkegr¸n - Hellgelb */
    ContourColourRed  =(int)((farbwert-128)/127.0*255.0);
    ContourColourGreen=(int)(170+(farbwert-128.0)/127.0*85.0);
    ContourColourBlue =(int)((farbwert-128)/127.0*200.0); 
//   MFC_FillElement1(nn,px,py,dc,((rstep-128)/127.0*255.0),(170+(rstep-128.0)/127.0*85.0),((rstep-128)/127.0*200.0));
  }
  return;
}

void CGraphics::plot(double x,double y,int ipen,int sc_xmax,int sc_ymax,CDC *dc)
{
  //static int sc_xmax, sc_ymax;           /* merken: screensize */
  static double xy_fakt; /* Massstab */
  static double x_ref, y_ref;            /* Nullpunkt */
  //if (ipen==1)    /* initialize graphics mode */
  xy_fakt=min((double)sc_xmax/x,(double)sc_ymax/y);
  x_ref = 0.0; /* Nullpunkt */
  y_ref = 0.0;
  /* Plot-Zweige: nur 2,3 implementiert */
  if (ipen==2) dc->LineTo (xpixel(x),ypixel(y));
  if (ipen==3) dc->MoveTo (xpixel(x),ypixel(y));
  // Isoline label
  if(display_labels&&label){
    CGLPoint m_pnt;
    m_pnt.x_pix = xpixel(x);
    m_pnt.y_pix = ypixel(y);
    m_pnt.circle_pix = 3;
    DrawPointPixel(dc,&m_pnt);
    dc->TextOut(xpixel(x),ypixel(y),m_strIsolineLabel);
    label = false;
  }
  return;
}


//C ALLE HOEHENSCHICHTLINIEN EINES ELEMENTS ZEICHNEN, DREIECKE
//C***            ICOL1    (I,ARG) : FARBE FUER HSL =< 0.5*(H1+H2)
//C***                               (GROESSERE LINIEN MIT ICOL1+1)
void CGraphics::iso3k (double h[3],double x[3],double y[3],\
            double h1, double hd, double h2,\
            CDC *dc,int width,int height)
{
      int i,i1,i2,ipen;      
      double epsh=1.E-8;
      double h05,hh;
      double r1,r2;

      h05=0.5*(h1+h2);

      double hmin=h[0], hmax= h[0];
      for(i=0;i<3;i++) {
         hmin=min(hmin,h[i]);// extreme h
         hmax=max(hmax,h[i]);// im Element
      }

      hh=h1-hd;

Marke10:   
      hh=hh+hd;
      if (hh > h2)   return;
      if (hh > hmax) return;
      if (hh <= hmin) goto Marke10;
      //if (HH.GT.H05) CALL NEWPEN(ICOL1+1)  //ANDRE FARBE

// HH-LINIE GEHT DURCH ELEMENT
      ipen=3;
      i1=2;
      for(i2=0;i2<3;i2++) { //DO 50 I2=1,3
          r1=h[i2]-hh;
          r2=hh-h[i1];
          //if (r1*r2) Marke50,Marke20,Marke30;
          if ((r1*r2)<0.0) goto Marke50;
          else if((r1*r2)==0.0) goto Marke20;
          else if((r1*r2)>0.0) goto Marke30;
// EINE DIFFERENZ 0.0
Marke20:   
          if (fabs(r2) > epsh) goto Marke50;
          r1=1.0;
          goto Marke40;
// HH-LINIE SCHNEIDET KANTE
Marke30:   
          r1=r1/(h[i2]-h[i1]);
          r2=r2/(h[i2]-h[i1]);
Marke40:   
      plot (r1*x[i1]+r2*x[i2],r1*y[i1]+r2*y[i2],ipen,width,height,dc);
      //CALL PLOT (R1*XY(1,I1)+R2*XY(1,I2),R1*XY(2,I1)+R2*XY(2,I2),IPEN)
      ipen=2;
Marke50:   
      i1=i2;
      }

      goto Marke10;

      return;
}


/**************************************************************************
GeoSys-GUI: 
Programing:
09/2004 OK Implementation
06/2005 OK MSH
**************************************************************************/
void CGraphics::DrawIsolines(CDC*pDC,CRFProcess*m_pcs)
{
  long i;
  if(!m_pcs){
    AfxMessageBox("no PCS data");
    return;
  }
  if(m_pcs->m_msh){
    CElem* m_ele = NULL;
    for(i=0;i<(long)m_pcs->m_msh->ele_vector.size();i++) {
      m_ele = m_pcs->m_msh->ele_vector[i];
      DrawIsolinesELE(pDC,m_ele,m_pcs);
    }
  }
  else{
    for(i=0;i<ElListSize();i++) {
      DrawIsolinesELE(i,0.0,0.0,pDC,width,height);
    }
  }
}

/**************************************************************************
GeoSys-GUI: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawIsolinesELE(CDC*dc,CElem*m_ele,CRFProcess*m_pcs)
{
  int j;
  label = false;
  long el[4];
  double x[4],y[4],u[4];
  int m_iLevels = 10;
  //----------------------------------------------------------------------
  int nn = m_ele->GetVertexNumber();
  if(nn==9) nn=4; // 9-Knoten-Elemente
  if(nn==6) nn=3; // prisms
  //----------------------------------------------------------------------
  // Labels
  label = false;
  if(fmod((double)m_ele->GetIndex(),label_elements)<1e-3){
    label = true;
  }
  //----------------------------------------------------------------------
  for(j=0;j<m_ele->GetVertexNumber();j++){
    el[j] = j; //element_nodes[j];
    x[j] = m_pcs->m_msh->nod_vector[m_ele->nodes_index[j]]->X();
    y[j] = m_pcs->m_msh->nod_vector[m_ele->nodes_index[j]]->Y();
    if(m_bDataRFO){
      u[j] = m_nodes_elements->nodevalues[m_ele->nodes_index[j]][m_iQuantityIndex][m_iTimeSelected]; 
    }
    else{
      m_iQuantityIndex = m_pcs->GetNodeValueIndex((string)m_strQuantityName)+1; //new time
      u[j] = m_pcs->GetNodeValue(m_ele->nodes_index[j],m_iQuantityIndex);
    }
  }
  //----------------------------------------------------------------------
  switch(m_ele->GetElementType()){
    //....................................................................
    case 2: // quads
      dc->SelectObject(&BlackSolidPen);
      //if(u[j]==m_dUmin) return;
      if((u[0]==u[1])&&(u[0]==u[2])) 
        return;
      if(m_dUmax-m_dUmin>MKleinsteZahl){
        iso4k(el,u,x,y,m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
      }
      break;
    //....................................................................
    case 4: // tris
      dc->SelectObject(&BlackSolidPen);
      if(m_dUmax-m_dUmin > MKleinsteZahl)
      iso3k (u,x,y,m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
      break;
    //....................................................................
    case 6: // pris
      dc->SelectObject(&BlackSolidPen);
      if(m_dUmax-m_dUmin > MKleinsteZahl)
      iso3k (u,x,y,m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
      break;
    //....................................................................
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI: 
Programing:
06/1999 OK Implementation
07/2004 CC Modification plot all quantities possible
**************************************************************************/
void CGraphics::DrawIsolinesELE(long index,double dx,double dy,CDC *dc,int width,int height)
{
  int j;
  int nn = ElGetElementNodesNumber(index);
  if(nn==9) nn=4; // 9-Knoten-Elemente
  double x0,y0;
  long *element_nodes = ElGetElementNodes(index);
  int m_iLevels = 25;
  int m_nGeometry2D = 0;
  vector<CQuantity*>::iterator p = quantity_vector.begin ();
  CQuantity *m_quantity = NULL;
  //if(m_nValueType==0) {
  label = false;
  long el[4];
  double x[4],y[4],u[4];
  //=====================================================================
  switch(ElGetElementType(index)) {
    //-------------------------------------------------------------------
    case 2:
      while(p!=quantity_vector.end()){
        m_quantity = *p;
        if(m_quantity->selected == true){
          m_dUmin = m_quantity->min;
          m_dUmax = m_quantity->max;
          m_iQuantityIndex = m_quantity->index;
          m_iLevels = m_quantity->levels;
          if(m_iQuantityIndex == 0){
            dc->SelectObject(&BlueSolidPen);
          }
          else{
            dc->SelectObject(&RedSolidPen);  
          }
          for(j=0;j<nn;j++) {
            el[j] = j; //element_nodes[j];
            x0 = GetNode(element_nodes[j])->x;
            y0 = 0.0;
            if(m_nGeometry2D==0) { // x - y system
              y0 = GetNode(element_nodes[j])->y;
            }
            else if (m_nGeometry2D==1) { // x - z system
              y0 = GetNode(element_nodes[j])->z;
            }
            x[j] = (x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
            y[j] = (y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
            if(m_bDataRFO){
              //--CC 07/2004 draw both pressure and concentration
              u[j] = m_nodes_elements->nodevalues[element_nodes[j]][m_iQuantityIndex][m_iTimeSelected]; 
            }
            else{
              m_iQuantityIndex = PCSGetNODValueIndex((string)m_strQuantityName,1);
              u[j] = GetNodeVal(element_nodes[j],m_iQuantityIndex);
            }
            //if(u[j]==m_dUmin) return;
          }
          if((u[0]==u[1])&&(u[0]==u[2])) 
            return;
          //..............................................................
          // Labels
          label = false;
          if(fmod((double)index,label_elements)<1e-3){
            label = true;
          }
          //..............................................................
          if(m_dUmax-m_dUmin>MKleinsteZahl)
            iso4k (el, u, x, y, m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
        }// end of my if
        ++p;
      }//end of while
      break;

    case 4:

        for(j=0;j<nn;j++) {
          x0 = GetNode(element_nodes[j])->x;
//          if(m_nGeometry2D==0) { // x - y system
          y0 = GetNode(element_nodes[j])->y;

		  x[j] = (x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin + dx;
          y[j] = (y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin + dy;
          u[j] = GetNodeVal(element_nodes[j],m_iQuantityIndex);
        }

        if(m_iLevels==0) m_iLevels=10;
        if(m_dUmax-m_dUmin > MKleinsteZahl)
        iso3k (u,x,y,m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
        break;

      case 6: // prisms
        for(j=0;j<3;j++) {
          x0 = GetNode(element_nodes[j])->x;
//          if(m_nGeometry2D==0) { // x - y system
            y0 = GetNode(element_nodes[j])->y;
//          }
//          else if (m_nGeometry2D==1) { // x - z system
//            y0 = GetNode(element_nodes[j])->z;
//          }
		  x[j] = (x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
          y[j] = (y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
          u[j] = GetNodeVal(element_nodes[j],m_iQuantityIndex);
        }
        if(m_iLevels==0) m_iLevels=10;
        if(m_dUmax-m_dUmin>MKleinsteZahl)
        iso3k (u,x,y,m_dUmin,(m_dUmax-m_dUmin)/m_iLevels,m_dUmax,dc,width,height);
        break;
      }  // switch
//  }

/*
    if((m_nValueType==1)&&(deformation)) {
    	dc->SelectObject(&BlackSolidPen); 
        long el[4];
        double x[4],y[4],u[4];
        double r[4],s[4];
        double *strain,*stress;
        //static int anzgp = 2;
        r[0] = 1.0;
        r[1] = -1.0;
        r[2] = -1.0;
        r[3] = 1.0;
        s[0] = 1.0;
        s[1] = 1.0;
        s[2] = -1.0;
        s[3] = -1.0;
        for(j=0;j<nn;j++) {
            el[j] = j; //element_nodes[j];
            x0 = GetNode(element_nodes[j])->x;
            y0 = GetNode(element_nodes[j])->y;
  		    x[j] = (x0-m_dXmin)*(m_dXmax-m_dXmin-2.*dx)/(m_dXmax-m_dXmin) + m_dXmin+dx;
            y[j] = (y0-m_dYmin)*(m_dYmax-m_dYmin-2.*dy)/(m_dYmax-m_dYmin) + m_dYmin+dy;
            //r = MXPGaussPkt(anzgp,i);
            //s = MXPGaussPkt(anzgp,j);
            strain = Calc2DStrainTensor0010(index,r[j],s[j]);
            stress = Calc2DStressTensor0010(index,r[j],s[j]);
    		if (strcmp(m_strPlotElementVariableName,"STRAIN_XX")==0) u[j] = strain[0];
    		if (strcmp(m_strPlotElementVariableName,"STRAIN_YY")==0) u[j] = strain[1];
    		if (strcmp(m_strPlotElementVariableName,"STRAIN_XY")==0) u[j] = strain[2];
    		if (strcmp(m_strPlotElementVariableName,"STRESS_XX")==0) u[j] = stress[0];
    		if (strcmp(m_strPlotElementVariableName,"STRESS_YY")==0) u[j] = stress[1];
    		if (strcmp(m_strPlotElementVariableName,"STRESS_XY")==0) u[j] = stress[2];

            if(1) {
                sprintf(strPuffer,"%le",u[j]);
                dc->SelectObject(&NumberFont);
                MFC_text(x[j],y[j], strPuffer, dc);
            }

        }

    	dc->SelectObject(&BlackSolidPen); 
        if(m_iLevels==0) m_iLevels=10;
        if(m_dUmax-m_dUmin>MKleinsteZahl)
            iso4k (el, u, x, y, m_dUmin,(m_dUmax-m_dUmin)/m_iLevels, m_dUmax,\
                   dc,width,height);

    }
*/
}

void CGraphics::iso4k (long el4[4], double h[], double x[], double y[],
            double h1, double hd, double h2,\
            CDC *dc,int width,int height)
/* Alle Hîhenschichtlinien eines Elements zeichnen, Vierecke
   bilin. Ansatz - R. Ratke, Jan 2000 -
   Parameter: el4  [4]: die 4 Eckennummern des Elements >=0
              h    [*]: Hîhen aller Knoten
              x    [*]: Plotkoordinaten (cm) aller Knoten, x
              y    [*]: Plotkoordinaten (cm) aller Knoten, y
              h1      : Wert der kleinsten Hîhenschichtlinie
              hd      : Abstand der Hîhenlinien
              h2      : Wert der grî·ten Hîhenschichtlinie
   es wird nichts verÑndert in den Feldern!
*/
{     
  double r4[2][4] ={{1., -1., -1.,  1.},  // Formfunktionen bzw.
                    {1.,  1., -1., -1.}}; // Eckenkoordinaten, r-s
  double r[2],p[2],a[2],q[2];
  double eps =1.e-4;  // rel. Genauigkeit gegenÅber 1.0
  double eps0=1.e-7;  // abs. Genauigkeit bezgl. Koord.
  int i, nstep=10;    // Anzahl Teilstriche je BogenstÅck
  double hmin=h[el4[0]], hmax= h[el4[0]];
  for (i=1; i<4; i++) {  
    hmin=min(hmin,h[el4[i]]);// extreme h
    hmax=max(hmax,h[el4[i]]);// im Element
    //hmin=min(hmin,h[i]);// extreme h
    //hmax=max(hmax,h[i]);// im Element
  }
  h1 -=hd; // wird Wert der zu zeichnenden HSL
//========================================================================
Neue_HSL:
  h1 +=hd;
  m_strIsolineLabel.Format("%5.2e",h1);
  if (h1 > h2)   return;
  if (h1 > hmax) return;
  if (h1 < hmin) goto Neue_HSL;
  //----------------------------------------------------------------------
  // h1-Linie geht durch Element
  { int nix[4]= {0, 0, 0, 0}, i1= el4[3], i, i2, j, k, l, gerade;
  double c, d, t1, t2, z, t, dt, et;
  // Vorwerte
  a[0]=  h[el4[0]]-h[el4[1]]-h[el4[2]]+h[el4[3]];
  a[1]=  h[el4[0]]+h[el4[1]]-h[el4[2]]-h[el4[3]];
  c   =  h[el4[0]]-h[el4[1]]+h[el4[2]]-h[el4[3]];
  //a[0]=  h[0]-h[1]-h[2]+h[3];
  //a[1]=  h[0]+h[1]-h[2]-h[3];
  //c   =  h[0]-h[1]+h[2]-h[3];
  gerade=dabs(c) <= eps0;
  if (! gerade) {a[0]=a[0]/c; a[1]=a[1]/c;}
  i1=el4[3];
  //----------------------------------------------------------------------
  for (i=0; i<4 ;i++) //Kanten auf Schnittpunkt absuchen
  { i2=el4[i];
    if (nix[i]) goto End_Kant; //Linie an Kante schon gezeichnet
    t1=h[i2]-h1;  t2=h1-h[i1];     //Differenzen auf Kante
    if (t1*t2 <0.0) goto End_Kant; //kein Schnittpunkt
    d=h[i2]-h[i1];
    if (dabs(d) <= eps0)      //gesamte Kante ist HSL
    {  plot (x[i1],y[i1],3,width,height,dc); //move_to
       plot (x[i2],y[i2],2,width,height,dc); //draw_to
       goto End_Kant;
    }
    // h1-Linie schneidet Kante
    t1=t1/d; t2=t2/d; //nat. Koord. bezueglich Kante
    for (l=0; l<2; l++)
    {  p[l] = r4[l][md1(i+3,4)]*t1+r4[l][i]*t2; //r,s Schnittpunkt
       r[l]=p[l]; q[l]=p[l]+a[1-l];      // Zielpunkt
    }
    // normales Zeichnen - keine Hyperbeln
    plot4k (0, r, el4, x, y, r4,dc,width,height); //move_to Schnittpunkt
                                                ;//nach r-s Koordinate (r)
    j=0; k=1; // vorl. Annahme: r hat Betrag 1.
    //....................................................................
    if (gerade)  // --- c=0.0:es gibt nur Geraden ---
    { if (dabs(a[0])  > eps) goto Test_a2;
      if (dabs(a[1]) <= eps) goto End_Kant;
// a1=0.0: beliebiges r, s=const, gerade auch in x-y
xy_gerade:  r[j]=-r[j];              //Ziel auf Gegenseite
      plot4k (1,r,el4,x,y,r4,dc,width,height);
      goto Gegenueber;
// a2=0.0: beliebiges s, r=const, ebenfalls gerade in x-y-system
Test_a2: if (dabs(a[1]) <= eps) {j=1; goto xy_gerade;} //wie oben, s statt r
// a1 und a2  .ne. 0.0, c=0.0
      if (dabs(p[0]) < dabs(p[1])) j=1; // s hat Betrag 1.
      k=1-j;                          // die andere Koordinate
// direkt auf Gegenseite zugehen
      r[j] = -r[j];   r[k] = (p[j]-r[j])*a[j]/a[k] +p[k];
      if (dabs(r[k]) <= 1.0-eps) // Kurve in r-s, stueckweise
      {  plot4k (nstep,r,el4,x,y,r4,dc,width,height);
         goto Gegenueber;
      }
      r[k]= sign(1.0,r[k]);
      r[j]= (p[k]-r[k])*a[k]/a[j] +p[j];
      plot4k (nstep,r,el4,x,y,r4,dc,width,height);
      goto Nachbarseite;
    } // end if (gerade)
    //....................................................................
    // ----- c von Null verschieden: es gibt Hyperbeln im r-s-System -----
    if (dabs(p[0])+eps < 1.0) j=1; k=1-j;
    z=q[0]*q[1];
    if (dabs(q[j]) <= eps)
    // z/Nenner = 0./0. :gerade r=p oder s=q, gerade im x-y-system
    {  r[k]=-p[k]; plot4k (1,r,el4,x,y,r4,dc,width,height);
       goto End_Kant;
    }
    if (dabs(q[k]) <= eps)
    // auf anderer Asymptote
    {  r[j]=-1.0;  plot4k (0,r,el4,x,y,r4,dc,width,height);
       r[j]= 1.0;  plot4k (1,r,el4,x,y,r4,dc,width,height);
       goto End_Kant;
    }
// echte Hyperbel, Asymptoten bei r=-a2 bzw. s=-a1
// Plotten ueber die Parameterdarstellung in Teilen
// Aenderung von t aus Ableitung  dr[j]/ dt= e**t *q[j]
    t = 0.0; dt= sign (1./nstep, -q[j]*p[j]); // max. nstep TeilstÅcke
Hyperbel: t +=dt;  et=exp(t);
    r[j]=q[j]*et -a[k];  if (dabs(r[j]) > 1.0+eps) goto Rest_G;
    r[k]=q[k]/et -a[j];  if (dabs(r[k]) > 1.0+eps) goto Rest_N;
    plot4k (1,r,el4,x,y,r4,dc,width,height);
    goto Hyperbel;
// Gegenkante erreicht
Rest_G:   r[j]=sign(1.0,r[j]);
    r[k]=z/(a[k]+r[j])-a[j]; if (dabs(r[k]) > 1.0+eps) goto Rest_N;
    plot4k (1,r,el4,x,y,r4,dc,width,height);
    goto Gegenueber;
//Nachbarkante erreicht
Rest_N:   r[k]=sign(1.0,r[k]);
    r[j]=z/(a[j]+r[k])-a[k];
    plot4k (1,r,el4,x,y,r4,dc,width,height);
Nachbarseite:   // in nix merken!
    if(i<3){ 
      if ((r4[k][i]+r4[k][i+1])*r[k] > 1.0) nix[i+1]=1;
    }
    if (i==0){ 
      if ((r4[k][2]+r4[k][  3])*r[k] > 1.0) nix[i+3]=1;
    }
    goto End_Kant;
Gegenueber:
    if (i<2) nix[i+2]=1;
End_Kant: i1=i2;
  } // for i...
  //----------------------------------------------------------------------
}  goto Neue_HSL;
//========================================================================
}  // end ISO4K


void CGraphics::plot4k (int nstep, double rs[2],long el4[4],double xk[],double yk[],double ff[2][4],\
             CDC *dc,int width,int height)
// Teilstueck plotten nach nat. Koordinate r,s
{
  double  r, s, x=0., y=0.;
  static double p, q;
  int i, is;
  if(nstep==0){ // initialize
    p=rs[0]; q=rs[1]; // save!
    for(i=0;i<4;i++){ 
      x=x+(1.+ff[0][i]*p)*(1.+ff[1][i]*q)*xk[el4[i]];
      y=y+(1.+ff[0][i]*p)*(1.+ff[1][i]*q)*yk[el4[i]];
    }
    plot (x*0.25,y*0.25,3,width,height,dc);
    return;
  }
  // Geraden im r-s-System bei nstep >1 evtl. Kurven in x-y-System
  r=p; s=q; // saved
  for(is=0;is<nstep;is++){ 
    r +=(rs[0]-p)/nstep;  s +=(rs[1]-q)/nstep;
    x=0.;  y=0.;
    for(i=0;i<4;i++){  
      x +=(1.+ff[0][i]*r)*(1.+ff[1][i]*s)*xk[el4[i]];
      y +=(1.+ff[0][i]*r)*(1.+ff[1][i]*s)*yk[el4[i]];
    }
    plot(x*0.25,y*0.25,2,width,height,dc);
  }
  p=r; q=s;  // save!
  return;
}//  end PLOT4K



/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGraphics::DrawCoordinateAxes(CDC *dc)
{
  char strPuffer[MAX_ZEILE];
  double x_i,y_i;

  dc->SelectObject(&BlackBoldPen);
  // x-axis
  //DrawLinePixelOffset(xpixel(m_dXmin),ypixel(m_dYmin),xpixel(m_dXmax),ypixel(m_dYmin),dc);
    x_i = m_dXmin+m_dDX;
    DrawLinePixel(xpixel(x_i),ypixel(m_dYmin+m_dDY),xpixel(x_i),ypixel(m_dYmax-m_dDY),dc);
  // y-axis
  //DrawLinePixelOffset(xpixel(m_dXmin),ypixel(m_dYmin),xpixel(m_dXmin),ypixel(m_dYmax),dc);
    y_i = m_dYmin+m_dDY;
    DrawLinePixel(xpixel(m_dXmin+m_dDX),ypixel(y_i),xpixel(m_dXmax-m_dDX),ypixel(y_i),dc);
  // labels
  dc->SelectObject(&AxisFont);
  dc->SelectObject(&BlackSolidPen);
  for (int i=0; i<m_iNx+1; i++) {
    x_i = m_dXmin+m_dDX + ((m_dXmax-m_dXmin-2.*m_dDX)/(double) m_iNx) * (double) (i);
    DrawLinePixel(xpixel(x_i),ypixel(m_dYmin+0.9*m_dDY),xpixel(x_i),ypixel(m_dYmin+1.1*m_dDY),dc);
	sprintf(strPuffer,"%lg",m_dXmin+(m_dXmax-m_dXmin)*(i)/m_iNx);
	dc->TextOut(xpixel(x_i),ypixel(m_dYmin+0.9*m_dDY),(CString)strPuffer);
    y_i = m_dYmin+m_dDY + ((m_dYmax-m_dYmin-2.*m_dDY)/(double) m_iNy) * (double) (i);
    DrawLinePixel(xpixel(m_dXmin+0.9*m_dDX),ypixel(y_i),xpixel(m_dXmin+1.1*m_dDX),ypixel(y_i),dc);
	sprintf(strPuffer,"%lg",m_dYmin+(m_dYmax-m_dYmin)*(i)/m_iNy);
	dc->TextOut(xpixel(m_dXmin+0.4*m_dDX),ypixel(y_i),(CString)strPuffer);
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGraphics::DrawGridAxes(CDC *dc)
{
  double x_i,y_i;
  dc->SelectObject(&BlackDashedPen);
  for (int i=0; i<m_iNx+1; i++) {
    x_i = m_dXmin+m_dDX + ((m_dXmax-m_dXmin-2.*m_dDX)/(double) m_iNx) * (double) (i);
    DrawLinePixel(xpixel(x_i),ypixel(m_dYmin+m_dDY),xpixel(x_i),ypixel(m_dYmax-m_dDY),dc);
  }
  for (int i=0; i<m_iNy+1; i++) {
    y_i = m_dYmin+m_dDY + ((m_dYmax-m_dYmin-2.*m_dDY)/(double) m_iNy) * (double) (i);
    DrawLinePixel(xpixel(m_dXmin+m_dDX),ypixel(y_i),xpixel(m_dXmax-m_dDX),ypixel(y_i),dc);
  }
}


/**************************************************************************
GeoSysGUI-File: 
Task: dialog to source terms
Programing:
12/1999 OK RF-ShellImplementierung
11/2003 OK GeoSys Implementation
**************************************************************************/
void CGraphics::DrawTemporalBreakthroughCurves(CDC *dc,CTimeDiscretization* m_tim)
{
  m_tim = m_tim;
  // 0 Daten
//    m_nPlotVariable = CGrafic::PlotVariableIndex();

  // 0 Projekt-Titel
/*
	dc->SelectObject(&FontArial14); 
    if(project_title) 
      MFC_text(xmin,ymax-0.1*dy,project_title,dc);
*/
  // 2 Daten-Information plotten
/*
	dc->SelectObject(&FontArial14);
	// 2.1 X-Achse: Zeit
	char string[128];
	char string1[10] = "X-Axis: ";
	char string2[15] = "Time in [s]: ";
	char string3[10];
    sprintf(string3,"%e",aktuelle_zeit);
    sprintf(string,"%s %s %s",string1,string2,string3);
	MFC_text((xmax-xmin)/2.+dx, ymax-dy, string, dc);
    // Y-Achse
	char string4[10] = "Y-Axis: ";
    sprintf(string,"%s %s %s",string4,\
                              nval_data[m_nPlotVariable].name,nval_data[m_nPlotVariable].einheit);
                            //datafield_n[g_nPlotVariable].name,datafield_n[g_nPlotVariable].einheit);
	MFC_text((xmax-xmin)/2.+dx, ymax-2.*dy, string, dc);
    // 2.2 Beobachtungspunkt
    Knoten *kno = NULL;
    CString m_str;
    for (int j=0;j<output_node_anz;j++) {  //
        if ((kno = GetNode(output_node_array[j]))==NULL) {  //
            DisplayErrorMsg("Fehler: Knoten existiert nicht (DK) !!");
        } 
        else {
            m_str.Empty();
            m_str = "Observation point: ";
   	        sprintf(strPuffer," %f ",kno->x);
            m_str += strPuffer;
	        sprintf(strPuffer," %f ",kno->y);
            m_str += strPuffer;
	        sprintf(strPuffer," %f ",kno->z);
            m_str += strPuffer;
	        strcpy(string,m_str);
        	MFC_text((xmax-xmin)/2.+dx, ymax-(3+j)*dy, string, dc);
          ChooseMultColours_SolidPen(dc, j);
          MFC_line((xmax-xmin)/2.+0.25*dx,ymax-(3.25+j)*dy,(xmax-xmin)/2.+0.75*dx,ymax-(3.25+j)*dy, dc);
        }
    }
*/
  if(m_strQuantityName.IsEmpty()) return;

  // 4 Referenz-Daten plotten
  // determine output grid nodes
  vector<long> out_node_vector;
  out_node_vector.clear();
/*OK_MMP
  OUTPUT *op = NULL;
  list_op_init();
  long node;
  while ((op = (OUTPUT *) get_list_op_next()) != NULL) {
    if(op->type==1) {
      node = GetNodeNumberClose(op->x[0],op->y[0],op->z[0]);
      out_node_vector.push_back(node);
    }
  }
*/
  // Observation points info
  CString m_strPointInfo;
  char char_string[80];
  int j;
  Knoten *msh_node=NULL;
  long out_node_vector_size = (long)out_node_vector.size();
  for (j=0;j<out_node_vector_size;j++) {  //
    msh_node = GetNode(out_node_vector[j]);
    if (!msh_node) {  //
     DisplayErrorMsg("Fehler: Knoten existiert nicht (DK) !!");
    }
    else {
      m_strPointInfo.Empty();
      m_strPointInfo = "Observation point: ";
   	  sprintf(char_string," %f ",msh_node->x);
      m_strPointInfo += char_string;
	  sprintf(char_string," %f ",msh_node->y);
      m_strPointInfo += char_string;
	  sprintf(char_string," %f ",msh_node->z);
      m_strPointInfo += char_string;
      dc->TextOut(500,50+j*10,m_strPointInfo);
      //MFC_text((xmax-xmin)/2.+dx, ymax-(3+j)*dy, string, dc);
      //ChooseMultColours_SolidPen(dc, j);
      //MFC_line((xmax-xmin)/2.+0.25*dx,ymax-(3.25+j)*dy,(xmax-xmin)/2.+0.75*dx,ymax-(3.25+j)*dy, dc);
    }   
  } 

  // 5 Durchbruchskurve zeichnen
  dc->SelectObject(&RedSolidPen);
  double x0,x1,y0,y1;
  CGLPoint m_point;
  strcpy(char_string,m_strQuantityName);
  int m_iNODValueIndex0 = PCSGetNODValueIndex(char_string,0);
  if(m_iNODValueIndex0<0) return;
  int m_iNODValueIndex1 = PCSGetNODValueIndex(char_string,1);
  if(m_iNODValueIndex1<0) return;
  //double *y01;
  //y01 = new double[out_node_vector.size()];
  if(aktueller_zeitschritt==1) {
    time_last=0.0;
  }

  for (j=0;j<out_node_vector_size;j++) {  //
    if (GetNode(out_node_vector[j])==NULL) {  //
     DisplayErrorMsg("Fehler: Knoten existiert nicht (DK) !!");
    } 
    else {
      x0 = time_last;
      x1 = aktuelle_zeit;
	  y0 = GetNodeVal(out_node_vector[j],m_iNODValueIndex0);
      y1 = GetNodeVal(out_node_vector[j],m_iNODValueIndex1);
      m_point.x = x1;
      m_point.y = y1;
      DrawPointOffset(dc,&m_point);
      if(aktueller_zeitschritt>0) {
        //ChooseMultColours_BoldPen(dc, j);
        DrawLineOffset(x0,y0,x1,y1,dc);
      }
    }
  }
  time_last = aktuelle_zeit;

  //delete(y01);
}

/**************************************************************************
GeoSysGUI-File: 
Task: OUT along polylines
Programing:
12/1999 OK RF-ShellImplementierung
12/2003 OK GeoSys Implementation
08/2004 OK OUT implementation
**************************************************************************/
void CGraphics::DrawProfileBreakthroughCurves(CDC *dc)
{
  long i;
  double x1,y1,z1;
  double x0=0.0,y0=0.0;
  double d12[3],dist12;
  long *nodes;
  long number_of_nodes;
  double value;
  char char_string[80];
  // Plot pen
  dc->SelectObject(&RedSolidPen); 
  // Plot value
  strcpy(char_string,m_strQuantityName);
  if(pcs_vector.size()==0)
    return;
  int m_iNODValueIndex = PCSGetNODValueIndex(char_string,1);
  if(m_iNODValueIndex<0) 
    return;
/*
  //----------------------------------------------------------------------
  // OUT Polylines
  CGLPoint m_point;
  CGLPolyline *m_polyline = NULL;
  COutput *m_out = NULL;
  int no_out = out_vector.size();
  for(i=0;i<no_out;i++){
    m_out = out_vector[i];
    if(m_out->geo_type_name.find("POLYLINE")!=string::npos){
      m_polyline = m_polyline->GEOGetPolyline(m_out->geo_name);
      if(!m_polyline) return;
      //OK nodes = m_polyline->MSHGetNodesClose(&number_of_nodes);
      nodes = m_polyline->MSHGetNodesCloseSorted(&number_of_nodes);
      // distance from first polyline point
      for(i=0;i<number_of_nodes;i++) {
        x1 = GetNodeX(nodes[i]);
        y1 = GetNodeY(nodes[i]);
        z1 = GetNodeZ(nodes[i]);
        d12[0]=m_polyline->point_vector[0]->x-x1;
        d12[1]=m_polyline->point_vector[0]->y-y1;
        d12[2]=m_polyline->point_vector[0]->z-z1;
        dist12=MBtrgVec(d12,3);
        value = GetNodeVal(nodes[i],m_iNODValueIndex);
        m_point.x = dist12;
        m_point.y = value;
        DrawPointOffset(dc,&m_point);
        if(i==0) {
          x0 = dist12;
          y0 = value;
        }
        DrawLineOffset(x0,y0,dist12,value,dc);
        x0 = dist12;
        y0 = value;
      }
    }
  }
  //----------------------------------------------------------------------
*/
  //----------------------------------------------------------------------
  // Selected OUT Polylines
  CGLPoint m_point;
  CGLPolyline *m_polyline = NULL;
  m_polyline = GEOGetPLYByName((string)m_strPolylineName);//CC
  if(!m_polyline) return;
  nodes = MSHGetNodesCloseSorted(&number_of_nodes,m_polyline);
  // distance from first polyline point
  for(i=0;i<number_of_nodes;i++) {
    x1 = GetNodeX(nodes[i]);
    y1 = GetNodeY(nodes[i]);
    z1 = GetNodeZ(nodes[i]);
    d12[0]=m_polyline->point_vector[0]->x-x1;
    d12[1]=m_polyline->point_vector[0]->y-y1;
    d12[2]=m_polyline->point_vector[0]->z-z1;
    dist12=MBtrgVec(d12,3);
    value = GetNodeVal(nodes[i],m_iNODValueIndex);
    m_point.x = dist12;
    m_point.y = value;
    DrawPointOffset(dc,&m_point);
    if(i==0) {
      x0 = dist12;
      y0 = value;
    }
    DrawLineOffset(x0,y0,dist12,value,dc);
    x0 = dist12;
    y0 = value;
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Method: 
Task: 
Programing:
06/1999 OK Implementation
06/2005 CC Modification
07/2005 CC when BMP ,manify the point size
**************************************************************************/
void CGraphics::DrawPointPixel(CDC *dc, CGLPoint *m_point)
{
   if(m_point->highlighted)
   {// CC
     dc->SelectObject(&HellBlauBrush); 
     m_point->circle_pix = 5;

   }
   else if (m_point->highlighted == false){//dc->SelectObject(BlackSolidPen); break;
    
       if (m_bDisplayBMP){
     dc->SelectObject(&RedBrush); 
     m_point->circle_pix = 5;
       }
       else{
       dc->SelectObject(&BlackBrush); 
       m_point->circle_pix = 3;
       }
   }
    //default: dc->SelectObject(&GreenBrush); break;
  
   CRect MyQuadrat (m_point->x_pix-m_point->circle_pix,m_point->y_pix+m_point->circle_pix,\
                   m_point->x_pix+m_point->circle_pix,m_point->y_pix-m_point->circle_pix);

   dc->Ellipse(MyQuadrat);
   dc->SelectObject(&BlackBrush); 
}

void CGraphics::DrawDoublePointPixel(CDC *dc, CGLPoint *m_point)
{
  dc->SelectObject(&RedSolidPen);
  CRect MyQuadrat (m_point->x_pix-m_point->circle_pix,m_point->y_pix+m_point->circle_pix,\
                   m_point->x_pix+m_point->circle_pix,m_point->y_pix-m_point->circle_pix);
  dc->Ellipse(MyQuadrat);
}

void CGraphics::DrawPointOffset(CDC *dc, CGLPoint *m_point)
{ 
  double scale_x = (m_dXmax-m_dXmin-2.*m_dDX)/(m_dXmax-m_dXmin);
  double scale_y = (m_dYmax-m_dYmin-2.*m_dDY)/(m_dYmax-m_dYmin);
  double x1_off = (m_point->x-m_dXmin)*scale_x + m_dXmin + m_dDX;
  double y1_off = (m_point->y-m_dYmin)*scale_y + m_dYmin + m_dDY;
  CRect MyQuadrat (xpixel(x1_off)-m_point->circle_pix,ypixel(y1_off)+m_point->circle_pix,\
                   xpixel(x1_off)+m_point->circle_pix,ypixel(y1_off)-m_point->circle_pix);
  dc->Ellipse(MyQuadrat);
  //----------------------------------------------------------------------
/*
  CString m_strViewInfo;
  m_strViewInfo.Format("%g",m_point->x);
  dc->TextOut(xpixel(x1_off),ypixel(y1_off),m_strViewInfo);
  m_strViewInfo.Format("%i",m_point->old_id);
  dc->TextOut(xpixel(x1_off),ypixel(y1_off)-25,m_strViewInfo);
*/
}


/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
01/2004 OK Implementation
04/2004 CC Modification CClientDC to CDC
07/2005 CC Highlighting
**************************************************************************/
void CGraphics::DrawPolylines(CDC *pDC)
{ 
  CGLPolyline *m_polyline = NULL;
  CGLPolyline *s_polyline = NULL;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC 08/2005
  while(p!=polyline_vector.end()) {
    m_polyline = *p;
    if (m_polyline->highlighted)
        s_polyline = *p;
      DrawPolyline(pDC,m_polyline);
    ++p;
  }
  if (s_polyline)
  DrawPolyline(pDC,s_polyline);
  else
      return;
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawPolylinesPoints(CDC *pDC)
{ 
  CGLPolyline *m_polyline = NULL;
  CGLPoint *m_point = NULL;
  long point_vector_length;
  long i;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC08/2005
  while(p!=polyline_vector.end()) {//CC08/2005
    m_polyline = *p;
   
      point_vector_length = (long)m_polyline->point_vector.size();
      for(i=0;i<point_vector_length;i++){
        m_point = m_polyline->point_vector[i];
        m_point->x_pix = xpixel(m_point->x);
        m_point->y_pix = ypixel(m_point->y);
        DrawPointPixel(pDC,m_point);
        if(m_bDisplayPLYLabels) {
          //sprintf(char_label,"%d",m_point->id);
          pDC->TextOut(m_point->x_pix,m_point->y_pix,m_point->name.c_str());
        }
      }
    
    ++p;
  }
}
/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawPolylinesLines(CDC *pDC)
{ 
  CGLPolyline *m_polyline = NULL;
  CGLPoint *m_point1 = NULL;
  CGLPoint *m_point2 = NULL;
  CGLLine *m_line = NULL;
  long line_list_length;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC 08/2005
  vector<CGLLine*>::iterator pl;
  char char_label[10];
  int x_pix,y_pix;

  while(p!=polyline_vector.end()) {//CC 08/2005
    m_polyline = *p;

      line_list_length = (long)m_polyline->line_vector.size();//CC
      pl = m_polyline->line_vector.begin();
      while(pl!=m_polyline->line_vector.end()){
        m_line = *pl;
        m_point1 = m_line->m_point1;
        m_point1->x_pix = xpixel(m_point1->x);
        m_point1->y_pix = ypixel(m_point1->y);
        m_point2 = m_line->m_point2;
        m_point2->x_pix = xpixel(m_point2->x);
        m_point2->y_pix = ypixel(m_point2->y);
        DrawLinePixel(xpixel(m_point1->x),ypixel(m_point1->y),xpixel(m_point2->x),ypixel(m_point2->y),pDC);
        if(m_bDisplayPLYLabels) {
          sprintf(char_label,"%d",m_point1->id);//CC
          pDC->TextOut(m_point1->x_pix,m_point1->y_pix,char_label);
          sprintf(char_label,"%d",m_point2->id);
          pDC->TextOut(m_point2->x_pix,m_point2->y_pix,char_label);
          sprintf(char_label,"%d",m_line->gli_line_id);
          x_pix = (int)((m_point1->x_pix+m_point2->x_pix)/2);
          y_pix = (int)((m_point1->y_pix+m_point2->y_pix)/2);
          //sprintf(char_label,"%d",char_label);
          //pDC->TextOut(x_pix,y_pix,char_label);
        }
        ++pl;
      }
    
    ++p;
  }
}
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
09/2003 OK Implementation
04/2004 CC Modification
04/2004 OK Labels
07/2004 CC Modification CPen
06/2005 CC highlight
07/2005 CC bitmap
**************************************************************************/
void CGraphics::DrawPolyline(CDC *dc,CGLPolyline *m_polyline)
{
   CPen HighlightSolidPen;
   CPen ColorSolidPen;
   BYTE r,g,b = 0;
   if(m_polyline->highlighted){// CC
      r= 255;
      g = 0;
      b = 0;
        HighlightSolidPen.CreatePen(PS_SOLID,2,RGB(r,g,b));
        dc->SelectObject(&HighlightSolidPen);
   }
   else if (m_polyline->highlighted == false)
   {//dc->SelectObject(BlackSolidPen); break;
       if(m_bDisplayBMP){
   r = 56;
   g = 13;
   b = 215;
   ColorSolidPen.CreatePen(PS_SOLID,2,RGB(r,g,b));
   dc->SelectObject(&ColorSolidPen);}
       else
       {
   r = (BYTE)m_polyline->m_color[0];
   g = (BYTE)m_polyline->m_color[1];
   b = (BYTE)m_polyline->m_color[2];
   ColorSolidPen.CreatePen(PS_SOLID,0,RGB(r,g,b));
   dc->SelectObject(&ColorSolidPen);
       }
       
   }
  //---------------------------------------------------------------
  CGLPoint *m_point1 = NULL;
  CGLPoint *m_point2 = NULL;
  CGLPoint *m_point = NULL;
  if(m_polyline->point_vector.size()==0) // OK 
    return;
  vector<CGLPoint*> vector_points = m_polyline->point_vector;
  vector<CGLPoint*>::iterator pp = vector_points.begin();
  m_point1 = *pp;
  if(vector_points.size()<=1)
    //dc->MoveTo(m_point1->x_pix,m_point1->y_pix);
    dc->MoveTo(xpixel(m_point1->x),ypixel(m_point1->y));
  if(vector_points.size()>1) {
    while(pp!=vector_points.end()) {
      //dc->MoveTo(m_point1->x_pix,m_point1->y_pix);
      dc->MoveTo(xpixel(m_point1->x),ypixel(m_point1->y));
      m_point2 = *pp;
      //dc->LineTo(m_point2->x_pix,m_point2->y_pix);
      dc->LineTo(xpixel(m_point2->x),ypixel(m_point2->y));
      m_point1 = m_point2;
      ++pp;
    }
  }
  if(m_polyline->closed) {
    dc->MoveTo(xpixel(m_polyline->point_vector[vector_points.size()-1]->x),\
               ypixel(m_polyline->point_vector[vector_points.size()-1]->y));
    dc->LineTo(xpixel(m_polyline->point_vector[0]->x),ypixel(m_polyline->point_vector[0]->y));
  }
  // names
  if (m_bDisplayPLYLabels) {
    m_point = m_polyline->CenterPoint();
    if(m_point)
      dc->TextOut(xpixel(m_point->x),ypixel(m_point->y),m_polyline->name.c_str());
  }
  dc->SelectObject(&BlackSolidPen);

}

/**************************************************************************
GeoSys-GUI-Method: Draw(CClientDC *dc,Surface *m_surface)
Task: draw surface
Programing:
12/2003 CC Implementation
07/2005 CC Highlighting
07/2005 CC get point by gli_point_id
08/2005 CC Modification
**************************************************************************/
void CGraphics::DrawSurface(CDC *dc,Surface *m_surface,int brush_type)
{
  POINT m_arrPoint[1024];
  CRgn polygon;
  int r = m_surface->m_color[0];
  int g = m_surface->m_color[1];
  int b = m_surface->m_color[2];
  long Psize = (long)m_surface->polygon_point_vector.size();
  for(int i=0;i<(int)m_surface->polygon_point_vector.size();i++){
    m_arrPoint[i].x = xpixel(m_surface->polygon_point_vector[i]->x);
    m_arrPoint[i].y = ypixel(m_surface->polygon_point_vector[i]->y);
  }
  polygon.CreatePolygonRgn(&m_arrPoint[0],(int)Psize,WINDING);
  CBrush PolygonBrush;
  if(brush_type==0)
      if(m_surface->highlighted)
      {
     PolygonBrush.CreateSolidBrush(RGB(255,204,0));
     dc->SelectObject(PolygonBrush);
      }
      
      else
      {
    PolygonBrush.CreateHatchBrush(2,RGB(r,g,b));
    dc->SelectObject(PolygonBrush);
      }
  else{
    PolygonBrush.CreateSolidBrush(RGB(r,g,b));
  dc->SelectObject(PolygonBrush);
  }
  dc->FillRgn(&polygon,&PolygonBrush);
  DeleteObject(polygon);
  DeleteObject(PolygonBrush);
}

/**************************************************************************
GeoSys-GUI-Method: DrawElementMaterialGroup
Task: 
Programing:
12/1999 OK Implementation
04/2004 CC Modification
**************************************************************************/
void CGraphics::DrawElementMaterialGroup(long index, CDC *dc)
{
  if(ElGetElement(index)==NULL) return;
  long *element_nodes = ElGetElementNodes(index);
  if(element_nodes==NULL) return;

  POINT m_arrPoint[8];
  CRgn polygon;
  int color;
  int i;

  int nn = ElGetElementNodesNumber(index);
  if(nn==9) nn=4; // 9-Knoten-Elemente

  for(i=0;i<nn;i++) {
    m_arrPoint[i].x = xpixel(GetNode(element_nodes[i])->x);
    m_arrPoint[i].y = ypixel(GetNode(element_nodes[i])->y);
  }

  //----------------------------------------------------------------------
  // Line colors
  COLORREF a[20];
  a[0] = RGB(0,0,200);
  a[1] = RGB(200,0,0);
  a[2] = RGB(0,200,0);
  a[3] = RGB(0,100,200);
  a[4] = RGB(200,100,0);
  a[5] = RGB(0,200,50);
  a[6] = RGB(100,0,200);
  a[7] = RGB(200,20,0);
  a[8] = RGB(30,200,0);
  int material_group_number = ElGetElementGroupNumber(index);
  if(material_group_number<9)
    color = material_group_number; //material_group_number;
  else 
    color = 0;
  //......................................................................
  CPen LinePen;
  LinePen.CreatePen(PS_SOLID,0,a[color]);
  dc->SelectObject(LinePen);
  if(ElGetElementType(index)==1) {
    DrawLinePixel(m_arrPoint[0].x,m_arrPoint[0].y,m_arrPoint[1].x,m_arrPoint[1].y,dc);
  }
  DeleteObject(LinePen);

  //----------------------------------------------------------------------
  // Brush colors
  CMediumProperties *m_mat_mp = NULL;
  material_group_number = ElGetElementGroupNumber(index);
  m_mat_mp = m_mat_mp->GetByGroupNumber(material_group_number);
  int r,g,b;
  r=g=b=0;
  if(m_mat_mp) {
    r = m_mat_mp->m_color[0];
    g = m_mat_mp->m_color[1];
    b = m_mat_mp->m_color[2];
  }
  CBrush PolygonBrush;
  PolygonBrush.CreateSolidBrush(RGB(r,g,b));
  //......................................................................
  polygon.CreatePolygonRgn(&m_arrPoint[0],nn,WINDING);
  dc->SelectObject(PolygonBrush);
  dc->FillRgn(&polygon,&PolygonBrush);
  DeleteObject(polygon);
  DeleteObject(PolygonBrush);
}

// CGraphicproperty dialog


/**************************************************************************
GeoSysGUI-File: 
Task: OUT along polylines
Programing:
02/2004 OK Implementation
04/2004 CC Modification
**************************************************************************/
void CGraphics::DrawPolylineValues(CDC *dc, CGLPolyline *m_polyline)
{
  double x0,y0,z0;
  
  long i;
  double value;
  double x1,y1,z1;
  double d12[3],dist12;

  // Which BC uses m_polyline
  CBoundaryCondition *m_bc = NULL;
  CGLPoint m_point;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if(m_bc->geo_name.compare(m_polyline->name)==0) 
      break;
    ++p_bc;
  }

  long node_number_vector_length = (long)m_bc->node_number_vector.size();
  x0 = GetNodeX(m_bc->node_number_vector[0]);
  y0 = GetNodeY(m_bc->node_number_vector[0]);
  z0 = GetNodeZ(m_bc->node_number_vector[0]);
  for(i=0;i<node_number_vector_length;i++) {
    x1 = GetNodeX(m_bc->node_number_vector[i]);
    y1 = GetNodeY(m_bc->node_number_vector[i]);
    z1 = GetNodeZ(m_bc->node_number_vector[i]);
    d12[0] = x1-x0;
    d12[1] = y1-y0;
    d12[2] = z1-z0;
    dist12 = MBtrgVec(d12,3);
    m_point.x = dist12;
    value = m_bc->node_value_vector[i];
    m_point.y = value;
    DrawPointOffset(dc,&m_point);
  }

/*
  CBoundaryConditionsGroup *m_bc_group = NULL;
  m_bc_group = m_bc_group->Get((string)m_strQuantityName);
  if(!m_bc_group)
    return;
  long bc_group_vector_length = m_bc_group->group_vector.size();
  for(i=0;i<bc_group_vector_length;i++) {
    // The new geo point indeces are computed in rf_bc_new.cpp 
    node = m_bc_group->group_vector[i]->geo_node_number;
    for(j=0;j<polyline_point_vector_length;j++){
      if(node==m_polyline->point_vector[j]->gli_point_id){
        x1 = m_polyline->point_vector[j]->x;
        y1 = m_polyline->point_vector[j]->y;
        z1 = m_polyline->point_vector[j]->z;
        d12[0] = x1-x0;
        d12[1] = y1-y0;
        d12[2] = z1-z0;
        dist12 = MBtrgVec(d12,3);
        m_point.x = dist12;
        value = m_bc_group->group_vector[i]->node_value;
        m_point.y = value;
        DrawPointOffset(dc,&m_point);
      }
    } // number_of_nodes
  } // m_bc_group->group_vector
*/
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
02/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawSurfaceTIN(CDC *dc,Surface *m_surface)
{
  long i;
  int j;
  CGLPoint gs_point1;
  CGLPoint gs_point2;
  // Pen
  dc->SelectObject(&BlackSolidPen);
  //
  if(!m_surface->TIN) return;
  long nb_TIN_elements = (long)m_surface->TIN->Triangles.size();
  for(i=0;i<nb_TIN_elements;i++) {
    for(j=0;j<3;j++) {
      gs_point1.x = m_surface->TIN->Triangles[i]->x[j];
      gs_point1.y = m_surface->TIN->Triangles[i]->y[j];
      gs_point1.z = m_surface->TIN->Triangles[i]->z[j];
      if(j==3-1) { // besser modula
        gs_point2.x = m_surface->TIN->Triangles[i]->x[0];
        gs_point2.y = m_surface->TIN->Triangles[i]->y[0];
        gs_point2.z = m_surface->TIN->Triangles[i]->z[0];
      } 
      else {
        gs_point2.x = m_surface->TIN->Triangles[i]->x[j+1];
        gs_point2.y = m_surface->TIN->Triangles[i]->y[j+1];
        gs_point2.z = m_surface->TIN->Triangles[i]->z[j+1];
      } 
      DrawLinePixel(xpixel(gs_point1.x),ypixel(gs_point1.y),xpixel(gs_point2.x),ypixel(gs_point2.y),dc);
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method: DrawSurfaces
Task: draw all the selected surfaces
Programing:
02/2004 CC Implementation
01/2005 CC Modification remove the surface type  
07/2005 CC Modification highlight
0772005 CC show label
**************************************************************************/
void CGraphics::DrawSurfaces(CDC *pDC)
{
  int x_pix,y_pix;
  Surface* m_surface = NULL;
  Surface* s_surface = NULL;
  vector<Surface*>::iterator p = surface_vector.begin();
  //----------------------------------------------------------------------
  while (p!= surface_vector.end()){
    m_surface = *p;
	if(m_surface->highlighted)
      s_surface = *p;
    DrawSurface(pDC,m_surface,0);
    ++p;
  }
  if(s_surface)
    DrawSurface(pDC,s_surface,0);
  //----------------------------------------------------------------------
  if(m_bDisplaySFCLabels){
    vector<Surface*>::iterator p = surface_vector.begin();
    p = surface_vector.begin();
    while (p!= surface_vector.end()){
      m_surface = *p;
      m_surface->CalcCenterPoint();
      x_pix = xpixel(m_surface->center_point[0]);
  	  y_pix = ypixel(m_surface->center_point[1]);
      pDC->TextOut(x_pix,y_pix,(CString)m_surface->name.data());
	  ++p;
	}
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawNodes(CDC *pDC)
{ 
  long i;
  CGLPoint m_point;
  char char_label[10];
  for(i=0;i<NodeListLength;i++){
    m_point.x_pix = xpixel(GetNodeX(i));
    m_point.y_pix = ypixel(GetNodeY(i));
    DrawPointPixel(pDC,&m_point);    
    if(m_bShowLabels) {
      sprintf(char_label,"%d",i);
      pDC->TextOut(m_point.x_pix,m_point.y_pix,char_label);
    }
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
**************************************************************************/
void CGraphics::FillMSHElement(CDC *pDC,int nn, long *element_nodes)
{ 
  int i;
  double px[10],py[10];
  for(i=0;i<nn;i++) {
    px[i] = GetNode(element_nodes[i])->x;
    py[i] = GetNode(element_nodes[i])->y;
  }
  MFCFillElement(nn,px,py,pDC,&RedBrush);
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
**************************************************************************/
void CGraphics::MFCFillElement(int nn,double *nx,double *ny,CDC *pDC,CBrush *Brush)
{
    CRgn quadrilateral;
    POINT m_arrPoint[4];
    for (int i=0;i<nn;i++) {
	    m_arrPoint[i].x = xpixel(nx[i]);
        m_arrPoint[i].y = ypixel(ny[i]);
    }
    quadrilateral.CreatePolygonRgn(&m_arrPoint[0],nn,WINDING);
    pDC->FillRgn(&quadrilateral,Brush);
    DeleteObject(quadrilateral);
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
04/2004 OK Implementation
10/2005 OK MAT-GEO
**************************************************************************/
void CGraphics::DrawMaterialGroups(CDC *pDC)
{
  int i;
  int x_pix,y_pix;
  long r_number;
  CGLPolyline *m_ply = NULL;
  Surface *m_sfc = NULL;
  CMediumProperties *m_mmp = NULL;
  //----------------------------------------------------------------------
  for(int m=0;m<(int)mmp_vector.size();m++){
    m_mmp = mmp_vector[m];
    if(!m_mmp->selected)
      continue;
    //....................................................................
    if(m_mmp->geo_type_name.compare("POLYLINE")==0){
      m_ply = GEOGetPLYByName(m_mmp->geo_name);
      if(m_ply){
        pDC->SelectObject(BlackBoldPen); 
        DrawPolyline(pDC,m_ply);
      }
      continue;
    }
    //....................................................................
    else if(m_mmp->geo_type_name.compare("SURFACE")==0){
      r_number = rand();
      m_mmp->m_color[0] = (long)(r_number/(double)RAND_MAX*256);
      r_number = rand();
      m_mmp->m_color[1] = (long)(r_number/(double)RAND_MAX*256);
      r_number = rand();
      m_mmp->m_color[2] = (long)(r_number/(double)RAND_MAX*256);
      for(i=0;i<(int)m_mmp->geo_name_vector.size();i++){
        m_sfc = GEOGetSFCByName(m_mmp->geo_name_vector[i]);
        if(m_sfc){
          m_sfc->m_color[0] = m_mmp->m_color[0];
          m_sfc->m_color[1] = m_mmp->m_color[1];
          m_sfc->m_color[2] = m_mmp->m_color[2];
          DrawSurface(pDC,m_sfc,1);
          m_sfc->CalcCenterPoint();
          x_pix = xpixel(m_sfc->center_point[0]);
          y_pix = ypixel(m_sfc->center_point[1]);
          pDC->TextOut(x_pix,y_pix,(CString)m_sfc->name.data());
        }
      }
/*
      if(m_bShowLabels){
        if(m_sfc->polygon_point_vector.size()==0){
          m_sfc->PolygonPointVector();//CC
        }
      }
*/
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
06/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawElements(CDC *pDC)
{
  long i;
  Element *element = NULL;
  if(NodeListLength>0) {
    for(i=0;i<ElListSize();i++) {
      element = ElGetElement(i);
      if(element->aktiv==0)
        continue;
      switch(ElGetElementType(i)) {
        case 1: // lines 
          if(m_iDisplayMSHLine) DrawElement(i,pDC);
          break;
        case 2: // quads
          if(m_bDisplayMSHQuad) DrawElement(i,pDC);
          break;
        case 4: // triangles
          if(m_iDisplayMSHTri)  DrawElement(i,pDC);
          break;
        case 6: // prisms
          if(m_bDisplayMSHPris) DrawElement(i,pDC);
          break;
      }
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonUpdate()
{
  UpdateData(true);
  int index = m_CB_Quantities.GetCurSel();
  quantity_vector[index]->min = m_dUmin;
  quantity_vector[index]->max = m_dUmax;
  quantity_vector[index]->levels = m_levels;
}

//---------------CQuantity
CQuantity::CQuantity(CWnd* pParent /*=NULL*/)
{
  pParent = pParent; //OK
  selected = false;
  levels = 25;
  min = 0.0;
  max = 1.0;
  selected = false;
}

CQuantity::~CQuantity()
{
}

/**************************************************************************
GeoSysGUI-File: 
Task:
Programing:
10/2004 OK 
**************************************************************************/
void CGraphics::DrawLoadPathAnalysis(CDC *dc)
{
  // Projekt-Titel
  if(m_strQuantityName.IsEmpty()) return;
  CGLPoint *m_pnt = NULL;
  COutput *m_out = NULL;
  int no_out = (int)out_vector.size();
  int no_pcs_names;
  long msh_node; 
  int i,j,k;
  int out_line_vector_size;
  double x1,y1,x2,y2;
  //----------------------------------------------------------------------
  dc->SelectObject(&RedSolidPen);
  //----------------------------------------------------------------------
  // Draw yield surface
   // Drucker-Prager 
   //   smat = msp_vector[elem->group_number];
  CSolidProperties* smat = msp_vector[0];
  smat->CalulateCoefficent_DP();
  //double I_ref = smat->Y0;
  //----------------------------------------------------------------------
  for(i=0;i<no_out;i++){
    m_out = out_vector[i];
    no_pcs_names = (int)m_out->nod_value_vector.size();
    for(j=0;j<no_pcs_names;j++){
      if(m_out->nod_value_vector[j].find("LOAD_PATH_ANALYSIS")!=string::npos){
        m_pnt = GEOGetPointByName(m_out->geo_name);//CC
        m_pnt->circle_pix = 4;
        //................................................................
        msh_node = GetNodeNumberClose(m_pnt->x,m_pnt->y,m_pnt->z);
        // Yield locus
        //double pls = PCSGetNODValue(msh_node,"STRAIN_PLS",1);
        //double I1 = smat->BetaN*(smat->Y0+smat->Hard*pls);
        //................................................................
        // Draw stress path
        out_line_vector_size = (int)m_out->out_line_vector.size();
        for(k=0;k<out_line_vector_size-1;k++){
          x1 = m_out->out_line_vector[k][0];
          y1 = m_out->out_line_vector[k][1];
          x2 = m_out->out_line_vector[k+1][0];
          y2 = m_out->out_line_vector[k+1][1];
          DrawLineOffset(x1,y1,x2,y2,dc);
          m_pnt->x = x1;
          m_pnt->y = y1;
          DrawPointOffset(dc,m_pnt);
          m_pnt->x = x2;
          m_pnt->y = y2;
          DrawPointOffset(dc,m_pnt);
        }
      }
    }
  }
  //----------------------------------------------------------------------
  //ChooseMultColours_BoldPen(dc, j);
  //out_line_vector.clear(); if simlation is finshed
}
/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
07/2005 OK Moving time point
**************************************************************************/
void CGraphics::DrawFCT(CDC*dc,CFunction*m_fct)
{
  int i;
  double x1,y1,x2,y2;
  CGLPoint m_pnt;
  //----------------------------------------------------------------------
  dc->SelectObject(&RedSolidPen);
  m_pnt.circle_pix = 3;
  //----------------------------------------------------------------------
  for(i=0;i<(int)m_fct->variable_data_vector.size()-1;i++){
    x1 = m_fct->variable_data_vector[i][0];
    y1 = m_fct->variable_data_vector[i][1];
    x2 = m_fct->variable_data_vector[i+1][0];
    y2 = m_fct->variable_data_vector[i+1][1];
    DrawLineOffset(x1,y1,x2,y2,dc);
    m_pnt.x = x1;
    m_pnt.y = y1;
    DrawPointOffset(dc,&m_pnt);
    m_pnt.x = x2;
    m_pnt.y = y2;
    DrawPointOffset(dc,&m_pnt);
  }
  //----------------------------------------------------------------------
  dc->SelectObject(&BlueSolidPen);
  m_pnt.circle_pix = 5;
  x1 = aktuelle_zeit;
  bool valid;
  y1 = m_fct->GetValue(x1,&valid);
  m_pnt.x = x1;
  m_pnt.y = y1;
  DrawPointOffset(dc,&m_pnt);
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
**************************************************************************/
void CGraphics::MarkPoints(CDC*dc,CPoint point)
{
  bool found = false;
  long i;
  CString m_str;
  CGLPoint* m_pnt = NULL;
  CSourceTerm* m_st = NULL;
  //----------------------------------------------------------------------
  // Re-calc pixel coordinates
  long gli_points_vector_size = (long)gli_points_vector.size();
  for(i=0;i<gli_points_vector_size;i++) {
    gli_points_vector[i]->x_pix = xpixel(gli_points_vector[i]->x);
    gli_points_vector[i]->y_pix = ypixel(gli_points_vector[i]->y);
  }
  //----------------------------------------------------------------------
  // PNT loop
  for(i=0;i<gli_points_vector_size;i++){
    m_pnt = gli_points_vector[i];
    found = MarkPoint(dc,point,m_pnt);
    if(found){
      if(m_bDisplayST){
        m_st = STGet(m_pnt->name);
        if(m_st){
          dc->TextOut(m_pnt->x_pix,m_pnt->y_pix,m_st->tim_type_name.c_str());
        }
      }
    }
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
**************************************************************************/
bool CGraphics::MarkPoint(CDC*dc,CPoint point,CGLPoint*m_pnt)
{
  if(abs(m_pnt->x_pix)>width||abs(m_pnt->y_pix>height)) 
    return false;
  CString m_str;
  CRgn circle;
  CPoint point1;
  point1.x = m_pnt->x_pix;
  point1.y = m_pnt->y_pix;
  //CC? dc->LPtoDP(&point1,nCount);
  m_pnt->x_pix = point1.x;
  m_pnt->y_pix = point1.y;
  circle.CreateEllipticRgn(m_pnt->x_pix-m_pnt->circle_pix,\
                           m_pnt->y_pix-m_pnt->circle_pix,\
                           m_pnt->x_pix+m_pnt->circle_pix,\
                           m_pnt->y_pix+m_pnt->circle_pix);
  if(circle.PtInRegion(point)){
    dc->SelectObject(MagentaBrush);
    dc->FillRgn(&circle,&MagentaBrush);
    if(m_iDisplayPNT){
      m_str.Format("%ld",m_pnt->id);
      dc->TextOut(m_pnt->x_pix,m_pnt->y_pix,m_str);
    }
    return true;
  }
  DeleteObject(circle);
  return false;
}
/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMesh(CDC*pDC)
{
  long i;
  Element* element = NULL;
  for(i=0;i<ElListSize();i++) {
    element = ElGetElement(i);
    if(element->aktiv==0)
      continue;
    switch(ElGetElementType(i)) {
      case 1: // lines 
        if(m_iDisplayMSHLine) DrawElement(i,pDC);
        break;
      case 2: // quads
        if(m_bDisplayMSHQuad) DrawElement(i,pDC);
        break;
      case 4: // triangles
        if(m_iDisplayMSHTri) DrawElement(i,pDC);
        break;
      case 6: // prisms
        if(m_bDisplayMSHPris) DrawElement(i,pDC);
        break;
    }
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
09/2005 OK MSH_ELEMENTS_DRAW
**************************************************************************/
void CGraphics::DrawMSHElements(CDC*pDC,CFEMesh*m_fem_msh)
{
  long no_elements;
  CElem* m_ele = NULL;
  if((long)m_fem_msh->ele_vector.size()<=m_lNoElements2Draw)
    no_elements = (long)m_fem_msh->ele_vector.size();
  else
    no_elements = m_lNoElements2Draw;
  for(long i=0;i<no_elements;i++){
    m_ele = m_fem_msh->ele_vector[i];
    DrawMSHElement(pDC,m_ele);
  }
}
/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
ToDo: faster
**************************************************************************/
void CGraphics::DrawMSHElement(CDC*dc,CElem*m_ele)
{
  //----------------------------------------------------------------------
  // Color
  switch(m_ele->GetElementType()) {
    case 1: // Line element
      dc->SelectObject(&BlackBoldPen);
      break;
    case 2: // Quad element
      dc->SelectObject(&RedBoldPen);
      break;
    case 4: // Tri element
      dc->SelectObject(&BlueSolidPen);
      break;
    case 6: // Pris element
      dc->SelectObject(&GreenSolidPen);
      break;
    default:
      dc->SelectObject(&BlackSolidPen);
  }
  //----------------------------------------------------------------------
  if(m_ele->GetPatchIndex()<0){
    dc->SelectObject(&HighlightingPen);
  } 
  //----------------------------------------------------------------------
  // Draw edges
  int j;
  CGLPoint gs_point1;
  CGLPoint gs_point2;
  CNode* m_nod_1; //WWOK
  CNode* m_nod_2; //WWOK
  for(j=0;j<m_ele->GetVertexNumber();j++) {
    m_nod_1 = m_ele->GetNode(j);
    gs_point1.x = m_nod_1->X();
    gs_point1.y = m_nod_1->Y();
    gs_point1.z = m_nod_1->Z();
    if(j==m_ele->GetVertexNumber()-1) { // besser modula
      m_nod_2 = m_ele->GetNode(0);
      gs_point2.x = m_nod_2->X();
      gs_point2.y = m_nod_2->Y();
      gs_point2.z = m_nod_2->Z();
    } 
    else {
      m_nod_2 = m_ele->GetNode(j+1);
      gs_point2.x = m_nod_2->X();
      gs_point2.y = m_nod_2->Y();
      gs_point2.z = m_nod_2->Z();
    } 
    DrawLinePixel(xpixel(gs_point1.x),ypixel(gs_point1.y),xpixel(gs_point2.x),ypixel(gs_point2.y),dc);
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnCbnSelchangeComboMSH()
{
  CFEMesh* m_msh = NULL;
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh)
    return;
/*???
  if(m_msh){
    if(m_msh->selected)
      m_msh->selected = false;
    else
      m_msh->selected = true;
    if(m_msh->selected)
      m_CB_MSH.SetCurSel(m_CB_MSH.GetCurSel());
    else
      m_CB_MSH.SetCurSel(-1);
  }
*/
  if((long)m_msh->ele_vector.size()>MAX_ELE_DRAW)
    m_lNoElements2Draw = MAX_ELE_DRAW; //OK
  else
    m_lNoElements2Draw = (long)m_msh->ele_vector.size(); //OK
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnCbnSelchangeComboPCS()
{
  int i;
  CRFProcess* m_pcs = NULL;
  m_CB_PCS.GetLBText(m_CB_PCS.GetCurSel(),m_strPCSName);
  m_pcs = PCSGet((string)m_strPCSName);
  if(m_pcs){
/*OK
    //....................................................................
    // PCS selection
    if(m_pcs->selected)
      m_pcs->selected = false;
    else
      m_pcs->selected = true;
*/
/*OK
    if(m_pcs->selected)
      m_CB_PCS.SetCurSel(m_CB_MSH.GetCurSel());
    else
      m_CB_PCS.SetCurSel(-1);
*/
    //....................................................................
    // Quantities
    m_CB_Quantities.ResetContent();
    m_CB_Quantities.InitStorage(m_pcs->pcs_number_of_primary_nvals+m_pcs->pcs_number_of_secondary_nvals,80);
    for(i=0;i<m_pcs->pcs_number_of_primary_nvals;i++){
      m_CB_Quantities.AddString((CString)m_pcs->pcs_primary_function_name[i]);
    }
    for(i=0;i<m_pcs->pcs_number_of_secondary_nvals;i++){
      m_CB_Quantities.AddString((CString)m_pcs->pcs_secondary_function_name[i]);
    }
    m_CB_Quantities.SetCurSel(0);
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnLbnSelchangeListFCT()
{
  CFunction* m_fct = NULL;
  m_LB_FCT.GetText(m_LB_FCT.GetCurSel(),m_strFCTName);
  m_fct = FCTGet((string)m_strFCTName);
  if(m_fct){
    m_fct->selected = true;
    m_LB_FCT.SetSel(m_LB_FCT.GetCurSel(),TRUE);
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonSelectAllFCT()
{
  int i;
  CFunction* m_fct = NULL;
  for(i=0;i<(int)fct_vector.size();i++){
    m_fct = fct_vector[i];
    m_fct->selected = true;
  }
  for(i=0;i<(int)m_LB_FCT.GetCount();i++){
    m_LB_FCT.SetSel(i,TRUE);
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonSelectNoneFCT()
{
  int i;
  CFunction* m_fct = NULL;
  for(i=0;i<(int)fct_vector.size();i++){
    m_fct = fct_vector[i];
    m_fct->selected = false;
  }
  for(i=0;i<(int)m_LB_FCT.GetCount();i++){
    m_LB_FCT.SetSel(i,FALSE);
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonFctInterpolate()
{
  int i,j,k;
  CFunction* m_fct = NULL;
  CGLPoint* m_pnt = NULL;
  CFEMesh* m_msh = NULL;
  CGLPoint m_pnt_msh;
  CGLPoint m_pnt_fct;
  double dist = 1.;
  double eps = 1e-3;
  //----------------------------------------------------------------------
  // File handling
  string fct_path;
  CGSProject* m_gsp = GSPGetMember("fct");
  if(!m_gsp)
    return;
  fct_path = m_gsp->path; 
  string fct_file_name;
  fct_file_name = fct_path + "scatter" + ".tec";
  fstream tec_file (fct_file_name.data(),ios::trunc|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  //----------------------------------------------------------------------
  // Header
  tec_file << "TITLE = Scatter of FCT data" << endl;
  tec_file << "VARIABLES = X,Y,Z,V" << endl;
  //----------------------------------------------------------------------
  // Zones
  m_fct = fct_vector[0];
  if(!m_fct)
    return;
  for(i=0;i<(int)fct_vector[0]->variable_data_vector.size();i++){
    tec_file << "ZONE T = " << '"' << m_fct->variable_data_vector[i][0] << '"' << endl;
    for(j=0;j<(int)fct_vector.size();j++){
      m_fct = fct_vector[j];
      m_pnt = GEOGetPointByName(m_fct->geo_name);//CC
      if(m_pnt){
        tec_file << m_pnt->x << " " << m_pnt->y << " " << m_pnt->z << " ";
        tec_file << m_fct->variable_data_vector[i][1] << endl;
      }
    }
  }
  tec_file.close();
  //======================================================================
  string fct_file_name2 = fct_path + "spatial" + ".tec";
  fstream tec_file2 (fct_file_name2.data(),ios::trunc|ios::out);
  tec_file2.setf(ios::scientific,ios::floatfield);
  tec_file2.precision(12);
  //----------------------------------------------------------------------
  // Header
  tec_file2 << "TITLE = Distribution of FCT data" << endl;
  tec_file2 << "VARIABLES = X,Y,Z,V" << endl;
  //----------------------------------------------------------------------
  // Zones
  m_fct = fct_vector[0];
  if(!m_fct)
    return;
  m_msh = FEMGet("PRECIPITATION");
  int no_nodes = 0;
  int no_elements = 0;
  if(m_msh){
    no_nodes = (int)m_msh->nod_vector.size();
    no_elements = (int)m_msh->ele_vector.size();
  }
  else
    return;
  for(i=0;i<(int)fct_vector[0]->variable_data_vector.size();i++){
    tec_file2 << "ZONE T = " << '"' << m_fct->variable_data_vector[i][0] << '"';
    tec_file2 << ", N = " << no_nodes << ", E = " << no_elements << ", F = FEPOINT, ET = TRIANGLE" << endl;
    // Point data
    for(j=0;j<no_nodes;j++){
      tec_file2 << m_msh->nod_vector[j]->X() << " " <<  m_msh->nod_vector[j]->Y() << " " <<  m_msh->nod_vector[j]->Z() << " ";
      m_pnt_msh.x = m_msh->nod_vector[j]->X();
      m_pnt_msh.y = m_msh->nod_vector[j]->Y();
      m_pnt_msh.z = m_msh->nod_vector[j]->Z();
      for(k=0;k<(int)fct_vector.size();k++){
        m_fct = fct_vector[k];
        m_pnt = GEOGetPointByName(m_fct->geo_name);//CC
        m_pnt_msh.z = m_pnt->z;
        dist = m_pnt->PointDis(&m_pnt_msh);
        if(dist<eps){
          tec_file2 << m_fct->variable_data_vector[i][1];
          break;
        }
      }
      if(dist>eps)
        tec_file2 << 0.0;
      tec_file2 << endl;
    }
    // Triangulation
    for(j=0;j<no_elements;j++){
      tec_file2 << m_msh->ele_vector[j]->GetNode(0)->GetIndex() + 1 << " ";
      tec_file2 << m_msh->ele_vector[j]->GetNode(1)->GetIndex() + 1 << " ";
      tec_file2 << m_msh->ele_vector[j]->GetNode(2)->GetIndex() + 1 << endl;
    }
  }
  tec_file2.close();
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMSHNodes(CDC*pDC,CFEMesh*m_fem_msh)
{
  CNode* m_nod = NULL;
  for(long i=0;i<(long)m_fem_msh->nod_vector.size();i++){
    m_nod = m_fem_msh->nod_vector[i];
    if(m_nod->GetMark())
      DrawMSHNode(pDC,m_nod);
  }
}
/**************************************************************************
GUILib-Function: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMSHNode(CDC*pDC,CNode*m_nod)
{
  //----------------------------------------------------------------------
  // Type graphics
/*
  switch(m_ele->ElementType) {
    case 1: // Line element
      dc->SelectObject(&BlackBoldPen);
      break;
    case 2: // Quad element
      dc->SelectObject(&RedBoldPen);
      break;
    case 4: // Tri element
      dc->SelectObject(&BlueSolidPen);
      break;
    case 6: // Pris element
      dc->SelectObject(&GreenSolidPen);
      break;
    default:
      dc->SelectObject(&BlackSolidPen);
  }
*/
  //----------------------------------------------------------------------
  // NOD
  CGLPoint m_pnt;
  m_pnt.x_pix = xpixel(m_nod->X());
  m_pnt.y_pix = ypixel(m_nod->Y());
  DrawPointPixel(pDC,&m_pnt);    
  //----------------------------------------------------------------------
  // NOD label
  CString m_strLabel;
  m_strLabel.Format("%ld",m_nod->GetIndex());
  bool m_bDisplayNODLabels = false;
  if(m_bDisplayNODLabels){
    pDC->TextOut(m_pnt.x_pix,m_pnt.y_pix,m_strLabel);
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawXYPlot(CDC*dc,vector<double>&x,vector<double>&y,int color)
{
  int i;
  double x0=0.0,y0=0.0;
  double x1=0.0,y1=0.0;
  CGLPoint m_point;
  //----------------------------------------------------------------------
  // Graphics
  SelectPenObject(dc,color); 
  //----------------------------------------------------------------------
  if((int)x.size()!=(int)y.size()){
    AfxMessageBox("x-y vectors incompatible");
    return;
  }
  for(i=0;i<(int)x.size();i++) {
    x0 = x[i];
    y0 = y[i];
    m_point.x = x0;
    m_point.y = y0;
    //OK m_point.old_id = i;
    DrawPointOffset(dc,&m_point);
  }
  //----------------------------------------------------------------------
if(max_dim<1) //OKToDo
  if(x.size()>1)
  {
    for(i=0;i<(int)x.size()-1;i++) 
    {
      x0 = x[i];
      y0 = y[i];
      x1 = x[i+1];
      y1 = y[i+1];
      DrawLineOffset(x0,y0,x1,y1,dc);
    }
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
03/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMATFunction(CDC*dc,CMediumProperties*m_mmp,CString m_strMATPropertyName)
{
  int i;
  int phase = 0;
  double x1,y1=0.0,x2,y2=0.0;
  CGLPoint m_pnt;
  //----------------------------------------------------------------------
  dc->SelectObject(&RedSolidPen);
  m_pnt.circle_pix = 3;
  //----------------------------------------------------------------------
  int m_iNX = 100;
  double m_dDX = (m_dXmax-m_dXmin)/double(m_iNX);
  m_mmp->mode = 2;
  for(i=0;i<m_iNX-1;i++){
    x1 = m_dXmin + i*m_dDX;
    x2 = x1 + m_dDX;
    if(m_strMATPropertyName.Compare("CAPILLARY_PRESSURE")==0){
      y1 = m_mmp->CapillaryPressureFunction(0,NULL,0.0,1,x1);
      y2 = m_mmp->CapillaryPressureFunction(0,NULL,0.0,1,x2);
    }
    else if(m_strMATPropertyName.Compare("PERMEABILITY_SATURATION")==0){
      m_mmp->argument = x1;
      y1 = m_mmp->PermeabilitySaturationFunction(0,NULL,0.0,phase);
      m_mmp->argument = x2;
      y2 = m_mmp->PermeabilitySaturationFunction(0,NULL,0.0,phase);
    }
    else if(m_strMATPropertyName.Compare("SATURATION_CAPILLARY_PRESSURE")==0){
      m_mmp->argument = x1;
      y1 = m_mmp->SaturationCapillaryPressureFunction(0,NULL,0.0,phase);
      m_mmp->argument = x2;
      y2 = m_mmp->SaturationCapillaryPressureFunction(0,NULL,0.0,phase);
    }
    else if(m_strMATPropertyName.Compare("D_SATURATION_D_CAPILLARY_PRESSURE")==0){
      m_mmp->argument = x1;
      y1 = m_mmp->SaturationPressureDependency(0,NULL,0.0);
      m_mmp->argument = x2;
      y2 = m_mmp->SaturationPressureDependency(0,NULL,0.0);
    }
    DrawLineOffset(x1,y1,x2,y2,dc);
    m_pnt.x = x1;
    m_pnt.y = y1;
    DrawPointOffset(dc,&m_pnt);
    m_pnt.x = x2;
    m_pnt.y = y2;
    DrawPointOffset(dc,&m_pnt);
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnCbnSelchangeComboQuantities()
{
  double m_dUMin = 1.e+19;
  double m_dUMax = -1.e+19;
  double value;
  long i;
  CRFProcess* m_pcs = NULL;
  //----------------------------------------------------------------------
  if(m_CB_Quantities.GetCount()>0) //CCOK
    m_CB_Quantities.GetLBText(m_CB_Quantities.GetCurSel(),m_strQuantityName);
  //----------------------------------------------------------------------
  m_pcs = PCSGet((string)m_strPCSName);
  if(!m_pcs){ //OK
    AfxMessageBox("No PCS data");
    return;
  }
  if(!m_pcs->m_msh){
    AfxMessageBox("No PCS->MSH data");
    return;
  }
  if(m_pcs){
    int nidx = m_pcs->GetNodeValueIndex((string)m_strQuantityName);
    for(i=0;i<(long)m_pcs->nod_val_vector.size();i++){
      value = m_pcs->GetNodeValue(i,nidx);
      if(value<m_dUMin) m_dUMin = value;
      if(value>m_dUMax) m_dUMax = value;
    }
    m_dUmin = m_dUMin;
    m_dUmax = m_dUMax;
    UpdateData(FALSE);
  }
  else{
    AfxMessageBox("No PCS selected");
  }
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonCalcMinMax()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  //----------------------------------------------------------------------
  double m_dXMin1 = 1.e+19;
  double m_dXMax1 = -1.e+19;
  double m_dYMin1 = 1.e+19;
  double m_dYMax1 = -1.e+19;
  double value;
  //----------------------------------------------------------------------
if((long)gli_points_vector.size()>0){
  for(long i=0;i<(long)gli_points_vector.size();i++) 
  {
    value = gli_points_vector[i]->x;
    if(value<m_dXMin1) m_dXMin1 = value;
    if(value>m_dXMax1) m_dXMax1 = value;
    value = gli_points_vector[i]->y;
    if(value<m_dYMin1) m_dYMin1 = value;
    if(value>m_dYMax1) m_dYMax1 = value;
    //....................................................................
    // Shrink a bit
    m_dXmin = m_dXMin1 - 0.05*(m_dXMax1-m_dXMin1);
    m_dXmax = m_dXMax1 + 0.05*(m_dXMax1-m_dXMin1);
    m_dYmin = m_dYMin1 - 0.05*(m_dYMax1-m_dYMin1);
    m_dYmax = m_dYMax1 + 0.05*(m_dYMax1-m_dYMin1);
    //....................................................................
    theApp->m_dXmin = m_dXmin;
    theApp->m_dXmax = m_dXmax;
    theApp->m_dYmin = m_dYmin;
    theApp->m_dYmax = m_dYmax;
  }
}
  //----------------------------------------------------------------------
  // try MSH data
  else if ((int)fem_msh_vector.size()>0){
    MSHCalcMinMaxMidCoordinates();
    //..................................................................
    m_dXmin = msh_x_min;
    m_dXmax = msh_x_max;
    m_dYmin = msh_y_min;
    m_dYmax = msh_y_max;
    //..................................................................
    theApp->m_dXmin = msh_x_min;
    theApp->m_dXmax = msh_x_max;
    theApp->m_dYmin = msh_y_min;
    theApp->m_dYmax = msh_y_max;
    //theApp->m_dZmin = m_dZmin;
    //theApp->m_dZmax = m_dZmax;
  }
  //----------------------------------------------------------------------
  if(GetSafeHwnd())
    UpdateData(FALSE);
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
07/2005 CC show label
**************************************************************************/
void CGraphics::DrawPoints(CDC*pDC)
{ 
  CGLPoint* m_pnt = NULL;
  for(long i=0;i<(long)gli_points_vector.size();i++){
    m_pnt = gli_points_vector[i];
    m_pnt->x_pix = xpixel(m_pnt->x);
    m_pnt->y_pix = ypixel(m_pnt->y);
    DrawPointPixel(pDC,m_pnt);
     if (m_bDisplayPNTLabels){//CC
       // sprintf(char_label,"%d",m_pnt->id);//CC remove
          pDC->TextOut(m_pnt->x_pix,m_pnt->y_pix,m_pnt->name.c_str());}//CC
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawPCSIsosurfaces(CRFProcess*m_pcs,CDC*pDC)
{ 
  CElem* m_ele = NULL;
  for(long i=0;i<(long)m_pcs->m_msh->ele_vector.size();i++) {
    m_ele = m_pcs->m_msh->ele_vector[i];
    if(m_strQuantityName.IsEmpty())
      AfxMessageBox("No quantity selected");
    else
      DrawIsosurfacesNew(m_ele,m_pcs,pDC);
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnEnChangeEditXMin()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp =(CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->m_dXmin = m_dXmin;
    theApp->g_graphics_modeless_dlg->m_dYmin = m_dXmin*height/width;
    UpdateData(FALSE);
    m_dXmin = theApp->g_graphics_modeless_dlg->m_dXmin;
    m_dYmin = theApp->g_graphics_modeless_dlg->m_dYmin;
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnEnChangeEditXMax()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp =(CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->m_dXmax = m_dXmax;
    theApp->g_graphics_modeless_dlg->m_dYmax = m_dXmax*height/width;
    UpdateData(FALSE);
    m_dXmax = theApp->g_graphics_modeless_dlg->m_dXmax;
    m_dYmax = theApp->g_graphics_modeless_dlg->m_dYmax;
  }
}

//////////////////////////////////////////////////////////////////////////
// NEW

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayMSH()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp =(CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->m_iDisplayMSH = m_iDisplayMSH;
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMSHElementsLabels(CDC*pDC,CFEMesh*m_fem_msh)
{
  CElem* m_ele = NULL;
  for(long i=0;i<(long)m_fem_msh->ele_vector.size();i++){
    m_ele = m_fem_msh->ele_vector[i];
    DrawMSHElementLabel(pDC,m_ele);
  }
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMSHElementLabel(CDC*dc,CElem*m_ele)
{
  CString m_str;
  double* gravity_center = m_ele->GetGravityCenter();
  m_str.Format("%d",m_ele->GetIndex());
  dc->TextOut(xpixel(gravity_center[0]),ypixel(gravity_center[1]),m_str);
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayMSHLabels()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayMSHLabels = m_bDisplayMSHLabels;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplaySFC()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplaySUF = m_iDisplaySUF;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayPLY()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayPLY = m_iDisplayPLY;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayPNT()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayPNT = m_iDisplayPNT;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayIC()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->g_graphics_modeless_dlg->m_bDisplayIC = m_bDisplayIC;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayBC()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayBC = m_iDisplayBC;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayST()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayST = m_bDisplayST;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayIsolines()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayIsolines = m_iDisplayIsolines;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayIsosurfaces()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayIsosurfaces = m_iDisplayIsosurfaces;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnEnChangeEditUmax()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_dUmax = m_dUmax;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CGraphics::OnEnChangeEditUmin()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_dUmin = m_dUmin;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

void CGraphics::OnBnClickedCheckDisplayBMP()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

void CGraphics::OnBnClickedCheckDisplayMATGroups()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonOK()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd(); //OK
  mainframe->m_bIsControlPanelOpen = false; //OK
  OnCancel();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGraphics::OnCancel() 
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd(); //OK
  mainframe->m_bIsControlPanelOpen = false; //OK
  CDialog::OnCancel();
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonDrawMSHAll()
{
  m_strMSHName.Empty();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayPlyPnt()
{
    UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayPLYPoints = m_bDisplayPLYPoints;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayPlyLin()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayPLYLines = m_bDisplayPLYLines;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedShowPlyLabels()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayPLYLabels = m_bDisplayPLYLabels;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedShowPntLabels()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayPNTLabels = m_bDisplayPNTLabels;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGraphics::OnBnClickedShowSfcLabels()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplaySFCLabels = m_bDisplaySFCLabels;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
08/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayMshMat()
{
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplaySFCLabels = m_bDisplayMATGroupsMSH;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
08/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawMSHMaterialGroups(CDC *pDC)
{
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }
  //----------------------------------------------------------------------
  // Generate colors
  CMediumProperties *m_mmp = NULL;
  long r_number;
  for(int m=0;m<(int)mmp_vector.size();m++){
    m_mmp = mmp_vector[m];
    r_number = rand();
    m_mmp->m_color[0] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_mmp->m_color[1] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_mmp->m_color[2] = (long)(r_number/(double)RAND_MAX*256);
  }
  //----------------------------------------------------------------------
  CElem* m_ele = NULL;
  POINT m_arrPoint[8];
  int i;
  int r,g,b;
  for(long e=0;e<(long)m_msh->ele_vector.size();e++){
    m_ele = m_msh->ele_vector[e];
    //....................................................................
    for(i=0;i<m_ele->GetNodesNumber(false);i++) {
      m_arrPoint[i].x = xpixel(m_msh->nod_vector[m_ele->nodes_index[i]]->X());
      m_arrPoint[i].y = ypixel(m_msh->nod_vector[m_ele->nodes_index[i]]->Y());
    }
    //....................................................................
    r=g=b=255;
    m_mmp = m_mmp->GetByGroupNumber(m_ele->GetPatchIndex());
    if(m_mmp&&m_mmp->selected){
      r = m_mmp->m_color[0];
      g = m_mmp->m_color[1];
      b = m_mmp->m_color[2];
      CBrush PolygonBrush;
      PolygonBrush.CreateSolidBrush(RGB(r,g,b));
      CRgn polygon;
      polygon.CreatePolygonRgn(&m_arrPoint[0],m_ele->GetNodesNumber(false),WINDING);
      pDC->SelectObject(PolygonBrush);
      pDC->FillRgn(&polygon,&PolygonBrush);
      DeleteObject(polygon);
      DeleteObject(PolygonBrush);
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
09/2005 OK Implementation, layer volumes
**************************************************************************/
void CGraphics::DrawVolumes(CDC *pDC)
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  if(mainframe->m_strLAYName.GetLength()<5)
    return;
  //----------------------------------------------------------------------
  CGLPolyline* m_ply = NULL;
  Surface* m_sfc = NULL;
  CGLVolume* m_vol = NULL;
  vector<CGLPolyline*>::const_iterator p_ply; 
  vector<Surface*>::const_iterator p_sfc; 
  vector<CGLVolume*>::const_iterator p_vol;
  // un-highlighted
  p_sfc = surface_vector.begin();
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_sfc->highlighted = false;
    ++p_sfc;
  }
  //----------------------------------------------------------------------
  CString m_str = mainframe->m_strLAYName.GetAt(5);
  int layer_selected = strtol(m_str,NULL,0);
  //----------------------------------------------------------------------
  p_vol = volume_vector.begin();
  while (p_vol!= volume_vector.end()){
    m_vol = *p_vol;
    if((m_vol->layer==layer_selected)&&(m_vol->surface_vector.size()>0)){
      p_sfc = m_vol->surface_vector.begin();
      m_sfc = *p_sfc;
      if(m_sfc){
        m_sfc->highlighted = true;
        DrawSurface(pDC,m_sfc,0);
        p_ply = m_sfc->polyline_of_surface_vector.begin();
        m_ply = *p_ply;
      }
      if(m_ply)
        DrawPolyline(pDC,m_ply);
    }
	++p_vol;
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayVOL()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayVOL = m_bDisplayVOL;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayNod()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayNOD = m_bDisplayNOD;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::OnEnChangeEditMshElementsDraw()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  UpdateData(TRUE);
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_lNoElements2Draw = m_lNoElements2Draw;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::OnLbnSelchangeListDOM()
{
  CPARDomain *m_dom = NULL;
  for(int i=0;i<(int)dom_vector.size();i++){
    m_dom = dom_vector[i];
    m_dom->selected = false;
  }
  //----------------------------------------------------------------------
  CString m_str;
  m_LB_DOM.GetText(m_LB_DOM.GetCurSel(),m_str);
  if(m_str.Compare("ALL")==0){
    for(int i=0;i<(int)dom_vector.size();i++){
      m_dom = dom_vector[i];
      m_dom->selected = true;
    }
  }
  else{
    int nSel = m_LB_DOM.GetCurSel();
    m_dom = dom_vector[nSel];
    m_dom->selected = true;
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawDOMElements(CDC*pDC)
{
pDC = pDC;
#ifdef PARALLEL
  int i,k;
  long j;
  CGLPoint m_pnt;
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }
  //----------------------------------------------------------------------
  // Generate colors
  CPARDomain *m_dom = NULL;
  long r_number;
  for(i=0;i<(int)dom_vector.size();i++){
    m_dom = dom_vector[i];
    if(!m_dom->selected)
      continue;
    r_number = rand();
    m_dom->m_color[0] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_dom->m_color[1] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_dom->m_color[2] = (long)(r_number/(double)RAND_MAX*256);
  }
  //----------------------------------------------------------------------
  CPARDomainElement* m_ele_dom = NULL;
  CElem* m_ele = NULL;
  POINT m_arrPoint[8];
  int r,g,b;
  for(i=0;i<(int)dom_vector.size();i++){
    m_dom = dom_vector[i];
    if(!m_dom->selected)
      continue;
    //....................................................................
    CBrush PolygonBrush;
    r = m_dom->m_color[0];
    g = m_dom->m_color[1];
    b = m_dom->m_color[2];
    PolygonBrush.CreateSolidBrush(RGB(r,g,b));
    for(j=0;j<(long)m_dom->elements.size();j++){
      m_ele_dom = m_dom->elements[j];
      m_ele = m_msh->ele_vector[m_ele_dom->global_number];
      //..................................................................
      for(k=0;k<m_ele->GetNodesNumber(false);k++) {
        m_arrPoint[k].x = xpixel(m_msh->nod_vector[m_ele->nodes_index[k]]->X());
        m_arrPoint[k].y = ypixel(m_msh->nod_vector[m_ele->nodes_index[k]]->Y());
      }
      //..................................................................
      CRgn polygon;
      polygon.CreatePolygonRgn(&m_arrPoint[0],m_ele->GetNodesNumber(false),WINDING);
      pDC->SelectObject(PolygonBrush);
      pDC->FillRgn(&polygon,&PolygonBrush);
      DeleteObject(polygon);
    }
    DeleteObject(PolygonBrush);
    //....................................................................
    for(j=0;j<(long)m_dom->nodes_inner.size();j++){
      m_pnt.x = m_msh->nod_vector[m_dom->nodes_inner[j]]->X();
      m_pnt.y = m_msh->nod_vector[m_dom->nodes_inner[j]]->Y();
      m_pnt.x_pix = xpixel(m_pnt.x);
      m_pnt.y_pix = ypixel(m_pnt.y);
      DrawPointPixel(pDC,&m_pnt);
    }
    //....................................................................
    for(j=0;j<(long)m_dom->nodes_halo.size();j++){
      m_pnt.x = m_msh->nod_vector[m_dom->nodes_halo[j]]->X();
      m_pnt.y = m_msh->nod_vector[m_dom->nodes_halo[j]]->Y();
      m_pnt.x_pix = xpixel(m_pnt.x);
      m_pnt.y_pix = ypixel(m_pnt.y);
      m_pnt.highlighted = true;
      DrawPointPixel(pDC,&m_pnt);
    }
  }
#endif
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplayDOM()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  UpdateData(TRUE);
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayDOM = m_bDisplayDOM;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedButtonWriteDomTec()
{
  DOMWriteTecplot((string)m_strMSHName);
}

/**************************************************************************
GUILib-Function: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGraphics::OnCbnSelchangeComboMat()
{
  CString m_strMATName;
  CMediumProperties* m_mmp = NULL;
  //----------------------------------------------------------------------
  for(int i=0;i<(int)mmp_vector.size();i++){
    m_mmp = mmp_vector[i];
    m_mmp->selected = false;
  }
  //......................................................................
  m_CB_MAT.GetLBText(m_CB_MAT.GetCurSel(),m_strMATName);
  if(m_strMATName.Compare("ALL")==0){
    for(int i=0;i<(int)mmp_vector.size();i++){
      mmp_vector[i]->selected = true;
    }
  }
  //......................................................................
  else{
    m_mmp = MMPGet((string)m_strMATName);
    m_mmp->selected = true;
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSysGUI-File: 
Task: 
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedShowSfcMat()
{
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayPLY = m_iDisplayPLY;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI: 
Programing:
12/2004 OK Implementation
**************************************************************************/
void CGraphics::DrawSurfacesValues(CDC*pDC,CRFProcess*m_pcs)
{
  int i;
  Surface* m_sfc = NULL;
  CFEMesh* m_msh = NULL;
  CGLPoint *m_pnt = NULL;
  m_pnt = GEOGetPointByName("PLOT_Z");
  m_iQuantityIndex = m_pcs->GetNodeValueIndex((string)m_strQuantityName)+1; //new time
  double value;
  double rstep;
  double u_int,u_rel;
  CString m_str;
  //----------------------------------------------------------------------
  // Tests
  if(!m_pcs){
    AfxMessageBox("no PCS data");
    return;
  }
  if(m_iQuantityIndex<0){
    AfxMessageBox("no NOD data");
    return;
  }
  //----------------------------------------------------------------------
  // Calc SFC values
  for(i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
m_sfc->mat_group_name = "PROFILE";
m_str.Format("%i",m_sfc->mat_group);
m_sfc->mat_group_name += m_str;
    m_msh = MSHGet(m_pcs->pcs_type_name,m_sfc->mat_group_name);
    if(!m_msh)
      continue;
    //msh_node_number = m_msh->GetNODOnPNT(m_pnt);
    //value = m_pcs->GetNodeValue(msh_node_number,m_iQuantityIndex);
m_dUmax = (double)surface_vector.size();
m_dUmin = 0.0;
value = i;
    //....................................................................
    // Color calculation
    if(m_dUmax-m_dUmin>MKleinsteZahl)
      u_rel = (value-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
    else
      u_rel = MKleinsteZahl;
    modf(u_rel*m_levels,&u_int);
    rstep = u_int * 255.0/m_levels;
    ChooseContourColours(rstep);
    m_sfc->m_color[0] = ContourColourRed;
    m_sfc->m_color[1] = ContourColourGreen;
    m_sfc->m_color[2] = ContourColourBlue;
  }
  //----------------------------------------------------------------------
  // Draw SFC values
  for(i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    m_sfc->display_mode_2d = 2;
    if(m_sfc->mat_group_name.size()>0){
      DrawSurfaceNew(pDC,m_sfc);
    }
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI: 
Programing:
12/2004 OK Implementation
**************************************************************************/
void CGraphics::OnBnClickedCheckDisplaySFCValues()
{  
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplaySFCValues = m_bDisplaySFCValues;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI-Method:
Task: draw surface
Programing:
12/2005 OK based on DrawSurface by CC
**************************************************************************/
void CGraphics::DrawSurfaceNew(CDC*dc,Surface*m_sfc)
{
  //----------------------------------------------------------------------
  // Color
  int r = m_sfc->m_color[0];
  int g = m_sfc->m_color[1];
  int b = m_sfc->m_color[2];
  //----------------------------------------------------------------------
  // Polygon
  POINT m_arrPoint[1024];
  for(int i=0;i<(int)m_sfc->polygon_point_vector.size();i++){
    m_arrPoint[i].x = xpixel(m_sfc->polygon_point_vector[i]->x);
    m_arrPoint[i].y = ypixel(m_sfc->polygon_point_vector[i]->y);
  }
  CRgn polygon;
  polygon.CreatePolygonRgn(&m_arrPoint[0],(int)m_sfc->polygon_point_vector.size(),WINDING);
  CBrush PolygonBrush;
  //----------------------------------------------------------------------
  // Brush type
  switch(m_sfc->display_mode_2d){
    case 0: // highlighted
      PolygonBrush.CreateSolidBrush(RGB(255,204,0));
      break;
    case 1: // hatch
      PolygonBrush.CreateHatchBrush(2,RGB(r,g,b));
      break;
    case 2: // solid
      PolygonBrush.CreateSolidBrush(RGB(r,g,b));
      break;
  }
  dc->SelectObject(PolygonBrush);
  //----------------------------------------------------------------------
  dc->FillRgn(&polygon,&PolygonBrush);
  //----------------------------------------------------------------------
  DeleteObject(polygon);
  DeleteObject(PolygonBrush);
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawSurfacesNew(CDC*pDC)
{
  Surface* m_sfc = NULL;
  //----------------------------------------------------------------------
  if(m_bDisplaySFC_MAT)
    SFCAssignMATColors();
  else
    SFCAssignColors();
  //----------------------------------------------------------------------
  for(int i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    m_sfc->display_mode_2d = 1;
    if(m_sfc->highlighted)
      m_sfc->display_mode_2d = 0;
    if(m_bDisplaySFC_MAT)
      m_sfc->display_mode_2d = 2;
    DrawSurfaceNew(pDC,m_sfc);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGraphics::DrawSurfacesLabels(CDC*pDC)
{
  Surface* m_sfc = NULL;
  int x_pix,y_pix;
  for(int i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    m_sfc->CalcCenterPoint();
    x_pix = xpixel(m_sfc->center_point[0]);
    y_pix = ypixel(m_sfc->center_point[1]);
    if(m_bDisplaySFC_MAT)
      pDC->TextOut(x_pix,y_pix,(CString)m_sfc->mat_group_name.data());
    else
      pDC->TextOut(x_pix,y_pix,(CString)m_sfc->name.data());
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
01/2006 OK Implementation
**************************************************************************/
void CGraphics::SelectPenObject(CDC*dc,int i)
{
  switch(i)
  {
    case 0:
      dc->SelectObject(&RedSolidPen);
      break;
    case 1:
      dc->SelectObject(&BlueSolidPen);
      break;
    case 2:
      dc->SelectObject(&GreenSolidPen);
      break;
    case 3:
      dc->SelectObject(&BlackSolidPen);
      break;
    default:
      dc->SelectObject(&RedSolidPen);
      break;
  }
}