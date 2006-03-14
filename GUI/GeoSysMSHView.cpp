// GeoSysMSHView.cpp : Implementierung der Klasse GeoSysMSHView
//
#include "stdafx.h"
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysMSHView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"
#include "rf_p.h" // GetWindowAttributes
#include "gs_project.h"
// RF Objects
extern long NodeListLength;
extern long ElListSize();
#include "rfpriref.h"
#include "solver.h"
extern void RFConfigRenumber(void);
#include "rf_pcs.h"
extern void RFPre_Model();
// Dialogs
#include "gs_sfc.h"
#include "gs_graphics.h"
#include "gs_mat_mp.h"
#include "gs_meshing.h"
#include "gs_prisgen.h"
#include "admin.h"
#include "dlg_rsm.h"
//OK_4023 using dtm::Dtmesh;
//OK_4023 Dtmesh* gs_dtm_msh;
#include ".\geosysmatgroupsview.h"
#include ".\geosysmshview.h"
// GeoLib
#include "geo_sfc.h"
#include "geo_vol.h"
// MshLib
#include "msh_lib.h"
#include "fem_ele.h"
//geosyszoomview
#include ".\geosyszoomview.h"

//file path
#include<direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysMSHView

IMPLEMENT_DYNCREATE(CGeoSysMSHView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysMSHView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_MSH_PROPERTIES, OnProperties)
    ON_COMMAND(ID_MSHLIB_MAT_CONNECT, OnMshLibMatConnect)
    ON_COMMAND(ID_MSHLIB_REFINE, OnMshLibRefine)
	ON_COMMAND(ID_MSHLIB_TRIANGULATION, OnMshLibTriangulation)
	ON_COMMAND(ID_MSHLIB_PRISGEN_TRI2PRI, OnMeshlibPrisgen)
	ON_COMMAND(ID_MSHLIB_PRISGEN_MAPPING, OnMeshlibPrisgenMapping)
	ON_COMMAND(ID_MSHLIB_PRISGEN_VERTICALDISC, OnMeshlibPrisgenVertDisc)
    ON_COMMAND(ID_MSHLIB_TETGEN, OnMshLibTetgen)
    ON_COMMAND(ID_PRISGEN_DELETE, OnPrisgenDelete)
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
    ON_COMMAND(ID_MSHLIB_MSHEDITOR, OnMSHLibEditor)
    ON_COMMAND(ID_LINEGEN_POLYLINES, OnLineGenPolylines)
    ON_COMMAND(ID_LINEGEN_TRIANGLES, OnLineGenTriangles)
    ON_COMMAND(ID_MSHLIB_HEXGEN, OnMSHLibHexGen)
    ON_COMMAND(ID_LINEGEN_QUADS, OnMSHLibLineGenFromQuads)
    ON_COMMAND(ID_QUADGEN_POLYLINES, OnQuadGenPolylines)
    ON_COMMAND(ID_QUADGEN_SURFACES, OnQuadGenSurfaces)
    ON_COMMAND(ID_LINEGEN_SURFACE, OnLineGenSurface)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysMSHView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
01/2004 OK Implementation
**************************************************************************/
CGeoSysMSHView::CGeoSysMSHView()
{
  m_iDisplayPNT = 0;//CC
  m_iDisplayPLY = 0;
  m_iDisplaySFC = 0;
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_iDisplayMSH = 0;
  m_iDisplayMSHLine = 0;
  m_iDisplayMSHTri = 0;
  m_bDisplayMaterialGroups = false;
  m_bDisplayMSHQuad = false;
  m_bDisplayMSHPris = false;
  m_bDisplayPLYPoints = false;
  m_bDisplayPLYLines = false;
  m_bShowLabels = false;
  m_bDisplayNOD = false;
  m_bDisplayMSHDoubleElements = false;
  m_bMSHActive1D = true;
  if(msh_no_line>0) m_bMSHActivateLine = true;
  if(msh_no_tris>0) m_bMSHActivateTri = true;
  if(msh_no_quad>0) m_bMSHActivateQuad = true;
  if(msh_no_tets>0) m_bMSHActivateTet = true;
  if(msh_no_pris>0) m_bMSHActivatePris = true;
  if(msh_no_hexs>0) m_bMSHActivateHex = true;
}

