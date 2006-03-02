// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"

#include "ChildFrm.h"
#include "out_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
   /*Size*/ 
   cs.cx = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
   cs.cy = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
   cs.cx = cs.cx/3;
   cs.cy = cs.cy/3;
   cs.cx = cs.cx*2;
   cs.cy = cs.cy*2;
   cs.cx = 200+cs.cx;
   cs.cy = cs.cy;
   cs.cx = (::GetSystemMetrics(SM_CXVIRTUALSCREEN)) - cs.cx;
   cs.cy = cs.cx;

   /*Position*/ 
   cs.x = ::GetSystemMetrics(SM_CXVIRTUALSCREEN) - (cs.cx);
   cs.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);

   cs.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);

   //cs.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN) + cs.cy;

   //cs.x = ::GetSystemMetrics(SM_XVIRTUALSCREEN);//((cs.cy * 1) - cs.cy) / 1;
   //cs.y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);//((cs.cx * 1) - cs.cx) / 1;

   return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers
