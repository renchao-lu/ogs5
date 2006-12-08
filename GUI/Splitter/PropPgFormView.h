// PropPgFormView.h : interface of the CPropPgFormView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHEETINVIEWVIEW_H__96C3F270_AD99_11D3_AB38_005004CF7A2A__INCLUDED_)
#define AFX_SHEETINVIEWVIEW_H__96C3F270_AD99_11D3_AB38_005004CF7A2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPgFormDoc.h"
#include "ViewPropertySheet.h"

class CPropPgFormView : public CFormView
{
public: // create from serialization only
	CPropPgFormView();
	CPropPgFormView(UINT nIDTemplate);
	CPropPgFormView(LPCTSTR lpszTemplateName);
	DECLARE_DYNCREATE(CPropPgFormView)

// Attributes
public:
	CPropPgFormDoc* GetDocument();

protected:
	CViewPropertySheet	m_PropSheet;
	CSize		m_sizeControl;			// extent of the template-based controls
	int			m_PS_Offset;				// offset of property sheet from bottom control on containing dialog

// Operations
public:
	virtual void	SetPropertySheetOffset(int iOffset = PS_Y_OFFSET, BOOL bRedraw = FALSE);
	virtual int		GetPropertySheetOffset();
	virtual void	ResizeForNewControl();

protected:
	virtual void	AdjustPropSheetPosition();
	virtual void	ResizeParentFrame();
	virtual CSize	GetMaxControl();				// gets control extent

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropPgFormView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropPgFormView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
protected:
	//{{AFX_MSG(CPropPgFormView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PropPgFormView.cpp
inline CPropPgFormDoc* CPropPgFormView::GetDocument()
   { return (CPropPgFormDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHEETINVIEWVIEW_H__96C3F270_AD99_11D3_AB38_005004CF7A2A__INCLUDED_)
