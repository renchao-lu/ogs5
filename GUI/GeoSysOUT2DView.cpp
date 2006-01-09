// GeoSysOUTView.cpp : Implementierung der Klasse CGeoSysView
//
#include "stdafx.h"
#include "GeoSys.h"

#include "GeoSysDoc.h"
#include "GeoSysOUT2DView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"
#include "GeoSysGEOView.h"

// GUI
#include "gs_graphics.h"
#include "gs_output.h"
#include "rf_p.h"
#include "gs_pnt.h"
#include "gs_plot.h"
#include "gs_idw.h"
#include "gs_interpolate.h"
#include "gs_legend.h"
#include "gs_welltable.h"
// GeoSys-FEMLib
#include "femlib.h"
#include "files.h"
#include "elements.h"
#include "nodes.h"
#include "rf_out_new.h"
#include "rf_pcs.h"
#include "loop_pcs.h"
#include "rf_apl.h"
#include "rf_out_new.h"
// GeoSys-GEOLib
#include "geo_pnt.h"

//global variable
bool captured = false;
int counter = 0;
//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING
#include ".\geosysout2dview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUT2DView

IMPLEMENT_DYNCREATE(CGeoSysOUT2DView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysOUT2DView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_OUT2D_PROPERTIES, OnProperties)
     //05/2004
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
        // 06/2004 CC
    
    ON_COMMAND(ID_OUTPUT_INTERPOLATION, OnOutputInterpolation)
    ON_COMMAND(ID_OUTPUT_EXPORTWELLS, OnOutputExportwells)
    ON_COMMAND(ID_TABLE_ATTRIBUTE, OnWellTable)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUT2DView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
06/2004 OK Implementation
06/2004 CC Modification
**************************************************************************/
CGeoSysOUT2DView::CGeoSysOUT2DView()
{
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_iDisplayPNT = 0;
  m_iDisplayMSH = 0;
  m_bDisplayMSHQuad = false;
  m_bShowLabels = false;
  m_iDisplayIsolines = 0;
  m_showaxis  = 0;
  m_Displaylegend = false;
  m_iDisplayIsosurfaces = 0;
  m_iQuantityIndex = -1;
  m_iDisplayObservaWells = 0;
  m_bTimeSelectedAll = true; //OK
  m_label_elements = 250;
}
CGeoSysOUT2DView::~CGeoSysOUT2DView()
{

}
/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
**************************************************************************/
void CGeoSysOUT2DView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
  // Properties dialog
  OnProperties();
}

BOOL CGeoSysOUT2DView::PreCreateWindow(CREATESTRUCT& cs)
{
	// ZU ERLEDIGEN: Ändern Sie hier die Fensterklasse oder das Erscheinungsbild, indem Sie
	//  CREATESTRUCT cs modifizieren.

	return CView::PreCreateWindow(cs);
}
/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Zeichnen
//###########################################################################
// Draw
//###########################################################################

