// F:\GeoSys3908OK\GUI\gs_meshing.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "MainFrm.h"
// this
#include "gs_meshing.h"
// Dialogs
#include "gs_graphics.h"
// GEOLib
#include "geo_ply.h"
// MSHLib
#include "msh_lib.h"
// FEMLib
#include "gs_project.h"
#include "rf_mmp_new.h"
extern void RFPre_Model();

// CMSHRefine dialog

IMPLEMENT_DYNAMIC(CMSHRefine, CDialog)
CMSHRefine::CMSHRefine(CWnd* pParent /*=NULL*/)
	: CDialog(CMSHRefine::IDD, pParent)
{
}

CMSHRefine::~CMSHRefine()
{
}

void CMSHRefine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMSHRefine, CDialog)
END_MESSAGE_MAP()


// CMSHRefine message handlers
// F:\GeoSys3909XX\GUI\gs_meshing.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_meshing.h"
#include ".\gs_meshing.h"

// CMSHPolylines dialog

IMPLEMENT_DYNAMIC(CMSHPolylines, CDialog)
CMSHPolylines::CMSHPolylines(CWnd* pParent /*=NULL*/)
	: CDialog(CMSHPolylines::IDD, pParent)
    , m_iNr(0)
{
  type = 0;
  m_iMeshingType = 1;
}

CMSHPolylines::~CMSHPolylines()
{
}

void CMSHPolylines::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_POLYLINES_GEO, m_LBPolylinesGEO);
    DDX_Control(pDX, IDC_LIST_POLYLINES_MSH, m_LBPolylinesMSH);
    DDX_Radio(pDX, IDC_RADIO1, m_iMeshingType);
    DDX_Control(pDX, IDC_PROGRESS_MESH, m_progress_msh); // CC 04/2005
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Text(pDX, IDC_EDIT1, m_iNr);
}


BEGIN_MESSAGE_MAP(CMSHPolylines, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_GEO2MSH, OnButtonGEO2MSH)
    ON_LBN_SELCHANGE(IDC_LIST_POLYLINES_GEO, OnSelchangeListPolylinesGeo)
    ON_BN_CLICKED(IDC_BUTTON_MSH_EXECUTE, OnButtonMSHExecute)
    ON_LBN_SELCHANGE(IDC_LIST_POLYLINES_MSH, OnLbnSelchangeListPolylinesMSH)
    ON_BN_CLICKED(ID_DELETE_DOUBLE, OnBnClickedDeleteDouble)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH, OnCbnSelchangeComboMSH)
    ON_BN_CLICKED(IDC_BUTTON_MSH_EXECUTE2, OnBnClickedButtonMSHSubdivisionExecute)
END_MESSAGE_MAP()


// CMSHPolylines message handlers
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
04/2005 OK MSH
**************************************************************************/
BOOL CMSHPolylines::OnInitDialog() 
{
  CGLPolyline *gs_polyline=NULL;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // GEO - Fill polyline lists
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC 08/2005
  while(p!=polyline_vector.end()) {
    gs_polyline = *p;
    m_LBPolylinesGEO.AddString((CString)gs_polyline->name.c_str());
    if((type==1)&&(gs_polyline->type==1))
      m_LBPolylinesMSH.AddString((CString)gs_polyline->name.c_str());
    if((type==2)&&(gs_polyline->type==2))
      m_LBPolylinesMSH.AddString((CString)gs_polyline->name.c_str());
    ++p;
  }
  //----------------------------------------------------------------------
  // MSH
  m_CB_MSH.ResetContent();
  CFEMesh* m_msh = NULL;
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_CB_MSH.AddString(m_msh->pcs_name.c_str());
  }
  //----------------------------------------------------------------------
  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
