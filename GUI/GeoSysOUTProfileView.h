// GeoSysOUTProfileView.h : Schnittstelle der Klasse CGeoSysView
//
/////////////////////////////////////////////////////////////////////////////
#include "GeoSysZoomView.h"
#if !defined(AFX_GEOSYS_OUT_PROFILE_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
#define AFX_GEOSYS_OUT_PROFILE_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "rf_out_new.h"

class CGeoSysOUTProfileView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysOUTProfileView();
	DECLARE_DYNCREATE(CGeoSysOUTProfileView)

// Attribute
public:
	CGeoSysDoc* GetDocument();

// Operationen

	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysOUTProfileView)
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
	virtual ~CGeoSysOUTProfileView();
     virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int width;
    int height;
    int m_iDisplayOUTProfileProperties;
    bool m_bDisplayOUTProfileProperties;
    CString m_strQuantityName;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    CString m_strPolylineName;
    COutput* m_out;
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysOUTProfileView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_GEOSYS_OUT_PROFILE_VIEW_H__97114578_861B_4DC1_BFCC_9F025CEB7A4B__INCLUDED_)