/**************************************************************************
GeoSys-GUI: CGeoSysOUT2DView()
Programing:
06/2004 OK Implementation
06/2004 CC Modification
07/2004 CC Modification show axis
06/2005 OK MSH
**************************************************************************/
void CGeoSysOUT2DView::OnDraw(CDC* pDC)
{
  long i;
  //CClientDC dc(this);
  GetWindowAttributes(this->m_hWnd,&width,&height);
  CGeoSysDoc *m_pDoc = GetDocument();
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_dXmax - m_dXmin);
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_dYmax - m_dYmin);
  m_graphics.m_dUmin = m_dUmin;
  m_graphics.m_dUmax = m_dUmax;
  m_graphics.display_labels = m_bShowLabels;
  m_graphics.m_iQuantityIndex = m_iQuantityIndex;
  m_graphics.label_elements = m_label_elements;
  m_graphics.m_strQuantityName = m_strQuantityName;
  //-----------------------------------------------------------------------
  pDC->TextOut(0,0," 2-D Output Results");
  //-----------------------------------------------------------------------
  // Bitmap
  //----------------------------------------------------------------------
  // Isosurfaces
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSName);
  if(m_iDisplayIsosurfaces&&m_pcs&&m_pcs->m_msh){
    CElem* m_ele = NULL;
    for(i=0;i<(long)m_pcs->m_msh->ele_vector.size();i++) {
      m_ele = m_pcs->m_msh->ele_vector[i];
      m_graphics.m_strQuantityName = m_strQuantityName;
      if(m_strQuantityName.IsEmpty())
        m_graphics.m_strQuantityName = m_pcs->pcs_primary_function_name[0];
      m_graphics.DrawIsosurfacesNew(m_ele,m_pcs,pDC);
    }
  }
  //----------------------------------------------------------------------
  // Isolines
  if(m_iDisplayIsolines&&m_pcs&&m_pcs->m_msh){
    m_graphics.m_strQuantityName = m_strQuantityName;
    if(m_strQuantityName.IsEmpty())
      m_graphics.m_strQuantityName = m_pcs->pcs_primary_function_name[0];
    m_graphics.DrawIsolines(pDC,m_pcs);
  }
  //-----------------------------------------------------------------------
  // Mesh
  if (m_iDisplayMSH){
    m_graphics.m_iDisplayMSH = m_iDisplayMSH;
    m_graphics.m_bDisplayMSHQuad = m_bDisplayMSHQuad;
    m_graphics.m_bShowLabels = m_bShowLabels;
    m_graphics.DrawElements(pDC);
  }
  //======================================================================
  // RFO Data
  //----------------------------------------------------------------------
  // Isosurfaces
  if (m_iDisplayIsosurfaces&&m_pDoc->m_bDataRFO){ //OK
    m_graphics.m_bDataRFO = m_pDoc->m_bDataRFO;
    if(m_pDoc->m_nodes_elements) //OK
      m_graphics.m_nodes_elements = m_pDoc->m_nodes_elements;
    if(m_bTimeSelectedAll){
    }
    else{
      m_graphics.m_iTimeSelected = m_iTimeSelected;
      if(m_pDoc->m_nodes_elements->number_of_nodes>0){
        for(i=0;i<m_pDoc->m_nodes_elements->number_of_elements;i++) {
          m_graphics.DrawIsosurfaces(i,pDC);
        }
      }
    }
  }
  //--------------------------------------------------------------------
  // Isolines
  if (m_iDisplayIsolines&&m_pDoc->m_bDataRFO){ //OK
   m_graphics.m_bDataRFO = m_pDoc->m_bDataRFO;
   m_graphics.m_nodes_elements = m_pDoc->m_nodes_elements;
   if(m_bTimeSelectedAll){
     int j;
     for(j=0;j<m_pDoc->m_nodes_elements->number_of_times;j++){
       m_graphics.m_iTimeSelected = j;
       for(i=0;i<m_pDoc->m_nodes_elements->number_of_elements;i++) {
         m_graphics.DrawIsolinesELE(i,0.0,0.0,pDC,width,height);
       }
     }
   }
   else{
     m_graphics.m_iTimeSelected = m_iTimeSelected;
     if(m_pDoc->m_nodes_elements->number_of_nodes>0)
       for(i=0;i<m_pDoc->m_nodes_elements->number_of_elements;i++) {
         m_graphics.DrawIsolinesELE(i,0.0,0.0,pDC,width,height);
       }
     }
   }
  //--------------------------------------------------------------------
  //add display points with tpye 3
  if(m_iDisplayObservaWells){
    for(int i=0;i<(long)gli_points_vector.size();i++) {
      gli_points_vector[i]->x_pix = xpixel(gli_points_vector[i]->x);
      gli_points_vector[i]->y_pix = ypixel(gli_points_vector[i]->y);
    }
    CGLPoint *m_point = NULL;
    vector<CGLPoint*>::iterator p = gli_points_vector.begin();
    while(p!=gli_points_vector.end()){
      m_point = *p;
      
      //  gs_point->DrawPoint(pDC,m_point);
      m_graphics.DrawPointPixel(pDC,m_point);// -> CGraphics ? [CC]
      ++p;
    }
   }
   //draw axis x and y
   if(m_showaxis){
 
    CPen purpleSolidPen;
    purpleSolidPen.CreatePen(PS_SOLID, 0, RGB(102,0,102));
    pDC->SelectObject(&purpleSolidPen);

    pDC->MoveTo(0,height);
    pDC->LineTo(width,height);
    pDC->MoveTo (0,height);
    pDC->LineTo (0,0);
    char c_strx[20];
    char c_stry[20];

    for(int i = 1; i<= 20; i++)
    {
       pDC->MoveTo(i*width/20,height);
       pDC->LineTo(i*width/20,height-5);

       sprintf(c_strx,"%d",(int)(i*m_dXmax/20));
     
       pDC->TextOut(i*width/20,height+5,(CString)c_strx);

       pDC->MoveTo(0,height-i*height/20);
       pDC->LineTo(5,height-i*height/20);
   
       sprintf(c_stry,"%d",(int)(i*m_dYmax/20));
     
       pDC->TextOut(-20,height-i*height/20,(CString)c_stry);
     }
        //origin
        pDC->TextOut(-10,height,"0",1);
        pDC->TextOut(width,height+20,"X",1);
        pDC->TextOut(-30,0,"Y",1);
    }//end of showaxis
   //-----------show legend---------------------------------------

    if(m_Displaylegend){

    CGSLegend gs_legend;
    gs_legend.DoModal();


    }
}
/**************************************************************************
GeoSys-GUI: 
Programing:
06/2004 CC Implementation Draw point
07/2004 CC Modification : delete writting well function 
**************************************************************************/
void CGeoSysOUT2DView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CGLPoint gs_point;
  if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
  {
    CGeoSysZoomView::OnLButtonDown(nFlags, point);
  }
  else if(GetZoomMode() == m_drawingPoint ){
     char c_string_pnt[MAX_ZEILE];
     CGLPoint *gl_point = NULL;
     CString m_Pointname;
     long number_of_points = (long)gli_points_vector.size();//CC
     sprintf(c_string_pnt, "%ld",number_of_points);
     m_Pointname = "POINT";
     m_Pointname += c_string_pnt;
     // Create new point
     strcpy(c_string_pnt,m_Pointname);
     vector<CGLPoint*> gs_points_vector = GetPointsVector();//CC
    
    gl_point = new CGLPoint();
    gl_point->name = (string)c_string_pnt;
    gl_point->highlighted = false; //CC
    gli_points_vector.push_back(gl_point);
     //---------------------------------------------------------
     //zoom convert device coordinate into logic point pixel coordinate.
     //CC
     ViewDPtoLP(&point);
     //gl_point = new CGLPoint;
     gl_point->x_pix = point.x;
     gl_point->y_pix = point.y;
     gl_point->id = (long)gli_points_vector.size();
     // char string_pntname[MAX_ZEILE];
     pixel2Real(&gs_point,point);
     gl_point->x = gs_point.x;
     gl_point->y = gs_point.y;
     gl_point->z = gs_point.z;
     //-------------------------------------------------------------------
  // OUT observation object
   /*  COutput *m_out;
     m_out = new COutput;
  //if Point
     m_out->geo_type = 0;
     m_out->geo_type_name = "POINT";
     m_out->msh_node_number = GetNodeNumberClose(gs_point.x,gs_point.y,gs_point.z);
  //if Polyline
  // OUT vector
     out_vector.push_back(m_out);
  CGeoSysDoc* pDoc = GetDocument();
  OUTWriteNODValues((string)pDoc->m_strFileNameBase,pDoc->m_nodes_elements);*/
  }
  Invalidate(TRUE);
  CView::OnLButtonDown(nFlags, point);
}
/**************************************************************************
GeoSys-GUI: 
Programing:
06/2004 CC/OK Implementation
last modified: 07/2004 CC right button click to draw concentration curve
**************************************************************************/
void CGeoSysOUT2DView::OnRButtonDown(UINT nFlags, CPoint point)
{
    if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
    {
    CGeoSysZoomView::OnRButtonDown(nFlags, point);
    }
    else if(out_vector.size()== 0 ){

     MessageBox("Please export wells first!",0,MB_OK);

    }
    else {

    CGLPoint gs_point;
    //CC 06/2004 convert device unit into logic unit
    ViewDPtoLP(&point);
    pixel2Real(&gs_point,point);//
    COutput *m_out = NULL;
    int out_node_number = m_out->GetPointClose(gs_point);
    m_out = out_vector[out_node_number];
// test ---------------------------------------------------------------
    CGeoSysDoc *m_pDoc = GetDocument();

    CGSPlot gl_plot;
    gl_plot.m_out = m_out;
    gl_plot.m_document = m_pDoc;
    if (captured == true)
    {
        gl_plot.MaxConcentration = m_dUmax;
        gl_plot.DoModal();
     
    }
    captured = false;
	}//end of else
 
// --------------------------------------------------------------------
  //delete &gs_point;
  CView::OnRButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysOUT2DView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}