**************************************************************************/
void CMSHPolylines::OnButtonGEO2MSH()
{
  CGLPolyline *gs_polyline = NULL;
  if(m_strNamePolyline.IsEmpty()) {
    AfxMessageBox("No polyline selected !");
    return;
  }
  // Set polyline type for meshing
  gs_polyline = GEOGetPLYByName((string)m_strNamePolyline);//CC
  switch(type) {
    case 1:
      gs_polyline->type = 1;
      break;
    case 2:
      gs_polyline->type = 2;
      break;
  }
  // Display MSH polyline list
  m_LBPolylinesMSH.ResetContent();
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC 08/2005
  while(p!=polyline_vector.end()){
    gs_polyline = *p;
    if(gs_polyline->type==type){
      m_LBPolylinesMSH.AddString((CString)gs_polyline->name.c_str());
    }
    ++p;
  }
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
04/2005 CC Modification Progress bar
**************************************************************************/
void CMSHPolylines::OnSelchangeListPolylinesGeo()
{
  // Get name of selected polyline
  m_LBPolylinesGEO.GetText(m_LBPolylinesGEO.GetCurSel(),m_strNamePolyline);
  m_progress_msh.SetPos(0);
  // Refresh view of data elements
  UpdateData(FALSE);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
02/2004 OK Implementation
04/2004 OK Delete double elements
04/2005 CC Modification progress bar
04/2005 OK MultiMSH extensions
**************************************************************************/
void CMSHPolylines::OnButtonMSHExecute()
{
  UpdateData(TRUE);
  CGLPolyline *m_polyline = NULL;
  //----------------------------------------------------------------------
  if(!m_msh){
    AfxMessageBox("No MSH selected");
    return;
  }
  //----------------------------------------------------------------------
  long no_double_elements;
  int steps = 0;
  steps = m_LBPolylinesMSH.GetCount();
  m_progress_msh.SetRange((short)0,(short)steps);
  m_progress_msh.SetStep(0);
  SetTimer(0,steps,NULL);
  //----------------------------------------------------------------------
  // Lines
  if(type==1) {
    CFEMesh* m_msh_ply = NULL;
    m_msh_ply = new CFEMesh();
    m_msh_ply->pcs_name = "POLYLINES";
    for(int i=0;i<m_LBPolylinesMSH.GetCount();i++){
      m_progress_msh.StepIt();
      UpdateData(FALSE);
      m_LBPolylinesMSH.GetText(i,m_strNamePolyline);
      m_polyline = GEOGetPLYByName((string)m_strNamePolyline);//CC
      if(m_polyline){
        m_msh->CreateLineELEFromPLY(m_polyline,m_iMeshingType,m_msh_ply); // OK recover old method
        no_double_elements = MSHMarkDoubleElementsType(1);
      }
      m_progress_msh.SetPos(i+1);
    }
    if(m_msh_ply->ele_vector.size()>0)
      fem_msh_vector.push_back(m_msh_ply);
    else 
      delete m_msh_ply;
    AfxMessageBox("line elements have been created!");
  }
  //----------------------------------------------------------------------
  // Quads
  if(type==2) {
    for(int i=0;i<m_LBPolylinesMSH.GetCount();i++){
      m_progress_msh.StepIt();
      
      UpdateData(FALSE);
      m_LBPolylinesMSH.GetText(i,m_strNamePolyline);
      m_polyline = GEOGetPLYByName((string)m_strNamePolyline);//CC
      if(m_polyline) {
        MSHCreateQuadsFromPLY(m_polyline,m_iMeshingType);
        no_double_elements = MSHMarkDoubleElementsType(2);
      }
      m_progress_msh.SetPos(i+1);
    }
    AfxMessageBox("Quadrilateral elements have been created!");
  }
  //----------------------------------------------------------------------
  // Delete double elements
 // MSHDeleteDoubleElements(type);  CC 04/2005
}

void CMSHPolylines::OnLbnSelchangeListPolylinesMSH()
{
  m_LBPolylinesMSH.DeleteString(m_LBPolylinesMSH.GetCurSel());
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CMSHPolylines::OnCbnSelchangeComboMSH()
{
  CString m_strMSHName;
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
  m_msh = FEMGet((string)m_strMSHName);
  if(m_msh){
    if(m_msh->selected)
      m_msh->selected = false;
    else
      m_msh->selected = true;
    if(m_msh->selected)
      m_CB_MSH.SetCurSel(m_CB_MSH.GetCurSel());
    else
      m_CB_MSH.SetCurSel(-1);
  }
}

////////////////////////////////////////////////////////////////////////////////
// CMSHEditor dialog

IMPLEMENT_DYNAMIC(CMSHEditor, CDialog)
CMSHEditor::CMSHEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CMSHEditor::IDD, pParent)
    , m_lNoMSHNodes(0)
    , m_strMSHNameNew(_T(""))
    , m_lNoMAT(0)
    , m_dMinEdgeLength(0)
{
  m_bMSHActivateLine = FALSE;
  m_bMSHActivateTri = FALSE;
  m_bMSHActivateQuad = FALSE;
  m_bMSHActivateTet = FALSE;
  m_bMSHActivatePris = FALSE;
  m_bMSHActivateHex = FALSE;
  m_lNumberOfLine = 0;
  m_lNumberOfTris = 0;
  m_lNumberOfQuad = 0;
  m_lNumberOfTets = 0;
  m_lNumberOfPris = 0;
  m_lNumberOfHexs = 0;
}

CMSHEditor::~CMSHEditor()
{
}

void CMSHEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_LINE, m_bMSHActivateLine);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_TRI, m_bMSHActivateTri);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_QUAD, m_bMSHActivateQuad);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_TET, m_bMSHActivateTet);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_PRIS, m_bMSHActivatePris);
    DDX_Check(pDX, IDC_CHECK_MSH_ACTIVATE_HEX, m_bMSHActivateHex);
    DDX_Text(pDX, IDC_EDIT_MSH_LINE, m_lNumberOfLine);
    DDX_Text(pDX, IDC_EDIT_MSH_TRI, m_lNumberOfTris);
    DDX_Text(pDX, IDC_EDIT_MSH_QUAD, m_lNumberOfQuad);
    DDX_Text(pDX, IDC_EDIT_MSH_TET, m_lNumberOfTets);
    DDX_Text(pDX, IDC_EDIT_MSH_PRIS, m_lNumberOfPris);
    DDX_Text(pDX, IDC_EDIT_MSH_HEX, m_lNumberOfHexs);
    DDX_Text(pDX, IDC_EDIT_MSH_ELEMENTS, m_lNumberOfElements);
    DDX_Text(pDX, IDC_EDIT_MSH_NODES, m_lNumberOfNodes);
    DDX_Control(pDX, IDC_LIST_MSH_MAT_GROUPS, m_LBMATGroups);
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Control(pDX, IDC_COMBO_GEO, m_CB_GEO);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Text(pDX, IDC_EDIT_MSH_NODES2, m_lNoMSHNodes);
    DDX_Control(pDX, IDC_COMBO_MSH_NEW, m_CB_MSH_NEW);
    DDX_Control(pDX, IDC_COMBO_GEO_LAYER, m_CB_GEO_LAYER);
    DDX_Text(pDX, IDC_EDIT_MSH_NO_MAT, m_lNoMAT);
    DDX_Text(pDX, IDC_EDIT_MSH_EDGE_MIN, m_dMinEdgeLength);
}


