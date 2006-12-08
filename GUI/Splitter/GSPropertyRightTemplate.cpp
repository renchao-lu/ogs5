// GSPropertyRightTemplate.cpp : implementation file
//

/*Template Instructions TK: 11.2006

How to add a new dialog to the right property sheets

MS.NET:
- Copy Template Resource in the Resource View
- Rename IDD
- Rename Caption

Windows Explorer:
- Copy GSPropertyRightTemplate.cpp + GSPropertyRightTemplate.h of and into the Splitter folder and rename them like GSPropertyRightPCS.cpp.
MS.NET:
- Add them also now to the solution inside the folder "Splitter".

CPP changes:
- Change Header: // GSPropertyRightTemplate.cpp : implementation file
- #include "GSPropertyRightTemplate.h" needs the new link
- Search CGSPropertyRightTemplate and Replace with new class name e.g. CGSPropertyRightPCS

H Changes (Header File):
- Search CGSPropertyRightTemplate and Replace with new class name e.g. CGSPropertyRightPCS
- Exchange enum IDD: IDD_CONTROL_RIGHT_TEMPLATE, e.g IDD_CONTROL_RIGHT_PCS

GSPropertyRight.h
Copy and change: #include "GSPropertyRightTemplate.h"
Position the Tab: copy and change CGSPropertyRightTemplate m_Page10;	

GSPropertyRight.cpp
Create and Add Page by copy and change of the line: m_PropSheet.AddPage(&m_Page10); 


*/ 


#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "GSPropertyRightTemplate.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_elements_rfi.h"
#include "msh_quality.h"
#include "dtmesh.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"


// CGSPropertyRightTemplate dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightTemplate, CViewPropertyPage)
CGSPropertyRightTemplate::CGSPropertyRightTemplate()
	: CViewPropertyPage(CGSPropertyRightTemplate::IDD)
{
}

CGSPropertyRightTemplate::~CGSPropertyRightTemplate()
{
}

void CGSPropertyRightTemplate::DoDataExchange(CDataExchange* pDX)
{
	CViewPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGSPropertyRightTemplate, CViewPropertyPage)
END_MESSAGE_MAP()


// CGSPropertyRightTemplate message handlers

