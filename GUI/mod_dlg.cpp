// mod_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "mod_dlg.h"
#include ".\mod_dlg.h"

#include "MainFrm.h"
#include "GeoSysDoc.h"
#include "gs_meshing.h"

// CDialogMOD dialog

IMPLEMENT_DYNAMIC(CDialogMOD, CDialog)
CDialogMOD::CDialogMOD(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMOD::IDD, pParent)
    , m_bCheckMOD_H(FALSE)
    , m_bCheckMSH_H(FALSE)
    , m_bCheckPCS_H(FALSE)
    , m_bCheckOBJ_H(FALSE)
    , m_bCheckEQS_H(FALSE)
    , m_bCheckNOD_H(FALSE)
    , m_bCheckELE_H(FALSE)
    , m_bCheckPCS(FALSE)
    , m_bCheckMOD_M(FALSE)
    , m_bCheckMOD_T(FALSE)
{
  m_bUseExistingMSH = true;
}

CDialogMOD::~CDialogMOD()
{
}

void CDialogMOD::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_MOD_H, m_CB_MOD_H);
    DDX_Check(pDX, IDC_CHECK_MOD_H, m_bCheckMOD_H);
    DDX_Check(pDX, IDC_CHECK_MSH_H, m_bCheckMSH_H);
    DDX_Check(pDX, IDC_CHECK_PCS_H, m_bCheckPCS_H);
    DDX_Check(pDX, IDC_CHECK_OBJ_H, m_bCheckOBJ_H);
    DDX_Check(pDX, IDC_CHECK_EQS_H, m_bCheckEQS_H);
    DDX_Check(pDX, IDC_CHECK_NOD_H, m_bCheckNOD_H);
    DDX_Check(pDX, IDC_CHECK_ELE_H, m_bCheckELE_H);
    DDX_Control(pDX, IDC_LIST_PCS, m_LB_PCS);
    DDX_Control(pDX, IDC_COMBO_MOD_M, m_CB_MOD_M);
    DDX_Control(pDX, IDC_COMBO_MOD_T, m_CB_MOD_T);
    DDX_Control(pDX, IDC_COMBO_MOD_C, m_CB_MOD_C);
    DDX_Check(pDX, IDC_CHECK_PCS, m_bCheckPCS);
    DDX_Check(pDX, IDC_CHECK_MOD_M, m_bCheckMOD_M);
    DDX_Check(pDX, IDC_CHECK_MOD_T, m_bCheckMOD_T);
}


BEGIN_MESSAGE_MAP(CDialogMOD, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_MOD_READ, OnBnClickedButtonMODRead)
    ON_BN_CLICKED(IDC_BUTTON_MOD_CREATE_H, OnBnClickedButtonMODCreate)
    ON_BN_CLICKED(IDC_BUTTON_MOD_RUN, OnBnClickedButtonMODRun)
    ON_BN_CLICKED(IDC_BUTTON_MSH_EDITOR, OnBnClickedButtonMSHEditor)
    ON_BN_CLICKED(IDC_BUTTON_MOD_DELETE_H, OnBnClickedButtonMODDelete)
    ON_LBN_SELCHANGE(IDC_LIST_PCS, OnLbnSelchangeListPCS)
    ON_BN_CLICKED(IDC_CHECK_PCS, OnBnClickedCheckPCS)
    ON_BN_CLICKED(IDC_BUTTON_MOD_READ_M, OnBnClickedButtonMODReadM)
    ON_BN_CLICKED(IDC_BUTTON_MOD_READ_T, OnBnClickedButtonMODReadT)
END_MESSAGE_MAP()


// CDialogMOD message handlers

/**************************************************************************
GeoSys-GUI-Method:
07/2007 OK Implementation
**************************************************************************/
BOOL CDialogMOD::OnInitDialog() 
{
  int i;
  CRFProcess* m_pcs = NULL;
  //---------------------------------------------------------------------------
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // PCS types
  //---------------------------------------------------------------------------
  m_CB_MOD_H.ResetContent();
  m_CB_MOD_M.ResetContent();
  m_CB_MOD_T.ResetContent();
  m_CB_MOD_C.ResetContent();
  //m_CB_MOD_H.InitStorage(10,80);
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSTypeName = (CString)m_pcs->pcs_type_name.c_str();
    //....................................................................
    if(m_pcs->m_msh)
      m_bCheckMSH_H = m_pcs->m_msh->m_bCheckMSH;
    m_bCheckOBJ_H = m_pcs->m_bCheckOBJ;
    m_bCheckNOD_H = m_pcs->m_bCheckNOD;
    m_bCheckELE_H = m_pcs->m_bCheckELE;
    m_bCheckEQS_H = m_pcs->m_bCheckEQS;
    m_bCheckPCS = m_pcs->m_bCheck;
    //....................................................................
    if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
    {
      if(m_pcs->selected) 
        m_bCheckMOD_H = true;
      else
        m_bCheckMOD_H = false;
      m_CB_MOD_H.AddString(m_strPCSTypeName);
      m_CB_MOD_H.SetCurSel(0);
    }
    //....................................................................
    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
    {
      if(m_pcs->selected) 
        m_bCheckMOD_M = true;
      else
        m_bCheckMOD_M = false;
      m_CB_MOD_M.AddString(m_strPCSTypeName);
      m_CB_MOD_M.SetCurSel(0);
    }
    //....................................................................
    if(m_pcs->pcs_type_name.find("HEAT")!=string::npos)
    {
      if(m_pcs->selected) 
        m_bCheckMOD_T = true;
      else
        m_bCheckMOD_T = false;
      m_CB_MOD_T.AddString(m_strPCSTypeName);
      m_CB_MOD_T.SetCurSel(0);
    }
    //....................................................................
    if(m_pcs->pcs_type_name.find("MASS")!=string::npos)
    {
      m_CB_MOD_C.AddString(m_strPCSTypeName);
      m_CB_MOD_C.SetCurSel(0);
    }
  }
  //---------------------------------------------------------------------------
  m_LB_PCS.ResetContent();
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    m_strPCSTypeName = (CString)m_pcs->pcs_type_name.c_str();
    m_LB_PCS.AddString(m_strPCSTypeName);
  }  
  //---------------------------------------------------------------------------
  Check();
  //----------------------------------------------------------------------
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  //----------------------------------------------------------------------
  return TRUE;
}

