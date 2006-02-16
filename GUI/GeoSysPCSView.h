// GeoSysView.h : Schnittstelle der Klasse CGeoSysView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"
#if !defined(AFX_GEOSYSVIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
#define AFX_GEOSYSVIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "msh_nodes_rfi.h"

class CGeoSysView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysView();
	DECLARE_DYNCREATE(CGeoSysView)
	double	m_dXmin;
	double	m_dXmax;
	double	m_dYmin;
	double	m_dYmax;
    CString m_strNamePolyline;
    bool m_bDisplayIsosurfaces;
    int m_iQuantityIndex;
    double m_dUmin,m_dUmax;
	bool add_geometry;
    bool m_bDisplayMaterial;
    int xpixel(double);
    int ypixel(double);
    BOOL m_bShowLabels;
    CString m_strQuantityName;

// Attribute
public:
	CGeoSysDoc* GetDocument();
    CString m_strFileNameBase;

// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysView)
	public:
	virtual void OnDraw(CDC* pDC);  // überladen zum Zeichnen dieser Ansicht
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CGeoSysView();
    virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    void WorldCoordinateTransformation(RF_POINT* rf_point, CPoint point);
    //CPoint WindowsCoordinateTransformation(RF_POINT* rf_point);
    int width;
    int height;
    CString m_strNamePoint;
    CPoint m_Point0;
    CPoint m_Point1;
    //CPoint m_Point0Zoom;
    //CPoint m_Point1Zoom;
    bool m_bOnLButtonUp;
    //bool m_bClipping;
    //double m_dXMinZoom;
    //double m_dXMaxZoom;
    //double m_dYMinZoom;
    //double m_dYMaxZoom;
// PNT
    bool m_bViewPoints;
// PLY
    BOOL m_bDisplayPLYPoints;
    BOOL m_bDisplayPLYLines;
    BOOL m_bDisplayTINs;
    int m_iDisplayPLY;
	int m_iDisplaySFC;
    int m_iDisplayMSH;
    BOOL m_bDisplayVOL;
    int m_iDisplayBC;
    int m_iDisplayPNT;
    int m_iDisplayIsolines;
    int m_iDisplayIsosurfaces;
    bool m_bDisplayMAT;
    BOOL m_bDisplayST;
// PCS
    CString m_strPCSName;
    // MSH
    CString m_strMSHName;
// Generierte Message-Map-Funktionen
protected:
	//{{AFX_MSG(CGeoSysView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDrawFromToolbar();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSimulatorProcess();
	afx_msg void OnSimulatorTimeStepping();
	afx_msg void OnSimulatorBoundaryConditions();
	afx_msg void OnSimulatorInitialConditions();
	afx_msg void OnSimulatorSourceTerms();
	afx_msg void OnMaterialFluidDensity();
	afx_msg void OnSimulatorRunSimulation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
     //CC Zoom function 05/2004
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
    //CC ploting and table of attributes.
    afx_msg void OnDrawWell();
    afx_msg void OnUpdateDrawWell(CCmdUI* pCmdUI);
    afx_msg void OnWellTable();
    afx_msg void OnPloting();
    //CC
    afx_msg void OnProperties();
    afx_msg void OnMFPEditor();
    afx_msg void OnMSPEditor();
    afx_msg void OnMMPEditor();
    afx_msg void OnPCSLibOUT();
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_GEOSYSVIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
