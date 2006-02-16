// gs_interpolation.cpp : implementation file
//
/**************************************************************************
GeoLibGUI-Method:Interpolation: inverse distance weighting 
Task: 
Programing:
07/2004 CC Implementation

**************************************************************************/
#include "stdafx.h"
#include "GeoSys.h"
#include "gs_idw.h"
#include ".\gs_idw.h"


// CGSIdw dialog

IMPLEMENT_DYNAMIC(CGSIdw, CDialog)
CGSIdw::CGSIdw(CWnd* pParent /*=NULL*/)
	: CDialog(CGSIdw::IDD, pParent)
 
    , m_settings(0)
    , m_ratiovalue(1)
    , m_anglevalue(0)
    , m_powervalue(2)
    , m_smoothingvalue(0)
{
  

}

CGSIdw::~CGSIdw()
{
}

void CGSIdw::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO_DEFAULT, m_settings);
    DDX_Control(pDX, IDC_EDIT_RATIO, m_ratio);
    DDX_Text(pDX, IDC_EDIT_RATIO, m_ratiovalue);
    DDX_Control(pDX, IDC_EDIT_ANGLE, m_angle);
    DDX_Text(pDX, IDC_EDIT_ANGLE, m_anglevalue);
    DDX_Control(pDX, IDC_EDIT_POWER, m_power);
    DDX_Text(pDX, IDC_EDIT_POWER, m_powervalue);
    DDX_Control(pDX, IDC_EDIT_SMOOTHING, m_smoothing);
    DDX_Text(pDX, IDC_EDIT_SMOOTHING, m_smoothingvalue);
}
BEGIN_MESSAGE_MAP(CGSIdw, CDialog)
    ON_BN_CLICKED(IDOK, OnOk)
  
    ON_BN_CLICKED(IDC_RADIO_USERDEFINED, OnBnClickedRadioUserdefined)
    ON_BN_CLICKED(IDC_RADIO_DEFAULT, OnBnClickedRadioDefault)
   
END_MESSAGE_MAP()
/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
BOOL CGSIdw::OnInitDialog() 
{
  CDialog::OnInitDialog();
  m_ratio.EnableWindow(0);
  m_angle.EnableWindow(0);
  m_power.EnableWindow(0);
  m_smoothing.EnableWindow(0);

  return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoSys GUI - Object: OnOk
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSIdw::OnOk()
{
    // TODO: Add your control notification handler code here
    
    UpdateData(TRUE);

    OnOK();
}

/**************************************************************************
GeoSys GUI - Object: OnBnClickedRadioUserdefined
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSIdw::OnBnClickedRadioUserdefined()
{
  m_settings = 1;
  m_ratio.EnableWindow(1);
  m_angle.EnableWindow(1);
  m_power.EnableWindow(1);
  m_smoothing.EnableWindow(1);
}
/**************************************************************************
GeoSys GUI - Object: OnBnClickedRadioUserdefined
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void CGSIdw::OnBnClickedRadioDefault()
{
 m_settings = 0;
 m_ratiovalue = 1;
 m_anglevalue = 0;
 m_powervalue = 2;
 m_smoothingvalue = 0;
 
 m_ratio.EnableWindow(0);
 m_angle.EnableWindow(0);
 m_power.EnableWindow(0);
 m_smoothing.EnableWindow(0);
 UpdateData(FALSE);

}