// F:\GeoSys3909OKXX\GUI\gs_st_new.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "gs_st_new.h"

// GEOLib
#include "geo_ply.h"
#include "geo_sfc.h"

// FEMLib
#include "makros.h"
//#include "strutils.h" CS106 libraries
#include "rf_pcs.h"
#include "rf_st_new.h"
#include "rf_node.h"

/**************************************************************************
GeoSysGUI-File: 
Task: dialog to source terms
Programing:
11/2003 OK Implementation
**************************************************************************/

// CSourceTerms dialog

IMPLEMENT_DYNAMIC(CSourceTerms, CDialog)
CSourceTerms::CSourceTerms(CWnd* pParent /*=NULL*/)
	: CDialog(CSourceTerms::IDD, pParent)
{
}

CSourceTerms::~CSourceTerms()
{
}

void CSourceTerms::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PCS_TYPE, m_LB_PCSType);
	DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
	DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
	DDX_Control(pDX, IDC_COMBO_DIS_TYPE, m_CB_DISType);
	DDX_Text(pDX, IDC_EDIT_VALUE, m_dValue);
	DDX_Control(pDX, IDC_LIST_TIM, m_LB_TIM);
	DDX_Control(pDX, IDC_GRID_ST, m_table);
}

BEGIN_MESSAGE_MAP(CSourceTerms, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
    ON_LBN_SELCHANGE(IDC_LIST_PCS_TYPE, OnLbnSelchangeListPCSType)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_NUMBER, OnCbnSelchangeComboPCSNumber)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_CBN_SELCHANGE(IDC_COMBO_DIS_TYPE, OnCbnSelchangeComboDISType)
    ON_BN_CLICKED(IDC_BUTTON_BC_CREATE_GROUP, OnBnClickedButtonCreateGroup)
    // Table
	ON_NOTIFY(VGN_GETDISPINFO,IDC_GRID_ST,OnGridGetDispInfo)
	ON_NOTIFY(VGN_CELLACCEPTCURSOR,IDC_GRID_ST,OnCursorOnGridRow)
    ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonWrite)
END_MESSAGE_MAP()

// CSourceTerms message handlers
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
BOOL CSourceTerms::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //---------------------------------------------------------------------------
  // PCS types
  m_LB_PCSType.ResetContent();
  CRFProcess* m_process = NULL;
  int no_processes = (int)pcs_vector.size();
  int i;
  for(i=0;i<no_processes;i++){
    m_process = pcs_vector[i];
    m_LB_PCSType.AddString((CString)m_process->GetPrimaryVName(0));
  }
  //---------------------------------------------------------------------------
  // GEO types
  m_CB_GEOType.ResetContent();
  m_CB_GEOType.InitStorage(4,80);
  m_CB_GEOType.AddString("POINT");
  m_CB_GEOType.AddString("POLYLINE");
  m_CB_GEOType.AddString("SURFACE");
  m_CB_GEOType.AddString("VOLUME");
  m_CB_GEOType.SetCurSel(0);
  //---------------------------------------------------------------------------
  // DID types
  m_CB_DISType.ResetContent();
  m_CB_DISType.InitStorage(2,80);
  m_CB_DISType.AddString("CONSTANT");
  m_CB_DISType.AddString("LINEAR");
  m_CB_DISType.SetCurSel(0);
  //---------------------------------------------------------------------------
  // ST Table
  ConfigNodeDataTable();   
  FillNodeDataTable();

  // Steuerelemente aktualisieren (Daten->Steuerelemente)
  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnLbnSelchangeListPCSType()
{
  m_LB_PCSType.GetText(m_LB_PCSType.GetCurSel(),m_strPCSType);
}

