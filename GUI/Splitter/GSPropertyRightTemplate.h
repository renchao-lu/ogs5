

#pragma once
#include "ViewPropertyPage.h"
#include "resource.h"
#include "afxcmn.h"
#include "GeoSysDoc.h"

// CGSPropertyRightTemplate dialog

class CGSPropertyRightTemplate : public CViewPropertyPage
{
	DECLARE_DYNAMIC(CGSPropertyRightTemplate)

public:
	CGSPropertyRightTemplate();
	virtual ~CGSPropertyRightTemplate();

// Dialog Data
	enum { IDD = IDD_CONTROL_RIGHT_TEMPLATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:

};
