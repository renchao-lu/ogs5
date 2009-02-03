// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GeoSys.h"
#include "MainFrm.h"
#include ".\mainfrm.h"
#include "GeoSysDoc.h"
#include "gs_graphics.h"
#include "COGLPickingView.h"	//PCH
#include "afxdisp.h"

// GeoSys-GUI
#include "gs_output.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
    ON_COMMAND(ID_VIEW_TEMPORALPROFILES_CREATE, OnViewTemporalProfilesCreate)
    ON_COMMAND(ID_VIEW_SPATIALPROFILES_CREATE, OnViewSpatialProfilesCreate)
    ON_COMMAND(ID_VIEW_MAT_FUNCTIONS_CREATE, OnViewMaterialPropertiesCreate)
    ON_COMMAND(ID_VIEW_MAT_GROUPS_CREATE, OnViewMATGroupsCreate)
    ON_COMMAND(ID_VIEW_MSH_CREATE, OnViewMSHCreate)
    ON_COMMAND(ID_VIEW_GEO_CREATE, OnViewGEOCreate)
	ON_COMMAND(ID_VIEW_FEM_CREATE,OnViewFEMCreate)
    ON_COMMAND(ID_BUTTON_DRAW, OnDrawFromToolbar)
    ON_COMMAND(ID_VIEW_OUT2D_CREATE, OnViewOUT2DCreate)
    ON_COMMAND(ID_VIEW_TIM_CREATE, OnViewTIMCreate)
    ON_COMMAND(ID_3DVIEW_SPLITTEROGLCREATE,OnViewSplitterOGLCreate)
    ON_COMMAND(ID_3DVIEW_OGLCREATE,OnViewOGLCreate)
    ON_COMMAND(ID_VIEW_FCT_CREATE, OnViewFCTCreate)
    //PCH
    ON_COMMAND(ID_OPENGLVIEWFORPICKING_OPENUPOPENGLWINDOW,OnViewOGLPicking)
    ON_COMMAND(ID_VIEWCONTROL,OnViewControl)
    ON_COMMAND(ID_OPENGLVIEWFORPICKING_PICKINGCONTROLPANEL, OnPickingHandle)
    ON_COMMAND(ID_PICKEDPROPERTY, OnPickedProperty)
    ON_COMMAND(ID_POINT, OnPointDlg)
    ON_UPDATE_COMMAND_UI(ID_POINT, OnUpdatePointDlg) //OK
	ON_COMMAND(ID_POLYLINE, OnPolylineDlg)
    ON_UPDATE_COMMAND_UI(ID_POLYLINE, OnUpdatePolylineDlg) //OK
	ON_COMMAND(ID_SURFACE, OnSurfaceDlg)
    ON_UPDATE_COMMAND_UI(ID_SURFACE, OnUpdateSurfaceDlg) //OK
	ON_COMMAND(ID_VOLUME, OnVolumeDlg)
    ON_UPDATE_COMMAND_UI(ID_VOLUME, OnUpdateVolumeDlg) //OK
	//ON_COMMAND(ID_MSH, OnMSHView) NW
	//ON_COMMAND(ID_PCS, OnPCSView) NW
	ON_COMMAND(ID_CONTROL_PANEL, OnControlPanel) //OK
	ON_COMMAND(ID_GEO, OnGEOView)
	ON_COMMAND(ID_SELECT, OnSelectInPicking) //PCH
	ON_COMMAND(ID_DESELECT, OnDeselectInPicking)
	ON_COMMAND(ID_SELECTALL, OnSelectAllInPicking)
	ON_COMMAND(ID_DESELECTALL, OnDeselectAllInPicking)
	ON_COMMAND(ID_RUN, OnRUN) //OK
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
  m_bIsGEOViewOpen = false; //OK
  m_bIsMSHViewOpen = false; //OK
  m_bIsPointEditorOn = 0; //OK
  m_bIsPolylineEditorOn = 0; //OK
  m_bIsSurfaceEditorOn = 0; //OK
  m_bIsVolumeEditorOn = 0; //OK
  dist_geo_object = -1;//TK
  m_bIsControlPanelOpen = false; //OK

