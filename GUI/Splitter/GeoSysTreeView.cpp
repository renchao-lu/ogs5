// GeoSysTree.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysTreeView.h"
#include "OGLEnabledView.h"
#include "TreeOptionsCtrl.h"
#include "GeoSysVisualFx.h"
#include "GeoSysMdiFrames.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int m_nSearchIndex;
/////////////////////////////////////////////////////////////////////////////
// CGeoSysTree

IMPLEMENT_DYNCREATE(CGeoSysTree, CTreeView)

CGeoSysTree::CGeoSysTree()
{
}

CGeoSysTree::~CGeoSysTree()
{
}


BEGIN_MESSAGE_MAP(CGeoSysTree, CTreeView)
	//{{AFX_MSG_MAP(CGeoSysTree)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED,OnSelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysTree drawing

void CGeoSysTree::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	pDC=pDC;
	pDoc=pDoc;
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysTree diagnostics

#ifdef _DEBUG
void CGeoSysTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void CGeoSysTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysTree message handlers

void CGeoSysTree::OnInitialUpdate() 
{
	//CTreeView::OnInitialUpdate(); 
	CreateLVData();
	CTreeView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
}


BOOL CGeoSysTree::PreCreateWindow(CREATESTRUCT& cs)
{ 
 cs.style |= TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT;
 /*ergänzen um wenn die Items  edititerbar sein soll| TVS_EDITLABELS*/ 
 return CTreeView::PreCreateWindow(cs); 

} 


int CGeoSysTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  // TODO: Add your specialized creation code here
  CTreeCtrl& tc = GetTreeCtrl();
  // Set tree control styles
  long style = GetWindowLong(tc.m_hWnd,GWL_STYLE);
  style |= (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | 
            TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP);
  SetWindowLong(tc.m_hWnd,GWL_STYLE,style);
  // Insert some items
  HTREEITEM h = tc.InsertItem("GEOMETRIC DATA",8,8);
  tc.InsertItem("Points",2,3,h);
  tc.InsertItem("Lines", 2,3,h);
  tc.InsertItem("Polylines", 2,3,h);
  tc.InsertItem("Surfaces", 2,3,h);
  tc.InsertItem("Volumes", 2,3,h);
  
  h = tc.InsertItem("MESH DATA",8,8);
  tc.InsertItem("Nodes", 2,3,h);
  tc.InsertItem("Elements", 2,3,h); 
	return 0;
}

void CGeoSysTree::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CTreeView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CGeoSysTree::DoDataExchange(CDataExchange* pDX)
{

	pDX=pDX;
	//CString ItemText = GetItemText(hItem);

}

void CGeoSysTree::CreateLVData()
{ 
    CTreeCtrl&  trCtrl = GetTreeCtrl(); 
	

	hItem = trCtrl.InsertItem( "MeshTest 1", 9, 9 ); 
	trCtrl.InsertItem( "Volume(3D)/Area(2D)", 2, 3, hItem );
	trCtrl.InsertItem( "Minimum Angle", 2, 3, hItem );
	trCtrl.InsertItem( "Maximum Angle", 2, 3, hItem );

	hItem = trCtrl.InsertItem( "MeshTest 2", 9, 9 );
	trCtrl.InsertItem( "Shear", 2, 3, hItem );
	trCtrl.InsertItem( "Shape", 2, 3, hItem );

	m_TreeImages.Create( IDB_TREE_CTRL_OPTIONS, 16, 1, RGB(0, 255, 0) ); 
	trCtrl.SetImageList(&m_TreeImages, TVSIL_NORMAL ); 
} 

void CGeoSysTree::OnLButtonDown(UINT nFlags, CPoint point)
{
  int nImage;
  int nSelectedImage;
  UINT uFlags=0;
  CTreeCtrl&  trCtrl = GetTreeCtrl(); 
  hItem = trCtrl.HitTest(point, &uFlags);
    //If the mouse was over the label, icon or to the left or right of the item ?
	if ((uFlags & TVHT_ONITEM) || (uFlags & TVHT_ONITEMINDENT) || (uFlags & TVHT_ONITEMRIGHT))
	{
		CString ItemText = trCtrl.GetItemText(hItem);
		 
		BOOL bSuccess = trCtrl.GetItemImage(hItem, nImage, nSelectedImage);
		ASSERT(bSuccess);
		 
        UpdateData(TRUE);
        //CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

		
		if (nImage == 2 && ItemText == "Points")
		{
			//Display_Control_OGLView(1); 
		}
		if (nImage == 2 && ItemText == "Lines")
		{
			Display_Control_OGLView(2); 
		}
		if (nImage == 2 && ItemText == "Polylines")
		{
			Display_Control_OGLView(3); 
		}
		if (nImage == 2 && ItemText == "Surfaces")
		{
			Display_Control_OGLView(4); 
		}
		if (nImage == 2 && ItemText == "Volumes")
		{
			Display_Control_OGLView(0); 
		}
	}  
	CWnd::OnLButtonDown(nFlags, point);
}

void CGeoSysTree::OnSelChanged(NMHDR * pNMHDR, LRESULT *pResult)
{
   NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
   pNMTreeView=pNMTreeView;//TK
   //Display_Control_OGLView(0);

   //GetDocument ()->UpdateAllViews (this, 1L , NULL);
   //&CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd; // Get the active MDI child window.
   //CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();// Get the active view attached to the active MDI child window.
   
   //CView *pView = pChild->GetActiveView();
   //pChild->SetActiveView(pView);

 
   //CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd; // Get the active MDI child window.
   //CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();// Get the active view attached to the active MDI child window.
   //CView *pViewOld = pChild->GetActiveView();
   //pChild->SetActiveView(COGLEnabledView,TRUE);

   //pDocTemplate = theApp.GetDocTemplate(DOCTEMPLATE_OGLSPLIT_VIEW);
   /*CFrameWnd			*pWndSecondChild ;
   	CMultiDocTemplate	*pDocTemplate ;
	CDocument* m_pDoc = GetDocument();
	pDocTemplate = theApp.GetDocTemplate(DOCTEMPLATE_OGLSPLIT_VIEW);
	pWndSecondChild = pDocTemplate->CreateNewFrame(m_pDoc, NULL);	
	ASSERT(pWndSecondChild != NULL);
	pDocTemplate->InitialUpdateFrame(pWndSecondChild, m_pDoc);
*/
   //Invalidate(TRUE);

   *pResult = 0;

}
