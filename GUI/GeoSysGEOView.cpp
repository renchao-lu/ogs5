// GeoSysGEOView.cpp : Implementierung der Klasse GeoSysGEOView
//
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysGEOView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"
#include "rf_p.h" // GetWindowAttributes
// RF Objects
extern long NodeListLength;
extern long ElListSize();
// Dialogs
#include "gs_pnt.h"
#include "gs_polyline.h"
#include "gs_polyline_new.h"//CC9999
#include "gs_sfc.h"
#include "vol_dlg.h"
#include "gs_graphics.h"
#include ".\geosysmatgroupsview.h"
#include ".\geosysmshview.h"
// GeoLib
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
// MshLib
#include "msh_elements_rfi.h"
#include ".\geosysgeoview.h"
//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING

#include "gs_project.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView

IMPLEMENT_DYNCREATE(CGeoSysGEOView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysGEOView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_GEO_PROPERTIES, OnProperties)
	ON_COMMAND(ID_GEOLIB_POINTS, OnGeoLibPoints)
	ON_COMMAND(ID_GEOLIB_POLYLINES, OnGeolibPolylines)
	ON_COMMAND(ID_GEOLIB_SURFACES, OnGeoLibSurfaces)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    //05/2004
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
    // 06/2004 CC
   
    ON_COMMAND(ID_GEOLIB_VOLUMES, OnGeolibVolumes)
    ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysGEOView()
Programing:
01/2004 OK Implementation
01/2005 CC Modification m_iDisplayPLY = 1;
**************************************************************************/
CGeoSysGEOView::CGeoSysGEOView()
{
  //----------------------------------------------------------------------
  // Display flags
  m_iDisplayPNT = 0;
  m_bDisplayPNTDouble = true;
  m_iDisplayPLY = 0;
  m_iDisplaySFC = 0;
  m_bDisplayTINs = false;
  m_bDisplayVOL = false;
  m_iDisplayBC = 0;
  nSelPoint = 0;
  //----------------------------------------------------------------------
  // Coordinates
  CalcXYMinMax();
  //----------------------------------------------------------------------
  // Labels
  m_bShowLabels = false;
  m_strNamePoint = "";
  m_strNamePolyline = "";
  add_geometry = false;
  m_bDisplayPLYPoints = false;
  m_bDisplayPLYLines = false;
}

/**************************************************************************
GeoSys-GUI Function 
Programing:
06/2005 CC Implementation
**************************************************************************/
CGeoSysGEOView::~CGeoSysGEOView()
{
  //CC_MODELESS
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->deleteMember();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd(); //OK
  mainframe->m_bIsGEOViewOpen = false; //OK
}

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
01/2005 CC Modification m_SHP
06/2005 OK Properties Dialog raus
**************************************************************************/
void CGeoSysGEOView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc *m_pDoc = GetDocument(); //OK
  m_strGEOFileBase = m_pDoc->m_strGSPFileBase; //OK41
  GSPAddMember((string)m_strGEOFileBase + ".gli");	//OK
  //----------------------------------------------------------------------
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(m_pDoc->GetDocSize());
  //----------------------------------------------------------------------
  // Set min, max of window
  if(m_SHP){
    double m_dXMin1 = 1.e+19;
    double m_dXMax1 = -1.e+19;
    double m_dYMin1 = 1.e+19;
    double m_dYMax1 = -1.e+19;
    double value;
    long i;
    //....................................................................
    // Min/Max of GEO x and y
    for(i=0;i<(long)gli_points_vector.size();i++) 
    {
      value = gli_points_vector[i]->x;
      if(value<m_dXMin1) m_dXMin1 = value;
      if(value>m_dXMax1) m_dXMax1 = value;
      value = gli_points_vector[i]->y;
      if(value<m_dYMin1) m_dYMin1 = value;
      if(value>m_dYMax1) m_dYMax1 = value;
    }
    m_dXmin = m_dXMin1;
    m_dXmax = m_dXMax1;
    m_dYmin = m_dYMin1;
    m_dYmax = m_dYMax1;
    Invalidate(FALSE);
  } //SHP
}

