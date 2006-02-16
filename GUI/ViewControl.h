#pragma once
#include "afxwin.h"


// ViewControl dialog

class ViewControl : public CDialog
{
	DECLARE_DYNAMIC(ViewControl)

public:
	ViewControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~ViewControl();
    void OnSelchangeModePublic();

// Dialog Data
	enum { IDD = IDD_OGL_VIEW_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    
	virtual BOOL OnInitDialog();
    void showChange(void);
    afx_msg void OnBnClickedNode();
    afx_msg void OnBnClickedElement();
    afx_msg void OnBnClickedBoundary();   
    afx_msg void OnBnClickedOriview();
    afx_msg void OnCbnSelchangeOglmode();

	DECLARE_MESSAGE_MAP()
public:
    
    CComboBox m_Mode;
    CString	m_strMode;
    
    afx_msg void OnBnClickedSurfaces();
    afx_msg void OnBnClickedVolumes();
};
// Tempary purpose
extern int mainPCH ( int argc, char *argv[] );
