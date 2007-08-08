// gs_mat_mp.cpp : implementation file
// 11/2003 OK Implementation
#include "stdafx.h"
#include "afxdisp.h"
#include "afxdlgs.h"
#include"makros.h"
#include "GeoSys.h"
#include "MainFrm.h"
// C++
#include <vector>
#include <cstring>
#include <string>
using namespace std;
// GEOLib
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
// FEMLib
#include "rf_mmp_new.h"
#include ".\gs_mat_mp.h"
#include ".\gs_mat_mp_dataedit.h"
#include "gs_project.h"

#include "tools.h"
#include "rf_out_new.h"
// CMATGroupEditor dialog

IMPLEMENT_DYNAMIC(CMATGroupEditor, CDialog)
CMATGroupEditor::CMATGroupEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CMATGroupEditor::IDD, pParent)
    , m_strMATName(_T(""))
{
  m_iSelectedMMPGroup = -1;
  m_iSelectedGEO = -1;
  dataeditdirect = NULL;
}

CMATGroupEditor::~CMATGroupEditor()
{
  if(dataeditdirect)
  {
    delete dataeditdirect;
    dataeditdirect = NULL;
  }
}

void CMATGroupEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FILENAME, m_fileopen);
    DDX_Control(pDX, IDC_FILENAME, m_sfilename);
    DDX_Text(pDX, IDC_NEWNAME, m_newgroup);
    DDX_Control(pDX, IDC_NEWNAME, m_snewname);
    DDX_Control(pDX, IDC_COMBO_TYPENAMES, m_combo_typenames);
    DDX_Text(pDX, IDC_COMBO_TYPENAMES, m_strDBTypeName);
    DDX_Control(pDX, IDC_LISTCONTROL_MMP, m_LC_MMP);
    DDX_Control(pDX, IDC_COMBO_MAT_PROPERTIES, m_CB_MMP_PROPERTIES);
    DDX_Control(pDX, IDC_LIST_MAT_GEO, m_LB_GEO);
    DDX_Control(pDX, IDC_COMBO_MAT_GEO_TYPE, m_CB_GEO_TYPE);
    DDX_Text(pDX, IDC_EDIT_MAT, m_strMATName);
    DDX_Control(pDX, IDC_LIST_MAT_GEO_TYPE, m_LB_GEO_TYPE);
}

BEGIN_MESSAGE_MAP(CMATGroupEditor, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_GEO2MSH, OnButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_NEW_MATGROUP, OnButtonNewMatGroup)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_MP, OnBnClickedButtonWriteMP)
    ON_BN_CLICKED(IDC_CREATE_FIELDS, OnBnClickedCreateFields)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_TEC, OnBnClickedButtonWriteTec)
    ON_BN_CLICKED(IDC_GETVALUES, OnBnClickedGetvalues)
    ON_BN_CLICKED(IDC_FILEOPEN, OnFileopen)
    ON_BN_CLICKED(IDC_BUTTON_MAT_UPDATE, OnBnClickedMATUpdateButton)
    ON_BN_CLICKED(IDC_GRID_REMOVE_BUTTON, OnBnClickedGridRemoveButton)
    ON_CBN_SELCHANGE(IDC_COMBO_TYPENAMES, OnCbnSelchangeComboMATNames)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCONTROL_MMP, OnLvnItemchangedListcontrolMMP)
    ON_CBN_SELCHANGE(IDC_COMBO_MAT_PROPERTIES, OnCbnSelchangeComboMMPProperties)
    ON_BN_CLICKED(IDC_BUTTON_GEO_MAT, OnBnClickedButtonGeoMat)
    ON_LBN_SELCHANGE(IDC_LIST_MAT_GEO, OnLbnSelchangeListMatGeo)
    ON_CBN_SELCHANGE(IDC_COMBO_MAT_GEO_TYPE, OnCbnSelchangeComboMatGeoType)
    ON_BN_CLICKED(IDC_BUTTON_MAT_GEO_CLEAR, OnBnClickedButtonMatGeoClear)
    ON_LBN_SELCHANGE(IDC_LIST_MAT_GEO_TYPE, OnLbnSelchangeListMatGeoType)
	ON_NOTIFY(NM_CLICK, IDC_LISTCONTROL_MMP, OnClick_LC_MMP)
	ON_NOTIFY(NM_RCLICK, IDC_LISTCONTROL_MMP, OnRclick_LC_MMP)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCONTROL_MMP, OnDblclick_LC_MMP)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LISTCONTROL_MMP, OnKillfocus_LC_MMP)
	ON_COMMAND(ID_MATGROUP_POPUP_DELETE, OnBnClickedGridRemoveButton)
	ON_COMMAND(ID_MATGROUP_POPUP_UPDATE, OnBnClickedMATUpdateButton)
    ON_EN_SETFOCUS(IDC_NEWNAME, OnEnSetfocusNewname)
END_MESSAGE_MAP()

