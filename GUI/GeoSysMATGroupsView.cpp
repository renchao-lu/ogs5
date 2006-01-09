// GeoSysMATGroupsView.cpp : Implementierung der Klasse GeoSysMATGroupsView
//

#include "stdafx.h"
#include "GeoSys.h"

#include "GeoSysDoc.h"
#include "GeoSysMATGroupsView.h"
#include "OGLEnabledView.h"
#include "MainFrm.h"

#include "rf_p.h" // GetWindowAttributes

// Dialogs
#include "gs_sfc.h"
#include "gs_graphics.h"
#include "gs_mat_mp.h"
#include "gs_mat_sp.h"
#include ".\geosysmatgroupsview.h"
// GeoLib
// MSHLib
#include "msh_lib.h"

#include <math.h> // amod

//zoom
#include ".\geosyszoomview.h"
#include "afxpriv.h"    // For WM_SETMESSAGESTRING

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysMATGroupsView

IMPLEMENT_DYNCREATE(CGeoSysMATGroupsView, CGeoSysZoomView)

BEGIN_MESSAGE_MAP(CGeoSysMATGroupsView, CGeoSysZoomView)
    ON_COMMAND(ID_VIEW_MAT_GROUPS_PROPERTIES, OnProperties)
    ON_COMMAND(ID_MMP_EDITOR, OnMATGroupEditor)
    ON_COMMAND(ID_MSP_EDITOR, OnMatSolidProperties)
    ON_WM_LBUTTONDBLCLK()
    ON_COMMAND(ID_MATGROUPS_EDITOR, OnMATGroupEditor)

    //zoom
     //05/2004 CC
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ZOOMFULL, OnViewZoomfull)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysMATGroupsView Konstruktion/Destruktion

/**************************************************************************
GeoSys-GUI: CGeoSysView()
Programing:
09/2003 OK Implementation
07/2005 OK m_dlg_mat_editor
**************************************************************************/
CGeoSysMATGroupsView::CGeoSysMATGroupsView()
{
  m_iDisplayPLY = 0;
  m_iDisplaySFC = 0;
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_bDisplayMaterialGroups = false;
  m_bShowLabels = false;
//OK_MODELESS
  //----------------------------------------------------------------------
  m_dlg_mat_editor = new CMATGroupEditor;
  m_dlg_mat_editor->Create(IDD_MAT_GROUP_EDITOR);
  m_dlg_mat_editor->ShowWindow(SW_SHOW);
}

CGeoSysMATGroupsView::~CGeoSysMATGroupsView()
{
//OK_MODELESS
  m_dlg_mat_editor->CloseWindow();
  delete m_dlg_mat_editor;
}


/**************************************************************************
GeoSys-GUI Function OnInitialUpdate
Programing:
05/2004 CC Implementation
09/2004 OK Properties Dialog
07/2005 OK Properties dialog wieder raus
**************************************************************************/
void CGeoSysMATGroupsView::OnInitialUpdate()
{
  CGeoSysZoomView::OnInitialUpdate();
  CGeoSysDoc* pDoc = GetDocument();
  // Initialize the CGeoSysZoomView class
  SetZoomSizes(pDoc->GetDocSize());
}

BOOL CGeoSysMATGroupsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// ZU ERLEDIGEN: Ändern Sie hier die Fensterklasse oder das Erscheinungsbild, indem Sie
	//  CREATESTRUCT cs modifizieren.

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Zeichnen
//###########################################################################
// Draw
//###########################################################################

