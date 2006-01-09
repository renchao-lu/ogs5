// geosyszoomview.cpp : Implements Zooming functions in a CScrollView window

//CC 04/2004 Implementation

/////////////////////////////////////////////////
#include "stdafx.h"
#include "geosyszoomview.h"
#include "resource.h"
#include "MainFrm.h"
#include "geosys.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CGeoSysZoomView, CScrollView)
	//{{AFX_MSG_MAP(CGeoSysZoomView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MAXZOOMIN    4        // Maximum zoom-in factor
#define PICKMARGIN  10        // Screen pixels apart for region zoom


IMPLEMENT_DYNCREATE(CGeoSysZoomView, CScrollView)


/**************************************************************************
GeoSys-GUI: CGeoSysZoomView()
Programing: Constructor for the CGeoSysZoomView class 
04/2004 CC Implementation
**************************************************************************/
CGeoSysZoomView::CGeoSysZoomView()
: CScrollView()
{
	// Init zoom mode to nothing
	m_zoomMode    = MODE_ZOOMOFF;
	m_bCaptured   = FALSE;
	m_zoomScale   = 1.0;
	m_ptDragRect.SetRectEmpty();

	// Load the zoom cursor
	m_hZoomCursor = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_ZOOMCURSOR));
    m_hPointCursor = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_CURSOR_DRAW));
    
	// Default to centering on full fit
	m_bCenter = TRUE;
} 


/**************************************************************************
GeoSys-GUI: CGeoSysZoomView()
Programing: Destructor for the CGeoSysZoomView class 
04/2004 CC Implementation
**************************************************************************/
CGeoSysZoomView::~CGeoSysZoomView()
{
	// Clean up the cursors if they were loaded properly
	if (m_hZoomCursor) DestroyCursor(m_hZoomCursor);
    if (m_hPointCursor) DestroyCursor(m_hPointCursor);
} // 

/////////////////////////////////////////////////////////////////////////////
// CGeoSysZoomView overridden default CScrollView members
/////////////////////////////////////////////////////////////////////////////