void CSourceTerms::OnCbnSelchangeComboPCSNumber()
{
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnCbnSelchangeComboGEOType()
{
  m_CB_GEOType.GetLBText(m_CB_GEOType.GetCurSel(),m_strGEOType);

  m_LB_GEO.ResetContent();
  //...........................................................................
  // Points
  if(m_strGEOType.Compare("POINT")==0)
  {
    long gli_points_vector_length = (long)gli_points_vector.size();
    long i;
    for(i=0;i<gli_points_vector_length;i++) {
      m_LB_GEO.AddString((CString)gli_points_vector[i]->name.c_str());
    }
  }
  //...........................................................................
  // Polylines
  if(m_strGEOType.Compare("POLYLINE")==0) {
    vector<CGLPolyline*>::iterator p_ply = polyline_vector.begin();//CC
    CGLPolyline *gs_polyline = NULL;
    while(p_ply!=polyline_vector.end()) {
      gs_polyline = *p_ply;
      m_LB_GEO.AddString((CString)gs_polyline->name.c_str());
      ++p_ply;
    }
  }
  //...........................................................................
  // Surfaces
  if(m_strGEOType.Compare("SURFACE")==0) {
    vector<Surface*>::iterator p_sfc = surface_vector.begin();
    Surface *gs_surface = NULL;
    while(p_sfc!=surface_vector.end()) {
      gs_surface = *p_sfc;
      m_LB_GEO.AddString((CString)gs_surface->name.data());
      ++p_sfc;
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnLbnSelchangeListGEO()
{
  m_LB_GEO.GetText(m_LB_GEO.GetCurSel(),m_strGEOName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnCbnSelchangeComboDISType()
{
  m_CB_DISType.GetLBText(m_CB_DISType.GetCurSel(),m_strDISType);
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
10/1999 OK Implementation
02/2004 OK new BCs
**************************************************************************/
void CSourceTerms::OnButtonAdd() 
{
  UpdateData(true);
  if(m_strPCSType.IsEmpty()) {
    AfxMessageBox("No PCS Type selected"); return; 
  }
  if(m_strGEOName.IsEmpty()) {
    AfxMessageBox("No GEO Name selected"); return; 
  }
  if(m_strGEOName.IsEmpty()) {
    AfxMessageBox("No DIS Type selected"); return;
  }
  if(!m_strPCSType.IsEmpty()&&!m_strGEOName.IsEmpty()&&!m_strDISType.IsEmpty()) {
    m_st = new CSourceTerm();
    m_st->pcs_type_name = m_strPCSType;
    m_st->geo_type_name = m_strGEOType;
    m_st->geo_name = m_strGEOName;
    m_st->SetGEOType();
    m_st->dis_type_name = m_strDISType;
    m_st->SetDISType();
    m_st->geo_node_value = atof(m_dValue);
    st_vector.push_back(m_st);
  }
  FillNodeDataTable();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnButtonRemove() 
{
  // 1 remove selected item
  m_st = NULL;
  int count = 0;
  long no_st = (long)st_vector.size();
  long i;
  for(i=0;i<no_st;i++){
    m_st = st_vector[i];
    if(m_st->pcs_type_name.find(m_strPCSType)!=string::npos) {
      if(m_iMarkedRow==count)
        break;
    }
    count++;
  }
  if(m_st) {
    delete m_st;
    //OK st_vector.erase(m_st);
    AfxMessageBox("CSourceTerms::OnButtonRemove() - ToDo");
  }
  // 2 refill table
  FillNodeDataTable();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnBnClickedButtonCreateGroup()
{
  if(m_strPCSType.IsEmpty()) {
    AfxMessageBox("No PCS type selected");
    return;
  }
  CSourceTermGroup *this_st_group = NULL;
  CSourceTermGroup *m_st_group = NULL;
  this_st_group = m_st_group->Get((string)m_strPCSType);
  list<CSourceTermGroup*>::const_iterator p_group = st_group_list.begin();
  while(p_group!=st_group_list.end()) {
    m_st_group = *p_group;
    if(m_st_group==this_st_group) 
      delete  m_st_group;
      st_group_list.remove(m_st_group);
    if(st_group_list.size()==0)
      break;
    ++p_group;
  }
  CRFProcess* m_pcs = NULL;
  m_st_group = new CSourceTermGroup();
  m_pcs = PCSGet(m_st_group->pcs_name);
  m_st_group->Set(m_pcs,0,m_st_group->pcs_pv_name);
  st_group_list.push_back(m_st_group);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
**************************************************************************/
void CSourceTerms::OnBnClickedButtonWrite()
{
  STWrite((string)m_strFileNameBase);
}


/////////////////////////////////////////////////////////////////////////////
// Table - Grid Operator

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
02/2004 WW Implementation
last modfied: 21.04.04 OK
**************************************************************************/
void CSourceTerms::ConfigNodeDataTable() 
{
  //  -----------------  Let's add some columns  --------------  
  m_table.AddColumn(_T(""),100,LVCFMT_LEFT);
  m_table.AddColumn(_T(""),64,LVCFMT_LEFT);
  m_table.AddColumn(_T(""),48,LVCFMT_LEFT);
  m_table.AddColumn(_T(""),80,LVCFMT_LEFT);
  m_table.AddColumn(_T(""),48,LVCFMT_LEFT);
  m_table.AddColumn(_T(""),32,LVCFMT_LEFT);
  //  ---------------  Let's put the grid header into shape  ------------
  pSections = m_table.GetHeader()->GetSections();
  pUpperSection = pSections->GetSection(0);
  pUpperSection->SetCaption(_T("PCS Type"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  pUpperSection = pSections->GetSection(1);
  pUpperSection->SetCaption(_T("GEO Type"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  pUpperSection = pSections->GetSection(2);
  pUpperSection->SetCaption(_T("GEO"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  pUpperSection = pSections->GetSection(3);
  pUpperSection->SetCaption(_T("DIS Type"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  pUpperSection = pSections->GetSection(4);
  pUpperSection->SetCaption(_T("DIS"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  pUpperSection = pSections->GetSection(5);
  pUpperSection->SetCaption(_T("TIM"));
  pUpperSection->SetAlignment(LVCFMT_CENTER);
  m_table.GetHeader()->SynchronizeSections();
  m_table.SetRowSelect(TRUE);
//FillTable();
	m_iMarkedRow=0;
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2004 OK Implementation
last modified:
ToDo: 
void CSourceTerms::FillNodeDataTable(CSourceTerm* m_obj,list<CSourceTerm*>obj_list)
**************************************************************************/
void CSourceTerms::FillNodeDataTable()
{
  CSourceTerm *m_obj = NULL;

  if(m_table_data.GetSize()) 
    m_table_data.RemoveAll();

  long list_length = (long)st_vector.size();
  m_table.SetRowCount(list_length);

  long i;
  for(i=0;i<list_length;i++){
    m_obj = st_vector[i];
    if(m_obj->pcs_type_name.find(m_strPCSType)!=string::npos) {
	  m_table_data.Add(CNodeDataTable(_T(m_obj->pcs_type_name.c_str()),\
                                      _T(m_obj->geo_type_name.c_str()),\
                                      _T(m_obj->geo_name.c_str()),\
                                      _T(m_obj->dis_type_name.c_str()),\
                                      m_obj->geo_node_value,\
                                      "0"));
    }
  }
}

//Grid operator
//  ---
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2004 WW Implementation
last modified:
**************************************************************************/
void CSourceTerms::OnGridGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;

	if (pDispInfo->item.mask & LVIF_TEXT)  {
		switch (pDispInfo->item.nColumn)  {
		case 0:	// PCS Type
			pDispInfo->item.strText 
				= m_table_data[pDispInfo->item.nRow].m_strPCSTypeName;
			break;
		case 1: // GEO Type
			pDispInfo->item.strText 
				= m_table_data[pDispInfo->item.nRow].m_strGEOTypeName;
			break;
		case 2: // GEO Name
			pDispInfo->item.strText 
				= m_table_data[pDispInfo->item.nRow].m_strGEOName;
			break;
		case 3: // DIS Type
			pDispInfo->item.strText 
				= m_table_data[pDispInfo->item.nRow].m_strDISTypeName;
			break;
		case 4: // DIS Value
			pDispInfo->item.strText
				= m_table_data[pDispInfo->item.nRow].m_strDISValue;
			break;
		case 5: // TIM
			pDispInfo->item.strText 
				= m_table_data[pDispInfo->item.nRow].m_strTIMName;
			break;
		default:
			break;
		}
	}
	*pResult = 0;
}

void CSourceTerms::OnCursorOnGridRow(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;
	*pResult = 0;
	
	m_table.CancelEdit();

	if (pDispInfo->item.nRow>0)
      m_iMarkedRow = pDispInfo->item.nRow;
	  
}