/**************************************************************************
GeoSys-GUI Function
Programing:
01/2004 OK Implementation
05/2004 CC Modification
09/2005 MB OK heterogeneous files
**************************************************************************/
void CGeoSysMATGroupsView::OnDraw(CDC* pDC)
{
  GetWindowAttributes(this->m_hWnd,&width,&height);
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //----------------------------------------------------------------------
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(!theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    AfxMessageBox("Open ControlPanel for MAT View applications");
    return;
  }
  //----------------------------------------------------------------------
  if(mainframe->m_strLAYName.IsEmpty())
    pDC->TextOut(0,0," Material groups");
  else
    pDC->TextOut(0,0," Material groups: "+mainframe->m_strLAYName);
  //----------------------------------------------------------------------
  theApp->g_graphics_modeless_dlg->width = width;
  theApp->g_graphics_modeless_dlg->height = height;
  m_dXmin = theApp->g_graphics_modeless_dlg->m_dXmin;
  m_dXmax = theApp->g_graphics_modeless_dlg->m_dXmax;
  m_dYmin = theApp->g_graphics_modeless_dlg->m_dYmin;
  m_dYmax = theApp->g_graphics_modeless_dlg->m_dYmax;
  //----------------------------------------------------------------------
  CGraphics m_graphics;
  m_graphics.width = width;
  m_graphics.height = height;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  //----------------------------------------------------------------------
  // MMP properties
  if(theApp->g_graphics_modeless_dlg->m_bDisplayMaterialGroups){
    //....................................................................
    pDC->TextOut(0,0," MATView: MSH");
    theApp->g_graphics_modeless_dlg->width = width;
    theApp->g_graphics_modeless_dlg->height = height;
    CFEMesh* m_msh = FEMGet((string)theApp->g_graphics_modeless_dlg->m_strMSHName);
    if(m_msh){
      theApp->g_graphics_modeless_dlg->DrawMSHElements(pDC,m_msh);
      if(theApp->g_graphics_modeless_dlg->m_bDisplayNOD)
        theApp->g_graphics_modeless_dlg->DrawMSHNodes(pDC,m_msh);
      if(theApp->g_graphics_modeless_dlg->m_bDisplayMSHLabels)
        theApp->g_graphics_modeless_dlg->DrawMSHElementsLabels(pDC,m_msh);
    }
    //....................................................................
    if(m_msh){
      double px[9],py[9];
      CElem* m_ele = NULL;
      CNode* m_nod = NULL;
      double mmp_value = 0;
      double m_dUmin = 10e10; //MB min/max
      double m_dUmax = 10e-10;
      int m_levels = 25;
      int j;
      double u_int;
      double rstep,u_rel;
      int m = -1;
      if(m_dlg_mat_editor->m_strMMPPropertyName.Compare("PERMEABILITY")==0) {
        m = 0;
        pDC->TextOut(150,0,"PERMEABILITY");
      }
      if(m_dlg_mat_editor->m_strMMPPropertyName.Compare("POROSITY")==0) {
        m = 1;
        pDC->TextOut(150,0,"POROSITY");
      }
      CString str;
      str.Format("%d",m_dlg_mat_editor->m_iSelectedMMPGroup+1);
      pDC->TextOut(300,0,"LAYER");
      pDC->TextOut(350,0,str);
      //....................................................................
      //Calculate Min/Max value
      for(long i=0;i<(long)m_msh->ele_vector.size();i++){
        m_ele = m_msh->ele_vector[i];
        if(m_ele->GetPatchIndex()!=m_dlg_mat_editor->m_iSelectedMMPGroup) //OK/MB
          continue;
        mmp_value = m_ele->mat_vector(m);
        // Max Min values //MB
        m_dUmax = max(mmp_value,m_dUmax);
        m_dUmin = min(mmp_value,m_dUmax); 
      }
      //....................................................................
      for(long i=0;i<(long)m_msh->ele_vector.size();i++){
        m_ele = m_msh->ele_vector[i];
        if(m_ele->GetPatchIndex()!=m_dlg_mat_editor->m_iSelectedMMPGroup) //OK/MB
          continue;
        if(m_ele->mat_vector.Size()>1)
          mmp_value = m_ele->mat_vector(m); //m
        // Element corner nodes
        for(j=0;j<m_ele->GetVertexNumber();j++){
          m_nod = m_ele->GetNode(j);
          px[j] = m_nod->X();
          py[j] = m_nod->Y();
        }
        // Color calculation
        if(m_dUmax-m_dUmin>MKleinsteZahl)
          u_rel = (mmp_value-m_dUmin)/(m_dUmax-m_dUmin); // 0-1
        else
          u_rel = MKleinsteZahl;
        modf(u_rel*m_levels,&u_int);
        rstep = u_int * 255.0/m_levels;
        theApp->g_graphics_modeless_dlg->ChooseContourColours(rstep);
        theApp->g_graphics_modeless_dlg->MFC_FillElement1(3,px,py,pDC,\
        theApp->g_graphics_modeless_dlg->ContourColourRed,theApp->g_graphics_modeless_dlg->ContourColourGreen,theApp->g_graphics_modeless_dlg->ContourColourBlue);
      }
    }
  }
  //----------------------------------------------------------------------
  vector<Surface*>::const_iterator p_sfc; 
  Surface* m_sfc = NULL;
  CGLVolume* m_vol = NULL;
  vector<CGLVolume*>::const_iterator p_vol;
  p_vol = volume_vector.begin();
  while (p_vol!= volume_vector.end()){
    m_vol = *p_vol;
    p_sfc = m_vol->surface_vector.begin();
    while(p_sfc!=m_vol->surface_vector.end()) {
      m_sfc = *p_sfc;
      if(m_sfc->highlighted){
        m_graphics.DrawSurface(pDC,m_sfc,0);
      }
      ++p_sfc;
    }
	++p_vol;
  }
}


