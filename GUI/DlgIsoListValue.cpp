// DlgIsoListValue.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "DlgIsoListValue.h"
#include ".\dlgisolistvalue.h"
#include "MainFrm.h"
#include ".\gsformrightpassive.h"

// CDlgIsoListValue dialog

IMPLEMENT_DYNAMIC(CDlgIsoListValue, CDialog)
CDlgIsoListValue::CDlgIsoListValue(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIsoListValue::IDD, pParent)
{
	m_max = 0.0;
	m_min = 0.0;
	m_step = 0.0;
	m_pParent = NULL;
}

CDlgIsoListValue::~CDlgIsoListValue()
{
}

void CDlgIsoListValue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Text(pDX,IDC_Max, m_max);
	DDX_Text(pDX,IDC_Min, m_min);
	DDX_Text(pDX,IDC_Step, m_step);
}


BEGIN_MESSAGE_MAP(CDlgIsoListValue, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgIsoListValue message handlers

BOOL CDlgIsoListValue::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
	m_max = m_frame->Iso_Max_Value;
	m_min = m_frame->Iso_Min_Value;
	m_step = m_frame->Iso_Step_Value;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgIsoListValue::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	OnOK();
}

void CDlgIsoListValue::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::PostNcDestroy();
}
