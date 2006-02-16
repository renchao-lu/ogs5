/**************************************************************************
GeoSys GUI - Object: List Control
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/

#if !defined(AFX_LISTCTRL_H__2EB671B4_0711_11D3_90AB_00E029355177__INCLUDED_)
#define AFX_LISTCTRL_H__2EB671B4_0711_11D3_90AB_00E029355177__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl window

class CMyListCtrl : public CListCtrl
{
public:
    CMyListCtrl (CString Text = "Some Text");
    virtual ~CMyListCtrl();

    CString DefaultText;

    CEdit*  EditSubItem (int Item, int Column);
    int	    HitTestEx (CPoint& Point, int* Column);
 
    void    Resize (int cx, int cy);


    //------------------------------------

    BOOL            m_bDragging;
	int             m_iItemDrag;
	int             m_iItemDrop;
	CPoint          m_ptHotSpot;
	CPoint          m_ptOrigin;
	CSize           m_sizeDelta;
	DWORD			m_oldDNDStyle;
	CImageList      *m_pimageListDrag;

    //{{AFX_VIRTUAL(CMyListCtrl)
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CMyListCtrl)
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    //----------------------------------------------------------------------
    afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
public:
    void    OnButtonUp(CPoint point);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRL_H__2EB671B4_0711_11D3_90AB_00E029355177__INCLUDED_)
