// PRISGEN.cpp: Implementierungsdatei

#include "stdafx.h"
#include "geosys.h"
#include "gs_prisgen.h"
#include "msh_gen_pris.h"
#include "elements.h"
#include "GeoSysDoc.h"
//#include "grid2sfc.h"
#include ".\gs_prisgen.h"
#include "MainFrm.h"

// GeoLib
#include "geo_sfc.h"
// MSHLib
#include "msh_lib.h"
#include "fem_ele.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CPRISGEN::CPRISGEN(CWnd* pParent /*=NULL*/)
	: CDialog(CPRISGEN::IDD, pParent)
    , m_iMATGroup(0)
    , m_iSubDivision(0)
    , m_lNoElements(0)
{
	m_numberofprismlayers = 0;
	m_thicknessofprismlayer = 0;
    m_strMSHType = "PRIS";
    m_iLayer = 0;
}


void CPRISGEN::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_PRISGEN1, m_numberofprismlayers);
    DDX_Text(pDX, IDC_EDIT_PRISGEN2, m_thicknessofprismlayer);
    DDX_Control(pDX, IDC_LIST_SURFACES, m_LBSurfaces);
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Text(pDX, IDC_EDIT_MAT_GROUP, m_iMATGroup);
    DDX_Control(pDX, IDC_COMBO_LAYER, m_CB_Layer);
    DDX_Text(pDX, IDC_EDIT_PRISGEN_REFINE, m_iSubDivision);
    DDX_Text(pDX, IDC_EDIT_PRISGEN_ELEMENTS, m_lNoElements);
}


BEGIN_MESSAGE_MAP(CPRISGEN, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CREATEPRISMS, OnButtonCreatePrisms)
    ON_LBN_SELCHANGE(IDC_LIST_SURFACES, OnLbnSelchangeListSurfaces)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_PRISM_ELEMENTS, OnBnClickedButtonCreatePrismElements)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH, OnCbnSelchangeComboMSH)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_PRISM_ELEMENTS2, OnBnClickedButtonAppendElements)
    ON_BN_CLICKED(IDC_BUTTON_PRISM_REFINE, OnBnClickedButtonPrismRefine)
    ON_CBN_SELCHANGE(IDC_COMBO_LAYER, OnCbnSelchangeComboLayer)
END_MESSAGE_MAP()


