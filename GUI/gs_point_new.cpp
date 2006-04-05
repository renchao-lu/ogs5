// GSPointnew.cpp : implementation file
//
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_point_new.h"
#include "gs_project.h"
#include "MainFrm.h" //CC

// C++ STL
#include <string>
#include <vector>
using namespace std;

// GeoLib
#include "geo_lib.h"
#include "geo_pnt.h"

// FEMLib
#include "makros.h"
#include "rf_out_new.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// CGSPointnew dialog

IMPLEMENT_DYNAMIC(CGSPointnew, CDialog)
CGSPointnew::CGSPointnew(CWnd* pParent /*=NULL*/)
	: CDialog(CGSPointnew::IDD, pParent)
{
}

CGSPointnew::~CGSPointnew()
{
}

void CGSPointnew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LISTCTRL_POINTNEW, m_Listctrlpointnew);
}


BEGIN_MESSAGE_MAP(CGSPointnew, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnClickedButtonCreatePoint)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemovePoint)
    ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveallPoints)
    ON_BN_CLICKED(IDC_BUTTON_UNSELECTPOINT, OnButtonUnselectpoint)
    ON_BN_CLICKED(IDC_POINT_READ, OnPointRead)
    ON_BN_CLICKED(IDC_POINT_WRITE, OnPointWrite)
    ON_BN_CLICKED(IDC_WRITE_POINT_TEC, OnWritePointTec)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
