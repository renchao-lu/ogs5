// dlg_shp.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "dlg_shp.h"
#include ".\dlg_shp.h"
#include "MainFrm.h"
// SHPLib
#include "shapefil.h"
#include "gs_project.h"
// CDialogSHP dialog

IMPLEMENT_DYNAMIC(CDialogSHP, CDialog)
CDialogSHP::CDialogSHP(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSHP::IDD, pParent)
    , m_strSHPFile(_T(""))
    , m_strGEOBaseName(_T("LAYER"))
    , m_strGEONumber(_T(""))
{
}

CDialogSHP::~CDialogSHP()
{
}

void CDialogSHP::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_SHP_FILE, m_strSHPFile);
    DDX_Control(pDX, IDC_COMBO_SHP_PNT, m_CB_SHP_PNT);
    DDX_Control(pDX, IDC_COMBO_SHP_PLY, m_CB_SHP_PLY);
    DDX_Text(pDX, IDC_GEO_BASE, m_strGEOBaseName);
    DDX_Control(pDX, IDC_COMBO_GEO_PNT, m_CB_GEO_PNT);
    DDX_Control(pDX, IDC_COMBO_GEO_PLY, m_CB_GEO_PLY);
    DDX_Control(pDX, IDC_COMBO_GEO_SFC, m_CB_GEO_SFC);
    DDX_Control(pDX, IDC_COMBO_GEO_VOL, m_CB_GEO_VOL);
    DDX_Text(pDX, IDC_GEO_NUMBER, m_strGEONumber);
}


BEGIN_MESSAGE_MAP(CDialogSHP, CDialog)
    ON_BN_CLICKED(IDC_FILE_SHP, OnBnClickedFileSHP)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_PNT, OnBnClickedButtonCreatePNT)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_PLY, OnBnClickedButtonCreatePLY)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_SFC, OnBnClickedButtonCreateSFC)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_VOL, OnBnClickedButtonCreateVOL)
    ON_BN_CLICKED(IDC_BUTTON_GEO_WRITE, OnBnClickedButtonGeoWrite)
END_MESSAGE_MAP()


// CDialogSHP message handlers

/**************************************************************************
SHPLibGUI-Method: 
Programing:
11/2005 OK Implementation based on SHPReadFile by CC
**************************************************************************/
BOOL CDialogSHP::OnInitDialog() 
{
  int i;
  CString m_str;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // PNT
  m_CB_GEO_PNT.ResetContent();
  for(i=0;i<(int)gli_points_vector.size();i++){
    m_str = gli_points_vector[i]->name.data();
    m_CB_GEO_PNT.AddString(m_str);
    m_CB_GEO_PNT.SetCurSel(0);
  }
  //----------------------------------------------------------------------
  // PLY
  m_CB_GEO_PLY.ResetContent();
  for(i=0;i<(int)polyline_vector.size();i++){
    m_str = polyline_vector[i]->name.data();
    m_CB_GEO_PLY.AddString(m_str);
    m_CB_GEO_PLY.SetCurSel(0);
  }
  //----------------------------------------------------------------------
  // SFC
  m_CB_GEO_SFC.ResetContent();
  for(i=0;i<(int)surface_vector.size();i++){
    m_str = surface_vector[i]->name.data();
    m_CB_GEO_SFC.AddString(m_str);
    m_CB_GEO_SFC.SetCurSel(0);
  }
  //----------------------------------------------------------------------
  // VOL
  m_CB_GEO_VOL.ResetContent();
  for(i=0;i<(int)volume_vector.size();i++){
    m_str = volume_vector[i]->name.data();
    m_CB_GEO_VOL.AddString(m_str);
    m_CB_GEO_VOL.SetCurSel(0);
  }
  //----------------------------------------------------------------------
  UpdateData(FALSE);
  return TRUE;
}


