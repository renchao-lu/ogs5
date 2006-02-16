#pragma once


// CGS_newproject dialog

class CGS_newproject : public CDialog
{
	DECLARE_DYNAMIC(CGS_newproject)

public:
	CGS_newproject(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGS_newproject();

// Dialog Data
	//{{AFX_DATA(CdemoDlg)
	enum { IDD = IDD_NEWPROJECT };
	CString		m_strFolderPath ;
    CString		m_strFolderPathBasic ;
	CString		m_strProjectName ;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CdemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	afx_msg void OnPRJBrowse();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangeProjectName();
};
