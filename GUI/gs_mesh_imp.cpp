// gs_mesh_imp.cpp : implementation file
//
#include "stdafx.h"
// C++
#include <string>
using namespace std;
//
#include "GeoSys.h"
#include "gs_mesh_imp.h"
#include "msh_quality.h"
#include "GeoSysDOC.h"

// Cgs_mesh_imp dialog

IMPLEMENT_DYNAMIC(Cgs_mesh_imp, CDialog)
Cgs_mesh_imp::Cgs_mesh_imp(CWnd* pParent /*=NULL*/)
	: CDialog(Cgs_mesh_imp::IDD, pParent)
{
	x=0.000;
	y=0.000;
	z=0.000;
	nnr=0;
}

Cgs_mesh_imp::~Cgs_mesh_imp()
{
}

void Cgs_mesh_imp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_MPA_X, x);
	DDX_Text(pDX, IDC_EDIT_MPA_Y, y);
	DDX_Text(pDX, IDC_EDIT_MPA_Z, z);
	DDX_Text(pDX, IDC_EDIT_REMOVE_NR, nnr);
}


BEGIN_MESSAGE_MAP(Cgs_mesh_imp, CDialog)
	ON_BN_CLICKED(IDC_REMESH_BWA, OnManualRemeshBWA)
	ON_BN_CLICKED(IDC_REMESH_REMOVE_POINT, OnManualRemovePoint)

END_MESSAGE_MAP()


// Cgs_mesh_imp message handlers

void Cgs_mesh_imp::OnManualRemeshBWA()
{
  UpdateData(TRUE);
  Get_Rockflow_RFI();
  CalculateBasicTriangleData();
  SearchElementsWithPointInCircumsphere(x, y, z);
  string projectname = GetProjectFileName();
  ExecuteBowyerWatsonAlgorithm(x, y, z,projectname);
}

void Cgs_mesh_imp::OnManualRemovePoint()
{
  UpdateData(TRUE);
  Get_Rockflow_RFI();
  CalculateBasicTriangleData();
  string projectname = GetProjectFileName();
  RemoveNode(nnr,projectname);
}
