// gs_sfc.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_sfc.h"
#include "MainFrm.h"
#include "GeoSysDoc.h"

// GeoLib
#include "geo_ply.h"
#include "geo_pnt.h"
#include "geo_sfc.h"

#include "msh_elements_rfi.h"
#include "gs_project.h"
#include ".\gs_sfc.h"
Surface *m_surface = NULL;
// CGSSurface dialog

IMPLEMENT_DYNAMIC(CGSSurface, CDialog)
CGSSurface::CGSSurface(CWnd* pParent /*=NULL*/)
	: CDialog(CGSSurface::IDD, pParent)
	, m_dBufferZone(0)
{
 
}

CGSSurface::~CGSSurface()
{
}

void CGSSurface::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SURFACES, m_LBSurfaces);
    DDX_Control(pDX, IDC_LIST_POLYLINES2, m_LBPolylines);
    DDX_Control(pDX, IDC_LIST_SURFACE_POLYLINES, m_LBSFCPolylines);
    DDX_Text(pDX, IDC_EDIT_SURFACE_NAME, m_strNameSurface);
    DDX_Text(pDX, IDC_EDIT_SURFACE_7, m_dBufferZone);
    DDX_Control(pDX, IDC_LIST_SURFACES2, m_LBSurfaces2);
    DDX_Control(pDX, IDC_LIST_SURFACES_MAP, m_LBSurfaces2Map);
    DDX_Text(pDX, IDC_EDIT_SURFACE_MAP_FILE, m_strMapFile);
    DDX_Control(pDX, IDC_PROGRESS_TIN, m_progress_TIN);
    DDX_Control(pDX, IDC_PROGRESS_TINS, m_progress_TINS);
    DDX_Control(pDX, IDC_LIST_SFC, m_LC_SFC);
    DDX_Control(pDX, IDC_COMBO_SFC_MSH_LAY, m_CB_MSH);
}