//IsoLines/Surfaces----by haibing
for (int i=0;i<1000;i++)
{
	IsoValueArray[i] = 0;
	IsoColorArray[i] = RGB(0,0,0);
	IsoWidthArray[i] = 1.0;
	IsoFrameColorArray[i] = RGB(0,0,0);
}
	Iso_If_Show_Iso = false;
	Iso_If_SwitchOff_ContourPlot = false;
	Iso_Type = 0;//0-static, 1-dynamic
	Iso_Form = 0;//0-lines, 1-surfaces
	Iso_Count = 0;
	Iso_Max_Value = 0.0;
	Iso_Min_Value = 0.0;
	Iso_Step_Value = 0.0;
 //-------------------------------
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	// Create a window without min/max buttons or sizable border
   cs.style = WS_OVERLAPPED | WS_SYSMENU | WS_BORDER | WS_MAXIMIZE | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX; 

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnViewTemporalProfilesCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTOUT;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}


void CMainFrame::OnViewSpatialProfilesCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate = ((CGeoSysApp*) AfxGetApp())->m_pMDTOUTProfile;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =	pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

void CMainFrame::OnViewMaterialPropertiesCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTMAT;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: View for MAT groups
Programing:
01/2004 OK Implementation
**************************************************************************/
void CMainFrame::OnViewMATGroupsCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTMATGroups;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: Create MSH View
Programing:
01/2004 OK Implementation
07/2005 OK Create MSHView only once
01/2009 NW Comment out
**************************************************************************/
void CMainFrame::OnViewMSHCreate()
{
	return;

//  CMultiDocTemplate	*pDocTemplate ;
//  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
//  //----------------------------------------------------------------------
//  // Check wether a MSHView exists
//  if(m_bIsMSHViewOpen){ //OK
//    pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_MSH_VIEW);
//	if(pDocTemplate!=NULL)
//	{
//	}
//    return;
//  }
//  m_bIsMSHViewOpen = true;
//  //----------------------------------------------------------------------
//	CMDIChildWnd* pActiveChild = MDIGetActive();
//	CDocument* pDocument;
//	if (pActiveChild == NULL ||
//			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
//		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
//		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
//		return; // Befehl ist fehlgeschlagen
//	}
//	// Wenn nicht, haben wir einen neuen Rahmen
//	CDocTemplate* pTemplate =
//		((CGeoSysApp*) AfxGetApp())->m_pMDTMSH;
//	ASSERT_VALID(pTemplate);
//	CFrameWnd* pFrame =
//		pTemplate->CreateNewFrame(pDocument, pActiveChild);
//	if (pFrame == NULL) {
//		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
//		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
//		return; // Befehl ist fehlgeschlagen
//	}
//	pTemplate->InitialUpdateFrame(pFrame, pDocument);
//pTemplate->yesAlreadyOpen;
}

/**************************************************************************
GeoSysGUI-Method: 
Task:
Programing:
01/2004 OK Implementation
07/2005 OK Create GEOView only once
**************************************************************************/
void CMainFrame::OnViewGEOCreate()
{
  //----------------------------------------------------------------------
  // Check wether a GEOView exists
  if(m_bIsGEOViewOpen){ //OK
    return;
  }
  m_bIsGEOViewOpen = true;
  //----------------------------------------------------------------------
  CDocTemplate *pTempl;
  CDocument *pDoc;
  CView *pView;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  POSITION posT = theApp->GetFirstDocTemplatePosition();
  while(posT) 
  {
    pTempl = theApp->GetNextDocTemplate(posT);
	POSITION posD = pTempl->GetFirstDocPosition();
	while(posD) 
	{
  	  pDoc = pTempl->GetNextDoc(posD);
	  POSITION posV = pDoc->GetFirstViewPosition();
	  while(posV) 
	  {
		pView = pDoc->GetNextView(posV);
        // if a GEOView already exists return
	  } 
	}
  }
  theApp->m_pMDTGEO;
  //----------------------------------------------------------------------
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTGEO;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: Create FEM View
Programing:
01/2004 OK Implementation
**************************************************************************/
void CMainFrame::OnViewFEMCreate()
{
	return;

	//CMDIChildWnd* pActiveChild = MDIGetActive();
	//CDocument* pDocument;
	//if (pActiveChild == NULL ||
	//		(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
	//	TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Befehl ist fehlgeschlagen
	//}
	//// Wenn nicht, haben wir einen neuen Rahmen
	//CDocTemplate* pTemplate =
	//	((CGeoSysApp*) AfxGetApp())->m_pMDTSecond;
	//ASSERT_VALID(pTemplate);
	//CFrameWnd* pFrame =
	//	pTemplate->CreateNewFrame(pDocument, pActiveChild);
	//if (pFrame == NULL) {
	//	TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Befehl ist fehlgeschlagen
	//}
	//pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: Create OUT2D View
Programing:
06/2004 OK Implementation
**************************************************************************/
void CMainFrame::OnViewOUT2DCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate = ((CGeoSysApp*) AfxGetApp())->m_pMDTOUT2D;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}


/**************************************************************************
GeoSysGUI-Method: OnViewSplitterOGLCreate
Task: Create second OGL View
Programing:
01/2004 TK Implementation
**************************************************************************/
void CMainFrame::OnViewSplitterOGLCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTMain;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}


