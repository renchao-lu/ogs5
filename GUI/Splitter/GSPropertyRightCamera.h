#pragma once
#include "afxcmn.h"
#include "GeoSysDoc.h"
#include "ViewPropertyPage.h"
#include "resource.h"

// CGSPropertyRightCamera dialog

class CGSPropertyRightCamera : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightCamera)

public:
	CGSPropertyRightCamera();
	~CGSPropertyRightCamera();

// Dialog Data
	//{{AFX_DATA(CGSPropertyRightCamera)
	enum { IDD = IDD_CONTROL_RIGHT_CAMERA };
   	CListBox	m_List;
    // NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

public:
    double m_image_distort_factor_x;
    double m_image_distort_factor_y;
    double m_image_distort_factor_z;
    BOOL point_numbers_button_on;
    int m_bounding_box;
    int m_element_numbers;
    int m_node_numbers;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnDataChange();

public:
   	virtual void OnPaint();  



	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBoundingboxOnOff();
    afx_msg void OnBnClickedElementnumbersOnOff();
    afx_msg void OnBnClickedNodenumbersOnOff();
    afx_msg void OnBnClickedUpdateAllViews();
    afx_msg void OnBnClickedGeoEditorButton();
    afx_msg void OnBnClickedMshEditorButton();
    afx_msg void OnDistortImage();
    afx_msg void OnNotDistortImage();
    afx_msg void OnBnClickedPointNumbersCheck();
	afx_msg void OnBnClickedBcEditorButton();
	afx_msg void OnBnClickedGspEditorButton();
	afx_msg void OnBnClickedIcEditorButton();
	afx_msg void OnBnClickedMcpEditorButton();
	afx_msg void OnBnClickedMmpEditorButton();
	afx_msg void OnBnClickedMfpEditorButton();
	afx_msg void OnBnClickedNumEditorButton();
	afx_msg void OnBnClickedOutEditorButton2();
	afx_msg void OnBnClickedPcsEditorButton();
	afx_msg void OnBnClickedRfeEditorButton();
	afx_msg void OnBnClickedStEditorButton();
	afx_msg void OnBnClickedTecEditorButton();
	afx_msg void OnBnClickedTimEditorButton();
};