CGeoSysMSHView::~CGeoSysMSHView()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsMSHViewOpen = false; //OK
}

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
04/2004 CC Implementation
09/2004 OK Properties Dialog
06/2005 OK Properties Dialog wieder raus
**************************************************************************/
void CGeoSysMSHView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
  // Properties dialog
  //OK OnProperties();
  // Create the LOGICAL font to draw with (only once!)
/*
  int height = (m_totalLog.cy / 10) / 5;  // 1/5 of box size
  m_font = new CFont();
  m_font->CreateFont(height, 0, 0, 0,
         FW_NORMAL, FALSE, FALSE,
         FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
         DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial");
*/
}

BOOL CGeoSysMSHView::PreCreateWindow(CREATESTRUCT& cs)
{
	// ZU ERLEDIGEN: Ändern Sie hier die Fensterklasse oder das Erscheinungsbild, indem Sie
	//  CREATESTRUCT cs modifizieren.
	return CView::PreCreateWindow(cs);
}

/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
04/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnViewZoomin()
{
	// Toggle zoomin mode
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
04/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnViewZoomout()
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
04/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
04/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
04/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

// CGraphics
int CGeoSysMSHView::xpixel(double x)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);

  return (int)(xminp + x * skalex);
}      	  	                                     

int CGeoSysMSHView::ypixel(double y)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return (int)( yminp - y * skaley);
}      	  	                                     

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Zeichnen
//###########################################################################
// Draw
//###########################################################################

