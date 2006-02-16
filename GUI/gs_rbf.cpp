// gs_rbf.cpp : interpolate method: radial basis function
// 07/2004 CC Implementation

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_rbf.h"
#include ".\gs_rbf.h"


// CGSRbf dialog

IMPLEMENT_DYNAMIC(CGSRbf, CDialog)
CGSRbf::CGSRbf(CWnd* pParent /*=NULL*/)
	: CDialog(CGSRbf::IDD, pParent)
    , m_settings(0)
    , m_ratiorbfvalue(1)
    , m_anglerbfvalue(0)
    , m_rsquaredvalue(16)
{
}

CGSRbf::~CGSRbf()
{
}

void CGSRbf::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO_DEFAULT2, m_settings);
    DDX_Control(pDX, IDC_EDIT_RATIO2, m_ratiorbf);
    DDX_Text(pDX, IDC_EDIT_RATIO2, m_ratiorbfvalue);
    DDX_Control(pDX, IDC_EDIT_ANGLE2, m_anglerbf);
    DDX_Text(pDX, IDC_EDIT_ANGLE2, m_anglerbfvalue);
    DDX_Control(pDX, IDC_COMBO_FUNCTION, m_function);
    DDX_Control(pDX, IDC_EDIT_R2, m_rsquared);
    DDX_Text(pDX, IDC_EDIT_R2, m_rsquaredvalue);
}


BEGIN_MESSAGE_MAP(CGSRbf, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_RADIO_DEFAULT2, OnBnClickedRadioDefault2)
    ON_BN_CLICKED(IDC_RADIO_USERDEFINED2, OnBnClickedRadioUserdefined2)
END_MESSAGE_MAP()

/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
BOOL CGSRbf::OnInitDialog() 
{
  CDialog::OnInitDialog();
  m_ratiorbf.EnableWindow(0);
  m_anglerbf.EnableWindow(0);
  m_rsquared.EnableWindow (0);
  
  m_function.InitStorage(3,1024);
  m_function.ResetContent();
  m_function.AddString("Inverse Multiquadratic");
  m_function.AddString("Multilog");
  m_function.AddString("Multiquadratic");
  m_function.AddString("Natural Cubic Spline");
  m_function.AddString("Thin Plate Spline");
  m_function.SetCurSel(2);
  m_function.EnableWindow(0);

  return TRUE; // return TRUE unless you set the focus to a contro	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSRbf::OnBnClickedOk()
{
    m_functionvalue = m_function.GetCurSel();
    UpdateData(TRUE);
    OnOK();
}
/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSRbf::OnBnClickedRadioDefault2()
{
 m_ratiorbfvalue = 1;
 m_anglerbfvalue= 0;
 m_rsquaredvalue= 16;
 
 m_ratiorbf.EnableWindow(0);
 m_anglerbf.EnableWindow(0);
 
 m_rsquared.EnableWindow(0);
 m_function.SetCurSel(2);
 m_function.EnableWindow(0);
 UpdateData(FALSE);
}

/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSRbf::OnBnClickedRadioUserdefined2()
{
  m_ratiorbf.EnableWindow(1);
  m_anglerbf.EnableWindow(1);
  m_rsquared.EnableWindow(1);
  m_function.EnableWindow(1);
}