/**************************************************************************
GeoSysGUI-Method: OnViewOGLCreate
Task: Create second OGL View
Programing:
01/2004 TK Implementation
**************************************************************************/
void CMainFrame::OnViewOGLCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTOGL_GRAPH;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}
/**************************************************************************
GeoSysGUI-Method: OnViewOGLPicking
Task: Create third OGL View for picking
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CMainFrame::OnViewOGLPicking()
{
    // Make a connection to access some global variables
    //CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warning:  No active document\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Instruction failed
	}
	// If not, we have a new framework
	CDocTemplate* pTemplate =
		((CGeoSysApp*) AfxGetApp())->m_pMDTGLPICKINGWIN;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame =
		pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warning:  New framework could not be provided\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Instruction failed
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);

    // The following few lines will make my window maximized.
	// Maximize both Frame and Child Frame windows 
	CMDIFrameWnd *pFramePCH = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CMDIChildWnd *pChildPCH = (CMDIChildWnd *) pFrame->GetActiveFrame();
	pFramePCH->MDIMaximize( pChildPCH );

	// Let's get the next window
	CMDIChildWnd *pNextWin = (CMDIChildWnd *) pFrame->GetNextWindow();
	pNextWin->DestroyWindow();
	/*
    // Open up ViewControl Panel with the main OpenGL window initially.
    // This dialog can be killed any time.
    theApp->pViewControlDlg = new ViewControl;
	theApp->pViewControlDlg->Create(IDD_OGL_VIEW_CONTROL);
	theApp->pViewControlDlg->ShowWindow(SW_SHOW);

    // I am going to open up object handling panel as a default, too.
    theApp->pPickHandleDlg = new PickingHandle;
	theApp->pPickHandleDlg->Create(IDD_PICKINGHANDLE);
	theApp->pPickHandleDlg->ShowWindow(SW_SHOW);
	*/

}


