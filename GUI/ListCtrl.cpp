// ListCtrl.cpp : implementation file
/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
08/2004 CC Modification 
**************************************************************************/


#include "stdafx.h"

#include "ListCtrl.h"
#include "EditCell.h"
#include "geosys.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_EDITCELL 1001

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl

CMyListCtrl::CMyListCtrl (CString Text /* = "Some Text" */)
{
    DefaultText = Text;
    m_bDragging = FALSE;
	m_pimageListDrag = NULL;
	m_oldDNDStyle = NULL;
}

CMyListCtrl::~CMyListCtrl()
{
}

BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(CMyListCtrl)
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_WM_LBUTTONDOWN()
    ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_NOTIFY_REFLECT(LVN_BEGINRDRAG, OnBeginDrag)
    ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl message handlers

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void 
CMyListCtrl::Resize (int cx, int cy)
{
    CRect Rect (0, 0, cx, cy);
    MoveWindow (&Rect);
    InvalidateRect (Rect);
    SetColumnWidth (2, LVSCW_AUTOSIZE_USEHEADER);
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
06/2005 CC Modification edit list control item
**************************************************************************/
CEdit* 
CMyListCtrl::EditSubItem (int Item, int Column)
{
    // The returned pointer should not be saved

    // Make sure that the item is visible
    if (!EnsureVisible (Item, TRUE)) 
    {
	
		if (!EnsureVisible (Item, TRUE)) 
			return NULL;
    }

    // Make sure that nCol is valid
    CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();
    if (Column >= nColumnCount || GetColumnWidth (Column) < 5)
		return NULL;

    // Get the column offset
    int Offset = 0;
    for (int iColumn = 0; iColumn < Column; iColumn++)
		Offset += GetColumnWidth (iColumn);

    CRect Rect;
    GetItemRect (Item, &Rect, LVIR_BOUNDS);

    // Now scroll if we need to expose the column
    CRect ClientRect;
    GetClientRect (&ClientRect);
    if (Offset + Rect.left < 0 || Offset + Rect.left > ClientRect.right)
    {
		CSize Size;
		if (Offset + Rect.left > 0)
			Size.cx = -(Offset - Rect.left);
		else
			Size.cx = Offset - Rect.left;
		Size.cy = 0;
		Scroll (Size);
		Rect.left -= Size.cx;
    }

    // Get Column alignment
    LV_COLUMN lvCol;
    lvCol.mask = LVCF_FMT;
    GetColumn (Column, &lvCol);
    DWORD dwStyle;
    if ((lvCol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
    else if ((lvCol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
    else dwStyle = ES_CENTER;

    Rect.left += Offset+4;
    Rect.right = Rect.left + GetColumnWidth (Column) - 3;
    if (Rect.right > ClientRect.right)
		Rect.right = ClientRect.right;

    dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
    CEdit *pEdit = new CMyEditCell (this, Item, Column, GetItemText (Item, Column));
    pEdit->Create (dwStyle, Rect, this, IDC_EDITCELL);
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    theApp->OnUpdateGeoListCtrl();//CC

    return pEdit;
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
int 
CMyListCtrl::HitTestEx (CPoint& Point, int* pColumn)
{
    int ColumnNum = 0;
    int Row = HitTest (Point, NULL);
    
    if (pColumn)
		*pColumn = 0;

    // Make sure that the ListView is in LVS_REPORT
    if ((GetWindowLong (m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return Row;

    // Get the top and bottom row visible
    Row = GetTopIndex();
    int Bottom = Row + GetCountPerPage();
    if (Bottom > GetItemCount())
	    Bottom = GetItemCount();
    
    // Get the number of columns
    CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();

    // Loop through the visible rows
    for(; Row <= Bottom; Row++)
    {
		// Get bounding rect of item and check whether point falls in it.
		CRect Rect;
		GetItemRect (Row, &Rect, LVIR_BOUNDS);
		if (Rect.PtInRect (Point))
		{
			// Now find the column
			for (ColumnNum = 0; ColumnNum < nColumnCount; ColumnNum++)
			{
				int ColWidth = GetColumnWidth (ColumnNum);
				if (Point.x >= Rect.left && Point.x <= (Rect.left + ColWidth))
				{
					if (pColumn)
						*pColumn = ColumnNum;
					return Row;
				}
				Rect.left += ColWidth;
			}
		}
    }
    return -1;
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
BOOL 
hexNumberToInt (CString HexNumber, int& Number)
{
    char* pStopString;
    Number = strtoul (HexNumber, &pStopString, 16);
    return Number != ULONG_MAX;
} // hexNumberToInt



/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void 
CMyListCtrl::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    if (GetFocus() != this) 
		SetFocus();
    
    CListCtrl::OnHScroll (nSBCode, nPos, pScrollBar);
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void 
CMyListCtrl::OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    if (GetFocus() != this) 
		SetFocus();
    
    CListCtrl::OnVScroll (nSBCode, nPos, pScrollBar);
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void 
CMyListCtrl::OnEndLabelEdit (NMHDR* pNMHDR, LRESULT* pResult) 
{
    LV_DISPINFO *plvDispInfo = (LV_DISPINFO *)pNMHDR;
    LV_ITEM	*plvItem = &plvDispInfo->item;

    if (plvItem->pszText != NULL)
    {
		SetItemText (plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
    }
    *pResult = FALSE;
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void 
CMyListCtrl::OnLButtonDown (UINT nFlags, CPoint Point) 
{
    CListCtrl::OnLButtonDown (nFlags, Point);

    int Index;
    int ColNum;
    if ((Index = HitTestEx (Point, &ColNum)) != -1)
    {
		if (GetWindowLong (m_hWnd, GWL_STYLE) & LVS_EDITLABELS)
			EditSubItem (Index, ColNum);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl message handlers
/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CMyListCtrl::OnBeginDrag(LPNMHDR pnmhdr, LRESULT* /*pResult*/)
{
	CPoint          ptItem, ptAction, ptImage;
	NM_LISTVIEW     *pnmListView = (NM_LISTVIEW *)pnmhdr;

	m_oldDNDStyle = GetExtendedStyle();
	if (m_oldDNDStyle != 0) SetExtendedStyle(0);	// styles model original icon DND behavior
	
	ASSERT(!m_bDragging);
	m_bDragging = TRUE;
	m_iItemDrag = pnmListView->iItem;
	ptAction = pnmListView->ptAction;
	GetItemPosition(m_iItemDrag, &ptItem);  // ptItem is relative to (0,0) and not the view origin
	GetOrigin(&m_ptOrigin);

	ASSERT(m_pimageListDrag == NULL);
	m_pimageListDrag = CreateDragImage(m_iItemDrag, &ptImage);
	m_sizeDelta = ptAction - ptImage;   // difference between cursor pos and image pos
	m_ptHotSpot = ptAction - ptItem + m_ptOrigin;  // calculate hotspot for the cursor
	m_pimageListDrag->DragShowNolock(TRUE);  // lock updates and show drag image
	m_pimageListDrag->SetDragCursorImage(0, CPoint(0, 0));// define the hot spot for the new cursor image
	m_pimageListDrag->BeginDrag(0, CPoint(0, 0));

	ptAction -= m_sizeDelta;
	m_pimageListDrag->DragEnter(this, ptAction);
	m_pimageListDrag->DragMove(ptAction);  // move image to overlap original icon
	m_iItemDrop = -1;// no drop target
	SetCapture();
}
/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
08/2004 CC Implementation
07/2005 CC Mark selected point ?
**************************************************************************/
void CMyListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	long        lStyle;
	int         iItem;
	LV_ITEM     lvitem;

	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	lStyle &= LVS_TYPEMASK;  // drag will do different things in list and report mode

	if (m_bDragging)
	{
		ASSERT(m_pimageListDrag != NULL);
		m_pimageListDrag->DragMove(point - m_sizeDelta);  // move the image 
		if ((iItem = HitTest(point)) != -1)
		{
			lvitem.mask = LVIF_STATE;
			lvitem.stateMask = LVIS_DROPHILITED;  // highlight the drop target
			if (m_iItemDrop != -1)  // remove the drophighlighted from previous item
			{
				m_pimageListDrag->DragLeave(this);
				lvitem.iItem = m_iItemDrop;
				lvitem.iSubItem = 0;
				lvitem.state = 0;	// sets drophighlighted to FALSE  

				SetItem(&lvitem);
			}
			
			m_iItemDrop = iItem;
			m_pimageListDrag->DragLeave(this); // unlock the window and hide drag image

			if (lStyle == LVS_REPORT || lStyle == LVS_LIST)
			{
				lvitem.iItem = iItem;
				lvitem.iSubItem = 0;
				lvitem.state = LVIS_DROPHILITED;	// sets the drophighlighted

				SetItem(&lvitem);
				UpdateWindow();
			}
			point -= m_sizeDelta;
			m_pimageListDrag->DragEnter(this, point);  // lock updates and show drag image
		}
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CMyListCtrl::OnButtonUp(CPoint point)
{
	if (m_bDragging)  // end of the drag operation
	{
		long        lStyle;
		CString     cstr;

		lStyle = GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK;
		if (m_oldDNDStyle != 0) SetExtendedStyle(m_oldDNDStyle);

		m_bDragging = FALSE;
		ASSERT(m_pimageListDrag != NULL);
		m_pimageListDrag->DragLeave(this);

		SetItemState(m_iItemDrop, 0, LVIS_DROPHILITED);	// remove the drophighlighted from last highlighted target

		m_pimageListDrag->EndDrag();
		delete m_pimageListDrag;
		m_pimageListDrag = NULL;

		// The drop target's sub-item text is replaced by the dragged item's
		// main text
		if (lStyle == LVS_REPORT && m_iItemDrop != m_iItemDrag)
		{
			cstr = GetItemText(m_iItemDrag, 0);
			SetItemText(m_iItemDrop, 1, cstr);
		}

		//the character string "**" is added to the end of the drop target's main text
		if (lStyle == LVS_LIST && m_iItemDrop != m_iItemDrag)
		{
			cstr = GetItemText(m_iItemDrop, 0);
			cstr += _T("**");
			SetItemText(m_iItemDrop, 0, cstr);
		}

		  // move the icon
		if (lStyle == LVS_ICON || lStyle == LVS_SMALLICON)
		{
			point -= m_ptHotSpot;  // the icon should be drawn exactly where the image is
			point += m_ptOrigin;
			SetItemPosition(m_iItemDrag, point);  // just move the dragged item
		}

		::ReleaseCapture();
	}
}
/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CMyListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(point);
	CListCtrl::OnLButtonUp(nFlags, point);
}
/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
08/2004 CC Implementation
**************************************************************************/
void CMyListCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(point);
	CListCtrl::OnRButtonUp(nFlags, point);
}