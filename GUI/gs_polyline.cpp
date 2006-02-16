// gs_polyline.cpp : implementation file
//

#include "stdafx.h"
#include "geosys.h"
#include "gs_polyline.h"
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
CGLPolyline *gs_polyline = NULL;
//MSHLib
#include"msh_lib.h"
// GUI
#include "gs_graphics.h"
#include ".\gs_polyline.h"

char c_string_ply[MAX_ZEILE];
CString m_string_ply;
bool v_drawpolyline = FALSE;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPolyline dialog
CPolyline::CPolyline(CWnd* pParent /*=NULL*/)
	: CDialog(CPolyline::IDD, pParent)
    , m_dPDMin(0)
{
	//{{AFX_DATA_INIT(CPolyline)
	m_dBufferZone = 0.0;
    m_PlyMeshDensity = 0.0;
	m_strNamePolyline = _T("");
    m_strPolylineObjectName = "POLYLINE";
	add_polyline = FALSE;
	//}}AFX_DATA_INIT
}

void CPolyline::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPolyline)
    //DDX_Control(pDX, IDC_LIST_POLYLINE, m_LBPolylineProperties);
    DDX_Control(pDX, IDC_LIST_POLYLINES, m_LBPolylines);
    DDX_Text(pDX, IDC_EDIT_POLYLINE_1, m_strNamePolyline);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_LISTCTRL_POLYLINE, m_listctrlPolyline);// CC 06/2005
    DDX_Text(pDX, IDC_EDIT_POLYLINE_7, m_dBufferZone);
    DDX_Text(pDX, IDC_EDIT_MD, m_PlyMeshDensity);
    DDX_Text(pDX, IDC_EDIT_PDMIN, m_dPDMin);
}
BEGIN_MESSAGE_MAP(CPolyline, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_POLYLINE_ADD, OnButtonPolylineAdd)
	ON_BN_CLICKED(IDC_BUTTON_POLYLINE_UPDATE, OnButtonPolylineUpdate)
	ON_BN_CLICKED(IDC_BUTTON_POLYLINE_REMOVE, OnButtonPolylineRemove)
	ON_BN_CLICKED(IDC_BUTTON_POLYLINE_REMOVEALL, OnButtonPolylineRemoveall)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_PROPERTIES_REMOVE, OnButtonPolylinePropertiesRemove)
	ON_BN_CLICKED(IDC_BUTTON_POLYLINE_PROPERTIES_REMOVEALL, OnButtonPolylinePropertiesRemoveall)
	ON_LBN_SELCHANGE(IDC_LIST_POLYLINES, OnSelchangeListPolylines)
    ON_LBN_SELCHANGE(IDC_LIST_POLYLINES, OnLbnSelOnlyThePickedPolylines)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_ADD_LAYER, OnBnClickedButtonPolylineAddLayer)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_WRITE_TEC, OnBnClickedButtonPolylineWriteTEC)
    ON_BN_CLICKED(IDC_WRITEGLI, OnBnClickedWritegli)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_PROPERTIES_ADD, OnBnClickedButtonPolylinePropertiesAdd)
    ON_BN_CLICKED(IDC_BUTTON_POLYLINE_UNSELECT, OnBnClickedButtonPolylineUnselect)
    ON_WM_ACTIVATE()
    ON_BN_CLICKED(IDC_BUTTON_UPDATE_MD, OnBnClickedButtonUpdateMd)
    ON_BN_CLICKED(IDC_BUTTON_CALC_PDMIN, OnBnClickedButtonCalcPdmin)
END_MESSAGE_MAP()

//===========================================================================
// Polyline list
//---------------------------------------------------------------------------
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
09/2003 CC/OK Implementation
06/2005 CC Modification change from list box to list control
**************************************************************************/
BOOL CPolyline::OnInitDialog() 
{
  CDialog::OnInitDialog();

  vector<CGLPolyline*>::iterator p = polyline_vector.begin();// CC

  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    m_dBufferZone = gs_polyline->epsilon;
     // Mark selected polylines
    ++p;
  }