BOOL CGeoSysGEOView::PreCreateWindow(CREATESTRUCT& cs)
{
  return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView Zeichnen
//###########################################################################
// Draw
//###########################################################################

/*************************************************************************
GeoSys-GUI Function
Programing:
01/2004 OK Implementation
05/2004 CC Modification
06/2005 OK g_graphics_modeless_dlg
07/2005 OK/TK BMP path
07/2005 CC Display points,lines, label from Ply
10/2005 OK UpdateView idea
*************************************************************************/
void CGeoSysGEOView::OnDraw(CDC* pDC)
{   
  //----------------------------------------------------------------------
  pDC->TextOut(0,0," GEOView -");
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bModifiedGEOView = false; // nach CMainFrame::OnDrawFromToolbar
  mainframe->m_bModifiedGEOView = true; // nach modification
  if(!mainframe->m_bModifiedGEOView)
    return;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    AfxMessageBox("Open ControlPanel for GEO View applications");
    return;
  }
  //----------------------------------------------------------------------
  GetWindowAttributes(this->m_hWnd,&width,&height);
  CGeoSysDoc *m_pDoc = GetDocument();
  //----------------------------------------------------------------------
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    m_dXmin = theApp->g_graphics_modeless_dlg->m_dXmin;
    m_dXmax = theApp->g_graphics_modeless_dlg->m_dXmax;
    m_dYmin = theApp->g_graphics_modeless_dlg->m_dYmin;
    m_dYmax = theApp->g_graphics_modeless_dlg->m_dYmax;
  }
  //----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_bShowLabels = m_bShowLabels;
  //----------------------------------------------------------------------
  // BMP OK
  //----------------------------------------------------------------------
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
  //-----------------------------------------------------------------------
  // GEO
  //......................................................................
  // VOL
  if(theApp->g_graphics_modeless_dlg->m_bDisplayVOL){
    pDC->TextOut(0,0," GEOView: VOL");
	theApp->g_graphics_modeless_dlg->DrawVolumes(pDC);
  }  
  //......................................................................
  // SFC
  if(theApp->g_graphics_modeless_dlg->m_iDisplaySUF){
    pDC->TextOut(0,0," GEOView: SFC");
	//OK theApp->g_graphics_modeless_dlg->DrawSurfaces(pDC);
	theApp->g_graphics_modeless_dlg->DrawSurfacesNew(pDC);
    if(theApp->g_graphics_modeless_dlg->m_bDisplaySFCLabels){
      theApp->g_graphics_modeless_dlg->DrawSurfacesLabels(pDC);
    }  
  }  
  //......................................................................
  // PLY
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPLY){
    pDC->TextOut(0,0," GEOView: PLY");
	theApp->g_graphics_modeless_dlg->DrawPolylines(pDC);
    if (theApp->g_graphics_modeless_dlg->m_bDisplayPLYPoints)                           
      theApp->g_graphics_modeless_dlg->DrawPolylinesPoints(pDC);             
    if (theApp->g_graphics_modeless_dlg->m_bDisplayPLYLines)                            
      theApp->g_graphics_modeless_dlg->DrawPolylinesLines(pDC);
  } //CC
  //......................................................................
  // PNT
  if(theApp->g_graphics_modeless_dlg->m_iDisplayPNT){
    pDC->TextOut(0,0," GEOView: PNT");
	theApp->g_graphics_modeless_dlg->DrawPoints(pDC);
  }
  //-----------------------------------------------------------------------
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView Drucken

BOOL CGeoSysGEOView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysGEOView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysGEOView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView Diagnose

#ifdef _DEBUG
void CGeoSysGEOView::AssertValid() const
{
	CView::AssertValid();
}

