#if !defined(AFX_VIEWPROPERTYPAGE_H__F522AE58_ABC9_11D2_AABA_00105A0729C7__INCLUDED_)
#define AFX_VIEWPROPERTYPAGE_H__F522AE58_ABC9_11D2_AABA_00105A0729C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ViewPropertyPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewPropertyPage dialog

class CViewPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CViewPropertyPage)

// Construction
public:
	CViewPropertyPage();
	CViewPropertyPage(int nID);
	~CViewPropertyPage();

protected:
	BOOL	m_bEnabled;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CViewPropertyPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CViewPropertyPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual void	GetOriginalRect(CRect *pRect);
  virtual void  EnableControls(BOOL bEnable = TRUE);
	virtual BOOL	IsEnabled();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPROPERTYPAGE_H__F522AE58_ABC9_11D2_AABA_00105A0729C7__INCLUDED_)