/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Drucken

BOOL CGeoSysMATGroupsView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Standardvorbereitung
	return DoPreparePrinting(pInfo);
}

void CGeoSysMATGroupsView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Zusätzliche Initialisierung vor dem Drucken hier einfügen
}

void CGeoSysMATGroupsView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// ZU ERLEDIGEN: Hier Bereinigungsarbeiten nach dem Drucken einfügen
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Diagnose

#ifdef _DEBUG
void CGeoSysMATGroupsView::AssertValid() const
{
	CView::AssertValid();
}

void CGeoSysMATGroupsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeoSysDoc* CGeoSysMATGroupsView::GetDocument() // Die endgültige (nicht zur Fehlersuche kompilierte) Version ist Inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Nachrichten-Handler
//###########################################################################
// Menus
//###########################################################################

void CGeoSysMATGroupsView::OnProperties()
{
  CGraphics m_graphics;
  m_graphics.m_dXmin = m_dXmin;
  m_graphics.m_dXmax = m_dXmax;
  m_graphics.m_dYmin = m_dYmin;
  m_graphics.m_dYmax = m_dYmax;
  m_graphics.m_iDisplayPLY = m_iDisplayPLY;
  m_graphics.m_iDisplaySUF = m_iDisplaySFC;
  m_graphics.m_bDisplayMaterialGroups = m_bDisplayMaterialGroups;
  m_graphics.m_bShowLabels = m_bShowLabels;

  m_graphics.DoModal();

  m_dXmin = m_graphics.m_dXmin;
  m_dXmax = m_graphics.m_dXmax;
  m_dYmin = m_graphics.m_dYmin;
  m_dYmax = m_graphics.m_dYmax;
  m_iDisplayPLY = m_graphics.m_iDisplayPLY;
  m_iDisplaySFC = m_graphics.m_iDisplaySUF;
  m_bDisplayMaterialGroups = m_graphics.m_bDisplayMaterialGroups;
  m_bShowLabels = m_graphics.m_bShowLabels;

  Invalidate(FALSE);
}

/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGeoSysMATGroupsView::OnMATGroupEditor()
{
  CMATGroupEditor m_dialog;
  CGeoSysDoc* m_pDoc = GetDocument();
  m_dialog.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_dialog.DoModal();
  Invalidate(FALSE);
}

/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
01/2004 WW Implementation
**************************************************************************/
void CGeoSysMATGroupsView::OnMatSolidProperties()
{
  MAT_Mech_dlg MMech_dlg;
  MMech_dlg.DoModal();
}

