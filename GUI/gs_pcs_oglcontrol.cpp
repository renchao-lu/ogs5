// gs_pcs_oglcontrol.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_pcs_oglcontrol.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "OGLEnabledView.h"
#include "nodes.h"
#include "elements.h"
#include "rf_pcs.h"
// Cgs_pcs_oglcontrol dialog

IMPLEMENT_DYNAMIC(Cgs_pcs_oglcontrol, CDialog)
Cgs_pcs_oglcontrol::Cgs_pcs_oglcontrol(CWnd* pParent /*=NULL*/)
	: CDialog(Cgs_pcs_oglcontrol::IDD, pParent)
{
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    m_pcs_min = 0.0;
    m_pcs_max = 1.0;
    m_pcs_name = mainframe->m_pcs_name;
}

Cgs_pcs_oglcontrol::~Cgs_pcs_oglcontrol()
{
}

void Cgs_pcs_oglcontrol::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX,IDC_MIN_EDIT, m_pcs_min);
    DDX_Text(pDX,IDC_MAX_EDIT, m_pcs_max);
    DDX_Text(pDX,IDC_PCS_NAME_EDIT, m_pcs_name);
}


BEGIN_MESSAGE_MAP(Cgs_pcs_oglcontrol, CDialog)
        ON_BN_CLICKED(IDC_SET_PCS_MINMAX_BUTTON, OnControlPanel)
        ON_BN_CLICKED(IDC_GET_PCS_MINMAX_BUTTON2, OnBnClickedGetPcsMinmax)
END_MESSAGE_MAP()


// Cgs_pcs_oglcontrol message handlers

void Cgs_pcs_oglcontrol:: OnControlPanel()
{   
    UpdateData(TRUE);
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->m_pcs_min = m_pcs_min;
    mainframe->m_pcs_max = m_pcs_max;
    mainframe->m_something_changed = 1;

    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	// Get the active view attached to the active MDI child window.
	COGLEnabledView *pView = (COGLEnabledView *) pChild->GetActiveView();
	pView->Invalidate();
  
   
}
void Cgs_pcs_oglcontrol::OnBnClickedGetPcsMinmax()
{
  long i;
  double value;
  m_pcs_min = 1.e+19;
  m_pcs_max = -1.e+19;
 
  //value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));

  int m_iQuantityIndex = PCSGetNODValueIndex((string)m_pcs_name,1);
 
   for(i=0;i<NodeListLength;i++) {
      value = GetNodeVal(i,m_iQuantityIndex);
      if(value<m_pcs_min) m_pcs_min = value;
      if(value>m_pcs_max) m_pcs_max = value;
    }
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->m_pcs_min = m_pcs_min;
    mainframe->m_pcs_max = m_pcs_max;
    mainframe->m_something_changed = 1;

   UpdateData(FALSE);

    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	// Get the active view attached to the active MDI child window.
	COGLEnabledView *pView = (COGLEnabledView *) pChild->GetActiveView();
	pView->Invalidate();

}
