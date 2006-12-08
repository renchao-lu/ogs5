// GSPropertyRightCamera.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSPropertyRightCamera.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "COGLPickingView.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_elements_rfi.h"
#include "msh_quality.h"
#include "dtmesh.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"

// CGSPropertyRightCamera dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightCamera, CViewPropertyPage)

CGSPropertyRightCamera::CGSPropertyRightCamera()
	: CViewPropertyPage(CGSPropertyRightCamera::IDD)
{
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();    
    mainframe->m_image_distort_factor_x = m_image_distort_factor_x = 1.0;
    mainframe->m_image_distort_factor_y = m_image_distort_factor_y = 1.0;
    mainframe->m_image_distort_factor_z = m_image_distort_factor_z = 1.0;
}

CGSPropertyRightCamera::~CGSPropertyRightCamera()
{
}

void CGSPropertyRightCamera::DoDataExchange(CDataExchange* pDX)
{
	CViewPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST, m_List);
    DDX_Text(pDX,IDC_X_IMAGEDISTORT, m_image_distort_factor_x);
    DDX_Text(pDX,IDC_Y_IMAGEDISTORT, m_image_distort_factor_y);
    DDX_Text(pDX,IDC_Z_IMAGEDISTORT, m_image_distort_factor_z);

  //OUTPUT LIST
  m_List.ResetContent();
  m_List.AddString(_T(">Updated View Information:"));
  m_List.AddString(_T(">"));
  if ((int)gli_points_vector.size() > 0)
  {
  OnPaint();
  }
}


BEGIN_MESSAGE_MAP(CGSPropertyRightCamera, CViewPropertyPage)
    ON_BN_CLICKED(IDC_BOUNDINGBOX_ON_OFF, OnBnClickedBoundingboxOnOff)
    ON_BN_CLICKED(IDC_BUTTON_NOT_DISTORT, OnNotDistortImage)
    ON_BN_CLICKED(IDC_BUTTON_DISTORT, OnDistortImage)
    ON_BN_CLICKED(IDC_POINT_NUMBERS_CHECK, OnBnClickedPointNumbersCheck)
    ON_BN_CLICKED(IDC_ELEMENTNUMBERS_ON_OFF, OnBnClickedElementnumbersOnOff)
    ON_BN_CLICKED(IDC_NODENUMBERS_ON_OFF, OnBnClickedNodenumbersOnOff)
    ON_BN_CLICKED(IDC_UPDATE_ALL_VIEWS, OnBnClickedUpdateAllViews)
    ON_BN_CLICKED(IDC_GEO_EDITOR_BUTTON, OnBnClickedGeoEditorButton)
    ON_BN_CLICKED(IDC_MSH_EDITOR_BUTTON, OnBnClickedMshEditorButton)
	ON_BN_CLICKED(IDC_BC_EDITOR_BUTTON, OnBnClickedBcEditorButton)
	ON_BN_CLICKED(IDC_GSP_EDITOR_BUTTON, OnBnClickedGspEditorButton)
	ON_BN_CLICKED(IDC_IC_EDITOR_BUTTON, OnBnClickedIcEditorButton)
	ON_BN_CLICKED(IDC_MCP_EDITOR_BUTTON, OnBnClickedMcpEditorButton)
	ON_BN_CLICKED(IDC_MMP_EDITOR_BUTTON, OnBnClickedMmpEditorButton)
	ON_BN_CLICKED(IDC_MFP_EDITOR_BUTTON, OnBnClickedMfpEditorButton)
	ON_BN_CLICKED(IDC_NUM_EDITOR_BUTTON, OnBnClickedNumEditorButton)
	ON_BN_CLICKED(IDC_OUT_EDITOR_BUTTON2, OnBnClickedOutEditorButton2)
	ON_BN_CLICKED(IDC_PCS_EDITOR_BUTTON, OnBnClickedPcsEditorButton)
	ON_BN_CLICKED(IDC_RFE_EDITOR_BUTTON, OnBnClickedRfeEditorButton)
	ON_BN_CLICKED(IDC_ST_EDITOR_BUTTON, OnBnClickedStEditorButton)
	ON_BN_CLICKED(IDC_TEC_EDITOR_BUTTON, OnBnClickedTecEditorButton)
	ON_BN_CLICKED(IDC_TIM_EDITOR_BUTTON, OnBnClickedTimEditorButton)