void CGeoSysGEOView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysGEOView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysGEOView Nachrichten-Handler
//###########################################################################
// Menus
//###########################################################################

void CGeoSysGEOView::OnProperties()
{
  CGraphics m_graphics;
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
  
  m_graphics.DoModal();
  
  m_dXmin = m_graphics.m_dXmin; //OK
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
  Invalidate(FALSE);
}

// CGraphics
void CGeoSysGEOView::OnGeoLibPoints() 
{
  CGSPoint m_point;
  m_point.m_Pointname = m_strNamePoint;
  m_point.DoModal();
  m_strNamePoint = m_point.m_Pointname;
  add_geometry = m_point.add_point;
  UpdateData(FALSE);
}

void CGeoSysGEOView::OnGeolibPolylines() 
{
  CPolyline m_polyline;
  m_polyline.m_strNamePolyline = m_strNamePolyline;
  m_polyline.m_strFileNameBase = m_strGEOFileBase;
  m_polyline.DoModal();
  m_strNamePolyline = m_polyline.m_strNamePolyline;
  add_geometry = m_polyline.add_polyline;
}

//---------------------------------------------------------------------------
// GeoSys-GUI View Function
// Programing:
//  11/2003 OK Implementation
void CGeoSysGEOView::OnGeoLibSurfaces() 
{
  CGSSurface m_surface;
  CGeoSysDoc* m_pDoc = GetDocument();
  m_surface.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_surface.m_strFileNamePath = m_pDoc->m_strGSPFilePath;
  m_surface.DoModal();
}

//#########################################################################
// Coordinate transformation
//#########################################################################

int CGeoSysGEOView::xpixel(double x)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);
  return (int)(xminp + x * skalex);
}      	  	                                     

int CGeoSysGEOView::ypixel(double y)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return (int)( yminp - y * skaley);
}      	  	                                     

double CGeoSysGEOView::x_world(int xpixel)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);
  return ((double) (xpixel-xminp)/(double) width)*(m_dXmax-m_dXmin);
}      	  	                                     

double CGeoSysGEOView::y_world(int ypixel)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return ((double) (yminp-ypixel)/(double) height)*(m_dYmax-m_dYmin);
}      	  	                                     

/**************************************************************************
GeoSys-GUI: pixel2Real
Programing: transfer pixel coordinate into world coordinate
03/2004 CC Implementation
**************************************************************************/
void CGeoSysGEOView::pixel2Real(CGLPoint* gl_point, CPoint point)
{
/*
  //CC
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  //-----------------------------------------------
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);
  gl_point->x = (double)(point.x - xminp)/skalex;
  gl_point->y = (double)(yminp-point.y)/skaley; 
  gl_point->z = 0.0;
*/
  gl_point->x = x_world(point.x);
  gl_point->y = y_world(point.y);
  gl_point->z = 0.0;

}


//#########################################################################
// Mouse Events
//#########################################################################