// initialize List control box
    CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("X");
    CString         strItem3= _T("Y");
    CString         strItem4= _T("Z");
    CString         strItem5= _T("value");
    CString         strItem6= _T("Mesh Density");
    CString         strItem7= _T("Min_Distance");
	  // let the base class do the default work
	
	// insert head colume
    m_listctrlPolyline.GetWindowRect(&rect);
    
    m_listctrlPolyline.SetTextBkColor(RGB(153,153,255));
    m_listctrlPolyline.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_listctrlPolyline.EnableTrackingToolTips();
    m_listctrlPolyline.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );// CC

	m_listctrlPolyline.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/9, 0);
	m_listctrlPolyline.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/7, 1);
    m_listctrlPolyline.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/7, 2);
    m_listctrlPolyline.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/7, 3);
    m_listctrlPolyline.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/7, 4);
    m_listctrlPolyline.InsertColumn(5, strItem6, LVCFMT_LEFT,
		rect.Width() * 1/7, 4);
    m_listctrlPolyline.InsertColumn(6, strItem7, LVCFMT_LEFT,
        rect.Width() * 1/5, 4);

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
09/2003 CC/OK Implementation
06/2005 OK g_graphics_modeless_dlg
07/2005 CC control the right button to finish creating polyline
07/2005 CC highlighting
07/2005 CC Control id of polyline
**************************************************************************/
void CPolyline::OnButtonPolylineAdd() 
{
  
  if (!add_polyline){
    add_polyline = TRUE;
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->m_bIsPolylineEditorOn = 1;
    long number_of_polylines;
    m_listctrlPolyline.DeleteAllItems();
    v_drawpolyline = TRUE;
  // Name identfier
  //----------------------------------
    CGLPolyline* ms_polyline = NULL; //CC
      long size = 0;
      number_of_polylines = (long)polyline_vector.size();
      // CC
      if(number_of_polylines==0){
        sprintf(c_string_ply, "%ld",number_of_polylines);
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
        sprintf(c_string_ply, "%ld",number_of_polylines);
      } 
  m_strNamePolyline = m_strPolylineObjectName;
  m_strNamePolyline += c_string_ply;
  // Create new polyline
  strcpy(c_string_ply,m_strNamePolyline);
  //gs_polyline = gs_polyline->Create(c_string_ply);//CC
  gs_polyline = new CGLPolyline;
  gs_polyline->name = c_string_ply;
  polyline_vector.push_back(gs_polyline);
  gs_polyline->id = size;
  // Insert to ListBox  
  m_LBPolylines.AddString(m_strNamePolyline);
 //------------------------------------------------ //CC
  CGLPolyline *m_polyline = NULL;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  
  while(p!=polyline_vector.end()) {
				 
				  
				  m_polyline = *p;
                  m_polyline->highlighted = false;
   ++p;}
  gs_polyline->highlighted = true; // CC

  m_LBPolylines.SetCurSel(number_of_polylines);
   m_LBPolylines.SetSel((int)number_of_polylines,TRUE);

  // Refresh view of data elements
  UpdateData(FALSE);
	
    // Additional implementation by PCH starts here.
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    // Create new polyline point
    UpdateData(TRUE);
    //change data in point
    strcpy(c_string_ply,m_strNamePolyline);
    gs_polyline = GEOGetPLYByName(c_string_ply);//CC

    
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
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    if(gli_points_vector.size()>0){
      theApp->g_graphics_modeless_dlg->m_iDisplayPLY = true;
    }
    else{
      theApp->g_graphics_modeless_dlg->m_iDisplayPLY = false;
    }
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
}
else
{
 MessageBox("Click OK to finish editing the polyline or right button click on the editing window!",0,MB_OK);
 add_polyline = false;
}
 
}

