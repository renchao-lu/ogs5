// OGLGraphView.h : interface of the COGLGraphView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OGLGRAPHVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_OGLVIEW_H__59CED136_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



class COGLGraphView : public COGLEnabledView
{
public: // create from serialization only
	COGLGraphView();
	DECLARE_DYNCREATE(COGLGraphView)

// Attributes
public:
	
	CGeoSysDoc* GetDocument();

   	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void On3DControl();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
     

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COView)

	//}}AFX_VIRTUAL

// Implementation
public:
	void OnCreateGL();
	void OnDrawGL();
	virtual ~COGLGraphView();
 	void SetClearCol(COLORREF rgb);
   	void DrawOGLGraphAxis();
    void DrawQualityFactor();
    void GetGraphMinMax();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
  	double x_graph_min,x_graph_max,y_graph_min,y_graph_max;
	double norm_max_fac;
    CRect lpRect;
    CRect preRect;
    int init_view_check;

	
// Generated message map functions
public:


    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

public:


};



#ifndef _DEBUG  // debug version in OGLGraphView.cpp
inline COGLDoc* COGLGraphView::GetDocument()
   { return (COGLDoc*)m_pDocument; };
#endif

class CViewElementQuality
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