// Behandlungsroutinen für Nachrichten CPRISGEN 

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK Implementation
04/2005 OK MSH
03/2006 OK bugfix
**************************************************************************/
BOOL CPRISGEN::OnInitDialog()
{
  int i;
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // MSH
  m_msh = NULL;
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_msh->selected = false;
    m_CB_MSH.AddString(m_msh->pcs_name.c_str());
    m_CB_MSH.SetCurSel(i);
  }
  //----------------------------------------------------------------------
  // Surfaces ListBox
  Surface *m_surface = NULL;
  m_LBSurfaces.ResetContent();
  vector<Surface*>::iterator ps = surface_vector.begin();
  while(ps!=surface_vector.end()) {
    m_surface = *ps;
    m_LBSurfaces.AddString((CString)m_surface->name.data());
    ++ps;
  }
  return TRUE;
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
11/2003 MB Implementation
03/2004 OK TINs
06/2005 MB only safe as function
last modification : 
**************************************************************************/
void CPRISGEN::OnButtonCreatePrisms() 
//TODO CHange name
{
  UpdateData(TRUE);
  if(m_msh){
  //Create_Triangles2Prisms(m_numberofprismlayers,m_thicknessofprismlayer,m_msh);
  GEOCreateLayerSurfaceTINs(m_numberofprismlayers,m_thicknessofprismlayer);
  CFileDialog fileDlg(FALSE);
  if (fileDlg.DoModal()==IDOK) {
    char a = '\\';
    int pos = fileDlg.GetPathName().ReverseFind(a);
    CString m_strFileNamePath = fileDlg.GetPathName().Left(pos);
	CString m_strMSHFileName = fileDlg.GetFileName();
	UpdateData(FALSE);
    FEMWrite((string)m_strMSHFileName);
	//MSH_WritePrisms2RFI((string)m_strMSHFileName);
    GEOWriteSurfaceTINs((string)m_strFileNamePath);
    GEOWriteSurfaceTINsTecplot((string)m_strFileNamePath);
	//CDialog::OnOK(); do not leave dialog
  }
}

else{   //Kann raus komplett raus, bei Umstellung auf msh
         
  //TODO Abfangen wenn kein File geöffnet ist
  if (NODListExists()) {	
    Create_Triangles2Prisms(m_numberofprismlayers,m_thicknessofprismlayer,NULL);
    // komplett raus erstetzt mit: CreatePriELEFromTri ?? (nur für m_msh)   MB
    GEOCreateLayerSurfaceTINs(m_numberofprismlayers,m_thicknessofprismlayer);
	   CFileDialog fileDlg(FALSE);
	   if (fileDlg.DoModal()==IDOK) {
      char a = '\\';
      int pos = fileDlg.GetPathName().ReverseFind(a);
      CString m_strFileNamePath = fileDlg.GetPathName().Left(pos);
	  CString m_strMSHFileName = fileDlg.GetFileName();
	  UpdateData(FALSE);
	  MSH_OverWriteMSH((string)m_strMSHFileName);
      GEOWriteSurfaceTINs((string)m_strFileNamePath);
      GEOWriteSurfaceTINsTecplot((string)m_strFileNamePath);
	  //CDialog::OnOK(); do not leave dialog
	}
  }
}
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK Implementation
last modification : 
**************************************************************************/
void CPRISGEN::OnLbnSelchangeListSurfaces()
{
  m_LBSurfaces.GetText(m_LBSurfaces.GetCurSel(),m_strSurface);
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
04/2005 OK Implementation
09/2005 OK Layer
last modification: 
**************************************************************************/
void CPRISGEN::OnCbnSelchangeComboMSH()
{
  CString m_str;
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh)
    return;
  m_CB_Layer.ResetContent();
  for(int i=0;i<(int)m_msh->no_msh_layer;i++){
    m_str = "LAYER";
    m_strLayer.Format("%i",i+1);
    m_str += m_strLayer;
    m_CB_Layer.AddString(m_str);
  }
  m_CB_Layer.SetCurSel(0);
  m_lNoElements = (long)m_msh->ele_vector.size();
  UpdateData(FALSE);
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
    theApp->g_graphics_modeless_dlg->m_strMSHName = m_strMSHName;
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK Implementation
last modification : 
**************************************************************************/
void CPRISGEN::OnBnClickedButtonDelete()
{
  long i,j;
  Element *element = NULL;
  //long selected_layer = strtol(m_strLayer.GetString(),NULL,0);
  Surface *m_surface = NULL;
  m_surface = GEOGetSFCByName((string)m_strSurface);//CC
  long number_of_surface_points = (long)m_surface->polygon_point_vector.size();
  CGLPoint *m_point = NULL;
  CPoint this_point;
  POINT *m_arrPoint = NULL;
  m_arrPoint = new POINT[number_of_surface_points+1];
  //POINT m_arrPoint[NUMBER_OF_SURFACE_POINTS];
  //-----------------------------------------------------------------------
  for(i=0;i<ElementListLength;i++) {
    element = ElGetElement(i);
    //if(element->layer==selected_layer) {
      this_point.x = (long)element->element_midpoint[0];
      this_point.y = (long)element->element_midpoint[1];
      for(j=0;j<number_of_surface_points;j++) {
        m_point = m_surface->polygon_point_vector[j];
        m_arrPoint[j].x = (long)m_point->x;
        m_arrPoint[j].y = (long)m_point->y;
      }
      CRgn surface_polygon;
      surface_polygon.CreatePolygonRgn(&m_arrPoint[0],number_of_surface_points,WINDING);
      if (surface_polygon.PtInRegion(this_point))
        ELEDeleteElementFromList(i,i);
      DeleteObject(surface_polygon);
    //}
  }
  //-----------------------------------------------------------------------
  delete [] m_arrPoint;
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK Implementation
04/2005 OK CreateHexELEFromQuad
04/2005 OK CreateLineELEFromQuad
last modification:
**************************************************************************/
void CPRISGEN::OnBnClickedButtonCreatePrismElements()
{
  UpdateData(TRUE);
  if(m_msh){
    if(m_strMSHType=="PRIS"){
      m_msh->CreatePriELEFromTri(m_numberofprismlayers,m_thicknessofprismlayer);
    }
    else if(m_strMSHType=="HEX"){
      m_msh->CreateHexELEFromQuad(m_numberofprismlayers,m_thicknessofprismlayer);
    }
    else if(m_strMSHType=="LINE"){
      m_msh->CreateLineELEFromQuad(m_numberofprismlayers,m_thicknessofprismlayer,m_iMATGroup);
    }
  }
  else{
    if(m_strMSHType=="PRIS"){
      MSHCreatePrismsFromTriangles();
      MSHCreateNodes();
    }
  }
  //----------------------------------------------------------------------
  // CP
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL){
    theApp->g_graphics_modeless_dlg->m_bDisplayMSHPris = true;
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
05/2005 OK Implementation
**************************************************************************/
void CPRISGEN::OnBnClickedButtonAppendElements()
{
  UpdateData(TRUE);
  if(m_strMSHType=="PRIS"){
  }
  else if(m_strMSHType=="HEX"){
  }
  else if(m_strMSHType=="LINE"){
    m_msh->no_msh_layer = m_numberofprismlayers;
    m_msh->z_min = m_thicknessofprismlayer;
    //OK m_msh->mat_group = m_iMATGroup;
    m_msh->AppendLineELE();
  }
}

void CPRISGEN::OnCancel() 
{
  MSH_Destroy_PrismDataConstructs();
  CDialog::OnCancel();
}


// CPrisGenMap dialog
CPrisGenMap::CPrisGenMap(CWnd* pParent /*=NULL*/)
	: CDialog(CPrisGenMap::IDD, pParent)
    , m_file_number(_T(""))
{
	m_iNumberOfLayers = 0;
	m_iRowNumber = 0;
	m_uiRangeFrom = 0;
	m_uiRangeTo = 1;
    m_msh = NULL;
}

void CPrisGenMap::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_RowNumber, m_iRowNumber);
    DDX_Text(pDX, IDC_CountOfPrismLayers, m_iNumberOfLayers);
    DDX_Control(pDX, IDC_LIST_FILE, m_LBFileNames);
    DDX_Text(pDX, IDC_EDIT1, m_file_number);
    DDX_Control(pDX, IDC_PROGRESS_MAPPING, m_progess_mapping);
    DDX_Control(pDX, IDC_COMBO_MSH, m_CB_MSH);
    DDX_Control(pDX, IDC_COMBO_LAYER, m_CB_LAY);
}


BEGIN_MESSAGE_MAP(CPrisGenMap, CDialog)
	ON_BN_CLICKED(IDC_MapRow, OnBnClickedButtonMapRow)
    ON_BN_CLICKED(IDC_BUTTON_ADD_FILE, OnBnClickedButtonAddFile)
    ON_BN_CLICKED(IDC_BUTTON_EXECUTE, OnBnClickedButtonExecute)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_LIST, OnBnClickedButtonClearList)
    ON_CBN_SELCHANGE(IDC_COMBO_MSH, OnCbnSelchangeComboMSH)
    ON_CBN_SELCHANGE(IDC_COMBO_LAYER, OnCbnSelchangeComboLayer)
END_MESSAGE_MAP()

// CPrisGenMap message handlers
/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK/MB Implementation
07/2005 OK MSH
09/2005 OK LAY
last modification : 
**************************************************************************/
BOOL CPrisGenMap::OnInitDialog() 
{
  CDialog::OnInitDialog();
  //-----------------------------------------------------------------------
  // Progress bar
  // Initially create progress control in horizontal position
  CWnd* pWnd = GetDlgItem( IDC_PROGRESS_MAPPING );
  CRect rect;
  pWnd->GetWindowRect(&rect);
  ScreenToClient(&rect);
  // Initialise controls
  //m_progess_mapping.Create( WS_VISIBLE | WS_CHILD, rect, this, IDC_PROGRESS_MAPPING );
  //m_progess_mapping.SetRange( static_cast<short>(m_uiRangeFrom), static_cast<short>(m_uiRangeTo) );
//RN
  m_progess_mapping.SetRange(0,100);
  m_progess_mapping.SetStep(1);
  SetTimer(1,100,NULL);
//RN
  //-----------------------------------------------------------------------
  // MSH
  m_CB_MSH.ResetContent();
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_CB_MSH.AddString(m_msh->pcs_name.c_str());
    m_CB_MSH.SetCurSel(i); //OK
    m_strMSHName = m_msh->pcs_name.data();
    m_iNumberOfLayers = m_msh->no_msh_layer; 
    m_iRowNumber = m_iNumberOfLayers+1;
  }
  UpdateData(FALSE);
  //-----------------------------------------------------------------------
  // LAY
  if(m_msh){
    CString m_str;
    CString m_strLayer;
    m_CB_LAY.ResetContent();
    for(i=0;i<(int)m_msh->no_msh_layer;i++){
      m_str = "LAYER";
      m_strLayer.Format("%i",i);
      m_str += m_strLayer;
      m_strLayer.Format("%i",i+1);
      m_str += m_strLayer;
      m_CB_LAY.AddString(m_str);
    }
    m_CB_LAY.SetCurSel(0);
  }
  //-----------------------------------------------------------------------
  return TRUE;
}

