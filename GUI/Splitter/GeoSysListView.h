#if !defined(AFX_GEOSYSLIST_H__A9AA1904_9687_11D2_899F_0040055A6A93__INCLUDED_)
#define AFX_GEOSYSLIST_H__A9AA1904_9687_11D2_899F_0040055A6A93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeoSysList.h : header file
//

#include <afxcview.h>

/////////////////////////////////////////////////////////////////////////////
// CGeoSysList view

class CGeoSysList : public CListView
{
public:
	CGeoSysList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGeoSysList)

// Attributes
public:
// Operations
public:
	afx_msg void FillList(int sceneselect); 
	int sceneselect;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeoSysList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeoSysList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
public:
	//{{AFX_MSG(CGeoSysList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAreaQualityList();
	afx_msg void OnAngleQualityList();
	afx_msg void OnLengthQualityList();
	afx_msg void OnTetraVolumeQualityList();
	afx_msg void OnTetraAngleQualityList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOSYSLIST_H__A9AA1904_9687_11D2_899F_0040055A6A93__INCLUDED_)