/**************************************************************************
GeoSys-GUI Function
Programing:
01/2004 OK Implementation
04/2004 CC Modification   Add Zoom function
01/2005 CC Modification display prism
07/2005 OK MSH selection
10/2005 OK DOM
**************************************************************************/
void CGeoSysMSHView::OnDraw(CDC* pDC)
{
  CString m_strInfo = " MSHView - ";
  //-----------------------------------------------------------------------
  pDC->TextOut(0,0,m_strInfo);
  //-----------------------------------------------------------------------
  long i;
  GetWindowAttributes(this->m_hWnd,&width,&height);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    AfxMessageBox("Open ControlPanel for MSH View applications");
    return;
  }
  //----------------------------------------------------------------------
  CGraphics m_graphics;
  //m_graphics.width = width;
  //m_graphics.height = height;
  //m_graphics.width  = m_totalDev.cx ;
  //m_graphics.height = m_totalDev.cy;
  m_graphics.width  = m_totalLog.cx ;
  m_graphics.height = m_totalLog.cy;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_bShowLabels = m_bShowLabels;
  m_graphics.m_strPCSName = m_strPCSName;
  m_graphics.m_strMSHName = m_strMSHName;
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!=NULL){
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
  }
  //----------------------------------------------------------------------
  // BMP OK
  //----------------------------------------------------------------------
  CGeoSysDoc *m_pDoc = GetDocument();
  HBITMAP hBitmap;
  if(theApp->g_graphics_modeless_dlg->m_bDisplayBMP){
    CGSProject* m_gsp = GSPGetMember("bmp");
    if(m_gsp){
      //OK CString bmp_file_name = m_gsp->base.c_str();
      CString bmp_file_name = m_gsp->path.c_str(); //OK/TK
      bmp_file_name += m_gsp->base.c_str();
      bmp_file_name += ".bmp";
      hBitmap = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(),
                                            bmp_file_name, IMAGE_BITMAP, 0, 0,
                                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
      if (hBitmap){
        if (m_pDoc->m_bmpBitmap.DeleteObject()) // letztes Bitmap löschen
          m_pDoc->m_bmpBitmap.Detach();         // letztes Bitmap lösen
        // Aktuell geladenes Bitmap mit Bitmap-Objekt verbinden
        m_pDoc->m_bmpBitmap.Attach(hBitmap);
      }
      //..................................................................
      if(hBitmap){ //OK
        BITMAP bm;
        m_pDoc->m_bmpBitmap.GetBitmap(&bm);
        // Geladenes Bitmap holen
        CDC dcMem;                          // Gerätekontext erzeugen, in den Bitmap geladen wird
        dcMem.CreateCompatibleDC(pDC);      // Bitmap in den kompatiblen Gerätekontext selektieren
        //CBitmap* pOldBitmap = (CBitmap*)dcMem.SelectObject(m_pDoc->m_bmpBitmap);
        dcMem.SelectObject(m_pDoc->m_bmpBitmap);
        // Anzeigebereich
        CRect lRect;
        GetClientRect(lRect);
        lRect.NormalizeRect();
        // Bitmap anzeigen
         // fixed size
        // m_pDoc->m_bmpBitmap.SetBitmapDimension(lRect.Width()*12,lRect.Height()*12);
        // 07/2004 CC add displacment and scale factor of x and y for georeferencing transportation
        pDC->StretchBlt(0,0,lRect.Width(),lRect.Height(),&dcMem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
      }
    }
  }
  //--------------------------------------------------------------------
  // DOM
  if(theApp->g_graphics_modeless_dlg->m_bDisplayDOM) {
    pDC->TextOut(0,0," MSHView: DOM");
    theApp->g_graphics_modeless_dlg->DrawDOMElements(pDC);
  }
  //-----------------------------------------------------------------------
  // MAT
  if (theApp->g_graphics_modeless_dlg->m_bDisplayMaterialGroups) {
    pDC->TextOut(0,0," MSHView: MAT");
    theApp->g_graphics_modeless_dlg->DrawMSHMaterialGroups(pDC);
  }
  //-----------------------------------------------------------------------
  // GEOLib
  //......................................................................
  // Surfaces
  if(theApp->g_graphics_modeless_dlg->m_iDisplaySUF){
    pDC->TextOut(0,0," MSHView: SFC");
	theApp->g_graphics_modeless_dlg->DrawSurfaces(pDC);
  }
  //......................................................................
  // Polylines
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPLY){
    pDC->TextOut(0,0," MSHView: PLY");
	theApp->g_graphics_modeless_dlg->DrawPolylines(pDC);
  }
  //......................................................................
  // Points
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPNT){
    pDC->TextOut(0,0," MSHView: PNT");
	theApp->g_graphics_modeless_dlg->DrawPoints(pDC);
  }
  //--------------------------------------------------------------------
  // Mesh
  CFEMesh* m_msh = NULL;
  if(m_iDisplayMSH){
    m_msh = FEMGet((string)m_strMSHName);
    if(m_msh){
      m_graphics.DrawMSHElements(pDC,m_msh);
      m_graphics.DrawMSHNodes(pDC,m_msh);
    }
  }
  //--------------------------------------------------------------------
  //
  if (m_bDisplayNOD){
	m_graphics.DrawNodes(pDC);
  }
  //......................................................................
  if(theApp->g_graphics_modeless_dlg->m_iDisplayMSH){
    m_strInfo = " MSHLib: ";
    CFEMesh* m_msh = NULL;
    if(theApp->g_graphics_modeless_dlg->m_strMSHName.IsEmpty()){
      m_strInfo += " All";
      pDC->TextOut(0,0,m_strInfo);
      for(i=0;i<(int)fem_msh_vector.size();i++){
        m_msh = fem_msh_vector[i];
        theApp->g_graphics_modeless_dlg->DrawMSHElements(pDC,m_msh);
        if(theApp->g_graphics_modeless_dlg->m_bDisplayNOD)
          theApp->g_graphics_modeless_dlg->DrawMSHNodes(pDC,m_msh);
        if(theApp->g_graphics_modeless_dlg->m_bDisplayMSHLabels)
          theApp->g_graphics_modeless_dlg->DrawMSHElementsLabels(pDC,m_msh);
      }
    }
    else{
      m_msh = FEMGet((string)theApp->g_graphics_modeless_dlg->m_strMSHName);
      if(m_msh){
        m_strInfo += theApp->g_graphics_modeless_dlg->m_strMSHName;
        pDC->TextOut(0,0,m_strInfo);
        theApp->g_graphics_modeless_dlg->DrawMSHElements(pDC,m_msh);
        if(theApp->g_graphics_modeless_dlg->m_bDisplayNOD)
          theApp->g_graphics_modeless_dlg->DrawMSHNodes(pDC,m_msh);
        if(theApp->g_graphics_modeless_dlg->m_bDisplayMSHLabels)
          theApp->g_graphics_modeless_dlg->DrawMSHElementsLabels(pDC,m_msh);
      }
    }
  }
  //--------------------------------------------------------------------
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysMSHView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysMSHView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysMSHView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysMSHView::AssertValid() const
{
	CGeoSysZoomView::AssertValid();
}

void CGeoSysMSHView::Dump(CDumpContext& dc) const
{
	CGeoSysZoomView::Dump(dc);
}

CGeoSysDoc* CGeoSysMSHView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Nachrichten-Handler