END_MESSAGE_MAP()


// CGSPropertyRightCamera message handlers


void CGSPropertyRightCamera::OnDataChange()
{
    	if (!UpdateData())
		return;

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();    

    mainframe->m_bounding_box = m_bounding_box;
    mainframe->m_element_numbers = m_element_numbers;
    mainframe->m_node_numbers = m_node_numbers;
    mainframe->m_image_distort_factor_x = m_image_distort_factor_x;
    mainframe->m_image_distort_factor_y = m_image_distort_factor_y;
    mainframe->m_image_distort_factor_z = m_image_distort_factor_z;

    mainframe->m_something_changed = 1;

	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    pDoc->UpdateAllViews(NULL);

}

void CGSPropertyRightCamera::OnBnClickedUpdateAllViews()
{
 CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 UpdateData(TRUE);

 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightCamera::OnBnClickedBoundingboxOnOff()
{
  if(m_bounding_box==1)m_bounding_box=0;
  else m_bounding_box=1;

  OnDataChange();   

}

void CGSPropertyRightCamera::OnBnClickedElementnumbersOnOff()
{
  if(m_element_numbers==1)m_element_numbers=0;
  else m_element_numbers=1;

  OnDataChange();   
}

void CGSPropertyRightCamera::OnBnClickedNodenumbersOnOff()
{
  if(m_node_numbers==1)m_node_numbers=0;
  else m_node_numbers=1;

  OnDataChange();   
}

void CGSPropertyRightCamera::OnBnClickedPointNumbersCheck()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();

  if(point_numbers_button_on==1)point_numbers_button_on=0;
  else point_numbers_button_on=1;

  if(point_numbers_button_on==1)
  {
    m_frame->m_3dcontrol_points = 1;
    m_frame->m_3dcontrol_point_numbers = 1; 
  }
  else
  {
    m_frame->m_3dcontrol_point_numbers = 0; 

  }

  OnDataChange();
  pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSPropertyRightCamera::OnPaint()
{
//OUTPUT LIST
  string info_string;
  //char string2add [56];
  m_List.ResetContent();
  m_List.AddString(_T(">INFO:"));
  m_List.AddString(_T(">***********************************"));
  if ((int)gli_points_vector.size() > 0) m_List.AddString(_T(">GEO data loaded"));
  if ((int)gli_points_vector.size() == 0) m_List.AddString(_T(">GEO data not loaded"));
  if ((int)fem_msh_vector.size() > 0) {
      m_List.AddString(_T(">MSH data loaded"));

      //sprintf(string2add, "%ld",NodeListSize());
      //info_string = "> Nodes:" ;
      //info_string.append(string2add);
      //m_List.AddString(_T(info_string.data()));
  
  }
  if ((int)fem_msh_vector.size() == 0) m_List.AddString(_T(">MSH data not loaded"));
  if ((int)pcs_vector.size() > 0) m_List.AddString(_T(">PCS data loaded"));
  if ((int)pcs_vector.size() == 0) m_List.AddString(_T(">PCS data not loaded"));
  m_List.AddString(_T(">***********************************"));
//

    
}

void CGSPropertyRightCamera::OnDistortImage()
{
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    pDoc->UpdateAllViews(NULL);
    OnDataChange();
}

void CGSPropertyRightCamera::OnNotDistortImage()
{

    m_image_distort_factor_x = 1.0;
    m_image_distort_factor_y = 1.0;
    m_image_distort_factor_z = 1.0;
    UpdateData(FALSE);
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    pDoc->UpdateAllViews(NULL);
    OnDataChange();
 
    // TODO: Add your control notification handler code here
}


void CGSPropertyRightCamera::OnBnClickedGeoEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString exe_call = "notepad.exe " + m_strFileNameGEO;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedMshEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}


void CGSPropertyRightCamera::OnBnClickedBcEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".bc";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}


void CGSPropertyRightCamera::OnBnClickedGspEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".gsp";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedIcEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".ic";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedMcpEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".mcp";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedMmpEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".mmp";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedMfpEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".mfp";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedNumEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".num";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedOutEditorButton2()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".out";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedPcsEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".pcs";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedRfeEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".rfe";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedStEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".st";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedTecEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".tec";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}

void CGSPropertyRightCamera::OnBnClickedTimEditorButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".tim";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}
