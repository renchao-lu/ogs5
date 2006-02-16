#pragma once



// CGSFormRightPicking form view

class CGSFormRightPicking : public CFormView
{
	DECLARE_DYNCREATE(CGSFormRightPicking)

protected:
	CGSFormRightPicking();           // protected constructor used by dynamic creation
	virtual ~CGSFormRightPicking();

 
// Form Data
public:
	//{{AFX_DATA(CGSFormRightPicking)
	enum { IDD = IDD_CONTROL_RIGHT_PICKING_FORM };

    //}}AFX_DATA

public:
	//enum { IDD = IDD_GSFORMRIGHTPASSIVE };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
    FiniteElement::CElement *fem;
    void showChange(void);    

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnInitialUpdate();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSimulateUnderDeveloperMode();
    afx_msg void OnBnClickedButton5();
    afx_msg void OnBnClickedPtvalue();
    afx_msg void OnBnClickedVelocityvector();
    afx_msg void OnBnClickedShowparticle();
    afx_msg void OnBnClickedInorout();
	afx_msg void OnBnClickedReadpct();
	afx_msg void OnBnClickedCinele();
	
	BOOL PeekAndPump();
};