BEGIN_MESSAGE_MAP(CMSHEditor, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_MSH_ACTIVATE, OnBnClickedButtonMSHActivate)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT, OnBnClickedButtonMATConnectAll)
    ON_BN_CLICKED(IDC_BUTTON_MAT_WRITE_TECPLOT, OnBnClickedButtonMATWriteTecplot)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_LINE, OnBnClickedButtonMATConnectLine)

    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_TRI, OnBnClickedButtonMATConnectTris)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_QUAD, OnBnClickedButtonMATConnectQuad)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_TET, OnBnClickedButtonMATConnectTets)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_PRIS, OnBnClickedButtonMATConnectPris)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_HEX, OnBnClickedButtonMATConnectHexs)
    ON_LBN_SELCHANGE(IDC_LIST_MSH_MAT_GROUPS, OnLbnSelchangeListMATGroups)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO, OnCbnSelchangeComboGEO)
    ON_BN_CLICKED(IDC_BUTTON_GEO_NODES, OnBnClickedButtonGEONodes)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH, OnCbnSelchangeComboMSH)
    ON_BN_CLICKED(IDC_BUTTON_MSH_NAME, OnBnClickedButtonMSHName)
    ON_BN_CLICKED(IDC_BUTTON_MSH_WRITE, OnBnClickedButtonMSHWrite)
    ON_BN_CLICKED(IDC_BUTTON_MSH_DELETE, OnBnClickedButtonMSHDelete)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH_NEW, OnCbnSelchangeComboMSHNew)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_THIS, OnBnClickedButtonMatConnectThis)
    ON_BN_CLICKED(IDC_BUTTON_MAT_RESET, OnBnClickedButtonMatReset)
    ON_BN_CLICKED(IDC_BUTTON_MAT_WRITE_TEC, OnBnClickedButtonMatWriteTec)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_LAYER, OnBnClickedButtonMatConnectLayer)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_LAYER, OnCbnSelchangeComboGeoLayer)
    ON_BN_CLICKED(IDC_BUTTON_LAYER_POLYLINES, OnBnClickedButtonLayerPolylines)
    ON_BN_CLICKED(IDC_BUTTON_LAYER_SURFACES, OnBnClickedButtonLayerSurfaces)
    ON_BN_CLICKED(IDC_BUTTON_MAT_CONNECT_FAILED, OnBnClickedButtonMatConnectFailed)
    ON_BN_CLICKED(IDC_BUTTON_EDGE_LENGTH, OnBnClickedButtonEdgeLength)
END_MESSAGE_MAP()

