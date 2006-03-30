/**************************************************************************
GeoSys GUI - Object: well table
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"

#include "gs_plot.h"

#include ".\gs_welltable.h"
#include "geo_pnt.h"
////////////////////////////////////////////////


//vector<CFMWell*> fem_wells_vector;

// CListWellTable dialog

IMPLEMENT_DYNAMIC(CListWellTable, CDialog)
//CListWellTable::CListWellTable(CWnd* pParent /*=NULL*/)
//	: CDialog(CListWellTable::IDD, pParent)
CListWellTable::CListWellTable(CWnd* pParent /*=NULL*/)
	: CDialog(CListWellTable::IDD, pParent)
{
  
}

CListWellTable::~CListWellTable()
{
}

void CListWellTable::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_WELL, m_listwell);
}


BEGIN_MESSAGE_MAP(CListWellTable, CDialog)
    ON_BN_CLICKED(IDC_DELETEALL, OnDeleteAll)
    ON_BN_CLICKED(IDC_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedOK)
    ON_BN_CLICKED(IDC_EXPORT, OnBnClickedExport)
END_MESSAGE_MAP()


// CListWellTable message handlers

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
BOOL CListWellTable::OnInitDialog()
{
	
//    m_listwell.DeleteAllItems();
	CRect           rect;
    CString         strItem1= _T("ID");
	CString         strItem2= _T("X");
    CString         strItem3= _T("Y");
    CString         strItem4= _T("Z");
    
    
   
	CDialog::OnInitDialog();  // let the base class do the default work
	
	// insert head colume
    m_listwell.GetWindowRect(&rect);
    
    m_listwell.SetTextBkColor(RGB(153,153,255));
    m_listwell.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_listwell.EnableTrackingToolTips();
    m_listwell.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES);

	m_listwell.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/4, 0);
	m_listwell.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/4, 1);
    m_listwell.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/4, 2);
    m_listwell.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/4, 3);


//insert the list
//
    char sID[1024];
    char Xvalue[1024];
    char Yvalue[1024];
    char Zvalue[1024];
    //char Tvalue[1024];
    //char Vvalue[1024];
    //char Dvalue[1024];
   ///////////////////////////77
    //put the well list in the table

    CGLPoint *m_point = NULL;
    vector<CGLPoint*>::iterator pi = gli_points_vector.begin();
    vector<CGLPoint*>::iterator p = gli_points_vector.begin();
    int i = 0;
    while(pi!=gli_points_vector.end()){
    m_point = *pi;

    
   
    ++i;
   
   ++pi;
    }
    long listip = 0;
    while(p!=gli_points_vector.end()){
      m_point = *p;
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    sprintf(sID,"%ld",m_point->id);
    
    sprintf(Xvalue,"%lg",m_point->x);
    sprintf(Yvalue,"%lg",m_point->y);
    sprintf(Zvalue,"%lg",m_point->z);
   

    lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list

    lvitem.iSubItem=0;
    
    m_listwell.InsertItem(&lvitem);

    m_listwell.SetItemText(listip,1,Xvalue);
    m_listwell.SetItemText(listip,2,Yvalue);

    m_listwell.SetItemText(listip,3,Zvalue);


    
    ++p;
    ++listip;
    }

    	//m_listwell.DeleteAllItems();

	
    return FALSE;  // there is no change in any control focus stuff here.
}
/**************************************************************************
GeoLibGUI-Method: 
Task: delete all the items in the list
Programing:
08/2004 CC Implementation
**************************************************************************/
void CListWellTable::OnDeleteAll()
{
   
  m_listwell.DeleteAllItems();
  long point_vector_size;
  long i;

  point_vector_size = (long)gli_points_vector.size();
 
  //long size = gli_points_vector.size();
  for(i = 0;i<point_vector_size;0)
  {
	  GEORemovePoint(i);//CC
	  point_vector_size = (long)gli_points_vector.size();
  }

 
   //fem_wells_vector.clear();
//todo: delete from the well list: remove
}

/**************************************************************************
GeoLibGUI-Method: OnDelete()
Task: delete the selected item in the list
Programing:
08/2004 CC Implementation
**************************************************************************/
void CListWellTable::OnDelete()
{
    
   POSITION p = m_listwell.GetFirstSelectedItemPosition();
   
   long nSelected = (long)m_listwell.GetNextSelectedItem(p);//CC
	// Do something with item nSelected
   m_listwell.DeleteItem(nSelected);

   // delete wells from list
   GEORemovePoint(nSelected);//CC

}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CListWellTable::OnBnClickedOK()
{
 
   CDialog::OnOK(); 
}
/**************************************************************************
GeoLibGUI-Method: export wells 
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CListWellTable::OnBnClickedExport()
{
     //if (counter ==0){
  CGLPoint * m_point = NULL;
  CGLPoint gs_point;
  out_vector.clear();
  vector<CGLPoint*>::iterator p = gli_points_vector.begin();
  while(p!=gli_points_vector.end())
  {
     m_point = *p;
     gs_point.x = m_point->x;
     gs_point.y = m_point->y;
     gs_point.z = m_point->z;
    COutput *m_out;
     m_out = new COutput;
  //if Point
     m_out->geo_type = 0;
     m_out->geo_type_name = "POINT";
     m_out->msh_node_number = GetNodeNumberClose(gs_point.x,gs_point.y,gs_point.z);
     out_vector.push_back(m_out);
    
    ++p;
  }//end of while

  OUTWriteNODValues((string)pDoc->m_strGSPFileBase,pDoc->m_nodes_elements);
  MessageBox("Wells have been successfully exported!",0,MB_OK);
}