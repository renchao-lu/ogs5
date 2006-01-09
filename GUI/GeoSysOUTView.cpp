// GeoSysOUTView.cpp : Implementierung der Klasse CGeoSysView
//

#include "stdafx.h"
#include "GeoSys.h"

#include "GeoSysDoc.h"
#include "GeoSysOUTView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"

// GUI
#include "gs_graphics.h"
#include "gs_output.h"
#include "rf_p.h"
#include ".\geosysoutview.h"

// GeoSys-FEM
#include "femlib.h"
#include "files.h"
#include "elements.h"
#include "nodes.h"
#include "rf_out_new.h"
#include "rf_pcs.h"
#include "loop_pcs.h"
#include "rf_apl.h"

// For stess path visualization
#include "pcs_dm.h"

//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUTView

IMPLEMENT_DYNCREATE(CGeoSysOUTView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysOUTView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_TEMPORALPROFILES_PROPERTIES, OnProperties)

     //05/2004
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysOUTView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
09/2003 OK Implementation
**************************************************************************/
CGeoSysOUTView::CGeoSysOUTView()
{
  m_iDisplayOUTProperties = 0;
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
}

CGeoSysOUTView::~CGeoSysOUTView()
{

}

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
**************************************************************************/
void CGeoSysOUTView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
  // Properties dialog
  OnProperties();
}

BOOL CGeoSysOUTView::PreCreateWindow(CREATESTRUCT& cs)
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
GeoSys-GUI: CGeoSysOUTView()
Programing:
12/2003 OK Implementation
05/2004 CC Modification
10/2004 OK LOAD_PATH_ANALYSIS
**************************************************************************/
void CGeoSysOUTView::OnDraw(CDC* pDC)
{
  //CClientDC dc(this);
  GetWindowAttributes(this->m_hWnd,&width,&height);
  //-----------------------------------------------------------------------
  // Doc
  CGeoSysDoc* m_pDoc = GetDocument();
  CTimeDiscretization* m_out_tim = NULL;
  m_out_tim = m_pDoc->m_doc_tim;
  //-----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
//OK_MMP  if(aktuelle_zeit>0.0)
//OK_MMP    m_dXmax = 1e+8; //aktuelle_zeit;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_dXmax - m_dXmin);
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_dYmax - m_dYmin);
  m_graphics.m_strQuantityName = m_strQuantityName;
  //-----------------------------------------------------------------------
  pDC->TextOut(0,0," Temporal Profiles in Observation Points");
  pDC->TextOut(300,0,"Quantity:");
  pDC->TextOut(370,0,m_strQuantityName);
  //-----------------------------------------------------------------------
  // Koordinatenachsen zeichnen und beschriften
  m_graphics.DrawCoordinateAxes(pDC);
  m_graphics.DrawGridAxes(pDC);
  //-----------------------------------------------------------------------
  // Fluid properties
  if(m_iDisplayOUTProperties){
    if(m_strQuantityName=="LOAD_PATH_ANALYSIS"){
      double stressInv[3];
      stressInv[0] = stressInv[1] = stressInv[2] = 0.0;
      //WW_LOAD_PATH calculation on I/II min,max values
      //OK CalMaxiumStressInvariants(stressInv);
      if(stressInv[0]==0.0) stressInv[0] = 1.0; 
      if(stressInv[1]==0.0) stressInv[1] = 1.0; 
      m_dXmax = stressInv[0];
      m_dYmax = stressInv[1];
      m_graphics.m_dXmin = m_dXmin;
      m_graphics.m_dXmax = m_dXmax;
      m_graphics.m_dDX = m_graphics.m_dXStep*(m_dXmax - m_dXmin);
      m_graphics.m_dYmin = m_dYmin;
      m_graphics.m_dYmax = m_dYmax;
      m_graphics.m_dDY = m_graphics.m_dYStep*(m_dYmax - m_dYmin);
      m_graphics.DrawCoordinateAxes(pDC);
      m_graphics.DrawGridAxes(pDC);
      m_graphics.DrawLoadPathAnalysis(pDC);
    }
    else{
      m_graphics.DrawTemporalBreakthroughCurves(pDC,m_out_tim);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysOUTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysOUTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysOUTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysOUTView::AssertValid() const
{

	CView::AssertValid();
}

void CGeoSysOUTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysOUTView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
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


void CGeoSysOUTView::OnProperties()
{
  COutputObservation m_output;

  m_output.m_dXmin = m_dXmin;
  m_output.m_dXmax = m_dXmax;
  m_output.m_dYmin = m_dYmin;
  m_output.m_dYmax = m_dYmax;
  m_output.m_strQuantityName = m_strQuantityName;

  m_output.DoModal();

  m_dXmin = m_output.m_dXmin;
  m_dXmax = m_output.m_dXmax;
  m_dYmin = m_output.m_dYmin;
  m_dYmax = m_output.m_dYmax;
  m_strQuantityName = m_output.m_strQuantityName;
  m_iDisplayOUTProperties = 1;

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
void CGeoSysOUTView::OnViewZoomin()
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
void CGeoSysOUTView::OnViewZoomout()
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
void CGeoSysOUTView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUTView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysOUTView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

