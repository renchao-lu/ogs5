/**************************************************************************
GeoSys GUI - Object: New point dialog
Task: 
Programing:
12/2005 CC Implementation
**************************************************************************/

#pragma once


#include "geo_pnt.h"
#include "ListCtrl.h"
#include "EditCell.h"
// CGSPointnew dialog

class CGSPointnew : public CDialog
{
	DECLARE_DYNAMIC(CGSPointnew)

public:
	CGSPointnew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGSPointnew();

// Dialog Data
	enum { IDD = IDD_POINT_NEW };
    CMyListCtrl m_Listctrlpointnew;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClickedButtonCreatePoint();
    afx_msg void OnButtonRemovePoint();
    afx_msg void OnButtonRemoveallPoints();
    afx_msg void OnButtonUnselectpoint();
    afx_msg void OnPointRead();
    afx_msg void OnPointWrite();
    afx_msg void OnWritePointTec();
    void UpdatePointList();
    void AddPointtoList(long);
    afx_msg void OnButtonPointPropertiesUpdate();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized); //CC
};
