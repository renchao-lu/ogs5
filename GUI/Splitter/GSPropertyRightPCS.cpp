// GSPropertyRightPCS.cpp : implementation file
//
/**************************************************************************
        TK Template
07/2007 OK PCS properties
**************************************************************************/

/*Template Instructions TK: 11.2006

How to add a new dialog to the right property sheets

MS.NET:
- Copy Template Resource in the Resource View
- Rename IDD
- Rename Caption

Windows Explorer:
- Copy GSPropertyRightTemplate.cpp + GSPropertyRightTemplate.h of and into the Splitter folder and rename them like GSPropertyRightPCS.cpp.
MS.NET:
- Add them also now to the solution inside the folder "Splitter".

CPP changes:
- Change Header: // GSPropertyRightTemplate.cpp : implementation file
- #include "GSPropertyRightTemplate.h" needs the new link
- Search CGSPropertyRightTemplate and Replace with new class name e.g. CGSPropertyRightPCS

H Changes (Header File):
- Search CGSPropertyRightTemplate and Replace with new class name e.g. CGSPropertyRightPCS
- Exchange enum IDD: IDD_CONTROL_RIGHT_TEMPLATE, e.g IDD_CONTROL_RIGHT_PCS

GSPropertyRight.h
Copy and change: #include "GSPropertyRightTemplate.h"
Position the Tab: copy and change CGSPropertyRightTemplate m_Page10;	

GSPropertyRight.cpp
Create and Add Page by copy and change of the line: m_PropSheet.AddPage(&m_Page10); 
*/ 

#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
//GEOLIB
//MSHLIB
//PCS
#include "problem.h"
#include "rf_tim_new.h"
#include "rf_react.h"
//DLG
#include "pcs_dlg_new.h"
#include "tim_dlg.h"
#include "out_dlg.h"
#include "ic_dlg.h"
#include "out_dlg.h"
#include "gs_bc.h"
#include "st_dlg.h"
#include "mfp_dlg.h"
#include "gs_mat_sp.h"
#include "gs_mat_mp.h"
#include "num_dlg.h"
#include "mod_dlg.h"
#include ".\gspropertyrightpcs.h"

// CGSPropertyRightTemplate dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightPCS, CViewPropertyPage)
CGSPropertyRightPCS::CGSPropertyRightPCS()
	: CViewPropertyPage(CGSPropertyRightPCS::IDD)
{
    m_check_pcs = false;
    m_check_tim = false;
    m_check_num = false;
    m_check_out = false;
    m_check_ic  = false;
    m_check_bc  = false;
    m_check_st  = false;
    m_check_mfp = false;
    m_check_msp = false;
    m_check_mcp = false;
    m_check_mmp = false;
    m_check_mod = false;
}

CGSPropertyRightPCS::~CGSPropertyRightPCS()
{
}

void CGSPropertyRightPCS::DoDataExchange(CDataExchange* pDX)
{
    if((int)pcs_vector.size()>0) m_check_pcs = true;
    if((int)time_vector.size()>0) m_check_tim = true;
    if((int)num_vector.size()>0) m_check_num = true;
    if((int)out_vector.size()>0) m_check_out = true;
    if((int)ic_vector.size()>0) m_check_ic = true;
    if((int)bc_list.size()>0) m_check_bc = true;
    if((int)st_vector.size()>0) m_check_st = true;
    if((int)mfp_vector.size()>0) m_check_mfp = true;
    if((int)msp_vector.size()>0) m_check_msp = true;
    if((int)cp_vec.size()>0) m_check_mcp = true;
    if((int)mmp_vector.size()>0) m_check_mmp = true;
    //if(aproblem) m_check_mod = true;
    //OK m_check_mod = PCSCheck();
    CViewPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_PCS, m_check_pcs);
    DDX_Check(pDX, IDC_CHECK_TIM, m_check_tim);
    DDX_Check(pDX, IDC_CHECK_NUM, m_check_num);
    DDX_Check(pDX, IDC_CHECK_OUT, m_check_out);
    DDX_Check(pDX, IDC_CHECK_IC, m_check_ic);
    DDX_Check(pDX, IDC_CHECK_BC, m_check_bc);
    DDX_Check(pDX, IDC_CHECK_ST, m_check_st);
    DDX_Check(pDX, IDC_CHECK_MFP, m_check_mfp);
    DDX_Check(pDX, IDC_CHECK_MSP, m_check_msp);
    DDX_Check(pDX, IDC_CHECK_MCP, m_check_mcp);
    DDX_Check(pDX, IDC_CHECK_MMP, m_check_mmp);
    DDX_Check(pDX, IDC_CHECK_MOD, m_check_mod);
    DDX_Control(pDX, IDC_COMBO_PCS, m_CB_PCS);
}

