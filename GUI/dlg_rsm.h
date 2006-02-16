#pragma once
#include "afxcmn.h"


// CRegionalSoilModel dialog

class CRegionalSoilModel : public CDialog
{
	DECLARE_DYNAMIC(CRegionalSoilModel)

public:
	CRegionalSoilModel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegionalSoilModel();

// Dialog Data
	enum { IDD = IDD_SOIL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
    void UpdateList();

	DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_LC_SFC;
    afx_msg void OnBnClickedReadProfileCode();
    afx_msg void OnBnClickedReadStructure();
    //afx_msg void OnBnClickedCreateProfiles();
    afx_msg void OnBnClickedCreateMSH();
    afx_msg void OnBnClickedWriteMSH();
    afx_msg void OnBnClickedWriteMMP();
    afx_msg void OnBnClickedCreateMMP();
    int m_iSubDivisions;
};

class CColumn : public CGLPolyline
{
  private:
    CGLLine* m_lin;
  public:
    ~CColumn();
    double geo_area;
    double center_point[3];
};

extern void COLDeleteLines();
extern void COLDelete();
extern CColumn* COLGet(int);
extern CColumn* COLGet(string);



