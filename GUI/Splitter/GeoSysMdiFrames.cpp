// MdiFrames.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysMdiFrames.h"
#include "GeoSysVisualFx.h"
#include "MainFrm.h"

#include "GeoSysTreeView.h"
#include "GeoSysListView.h"
#include "GeoSysEditView.h"
#include "GSForm3DLeft.h"
#include "GSFormRightPassive.h"
#include "GSFormRightPicking.h"
#include "GSFormRightMeshing.h"
#include "GeoSysGEOView.h"
#include "GeoSysMSHView.h"
#include "GeoSysMATView.h"
#include "GeoSysOUTView.h"
#include "GeoSysTIMView.h"
#include "GeoSysFCTView.h"
#include "GeoSysPCSView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabSplitterFrame

IMPLEMENT_DYNCREATE(CTabSplitterFrame, CMDIChildWnd)

CTabSplitterFrame::CTabSplitterFrame()
{
}

CTabSplitterFrame::~CTabSplitterFrame()
{
}


BEGIN_MESSAGE_MAP(CTabSplitterFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CTabSplitterFrame)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/**************************************************************************
GEOSYS-Methods for SplitteredView
Task: Dieses Framework wird momentan verwendet
Programing:
03/2004 TK Splittered View with Tabs
**************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CTabSplitterFrame message handlers

BOOL CTabSplitterFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
  lpcs=lpcs;
  // TODO: Add your specialized code here and/or call the base class
  // Tab. First pane is edit view. Second pane is a splitter (1 row, 2 columns)
  TVisualFramework *pWndTrans=NULL;
  int i=0;

  TVisualObject *pTab = new TVisualObject(i++,"",pContext,RUNTIME_CLASS(TTabWnd));

  TVisualObject *pTab2 = new TVisualObject(i++,"3D-Views", pContext,RUNTIME_CLASS(TTabWnd));

  TVisualObject *pTab2View1 = new TVisualObject(i++,"3D-PASSIVE", 1,3,pContext);
  TVisualObject *pView5 = new TVisualObject(i++,0,0,pContext, RUNTIME_CLASS(CGSForm3DLeft),CSize(215,0));
  TVisualObject *pView6 = pWndTrans->OGLView_Access(pContext,i++);
  TVisualObject *pView12 = new TVisualObject(i++,0,2,pContext, RUNTIME_CLASS(CGSFormRightPassive),CSize(0,0));
  
  TVisualObject *pTab2View2 = new TVisualObject(i++,"3D-PICKING", 1,3,pContext);
  TVisualObject *pView7 = new TVisualObject(i++,0,0,pContext, RUNTIME_CLASS(CGSForm3DLeft),CSize(215,0));
  TVisualObject *pView8 = pWndTrans->PickingGLView_Access(pContext,i++);
  TVisualObject *pView13 = new TVisualObject(i++,0,2,pContext, RUNTIME_CLASS(CGSFormRightPicking),CSize(0,0));

  TVisualObject *pTabView3 = new TVisualObject(i++,"Graph", 1,2,pContext);
  TVisualObject *pView3 = new TVisualObject(i++,0,0,pContext, RUNTIME_CLASS(CGeoSysList),CSize(215,0));
  TVisualObject *pView4 = pWndTrans->OGLView_Access(pContext,i++);
  
  TVisualObject *pTabView4 = new TVisualObject(i++,"GEO", pContext, RUNTIME_CLASS(CGeoSysGEOView));
  TVisualObject *pTabView5 = new TVisualObject(i++,"MSH", pContext, RUNTIME_CLASS(CGeoSysMSHView));
  //TVisualObject *pTabView6 = new TVisualObject(i++,"MAT", pContext, RUNTIME_CLASS(CGeoSysMATView));
  //TVisualObject *pTabView7 = new TVisualObject(i++,"OUT", pContext, RUNTIME_CLASS(CGeoSysOUTView));
  TVisualObject *pTabView8 = new TVisualObject(i++,"TIM", pContext, RUNTIME_CLASS(CGeoSysTIMView));
  TVisualObject *pTabView9 = new TVisualObject(i++,"FCT", pContext, RUNTIME_CLASS(CGeoSysFCTView));
  TVisualObject *pTabView10 = new TVisualObject(i++,"PCS", pContext, RUNTIME_CLASS(CGeoSysView));



  TVisualObject *pTab2View5 = new TVisualObject(i++,"Mesh Tools", 1,3,pContext);
  TVisualObject *pView9 = new TVisualObject(i++,0,0,pContext, RUNTIME_CLASS(CGSForm3DLeft),CSize(215,0));
  TVisualObject *pView10 = pWndTrans->OGLView_Access(pContext,i++);
  TVisualObject *pView11 = new TVisualObject(i++,0,2,pContext, RUNTIME_CLASS(CGSFormRightMeshing),CSize(0,0));

  /*Define TABS*/ 
  m_Framework.Add(pTab);
  m_Framework.Add(pTab, pTab2);
  //m_Framework.Add(pTab, pTabView3);
  m_Framework.Add(pTab, pTabView4);
  m_Framework.Add(pTab, pTabView5);
  //m_Framework.Add(pTab, pTabView6);
  //m_Framework.Add(pTab, pTabView7);
  //m_Framework.Add(pTab, pTabView8);
  //m_Framework.Add(pTab, pTabView9);
  m_Framework.Add(pTab, pTabView10);
  
  /*Define VIEW*/ 
 
  m_Framework.Add(pTab2, pTab2View1);
  m_Framework.Add(pTab2View1, pView5);
  m_Framework.Add(pTab2View1, pView6);
  m_Framework.Add(pTab2View1, pView12);

  m_Framework.Add(pTab2, pTab2View5);
  m_Framework.Add(pTab2View5, pView9);
  m_Framework.Add(pTab2View5, pView10);
  m_Framework.Add(pTab2View5, pView11);

  m_Framework.Add(pTab2, pTab2View2);
  m_Framework.Add(pTab2View2, pView7);
  m_Framework.Add(pTab2View2, pView8);
  m_Framework.Add(pTab2View2, pView13);
  
  m_Framework.Add(pTabView3, pView3);
  m_Framework.Add(pTabView3, pView4);

 
  return m_Framework.Create(this); 
}

