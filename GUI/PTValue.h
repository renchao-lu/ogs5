#pragma once
#include "afxwin.h"
#include "rf_random_walk.h"
#include "GeoSys.h"


// PTValue dialog


class PTValue : public CDialog
{
	DECLARE_DYNAMIC(PTValue)

public:
	PTValue(CWnd* pParent = NULL);   // standard constructor
	virtual ~PTValue();

public:  

// Dialog Data
	enum { IDD = IDD_PTVALUES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    int m_NumOfParticles;
	int m_PID;
    afx_msg void OnBnClickedOk();
    
private:
    CStatic m_PickedInfo;
    FiniteElement::CElement *fem;
    CRFProcess* m_pcs;
    CFEMesh* m_msh;
    CElem* m_ele;

    void AssignAParticleToTheElement(int no, CElem* m_ele);
public:
};
