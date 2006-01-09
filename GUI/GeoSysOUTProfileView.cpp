// GeoSysOUTProfileView.cpp : Implementierung der Klasse CGeoSysView
//
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysOUTProfileView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"
// GUI
#include "gs_graphics.h"
#include "gs_output.h"
#include "rf_p.h"
#include "out_dlg.h"
//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING
#include <math.h>
// MSHLib
#include "msh_lib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUTProfileView

IMPLEMENT_DYNCREATE(CGeoSysOUTProfileView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysOUTProfileView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_SPATIALPROFILES_PROPERTIES, OnProperties)

    //05/2004
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUTProfileView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
09/2003 OK Implementation
**************************************************************************/
CGeoSysOUTProfileView::CGeoSysOUTProfileView()
{
  m_iDisplayOUTProfileProperties = 0;
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_bDisplayOUTProfileProperties = false;
}

CGeoSysOUTProfileView::~CGeoSysOUTProfileView()
{
}

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
**************************************************************************/
void CGeoSysOUTProfileView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
  // Properties dialog
  OnProperties();
}

BOOL CGeoSysOUTProfileView::PreCreateWindow(CREATESTRUCT& cs)
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
GeoSys-GUI: CGeoSysView()
Programing:
12/2003 OK Implementation
05/2004 CC Modification
05/2005 OK OUT,MSH,PCS concept
**************************************************************************/
void CGeoSysOUTProfileView::OnDraw(CDC* pDC)
{
  vector<double>x_vector;
  x_vector.clear(); //OK
  vector<double>y_vector;
  y_vector.clear(); //OK
  //CClientDC dc(this);
  GetWindowAttributes(this->m_hWnd,&width,&height);
  //----------------------------------------------------------------------
if(m_out->geo_type_name.compare("POLYLINE")==0){
  // OUT->GEO->MSH (x data)
  double x,y,z,dist;
  CGLPolyline* m_ply = NULL;
  m_ply = GEOGetPLYByName(m_out->geo_name);//CC
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet(m_out->pcs_type_name);
  vector<long>nodes_vector;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(m_out->pcs_type_name);
  if(m_ply&&m_msh&&m_pcs){
    //m_msh->GetNODOnPLYSorted(m_ply,nodes_vector);
    m_msh->GetNODOnPLY(m_ply,nodes_vector);
    //GetNodeByDistance_Polyline(m_ply,nodes_vector); 
    for(int i=0;i<(int)nodes_vector.size();i++){
      x = m_msh->nod_vector[nodes_vector[i]]->X();
      y = m_msh->nod_vector[nodes_vector[i]]->Y();
      z = m_msh->nod_vector[nodes_vector[i]]->Z();
      dist = sqrt(x*x+y*y+z*z);
      if(i==0) m_dXmin = dist;
      if(i==(int)nodes_vector.size()-1) m_dXmax = dist;
      x_vector.push_back(dist);
    }
  }
  //----------------------------------------------------------------------
  // OUT->MSH->PCS (y data)
  int nidx1;
  double nod_value;
  m_dYmin = 1.e+19;
  m_dYmax = -1.e+19;
  if(m_ply&&m_msh&&m_pcs){ //
    nidx1 = m_pcs->GetNodeValueIndex(m_out->nod_value_name)+1; //ToDo
    for(int i=0;i<(int)nodes_vector.size();i++){
      nod_value = m_pcs->GetNodeValue(nodes_vector[i],nidx1);
      if(nod_value<m_dYmin) m_dYmin = nod_value;
      if(nod_value>m_dYmax) m_dYmax = nod_value;
      y_vector.push_back(nod_value);
    }
  }
  if(m_dYmin==m_dYmax){
    if(fabs(m_dYmin)<MKleinsteZahl){
      m_dYmin -= 0.5;
      m_dYmax += 0.5;
    }
    else{
      m_dYmin -= 0.5*fabs(m_dYmin);
      m_dYmax += 0.5*fabs(m_dYmax);
    }
  }
}
  //----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_dXmax - m_dXmin);
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_dYmax - m_dYmin);
  m_graphics.m_strQuantityName = m_strQuantityName;
  m_graphics.m_strPolylineName = m_strPolylineName;
  //-----------------------------------------------------------------------
  CString m_strViewInfo = "Profiles along Polylines: ";
  m_strViewInfo += m_out->nod_value_name.c_str();//m_strQuantityName;
  pDC->TextOut(0,0,m_strViewInfo);
  //-----------------------------------------------------------------------
  // Koordinatenachsen zeichnen und beschriften
  m_graphics.DrawGridAxes(pDC);
  m_graphics.DrawCoordinateAxes(pDC);
  //-----------------------------------------------------------------------
  // Draw x-y plot
  m_graphics.DrawXYPlot(pDC,x_vector,y_vector);
  //m_graphics.DrawProfileBreakthroughCurves(pDC);
  //m_polyline = m_polyline->GEOGetPolyline((string)m_strPolylineName);
  //m_graphics.DrawPolylineValues(pDC,m_polyline);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysOUTProfileView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysOUTProfileView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysOUTProfileView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysOUTProfileView::AssertValid() const
{

	CView::AssertValid();
}

void CGeoSysOUTProfileView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysOUTProfileView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
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

void CGeoSysOUTProfileView::OnProperties()
{
  CDialogOUT m_dlg;
  m_dlg.DoModal();
  m_out = m_dlg.m_obj;
/*
  COUTObservationPolylines m_out_polylines;

  m_out_polylines.m_dXmin = m_dXmin;
  m_out_polylines.m_dXmax = m_dXmax;
  m_out_polylines.m_dYmin = m_dYmin;
  m_out_polylines.m_dYmax = m_dYmax;
  m_out_polylines.m_strQuantityName = m_strQuantityName;
  m_out_polylines.m_strPolylineName = m_strPolylineName;

  m_out_polylines.DoModal();

  m_dXmin = m_out_polylines.m_dXmin;
  m_dXmax = m_out_polylines.m_dXmax;
  m_dYmin = m_out_polylines.m_dYmin;
  m_dYmax = m_out_polylines.m_dYmax;
  m_strQuantityName = m_out_polylines.m_strQuantityName;
  m_strPolylineName = m_out_polylines.m_strPolylineName;

  m_bDisplayOUTProfileProperties = true; 
*/
  Invalidate(TRUE);
}

//////////////////////////////////////////////////////////////////////
/////////////////// CC 05/2004 zoom function


/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUTProfileView::OnViewZoomin()
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
void CGeoSysOUTProfileView::OnViewZoomout()
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
void CGeoSysOUTProfileView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUTProfileView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUTProfileView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