//###########################################################################
// Menus
//###########################################################################

void CGeoSysMSHView::OnProperties()
{
  CGraphics m_graphics;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_iDisplayPLY = m_iDisplayPLY;
  m_graphics.m_iDisplayPNT = m_iDisplayPNT;
  m_graphics.m_iDisplaySUF = m_iDisplaySFC;
  m_graphics.m_iDisplayMSH = m_iDisplayMSH;
  m_graphics.m_iDisplayMSHLine = m_iDisplayMSHLine;
  m_graphics.m_iDisplayMSHTri = m_iDisplayMSHTri;
  m_graphics.m_bDisplayMSHQuad = m_bDisplayMSHQuad;
  m_graphics.m_bDisplayMSHPris = m_bDisplayMSHPris;
  m_graphics.m_bDisplayPLYPoints = m_bDisplayPLYPoints;
  m_graphics.m_bDisplayPLYLines = m_bDisplayPLYLines;
  m_graphics.m_bShowLabels = m_bShowLabels;
  m_graphics.m_bDisplayNOD = m_bDisplayNOD;
  m_graphics.m_bDisplayMSHDoubleElements = m_bDisplayMSHDoubleElements;
  m_graphics.m_bDisplayMaterialGroups = m_bDisplayMaterialGroups;
  m_graphics.m_strMSHName = m_strMSHName;

  m_graphics.DoModal();

  m_dXmin = m_graphics.m_dXmin;
  m_dXmax = m_graphics.m_dXmax;
  m_dYmin = m_graphics.m_dYmin;
  m_dYmax = m_graphics.m_dYmax;
  m_iDisplayPNT = m_graphics.m_iDisplayPNT;
  m_iDisplayPLY = m_graphics.m_iDisplayPLY;
  m_iDisplaySFC = m_graphics.m_iDisplaySUF;
  m_iDisplayMSH = m_graphics.m_iDisplayMSH;
  m_iDisplayMSHLine = m_graphics.m_iDisplayMSHLine;
  m_iDisplayMSHTri = m_graphics.m_iDisplayMSHTri;
  m_bDisplayMSHQuad = m_graphics.m_bDisplayMSHQuad;
  m_bDisplayMSHPris = m_graphics.m_bDisplayMSHPris;
  m_bDisplayPLYPoints = m_graphics.m_bDisplayPLYPoints;
  m_bDisplayPLYLines = m_graphics.m_bDisplayPLYLines;
  m_bShowLabels = m_graphics.m_bShowLabels;
  m_bDisplayNOD = m_graphics.m_bDisplayNOD;
  m_bDisplayMSHDoubleElements = m_graphics.m_bDisplayMSHDoubleElements;
  m_bDisplayMaterialGroups = m_graphics.m_bDisplayMaterialGroups;
  m_strPCSName = m_graphics.m_strPCSName;
  m_strMSHName = m_graphics.m_strMSHName;

  Invalidate(FALSE);
}

