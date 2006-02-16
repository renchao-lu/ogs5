// GeoSysEditView.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysEdit

IMPLEMENT_DYNCREATE(CGeoSysEdit, CEditView)

CGeoSysEdit::CGeoSysEdit()
{
}

CGeoSysEdit::~CGeoSysEdit()
{
}


BEGIN_MESSAGE_MAP(CGeoSysEdit, CEditView)
	//{{AFX_MSG_MAP(CGeoSysEdit)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysEdit drawing

void CGeoSysEdit::OnDraw(CDC* pDC)
{
    pDC=pDC;//TK
	//CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysEdit diagnostics

#ifdef _DEBUG
void CGeoSysEdit::AssertValid() const
{
	CEditView::AssertValid();
}

void CGeoSysEdit::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysEdit message handlers
