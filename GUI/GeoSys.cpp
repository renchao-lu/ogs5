// GeoSys.cpp : RockFlow/RockMech / GeoLib and MeshLib / Graphical User Interface
// Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OGLControl.h"
#include "GeoSys.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "GeoSysDoc.h"
// GeoSys Views
#include "OGLEnabledView.h"
#include "OGLView.h"
#include "OGLGraphView.h"
#include "GeoSysPCSView.h"
#include "GeoSysMATView.h"
#include "GeoSysOUTView.h"
#include "GeoSysOUTProfileView.h"
#include "GeoSysMATGroupsView.h"
#include "GeoSysMSHView.h"
#include "GeoSysGEOView.h"
#include "GeoSysMdiFrames.h"
#include "GeoSysVisualFx.h"
#include "GeoSysOUT2DView.h"
#include "GeoSysTIMView.h"
#include "GeoSysTreeView.h"
#include "GeoSysListView.h"
#include "GeoSysEditView.h"
#include "GSForm3DLeft.h"
#include "GeoSysFCTView.h"
// PCH
#include "COGLPickingView.h"
// PCSLib
#include "rf_ic_new.h"
#include "rf_st_new.h"
#include "rf_bc_new.h"
#include "gs_project.h"
// MSHLib
#include "msh_lib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long DocViewInteractionValue = 0;
TVisualObject *TranferView;
/////////////////////////////////////////////////////////////////////////////
// CGeoSysApp

BEGIN_MESSAGE_MAP(CGeoSysApp, CWinApp)
	//{{AFX_MSG_MAP(CGeoSysApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysApp construction

