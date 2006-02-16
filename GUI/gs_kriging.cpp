// gs_kriging.cpp : implementation file
// 07/2004 CC implementation

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_kriging.h"
#include ".\gs_kriging.h"


// CGSKriging dialog

IMPLEMENT_DYNAMIC(CGSKriging, CDialog)
CGSKriging::CGSKriging(CWnd* pParent /*=NULL*/)
	: CDialog(CGSKriging::IDD, pParent)
    , m_settings(0)
    , m_ratiovalue(1)
    , m_anglevalue(0)
    , m_nuggetvalue(0)
    , m_slopevalue(1)
    , m_sillvalue(1)
    , m_rangevalue(1)
    , m_check(FALSE)
    , m_nugget(FALSE)
{
}

CGSKriging::~CGSKriging()
{
}

void CGSKriging::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO_DEFAULT_KRIGING, m_settings);
    DDX_Control(pDX, IDC_EDIT_RATIO, m_ratio);
    DDX_Text(pDX, IDC_EDIT_RATIO, m_ratiovalue);
    DDX_Control(pDX, IDC_EDIT_ANGLE, m_angle);
    DDX_Text(pDX, IDC_EDIT_ANGLE, m_anglevalue);

    DDX_Text(pDX, IDC_EDIT_ANGLE_NUGGET, m_nuggetvalue);
    DDX_Control(pDX, IDC_EDIT_POWER_SLOPE, m_slope);
    DDX_Text(pDX, IDC_EDIT_POWER_SLOPE, m_slopevalue);
    DDX_Control(pDX, IDC_EDIT_SILL, m_sill);
    DDX_Text(pDX, IDC_EDIT_SILL, m_sillvalue);
    DDX_Control(pDX, IDC_EDIT_RANGE, m_range);
    DDX_Text(pDX, IDC_EDIT_RANGE, m_rangevalue);

    DDX_Control(pDX, IDC_COMBO_VARIO, m_variogram);
    DDX_Control(pDX, IDC_COMBO_TREND, m_trendbox);
    DDX_Check(pDX, IDC_CHECK_TREND, m_check);
    DDX_Check(pDX, IDC_CHECK_NUGGET, m_nugget);
    DDX_Control(pDX, IDC_EDIT_ANGLE_NUGGET, m_nuggetedit);
}


BEGIN_MESSAGE_MAP(CGSKriging, CDialog)
   
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_RADIO_DEFAULT_KRIGING, OnBnClickedRadioDefaultKriging)
    ON_BN_CLICKED(IDC_CHECK_NUGGET, OnBnClickedCheckNugget)
    ON_BN_CLICKED(IDC_CHECK_TREND, OnBnClickedCheckTrend)
    ON_BN_CLICKED(IDC_RADIO_USERDEFINED_3, OnBnClickedRadioUserdefined3)
    ON_CBN_SELCHANGE(IDC_COMBO_VARIO, OnCbnSelchangeComboVario)
    ON_CBN_SELCHANGE(IDC_COMBO_TREND, OnCbnSelchangeComboTrend)
END_MESSAGE_MAP()


// CGSKriging message handlers


/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
BOOL CGSKriging::OnInitDialog() 
{
  CDialog::OnInitDialog();
  m_ratio.EnableWindow(0);
  m_angle.EnableWindow(0);
  m_slope.EnableWindow(0);
  m_sill.EnableWindow(0);
  m_range.EnableWindow(0);
  m_nuggetedit.EnableWindow(0);

  m_variogram.InitStorage(3,1024);
  m_variogram.ResetContent();
  m_variogram.AddString("Linear");
  m_variogram.AddString("Exponential");
  m_variogram.AddString("Gaussian");
  m_variogram.AddString("Spherical");
  m_variogram.SetCurSel(0);
  m_variogram.EnableWindow(0);


  m_trendbox.InitStorage(3,1024);
  m_trendbox.ResetContent();
  m_trendbox.AddString("No trend");
  m_trendbox.AddString("1st order");
  m_trendbox.AddString("2nd order");
  m_trendbox.SetCurSel(0);
  m_trendbox.EnableWindow(0);

  m_check = false;
  m_nugget = false;
  
  UpdateData(false);

  return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CGSKriging::OnBnClickedOk()
{
    UpdateData(true);
    OnOK();
}

/**************************************************************************
GeoSys GUI - Object: OnBnClickedRadioDefaultKriging
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnBnClickedRadioDefaultKriging()
{
  m_settings = 0;
  m_ratio.EnableWindow(0);
  m_angle.EnableWindow(0);
  m_slope.EnableWindow(0);
  m_sill.EnableWindow(0);
  m_range.EnableWindow(0);
  m_nuggetedit.EnableWindow(0);

  m_variogram.SetCurSel(0);
  m_variogram.EnableWindow(0);

  m_trendbox.SetCurSel(0);
  m_trendbox.EnableWindow(0);

  m_check = false;
  m_nugget = false;
  
  UpdateData(false);
}

/**************************************************************************
GeoSys GUI - Object: OnBnClickedRadioUserdefined3
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnBnClickedRadioUserdefined3()
{
  m_settings = 1;
  m_ratio.EnableWindow(1);
  m_angle.EnableWindow(1);
  m_slope.EnableWindow(1);
  m_sill.EnableWindow(1);
  m_range.EnableWindow(1);

  m_variogram.EnableWindow(1);

  m_sill.EnableWindow(0);
  m_range.EnableWindow(0);

  m_check = false;
  m_nugget = false;
  
  UpdateData(false);
}


/**************************************************************************
GeoSys GUI - Object: OnBnClickedCheckNugget
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnBnClickedCheckNugget()
{
   if(m_nugget == false)
   {
   m_nugget = true;
   m_nuggetedit.EnableWindow(1);

   }
   else
   {
   m_nugget = false;
   m_nuggetedit.EnableWindow(0);
   }

}

/**************************************************************************
GeoSys GUI - Object: OnBnClickedCheckTrend
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnBnClickedCheckTrend()
{
    if(m_check == false)
    {
    m_check = true;
    m_trendbox.EnableWindow(1);
    }
    else 
    {
    m_check = false;
    m_trendbox.EnableWindow(0);
    }
}
/**************************************************************************
GeoSys GUI - Object: OnCbnSelchangeComboVario
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnCbnSelchangeComboVario()
{
  if(m_variogram.GetCurSel() == 0)
  {
    m_sill.EnableWindow(0);
    m_range.EnableWindow(0);
    variotype = m_variogram.GetCurSel();
  }
  else
  {
    m_slope.EnableWindow(0);
      m_sill.EnableWindow(1);
    m_range.EnableWindow(1);
    variotype = m_variogram.GetCurSel();
  }

}
/**************************************************************************
GeoSys GUI - Object: OnCbnSelchangeComboVario
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSKriging::OnCbnSelchangeComboTrend()
{
    trendtype = m_trendbox.GetCurSel();
}
