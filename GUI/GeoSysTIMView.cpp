// ViewView.cpp : implementation of the CGeoSysTIMView class
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysTIMView.h"

#include "gs_graphics.h"
#include "rf_tim_new.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGeoSysTIMView

IMPLEMENT_DYNCREATE(CGeoSysTIMView, CView)

BEGIN_MESSAGE_MAP(CGeoSysTIMView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CGeoSysTIMView construction/destruction

CGeoSysTIMView::CGeoSysTIMView()
{
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  time_last = 0.0;
  time_step_last = 0.0;
  if(time_vector.size()>0){
    time_step_max = time_vector[0]->time_step_vector[0];
    time_step_current = time_vector[0]->time_step_vector[0];
  }
}

CGeoSysTIMView::~CGeoSysTIMView()
{
}

BOOL CGeoSysTIMView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGeoSysTIMView drawing

/**************************************************************************
GeoSys-GUI:
Programing:
09/2004 OK Implementation
**************************************************************************/
void CGeoSysTIMView::OnDraw(CDC* pDC)
{
  CGeoSysDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (!pDoc)
    return;
  //-----------------------------------------------------------------------
  pDC->TextOut(0,0," TIM objects");
  //-----------------------------------------------------------------------
//  GetWindowAttributes(this->m_hWnd,&width,&height);
  RECT rect;
  GetClientRect(&rect);
  width=rect.right;
  height=rect.bottom;
  //-----------------------------------------------------------------------
  CTimeDiscretization* m_tim = NULL;
  if(time_vector.size()==0)
    return;
  m_tim = time_vector[0];
  m_tim->time_end;
  if(m_tim->step_current>time_step_max)
    time_step_max = m_tim->step_current;
  //-----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_tim->time_start; //m_dXmin;
  m_graphics.m_dXmax = m_tim->time_end; //m_dXmax;
  m_graphics.m_dDX = m_graphics.m_dXStep*(m_graphics.m_dXmax-m_graphics.m_dXmin);
  m_graphics.m_dYmin = 0.0; //m_dYmin;
  m_graphics.m_dYmax = 2.*time_step_max; //m_dYmax;
  m_graphics.m_dDY = m_graphics.m_dYStep*(m_graphics.m_dYmax-m_graphics.m_dYmin);
  //-----------------------------------------------------------------------
  // Koordinatenachsen zeichnen und beschriften
  m_graphics.DrawGridAxes(pDC);
  m_graphics.DrawCoordinateAxes(pDC);
  //-----------------------------------------------------------------------
  time_step_current = m_tim->time_current - time_last;
  //m_graphics.DrawLineOffset(time_last,time_step_last,m_tim->time_current,time_step_current,pDC);
  //time_step_last = time_step_current;
  CGLPoint m_pnt;
  m_pnt.x = m_tim->time_current;
  m_pnt.y = time_step_current;
  m_pnt.circle_pix = 5;
  pDC->SelectObject(&m_graphics.RedBoldPen);
  m_graphics.DrawPointOffset(pDC,&m_pnt);
  time_last = m_tim->time_current;
}


// CGeoSysTIMView printing

BOOL CGeoSysTIMView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGeoSysTIMView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGeoSysTIMView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CGeoSysTIMView diagnostics

#ifdef _DEBUG
void CGeoSysTIMView::AssertValid() const
{
	CView::AssertValid();
}

void CGeoSysTIMView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysTIMView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG


// CGeoSysTIMView message handlers
