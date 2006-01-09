// gs_pnt.cpp : implementation file
//

/**************************************************************************
GeoSys GUI - Object: Point Editor
Task: 
Programing:
10/2003 CC Implementation
**************************************************************************/

#include "stdafx.h"
#include "geosys.h"
#include "MainFrm.h" //CC
#include "gs_pnt.h"

// C++ STL
#include <string>
#include <vector>
using namespace std;

// GeoLib
#include "geo_lib.h"
#include "geo_pnt.h"

// FEMLib
#include "makros.h"
#include ".\gs_pnt.h"
#include "rf_out_new.h"

int nSel(0);
char c_string_pnt[MAX_ZEILE];
bool v_drawpoint =FALSE;

CGLPoint *gs_point = NULL;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGSPoint dialog


CGSPoint::CGSPoint(CWnd* pParent /*=NULL*/)
	: CDialog(CGSPoint::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGSPoint)

	m_Pointname = _T("");
	add_point = FALSE;
	//}}AFX_DATA_INIT
}


void CGSPoint::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGSPoint)
	
	DDX_Control(pDX, IDC_LIST_POINTS, m_ListPoint);
	DDX_Text(pDX, IDC_EDIT_POINT_1, m_Pointname);
    DDX_Control(pDX, IDC_LISTCTRL_POINT, m_Listctrlpoint);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGSPoint, CDialog)
	//{{AFX_MSG_MAP(CGSPoint)
	ON_BN_CLICKED(IDC_BUTTON_POINT_REMOVE, OnButtonPointRemove)
	ON_BN_CLICKED(IDC_BUTTON_POINT_REMOVEALL, OnButtonPointRemoveall)
	ON_BN_CLICKED(IDC_BUTTON_POINT_PROPERTIES_UPDATE, OnButtonPointPropertiesUpdate)
	ON_LBN_SELCHANGE(IDC_LIST_POINTS, OnSelchangeListPoints)
	//}}AFX_MSG_MAP
	
	ON_BN_CLICKED(IDC_BUTTON_POINT_UPDATE2, OnBnClickedButtonPointUpdateAll)
    ON_BN_CLICKED(IDC_BUTTON_POINT_REMOVE_DOUBLEPOINTS, OnBnClickedButtonPointRemoveDoublePoints)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_POINT_NO_HIGHLIGHT, OnBnClickedButtonPointNoHighlight)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGSPoint message handlers

/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
BOOL CGSPoint::OnInitDialog() 
{
  CDialog::OnInitDialog();
  long i;
  long point_vector_size;
  vector<CGLPoint*> gs_points_vector;
  gs_points_vector = GetPointsVector();
  point_vector_size = (long)gs_points_vector.size();
  for(i=0;i<point_vector_size;i++)
  {  
  
    m_ListPoint.AddString(gs_points_vector[i]->name.data());

  }
  ////////////////////////////////////////////////////////////
  ////////////////////06/2004 CC list table

    CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("X");
    CString         strItem3= _T("Y");
    CString         strItem4= _T("Z");
    CString         strItem5= _T("Value");
    CString         strItem6= _T("Mesh Density");
	  // let the base class do the default work
	
	// insert head colume
    m_Listctrlpoint.GetWindowRect(&rect);
    
    m_Listctrlpoint.SetTextBkColor(RGB(153,153,255));
    m_Listctrlpoint.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_Listctrlpoint.EnableTrackingToolTips();
    m_Listctrlpoint.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );

	m_Listctrlpoint.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/6, 0);
	m_Listctrlpoint.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/6, 1);
    m_Listctrlpoint.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/6, 2);
    m_Listctrlpoint.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/6, 3);
    m_Listctrlpoint.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/9, 4);
    m_Listctrlpoint.InsertColumn(5, strItem6, LVCFMT_LEFT,
        rect.Width() * 2/9, 5);