// CMSHEditor message handlers
/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
04/2005 OK GEO
09/2005 OK Layer
**************************************************************************/
BOOL CMSHEditor::OnInitDialog() 
{
  int i;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // GEO
  m_CB_GEO.InitStorage(5,80);
  m_CB_GEO.ResetContent();
  m_CB_GEO.AddString("POINTS");
  m_CB_GEO.AddString("POLYLINES");
  m_CB_GEO.AddString("SURFACES");
  m_CB_GEO.AddString("VOLUMES");
  m_CB_GEO.AddString("LAYER"); //OK
  m_CB_GEO.SetCurSel(0);
  //----------------------------------------------------------------------
  // MSH
  m_CB_MSH.ResetContent();
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_msh->selected = false;
    m_strMSHName = m_msh->pcs_name.c_str();
    m_CB_MSH.AddString(m_strMSHName);
    m_CB_MSH.SetCurSel(i);
    m_msh->EdgeLengthMinMax();
    m_dMinEdgeLength = m_msh->min_edge_length;
  }
  //----------------------------------------------------------------------
  // MSH_NEW
  m_CB_MSH_NEW.ResetContent();
  m_CB_MSH_NEW.AddString("OVERLAND_FLOW");
  m_CB_MSH_NEW.AddString("RICHARDS_FLOW");
  m_CB_MSH_NEW.AddString("GROUNDWATER_FLOW");
  m_CB_MSH_NEW.AddString("DEFORMATION"); //OK
  m_CB_MSH_NEW.AddString("HEAT_TRANSPORT"); //OK
  m_CB_MSH_NEW.SetCurSel(0);
  m_strMSHNameNew = "OVERLAND_FLOW";
  //----------------------------------------------------------------------
  // Fill MAT groups list
  m_LBMATGroups.ResetContent();
  CMediumProperties *m_mat_mp = NULL;
  int m;
  int no_mat_mp = (int)mmp_vector.size();
  CString m_strLB;
  for(m=0;m<no_mat_mp;m++){
    m_mat_mp = mmp_vector[m];
    m_strLB = m_mat_mp->name.data();
    m_strLB +=  " | ";
    m_strLB += m_mat_mp->geo_name.c_str();
    m_LBMATGroups.AddString(m_strLB);
  }
  UpdateData(FALSE);
  return TRUE;
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2004 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMSHActivate()
{
  long i;
  Element *element=NULL;
  //----------------------------------------------------------------------
  UpdateData(TRUE);
  //----------------------------------------------------------------------
  for(i=0;i<ElListSize();i++){
    element = ElGetElement(i);
    //....................................................................
    if(element->elementtyp==1){
      if(m_bMSHActivateLine)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
    //....................................................................
    if(element->elementtyp==2){
      if(m_bMSHActivateQuad)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
    //....................................................................
    if(element->elementtyp==3){
      if(m_bMSHActivateHex)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
    //....................................................................
    if(element->elementtyp==4){
      if(m_bMSHActivateTri)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
    //....................................................................
    if(element->elementtyp==5){
      if(m_bMSHActivateTet)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
    //....................................................................
    if(element->elementtyp==6){
      if(m_bMSHActivatePris)
        element->aktiv = 1;
      else
        element->aktiv = 0;
    }
  }
  //----------------------------------------------------------------------
  UpdateData(FALSE);
}


/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2004 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMATConnectLine()
{
  MSHAssignMATGroup2LineElements();
}

void CMSHEditor::OnBnClickedButtonMATConnectTris()
{
  MSHAssignMATGroup2TrisElements((string)m_strMSHName); //OK
  //......................................................................
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_bDisplayMaterialGroups = true;
  //......................................................................
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

void CMSHEditor::OnBnClickedButtonMATConnectQuad()
{
  MSHAssignMATGroup2QuadElements();
}

void CMSHEditor::OnBnClickedButtonMATConnectTets()
{
  MSHAssignMATGroup2TetsElements();
}

void CMSHEditor::OnBnClickedButtonMATConnectPris()
{
  //MSHAssignMATGroup2PrisElementsNew();
  MSH2MATPris();
}

void CMSHEditor::OnBnClickedButtonMATConnectHexs()
{
  MSHAssignMATGroup2HexsElements();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2004 OK Implementation
09/2005 OK LAYER
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMATConnectAll()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //----------------------------------------------------------------------
  // un-highlighted
  GEOUnselectSFC();
  //----------------------------------------------------------------------
  vector<CGLVolume*>::const_iterator p_vol;
  CGLVolume* m_vol = NULL;
  if(m_strGEOTypeName.Compare("LAYER")==0){
    if(!m_msh)
      return;
    for(int i=1;i<m_msh->no_msh_layer+1;i++){
      //..................................................................
      m_CB_GEO_LAYER.GetLBText(i,m_strLAYName);
      mainframe->m_strLAYName = m_strLAYName;
      m_CB_GEO_LAYER.SetCurSel(i);
      UpdateData(FALSE);
      //..................................................................
      m_LB_GEO.ResetContent();
      p_vol = volume_vector.begin();
      while(p_vol!=volume_vector.end()) {
        m_vol = *p_vol;
        if(m_vol->layer==i)
          m_LB_GEO.AddString((CString)m_vol->name.c_str());
        ++p_vol;
      }
      //..................................................................
      OnBnClickedButtonMatConnectLayer();
      //..................................................................
      mainframe->OnDrawFromToolbar();
      AfxMessageBox("MAT data connected");
    }
  }
  else{
    MSHAssignMATGroup2Elements((string)m_strMSHName);
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
05/2004 OK Implementation
**************************************************************************/
void CMSHEditor::OnLbnSelchangeListMATGroups()
{
  m_iMMPGroup = m_LBMATGroups.GetCurSel();
  CMediumProperties *m_mat_mp = NULL;
  m_mat_mp = m_mat_mp->GetByGroupNumber(m_iMMPGroup);
  if(m_mat_mp)
    m_mat_mp->selected = true;
 
}

/**************************************************************************
GeoSys-GUI-Method:
Task: delete the double elements
Programing:
04/2005 CC Implementation
**************************************************************************/
void CMSHPolylines::OnBnClickedDeleteDouble()
{
  MSHDeleteDoubleElements(type);
}
/**************************************************************************
Task:
Programing:
01/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMATWriteTecplot()
{
  RFIWriteTecplot();
  MSHWriteTecplot();
  MSHLayerWriteTecplot();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnCbnSelchangeComboGEO()
{
  int i;
  //----------------------------------------------------------------------
  m_CB_GEO.GetLBText(m_CB_GEO.GetCurSel(),m_strGEOTypeName); //OK
  //----------------------------------------------------------------------
  CString m_str,m_strLayer;
  CGLPolyline *m_ply = NULL;
  vector<CGLPolyline*>::iterator p_ply = polyline_vector.begin();//CC 08/2005
  Surface *m_sfc = NULL;
  vector<Surface*>::iterator p_sfc = surface_vector.begin();
  CGLVolume *m_vol = NULL;
  vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC
  //----------------------------------------------------------------------
  m_LB_GEO.ResetContent();
  m_iGeoType = m_CB_GEO.GetCurSel();
  switch(m_CB_GEO.GetCurSel()){
    case 0:
      for(i=0;i<(int)gli_points_vector.size();i++){
        m_LB_GEO.AddString((CString)gli_points_vector[i]->name.c_str());
      }
      break;
    case 1:
      while(p_ply!=polyline_vector.end()) {
        m_ply = *p_ply;
        m_LB_GEO.AddString((CString)m_ply->name.c_str());
        ++p_ply;
      }
      break;
    case 2:
      while(p_sfc!=surface_vector.end()) {
        m_sfc = *p_sfc;
        m_LB_GEO.AddString((CString)m_sfc->name.c_str());
        ++p_sfc;
      }
      break;
    case 3:
      while(p_vol!=volume_vector.end()) {
        m_vol = *p_vol;
        m_LB_GEO.AddString((CString)m_vol->name.c_str());
        ++p_vol;
      }
      break;
    case 4: // LAYER
      m_strGEOTypeName = "LAYER";
      while(p_vol!=volume_vector.end()) {
        m_vol = *p_vol;
        if(m_vol->layer>0)
          m_LB_GEO.AddString((CString)m_vol->name.c_str());
        ++p_vol;
      }
      m_msh = FEMGet((string)m_strMSHName);
      m_CB_GEO_LAYER.ResetContent();
      m_CB_GEO_LAYER.AddString("ALL");
      m_CB_GEO_LAYER.SetCurSel(0);
      for(i=0;i<(int)m_msh->no_msh_layer;i++){
        m_str = "LAYER";
        m_strLayer.Format("%i",i+1);
        m_str += m_strLayer;
        m_CB_GEO_LAYER.AddString(m_str);
      }
      break;
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonGEONodes()
{
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh)
    return;
  vector<long>msh_nodes_vector;
  msh_nodes_vector.clear();
  Surface* m_sfc = NULL;
  CGLPolyline* m_ply = NULL;
  //----------------------------------------------------------------------
  switch(m_iGeoType){
    case 0:
      break;
    case 1:
      m_ply = GEOGetPLYByName((string)m_strGEOName);
      if(!m_ply)
        return;
      m_msh->GetNODOnPLY(m_ply,msh_nodes_vector);
      break;
    case 2: // SFC
      m_sfc = GEOGetSFCByName((string)m_strGEOName);//CC
      if(!m_sfc)
        return;
      //nodes_vector = m_sfc->GetMSHNodesClose();
      m_msh->GetNODOnSFC(m_sfc,msh_nodes_vector);
      break;
    case 3:
      break;
  }
  //----------------------------------------------------------------------
  for(long i=0;i<(long) m_msh->nod_vector.size();i++){
    m_msh->nod_vector[i]->SetMark(false);
  }
  for(long i=0;i<(long)msh_nodes_vector.size();i++){
    m_msh->nod_vector[msh_nodes_vector[i]]->SetMark(true);
  }
  m_lNoMSHNodes = (long)msh_nodes_vector.size();
  //----------------------------------------------------------------------
  //UpdateData(TRUE);
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
    theApp->g_graphics_modeless_dlg->m_bDisplayNOD = true;
  //----------------------------------------------------------------------
  UpdateData(FALSE);
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
} 

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2005 OK Implementation
09/2005 OK highlight selection
**************************************************************************/
void CMSHEditor::OnLbnSelchangeListGEO()
{
  //----------------------------------------------------------------------
  m_LB_GEO.GetText(m_LB_GEO.GetCurSel(),m_strGEOName);
  //----------------------------------------------------------------------
  vector<Surface*>::const_iterator p_sfc; 
  Surface* m_sfc = NULL;
  // un-highlighted
  p_sfc = surface_vector.begin();
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_sfc->highlighted = false;
    ++p_sfc;
  }
  // highlight
  CGLVolume* m_vol = NULL;
  m_vol = GEOGetVOL((string)m_strGEOName);
  if(m_vol&&m_vol->surface_vector.size()>0){
    p_sfc = m_vol->surface_vector.begin();
    m_sfc = *p_sfc;
    if(m_sfc)
      m_sfc->highlighted = true;
  }
  // Redraw
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
04/2005 OK Implementation
10/2009 OK MSH
**************************************************************************/
void CMSHEditor::OnCbnSelchangeComboMSH()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
  m_strMSHNameNew = m_strMSHName;
  m_msh = FEMGet((string)m_strMSHName);
  if(m_msh){
    m_lNumberOfElements = (long)m_msh->ele_vector.size();
    m_lNumberOfNodes = (long)m_msh->nod_vector.size();
    m_lNumberOfLine = 0;
    m_lNumberOfTris = 0;
    m_lNumberOfQuad = 0;
    m_lNumberOfTets = 0;
    m_lNumberOfPris = 0;
    m_lNumberOfHexs = 0;
    for(long i=0;i<m_lNumberOfElements;i++){
      switch(m_msh->ele_vector[i]->GetElementType()){
        case 1:
          m_lNumberOfLine++;
          break;
        case 2:
          m_lNumberOfQuad++;
          break;
        case 3:
          m_lNumberOfHexs++;
          break;
        case 4:
          m_lNumberOfTris++;
          break;
        case 5:
          m_lNumberOfTets++;
          break;
        case 6:
          m_lNumberOfPris++;
          break;
      }
    }
    //....................................................................
    if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
      theApp->g_graphics_modeless_dlg->m_strMSHName = m_strMSHName;
    //....................................................................
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->OnDrawFromToolbar();
    //....................................................................
  }
  UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
// CMSHSurfaces dialog

IMPLEMENT_DYNAMIC(CMSHSurfaces, CDialog)
CMSHSurfaces::CMSHSurfaces(CWnd* pParent /*=NULL*/)
	: CDialog(CMSHSurfaces::IDD, pParent)
    , m_iMSHType(FALSE)
    , m_iNr(0)
    , m_iNs(0)
{
}

CMSHSurfaces::~CMSHSurfaces()
{
}

void CMSHSurfaces::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SFC, m_LB_SFC);
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Control(pDX, IDC_LIST_SFC_MSH, m_LB_GEO2MSH);
    DDX_Radio(pDX, IDC_RADIO_TRIANGULATION, m_iMSHType);
    DDX_Text(pDX, IDC_EDIT_MSH_DENSITY_I, m_iNr);
    DDX_Text(pDX, IDC_EDIT_MSH_DENSITY_J, m_iNs);
}


BEGIN_MESSAGE_MAP(CMSHSurfaces, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_GEO2MSH, OnBnClickedButtonGEO2MSH)
    ON_LBN_SELCHANGE(IDC_LIST_SFC, OnLbnSelchangeListSFC)
    ON_BN_CLICKED(IDC_BUTTON_MSH_EXECUTE, OnBnClickedButtonMSHExecute)
    ON_EN_CHANGE(IDC_EDIT_MSH_DENSITY_I, OnEnChangeEditMshDensityI)
    ON_EN_CHANGE(IDC_EDIT_MSH_DENSITY_J, OnEnChangeEditMshDensityJ)
END_MESSAGE_MAP()

// CMSHSurfaces message handlers
/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
11/2005 OK m_LB_GEO2MSH
**************************************************************************/
BOOL CMSHSurfaces::OnInitDialog() 
{
  int i;
  Surface* m_sfc = NULL;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // GEO - Fill surface lists
  vector<Surface*>::iterator p_sfc = surface_vector.begin();
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_LB_SFC.AddString((CString)m_sfc->name.c_str());
    ++p_sfc;
  }
  //----------------------------------------------------------------------
  m_LB_GEO2MSH.ResetContent();
  for(i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    if(m_sfc->meshing_allowed)
      m_LB_GEO2MSH.AddString((CString)m_sfc->name.c_str());
  }
  //----------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL;
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_CB_MSH.AddString(m_msh->pcs_name.c_str());
  }
  //----------------------------------------------------------------------
  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CMSHSurfaces::OnLbnSelchangeListSFC()
{
  m_LB_SFC.GetText(m_LB_SFC.GetCurSel(),m_strNameSFC);
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
06/2005 OK Draw from dialog
11/2005 OK m_sfc->highlighted
**************************************************************************/
void CMSHSurfaces::OnBnClickedButtonGEO2MSH()
{
  Surface* m_sfc = NULL;
  if(m_strNameSFC.IsEmpty()) {
    AfxMessageBox("No surface selected !");
    return;
  }
  GEOUnselectSFC();
  m_sfc = GEOGetSFCByName((string)m_strNameSFC);//CC
  if(m_sfc){
    m_LB_GEO2MSH.AddString((CString)m_sfc->name.c_str());
    m_sfc->highlighted = true;
  }
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplaySUF = 1;
  //----------------------------------------------------------------------
  // Display selected surfaces in MSHView
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //CMultiDocTemplate	*pDocTemplate = NULL;
  //pDocTemplate = theApp.GetDocTemplate(DOCTEMPLATE_MSH_VIEW);
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
06/2005 OK Display mesh in MSHView
11/2005 OK case 0: msh_selected for triangulation
**************************************************************************/
void CMSHSurfaces::OnBnClickedButtonMSHExecute()
{
  UpdateData(TRUE);
  CFEMesh* m_msh = NULL;
  Surface* m_sfc = NULL;
  m_sfc = GEOGetSFCByName((string)m_strNameSFC);//CC
  if(!m_sfc)
    return;
  switch(m_iMSHType){
    case 0: //OK
      MSHUnselectSFC();
      for(int i=0;i<m_LB_GEO2MSH.GetCount();i++){
        m_LB_GEO2MSH.GetText(i,m_strNameSFC);
        m_sfc = GEOGetSFCByName((string)m_strNameSFC);
        if(m_sfc)
          m_sfc->meshing_allowed = 1;
      }
      break;
    case 1:
      m_msh = FEMGet("QUADfromSFC");
      if(m_msh){
        MSHDelete("QUADfromSFC");
      }
      m_msh = new CFEMesh();
      m_msh->pcs_name = "QUADfromSFC";
      m_msh->nr = m_iNr;
      m_msh->ns = m_iNs;
      //OK m_msh->mat_group = (int)mmp_vector.size();
      m_msh->ele_type = 2; // quads
      m_msh->CreateQuadELEFromSFC(m_sfc);
      if(m_msh->ele_vector.size()>0){
        fem_msh_vector.push_back(m_msh);
        CGSProject* m_gsp = NULL;
        m_gsp = GSPGetMember("gli");
        GSPAddMember(m_gsp->base + ".msh");
        RFPre_Model(); //OK
        start_new_elems = ElListSize(); //OK
        ConfigTopology(); //OK
      }
      //OnOK();
      break;
  }
  //----------------------------------------------------------------------
  // Display mesh in MSHView
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->m_iDisplayMSH = 1;
    theApp->g_graphics_modeless_dlg->m_bDisplayMSHQuad = true;
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
07/2005 OK Update PCS-MSH
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMSHName()
{
  //CRFProcess* m_pcs = NULL;
  //CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  m_msh = FEMGet((string)m_strMSHName);
  if(m_msh)
  {
    m_msh->pcs_name = (string)m_strMSHNameNew;
/*OK
    //theApp->g_graphics_modeless_dlg->OnInitDialog();
    m_pcs = PCSGet(m_msh->pcs_name); //OK
    if(m_pcs)
    {
      m_pcs->m_msh = m_msh;
      m_pcs->Create(); // Resize existing process //OK
    }
*/
  }
  OnInitDialog();
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMSHWrite()
{
  msh_file_binary = true;
  AfxMessageBox("BINARY file");
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(m_gsp)
    MSHWrite(m_gsp->path + m_gsp->base);
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CMSHPolylines::OnBnClickedButtonMSHSubdivisionExecute()
{
  UpdateData(TRUE);
  CFEMesh* m_msh = NULL;
  CGSProject* m_gsp = NULL;
  for(int i=0;i<m_LBPolylinesMSH.GetCount();i++){
    m_LBPolylinesMSH.GetText(i,m_strNamePolyline);
    m_ply = GEOGetPLYByName((string)m_strNamePolyline);//CC
    if(!m_ply)
      continue;
    m_msh = FEMGet("LINEfromPLY");
    if(m_msh){
      MSHDelete("LINEfromPLY");
    }
    m_msh = new CFEMesh();
    m_msh->pcs_name = "LINEfromPLY";
    m_msh->nr = m_iNr;
    m_msh->CreateLineELEFromPLY(m_ply);
    fem_msh_vector.push_back(m_msh);
    m_gsp = GSPGetMember("gli");
    GSPAddMember(m_gsp->base + ".msh");
  }
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CMSHSurfaces::OnEnChangeEditMshDensityI()
{
  //----------------------------------------------------------------------
  CGLPoint* m_pnt = NULL;
  Surface* m_sfc = NULL;
  m_sfc = GEOGetSFCByName((string)m_strNameSFC);//CC
  if(m_sfc){
    if(m_sfc->polygon_point_vector.size()<4){
      AfxMessageBox("OnEnChangeEditMshDensityI(): not enough SFC-PLY points");
      return;
    }
    m_pnt = new CGLPoint();
    m_pnt->id = -1; // temporary point
    m_pnt->x = 0.5*(m_sfc->polygon_point_vector[0]->x + m_sfc->polygon_point_vector[1]->x);
    m_pnt->y = 0.5*(m_sfc->polygon_point_vector[0]->y + m_sfc->polygon_point_vector[1]->y);
    m_pnt->circle_pix = 5;
    m_pnt->highlighted = true;
    gli_points_vector.push_back(m_pnt);
  }
  //----------------------------------------------------------------------
  // Display selected surfaces in MSHView
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayPNT = 1;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
/*
  long i = (long)gli_points_vector.size() - 1;
  delete m_pnt;
  gli_points_vector.erase(gli_points_vector.begin()+i);
*/
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
06/2005 OK Implementation
**************************************************************************/
void CMSHSurfaces::OnEnChangeEditMshDensityJ()
{
  //----------------------------------------------------------------------
  CGLPoint* m_pnt = NULL;
  Surface* m_sfc = NULL;
  m_sfc = GEOGetSFCByName((string)m_strNameSFC);//CC
  if(m_sfc){
    if(m_sfc->polygon_point_vector.size()<4){
      AfxMessageBox("OnEnChangeEditMshDensityJ(): not enough SFC-PLY points");
      return;
    }
    m_pnt = new CGLPoint();
    m_pnt->id = -1; // temporary point
    m_pnt->x = 0.5*(m_sfc->polygon_point_vector[1]->x + m_sfc->polygon_point_vector[2]->x);
    m_pnt->y = 0.5*(m_sfc->polygon_point_vector[1]->y + m_sfc->polygon_point_vector[2]->y);
    m_pnt->circle_pix = 5;
    m_pnt->highlighted = true;
    gli_points_vector.push_back(m_pnt);
  }
  //----------------------------------------------------------------------
  // Display selected surfaces in MSHView
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_iDisplayPNT = 1;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
/*
  long i = (long)gli_points_vector.size() - 1;
  delete m_pnt;
  gli_points_vector.erase(gli_points_vector.begin()+i);
*/
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMSHDelete()
{
  m_msh = FEMGet((string)m_strMSHName);
  if(m_msh)
    MSHDelete((string)m_strMSHName);
  else
    AfxMessageBox("no MSH data");
  OnInitDialog();
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->OnInitDialog(); //OK
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnCbnSelchangeComboMSHNew()
{
  m_CB_MSH_NEW.GetLBText(m_CB_MSH_NEW.GetCurSel(),m_strMSHNameNew);
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMatConnectThis()
{
  //-----------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no MSH data");
    return;
  }
  //-----------------------------------------------------------------------
  // MMP
  if((m_iMMPGroup<0)||(m_iMMPGroup>(int)mmp_vector.size())){
    AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no MMP data");
    return;
  }
  //-----------------------------------------------------------------------
  // GEO
  Surface* m_sfc = NULL;
  CGLVolume* m_vol = NULL;
  vector<Surface*>::const_iterator p_sfc; 
  string m_sfc_name;
  switch(m_iGeoType){
    case 0: // PNT
      AfxMessageBox("Warning in OnBnClickedButtonMatConnectThis: not implemented");
      break;
    case 1: // PLY
      AfxMessageBox("Warning in OnBnClickedButtonMatConnectThis: not implemented");
      break;
    case 2: // SFC
      AfxMessageBox("Warning in OnBnClickedButtonMatConnectThis: not implemented");
      break;
    case 3: // VOL
      AfxMessageBox("Warning in OnBnClickedButtonMatConnectThis: not implemented");
      break;
    //--------------------------------------------------------------------
    case 4: // VOL-LAYER
      m_vol = GEOGetVOL((string)m_strGEOName);
      if(!m_vol){
        AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no VOL data");
        return;
      }
      if((int)m_vol->surface_vector.size()==0){
        AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no SFC data");
        return;
      }
      p_sfc = m_vol->surface_vector.begin();
      m_sfc = *p_sfc;
      if(!m_sfc){
        AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no SFC data");
        return;
      }
      if(m_sfc) // highlight
        m_sfc->highlighted = true;
      m_vol->mat_group = m_iMMPGroup;
      m_msh->SetMATGroupFromVOLLayer(m_vol);
      break;
    //--------------------------------------------------------------------
  }
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMatReset()
{
  AfxMessageBox("Reset of ELE-MAT groups = -1",MB_OK,0);
  //-----------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("Error in OnBnClickedButtonMatConnectThis: no MSH data");
    return;
  }
  //-----------------------------------------------------------------------
  // Initialize MAT groups
  CElem* m_ele = NULL;
  for(long j=0;j<(long)m_msh->ele_vector.size();j++){
    m_ele = m_msh->ele_vector[j];
    m_ele->SetPatchIndex(-1);
  }
  //-----------------------------------------------------------------------
  // un-highlighted
  GEOUnselectSFC();
  // Redraw
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMatWriteTec()
{
  MMPWriteTecplot((string)m_strMSHName);
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMatConnectLayer()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //----------------------------------------------------------------------
  // un-highlighted
  GEOUnselectSFC();
  //----------------------------------------------------------------------
  if(m_strGEOTypeName.Compare("LAYER")==0){
    vector<CGLVolume*>::const_iterator p_vol;
    p_vol = volume_vector.begin();
    CGLVolume* m_vol = NULL;
    CString m_str = m_strLAYName.GetAt(5);
    int this_layer = strtol(m_str,NULL,0);
    int counter = 0;
    CMediumProperties* m_mmp = NULL;
    while(p_vol!=volume_vector.end()) {
      m_vol = *p_vol;
      if(m_vol->layer==this_layer){
        m_strGEOName = m_vol->name.data();
        m_iMMPGroup = -1;
        m_mmp = MMPGet(m_vol->mat_group_name);
        if(m_mmp)
          m_iMMPGroup = m_mmp->number;
        OnBnClickedButtonMatConnectThis();
        m_LB_GEO.SetCurSel(counter);
        UpdateData(FALSE);
        mainframe->OnDrawFromToolbar();
//AfxMessageBox("Pause");
      }
      counter++;
      ++p_vol;
    }
  }
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnCbnSelchangeComboGeoLayer()
{
  m_CB_GEO_LAYER.GetLBText(m_CB_GEO_LAYER.GetCurSel(),m_strLAYName);
  int layer = m_CB_GEO_LAYER.GetCurSel();
  vector<CGLVolume*>::const_iterator p_vol;
  CGLVolume* m_vol = NULL;
  m_LB_GEO.ResetContent();
  p_vol = volume_vector.begin();
  if(m_strLAYName.Compare("ALL")==0){
    while(p_vol!=volume_vector.end()) {
      m_vol = *p_vol;
      if(m_vol->layer>0)
        m_LB_GEO.AddString((CString)m_vol->name.c_str());
      ++p_vol;
    }
  }
  else{
    while(p_vol!=volume_vector.end()) {
      m_vol = *p_vol;
      if(m_vol->layer==layer)
        m_LB_GEO.AddString((CString)m_vol->name.c_str());
      ++p_vol;
    }
  }
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonLayerPolylines()
{
  CGLPolyline* m_ply = NULL;
  if(m_strGEOTypeName.Compare("POLYLINES")==0){
    m_ply = GEOGetPLYByName((string)m_strGEOName);
    if(m_ply&&m_msh){
      if(AfxMessageBox("Existing layer polylines will be deleted")){
        m_msh->EdgeLengthMinMax(); //OK
        m_ply->epsilon = m_msh->min_edge_length / 2.; //OK
        m_msh->CreateLayerPolylines(m_ply);
        OnCbnSelchangeComboGEO();
      }
      //m_LB_GEO.SetCurSel(last entry);
    }
    else
      AfxMessageBox("no MSH or PLY selected");
  }
  else
    AfxMessageBox("no PLY selected");
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonLayerSurfaces()
{
  int i;
  //----------------------------------------------------------------------
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }  
  //......................................................................
  if(m_strGEOTypeName.Compare("POLYLINES")!=0){
    AfxMessageBox("no PLY data selected");
    return;
  }
  //......................................................................
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(!m_gsp)
    return;
  //......................................................................
  CGLPolyline* m_ply = NULL;
  m_ply = GEOGetPLYByName((string)m_strGEOName);
  if(!m_ply){
    AfxMessageBox("no PLY data");
    return;
  }  
  //---------------------------------------------------------------------
  // Delete existing layer surfaces
  AfxMessageBox("Existing layer surfaces will be deleted");
  string sfc_lay_name = "SFC_" + m_ply->name + "_L";
  Surface* m_sfc = NULL;
  for(i=0;i<(int)surface_vector.size();i++){
    m_sfc = surface_vector[i];
    if(m_sfc->name.find(sfc_lay_name)!=string::npos){
      GEORemoveSFC(m_sfc);
      i--; //OK
    }
  }
  //---------------------------------------------------------------------
  string m_ply_sfc_name;
  CGLPolyline* m_ply_this = NULL;
  CGLPolyline* m_ply_sfc_l0 = NULL;
  CGLPolyline* m_ply_sfc_l1 = NULL;
  vector<CGLPolyline*>::const_iterator p_ply;
  //---------------------------------------------------------------------
  p_ply = polyline_vector.begin();
  while(p_ply!=polyline_vector.end()){
    m_ply_this = *p_ply;
    //--------------------------------------------------------------------
    if((m_ply_this->data_type==1)\
     &&(m_ply_this->name.find("_L0")!=string::npos)\
     &&(m_ply_this->name.find(m_ply->name)!=string::npos)){ // Layer polyline
      for(i=0;i<m_msh->no_msh_layer;i++){
        m_sfc = new Surface();
        m_sfc->type=1;
        m_sfc->name = "SFC_";
        //................................................................
        m_ply_sfc_l0 = *p_ply;
        m_sfc->polyline_of_surface_vector.push_back(m_ply_sfc_l0);
        p_ply++;
        m_ply_sfc_l1 = *p_ply;
        if(!m_ply_sfc_l1){
          AfxMessageBox("no PLY data");
          delete m_sfc;
          return;
        }
        m_sfc->name += m_ply_sfc_l1->name;
        m_sfc->polyline_of_surface_vector.push_back(m_ply_sfc_l1);
        //................................................................
        GEOSurfaceTopology();
        m_sfc->PolygonPointVector();
        //................................................................
        m_sfc->CreateTIN();
        m_sfc->WriteTIN(m_gsp->path);
        m_sfc->WriteTINTecplot(m_gsp->path);
        //................................................................
        surface_vector.push_back(m_sfc);
      }
    }
    //--------------------------------------------------------------------
    p_ply++;
  }
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonMatConnectFailed()
{
  m_lNoMAT = 0;
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }
  for(long i=0;i<(long)m_msh->ele_vector.size();i++){
    if(m_msh->ele_vector[i]->GetPatchIndex()==-1)
      m_lNoMAT++;
  }
  UpdateData(FALSE);
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
MSHLibGUI-Method: 
Task: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void CMSHEditor::OnBnClickedButtonEdgeLength()
{
  if(m_msh)
    m_msh->EdgeLengthMinMax();
  m_dMinEdgeLength = m_msh->min_edge_length;
  UpdateData(FALSE);
}