/**************************************************************************
GeoSysGUI-Method: OnViewControl
Task: Create a dialog to control view for picking
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CMainFrame::OnViewControl()
{
    // Make a connection to access some global variables
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	if(theApp->pViewControlDlg->GetSafeHwnd() == NULL) 
	{
		
		theApp->pViewControlDlg = new ViewControl;
		theApp->pViewControlDlg->Create(IDD_OGL_VIEW_CONTROL);
		theApp->pViewControlDlg->ShowWindow(SW_SHOW);
	}
	else
		theApp->pViewControlDlg->ShowWindow(SW_SHOW);
}

/**************************************************************************
GeoSysGUI-Method: OnPickingHandle
Task: Create a dialog to control picking in various fashions
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CMainFrame::OnPickingHandle()
{
    // Make a connection to access some global variables
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	if(theApp->pPickHandleDlg->GetSafeHwnd() == NULL) 
	{
		
		theApp->pPickHandleDlg = new PickingHandle;
		theApp->pPickHandleDlg->Create(IDD_PICKINGHANDLE);
		theApp->pPickHandleDlg->ShowWindow(SW_SHOW);
	}
	else
		theApp->pPickHandleDlg->ShowWindow(SW_SHOW);
}


/**************************************************************************
GeoSysGUI-Method: OnPickedProperty
Task: Create a dialog to access and update properties picked.
Programing:
03/2005 PCH Implementation
**************************************************************************/
void CMainFrame::OnPickedProperty()
{
    // Make a connection to access some global variables
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	if(theApp->pPickedProperty->GetSafeHwnd() == NULL) 
	{
		
		theApp->pPickedProperty = new PickedProperties;
		theApp->pPickedProperty->Create(IDD_PICKEDPROPERTY);
		theApp->pPickedProperty->ShowWindow(SW_SHOW);
	}
	else
		theApp->pPickedProperty->ShowWindow(SW_SHOW);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
06/2005 CC Implementation
06/2005 OK Create GEOView
07/2005 CC g_graphics_modeless_dlg
**************************************************************************/
void CMainFrame::OnPointDlg()
{
  //----------------------------------------------------------------------
  // Set GEO DLG flags
  if(m_bIsPointEditorOn){
     
     m_bIsPointEditorOn = 0;

}
  else{    //CC
     m_bIsPolylineEditorOn = 0;
     m_bIsSurfaceEditorOn = 0;
     m_bIsVolumeEditorOn = 0;
     m_bIsPointEditorOn = 1;
}
 //----------------------------------------------------------------------
  // Modeless PNT dialog
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->OnCreatePoint();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!=NULL) //CC
    theApp->g_graphics_modeless_dlg->m_iDisplayPNT = true;//CC
  if (theApp->pPolylinenew->GetSafeHwnd()!=NULL)//CC9999
   theApp->pPolylinenew->add_polyline = false;
  //----------------------------------------------------------------------
  // Create GEOView
  OnViewGEOCreate();
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnUpdatePointDlg(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_bIsPointEditorOn);
}


/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2005 PCH Implementation
06/2005 OK Create GEOView
07/2005 OK m_bIsPolylineEditorOn
**************************************************************************/
void CMainFrame::OnPolylineDlg()
{
  //----------------------------------------------------------------------
  if(m_bIsPolylineEditorOn)
    m_bIsPolylineEditorOn = 0;
  else{
    m_bIsPointEditorOn = 0;
    m_bIsPolylineEditorOn = 1;
    m_bIsSurfaceEditorOn = 0;
    m_bIsVolumeEditorOn = 0;
  }
  //----------------------------------------------------------------------
  // Create GEOView
  OnViewGEOCreate();
  //----------------------------------------------------------------------
  // Modeless dialog
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->OnCreatePolyline();
  //update draw point button
  //----------------------------------------------------------------------
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnUpdatePolylineDlg(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_bIsPolylineEditorOn);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2005 PCH Implementation
06/2005 OK Create GEOView
07/2005 CC add_polyline
07/2005 OK m_bIsSurfaceEditorOn
**************************************************************************/
void CMainFrame::OnSurfaceDlg()
{
  //----------------------------------------------------------------------
  if(m_bIsSurfaceEditorOn)
    m_bIsSurfaceEditorOn = 0;
  else{
    m_bIsPointEditorOn = 0;
    m_bIsPolylineEditorOn = 0;
    m_bIsSurfaceEditorOn = 1;
    m_bIsVolumeEditorOn = 0;
  }
  //----------------------------------------------------------------------
  // Create GEOView
  OnViewGEOCreate();
  //----------------------------------------------------------------------
  // Modeless dialog
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->pPolylinenew->GetSafeHwnd()!= NULL) //CC9999
    theApp->pPolylinenew->add_polyline = false;// CC
  theApp->OnCreateSurface();
  //----------------------------------------------------------------------
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnUpdateSurfaceDlg(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_bIsSurfaceEditorOn);
}


