// ViewPropertySheet.cpp : implementation file
//

#include "stdafx.h"

#include "ViewPropertyPage.h"
#include "ViewPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define WM_ADJUST WM_USER + 111


/////////////////////////////////////////////////////////////////////////////
// CViewPropertySheet

IMPLEMENT_DYNAMIC(CViewPropertySheet, CPropertySheet)

CViewPropertySheet::CViewPropertySheet()
	:CPropertySheet(), m_MaxPageRect(0,0,0,0), m_MinPageRect(0,0,0,0), m_Center(FALSE), m_bOKToLeaveTab(TRUE)
{
}

CViewPropertySheet::CViewPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage), m_MaxPageRect(0,0,0,0), m_MinPageRect(0,0,0,0), m_Center(FALSE), m_bOKToLeaveTab(true)
{
}

CViewPropertySheet::CViewPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage), m_MaxPageRect(0,0,0,0), m_MinPageRect(0,0,0,0), m_Center(FALSE), m_bOKToLeaveTab(true)
{
}

CViewPropertySheet::~CViewPropertySheet()
{
}

BOOL CViewPropertySheet::Create(CWnd *pParentWnd, DWORD dwStyle, DWORD dwExStyle)
{ 
	// Default to a Child Window overriding normal CPropertySheet Style
	if (dwStyle == -1)
		dwStyle = DS_3DLOOK | DS_CONTEXTHELP | DS_SETFONT |	WS_VISIBLE | WS_CHILD;
	
	dwExStyle = dwExStyle | WS_EX_CONTROLPARENT; // Handle tab key
	
	if (!CPropertySheet::Create(pParentWnd,dwStyle,dwExStyle))
		return FALSE;
	GetClientRect(m_rectOriginal);	// save original size, which is optimized for all initial pages
    m_rectOriginal.right = 116;
    return TRUE;
}

BOOL CViewPropertySheet::PreTranslateMessage(MSG* pMsg) 
{
	// NOTE: this code will handle hotkeys for the tab controls
	// Hotkeys only get here if the propsheet has the focus, so must
  // route hotkeys from parent view to get this to work (See CPropPgFormView::PreTranslateMessage()) 
	if ((pMsg->message == WM_SYSKEYDOWN) && (pMsg->wParam > 32)) {
		CTabCtrl *pTab = GetTabControl();
		int n = pTab->GetItemCount();

		char buf[80], shortcut[3];
		TC_ITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = buf;
		shortcut[0] = '&';
		shortcut[2] = '\0';
		for (int i = 0; i < n; i++ ) {
			tcItem.cchTextMax = sizeof(buf)-1;
			pTab->GetItem(i, &tcItem);
			shortcut[1] = (char)pMsg->wParam;
			if (strstr(buf, shortcut)) {
				if (m_bOKToLeaveTab)
					SetActivePage(i);
				return TRUE;
			}
			else {
				shortcut[1] = (char)(isupper(pMsg->wParam) ? tolower : toupper)(pMsg->wParam);
				if (strstr(buf, shortcut)) {
					if (m_bOKToLeaveTab)
						SetActivePage(i);
					return TRUE;
				}
			}
		}
	}

	// allow view to pre-translate for hotkeys
	CView	*pView = (CView*)GetParent();
	if (pView->PreTranslateMessage(pMsg))
		return TRUE;
	
	return CPropertySheet::PreTranslateMessage(pMsg);
}


// Adjusts the size and position of Tabs and PropertyPages
//	
void CViewPropertySheet::AdjustPages()
{
	CRect rect;
	
	GetClientRect(&rect);

	CTabCtrl* pTab = GetTabControl();

	if (pTab)	{
		pTab->Invalidate(FALSE);
		pTab->MoveWindow(rect, FALSE);		// Resize the TabsCtrl to new size but don't draw now
		pTab->AdjustRect(FALSE, &rect);		// Get the display area below the Tabs
 
		int iTabHeight = rect.top;				// Save the Tab Height

		// Which Page is being adjusted?
		CPropertyPage* pPage = GetActivePage();
		if (pPage) {
			CRect pageRect;

			//Going to be moving things without redrawing
			pPage->Invalidate(FALSE); 

			// Return to Original Dialog Size
			((CViewPropertyPage*)pPage)->GetOriginalRect(&pageRect);

			pPage->MoveWindow(pageRect, FALSE); //Move but don't draw
		
			if (m_Center)			// Move to the Center of the display area
				pPage->CenterWindow(pTab);				// Center in Window

			// Adjust for tab height
			pPage->GetWindowRect(&pageRect);
			ScreenToClient(&pageRect);
			pageRect.OffsetRect(0,iTabHeight);
			
			pageRect.IntersectRect(pageRect,rect);		// restrict to the display area
 
			// Fit Page to the new size
			pPage->MoveWindow(pageRect); 
		}
	}
}

void CViewPropertySheet::CenterControls(BOOL bCenter /* = TRUE */)
{
	m_Center = bCenter;
	PostMessage(WM_ADJUST);
}

void CViewPropertySheet::AllowPageChange(BOOL bAllowPageChange /* = TRUE */)
{
	m_bOKToLeaveTab = bAllowPageChange;
}

BEGIN_MESSAGE_MAP(CViewPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CViewPropertySheet)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ADJUST, OnAdjust)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewPropertySheet message handlers



LRESULT CViewPropertySheet::OnAdjust(WPARAM wParam, LPARAM lParam)
{
    lParam=lParam;
	wParam=wParam;
	AdjustPages();
	return 0;
}


BOOL CViewPropertySheet::OnInitDialog() 
{
  ModifyStyleEx(0,WS_EX_CONTROLPARENT);
	BOOL bResult = CPropertySheet::OnInitDialog();

	// Resize the CTabCtrl and PropertyPages
	PostMessage(WM_ADJUST);

	return bResult;
}


BOOL CViewPropertySheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR* pnmh = (LPNMHDR)lParam;

  if (TCN_SELCHANGING == pnmh->code)			// save the current page index
    m_nLastActive = GetActiveIndex();       
	else if (TCN_SELCHANGE == pnmh->code) {		// just changed tabs
    if (!m_bOKToLeaveTab)
      PostMessage(PSM_SETCURSEL, m_nLastActive);       
    else {
			// NOTE: ON_NOTIFY_REFLECT does not seem to work for non-CDialog based windows, oh well...
			PostMessage(WM_ADJUST); // adjust the pages
			CView	*pView = (CView*)GetParent();
			static NMHDR	nmh = *pnmh;
			pView->PostMessage(WM_NOTIFY, wParam, (LPARAM)&nmh);		// notify view of page change
		}
	}

	return CPropertySheet::OnNotify(wParam, lParam, pResult);
}


void CViewPropertySheet::OnSize(UINT nType, int cx, int cy) 
{
	CPropertySheet::OnSize(nType, cx, cy);
	
	PostMessage(WM_ADJUST); // adjust the pages
}


