// gs_structured_mesh.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "gs_structured_mesh.h"


// CStructured_Mesh dialog

IMPLEMENT_DYNAMIC(CStructured_Mesh, CDialog)
CStructured_Mesh::CStructured_Mesh(CWnd* pParent /*=NULL*/)
	: CDialog(CStructured_Mesh::IDD, pParent)
{
    m_edit_x_min = 0.0;
    m_edit_y_min = 0.0;
    m_edit_x_max = 1.0;
    m_edit_y_max = 1.0;
    m_delta_x = 0;
    m_delta_y = 0;
    slope_x = 0.0;
    slope_y = 0.0;
    m_do_structured_triangle_mesh = false;
    m_do_structured_rectangle_mesh = false;

}

CStructured_Mesh::~CStructured_Mesh()
{
}

void CStructured_Mesh::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_X_Min, m_edit_x_min);
    DDX_Text(pDX, IDC_EDIT_Y_Min, m_edit_y_min);
    DDX_Text(pDX, IDC_EDIT_X_Max, m_edit_x_max);
    DDX_Text(pDX, IDC_EDIT_Y_Max, m_edit_y_max);
    DDX_Text(pDX, IDC_EDIT_SQUARE_DELTA_X, m_delta_x);
    DDX_Text(pDX, IDC_EDIT_SQUARE_DELTA_Y, m_delta_y);
    DDX_Check(pDX, IDC_CHECK_TRIANGLE_MESH, m_do_structured_triangle_mesh);
    DDX_Check(pDX, IDC_CHECK_RECTANGLE_MESH, m_do_structured_rectangle_mesh);
    DDX_Text(pDX, IDC_EDIT_SLOPE_X, slope_x);
    DDX_Text(pDX, IDC_EDIT_SLOPE_Y, slope_y);

}


BEGIN_MESSAGE_MAP(CStructured_Mesh, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CStructured_Mesh message handlers

void CStructured_Mesh::OnBnClickedOk()
{
  //CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  //CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //MSHOpen((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  Invalidate(TRUE);

    // TODO: Add your control notification handler code here
    OnOK();
}