/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
01/2004 OK/JG Implementation
**************************************************************************/
void CGeoSysMATGroupsView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  nFlags=nFlags;
  POINT m_arrPoint[1024];
  Surface *m_surface = NULL;
  int polygon_point_vector_length;
  int i;
  mat_group_name_selected.clear();
  // 1 identity MAT group
  //CMediumProperties *m_mat_mp = NULL;
  int m;
  for(m=0;m<(int)mmp_vector.size();m++){
    m_mat_mp = mmp_vector[m];
    m_surface = GEOGetSFCByName(m_mat_mp->geo_name);//CC
    if(!m_surface) {
      AfxMessageBox("Surface not found!");
      return;
    }
    polygon_point_vector_length = (int)m_surface->polygon_point_vector.size();
    for(i=0;i<polygon_point_vector_length;i++) {
      m_arrPoint[i].x = xpixel(m_surface->polygon_point_vector[i]->x);
      m_arrPoint[i].y = ypixel(m_surface->polygon_point_vector[i]->y);
    }
    CRgn surface_polygon;
    surface_polygon.CreatePolygonRgn(&m_arrPoint[0],polygon_point_vector_length,WINDING);
    if (surface_polygon.PtInRegion(point)) {
      mat_group_selected = m_mat_mp->number;
      mat_group_name_selected = m_mat_mp->name;
      break;
    }
    DeleteObject(surface_polygon);
  }
  if(mat_group_name_selected.size()) {
    // 2 Dialog with table
    m_mat_mp = MMPGet(mat_group_name_selected);
    mat_prop_pad mat_p_dlg;
    mat_p_dlg.SetMaterial(m_mat_mp);
    mat_p_dlg.DoModal();
  }
  // CView::OnLButtonDblClk(nFlags, point);
}

/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
02/2004 OK Implementation
**************************************************************************/
int CGeoSysMATGroupsView::xpixel(double x)
{
  double skalex = ((double)width)/(m_dXmax-m_dXmin);
  int xminp = (int) ( - m_dXmin * skalex);

  return (int)(xminp + x * skalex);
}      	  	                                     

/**************************************************************************
GeoSys-GUI: CGeoSysMATGroupsView()
Programing:
02/2004 OK Implementation
**************************************************************************/
int CGeoSysMATGroupsView::ypixel(double y)
{
  double skaley = ((double)height)/(m_dYmax-m_dYmin);
  int yminp = (int) ( height + m_dYmin * skaley);
  return (int)( yminp - y * skaley);
}      	  	                                     


/**************************************************************************
GeoSys-GUI: mat_prop_pad
Programing:
02/2004 WW/JD Implementation
**************************************************************************/

IMPLEMENT_DYNAMIC(mat_prop_pad, CDialog)
mat_prop_pad::mat_prop_pad(CWnd* pParent /*=NULL*/)
	: CDialog(mat_prop_pad::IDD, pParent)
{
}

mat_prop_pad::~mat_prop_pad()
{
}

void mat_prop_pad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAT_PAD_GRID, m_grid);
	DDX_Control(pDX, IDC_MAT_COMBO_PAD, Mat_Comb);
}
BEGIN_MESSAGE_MAP(mat_prop_pad, CDialog)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_MAT_PAD_GRID, OnGridGetDispInfo)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_MAT_PAD_GRID, OnGridSetDispInfo)
	ON_CBN_SELCHANGE(IDC_MAT_COMBO_PAD, OnCbnSelchangeMatComboPad)
END_MESSAGE_MAP()

