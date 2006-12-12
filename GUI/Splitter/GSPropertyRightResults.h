#pragma once
#include "afxcmn.h"
#include "GeoSysDoc.h"
#include "ViewPropertyPage.h"
#include "resource.h"


// CGSPropertyRightResults dialog

class CGSPropertyRightResults : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightResults)

public:
	CGSPropertyRightResults();
	 ~CGSPropertyRightResults();
    CWnd* m_pParent;

// Dialog Data
//{{AFX_DATA(CGSPropertyRightResults)
	enum { IDD = IDD_CONTROL_RIGHT_RESULTS };
	CListCtrl m_Iso_List;
    // NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

public:
    double m_pcs_min_r;
    double m_pcs_max_r;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedGetPcsMinmaxButton3();
    afx_msg void GetPcsMinmax();
    afx_msg void OnBnClickedSetPcsMinmaxButton2();
    afx_msg void OnBnClickedValuePointsButton();

    CSliderCtrl m_slider1;
    CSliderCtrl m_slider2;
    afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnBnClickedIsolineIsosurfaceButton();
	
	
	//IsoLines/Surfaces by Haibing
	double Iso_Max_Value;
	double Iso_Min_Value;
	double Iso_Step_Value;
	bool Iso_If_Show_Iso;
	
	bool If_Switch_Off_ContourPlot;// 1-switch off, 0-switch on
	int Iso_Form;//0-Isoline, 1-Isosurface.
	int Iso_Type;//0-static, 1-dynamic
	afx_msg void OnBnClickedCheck2();
	CButton Control_Show_Iso;
	CButton Control_SwitchOff_ContourPlot;
	afx_msg void OnBnClickedCheck3();
	CListCtrl m_Iso_List_1;
	int Iso_Num;
	void InsertIsoListItem(double max, double min, int Num);
	void InsertIsoListItem(double max, double min, double step);
	afx_msg void OnBnClickedIsoRefresh();
	afx_msg void OnBnClickedIsoAddlistitem();
	afx_msg void OnBnClickedIsoDellistitem();
	afx_msg void OnBnClickedIsoApply();
	afx_msg void OnCustomdrawMyIsoList ( NMHDR* pNMHDR, LRESULT* pResult );
	void SetCell(HWND hWnd1, CString value, int nRow, int nCol);
	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	int nItem, nSubItem;
	void OnClickMyIsoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocusIsoValueEdit();
	afx_msg void OnNMDblclkList6(NMHDR *pNMHDR, LRESULT *pResult);
	COLORREF IsoColor[1000];
	COLORREF IsoFrameColor[1000];
	double IsoFrameWidth[1000];
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnItemclickIsoListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
	double Get_Blue_Value(double value_norm);
	double Get_Red_Value(double value_norm);
	double Get_Green_Value(double value_norm);

	afx_msg void OnBnClickedIsoRefresh2();
	afx_msg void OnBnClickedIsoAddlistitem2();
	afx_msg void OnBnClickedIsoDellistitem2();
	afx_msg void OnBnClickedIsoDeleteall();
};