void CGeoSysOUT2DView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}
void CGeoSysOUT2DView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}
/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysOUT2DView::AssertValid() const
{

	CView::AssertValid();
}
void CGeoSysOUT2DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
CGeoSysDoc* CGeoSysOUT2DView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
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


void CGeoSysOUT2DView::OnProperties()
{
  CGraphics m_graphics;

  CGeoSysDoc *m_pDoc = GetDocument();
  m_graphics.m_bDataRFO = m_pDoc->m_bDataRFO;
  m_graphics.m_nodes_elements = m_pDoc->m_nodes_elements;

  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_iDisplayMSH = m_iDisplayMSH;
  m_graphics.m_bDisplayMSHQuad = m_bDisplayMSHQuad;
  m_graphics.m_iDisplayIsolines = m_iDisplayIsolines;
  m_graphics.m_showaxis = m_showaxis;
  m_graphics.m_Displaylegend  = m_Displaylegend;
  
  m_graphics.m_iDisplayObservaWells = m_iDisplayObservaWells;
  m_graphics.m_iDisplayIsosurfaces = m_iDisplayIsosurfaces;
  m_graphics.m_dUmin = m_dUmin;
  m_graphics.m_dUmax = m_dUmax;
  m_graphics.m_iQuantityIndex = m_iQuantityIndex;
  m_graphics.m_iDisplayPNT = m_iDisplayPNT;
  m_graphics.m_iTimeSelected = m_iTimeSelected;
  m_graphics.label_elements = m_label_elements;
  m_graphics.display_labels = m_bShowLabels;
  m_graphics.m_strPCSName = m_strPCSName; //OK
  
  m_graphics.DoModal();

  m_dXmin = m_graphics.m_dXmin;
  m_dXmax = m_graphics.m_dXmax;
  m_dYmin = m_graphics.m_dYmin;
  m_dYmax = m_graphics.m_dYmax;
  m_iDisplayMSH = m_graphics.m_iDisplayMSH;
  m_bDisplayMSHQuad = m_graphics.m_bDisplayMSHQuad;
  m_iDisplayIsolines = m_graphics.m_iDisplayIsolines;
  m_showaxis = m_graphics.m_showaxis;
  m_Displaylegend = m_graphics.m_Displaylegend;
  m_iDisplayObservaWells = m_graphics.m_iDisplayObservaWells;
  m_iDisplayIsosurfaces = m_graphics.m_iDisplayIsosurfaces;
  m_iDisplayPNT = m_graphics.m_iDisplayPNT;
  m_dUmin = m_graphics.m_dUmin;
  m_dUmax = m_graphics.m_dUmax;
  m_iQuantityIndex = m_graphics.m_iQuantityIndex;
  m_iTimeSelected = m_graphics.m_iTimeSelected;
  m_bTimeSelectedAll = m_graphics.m_bTimeSelectedAll;
  m_label_elements = m_graphics.label_elements;
  m_bShowLabels = m_graphics.display_labels;
  m_strPCSName = m_graphics.m_strPCSName; //OK

  Invalidate(FALSE);
}
//////////////////////////////////////////////////////////////////////
/////////////////// CC 05/2004 zoom function /////////////////////////


