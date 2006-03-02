#pragma once

// CGS_Tolerance dialog

class CGS_Tolerance : public CDialog
{
	DECLARE_DYNAMIC(CGS_Tolerance)

public:
	CGS_Tolerance(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGS_Tolerance();

// Dialog Data
	enum { IDD = IDD_TOLERANCE };
	double m_tolerancefactor;


protected: 
	afx_msg void OnOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditTolerance();
};