/**************************************************************************
GeoSys-GUI: 
Programing:
09/2003 OK Implementation
05/2004 CC Last Modification put drawing and zooming function together
06/2004 CC mouse move for label
**************************************************************************/
void CGeoSysGEOView::OnMouseMove(UINT nFlags, CPoint point)
{
  if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
  {
    CGeoSysZoomView::OnMouseMove(nFlags, point);
  }
else {
  //CClientDC dc(this);
  CClientDC dc(this);
  GetWindowAttributes(this->m_hWnd, &width, &height);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    return;
  // sh_graph
  CBrush MagentaBrush;
  MagentaBrush.CreateSolidBrush(RGB(255,0,255));
  //ViewDPtoLP(&point);
  //coordinates
  CGraphics m_grafic;
  m_grafic.m_dXmax = m_dXmax;
  m_grafic.m_dXmin = m_dXmin;
  m_grafic.m_dYmax = m_dYmax;
  m_grafic.m_dYmin = m_dYmin;
  m_grafic.m_dDX = m_grafic.m_dXStep*(m_grafic.m_dXmax-m_grafic.m_dXmin);
  m_grafic.m_dDY = m_grafic.m_dYStep*(m_grafic.m_dYmax-m_grafic.m_dYmin);
  //----------------------------------------------------------------------
  // Mark polyline points
  CGLPoint *gl_point = NULL;
  CGLPolyline *gl_polyline = NULL;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC 08/2005
  while(p!=polyline_vector.end()) {
    gl_polyline = *p;
    // loop over polyline nodes
    vector<CGLPoint*>::iterator pp = gl_polyline->point_vector.begin();
    while(pp!=gl_polyline->point_vector.end()) {
      gl_point = *pp;
      //OKnew
      CPoint point1;
      point1.x = gl_point->x_pix;
      point1.y = gl_point->y_pix;
      ViewLPtoDP(&point1);
      gl_point->x_pix = point1.x;
      gl_point->y_pix = point1.y;
      // if(abs(gl_point->x_pix)>width||abs(gl_point->y_pix)>height) return;
      CRgn circle;
      //CC 06/2004
      circle.CreateEllipticRgn(gl_point->x_pix-gl_point->circle_pix,\
                               gl_point->y_pix-gl_point->circle_pix,\
                               gl_point->x_pix+gl_point->circle_pix,\
                               gl_point->y_pix+gl_point->circle_pix);
      if (circle.PtInRegion(point)) {
        dc.SelectObject(MagentaBrush);
        dc.FillRgn(&circle,&MagentaBrush);
		//sprintf(m_str,"%ld",gl_point->id);//CC remove 08/05
		//dc.TextOut(gl_point->x_pix,gl_point->y_pix,(CString)m_str);//CC remove 08/05
        dc.TextOut(gl_point->x_pix,gl_point->y_pix,gl_point->name.c_str());
      }
      DeleteObject(circle);
      ++pp;
    } // node loop
    ++p;
  }
  //----------------------------------------------------------------------
  //CC implemetation mark the points
  CGLPoint* m_point = NULL;
  for(int i=0;i<(long)gli_points_vector.size();i++) {
    gli_points_vector[i]->x_pix = xpixel(gli_points_vector[i]->x);
    gli_points_vector[i]->y_pix = ypixel(gli_points_vector[i]->y);
  }
  vector<CGLPoint*>::iterator pn = gli_points_vector.begin();
  long nsel = 0;
  while(pn!=gli_points_vector.end())
  {
   m_point = *pn;

   if(abs(m_point->x_pix)>width||abs(m_point->y_pix>height)) return;
      CRgn circle1;
      /////////////////////7706/2004 CC
      CPoint point1;
      point1.x = m_point->x_pix;
      point1.y = m_point->y_pix;
      ViewLPtoDP(&point1);
      m_point->x_pix = point1.x;
      m_point->y_pix = point1.y;
      //----------------------------
      circle1.CreateEllipticRgn(m_point->x_pix-m_point->circle_pix,\
                               m_point->y_pix-m_point->circle_pix,\
                               m_point->x_pix+m_point->circle_pix,\
                               m_point->y_pix+m_point->circle_pix);
      if (circle1.PtInRegion(point)) {
        if(m_iDisplayPNT||theApp->g_graphics_modeless_dlg->m_iDisplayPNT){ //OK
          dc.SelectObject(MagentaBrush);
          dc.FillRgn(&circle1,&MagentaBrush);
		  //sprintf(m_str,"%ld",m_point->id);//CC remove
          //dc.TextOut(m_point->x_pix,m_point->y_pix,(CString)m_str);//CC remove
          dc.TextOut(m_point->x_pix,m_point->y_pix,m_point->name.c_str());//CC
          nSelPoint = nsel;
        }
      }
      DeleteObject(circle1);
      ++pn;
      ++nsel;
    }
  }   // end of else
  CView::OnMouseMove(nFlags, point);
}

