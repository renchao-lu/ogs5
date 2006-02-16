#pragma once


// CGS_OGL_Tetra_View dialog

class CGS_OGL_Tetra_View : public CDialog
{
	DECLARE_DYNAMIC(CGS_OGL_Tetra_View)

public:
	CGS_OGL_Tetra_View(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGS_OGL_Tetra_View();

// Dialog Data
	enum { IDD = IDD_OGL_TETRA_VIEW };
	double m_min_quality_value;
	double m_max_quality_value;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
