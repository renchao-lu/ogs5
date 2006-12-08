// dlg_vol.cpp : implementation file
// 01/2005 OK Implementation

#include "stdafx.h"
#include "GeoSys.h"
#include "vol_dlg.h"
#include "MainFrm.h"
// GEOLib
#include "geo_lib.h"
// MSHLib
#include "msh_elements_rfi.h"
//PCH
#include "GeoSysDoc.h"
// CDialogVolume dialog
#include "gs_project.h"

IMPLEMENT_DYNAMIC(CDialogVolume, CDialog)
CDialogVolume::CDialogVolume(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogVolume::IDD, pParent)
{
}

CDialogVolume::~CDialogVolume()
{
}

void CDialogVolume::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SURFACES, m_LBSurfaces);
    DDX_Control(pDX, IDC_LIST_VOLUMES, m_LBVolumes);
// PCH
    DDX_Control(pDX, IDC_LIST_SURFACE_POLYLINES, m_LBVOLSurfaces);
    DDX_Text(pDX, IDC_EDIT_VOLUME_NAME, m_strNameVolume);
}

BEGIN_MESSAGE_MAP(CDialogVolume, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_LAYER_VOLUMES, OnBnClickedButtonCreateLayerVolumes)
    ON_LBN_SELCHANGE(IDC_LIST_SURFACES, OnLbnSelchangeListSurfaces)
    ON_BN_CLICKED(IDC_BUTTON_VOLUMES_WRITE_TEC, OnBnClickedButtonVolumesWriteTec)
    ON_BN_CLICKED(IDC_BUTTON_SURFACE_ADD, OnBnClickedButtonSurfaceAdd)
    ON_BN_CLICKED(IDC_OK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_CANCEL, OnBnClickedCancel)
    ON_LBN_SELCHANGE(IDC_LIST_VOLUMES, OnLbnSelchangeListVolumes)
    ON_EN_CHANGE(IDC_EDIT_VOLUME_NAME, OnEnChangeEditVolumeName)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_PLY2SFC, OnBnClickedButtonSurface2Volume)
	ON_LBN_SELCHANGE(IDC_LIST_SURFACE_POLYLINES, OnLbnSelchangeListSurfacePolylines)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

// CDialogVolume message handlers

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
last modification: 
04/2005 PCH modified to automatically add the picked surfaces
**************************************************************************/
BOOL CDialogVolume::OnInitDialog() 
{
  //int nSel = 0;//CC
  //----------------------------------------------------------------------
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  m_LBVolumes.ResetContent();
    vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC
  while (p_vol!= volume_vector.end()){
    m_vol = *p_vol;
    m_LBVolumes.AddString((CString)m_vol->name.data());
	++p_vol;
  }
  //----------------------------------------------------------------------
  m_LBSurfaces.ResetContent();
  vector<Surface*>::iterator p_sfc = surface_vector.begin();//CC
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_LBSurfaces.AddString((CString)m_sfc->name.data());
    ++p_sfc;
	}
  // PCH added this to display the picked polylines automatically.
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	for(int i=0; i< theApp->hitsSurfaceTotal; ++i)
	{
		Surface* thisSurface = surface_vector[theApp->surfacePickedTotal[i]];//CC
		m_LBSurfaces.AddString((CString)thisSurface->name.c_str());
	}
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
last modification: 
**************************************************************************/
void CDialogVolume::OnLbnSelchangeListSurfaces()
{
  m_LBSurfaces.GetText(m_LBSurfaces.GetCurSel(),m_strSurfaceSelected);
}
/**************************************************************************
GeoLibGUI-Method: 
Task: Automatic picking handling in volume
Programing:
05/2005 PCH Implementation
last modification: 
**************************************************************************/
void CDialogVolume::OnUpdateSurfaceListPicked()
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Update in one CListBox
	OnUpdateSurfaceList(&m_LBSurfaces);

	// Update the other CListBox
	OnUpdateSurfaceList(&m_LBVOLSurfaces);
}

