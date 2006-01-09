#if !defined(AFX_GEOSYSMDIFRAMES_H__07DE5383_C0DC_11D3_8FB3_0080AD4311B1__INCLUDED_)
#define AFX_GEOSYSMDIFRAMES_H__07DE5383_C0DC_11D3_8FB3_0080AD4311B1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER > 1000
// GeoSysMdiFrames.h : header file
//

#include "GeoSysVisualFx.h"


/////////////////////////////////////////////////////////////////////////////
// CSimpleViewFrame frame


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CTabSplitterFrame frame

class CTabSplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CTabSplitterFrame)
protected:
	CTabSplitterFrame();           // protected constructor used by dynamic creation

// Attributes
public:
  TVisualFramework m_Framework;
  //CMDIFrameWnd *pFrame;
  //CMDIChildWnd *pChild;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabSplitterFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabSplitterFrame();

	// Generated message map functions
	//{{AFX_MSG(CTabSplitterFrame)
	afx_msg void OnDestroy();
	afx_msg virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg virtual void CTabSplitterFrame::ActivateFrame(int nCmdShow);
    afx_msg void ActivateOGLView(int getset);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOSYSMDIFRAMES_H__07DE5383_C0DC_11D3_8FB3_0080AD4311B1__INCLUDED_)
