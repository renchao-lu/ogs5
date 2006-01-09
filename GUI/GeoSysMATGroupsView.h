// GeoSysMATGroupsView.h : Schnittstelle der Klasse CGeoSysView
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// C++ STL
#include <string>
#include "grid\virtualgridctrl.h"
#include "afxwin.h"

//zoom
#include "GeoSysZoomView.h"
// RF Objects
#include "rf_mmp_new.h"
#include "gs_mat_mp.h"

using namespace std;

class CGeoSysMATGroupsView : public CGeoSysZoomView
{
protected: // Nur aus Serialisierung erzeugen
	CGeoSysMATGroupsView();
	DECLARE_DYNCREATE(CGeoSysMATGroupsView)

// Attribute
public:
	CGeoSysDoc* GetDocument();

// Operationen
	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CGeoSysMATGroupsView)
	public:
	virtual void OnDraw(CDC* pDC);  // überladen zum Zeichnen dieser Ansicht
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CGeoSysMATGroupsView();
     virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int width;
    int height;
    int m_iDisplayPLY;
    int m_iDisplaySFC;
    double m_dXmin,m_dXmax,m_dYmin,m_dYmax;
    int mat_group_selected;
    string mat_group_name_selected;
    int xpixel(double);
    int ypixel(double);
    BOOL m_bDisplayMaterialGroups;
    BOOL m_bShowLabels;
    CMATGroupEditor* m_dlg_mat_editor;
// Generierte Message-Map-Funktionen
protected:
	DECLARE_MESSAGE_MAP()

	friend class Mat_Grid;
public:
    afx_msg void OnProperties();
    afx_msg void OnMATGroupEditor();
    afx_msg void OnMatSolidProperties();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  //zoom
    afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewZoomfull();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
private:
	CMediumProperties *m_mat_mp;
};

#ifndef _DEBUG  // Testversion in GeoSysView.cpp
inline CGeoSysDoc* CGeoSysMATGroupsView::GetDocument()
   { return (CGeoSysDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////


// mat_prop_pad dialog
class Mat_Grid  {
public:
	Mat_Grid(LPCTSTR lpParaName = _T(""), double paraVal = 0, 
		        LPCTSTR lpUnitName = _T(""))
	{
		m_strParaName = lpParaName;
		m_strUnitName = lpUnitName;
		ParaVal = paraVal;

		m_crBkColor = ::GetSysColor(COLOR_WINDOW);
		m_crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

		m_bAcceptCursor = TRUE;
		m_bReadOnly = FALSE;
		m_bChecked = FALSE;
	}

	CString m_strParaName;
	CString m_strUnitName;

	//  Value
	double ParaVal;

	//  Additional demo data
	COLORREF m_crBkColor;
	COLORREF m_crTextColor;
	BOOL m_bAcceptCursor;
	BOOL m_bReadOnly;

	//  -------  NEW  --------
	BOOL m_bChecked;
};


class mat_prop_pad : public CDialog
{
	DECLARE_DYNAMIC(mat_prop_pad)

public:
	mat_prop_pad(CWnd* pParent = NULL);   // standard constructor
	virtual ~mat_prop_pad();
	BOOL OnInitDialog();

	void SetMaterial(CMediumProperties *m_mat)
	  {  medium_mat = m_mat;	  }
	void ShowMediaProperties();

	CComboBox Mat_Comb;
// Dialog Data
	enum { IDD = IDD_MAT_PAD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CVirtualGridCtrl m_grid;
	CArray<Mat_Grid, Mat_Grid> m_Data;

	afx_msg void OnGridGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnGridSetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeMatComboPad();

private:
	int MaterialType;
	CMediumProperties *medium_mat;
};
