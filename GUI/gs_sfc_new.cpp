// GSSurfacenew.cpp : implementation file
/**************************************************************************
GeoSys GUI - Object: New surface dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_sfc_new.h"
#include "MainFrm.h"
#include "GeoSysDoc.h"
#include "gs_project.h"
// GeoLib
#include "geo_ply.h"
#include "geo_pnt.h"
#include "geo_sfc.h"
//MSHLib
#include "msh_elements_rfi.h"
#include ".\gs_sfc_new.h"
// CGSSurfacenew dialog

IMPLEMENT_DYNAMIC(CGSSurfacenew, CDialog)
CGSSurfacenew::CGSSurfacenew(CWnd* pParent /*=NULL*/)
	: CDialog(CGSSurfacenew::IDD, pParent)
{
m_strNameSurface = _T("");
}

CGSSurfacenew::~CGSSurfacenew()
{
}

void CGSSurfacenew::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PLY_MSH2, m_CB_plys);
    DDX_Control(pDX, IDC_COMBO_MSH_TIN, m_CB_tin_msh);
    DDX_Control(pDX, IDC_LIST_SFC_NEW, m_Listctrlsurfacenew);
    DDX_Control(pDX, IDC_LIST_SURFACE_POLYLINES, m_sfc_plys);
}


BEGIN_MESSAGE_MAP(CGSSurfacenew, CDialog)
    ON_BN_CLICKED(IDC_SURFACE_ADD, OnClickedSurfaceAdd)
    ON_BN_CLICKED(IDC_SURFACE_REMOVE, OnClickedSurfaceRemove)
    ON_BN_CLICKED(IDC_SURFACE_REMOVEALL, OnClickedSurfaceRemoveall)
    ON_BN_CLICKED(IDC_UNSELECT_SFC, OnClickedUnselectSfc)
    ON_BN_CLICKED(IDC_SFC_READ, OnClickedSfcRead)
    ON_BN_CLICKED(IDC_SFC_WRITE, OnClickedSfcWrite)
    ON_BN_CLICKED(IDC_SURFACE_WRITE_TEC, OnClickedSurfaceWriteTec)
    ON_BN_CLICKED(IDC_CREATE_SURFACE, OnClickedCreateSurface)
    ON_BN_CLICKED(IDC_PLY2SFC, OnClickedPly2sfc)
    ON_BN_CLICKED(IDC_CREATE_LAYER_TINs, OnClickedCreateLayerTins)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
01/2006 CC
**************************************************************************/
BOOL CGSSurfacenew::OnInitDialog() 
{
    CDialog::OnInitDialog();
    CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("TYPE");
    CString         strItem3= _T("DATA");
    CString         strItem4= _T("EPS");
    CString         strItem5= _T("MD");
    CString         strItem6= _T("MAT");
    m_Listctrlsurfacenew.GetWindowRect(&rect); 
    m_Listctrlsurfacenew.SetTextBkColor(RGB(153,153,255));
    m_Listctrlsurfacenew.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_Listctrlsurfacenew.EnableTrackingToolTips();
    m_Listctrlsurfacenew.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );
	m_Listctrlsurfacenew.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/6, 0);
	m_Listctrlsurfacenew.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/6, 1);
    m_Listctrlsurfacenew.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/6, 2);
    m_Listctrlsurfacenew.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/6, 3);
    m_Listctrlsurfacenew.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/6, 4);
    m_Listctrlsurfacenew.InsertColumn(5, strItem6, LVCFMT_LEFT,
        rect.Width() * 1/6, 5);
  
    UpdateSurfaceList();

//Polyline list
     CGLPolyline* m_ply = NULL;
     m_CB_plys.ResetContent();
     m_CB_plys.InitStorage(2048,80);
     for(int j=0;j<(int)polyline_vector.size();j++){
     m_ply = polyline_vector[j];
     m_CB_plys.AddString((CString)m_ply->name.data());
    // m_CB_plys.SetCurSel(j);
     }
     m_CB_plys.SetCurSel(0);

