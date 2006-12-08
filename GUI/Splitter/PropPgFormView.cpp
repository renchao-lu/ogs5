// PropPgFormView.cpp : implementation of the CPropPgFormView class
//

#include "stdafx.h"
#include "../src/mfc/afximpl.h" // Get access to afxData
#include "PropPgFormView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPropPgFormView

IMPLEMENT_DYNCREATE(CPropPgFormView, CFormView)

BEGIN_MESSAGE_MAP(CPropPgFormView, CFormView)
	//{{AFX_MSG_MAP(CPropPgFormView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormView construction/destruction

CPropPgFormView::CPropPgFormView()
	: CFormView(IDD_CONTROL_RIGHT), m_PS_Offset(PS_Y_OFFSET)
{
}

CPropPgFormView::CPropPgFormView(UINT nIDTemplate)
	: CFormView(nIDTemplate), m_PS_Offset(PS_Y_OFFSET)
{
}

CPropPgFormView::CPropPgFormView(LPCTSTR lpszTemplateName)
	: CFormView(lpszTemplateName), m_PS_Offset(PS_Y_OFFSET)
{
}

CPropPgFormView::~CPropPgFormView()
{
}

// overrides

BOOL CPropPgFormView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	if (!CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
		return FALSE;

	m_sizeControl = GetMaxControl();	// get max control for placing prop sheet below
	
	return TRUE;
}

BOOL CPropPgFormView::PreTranslateMessage(MSG* pMsg) 
{
	static BOOL bEntered = FALSE;		// watch for re-entry from property sheet call to views PreTranslateMessage
	if (!bEntered) {
		BOOL	bRet;
		bEntered = TRUE;
		bRet = m_PropSheet.PreTranslateMessage(pMsg);
		bEntered = FALSE;
		if (bRet)
			return TRUE;
	}
	return CFormView::PreTranslateMessage(pMsg);
}

// operations

void CPropPgFormView::AdjustPropSheetPosition()
{
	if (m_PropSheet.GetSafeHwnd()) {
		CRect	rect;
		GetClientRect(&rect);
		rect.DeflateRect(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT) + m_sizeControl.cy + m_PS_Offset, 0, 0);
		m_PropSheet.MoveWindow(&rect);
	}
}


// resize frame window to encompass all controls
// also sets scroll sizes
//
void CPropPgFormView::ResizeParentFrame()
{
	CFrameWnd* pFrame = GetParentFrame();
	
	if (pFrame != NULL && m_PropSheet.GetSafeHwnd()) {
		
		// remove the 3d style from the frame, since the view is providing it.
		if (afxData.bWin95 && (GetExStyle() & WS_EX_CLIENTEDGE))
			ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);

		CRect	rectFrame;
		CRect	rectResized;
		CSize	sizeScroll;		// min scroll size

		pFrame->GetWindowRect(rectFrame);

		// compute right and bottom of frame for template controls and prop sheet
		rectResized.top = rectResized.left = 0;
		rectResized.right = m_sizeControl.cx;
		rectResized.bottom = m_sizeControl.cy;
		rectResized.right = max(m_sizeControl.cx, m_PropSheet.m_rectOriginal.right);
		rectResized.bottom += m_PS_Offset + m_PropSheet.m_rectOriginal.bottom;

		// adjust for sheet's tab
		CTabCtrl	*pTab;
		pTab = m_PropSheet.GetTabControl();
		RECT	rect;
		pTab->GetItemRect(1, &rect);
		rectResized.bottom -= (rect.bottom - rect .top);
 
		sizeScroll.cx = rectResized.Width();		// save scroll size
		sizeScroll.cy = rectResized.Height();

		// adjust for border,menu,etc
		// NOTE: CalcWindowRect() does not correctly adjust for all styles, use AdjustWindowRectEx()
		//pFrame->CalcWindowRect(rectResized);	// incorrect!!!
		DWORD	st   = pFrame->GetStyle();
		DWORD	exst = pFrame->GetExStyle();
		::AdjustWindowRectEx(&rectResized, st, FALSE, exst);
		
		// move back to original frame origin
		rectResized.OffsetRect(rectFrame.left-rectResized.left, rectFrame.top-rectResized.top);

		CWnd* pParent = pFrame->GetParent();
		if (pParent)
			pParent->ScreenToClient(rectResized);

		pFrame->MoveWindow(rectResized);

		SetScrollSizes(MM_TEXT, sizeScroll);
	}
}

// determine the maximum extent of the forms controls.
// NOTE: this is called in the Create function to gather the control extent of the
//	template-based controls and assigned to m_sizeControl.  Any app created controls will not
//	be reflected in m_sizeControl!
// If you create new controls, 
//
CSize CPropPgFormView::GetMaxControl()
{
	CSize	size(0, 0);
	WINDOWPLACEMENT	wp;
	CWnd	*wnd = GetWindow(GW_CHILD);

	while (wnd) {
		wnd->GetWindowPlacement(&wp);
		if (wp.rcNormalPosition.right > size.cx)
			size.cx = wp.rcNormalPosition.right;
		if (wp.rcNormalPosition.bottom > size.cy)
			size.cy = wp.rcNormalPosition.bottom;
		wnd = wnd->GetNextWindow(GW_HWNDNEXT);
	}
	return size;
}


// call this function to set the horiz spacing between the bottom-most control on form and prop sheet
void CPropPgFormView::SetPropertySheetOffset(int iOffset /* = PS_Y_OFFSET */, BOOL bRedraw /* = FALSE */)
{
	m_PS_Offset = iOffset;
	if (bRedraw)
		ResizeParentFrame();
}

int CPropPgFormView::GetPropertySheetOffset()
{
	return m_PS_Offset;
}

// call this function after adding new controls on the fly
void CPropPgFormView::ResizeForNewControl()
{
	m_sizeControl = GetMaxControl();
	ResizeParentFrame();
}


/////////////////////////////////////////////////////////////////////////////
// CPropPgFormView diagnostics

#ifdef _DEBUG
void CPropPgFormView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPropPgFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CPropPgFormDoc* CPropPgFormView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPropPgFormDoc)));
	return (CPropPgFormDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormView message handlers

void CPropPgFormView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	AdjustPropSheetPosition();
}

void CPropPgFormView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
	AdjustPropSheetPosition();
}

void CPropPgFormView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
	AdjustPropSheetPosition();
}


void CPropPgFormView::OnInitialUpdate() 
{
	ResizeParentFrame();				// resize frame to fit
	CFormView::OnInitialUpdate();
}


