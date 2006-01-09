// ViewView.h : interface of the CViewView class
//
#pragma once

#include "gs_graphics.h"

class CGeoSysFCTView : public CView
{
protected: // create from serialization only
	CGeoSysFCTView();
	DECLARE_DYNCREATE(CGeoSysFCTView)

// Attributes
public:
	CGeoSysDoc* GetDocument() const;
private:
  double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
  int width,height;
  double time_last,time_step_last,time_step_max,time_step_current;
  CGraphics* m_graphics_dlg;
  CFunction* m_fct;
// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CGeoSysFCTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnProperties();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // debug version in ViewView.cpp
inline CGeoSysDoc* CGeoSysFCTView::GetDocument() const
   { return reinterpret_cast<CGeoSysDoc*>(m_pDocument); }
#endif