//MSH
     CFEMesh* m_msh = NULL;
     m_CB_tin_msh.ResetContent();
     m_CB_tin_msh.InitStorage(10,80);
     for(int i=0;i<(int)fem_msh_vector.size();i++){
     m_msh = fem_msh_vector[i];
     m_CB_tin_msh.AddString((CString)m_msh->pcs_name.data());
     }
     m_CB_tin_msh.SetCurSel(0); 


     return TRUE; // return TRUE unless you set the focus to a control	              // EXCEPTION: OCX Property Pages should return FALSE
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSurfaceAdd()
{
    char sfc_name[80];
    Surface *m_sfc = NULL;
    long size = 0;
    int no_surfaces = (int)surface_vector.size();//CC
    if(no_surfaces==0){
       size = 0;
       }
    else
       { 
     Surface* s_sfc = NULL;
     vector<Surface*>::iterator ps =surface_vector.begin();//CC
     while (ps!=surface_vector.end()){
        s_sfc = *ps;
        ++ps;
        }
     size = s_sfc->id + 1;
        } 
    m_sfc = new Surface();
    sprintf(sfc_name,"%s%ld","SURFACE",size);
    m_sfc->name = sfc_name;
    m_sfc->id = size;
    surface_vector.push_back(m_sfc);
    Surface *m_surface = NULL;
    vector<Surface*>::iterator p = surface_vector.begin();
  
    while(p!=surface_vector.end()) {
				 
				  m_surface = *p;
                  m_surface->highlighted = false;
                  ++p;
                  }
    m_sfc->highlighted = true; // CC
    m_strNameSurface = sfc_name; //CC
    size = (long)surface_vector.size()-1;
    AddSurfacetoList(size);
    m_sfc_plys.ResetContent();// CC
    UpdateData(FALSE);
  //----------------------------------------------------------------------
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    if(gli_points_vector.size()>0)
    theApp->g_graphics_modeless_dlg->m_iDisplaySUF = true;
     else
    theApp->g_graphics_modeless_dlg->m_iDisplaySUF = false;
       
    theApp->g_graphics_modeless_dlg->OnInitDialog();
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSurfaceRemove()
{
  POSITION p = m_Listctrlsurfacenew.GetFirstSelectedItemPosition();
  if(p==NULL) return;
  else{
  long m_iSel =(long) m_Listctrlsurfacenew.GetNextSelectedItem(p);
  //m_ply = polyline_vector[m_iSel];
  GEORemoveSurface(m_iSel);//CC
  UpdateSurfaceList();
  }
  m_sfc_plys.ResetContent();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
01/2006 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSurfaceRemoveall()
{
  long surface_vector_size;
  long i;
  surface_vector_size = (long)surface_vector.size();
  for(i = 0;i<surface_vector_size;0)
  {
	  GEORemoveSurface(i);
	  surface_vector_size = (long)surface_vector.size();
  }
   m_sfc_plys.ResetContent();
  m_Listctrlsurfacenew.DeleteAllItems();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedUnselectSfc()
{
   Surface *gl_sfc = NULL;
   vector<Surface*>::iterator p = surface_vector.begin();
				  while(p!=surface_vector.end())
				  {
				  gl_sfc = *p;
                  gl_sfc->highlighted = false;
                  ++p;
                   }
   CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
   mainframe->OnDrawFromToolbar();
   UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSfcRead()
{
    // TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSfcWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  string path_base = m_gsp->path + m_gsp->base;
  GEOWrite(path_base); 
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedSurfaceWriteTec()
{
  string file_path;
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp)
    file_path = m_gsp->path; //CC
  vector<Surface*>::iterator p = surface_vector.begin();//CC
  Surface *m_sfc =NULL;
  while(p!=surface_vector.end()) { 
    m_sfc = *p;
    m_sfc->WriteTINTecplot(file_path);//CC
    ++p;
  }
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::OnClickedCreateSurface()
{
  //POSITION p = m_Listctrlsurfacenew.GetFirstSelectedItemPosition();
  // if (p == NULL) return;
  // else{
  //int nSel = m_Listctrlsurfacenew.GetNextSelectedItem(p);
  int i;
  CString m_strPLYName;
  CGLPolyline* m_ply = NULL; 
  Surface* m_sfc = NULL;
  int size = (int)surface_vector.size();
 // m_sfc = surface_vector[nSel];//CC
  if (size<1) return;
  m_sfc = GEOGetSFCByName((string)m_strNameSurface);//CC
  for(i=0;i<m_sfc_plys.GetCount();i++){
    m_sfc_plys.GetText(i,m_strPLYName);
    m_ply = GEOGetPLYByName((string)m_strPLYName);//CC
    if(m_ply){
      if(m_sfc){
   
		m_sfc->polyline_of_surface_vector.push_back(m_ply);//CC
        if(m_ply->data_type==1){
          m_sfc->type = 1;
        }
      }
    }
  }
  //----------------------------------------------------------------------
  // 3 Create point vector of surface polygon
  GEOSurfaceTopology();//CC
  m_sfc->PolygonPointVector();//CC
  //----------------------------------------------------------------------
  // 4 Create TIN for surfaces based on layer polylines
  if((m_sfc_plys.GetCount()==2)&&(m_sfc->type==1)){ //OK41
    m_sfc->CreateTIN();
//    m_sfc->WriteTIN((string)m_strFileNamePath);//CC
  }

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->OnDrawFromToolbar();
  //UpdateAllViews(NULL);
//}
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
01/2006 CC
**************************************************************************/
void CGSSurfacenew::OnClickedPly2sfc()
{
    if(m_CB_plys.GetCurSel()== CB_ERR)
    {
    AfxMessageBox("No Surface selected !");
    return;
    }
    CString m_strPLYName;
    m_CB_plys.GetLBText(m_CB_plys.GetCurSel(),m_strPLYName);
    m_sfc_plys.AddString(m_strPLYName);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
11/2005 OK MSH
11/2005 OK Path
02/2006 CC new dialog
**************************************************************************/
void CGSSurfacenew::OnClickedCreateLayerTins()
{
  if(m_CB_tin_msh.GetCurSel()== CB_ERR)
    {
    AfxMessageBox("No mesh was selected !");
    return;
    }
    CString m_strMSHName;
    m_CB_tin_msh.GetLBText(m_CB_tin_msh.GetCurSel(),m_strMSHName);
     //----------------------------------------------------------------------
  Surface * m_sfc = NULL;
  POSITION p = m_Listctrlsurfacenew.GetFirstSelectedItemPosition();
  if(p==NULL) {
  AfxMessageBox("No surface was selected !");
  return;
  }
  else{
  int m_iSel = m_Listctrlsurfacenew.GetNextSelectedItem(p);
  m_sfc = surface_vector[m_iSel];
  } 
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }  
  //---------------------------------------------------------------------
  // Create layer surfaces
if (m_msh->ele_type == 6)//CC02/06
 m_msh->CreateLayerSurfaceTINsfromPris(m_sfc);//CC02/06
else if (m_msh->ele_type ==4)  //CC02/06
  m_msh->CreateSurfaceTINfromTri(m_sfc);//CC02/06
  AfxMessageBox("TINs have been created!");
/*
  m_progress_TINS.SetRange((short)0,(short)no_triangles);
  m_progress_TINS.SetStep(0);
  SetTimer(0,no_triangles,NULL);
  m_progress_TINS.StepIt();
  m_progress_TINS.SetPos(i+1);
  UpdateData(FALSE);
*/
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp){
    GEOWriteSurfaceTINs(m_gsp->path);
    GEOWriteSurfaceTINsTecplot(m_gsp->path);
  }
  // Refesh data
UpdateSurfaceList();//CC
//  OnInitDialog(); //OK
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSSurfacenew::AddSurfacetoList(long size)
{
    char sID[1024];
    char Typevalue[1024];
    char DATAvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    Surface* gl_sfc = NULL;
    gl_sfc = surface_vector[size];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_sfc->name.data());//CC
    sprintf(Typevalue,"%s",gl_sfc->type_name.data());
    sprintf(DATAvalue,"%s",gl_sfc->data_name.data());
    sprintf(EPSvalue,"%lg",gl_sfc->epsilon);
    sprintf(MDvalue,"%lg",gl_sfc->mesh_density);
    sprintf(MATvalue,"%d",gl_sfc->mat_group);
    lvitem.pszText=sID;
    lvitem.iItem=(int)size;//size of list
    lvitem.iSubItem=0;
    m_Listctrlsurfacenew.InsertItem(&lvitem);
    m_Listctrlsurfacenew.SetItemText((int)size,1,Typevalue);
    m_Listctrlsurfacenew.SetItemText((int)size,2,DATAvalue);
    m_Listctrlsurfacenew.SetItemText((int)size,3,EPSvalue);
    m_Listctrlsurfacenew.SetItemText((int)size,4,MDvalue);
    m_Listctrlsurfacenew.SetItemText((int)size,5,MATvalue);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
03/2006 CC 
**************************************************************************/
void CGSSurfacenew::UpdateSurfaceList()
{
    char sID[1024];
    char TYPEvalue[1024];
    char DATAvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    long listip = 0;
    Surface* gl_sfc = NULL;
    m_Listctrlsurfacenew.DeleteAllItems();
	vector<Surface*>::iterator p = surface_vector.begin();
    while(p!=surface_vector.end()) {
    gl_sfc = *p;  
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_sfc->name.data());//CC
    sprintf(TYPEvalue,"%s",gl_sfc->type_name.data());
    sprintf(DATAvalue,"%s",gl_sfc->data_name.data());
    sprintf(EPSvalue,"%lg",gl_sfc->epsilon);
    sprintf(MDvalue,"%lg",gl_sfc->mesh_density);
    sprintf(MATvalue,"%d",gl_sfc->mat_group);
    lvitem.pszText=sID;
    lvitem.iItem=listip;
    lvitem.iSubItem=0;
    m_Listctrlsurfacenew.InsertItem(&lvitem);
    m_Listctrlsurfacenew.SetItemText(listip,1,TYPEvalue);
    m_Listctrlsurfacenew.SetItemText(listip,2,DATAvalue);
    m_Listctrlsurfacenew.SetItemText(listip,3,EPSvalue);
    m_Listctrlsurfacenew.SetItemText(listip,4,MDvalue);
    m_Listctrlsurfacenew.SetItemText(listip,5,MATvalue);
 
    ++p;
    ++listip;
     
    }
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
01/2006 CC Implementation
03/2006 CC
**************************************************************************/
void CGSSurfacenew::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  CDialog::OnActivate(nState, pWndOther, bMinimized);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if (theApp->pPolylinenew->GetSafeHwnd()!=NULL)
   theApp->pPolylinenew->add_polyline = false;
  //----------------------------------------------------------------------
  theApp->ActiveDialogIndex = 3;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
  mainframe->m_bIsPolylineEditorOn = 0;
  mainframe->m_bIsSurfaceEditorOn = 1;
  mainframe->m_bIsVolumeEditorOn = 0;
  //----------------------------------------------------------------------
  UpdateSurfaceList();
  //Polylines
  m_CB_plys.ResetContent();
  CGLPolyline *gs_polyline = NULL;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_CB_plys.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
  //MSH
  CFEMesh* m_msh = NULL;
  m_CB_tin_msh.ResetContent();
  m_CB_tin_msh.InitStorage(10,80);
  for(int i=0;i<(int)fem_msh_vector.size();i++){
  m_msh = fem_msh_vector[i];
  m_CB_tin_msh.AddString((CString)m_msh->pcs_name.data());
  }
  m_CB_tin_msh.SetCurSel(0); 
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
01/2006 CC 
**************************************************************************/
void CGSSurfacenew::OnButtonSurfacePropertiesUpdate() 
{
  UpdateData(TRUE);
  Surface *gl_sfc = NULL;
  POSITION p = m_Listctrlsurfacenew.GetFirstSelectedItemPosition();
  if (p == NULL) return;
  else
  {
  int nSel = m_Listctrlsurfacenew.GetNextSelectedItem(p);
  int listip = 0;
  Surface *m_sfc = NULL;
  CGLPolyline *m_ply = NULL;
  m_sfc = surface_vector[nSel];
  m_sfc_plys.ResetContent();
  vector<CGLPolyline*>::iterator p = m_sfc->polyline_of_surface_vector.begin();
    
  while(p!=m_sfc->polyline_of_surface_vector.end())
   {
    m_ply = *p;
    m_sfc_plys.AddString((CString)m_ply->name.data());

    ++p;
    }
  vector<Surface*>::iterator pl = surface_vector.begin();
  while(pl!=surface_vector.end())
  {
  gl_sfc= *pl;
  string nameID;
  string TYPEvalue;
  string DATAvalue;
  string EPSvalue;
  string MDvalue;
  string MATvalue;

  nameID = (string)m_Listctrlsurfacenew.GetItemText(listip,0);
  TYPEvalue = (string)m_Listctrlsurfacenew.GetItemText(listip,1);
  DATAvalue = (string)m_Listctrlsurfacenew.GetItemText(listip,2);
  EPSvalue = (string)m_Listctrlsurfacenew.GetItemText(listip,3);
  MDvalue = (string)m_Listctrlsurfacenew.GetItemText(listip,4);
  MATvalue = (string)m_Listctrlsurfacenew.GetItemText(listip,5);

  gl_sfc->name = nameID;
  gl_sfc->type_name = TYPEvalue;
  gl_sfc->data_name = DATAvalue;
  gl_sfc->epsilon = strtod(EPSvalue.data(),NULL);
  gl_sfc->mesh_density = strtod(MDvalue.data(), NULL);
//----------------------------------------------------------
  CGLPolyline* gl_ply = NULL;
  vector<CGLPolyline*>::iterator pv = gl_sfc->polyline_of_surface_vector.begin();
      while (pv!=gl_sfc->polyline_of_surface_vector.end())
      {
        gl_ply = *pv;
        gl_ply->mesh_density = gl_sfc->mesh_density ;
        CGLPoint* gl_point = NULL;
        vector<CGLPoint*>::iterator ps = gl_ply->point_vector.begin();
        while (ps!=gl_ply->point_vector.end())
        {
        gl_point = *ps;
        gl_point->mesh_density = gl_ply->mesh_density ;
        ++ps;    
        }
        ++pv;    
      }

//-----------------------------------------------------------
  gl_sfc->mat_group = atoi(MATvalue.data());
  gl_sfc->highlighted = false;
  ++pl;
  ++listip;
  }
  surface_vector[nSel]->highlighted = true;
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
01/2006 CC
**************************************************************************/
void CGSSurfacenew::OnCancel() 
{
  CDialog::OnCancel();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsSurfaceEditorOn = 0;

}