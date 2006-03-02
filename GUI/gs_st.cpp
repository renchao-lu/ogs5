// gs_st.cpp : implementation file
//
/**************************************************************************
GeoSysGUI-File: 
Task: dialog to source terms
Programing:
11/2003 OK Implementation
**************************************************************************/

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_st.h"

// C++ STL
#include <list>
using namespace std;
// GeoLib
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
// FEMLib
#include "rfsousin.h"
#include "rf_st_new.h"
#include "rf_pcs.h"
#include ".\gs_st.h"

enum ST_NAMES {PRESSURE1='P'};

#define ST_NEW

// CGSSourceTerm dialog

IMPLEMENT_DYNAMIC(CGSSourceTerm, CDialog)
CGSSourceTerm::CGSSourceTerm(CWnd* pParent /*=NULL*/)
	: CDialog(CGSSourceTerm::IDD, pParent)
{
  m_dValue = 0.0;
  m_iGeoType = 0;
  m_iDisType = 0;
}

CGSSourceTerm::~CGSSourceTerm()
{
}

void CGSSourceTerm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ST_QUANTITIES, m_LBQuantities);
	DDX_Control(pDX, IDC_LIST_ST_POINTS, m_LBPoints);
	DDX_Control(pDX, IDC_LIST_ST_POLYLINES, m_LBPolylines);
	DDX_Control(pDX, IDC_LIST_ST_SURFACES, m_LBSurfaces);
	DDX_Radio(pDX, IDC_RADIO_ST_GEO_TYPE1, m_iGeoType);
	DDX_Radio(pDX, IDC_RADIO_ST_DIS_TYPE1, m_iDisType);
	DDX_Text(pDX, IDC_EDIT_ST_VALUE, m_dValue);
	DDX_Control(pDX, IDC_LIST_ST_PROPERTIES, m_grid);
}


BEGIN_MESSAGE_MAP(CGSSourceTerm, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_ST_ADD, OnButtonAdd)
    ON_LBN_SELCHANGE(IDC_LIST_ST_QUANTITIES, OnSelchangeListQuantities)
    ON_BN_CLICKED(IDC_RADIO_ST_GEO_TYPE1, OnBnClickedRadioStGeoType1)
    ON_LBN_SELCHANGE(IDC_LIST_ST_POINTS, OnSelchangeListPoints)
    //ON_LBN_SELCHANGE(IDC_LIST_ST_PROPERTIES, OnSelchangeListProperties)
    ON_BN_CLICKED(IDC_BUTTON_BC_UPDATE, OnButtonUpdate)
    ON_BN_CLICKED(IDC_BUTTON_ST_REMOVE, OnBnClickedRemove)
    ON_BN_CLICKED(IDC_BUTTON_ST_REMOVEALL, OnBnClickedRemoveAll)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_LIST_ST_PROPERTIES, OnGridGetDispInfo)
	ON_NOTIFY(VGN_CELLACCEPTCURSOR, IDC_LIST_ST_PROPERTIES, OnCursorOnGridRow)
END_MESSAGE_MAP()


