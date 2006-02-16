
// Implements Zooming functions in a CScrollView window
//   CC 04/2004 Implementation

//////////////////////////////////////////////////////////////////////////

#ifndef GEOSYSZOOMVIEW_H
#define GEOSYSZOOMVIEW_H

/////////////////////////////////////////////////////////////////////////////
// CGeoSysZoomView view
class CGeoSysZoomView : public CScrollView
{
	DECLARE_DYNCREATE(CGeoSysZoomView)
protected:
	CGeoSysZoomView();			// protected constructor used by dynamic creation

// Operations
public:
	// Overridden CScrollView member functions
	void SetZoomSizes(SIZE sizeTotal, const SIZE& sizePage = sizeDefault, const SIZE& sizeLine = sizeDefault);
	void CenterOnLogicalPoint(CPoint ptCenter);
	CPoint GetLogicalCenterPoint(void);

	// Zooming functions
	typedef enum {MODE_ZOOMOFF, MODE_ZOOMIN, MODE_ZOOMOUT,m_drawingPoint} ZoomMode_;
	void      SetZoomMode(ZoomMode_ zoomMode);
	ZoomMode_ GetZoomMode() {return m_zoomMode;};
	int       DoZoomIn    (CRect &rect);
	int       DoZoomIn    (CPoint *point = NULL, float delta = 1.25);
	int       DoZoomOut   (CPoint *point = NULL, float delta = 1.25);
	int       DoZoomFull  ();
HCURSOR   m_hZoomCursor;
HCURSOR   m_hPointCursor;
	// Override this to get notified of zoom scale change
	virtual void NotifyZoom(void) {};

	// Zooming utility functions
	void      ViewDPtoLP (LPPOINT lpPoints, int nCount = 1);
	void      ViewLPtoDP (LPPOINT lpPoints, int nCount = 1);
	void      ClientToDevice(CPoint &point);
	void      NormalizeRect(CRect &rect);
    void 		 DrawBox(CDC &dc, CRect &rect, BOOL xor = TRUE);
	void 		 DrawLine(CDC &dc, const int &x1, const int &y1, 
							 const int &x2, const int &y2, BOOL xor = TRUE);
	

// Implementation
protected:
	virtual ~CGeoSysZoomView();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif //_DEBUG
	
    virtual void OnDraw(CDC* pDC) {CGeoSysZoomView::OnDraw(pDC);}; // Pure Virtual
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

private:
	// Internally called functions
	void 		 PersistRatio(const CSize &orig, CSize &dest, CPoint &remainder);
	void      CalcBars(void);

	// Private member variables
	ZoomMode_ m_zoomMode;
	BOOL      m_bCaptured;
	CRect     m_ptDragRect;
	CSize     m_origTotalDev;           // Origional total size in device units
	CSize     m_origPageDev;            // Origional per page scroll size in device units
	CSize     m_origLineDev;            // Origional per line scroll size in device units
	float     m_zoomScale;
public:
	// Generated message map functions
	//{{AFX_MSG(CGeoSysZoomView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
#endif