void CDialogMOD::OnBnClickedButtonMODRead()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->OnAddFEM();
  m_strPCSTypeName = m_pDoc->m_strPCSTypeName;
  OnBnClickedButtonMODCreate();
  OnInitDialog();
}

void CDialogMOD::OnBnClickedButtonMODCreate()
{
  CFEMesh* m_msh = NULL;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSTypeName);
  //----------------------------------------------------------------------
  if(m_pcs)
  {
    //....................................................................
    if(!m_bUseExistingMSH)
    {
      m_msh = MSHGet((string)m_strPCSTypeName);
      if(!m_msh)
      {
        AfxMessageBox("Create MOD failed - no MSH data");
        return;
      }
      if(!m_msh->m_bCheckMSH)
      {
        if(!CompleteMesh((string)m_strPCSTypeName))
        {
          AfxMessageBox("Create MOD failed - no MSH data");
          return;
        }
      }
    }
    //....................................................................
    ConfigSolverProperties();
    //....................................................................
    m_pcs->CreateNew();
    m_bCheckOBJ_H = m_pcs->m_bCheckOBJ;
    m_bCheckNOD_H = m_pcs->m_bCheckNOD;
    m_bCheckELE_H = m_pcs->m_bCheckELE;
    m_bCheckEQS_H = m_pcs->m_bCheckEQS;
    m_bCheckMSH_H = m_pcs->m_msh->m_bCheckMSH;
    m_bCheckPCS = m_pcs->m_bCheck;
    //....................................................................
  }
  else
    AfxMessageBox("Create MOD failed - no PCS data");
  //----------------------------------------------------------------------
  OnInitDialog();
}

void CDialogMOD::OnBnClickedButtonMODRun()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->OnSimulatorForward();
}

void CDialogMOD::OnBnClickedButtonMSHEditor()
{
  CMSHEditor m_dlg;
  if (m_dlg.DoModal()==IDOK) 
  { 
  }
}

void CDialogMOD::OnBnClickedButtonMODDelete()
{
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs)
    m_pcs->Delete();
  m_pcs->m_bCheckOBJ = false;
  m_pcs->m_bCheckNOD = false;
  m_pcs->m_bCheckELE = false;
  m_pcs->m_bCheckEQS = false;
  OnInitDialog();
}

void CDialogMOD::OnLbnSelchangeListPCS()
{
  int nSel = m_LB_PCS.GetCurSel();
  m_LB_PCS.GetText(nSel,m_strPCSTypeName);
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs)
  {
    m_bCheckOBJ_H = m_pcs->m_bCheckOBJ;
    m_bCheckNOD_H = m_pcs->m_bCheckNOD;
    m_bCheckELE_H = m_pcs->m_bCheckELE;
    m_bCheckEQS_H = m_pcs->m_bCheckEQS;
    m_bCheckPCS = m_pcs->m_bCheck;
  }
  if(m_pcs->m_msh)
    m_bCheckMSH_H = m_pcs->m_msh->m_bCheckMSH;
  Check();
}

void CDialogMOD::Check()
{
  if(m_bCheckMSH_H&&m_bCheckOBJ_H&&m_bCheckELE_H&&m_bCheckNOD_H&&m_bCheckEQS_H)
  {
    GetDlgItem(IDC_BUTTON_MOD_RUN)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_MOD_CREATE_H)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_MOD_DELETE_H)->EnableWindow(TRUE);
  }
  else
  {
    GetDlgItem(IDC_BUTTON_MOD_RUN)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_MOD_CREATE_H)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_MOD_DELETE_H)->EnableWindow(FALSE);
  }
  UpdateData(FALSE);
}

void CDialogMOD::OnBnClickedCheckPCS()
{
  UpdateData(TRUE);
  CRFProcess* m_pcs = PCSGet((string)m_strPCSTypeName);
  if(m_pcs)
  {
    m_pcs->m_bCheck = m_bCheckPCS;
  }
}

void CDialogMOD::OnBnClickedButtonMODReadM()
{
  OnBnClickedButtonMODRead();
}

void CDialogMOD::OnBnClickedButtonMODReadT()
{
  OnBnClickedButtonMODRead();
}
