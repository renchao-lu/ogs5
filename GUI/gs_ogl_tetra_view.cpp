// gs_ogl_tetra_view.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_ogl_tetra_view.h"

// CGS_OGL_Tetra_View dialog

IMPLEMENT_DYNAMIC(CGS_OGL_Tetra_View, CDialog)
CGS_OGL_Tetra_View::CGS_OGL_Tetra_View(CWnd* pParent /*=NULL*/)
	: CDialog(CGS_OGL_Tetra_View::IDD, pParent)
{
	m_min_quality_value = 0.0;
	m_max_quality_value = 0.0;
}

CGS_OGL_Tetra_View::~CGS_OGL_Tetra_View()
{
}

void CGS_OGL_Tetra_View::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   	DDX_Text(pDX, IDC_EDIT_MIN_QUALITY_VALUE, m_min_quality_value);
	DDX_Text(pDX, IDC_EDIT_MAX_QUALITY_VALUE, m_max_quality_value);

}


BEGIN_MESSAGE_MAP(CGS_OGL_Tetra_View, CDialog)
END_MESSAGE_MAP()


// CGS_OGL_Tetra_View message handlers
