// GS_Tolerance.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_tolerance.h"

//GEO
#include "geo_lib.h"
#include "geo_pnt.h"

// CGS_Tolerance dialog

IMPLEMENT_DYNAMIC(CGS_Tolerance, CDialog)
CGS_Tolerance::CGS_Tolerance(CWnd* pParent /*=NULL*/)
	: CDialog(CGS_Tolerance::IDD, pParent)
{
	m_tolerancefactor=0.000;
}

CGS_Tolerance::~CGS_Tolerance()
{
}

void CGS_Tolerance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TOLERANCE, m_tolerancefactor);

}


BEGIN_MESSAGE_MAP(CGS_Tolerance, CDialog)
	    ON_BN_CLICKED(IDOK, OnOk)
		ON_EN_CHANGE(IDC_EDIT_TOLERANCE, OnEnChangeEditTolerance)
END_MESSAGE_MAP()


// CGS_Tolerance message handlers

void CGS_Tolerance::OnOk()
{
	UpdateData(TRUE);
    GEO_Search_DoublePoints(m_tolerancefactor);
	CDialog::OnOK();
}

void CGS_Tolerance::OnEnChangeEditTolerance()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