BEGIN_MESSAGE_MAP(CGSPropertyRightPCS, CViewPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_PCS, OnBnClickedButtonPCS)
    ON_BN_CLICKED(IDC_BUTTON_TIM, OnBnClickedButtonTIM)
    ON_BN_CLICKED(IDC_BUTTON_NUM, OnBnClickedButtonNUM)
    ON_BN_CLICKED(IDC_BUTTON_OUT, OnBnClickedButtonOUT)
    ON_BN_CLICKED(IDC_BUTTON_IC, OnBnClickedButtonIC)
    ON_BN_CLICKED(IDC_BUTTON_BC, OnBnClickedButtonBC)
    ON_BN_CLICKED(IDC_BUTTON_ST, OnBnClickedButtonST)
    ON_BN_CLICKED(IDC_BUTTON_MFP, OnBnClickedButtonMFP)
    ON_BN_CLICKED(IDC_BUTTON_MSP, OnBnClickedButtonMSP)
    ON_BN_CLICKED(IDC_BUTTON_MCP, OnBnClickedButtonMCP)
    ON_BN_CLICKED(IDC_BUTTON_MMP, OnBnClickedButtonMMP)
    ON_BN_CLICKED(IDC_BUTTON_OBJ, OnBnClickedButtonMOD)
    ON_BN_CLICKED(IDC_BUTTON_MOD_CHECK, OnBnClickedButtonMODCheck)
    ON_BN_CLICKED(IDC_BUTTON_UPDATE_TREE, &CGSPropertyRightPCS::OnBnClickedButtonUpdateTree)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS, &CGSPropertyRightPCS::OnCbnSelchangeComboPCS)
    ON_BN_CLICKED(IDC_BUTTON_MOD_READ, &CGSPropertyRightPCS::OnBnClickedButtonMODRead)
    ON_BN_CLICKED(IDC_BUTTON_MOD_EDITOR, &CGSPropertyRightPCS::OnBnClickedButtonMODEditor)
END_MESSAGE_MAP()


// CGSPropertyRightTemplate message handlers
BOOL CGSPropertyRightPCS::OnInitDialog()
{
  CDialog::OnInitDialog();
    
  m_CB_PCS.ResetContent();
  for(int i=0;i<(int)pcs_vector.size();i++)
  {
    m_CB_PCS.AddString(pcs_vector[i]->pcs_type_name.data());
    m_CB_PCS.SetCurSel(i);
  }
  
  UpdateData(FALSE);
  return TRUE;
}

