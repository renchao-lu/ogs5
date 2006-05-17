#pragma once
#include "afxcmn.h"



// CGSFormRightPassive form view

class CGSFormRightPassive : public CFormView
{
	DECLARE_DYNCREATE(CGSFormRightPassive)

protected:
	CGSFormRightPassive();           // protected constructor used by dynamic creation
	virtual ~CGSFormRightPassive();

 
// Form Data
public:
	//{{AFX_DATA(CGSFormRightPassive)
    enum { IDD = IDD_CONTROL_RIGHT_PASSIVE_FORM };
   	CListBox	m_List;
    int m_bounding_box;
    int m_3dcontrol_double_points;

    //}}AFX_DATA

// Attributes
public:
   	CGeoSysDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
				return (CGeoSysDoc*) m_pDocument;
			}


public:
    double m_image_distort_factor_x;
    double m_image_distort_factor_y;
    double m_image_distort_factor_z;
    BOOL double_points_button_check_on;
    BOOL point_numbers_button_on;
    double m_tolerancefactor;
    double m_polyline_min_seg_length;
    double m_polyline_max_seg_length;
    double m_polyline_smaller_seg_length_def;
    double m_pcs_min;
    double m_pcs_max;
    CString m_pcs_name;



#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSFormRightPassive)
	public:
	virtual void OnInitialUpdate();
   	virtual void OnDraw(CDC* pDC);  
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Generated message map functions
	//{{AFX_MSG(CGSFormRightPassive)
		afx_msg void OnDataChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBoundingboxOnOff();
    afx_msg void OnDistortImage();
    afx_msg void OnNotDistortImage();
    afx_msg void OnBnClickedCheckDoublePoints();
    afx_msg void OnBnClickedDeleteDoublePointsButton();

    afx_msg void OnBnClickedAddPolygonButton();
    afx_msg void OnBnClickedAddNextPolygonButton();
    afx_msg void OnBnClickedGetMinmaxseglengthButton();
    afx_msg void OnBnClickedSetMinmaxseglengthButton();
    afx_msg void OnBnClickedReloadGeoButton();
    afx_msg void OnBnClickedEditorGeoButton();
    afx_msg void OnBnClickedPointNumbersCheck();
    afx_msg void OnBnClickedGetSeglengthSmallerDef();
    afx_msg void OnBnClickedGetPcsMinmaxButton3();

    void GetPcsMinmax();
    afx_msg void OnBnClickedSetPcsMinmaxButton2();
    afx_msg void OnBnClickedValuePointsButton();

    CSliderCtrl m_slider1;
    CSliderCtrl m_slider2;
    afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
};


