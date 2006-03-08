#pragma once


// PickedProperties dialog
#include "ListCtrl.h"

// MshLib
#include "msh_nodes_rfi.h"
#include "fem_ele_std.h"
// GeoSys-FEM
#include "nodes.h"
#include "elements.h"
#include "rf_pcs.h"
#include "rfadt.h"
#include "rf_out_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
// GeoSys-GUI
#include "gs_polyline.h"
// GeoSys-GeoLib
#include "geo_sfc.h"
#include "geo_ply.h"
#include "geo_pnt.h"
class PickedProperties : public CDialog
{
	DECLARE_DYNAMIC(PickedProperties)

public:
	PickedProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~PickedProperties();

    void ShowThePicked();
    void ListGLINodeSelected();
	void ListRFINodeSelected(int PCSSwitch);
	void ListElementSelected(int PCSSwitch);
	void ListPolylineSelected();
    void ListSurfaceSelected();
    void ListVolumeSelected();
	void ListParticleSelected();

	void ListGLINodeAll();
	void ListRFINodeAll(int PCSSwitch);
	void ListElementAll(int PCSSwitch);
	void ListPolylineAll();
    void ListSurfaceAll();
    void ListVolumeAll();


// Dialog Data
	enum { IDD = IDD_PICKEDPROPERTY };
    CMyListCtrl m_SmallList;

private:
	int sizeOfWord;
	int widthOfCell;
	int PCSSwitch;
	int BCnSTSwitch;

	int numOfItemsEle;
	int numOfItemsNode;

    CFEMesh* m_msh;
    CElem* m_ele;
	char** CreateWordMemory(int numOfItems);
	int GetNumberOfNodesInElement(int index);

	int IsThisPointBCIfYesStoryValue(int index, CBoundaryConditionsGroup* m_bc_group, double* value);
	int IsThisPointSTIfYesStoryValue(int index, CSourceTermGroup* m_st_group, double* value);

	void ShowAll();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedClose();
	afx_msg void OnLvnItemchangedPickedlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBothorone();
	afx_msg void OnBnClickedBcnst();
	afx_msg void OnBnClickedNodeindex();
	afx_msg void OnBnClickedShowall();
	afx_msg void OnBnClickedSaveastxt();
};