/**************************************************************************
GeoLibGUI-Method: OnSelchangeListPolylines() 
Task: 
Programing:
04/2005 CC Modificatin minimal distance between points
06/2005 CC Modification List control
06/2005 CC highlight polyline
**************************************************************************/
void CPolyline::OnSelchangeListPolylines() 
{
  // Get selected polyline from ListBox
  int nSel = m_LBPolylines.GetCurSel();
  m_LBPolylines.GetText(nSel,m_strNamePolyline);
  // Search polyline with name: m_strNamePolyline 
  strcpy(c_string_ply,m_strNamePolyline);
  CGLPolyline *gl_polyline = NULL;
  CGLPolyline *m_polyline = NULL;
  CGLPoint *gl_point = NULL;
  gl_polyline = GEOGetPLYByName(c_string_ply);//CC
  gl_polyline->highlighted = true;
  int sel = 0;
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) 
  {
    m_polyline = *p;
    if (sel == nSel)
      m_polyline->highlighted = true;
    else
      m_polyline->highlighted = false;
	++p;
    ++sel;
  }
  if(gl_polyline) 
  {
    m_dBufferZone = gl_polyline->epsilon;
    m_PlyMeshDensity = gl_polyline->minDis/2;
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    char Tvalue[1024];
    char Vvalue[1024];
    char Dvalue[1024];
    long listip = 0;
    m_listctrlPolyline.DeleteAllItems();
	vector<CGLPoint*>::iterator p = gl_polyline->point_vector.begin();
    while(p!=gl_polyline->point_vector.end()) {
    gl_point = *p;
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_point->name.data());//CC
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);
    sprintf(Tvalue,"%lg",gl_point->value);
    sprintf(Vvalue,"%lg",gl_point->mesh_density);
    sprintf(Dvalue,"%lg",gl_polyline->minDis);
    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list
    lvitem.iSubItem=0;
    m_listctrlPolyline.InsertItem(&lvitem);
    m_listctrlPolyline.SetItemText(listip,1,Xvalue);
    m_listctrlPolyline.SetItemText(listip,2,Yvalue);
    m_listctrlPolyline.SetItemText(listip,3,Zvalue);
    m_listctrlPolyline.SetItemText(listip,4,Tvalue);
    m_listctrlPolyline.SetItemText(listip,5,Vvalue);
    m_listctrlPolyline.SetItemText(listip,6,Dvalue);
    m_dPDMin = gl_polyline->minDis;
    ++p;
    ++listip;
    }
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  // Refresh view of data elements
  UpdateData(FALSE);
	
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
06/2005 CC Implementation edit polyline properties in listctrl
07/2005 CC update the name of point
**************************************************************************/
void CPolyline::OnButtonPolylinePropertiesUpdate() 
{
  //get point by selection
  UpdateData(TRUE);

  strcpy(c_string_ply,m_strNamePolyline);
  gs_polyline = GEOGetPLYByName(c_string_ply);//CC
  CGLPoint *gl_point = NULL;
  int listip = 0;
  vector<CGLPoint*>::iterator p = gs_polyline->point_vector.begin();
  while(p!=gs_polyline->point_vector.end()) {
  gl_point = *p;
 // string nameID;//CC
  string Xvalue;
  string Yvalue;
  string Zvalue;
  string Tvalue;
  string Vvalue;
  string Dvalue;
  
//get text from list item
  //nameID = (string)m_listctrlPolyline.GetItemText(listip,0);//CC
  Xvalue = (string)m_listctrlPolyline.GetItemText(listip,1);
  Yvalue = (string)m_listctrlPolyline.GetItemText(listip,2);
  Zvalue = (string)m_listctrlPolyline.GetItemText(listip,3);
  Tvalue = (string)m_listctrlPolyline.GetItemText(listip,4);
  Vvalue = (string)m_listctrlPolyline.GetItemText(listip,5);
  Dvalue = (string)m_listctrlPolyline.GetItemText(listip,6);

 // update the data in the point
 // gl_point->name = nameID;//CC
  gl_point->x = strtod(Xvalue.data(), NULL);
  gl_point->y = strtod(Yvalue.data(), NULL);
  gl_point->z = strtod(Zvalue.data(), NULL);
  gl_point->value = strtod(Tvalue.data(),NULL);
  gl_point->mesh_density = strtod(Vvalue.data(), NULL);
  gs_polyline->minDis = strtod(Dvalue.data(), NULL);

++p;
++listip;
    }
  UpdateData(FALSE);	
}