BEGIN_MESSAGE_MAP(CGSSurface, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_SURFACE_ADD, OnBnSurfaceAdd)
    ON_BN_CLICKED(IDC_BUTTON_PLY2SFC, OnBnPolyline2Surface)
    ON_LBN_SELCHANGE(IDC_LIST_SURFACES, OnLbnSelchangeListSurfaces)
	ON_LBN_SELCHANGE(IDC_LIST_SURFACES, OnLbnSelOnlyThePickedSurfaces)	// Added by PCH
    ON_LBN_SELCHANGE(IDC_LIST_POLYLINES2, OnLbnSelchangeListPolylines)
    ON_BN_CLICKED(IDC_BUTTON_SURFACE_REMOVE, OnBnSurfaceRemove)
	ON_BN_CLICKED(IDC_BUTTON_SURFACE_UPDATE, OnBnClickedButtonSurfaceUpdate)
	ON_BN_CLICKED(IDC_BUTTON_SURFACE_REMOVEALL, OnBnClickedButtonSurfaceRemoveall)
    ON_BN_CLICKED(IDC_BUTTON_SURFACE_CREATE_TIN, OnBnClickedButtonSurfaceCreateTIN)
    ON_BN_CLICKED(IDC_BUTTON_SFC2MAP, OnBnClickedButtonSurfaceMap)
    ON_LBN_SELCHANGE(IDC_LIST_SURFACES2, OnLbnSelchangeListSurfaces2)
    ON_BN_CLICKED(IDC_BUTTON_FILE, OnBnClickedButtonFile)
    ON_BN_CLICKED(IDC_BUTTON_EXECUTE, OnBnClickedButtonExecute)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_SURFACE_WRITE_TEC, OnBnClickedButtonSurfaceWriteTEC)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_LAYER_TINs, OnBnClickedButtonCreateLayerTINs)
    ON_BN_CLICKED(IDC_OK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_CANCEL, OnBnClickedCancel)
    ON_EN_CHANGE(IDC_EDIT_SURFACE_NAME, OnEnChangeEditSurfaceName)
	ON_LBN_SELCHANGE(IDC_LIST_SURFACE_POLYLINES, OnLbnSelchangeListSurfacePolylines)
    ON_BN_CLICKED(IDC_UNSELECT, OnBnClickedUnselect) //CC
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
04/2005 PCH Modified to automatically select the picked polylines
04/2005 CC comment the OnUpdatePolylineListPicked() agree with PCH
10/2005 OK LC
11/2005 OK MSH
**************************************************************************/
BOOL CGSSurface::OnInitDialog()
{
  CGLPolyline *gs_polyline = NULL;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // Polylines
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
	// PCH added this to display the picked polylines automatically.
  // CC 04/2005 agree with PCH
	//OnUpdatePolylineListPicked();
  //----------------------------------------------------------------------
  // Surfaces
  Surface *gs_surface = NULL;
  const char *char_string;
  int i = 0;
  m_LBSurfaces.ResetContent();
  m_LBSurfaces2.ResetContent();
  vector<Surface*>::iterator ps = surface_vector.begin();
  while(ps!=surface_vector.end()) {
    gs_surface = *ps;
    char_string = gs_surface->name.data();
    m_LBSurfaces.AddString((CString)char_string);
    m_LBSurfaces2.AddString((CString)char_string);
    ++i;
    ++ps;
  }
  //----------------------------------------------------------------------
  // MSH
  //----------------------------------------------------------------------
  m_CB_MSH.ResetContent();
  CFEMesh* m_msh = NULL;
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_strMSHName = m_msh->pcs_name.c_str();
    m_CB_MSH.AddString(m_strMSHName);
    m_CB_MSH.SetCurSel(i);
  }
  //-----------------------------------------------------------------------
  // Progress bar
  CWnd* pWnd = GetDlgItem(IDC_PROGRESS_TIN);
  CRect rect;
  pWnd->GetWindowRect(&rect);
  ScreenToClient(&rect);
  //----------------------------------------------------------------------
  // TABLE
  CRect m_rect_table;
  CString m_strItem;
  // Table configuration
  m_LC_SFC.GetWindowRect(&m_rect_table);
  m_LC_SFC.SetTextBkColor(RGB(153,153,255));
  m_LC_SFC.SetTextColor(RGB(0,0,255));
   // pause the mouse with highlight or you can select it
   // so this will be the hot item.
  m_LC_SFC.EnableTrackingToolTips();
  m_LC_SFC.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES);
  LV_ITEM lvitem;
  lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
  // Insert head line
  m_LC_SFC.DeleteColumn(0);
  m_LC_SFC.InsertColumn(0,"SFC",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC_SFC.DeleteColumn(1);
  m_LC_SFC.InsertColumn(1,"SFC Type",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC_SFC.DeleteColumn(2);
  m_LC_SFC.InsertColumn(2,"DATA",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC_SFC.DeleteColumn(3);
  m_LC_SFC.InsertColumn(3,"MAT",LVCFMT_LEFT,rect.Width()*1/5,0);
  // Insert data
  UpdateList();
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
12/2003 CC
10/2004 OK ReDone
06/2005 OK g_graphics_modeless_dlg
07/2005 CC highlighting
07/2005 CC add surface
11/2005 OK sfc_ID_max
**************************************************************************/
void CGSSurface::OnBnSurfaceAdd()
{
  char sfc_name[80];
  Surface *m_sfc = NULL;
  long size = 0;
  int no_surfaces = (int)surface_vector.size();//CC
  // CC
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
  size = sfc_ID_max; //OK
  sprintf(sfc_name,"%s%ld","SURFACE",size);
  m_sfc->name = sfc_name;
  m_sfc->id = size;
  surface_vector.push_back(m_sfc);
  //----------------------------------------------//CC
  Surface *m_surface = NULL;
  vector<Surface*>::iterator p = surface_vector.begin();
  while(p!=surface_vector.end()) {
    m_surface = *p;
    m_surface->highlighted = false;
   ++p;}
  m_sfc->highlighted = true; // CC
  //-------------------------------------------------
  m_strNameSurface = sfc_name; //CC
  m_LBSFCPolylines.ResetContent();// CC
  m_LBSurfaces.AddString((CString)sfc_name);
  //m_LBSurfaces.SetCurSel(size);
  m_LBSurfaces.SetCurSel(m_LBSurfaces.GetCount()-1); //OK
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    if(gli_points_vector.size()>0){
      theApp->g_graphics_modeless_dlg->m_iDisplaySUF = true;
    }
    else{
      theApp->g_graphics_modeless_dlg->m_iDisplaySUF = false;
    }
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
07/2005 CC Modification
10/05 CC test Geolib dll
**************************************************************************/
void CGSSurface::OnBnSurfaceRemove()
{
  // Get selected surface from ListBox
  int nSel = m_LBSurfaces.GetCurSel();
  if(nSel== LB_ERR)
  return; //CC

  m_LBSurfaces.GetText(nSel,m_strNameSurface);
  // Remove this surface
  Surface *gs_surface = NULL;

  GEORemoveSurface((long)nSel);//CC 08/05
 // gs_surface->Remove((string)m_strNameSurface);
  m_strNameSurface.Empty();
  // Refresh surface listbox
  const char *char_string;
  m_LBSurfaces.ResetContent();
  vector<Surface*>::iterator ps = surface_vector.begin();
  while(ps!=surface_vector.end()) {
    gs_surface = *ps;
    char_string = gs_surface->name.data();
    m_LBSurfaces.AddString((CString)char_string);
    ++ps;
  }
  m_LBSFCPolylines.ResetContent();//CC
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();//CC
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: OnLbnSelchangeListSurfaces()
Task: 
Programing:
11/2003 OK Implementation
last modification: 12/2003 CC
01/2005 CC Modification progress bar refresh and inactivate the CreateTINS bar
07/2005 CC Modification highlight
**************************************************************************/
void CGSSurface::OnLbnSelchangeListSurfaces()
{
  // Get selected surface from ListBox
  int nSel = m_LBSurfaces.GetCurSel();
  m_LBSurfaces.GetText(nSel,m_strNameSurface);
  // Display surface polyline list
  Surface *gs_surface = NULL;
  Surface *m_surface = NULL;
  CGLPolyline *gs_polyline = NULL;
  m_LBSFCPolylines.ResetContent();
  gs_surface = GEOGetSFCByName(string(m_strNameSurface));//CC

  gs_surface->highlighted = true;
  int sel = 0;
  vector<Surface*>::iterator ps = surface_vector.begin();
  while(ps!=surface_vector.end()) {
				 
				  
				  m_surface = *ps;
                  if (sel == nSel)
                       m_surface->highlighted = true;
                  else
                  m_surface->highlighted = false;
				  ++ps;
                  ++sel;
                  }
  //change selection
 /* if (gs_surface->selected == TRUE)
  gs_surface->selected = FALSE;
  else
  gs_surface->selected = TRUE;*/ // CC

  //list<CGLPolyline*>::const_iterator p = gs_surface->polyline_of_surface_list.begin();//CC
vector<CGLPolyline*>::iterator p = gs_surface->polyline_of_surface_vector.begin();
  while(p!=gs_surface->polyline_of_surface_vector.end()) {
    gs_polyline = *p;
    if(gs_polyline) {
//      char_string = gs_polyline->name;
      m_LBSFCPolylines.AddString((CString)gs_polyline->name.c_str());
    }
    else {
      AfxMessageBox("Error in polylines !");
    }
    ++p;
  }
  
  long gs_surface_TIN_Triangles_size;

  GetDlgItem(IDC_BUTTON_SURFACE_CREATE_TIN)->EnableWindow(TRUE); 
  GetDlgItem(IDC_BUTTON_CREATE_LAYER_TINs)->EnableWindow(TRUE);//OK41
  if(gs_surface->TIN) {
    gs_surface_TIN_Triangles_size = (long)gs_surface->TIN->Triangles.size();
    if(gs_surface_TIN_Triangles_size>0) {
      GetDlgItem(IDC_BUTTON_SURFACE_CREATE_TIN)->EnableWindow(FALSE);
    }
  }
  if(gs_surface->createtins)
 GetDlgItem(IDC_BUTTON_CREATE_LAYER_TINs)->EnableWindow(FALSE);

  m_progress_TINS.SetPos(0);
  m_progress_TIN.SetPos(0);
  // Refresh view of data elements
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  // Refresh view of data elements
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: OnLbnSelOnlyThePickedSurfaces()
Task: This will highlist the surfaces picked in CListBox of Surface 
Programing:
04/2005 PCH Implementation
**************************************************************************/
void CGSSurface::OnLbnSelOnlyThePickedSurfaces()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Let's delect them first before we highlight the picked surfaces

	// Let's highlist the surfaces picked.
	for(int i=0; i< theApp->hitsSurfaceTotal; ++i)
	{
		m_LBSurfaces.SetSel(theApp->surfacePickedTotal[i],TRUE);
	}
	
	UpdateData(FALSE);
}

///////////////////////////////////////////////////////////////////////////
// Surface properties
///////////////////////////////////////////////////////////////////////////

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
10/2004 OK ReDone
**************************************************************************/
void CGSSurface::OnBnPolyline2Surface()
{
  // Get selected surface
  if(m_strNameSurface.IsEmpty()) {
    AfxMessageBox("No Surface selected !");
    return;
  }
/* OK
  Surface *gs_surface = NULL;
  CGLPolyline *gs_polyline = NULL;
  gs_surface = gs_surface->Get(string(m_strNameSurface));
  // Add selected polyine to selected surface
  gs_polyline = gs_polyline->GEOGetPolyline((string)m_strPolylineSelected);
  gs_surface->addPolyline(gs_polyline);    				   
  // Display surface polyline list
  m_LBSFCPolylines.ResetContent();
  list<CGLPolyline*>::const_iterator p = gs_surface->polyline_of_surface_list.begin();
  while(p!=gs_surface->polyline_of_surface_list.end()) {
    gs_polyline = *p;
    m_LBSFCPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
*/
  // Display surface polyline list
  m_LBSFCPolylines.AddString(m_strPolylineSelected);
}


/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSurface::OnLbnSelchangeListPolylines()
{
  // Get selected polyline
  int nSel = m_LBPolylines.GetCurSel();
  m_LBPolylines.GetText(nSel,m_strPolylineSelected);
}




/**************************************************************************
GeoSys-GUI-Method: OnBnClickedButtonSurfaceUpdate()
Task: update surface type
Programing:
12/2003 CC Implementation
08/2005 CC check the name of surface should not repeat
11/2005 OK nSel test
**************************************************************************/
void CGSSurface::OnBnClickedButtonSurfaceUpdate()
{
  UpdateData(TRUE);
  long nSel = m_LBSurfaces.GetCurSel();
  if(nSel<0){
    AfxMessageBox("Error in SFC selection");
    return;
  }
  Surface *gl_surface = NULL;
  gl_surface = surface_vector[nSel];//CC
  if(gl_surface) {
//--------------------------CC name checking
  Surface* m_sfc = NULL;
  vector<Surface*>::iterator p1 = surface_vector.begin();
  int i = 0;
  while(p1!=surface_vector.end()){
    m_sfc = *p1;
    if (m_sfc->name.compare(m_strNameSurface)==0 && nSel != i )
    {
      AfxMessageBox(" Please choose a different name: the surface name already exists");
      return;
    }
    ++p1;
    ++i;}
//--------------------------
    gl_surface->name = (string)m_strNameSurface;
  }
  // Refresh view of data elements
  UpdateData(FALSE);
  // Fill surface list
  m_LBSurfaces.ResetContent();
  vector<Surface*>::iterator p = surface_vector.begin();//CC
  while(p!=surface_vector.end()) {
    m_surface = *p;
    m_LBSurfaces.AddString(m_surface->name.data());
    ++p;
  }
}

/**************************************************************************
GeoSys-GUI-Method: OnBnClickedButtonSurfaceRemoveall()
Task: remove all surfaces from the surface list
Programing:
12/2003 CC Implementation
07/2005 CC Modification
**************************************************************************/
void CGSSurface::OnBnClickedButtonSurfaceRemoveall()
{
  GEORemoveAllSurfaces();
     
  m_LBSurfaces.ResetContent();
  m_strNameSurface = "";
  m_LBSFCPolylines.ResetContent();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();//CC
  UpdateData(FALSE);
	
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
03/2004 OK 
08/2005 CC
**************************************************************************/
void CGSSurface::OnBnClickedButtonSurfaceCreateTIN()
{
  if(ElementListLength==0){
    AfxMessageBox("No MSH data");
    return;
  } 
  m_progress_TIN.SetRange((short)0,(short)ElementListLength);
  m_progress_TIN.SetStep(0);
  SetTimer(0,ElementListLength,NULL);
  int iSel = 0;
  long i;

  vector<Surface*>::iterator ps = surface_vector.begin();//CC
  Surface *m_surface =NULL;
  while(ps!=surface_vector.end()) { 
    m_surface = *ps;
    if(m_surface->highlighted) {
      if(m_surface->TIN==NULL) {
        //m_surface->CreateTIN();
        m_surface->TIN = new CTIN();
        m_surface->TIN->name = m_surface->name;
        for(i=0;i<ElementListLength;i++) {
          m_progress_TIN.StepIt();
          m_progress_TIN.SetPos(i+1);
          UpdateData(FALSE);
          CreateTINfromMSH(m_surface,i);//CC
        }
        if(m_surface->TIN->Triangles.size()==0) {
          delete m_surface->TIN;
        }
        m_surface->WriteTIN((string)m_strFileNamePath);//CC
        m_surface->WriteTINTecplot((string)m_strFileNamePath);//CC
      }
      else
        AfxMessageBox("TIN already exists");
      iSel++;
    }
    ++ps;
  }

//RN reset progress bar

  //GEOWriteSurfaceTINs((string)m_strFileNameBase);  
}


//=========================================================================
// Mapping 

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSurface::OnBnClickedButtonSurfaceMap()
{
  if(m_strNameSurface.IsEmpty()) {
    AfxMessageBox("No Surface selected !");
    return;
  }
  m_LBSurfaces2Map.AddString(m_strNameSurface);
}

void CGSSurface::OnLbnSelchangeListSurfaces2()
{
  m_LBSurfaces2.GetText(m_LBSurfaces2.GetCurSel(),m_strNameSurface);
}

void CGSSurface::OnBnClickedButtonFile()
{
  // Get file name
  CFileDialog dlg(TRUE, "dat", NULL, OFN_ENABLESIZING ,
    " ArcView Files (*.asc)|*.asc| Grid Files (*.dat)|*.dat|  All Files (*.*)|*.*||", this );
  dlg.DoModal();
  m_strMapFile = dlg.GetPathName();
  UpdateData(FALSE);
}

void CGSSurface::OnBnClickedButtonExecute()
{
  int i;
  CString m_strThisNameSurface;
  int no_surfaces = m_LBSurfaces2Map.GetCount();
  for(i=0;i<no_surfaces;i++){
    m_LBSurfaces2Map.GetText(i,m_strThisNameSurface);
    m_surface = GEOGetSFCByName((string)m_strThisNameSurface);//CC
    if (m_surface) {
      MapTIN(m_surface,(string)m_strMapFile);//CC
      m_surface->WriteTIN((string)m_strFileNamePath);//CC
      m_surface->WriteTINTecplot((string)m_strFileNamePath);//CC
    }
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
03/2004 CC/OK Implementation
10/2004 OK type 22 surfaces
01/2005 OK completely new: type 22 surfaces became TINs
**************************************************************************/
void CGSSurface::OnBnClickedButtonCreate()
{
  if(m_LBSFCPolylines.GetCount()<1)
    return;
  int i;
  CString m_strPLYName;
  CGLPolyline* m_ply = NULL;
  Surface* m_sfc = NULL;
//OK41
  m_sfc = GEOGetSFCByName((string)m_strNameSurface);//CC
  if(!m_sfc){
    AfxMessageBox("SFC creation failed");
    return;
  }
  //----------------------------------------------------------------------
  // 2 Create surface polygons and check polyline type
  for(i=0;i<m_LBSFCPolylines.GetCount();i++){
    m_LBSFCPolylines.GetText(i,m_strPLYName);
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
  GEOUnselectSFC();
  m_sfc->highlighted = true;
  //----------------------------------------------------------------------
  // 3 Create point vector of surface polygon
  GEOSurfaceTopology();//CC
  m_sfc->PolygonPointVector();//CC
  //----------------------------------------------------------------------
  // 4 Create TIN for surfaces based on layer polylines
  if((m_LBSFCPolylines.GetCount()==2)&&(m_sfc->type==1)){ //OK41
    m_sfc->CreateTIN();
    m_sfc->WriteTIN((string)m_strFileNamePath);//CC
  }
//OK41
//TK
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->OnDrawFromToolbar();
  //UpdateAllViews(NULL);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGSSurface::OnBnClickedButtonSurfaceWriteTEC()
{
  vector<Surface*>::iterator ps = surface_vector.begin();//CC
  Surface *m_sfc =NULL;
  while(ps!=surface_vector.end()) { 
    m_sfc = *ps;
    m_sfc->WriteTINTecplot((string)m_strFileNamePath);//CC
    ++ps;
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
01/2005 CC Modification progress bar
11/2005 OK MSH
11/2005 OK Path
**************************************************************************/
void CGSSurface::OnBnClickedButtonCreateLayerTINs()
{
  //----------------------------------------------------------------------
  Surface *m_sfc = NULL;
  m_sfc = GEOGetSFCByName((string)m_strNameSurface);//CC
  if(!m_sfc){
    AfxMessageBox("no SFC data");
    return;
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
  m_msh->CreateLayerSurfaceTINsfromPris(m_sfc);
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
  OnInitDialog(); //OK
}

void CGSSurface::OnBnClickedOk()
{
    // Save the change in .gli file
    // To do this, we need guy after guy until we reach pDoc (CGeoSysDoc.cpp)
    //CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    // Now we got the address.
    CGeoSysDoc* pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  
    // Just call OnFileSave that TK wrote. 
    pDoc->OnFileSave();
}

void CGSSurface::OnBnClickedCancel()
{
    OnCancel(); 
}

void CGSSurface::OnEnChangeEditSurfaceName()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

/**************************************************************************
GeoLibGUI-Method: OnUpdatePolylineListPicked()
Task: This updates the polylines in the CListBox
Programing:
04/2005 PCH Implementation
**************************************************************************/
void CGSSurface::OnUpdatePolylineListPicked()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  // Make CListBox empty before we proceed our picking
  CGLPolyline* thisPolyline = NULL;
  m_LBPolylines.ResetContent();
  for(int i=0; i< theApp->hitsPolylineTotal; ++i)
  {
		 thisPolyline = GEOGetPLYById(theApp->polylinePickedTotal[i]);//CC
	int idx = m_LBPolylines.AddString((CString)thisPolyline->name.c_str());
	m_LBPolylines.SetItemData(idx, i+1);
  }
  // Let's just do the other CListBox, too. 
  // It seems that we don't need two CListBox. It is redundant.
  OnLbnSelchangeListSurfacePolylines();
}

/**************************************************************************
GeoLibGUI-Method: OnLbnSelchangeListSurfacePolylines()
Task: Another duplicate function of OnUpdatePolylineListPicked(). 
	We only need one. 
Programing:
04/2005 PCH Implementation
07/2005 CC misunderstand? deleting polylines from surface??
**************************************************************************/
void CGSSurface::OnLbnSelchangeListSurfacePolylines()
{
	/*CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Make CListBox empty before we proceed our picking
	m_LBSFCPolylines.ResetContent();
	
	for(int i=0; i< theApp->hitsBoundaryTotal; ++i)
	{
		CGLPolyline* thisPolyline = thisPolyline->GetPolyline(theApp->boundaryPickedTotal[i]);
		int idx = m_LBSFCPolylines.AddString((CString)thisPolyline->name.c_str());
		m_LBSFCPolylines.SetItemData(idx, i+1);
	}*/ 
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGSSurface::OnBnClickedUnselect()
{
   Surface *gl_surface = NULL;
   //int nSel = m_ListPoint.GetCurSel();
   m_LBSurfaces.SetCurSel(-1);
   vector<Surface*>::iterator p =surface_vector.begin();
				  while(p!=surface_vector.end())
				  {
				  gl_surface = *p;
                  gl_surface->highlighted = false;
                 ++p;
     }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGSSurface::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  //----------------------------------------------------------------------
  CDialog::OnActivate(nState, pWndOther, bMinimized);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if (theApp->pPolyline->GetSafeHwnd()!=NULL)
   theApp->pPolyline->add_polyline = false;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
  mainframe->m_bIsPolylineEditorOn = 0;
  mainframe->m_bIsSurfaceEditorOn = 1;
  mainframe->m_bIsVolumeEditorOn = 0;
  //----------------------------------------------------------------------
  // Polylines
  m_LBPolylines.ResetContent();
  CGLPolyline *gs_polyline = NULL;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGSSurface::OnOK() 
{   
  CDialog::OnOK();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsSurfaceEditorOn = 0;
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGSSurface::OnCancel() 
{   
  CDialog::OnCancel();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsSurfaceEditorOn = 0;
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGSSurface::UpdateList()
{
  int listip = 0;
  string sfc_name;
  CGLPolyline* m_ply = NULL;
  vector<CGLPolyline*>::const_iterator p_ply; 
  Surface* m_sfc = NULL;
  vector<Surface*>::const_iterator p_sfc; 
  p_sfc = surface_vector.begin();
  m_LC_SFC.DeleteAllItems();
  while(p_sfc!= surface_vector.end()){
    m_sfc = *p_sfc;
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_sfc->name.data();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    //....................................................................
    m_LC_SFC.InsertItem(&lvitem);
    //....................................................................
    switch(m_sfc->type){
      case 0:
        m_LC_SFC.SetItemText(listip,1,"POLYLINES");
        break;
      case 1:
        m_LC_SFC.SetItemText(listip,1,"TIN");
        break;
      case 2:
        m_LC_SFC.SetItemText(listip,1,"ARC");
        break;
    }
    //....................................................................
    if(m_sfc->polyline_of_surface_vector.size()>0){
      p_ply = m_sfc->polyline_of_surface_vector.begin();
      m_ply = *p_ply;
    }
    else
      m_ply = NULL;
    if(m_ply){
      switch(m_sfc->type)
      {
        case 0: // PLY-TOP
          if(m_sfc->polyline_of_surface_vector.size()==1){
            m_LC_SFC.SetItemText(listip,2,m_ply->name.data());
          }
          else{
            m_LC_SFC.SetItemText(listip,2,"LIST");
          }
          break;
        case 1: // PLY-GEO
          m_LC_SFC.SetItemText(listip,2,m_ply->name.data());
          break;
      }
    }
    else
      m_LC_SFC.SetItemText(listip,2,"ERROR");
    //....................................................................
    m_LC_SFC.SetItemText(listip,3,m_sfc->mat_group_name.data());
    //....................................................................
    listip++;
	++p_sfc;
  }
}
