// mfp_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "mfp_dlg.h"
#include "gs_project.h"
#include ".\mfp_dlg.h"


// CDialogMFP dialog

IMPLEMENT_DYNAMIC(CDialogMFP, CDialog)
CDialogMFP::CDialogMFP(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMFP::IDD, pParent)
{
}

CDialogMFP::~CDialogMFP()
{
}

void CDialogMFP::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_MFP_STD, m_CB_MFP_STD);
    DDX_Control(pDX, IDC_LISTCONTROL_MMP, m_LC);
}

BEGIN_MESSAGE_MAP(CDialogMFP, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_MFP_STD, OnCbnSelchangeComboMFPStd)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_MFP, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_MFP, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_MFP_CLOSE, &CDialogMFP::OnBnClickedButtonMFPClose)
END_MESSAGE_MAP()

// CDialogMFP message handlers

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
BOOL CDialogMFP::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // MFP STD types
  m_CB_MFP_STD.ResetContent();
  m_CB_MFP_STD.InitStorage(2,80);
  m_CB_MFP_STD.AddString("WATER");
  m_CB_MFP_STD.AddString("AIR");
  m_CB_MFP_STD.SetCurSel(0);
  //----------------------------------------------------------------------
  // MFP data
  CRect rect;
  m_LC.GetClientRect(&rect);
  m_LC.SetExtendedStyle (LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
  m_LC.DeleteAllItems(); // Delete the current contents
  // create columns
  int nColInterval = rect.Width()/5;
  m_LC.DeleteColumn(0);
  m_LC.InsertColumn(0,_T("MFP No."),LVCFMT_LEFT, nColInterval);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,_T("MFP Type"),LVCFMT_LEFT, nColInterval*2);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,_T("GEO Type"),LVCFMT_LEFT, nColInterval*2);
  //......................................................................
  CString strItem;
  LVITEM lvi;
  for(int i=0;i<(int)mfp_vector.size();i++){
    m_mfp = mfp_vector[i];
    lvi.mask =  LVIF_TEXT;
    // 1 column
	strItem.Format("%i",i);
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
    m_LC.InsertItem(&lvi);
    // 2 column
    lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(m_mfp->name.c_str());
	m_LC.SetItem(&lvi);
    // 3 column
    lvi.iSubItem = 2;
	lvi.pszText = (LPTSTR)(LPCTSTR)(m_mfp->name.c_str());
	m_LC.SetItem(&lvi);
  }
  //---------------------------------------------------------------------------
  return TRUE;
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogMFP::OnCbnSelchangeComboMFPStd()
{
  m_CB_MFP_STD.GetLBText(m_CB_MFP_STD.GetCurSel(),m_strMFPName);
  GetDlgItem(IDC_BUTTON_CREATE_MFP)->EnableWindow(TRUE);
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogMFP::OnBnClickedButtonCreate()
{
  m_mfp = new CFluidProperties();
  m_mfp->name = m_strMFPName;
  //m_mfp->Create((string)m_strMFPName);
  mfp_vector.push_back(m_mfp);
  //----------------------------------------------------------------------
  OnInitDialog();
  //----------------------------------------------------------------------
  if(mfp_vector.size()>0){
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    GSPAddMember(m_gsp->base + ".mfp");
  else
    AfxMessageBox("Warning: no PCS data");
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
GUILib-Method:
05/2005 OK Implementation
**************************************************************************/
void CDialogMFP::OnBnClickedButtonWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("pcs");
  if(m_gsp)
    MFPWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
GUILib-Method:
06/2009 OK Implementation
**************************************************************************/
void CDialogMFP::OnBnClickedButtonMFPClose()
{
  OnOK();
}