/**************************************************************************
GeoLibGUI-Method: OnLbnSelOnlyThePickedPolylines()
Task: This will highlist the polylines picked in CListBox of Polyline 
Programing:
04/2005 PCH Implementation
**************************************************************************/
void CPolyline::OnLbnSelOnlyThePickedPolylines()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Let's delect them first before we highlight the picked polylines
	//OnBnClickedButtonSelectnone();//CC

	// Let's highlist the surfaces picked.
	for(int i=0; i< theApp->hitsPolylineTotal; ++i)
	{
		m_LBPolylines.SetSel(theApp->polylinePickedTotal[i],TRUE);
	}
	
	UpdateData(FALSE);
}
/**************************************************************************
GeoLibGUI-Method:
Task: 
Programing:
08/2005 CC Implementation
08/2005 CC modification name of polyline should not repeat
**************************************************************************/
void CPolyline::OnButtonPolylineUpdate() 
{
  // Retrieve data elements
  UpdateData(TRUE);
  int nSel = m_LBPolylines.GetCurSel();
  CGLPolyline *gl_polyline = NULL;
  gl_polyline = polyline_vector[nSel];
  if(gl_polyline) {
    //---------------------------------------//CC check no double name
   CGLPolyline* m_ply = NULL;
   vector<CGLPolyline*>::iterator p1 = polyline_vector.begin();
   int i = 0;
   while(p1!=polyline_vector.end()){
    m_ply = *p1;
    if (m_ply->name.compare(m_strNamePolyline)==0 && nSel != i )
     {
     AfxMessageBox(" Please choose a different name: the polyline name already exists");
     return;
     }
     ++p1;
     ++i;}
    //-----------------------------------
    gl_polyline->name = m_strNamePolyline;
    gl_polyline->epsilon = m_dBufferZone;
   
  }
  // Refresh view of data elements
  UpdateData(FALSE);
  // Fill polyline list
  m_LBPolylines.ResetContent();
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
04/2005 CC Modificatin 
06/2005 CC Modification highlight
07/2005 CC Modification min distance recalculate
10/05 CC test Geolib dll
**************************************************************************/
void CPolyline::OnButtonPolylineRemove() 
{
  add_polyline = false;
  strcpy(c_string_ply,m_strNamePolyline);
  long nSel = m_LBPolylines.GetCurSel();
 
  if (nSel != LB_ERR){
  m_LBPolylines.DeleteString(nSel);
  m_listctrlPolyline.DeleteAllItems();
//  CGLPolyline *gl_polyline = NULL;
  GEORemovePolyline(polyline_vector.begin()+nSel);//CC
  m_strNamePolyline = "";
  m_LBPolylines.ResetContent();
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
  }// end of if CC
  else 
      return;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();

  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: update new created polyline point list control box
Task: 
Programing:
06/2005 CC Modificatin 
07/2005 CC Mesh density
**************************************************************************/
void CPolyline::OnUpdatePolylinePointList()
{
  strcpy(c_string_ply,m_strNamePolyline);

  CGLPolyline *gl_polyline = NULL;
  CGLPoint *gl_point = NULL;
  gl_polyline = GEOGetPLYByName(c_string_ply);//CC

  //---------------------------------------------------------
  CGLPoint* start_point = NULL;
      CGLPoint* end_point = NULL;
      double m_dXMin = 1.e+19;
      vector<CGLPoint*> d_points_vector;
      long number_of_points = 0;
      d_points_vector = gl_polyline->point_vector;
      number_of_points = (long)d_points_vector.size();
      for (int i = 0;i<number_of_points-1;i++)
      {
        start_point = d_points_vector[i];
        end_point = d_points_vector[i+1];
        double min = sqrt((start_point->x-end_point->x)*(start_point->x-end_point->x)+(start_point->y-end_point->y)*(start_point->y-end_point->y));
        if(min<m_dXMin)
        m_dXMin = min;
      }
      gl_polyline->minDis = m_dXMin;
  //---------------------------------------------------------

   if(gl_polyline) {
     m_dBufferZone = gl_polyline->epsilon;
     m_PlyMeshDensity = gl_polyline->minDis/2;
      //
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    char Tvalue[1024];
    char Vvalue[1024];
    char Dvalue[1024];
  
    long listip = 0;
    m_listctrlPolyline.DeleteAllItems();
	vector<CGLPoint*>::iterator p = gl_polyline->point_vector.begin();
    while(p!=gl_polyline->point_vector.end()) {
    gl_point = *p;
      
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_point->name.data());//CC
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);
    sprintf(Tvalue,"%lg",gl_point->value);
    sprintf(Vvalue,"%lg",gl_point->mesh_density);
    sprintf(Dvalue,"%lg",gl_polyline->minDis);
   
    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list

    lvitem.iSubItem=0;
    
    m_listctrlPolyline.InsertItem(&lvitem);

    m_listctrlPolyline.SetItemText(listip,1,Xvalue);
    m_listctrlPolyline.SetItemText(listip,2,Yvalue);
    m_listctrlPolyline.SetItemText(listip,3,Zvalue);
    m_listctrlPolyline.SetItemText(listip,4,Tvalue);
    m_listctrlPolyline.SetItemText(listip,5,Vvalue);
    m_listctrlPolyline.SetItemText(listip,6,Dvalue);
 
    ++p;
    ++listip;
     
    }
  }

  // Refresh view of data elements
  UpdateData(FALSE);
}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 CC remove all the polylines
**************************************************************************/
void CPolyline::OnButtonPolylineRemoveall() 
{
  add_polyline = false;
  GEORemoveAllPolylines();
  m_LBPolylines.ResetContent();
  m_strNamePolyline = "";
  m_listctrlPolyline.DeleteAllItems();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
	
}
//===========================================================================
// Polyline properties
//---------------------------------------------------------------------------