// CMATGroupEditor message handlers

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
01/2004 WW/JG Mat GRID
03/2005 OK/JG ListControl
07/2005 MB LAYER
10/2005 OK GEO data removed
last modified:
**************************************************************************/
BOOL CMATGroupEditor::OnInitDialog()
{
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // Data base
  if(!dataedit_pre){
    m_fileopen = "C:\\";
  }
  if(m_fileopen == ""){//invalid typename, new filename selection
    m_fileopen = "C:\\";
  }
  //----------------------------------------------------------------------
  // GEO data
  m_CB_GEO_TYPE.ResetContent();
  m_CB_GEO_TYPE.InitStorage(6,80);
  m_CB_GEO_TYPE.AddString("POINT");
  m_CB_GEO_TYPE.AddString("POLYLINE");
  m_CB_GEO_TYPE.AddString("SURFACE");
  m_CB_GEO_TYPE.AddString("VOLUME");
  m_CB_GEO_TYPE.AddString("DOMAIN");
  m_CB_GEO_TYPE.AddString("LAYER");
  //----------------------------------------------------------------------
  // MAT data
  dataremove = false;
  switch(mat_type)
  {
    case 0: //MMP
      FillTable();
      break;
    case 1: //MSP
      FillTableMSP();
      break;
    case 2: //MFP
      //MFPFillTable();
      break;
    case 3: //MCP
      //MCPFillTable();
      break;
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->dataedit = false;//zurücksetzen
  mainframe->dataupdate = false;//setzen zu Initialisierung
  mainframe->datanew = false;//zurücksetzen
  m_newgroup = "";
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  if(strlen(m_strDBTypeName) == 0){  
    dataedit_pre = false;
  }
  //----------------------------------------------------------------------
  GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(FALSE);
  GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(FALSE);

  if(dataedit_pre == true){
  GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(TRUE);
  m_combo_typenames.SetCurSel(m_combo_typenames.GetCurSel());
  }
  else{
  GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(FALSE);
  }
  mainframe->m_iSelectedMMPGroup = -1;
  return TRUE;  // return TRUE unless you set the focus to a control
}

//////////////////////////////////////////////////////////////////////////
// MAT Data Base

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
03/2005 JG Implementation
last modified:
**************************************************************************/
void CMATGroupEditor::OnFileopen()
{
    
    if(m_sfilename.GetModify() == TRUE){
      UpdateData(TRUE);
      if(m_fileopen.Find(".xls")== -1 && m_fileopen.Find(".csv")== -1){
        AfxMessageBox("No Valid Input File!");
        return;
      }
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->m_fileopen = m_fileopen;
    }    
    else{
      char szFilters[]="Excel Files (*.xls)|*.xls|Text Files (*.csv)|*.csv|All Files (*.*)|*.*||";
      
      CFileDialog m_ldFile(TRUE, "xls", "*.xls", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, this);
	  if(m_ldFile.DoModal() == IDOK){
	    m_fileopen = m_ldFile.GetPathName();
        if(m_fileopen.Find(".xls")== -1 && m_fileopen.Find(".csv")== -1){
          AfxMessageBox("No Valid Input File!");
          return;
        }
        CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
        mainframe->m_fileopen = m_fileopen;
        UpdateData(FALSE);
      }
    }

    if(m_fileopen.Find(".xls")!= -1){
    //fill typenames from excel into combobox
    Excel_typenames2Combo();
    }
    if(m_fileopen.Find(".csv")!= -1){
    //fill typenames from csv-Textfile into combobox
    CSV_typenames2Combo((string)m_fileopen);
    }

    GetDlgItem(IDC_FILENAME) -> EnableWindow(TRUE);//reactivate after creating a new group
    GetDlgItem(IDC_COMBO_TYPENAMES) -> EnableWindow(TRUE);
    GetDlgItem(IDC_GETVALUES) -> EnableWindow(TRUE);
    GetDlgItem(IDC_CHOOSETEXT) -> EnableWindow(TRUE);

    m_combo_typenames.SetCurSel(0);
    dataedit_pre = true;
    UpdateData(FALSE);//display typenames in combobox

    //OKif(geotypeselection == true){
      GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(TRUE);
    //OIK}

    oExcel.Quit();
}
//////////////////////////////////////////////////////////////////////////
// GEO Types

/**************************************************************************
GUI-Method: 
03/2005 JG Implementation
07/2007 OK 
**************************************************************************/
void CMATGroupEditor::OnBnClickedGetvalues()
{
  if (oExcel.m_lpDispatch != NULL){
    oExcel.Quit();
  }
  GetTypenamefromComboBox();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
//OK
  mainframe->m_iSelectedMMPGroup = m_iSelectedMMPGroup;
  mainframe->dataupdate = true;
//OK
  //--- open DataEdit Dialog ---
  CMATGroupEditorDataEdit dlg;
//OK
  dlg.m_mmp = m_mmp;
  dlg.m_msp = m_msp;
//OK
  if(dlg.DoModal() == IDCANCEL)
  {
    if (oExcel.m_lpDispatch != NULL)
    {
      oExcel.Quit();
    }
    mainframe->dataedit = false;
  }
}

//////////////////////////////////////////////////////////////////////////
// MMP Data

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
11/2003 OK Implementation
01/2004 WW/JG Mat GRID
03/2005 JG EXCEL interface
05/2005 OK Tests
07/2005 JG/MB strings for filenames
10/2005 OK/YD Bugfixes Richards model
10/2005 OK DIMENSION
10/2005 OK Check existing MMP
last modified:
**************************************************************************/
void CMATGroupEditor::OnButtonCreate()
{
  int i;
  EmptyName = false;
  Created = false;
  CGLPolyline *m_polyline=NULL;
  Surface *m_surface=NULL;
  CGLVolume *m_volume = NULL;
  CString strItem;
  dataeditdirect = new CMATGroupEditorDataEdit();
  //----------------------------------------------------------------------
  if (oExcel.m_lpDispatch != NULL){
    oExcel.Quit();
  }
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //----if data have NOT been edited-------------
  if(mainframe->dataedit == false && mainframe->datanew == false) 
  {
    GetTypenamefromComboBox();
    //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    //----excel------------------------------------
    if(mainframe->m_fileopen.Find(".xls")!= -1){
        //CMATGroupEditorDataEdit exceldirect();
        dataeditdirect->ExcelDirect2SafeArray();
        //delete exceldirect;
    }
    //----csv-text---------------------------------
    if(mainframe->m_fileopen.Find(".csv")!= -1){
        //CMATGroupEditorDataEdit csvdirect;
        dataeditdirect->CSVtextDirect2SafeArray();
        //delete csvdirect;
    }
  }
  //---------------------------------------------
  //----if data are new-------------
  if(mainframe->datanew == true)
  {
    if(strlen(m_strDBTypeName) == 0)
    {
      m_strDBTypeName = mainframe->m_strDBTypeName;
    }
  }
  //======================================================================
  // MAT
  //======================================================================
  // MMP
  //----------------------------------------------------------------------
if(mat_type==0)
{
  // Check existing MMP
  for(i=0;i<(int)mmp_vector.size();i++){
    m_mmp = mmp_vector[i];
    if(m_strDBTypeName.Compare(m_mmp->name.data())==0)
    {
      AfxMessageBox(m_strDBTypeName+" already exists.");
      Created = true;
      return;
    }
  }
  //......................................................................
  if(strlen(m_strDBTypeName)){
    EmptyName = false;
    }
  else{
    EmptyName = true;
    }
  //......................................................................
  if((Created)||(EmptyName))
  {
    return;
  }
  //----------------------------------------------------------------------
  m_mmp = new CMediumProperties();
  m_mmp->geo_type_name = m_strGEOTypeName;
  m_mmp->geo_name = m_strGEOName;
  m_mmp->number = (int)mmp_vector.size();
  m_mmp->name = m_strDBTypeName;
  //----------------------------------------------------------------------
  dataeditdirect->SafeArray2MMP(m_mmp);//Copy parameter-data to MMP in CMediumProperties class (edited AND not edited)
  //----------------------------------------------------------------------
  // GEO
  //.....................................................................
  m_polyline = GEOGetPLYByName((string)m_strGEOName);//CC
  if(m_polyline) {
    m_polyline->mat_group = m_mmp->number;
    m_mmp->geo_dimension = 1;
    m_mmp->geo_area = 1.0;
  }
  //.....................................................................
  m_surface = GEOGetSFCByName((string)m_strGEOName);//CC
  if(m_surface) {
    m_surface->mat_group = m_mmp->number;
    string this_mat_group_name((string)m_strMATDbName);
    m_surface->mat_group_name = this_mat_group_name;
    m_mmp->geo_dimension = 2;
    m_mmp->geo_area = 1.0;
  }
  //.....................................................................
  m_volume = GEOGetVOL((string)m_strGEOName);
  if(m_volume) {
    m_volume->mat_group = m_mmp->number;
    string this_mat_group_name((string)m_strMATDbName);
    m_volume->mat_group_name = this_mat_group_name;
    m_mmp->geo_dimension = 3;
  }
  //--------------------------------------------------------------------
  mmp_vector.push_back(m_mmp);
  //--------------------------------------------------------------------
//OK
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp){
    GSPAddMember(m_gsp->base + ".mmp");
  }
}
  //=======================================================================
else if (mat_type==1)
{
    CreateMSP();
    CGSProject* m_gsp = GSPGetMember("gli");
    if(m_gsp&&(int)msp_vector.size()>0)
    {
      GSPAddMember(m_gsp->base + ".msp");
    }
}
  //=======================================================================
  dataeditdirect->EmptytnkwVectors();//Empty typename- and keywordvector
  if(dataeditdirect)
  {
     delete dataeditdirect;
     dataeditdirect = NULL;
  }  
  OnInitDialog();
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
03/2005 JG Implementation
10/2005 OK V2, JG please check
last modified:
**************************************************************************/
//Delete material groups
void CMATGroupEditor::OnBnClickedGridRemoveButton()
{
  //----------------------------------------------------------------------
  // Get selected item
  POSITION p = m_LC_MMP.GetFirstSelectedItemPosition();
  if (p == NULL){
    AfxMessageBox("No Material Group selected for removing!");
  }
  m_iSelectedMMPGroup = m_LC_MMP.GetNextSelectedItem(p);
  //----------------------------------------------------------------------
  // Remove MMP item from MMP vector
  if((m_iSelectedMMPGroup>-1)&&(m_iSelectedMMPGroup<(int)mmp_vector.size())){
    m_mmp = mmp_vector[m_iSelectedMMPGroup];
    delete m_mmp;
    mmp_vector.erase(mmp_vector.begin()+m_iSelectedMMPGroup);
  }
  else{
    AfxMessageBox("Remove failed!");
  }
  //----------------------------------------------------------------------
  // Refresh ListCtrl
  dataremove = TRUE;
  FillTable();
  //----------------------------------------------------------------------
  GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(FALSE);
  GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(FALSE);

  UpdateData(FALSE);
}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
03/2005 OK/JG Implementation
last modified:
**************************************************************************/
/*/TODO JG
void CMATGroupEditor::OnBnClickedButtonMATUpdate()
{
  // Get clicked list row number
  int mmp_selected = 0; 
  // Get corresponding MMP instance
  int mmp_vector_size = (int)mmp_vector.size();
  if(mmp_selected<mmp_vector_size)
    m_mmp = mmp_vector[mmp_selected];

  matrix2MMP();

}*/
/**************************************************************************
GUI-Method: 
11/2003 OK Implementation
10/2005 OK GSP
07/2007 OK MMP/MSP
**************************************************************************/
void CMATGroupEditor::OnBnClickedButtonWriteMP()
{
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(!m_gsp)
    m_gsp = GSPGetMember("msh");
  if(!m_gsp)
    m_gsp = GSPGetMember("pcs");
  if(m_gsp)
  {
    switch(mat_type)
    {
      case 0: MMPWrite(m_gsp->path+m_gsp->base);
              break;
      case 1: MSPWrite(m_gsp->path+m_gsp->base);
              break;
    }
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 MB/OK Implementation
last modified:
**************************************************************************/
void CMATGroupEditor::OnBnClickedCreateFields()
{
  GetHeterogeneousFields();
  //char file_name[80];
  //CRFProcess *m_pcs = NULL;
  //// Get selected items
  //POSITION p = m_LC_MMP.GetFirstSelectedItemPosition();
  //m_iSelectedMMPGroup = m_LC_MMP.GetNextSelectedItem(p);
  //if((m_iSelectedMMPGroup>-1)&&(m_iSelectedMMPGroup<(int)mmp_vector.size())){
  //  m_mmp = mmp_vector[m_iSelectedMMPGroup];
  //}
  //
  ////----------------------------------------------------------------------
  //if(m_mmp){
  //  m_pcs = m_mmp->m_pcs;
  //  //....................................................................
  //  // permeability
  //  if(m_mmp->permeability_file.size()>0){
  //    strcpy(file_name,m_mmp->permeability_file.data());
  //    FctReadHeterogeneousFields(file_name);
  //  }
  //  //....................................................................
  //  // porosity
  //  if(m_mmp->porosity_file.size()>0){
  //    strcpy(file_name,m_mmp->porosity_file.data());
  //    FctReadHeterogeneousFields(file_name);
  //  }
  //}
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation for prism based volumes
06/2005 MB Heterogeneous fields
last modified:
**************************************************************************/
void CMATGroupEditor::OnBnClickedButtonWriteTec()
{
  int m,j;
  long i;
  long no_mmp_elements;
  long* nodes = NULL;
  double x,y,z;
  ios::pos_type position;
  CGLPoint m_point;
  CGLVolume* m_vol = NULL;
  int no_mmp = (int)mmp_vector.size();
  CRFProcess *m_pcs = NULL;
  int layer = 0;
  CElem* m_ele = NULL;
  int temp;
  temp = 1; // 1 für Het Fields, 2 alt 
  for(m=0;m<no_mmp;m++){
    m_mmp = mmp_vector[m];
    m_pcs = m_mmp->m_pcs;
    no_mmp_elements = 0;
    //--------------------------------------------------------------------
    // File handling
    string mat_file_name = m_mmp->name + TEC_FILE_EXTENSION;
    fstream mat_file (mat_file_name.data(),ios::trunc|ios::out);
    mat_file.setf(ios::scientific,ios::floatfield);
    mat_file.precision(12);
    if (!mat_file.good()) return;
    mat_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    if (temp ==1){

      long NumberOfElements = (long)m_pcs->m_msh->ele_vector.size();
      long NumberOfNodes = (long)m_pcs->m_msh->nod_vector.size();
      int EleStart = -1;
      int EleEnd = -1;
      int NodeStart = -1;
      int NodeEnd = -1;
      int NumberOfLayers = m_pcs->m_msh->no_msh_layer;
      long NumberOfElementsPerLayer = NumberOfElements / NumberOfLayers;
      long NumberOfNodesPerLayer = NumberOfNodes / (NumberOfLayers + 1);
      double DeltaZ = 0.;

      
      //Schreiben von Elementdaten
      //------------------------------------------------------------------
      //Schichtbezogen
      if(m_mmp->geo_type_name.compare("LAYER") == 0){
    
        char* temp = strdup(m_mmp->geo_name.c_str());
        layer = atoi(temp);
        EleStart = (layer - 1) * NumberOfElementsPerLayer;
        EleEnd = layer * NumberOfElementsPerLayer;
        NodeStart = (layer - 1) * NumberOfNodesPerLayer;
        NodeEnd = (layer + 1) * NumberOfNodesPerLayer;
        //for spacing between layers
        DeltaZ = 10. * (layer -1);
      
      }
      //Gesamtes Netz
      if(m_mmp->geo_type_name.compare("DOMAIN") == 0){
    
        layer = 1;
        EleStart = 0;
        EleEnd = NumberOfElementsPerLayer;
        NodeStart = 0;
        NodeEnd = 2 * NumberOfNodesPerLayer;        
      }

      //------------------------------------------------------------------
      // Write Header
      int NumberHetVariables = 0;
      mat_file << "VARIABLES = X,Y,Z";
      // permeabilty
      if(m_mmp->permeability_file.size() > 0) {
        mat_file << " ,Permeability";
        NumberHetVariables ++;
      }
      // porosity      
      if(m_mmp->porosity_file.size() > 0) {
        mat_file << " ,Porosity";
        NumberHetVariables ++;
      }
      mat_file << endl;
      mat_file << "ZONE";
      mat_file << ", N=" << NumberOfNodesPerLayer*2;
      mat_file << ", E=" << NumberOfElementsPerLayer;
      mat_file << ", DATAPACKING=BLOCK"; //necessary for cellcentered data in Tecplot10
      // which variables are cellcentered
      if(NumberHetVariables > 0){
        mat_file << ", VARLOCATION=("; 
        for(j=0;j<NumberHetVariables;j++) {
          mat_file << j+4 <<"=CELLCENTERED ";
        }
        mat_file << ")"; 
      } 
      mat_file << ", ZONETYPE=FEBRICK";
      mat_file << endl;
   
      //--------------------------------------------------------------------
      // Write Node data
      int count = 0;
      for(i=NodeStart;i<NodeEnd;i++){
        double x = m_pcs->m_msh->nod_vector[i]->X();
        mat_file << x  << " ";
        count++;
        if (count == 20){ //Tecplot does not read in very long lines
          mat_file << endl;
          count = 0;
        }
      }
      count = 0;
      mat_file << endl;
      for(i=NodeStart;i<NodeEnd;i++){
        double y = m_pcs->m_msh->nod_vector[i]->Y();
        mat_file << y << " ";
        count++;
        if (count == 20){
          mat_file << endl;
          count = 0;
        }
      }
      count = 0;
      mat_file << endl;
      for(i=NodeStart;i<NodeEnd;i++){
        double z = m_pcs->m_msh->nod_vector[i]->Z() - DeltaZ;
        mat_file << z << " ";
        count++;
        if (count == 20){
          mat_file << endl;
          count = 0;
        }
      }
      mat_file << endl;
      double test;
      //--------------------------------------------------------------------
      // Write Element Variable data
      for(j=0;j<NumberHetVariables;j++) {
        
        count = 0;
        for(i=EleStart;i<EleEnd;i++){
          m_ele = m_pcs->m_msh->ele_vector[i];
          test = m_ele->mat_vector(j);
          mat_file << m_ele->mat_vector(j) << " ";
          count++;
          if (count == 20){
            mat_file << endl;
            count = 0;
          }
        }
        mat_file << endl;
      }
      //--------------------------------------------------------------------
      // Write Element Nodes
      /*  for(i=EleStart;i<EleEnd;i++){
            m_ele = m_pcs->m_msh->ele_vector[i];
            mat_file
                << m_ele->nodes[0]+1 << " " << m_ele->nodes[0]+1 << " " << m_ele->nodes[1]+1 << " " << m_ele->nodes[2]+1 << " " \
                << m_ele->nodes[3]+1 << " " << m_ele->nodes[3]+1 << " " << m_ele->nodes[4]+1 << " " << m_ele->nodes[5]+1 << endl;
        }*/

      for(i=EleStart;i<EleEnd;i++){
        m_ele = m_pcs->m_msh->ele_vector[i];
        switch(m_ele->GetElementType()){
          case 3: //HEx
            mat_file
            << m_ele->nodes_index[0]+1 - (layer -1)* NumberOfNodesPerLayer << " " << m_ele->nodes_index[1]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[2]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[3]+1 - (layer -1) * NumberOfNodesPerLayer << " " \
            << m_ele->nodes_index[4]+1 - (layer -1)* NumberOfNodesPerLayer << " " << m_ele->nodes_index[5]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[6]+1 -(layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[7]+1 - (layer -1) * NumberOfNodesPerLayer << endl;
          break;
          case 6: //Pris
            mat_file
            << m_ele->nodes_index[0]+1 - (layer -1)* NumberOfNodesPerLayer << " " << m_ele->nodes_index[0]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[1]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[2]+1 - (layer -1) * NumberOfNodesPerLayer << " " \
            << m_ele->nodes_index[3]+1 - (layer -1)* NumberOfNodesPerLayer << " " << m_ele->nodes_index[3]+1 - (layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[4]+1 -(layer -1) * NumberOfNodesPerLayer << " " << m_ele->nodes_index[5]+1 - (layer -1) * NumberOfNodesPerLayer << endl;
          break;
        }
      }
       
   } //endif temp =1
    
    else{
    //---------------------------------------------------------------------
    switch(m_mmp->geo_dimension){
      case 3:
        if(m_mmp->geo_type_name.compare("VOLUME")==0)
          m_vol = GEOGetVOL(m_mmp->geo_name);// CC 10/05
        if(!m_vol)
          return;
        //................................................................
        // Header
        mat_file << "VARIABLES = X,Y,Z,MAT" << endl;
      
        long no_nodes = 0;
        if (m_pcs->m_msh){
          no_nodes = (long)m_pcs->m_msh->nod_vector.size();
        }
        else {
          no_nodes = NodeListLength;
        }

        mat_file << "ZONE T = " << m_mmp->name << ", " \
                 << "N = " << no_nodes << ", " \
                 << "E = " << no_nodes << ", " \
                 << "F = FEPOINT" << ", " << "ET = BRICK" << endl;
        //................................................................
        // Node data
        for(i=0;i<no_nodes;i++){
          if (m_pcs->m_msh){
            //m_ele = m_msh->ele_vector[i];
            double x = m_pcs->m_msh->nod_vector[i]->X();
            double y = m_pcs->m_msh->nod_vector[i]->Y();
            double z = m_pcs->m_msh->nod_vector[i]->Z();
             mat_file \
              << x << " " << y << " " << z \
              << " " << m << endl;
          }
          else{
            mat_file \
              << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) \
              << " " << m << endl;
          }
        }
        //................................................................
        // Element data
        for(i=0;i<ElementListLength;i++) {
          if(ElGetElementType(i)==6) {
            // Element center point
            nodes = ElGetElementNodes(i);
            x=0.0; y=0.0; z=0.0;
            for(j=0;j<6;j++) {
              x += GetNodeX(nodes[j]);
              y += GetNodeY(nodes[j]);
              z += GetNodeZ(nodes[j]);
            }
            x /= double(6);
            y /= double(6);
            z /= double(6);
            m_point.x = x;
            m_point.y = y;
            m_point.z = z;
            if(m_vol->PointInVolume(&m_point,0)){
              mat_file \
                << nodes[0]+1 << " " << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << " " \
                << nodes[3]+1 << " " << nodes[3]+1 << " " << nodes[4]+1 << " " << nodes[5]+1 << endl;
              no_mmp_elements++;
            }
          }
        } // Element data
        mat_file.seekg(position,ios::beg);
        mat_file << "VARIABLES = X,Y,Z,MAT" << endl;
        mat_file << "ZONE T = " << m_mmp->name << ", " \
                 << "N = " << no_nodes << ", " \
                 << "E = " << no_mmp_elements << ", " \
                 << "F = FEPOINT" << ", " << "ET = BRICK" << endl;
    }
    //--------------------------------------------------------------------
   }//end else temp !=1   

 } // mmp_vector
 // delete out;
}

void CMATGroupEditor::GetTypenamefromComboBox(void)
{
  UpdateData(TRUE);//get Typename from Combobox

  m_iSiteSelected = m_combo_typenames.GetCurSel();
  if(m_iSiteSelected<0){return;}
  m_combo_typenames.GetLBText( m_iSiteSelected, m_strDBTypeName );

  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_strDBTypeName = m_strDBTypeName;
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
05/2003 OK Implementation
last modified:
**************************************************************************/
void CMATGroupEditor::OnCbnSelchangeComboMATNames()
{
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->dataedit = false;
    GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(TRUE);
}


/**************************************************************************
GUI-Method: 
07/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnLvnItemchangedListcontrolMMP(NMHDR *pNMHDR, LRESULT *pResult)
{
  CString m_str;
  int i;
  //-----------------------------------------------------------------------
  if(0>=m_LC_MMP.GetSelectedCount())
  {
    GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(FALSE);
    GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(FALSE);
  }
  else
  {
    GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(TRUE);
    GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(TRUE);
  }
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  // TODO: Add your control notification handler code here
  pResult = pResult;
  POSITION p = m_LC_MMP.GetFirstSelectedItemPosition();
  int mat_group = m_LC_MMP.GetNextSelectedItem(p);
  mat_group = pNMLV->iItem;
  m_CB_MMP_PROPERTIES.ResetContent();
  m_CB_MMP_PROPERTIES.InitStorage(20,80);
  //=======================================================================
  // MAT types
  //-----------------------------------------------------------------------
  switch(mat_type)
  {
    case 0: // MMP
      m_mmp = NULL;
      if((mat_group>-1)&&(mat_group<(int)mmp_vector.size()))
      {
        m_mmp = mmp_vector[mat_group];
        m_iSelectedMMPGroup = mat_group;
        m_strMATName = m_mmp->name.data();
      }
      else
        AfxMessageBox("no MMP data");
      //...................................................................
      if(m_mmp)
      {
        if(m_mmp->porosity_model>-1)
          m_CB_MMP_PROPERTIES.AddString("POROSITY");
        if(m_mmp->conductivity_model>-1)
          m_CB_MMP_PROPERTIES.AddString("CONDUCTIVITY");
        if(m_mmp->storage_model>-1)
          m_CB_MMP_PROPERTIES.AddString("STORAGE");
        if(m_mmp->permeability_tensor_type>-1)
          m_CB_MMP_PROPERTIES.AddString("PERMEABILITY_TENSOR");
        if(m_mmp->permeability_saturation_model[0]>-1)
          m_CB_MMP_PROPERTIES.AddString("PERMEABILITY_SATURATION");
        if(m_mmp->capillary_pressure_model>-1)
          m_CB_MMP_PROPERTIES.AddString("CAPILLARY_PRESSURE");
        if(m_mmp->heat_dispersion_model>-1)
          m_CB_MMP_PROPERTIES.AddString("HEAT_DISPERSION");
        if(m_mmp->mass_dispersion_model>-1)
          m_CB_MMP_PROPERTIES.AddString("MASS_DISPERSION");
        if(m_mmp->friction_coefficient>-1)
          m_CB_MMP_PROPERTIES.AddString("DARCY_WEISBACH_COEFFICIENT");
        if(m_mmp->friction_coefficient>-1)
          m_CB_MMP_PROPERTIES.AddString("CHEZY_COEFFICIENT");
        if(m_mmp->friction_coefficient>-1)
         m_CB_MMP_PROPERTIES.AddString("MANNING_COEFFICIENT");
      }
      m_CB_MMP_PROPERTIES.SetCurSel(0);
      //...................................................................
      m_LB_GEO.ResetContent();
      for(i=0;i<(int)m_mmp->geo_name_vector.size();i++)
      {
        m_str = m_mmp->geo_type_name.data();
        m_str += ": ";
        m_str += m_mmp->geo_name_vector[i].data();
        m_LB_GEO.AddString(m_str);
      }
      break;
      //-------------------------------------------------------------------
    case 1: // MSP
      m_msp = NULL;
      if((mat_group>-1)&&(mat_group<(int)msp_vector.size()))
      {
        m_msp = msp_vector[mat_group];
        m_iSelectedMMPGroup = mat_group;
        m_strMATName = m_msp->name.data();
      }
      else
        AfxMessageBox("no MSP data");
      //...................................................................
      for(i=0;i<(int)msp_key_word_vector.size();i++)
      {
        m_str = msp_key_word_vector[i].data();
        m_CB_MMP_PROPERTIES.AddString(m_str);
      }
      break;
  }
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
last modified:
**************************************************************************/
void CMATGroupEditor::OnCbnSelchangeComboMMPProperties()
{
  m_CB_MMP_PROPERTIES.GetLBText(m_CB_MMP_PROPERTIES.GetCurSel(),m_strMMPPropertyName);
//OK  POSITION p = m_LC_MMP.GetFirstSelectedItemPosition();
//OK  int m_mmp_group = m_LC_MMP.GetNextSelectedItem(p);
}

/*
  geotypeselection = true;
  if(dataedit_pre){
  GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(TRUE);
  */
/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
03/2006 CC geo_dimension
**************************************************************************/
void CMATGroupEditor::OnBnClickedButtonGeoMat()
{
  //----------------------------------------------------------------------
  // Tests
  if(m_strGEOName.IsEmpty()) {
    AfxMessageBox("No GEO selected");
    return;
  }
  CString m_str;
  for(int i=0;i<m_LB_GEO.GetCount();i++){
    m_LB_GEO.GetText(i,m_str);
    if(m_str.Compare(m_strGEOName)==0){
      AfxMessageBox("GEO already selected");
      return;
    }
  }
  //----------------------------------------------------------------------
  m_str = m_strGEOTypeName + ": " + m_strGEOName;
  m_LB_GEO.AddString(m_str);
  //----------------------------------------------------------------------
  m_mmp = mmp_vector[m_iSelectedMMPGroup];
  m_mmp->geo_type_name = m_strGEOTypeName;
  if(m_strGEOTypeName.Compare("POLYLINE")==0) //CC4304
     m_mmp->geo_dimension = 1;
  if(m_strGEOTypeName.Compare("SURFACE")==0)  //CC4304
     m_mmp->geo_dimension = 2;
  if(m_strGEOTypeName.Compare("VOLUME")==0)  //CC4304
     m_mmp->geo_dimension = 3;
  if(m_strGEOTypeName.Compare("DOMAIN")==0)  //CC4304
     m_mmp->geo_dimension = 3;
  m_mmp->geo_name_vector.push_back((string)m_strGEOName);
}

/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnLbnSelchangeListMatGeo()
{
  //m_LB_GEO.GetLBText(m_LB_GEO.GetCurSel(),m_strGEOName);
  m_iSelectedGEO = m_LB_GEO.GetCurSel();
}

/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
nFlags = nFlags;
nRepCnt = nRepCnt;
  if(nChar==VK_DELETE){
    m_mmp = mmp_vector[m_iSelectedMMPGroup];
    if(!m_mmp)
      return;
    if(m_iSelectedGEO>0){
      m_mmp->geo_name_vector.erase(m_mmp->geo_name_vector.begin()+m_iSelectedGEO);
      m_LB_GEO.DeleteString(m_iSelectedGEO);
    }
  }
}

/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnCbnSelchangeComboMatGeoType()
{
  m_CB_GEO_TYPE.GetLBText(m_CB_GEO_TYPE.GetCurSel(),m_strGEOTypeName);
  //......................................................................
  m_LB_GEO_TYPE.ResetContent();
  if(m_strGEOTypeName.Compare("POLYLINE")==0){
    CGLPolyline *m_ply = NULL;
    vector<CGLPolyline*>::const_iterator p_ply = polyline_vector.begin();
    while(p_ply!=polyline_vector.end()) {
      m_ply = *p_ply;
      m_LB_GEO_TYPE.AddString((CString)m_ply->name.c_str());
      ++p_ply;
      m_LB_GEO_TYPE.SetCurSel(0);
    }
    if(m_mmp){
      m_mmp->geo_type_name = "POLYLINE";
      m_mmp->geo_dimension = 1;//CC4304
    } 
  }
  //......................................................................
  if(m_strGEOTypeName.Compare("SURFACE")==0){
    Surface *m_sfc = NULL;
    vector<Surface*>::const_iterator p_sfc = surface_vector.begin();
    while(p_sfc!=surface_vector.end()) {
      m_sfc = *p_sfc;
      m_LB_GEO_TYPE.AddString((CString)m_sfc->name.data());
      ++p_sfc;
      m_LB_GEO_TYPE.SetCurSel(0); 
    }  
    if(m_mmp){
      m_mmp->geo_type_name = "SURFACE";
      m_mmp->geo_dimension = 2;//CC4304
    } 
  }
  //......................................................................
  if(m_strGEOTypeName.Compare("VOLUME")==0){
    CGLVolume *m_vol = NULL;
    vector<CGLVolume*>::const_iterator p_vol = volume_vector.begin();
    while(p_vol!=volume_vector.end()) {
      m_vol = *p_vol;
      m_LB_GEO_TYPE.AddString((CString)m_vol->name.data());
      ++p_vol;
      m_LB_GEO_TYPE.SetCurSel(0);
    }  
    if(m_mmp){
      m_mmp->geo_type_name = "VOLUME";
      m_mmp->geo_dimension = 3;//CC4304
    }
  }
  //......................................................................
  if(m_strGEOTypeName.Compare("DOMAIN")==0){
    if(m_mmp){
      m_mmp->geo_type_name = "DOMAIN";
      m_mmp->geo_dimension = 3;//CC4304
    }
  }
  //......................................................................
  if(m_strGEOTypeName.Compare("LAYER")==0){
    if(m_mmp)
      m_mmp->geo_type_name = "LAYER";
  }
  //......................................................................
}

/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnBnClickedButtonMatGeoClear()
{
  if(m_iSelectedMMPGroup<0)
    return;
  if(m_iSelectedMMPGroup>(int)mmp_vector.size())
    return;
  m_mmp = mmp_vector[m_iSelectedMMPGroup];
  m_mmp->geo_name_vector.clear();
  m_LB_GEO.ResetContent();
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnLbnSelchangeListMatGeoType()
{
  m_LB_GEO_TYPE.GetText(m_LB_GEO_TYPE.GetCurSel(),m_strGEOName);
  CGLPolyline *m_ply = NULL;
  GEOUnselectPLY();
  if(m_strGEOTypeName.Compare("POLYLINE")==0){
    m_ply = GEOGetPLYByName((string)m_strGEOName);
    if(m_ply)
      m_ply->highlighted = true;
  }
  Surface* m_sfc = NULL;
  GEOUnselectSFC();
  if(m_strGEOTypeName.Compare("SURFACE")==0){
    m_sfc = GEOGetSFCByName((string)m_strGEOName);
    if(m_sfc)
      m_sfc->highlighted = true;
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK/JG Implementation
last modified:
**************************************************************************/
void CMATGroupEditor::FillTable()
{
  //----------------------------------------------------------------------
  // MMP data
  if(dataremove == FALSE){
  CRect rect;
  m_LC_MMP.GetClientRect(&rect);
  m_LC_MMP.SetExtendedStyle (LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
    int nColInterval = rect.Width()/2;
  // create columns
  if(strlen(m_strDBTypeName) == 0){  
      m_LC_MMP.InsertColumn(0,_T("MAT Name"),LVCFMT_LEFT, nColInterval);
      m_LC_MMP.InsertColumn(1,_T("Geo Dimension"),LVCFMT_LEFT, nColInterval);
    }
  }
  m_LC_MMP.DeleteAllItems(); // Delete the current contents
  //......................................................................
  CString strItem;
  CString temp;
  CMediumProperties* m_mmp = NULL;
  LVITEM lvi;
  for(int i=0;i<(int)mmp_vector.size();i++)
  {
    m_mmp = mmp_vector[i];
    lvi.mask =  LVIF_TEXT;
    // 1 column
	strItem.Format("%i",i);
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(m_mmp->name.c_str());
    m_LC_MMP.InsertItem(&lvi);
    // 2 column
    strItem.Format("%i",m_mmp->geo_dimension);
    lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_LC_MMP.SetItem(&lvi);
  }
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::Excel_typenames2Combo()
{
    m_combo_typenames.ResetContent();

    CWorksheets oSheets;
	CWorksheet oSheet;
	CRange oRange, oRangeCols;
	COleVariant covOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);

	if (oExcel.m_lpDispatch == NULL) {
      oExcel.CreateDispatch("Excel.Application");
      if (oExcel.m_lpDispatch == NULL){
        AfxMessageBox("Excel has not started!");
      }
	}
	//Open workbook
    oBooks = oExcel.get_Workbooks();
    oBook = oBooks.Open (m_fileopen, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional, covOptional);
    //Open first sheet
    oSheets = oBook.get_Worksheets();
    oSheet = oSheets.get_Item(COleVariant((short)1));//first worksheet!!
    //get all data from sheet
    oRange = oSheet.get_UsedRange();

    COleSafeArray saTN(oRange.get_Value());

    //long TNRows;
    long TNCols;
    //saTN.GetUBound(1, &TNRows);//number of rows
    saTN.GetUBound(2, &TNCols);//number of columns
    long index[2];//1 based index for retrieving data from saTN
    //typenames into combobox
    CString tnstr;
    int rowCounter = 1;
	    for (int colCounter = 2; colCounter <= TNCols; colCounter++) {
	        index[0]=rowCounter;
	        index[1]=colCounter;
	        COleVariant vData;
	        saTN.GetElement(index,vData);

            if(vData.vt == VT_BSTR){ 
                tnstr = vData.bstrVal;
		        m_combo_typenames.AddString(tnstr);
            } 
            else{ 
            AfxMessageBox("Invalid Typename found!");
            return;
           }
	    }
    oExcel.Quit();
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::CSV_typenames2Combo(string csv_file_name)
{
  string in;
  string line;
  string z_rest;
  string type_name;
  string type_name_tmp("TYPE_NAME");
  char line_char[MAX_ZEILE];
  string in1;//zwischenstring zum abschneiden der einheit
  string delimiter(";");
  m_combo_typenames.ResetContent();
  ifstream eingabe(csv_file_name.data(),ios::in);
  if (eingabe.good()) {
    eingabe.seekg(0L,ios::beg);//rewind um materialgruppen auszulesen
	eingabe.getline(line_char, MAX_ZEILE);
    line = line_char;
    //typenames auslesen 
    if(line.find(delimiter)!=string::npos) {
      in = line.substr(1);

      while(!type_name_tmp.empty()) {
         type_name_tmp = readCSV_type_name(in,&z_rest);
        if(type_name_tmp.empty()) 
          break; 
        else {
          type_name = type_name_tmp;
          m_combo_typenames.AddString(type_name.c_str());
          in = z_rest;
        }
      }
    }
  }
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
string CMATGroupEditor::readCSV_type_name(string in, string *z_rest_out)
{
  string mat_name;
  string z_rest;
  string delimiter(";");
  if(in.find_first_not_of(delimiter)!=string::npos)//if matrial group is found
  {
    z_rest = in.substr(in.find_first_not_of(delimiter));
    mat_name = z_rest.substr(0,z_rest.find_first_of(delimiter)); //string for name of material group
    *z_rest_out = z_rest.substr(mat_name.length());
	return mat_name;
  }
  else
    return "";
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnClick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pNMHDR = pNMHDR;
  // Get the cursor position
  CPoint matgroupCursorPoint = (0, 0);
  GetCursorPos(&matgroupCursorPoint);
  if (0 >= m_LC_MMP.GetSelectedCount())
  {
    return;
  }
  GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(TRUE);
  GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(TRUE);
  UpdateData(FALSE);
  *pResult = 0;
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnRclick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pNMHDR = pNMHDR;
  // Create the pop up menu
  CMenu matgrouppopupMenu;
  matgrouppopupMenu.LoadMenu(IDR_MATGROUP_POPUP); 
  CMenu* pPopupMenu = matgrouppopupMenu.GetSubMenu(0);
  ASSERT(pPopupMenu); 
  // Get the cursor position
  CPoint matgroupCursorPoint = (0, 0);
  GetCursorPos(&matgroupCursorPoint);
  if(0 >= m_LC_MMP.GetSelectedCount())
  {
    return;
  }
  // Track the popup menu
  pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, matgroupCursorPoint.x,\
                            matgroupCursorPoint.y, this);
  *pResult = 0;
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnDblclick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pNMHDR = pNMHDR;
  // Get the cursor position
  CPoint matgroupCursorPoint = (0, 0);
  GetCursorPos(&matgroupCursorPoint);
  if (0 >= m_LC_MMP.GetSelectedCount())
  {
    //No Material Group selected
    //AfxMessageBox("Select a Material Group!");
    return;
  }
  m_iSelectedMMPGroup = m_LC_MMP.GetSelectedCount()-1;
  m_mmp = NULL; //OK
  m_msp = NULL; //OK
// OK
  switch(mat_type)
  {
    case 0: //MMP
      m_mmp = MMPGet((string)m_strDBTypeName); //OK
      m_mmp = mmp_vector[m_iSelectedMMPGroup];
      if(!m_mmp) return;
      break;
    case 1: //MSP
      m_msp = MSPGet((string)m_strDBTypeName); //OK
      m_msp = msp_vector[m_iSelectedMMPGroup];
      if(!m_msp) return;
      break;
  }
OnBnClickedGetvalues();
return;
// OK
  OnBnClickedMATUpdateButton();
  *pResult = 0;
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnKillfocus_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pNMHDR = pNMHDR;
  CWnd* pWndUpdateBut = GetDlgItem(IDC_BUTTON_MAT_UPDATE);
  CWnd* pWndRemoveBut = GetDlgItem(IDC_GRID_REMOVE_BUTTON);
  if(GetFocus() != pWndUpdateBut && GetFocus() != pWndRemoveBut)
  {
    GetDlgItem(IDC_BUTTON_MAT_UPDATE) -> EnableWindow(FALSE);
    GetDlgItem(IDC_GRID_REMOVE_BUTTON) -> EnableWindow(FALSE);
    UpdateData(FALSE);
  }
  *pResult = 0;
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
07/2007 OK MSP
**************************************************************************/
void CMATGroupEditor::OnBnClickedMATUpdateButton() 
{
  //----------------------------------------------------------------------
  // Get selected item
  POSITION p = m_LC_MMP.GetFirstSelectedItemPosition();
  if (p == NULL){
    AfxMessageBox("No Material Group selected!");
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_iSelectedMMPGroup = m_LC_MMP.GetNextSelectedItem(p);
  mainframe->dataupdate = true;
  //--- open DataEdit Dialog ---
  CMATGroupEditorDataEdit dlg;
  switch(mat_type) //OK
  {
    case 0: // MMP
      dlg.m_mmp = m_mmp;
      dlg.m_msp = NULL;
      break;
    case 1: // MSP
      dlg.m_msp = m_msp;
      dlg.m_mmp = NULL;
      break;
  }
  if(dlg.DoModal() == IDCANCEL)
  {
    if (oExcel.m_lpDispatch != NULL)
    {
      oExcel.Quit();
    }
    mainframe->dataupdate = false;
  }
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnButtonNewMatGroup()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

  if(m_snewname.GetModify() == FALSE){
        AfxMessageBox("Enter name for new group!");
        m_snewname.SetFocus();
        return;
  }
  if(m_snewname.GetModify() == TRUE){
      UpdateData(TRUE);
      if(strlen(m_newgroup) == 0){

        AfxMessageBox("Enter name for new group!");
        m_snewname.SetFocus();
        return;
      }
      else{
        // Check existing MMP
        for(int i=0;i<(int)mmp_vector.size();i++){
            m_mmp = mmp_vector[i];
            if(m_newgroup.Compare(m_mmp->name.data())==0){
            AfxMessageBox(m_newgroup+" already exists.");
            m_newgroup = "";
            UpdateData(FALSE);
            m_snewname.SetFocus();
            return;
            }
        }
      }
  }

  m_strDBTypeName = m_newgroup;
  mainframe->m_strDBTypeName = m_strDBTypeName;
  UpdateData(TRUE);

  mainframe->datanew = true;

  //--- open DataEdit Dialog ---
  CMATGroupEditorDataEdit dlg;
    if(dlg.DoModal() == IDCANCEL){
      if (oExcel.m_lpDispatch != NULL){
        oExcel.Quit();
      }
      mainframe->datanew = false;
    }
  GetDlgItem(IDC_BUTTON_GEO2MSH) -> EnableWindow(TRUE);
}

/**************************************************************************
GeoLibGUI-Method: 
01/2006 JG Implementation
**************************************************************************/
void CMATGroupEditor::OnEnSetfocusNewname()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  m_fileopen = "C:\\";
  GetDlgItem(IDC_FILENAME) -> EnableWindow(FALSE);
  m_combo_typenames.ResetContent();
  GetDlgItem(IDC_COMBO_TYPENAMES) -> EnableWindow(FALSE);
  m_strDBTypeName = "";
  mainframe->m_strDBTypeName = m_strDBTypeName;
  GetDlgItem(IDC_GETVALUES) -> EnableWindow(FALSE);
  UpdateData(FALSE);
}

/**************************************************************************
GUI-Method: 
07/2007 OK Implementation
**************************************************************************/
void CMATGroupEditor::OnOK()
{
  CDialog::OnOK();
}

/**************************************************************************
GUI-Method: 
07/2007 OK Implementation
**************************************************************************/
void CMATGroupEditor::CreateMSP()
{
  EmptyName = false;
  Created = false;
  CSolidProperties* m_msp = NULL;
  // Check existing MSP
  for(int i=0;i<(int)msp_vector.size();i++){
    m_msp = msp_vector[i];
    if(m_strDBTypeName.Compare(m_msp->name.data())==0)
    {
      AfxMessageBox(m_strDBTypeName+" already exists.");
      Created = true;
      return;
    }
  }
  //......................................................................
  if(strlen(m_strDBTypeName))
    EmptyName = false;
  else
    EmptyName = true;
  //......................................................................
  if((Created)||(EmptyName))
    return;
  //----------------------------------------------------------------------
  m_msp = new CSolidProperties();
  //m_mmp->geo_type_name = m_strGEOTypeName;
  //m_mmp->geo_name = m_strGEOName;
  //m_msp->number = (int)msp_vector.size();
  m_msp->name = m_strDBTypeName;
  //----------------------------------------------------------------------
  //Copy parameter-data to MMP in CMediumProperties class (edited AND not edited)
  dataeditdirect->SafeArray2MSP(m_msp);
  //----------------------------------------------------------------------
  // GEO ???
  //-----------------------------------------------------------------------
  msp_vector.push_back(m_msp);
  //-----------------------------------------------------------------------
}

/**************************************************************************
GUI-Method: 
07/2007 OK Implementation
**************************************************************************/
void CMATGroupEditor::FillTableMSP()
{
  //-----------------------------------------------------------------------
  // Table header
  if(dataremove == FALSE)
  {
    CRect rect;
    m_LC_MMP.GetClientRect(&rect);
    m_LC_MMP.SetExtendedStyle (LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
    int nColInterval = rect.Width()/2;
    // create columns
    if(strlen(m_strDBTypeName) == 0)
    {  
      m_LC_MMP.InsertColumn(0,_T("MAT Name"),LVCFMT_LEFT, nColInterval);
      m_LC_MMP.InsertColumn(1,_T("Geo Dimension"),LVCFMT_LEFT, nColInterval);
    }
  }
  m_LC_MMP.DeleteAllItems(); // Delete the current contents
  //-----------------------------------------------------------------------
  CString strItem;
  CString temp;
  CSolidProperties* m_msp = NULL;
  LVITEM lvi;
  for(int i=0;i<(int)msp_vector.size();i++)
  {
    m_msp = msp_vector[i];
    lvi.mask =  LVIF_TEXT;
    // 1 column
	strItem.Format("%i",i);
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(m_msp->name.c_str());
    m_LC_MMP.InsertItem(&lvi);
    // 2 column
    //OK strItem.Format("%i",m_msp->geo_dimension);
    lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_LC_MMP.SetItem(&lvi);
  }
}
