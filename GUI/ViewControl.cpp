// ViewControl.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "ViewControl.h"
#include ".\viewcontrol.h"
#include "COGLPickingView.h"

// Declare the static variables for combo box
static CString CMode[] = {"Rotate", "Zoom", "Move"}; 
// ViewControl dialog

IMPLEMENT_DYNAMIC(ViewControl, CDialog)
ViewControl::ViewControl(CWnd* pParent /*=NULL*/)
	: CDialog(ViewControl::IDD, pParent)
{
}

ViewControl::~ViewControl()
{
}

void ViewControl::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_OGLMODE, m_Mode);
}


BEGIN_MESSAGE_MAP(ViewControl, CDialog)
    ON_CBN_SELCHANGE(IDC_OGLMODE, OnCbnSelchangeOglmode)
    ON_BN_CLICKED(IDC_NODE, OnBnClickedNode)
    ON_BN_CLICKED(IDC_ELEMENT, OnBnClickedElement)
    ON_BN_CLICKED(IDC_BOUNDARY, OnBnClickedBoundary)

    ON_BN_CLICKED(IDC_ORIVIEW, OnBnClickedOriview)
    ON_BN_CLICKED(IDC_SURFACES, OnBnClickedSurfaces)
    ON_BN_CLICKED(IDC_VOLUMES, OnBnClickedVolumes)
END_MESSAGE_MAP()


// ViewControl message handlers

BOOL ViewControl::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Make an access to global variables as usual
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    // ComboBox Handling
    // Make Element mode as a default
	for(int i=0; i<3; ++i)
		m_Mode.AddString(CMode[i]);
	m_Mode.SetCurSel(1);
    // Set rotate mode and none-editing mode as a default
    theApp->mMouseRotateMode = TRUE;
	theApp->mMouseZoomMode = FALSE;
	theApp->mMouseMoveMode = FALSE;
	theApp->mEditMode = FALSE;
    
    // Check Node initially
	CheckRadioButton( IDC_NODE, IDC_BOUNDARY, IDC_NODE);
	theApp->GLINodeSwitch = 1;

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void ViewControl::OnCbnSelchangeOglmode()
{
    // TODO: Add your control notification handler code here
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();


	// Enable the mouse manuver mode
	GetDlgItem(IDC_OGLMODE)->EnableWindow();

#ifdef COMBO
	// Disable the selection and deselection mode
	GetDlgItem(IDC_SELECT_MODE)->EnableWindow(FALSE);
	GetDlgItem(IDC_DESELECT_MODE)->EnableWindow(FALSE);
	// Uncheck the other
	CheckDlgButton(IDC_EDIT_MODE, 0);
	CheckDlgButton(IDC_SELECT_MODE, 0);
	CheckDlgButton(IDC_DESELECT_MODE, 0);
	CheckDlgButton(IDC_CONTINUOUS, 0);
#endif

	switch( m_Mode.GetCurSel())
	{
	case 0:
		theApp->mMouseRotateMode = FALSE;
		theApp->mMouseZoomMode = FALSE;
		theApp->mMouseMoveMode = TRUE;

		theApp->mEditMode = FALSE;
		break;
	
	case 1:
		theApp->mMouseRotateMode = TRUE;
		theApp->mMouseZoomMode = FALSE;
		theApp->mMouseMoveMode = FALSE;

		theApp->mEditMode = FALSE;
		break;
	
	case 2:
		theApp->mMouseRotateMode = FALSE;
		theApp->mMouseZoomMode = TRUE;
		theApp->mMouseMoveMode = FALSE;

		theApp->mEditMode = FALSE;
		break;

	}	

}


void ViewControl::OnBnClickedNode()
{
    // TODO: Add your control notification handler code here
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    UpdateData(TRUE);

	if(IsDlgButtonChecked(IDC_NODE))
		theApp->GLINodeSwitch = 1;
	else 
		theApp->GLINodeSwitch = 0;

	showChange();
}

void ViewControl::OnBnClickedElement()
{
    // TODO: Add your control notification handler code here
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    
    UpdateData(TRUE);

	if(IsDlgButtonChecked(IDC_ELEMENT))
		theApp->ElementSwitch = 1;
	else
		theApp->ElementSwitch = 0;

	showChange();
}

void ViewControl::OnBnClickedBoundary()
{
    // TODO: Add your control notification handler code here
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    UpdateData(TRUE);

	if(IsDlgButtonChecked(IDC_BOUNDARY))
		theApp->PolylineSwitch = 1;
	else
		theApp->PolylineSwitch = 0;

	showChange();
}

void ViewControl::showChange()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
	pView->Invalidate();
}



void ViewControl::OnBnClickedOriview()
{
    // TODO: Add your control notification handler code here
    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

	pView->OriginalView();
}

void ViewControl::OnSelchangeModePublic()
{
    OnCbnSelchangeOglmode();   
}

void ViewControl::OnBnClickedSurfaces()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    UpdateData(TRUE);

	if(IsDlgButtonChecked(IDC_SURFACES))
		theApp->SurfaceSwitch = 1;
	else
		theApp->SurfaceSwitch = 0;

	showChange();
}

void ViewControl::OnBnClickedVolumes()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    UpdateData(TRUE);

	if(IsDlgButtonChecked(IDC_VOLUMES))
		theApp->VolumeSwitch = 1;
	else
		theApp->VolumeSwitch = 0;

	showChange();
}
