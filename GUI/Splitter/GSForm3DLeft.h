#if !defined(AFX_GEOSYSFORM_H__1400D4B6_BDC3_11D3_8FAD_0080AD4311B1__INCLUDED_)
#define AFX_GEOSYSFORM_H__1400D4B6_BDC3_11D3_8FAD_0080AD4311B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeoSysForm.h : header file
//

#include "GeoSysVisualFx.h"
#include "gs_project.h"

#include <vector>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CGSForm3DLeft form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CGSForm3DLeft : public CFormView
{
public:
	CGSForm3DLeft();           // protected constructor used by dynamic creation

    
    
   

// Form Data
public:
	//{{AFX_DATA(CGSForm3DLeft)
	enum { IDD = IDD_CONTROL_FORM };
    TVINSERTSTRUCT tvInsert;
    int m_3dcontrol_points;
    int m_3dcontrol_lines;
    int m_3dcontrol_polylines;
    int m_3dcontrol_surfaces; 
    int m_3dcontrol_volumes;
    int m_3dcontrol_nodes;
    int m_3dcontrol_elements;
    int m_3dcontrol_matgroups;
    int m_3dcontrol_bc;
    int m_3dcontrol_pcs;
    int m_3dcontrol_sourcesink;
    int m_selected_wire_frame;
    int m_x_value_color;
    int m_y_value_color;
    int m_z_value_color;
    int m_permeability_value_color;

    CString m_pcs_name;
    CTreeCtrl* pCtrl;
    CTreeCtrl* pCtrl_copy;

    HTREEITEM hPOINTS;
    HTREEITEM hLINES;
    HTREEITEM hGEOMETRY;
	//}}AFX_DATA
protected:
    CImageList m_TreeImages;

public:
    int point_list_length;
    int line_list_length;

// Attributes
public:
   	CGeoSysDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
				return (CGeoSysDoc*) m_pDocument;
			}


// Operations
public:
	void showChange(void);
	void DoThisPolyline(CString item_name, HTREEITEM hItem, HTREEITEM* hChildItem);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSForm3DLeft)
	public:
	virtual void OnInitialUpdate();
   	virtual void OnDraw(CDC* pDC);  
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGSForm3DLeft();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CGSForm3DLeft)
		afx_msg void OnDataChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    DECLARE_DYNCREATE(CGSForm3DLeft)

public:
    afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);

  //Check Box Control
  BOOL SetCheckBox(HTREEITEM hItem, BOOL bCheck);
  BOOL GetCheckBox(HTREEITEM hItem, BOOL& bCheck);
  BOOL IsCheckBox(HTREEITEM hItem);
  BOOL GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable);
  BOOL SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable);

  // TREE CONTROL
  void SetTree();
  void UpdateViewbyTree();
  void DeactivateAll();

};

/////////////////////////////////////////////////////////////////////////////
class CMATGroupsControl
{
  public:
    long materialnumber;
	long display_mode;
    //vector<CMATGroupsControl*> material_groups_vector;
};
extern vector<CMATGroupsControl*> material_groups_vector;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOSYSFORM_H__1400D4B6_BDC3_11D3_8FAD_0080AD4311B1__INCLUDED_)

