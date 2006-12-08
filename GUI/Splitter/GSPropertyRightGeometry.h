#pragma once
#include "ViewPropertyPage.h"
#include "resource.h"
#include "afxcmn.h"
#include "GeoSysDoc.h"

// CGSPropertyRightGeometry dialog

class CGSPropertyRightGeometry : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightGeometry)

public:
	CGSPropertyRightGeometry();
	virtual ~CGSPropertyRightGeometry();

// Dialog Data
	enum { IDD = IDD_CONTROL_RIGHT_GEO };
    CListBox	m_List;


public:
	int m_3dcontrol_double_points;
    BOOL double_points_button_check_on;
    BOOL point_numbers_button_on;
    double m_tolerancefactor;
    double m_polyline_min_seg_length;
    double m_polyline_max_seg_length;
    double m_polyline_smaller_seg_length_def;

public:
	afx_msg void OnDataChange();
    afx_msg void OnBnClickedCheckDoublePoints();
    afx_msg void OnBnClickedDeleteDoublePointsButton();
    afx_msg void OnBnClickedAddPolygonButton();
    afx_msg void OnBnClickedAddNextPolygonButton();
    afx_msg void OnBnClickedGetMinmaxseglengthButton();
    afx_msg void OnBnClickedSetMinmaxseglengthButton();
    afx_msg void OnBnClickedReloadGeoButton();
    afx_msg void OnBnClickedEditorGeoButton();
    afx_msg void OnBnClickedGetSeglengthSmallerDef();
    afx_msg void OnBnClickedGeoInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:

};