BOOL mat_prop_pad::OnInitDialog()
{
	CDialog::OnInitDialog();

	Mat_Comb.SetCurSel(0);
	MaterialType = 0;
	//medium_mat = NULL;
	
	/////// Grid
	//  ---------------  Required variables  -----------------
	CGridColumn *pColumn;
	CGridHeaderSections *pSections;
	//CGridHeaderSection *pSection;
	CGridHeaderSection *pUpperSection;

	m_grid.SetRowHeight(25);

	//  -----------------  Let's add some columns  --------------  
	m_grid.AddColumn(_T(""), 128, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 128, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 64, LVCFMT_LEFT);
	

	//  ---------------  Set additional column properties  ----------------  

	pColumn = m_grid.GetColumn(0);
	pColumn->SetReadOnly(TRUE);
	//pColumn->SetWordWrap(TRUE);

	pColumn = m_grid.GetColumn(2);
	pColumn->SetReadOnly(TRUE);

	//  ---------------  Let's put the grid header into shape  ------------
	pSections = m_grid.GetHeader()->GetSections();
	pUpperSection = pSections->GetSection(0);
	pUpperSection->SetCaption(_T("Parameter"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(1);
	pUpperSection->SetCaption(_T("Value"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(2);
	pUpperSection->SetCaption(_T("Unit"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	m_grid.GetHeader()->SynchronizeSections();

	//Default: Medium
	ShowMediaProperties();

	return TRUE;

}


//  ---
void mat_prop_pad::OnGridGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    char buffer[128];
	pResult = pResult;

	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;

	if (pDispInfo->item.mask & LVIF_TEXT)  {
		switch (pDispInfo->item.nColumn)  {
		case 0:		//  Parameter
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strParaName;
            break;
		case 1:		//  Value
            _gcvt(m_Data[pDispInfo->item.nRow].ParaVal, 12, buffer );
			pDispInfo->item.strText = buffer; 
			break;
		case 2:		//  Unit
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strUnitName;
			break;
		default:
			break;
		}
	}

}
//  ---
void mat_prop_pad::OnGridSetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;
	double Val=0.0;
    int TheRow=0; 


	switch (pDispInfo->item.nColumn)  {
	case 0:		//  
        break;
	case 1:		//  Value
		Val = strtod(pDispInfo->item.strText, NULL);
		m_Data[pDispInfo->item.nRow].ParaVal = Val;
		TheRow=pDispInfo->item.nRow;
	    switch(MaterialType)
	    {
	       case 0:
              if(TheRow==0)
				 medium_mat->conductivity = Val;    
              else if(TheRow==1)
				  medium_mat->permeability = Val;    
              else if(TheRow==2)
				  medium_mat->porosity = Val;    
			  break;
	       case 1:
             break;
	       default:
             break;
    	}

		break;
	default:
		break;
	}
	
 
	*pResult = 0;
}




// mat_prop_pad message handlers

void mat_prop_pad::OnCbnSelchangeMatComboPad()
{
    ///Combobox Plasticity
	MaterialType =Mat_Comb.GetCurSel();
	switch(MaterialType)
	{
	   case 0:
         ShowMediaProperties();
         break;
	   case 1:
         break;
	   default:
         break;
	}
	m_grid.RedrawWindow();

}

void mat_prop_pad::ShowMediaProperties()
{
  	m_grid.SetAllowEdit();
    m_grid.SetRowCount(3);
    m_Data.RemoveAll();

	m_Data.Add(Mat_Grid(_T("Conductivity "), medium_mat->conductivity, _T("m/s")));
	m_Data.Add(Mat_Grid(_T("Permeability"), medium_mat->permeability, _T("--")));
	m_Data.Add(Mat_Grid(_T("Porosity"), 	medium_mat->porosity, _T("--")));
}


/**************************************************************************
GeoSys-GUI: OnViewZoomin
Programing:User pressed the ZOOM IN toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMATGroupsView::OnViewZoomin()
{
	// Toggle zoomin mode
	CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == MODE_ZOOMIN) {
		SetZoomMode(MODE_ZOOMOFF);
		// Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(MODE_ZOOMIN);
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click to zoom in on point or drag a zoom box.");
	}
} // OnViewZoomin


/**************************************************************************
GeoSys-GUI: OnViewZoomout
Programing:User pressed the ZOOM OUT toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMATGroupsView::OnViewZoomout()
{
	// Toggle zoomout mode
	CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
	if (GetZoomMode() == MODE_ZOOMOUT) {
		SetZoomMode(MODE_ZOOMOFF);
		// Clear the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"");
	} else {
		SetZoomMode(MODE_ZOOMOUT);
		// Give instructions in the statusbar
		pWin->SendMessage(WM_SETMESSAGESTRING, 0, 
			(LPARAM)(LPCSTR)"Click to zoom out on point.");
	}
} // OnViewZoomout


/**************************************************************************
GeoSys-GUI: OnViewZoomfull
Programing:User pressed the ZOOM FULL toolbutton or menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMATGroupsView::OnViewZoomfull()
{
	DoZoomFull();  // Call CGeoSysZoomView member to zoom full scale
} // OnViewZoomfull


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomin
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMATGroupsView::OnUpdateViewZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
} // OnUpdateViewZoomin


/**************************************************************************
GeoSys-GUI: OnUpdateViewZoomout
Programing:Tell MFC whether to depress the button or check the menu
05/2004 CC Implementation
last modified: 
**************************************************************************/
void CGeoSysMATGroupsView::OnUpdateViewZoomout(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