//###########################################################################
// MSH
/**************************************************************************
GUILib-Method: 
Task:
Programing:
09/2003 OK Implementation
08/2004 CC Modification file path is passed to create geo,rfi,msh files
04/2005 OK MSH data and file
07/2005 OK gmsh in GeoSys.exe path
11/2005 OK Select SFC for meshing
**************************************************************************/
void CGeoSysMSHView::OnMshLibTriangulation() 
{
  CGSProject* m_gsp = NULL;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  //-----------------------------------------------------------------------
  m_gsp = GSPGetMember("gli");
  //======================================================================
  if(m_gsp){
    //--------------------------------------------------------------------
    // File handling
    CString m_strProjectPath = m_gsp->path.data();
    CString m_strProjectBase = m_gsp->base.data();
    CString m_strProjectPathBase = m_strProjectPath + m_strProjectBase;
    CString m_strFileNameGmshGEO = m_strProjectPathBase + "_gmsh" + ".geo";                          
    CString m_strFileNameGmshMSH = m_strProjectPathBase + "_gmsh" + ".msh";
    CString m_strFileNameMSH = m_strProjectPathBase + ".msh";
    //--------------------------------------------------------------------
    // Delete old gmsh files
    CString m_strExecute;
    m_strExecute = "del " + m_strFileNameGmshGEO;
    system(m_strExecute);
    m_strExecute = "del " + m_strFileNameGmshMSH;
    system(m_strExecute);
    //--------------------------------------------------------------------
    FILE *gmsh_geo_file=NULL;
    gmsh_geo_file = fopen(m_strFileNameGmshGEO, "w+t");
    FILE *gmsh_msh_file=NULL;
    char file_name_gmsh[256];
    strcpy(file_name_gmsh,m_strFileNameGmshMSH);
    //--------------------------------------------------------------------
    // Select SFC for meshing
    CMSHSurfaces m_dlg;
    m_dlg.DoModal();
    GEOSurfaceTopology();
    GEOCreateSurfacePointVector(); //OK
    //--------------------------------------------------------------------
    // Convert PLY 2 GEO file
    // Write SFC 2 GEO file
    GEOPolylineGLI2GEO(gmsh_geo_file);
    fclose(gmsh_geo_file);
    // Call mesh generator
    //--------------------------------------------------------------------
    // gmsh call
    //....................................................................
    CString m_strGmshExePath = theApp->m_strAPLFilePath;
    m_strExecute = m_strGmshExePath;
    m_strExecute += "gmsh " + m_strFileNameGmshGEO + " -2";
    system(m_strExecute);
    AfxMessageBox(m_strExecute);
    gmsh_msh_file = fopen(file_name_gmsh,"r");
    //....................................................................
    if(!gmsh_msh_file) { //OK
      m_strExecute = m_strProjectPath;
      m_strExecute += "gmsh " + m_strFileNameGmshGEO + " -2";
      system(m_strExecute);
      AfxMessageBox(m_strExecute);
    }
    gmsh_msh_file = fopen(file_name_gmsh,"r");
    //....................................................................
    if(!gmsh_msh_file){ //OK
      if(!gmsh_geo_file)
        AfxMessageBox("gmsh failed to mesh");
      else
        AfxMessageBox("gmsh.exe not found");
      return;
    }
    else{
      fclose(gmsh_msh_file); //OK
    }
    //--------------------------------------------------------------------
    // Write RFI file
    //--------------------------------------------------------------------
    // MSH data
    if(gmsh_msh_file){
      CFEMesh*m_msh = NULL;
      CFEMesh*m_msh_check = NULL;
      //..................................................................
      // Ckeck exisiting MSH
      m_msh = FEMGet((string)m_strMSHName);    
      if(m_msh){
        MSHDelete((string)m_strMSHName);
      }
      //..................................................................
      // Create MSH data
      m_msh = new CFEMesh();
      if(m_strMSHName.IsEmpty()){
        int no_msh = (int)fem_msh_vector.size();
        CString m_strNoMSH;
        m_strNoMSH.Format("%i",no_msh);
        m_msh->pcs_name = "TRIANGLES" + m_strNoMSH;
        m_msh_check = FEMGet(m_msh->pcs_name); //OK
        if(m_msh_check)
          m_msh->pcs_name += "NEW";
      }
      else{
        m_msh->pcs_name = (string)m_strMSHName;
      }
      GMSH2MSH(file_name_gmsh,m_msh);
      msh_no_tris = (long)m_msh->ele_vector.size(); //OK
      m_msh->ele_type = 4;
      fem_msh_vector.push_back(m_msh);
      //..................................................................
      // Write MSH data
      //OK FEMWrite((string)m_strFileNameBase);
      //..................................................................
      // GSP data //OK
      string msh_base_type = m_gsp->base + ".msh"; //OK
      GSPAddMember(msh_base_type);
      //------------------------------------------------------------------
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
      if(fem_msh_vector.size()>0){
        theApp->g_graphics_modeless_dlg->m_iDisplayMSH = true;
        theApp->g_graphics_modeless_dlg->m_iDisplayMSHTri= true;
        RFPre_Model(); //OK
        start_new_elems = ElListSize(); //OK
        ConfigTopology(); //OK
      }
      //..................................................................
      else{
        theApp->g_graphics_modeless_dlg->m_iDisplayMSH = false;
        theApp->g_graphics_modeless_dlg->m_iDisplayMSHTri= false;
      }
      theApp->g_graphics_modeless_dlg->OnInitDialog();
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->OnDrawFromToolbar();
    }
  //======================================================================
  }
  else{
    AfxMessageBox("No GEO data");
    return;
  }
  //-----------------------------------------------------------------------
  // Draw mesh
  UpdateWindow();
}