//insert the list
//
    

  ////////////////////////////////////////////////////////////  
  UpdateData(FALSE);

  return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CGSPoint::OnBnClickedButtonPointUpdateAll()
{
  UpdateData(TRUE);
  long length = (long)gli_points_vector.size();
  for (long i=0;i<length;i++) {
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
06/2005 CC Implementation
**************************************************************************/
void CGSPoint::OnUpdatePointListbox()
{
  m_ListPoint.ResetContent();
  CGLPoint* m_pnt = NULL;
  for(long i=0;i<(long)gli_points_vector.size();i++)
  {  
    m_pnt = gli_points_vector[i];
    m_ListPoint.AddString((CString)m_pnt->name.c_str());
  }
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
06/2005 CC Implementation and Modification
07/2005 CC ToDo: remove point from polyline also
**************************************************************************/
void CGSPoint::OnButtonPointRemove() 
{
  //strcpy(c_string_pnt,m_Pointname);
  int nSel = m_ListPoint.GetCurSel();
  if (nSel != LB_ERR){
  //CGLPoint *gl_point = NULL;
  m_ListPoint.DeleteString(nSel);
  m_Pointname = "";
  m_Listctrlpoint.DeleteAllItems();
  GEORemovePoint( gli_points_vector.begin()+nSel);
  }
  else
      return;
 
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
 }

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
06/2005 CC  Modification
**************************************************************************/
void CGSPoint::OnButtonPointRemoveall() 
{  
  long point_vector_size;
  long i;
  point_vector_size = (long)gli_points_vector.size();
  //long size = gli_points_vector.size();
  for(i = 0;i<point_vector_size;0)
  {
	  GEORemovePoint(gli_points_vector.begin()+i);
	  point_vector_size = (long)gli_points_vector.size();
  }
  //CCtodo point_vector.empty();
  m_ListPoint.ResetContent();
 
  m_Pointname="";
  m_Listctrlpoint.DeleteAllItems();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}

/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
06/2005 CC Implementation
07/2005 CC Modification LB_ERR
07/2005 CC update the point list
08/2005 CC check whether point name already exist
**************************************************************************/
void CGSPoint::OnButtonPointPropertiesUpdate() 
{
  //get point by selection
  UpdateData(TRUE);
  CGLPoint *gl_point = NULL;
  int nSel = m_ListPoint.GetCurSel();
  if ( nSel!= LB_ERR){

  gl_point = gli_points_vector[nSel];
  string nameID;
  string Xvalue;
  string Yvalue;
  string Zvalue;
 // string Tvalue;
  string Vvalue;
  string Dvalue;
//get text from list item
  nameID = (string)m_Listctrlpoint.GetItemText(0,0);
  Xvalue = (string)m_Listctrlpoint.GetItemText(0,1);
  Yvalue = (string)m_Listctrlpoint.GetItemText(0,2);
  Zvalue = (string)m_Listctrlpoint.GetItemText(0,3);
 // Tvalue = (string)m_Listctrlpoint.GetItemText(0,4);
  Vvalue = (string)m_Listctrlpoint.GetItemText(0,4);
  Dvalue = (string)m_Listctrlpoint.GetItemText(0,5);
  m_Pointname = nameID.data();
 // update the data in the point
//checking whether point name already exists CC 08/2005
  CGLPoint* s_pnt = NULL;
  int i = 0;
  vector<CGLPoint*>:: iterator p = gli_points_vector.begin();
  while(p!=gli_points_vector.end())
  {
    s_pnt = *p;
    if (nameID == s_pnt->name && nSel != i )
     {
     AfxMessageBox(" Please choose a different name: the point name already exists");
     return;
     }
     ++p;
     ++i;
     }
  
  gl_point->name = nameID;
  gl_point->x = strtod(Xvalue.data(), NULL);
  gl_point->y = strtod(Yvalue.data(), NULL);
  gl_point->z = strtod(Zvalue.data(), NULL);
 // gl_point->type = atoi(Tvalue.data());
  gl_point->value = strtod(Vvalue.data(), NULL);
  gl_point->mesh_density = strtod(Dvalue.data(), NULL);
  UpdateData(FALSE);
  }
  else 
      return;
  m_ListPoint.ResetContent();
  CGLPoint* m_pnt = NULL;
  
  for(long i=0;i<(long)gli_points_vector.size();i++)
  {  
    m_pnt = gli_points_vector[i];
    m_ListPoint.AddString(m_pnt->name.data());
    if(m_pnt->highlighted)
      m_ListPoint.SetCurSel(i);
 
  }
  UpdateData(FALSE);
	
}

/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
07/2005 CC 
**************************************************************************/
void CGSPoint::OnCancel() 
{
  CDialog::OnCancel();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;

}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
07/2005 CC
**************************************************************************/
void CGSPoint::OnOK() 
{
  CDialog::OnOK();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
}


/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
06/2005 CC Modification highlight
**************************************************************************/
void CGSPoint::OnSelchangeListPoints() 
{
// Get selected point from ListBox
   nSel = m_ListPoint.GetCurSel();
   m_ListPoint.GetText(nSel,m_Pointname);
 

  // Search point with name: m_Pointname
   strcpy(c_string_pnt,m_Pointname);
  
   CGLPoint *m_point = NULL;
   CGLPoint *gl_point = NULL;
   int sel = 0;
   vector<CGLPoint*>::iterator p = gli_points_vector.begin();
				  while(p!=gli_points_vector.end())
				  {
				  m_point = *p;
                  if (sel == nSel)
                       m_point->highlighted = true;
                  else
                  m_point->highlighted = false;
				  ++p;
                  ++sel;
                  }
     gl_point = gli_points_vector[nSel];
   //gl_point->highlighted = true;

/*   if(gl_point->selected == TRUE)
      gl_point->selected = FALSE;
   else
      gl_point->selected = TRUE;*/
   if(gl_point) {
   //////////////////06/2004 CC///////////////////////////
    // clear the list control box
    m_Listctrlpoint.DeleteAllItems();
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
   // char Tvalue[1024];
    char Vvalue[1024];
    char Dvalue[1024];
    //long i = 0;
      sprintf(sID,"%s",gl_point->name.data());
    //sprintf(sID,"%lg",gl_point->name); // CC 0772005
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);

    sprintf(Vvalue,"%lg",gl_point->value);
    sprintf(Dvalue,"%lg",gl_point->mesh_density);
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText=sID;
    lvitem.iItem=1;//size of list
    lvitem.iSubItem=0;
    m_Listctrlpoint.InsertItem(&lvitem);
    m_Listctrlpoint.SetItemText(0,1,Xvalue);
    m_Listctrlpoint.SetItemText(0,2,Yvalue);
    m_Listctrlpoint.SetItemText(0,3,Zvalue);
   // m_Listctrlpoint.SetItemText(0,4,Tvalue);
    m_Listctrlpoint.SetItemText(0,4,Vvalue);
    m_Listctrlpoint.SetItemText(0,5,Dvalue);
    }
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
 UpdateData(FALSE);	
}

/**************************************************************************
GeoLibGUI-Method: draw point
Task: 
Programing:
10/2003 CC Implementation
**************************************************************************/
/*void CGSPoint::DrawPoint(CDC *dc, CGLPoint *m_point)
{ 
  int r = m_point->m_color[0];
  int g = m_point->m_color[1];
  int b = m_point->m_color[2];
  CBrush ColorSolidBrush;

  ColorSolidBrush.CreateSolidBrush(RGB(r,g,b));
  dc->SelectObject(&ColorSolidBrush);

  CRect MyQuadrat (m_point->x_pix-m_point->circle_pix,m_point->y_pix+m_point->circle_pix,\
                   m_point->x_pix+m_point->circle_pix,m_point->y_pix-m_point->circle_pix);
  dc->Ellipse(MyQuadrat);
  DeleteObject(ColorSolidBrush);
    
}*/


void CGSPoint::OnBnClickedButtonPointRemoveDoublePoints()
{
  // not initialized gli_point_id
  //GEO_Delete_DoublePoints(); //CC+TK
}
void CGSPoint::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    OnOK();
}

/**************************************************************************
GeoLibGUI-Method: cancel highlight on the point list
Task: 
Programing:
07/2005 CC Implementation
**************************************************************************/
void CGSPoint::OnBnClickedButtonPointNoHighlight()
{
   CGLPoint *gl_point = NULL;
   //int nSel = m_ListPoint.GetCurSel();
   m_ListPoint.SetCurSel(-1);
   vector<CGLPoint*>::iterator p = gli_points_vector.begin();
				  while(p!=gli_points_vector.end())
				  {
				  gl_point = *p;
                  gl_point->highlighted = false;
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
07/2005 CC Modification  list control
**************************************************************************/
void CGSPoint::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CDialog::OnActivate(nState, pWndOther, bMinimized);
    //SetActiveWindow( );
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    if(mainframe->m_bIsPointEditorOn == 0)
    mainframe->m_bIsPointEditorOn = 1;
    if(mainframe->m_bIsPointEditorOn)
    mainframe->m_bIsPolylineEditorOn = 0;

    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    if (theApp->pPolyline->GetSafeHwnd()!=NULL)
    theApp->pPolyline->add_polyline = false;
    theApp->ActiveDialogIndex = 1;
    m_ListPoint.ResetContent();
    //m_Listctrlpoint.DeleteAllItems();   
    //CC
    long i;
    long point_vector_size;
    vector<CGLPoint*> gs_points_vector;
    gs_points_vector = GetPointsVector();
    point_vector_size = (long)gs_points_vector.size();
    if(point_vector_size>0){
    for(i=0;i<point_vector_size;i++)
    {  
       m_ListPoint.AddString(gs_points_vector[i]->name.data());
      if(gs_points_vector[i]->highlighted)
         m_ListPoint.SetCurSel(i);
     }
    }
   else 
   return;
  //
   nSel = m_ListPoint.GetCurSel();
  if (nSel !=LB_ERR){
   CGLPoint *gl_point = NULL;
   gl_point = gli_points_vector[nSel];
   m_Pointname = gl_point->name.data();
   if(gl_point) {
    m_Listctrlpoint.DeleteAllItems();
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
   // char Tvalue[1024];
    char Vvalue[1024];
    char Dvalue[1024];
    //long i = 0;
    sprintf(sID,"%s",gl_point->name.data());
    //sprintf(sID,"%lg",gl_point->name); // CC 0772005
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);
    sprintf(Vvalue,"%lg",gl_point->value);
    sprintf(Dvalue,"%lg",gl_point->mesh_density);
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText=sID;
    lvitem.iItem=1;//size of list
    lvitem.iSubItem=0;
    m_Listctrlpoint.InsertItem(&lvitem);
    m_Listctrlpoint.SetItemText(0,1,Xvalue);
    m_Listctrlpoint.SetItemText(0,2,Yvalue);
    m_Listctrlpoint.SetItemText(0,3,Zvalue);
    m_Listctrlpoint.SetItemText(0,4,Vvalue);
    m_Listctrlpoint.SetItemText(0,5,Dvalue);
    }
    }
    else 
      return;
     UpdateData(FALSE);
}
