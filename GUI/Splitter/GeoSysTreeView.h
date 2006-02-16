#if !defined(AFX_GEOSYSTREE_H__A9AA1905_9687_11D2_899F_0040055A6A93__INCLUDED_)
#define AFX_GEOSYSTREE_H__A9AA1905_9687_11D2_899F_0040055A6A93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeoSysTree.h : header file
//

#include <afxcview.h>

/////////////////////////////////////////////////////////////////////////////
// CGeoSysTree view

class CGeoSysTree : public CTreeView
{
public:
	CGeoSysTree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGeoSysTree)
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Attributes
public:
	HTREEITEM hItem; 

// Operations
public:
	CView* pOGLView;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeoSysTree)
	public:
	virtual void OnInitialUpdate();


	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeoSysTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CGeoSysTree)
	afx_msg BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelChanged(NMHDR * pNMHDR, LRESULT *pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CImageList m_TreeImages;
	void CreateLVData();

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOSYSTREE_H__A9AA1905_9687_11D2_899F_0040055A6A93__INCLUDED_)
