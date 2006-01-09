// GeoSysView.cpp : Implementierung der Klasse CGeoSysView
//
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysPCSView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"
#include "GeoSysOUTView.h"
// RF Objects
#include "rf_p.h"
#include "elements.h"
#include "files.h"
#include "rf_apl.h"
#include "loop_pcs.h"
#include "solver.h"
#include "rf_pcs.h"
#include "femlib.h"
#include "rf_out_new.h"
// Dialogs
#include "gs_graphics.h"
#include "gs_pnt.h"
#include "gs_polyline.h"
#include "gs_bc.h"
#include "gs_prisgen.h"
#include "gs_sfc.h"
#include "gs_st_new.h"
#include "gs_mat_fp.h"
#include "gs_meshing.h"
#include "pcs_dlg.h"
#include "pcs_dlg_new.h"
#include "ic_dlg.h"
#include "ic_dlg_new.h"
#include "mfp_dlg.h"
#include "gs_mat_mp.h"
#include "gs_mat_sp.h"
#include "st_dlg.h"
#include "tim_dlg.h"
#include "out_dlg.h"
// GeoLib
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_dom.h"
// MshLib
#include "msh_lib.h"
#include "msh_nodes_rfi.h"
//OKtm
#include "admin.h"
//OK_4023 using dtm::Dtmesh;
//OK_4023 Dtmesh* gs_dtm;
#include "rfpriref.h"

//For _getcwd(). WW
#include <direct.h>

#include "makros.h"
#include ".\geosysPCSview.h"
//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView

IMPLEMENT_DYNCREATE(CGeoSysView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysView, CGeoSysZoomView)
	// Standard-Druckbefehle
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    // Draw
	ON_COMMAND(ID_BUTTON_DRAW, OnDrawFromToolbar)
    // SIM
	ON_COMMAND(ID_SIMULATOR_PROCESSES, OnSimulatorProcess)
	ON_COMMAND(ID_SIMULATOR_TIMESTEPPING, OnSimulatorTimeStepping)
	ON_COMMAND(ID_SIMULATOR_BOUNDARYCONDITIONS, OnSimulatorBoundaryConditions)
	ON_COMMAND(ID_SIMULATOR_INITIALCONDITIONS, OnSimulatorInitialConditions)
	ON_COMMAND(ID_SIMULATOR_SOURCE_TERM, OnSimulatorSourceTerms)
	ON_COMMAND(ID_MATERIAL_FLUID_DENSITY, OnMaterialFluidDensity)
    ON_COMMAND(ID_MFP_EDITOR, OnMFPEditor)
    ON_COMMAND(ID_MMP_EDITOR, OnMMPEditor)
    ON_COMMAND(ID_MSP_EDITOR, OnMSPEditor)
    // Mouse
	//ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_SIMULATOR_RUNSIMULATION, OnSimulatorRunSimulation)
    ON_COMMAND(ID_VIEW_FEM_PROPERTIES, OnProperties) //CC 05/2004
    // Zoom
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin) //CC 05/2004
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
    ON_COMMAND(ID_PCSLIB_OUT, OnPCSLibOUT)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
09/2003 OK Implementation
**************************************************************************/
CGeoSysView::CGeoSysView()
{
  CGLDomain *m_domain=NULL;
  m_domain = m_domain->Get("DOMAIN");
  if(m_domain) {
    m_dXmin = m_domain->x_min;
    m_dXmax = m_domain->x_max;
    m_dYmin = m_domain->y_min;
    m_dYmax = m_domain->y_max;
  }
  else {
    m_dXmin = 0.;
    m_dXmax = 1.;
    m_dYmin = 0.;
    m_dYmax = 1.;
  }
  m_strNamePolyline = "";
  m_strNamePoint = "";
  m_iDisplayMSH = FALSE;
  m_bDisplayIsosurfaces = FALSE;
  m_iQuantityIndex = 0;
  add_geometry = FALSE;
  m_dUmin = 0.;
  m_dUmax = 1.;
  m_iDisplayPNT = FALSE;
  m_iDisplayPLY = FALSE;
  m_iDisplayMSH = FALSE;
  m_iDisplayBC = FALSE;
  m_iDisplayIsolines = FALSE;
  m_iDisplayIsosurfaces = FALSE;
  m_bDisplayMAT = FALSE;
  m_iDisplaySFC = 0;
  m_bShowLabels = false;
  m_bDisplayST = false;
}

