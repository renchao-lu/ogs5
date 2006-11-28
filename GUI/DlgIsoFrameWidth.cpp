// DlgIsoFrameWidth.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "DlgIsoFrameWidth.h"
#include ".\dlgisoframewidth.h"


// CDlgIsoFrameWidth dialog

IMPLEMENT_DYNAMIC(CDlgIsoFrameWidth, CDialog)
CDlgIsoFrameWidth::CDlgIsoFrameWidth(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIsoFrameWidth::IDD, pParent)
{
	Iso_Frame_Width = 1.0;
}

CDlgIsoFrameWidth::~CDlgIsoFrameWidth()
{
}

void CDlgIsoFrameWidth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_EDIT1, Iso_Frame_Width);
}


BEGIN_MESSAGE_MAP(CDlgIsoFrameWidth, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgIsoFrameWidth message handlers

void CDlgIsoFrameWidth::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	OnOK();
}
