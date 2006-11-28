// GeoSys.h : main header file for the TESTGL application
//

#if !defined(AFX_TESTGL_H__59CED12C_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_TESTGL_H__59CED12C_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

// PCH
#include "ViewControl.h"
#include "PickingHandle.h"
#include "PickedProperties.h"
#include "PTValue.h"
#include "gs_pnt.h"
#include "gs_point_new.h" //CC12/2005
#include "gs_polyline.h"
#include "gs_polyline_new.h" //CC9999
#include "gs_sfc.h"
#include "gs_sfc_new.h"
#include "vol_dlg.h"
#include "gs_graphics.h"

/////////////////////////////////////////////////////////////////////////////
// CGeoSysApp:
// See GeoSys.cpp for the implementation of this class
//
#define DOCTEMPLATE_MAINVIEW	1
#define DOCTEMPLATE_SECONDVIEW	2
#define DOCTEMPLATE_MAT_VIEW	3
#define DOCTEMPLATE_OUT_VIEW	4
#define DOCTEMPLATE_MAT_GROUPS_VIEW	5
#define DOCTEMPLATE_MSH_VIEW	6
#define DOCTEMPLATE_GEO_VIEW	7
#define DOCTEMPLATE_OUT2D_VIEW	8
#define DOCTEMPLATE_TIM_VIEW	9
#define DOCTEMPLATE_OGLSPLIT_VIEW	10
#define DOCTEMPLATE_GRAPH_VIEW	11
#define DOCTEMPLATE_FCT_VIEW	12
#define DOCTEMPLATE_PICKING_VIEW	13
#define DOCTEMPLATE_OGLGRAPH_VIEW   14

class CGeoSysApp : public CWinApp
{
public:
	CMultiDocTemplate * GetDocTemplate(UINT uTemplate);
	CGeoSysApp();
    //CMultiDocTemplate * m_pMDTMain;
	CMultiDocTemplate * m_pMDTSecond;
	CMultiDocTemplate * m_pMDTOUT;
	CMultiDocTemplate * m_pMDTOUTProfile;
	CMultiDocTemplate * m_pMDTMAT;
	CMultiDocTemplate * m_pMDTMATGroups;
	CMultiDocTemplate * m_pMDTMSH;
	CMultiDocTemplate * m_pMDTGEO;
	CMultiDocTemplate * m_pMDTOUT2D;
	CMultiDocTemplate * m_pMDT_TIM;
	CMultiDocTemplate * m_pMDTOGL_NOSPLIT;
    CMultiDocTemplate * m_pMDTOGL_GRAPH;
	CMultiDocTemplate * m_pMDT_FCT;
    //PCH 4105 I'm adding one window pointer here for a window that is going to be used for picking
    CMultiDocTemplate * m_pMDTGLPICKINGWIN;

    // Dialog
    // Here some modaless dialogs
    ViewControl* pViewControlDlg; // View Control Dialog
    PickingHandle* pPickHandleDlg; // Picking handling dialog
    PickedProperties* pPickedProperty; // Dialog for internal database for picked items
    PTValue pPTValue;
    CGSPoint *pPoint; //06/2005 CC
    CGSPointnew *pPointnew; //CC9999
    CPolyline* pPolyline;
    CGSPolylinenew* pPolylinenew;//CC9999
    CGSSurface* pSurface;
    CGSSurfacenew* pSurfacenew;//CC9999
    CDialogVolume* pVolume;
    CDialogVolumeNew* m_vol_dlg; //OK
    bool m_bPickPLYPoints;
    int ActiveDialogIndex;// CC 07/2005
    CGraphics* g_graphics_modeless_dlg;
    void CreateControlPanel(); //OK
public:
    int m_3dview_points_on_off;
    // PCH
    int ObjectSwitch;  // 1: Node 2: Element 3: Polyline 4: Surface 5: Volume
	int GLINodeSwitch, RFINodeSwitch, ElementSwitch, PolylineSwitch, SurfaceSwitch, VolumeSwitch, ReferenceSwitch; // 1: On 0: Off
	int LineSwitch, SphereSwitch, DiskSwitch;
	int AngleSwitch;
	int DrawModeSwitch;
    int VelocitySwitch;
	int CrossroadSwitch;
    int ParticleSwitch;
	int GDebugSwitch;
	int PID;

    double mouseX, mouseY; // This can be done better way later. But for now, keep this way.
    double zoomRatio;
    BOOL mMouseRotateMode;
	BOOL mMouseZoomMode;
	BOOL mMouseMoveMode;
	BOOL mEditMode;
	BOOL mSelectMode;
	BOOL mDeselectMode;
	BOOL mContinuous;
    BOOL colorSwitch;

	int hitsNodeTotal;
	int hitsGLINodeTotal;
	int hitsRFINodeTotal;
	int hitsParticleTotal;
	int hitsElementTotal;
	int hitsPolylineTotal;
    int hitsSurfaceTotal;
    int hitsVolumeTotal;

	// By default, these are off
	int BothPrimaryVariable;
	int BCAndST;
	int NodeIndexOfElementSwitch;
	int ShowAllSwitch;
	// For the temparily store to calculate the correct number of hitsElementTotal
	int hitsElement;

	int	*GLInodePicked;
	int	*RFInodePicked;
	int	*ParticlePicked;
	int	*elementPicked;
	int	*boundaryPicked;
    int	*surfacePicked;
    int	*volumePicked;
	int *nodePickedTotal;
	int *GLInodePickedTotal;
	int *ParticlePickedTotal;
	int *RFInodePickedTotal;
	int *elementPickedTotal;
	int *polylinePickedTotal;
    int *surfacePickedTotal;
    int *volumePickedTotal;
    // Layer Display Switch
	BOOL layerAllSwitch;
	int layerNo;
    // End by PCH
    double m_dXmin,m_dXmax;
    double m_dYmin,m_dYmax;
    // Application path
    CString m_strAPLFilePath;
private:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeoSysApp)
	public:
    void deleteMember();//CC 06/2005
     void OnUpdateGeoListCtrl();//CC 06/2005
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGeoSysApp)
	afx_msg void OnAppAbout();
    afx_msg void OnCreatePoint();//CC 06/2005
    afx_msg void OnCreatePolyline();
	afx_msg void OnCreateSurface();
	afx_msg void OnCreateVolume();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CMultiDocTemplate * m_pMDTMain;
};

extern CGeoSysApp theApp;
extern void DestroyOGLViewData();
extern int DocView_Interaction();
extern int Display_Control_OGLView(int sceneselect);
//list <TVisualObject*> 

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTGL_H__59CED12C_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
