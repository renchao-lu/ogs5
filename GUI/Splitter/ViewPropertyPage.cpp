// ViewPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "ViewPropertyPage.h"
#include "../src/mfc/afximpl.h" // Get access to afxData

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CViewPropertyPage property page

IMPLEMENT_DYNCREATE(CViewPropertyPage, CPropertyPage)

CViewPropertyPage::CViewPropertyPage() : CPropertyPage(-1)	//  should not use!
{
}

CViewPropertyPage::CViewPropertyPage(int nID) : CPropertyPage(nID), m_bEnabled(TRUE)
{
	//{{AFX_DATA_INIT(CViewPropertyPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CViewPropertyPage::~CViewPropertyPage()
{
}


void CViewPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewPropertyPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


// Returns the original Dialog Resource Rectangle in Screen units
//
void CViewPropertyPage::GetOriginalRect(CRect *pRect)
{
	DLGTEMPLATEEX* pTmp = NULL;

	pRect->SetRectEmpty();

	// One of these should have the Dialog Template
	if(m_hDialogTemplate)
		pTmp = (DLGTEMPLATEEX*)m_hDialogTemplate;
	else if (m_lpDialogTemplate)
		pTmp = (DLGTEMPLATEEX*)m_lpDialogTemplate;

	// Found it, Set the Rectangle
	if (pTmp) {
		pRect->SetRect(0, 0, pTmp->cx, pTmp->cy);

		//It's in Dialog units so convert
		MapDialogRect(pRect);
	}
}

void CViewPropertyPage::EnableControls(BOOL bEnable)
{
   CWnd* hwndCtrl;

   m_bEnabled = bEnable;
   for (hwndCtrl = GetWindow(GW_CHILD); hwndCtrl; hwndCtrl = hwndCtrl->GetWindow(GW_HWNDNEXT))
      hwndCtrl->EnableWindow(bEnable);
}

int CViewPropertyPage::IsEnabled()
{
	return m_bEnabled;
}


BEGIN_MESSAGE_MAP(CViewPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CViewPropertyPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

