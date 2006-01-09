// ViewView.h : interface of the CViewView class
//


#pragma once


class CGeoSysTIMView : public CView
{
protected: // create from serialization only
	CGeoSysTIMView();
	DECLARE_DYNCREATE(CGeoSysTIMView)

// Attributes
public:
	CGeoSysDoc* GetDocument() const;
private:
  double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
  int width,height;
   double time_last,time_step_last,time_step_max,time_step_current;

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
	virtual ~CGeoSysTIMView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ViewView.cpp
inline CGeoSysDoc* CGeoSysTIMView::GetDocument() const
   { return reinterpret_cast<CGeoSysDoc*>(m_pDocument); }
#endif