/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2005 PCH Implementation
06/2005 OK Create GEOView
07/2005 OK m_bIsVolumeEditorOn
**************************************************************************/
void CMainFrame::OnVolumeDlg()
{
  //----------------------------------------------------------------------
  if(m_bIsVolumeEditorOn)
    m_bIsVolumeEditorOn = 0;
  else{
    m_bIsPointEditorOn = 0;
    m_bIsPolylineEditorOn = 0;
    m_bIsSurfaceEditorOn = 0;
    m_bIsVolumeEditorOn = 1;
  }
  //----------------------------------------------------------------------
  // Create GEOView
  OnViewGEOCreate();
  //----------------------------------------------------------------------
  // Modeless dialog
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  theApp->OnCreateVolume();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnUpdateVolumeDlg(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(m_bIsVolumeEditorOn);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: Create TIM View
Programing:
01/2004 OK Implementation
**************************************************************************/
void CMainFrame::OnViewTIMCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate = ((CGeoSysApp*) AfxGetApp())->m_pMDT_TIM;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

/**************************************************************************
GeoSysGUI-Method: 
Task: redraw all the views
Programing:
03/2004 CC Implementation
**************************************************************************/
void CMainFrame::OnDrawFromToolbar() 
{
  CDocTemplate *pTempl;
  CDocument *pDoc;
  CView *pView;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  POSITION posT = theApp->GetFirstDocTemplatePosition();
  while(posT) 
  {
    pTempl = theApp->GetNextDocTemplate(posT);
	POSITION posD = pTempl->GetFirstDocPosition();
	while(posD) 
	{
  	  pDoc = pTempl->GetNextDoc(posD);
	  POSITION posV = pDoc->GetFirstViewPosition();
	  while(posV) 
	  {
		pView = pDoc->GetNextView(posV);
		pView->Invalidate();
	  } 
	}
  }
}

/**************************************************************************
GeoSysGUI-Method: 
Task: Progress bar
Programing:
09/2004 RN/OK Implementation
**************************************************************************/
void CMainFrame::OnProgressBar() 
{
  RECT MyRect;
  m_wndStatusBar.GetItemRect(1,&MyRect);
  if(m_ProgressBar.m_hWnd==NULL){
    m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_wndStatusBar,1);
	m_ProgressBar.SetRange(0,100);
	m_ProgressBar.SetStep(1);
	for(int i=0;i<100;i++){
      Sleep(40);
	  m_ProgressBar.StepIt();
    }
	//m_ProgressBar.DestroyWindow();
  }
}
/**************************************************************************
GeoSysGUI-Method:
Task:
Programing:
02/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnViewFCTCreate()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
			(pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warnung:  Kein aktives Dokument f? WindowNew vorhanden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	// Wenn nicht, haben wir einen neuen Rahmen
	CDocTemplate* pTemplate = ((CGeoSysApp*) AfxGetApp())->m_pMDT_FCT;
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL) {
		TRACE("Warnung:  Neuer Rahmen konnte nicht erstellt werden\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Befehl ist fehlgeschlagen
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}


/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
06/2005 OK Implementation
01/2009	NW Comment out
**************************************************************************/
void CMainFrame::OnMSHView()
{
  //----------------------------------------------------------------------
  // Create MSHView
  //OnViewMSHCreate();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
06/2005 OK Implementation
01/2009	NW Comment out
**************************************************************************/
void CMainFrame::OnPCSView()
{
  //----------------------------------------------------------------------
  // Create PCSView
  //OnViewFEMCreate();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
08/2005 CC Modification
**************************************************************************/
void CMainFrame::OnControlPanel()
{
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!m_bIsControlPanelOpen)
    theApp->CreateControlPanel();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CMainFrame::OnGEOView()
{
  //m_bIsGEOViewOpen = false;
  OnViewGEOCreate();
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void CMainFrame::OnSelectInPicking()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

	pView->OnSelectInPicking();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void CMainFrame::OnDeselectInPicking()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

	pView->OnDeselectInPicking();
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void CMainFrame::OnSelectAllInPicking()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

	pView->OnSelectAllInPicking();
}
/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void CMainFrame::OnDeselectAllInPicking()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

	pView->OnDeselectAllInPicking();
}

/**************************************************************************
GeoSysGUI-Method: 
Task: 
Programing:
04/2006 TK Implementation
**************************************************************************/
void CMainFrame::UpdateSpecificView(char *view_class_name, CDocument *m_pDoc)
{

 	POSITION position = m_pDoc->GetFirstViewPosition();
    while (position != NULL)
    {
        CView* pView = m_pDoc->GetNextView(position);
        CRuntimeClass* prt = pView->GetRuntimeClass();
        prt->m_lpszClassName;
        if ( strcmp( prt->m_lpszClassName, view_class_name)  == 0 )
        {
            pView->Invalidate(TRUE);
            
        }
    }
}

/**************************************************************************
GeoSysGUI-Method: 
08/2007 OK Implementation
**************************************************************************/
void CMainFrame::OnRUN()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->OnSimulatorForward();
}