/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
0?/2005 PCH Here initialize some dialog pointer
07/2005 OK/TK APL file path
**************************************************************************/
CGeoSysApp::CGeoSysApp()
{
  // Place all significant initialization in InitInstance
  //......................................................................
  // PCH: Here initialize some dialog pointer
  pViewControlDlg = NULL;
  pPickHandleDlg = NULL;
  pPickedProperty = NULL;
  pPoint = NULL;
  pPolyline = NULL;
  pSurface = NULL;
  pVolume = NULL;
  m_bPickPLYPoints = false;
  ActiveDialogIndex = 0; // CC
  //......................................................................
  // APL file path //TK/OK
  char a = '\\';
  TCHAR szModule[_MAX_PATH]; 
  GetModuleFileName(NULL, szModule, _MAX_PATH); //TK/OK
  CString m_strAPL = szModule;
  int pos = m_strAPL.ReverseFind(a);
  m_strAPLFilePath = m_strAPL.Left(pos+1);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeoSysApp object

CGeoSysApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGeoSysApp initialization

BOOL CGeoSysApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// --- initialize OLE-Libraries ---
	if (!AfxOleInit())
	{
		AfxMessageBox("Cannot initialize COM");
		return FALSE;
	}
	AfxEnableControlContainer();
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),
		RUNTIME_CLASS(CTabSplitterFrame), // custom MDI child frame = CChildFrame
		RUNTIME_CLASS(COGLView));
	AddDocTemplate(pDocTemplate);
    // Store this template so that we can refer to it.
	m_pMDTMain = pDocTemplate ;

    // Now create and store the second template.
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CGeoSysView));	// Second view.
    // Note that we do not add this second doc template to the app - if we did, it 
    // would have two to choose from and would not know which to use when creating 
    // a new document - so it would ask the user. Aaaarrrggghhh!!!
    // However, we must now explicitly delete this doc template in ExitInstance(), 
    // or it will be a memory leak.
	m_pMDTSecond = pDocTemplate ;
	// The second view is created when the document first gets initialised, which 
    // is CDocument::OnNewDocument() - as both views share the same document, this 
    // only gets called once per pair of child frames.

    //-------------------------------------------------------------------------------
    // MATView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysMATView));	// MAT view.
	m_pMDTMAT = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // OUTView for temporal profiles
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysOUTView));	// OUT view.
	m_pMDTOUT = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // OUTView for spatial profiles
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysOUTProfileView));	// OUT view.
	m_pMDTOUTProfile = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // MATGroupsView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysMATGroupsView));	// MAT view.
	m_pMDTMATGroups = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // MSHView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysMSHView));	// MSH view.
	m_pMDTMSH = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // GEOView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysGEOView));	// MAT view.
	m_pMDTGEO = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // OUT2DView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysOUT2DView));	// OUT2D view.
	m_pMDTOUT2D = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // TIMView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysTIMView));	// OUT2D view.
	m_pMDT_TIM = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // Splittered OGL View
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(COGLView));	    // OGL view.
	m_pMDTOGL_NOSPLIT = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // FCTView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(CGeoSysFCTView));	// OUT2D view.
	m_pMDT_FCT = pDocTemplate ;
    //-------------------------------------------------------------------------------
    // OGLGRAPHView
	pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),		// Same document.
		RUNTIME_CLASS(CChildFrame),     // custom MDI child frame
		RUNTIME_CLASS(COGLGraphView));	// OUT2D view.
	m_pMDTOGL_GRAPH = pDocTemplate ;
    //-------------------------------------------------------------------------------
	//COGLPickingView  - PCH4105 OK, I'm going to create my OpenGL View Window here
    pDocTemplate = new CMultiDocTemplate(
		IDR_GEOSYSTYPE,
		RUNTIME_CLASS(CGeoSysDoc),
        //RUNTIME_CLASS(CChildFrame), // PCH I'll try CChildFrame first.
		RUNTIME_CLASS(CTabSplitterFrame),
		RUNTIME_CLASS(COGLPickingView));
	//AddDocTemplate(pDocTemplate);
    m_pMDTGLPICKINGWIN = pDocTemplate;
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	//if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTtnGetDispInfoCustom1(NMHDR *pNMHDR, LRESULT *pResult);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGeoSysApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
/**************************************************************************
GeoSys-Method: close and delete all the Geo dialogs
Task: 
Programing:
06/2005 CC 
**************************************************************************/
void CGeoSysApp::deleteMember()
{ 
    // test whether point editor is already open or not
    //destroy window will close the dialog window
    if (pPoint->GetSafeHwnd() != NULL )
    pPoint->DestroyWindow();
  //delete pPoint;
   // }
 
    if (pPolyline->GetSafeHwnd() != NULL )
    pPolyline->DestroyWindow();

        if (pSurface->GetSafeHwnd() != NULL )
    pSurface->DestroyWindow();
          if (pVolume->GetSafeHwnd() != NULL )
    pVolume->DestroyWindow();
  //delete pPolyline;
  //  }
  
}
/**************************************************************************
GeoSys-Method: updata list control box
Task: 
Programing:
06/2005 CC Implementation
07/2005 CC Modification check the active dialog index
**************************************************************************/
void CGeoSysApp::OnUpdataGeoListCtrl()
{ 
  // test whether point editor is already open or not
  if (pPoint->GetSafeHwnd() != NULL && ActiveDialogIndex == 1)
    pPoint->OnButtonPointPropertiesUpdate();
  if (pPolyline->GetSafeHwnd() != NULL && ActiveDialogIndex == 2){
    pPolyline->OnButtonPolylinePropertiesUpdate();
    pPolyline->OnMarkselectedPoint();//CC
  }
}
/////////////////////////////////////////////////////////////////////////////
// CGeoSysApp commands
CMultiDocTemplate * CGeoSysApp::GetDocTemplate(UINT uTemplate)
{
	CMultiDocTemplate	*pDocTemplate = NULL ;	// In case the index is bad.

	switch (uTemplate)
	{
// Definitions of template indexes are in this class' header file.
	    case DOCTEMPLATE_MAINVIEW :
			pDocTemplate = m_pMDTMain ;
			break ;

		case DOCTEMPLATE_SECONDVIEW :
			pDocTemplate = m_pMDTSecond ;
			break ;

		case DOCTEMPLATE_MAT_VIEW :
			pDocTemplate = m_pMDTMAT ;
			break ;

		case DOCTEMPLATE_OUT_VIEW :
			pDocTemplate = m_pMDTOUT ;
			break ;

		case DOCTEMPLATE_MAT_GROUPS_VIEW :
			pDocTemplate = m_pMDTMATGroups ;
			break ;

		case DOCTEMPLATE_MSH_VIEW :
			pDocTemplate = m_pMDTMSH ;
			break ;

		case DOCTEMPLATE_GEO_VIEW :
			pDocTemplate = m_pMDTGEO ;
			break ;

		case DOCTEMPLATE_OUT2D_VIEW :
			pDocTemplate = m_pMDTOUT2D ;
			break ;

		case DOCTEMPLATE_TIM_VIEW :
			pDocTemplate = m_pMDT_TIM ;
			break ;

        case DOCTEMPLATE_OGLSPLIT_VIEW  :
			pDocTemplate = m_pMDTOGL_NOSPLIT;
			break ;
        case DOCTEMPLATE_OGLGRAPH_VIEW  :
			pDocTemplate = m_pMDTOGL_GRAPH;
			break ;
		case DOCTEMPLATE_FCT_VIEW :
			pDocTemplate = m_pMDT_FCT ;
			break ;
		case DOCTEMPLATE_PICKING_VIEW :
			pDocTemplate = m_pMDTGLPICKINGWIN ;
			break ;
	}

	return pDocTemplate ;
}

