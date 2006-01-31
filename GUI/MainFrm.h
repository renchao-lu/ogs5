// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__59CED130_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_MAINFRM_H__59CED130_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CProgressCtrl m_ProgressBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnViewTemporalProfilesCreate();
    afx_msg void OnViewSpatialProfilesCreate();
    afx_msg void OnViewMaterialPropertiesCreate();
    afx_msg void OnViewMATGroupsCreate();
    afx_msg void OnViewMSHCreate();
    afx_msg void OnViewGEOCreate();
    afx_msg void OnViewFEMCreate();
    afx_msg void OnDrawFromToolbar();
    afx_msg void OnViewOUT2DCreate();
    afx_msg void OnViewTIMCreate();
	afx_msg void OnViewSplitterOGLCreate();
	afx_msg void OnViewOGLCreate();
    afx_msg void OnViewFCTCreate();
    //PCH
    afx_msg void OnViewOGLPicking(); // Main OpenGL Window for picking
    afx_msg void OnViewControl();  // For Control Pannel Dialog
    afx_msg void OnPickingHandle(); // For picking handling dialog
    afx_msg void OnPickedProperty(); // For picking handling dialog
    afx_msg void OnPointDlg();  //CC 06/2005
    afx_msg void OnUpdatePointDlg(CCmdUI* pCmdUI); //OK
	afx_msg void OnPolylineDlg();
    afx_msg void OnUpdatePolylineDlg(CCmdUI* pCmdUI); //OK
	afx_msg void OnSurfaceDlg();
    afx_msg void OnUpdateSurfaceDlg(CCmdUI* pCmdUI); //OK
	afx_msg void OnVolumeDlg();
    afx_msg void OnUpdateVolumeDlg(CCmdUI* pCmdUI); //OK
    afx_msg void OnGEOView(); //OK
    afx_msg void OnMSHView(); //OK
    afx_msg void OnPCSView(); //OK
    afx_msg void OnControlPanel(); //OK
 
    //PCH ends
    void OnProgressBar();
    int m_rebuild_formtree;
    int m_something_changed;
    int m_3dcontrol_points;
    int m_3dcontrol_lines;
    int m_3dcontrol_polylines;
    int m_3dcontrol_surfaces;
    int m_3dcontrol_volumes;
    int m_3dcontrol_nodes;
    int m_3dcontrol_elements;
    int m_3dcontrol_matgroups;
    int m_3dcontrol_double_points;
    int m_bounding_box;
    int m_3dcontrol_pcs;
    int m_3dcontrol_sourcesink;
    int m_3dcontrol_msh_quality;
    int m_3dcontrol_mesh_quality_type;
    int m_3dcontrol_mesh_quality_graph;
    int m_3dcontrol_point_numbers;
    int m_selected_wire_frame;
    int m_x_value_color;
    int m_y_value_color;
    int m_z_value_color;
    CString m_pcs_name;
    int m_3dcontrol_bc;
    double m_tolerancefactor;
    double m_image_distort_factor_x;
    double m_image_distort_factor_y;
    double m_image_distort_factor_z;
    double quality_range_min;
    double quality_range_max; 
    double m_pcs_min;
    double m_pcs_max;
    double dist_geo_object;
    // View control
    bool m_bIsGEOViewOpen; //OK
    bool m_bIsMSHViewOpen; //OK
    int m_bIsPointEditorOn; //OK
    int m_bIsPolylineEditorOn; //OK
    int m_bIsSurfaceEditorOn; //OK
    int m_bIsVolumeEditorOn; //OK
    CString m_strLAYName; //OK
    bool m_bModifiedGEOView; //OK
    bool m_bIsControlPanelOpen; //OK
public:
//--- Excel Data-Exchange ---
    CString m_fileopen;//JG
    CString m_strDBTypeName;//JG
    bool dataedit;//JG
    COleSafeArray saEdt;//JG
    int m_iSelectedMMPGroup;//JG 
    bool dataupdate;//JG
    bool datanew;//JG
//---------------------------
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__59CED130_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