void CGSPropertyRightPCS::OnBnClickedButtonPCS()
{
  CDialogPCS m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)pcs_vector.size()>0) m_check_pcs = true;
    else m_check_pcs = false;  
    CGSProject* m_gsp = GSPGetMember("msh");
    if(m_gsp)
    {
      string pcs_base_type = m_gsp->base + ".pcs";
      GSPAddMember(pcs_base_type);
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_pcs = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonTIM()
{
  CDialogTimeDiscretization m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)time_vector.size()>0) m_check_tim = true;
    else m_check_tim = false;  
    CGSProject* m_gsp = GSPGetMember("msh");
    if(m_gsp)
    {
      string tim_base_type = m_gsp->base + ".tim";
      GSPAddMember(tim_base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_tim = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonNUM()
{
  CDialogNUM m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)num_vector.size()>0) m_check_num = true;
    else m_check_num = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string num_base_type = m_gsp->base + ".num";
      GSPAddMember(num_base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_num = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonOUT()
{
  CDialogOUT m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)out_vector.size()>0) m_check_out = true;
    else m_check_out = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string out_base_type = m_gsp->base + ".out";
      GSPAddMember(out_base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_out = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonIC()
{
  CDialogInitialConditions m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)ic_vector.size()>0) m_check_ic = true;
    else m_check_ic = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string base_type = m_gsp->base + ".ic";
      GSPAddMember(base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_ic = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonBC()
{
  CBoundaryConditions m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)bc_list.size()>0) m_check_bc = true;
    else m_check_bc = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string base_type = m_gsp->base + ".bc";
      GSPAddMember(base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_bc = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonST()
{
  CDialogSourceTerms m_dlg;
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)st_vector.size()>0) m_check_st = true;
    else m_check_st = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string base_type = m_gsp->base + ".st";
      GSPAddMember(base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_st = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonMFP()
{
  CDialogMFP m_dlg;
  m_dlg.DoModal();
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonMSP()
{
  //OK MAT_Mech_dlg m_dlg;
  CMATGroupEditor m_dlg;
  m_dlg.mat_type = 1; // MSP
  MSPStandardKeywords();
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)mmp_vector.size()>0) m_check_msp = true;
    else m_check_msp = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string base_type = m_gsp->base + ".msp";
      GSPAddMember(base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_msp = false;
  }
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonMCP()
{
//  CDialogMCP m_dlg;
//  m_dlg.DoModal();
  AfxMessageBox("Not implemented");
}

void CGSPropertyRightPCS::OnBnClickedButtonMMP()
{
  CMATGroupEditor m_dlg;
  m_dlg.mat_type = 0; // MMP
  if (m_dlg.DoModal()==IDOK)
  {
    if((int)mmp_vector.size()>0) m_check_mmp = true;
    else m_check_mmp = false;  
    CGSProject* m_gsp = GSPGetMember("pcs");
    if(m_gsp)
    {
      string base_type = m_gsp->base + ".mmp";
      GSPAddMember(base_type); 
    }
    else
    {
      AfxMessageBox("Error: Updating GSP failed");
    }
  }
  else
  {
    m_check_mmp = false;
  }
  UpdateData(FALSE);
}

/**************************************************************************
OGS-GUI-Method:
12/2007 OK Implementation
05/2009 OK Create MOD
**************************************************************************/
void CGSPropertyRightPCS::OnBnClickedButtonMOD()
{
  //......................................................................
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Create MOD");
  try
  {
    //----------------------------------------------------------------------
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    //......................................................................
    if(m_pDoc->problem_win)
    {
      delete m_pDoc->problem_win; // problem if repeated
      CGSProject* m_gsp = GSPGetMember("pcs");
      if(m_gsp)
      {
        GSPReadData(m_gsp->path+m_gsp->base);
      }
      else
      {
        AfxMessageBox("no GSP data. Read Data failed");
        return;
      }
      MSHConfig();
      if(!MODCreate())
      {
        AfxMessageBox("Not enough data for MOD creation.");
      }
      m_pDoc->problem_win = new Problem();
      aproblem = m_pDoc->problem_win;
      //....................................................................
      if(m_pDoc->problem_win->print_result)
      //GetDlgItem(IDC_BUTTON_OBJ) -> EnableWindow(FALSE);
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MOD created");
    }
  }
  catch(...)
  {
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"no MOD created");    
  }
}

void CGSPropertyRightPCS::OnBnClickedButtonMODCheck()
{
  PCSCheck();
}

void CGSPropertyRightPCS::OnBnClickedButtonUpdateTree()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
//  m_frame->m_something_changed = 1;
  m_frame->m_rebuild_formtree = 1;
//  m_frame->UpdateSpecificView("CTabSplitterFrame", m_pDoc);
  m_pDoc->UpdateAllViews(NULL);
  //Invalidate(TRUE);
}

void CGSPropertyRightPCS::OnCbnSelchangeComboPCS()
{
  m_check_pcs = false;
  m_check_tim = false;
  m_check_num = false;
  m_check_out = false;
  m_check_ic  = false;
  m_check_bc  = false;
  m_check_st  = false;
  m_check_mfp = false;
  m_check_msp = false;
  m_check_mcp = false;
  m_check_mmp = false;
  m_check_mod = false;

  for(int i=0;i<(int)pcs_vector.size();i++)
  {
    pcs_vector[i]->selected = false;
  }

  m_CB_PCS.GetLBText(m_CB_PCS.GetCurSel(),m_strPCSTypeName);
  CRFProcess* m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs)
  {
    m_check_pcs = true;
    if(TIMGet((string)m_strPCSTypeName))
      m_check_tim = true;
    if(NUMGet((string)m_strPCSTypeName))
      m_check_num = true;
    if(OUTGet((string)m_strPCSTypeName))
      m_check_out = true;
    if(ICGet((string)m_strPCSTypeName))
      m_check_ic = true;
    if(BCGet((string)m_strPCSTypeName))
      m_check_bc = true;
    if(STGet((string)m_strPCSTypeName))
      m_check_st = true;
    if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
      if((int)mfp_vector.size()>0) m_check_mfp = true;
    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
      if((int)msp_vector.size()>0) m_check_msp = true;
    if(m_pcs->pcs_type_name.find("HEAT")!=string::npos)
      if((int)msp_vector.size()>0) m_check_msp = true;
    if(m_pcs->pcs_type_name.find("MASS")!=string::npos)
      if((int)cp_vec.size()>0) m_check_mcp = true;
    m_check_mmp = MSHTestMATGroups();
    m_pcs->selected = true;
  }
  m_CB_PCS.SetCurSel(m_CB_PCS.GetCurSel());
  UpdateData(FALSE);
}

void CGSPropertyRightPCS::OnBnClickedButtonMODRead()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->OnAddFEM();
  OnInitDialog();
}

void CGSPropertyRightPCS::OnBnClickedButtonMODEditor()
{
  CDialogMOD m_dlg;
  if (m_dlg.DoModal()==IDOK) 
  { 
  }
}