int CGeoSysApp::ExitInstance() 
{
	delete m_pMDTSecond ;	// As it wasn't added to the app, it will not get 
							// automatically deleted when the app ends as the 
							// original will.
	delete m_pMDTMAT ;
	delete m_pMDTOUT ;
	delete m_pMDTMATGroups ;
	delete m_pMDTMSH ;
	delete m_pMDTGEO ;
	delete m_pMDTOUT2D ;
	delete m_pMDT_TIM ;
	delete m_pMDTOGL_NOSPLIT; 
    delete m_pMDTOGL_GRAPH;
	delete m_pMDT_FCT ;
	delete m_pMDTGLPICKINGWIN ;
	
	return CWinApp::ExitInstance();
}

/**************************************************************************
GeoSys-Method: DestroyOGLViewData
Task: 
Programing:
02/2004 TK project related file handling
**************************************************************************/
void DestroyOGLViewData() 
{
vector<CViewPoints*> view_points_vector;
vector<CViewLines*> view_lines_vector;
vector<CViewPolylines*> view_polylines_vector;
vector<CViewSurfaces*> view_surfaces_vector;
vector<CViewNodes*> view_nodes_vector;
vector<CViewElements*> view_elements_vector;
view_points_vector.clear();
view_lines_vector.clear();
view_polylines_vector.clear();
view_surfaces_vector.clear();
view_nodes_vector.clear();
view_elements_vector.clear();

DocViewInteractionValue = 1;
//view_counter=NULL;

}

/**************************************************************************
GeoSys-Method: DocView_Interaction
Task: Gibt eine 1 oder eine 0 zur?k jedachdem was als Aussage (
DocViewInteractionValue) gesetzt wurde Dannach wird
DocViewInteractionValue wieder auf 0 gesetzt
Programing:
02/2004 TK project related file handling
**************************************************************************/
int DocView_Interaction() 
{
	if (DocViewInteractionValue == 1 )
	{
	  DocViewInteractionValue = 0;
      return 1;
	}
		
		
	else return 0;

}

/**************************************************************************
GeoSys-Method: DocView_Interaction
Task: Gibt ein den Zugriff auf OpenGL
02/2004 TK project related file handling
**************************************************************************/
TVisualObject *TVisualFramework:: OGLView_Access(CCreateContext* pContext, int pos) 
{	
    TVisualObject *OGLView = new TVisualObject(pos,0,1,pContext, RUNTIME_CLASS(COGLView),CSize((::GetSystemMetrics(SM_CXVIRTUALSCREEN)-215-215),0));

    return (OGLView);

}

TVisualObject *TVisualFramework:: PickingGLView_Access(CCreateContext* pContext, int pos) 
{	
    TVisualObject *PickingGLView = new TVisualObject(pos,0,1,pContext, RUNTIME_CLASS(COGLPickingView),CSize((::GetSystemMetrics(SM_CXVIRTUALSCREEN)-215-215),0));

    return (PickingGLView);

}
/**************************************************************************
GeoSys-Method: DocView_Interaction
Task: Externer Aufruf zur Darstellung der Punkte im OpenGL-View
Programing:
03/2004 TK project related file handling
**************************************************************************/
int Display_Control_OGLView(int sceneselect) 
{
	COGLView ogl;
	ogl.GetGLIPointsforView();
	ogl.sceneselect=sceneselect;
	
	return sceneselect;
}

/**************************************************************************
GeoSys-Method: 
Task: open the modeless point dialog
Programing:
06/2005 CC
**************************************************************************/
void CGeoSysApp::OnCreatePoint()
{	
   if(pPoint->GetSafeHwnd() == NULL) {

		
		pPoint = new CGSPoint;
		pPoint->Create(IDD_POINT);
		pPoint->ShowWindow(SW_SHOW);
	}
	else
		pPoint->ShowWindow(SW_SHOW);
}
/**************************************************************************
Task: This dialog handles picked points to create polyline
Programing:
03/2005 PCH gs_polyline dialog handling at the global variable level
**************************************************************************/
void CGeoSysApp::OnCreatePolyline()
{	
  if(pPolyline->GetSafeHwnd() == NULL) 
  {
	pPolyline = new CPolyline;
	pPolyline->Create(IDD_POLYLINE);
	pPolyline->ShowWindow(SW_SHOW);
  }
  else
	pPolyline->ShowWindow(SW_SHOW);
  m_bPickPLYPoints = true; //OK
}