BOOL CGSPointnew::OnInitDialog() 
{
    CDialog::OnInitDialog();
    CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("X");
    CString         strItem3= _T("Y");
    CString         strItem4= _T("Z");
    CString         strItem5= _T("EPS");
    CString         strItem6= _T("MD");
    CString         strItem7= _T("MAT");
    m_Listctrlpointnew.GetWindowRect(&rect); 
    m_Listctrlpointnew.SetTextBkColor(RGB(153,153,255));
    m_Listctrlpointnew.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_Listctrlpointnew.EnableTrackingToolTips();
    m_Listctrlpointnew.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );
	m_Listctrlpointnew.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/7, 0);
	m_Listctrlpointnew.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/7, 1);
    m_Listctrlpointnew.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/7, 2);
    m_Listctrlpointnew.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/7, 3);
    m_Listctrlpointnew.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/7, 4);
    m_Listctrlpointnew.InsertColumn(5, strItem6, LVCFMT_LEFT,
        rect.Width() * 1/7, 5);
    m_Listctrlpointnew.InsertColumn(6, strItem7, LVCFMT_LEFT,
        rect.Width() * 1/7, 5);
  
    UpdatePointList();

  return TRUE; // return TRUE unless you set the focus to a control	              // EXCEPTION: OCX Property Pages should return FALSE
}
// CGSPointnew message handlers
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnClickedButtonCreatePoint()
{
    // TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnButtonRemovePoint()
{
  POSITION p = m_Listctrlpointnew.GetFirstSelectedItemPosition();
  if(p==NULL) return;
  else{
  long m_iSel = (long)m_Listctrlpointnew.GetNextSelectedItem(p);
  GEORemovePoint(m_iSel);
  UpdatePointList();
  }
 
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnButtonRemoveallPoints()
{
  long point_vector_size;
  long i;
  point_vector_size = (long)gli_points_vector.size();
  for(i = 0;i<point_vector_size;0)
  {
	  GEORemovePoint(i);
	  point_vector_size = (long)gli_points_vector.size();
  }

  m_Listctrlpointnew.DeleteAllItems();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();// CC
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnButtonUnselectpoint()
{
   CGLPoint *gl_point = NULL;

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
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnPointRead()
{
    // TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnPointWrite()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  string path_base = m_gsp->path + m_gsp->base;
  GEOWrite(path_base); 
}
/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnWritePointTec()
{
    // TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC 
**************************************************************************/
void CGSPointnew::OnCancel() 
{
  CDialog::OnCancel();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;

}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC
**************************************************************************/
void CGSPointnew::OnOK() 
{
  CDialog::OnOK();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGSPointnew::UpdatePointList()
{
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    long listip = 0;
    CGLPoint* gl_point = NULL;
    m_Listctrlpointnew.DeleteAllItems();
	vector<CGLPoint*>::iterator p = gli_points_vector.begin();
    while(p!=gli_points_vector.end()) {
    gl_point = *p;
      
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_point->name.data());//CC
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);
    sprintf(EPSvalue,"%lg",gl_point->epsilon);
    sprintf(MDvalue,"%lg",gl_point->mesh_density);
    sprintf(MATvalue,"%d",gl_point->mat);
   
    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list

    lvitem.iSubItem=0;
    
    m_Listctrlpointnew.InsertItem(&lvitem);

    m_Listctrlpointnew.SetItemText(listip,1,Xvalue);
    m_Listctrlpointnew.SetItemText(listip,2,Yvalue);
    m_Listctrlpointnew.SetItemText(listip,3,Zvalue);
    m_Listctrlpointnew.SetItemText(listip,4,EPSvalue);
    m_Listctrlpointnew.SetItemText(listip,5,MDvalue);
    m_Listctrlpointnew.SetItemText(listip,6,MATvalue);
 
    ++p;
    ++listip;
     
    }
  UpdateData(FALSE);
}
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CGSPointnew::AddPointtoList(long size)
{
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    char EPSvalue[1024];
    char MDvalue[1024];
    char MATvalue[1024];
    CGLPoint* gl_point = NULL;
    gl_point = gli_points_vector[size];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%s",gl_point->name.data());//CC
    sprintf(Xvalue,"%lg",gl_point->x);
    sprintf(Yvalue,"%lg",gl_point->y);
    sprintf(Zvalue,"%lg",gl_point->z);
    sprintf(EPSvalue,"%lg",gl_point->epsilon);
    sprintf(MDvalue,"%lg",gl_point->mesh_density);
    sprintf(MATvalue,"%d",gl_point->mat);
   
    lvitem.pszText=sID;
    lvitem.iItem=(int)size;//size of list

    lvitem.iSubItem=0;
    
    m_Listctrlpointnew.InsertItem(&lvitem);

    m_Listctrlpointnew.SetItemText((int)size,1,Xvalue);
    m_Listctrlpointnew.SetItemText((int)size,2,Yvalue);
    m_Listctrlpointnew.SetItemText((int)size,3,Zvalue);
    m_Listctrlpointnew.SetItemText((int)size,4,EPSvalue);
    m_Listctrlpointnew.SetItemText((int)size,5,MDvalue);
    m_Listctrlpointnew.SetItemText((int)size,6,MATvalue);
 
}
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnButtonPointPropertiesUpdate() 
{
  UpdateData(TRUE);
  CGLPoint *gl_point = NULL;
  POSITION p = m_Listctrlpointnew.GetFirstSelectedItemPosition();
  if (p == NULL) return;
  else
  {
  int nSel = m_Listctrlpointnew.GetNextSelectedItem(p);
  int listip = 0;
  vector<CGLPoint*>::iterator pl = gli_points_vector.begin();
  while(pl!=gli_points_vector.end())
  {
  gl_point= *pl;
  string nameID;
  string Xvalue;
  string Yvalue;
  string Zvalue;
  string Tvalue;
  string Vvalue;
  string Dvalue;

  nameID = (string)m_Listctrlpointnew.GetItemText(listip,0);
  Xvalue = (string)m_Listctrlpointnew.GetItemText(listip,1);
  Yvalue = (string)m_Listctrlpointnew.GetItemText(listip,2);
  Zvalue = (string)m_Listctrlpointnew.GetItemText(listip,3);
  Tvalue = (string)m_Listctrlpointnew.GetItemText(listip,4);
  Vvalue = (string)m_Listctrlpointnew.GetItemText(listip,5);
  Dvalue = (string)m_Listctrlpointnew.GetItemText(listip,6);
  //CGLPoint* s_pnt = NULL;
 /* int i = 0;
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
  */
  gl_point->name = nameID;
  gl_point->x = strtod(Xvalue.data(), NULL);
  gl_point->y = strtod(Yvalue.data(), NULL);
  gl_point->z = strtod(Zvalue.data(), NULL);
  gl_point->epsilon = strtod(Tvalue.data(),NULL);
  gl_point->mesh_density = strtod(Vvalue.data(), NULL);
  gl_point->mat = atoi(Dvalue.data());
  gl_point->highlighted = false;
  ++pl;
  ++listip;
  }
  gli_points_vector[nSel]->highlighted = true;
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  UpdateData(FALSE);	
}
/**************************************************************************
GeoLibGUI-Method: set the index for the activate dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/
void CGSPointnew::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CDialog::OnActivate(nState, pWndOther, bMinimized);
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    if(mainframe->m_bIsPointEditorOn == 0)
    mainframe->m_bIsPointEditorOn = 1;
    if(mainframe->m_bIsPointEditorOn)
    mainframe->m_bIsPolylineEditorOn = 0;
   // OnButtonPointPropertiesUpdate();
    UpdatePointList();
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    if (theApp->pPolylinenew->GetSafeHwnd()!=NULL)
    theApp->pPolylinenew->add_polyline = false;
    theApp->ActiveDialogIndex = 1;
     UpdateData(FALSE);
}