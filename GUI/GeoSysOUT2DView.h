// GeoSysOUTView.h : Schnittstelle der Klasse CGeoSysView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"
#include "GeoSysDoc.h"
// GeoLib
#include "geo_pnt.h"
#if !defined(GEOSYS_OUT_2D_VIEW)
#define GEOSYS_OUT_2D_VIEW

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGeoSysOUT2DView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysOUT2DView();
	DECLARE_DYNCREATE(CGeoSysOUT2DView)

// Attribute
public:
	CGeoSysDoc* GetDocument();
    
// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysOUT2DView)
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
	virtual ~CGeoSysOUT2DView();
    virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int width;
    int height;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    CString m_strQuantityName;
    int m_iDisplayMSH;
    BOOL m_bDisplayMSHQuad;
    BOOL m_bShowLabels;
    int m_iDisplayIsolines;
    int m_iDisplayIsosurfaces;
    int m_iDisplayObservaWells;
    int m_iDisplayPNT;
    int m_showaxis;
    BOOL m_Displaylegend;
    double m_dUmin,m_dUmax;
    int m_iQuantityIndex;
    int xpixel(double);
    int ypixel(double);
    double x_world(int);
    double y_world(int);
    void pixel2Real(CGLPoint* gl_point, CPoint point);
    int m_iTimeSelected;
    bool m_bTimeSelectedAll;
    int m_label_elements;
    CString m_strPCSName; //OK
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
     //CC Zoom function 05/2004
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
    // 06/2004
    afx_msg void OnDrawPoint();
    afx_msg void OnUpdateDrawPoint(CCmdUI* pCmdUI);

    afx_msg void OnOutputInterpolation();
    afx_msg void OnOutputExportwells();
    afx_msg void OnWellTable();
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysOUT2DView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(GEOSYS_OUT_2D_VIEW)