/**************************************************************************
GeoSys-GUI: 
Programing:
03/2004 CC Implementation
05/2004 CC Modification put drawing + zooming function together
06/2004 CC Modification Draw point
05/2005 OK Create PLY points from PNT points
06/2005 CC Modification automatically renew the modeless point dialog
06/2005 CC modification related to remove points
07/2005 CC Modification catch point to draw polyline
**************************************************************************/
void CGeoSysGEOView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    return;
  m_dXmin = theApp->g_graphics_modeless_dlg->m_dXmin;
  m_dYmin = theApp->g_graphics_modeless_dlg->m_dYmin;
  m_dXmax = theApp->g_graphics_modeless_dlg->m_dXmax;
  m_dYmax = theApp->g_graphics_modeless_dlg->m_dYmax;
  if((long)gli_points_vector.size()>0){ //OK
    theApp->g_graphics_modeless_dlg->m_iDisplayPNT = true;
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  if((long)polyline_vector.size()>0){ //CC
    theApp->g_graphics_modeless_dlg->m_iDisplayPLY = true;
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //CGeoSysZoomView *m_zoomview;
  //----------------------------------------------------------------------
  if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
  {
    CGeoSysZoomView::OnLButtonDown(nFlags, point);
  }
  //----------------------------------------------------------------------
  //else if(GetZoomMode() == m_drawingPoint ){
  
    //....................................................................
    // Create points
   else if(mainframe->m_bIsPointEditorOn){ // set
      CGLPoint *gl_point = NULL;
      char c_string_pnt[MAX_ZEILE];
      long number_of_points = GEOPointID();
      long size = number_of_points;
      CString m_Pointname;
        sprintf(c_string_pnt, "%ld",number_of_points);
      
      m_Pointname = "POINT";
      m_Pointname += c_string_pnt;
      // Create new point
      strcpy(c_string_pnt,m_Pointname);
      gl_point = new CGLPoint();
      gl_point->name = (string)c_string_pnt;
      gl_point->highlighted = false; //CC
      gli_points_vector.push_back(gl_point);
      vector<CGLPoint*> gs_points_vector = GetPointsVector();//CC

      //zoom convert device coordinate into logic point pixel coordinate.
      ViewDPtoLP(&point);
      gl_point->x_pix = point.x;
      gl_point->y_pix = point.y;
      gl_point->id = size;
      CGLPoint gs_point;
      pixel2Real(&gs_point,point);
      gl_point->x = gs_point.x;
      gl_point->y = gs_point.y;
      gl_point->z = gs_point.z;
      if(theApp->pPointnew->GetSafeHwnd()!=NULL) //OK safe dialog access
      size = (long)gli_points_vector.size()-1;//CC8888
      theApp->pPointnew->AddPointtoList(size);
    }
    //....................................................................
    // Identify points
    //CC 06/2005 draw polyline from picking exiting 2D points
    else if(mainframe->m_bIsPolylineEditorOn){
      char c_string_ply[2048];
      if(theApp->pPolylinenew->GetSafeHwnd() != NULL) {//CC9999
        if(theApp->pPolylinenew->add_polyline){
        strcpy(c_string_ply,theApp->pPolylinenew->m_strNamePolyline);
        CGLPolyline *gl_polyline = NULL;
        gl_polyline = GEOGetPLYByName(c_string_ply);//CC
        //push back the data into point vector
        CGLPoint* m_point = NULL;
        if(nSelPoint!=-1){
          m_point = gli_points_vector[nSelPoint];
          gl_polyline->point_vector.push_back(m_point);
          nSelPoint = -1;
        }
        else
         return;
      }
    }
  }
  //----------------------------------------------------------------------
  Invalidate(TRUE);
  //----------------------------------------------------------------------
  CView::OnLButtonDown(nFlags, point);
}
/**************************************************************************
GeoSys-GUI: OnRButtonDown
Programing: right button control stopping drawing point or polyline
03/2004 CC Implementation
05/2004 CC Modification put drawing + zooming function together
06/2005 CC Modification stop editing polyline
07/2005 CC Edit the messagebox for finish editing polyline
**************************************************************************/
void CGeoSysGEOView::OnRButtonDown(UINT nFlags, CPoint point)
{
	//extern bool v_drawpoint;
	//extern bool v_drawpolyline;
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
    {
    CGeoSysZoomView::OnRButtonDown(nFlags, point);
    }
    else if (theApp->pPolylinenew->GetSafeHwnd() != NULL) {//CC9999
      if( theApp->pPolylinenew->add_polyline ){
       MessageBox("Polyline has been created successfully!",0,MB_OK);
       theApp->pPolylinenew->add_polyline = false;}
         }
	CView::OnRButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////
/////////////////// CC 05/2004 zoom function


/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysGEOView::OnViewZoomin()
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
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysGEOView::OnViewZoomout()
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
void CGeoSysGEOView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysGEOView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysGEOView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}


/**************************************************************************
GeoSys-GUI: OnDrawWell
Programing:mouse click to draw point
06/2004 CC Implementation
07/2005 CC display point
**************************************************************************/
void CGeoSysGEOView::OnDrawPoint()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    return;
  // Toggle zoomin mode
   CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == m_drawingPoint) {
		SetZoomMode(MODE_ZOOMOFF);
    // Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(m_drawingPoint);
       theApp->g_graphics_modeless_dlg->m_iDisplayPNT = true;//CC

       //m_iDisplayPNT = theApp->g_graphics_modeless_dlg->m_iDisplayPNT;
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click here to draw");
	}
} // OnViewZoomin
void CGeoSysGEOView::OnUpdateDrawPoint(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == m_drawingPoint);
} 

