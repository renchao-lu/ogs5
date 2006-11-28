#pragma once


// CIsoControl dialog

class CIsoControl : public CDialog
{
	DECLARE_DYNAMIC(CIsoControl)

public:
	CIsoControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIsoControl();
    CWnd* m_pParent;

// Dialog Data
	enum { IDD = IDD_ISO_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

public:
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
	CListCtrl m_Iso_List;
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
    afx_msg void OnBnClickedCancel();
};
