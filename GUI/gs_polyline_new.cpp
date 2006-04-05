// GSPolylinenew.cpp : implementation file
//
/**************************************************************************
GeoSys GUI - Object: New polyline dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
#include "stdafx.h"
#include "geosys.h"
#include "stdio.h"
#include "mathlib.h"
#include "MainFrm.h" //CC
//PCH added this
#include "GeoSysDoc.h"
#include "COGLPickingView.h"
// Commmon
#include "makros.h"
#include "gs_project.h"
// GeoLib
#include "geo_ply.h"
#include "geo_lib.h" //OK41

//MSHLib
#include"msh_lib.h"
#include "msh_mesh.h"
// GUI
#include "gs_graphics.h"
#include "gs_polyline_new.h"
#include ".\gs_polyline_new.h"

char c_string_plys[MAX_ZEILE];

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CGSPolylinenew, CDialog)
CGSPolylinenew::CGSPolylinenew(CWnd* pParent /*=NULL*/)
	: CDialog(CGSPolylinenew::IDD, pParent)
{
    add_polyline = FALSE;
	m_strNamePolyline = _T("");
    m_strPolylineObjectName = "POLYLINE";
}

CGSPolylinenew::~CGSPolylinenew()
{
}

void CGSPolylinenew::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LISTCTRL_POLYLINENEW, m_Listctrlpolylinenew);
    DDX_Control(pDX, IDC_COMBO_PLY_MSH, m_ply_mshcombo);
}


BEGIN_MESSAGE_MAP(CGSPolylinenew, CDialog)
    ON_BN_CLICKED(IDC_POLYLINE_ADD, OnClickedPolylineAdd)
    ON_BN_CLICKED(IDC_POLYLINE_REMOVE, OnClickedPolylineRemove)
    ON_BN_CLICKED(IDC_POLYLINE_REMOVEALL, OnClickedPolylineRemoveall)
    ON_BN_CLICKED(IDC_POLYLINE_UNSELECT, OnClickedPolylineUnselect)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_READ, OnClickedButtonPolylineRead)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_WRITE, OnClickedButtonPolylineWrite)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_WRITETEC, OnClickedButtonPolylineWritetec)
    ON_BN_CLICKED(IDC_POLYLINE_ADD_LAYER, OnClickedPolylineAddLayer)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CGSPolylinenew message handlers
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
BOOL CGSPolylinenew::OnInitDialog() 
{
    CDialog::OnInitDialog();
    CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("TYPE");
    CString         strItem3= _T("DATA");
    CString         strItem4= _T("EPS");
    CString         strItem5= _T("MD");
    CString         strItem6= _T("MAT");
    m_Listctrlpolylinenew.GetWindowRect(&rect); 
    m_Listctrlpolylinenew.SetTextBkColor(RGB(153,153,255));
    m_Listctrlpolylinenew.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_Listctrlpolylinenew.EnableTrackingToolTips();
    m_Listctrlpolylinenew.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );
	m_Listctrlpolylinenew.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/6, 0);
	m_Listctrlpolylinenew.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/4, 1);
    m_Listctrlpolylinenew.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/6, 2);
    m_Listctrlpolylinenew.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/7, 3);
    m_Listctrlpolylinenew.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/7, 4);
    m_Listctrlpolylinenew.InsertColumn(5, strItem6, LVCFMT_LEFT,
        rect.Width() * 1/7, 5);
  
    UpdatePolylineList();