BOOL CTabSplitterFrame::PreCreateWindow(CREATESTRUCT& cs)
{
// Create a window without min/max buttons or sizable border 
   //cs.cx = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
   //cs.cy = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
   //cs.cx = cs.cx/3;
   //cs.cy = cs.cy/3;
   //cs.cx = cs.cx*2;
   //cs.cy = cs.cy*2;
   //cs.cx = 200+cs.cx;
   //cs.cy = cs.cy;

   //RECT rect;
   //SystemParametersInfo(SPI_GETWORKAREA,0,&rect,NULL);

   cs.cx = ::GetSystemMetrics(SM_CXFULLSCREEN);
   cs.cy = ::GetSystemMetrics(SM_CYFULLSCREEN)-71;

   cs.x = ::GetSystemMetrics(SM_XVIRTUALSCREEN);//((cs.cy * 1) - cs.cy) / 1;
   cs.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);//((cs.cx * 1) - cs.cx) / 1;


   return CMDIChildWnd::PreCreateWindow(cs);
}

void CTabSplitterFrame::ActivateFrame(int nCmdShow)
{
    //Maximize the childview
    //if (nCmdShow == -1)
        //nCmdShow = SW_SHOWMAXIMIZED;

    CMDIChildWnd::ActivateFrame(nCmdShow);
} 


void CTabSplitterFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	// TODO: Add your message handler code here
  m_Framework.Destroy();
}

// Gets active OpenGLView (getset==0) and sets OpenGLView aktive(getset==1)
void CTabSplitterFrame::ActivateOGLView(int getset) 
{
  if (getset==0){
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd; // Get the active MDI child window.
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();// Get the active view attached to the active MDI child window.
  CView *pView = pChild->GetActiveView();
  CGeoSysTree *dtree = NULL;
  dtree=new CGeoSysTree();
  dtree->pOGLView = pView;
  }
  
  if (getset>0){
	  //pChild->SetActiveView(pView);
  }
  
  
}


