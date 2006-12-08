#if !defined(AFX_VIEWPROPERTYSHEET_H__C193DDD2_9033_11D2_B983_00105A072D03__INCLUDED_)
#define AFX_VIEWPROPERTYSHEET_H__C193DDD2_9033_11D2_B983_00105A072D03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ViewPropertySheet.h : header file
//

#define	PS_Y_OFFSET	3		// distance from bottom control to prop sheet


/////////////////////////////////////////////////////////////////////////////
// CViewPropertySheet

class CViewPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CViewPropertySheet)

// Construction
public:
	CViewPropertySheet();
	CViewPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CViewPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	~CViewPropertySheet();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewPropertySheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL	m_Center;
	CRect m_MinPageRect;
	CRect m_MaxPageRect;
	CRect m_rectOriginal;	// original, optimized size
  BOOL	m_bOKToLeaveTab;
  int		m_nLastActive;

	virtual BOOL	Create(CWnd* pParentWnd, DWORD dwStyle = (DWORD)-1, DWORD dwExStyle = 0);
	virtual void	AdjustPages();
	virtual void	CenterControls(BOOL bCenter = TRUE);
	virtual void	AllowPageChange(BOOL bAllowPageChange = TRUE);

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewPropertySheet)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnAdjust(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPROPERTYSHEET_H__C193DDD2_9033_11D2_B983_00105A072D03__INCLUDED_)
