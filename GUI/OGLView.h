// OGLView.h : interface of the COGLView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OGLVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_OGLVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// GEOLib
#include "geo_pnt.h" 
#include "geo_lin.h" 

#include "IsoSurface.h"


class COGLView : public COGLEnabledView
{
public: // create from serialization only
	COGLView();
	DECLARE_DYNCREATE(COGLView)

// Attributes
public:
	
	CGeoSysDoc* GetDocument();

   	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void On3DControl();

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
    int m_3dcontrol_bc;
    int m_3dcontrol_pcs;
    int m_3dcontrol_sourcesink;
    int m_3dcontrol_msh_quality;
    int m_3dcontrol_mesh_quality_graph;
    int m_3dcontrol_point_numbers;
    int m_selected_wire_frame;
    int m_pcs_values_mesh;
    int m_element_numbers;
    int m_node_numbers;
    CString m_pcs_name;

    double m_tolerancefactor;
    double m_image_distort_factor_x;
    double m_image_distort_factor_y;
    double m_image_distort_factor_z;
    double m_pcs_min;
    double m_pcs_max;
    int m_x_value_color;
    int m_y_value_color;
    int m_z_value_color;
    int m_permeability_value_color;
    CString m_strQuantityName;
    CTreeCtrl* pCtrl;
    double pre_time;
    int move_distance;
    int df;



     

// Operations
public:
	COLORREF m_ClearCol;
	float m_RGB_Character[3];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COView)

	//}}AFX_VIRTUAL

// Implementation
public:
	void OnCreateGL();
	void OnDrawGL();
	virtual ~COGLView();

	double scale_x,scale_y,scale_z;
	double trans_x,trans_y, trans_z;
	double x, y, z;
	double max, min;
	double current_x,current_y,current_z;
	double x_min,x_max,y_min,y_max,z_min,z_max;
	double x_dist,  y_dist, z_dist, dist;
    double x_dist_mesh,  y_dist_mesh, z_dist_mesh;
	double x_graph_min,x_graph_max,y_graph_min,y_graph_max;
	double x_mid,y_mid,z_mid,x_total,y_total,z_total;
    double x_count1,x_count2,y_count1,y_count2,z_count1,z_count2;
    long points_vectorlength, lines_vectorlength, polylines_vectorlength, surface_vectorlength, volume_vectorlength;
	long polylinepoints_vectorlength, surfacepolyline_vectorlength, surfacepolylinepoints_vectorlength;
	long rfi_nodes_vectorlength, rfi_elements_vectorlength;
	long point1, point2, point_id;
    int zoom_control;
	double ClipSize;
	long colorsignal;
	double norm_max_fac;
    int graph_view_onoff;
	double quality_range_min, quality_range_max;
	void SetClearCol(COLORREF rgb);
	int dx,dy;    
    void ShowMeshBoundingBox();
    long boundingbox;
	double bc_x, bc_y, bc_z;
    CRect lpRect;
    CRect preRect;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	BOOL m_bDisplayMSHQuad;
	BOOL m_bDisplayMSHTet;
	BOOL m_bDisplayMSHHex;
	BOOL m_bDisplayMSHPris;
    int m_iDisplayMSH;
	int m_iDisplayMSHLine;
	int m_iDisplayMSHTri;
	int m_iDisplayIsosurfaces;
	int m_iDisplayIsolines;
    int m_iDisplayBC;
    double m_dUmin;
	double m_dUmax;

	    
	
