// dlg_rsm.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "dlg_rsm.h"
#include ".\dlg_rsm.h"
#include "gs_project.h"
#include "geo_strings.h"

vector<CColumn*>column_vector;

// CRegionalSoilModel dialog

IMPLEMENT_DYNAMIC(CRegionalSoilModel, CDialog)
CRegionalSoilModel::CRegionalSoilModel(CWnd* pParent /*=NULL*/)
	: CDialog(CRegionalSoilModel::IDD, pParent)
    , m_iSubDivisions(1)
{
}

CRegionalSoilModel::~CRegionalSoilModel()
{
}

void CRegionalSoilModel::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SFC, m_LC_SFC);
    DDX_Text(pDX, IDC_EDIT1, m_iSubDivisions);
}

BEGIN_MESSAGE_MAP(CRegionalSoilModel, CDialog)
    ON_BN_CLICKED(IDC_READ1, OnBnClickedReadProfileCode)
    ON_BN_CLICKED(IDC_READ2, OnBnClickedReadStructure)
    //ON_BN_CLICKED(IDC_CREATE_PROFILES, OnBnClickedCreateProfiles)
    ON_BN_CLICKED(IDC_CREATE, OnBnClickedCreateMSH)
    ON_BN_CLICKED(IDC_WRITE_MSH, OnBnClickedWriteMSH)
    ON_BN_CLICKED(IDC_WRITE_MMP, OnBnClickedWriteMMP)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedCreateMMP)
END_MESSAGE_MAP()

// CRegionalSoilModel message handlers

