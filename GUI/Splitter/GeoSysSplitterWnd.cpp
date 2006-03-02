// GeoSysSplitter.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSysSplitterWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysSplitter

CGeoSysSplitter::CGeoSysSplitter()
{
}

CGeoSysSplitter::~CGeoSysSplitter()
{
}


BEGIN_MESSAGE_MAP(CGeoSysSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CGeoSysSplitter)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CGeoSysSplitter::SetActivePane( int row, int col, CWnd* pWnd)
{

  CSplitterWnd::SetActivePane(row,col,pWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysSplitter message handlers
