/**************************************************************************
GUILib-Object: FCT
Task:
Programing:
01/2005 OK Implementation
**************************************************************************/

// ViewView.cpp : implementation of the CGeoSysFCTView class
//
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysFCTView.h"

#include "gs_graphics.h"
#include "rf_fct.h"
#include ".\geosysfctview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGeoSysFCTView

IMPLEMENT_DYNCREATE(CGeoSysFCTView, CView)

BEGIN_MESSAGE_MAP(CGeoSysFCTView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    ON_COMMAND(ID_VIEW_FCT_PROPERTIES, OnProperties)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CGeoSysFCTView construction/destruction

/**************************************************************************
GUILib:
Programing:
01/2005 OK Implementation
03/2005 OK modeless dialog
**************************************************************************/
CGeoSysFCTView::CGeoSysFCTView()
{
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
/*
//OK_MODELESS
  //----------------------------------------------------------------------
  m_graphics_dlg = new CGraphics;
  m_graphics_dlg->Create(IDD_DIALOG_GRAPHICS);
  m_graphics_dlg->ShowWindow(SW_SHOW);
  //----------------------------------------------------------------------
  int i;
  for(i=0;i<(int)fct_vector.size();i++){
    m_fct = fct_vector[i];
    m_graphics_dlg->m_LB_FCT.AddString((CString)m_fct->geo_name.c_str());
    if(m_fct->selected)
      m_graphics_dlg->m_LB_FCT.SetSel(i,TRUE);
  }
  //----------------------------------------------------------------------
*/
}

CGeoSysFCTView::~CGeoSysFCTView()
{
/*
//OK_MODELESS
  m_graphics_dlg->CloseWindow();
  delete m_graphics_dlg;
*/
}

BOOL CGeoSysFCTView::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  return CView::PreCreateWindow(cs);
}

// CGeoSysFCTView drawing

/**************************************************************************
GUILib:
Programing:
01/2005 OK Implementation
04/2005 OK Min/Max values
**************************************************************************/
void CGeoSysFCTView::OnDraw(CDC* pDC)
{
  int i,j;
  //-----------------------------------------------------------------------
  CGeoSysDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (!pDoc)
    return;
  //-----------------------------------------------------------------------
  // ? something to view
  int fct_vector_size = (int)fct_vector.size();
  if(fct_vector_size==0)
    return;
  //-----------------------------------------------------------------------
  CString m_strViewHeader;
  CString m_strViewAxisX;
  CString m_strViewAxisY;
  m_strViewHeader = " FCTView - ";
  if(fct_vector_size>0){
    m_fct = fct_vector[0];
    m_strViewAxisX.Format(" X: %s ",m_fct->variable_names_vector[0].c_str());
    m_strViewAxisY.Format(" Y: %s ",m_fct->variable_names_vector[1].data());
    m_strViewHeader += m_strViewAxisX + ", " + m_strViewAxisY;
  }
  pDC->TextOut(0,0,m_strViewHeader);
  //-----------------------------------------------------------------------
  RECT rect;
  GetClientRect(&rect);
  width=rect.right;
  height=rect.bottom;
  //-----------------------------------------------------------------------
  // Min, Max values
  m_dXmin = 1.e+19;
  m_dXmax = -1.e+19;
  m_dYmin = 1.e+19;
  m_dYmax = -1.e+19;
  double value;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    m_fct = FCTGet((string)theApp->g_graphics_modeless_dlg->m_strFCTName);
  }
  if(m_fct){
  //for(i=0;i<(int)fct_vector.size();i++){
    //m_fct = fct_vector[i];
    for(j=0;j<(int)m_fct->variable_data_vector.size();j++){
      value = m_fct->variable_data_vector[j][0];
      if(value<m_dXmin) m_dXmin = value;
      if(value>m_dXmax) m_dXmax = value;
      value = m_fct->variable_data_vector[j][1];
      if(value<m_dYmin) m_dYmin = value;
      if(value>m_dYmax) m_dYmax = value;
    }
  }
/*
  CTimeDiscretization* m_tim = NULL;
  if(time_vector.size()==0)
    return;
  m_tim = time_vector[0];
  m_dXmax = m_tim->time_end;
  m_dYmax = 1e-3;
*/
  //-----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_graphics.m_dXmax-m_graphics.m_dXmin);
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_graphics.m_dYmax-m_graphics.m_dYmin);
  //-----------------------------------------------------------------------
  // Coordinate axes
  m_graphics.DrawGridAxes(pDC);
  m_graphics.DrawCoordinateAxes(pDC);
  //-----------------------------------------------------------------------
  for(i=0;i<fct_vector_size;i++){
    m_fct = fct_vector[i];
    if(m_fct->selected)
      m_graphics.DrawFCT(pDC,m_fct);
  }
}


// CGeoSysFCTView printing

BOOL CGeoSysFCTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGeoSysFCTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGeoSysFCTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CGeoSysFCTView diagnostics

#ifdef _DEBUG
void CGeoSysFCTView::AssertValid() const
{
	CView::AssertValid();
}

void CGeoSysFCTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysFCTView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG


// CGeoSysFCTView message handlers

void CGeoSysFCTView::OnProperties()
{
}

//#########################################################################
// Mouse Events
//#########################################################################

void CGeoSysFCTView::OnMouseMove(UINT nFlags, CPoint point)
{
  Invalidate();
  CView::OnMouseMove(nFlags, point);
}