/**************************************************************************
GeoSys-GUI: SetZoomSizes()
Programing: Set up the CGeoSysZoomView class with the logical page size, and 
   			 scrolling page/line units.  
   			 This replaces CScrollView::SetScrollSizes.
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::SetZoomSizes (
	SIZE sizeTotal,
	const SIZE& sizePage, // in logical units
	const SIZE& sizeLine) // in logical units
{
	// Set up the defaults
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);
	m_nMapMode    = MM_ANISOTROPIC; // Need for arbitrary scaling
	m_totalLog    = sizeTotal;
	// Setup default Viewport extent to be conversion of Window extent
	// into device units.
	//BLOCK for DC
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);

		// total size
		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
	} // Release DC here

	// Save the origional Viewport Extent
	m_origTotalDev = m_totalDev;
	// Save the origional scrollbar info - for CalcBars
	m_origPageDev = sizePage;
	m_origLineDev = sizeLine;
	// Fugure out scroll bar info
	CalcBars();
	// Notify the class that the zoom scale was set
	NotifyZoom();
} // SetZoomSizes


/**************************************************************************
GeoSys-GUI: OnPrepareDC()
Programing: Override of CScrollView for MM_ANISOTROPIC zoom mode
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::OnPrepareDC (
	CDC* pDC, 
	CPrintInfo* pInfo)
{
#ifdef _DEBUG
	if (m_nMapMode != MM_ANISOTROPIC) {
		TRACE0("Error: must call SetZoomSizes() before painting zoom view\n");
		ASSERT(FALSE);
		return;
	}
#endif //_DEBUG

	ASSERT_VALID(pDC);
	ASSERT(m_totalLog.cx >= 0 && m_totalLog.cy >= 0);
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cx >= 0);
	// Set the Mapping mode, and the window and viewport extents
	pDC->SetMapMode(m_nMapMode);
	pDC->SetWindowExt(m_totalLog);   // in logical coordinates
	CPoint ptVpOrg;

	if (!pDC->IsPrinting()) {
		pDC->SetViewportExt(m_totalDev); // in device  coordinates

		// by default shift viewport origin in negative direction of scroll
		ASSERT(pDC->GetWindowOrg() == CPoint(0,0));
		ptVpOrg = -GetDeviceScrollPosition();

		// Center full fit
		if (m_bCenter) {
			CRect rect;
			GetClientRect(&rect);

			// if client area is larger than total device size,
			// override scroll positions to place origin such that
			// output is centered in the window
			if (m_totalDev.cx < rect.Width())
				ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
			if (m_totalDev.cy < rect.Height())
				ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
		}
	} else {
		// Special case for printing
		CSize  printSize;
		printSize.cx = pDC->GetDeviceCaps(HORZRES);
		printSize.cy = pDC->GetDeviceCaps(VERTRES);
		// Maintain the origional ratio, setup origin shift
		PersistRatio(m_totalLog, printSize, ptVpOrg);
		// Zoom completely out
		pDC->SetViewportExt(printSize);
	}
	// Set the new origin
	pDC->SetViewportOrg(ptVpOrg);
	// For default Printing behavior
	CView::OnPrepareDC(pDC, pInfo);
} // OnPrepareDC


/**************************************************************************
GeoSys-GUI: CalcBars()
Programing: Update the scrollbars - uses logical units
				 Call when the Viewport changes size.
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::CalcBars (void)
{
	{	// BLOCK for DC
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);

		// Calculate new device units for scrollbar
		// Start with origional logical units from SetScrollPos
		m_pageDev = m_origPageDev;
		dc.LPtoDP((LPPOINT)&m_pageDev);
		m_lineDev = m_origLineDev;
		dc.LPtoDP((LPPOINT)&m_lineDev);
	} // Free DC

	// Make sure of the range
	if (m_pageDev.cy < 0)  m_pageDev.cy = -m_pageDev.cy;
	if (m_lineDev.cy < 0)  m_lineDev.cy = -m_lineDev.cy;

	// If none specified - use one tenth
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	if (m_pageDev.cx == 0) m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0) m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0) m_lineDev.cx = m_pageDev.cx  / 10;
	if (m_lineDev.cy == 0) m_lineDev.cy = m_pageDev.cy  / 10;

	// Now update the scrollbars
	if (m_hWnd != NULL) {
		UpdateBars();
		Invalidate(TRUE); // Zoom scale changed, redraw all
	}
} // CalcBars


/**************************************************************************
GeoSys-GUI: AssertValid()
Programing:  Make sure valid class
04/2004 CC Implementation
**************************************************************************/
#ifdef _DEBUG
void CGeoSysZoomView::AssertValid() const
{
	// Bypass CScrollView because of MM_ANISOTROPIC map mode
	CView::AssertValid();
} // AssertValid
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysZoomView custom members to implement zooming functionality
/////////////////////////////////////////////////////////////////////////////

/**************************************************************************
GeoSys-GUI: DoZoomIn()
Programing:  Zoom the view in on a rect
04/2004 CC Implementation
**************************************************************************/
int  CGeoSysZoomView::DoZoomIn (
	CRect &rect)   // rect in logical coordinates
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);
	// Make sure that the rect is normalized
	CRect zoomRect = rect;
	NormalizeRect(zoomRect);

	// Get the center of rect
	CPoint ptCenter;
	ptCenter.x = ((zoomRect.left + zoomRect.right)  / 2);
	ptCenter.y = ((zoomRect.top  + zoomRect.bottom) / 2);

	// See if the rect is small enough for a point zoom (Device coordinates)
	CRect rectDP = zoomRect;
	ViewLPtoDP((LPPOINT)&rectDP, 2);
   BOOL bPointZoom = (max(rectDP.Width(), rectDP.Height()) < PICKMARGIN);
	if (bPointZoom) {
		// Just do normal point zoom
		return DoZoomIn(&ptCenter);
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	// Calculate the new zoom scale.
	float scaleH = (float) (clientRect.right  + 1) / (float) zoomRect.Width();
	float scaleV = (float) (clientRect.bottom + 1) / (float) zoomRect.Height();
	// Keep the scale Isotropic
	m_zoomScale = min(scaleH, scaleV);

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);
	CalcBars();

	// Set the current center point.
	CenterOnLogicalPoint(ptCenter);
	// Notify the class that a new zoom scale was done
	NotifyZoom();
	return TRUE;
} // DoZoomIn (Rect)