/**************************************************************************
GeoSys-Method: 
Task: This dialog handles picked polylines to create surface
Programing:
04/2005 PCH 
**************************************************************************/
void CGeoSysApp::OnCreateSurface()
{	
	if(pSurface->GetSafeHwnd() == NULL) 
	{
		pSurface = new CGSSurface;
		pSurface->Create(IDD_SURFACE);
		pSurface->ShowWindow(SW_SHOW);
	}
	else
		pSurface->ShowWindow(SW_SHOW);
}

/**************************************************************************
GeoSys-Method: 
Task: This dialog handles picked surfaces to create volume
Programing:
04/2005 PCH 
09/2005 OK new dialog
**************************************************************************/
void CGeoSysApp::OnCreateVolume()
{	
/*
  if(pVolume->GetSafeHwnd() == NULL) 
  {
	pVolume = new CDialogVolume;
	pVolume->Create(IDD_VOLUME);
	pVolume->ShowWindow(SW_SHOW);
  }
  else
	pVolume->ShowWindow(SW_SHOW);
*/
  if(m_vol_dlg->GetSafeHwnd() == NULL) 
  {
	m_vol_dlg = new CDialogVolumeNew;
	m_vol_dlg->Create(IDD_VOLUME_NEW);
	m_vol_dlg->ShowWindow(SW_SHOW);
  }
  else
	m_vol_dlg->ShowWindow(SW_SHOW);
}

/**************************************************************************
GeoSys-Method: 
Task:
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGeoSysApp::CreateControlPanel()
{
  g_graphics_modeless_dlg = new CGraphics;
  //----------------------------------------------------------------------
  // BMP
  if(GSPGetMember("bmp"))
    g_graphics_modeless_dlg->m_bDisplayBMP = true;
  //----------------------------------------------------------------------
  // GEO
  if(gli_points_vector.size()>0) g_graphics_modeless_dlg->m_iDisplayPNT = true;
  if(polyline_vector.size()>0)     g_graphics_modeless_dlg->m_iDisplayPLY = true;//CC
  if(surface_vector.size()>0)      g_graphics_modeless_dlg->m_iDisplaySUF = true;//CC
  //if(volume_vector.size()>0)     g_graphics_modeless_dlg->m_iDisplayVOL = true;
  g_graphics_modeless_dlg->OnBnClickedButtonCalcMinMax();
  //----------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL; //OK
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    g_graphics_modeless_dlg->m_iDisplayMSH = true;
    m_msh = fem_msh_vector[i];
    switch(m_msh->ele_type){
      case 1: g_graphics_modeless_dlg->m_iDisplayMSHLine = true; break;
      case 2: g_graphics_modeless_dlg->m_bDisplayMSHQuad = true; break;
      case 3: g_graphics_modeless_dlg->m_bDisplayMSHHex = true; break;
      case 4: g_graphics_modeless_dlg->m_iDisplayMSHTri = true; break;
      case 5: g_graphics_modeless_dlg->m_bDisplayMSHTet = true; break;
      case 6: g_graphics_modeless_dlg->m_bDisplayMSHPris = true; break;
    }
  }
  //----------------------------------------------------------------------
  // PCS
  //......................................................................
  // IC
  if((int)ic_vector.size()>0){
    g_graphics_modeless_dlg->m_bDisplayIC = true;
    g_graphics_modeless_dlg->m_iDisplayIsolines = true;
    g_graphics_modeless_dlg->m_iDisplayIsosurfaces = true;
  }
  //......................................................................
  // BC
  if((int)bc_list.size()>0&&(int)bc_group_list.size()){
    g_graphics_modeless_dlg->m_iDisplayBC = true;
  }
  //......................................................................
  // ST
  if((int)st_vector.size()>0&&(int)st_group_list.size()>0){
    g_graphics_modeless_dlg->m_bDisplayST = true;
  }
  //----------------------------------------------------------------------
  g_graphics_modeless_dlg->Create(IDD_DIALOG_GRAPHICS);
  g_graphics_modeless_dlg->ShowWindow(SW_SHOW);
  g_graphics_modeless_dlg->UpdateData(FALSE); //OK
}
