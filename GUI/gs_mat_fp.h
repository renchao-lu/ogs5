#pragma once

// CMaterialFluidDensity dialog

class CMaterialFluidDensity : public CDialog
{
	DECLARE_DYNAMIC(CMaterialFluidDensity)

public:
	CMaterialFluidDensity(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMaterialFluidDensity();

// Dialog Data
	enum { IDD = IDD_MAT_FP_DENSITY };
	CListBox	m_LBFluidPhases;
    CString m_strPhaseNumber;
    char char_string[80];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
    void Set(char* name);
    int m_iType;
    void Get(char* name);

	DECLARE_MESSAGE_MAP()
public:
    double m_dXmin;
    double m_dXmax;
    afx_msg void OnButtonMatFPCalc();
    void Calc();
    double m_dDensityMin;
    double m_dDensityMax;
    int m_iFluidPhase;
private:
    double m_dDensityRef;
public:
    double m_dCompressibility;
    double m_dExpansivityC;
    double m_dExpansivityT;
    afx_msg void OnOk();
    afx_msg void OnSelchangeListMatFPDensity();
    double m_dYmin;
    double m_dYmax;
};
