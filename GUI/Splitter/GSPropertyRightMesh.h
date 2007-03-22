

#pragma once
#include "ViewPropertyPage.h"
#include "resource.h"
#include "afxcmn.h"
#include "GeoSysDoc.h"

// CGSPropertyRightMesh dialog

class CGSPropertyRightMesh : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightMesh)

public:
	CGSPropertyRightMesh();
	virtual ~CGSPropertyRightMesh();

// Dialog Data
	enum { IDD = IDD_CONTROL_RIGHT_MESH };
    CListBox	m_List;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSPropertyRightMesh)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	void OnPaint();  
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

public:
	long m_patch_index;
    double m_mesh_density;

public:
  afx_msg void OnBnClickedTri2priButton();
  afx_msg void OnBnClickedQuad2hexButton();
  afx_msg void OnBnClickedHex2tetButton();
  afx_msg void OnBnClickedQuad2triButton();
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
    afx_msg void OnBnClickedMergeMeshes();
    afx_msg void OnBnClickedCheckdoublenodes();
    afx_msg void GetMinMaxEdgeLength(CFEMesh*m_msh);
    afx_msg void OnBnClickedDeletedoublenodes();
    afx_msg void OnBnClickedSplitelements();
    afx_msg void OnBnClickedY2z();
    afx_msg void OnBnClickedX2y();
    afx_msg void OnBnClickedX2z();
    afx_msg void OnBnClickedMappingRight();
    afx_msg void OnBnClickedCreateEllipsoid();
    afx_msg void OnBnClickedNewMeshfromMATMesh();
	afx_msg void OnBnClickedMshTestDelaunay3DLib();
	afx_msg void OnBnClickedCreateEllipse2d();
	afx_msg void OnBnClickedMshTestDelaunay2dlib();

	afx_msg void OnBnClickedMshnodesAlongPolyline();
};

