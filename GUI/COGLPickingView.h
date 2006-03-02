// OpenGLMDIView.h : interface of the COpenGLMDIView class for PICKING by PCH
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OpenGLMDIVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_OpenGLMDIVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GeoSysDoc.h"
#include "CGLBaseView.h"

#define BUFSIZE 4095

class COGLPickingView : public CGLBaseView
{
protected: // create from serialization only
	COGLPickingView();
	DECLARE_DYNCREATE(COGLPickingView)

// Attributes
public:
	CGeoSysDoc* GetDocument();
	BOOL	m_bMouseRotate;
	
// Operations
public:
	void callToRedraw(void);
	void OriginalView(void);
    COLORREF m_ClearCol;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COGLPickingView)
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnCreateGL();
	void OnDrawGL(void);
	virtual ~COGLPickingView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// Chan-Hee
	BOOL m_bOldAllowRotate;

// Generated message map functions
protected:
	//{{AFX_MSG(COGLPickingView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags,short zDelta,CPoint point);
public:
	afx_msg void OnFileSaveMesh();
	afx_msg void OnFileSaveResult();
  	afx_msg void OnBackColour();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	int sceneselect;
	CPoint MouseDownPoint;
	double X_Angle;
	double Y_Angle;

	// Chan-Hee
	CPoint mousePoint;
	int hitsGLINode;
	int hitsRFINode;
	int hitsElement;
	int hitsBoundary;
    int hitsSurface;
    int hitsVolume;

	// Symbol length
	double symbolLengthX;
	double symbolLengthY;
	double symbolLengthZ;
    double longest_axis;
    double ScaleFactor;

    // From Thomas
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

    // New FEM Sturcture
    CRFProcess* m_pcs;
    CFEMesh* m_msh;
    CElem* m_ele;

    // Something about post-processing
    double LengthOfSideOfelement;
    double WithNoZoomScale;

	// This is for getting rid of .rfi file
	vector<CGLPoint*> gli_points_vector_view;

	void DrawGLINode(void);
	void DrawRFINode(void);
	void DrawElement(void);
	void DrawBoundary(void);
    void DrawSurface(void);
    void DrawVolume(void);
	void DrawReference(void);
	void drawUnitElement(int i);
	void drawHittedUnitElement(int index);
	void drawUnitBoundary(int i);
    void drawUnitSurface(int i);
    void drawUnitVolume(int i);
	void DrawPostprocess(void);
	void drawColoredElement(int index);
	double computeCmax(void);

    void DrawVectorOnNode(void);
    void DrawParticles(void);
    double GetSquareOfMaximumOfMagnitudeOfVector(void);
    char* string2CharArrary(string aString);

	void DrawGLINodeScene(GLenum mode);
	void DrawRFINodeScene(GLenum mode);
	void drawElementScene(GLenum mode);
	void drawBoundaryScene(GLenum mode);
    void drawSurfaceScene(GLenum mode);
    void drawVolumeScene(GLenum mode);
	void drawReferenceScene(GLenum mode);
    void DrawParticleScene(GLenum mode);
	void mouseGLINode(int x, int y);
	void mouseRFINode(int x, int y);
	void mouseElement(int x, int y);
	void mouseBoundary(int x, int y);
    void mouseSurface(int x, int y);
    void mouseVolume(int x, int y);
	void processhitsGLINode(GLint hits, GLuint buffer[]);
	void processhitsRFINode(GLint hits, GLuint buffer[]);
	void processHitsElement(GLint hits, GLuint buffer[]);
	void processHitsBoundary(GLint hits, GLuint buffer[]);
    void processHitsSurface(GLint hits, GLuint buffer[]);
    void processHitsVolume(GLint hits, GLuint buffer[]);
    

    // PCH from Thomas
    void GetRFIMinMaxPoints();
    void GetMinMaxPoints();
    void GetMidPoint(); 
    // PCH
    void InitializeScalesForOpenGL();
    void AssiginSymbolLength();

public:
    CGLPoint ConvertScaleToOpenGL(CGLPoint real);
    CGLPoint ConvertScaleToOpenGL(double x, double y, double z);
	int numberOfGLIPoints();
	CGLPoint GetGLIPointByIndex(int i);

private:
	bool bSelectEnabled;
	bool bDeselectEnabled;

	afx_msg void OnSelectInPicking();
	afx_msg void OnDeselectInPicking();
	afx_msg void OnPickedProperty();
	afx_msg void OnSelectAllInPicking();
	afx_msg void OnDeselectAllInPicking();
	afx_msg void OnPolylineDlg();
	afx_msg void OnSurfaceDlg();
	afx_msg void OnVolumeDlg();

	afx_msg void OnSelectInPickingUpdate(CCmdUI *pCmdUI);
	afx_msg void OnDeselectInPickingUpdate(CCmdUI *pCmdUI);

	void OnStopPicking();

};

#ifndef _DEBUG  // debug version in OpenGLMDIView.cpp
inline COGLDoc* COGLPickingView::GetDocument()
   { return (COGLDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OpenGLMDIVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