/**************************************************************************
GeoSys-GUI: DoZoomIn()
Programing:  Zoom the view in on a point by the specified scale factor
04/2004 CC Implementation
**************************************************************************/
int  CGeoSysZoomView::DoZoomIn (
	CPoint *point,   // point in logical coordinates
	float  delta)    // scale factor
{
	CPoint ptCenter;

	ASSERT(m_nMapMode == MM_ANISOTROPIC);
	// Save the current center point.
	if (!point) {
   	ptCenter = GetLogicalCenterPoint();
	} else {
   	ptCenter = *point;
	}

	// Increase the zoom scale.
	m_zoomScale *= delta;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);
	CalcBars();
	// Set the current center point.
	CenterOnLogicalPoint(ptCenter);
	// Notify the class that a new zoom scale was done
	NotifyZoom();
	return TRUE;
} // DoZoomIn (Pt)


/**************************************************************************
GeoSys-GUI: DoZoomOut()
Programing:  Zoom the view out on a point by one scale factor
04/2004 CC Implementation
**************************************************************************/
int  CGeoSysZoomView::DoZoomOut (
	CPoint *point,   // point in logical coordinates
	float  delta)    // scale factor
{
	CPoint ptCenter;

	ASSERT(m_nMapMode == MM_ANISOTROPIC);
	// Save the current center point.
	if (!point) {
   	ptCenter = GetLogicalCenterPoint();
	} else {
   	ptCenter = *point;
	}

	// Decrease the zoom scale.
	m_zoomScale /= delta;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);
	CalcBars();
	// Set the current center point (logical coordinates.
	CenterOnLogicalPoint(ptCenter);
	// Notify the class that a new zoom scale was done
	NotifyZoom();
	return TRUE;
} // DoZoomOut


/**************************************************************************
GeoSys-GUI: DoZoomFull()
Programing:  Zoom the view to full state
04/2004 CC Implementation
**************************************************************************/
int  CGeoSysZoomView::DoZoomFull (void)
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);
	CRect  rc;
	CPoint pt;
	CSize  sizeSb;
	// Just set Viewport Extent to Client size for full fit
	GetTrueClientSize(m_totalDev, sizeSb);
	// Maintain origional ratio
	PersistRatio(m_totalLog, m_totalDev, pt);
	// Set the new zoom scale (could use cx or cy)
	m_zoomScale = (float) m_totalDev.cx / m_origTotalDev.cx;
	// Remove the scrollbars
	UpdateBars();
	// Complete redraw
	Invalidate(TRUE);
	// Notify the class that a new zoom scale was done
	NotifyZoom();
	return TRUE;
} // DoZoomInFull


/**************************************************************************
GeoSys-GUI: SetZoomMode()
Programing:  Put the view into the specified zoom mode
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::SetZoomMode (
	ZoomMode_ zoomMode)
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);
	if (zoomMode != m_zoomMode) {
		m_zoomMode = zoomMode;
		// Force cursor change now
		OnSetCursor(NULL, HTCLIENT, 0);
	}
} // SetZoomMode



/**************************************************************************
GeoSys-GUI: CenterOnLogicalPoint()
Programing:  Same as CScrollView::CenterOnPoint, but for logical coordinates
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::CenterOnLogicalPoint(CPoint pt)
{
	// Convert the point to device coordinates
	ViewLPtoDP(&pt);
	// Account for scroll bar position
	ClientToDevice(pt);
	// Use CScrollView's function for device coordinates
	CScrollView::CenterOnPoint(pt);
} // CenterOnLogicalPoint

/**************************************************************************
GeoSys-GUI: GetLogicalCenterPoint()
Programing:  Get the center of screen in logical coordinates
04/2004 CC Implementation
**************************************************************************/
CPoint CGeoSysZoomView::GetLogicalCenterPoint (void)  // Point in logical units
{
	CPoint pt;
	CRect rect;
	// Get the center of screen
	GetClientRect(&rect);

	pt.x = (rect.Width()  / 2);
	pt.y = (rect.Height() / 2);


	// Convert the point to logical coordinates
	ViewDPtoLP(&pt);
	return pt;
} // GetLogicalCenterPoint




