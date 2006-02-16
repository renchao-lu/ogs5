#pragma once

#include "nodes.h"

// PickingHandle dialog

class PickingHandle : public CDialog
{
	DECLARE_DYNAMIC(PickingHandle)

public:
	PickingHandle(CWnd* pParent = NULL);   // standard constructor
	virtual ~PickingHandle();

// Dialog Data
	enum { IDD = IDD_PICKINGHANDLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    void showChange(void);
    void solvePlane(CGLPoint* point, double* p);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedSelectMode();
    afx_msg void OnBnClickedDeselectMode();
    afx_msg void OnBnClickedSelectall();
    afx_msg void OnBnClickedDeselectall();
    afx_msg void OnBnClickedStopPicking();
    afx_msg void OnBnClickedSelectPlane();
};