/**************************************************************************
GEOLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
09/2005 OK new dialog
last modification: 
**************************************************************************/
void CGeoSysGEOView::OnGeolibVolumes()
{
  //OK CDialogVolume m_dlg_vol;
  CDialogVolumeNew m_dlg_vol; //OK
  //CGeoSysDoc* m_pDoc = GetDocument();
  //m_dlg_vol.m_strFileNameBase = m_pDoc->m_strFileNameBase;
  //m_dlg_vol.m_strFileNamePath = m_pDoc->m_strGSPFilePath;
  m_dlg_vol.DoModal();
}

/**************************************************************************
GEOLibGUI-Method: 
Task: 
Programing:
06/2005 OK Implementation
last modification: 
**************************************************************************/
void CGeoSysGEOView::CalcXYMinMax()
{
  //----------------------------------------------------------------------
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  if((long)gli_points_vector.size()==0)
    return;
  //----------------------------------------------------------------------
  double m_dXMin1 = 1.e+19;
  double m_dXMax1 = -1.e+19;
  double m_dYMin1 = 1.e+19;
  double m_dYMax1 = -1.e+19;
  double value;
  //----------------------------------------------------------------------
  for(long i=0;i<(long)gli_points_vector.size();i++) 
  {
    value = gli_points_vector[i]->x;
    if(value<m_dXMin1) m_dXMin1 = value;
    if(value>m_dXMax1) m_dXMax1 = value;
    value = gli_points_vector[i]->y;
    if(value<m_dYMin1) m_dYMin1 = value;
    if(value>m_dYMax1) m_dYMax1 = value;
  }
  //----------------------------------------------------------------------
  // Shrink a bit
  m_dXmin = m_dXMin1 - 0.05*(m_dXMax1-m_dXMin1);
  m_dXmax = m_dXMax1 + 0.05*(m_dXMax1-m_dXMin1);
  m_dYmin = m_dYMin1 - 0.05*(m_dYMax1-m_dYMin1);
  m_dYmax = m_dYMax1 + 0.05*(m_dYMax1-m_dYMin1);
  //----------------------------------------------------------------------
}
