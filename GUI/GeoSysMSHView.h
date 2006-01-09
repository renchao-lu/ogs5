// GeoSysMSHView.h : Schnittstelle der Klasse CGeoSysMSHView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"

#if !defined(AFX_GEOSYS_MSH_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
#define AFX_GEOSYS_MSH_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGeoSysMSHView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysMSHView();
	DECLARE_DYNCREATE(CGeoSysMSHView)

// Attribute
public:
	CGeoSysDoc* GetDocument();


// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysMSHView)
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
	virtual ~CGeoSysMSHView();
    virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int width;
    int height;
    int m_iDisplayPNT;
    int m_iDisplayPLY;
    int m_iDisplaySFC;
    int m_iDisplayMSH;
    int m_iDisplayMSHLine;
    int m_iDisplayMSHTri;
    BOOL m_bDisplayMaterialGroups;
    BOOL m_bDisplayMSHQuad;
    BOOL m_bDisplayMSHPris;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    void MSHAssignMATGroup2Elements();
    int xpixel(double);
    int ypixel(double);
    CString m_strFileNameBase;
    BOOL m_bDisplayPLYPoints;
    BOOL m_bDisplayPLYLines;
    BOOL m_bShowLabels;
    BOOL m_bDisplayNOD;
    BOOL m_bDisplayMSHDoubleElements;
    BOOL m_bMSHActive1D;
    BOOL m_bMSHActivateLine;
    BOOL m_bMSHActivateTri;
    BOOL m_bMSHActivateQuad;
    BOOL m_bMSHActivateTet;
    BOOL m_bMSHActivatePris;
    BOOL m_bMSHActivateHex;
    CString m_strPCSName;
    CString m_strMSHName;
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
    afx_msg void OnMshLibMatConnect();
	afx_msg void OnMeshlibPrisgen();
	afx_msg void OnMeshlibPrisgenMapping();
	afx_msg void OnMeshlibPrisgenVertDisc();
	afx_msg void OnMshLibTetgen();
    afx_msg void OnMshLibRefine();
	afx_msg void OnMshLibTriangulation();
    afx_msg void OnPrisgenDelete();
    afx_msg void OnMSHLibEditor();
    afx_msg void OnLineGenPolylines();
    afx_msg void OnLineGenTriangles();
    afx_msg void OnMSHLibHexGen();
    afx_msg void OnMSHLibLineGenFromQuads();
    //CC Zoom function
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
    afx_msg void OnQuadGenPolylines();
    afx_msg void OnQuadGenSurfaces();
    afx_msg void OnLineGenSurface();
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysMSHView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_GEOSYS_MSH_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
