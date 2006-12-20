#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPgFormView.h"
#include "GSPropertyRightCamera.h"
#include "GSPropertyRightResults.h"
#include "GSPropertyRightMesh.h"
#include "GSPropertyRightGeometry.h"
#include "GSPropertyRightTemplate.h"
#include "GSPropertyRightImportData.h"


// CGSPropertyRight form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CGSPropertyRight : public CPropPgFormView
{

protected:
  	CGSPropertyRight();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGSPropertyRight)


public:
    //{{AFX_DATA(CMyFormView)
	enum { IDD = IDD_CONTROL_RIGHT };
   	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

    //}}AFX_DATA

public:
  	CGSPropertyRightCamera	m_Page1;	
	CGSPropertyRightGeometry m_Page2;	
	CGSPropertyRightMesh m_Page3;	
	CGSPropertyRightResults	m_Page9;	
	CGSPropertyRightImportData m_Page10;
	CGSPropertyRightTemplate m_Page11;	
	

    
// Implementation
protected:
	virtual ~CGSPropertyRight();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnDraw(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};


