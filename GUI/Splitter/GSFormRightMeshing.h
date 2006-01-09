#pragma once



// CGSFormRightMeshing form view

class CGSFormRightMeshing : public CFormView
{
	DECLARE_DYNCREATE(CGSFormRightMeshing)


public:
  afx_msg void OnBnClickedTri2priButton();
  afx_msg void OnBnClickedQuad2hexButton();
  afx_msg void OnBnClickedHex2tetButton();
  afx_msg void OnBnClickedQuad2triButton();

protected:
	CGSFormRightMeshing();           // protected constructor used by dynamic creation
	virtual ~CGSFormRightMeshing();

 
// Form Data
public:
	//{{AFX_DATA(CGSFormRightMeshing)
	enum { IDD = IDD_CONTROL_RIGHT_MESHING_FORM };
    CListBox	m_List;
    //}}AFX_DATA

public:
	long m_patch_index;
    double m_mesh_density;
    // Attributes
public:
   	CGeoSysDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
				return (CGeoSysDoc*) m_pDocument;
			}


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSFormRightMeshing)

public:
    virtual void OnDraw(CDC* pDC);  
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedStruc2dButton();
    afx_msg void OnBnClickedTriDelaunayButton();
    afx_msg void OnBnClickedTriAreaiButton();
    afx_msg void OnBnClickedTriAngleButton();
    afx_msg void OnBnClickedTriLengthButton();
    afx_msg void OnBnClickedTriQualityGraphButton();
    afx_msg void OnBnClickedAutomaticMeshDensity();
    afx_msg void OnBnClickedGeometricMeshDensity2();
    afx_msg void OnBnClickedGlobalSplitMeshDensity3();
    afx_msg void OnBnClickedStartGmshMeshing();
    afx_msg void OnBnClickedGeoEditorButton();
    afx_msg void OnBnClickedMshEditorButton();
    afx_msg void OnBnClickedMshnodesInPolygon();
    afx_msg void OnBnClickedClearSelection();
    afx_msg void OnBnClickedMinMaxEdgeLength();
    afx_msg void OnBnClickedSelectAll();
    afx_msg void OnBnClickedSetPatchIndex();
    afx_msg void OnBnClickedGmshMshImport();
    afx_msg void OnBnClickedCreateGeoBoundingbox();
    afx_msg void OnBnClickedTetVolumeButton();
    afx_msg void OnBnClickedTetAngleButton();
    afx_msg void OnBnClickedSetMeshdensity();
    afx_msg void OnBnClickedDeleteDisplayedMesh();
    afx_msg void OnBnClickedPatchindexForPlains();
    afx_msg void OnBnClickedCombinePatchindex();
    afx_msg void OnBnClickedCompressPatchindex2();
};