/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
05/2004 CC Implementation
**************************************************************************/
void CGeoSysOUT2DView::OnViewZoomin()
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
void CGeoSysOUT2DView::OnViewZoomout()
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
void CGeoSysOUT2DView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull

/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUT2DView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin
/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUT2DView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}
/**************************************************************************
GeoSys-GUI: OnDrawWell
Programing:mouse click to draw point
06/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUT2DView::OnDrawPoint()
{
	// Toggle zoomin mode


    CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == m_drawingPoint) {
		SetZoomMode(MODE_ZOOMOFF);
		// Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(m_drawingPoint);
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click here to draw a well ");
	}


	
} // OnViewZoomin
void CGeoSysOUT2DView::OnUpdateDrawPoint(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == m_drawingPoint);
} 
/**************************************************************************
GeoSys-GUI: pixel2Real
Programing: transfer pixel coordinate into world coordinate
06/2004 CC Implementation
**************************************************************************/
void CGeoSysOUT2DView::pixel2Real(CGLPoint* gl_point, CPoint point)
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
double CGeoSysOUT2DView::x_world(int xpixel)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);
  return ((double) (xpixel-xminp)/(double) width)*(m_dXmax-m_dXmin);
}      	  	                                     
double CGeoSysOUT2DView::y_world(int ypixel)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return ((double) (yminp-ypixel)/(double) height)*(m_dYmax-m_dYmin);
}      	  
int CGeoSysOUT2DView::xpixel(double x)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);
  return (int)(xminp + x * skalex);
}      	  	                                     
int CGeoSysOUT2DView::ypixel(double y)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return (int)( yminp - y * skaley);
}  
/**************************************************************************
GeoSys-GUI: 
Programing:
06/2004 CC Last Modification put drawing and zooming function together
**************************************************************************/
void CGeoSysOUT2DView::OnMouseMove(UINT nFlags, CPoint point)
{

    if(GetZoomMode() == MODE_ZOOMIN ||GetZoomMode() == MODE_ZOOMOUT)
  {
      CGeoSysZoomView::OnMouseMove(nFlags, point);
  }
  else {

      //

  //CClientDC dc(this);
  CClientDC dc(this);
  GetWindowAttributes(this->m_hWnd, &width, &height);
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
  //---------------------------------------------------------------------------
  char m_str[10];
  CGLPoint* m_point = NULL;
  for(long i=0;i<(long)gli_points_vector.size();i++) {
    gli_points_vector[i]->x_pix = xpixel(gli_points_vector[i]->x);
    gli_points_vector[i]->y_pix = ypixel(gli_points_vector[i]->y);
  }
  vector<CGLPoint*>::iterator pn = gli_points_vector.begin();
  while(pn!=gli_points_vector.end())
  {
   m_point = *pn;

   if(abs(m_point->x_pix)>width||abs(m_point->y_pix>height)) return;
      CRgn circle1;
      /////////////////////06/2004 CC
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
        if(m_iDisplayObservaWells){
        dc.SelectObject(MagentaBrush);
        dc.FillRgn(&circle1,&MagentaBrush);
		sprintf(m_str,"%ld",m_point->id);
        dc.TextOut(m_point->x_pix,m_point->y_pix,(CString)m_str);
        captured = true;
        }
      }
      DeleteObject(circle1);

  ++pn;
  }
  }// end of else
  CView::OnMouseMove(nFlags, point);
}
/**************************************************************************
GeoSys-GUI: OnInterpolation
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGeoSysOUT2DView::OnOutputInterpolation()
{
     CGeoSysDoc* pDoc = GetDocument();
     GSInterpolate gs_interpolation;
     gs_interpolation.m_basename= pDoc->m_strGSPFileBase;
    
     gs_interpolation.DoModal();
      
}
/**************************************************************************
GeoSys-GUI: OnOutputExportwells
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGeoSysOUT2DView::OnOutputExportwells()
{
  
  //if (counter ==0){
  CGLPoint * m_point = NULL;
  CGLPoint gs_point;
  out_vector.clear();
  vector<CGLPoint*>::iterator p = gli_points_vector.begin();
  while(p!=gli_points_vector.end())
  {
     m_point = *p;
     gs_point.x = m_point->x;
     gs_point.y = m_point->y;
     gs_point.z = m_point->z;
     COutput *m_out;
     m_out = new COutput;
  //if Point
     m_out->geo_type = 0;
     m_out->geo_type_name = "POINT";
     m_out->msh_node_number = GetNodeNumberClose(gs_point.x,gs_point.y,gs_point.z);
  //if Polyline
  // OUT vector
     //-----------------------
    
    out_vector.push_back(m_out);

    ++p;
  }//end of while
  CGeoSysDoc* pDoc = GetDocument();
     
  OUTWriteNODValues((string)pDoc->m_strGSPFileBase,pDoc->m_nodes_elements);
  //counter = counter +1;
 // }
  /*else 
  {
  }*/
}

/**************************************************************************
GeoSys-GUI: OnDrawWell
Programing:display well table of attributes
08/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUT2DView::OnWellTable()
{
// Toggle zoomin mode
	CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	
	pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"open the table of wells  ");
   
	/*CMainFrame* pMainFrame = new CMainFrame;
	pMainFrame->LoadFrame(IDR_MAINFRAME);
    pMainFrame->OnViewGEOCreate();*/
  CListWellTable m_wellsdialog;
    //CListWellTable m_wellsdialog(GetDocument(),this);
  m_wellsdialog.pDoc = GetDocument();
    m_wellsdialog.DoModal();
    
   // SetZoomMode(m_drawingWell);
} 