/**************************************************************************
GeoSys-GUI: OnLButtonDown()
Programing:  Handle the left mouse click
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::OnLButtonDown(
	UINT nFlags, 
	CPoint point)
{
	// Pass the message along
	CScrollView::OnLButtonDown(nFlags, point);

	switch (m_zoomMode) {
		case MODE_ZOOMIN:
			// Capture the mouse for zooming in
			m_bCaptured = TRUE;
			SetCapture();
			// Save the mouse down point for XOR rect
			ViewDPtoLP(&point);
			m_ptDragRect.SetRect(point.x, point.y, point.x, point.y);
			// Set the cursor to the cross hair
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS)));
			break;

		default:
			// Do nothing.
			break;
	}
} // OnLButtonDown


/**************************************************************************
GeoSys-GUI: OnMouseMove()
Programing:  Handle the mouse movement
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::OnMouseMove(UINT nFlags, CPoint point)
{
	// Pass the message along
	CScrollView::OnMouseMove(nFlags, point);

	if (m_bCaptured) {
		// Get the Device Context
		CClientDC dc(this);
		OnPrepareDC(&dc);
	
		switch (m_zoomMode) {
			case MODE_ZOOMIN:
				// Draw the drag-rect
				// Erase last rect
				DrawBox(dc, m_ptDragRect);
				// Draw new rect
				dc.DPtoLP(&point);
				m_ptDragRect.BottomRight() = point;
				DrawBox(dc, m_ptDragRect);
				break;

			default:
				// Do nothing.
				break;
		}
	}
} // OnMouseMove


/**************************************************************************
GeoSys-GUI: OnLButtonUp()
Programing:  Handle the left mouse release
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::OnLButtonUp (
	UINT nFlags, 
	CPoint point)
{
	// Pass the message along
	CScrollView::OnLButtonUp(nFlags, point);
	
	switch (m_zoomMode) {
		case MODE_ZOOMIN:
			// Uncapture the mouse?
			if (m_bCaptured) {
				m_bCaptured = FALSE;
				ReleaseCapture();
				// Set back the cross cursor to the Z
				::SetCursor(m_hZoomCursor);
				// Get the Device Context
				CClientDC dc(this);
				OnPrepareDC(&dc);
				// Erase the bounding box
				DrawBox(dc, m_ptDragRect);
				// Now Zoom in on logical rectangle
				DoZoomIn(m_ptDragRect);
			}
			break;

		case MODE_ZOOMOUT:
			ViewDPtoLP(&point);
			DoZoomOut(&point);
			break;

		default:
			// Do nothing.
			break;
	}
} // OnLButtonUp


/**************************************************************************
GeoSys-GUI: OnRButtonDown()
Programing: Handle the right mouse click - CANCELS CURRENT ZOOM MODE OR DRAG
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonDown(nFlags, point);

	// See if currently captured
	if (m_bCaptured) {
		// Maintain current mode, just stop current drag
		m_bCaptured = FALSE;
		ReleaseCapture();
		// Get the Device Context
		CClientDC dc(this);
		OnPrepareDC(&dc);

		switch (m_zoomMode) {
			case MODE_ZOOMIN:
				// Erase last rect
				DrawBox(dc, m_ptDragRect);
				break;

			default:
				// Do nothing.
				break;
		}
	} else {
		// Cancel current mode
		m_zoomMode = MODE_ZOOMOFF;
	}
} // OnRButtonDown


/**************************************************************************
GeoSys-GUI: DrawBox()
Programing: Draw a box - XOR if want to erase
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::DrawBox (
	CDC   &dc,
	CRect &rect,
	BOOL  xor)
{
	CPen pen;
	// Save the device context
	dc.SaveDC();
	if (xor) {
		dc.SetROP2(R2_NOTXORPEN);
		pen.CreatePen(PS_DASH,  0, RGB(0, 0, 0)); // 0 width = 1 device unit
	} else {
		pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}
	dc.SelectObject(&pen);
	// Draw the rect with lines (eliminate rect middle fill)
	dc.MoveTo(rect.left,  rect.top);
	dc.LineTo(rect.right, rect.top);
	dc.LineTo(rect.right, rect.bottom);
	dc.LineTo(rect.left,  rect.bottom);
	dc.LineTo(rect.left,  rect.top);
	// Clean up
	dc.RestoreDC(-1);
} // DrawBox



/**************************************************************************
GeoSys-GUI: DrawLine()
Programing: Draw a line - XOR to erase
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::DrawLine (
	CDC &dc,
	const int &x1, // Logical units
	const int &y1,
	const int &x2,
	const int &y2,
	BOOL      xor)
{	
	CPen pen;
	// Save the device context
	dc.SaveDC();
	if (xor) {
		dc.SetROP2(R2_NOTXORPEN);
		pen.CreatePen(PS_DASH,  0, RGB(0, 0, 0)); // 0 width = 1 device unit
	} else {
		pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}
	dc.SelectObject(&pen);

	// Draw the line
	dc.MoveTo(x1, y1);
	dc.LineTo(x2, y2);
	// Clean up
	dc.RestoreDC(-1);
} // DrawLine


/**************************************************************************
GeoSys-GUI: OnSetCursor()
Programing: Set the cursor depending on the zoom mode
04/2004 CC Implementation
07/2005 CC
**************************************************************************/
BOOL CGeoSysZoomView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest != HTCLIENT)
		return CScrollView::OnSetCursor(pWnd, nHitTest, message);

	switch (m_zoomMode) {
		case MODE_ZOOMOFF:
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			break;
        case m_drawingPoint:
            ::SetCursor(m_hPointCursor);
            break;
		default:
			// All other zoom modes
			::SetCursor(m_hZoomCursor);
			break;
	} // Zoom mode select
    //
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();//CC
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();//CC
 /* if(mainframe->m_bIsPointEditorOn)
  SetCursor(m_hPointCursor);
  else 
  SetCursor(::LoadCursor(NULL, IDC_ARROW));
  if(theApp->pPolyline->GetSafeHwnd()!=NULL)
  {
   if(theApp->pPolyline->add_polyline)
   SetCursor(m_hPointCursor);
   else
   SetCursor(::LoadCursor(NULL, IDC_ARROW));

  }*/
  if(mainframe->m_bIsPointEditorOn)
  SetCursor(m_hPointCursor);
  else if(theApp->pPolyline->GetSafeHwnd()!=NULL){
  if(theApp->pPolyline->add_polyline)
   SetCursor(m_hPointCursor);
   else
   SetCursor(::LoadCursor(NULL, IDC_ARROW));
  }
  else if(mainframe->m_bIsPointEditorOn==0)
  SetCursor(::LoadCursor(NULL, IDC_ARROW));
  

    return TRUE;
}