CGeoSysView::~CGeoSysView()
{

}

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
06/2005 OK Properties Dialog wieder raus
**************************************************************************/
void CGeoSysView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
  // Properties dialog
  //OK OnProperties();
}

BOOL CGeoSysView::PreCreateWindow(CREATESTRUCT& cs)
{
  // Ändern Sie hier die Fensterklasse oder das Erscheinungsbild, 
  // indem Sie CREATESTRUCT cs modifizieren.
  return CView::PreCreateWindow(cs);
}

/**************************************************************************
GeoSys-GUI Function OnProperties
Programing:
05/2004 CC Implementation
**************************************************************************/
void CGeoSysView::OnProperties()
{
  CGraphics m_graphics;
  m_graphics.m_strPCSName = m_strPCSName;
  m_graphics.m_strQuantityName = m_strQuantityName;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_iDisplayPLY = m_iDisplayPLY;
  m_graphics.m_iDisplaySUF = m_iDisplaySFC;
  m_graphics.m_iDisplayBC = m_iDisplayBC;
  m_graphics.m_bDisplayTINs = m_bDisplayTINs;
  m_graphics.m_bDisplayVOL = m_bDisplayVOL;
  m_graphics.m_iDisplayPNT = m_iDisplayPNT;
  m_graphics.m_bShowLabels = m_bShowLabels;
  m_graphics.m_bDisplayPLYPoints = m_bDisplayPLYPoints;
  m_graphics.m_bDisplayPLYLines = m_bDisplayPLYLines;
  m_graphics.m_iDisplayMSH = m_iDisplayMSH;
  m_graphics.m_iDisplayIsosurfaces = m_iDisplayIsosurfaces;
  m_graphics.m_iDisplayIsolines = m_iDisplayIsolines;
  m_graphics.m_strQuantityName = m_strQuantityName;
  m_graphics.m_dUmin = m_dUmin;
  m_graphics.m_dUmax = m_dUmax;
  m_graphics.m_bDisplayST = m_bDisplayST;
  m_graphics.m_strMSHName = m_strMSHName;

  m_graphics.DoModal();

  m_strPCSName = m_graphics.m_strPCSName;
  m_dXmin = m_graphics.m_dXmin;
  m_dXmax = m_graphics.m_dXmax;
  m_dYmin = m_graphics.m_dYmin;
  m_dYmax = m_graphics.m_dYmax;
  m_iDisplayPLY = m_graphics.m_iDisplayPLY;
  m_iDisplaySFC = m_graphics.m_iDisplaySUF;
  m_iDisplayBC = m_graphics.m_iDisplayBC;
  m_bDisplayTINs = m_graphics.m_bDisplayTINs;
  m_bDisplayVOL = m_graphics.m_bDisplayVOL;
  m_iDisplayPNT = m_graphics.m_iDisplayPNT;
  m_bShowLabels = m_graphics.m_bShowLabels;
  m_bDisplayPLYPoints = m_graphics.m_bDisplayPLYPoints;
  m_bDisplayPLYLines = m_graphics.m_bDisplayPLYLines;
  m_strQuantityName = m_graphics.m_strQuantityName;
  m_iDisplayMSH = m_graphics.m_iDisplayMSH;
  m_iDisplayIsosurfaces = m_graphics.m_iDisplayIsosurfaces;
  m_iDisplayIsolines = m_graphics.m_iDisplayIsolines;
  m_dUmin = m_graphics.m_dUmin;
  m_dUmax = m_graphics.m_dUmax;
  m_bDisplayST = m_graphics.m_bDisplayST;
  m_strMSHName = m_graphics.m_strMSHName;

  Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Zeichnen
/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
09/2003 OK/CC Implementation
**************************************************************************/
void CGeoSysView::OnDraw(CDC* pDC)
{
  //-----------------------------------------------------------------------
  pDC->TextOut(0,0," PCSView - ");
  //-----------------------------------------------------------------------
  long i;
  CGraphics m_graphics;
  GetWindowAttributes(this->m_hWnd, &width, &height);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    AfxMessageBox("Open ControlPanel for PCS View applications");
    return;
  }
  //--------------------------------------------------------------------
  // Coordinates
  CGLDomain *m_domain=NULL;
  m_domain = m_domain->Get("DOMAIN");
  if(m_domain) {
    m_dXmin = m_domain->x_min;
    m_dXmax = m_domain->x_max;
    m_dYmin = m_domain->y_min;
    m_dYmax = m_domain->y_max;
  }
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_graphics.m_dXmax - m_graphics.m_dXmin);
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_graphics.m_dYmax - m_graphics.m_dYmin);
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_iQuantityIndex = m_iQuantityIndex;
  m_graphics.m_dUmin = m_dUmin;
  m_graphics.m_dUmax = m_dUmax;
  m_graphics.m_bShowLabels = m_bShowLabels;
  m_graphics.m_strQuantityName = m_strQuantityName;
  m_graphics.m_iDisplayIsolines = m_iDisplayIsolines;
  m_graphics.m_strMSHName = m_strMSHName;
  //CFont *pOldFont = NULL;
  //pOldFont = pDC->SelectObject(pOldFont);
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPNT){
    m_dXmin = theApp->g_graphics_modeless_dlg->m_dXmin;
    m_dXmax = theApp->g_graphics_modeless_dlg->m_dXmax;
    m_dYmin = theApp->g_graphics_modeless_dlg->m_dYmin;
    m_dYmax = theApp->g_graphics_modeless_dlg->m_dYmax;
  }
  //=======================================================================
  // GEOLib
  //-----------------------------------------------------------------------
  CGLPolyline *m_polyline = NULL;
  CGLPoint *m_point = NULL;
  CGLPoint m_point1;
  Surface *m_surface = NULL;
  //-----------------------------------------------------------------------
  // Surfaces
  //......................................................................
  if(theApp->g_graphics_modeless_dlg->m_iDisplaySUF){
    pDC->TextOut(0,0," PCSView: PNT");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
	theApp->g_graphics_modeless_dlg->DrawSurfaces(pDC);
  }
  //-----------------------------------------------------------------------
  // Polylines
  //......................................................................
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPLY){
    pDC->TextOut(0,0," PCSView: PLY");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
	theApp->g_graphics_modeless_dlg->DrawPolylines(pDC);
  }
  //-----------------------------------------------------------------------
  // Points
  //......................................................................
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPNT){
    pDC->TextOut(0,0," PCSView: PNT");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
	theApp->g_graphics_modeless_dlg->DrawPoints(pDC);
  }
  //=======================================================================
  // MSHLib
  //-----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  if(m_iDisplayMSH){
    m_msh = FEMGet((string)m_strMSHName);
    if(m_msh){
      m_graphics.DrawMSHElements(pDC,m_msh);
      //m_graphics.DrawMSHNodes(pDC,m_msh);
    }
  }
  //......................................................................
  if(theApp->g_graphics_modeless_dlg->m_iDisplayMSH){
    pDC->TextOut(0,0," PCSLib: MSH");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    CFEMesh* m_msh = NULL;
    //m_msh = FEMGet((string)theApp->g_graphics_modeless_dlg->m_strMSHName);
    m_msh = FEMGet((string)m_strPCSName);
    if(m_msh){
      theApp->g_graphics_modeless_dlg->DrawMSHElements(pDC,m_msh);
      if(theApp->g_graphics_modeless_dlg->m_bDisplayNOD)
        theApp->g_graphics_modeless_dlg->DrawMSHNodes(pDC,m_msh);
      if(theApp->g_graphics_modeless_dlg->m_bDisplayMSHLabels)
        theApp->g_graphics_modeless_dlg->DrawMSHElementsLabels(pDC,m_msh);
    }
  }
  //=======================================================================
  // PCSLib
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)theApp->g_graphics_modeless_dlg->m_strPCSName);
  //--------------------------------------------------------------------
  // MAT groups
  if(theApp->g_graphics_modeless_dlg->m_bDisplayMaterialGroups){
    pDC->TextOut(0,0," PCSView: MAT groups");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    theApp->g_graphics_modeless_dlg->DrawMaterialGroups(pDC);
  }
  //--------------------------------------------------------------------
  // Isosurfaces
  //OK if (m_iDisplayIsosurfaces&&!(pcs_vector.size()==0)) {
  if(theApp->g_graphics_modeless_dlg->m_iDisplayIsosurfaces){
   if(NodeListLength>0)
    for(i=0;i<ElListSize();i++) {
      //OK m_graphics.DrawIsosurfaces(i,pDC);
      theApp->g_graphics_modeless_dlg->DrawIsosurfaces(i,pDC);
    }
  }
  if(theApp->g_graphics_modeless_dlg->m_iDisplayIsosurfaces){
    pDC->TextOut(0,0," PCSView: Isosurfaces");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    if(m_pcs) //OK
      if(m_pcs->m_msh)
        theApp->g_graphics_modeless_dlg->DrawPCSIsosurfaces(m_pcs,pDC);
  }
  //--------------------------------------------------------------------
  // Isolines
  if (m_iDisplayIsolines) {
   if(NodeListLength>0&&!(pcs_vector.size()==0))
    pDC->SelectObject(&m_graphics.IsolineFont);
    m_graphics.DrawIsolines(pDC,NULL); //OK
  }
  //-----------------------------------------------------------------------
  // Boundary conditions
  CBoundaryCondition *m_bc = NULL;
  if (m_iDisplayBC) {
    list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
    while(p_bc!=bc_list.end()) {
      m_bc = *p_bc;
      //...................................................................
      if(m_bc->geo_type_name.compare("POINT")==0) {
        //m_point = m_point->GetByName(m_bc->geo_name);
        m_point = GEOGetPointByName(m_bc->geo_name);//CC
        m_point->x_pix = xpixel(m_point->x);
        m_point->y_pix = ypixel(m_point->y);
        m_graphics.DrawPointPixel(pDC,m_point);
      }
      //...................................................................
      if(m_bc->geo_type_name.compare("POLYLINE")==0) {
        m_polyline = GEOGetPLYByName(m_bc->geo_name);//CC
        if(m_polyline) {
          m_graphics.DrawPolyline(pDC,m_polyline);
          m_point = m_polyline->CenterPoint();
          if(m_point)
            pDC->TextOut(xpixel(m_point->x),ypixel(m_point->y),m_polyline->name.c_str());
        }
      }
      //...................................................................
      if(m_bc->geo_type_name.compare("SURFACE")==0) {
        m_surface = GEOGetSFCByName(m_bc->geo_name);//CC
        if(m_surface) {
         // list<CGLPolyline*>::const_iterator p_sfc_ply = m_surface->polyline_of_surface_list.begin();
     vector<CGLPolyline*>::iterator p_sfc_ply = m_surface->polyline_of_surface_vector.begin();//CC 08/2005//CC
          while(p_sfc_ply!=m_surface->polyline_of_surface_vector.end()) {
            m_polyline = *p_sfc_ply; 
            if(m_polyline) {
              m_graphics.DrawPolyline(pDC,m_polyline);
              m_point = m_polyline->CenterPoint();
              if(m_point) {
                pDC->TextOut(xpixel(m_point->x),ypixel(m_point->y),m_polyline->name.c_str());
              }
            }
            ++p_sfc_ply;
          }
        }
      }
      //...................................................................
      ++p_bc;
    }
  }
  //-----------------------------------------------------------------------
  if (m_iDisplayBC||theApp->g_graphics_modeless_dlg->m_iDisplayBC){ //OK
    CBoundaryConditionsGroup* m_bc_group = NULL;
    CBoundaryConditionNode *m_node = NULL;
    if(theApp->g_graphics_modeless_dlg->m_iDisplayBC) //OK
      m_strPCSName = theApp->g_graphics_modeless_dlg->m_strPCSName;
    m_bc_group = m_bc_group->Get((string)m_strPCSName);
    CFEMesh* m_msh = FEMGet((string)m_strPCSName);
    if(m_bc_group&&m_msh){ //OK
      //Info
      CString m_strInfo;
      CString m_strLabel;
      m_strInfo.Format("%d",(long)m_bc_group->group_vector.size());
      //pDC->TextOut(0,0,m_strInfo);
      for (i=0;i<(long)m_bc_group->group_vector.size();i++) {
        m_node = m_bc_group->group_vector[i];
        if(m_node->msh_node_number>=0) {
          m_point1.x_pix = xpixel(m_msh->nod_vector[m_node->msh_node_number]->X()); //xpixel(GetNodeX(m_node->msh_node_number));
          m_point1.y_pix = ypixel(m_msh->nod_vector[m_node->msh_node_number]->Y()); //ypixel(GetNodeY(m_node->msh_node_number));
          m_graphics.DrawPointPixel(pDC,&m_point1);
          if(m_bShowLabels){//OK
            m_strLabel.Format("%5.3e", m_node->node_value);
            pDC->TextOut(m_point1.x_pix,m_point1.y_pix,m_strLabel);
          }
        }
      }
    }
  }
  //-----------------------------------------------------------------------
  if (m_bDisplayST||theApp->g_graphics_modeless_dlg->m_bDisplayST){ //OK
    CSourceTermGroup* m_st_group = NULL;
    CNodeValue *m_node = NULL;
    if(theApp->g_graphics_modeless_dlg->m_bDisplayST){ //OK
      m_strPCSName = theApp->g_graphics_modeless_dlg->m_strPCSName;
      m_strQuantityName = theApp->g_graphics_modeless_dlg->m_strQuantityName;
    }
    CFEMesh* m_msh = FEMGet((string)m_strPCSName);
    m_st_group = m_st_group->Get((string)m_strQuantityName);
    m_st_group = STGetGroup((string)m_strPCSName,(string)m_strQuantityName);
    for (i=0;i<(long)m_st_group->group_vector.size();i++) {
      m_node = m_st_group->group_vector[i];
      if(m_node->msh_node_number>=0) {
      
        if(m_msh){
          m_point1.x_pix = xpixel(m_msh->nod_vector[m_node->msh_node_number]->X());
          m_point1.y_pix = ypixel(m_msh->nod_vector[m_node->msh_node_number]->Y());
          m_graphics.DrawPointPixel(pDC,&m_point1);
        }
      }
    }
  }
  //--------------------------------------------------------------------
  // Isosurfaces
  if(theApp->g_graphics_modeless_dlg->m_bDisplaySFCValues){
    pDC->TextOut(0,0," PCSView: SFC values");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    if(m_pcs) //OK
      theApp->g_graphics_modeless_dlg->DrawSurfacesValues(pDC,m_pcs);
  }
  //-----------------------------------------------------------------------
  //CString m_strViewInfo = "FEMView: " + m_strQuantityName;
  //pDC->SelectObject(pOldFont);
  //pDC->SelectObject(&m_graphics.FontArial14);
  //pDC->TextOut(0,0,m_strViewInfo);
}
void CGeoSysView::OnDrawFromToolbar() 
{
  //CC please check
  Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysView::AssertValid() const
{

	CView::AssertValid();
}

void CGeoSysView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Nachrichten-Handler

//************************************************************************
// FEM Interface
//************************************************************************
//------------------------------------------------------------------------
// PCS
void CGeoSysView::OnSimulatorProcess() 
{
  CGeoSysDoc *m_pDoc = GetDocument();
  m_pDoc->m_bDataPCS = false;
  CDialogPCS m_pcs;
  m_pcs.DoModal();
  m_pDoc->m_bDataPCS = true;
}

//------------------------------------------------------------------------
// TIM
void CGeoSysView::OnSimulatorTimeStepping() 
{
  CDialogTimeDiscretization m_dlg;
  m_dlg.DoModal();
}
//------------------------------------------------------------------------
// NUM
//------------------------------------------------------------------------
// IC
void CGeoSysView::OnSimulatorInitialConditions() 
{
/*
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
*/
  //CDialogInitialConditionsNew m_ic_dlg;
  CDialogInitialConditions m_ic_dlg;
  m_ic_dlg.DoModal();
}
//------------------------------------------------------------------------
// BC
void CGeoSysView::OnSimulatorBoundaryConditions() 
{
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
  CBoundaryConditions m_bc;
  m_bc.DoModal();
}
//------------------------------------------------------------------------
// ST
/**************************************************************************
GeoSys-GUI: 
Programing:
12/2003 OK Implementation
03/2004 OK new dialog
last modified: 
**************************************************************************/
void CGeoSysView::OnSimulatorSourceTerms() 
{
  CDialogSourceTerms m_dlg;
  m_dlg.DoModal();
/*
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
  CGeoSysDoc* m_pDoc = GetDocument();
  CSourceTerms m_st;
  m_st.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_st.DoModal();
*/
}
//------------------------------------------------------------------------
// MFP
/**************************************************************************
GUILib:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CGeoSysView::OnMFPEditor()
{
  CDialogMFP m_dlg;
  m_dlg.DoModal();
}

void CGeoSysView::OnMaterialFluidDensity() 
{
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
  CMaterialFluidDensity m_mat_fp_density;
  CGeoSysDoc* m_pDoc = GetDocument();
  m_mat_fp_density.m_dXmin = m_pDoc->m_dXmin;
  m_mat_fp_density.m_dXmax = m_pDoc->m_dXmax;
  m_mat_fp_density.m_dDensityMin = m_pDoc->m_dYmin;
  m_mat_fp_density.m_dDensityMax = m_pDoc->m_dYmax;
  if (m_mat_fp_density.DoModal()==IDOK){
    m_pDoc->m_dXmin = m_mat_fp_density.m_dXmin;
    m_pDoc->m_dXmax = m_mat_fp_density.m_dXmax;
    m_pDoc->m_dYmin = m_mat_fp_density.m_dDensityMin;
    m_pDoc->m_dYmax = m_mat_fp_density.m_dDensityMax;
    m_pDoc->m_iFluidPhase = m_mat_fp_density.m_iFluidPhase;
  }
}
//------------------------------------------------------------------------
// MSP
/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
01/2004 WW Implementation
**************************************************************************/
void CGeoSysView::OnMSPEditor()
{
  MAT_Mech_dlg MMech_dlg;
  MMech_dlg.DoModal();
}
//------------------------------------------------------------------------
// MMP
/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGeoSysView::OnMMPEditor()
{
  CMATGroupEditor m_dialog;
  CGeoSysDoc* m_pDoc = GetDocument();
  m_dialog.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_dialog.DoModal();
  Invalidate(FALSE);
}
//------------------------------------------------------------------------
// MCP
//------------------------------------------------------------------------
// SIM
void CGeoSysView::OnSimulatorRunSimulation() 
{
  if(!NODListExists()||!ELEListExists()) {
    AfxMessageBox("No MSH data !");
    return;
  }
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
      CGeoSysDoc* pDoc = GetDocument();
      POSITION pos = pDoc->GetFirstViewPosition();
      while(pos!=NULL) {
        CView* pView = pDoc->GetNextView(pos);
        pView->UpdateWindow();
      }
      pDoc->SetModifiedFlag(1);
      pDoc->UpdateAllViews(NULL,0L,NULL);
}

//===========================================================================
// Coordinate transformation: windows (pixel) to world (real) coordinates
// 10/2002   OK   Implementation
//
void CGeoSysView::WorldCoordinateTransformation(RF_POINT* rf_point, CPoint point)
{
  CGraphics m_graphics;
  
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_dXmax-m_dXmin);
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_dYmax-m_dYmin);

  // Coordinate transformation
  MFC_setcor(m_dXmin,m_dXmax,m_dYmin,m_dYmax, width, height);

  rf_point->x = (x_real(point.x,width) - m_graphics.m_dDX) \
              * (m_dXmax-m_dXmin)/ \
                (m_dXmax-m_dXmin-2.*m_graphics.m_dDX) \
              + m_dXmin;
  rf_point->y = (y_real(point.y,height) - m_graphics.m_dDY - m_dYmin) \
              * (m_dYmax-m_dYmin)/ \
                (m_dYmax-m_dYmin-2.*m_graphics.m_dDY) \
              + m_dYmin;
  rf_point->z = 0.0;
}