//MSH
     CFEMesh* m_msh = NULL;
     m_ply_mshcombo.ResetContent();
     m_ply_mshcombo.InitStorage(10,80);
     for(int i=0;i<(int)fem_msh_vector.size();i++){
     m_msh = fem_msh_vector[i];
     m_ply_mshcombo.AddString((CString)m_msh->pcs_name.data());
     m_ply_mshcombo.SetCurSel(0);
     }
  return TRUE; // return TRUE unless you set the focus to a control	              // EXCEPTION: OCX Property Pages should return FALSE
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
01/2006 CC Modification
**************************************************************************/
void CGSPolylinenew::OnClickedPolylineAdd()
{
    CGLPolyline* gs_polyline = NULL;
    if (!add_polyline){
    add_polyline = TRUE;
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->m_bIsPolylineEditorOn = 1;
    long number_of_polylines;
    //m_Listctrlpolylinenew.DeleteAllItems();
    CGLPolyline* ms_polyline = NULL; //CC
    long size = 0;
    number_of_polylines = (long)polyline_vector.size();
    if(number_of_polylines==0){
        sprintf(c_string_plys, "%ld",number_of_polylines);
        size = 0;
      }
    else
      {
        vector<CGLPolyline*>::iterator ps = polyline_vector.begin();
        while (ps!=polyline_vector.end()){
          ms_polyline = *ps;
          ++ps;
        }
        size = ms_polyline->id + 1;
        number_of_polylines =  ms_polyline->id + 1;
        sprintf(c_string_plys, "%ld",number_of_polylines);
      } 
    m_strNamePolyline = m_strPolylineObjectName;
    m_strNamePolyline += c_string_plys;
  // Create new polyline
    strcpy(c_string_plys,m_strNamePolyline);
  //gs_polyline = gs_polyline->Create(c_string_ply);//CC
    gs_polyline = new CGLPolyline;
    gs_polyline->name = c_string_plys;
    polyline_vector.push_back(gs_polyline);
    gs_polyline->id = size;
    gs_polyline->ply_type = "GEO_POINTS";
    gs_polyline->ply_data = "POINTS";
      
  // Insert to list control
 // 
    size = (long)polyline_vector.size()-1;
    AddPolylinetoList(size);
 //------------------------------------------------ //CC
    CGLPolyline *m_polyline = NULL;
    vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  
    while(p!=polyline_vector.end()) {
				  m_polyline = *p;
                  m_polyline->highlighted = false;
    ++p;}
    gs_polyline->highlighted = true; // CC
  // Refresh view of data elements
    UpdateData(FALSE);
	
    // Additional implementation by PCH starts here.
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    // Create new polyline point
    UpdateData(TRUE);
    //change data in point
    strcpy(c_string_plys,m_strNamePolyline);
    gs_polyline = GEOGetPLYByName(c_string_plys);//CC

    
    // Just add picked points to polyline
    for (int i = 0; i < theApp->hitsNodeTotal; ++i)
	{
        // According to the convention of .gli file, the real index number of nodes is not used to create polylines
        // which is different from my TechFlow
        // Getting the number of gli points in the list
        long numOfGLIPoints = (long)gli_points_vector.size();

        // Let's have a temp CGLPoint.
        CGLPoint *m_point = NULL;
        m_point = new CGLPoint;

        // Just update the value of this temp CGLPoint
        m_point->id = numOfGLIPoints;
        m_point->mesh_density = 0.0;  // I just set 0.0 because I don't know which is default value
        m_point->value = 0.0; // I just set 0.0 because I don't know which is default value

		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		
		CGLPoint aPoint = pView->GetGLIPointByIndex(theApp->nodePickedTotal[i]);
		
		m_point->x = aPoint.x;	m_point->y = aPoint.y;	m_point->z = aPoint.z;
        
        // Check if the point exists in the list
        int InThePointList = m_point->IsPointExist();
        // If not exist in the list
        if (InThePointList == -100) // The number is set to be negative intentionally.
            gli_points_vector.push_back(m_point);
        else
            // If exist, don't add the duplicate.
            // But, update the index of the point list
            m_point->id = InThePointList;

        // Use addPoint the polyline list everytime whether the point exist or not.
        gs_polyline->AddPoint(m_point);   
    }

    // Let's display in the list in this dialog
    // "theApp->hitsNodeTotal = gs_polyline->point_vector.size()" should be right at this line.
  //----------------------------------------------------------------------
    if(gli_points_vector.size()>0){
    theApp->g_graphics_modeless_dlg->m_iDisplayPLY = true;
    }
    else{
    theApp->g_graphics_modeless_dlg->m_iDisplayPLY = false;
    }
    theApp->g_graphics_modeless_dlg->OnInitDialog();
    }
    else

    {
    MessageBox("Click OK to finish editing the polyline or right button click on the editing window!",0,MB_OK);
    add_polyline = false;
    }
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
03/2006 CC
**************************************************************************/
void CGSPolylinenew::OnClickedPolylineRemove()
{
  
  add_polyline = false;
  //CGLPolyline* m_ply = NULL;
  POSITION p = m_Listctrlpolylinenew.GetFirstSelectedItemPosition();
  if(p==NULL) return;
  else{
  long m_iSel = (long) m_Listctrlpolylinenew.GetNextSelectedItem(p);
  //m_ply = polyline_vector[m_iSel];
  //GEORemovePolyline(polyline_vector.begin() + m_iSel);//CC
  GEORemovePolyline(m_iSel);//CC
  //GEORemovePLY(m_ply);
  UpdatePolylineList();
  }
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
void CGSPolylinenew::OnClickedPolylineRemoveall()
{
  add_polyline = false;
  long polyline_vector_size;
  long i;
  polyline_vector_size = (long)polyline_vector.size();
  for(i = 0;i<polyline_vector_size;0)
  {
	  //GEORemovePolyline(polyline_vector.begin()+i);
      GEORemovePolyline(i);
	  polyline_vector_size = (long)polyline_vector.size();
  }

  m_Listctrlpolylinenew.DeleteAllItems();
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
void CGSPolylinenew::OnClickedPolylineUnselect()
{
   CGLPolyline *gl_ply = NULL;
   vector<CGLPolyline*>::iterator p = polyline_vector.begin();
				  while(p!=polyline_vector.end())
				  {
				  gl_ply = *p;
                  gl_ply->highlighted = false;
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
void CGSPolylinenew::OnClickedButtonPolylineRead()
{
    // TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPolylinenew::OnClickedButtonPolylineWrite()
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
void CGSPolylinenew::OnClickedButtonPolylineWritetec()
{
  string file_path;
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp)
    file_path = m_gsp->path; //CC
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  CGLPolyline *m_ply =NULL;
  while(p!=polyline_vector.end()) { 
    m_ply = *p;
    m_ply->WriteTecplot(file_path);//CC
    ++p;
  }
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
02/2006 CC Implementation
**************************************************************************/
void CGSPolylinenew::OnClickedPolylineAddLayer()
{
//get the polyline;
  CFEMesh* m_msh = NULL;
  CGLPolyline * m_ply = NULL;
  POSITION p = m_Listctrlpolylinenew.GetFirstSelectedItemPosition();
  if(p==NULL) {
  AfxMessageBox("No polyline is selected !");
  return;
  }
  else{
  int m_iSel = m_Listctrlpolylinenew.GetNextSelectedItem(p);
  m_ply = polyline_vector[m_iSel];
  }
//get msh file
  if(m_ply_mshcombo.GetCurSel()== CB_ERR)
    {
    AfxMessageBox("No mesh file is selected !");
    return;
    }
  else{
     int m_mSel = m_ply_mshcombo.GetCurSel();
     m_msh = fem_msh_vector[m_mSel];
    }
  m_msh->CreateLayerPolylines(m_ply);
UpdatePolylineList();
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGSPolylinenew::UpdatePolylineList()
{
    char sID[1024];
    char TYPEvalue[1024];
    char DATAvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    long listip = 0;
    CGLPolyline* gl_ply = NULL;
    m_Listctrlpolylinenew.DeleteAllItems();
	vector<CGLPolyline*>::iterator p = polyline_vector.begin();
    while(p!=polyline_vector.end()) {
    gl_ply = *p;  
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_ply->name.data());//CC
    sprintf(TYPEvalue,"%s",gl_ply->ply_type.data());
    sprintf(DATAvalue,"%s",gl_ply->ply_data.data());
    sprintf(EPSvalue,"%lg",gl_ply->epsilon);
    sprintf(MDvalue,"%lg",gl_ply->mesh_density);
    sprintf(MATvalue,"%d",gl_ply->mat_group);
   
    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list

    lvitem.iSubItem=0;
    
    m_Listctrlpolylinenew.InsertItem(&lvitem);

    m_Listctrlpolylinenew.SetItemText(listip,1,TYPEvalue);
    m_Listctrlpolylinenew.SetItemText(listip,2,DATAvalue);
    m_Listctrlpolylinenew.SetItemText(listip,3,EPSvalue);
    m_Listctrlpolylinenew.SetItemText(listip,4,MDvalue);
    m_Listctrlpolylinenew.SetItemText(listip,5,MATvalue);
 
    ++p;
    ++listip;
     
    }
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
12/2005 CC
**************************************************************************/
void CGSPolylinenew::OnCancel() 
{
  CDialog::OnCancel();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPolylineEditorOn = 0;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();//CC
  if(theApp->pPolylinenew->GetSafeHwnd()!=NULL)
  theApp->pPolylinenew->add_polyline = false;
  //----------------------------------------------------------------------
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGSPolylinenew::AddPolylinetoList(long size)
{
    char sID[1024];
    char Typevalue[1024];
    char DATAvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    CGLPolyline* gl_ply = NULL;
    gl_ply = polyline_vector[size];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_ply->name.data());//CC
    sprintf(Typevalue,"%s",gl_ply->ply_type.data());
    sprintf(DATAvalue,"%s",gl_ply->ply_data.data());
    sprintf(EPSvalue,"%lg",gl_ply->epsilon);
    sprintf(MDvalue,"%lg",gl_ply->mesh_density);
    sprintf(MATvalue,"%d",gl_ply->mat_group);
    lvitem.pszText=sID;
    lvitem.iItem=(int)size;//size of list

    lvitem.iSubItem=0;
    
    m_Listctrlpolylinenew.InsertItem(&lvitem);

    m_Listctrlpolylinenew.SetItemText((int)size,1,Typevalue);
    m_Listctrlpolylinenew.SetItemText((int)size,2,DATAvalue);
    m_Listctrlpolylinenew.SetItemText((int)size,3,EPSvalue);
    m_Listctrlpolylinenew.SetItemText((int)size,4,MDvalue);
    m_Listctrlpolylinenew.SetItemText((int)size,5,MATvalue);
 
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation edit polyline properties in listctrl
**************************************************************************/
void CGSPolylinenew::OnButtonPolylinePropertiesUpdate() 
{
  UpdateData(TRUE);
  CGLPolyline *gl_ply = NULL;
  POSITION p = m_Listctrlpolylinenew.GetFirstSelectedItemPosition();
  if (p == NULL) return;
  else
  {
  int nSel = m_Listctrlpolylinenew.GetNextSelectedItem(p);
  int listip = 0;
  vector<CGLPolyline*>::iterator pl = polyline_vector.begin();
  while(pl!=polyline_vector.end())
  {
  gl_ply= *pl;
  string nameID;
  string TYPEvalue;
  string DATAvalue;
  string EPSvalue;
  string MDvalue;
  string MATvalue;

  nameID = (string)m_Listctrlpolylinenew.GetItemText(listip,0);
  TYPEvalue = (string)m_Listctrlpolylinenew.GetItemText(listip,1);
  DATAvalue = (string)m_Listctrlpolylinenew.GetItemText(listip,2);
  EPSvalue = (string)m_Listctrlpolylinenew.GetItemText(listip,3);
  MDvalue = (string)m_Listctrlpolylinenew.GetItemText(listip,4);
  MATvalue = (string)m_Listctrlpolylinenew.GetItemText(listip,5);

  gl_ply->name = nameID;
  gl_ply->ply_type = TYPEvalue;
  gl_ply->ply_data = DATAvalue;
  gl_ply->epsilon = strtod(EPSvalue.data(),NULL);
  gl_ply->mesh_density = strtod(MDvalue.data(), NULL);
  //changing the MD of PLY will also change the MD of the points
  CGLPoint* gl_point = NULL;
  vector<CGLPoint*>::iterator pv = gl_ply->point_vector.begin();
      while (pv!=gl_ply->point_vector.end())
      {
        gl_point = *pv;
        gl_point->mesh_density = gl_ply->mesh_density ;
        ++pv;    
      }
  //--------------------------------------------------------------------
  gl_ply->mat_group = atoi(MATvalue.data());
  gl_ply->highlighted = false;
  ++pl;
  ++listip;
  }
  polyline_vector[nSel]->highlighted = true;
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
void CGSPolylinenew::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
      //----------------------------------------------------------------------
  CDialog::OnActivate(nState, pWndOther, bMinimized);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->ActiveDialogIndex = 2;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
  mainframe->m_bIsPolylineEditorOn = 1;
  mainframe->m_bIsSurfaceEditorOn = 0;
  mainframe->m_bIsVolumeEditorOn = 0;
  UpdatePolylineList();
  //OnButtonPolylinePropertiesUpdate();
 //MSH
  CFEMesh* m_msh = NULL;
  m_ply_mshcombo.ResetContent();
  m_ply_mshcombo.InitStorage(10,80);
  for(int i=0;i<(int)fem_msh_vector.size();i++){
  m_msh = fem_msh_vector[i];
  m_ply_mshcombo.AddString((CString)m_msh->pcs_name.data());
  m_ply_mshcombo.SetCurSel(i);}
}