void CPrisGenMap::OnBnClickedButtonMapRow()
{

  UpdateData(TRUE);
  /* Get Path name */
  CFileDialog dlg(TRUE, "dat", NULL, OFN_ENABLESIZING ,
                 "Grid Files (*.dat)|*.dat| ArcView Files (*.asc)|*.asc| All Files (*.*)|*.*||", this );
  dlg.DoModal();
  CString m_filepath = dlg.GetPathName();

  //switch cases je nach extension
  CString m_strFileNameExtension = m_filepath.Right(4);
  int DataType = 0;
  if(m_strFileNameExtension==".dat") DataType = 1;
  else if(m_strFileNameExtension==".asc") DataType = 2;
  else 
	  AfxMessageBox("Not a valid file extension");
  if(!m_msh)
    AfxMessageBox("no MSH data");
  else
  MSHMapping(m_filepath, m_iNumberOfLayers, m_iRowNumber, DataType,m_msh); //OK
 
  AfxMessageBox("Ready! Safe rfi: File --> Export --> MSH File");

  CDialog::OnOK();

}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK/MB Implementation
last modification : 
**************************************************************************/
void CPrisGenMap::OnBnClickedButtonAddFile()
{
  // Get file name
  CFileDialog dlg(TRUE, "dat", NULL, OFN_ENABLESIZING ,
                 " Grid Files (*.dat)|*.dat| ArcView Files (*.asc)|*.asc| All Files (*.*)|*.*||", this );
  dlg.DoModal();
  CString m_filepath = dlg.GetPathName();
  //
  if(!m_filepath.IsEmpty()) //OK
    m_LBFileNames.AddString(m_filepath);

}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
03/2004 OK/MB Implementation
03/2004 RN progress bar
07/2005 OK MSH
last modification : 
**************************************************************************/
void CPrisGenMap::OnBnClickedButtonExecute()
{
  CString m_strFileName;
  CString m_strFileNameNew;
  int i;
  int NumberOfLayers;
  CString m_strFileNameExtension; //with point
  char number[10];
  sprintf(number,"%ld",0);
  m_file_number = number;
  int RowNumber;
  //----------------------------------------------------------------------
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh){
    AfxMessageBox("no MSH data");
    return;
  }
  if(m_msh->no_msh_layer!=(m_LBFileNames.GetCount()-1)){
    AfxMessageBox("not enough MAP data");
    return;
  }
  //----------------------------------------------------------------------
  UpdateData();
  NumberOfLayers = m_LBFileNames.GetCount()-1;
  m_progess_mapping.SetRange((short)0,(short)m_LBFileNames.GetCount());
  m_progess_mapping.SetStep(0);
  SetTimer(0,NumberOfLayers,NULL);
  //----------------------------------------------------------------------
  for(i=0;i< m_LBFileNames.GetCount() ;i++) {
    sprintf(number,"%ld",i+1);
    m_file_number = number;
    // progress bar
    //Sleep(50);
	m_progess_mapping.StepIt();
	m_progess_mapping.SetPos(i+1);
	UpdateData(FALSE);
    //
    RowNumber = i+1;
    m_LBFileNames.GetText(i,m_strFileName);
    m_strFileNameExtension = m_strFileName.Right(4);
    int DataType = 0;
    if(m_strFileNameExtension==".dat") DataType = 1;
    else if(m_strFileNameExtension==".asc") DataType = 2;
    else 
	  AfxMessageBox("Not a valid file extension");
    m_msh = FEMGet((string)m_strMSHName);
    if(m_msh)
      MSHMapping(m_strFileName,NumberOfLayers,RowNumber,DataType,m_msh);
    else
      AfxMessageBox("no MSH data");
  }
  //m_strFileNameNew = m_strFileNameBase + "_mapped.rfi";
  //DATWriteRFIFile(m_strFileNameNew);
  OnOK();
  AfxMessageBox("Ready! Safe rfi: File --> Export --> MSH File");