void CPolyline::OnUpdatePointListPicked()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Let's display in the list in this dialog
	// Make CListBox empty before we proceed our picking
	
	
    // "theApp->hitsNodeTotal = gs_polyline->point_vector.size()" should be right at this line.
    for (int i = 0; i < theApp->hitsNodeTotal; ++i)
    {
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		
		CGLPoint aPoint = pView->GetGLIPointByIndex(theApp->nodePickedTotal[i]);
		
        sprintf(c_string_ply, "%lg, %lg, %lg - 0.0, 0.0", aPoint.x, aPoint.y, aPoint.z);
        //m_LBPolylineProperties.DeleteString(i);
        //m_LBPolylineProperties.InsertString(i,(CString) c_string_ply);
    }
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
06/2005  CC Implementation remove one point
07/2005  CC Modification refresh the list control list
07/2005  CC Remove points from polyline
**************************************************************************/
void CPolyline::OnButtonPolylinePropertiesRemove() 
{
  if(m_strNamePolyline == "")
  return;
  else{
   POSITION p = m_listctrlPolyline.GetFirstSelectedItemPosition();
   
   int nSelected = m_listctrlPolyline.GetNextSelectedItem(p);
	// Do something with item nSelected
  if(nSelected != -1){
   m_listctrlPolyline.DeleteItem(nSelected);

   // delete wells from list
  strcpy(c_string_ply,m_strNamePolyline);
  gs_polyline = GEOGetPLYByName(c_string_ply);//CC
  gs_polyline->point_vector.erase(gs_polyline->point_vector.begin()+nSelected);
   //GEOPointRemove( gli_points_vector.begin()+nSelected);
  OnUpdatePolylinePointList();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
 }
 else{
MessageBox("Please click on the list to select the point first!",0,MB_OK);
 return;

}
}
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
06/2005  CC Implementation remove all points belonging to the selected polyline
07/2005  CC remove polyline when remove all the points
**************************************************************************/
void CPolyline::OnButtonPolylinePropertiesRemoveall() 
{
 
 OnButtonPolylineRemove();
 /* m_listctrlPolyline.DeleteAllItems();
  long i;
  long point_vector_size;
  strcpy(c_string_ply,m_strNamePolyline);
  gs_polyline = gs_polyline->GEOGetPolyline(c_string_ply);
  for(i=0;i<(long)gs_polyline->point_vector.size();i++)
  {
	gs_polyline->point_vector.erase(gs_polyline->point_vector.begin()+i);
	point_vector_size = (long)gs_polyline->point_vector.size();
  }*/
}      
      
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
07/2005 OK m_bIsPointEditorOn
07/2005 CC Cursor
**************************************************************************/
void CPolyline::OnOK() 
{   
  CDialog::OnOK();
  //OK Invalidate(FALSE);
  //OK UpdateWindow();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPolylineEditorOn = 0;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();//CC
  if(theApp->pPolyline->GetSafeHwnd()!=NULL)
  theApp->pPolyline->add_polyline = false;
  //----------------------------------------------------------------------
}
 
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
07/2005 OK m_bIsPointEditorOn
07/2005 CC Cursor
**************************************************************************/
void CPolyline::OnCancel() 
{
  CDialog::OnCancel();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPolylineEditorOn = 0;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();//CC
  if(theApp->pPolyline->GetSafeHwnd()!=NULL)
  theApp->pPolyline->add_polyline = false;
  //----------------------------------------------------------------------
}
//---------------------------------------------------------------------------
// 09/2003 OK Implementation
void CPolyline::DrawPoints(CClientDC *dc,CGLPolyline *m_polyline)
{
  CGLPoint *m_point = NULL;
  vector<CGLPoint*> vector_points = m_polyline->point_vector;
  vector<CGLPoint*>::iterator pp = vector_points.begin();
  while(pp!=vector_points.end()) {
    m_point = *pp;
    CRect MyQuadrat (m_point->x_pix-m_point->circle_pix,m_point->y_pix+m_point->circle_pix,\
                     m_point->x_pix+m_point->circle_pix,m_point->y_pix-m_point->circle_pix);
    dc->Ellipse(MyQuadrat);
    ++pp;
  }
}

//---------------------------------------------------------------------------
// 09/2003 OK Implementation
void CPolyline::FillPolygon(CClientDC *dc,CGLPolyline *gl_polyline)
{
  POINT m_arrPoint[100];
  CGLPoint *gl_point = NULL;
  vector<CGLPoint*>::iterator pp = gl_polyline->point_vector.begin();
  long i=0;
  while(pp!=gl_polyline->point_vector.end()) {
    gl_point = *pp;
    m_arrPoint[i].x = gl_point->x_pix;
    m_arrPoint[i].y = gl_point->y_pix;
    i++;
    ++pp;
  }
  CRgn polygon;
  polygon.CreatePolygonRgn(&m_arrPoint[0],(int)gl_polyline->point_vector.size(),WINDING);//CC
  CBrush PolygonBrush;
  PolygonBrush.CreateSolidBrush(RGB(240,240,240));
  dc->SelectObject(PolygonBrush);
  dc->FillRgn(&polygon,&PolygonBrush);
  DeleteObject(polygon);
  DeleteObject(PolygonBrush);
}




/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
01/2005 OK GEOWrite
01/2005 CC refresh polyline list
**************************************************************************/
void CPolyline::OnBnClickedButtonPolylineAddLayer()
{
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  int no_polylines = (int)polyline_vector.size();
  for(int i=0;i<no_polylines;i++) {
    gs_polyline = *p;
    if(gs_polyline->highlighted)

      MSHCreateLayerPolylines(gs_polyline); //CC
    ++p;
  }
  //----------------------------------------------------------------------
  string file_name_path_base;
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(m_gsp)
    file_name_path_base = m_gsp->path + m_gsp->base;
  GEOWrite(file_name_path_base); //OK41
  //----------------------------------------------------------------------
// CC 01/2005
  m_LBPolylines.ResetContent();
  vector<CGLPolyline*>::iterator p1 = polyline_vector.begin();//CC
  int ik = 0;
  CGLPolyline *gg_polyline = NULL;
  while(p1!=polyline_vector.end()) {
    gg_polyline = *p1;
    m_LBPolylines.AddString((CString)gg_polyline->name.c_str());
    m_dBufferZone = gg_polyline->epsilon;
   
     // Mark selected polylines
    if(gg_polyline->highlighted)
      m_LBPolylines.SetCurSel(ik);
  
    ++ik;
    ++p1;
  }
 UpdateData(FALSE);	
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
01/2005 OK Implementation
08/2005 CC
**************************************************************************/
void CPolyline::OnBnClickedButtonPolylineWriteTEC()
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
GeoSys-GUI-Method:
Task: This function saves the change made in .gli file
Programing: 
03/2005 PCH Implementation
**************************************************************************/
void CPolyline::OnBnClickedWritegli()
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

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing: 
06/2005 OK Implementation
**************************************************************************/
void CPolyline::OnBnClickedButtonPolylinePropertiesAdd()
{
  // 1 - Create new polyline point
  CGLPoint* m_pnt = NULL;
  m_pnt = new CGLPoint();
  // 2 - Get selected polyline from ListBox
  CGLPolyline *m_ply = NULL;
  m_ply = GEOGetPLYByName((string)m_strNamePolyline);//CC
  // 3 - Add point to polyline
  if(m_ply){
    m_ply->point_vector.push_back(m_pnt);
    //m_ply->point_list.push_back(m_pnt);//CC remove
  }
  // 4 - Add polyline properties to ListCtrl
  if(m_ply){
    FillPropertiesListCtrl(m_ply);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing: 
06/2005 OK Implementation
**************************************************************************/
void CPolyline::FillPropertiesListCtrl(CGLPolyline*m_ply)
{
  m_dBufferZone = m_ply->epsilon;
  char sID[80];
  char Xvalue[80];
  char Yvalue[80];
  char Zvalue[80];
  char Tvalue[80];
  char Vvalue[80];
  char Dvalue[80];
  long listip = 0;
  CGLPoint* m_pnt = NULL;
  m_listctrlPolyline.DeleteAllItems();
  //----------------------------------------------------------------------
  for(int i=0;i<(int)m_ply->point_vector.size();i++){
    m_pnt = m_ply->point_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",m_pnt->name.data()); // CC
    sprintf(Xvalue,"%lg",m_pnt->x);
    sprintf(Yvalue,"%lg",m_pnt->y);
    sprintf(Zvalue,"%lg",m_pnt->z);
    sprintf(Tvalue,"%lg",m_pnt->value);
    sprintf(Vvalue,"%lg",m_pnt->mesh_density);
    sprintf(Dvalue,"%lg",m_ply->minDis);
    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list
    lvitem.iSubItem=0;
    m_listctrlPolyline.InsertItem(&lvitem);
    m_listctrlPolyline.SetItemText(listip,1,Xvalue);
    m_listctrlPolyline.SetItemText(listip,2,Yvalue);
    m_listctrlPolyline.SetItemText(listip,3,Zvalue);
    m_listctrlPolyline.SetItemText(listip,4,Tvalue);
    m_listctrlPolyline.SetItemText(listip,5,Vvalue);
    m_listctrlPolyline.SetItemText(listip,6,Dvalue);
    ++listip;
  }
  //----------------------------------------------------------------------
  // Refresh view of data elements
  UpdateData(FALSE);
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing: 
07/2005 CC Implementation
**************************************************************************/
void CPolyline::OnBnClickedButtonPolylineUnselect()
{
   CGLPolyline *gl_polyline = NULL;
   //int nSel = m_ListPoint.GetCurSel();
   m_LBPolylines.SetCurSel(-1);
    vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
				  while(p!=polyline_vector.end())
				  {
				  gl_polyline = *p;
                  gl_polyline->highlighted = false;
                 ++p;
     }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoLibGUI-Method: set the index for the activate dialog
Task: 
Programing:
07/2005 CC Implementation
07/2005 OK mainframe
08/2005 CC update m_LBPolylines list box
**************************************************************************/
void CPolyline::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
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
  //----------------------------------------------------------------------
   long i;
   m_LBPolylines.ResetContent();
    long ply_vector_size;
    vector<CGLPolyline*> gs_ply_vector;
    gs_ply_vector = GetPolylineVector();
    ply_vector_size = (long)gs_ply_vector.size();
    if(ply_vector_size>0){
    for(i=0;i<ply_vector_size;i++)
    {  
       m_LBPolylines.AddString(gs_ply_vector[i]->name.data());
      if(gs_ply_vector[i]->highlighted)
         m_LBPolylines.SetCurSel(i);
     }
    }
   else 
   return;

}

/**************************************************************************
GeoLibGUI-Method: set the index for the activate dialog
Task: 
Programing:
07/2005 CC Implementation
**************************************************************************/
void CPolyline::OnBnClickedButtonUpdateMd()
{
   UpdateData(true);

  strcpy(c_string_ply,m_strNamePolyline);
  gs_polyline =GEOGetPLYByName(c_string_ply);//CC
  CGLPoint *gl_point = NULL;
  CGLPoint *m_point = NULL;
 
  vector<CGLPoint*>::iterator p = gs_polyline->point_vector.begin();
  while(p!=gs_polyline->point_vector.end()) {
  gl_point = *p;
  gl_point->mesh_density = m_PlyMeshDensity;

  ++p;
  
    }

 // refresh the list control list of polyline
    if(gs_polyline) {
    m_dBufferZone = gs_polyline->epsilon;
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    char Tvalue[1024];
    char Vvalue[1024];
    char Dvalue[1024];
    long listipp = 0;
    m_listctrlPolyline.DeleteAllItems();
	vector<CGLPoint*>::iterator ps = gs_polyline->point_vector.begin();
    while(ps!=gs_polyline->point_vector.end()) {
    m_point = *ps;
      
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",m_point->name.data());//CC
    sprintf(Xvalue,"%lg",m_point->x);
    sprintf(Yvalue,"%lg",m_point->y);
    sprintf(Zvalue,"%lg",m_point->z);
    sprintf(Tvalue,"%lg",m_point->value);
    sprintf(Vvalue,"%lg",m_point->mesh_density);
    sprintf(Dvalue,"%lg",gs_polyline->minDis);
   
    lvitem.pszText=sID;
    lvitem.iItem=listipp;//size of list

    lvitem.iSubItem=0;
    
    m_listctrlPolyline.InsertItem(&lvitem);

    m_listctrlPolyline.SetItemText(listipp,1,Xvalue);
    m_listctrlPolyline.SetItemText(listipp,2,Yvalue);
    m_listctrlPolyline.SetItemText(listipp,3,Zvalue);
    m_listctrlPolyline.SetItemText(listipp,4,Tvalue);
    m_listctrlPolyline.SetItemText(listipp,5,Vvalue);
    m_listctrlPolyline.SetItemText(listipp,6,Dvalue);
 
    ++ps;
    ++listipp;
     
    }
  }
      UpdateData(FALSE);
}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 CC Implementation release all highlight
**************************************************************************/
void CPolyline::OnReleaseAllPlyhighlight()
{
 vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  CGLPolyline *m_polyline =NULL;
  while(p!=polyline_vector.end()) {
				 
				  
				  m_polyline = *p;
                  m_polyline->highlighted = false;
   ++p;}
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);

}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 CC Implementation mark the selected point
**************************************************************************/
void CPolyline::OnMarkselectedPoint()
{
   POSITION p = m_listctrlPolyline.GetFirstSelectedItemPosition();
   
   int nSelected = m_listctrlPolyline.GetNextSelectedItem(p);
	// Do something with item nSelected
   if(nSelected != -1){
 

   // delete wells from list
  CGLPoint *m_point = NULL;
m_point = new CGLPoint;
  CGLPoint *gl_point = NULL;
  strcpy(c_string_ply,m_strNamePolyline);
  gs_polyline = GEOGetPLYByName(c_string_ply);//CC
m_point = gs_polyline->point_vector[nSelected];  
  
   vector<CGLPoint*>::iterator p = gli_points_vector.begin();
				  while(p!=gli_points_vector.end())
				  {
				  gl_point = *p;
                
                  gl_point->highlighted = false;
				  ++p;
             
                  }
  m_point->highlighted = true;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);

}
else
return;
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 OK Implementation
**************************************************************************/
void CPolyline::OnBnClickedButtonCalcPdmin()
{
  UpdateData(TRUE);
  CGLPolyline* m_ply = NULL;
  m_ply = GEOGetPLYByName((string)m_strNamePolyline);
  if(m_ply)
  {
    m_ply->CalcMinimumPointDistance();
    m_dPDMin = m_ply->minDis;
  }
  UpdateData(FALSE);
}

