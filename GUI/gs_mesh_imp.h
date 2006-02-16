#pragma once


// Cgs_mesh_imp dialog

class Cgs_mesh_imp : public CDialog
{
	DECLARE_DYNAMIC(Cgs_mesh_imp)

public:
	Cgs_mesh_imp(CWnd* pParent = NULL);   // standard constructor
	virtual ~Cgs_mesh_imp();

// Dialog Data
	enum { IDD = IDD_GS_MESH_IMP };
	double x;
	double y;
	double z;
	long nnr;



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnManualRemeshBWA();
	afx_msg void OnManualRemovePoint();
};