// CGraphics
int CGeoSysView::xpixel(double x)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);

  return (int)(xminp + x * skalex);
}      	  	                                     

int CGeoSysView::ypixel(double y)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return (int)( yminp - y * skaley);
}      	  	                                     

/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysView::OnViewZoomin()
{
	// Toggle zoomin moden
	CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == MODE_ZOOMIN) {
		SetZoomMode(MODE_ZOOMOFF);
		// Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(MODE_ZOOMIN);
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click to zoom in on point or drag a zoom box.");
	}
} // OnViewZoomin


/**************************************************************************
GeoSys-GUI: OnViewZoomout
Programing:User pressed the ZOOM OUT toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysView::OnViewZoomout()
{
	// Toggle zoomout mode
	CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == MODE_ZOOMOUT) {
		SetZoomMode(MODE_ZOOMOFF);
		// Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(MODE_ZOOMOUT);
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click to zoom out on point.");
	}
} // OnViewZoomout


/**************************************************************************
GeoSys-GUI: OnViewZoomfull
Programing:User pressed the ZOOM FULL toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

//************************************************************************
// Mouse
//************************************************************************

/**************************************************************************
GeoSys-GUI: OnRButtonDown
Programing: right button control stopping drawing point or polyline
05/2004 CC Modification put drawing + zooming function together
**************************************************************************/
void CGeoSysView::OnRButtonDown(UINT nFlags, CPoint point)
{

    if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
    {
    CGeoSysZoomView::OnRButtonDown(nFlags, point);
    }
    else {
	
    }

    
	CView::OnRButtonDown(nFlags, point);
}
/**************************************************************************
GeoSys-GUI: 
Programing:
05/2004 CC Last Modification put drawing and zooming function together
**************************************************************************/
void CGeoSysView::OnMouseMove(UINT nFlags, CPoint point)
{
  if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
  {
    CGeoSysZoomView::OnMouseMove(nFlags, point);
  }
  else {

  }
  CView::OnMouseMove(nFlags, point);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CGeoSysView::OnPCSLibOUT()
{
  CDialogOUT m_dlg;
  m_dlg.DoModal();
}
