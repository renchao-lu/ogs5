// GeoSysGEOView.h : Schnittstelle der Klasse CGeoSysGEOView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"
#if !defined(AFX_GEOSYS_GEO_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
#define AFX_GEOSYS_GEO_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// GeoLib
#include "geo_pnt.h"
#include "geo_sfc.h"

class CGeoSysGEOView : public CGeoSysZoomView
{
friend class CGraphics;

protected: // Nur aus Serialisierung erzeugen
	CGeoSysGEOView();
	DECLARE_DYNCREATE(CGeoSysGEOView)

// Attribute
public:
	CGeoSysDoc* GetDocument();


// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysGEOView)
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
	virtual ~CGeoSysGEOView();
    virtual void OnInitialUpdate();
  
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CString m_strNamePoint;
    CString m_strNamePolyline;
    int width;
    int height;
    int m_iDisplayPNT;
    BOOL m_bDisplayPNTDouble;
    int m_iDisplayPLY;
    int m_iDisplaySFC;
    BOOL m_bDisplayTINs;
    BOOL m_bDisplayVOL;
    int m_iDisplayBC;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    int xpixel(double);
    int ypixel(double);
    double x_world(int);
    double y_world(int);
    bool add_geometry;
    long nSelPoint;
    BOOL m_bShowLabels;
    void pixel2Real(CGLPoint* gl_point, CPoint point);
    BOOL m_bDisplayPLYPoints;
    BOOL m_bDisplayPLYLines;
    CString m_strGEOFileBase; //without extension
    void CalcXYMinMax();
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
	afx_msg void OnGeoLibPoints();
	afx_msg void OnGeolibPolylines();
	afx_msg void OnGeoLibSurfaces();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
     //CC Zoom function 05/2004
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
    //CC 06/2004
    afx_msg void OnDrawPoint();
    afx_msg void OnUpdateDrawPoint(CCmdUI* pCmdUI);
    afx_msg void OnGeolibVolumes();
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysGEOView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif
  

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_GEOSYS_GEO_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
