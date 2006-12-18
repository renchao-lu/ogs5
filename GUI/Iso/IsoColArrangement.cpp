// IsoColArrangement.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "IsoColArrangement.h"
#include ".\isocolarrangement.h"


// CIsoColArrangement dialog

IMPLEMENT_DYNAMIC(CIsoColArrangement, CDialog)
CIsoColArrangement::CIsoColArrangement(CWnd* pParent /*=NULL*/)
	: CDialog(CIsoColArrangement::IDD, pParent)
{
	UniformCol = RGB(0,0,0);
	UsingUniformCol = FALSE;
}

CIsoColArrangement::~CIsoColArrangement()
{
}

void CIsoColArrangement::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_UsingContourPlotCol, UsingUniformCol);
}


BEGIN_MESSAGE_MAP(CIsoColArrangement, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CIsoColArrangement message handlers

void CIsoColArrangement::OnBnClickedButton1()
{
	UpdateData(true);
	if ( UsingUniformCol == TRUE)
	{
		CColorDialog dlg;
		if ( dlg.DoModal() == IDOK )
		{
			UniformCol = dlg.GetColor();
		}
	}
}

void CIsoColArrangement::OnBnClickedOk()
{
	UpdateData(true);
	OnOK();
}