/**************************************************************************
GeoSys-GUI: 
Programing:
12/2003 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnLineGenPolylines()
{
  CMSHPolylines m_dialog;
  m_dialog.type = 1;
  m_dialog.DoModal();
  // Draw mesh
  Invalidate(FALSE);
}

/**************************************************************************
GeoSys-GUI: 
Programing:
04/2005 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMSHLibLineGenFromQuads()
{
  CPRISGEN m_prisgen;
  m_prisgen.m_strMSHType = "LINE";
  m_prisgen.DoModal();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2003 TK/MB Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMeshlibPrisgen() 
{
  CPRISGEN m_prisgen;
  m_prisgen.DoModal();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2003 TK/MB Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMeshlibPrisgenMapping() 
{
  CGeoSysDoc* m_pDoc = GetDocument();
  CPrisGenMap m_mapping;
  m_mapping.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_mapping.DoModal();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
11/2003 TK/MB Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMeshlibPrisgenVertDisc() 
{
  CPrisGenVertDisc m_vertdisc;
  m_vertdisc.DoModal();
}

void CGeoSysMSHView::OnPrisgenDelete()
{
}

//TETGEN 
void CGeoSysMSHView::OnMshLibTetgen() 
{
//OKtm     
//OK_4023  gs_dtm_msh->Mesh();
}

/**************************************************************************
GUILib-Method: 
Task:
Programing:
04/2005 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMSHLibHexGen() 
{
  CPRISGEN m_prisgen;
  m_prisgen.m_strMSHType = "HEX";
  m_prisgen.DoModal();
}

//###########################################################################
// MAT
void CGeoSysMSHView::OnMshLibMatConnect()
{
  MSHAssignMATGroup2Elements();
  CGeoSysDoc *m_pDoc = GetDocument();
  m_strFileNameBase = m_pDoc->m_strGSPFileBase;
//OK_MMP  MMPWriteTecplot((string)m_strFileNameBase);
  Invalidate(FALSE);
}

/**************************************************************************
GeoSysGUI-Function: MSHAssignMATGroup2Elements
Programing:
01/2003 OK Implementation
01/2004 OK Pixel variant
ToDo for Renumber
**************************************************************************/
void CGeoSysMSHView::MSHAssignMATGroup2Elements(void)
{
  long j;
  int nn,k;
  double x,y,z;
  long *element_nodes;
  CGLVolume *m_volume = NULL;
  CGLPoint m_point;
  CGLPolyline *m_polyline = NULL;
  Surface *m_surface = NULL;
CDC* pDC =  GetDC( );
char counter[10];
  long l,no_ply_nodes;
  long *ply_nodes = NULL;

  //-----------------------------------------------------------------------
  CMediumProperties *m_mat_mp = NULL;
  int m;
  int no_mat_mp = (int)mmp_vector.size();
  for(m=0;m<no_mat_mp;m++){
    m_mat_mp = mmp_vector[m];
    pDC->TextOut(0,0,m_mat_mp->geo_name.c_str());
    //.....................................................................
    if(m_mat_mp->geo_type_name.compare("POLYLINE")==0){
      m_polyline = GEOGetPLYByName(m_mat_mp->geo_name);//CC
      ply_nodes = MSHGetNodesClose(&no_ply_nodes,m_polyline);//CC
      for(j=0;j<ElListSize();j++){
sprintf(counter,"%ld",j);
pDC->TextOut(0,15,counter);
        if((ElGetElementType(j)==1)){ // Lines
          element_nodes = ElGetElementNodes(j);
          for(k=0;k<no_ply_nodes;k++) {
            if(element_nodes[0]==ply_nodes[k]){
              for(l=0;l<no_ply_nodes;l++) {
                if(element_nodes[1]==ply_nodes[l]){
                  ElSetElementGroupNumber(j,m_mat_mp->number);
                }
              }
            }
          }
        } 
      }
    }
    //.....................................................................
    if(m_mat_mp->geo_type_name.compare("SURFACE")==0){
      for(j=0;j<ElListSize();j++){
sprintf(counter,"%ld",j);
pDC->TextOut(0,15,counter);
        if((ElGetElementType(j)==2)||(ElGetElementType(j)==4)){ // Tri or Quads
          element_nodes = ElGetElementNodes(j);
          nn = ElNumberOfNodes[ElGetElementType(j)-1];
          x=0.0; y=0.0; z=0.0;
          for(k=0;k<nn;k++) {
            x += GetNodeX(element_nodes[k]);
            y += GetNodeY(element_nodes[k]);
            z += GetNodeZ(element_nodes[k]);
          }
          x /= double(nn);
          y /= double(nn);
          z /= double(nn);
          m_point.x = x;
          m_point.y = y;
          m_point.z = z;
          m_surface = GEOGetSFCByName(m_mat_mp->geo_name);//CC
          if(m_surface){
            if(IsPointInSurface(m_surface,&m_point)){//CC
              ElSetElementGroupNumber(j,m_mat_mp->number);
            }
          }
        } 
      }
    }
    //.....................................................................
    if(m_mat_mp->geo_type_name.compare("VOLUME")==0){
      for(j=0;j<ElListSize();j++){
        if((ElGetElementType(j)==3)||(ElGetElementType(j)==5)||(ElGetElementType(j)==6)){
sprintf(counter,"%ld",j);
pDC->TextOut(0,15,counter);
          element_nodes = ElGetElementNodes(j);
          nn = ElNumberOfNodes[ElGetElementType(j)-1];
          x=0.0; y=0.0; z=0.0;
          for(k=0;k<nn;k++) {
            x += GetNodeX(element_nodes[k]);
            y += GetNodeY(element_nodes[k]);
            z += GetNodeZ(element_nodes[k]);
          }
          x /= double(nn);
          y /= double(nn);
          z /= double(nn);
          m_point.x = x;
          m_point.y = y;
          m_point.z = z;
          //list<CGLVolume*>::const_iterator p_vol;
          vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC
         // p_vol = volume_vector.begin();
          while(p_vol!=volume_vector.end()) {
            m_volume = *p_vol;
            if(m_volume->PointInVolume(&m_point,0)){
              ElSetElementGroupNumber(j,m_volume->mat_group);
            }
            ++p_vol;
          }
        } 
      }
    }
  }

/*  
  //-----------------------------------------------------------------------
  for(i=0;i<ElListSize();i++) {
sprintf(counter,"%ld",i);
pDC->TextOut(0,0,counter);
    //.....................................................................
    // Element center point
    element_nodes = ElGetElementNodes(i);
    nn = ElNumberOfNodes[ElGetElementType(i)-1];
    x=0.0; y=0.0; z=0.0;
    for(j=0;j<nn;j++) {
      x += GetNodeX(element_nodes[j]);
      y += GetNodeY(element_nodes[j]);
      z += GetNodeZ(element_nodes[j]);
    }
    x /= double(nn);
    y /= double(nn);
    z /= double(nn);
    m_point.x = x;
    m_point.y = y;
    m_point.z = z;
    //.....................................................................
    // Element type
    list<CGLVolume*>::const_iterator p_vol;
    list<Surface*>::const_iterator p_sfc;
    list<CGLPolyline*>::const_iterator p_ply;
    switch(ElGetElementType(i)){
      case 1: // line elements
          // inherited from polyline
        break;
      case 2: // quad elements
          // vertically extended lines: inherited from polyline
        break;
      case 3: // hex elements
        break;
      case 4: // tri elements
	    p_sfc = surface_vector.begin();
	    while (p_sfc!= surface_vector.end()){
          m_surface = *p_sfc;
          if(m_surface->type==24) // MAT type
            if(m_surface->PointInSurface(&m_point)){
              ElSetElementGroupNumber(i,m_surface->mat_group);
            }
          ++p_sfc;
	    }
        break; 
      case 5: // tet elements
        break;
      case 6: // prism elements
	    p_vol = volume_vector.begin();
	    while (p_vol!= volume_vector.end()){
          m_volume = *p_vol;
          if(m_volume->type==24) // MAT type
            if(m_volume->PointInVolume(&m_point,0)){
              ElSetElementGroupNumber(i,m_volume->mat_group);
              break;
            }
          ++p_vol;
	    }
        break;
    }
    //--------------------------------------------------------------------
  } // Element loop
  //----------------------------------------------------------------------
*/
}