// Generated message map functions
public:
	void OnSizeGL(int cx, int cy);
	void VideoMode(ColorsNumber &c,ZAccuracy &z,BOOL &dbuf);

	//{{AFX_MSG(COView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags,short zDelta,CPoint point);
	afx_msg void OnHelpOglinfo();
	afx_msg void OnEnvirFlat();
	afx_msg void OnEnvirLighting();
    afx_msg void OnColoredElevation();
	afx_msg void OnEnvirSmooth();
	afx_msg void OnStartOpenGLExamples();
    afx_msg void On3DViewPoints();
    afx_msg void On3DViewAddPoints();
	afx_msg void On3DViewLines();
	afx_msg void On3DViewPolylines();
	afx_msg void On3DViewSurfaces();
	afx_msg void On3DViewVolumes();
	afx_msg void On3DViewPointNumbers();
	afx_msg void On3DViewDoublePoints();	
	afx_msg void On3DViewPolylineNames();
	afx_msg void On3DViewRFINodes();
	afx_msg void On3DViewRFIElements();
	afx_msg void On3DViewFEMModel();
	afx_msg void On3DViewRFINodeNumbers();
	afx_msg void On3DViewRFIElementNumbers();
    afx_msg int ZoomAndMove(int switcher);
	afx_msg void GetMidPoint();
    afx_msg void GetMinMaxPoints();
	afx_msg void DisplayPoints();
	afx_msg void DisplayPointNumbers();
	afx_msg void DisplayDoublePoints();
	afx_msg void DisplayPolylineNames();
	afx_msg void DisplayNodeNumbers();
	afx_msg void DisplayElementNumbers();
	afx_msg void GetGLIPointsforView();
	afx_msg void GetGLILinesforView();
	afx_msg void GetGLIPolylinesforView();
	afx_msg void GetGLISurfacesforView();
    afx_msg void GetMSHMinMax();
	afx_msg void GetRFINodesforView();
	afx_msg void GetRFIElementsforView();
    afx_msg void GetRFIMinMaxPoints();
	afx_msg void InitializeOGLViewDataConstructs();
	afx_msg void OnUpdateEnvirLighting(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColoredElevation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEnvirFlat(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEnvirSmooth(CCmdUI* pCmdUI);
	afx_msg void OnUpdate3DViewPointNumbers(CCmdUI* pCmdUI);
	afx_msg void OnUpdate3DViewDoublePoints(CCmdUI* pCmdUI);
	afx_msg void OnUpdate3DViewPolylineNames(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdate3DViewRFINodeNumbers(CCmdUI* pCmdUI);
	afx_msg void OnUpdate3DViewRFIElementNumbers(CCmdUI* pCmdUI);
	afx_msg void OnUpdate3DViewPoints(CCmdUI* pCmdUI);
	afx_msg void OnMeshTest_Area();
	afx_msg void OnMeshTest_Angle();
	afx_msg void OnMeshTest_Length();
	afx_msg void OnMeshTest_Area_Graph();
    afx_msg void OnMeshTest_Angle_Graph();
	afx_msg void OnMeshTest_Length_Graph();
	afx_msg void OnBackColour();
	afx_msg void DrawOGLGraph();
	afx_msg void DrawOGLGraphAxis();
	afx_msg void DrawQualityFactor();
	afx_msg void OnMeshTest_Tetra_Volume();
	afx_msg void OnMeshTest_Tetra_Volume_Graph();
	afx_msg void OnMeshTest_Tetra_Angle();
	afx_msg void OnMeshTest_Tetra_Angle_Graph();
   	afx_msg void OnMeshGenerator_Delaunay();
	afx_msg void OnMeshImprovement();
    void DrawPoint(int,double*); //OK
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void Draw_LineWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_QuadWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_HexWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_TriWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_TetWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_PrismWireFrame(int msh_vector_position, int element_vector_position);
    void Draw_SelectedTriangles(int msh_vector_position, int element_vector_position);
    void Draw_SelectedQuads(int msh_vector_position, int element_vector_position);
	void Draw_Element_Numbers(int msh_vector_position, int element_vector_position); 
    void Draw_Node_Numbers(int msh_vector_position, int node_vector_position);
	double Get_Red_Value(double value_norm);
    double Get_Green_Value(double value_norm);
    double Get_Blue_Value(double value_norm);
    void UpdatePcsMinmax();
    void Arrange_and_Display();
	void DrawEllipseAroundSelection();

private:
	CGLDispList anothercube;
	CPoint MouseDownPoint;
	COGLControl trackball;

public:
	int sceneselect;
	GLUnurbsObj *m_ogl_surface;

// IsoLines/Surfaces----by Haibing 09-2006-------
	void SetIsoSurfaceCol(COLORREF m_color);
	void drawIsoSurface(std::vector<CIsoSurfaceTriangle*>* mTriangles);
	void drawIsoSurfaceFrame(std::vector<CIsoSurfaceTriangle*>* mTriangles);
	void drawIsoLine(std::vector<CIsoSurfaceLine*>* mIsoLines);
	void SetIsoLineWidth(float Width);
//-----------------------------------------------

    afx_msg void OnEnvironmentCharactercolor();
};



#ifndef _DEBUG  // debug version in OGLView.cpp
inline CGeoSysDoc* COGLView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; };
#endif

/////////////////////////////////////////////////////////////////////////////
///VECTORS FOR VIEW//////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CViewPoints
{
  public:
    double x;
    double y;
    double z;
    long pointnumber;
	double meshdensity;
    vector<CGLPoint*> view_points_vector;
};


class CViewLines
{
  public:
    double x1, x2;
    double y1, y2;
    double z1, z2;
    long linenumber;
	long pointnumber1;
	long pointnumber2;
    vector<CGLLine*> view_lines_vector;
};

class CViewPolylines
{
  public:
    double x;
    long polyline_id;
    double y;
    double z;
    CString polylinename;
    double plg_seg_min_def;
	vector<CGLPoint*> polyline_point_vector;


};

class CViewSurfaces
{
  public:
    long surface_id;
	double surface_midpoint_x, surface_midpoint_y, surface_midpoint_z;
    string surfacename;
	vector<CViewPolylines*> surface_polyline_vector;
	vector<CGLPoint*> surface_point_vector;


};


class CViewNodes
{
  public:
    double x;
    double y;
    double z;
    long nodenumber;
};

class CViewElements
{
  public:
    long element_type;
	long elementnumber;
	long materialnumber;
    double x1,x2,x3,x4,x5,x6,x7,x8;
    double y1,y2,y3,y4,y5,y6,y7,y8;
    double z1,z2,z3,z4,z5,z6,z7,z8;
    long nodenumber1, nodenumber2, nodenumber3, nodenumber4;
	long nodenumber5, nodenumber6, nodenumber7, nodenumber8;
	double quality_factor, area_quality, angle_quality;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OGLVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