/**************************************************************************
SHPLibGUI-Method: 
Programing:
11/2005 OK Implementation based on SHPReadFile by CC
**************************************************************************/
void CDialogSHP::OnBnClickedFileSHP()
{
  CFileDialog fileDlg(TRUE,"shp",NULL,OFN_ENABLESIZING,"SHP Files (*.shp)|*.shp||");
  if (fileDlg.DoModal()==IDOK) {
    // Test
    CString SHPBaseType = fileDlg.GetFileName(); // base_type
    if(SHPBaseType.IsEmpty())
      return; 
    // Determine path plus base name
    CString SHPPathBaseType = fileDlg.GetPathName();
    int pos = SHPPathBaseType.ReverseFind('.');
    if(pos<1)
      return; 
    m_strPathBase = SHPPathBaseType.Left(pos);
	//file type cast
	char* shp_name = NULL;
	int datlen = m_strPathBase.GetLength();
    shp_name = (char *) Malloc(datlen);
	strcpy(shp_name,m_strPathBase);
    m_strSHPFile = m_strPathBase + ".shp";
    UpdateData(FALSE);
    ReadInfo();
//	SHPReadFile(shp_name); //CCToDo: string argument
  }
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
10/2005 OK Implementation based on SHPReadFile by CC
**************************************************************************/
void CDialogSHP::ReadInfo()
{
  CString m_strCount;
  int no_points = -1;
  int no_polylines = -1;
  //----------------------------------------------------------------------
  // SHP objects
  SHPHandle hSHP;
  SHPObject *hSHPObject;
  //----------------------------------------------------------------------
  // File handling
  string shp_file_name;
  shp_file_name = m_strSHPFile;
  hSHP = SHPOpen(shp_file_name.c_str(),"rb");
  if(!hSHP)
    return;
  //----------------------------------------------------------------------
  // Read SHP contents
  m_CB_SHP_PNT.ResetContent();
  m_CB_SHP_PLY.ResetContent();
  long nRecord;
  nRecord=(long)hSHP->nRecords;
  for(int i=0;i<=(nRecord-1);i++)
  {   
    hSHPObject=SHPReadObject(hSHP,i);
    switch (hSHPObject->nSHPType)
    {
      case 1: // POINT
        no_points++;
        m_strSHPPoint = "POINT";
        m_strCount.Format("%i",no_points);
        m_strSHPPoint += m_strCount;
        m_CB_SHP_PNT.AddString(m_strSHPPoint);
        m_CB_SHP_PNT.SetCurSel(0);
        break;
      case 3: // POLYLINE
        no_polylines++;
        m_strSHPPolyline = "POLYLINE";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_SHP_PLY.AddString(m_strSHPPolyline);
        m_CB_SHP_PLY.SetCurSel(0);
        break;
      case 5: // POLYLINE
        no_polylines++;
        m_strSHPPolyline = "POLYLINE";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_SHP_PLY.AddString(m_strSHPPolyline);
        m_CB_SHP_PLY.SetCurSel(0);
        break;
      case 11: // PointZ
        no_points++;
        m_strSHPPoint = "POINT";
        m_strCount.Format("%i",no_points);
        m_strSHPPoint += m_strCount;
        m_CB_SHP_PNT.AddString(m_strSHPPoint);
        m_CB_SHP_PNT.SetCurSel(0);
        break;
      case 15: // PolygonZ
        no_polylines++;
        m_strSHPPolyline = "POLYLINE";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_SHP_PLY.AddString(m_strSHPPolyline);
        m_CB_SHP_PLY.SetCurSel(0);
        break;
      default: AfxMessageBox("undeclared shapefile type!",MB_OK,0);
        break;
	 }
   }
  //----------------------------------------------------------------------
  SHPClose(hSHP);
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
10/2005 OK Implementation based on SHPReadFile by CC
**************************************************************************/
void CDialogSHP::OnBnClickedButtonCreatePNT()
{
  UpdateData(TRUE);
  CString m_strCount;
  int no_points = -1;
  CGLPoint* m_pnt = NULL;
  int pnt_number = 0;
  //----------------------------------------------------------------------
  // SHP objects
  SHPHandle hSHP;
  SHPObject *hSHPObject;
  //----------------------------------------------------------------------
  // File handling
  string shp_file_name;
  shp_file_name = m_strSHPFile;
  hSHP = SHPOpen(shp_file_name.c_str(),"rb");
  if(!hSHP)
    return;
  //----------------------------------------------------------------------
  // Read SHP contents
  m_CB_GEO_PNT.ResetContent();
  long nRecord;
  nRecord=(long)hSHP->nRecords;
  for(int i=0;i<=(nRecord-1);i++)
  {   
    hSHPObject=SHPReadObject(hSHP,i);
    switch (hSHPObject->nSHPType)
    {
      case 1: // POINT
        no_points++;
        m_strSHPPoint = m_strGEOBaseName;
        m_strCount.Format("%i",no_points);
        m_strSHPPoint += m_strCount;
        m_CB_GEO_PNT.AddString(m_strSHPPoint);
        m_CB_GEO_PNT.SetCurSel(0);
        m_pnt = new CGLPoint();
        m_pnt->x = *(hSHPObject->padfX);
        m_pnt->y = *(hSHPObject->padfY);
        m_pnt->z = *(hSHPObject->padfZ);
        pnt_number = m_pnt->IsPointExist();
        if(pnt_number<0){
          gli_points_vector.push_back(m_pnt);
        }
        break;
      case 11: // PointZ
        no_points++;
        m_strSHPPoint = m_strGEOBaseName;
        m_strCount.Format("%i",no_points);
        m_strSHPPoint += m_strCount;
        m_CB_GEO_PNT.AddString(m_strSHPPoint);
        m_CB_GEO_PNT.SetCurSel(0);
        m_pnt = new CGLPoint();
        m_pnt->x = *(hSHPObject->padfX);
        m_pnt->y = *(hSHPObject->padfY);
        m_pnt->z = *(hSHPObject->padfZ);
        if(pnt_number<0){
          gli_points_vector.push_back(m_pnt);
        }
        break;
      default: AfxMessageBox("undeclared shapefile type!",MB_OK,0);
        break;
	 }
   }
  //----------------------------------------------------------------------
  SHPClose(hSHP);
  //----------------------------------------------------------------------
  GSPAddMember((string)m_strGSPFileBase + ".gli");
  //----------------------------------------------------------------------
  // Min/Max //OK
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  GEOCalcPointMinMaxCoordinates();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
  {
    theApp->g_graphics_modeless_dlg->m_dXmin = pnt_x_min;
    theApp->g_graphics_modeless_dlg->m_dXmax = pnt_x_max;
    theApp->g_graphics_modeless_dlg->m_dYmin = pnt_y_min;
    theApp->g_graphics_modeless_dlg->m_dYmax = pnt_y_max;
    // Shrink a bit
    theApp->g_graphics_modeless_dlg->m_dXmin -= 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dXmax += 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dYmin -= 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->m_dYmax += 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //----------------------------------------------------------------------
  OnInitDialog();
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
10/2005 OK Implementation based on SHPReadFile by CC
**************************************************************************/
void CDialogSHP::OnBnClickedButtonCreatePLY()
{
  UpdateData(TRUE);
  CString m_strCount;
  int no_polylines = -1;
  int j;
  CGLPolyline* m_ply = NULL;
  CGLPoint m_pnt;
  CGLPoint *m_pnt_exist = NULL;
  CGLPoint *m_pnt_new = NULL;
  //----------------------------------------------------------------------
  // SHP objects
  SHPHandle hSHP;
  SHPObject *hSHPObject;
  //----------------------------------------------------------------------
  // File handling
  string shp_file_name;
  shp_file_name = m_strSHPFile;
  hSHP = SHPOpen(shp_file_name.c_str(),"rb");
  if(!hSHP)
    return;
  //----------------------------------------------------------------------
  // Read SHP contents
  m_CB_GEO_PLY.ResetContent();
  long nRecord;
  nRecord=(long)hSHP->nRecords;
  for(int i=0;i<=(nRecord-1);i++)
  {   
    hSHPObject=SHPReadObject(hSHP,i);
    switch (hSHPObject->nSHPType)
    {
      case 3: // POLYLINE
        no_polylines++;
        m_strSHPPolyline = m_strGEOBaseName + m_strGEONumber + "_";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_GEO_PLY.AddString(m_strSHPPolyline);
        m_CB_GEO_PLY.SetCurSel(0);
        m_ply = GEOGetPLYByName((string)m_strSHPPolyline);
        if(m_ply){
          GEORemovePLY(m_ply);
          //continue;
        }
        m_ply = new CGLPolyline((string)m_strSHPPolyline); 
		for(j=0;j<=(hSHPObject->nVertices-1);j++){
          m_pnt.x = *(hSHPObject->padfX+j);
          m_pnt.y = *(hSHPObject->padfY+j);
		  m_pnt.z = *(hSHPObject->padfZ+j);
          //pnt_number = m_pnt.IsPointExist(); //CC/PCH
          m_pnt_exist = m_pnt.Exist();
          if(m_pnt_exist){
            m_ply->point_vector.push_back(m_pnt_exist);
          }
          else{
            m_pnt_new = new CGLPoint;
            m_pnt_new->x = m_pnt.x;
            m_pnt_new->y = m_pnt.y;
            m_pnt_new->z = m_pnt.z;
            gli_points_vector.push_back(m_pnt_new);
            m_ply->point_vector.push_back(m_pnt_new);
          }
        }
        polyline_vector.push_back(m_ply);
        break;
      case 5: // POLYLINE
        no_polylines++;
        m_strSHPPolyline = m_strGEOBaseName + m_strGEONumber + "_";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_GEO_PLY.AddString(m_strSHPPolyline);
        m_CB_GEO_PLY.SetCurSel(0);
        m_ply = GEOGetPLYByName((string)m_strSHPPolyline);
        if(m_ply){
          GEORemovePLY(m_ply);
          //continue;
        }
        m_ply = new CGLPolyline((string)m_strSHPPolyline);
		for(j=0;j<=(hSHPObject->nVertices-1);j++){
          m_pnt.x = *(hSHPObject->padfX+j);
          m_pnt.y = *(hSHPObject->padfY+j);
		  m_pnt.z = *(hSHPObject->padfZ+j);
          m_pnt_exist = m_pnt.Exist();
          if(m_pnt_exist){
            m_ply->point_vector.push_back(m_pnt_exist);
          }
          else{
            m_pnt_new = new CGLPoint();
            m_pnt_new->x = m_pnt.x;
            m_pnt_new->y = m_pnt.y;
            m_pnt_new->z = m_pnt.z;
            gli_points_vector.push_back(m_pnt_new);
            m_ply->point_vector.push_back(m_pnt_new);
          }
        }
        polyline_vector.push_back(m_ply);
        break;
      case 15: // PolygonZ
        no_polylines++;
        m_strSHPPolyline = m_strGEOBaseName + m_strGEONumber + "_";
        m_strCount.Format("%i",no_polylines);
        m_strSHPPolyline += m_strCount;
        m_CB_GEO_PLY.AddString(m_strSHPPolyline);
        m_CB_GEO_PLY.SetCurSel(0);
        m_ply = GEOGetPLYByName((string)m_strSHPPolyline);
        if(m_ply)
          continue;
        m_ply = new CGLPolyline((string)m_strSHPPolyline);
		for(j=0;j<=(hSHPObject->nVertices-1);j++){
          m_pnt.x = *(hSHPObject->padfX+j);
          m_pnt.y = *(hSHPObject->padfY+j);
		  m_pnt.z = *(hSHPObject->padfZ+j);
          m_pnt_exist = m_pnt.Exist();
          if(m_pnt_exist){
            m_ply->point_vector.push_back(m_pnt_exist);
          }
          else{
            m_pnt_new = new CGLPoint;
            m_pnt_new->x = m_pnt.x;
            m_pnt_new->y = m_pnt.y;
            m_pnt_new->z = m_pnt.z;
            gli_points_vector.push_back(m_pnt_new);
            m_ply->point_vector.push_back(m_pnt_new);
          }
        }
        polyline_vector.push_back(m_ply);
        break;
      default: AfxMessageBox("undeclared shapefile type!",MB_OK,0);
        break;
	 }
   }
  //----------------------------------------------------------------------
  SHPClose(hSHP);
  //----------------------------------------------------------------------
  GSPAddMember((string)m_strGSPFileBase + ".gli");
  //----------------------------------------------------------------------
  // Min/Max //OK
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  GEOCalcPointMinMaxCoordinates();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
  {
    theApp->g_graphics_modeless_dlg->m_dXmin = pnt_x_min;
    theApp->g_graphics_modeless_dlg->m_dXmax = pnt_x_max;
    theApp->g_graphics_modeless_dlg->m_dYmin = pnt_y_min;
    theApp->g_graphics_modeless_dlg->m_dYmax = pnt_y_max;
    // Shrink a bit
    theApp->g_graphics_modeless_dlg->m_dXmin -= 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dXmax += 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dYmin -= 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->m_dYmax += 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //----------------------------------------------------------------------
  OnInitDialog();
  //----------------------------------------------------------------------
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
  {
    if((int)polyline_vector.size()>0){
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
      theApp->g_graphics_modeless_dlg->m_iDisplayPLY = 1;
      theApp->g_graphics_modeless_dlg->UpdateData(FALSE);
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->OnDrawFromToolbar();
    }
  }
// m_ply->CheckDoublePoints, in PLY dialog (OnInitDialog)
// m_ply->CalcMinPointDistance, in PLY dialog (OnInitDialog)
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void CDialogSHP::OnBnClickedButtonCreateSFC()
{
  CGLPolyline* m_ply = NULL;
  Surface* m_sfc = NULL;
  for(int i=0;i<(int)polyline_vector.size();i++){
    m_ply = polyline_vector[i];
    m_sfc = GEOGetSFCByName(m_ply->name);
    if(m_sfc){
      //continue;
      GEORemoveSFC(m_sfc);
    }
    m_sfc = new Surface();
    m_sfc->name = m_ply->name;
    m_sfc->polyline_of_surface_vector.push_back(m_ply);
    surface_vector.push_back(m_sfc);
    GEOSurfaceTopology(); //OKSB
    m_sfc->PolygonPointVector();
  }
  OnInitDialog();
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    if((int)surface_vector.size()>0){
      theApp->g_graphics_modeless_dlg->m_iDisplaySUF = 1;
      theApp->g_graphics_modeless_dlg->UpdateData(FALSE);
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->OnDrawFromToolbar();
    }
  }
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void CDialogSHP::OnBnClickedButtonCreateVOL()
{
  Surface* m_sfc = NULL;
  CGLVolume* m_vol = NULL;
  UpdateData(TRUE);
  for(int i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    m_vol = GEOGetVOL(m_sfc->name);
    if(m_vol){
      GEORemoveVOL(m_vol);
      //continue;
    }
    m_vol = new CGLVolume();
    m_vol->name = m_sfc->name;
    m_vol->surface_vector.push_back(m_sfc);
    m_vol->layer = strtol(m_strGEONumber,NULL,0);
    volume_vector.push_back(m_vol);
  }
  OnInitDialog();
}

/**************************************************************************
SHPLibGUI-Method: 
Programing:
11/2005 OK Implementation
**************************************************************************/
void CDialogSHP::OnBnClickedButtonGeoWrite()
{
  GEOWrite((string)m_strGSPFilePathBase);
} 

