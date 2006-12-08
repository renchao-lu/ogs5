// GSPropertyRight.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GSPropertyRight.h"
#include "PropPgFormView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CGSPropertyRight

IMPLEMENT_DYNCREATE(CGSPropertyRight, CPropPgFormView)

CGSPropertyRight::CGSPropertyRight()
	: CPropPgFormView(CGSPropertyRight::IDD)
{
	//{{AFX_DATA_INIT(CGSPropertyRight)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CGSPropertyRight::~CGSPropertyRight()
{
}

void CGSPropertyRight::DoDataExchange(CDataExchange* pDX)
{
	CPropPgFormView::DoDataExchange(pDX);
}

void CGSPropertyRight::OnDraw(CDC* pDC)
{
	CPropPgFormView::OnDraw(pDC);
}

BEGIN_MESSAGE_MAP(CGSPropertyRight, CFormView)
    //{{AFX_MSG_MAP(CGSPropertyRight)
	

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CGSPropertyRight diagnostics

#ifdef _DEBUG
void CGSPropertyRight::AssertValid() const
{
	CPropPgFormView::AssertValid();
}

void CGSPropertyRight::Dump(CDumpContext& dc) const
{
	CPropPgFormView::Dump(dc);
}
#endif //_DEBUG


// CGSPropertyRight message handlers

BOOL CGSPropertyRight::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	if (!CPropPgFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
	 return FALSE;

	m_PropSheet.AddPage(&m_Page1);
	m_PropSheet.AddPage(&m_Page2);
	m_PropSheet.AddPage(&m_Page3);
	m_PropSheet.AddPage(&m_Page9);
	m_PropSheet.AddPage(&m_Page10);

	// create a modeless property sheet
	if (!m_PropSheet.Create(this)) {
		DestroyWindow();
		return FALSE;
	}
	//((CButton*)GetDlgItem(IDC_CENTER))->SetCheck(m_PropSheet.m_Center);
	//((CButton*)GetDlgItem(IDC_ALLOW_PAGE_CHANGE))->SetCheck(m_PropSheet.m_bOKToLeaveTab);

	return TRUE;	
}