/**************************************************************************
GeoSys-GUI: 
Programing:
11/2003 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMshLibRefine()
{
//  CMSHRefine m_msh_refine;
//  m_msh_refine.DoModal();
//   AprioriRefineElements ("APRIORI_REFINE_ELEMENT");
}

/**************************************************************************
GeoSys-GUI: 
Programing:
05/2004 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnMSHLibEditor()
{
  CMSHEditor m_dialog;

  m_dialog.m_bMSHActivateLine = m_bMSHActivateLine;
  m_dialog.m_bMSHActivateTri = m_bMSHActivateTri;
  m_dialog.m_bMSHActivateQuad = m_bMSHActivateQuad;
  m_dialog.m_bMSHActivateTet = m_bMSHActivateTet;
  m_dialog.m_bMSHActivatePris = m_bMSHActivatePris;
  m_dialog.m_bMSHActivateHex = m_bMSHActivateHex;

  m_dialog.DoModal();

  m_bMSHActivateLine = m_dialog.m_bMSHActivateLine;
  m_bMSHActivateTri = m_dialog.m_bMSHActivateTri;
  m_bMSHActivateQuad = m_dialog.m_bMSHActivateQuad;
  m_bMSHActivateTet = m_dialog.m_bMSHActivateTet;
  m_bMSHActivatePris = m_dialog.m_bMSHActivatePris;
  m_bMSHActivateHex = m_dialog.m_bMSHActivateHex;
}

/**************************************************************************
GeoSys-GUI: 
Programing:
05/2005 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnQuadGenPolylines()
{
  CMSHPolylines m_dialog;
  m_dialog.type = 2;
  m_dialog.DoModal();
  Invalidate(FALSE);
}

/**************************************************************************
GeoSys-GUI: 
Programing:
05/2005 OK Implementation
06/2005 OK modeless dialog
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnQuadGenSurfaces()
{
/*
  CMSHSurfaces m_dialog = NULL;
  if (m_dialog.DoModal()==IDOK) { 
    CGeoSysDoc* pDoc = GetDocument();
    GSPAddMember((string)pDoc->m_strGSPFileBase + ".msh");
  }
*/
  CMSHSurfaces* m_dialog = NULL;
  m_dialog = new CMSHSurfaces;
  m_dialog->Create(IDD_MSH_SURFACES);
  m_dialog->ShowWindow(SW_SHOW);
}