/*
//RN
  for(int i=0;i<=100;i++)
  {
	  int value=i;
	  Sleep(50);
	  m_progess_mapping.StepIt();
	  m_progess_mapping.SetPos(value);
	  UpdateData(FALSE);
  }
//RN
*/
}


// CPrisGenVertDisc dialog
CPrisGenVertDisc::CPrisGenVertDisc(CWnd* pParent /*=NULL*/)
	: CDialog(CPrisGenVertDisc::IDD, pParent)
{
	m_iNumberOfLayers = 0;
	m_iLayerToBeDiscretized = 0;
	m_iNumberOfSubLayers = 0;
}

void CPrisGenVertDisc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CountOfPrismLayers, m_iNumberOfLayers);
    DDX_Text(pDX, IDC_LayerToBeDiscretized, m_iLayerToBeDiscretized);
	DDX_Text(pDX, IDC_NumberOfSublayers, m_iNumberOfSubLayers);
}


BEGIN_MESSAGE_MAP(CPrisGenVertDisc, CDialog)
	ON_BN_CLICKED(IDC_RefineLayer, OnBnClickedButtonRefineLayer)
END_MESSAGE_MAP()


// CPrisGenVertDisc message handlers
void CPrisGenVertDisc::OnBnClickedButtonRefineLayer()
{
  UpdateData(TRUE);
  PrismRefine(m_iNumberOfLayers, m_iLayerToBeDiscretized, m_iNumberOfSubLayers);
  CDialog::OnOK();
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
01/2005 CC Implementation
**************************************************************************/
void CPrisGenMap::OnBnClickedButtonClearList()
{
    m_LBFileNames.ResetContent();
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
09/2005 OK LAY
**************************************************************************/
void CPrisGenMap::OnCbnSelchangeComboMSH()
{
  //----------------------------------------------------------------------
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
  //----------------------------------------------------------------------
  m_msh = FEMGet((string)m_strMSHName);
  if(!m_msh)
    return;
  CString m_str;
  CString m_strLayer;
  m_CB_LAY.ResetContent();
  for(int i=0;i<(int)m_msh->no_msh_layer;i++){
    m_str = "LAYER";
    m_strLayer.Format("%i",i+1);
    m_str += m_strLayer;
    m_CB_LAY.AddString(m_str);
    m_CB_LAY.SetCurSel(0);
  }
  m_iNumberOfLayers = m_msh->no_msh_layer; 
  m_iRowNumber = m_iNumberOfLayers+1;
  UpdateData(FALSE);
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CPRISGEN::OnBnClickedButtonPrismRefine()
{
  UpdateData(TRUE);
  if(m_msh&&(m_iLayer>0)&&(m_iSubDivision>0)){
    m_msh->PrismRefine(m_iLayer,m_iSubDivision);
    m_lNoElements = (long)m_msh->ele_vector.size();
    UpdateData(FALSE);
  }
  else
    AfxMessageBox("no MSH data");
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CPRISGEN::OnCbnSelchangeComboLayer()
{
  m_iLayer = m_CB_Layer.GetCurSel() + 1;
}

/**************************************************************************
GeoSys-GUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
void CPrisGenMap::OnCbnSelchangeComboLayer()
{
  m_iRowNumber = m_CB_LAY.GetCurSel()+1;
  UpdateData(FALSE);
}