/**************************************************************************
GeoLibGUI-Method: 
Programing:
12/2005 OK Implementation
**************************************************************************/
BOOL CRegionalSoilModel::OnInitDialog()
{
  CDialog::OnInitDialog();
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
  m_LC_SFC.InsertColumn(0,"SFC",LVCFMT_LEFT,m_rect_table.Width()*1/4,0);
  m_LC_SFC.DeleteColumn(1);
  m_LC_SFC.InsertColumn(1,"GIS-ID",LVCFMT_LEFT,m_rect_table.Width()*1/4,0);
  m_LC_SFC.DeleteColumn(2);
  m_LC_SFC.InsertColumn(2,"Profile Code",LVCFMT_LEFT,m_rect_table.Width()*1/4,0);
  m_LC_SFC.DeleteColumn(3);
  m_LC_SFC.InsertColumn(3,"Sections",LVCFMT_LEFT,m_rect_table.Width()*1/4,0);
  // Insert data
  UpdateList();
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::UpdateList()
{
  CString m_str;
  string sfc_name;
  Surface* m_sfc = NULL;
  CColumn* m_col = NULL;
  m_LC_SFC.DeleteAllItems();
  //----------------------------------------------------------------------
  for(int i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_sfc->name.data();
    lvitem.iItem = i; //size of list
    lvitem.iSubItem = 0;
    m_LC_SFC.InsertItem(&lvitem);
    m_str.Format("%i",m_sfc->id);
    m_LC_SFC.SetItemText(i,1,m_str);
    m_str.Format("%i",m_sfc->mat_group);
    m_LC_SFC.SetItemText(i,2,m_str);
    //m_col = COLGet(m_sfc->name);
    m_col = COLGet(m_sfc->mat_group);
    if(!m_col)
      continue;
    m_str.Format("%i:",m_col->line_vector.size());
    for(int j=0;j<(int)m_col->line_vector.size();j++){
      m_str += m_col->line_vector[j]->name.data();
      m_str += ",";
    }    
    m_LC_SFC.SetItemText(i,3,m_str);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedReadProfileCode()
{
  int pos1,pos2;
  char line[MAX_ZEILE];
  string sub_string;
  string line_string;
  string delimiter(";");
  int idummy;
  double ddummy;
  double area;
  std::stringstream in;
  Surface* m_sfc = NULL;
  CColumn* m_col = NULL;
  CString m_str;
  //======================================================================
  CFileDialog fileDlg(TRUE,"csv", NULL, OFN_ENABLESIZING," EXCEL files (*.csv)|*.csv|| ");
  if (fileDlg.DoModal()==IDOK) {
    string csv_file_name = fileDlg.GetPathName();
    ifstream csv_file (csv_file_name.data(),ios::in);
    if(!csv_file.good()) 
      return;
    csv_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    // Evaluate header
    csv_file.getline(line,MAX_ZEILE);
    //--------------------------------------------------------------------
    bool ok = true;
    while(ok){
      csv_file.getline(line,MAX_ZEILE);
      line_string = line;
      if(line_string.empty()){
        UpdateList();
        return;
      }
      if(csv_file.eof()){
        UpdateList();
        return;
      }
      if(line_string.find(";;")!=string::npos){
        UpdateList();
        return;
      }
      //..................................................................
      pos1 = 0;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy;
	  in.clear();
      if(idummy>=(int)surface_vector.size()){
        //AfxMessageBox("Error: not enough SFC data");
        //return;
        continue;
      }
      m_sfc = surface_vector[idummy]; //?CC SFCGetByID()
      m_sfc->CalcCenterPoint();
      m_sfc->id = idummy;
      //..................................................................
      // Area
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> area;
	  in.clear();
      //..................................................................
      // Perimeter
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> ddummy;
	  in.clear();
      //..................................................................
      // Influence area number
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy;
	  in.clear();
      //..................................................................
      // Profile code
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      if(sub_string.size()==0){
        AfxMessageBox("Warning: no data");
        continue;
      }
      in.str(sub_string);
      in >> idummy;
	  in.clear();
      m_col = COLGet(idummy);
      if(!m_col){
        m_col = new CColumn();
        m_col->mat_group = idummy;
        m_col->geo_area = area;
        m_col->name = m_sfc->name;
        m_col->center_point[0] = m_sfc->center_point[0];
        m_col->center_point[1] = m_sfc->center_point[1];
        m_col->center_point[2] = m_sfc->center_point[2];
        column_vector.push_back(m_col);
      }
      m_sfc->mat_group = idummy;
      m_str.Format("%i",m_col->mat_group);
      m_sfc->mat_group_name = "PROFILE";
      m_sfc->mat_group_name += m_str;
      //..................................................................
    }
    //--------------------------------------------------------------------
  }
  //======================================================================
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedReadStructure()
{
  int sfc_max_mat_group_number = SFCGetMaxMATGroupNumber();
  if(sfc_max_mat_group_number<0){
    AfxMessageBox("Warning in CRegionalSoilModel::OnBnClickedReadStructure - no SFC-MAT groups");
    return;
  }
  //----------------------------------------------------------------------
  COLDeleteLines();
  //----------------------------------------------------------------------
  int pos1,pos2;
  char line[MAX_ZEILE];
  string sub_string;
  string line_string;
  string delimiter(";");
  int idummy;
  double ddummy;
  string sdummy;
  std::stringstream in;
  CColumn* m_col = NULL;
  CGLLine* m_lin = NULL;
  CGLPoint* m_pnt1 = NULL;
  CGLPoint* m_pnt2 = NULL;
  CMediumProperties* m_mmp = NULL;
  //======================================================================
  CFileDialog fileDlg(TRUE,"csv", NULL, OFN_ENABLESIZING," EXCEL files (*.csv)|*.csv|| ");
  if (fileDlg.DoModal()==IDOK) {
    string csv_file_name = fileDlg.GetPathName();
    ifstream csv_file (csv_file_name.data(),ios::in);
    if(!csv_file.good()) 
      return;
    csv_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    // Evaluate header
    csv_file.getline(line,MAX_ZEILE);
    //--------------------------------------------------------------------
    bool ok = true;
    while(ok){
      csv_file.getline(line,MAX_ZEILE);
      line_string = line;
      if(line_string.empty()){
        UpdateList();
        return;
      }
      if(csv_file.eof()){
        UpdateList();
        return;
      }
      if(line_string.find(";;")!=string::npos){
        UpdateList();
        return;
      }
      //..................................................................
      pos1 = 0;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy; // EXCEL ID
	  in.clear();
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy; // Profile code
	  in.clear();
      m_col = COLGet(idummy);
      if(!m_col){
        //AfxMessageBox("Warning in CRegionalSoilModel::OnBnClickedReadStructure - no COL data");
        //return;
        continue;
      }
      //------------------------------------------------------------------
      m_lin = new CGLLine();
      m_col->line_vector.push_back(m_lin);
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> sdummy; // Soil code name
	  in.clear();
      m_lin->name = sdummy;
      //..................................................................
      m_mmp = MMPGet(m_lin->name);
      if(!m_mmp){
/*
        m_mmp = new CMediumProperties();
        m_mmp->number = (int)mmp_vector.size();
        m_mmp->name = m_lin->name;
        m_mmp->geo_type_name = "LINE";
        m_mmp->geo_dimension = 1;
        m_mmp->geo_area = m_col->geo_area;
        mmp_vector.push_back(m_mmp);
*/
        AfxMessageBox("Warning in CRegionalSoilModel::OnBnClickedReadStructure() - no MMP data");
        continue;
      }
      m_mmp->geo_type_name = "LINE";
      m_mmp->geo_dimension = 1;
      m_mmp->geo_area = m_col->geo_area;
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy; // Soil code number
	  in.clear();
      m_lin->mat_group = idummy;
      //------------------------------------------------------------------
      m_pnt1 = new CGLPoint();
      m_lin->m_point1 = m_pnt1;
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> ddummy; // Depth1
	  in.clear();
      m_pnt1->z = ddummy;
      //------------------------------------------------------------------
      m_pnt2 = new CGLPoint();
      m_lin->m_point2 = m_pnt2;
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> ddummy; // Depth2
	  in.clear();
      m_pnt2->z = ddummy;
      //..................................................................
    }
    //--------------------------------------------------------------------
  }
  //======================================================================
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
/*
void CRegionalSoilModel::OnBnClickedCreateProfiles()
{
  int sfc_max_mat_group_number = SFCGetMaxMATGroupNumber();
  CColumn* m_col = NULL;
  COLDelete();
  for(int i=0;i<sfc_max_mat_group_number;i++){
    m_col = new CColumn();
    column_vector.push_back(m_col);
  }
  //OnBnClickedReadProfileCode();
}
*/

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedCreateMSH()
{
  int i,j,k;
  CGLPoint* m_pnt = NULL;
  CGLLine* m_lin = NULL;
  Surface* m_sfc = NULL;
  CNode* m_nod = NULL;
  Mesh_Group::CElem* m_ele = NULL;
  CFEMesh* m_msh = NULL;
  CColumn* m_col = NULL;
  double dz;
  UpdateData(TRUE);
  //======================================================================
  for(i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    MSHDelete(m_sfc->name);
    if(MSHGet(m_sfc->mat_group_name))
      continue;
    m_msh = new CFEMesh();
    m_msh->pcs_name = m_sfc->name;
    m_msh->ele_type = 1;
    m_msh->geo_name = m_sfc->mat_group_name;
    m_col = COLGet(m_sfc->name);
    if(m_col)
      m_msh->no_msh_layer = (int)m_col->line_vector.size();
    else{
      AfxMessageBox("Warning in CRegionalSoilModel::OnBnClickedCreateMSH - no COL data");
      continue;
    }
    //--------------------------------------------------------------------
    // Nodes
    for(j=0;j<(int)m_col->line_vector.size();j++){
/*
      m_pnt = m_col->line_vector[j]->m_point1;
      m_nod = new CNode(j,m_col->center_point[0],m_col->center_point[1],m_pnt->z);
      m_msh->nod_vector.push_back(m_nod);
*/
      m_lin = m_col->line_vector[j];
      dz = (m_lin->m_point2->z - m_lin->m_point1->z)/(double)m_iSubDivisions;
      for(k=0;k<m_iSubDivisions;k++){
        m_nod = new CNode(j,m_col->center_point[0],m_col->center_point[1],m_lin->m_point1->z+dz*k);
        m_nod->SetIndex((long)m_msh->nod_vector.size());
	    m_msh->nod_vector.push_back(m_nod);
      }
    }
    if(m_col->line_vector.size()>0){
      m_pnt = m_col->line_vector[(int)m_col->line_vector.size()-1]->m_point2;
      m_nod = new CNode(j,m_col->center_point[0],m_col->center_point[1],m_pnt->z);
      m_nod->SetIndex((long)m_msh->nod_vector.size());
      m_msh->nod_vector.push_back(m_nod);
    }
    //--------------------------------------------------------------------
    // Elements
    for(j=0;j<(int)m_col->line_vector.size();j++){
      for(k=0;k<m_iSubDivisions;k++){
        m_ele = new Mesh_Group::CElem;
        m_ele->SetIndex(j*m_iSubDivisions+k);
        m_ele->SetElementType(1);
        m_ele->SetNodesNumber(2);
        m_ele->nodes_index.resize(2);
        m_ele->nodes_index[0] = j*m_iSubDivisions+k;
        m_ele->nodes_index[1] = m_ele->nodes_index[0] + 1;
        m_ele->SetPatchIndex(m_col->line_vector[j]->mat_group);
        m_msh->ele_vector.push_back(m_ele);
      }
    }
    //--------------------------------------------------------------------
    fem_msh_vector.push_back(m_msh);
    //--------------------------------------------------------------------
  }
  //======================================================================
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedWriteMSH()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(m_gsp){
    FEMWrite(m_gsp->path + m_gsp->base);
    GSPAddMember(m_gsp->base + ".msh");
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedWriteMMP()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(m_gsp){
    MMPWrite(m_gsp->path+m_gsp->base);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void CRegionalSoilModel::OnBnClickedCreateMMP()
{
  int pos1,pos2;
  char line[MAX_ZEILE];
  string sub_string;
  string line_string;
  string delimiter(";");
  int idummy;
  string sdummy;
  std::stringstream in;
  CMediumProperties* m_mmp = NULL;
  //======================================================================
  CFileDialog fileDlg(TRUE,"csv", NULL, OFN_ENABLESIZING," EXCEL files (*.csv)|*.csv|| ");
  if (fileDlg.DoModal()==IDOK) {
    string csv_file_name = fileDlg.GetPathName();
    ifstream csv_file (csv_file_name.data(),ios::in);
    if(!csv_file.good()) 
      return;
    csv_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    // Evaluate header
    csv_file.getline(line,MAX_ZEILE);
    //--------------------------------------------------------------------
    bool ok = true;
    while(ok){
      csv_file.getline(line,MAX_ZEILE);
      line_string = line;
      if(line_string.empty())
        return;
      if(csv_file.eof())
        return;
      if(line_string.find(";;")!=string::npos)
        return;
      //..................................................................
      pos1 = 0;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> sdummy; // Soil code name
	  in.clear();
      //..................................................................
      pos1 = pos2+1;
      sub_string = get_sub_string(line_string,delimiter,pos1,&pos2);
      sub_string = line_string.substr(pos1,pos2-pos1);
      in.str(sub_string);
      in >> idummy; // Soil code number
	  in.clear();
      //------------------------------------------------------------------
      if(idummy!=(int)mmp_vector.size()){
        AfxMessageBox("Warning in CRegionalSoilModel::OnBnClickedReadStructure() - incompatible data");
        continue;
      }
      m_mmp = new CMediumProperties();
      m_mmp->number = (int)mmp_vector.size();
      m_mmp->name = sdummy;
      mmp_vector.push_back(m_mmp);
    }
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
CColumn::~CColumn()
{
  for(int i=0;i<(int)line_vector.size();i++){
    m_lin = line_vector[i];
    delete m_lin;
  }
  line_vector.clear();
}

/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void COLDelete()
{
  CColumn* m_col = NULL;
  for(int i=0;i<(int)column_vector.size();i++){
    m_col = column_vector[i];
    delete m_col;
  }
  column_vector.clear();
}
/**************************************************************************
GeoSys-GUI-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
void COLDeleteLines()
{
  int j;
  CColumn* m_col = NULL;
  CGLLine* m_lin = NULL;
  for(int i=0;i<(int)column_vector.size();i++){
    m_col = column_vector[i];
    for(j=0;j<(int)m_col->line_vector.size();j++){
      m_lin = m_col->line_vector[j];
      delete m_lin;
    } 
    m_col->line_vector.clear();
  }
}

/**************************************************************************
GEOLib-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
CColumn* COLGet(int col_id)
{
  CColumn* m_col = NULL;
  for(int i=0;i<(int)column_vector.size();i++){
    m_col = column_vector[i];
    if(m_col->mat_group==col_id)
      return m_col;
  }
  return NULL;
}

/**************************************************************************
GEOLib-Method:
Programing:
12/2005 OK Implementation
**************************************************************************/
CColumn* COLGet(string col_name)
{
  CColumn* m_col = NULL;
  for(int i=0;i<(int)column_vector.size();i++){
    m_col = column_vector[i];
    if(m_col->name.compare(col_name)==0)
      return m_col;
  }
  return m_col;
}