/**************************************************************************
GeoSys-GUI: 
Programing:
12/2005 OK Implementation
last modified: 
**************************************************************************/
void CGeoSysMSHView::OnLineGenSurface()
{
  CRegionalSoilModel m_dlg;
  m_dlg.DoModal();
}

/**************************************************************************
GeoSys-GUI: 
01/2005 OK/YD Implementation
02/2005 OK Bugfix
02/2006 OK MSH concept
**************************************************************************/
void CGeoSysMSHView::OnLineGenTriangles() //OK41
{
  CFEMesh* m_msh_this = NULL;
  m_msh_this = FEMGet((string)m_strMSHName);
  //m_msh_this = FEMGet("GROUNDWATER_FLOW");
  if(!m_msh_this)
  {
    AfxMessageBox("no MSH data");
    return;
  }
  m_msh_this->CreateLineELEFromTri();
}
/* old version
void CGeoSysMSHView::OnLineGenTriangles() //OK41
{
  int j;
  long i;
  long* nodes = NULL;
  double x,y,z;
  CGLLine* m_line = NULL;
  Knoten* node = NULL;
  //----------------------------------------------------------------------
  // Data required
  double length = 5.0;
  int no_elements = 5;
  int no_nodes = no_elements + 1;
  //----------------------------------------------------------------------
  // Surfacte triangle loop
  for(i=0;i<ElListSize();i++){
    // Select triangles
    if(ElGetElementType(i)==4){
      nodes = ElGetElementNodes(i);
      x=y=z=0.0;
      for(j=0;j<3;j++){
        x += GetNodeX(nodes[j]);
        y += GetNodeY(nodes[j]);
        z += GetNodeZ(nodes[j]);
      }
      x /= 3.;
      y /= 3.;
      z /= 3.;
      //...................................................................
      // Generating lines
      for(j=0;j<no_elements+1;j++){
        node = NewNode();
        node->x = x;
        node->y = y;
        node->z = z - j*(length/(double)no_nodes); //OK
        AddNode(node);
      }
      //...................................................................
      // Generating lines
      for(j=0;j<no_elements;j++){
        m_line = new CGLLine();
        m_line->no_msh_nodes = 2;
        m_line->msh_nodes = new long[2];
        m_line->msh_nodes[0] = NodeListSize()- no_elements - 1 + j;
        m_line->msh_nodes[1] = NodeListSize()- no_elements + j;
        CreateMSHLines(m_line);//CC
      }
      //...................................................................
    } // Triangles
  } // ElListSize
  //----------------------------------------------------------------------
}
*/