// CGSSourceTerm message handlers

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
BOOL CGSSourceTerm::OnInitDialog() 
{
  CDialog::OnInitDialog();

  //---------------------------------------------------------------------------
  // ST Quantities
  m_LBQuantities.ResetContent();
  CRFProcess* m_process = NULL;
  if (list_processes) {
    list_current_init(list_processes->list_items);
    while ( (m_process = (CRFProcess*) get_list_processes_next()) != NULL ) {
      GetSTName((CString)m_process->GetPrimaryVName(0));
      //m_LBQuantities.AddString((CString)m_process->GetPrimaryVName(0));
      m_LBQuantities.AddString(m_strSTName);
    }
  }
  //---------------------------------------------------------------------------
  // Points
  long i;
  m_LBPoints.ResetContent();
  long gli_point_vector_size = gli_points_vector.size();
  for(i=0;i<gli_point_vector_size;i++) {
    m_LBPoints.AddString((CString)gli_points_vector[i]->pointname);
  }
  //...........................................................................
  // Polylines
  m_LBPolylines.ResetContent();
  list<CGLPolyline*>::const_iterator p = polyline_list.begin();
  CGLPolyline *gs_polyline = NULL;
  while(p!=polyline_list.end()) {
    gs_polyline = *p;
    m_LBPolylines.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
  //...........................................................................
  // Surfaces
  m_LBSurfaces.ResetContent();
  list<Surface*>::const_iterator ps = surface_list.begin();
  Surface *gs_surface = NULL;
  while(ps!=surface_list.end()) {
    gs_surface = *ps;
    m_LBSurfaces.AddString((CString)gs_surface->name.data());
    ++ps;
  }
  //---------------------------------------------------------------------------
  // ST instances properties
  OnInitGrid();   

  /*
  m_LBProperties.ResetContent();
  list<CSourceTerm*>::const_iterator p_st = st_list_new.begin();
  while(p_st!=st_list_new.end()) {
    m_st = *p_st;
    SetPropertyString();
    m_LBProperties.AddString(m_strLBProperties);
    ++p_st;
  }
  */

  //---------------------------------------------------------------------------
  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 WW Implementation
**************************************************************************/
void CGSSourceTerm::OnInitGrid() 
{
	//  ---------------  Required variables  -----------------
	CGridHeaderSections *pSections;
	CGridHeaderSection *pUpperSection;

	//  -----------------  Let's add some columns  --------------  
	m_grid.AddColumn(_T(""), 64, LVCFMT_CENTER);
	m_grid.AddColumn(_T(""), 96, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 96, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 96, LVCFMT_LEFT);

	//  ---------------  Set additional column properties  ----------------  

	//  ---------------  Let's put the grid header into shape  ------------
	pSections = m_grid.GetHeader()->GetSections();
	pUpperSection = pSections->GetSection(0);
	pUpperSection->SetCaption(_T("#"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(1);
	pUpperSection->SetCaption(_T("Process Type"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(2);
	pUpperSection->SetCaption(_T("GEO Type"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(3);
	pUpperSection->SetCaption(_T("Distributions"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	m_grid.GetHeader()->SynchronizeSections();

	m_grid.SetRowSelect(TRUE);

	MarkedRow=0;
    Count = 0;


}

//Grid operator
//  ---
void CGSSourceTerm::OnGridGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    char buffer[128];
	pResult = pResult;
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;

	if (pDispInfo->item.mask & LVIF_TEXT)  {
		switch (pDispInfo->item.nColumn)  {
		case 0:		//  Index
			itoa(m_Data[pDispInfo->item.nRow].index, buffer, 10);
			pDispInfo->item.strText = buffer; 
			break;
		case 1:		//  PCS Type
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strPCSName;
			break;
		case 2:		//  GEO Type
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strGEOName;
			break;
		case 3:		//  DISTR Type
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strDISName;
			break;
		default:
			break;
		}
	}

}

void CGSSourceTerm::OnCursorOnGridRow(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;
	*pResult = 0;
	
	m_grid.CancelEdit();

	if (pDispInfo->item.nRow>0)
      MarkedRow=pDispInfo->item.nRow;
	  
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/1999 OK Implementation
**************************************************************************/
void CGSSourceTerm::OnButtonAdd() 
{
#ifdef ST_NEW
  // 1 - create ST instance
  m_st = new CSourceTerm();
  // 2 - get ST data from dialog
   // PCS type
  m_st->primary_variable = m_strQuantityName;
  m_st->pcs_number = m_iNumber;
  m_st->component= m_iNumber;
   // GEO type
  m_st->geo_type = m_iGeoType;
  m_st->geo_prop_name = GeoTypeName(m_iGeoType);
   // DIS type
  m_st->dis_type = m_iDisType;
  m_st->dis_type_name = DisTypeName(m_iDisType);
  //m_st->dis_prop = DisTypeValues(m_iDisType);
  // 3 - put into ST list
  st_list_new.push_back(m_st);
#else
  //-----------------------------------------------------------------------
  // create OBJ list if necessary
  if (!STListExists())
     CreateSourceSinkList();
  //-----------------------------------------------------------------------
  // new ST instance
  CString m_strList;
  SOURCE_SINK *st = NULL;
  strcpy(quantity_name,m_strQuantityName);
  // 1 - create new instance and insert to list
  st = AddSourceSinkObject(quantity_name);
  // 2 - refresh data elements
  UpdateData(TRUE);
  // 3 - set data
  SetData(st);
#endif
  //-----------------------------------------------------------------------
  // 4 - insert to properties list
 //  SetPropertyString();

    //
  int Size;
  Size = st_list_new.size();
  //SOIL_PROPERTIES *sp = NULL;

  if(Size>0)
  {
    Count = 0;
	m_grid.SetRowCount(Size);
    if(m_Data.GetSize()) m_Data.RemoveAll();
    list<CSourceTerm*>::const_iterator p_st = st_list_new.begin();
    while(p_st!=st_list_new.end()) {
      m_st = *p_st;
      Count++;
      m_Data.Add(SourceList(Count, 
		  _T((CString)m_st->primary_variable.data()),  
		  _T(GeoTypeName(m_st->geo_type) + ":" + (CString)m_st->geo_prop_name.data()),
		  _T((CString)m_st->dis_type_name.data())));
	  ++p_st;
    }
	m_grid.RedrawWindow();
  }

  //TEST WW 
  /*
  long nSel = m_LBProperties.AddString(m_strLBProperties);
  // 5 - mark string in list box
  m_LBProperties.SelectString(nSel,m_strLBProperties);
  */
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2003 OK Implementation
02/2004 WW 
**************************************************************************/
void CGSSourceTerm::OnBnClickedRemove()
{
 
  list<CSourceTerm*>::const_iterator p_st = st_list_new.begin();
  while(p_st!=st_list_new.end()) {
    m_st = *p_st;
    if(m_st->primary_variable.compare(m_strQuantityName)&& \
       m_st->geo_prop_name.compare(m_strGEONameThis))
      st_list_new.remove(*p_st);
    ++p_st;
  }

 int Size = st_list_new.size();

  if(MarkedRow>=0&&MarkedRow<=Size)
  {
    list<CSourceTerm*>::const_iterator p_st = st_list_new.begin();
	Count = 0;
    while(p_st!=st_list_new.end())
    {
      m_st = *p_st;
  	  if(MarkedRow==Count)
		 break;
        // MessageBox(" Group has already been created!\n ");
	  ++p_st;
	  Count++;
    }
    if(m_st) st_list_new.remove(m_st);

	//Fill the data
    Count=0;
    Size = st_list_new.size();
    m_grid.SetRowCount(Size);
	if(m_Data.GetSize()) m_Data.RemoveAll();
    p_st = st_list_new.begin();
    while(p_st!=st_list_new.end()) {
      m_st = *p_st;
      Count++;
      m_Data.Add(SourceList(Count, 
		  _T((CString)m_st->primary_variable.data()),  
		  _T(GeoTypeName(m_st->geo_type) + ":" + (CString)m_st->geo_prop_name.data()),
		  _T((CString)m_st->dis_type_name.data())));
	  ++p_st;
    }
  }
  m_grid.RedrawWindow();

/*TEST WW
#ifdef ST_NEW
  //st_vector_new.erase(st_vector_new.begin(i));
  list<CSourceTerm*>::const_iterator p_st = st_list_new.begin();
  while(p_st!=st_list_new.end()) {
    m_st = *p_st;
    if(m_st->primary_variable.compare(m_strQuantityName)&& \
       m_st->geo_prop_name.compare(m_strGEONameThis))
      st_list_new.remove(*p_st);
    ++p_st;
  }
  OnInitDialog();
#else
 
  // 1 - get cursor position
  int nSel = m_LBProperties.GetCurSel();
  // 2 - delete corresponding object
  strcpy(m_charSTName,m_strSTName);
  DestroySourceSinkObject(nSel,m_charSTName);
  // 3 - delete corresponding string in list box
  m_LBProperties.DeleteString(nSel);
  
#endif */
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::OnBnClickedRemoveAll()
{
#ifdef ST_NEW
  m_grid.SetRowCount(0);
  if(m_Data.GetSize()) m_Data.RemoveAll();

  m_grid.RedrawWindow();
#else
  strcpy(m_charSTName,m_strSTName);
  DestroySourceSinkGroup(m_charSTName);


 //TEST WW m_LBProperties.ResetContent();
#endif
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::OnSelchangeListQuantities() 
{
#ifdef ST_NEW
#else
  SOURCE_SINK *st = NULL;
  //LPTSTR quantity_name = new TCHAR[m_strQuantityName.GetLength()+1];
  //_tcscpy(quantity_name,m_strQuantityName);
  // ST group identification by name
  m_LBQuantities.GetText(m_LBQuantities.GetCurSel(),m_strQuantityName);
  // List all ST group instances
  /*WW TEST
  m_LBProperties.ResetContent();
  sprintf(quantity_name,"%s",m_strQuantityName);
  st = GetSourceSinkGroup(quantity_name,st);
  while(st) {
    SetPropertyString();
    m_LBProperties.AddString(m_strLBProperties);
    st = GetSourceSinkGroup(quantity_name,st);
  }
  */
#endif
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::OnSelchangeListPoints()
{
  m_LBPoints.GetText(m_LBPoints.GetCurSel(),m_strPointName);
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
/* WW TEST
void CGSSourceTerm::OnSelchangeListProperties()
{
  m_LBProperties.GetText(m_LBProperties.GetCurSel(),m_strLBProperties);
#ifdef ST_NEW
#else
  st = NULL;
  // 1 - get cursor position
  int nSel = m_LBProperties.GetCurSel();
  // 2 - get corresponding IC object
  strcpy(quantity_name,m_strQuantityName);
  st = GetSourceSinkObject(nSel,quantity_name);
  // 3 - get data elements
   //3.1 geometric data
   m_iGeoType = 0;
   //3.2 distribution data
   m_iDisType = 0;
   //3.3 value data
  sprintf(value_char,"%g",st->values[0]);
  m_strValue = (CString)value_char;
  m_dValue = st->values[0];
  // 4 - refresh resource elements
  UpdateData(FALSE);
  
#endif
}
*/

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::OnButtonUpdate()
{
#ifdef ST_NEW
#else
  UpdateData(TRUE);
// 1 - get cursor position
//TEST WW  long nSel = m_LBProperties.GetCurSel();
// 2 - get corresponding instance
  strcpy(quantity_name,m_strQuantityName);
  st = GetSourceSinkObject(nSel,quantity_name);
// 3 - refresh data elements
  UpdateData(TRUE);
// 4 - set data
  if(st)
    SetData(st);
  else {
    AfxMessageBox("No property selected !");
    return;
  }
// 5 - get IC object string
  SetPropertyString();
/*TEST WW
// 6 - delete corresponding string in list box
  m_LBProperties.DeleteString(nSel);
// 7 - insert string to list box
  m_LBProperties.InsertString(nSel,m_strLBProperties);
  */
#endif
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::SetData(SOURCE_SINK *st)
{
  CGLPoint *m_point = NULL;  

  switch (m_strQuantityName.GetAt(0)) {
    case PRESSURE1:
      sprintf(st->name,"%s","SOURCE_VOLUME_FLUID_PHASE1");
      break;
  }

  if(st!=NULL) {
    set_sosi_mode(st,0);
    set_sosi_curve(st,0);

    switch(m_iGeoType) {
      case 0: // point
        set_sosi_type(st,1);
        m_point = m_point->GetByName((string)m_strPointName);
        set_sosi_x(st,0,m_point->x);
        set_sosi_y(st,0,m_point->y);
        set_sosi_z(st,0,m_point->z);
        set_sosi_value(st,0,m_dValue);
        break;
      case 1: // polyline
        sprintf(st->polyline_name,m_strPolylineName);
        set_sosi_value(st,0,m_dValue);
        break;
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
CString CGSSourceTerm::GeoTypeName(int type) 
{
  switch(type) {
    case GS_POINT:
      m_strGEOTypeName = "POINT";
      break;
    case GS_POLYLINE:
      m_strGEOTypeName = "POLYLINE";
      break;
    case GS_SURFACE:
      m_strGEOTypeName = "SURFACE";
      break;
    case GS_VOLUME:
      m_strGEOTypeName = "VOLUME";
      break;
  }
  return m_strGEOTypeName;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
CString CGSSourceTerm::DisTypeName(int type) 
{
  switch(type) {
    case CONSTANT:
      m_strDISTypeName = "CONSTANT";
      break;
    case LINEAR:
      m_strDISTypeName = "LINEAR";
      break;
  }
  return m_strDISTypeName;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
double* CGSSourceTerm::DisTypeValues(int type) 
{
  switch(type) {
    case 0: // CONSTANT
      break;
    case 1: // LINEAR
      break;
  }
  return NULL;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
void CGSSourceTerm::GetSTName(CString m_strQuantityName) 
{
  switch (m_strQuantityName.GetAt(0)) {
    case PRESSURE1:
      m_strSTName =  "SOURCE_VOLUME_FLUID_PHASE1";
      break;
  }

}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
11/2003 OK Implementation
**************************************************************************/
#ifdef ST_NEW
void CGSSourceTerm::SetPropertyString(void)
{
  m_strLBProperties =   "Q:" + (CString)m_st->primary_variable.data() \
                  + " | " \
                  + "S:" 
                  + " | " \
                  + "G:" + GeoTypeName(m_st->geo_type) + ":" + (CString)m_st->geo_prop_name.data() \
                  + " | " \
                  + "D:" + (CString)m_st->dis_type_name.data();
}
#else

void CGSSourceTerm::SetPropertyString(SOURCE_SINK *st)
{
  sprintf(value_char,"%g",st->values[0]);
  m_strProperty =   "Q:" + (CString)st->name \
                  + " - " \
                  + "G:" + GeoTypeName(st->type) + ":" + m_strPointName \
                  + " - " \
                  + "D:" + DisTypeName(st->type) + ":" + (CString)value_char;
}
#endif

void CGSSourceTerm::OnBnClickedRadioStGeoType1()
{
    // TODO: Add your control notification handler code here
}