/**************************************************************************
GeoLibGUI-Method: 
Task: Automatic picking handling in volume
Programing:
05/2005 PCH Implementation
last modification: 
**************************************************************************/
void CDialogVolume::OnUpdateSurfaceList(CListBox* list)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Make CListBox empty before we proceed our picking
	list->ResetContent();
	
	for(int i=0; i< theApp->hitsSurfaceTotal; ++i)
	{
		Surface* thisSurface = surface_vector[theApp->surfacePickedTotal[i]];//CC
		int idx = list->AddString((CString)thisSurface->name.c_str());
	list->SetItemData(idx, i+1);
  }
}
/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
last modification: 
**************************************************************************/
void CDialogVolume::OnBnClickedButtonCreateLayerVolumes()
{
  int i;
  m_sfc = GEOGetSFCByName((string)m_strSurfaceSelected);//CC
if(m_sfc){
  string sfc_name;
  char layer_number[3];
  int no_layers = (int)(msh_no_pris/msh_no_tris);
  //----------------------------------------------------------------------
  if(no_layers>0){
    m_vol = new CGLVolume();
    m_sfc = GEOGetSFCByName((string)m_strSurfaceSelected);//CC
    m_vol->surface_vector.push_back(m_sfc);//CC
    sfc_name = (string)m_strSurfaceSelected + "_layer_1";
    m_sfc = GEOGetSFCByName(sfc_name);//CC
    if(m_sfc){
      m_vol->surface_vector.push_back(m_sfc);
      m_vol->name = "VOLUME_" + sfc_name; //OK41
      volume_vector.push_back(m_vol);
    }
    else
      AfxMessageBox("Surface not found");
  }
  for(i=1;i<no_layers;i++){
    m_vol = new CGLVolume();
    sprintf(layer_number,"%i",i);
    sfc_name = (string)m_strSurfaceSelected + "_layer_" + layer_number;
    m_sfc = GEOGetSFCByName(sfc_name);//CC
    if(!m_sfc){
      AfxMessageBox("Surface not found");
      continue;
    }
    m_vol->surface_vector.push_back(m_sfc);//CC
    sprintf(layer_number,"%i",i+1);
    sfc_name = (string)m_strSurfaceSelected + "_layer_" + layer_number;
    m_sfc = GEOGetSFCByName(sfc_name);//CC
    if(!m_sfc){
      AfxMessageBox("Surface not found");
      continue;
    }
    m_vol->surface_vector.push_back(m_sfc);//CC
    m_vol->name = "VOLUME_" + sfc_name;
    volume_vector.push_back(m_vol);
  }
 }
 else
   AfxMessageBox("Surface not found");
  //----------------------------------------------------------------------
  OnInitDialog();
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
01/2005 OK Implementation
last modification: 
**************************************************************************/
void CDialogVolume::OnBnClickedButtonVolumesWriteTec()
{
  MSHWriteVOL2TEC((string)m_strFileNamePath);
}
/**************************************************************************
GeoLibGUI-Method: 
Task: Add a volume that contains surface info according to the definition
      of this class (volume class - this dialog)
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CDialogVolume::OnBnClickedButtonSurfaceAdd()
{
    //----------------------------------------------------------------------
  // Volume list
  int numberOfVolumes = (int)volume_vector.size();
  char volumeName[80];
  CGLVolume*  m_vol = NULL;
  m_vol = new CGLVolume();
  sprintf(volumeName,"%s%d","VOLUME",numberOfVolumes);
  m_vol->name = volumeName;
  
  volume_vector.push_back(m_vol);
  //----------------------------------------------------------------------
  // ListBox
  m_LBVolumes.AddString((CString)volumeName);
  m_LBVolumes.SetCurSel(numberOfVolumes);
  m_LBVolumes.SetSel((int)numberOfVolumes,TRUE);
}

void CDialogVolume::OnBnClickedOk()
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
    
//    OnOK();
}

void CDialogVolume::OnBnClickedCancel()
{
    OnCancel();
}

/**************************************************************************
GeoLibGUI-Method: OnLbnSelchangeListVolumes()
Task: Do the similar thing that OnLbnSelchangeListSurfaces() does in Volume
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CDialogVolume::OnLbnSelchangeListVolumes()
{
    // Get selected surface from ListBox
    int nSel = m_LBVolumes.GetCurSel();
    m_LBVolumes.GetText(nSel,m_strNameVolume);
    // Display volume surface list
    CGLVolume*  gs_volume = NULL;
    Surface *gs_surface = NULL;
    m_LBVOLSurfaces.ResetContent();
    gs_volume = GEOGetVOL(string(m_strNameVolume));//CC

    //change selection
    if (gs_volume->selected == TRUE)
        gs_volume->selected = FALSE;
    else
        gs_volume->selected = TRUE;

    vector<Surface*>::iterator p = gs_volume->surface_vector.begin();
    while(p!=gs_volume->surface_vector.end()) {
        gs_surface = *p;
        if(gs_surface) {
            m_LBVOLSurfaces.AddString((CString)gs_surface->name.c_str());
        }
        else {
        AfxMessageBox("Error in surfaces !");
        }
        ++p;
    }
    UpdateData(FALSE);
}

void CDialogVolume::OnEnChangeEditVolumeName()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

/**************************************************************************
GeoLibGUI-Method: 
Task: This just does the same thing that OnBnClickedButtonCreate() does in 
        surface
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CDialogVolume::OnBnClickedButtonCreate()
{
    CString m_strSFCName;
    CGLVolume* m_vol = NULL;
    m_vol = GEOGetVOL((string)m_strNameVolume);//CC
    // Create volume surfaces and check surface type
    for(int i=0;i< m_LBVOLSurfaces.GetCount();i++)
    {
        m_LBVOLSurfaces.GetText(i,m_strSFCName);
        m_sfc = GEOGetSFCByName((string)m_strSFCName); //OK CC change
       
        if(m_sfc)
        {
            if(m_vol)
            {
                m_vol->AddSurface(m_sfc);		   
                if(m_sfc->type==1)
                    m_vol->data_type = 1;
            }
        }
    }
    

  if(m_sfc->polygon_point_vector.size()==0)
   m_sfc->PolygonPointVector();//CC
}

void CDialogVolume::OnBnClickedButtonSurface2Volume()
{
    //CGLVolume* gs_volume = NULL;
    //Surface *gs_surface = NULL;
    // Get the selected volume
    if(m_strNameVolume.IsEmpty()) {
        AfxMessageBox("No Volume selected !");
        return;
    }
    // Display volume surface list
    m_LBVOLSurfaces.AddString(m_strSurfaceSelected);
}

void CDialogVolume::OnLbnSelchangeListSurfacePolylines()
{
	// TODO: Add your control notification handler code here
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CDialogVolume::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  //----------------------------------------------------------------------
  CDialog::OnActivate(nState, pWndOther, bMinimized);
  //----------------------------------------------------------------------
  
  //CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  //if (theApp->pVolume->GetSafeHwnd()!=NULL)
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsPointEditorOn = 0;
  mainframe->m_bIsPolylineEditorOn = 0;
  mainframe->m_bIsSurfaceEditorOn = 0;
  mainframe->m_bIsVolumeEditorOn = 1;
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CDialogVolume::OnOK() 
{   
  CDialog::OnOK();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsVolumeEditorOn = 0;
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CDialogVolume::OnCancel() 
{   
  CDialog::OnCancel();
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_bIsVolumeEditorOn = 0;
  //----------------------------------------------------------------------
}
// vol_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "vol_dlg.h"

// CDialogVolumeNew dialog

IMPLEMENT_DYNAMIC(CDialogVolumeNew, CDialog)
CDialogVolumeNew::CDialogVolumeNew(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogVolumeNew::IDD, pParent)
    , m_iType(FALSE)
{
}

CDialogVolumeNew::~CDialogVolumeNew()
{
}

void CDialogVolumeNew::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_VOL_LAYER, m_CB_Layer);
    DDX_Control(pDX, IDC_COMBO_VOL_MSH, m_CB_MSH);
    DDX_Control(pDX, IDC_COMBO_VOL_LAYER_SFC, m_CB_SFC);
    DDX_Control(pDX, IDC_LIST_VOL, m_LC);
    DDX_Radio(pDX, IDC_RADIO1, m_iType);
    DDX_Control(pDX, IDC_COMBO_VOL_MAT, m_CB_VOL2MAT);
}


BEGIN_MESSAGE_MAP(CDialogVolumeNew, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_VOL_CREATE, OnBnClickedButtonCreate)
    ON_CBN_SELCHANGE(IDC_COMBO_VOL_MSH, OnCbnSelchangeComboVolMsh)
    ON_CBN_SELCHANGE(IDC_COMBO_VOL_LAYER, OnCbnSelchangeComboVolLayer)
    ON_CBN_SELCHANGE(IDC_COMBO_VOL_LAYER_SFC, OnCbnSelchangeComboVolLayerSfc)
    ON_BN_CLICKED(IDC_BUTTON_VOL_WRITE_TEC, OnBnClickedButtonVolWriteTec)
    ON_CBN_SELCHANGE(IDC_COMBO_VOL_MAT, OnCbnSelchangeComboVolMat)
    ON_BN_CLICKED(IDC_BUTTON_VOL_MAT, OnBnClickedButtonVolMat)
    ON_BN_CLICKED(IDC_BUTTON_VOL_WRITE, OnBnClickedButtonVolWrite)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_VOL, OnLvnItemchangedListVol)
END_MESSAGE_MAP()


// CDialogVolumeNew message handlers

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
09/2005 OK Implementation
**************************************************************************/
BOOL CDialogVolumeNew::OnInitDialog() 
{
  int i;
  CString m_str;
  CString m_strLayer;
  //----------------------------------------------------------------------
  CDialog::OnInitDialog();
  //----------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL;
  m_CB_MSH.ResetContent();
  m_CB_MSH.InitStorage(10,80);
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    m_CB_MSH.AddString((CString)m_msh->pcs_name.data());
    m_CB_MSH.SetCurSel(i);
    m_strMSHName = m_msh->pcs_name.data();
  }
  //----------------------------------------------------------------------
  // MSH-LAY
  m_CB_Layer.ResetContent();
  m_CB_Layer.InitStorage(10,80);
  m_CB_Layer.AddString("ALL");
  if(m_msh){
    for(i=0;i<m_msh->no_msh_layer;i++){
      m_str = "LAYER";
      m_strLayer.Format("%i",i+1);
      m_str += m_strLayer;
      m_CB_Layer.AddString(m_str);
    }
  }
  m_CB_Layer.SetCurSel(0);
  m_CB_Layer.GetLBText(0,m_strLAYName);
  //----------------------------------------------------------------------
  // SFC
  Surface* m_sfc = NULL;
  m_CB_SFC.ResetContent();
  m_CB_SFC.InitStorage(10,80);
  vector<Surface*>::const_iterator p_sfc = surface_vector.begin();
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_CB_SFC.AddString((CString)m_sfc->name.data());
    ++p_sfc;
    m_CB_SFC.SetCurSel(0);
    m_CB_SFC.GetLBText(0,m_strSFCName);
  }
  //----------------------------------------------------------------------
  // MAT
  for(i=0;i<(int)mmp_vector.size();i++){
    m_CB_VOL2MAT.AddString((CString)mmp_vector[i]->name.data());
    m_CB_VOL2MAT.SetCurSel(0);
    m_CB_VOL2MAT.GetLBText(0,m_strMATName);
  }
  //----------------------------------------------------------------------
  // Table
  m_vol = NULL;
  CRect rect;
  CString m_strItem;
  // Table configuration
  m_LC.GetWindowRect(&rect);
  m_LC.SetTextBkColor(RGB(153,153,255));
  m_LC.SetTextColor(RGB(0,0,255));
   // pause the mouse with highlight or you can select it
   // so this will be the hot item.
  m_LC.EnableTrackingToolTips();
  m_LC.SetExtendedStyle (LVS_EX_TRACKSELECT|LVS_EX_UNDERLINEHOT|LVS_EX_GRIDLINES);//CC
  LV_ITEM lvitem;
  lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
  // Insert head line
  m_LC.DeleteColumn(0);
  m_LC.InsertColumn(0,"VOL",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(1);
  m_LC.InsertColumn(1,"VOL Type",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(2);
  m_LC.InsertColumn(2,"SFC",LVCFMT_LEFT,rect.Width()*1/5,0);
  m_LC.DeleteColumn(3);
  m_LC.InsertColumn(3,"MAT Group",LVCFMT_LEFT,rect.Width()*1/5,0);
  // Insert data
  UpdateList();
  //----------------------------------------------------------------------
  return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::OnBnClickedButtonCreate()
{
  UpdateData(TRUE);
  UpdateData(FALSE);
  string sub_string;
  //
  m_vol = new CGLVolume();
  m_vol->type = 0;
  m_vol->type_name = m_strLAYName;
  CString m_str;
  m_str.Format("%i",volume_vector.size());
  m_vol->name += m_str;
  Surface* m_sfc = NULL;
  m_sfc = GEOGetSFCByName((string)m_strSFCName);
  if(m_sfc)
    m_vol->surface_vector.push_back(m_sfc);
  m_vol->mat_group = (int)volume_vector.size();
  sub_string = m_vol->type_name.substr(5,string::npos);
  m_vol->layer = strtol(sub_string.c_str(),NULL,0);
  //
  volume_vector.push_back(m_vol);
  //
  UpdateList();
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::UpdateList()
{
  int listip = 0;
  string sfc_name;
  vector<Surface*>::const_iterator p_sfc; 
  Surface* m_sfc = NULL;
  m_LC.DeleteAllItems(); //OK
  vector<CGLVolume*>::const_iterator p_vol;
  p_vol = volume_vector.begin();
  while (p_vol!= volume_vector.end()){
    m_vol = *p_vol;
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvitem.pszText = (LPSTR)m_vol->name.data();
    lvitem.iItem = listip; //size of list
    lvitem.iSubItem = 0;
    m_LC.InsertItem(&lvitem);
    m_LC.SetItemText(listip,1,m_vol->type_name.data());
    p_sfc = m_vol->surface_vector.begin();
    while(p_sfc != m_vol->surface_vector.end()){
      m_sfc = *p_sfc;
      m_LC.SetItemText(listip,2,m_sfc->name.data());
      ++p_sfc;
    }
    m_LC.SetItemText(listip,3,m_vol->mat_group_name.data());
    listip++;
	++p_vol;
  }
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::OnCbnSelchangeComboVolMsh()
{
  m_CB_MSH.GetLBText(m_CB_MSH.GetCurSel(),m_strMSHName);
}

void CDialogVolumeNew::OnCbnSelchangeComboVolLayer()
{
  m_CB_Layer.GetLBText(m_CB_Layer.GetCurSel(),m_strLAYName);
  if(m_strLAYName.GetLength()<5)
    return;
  CString m_str = m_strLAYName.GetAt(5);
  int layer_selected = strtol(m_str,NULL,0);
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->m_strLAYName = m_strLAYName;
  //......................................................................
  // un-highlighted
  Surface* m_sfc = NULL;
  vector<Surface*>::const_iterator p_sfc; 
  p_sfc = surface_vector.begin();
  while(p_sfc!=surface_vector.end()) {
    m_sfc = *p_sfc;
    m_sfc->highlighted = false;
    ++p_sfc;
  }
  //......................................................................
  // Highlight layer surfaces
  vector<CGLVolume*>::const_iterator p_vol;
  p_vol = volume_vector.begin();
  while (p_vol!= volume_vector.end()){
    m_vol = *p_vol;
    if((m_vol->layer==layer_selected)&&(m_vol->surface_vector.size()>0)){
      p_sfc = m_vol->surface_vector.begin();
      m_sfc = *p_sfc;
      if(m_sfc)
        m_sfc->highlighted = true;
    }
	++p_vol;
  }
  //......................................................................
  // Redraw
  mainframe->OnDrawFromToolbar();

}

void CDialogVolumeNew::OnCbnSelchangeComboVolLayerSfc()
{
  m_CB_SFC.GetLBText(m_CB_SFC.GetCurSel(),m_strSFCName);
}

void CDialogVolumeNew::OnCbnSelchangeComboVolMat()
{
  m_CB_VOL2MAT.GetLBText(m_CB_VOL2MAT.GetCurSel(),m_strMATName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::OnBnClickedButtonVolWriteTec()
{
  MSHWriteVOL2TEC((string)m_strMSHName);
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
11/2005 CC Modification
**************************************************************************/
void CDialogVolumeNew::OnBnClickedButtonVolMat()
{
  UpdateData(TRUE);
  UpdateData(FALSE);
  //......................................................................
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int m_iSel = m_LC.GetNextSelectedItem(p);
  CString m_strGEOName = m_LC.GetItemText(m_iSel,0);
  m_vol = GEOGetVOL((string)m_strGEOName);
  CMediumProperties* m_mmp = NULL;
  m_mmp = MMPGet((string)m_strMATName);
  if(m_vol&&m_mmp){
    m_vol->mat_group_name = (string)m_strMATName;
  }

  if (m_iSel!=-1)
  {
  LV_ITEM lvitem;
  lvitem.mask = LVIF_STATE;		
  lvitem.stateMask = LVIS_DROPHILITED;
  lvitem.iItem= m_iSel;//size of list
  lvitem.iSubItem=0;
  lvitem.pszText = (LPSTR)m_vol->name.data();
  m_LC.SetItemText(m_iSel,3,m_vol->mat_group_name.data());
  m_LC.SetHotItem(8);
  }
  else
  {
   CWnd * pWnd = NULL;
   pWnd->MessageBox("Please select at first a volume to assign material properties!","Info", MB_ICONINFORMATION);
  }
  //......................................................................
  //UpdateList();
  //......................................................................
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::OnBnClickedButtonVolWrite()
{
  string file_name_path_base;
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(m_gsp)
    file_name_path_base = m_gsp->path + m_gsp->base;
  GEOWrite(file_name_path_base); //OK41
}

/**************************************************************************
GeoSys-GUI-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CDialogVolumeNew::OnLvnItemchangedListVol(NMHDR *pNMHDR, LRESULT *pResult)
{
  //LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  pNMHDR = pNMHDR;
  *pResult = 0;
  //......................................................................
  // Display layer surface
  POSITION p = m_LC.GetFirstSelectedItemPosition();
  int m_iSel = m_LC.GetNextSelectedItem(p);
  CString m_strGEOName = m_LC.GetItemText(m_iSel,0);
  m_vol = GEOGetVOL((string)m_strGEOName);
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
  if(m_vol&&m_vol->surface_vector.size()>0){
    p_sfc = m_vol->surface_vector.begin();
    m_sfc = *p_sfc;
    if(m_sfc)
      m_sfc->highlighted = true;
  }
  // Redraw
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
}
