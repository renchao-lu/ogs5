// GeoSysMATView.h : Schnittstelle der Klasse CGeoSysView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"
#if !defined(AFX_GEOSYS_MAT_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
#define AFX_GEOSYS_MAT_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gs_output.h"

class CGeoSysMATView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysMATView();
	DECLARE_DYNCREATE(CGeoSysMATView)

// Attribute
public:
	CGeoSysDoc* GetDocument();


// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysMATView)
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
	virtual ~CGeoSysMATView();
    virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int width;
    int height;
    int m_iDisplayMATProperties;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    int m_iFluidPhase;
    double m_dUmin,m_dUmax;
    COUTMaterialProperties* m_dlg_modeless;
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
    afx_msg void OnSolidPropertiesDensity();
    afx_msg void OnMatSolidProperties();

    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);

    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysMATView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_GEOSYS_MAT_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