/**************************************************************************
GeoSys-GUI: ViewDPtoLP()
Programing: Same as DPtoLP, but gets the Client DC for the view
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::ViewDPtoLP (
	LPPOINT lpPoints,
	int     nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	ASSERT(m_nMapMode > 0); // must be set
	CWindowDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(lpPoints, nCount);
} // ViewDPtoLP

/**************************************************************************
GeoSys-GUI: ViewLPtoDP()
Programing:  Same as LPtoDP, but gets the Client DC for the view
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::ViewLPtoDP (
	LPPOINT lpPoints,
	int     nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	ASSERT(m_nMapMode > 0); // must be set
	CWindowDC dc(this);
	OnPrepareDC(&dc);
	dc.LPtoDP(lpPoints, nCount);
} // ViewLPtoDP


/**************************************************************************
GeoSys-GUI: ClientToDevice()
Programing:  Convert from Client coordinates to relative Device coordinates
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::ClientToDevice (
	CPoint &point)
{
	// Need to account for scrollbar position
	CPoint scrollPt = GetDeviceScrollPosition();
	point.x += scrollPt.x;
	point.y += scrollPt.y;
} // ClientToDevice


/**************************************************************************
GeoSys-GUI: NormalizeRect()
Programing:  Normalize the rectangle
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::NormalizeRect (
	CRect &rect)
{
   if (rect.left > rect.right) {
      int r = rect.right;
      rect.right = rect.left;
      rect.left = r;
   }
   if (rect.top > rect.bottom) {
      int b = rect.bottom;
      rect.bottom = rect.top;
      rect.top = b;
   }
} // NormalizeRect


/**************************************************************************
GeoSys-GUI: PersistRatio()
Programing:  Make a CSize maintain the given ratio (by shrinking if nescessary)
04/2004 CC Implementation
**************************************************************************/
void CGeoSysZoomView::PersistRatio (
	const CSize &orig,
	CSize       &dest,
	CPoint      &remainder)
{
	float ratio1 = (float) orig.cx / orig.cy;
	float ratio2 = (float) dest.cx / dest.cy;
	int   newSize;

	// Do nothing if they are the same
	if (ratio1 > ratio2) {
		// Shrink hieght
		newSize = (int)(dest.cx / ratio1);
		remainder.x = 0;
		remainder.y = dest.cy - newSize;
		dest.cy = newSize;
	} else if (ratio2 > ratio1) {
		// Shrink width
		newSize = (int)(dest.cy * ratio1);
		remainder.x = dest.cx - newSize;
		remainder.y = 0;
		dest.cx = newSize;
	}
} // PersistRatio

