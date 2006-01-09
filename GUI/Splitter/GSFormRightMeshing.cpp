// GSFormRightPassive.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "Msctf.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSFormRightMeshing.h"
#include "GeoSysTreeView.h"
#include "COGLPickingView.h"
#include "GSForm3DLeft.h"
#include "gs_structured_mesh.h"
#include "gs_ogl_tetra_view.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_elements_rfi.h"
#include "msh_quality.h"
#include "dtmesh.h"
#include "tricommon.h"
#include "mathlib.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"
#include ".\gsformrightmeshing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CGSFormRightMeshing

IMPLEMENT_DYNCREATE(CGSFormRightMeshing, CFormView)

CGSFormRightMeshing::CGSFormRightMeshing()
	: CFormView(CGSFormRightMeshing::IDD)
{
    m_patch_index = 0;
    m_mesh_density = 0.0;
}

CGSFormRightMeshing::~CGSFormRightMeshing()
{
}

void CGSFormRightMeshing::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGSFormRightMeshing)
	DDX_Control(pDX, IDC_LIST_MESH, m_List);
    DDX_Text(pDX, IDC_PATCH_INDEX, m_patch_index);
    DDX_Text(pDX, IDC_MESHDENSITY_INDEX, m_mesh_density);
    //}}AFX_DATA_MAP
}

void CGSFormRightMeshing::OnDraw(CDC* pDC)
{
    pDC=pDC;//TK

    //Aktivate Buttons
    if(fem_msh_vector.size()==1 && fem_msh_vector[0]->ele_vector.size()>0){      
    int element_type = fem_msh_vector[0]->ele_vector[0]->GetElementType();
    if (element_type == 4)GetDlgItem(IDC_TRI2PRI_BUTTON)->EnableWindow(TRUE);
    else GetDlgItem(IDC_TRI2PRI_BUTTON)->EnableWindow(FALSE);
    if (element_type == 2)GetDlgItem(IDC_QUAD2HEX_BUTTON)->EnableWindow(TRUE);
    else GetDlgItem(IDC_QUAD2HEX_BUTTON)->EnableWindow(FALSE);
    if (element_type == 3 || element_type == 6 )GetDlgItem(IDC_HEX2TET_BUTTON)->EnableWindow(TRUE);
    else GetDlgItem(IDC_HEX2TET_BUTTON)->EnableWindow(FALSE);
    if (element_type == 2)GetDlgItem(IDC_QUAD2TRI_BUTTON)->EnableWindow(TRUE);
    else GetDlgItem(IDC_QUAD2TRI_BUTTON)->EnableWindow(FALSE);

    }
    else
    {
    GetDlgItem(IDC_TRI2PRI_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_QUAD2HEX_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_HEX2TET_BUTTON)->EnableWindow(FALSE);
    }

  
}

BEGIN_MESSAGE_MAP(CGSFormRightMeshing, CFormView)
	//{{AFX_MSG_MAP(CGSFormRightMeshing)

	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_TRI2PRI_BUTTON, OnBnClickedTri2priButton)
    ON_BN_CLICKED(IDC_QUAD2HEX_BUTTON, OnBnClickedQuad2hexButton)
    ON_BN_CLICKED(IDC_HEX2TET_BUTTON, OnBnClickedHex2tetButton)
    ON_BN_CLICKED(IDC_QUAD2TRI_BUTTON, OnBnClickedQuad2triButton)
    
    ON_BN_CLICKED(IDC_STRUC_2D_BUTTON, OnBnClickedStruc2dButton)
    ON_BN_CLICKED(IDC_TRI_DELAUNAY_BUTTON, OnBnClickedTriDelaunayButton)
    ON_BN_CLICKED(IDC_TRI_AREAI_BUTTON, OnBnClickedTriAreaiButton)
    ON_BN_CLICKED(IDC_TRI_ANGLE_BUTTON, OnBnClickedTriAngleButton)
    ON_BN_CLICKED(IDC_TRI_LENGTH_BUTTON, OnBnClickedTriLengthButton)
    ON_BN_CLICKED(IDC_TRI_QUALITY_GRAPH_BUTTON, OnBnClickedTriQualityGraphButton)
    ON_BN_CLICKED(IDC_AUTOMATIC_MESH_DENSITY, OnBnClickedAutomaticMeshDensity)
    ON_BN_CLICKED(IDC_GEOMETRIC_MESH_DENSITY2, OnBnClickedGeometricMeshDensity2)
    ON_BN_CLICKED(IDC_GLOBAL_SPLIT_MESH_DENSITY3, OnBnClickedGlobalSplitMeshDensity3)
    ON_BN_CLICKED(IDC_START_GMSH_MESHING, OnBnClickedStartGmshMeshing)
    ON_BN_CLICKED(IDC_GEO_EDITOR_BUTTON, OnBnClickedGeoEditorButton)
    ON_BN_CLICKED(IDC_MSH_EDITOR_BUTTON, OnBnClickedMshEditorButton)
    ON_BN_CLICKED(IDC_MSHNODES_IN_POLYGON, OnBnClickedMshnodesInPolygon)
    ON_BN_CLICKED(IDC_CLEAR_SELECTION, OnBnClickedClearSelection)
    ON_BN_CLICKED(IDC_MIN_MAX_EDGE_LENGTH, OnBnClickedMinMaxEdgeLength)
    ON_BN_CLICKED(IDC_SELECT_ALL, OnBnClickedSelectAll)
    ON_BN_CLICKED(IDC_SET_PATCH_INDEX, OnBnClickedSetPatchIndex)
    ON_BN_CLICKED(IDC_GMSH_MSH_IMPORT, OnBnClickedGmshMshImport)
    ON_BN_CLICKED(IDC_CREATE_GEO_BOUNDINGBOX, OnBnClickedCreateGeoBoundingbox)
    ON_BN_CLICKED(IDC_TET_VOLUME_BUTTON, OnBnClickedTetVolumeButton)
    ON_BN_CLICKED(IDC_TET_ANGLE_BUTTON, OnBnClickedTetAngleButton)
    ON_BN_CLICKED(IDC_SET_MESHDENSITY, OnBnClickedSetMeshdensity)
    ON_BN_CLICKED(IDC_DELETE_LAST_MESH, OnBnClickedDeleteDisplayedMesh)
    ON_BN_CLICKED(IDC_PATCHINDEX_FOR_PLAINS, OnBnClickedPatchindexForPlains)
    ON_BN_CLICKED(IDC_COMBINE_PATCHINDEX, OnBnClickedCombinePatchindex)
    ON_BN_CLICKED(IDC_COMPRESS_PATCHINDEX2, OnBnClickedCompressPatchindex2)
END_MESSAGE_MAP()


// CGSFormRightMeshing diagnostics

#ifdef _DEBUG
void CGSFormRightMeshing::AssertValid() const
{
	CFormView::AssertValid();
}

void CGSFormRightMeshing::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// CGSFormRightMeshing message handlers

void CGSFormRightMeshing::OnBnClickedTri2priButton()
{
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  //========================================================================
  CFEMesh*m_msh = fem_msh_vector[0];

    CMeshTypeChange meshtypechange;
    if (meshtypechange.DoModal() == IDOK)
    {
      UpdateData(TRUE);
      if(fem_msh_vector.size()==1)
      {	
        Create_Triangles2Prisms(meshtypechange.m_numberofprismlayers,meshtypechange.m_thicknessofprismlayer,m_msh);
        MSH_OverWriteMSH((string)m_strFileNameMSH);
      }


    }

  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  //GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".rfi");



}

void CGSFormRightMeshing::OnBnClickedQuad2hexButton()
{
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  //========================================================================
  CFEMesh*m_msh = fem_msh_vector[0];

    CMeshTypeChange meshtypechange;
    if (meshtypechange.DoModal() == IDOK)
    {
      UpdateData(TRUE);
      if(fem_msh_vector.size()==1)
      {	
        Create_Quads2Hex(meshtypechange.m_numberofprismlayers,meshtypechange.m_thicknessofprismlayer,m_msh);
        MSH_OverWriteMSH((string)m_strFileNameMSH);
      }


    }

  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);


}


void CGSFormRightMeshing::OnBnClickedHex2tetButton()
{
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameMSH;
  //========================================================================
  CFEMesh*m_msh = fem_msh_vector[0];
  	using dtm::Dtmesh;
	Dtmesh* dtmesh;
    dtmesh = new Dtmesh;
    dtmesh->inputMSH(m_msh);
    dtmesh->meshPointToTetra();
    dtmesh->outputMSHTetra((char*)file_name_const_char);
  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedQuad2triButton()
{
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  //========================================================================
  CFEMesh*m_msh = fem_msh_vector[0];

      UpdateData(TRUE);
      if(fem_msh_vector.size()==1)
      {	
          Create_Quads2Tri(m_msh);
          MSH_OverWriteMSH((string)m_strFileNameMSH);
      }
  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

}


void CGSFormRightMeshing::OnBnClickedStruc2dButton()
{
      //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  // Progress bar
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //CStatusBar* pStatus = &m_frame->m_wndStatusBar;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;

 
  CProgressCtrl m_ProgressBar;
  RECT MyRect;
  m_frame->m_wndStatusBar.GetItemRect(1,&MyRect);
  MyRect.left = 600;
    m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Start");  
  //========================================================================
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  m_pDoc->OnRemoveGEO();
  m_pDoc->OnRemoveMSH();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  //========================================================================

  const char *gli_file_name_const_char = 0;
  const char *msh_file_name_const_char = 0;
  gli_file_name_const_char = m_strFileNameGEO;
  msh_file_name_const_char = m_strFileNameMSH;

  
  CStructured_Mesh dlg;
  if (dlg.DoModal()==IDOK && dlg.m_delta_x != 0 && dlg.m_delta_y != 0)
  {
      double origin = 0.0;
      double x_min = dlg.m_edit_x_min;
      double y_min = dlg.m_edit_y_min;
      double x=0.0,y=0.0,z=0.0;
      double x_max = dlg.m_edit_x_max;
      double y_max = dlg.m_edit_y_max;
      long delta_x = dlg.m_delta_x;
      long delta_y = dlg.m_delta_y;
      double x_length = x_max - x_min;
      double y_length = y_max - y_min;
      double x_step = x_length/delta_x;
      double y_step = y_length/delta_y;
      long point1,point2,point3,point4;

      //write gli file
       FILE *geo_file=NULL;
       geo_file = fopen(gli_file_name_const_char, "w+t");

      	fprintf(geo_file,"%s\n","#POINTS");
        fprintf(geo_file,"%i %lg %lg %lg\n",0, x_min, y_min, origin);
        fprintf(geo_file,"%i %lg %lg %lg\n",1, x_max, y_min, origin);
        fprintf(geo_file,"%i %lg %lg %lg\n",2, x_max, y_max, origin);
        fprintf(geo_file,"%i %lg %lg %lg\n",3, x_min, y_max, origin);
	    fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  SOUTH");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n","  1");

      	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  EAST");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n","  2");
       	
	    fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  NORTH");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n","  3");
      	
	    fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  WEST");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n","  0");
       	
        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  DOMAIN");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  SOUTH");
        fprintf(geo_file,"%s\n","  EAST");
        fprintf(geo_file,"%s\n","  NORTH");
        fprintf(geo_file,"%s\n","  WEST");
        fprintf(geo_file,"%s\n","#STOP");
        fclose(geo_file);


  //write msh file
    int i=0, j=0, nEle=0, nNod=0;

    if (dlg.m_do_structured_triangle_mesh == FALSE && dlg.m_do_structured_rectangle_mesh == FALSE) return;  

       FILE *msh_file=NULL;
       msh_file = fopen(msh_file_name_const_char, "w+t");

    //write MSH Head
	   	fprintf( msh_file, "%s\n", "#FEM_MSH");
    //write PCS Type
        fprintf( msh_file, "%s\n", " $PCS_TYPE");
	   	fprintf( msh_file, "%s\n", "  NO_PCS"); //TODO
    //Write Nodes
	   	fprintf( msh_file, "%s\n", " $NODES");
        nNod = (delta_x +1)* (delta_y +1);
        fprintf(msh_file,"% ld\n",nNod);
    //write Geometry    
    m_ProgressBar.SetRange((short)0,(short)nNod+(short)nEle);
    m_ProgressBar.SetStep(1); 
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Creating Nodes");  
        int nnb=0;
   	for(j=0 ;j<=delta_y;j++) 
    {
        y = y_min + (j*y_step);
        for(i=0 ;i<=delta_x;i++) 
        {
        x = x_min + (i*x_step);
        fprintf( msh_file, "%d ", nnb);
        fprintf( msh_file, "%20.14f %20.14f %20.14f\n", x, y, z);
        nnb++;
        m_ProgressBar.StepIt();
	    }
    }
    //Write Elements
	   	fprintf( msh_file, "%s\n", " $ELEMENTS");
        if (dlg.m_do_structured_rectangle_mesh == TRUE) nEle = delta_x * delta_y;
        if (dlg.m_do_structured_triangle_mesh == TRUE)  nEle = delta_x * delta_y * 2;
		fprintf(msh_file,"% ld\n",nEle);
    //write Topology
    int nel=0;
    int y_jump = 0;
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Creating Elements");  
    if (dlg.m_do_structured_rectangle_mesh == TRUE)
    {
   	    for(j=0;j<delta_y;j++) 
        {
            y_jump = (delta_x+1)*j;

        for(i=0;i<delta_x;i++) 
        {
            point1 = y_jump+i;
            point2 = y_jump+i+1;
            point3 = y_jump+i+1+delta_x+1;
            point4 = y_jump+i+delta_x+1;
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 quad ");     
		    fprintf( msh_file, "%d  %d  %d  %d\n",point1, point2, point3, point4 );
            nel++;
            m_ProgressBar.StepIt();
	    }
        }
    }

    if (dlg.m_do_structured_triangle_mesh == TRUE)
    {
   	    for(j=0;j<delta_y;j++) 
        {
            y_jump = (delta_x+1)*j;

        for(i=0;i<delta_x;i++) 
        {
            point1 = y_jump+i;
            point2 = y_jump+i+1;
            point3 = y_jump+i+1+delta_x+1;
            point4 = y_jump+i+delta_x+1;
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point1, point2, point3);
            nel++;
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point1, point3, point4);
            nel++;
            m_ProgressBar.StepIt();
	    }
        }

    }

    //Write STOP
	   	fprintf( msh_file, "%s\n", "#STOP");

  fclose(msh_file);



  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Finish....Load Data");  

  GEOLIB_Read_GeoLib((string)m_strFileNameBase);
  FEMRead((string)m_strFileNameBase);

  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".gli");
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");
  }

}

void CGSFormRightMeshing::OnBnClickedTriDelaunayButton()
{
   //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  // Progress bar
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //CStatusBar* pStatus = &m_frame->m_wndStatusBar;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;

  CProgressCtrl m_ProgressBar;
  RECT MyRect;
  m_frame->m_wndStatusBar.GetItemRect(1,&MyRect);
  MyRect.left = 600;
    m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
	m_ProgressBar.SetRange(0,1);
	m_ProgressBar.SetStep(1); 
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Start");  //========================================================================
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  //========================================================================

  const char *gli_file_name_const_char = 0;
  const char *msh_file_name_const_char = 0;
  gli_file_name_const_char = m_strFileNameGEO;
  msh_file_name_const_char = m_strFileNameMSH;

  bLoadStart_TRI( (char*)gli_file_name_const_char, (char*)msh_file_name_const_char, 1 );

  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Finish....Load Data");  

  FEMRead((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");


}

void CGSFormRightMeshing::OnBnClickedTriAreaiButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CGeoSysDoc *m_pDoc = GetDocument();
    int i=0;
 
    if(m_frame->m_3dcontrol_msh_quality==1)
    {
        m_frame->m_something_changed = 1;
        m_frame->m_3dcontrol_msh_quality=0;
    }
    else 
    {    
    for (i=0;i<(int)fem_msh_vector.size();i++)
    {
        if (fem_msh_vector[i]->ele_display_mode == 1 ||
            fem_msh_vector[i]->nod_display_mode == 1)
        {
  	        Get_MSH_for_Quality (i);
	        CalculateBasicTriangleData();
	        CalculateTriangleAreaQuality();
            break;
        }
    }
    m_frame->m_3dcontrol_msh_quality=1;
    m_frame->m_something_changed = 1;
    }
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

}

void CGSFormRightMeshing::OnBnClickedTriAngleButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CGeoSysDoc *m_pDoc = GetDocument();
    int i=0;
    if(m_frame->m_3dcontrol_msh_quality==1)
    {
        Get_MSH_for_Quality (0);
        m_frame->m_something_changed = 1;
        m_frame->m_3dcontrol_msh_quality=0;
    }
    else 
    {    
    for (i=0;i<(int)fem_msh_vector.size();i++)
    {
        if (fem_msh_vector[i]->ele_display_mode == 1 ||
            fem_msh_vector[i]->nod_display_mode == 1)
        {
  	        Get_MSH_for_Quality (i);
	        CalculateBasicTriangleData();
	        CalculateTriangleAngleQuality();
            break;
        }
    }
    m_frame->m_3dcontrol_msh_quality=1;
    m_frame->m_something_changed = 1;
    }
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedTriLengthButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CGeoSysDoc *m_pDoc = GetDocument();
    int i=0;
    if(m_frame->m_3dcontrol_msh_quality==1)
    {
        Get_MSH_for_Quality (0);
        m_frame->m_something_changed = 1;
        m_frame->m_3dcontrol_msh_quality=0;
    }
    else 
    {    
    for (i=0;i<(int)fem_msh_vector.size();i++)
    {
        if (fem_msh_vector[i]->ele_display_mode == 1 ||
            fem_msh_vector[i]->nod_display_mode == 1)
        {
            Get_MSH_for_Quality (i);
	        CalculateBasicTriangleData();
	        CalculateTriangleLengthQuality();
            break;
        }
    }
    m_frame->m_3dcontrol_msh_quality=1;
    m_frame->m_something_changed = 1;
    }
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedTriQualityGraphButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    m_frame->OnViewOGLCreate();
}

void CGSFormRightMeshing::OnBnClickedAutomaticMeshDensity()
{
    GEO_Get_Min_Max_Distance_of_polyline_neighbor_points();
    GEO_Copy_Min_Distance_Of_Neighbor_Points_To_Mesh_Density();
}

void CGSFormRightMeshing::OnBnClickedGeometricMeshDensity2()
{
    GEO_Get_Min_Distance_of_neighbor_points();
    GEO_Copy_Min_Distance_Of_Neighbor_Points_To_Mesh_Density();
}

void CGSFormRightMeshing::OnBnClickedGlobalSplitMeshDensity3()
{
    GEO_Mesh_Density_BiSect();
}

void CGSFormRightMeshing::OnBnClickedStartGmshMeshing()
{
  //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  // Progress bar
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //CStatusBar* pStatus = &m_frame->m_wndStatusBar;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;

  CProgressCtrl m_ProgressBar;
  RECT MyRect;
  m_frame->m_wndStatusBar.GetItemRect(1,&MyRect);
  MyRect.left = 600;
    m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
	m_ProgressBar.SetRange(0,6);
	m_ProgressBar.SetStep(1); 
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Start");
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGEO = m_strFileNameBase + "_temp.geo";
  CString m_strFileNameGMSH = m_strFileNameBase + "_temp.msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Generation of GMSH input data");
  //GEO_Write_GMSH_Input_File(file_name_const_char);/*GLI2GMSH*/ 
  
  FILE *geo_file=NULL;
  geo_file = fopen(file_name_const_char, "w+t");
  int ready2mesh = 0;
  int i=0;

  for (i=0;i<(int)surface_vector.size();i++)
  {
      if (surface_vector[i]->display_mode_3d == 1)
      {
          surface_vector[i]->meshing_allowed = 1;
          ready2mesh = 1;
      }
      else
      {
          surface_vector[i]->meshing_allowed = 0;
          surface_vector[i]->display_mode_3d = 0;
      }
  }

  if (ready2mesh == 1)
  {
  GEOPolylineGLI2GEO(geo_file);
  fclose(geo_file);
  
  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Meshing");
  CString m_strExecute = "..\\LIB\\gmsh " + m_strFileNameGEO +" -2";
  system(m_strExecute); /*Meshing*/ 
  m_ProgressBar.StepIt();
  FILE *msh_file=NULL;
  file_name_const_char = m_strFileNameMSH;
  msh_file = fopen(file_name_const_char, "w+t");
  file_name_const_char = m_strFileNameGMSH;
  char file_name__char [1024];
  strcpy(file_name__char,file_name_const_char);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: GMSH2MSH Converter");
  
    CFEMesh*m_msh = NULL;
    m_msh = new CFEMesh();
    long msh_no_tris;
    CString pcs_name;
    pcs_name.Format(_T("%d"),fem_msh_vector.size());
    _tprintf(_T("%s"), (LPCTSTR) pcs_name);
    pcs_name = "NO_PCS_" + pcs_name;
    m_msh->pcs_name = pcs_name;
    GMSH2MSH (file_name__char, m_msh);/*GMSH2MSH*/
    msh_no_tris = (long)m_msh->ele_vector.size(); //OK
    m_msh->ele_type = 4;
    fem_msh_vector.push_back(m_msh);
  
  m_ProgressBar.StepIt();
  file_name_const_char = m_strFileNameMSH;
  fclose(msh_file);
  //pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Serialize RFI");
  //AfxMessageBox("The mesh will be serialized now. That might need a few minutes");
  //Serialize_RFI (file_name_const_char);/*Serialisierung*/  
  m_ProgressBar.StepIt();
  //remove(m_strFileNameGEO);
  //remove(m_strFileNameGMSH);
  file_name_const_char = m_strFileNameBase;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Load Mesh");
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");
  //MSHOpen(file_name_const_char);/*OpenRFI*/ 
  m_ProgressBar.StepIt();
  //
 
 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Load View Data");
 //FEMRead((string)m_strFileNameBase);/*OpenRFI*/ 
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
 m_ProgressBar.StepIt();
 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
 }
}



void CGSFormRightMeshing::OnBnClickedGeoEditorButton()
{
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString exe_call = "notepad.exe " + m_strFileNameGEO;
  WinExec(exe_call, SW_SHOW);
}

void CGSFormRightMeshing::OnBnClickedMshEditorButton()
{
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString exe_call = "notepad.exe " + m_strFileNameMSH;
  WinExec(exe_call, SW_SHOW);
}


void CGSFormRightMeshing::OnBnClickedMshnodesInPolygon()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysDoc *m_pDoc = GetDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase + "_temp";
  CString m_strFileNameGLI = m_strFileNameBase + ".gli";
  CString m_strFileNameGEO = m_strFileNameBase + ".geo";
  CString m_strFileNameGMSH = m_strFileNameBase + ".msh";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameBase; 
  
  int i=0, j=0, k=0;
  for (i=0;i<(int)surface_vector.size();i++)
  {
      if (surface_vector[i]->display_mode_3d == 1)
      {
       /*Surface Meshing*/ 
       Mesh_Single_Surface(surface_vector[i]->name, file_name_const_char);
       break;
      }
  }
       /*Node and Element Selection*/ 
       Select_Nodes_Elements_by_GMSHFile(file_name_const_char);

 m_frame->dist_geo_object = 0.001;
 UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedClearSelection()
{
 Clear_Selected_Nodes_Elements();
 
 CGeoSysDoc *m_pDoc = GetDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedMinMaxEdgeLength()
{
 int i=0, j=0;
 double edge_length;
 double min_edge_length=0.0;
 double max_edge_length=0.0;
 char string2add [56];
 string info_string;
    
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
        for(i=0;i<(long)fem_msh_vector[j]->edge_vector.size();i++)
        {
            edge_length = fem_msh_vector[j]->edge_vector[i]->Length();

            if (j==0 && i==0)
            {
              min_edge_length = edge_length;
              max_edge_length = edge_length;

              m_List.ResetContent();
              m_List.AddString(_T("***********************************"));
              m_List.AddString(_T(">MSH data loaded"));
              m_List.AddString(_T("***********************************"));
            }
            else
            {
              if (min_edge_length > edge_length)min_edge_length = edge_length;
              if (max_edge_length < edge_length)max_edge_length = edge_length;
            }
        }     
        
        fem_msh_vector[j]->min_edge_length = min_edge_length;
        fem_msh_vector[j]->max_edge_length = max_edge_length;

        info_string = ">MESH:" ;
        info_string.append(fem_msh_vector[j]->pcs_name.data());
        m_List.AddString(_T(info_string.data()));
        m_List.AddString(_T(">EDGES:"));
        sprintf(string2add, "%lg",fem_msh_vector[j]->min_edge_length);
        info_string = ">Min:" ;
        info_string.append(string2add);
        m_List.AddString(_T(info_string.data()));
        sprintf(string2add, "%lg",fem_msh_vector[j]->max_edge_length);
        info_string = ">Max:" ;
        info_string.append(string2add);
        m_List.AddString(_T(info_string.data()));
    }

        m_List.AddString(_T("***********************************"));
        


 CGeoSysDoc *m_pDoc = GetDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);

}

void CGSFormRightMeshing::OnBnClickedSelectAll()
{
 int i=0, j=0;
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
          fem_msh_vector[j]->nod_vector[i]->selected= 1;
        }     

        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
          fem_msh_vector[j]->ele_vector[i]->selected= 1;
        }     

    }
 CGeoSysDoc *m_pDoc = GetDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedSetPatchIndex()
{
 int i=0, j=0;

 UpdateData(TRUE);
 
 

    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
    
        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            if (fem_msh_vector[j]->ele_vector[i]->selected == 1)
            fem_msh_vector[j]->ele_vector[i]->SetPatchIndex(m_patch_index);
        }     

    }

 UpdateData(FALSE);
 CGeoSysDoc *m_pDoc = GetDocument();
 CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedGmshMshImport()
{
  //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  // Progress bar
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //CStatusBar* pStatus = &m_frame->m_wndStatusBar;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"GMSH IMPORT:");
  // File dialog
  CFileDialog fileDlg(TRUE, "gmsh", NULL, OFN_ENABLESIZING," GMSH Files (*.gmsh)|*.gmsh|| ");
  if(fileDlg.DoModal()==IDOK){
    if(fileDlg.GetFileExt()=="gmsh")
    {
    CProgressCtrl m_ProgressBar;
    RECT MyRect;
    m_frame->m_wndStatusBar.GetItemRect(1,&MyRect);
    MyRect.left = 600;
        m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
	    m_ProgressBar.SetRange(0,6);
	    m_ProgressBar.SetStep(1); 
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"GMSH IMPORT: Start");
    //========================================================================
    CGeoSysDoc *m_pDoc = GetDocument();
    CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
    CString m_strFileNameMSH = m_strFileNameBase + ".msh";
    CString m_strFileNameGEO = m_strFileNameBase + ".geo";    
    CString m_strFileNameGSP = m_strFileNameBase + ".gsp";

    CString m_strFileNameGMSH = fileDlg.GetPathName();

    const char *file_name_const_char = 0;
      
    m_ProgressBar.StepIt();
    m_ProgressBar.StepIt();
    //FILE *msh_file=NULL;
    file_name_const_char = m_strFileNameMSH;
    //msh_file = fopen(file_name_const_char, "w+t");
    file_name_const_char = m_strFileNameGMSH;
    char file_name__char [1024];
    strcpy(file_name__char,file_name_const_char);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"GMSH IMPORT: MSH2RFI Converter");

    CFEMesh*m_msh = NULL;
    m_msh = new CFEMesh();
    long msh_no_tris;
    m_msh->pcs_name = "NO_PCS";
    GMSH2MSH (file_name__char, m_msh);/*GMSH2MSH*/
    msh_no_tris = (long)m_msh->ele_vector.size(); //OK
    m_msh->ele_type = 4;
    fem_msh_vector.push_back(m_msh);



    m_ProgressBar.StepIt();
    file_name_const_char = m_strFileNameMSH;
    //fclose(msh_file);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"GMSH IMPORT: Serialize RFI");
    //AfxMessageBox("The mesh will be serialized now. That might need a few minutes");
    //Serialize_RFI (file_name_const_char);/*Serialisierung*/  
    m_ProgressBar.StepIt();
    //remove(m_strFileNameGEO);
    //remove(m_strFileNameGMSH);
    file_name_const_char = m_strFileNameBase;
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"GMSH IMPORT: Load Mesh");
    //FEMRead(file_name_const_char);/*OpenMSH*/ 
        // FEMRead((string)m_strFileNameBase);
    GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");

    m_ProgressBar.StepIt();
    //
     
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Load View Data");
    //MSHOpen((string)m_strFileNameBase);/*OpenRFI*/ 
    m_frame->m_something_changed = 1;
    m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
    m_pDoc->UpdateAllViews(NULL);
    Invalidate(TRUE);
    m_ProgressBar.StepIt();
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
    }
  }
}
void CGSFormRightMeshing::OnBnClickedCreateGeoBoundingbox()
{
    //========================================================================
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 
  //========================================================================
  //File handling
  //========================================================================
  CGeoSysDoc *m_pDoc = GetDocument();
  m_pDoc->OnRemoveGEO();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  //========================================================================

  const char *gli_file_name_const_char = 0;
  const char *msh_file_name_const_char = 0;
  gli_file_name_const_char = m_strFileNameGEO;
  msh_file_name_const_char = m_strFileNameMSH;

  int j=0,i=0;
  double x_min = 0.0;
  double y_min = 0.0;
  double z_min = 0.0;
  double x_max = 0.0;
  double y_max = 0.0;
  double z_max = 0.0;
  double x_count1,y_count1,z_count1;

  for(j=0;j<(long)fem_msh_vector.size();j++)
  {
     for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
     {
         if (i==0) {
            x_min = fem_msh_vector[j]->nod_vector[i]->X();
            x_max = fem_msh_vector[j]->nod_vector[i]->X();
            y_min = fem_msh_vector[j]->nod_vector[i]->Y();
            y_max = fem_msh_vector[j]->nod_vector[i]->Y();
            z_min = fem_msh_vector[j]->nod_vector[i]->Z();
            z_max = fem_msh_vector[j]->nod_vector[i]->Z();
         }
         else {
         x_count1 = fem_msh_vector[j]->nod_vector[i]->X();
         y_count1 = fem_msh_vector[j]->nod_vector[i]->Y();
         z_count1 = fem_msh_vector[j]->nod_vector[i]->Z();

          if (x_count1 < x_min) x_min = x_count1;
          if (x_count1 > x_max) x_max = x_count1;
		  if (y_count1 < y_min) y_min = y_count1;
          if (y_count1 > y_max) y_max = y_count1;
		  if (z_count1 < z_min) z_min = z_count1;
          if (z_count1 > z_max) z_max = z_count1;
         }
     }     
	}


      //write gli file
       FILE *geo_file=NULL;
       geo_file = fopen(gli_file_name_const_char, "w+t");

      	fprintf(geo_file,"%s\n","#POINTS");
        fprintf(geo_file,"%i %lg %lg %lg\n",0, x_min, y_min, z_min);
        fprintf(geo_file,"%i %lg %lg %lg\n",1, x_min, y_min, z_max);
        fprintf(geo_file,"%i %lg %lg %lg\n",2, x_min, y_max, z_min);
        fprintf(geo_file,"%i %lg %lg %lg\n",3, x_min, y_max, z_max);
        fprintf(geo_file,"%i %lg %lg %lg\n",4, x_max, y_min, z_min);
        fprintf(geo_file,"%i %lg %lg %lg\n",5, x_max, y_min, z_max);
        fprintf(geo_file,"%i %lg %lg %lg\n",6, x_max, y_max, z_min);
        fprintf(geo_file,"%i %lg %lg %lg\n",7, x_max, y_max, z_max);
	    fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_1");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n","  0");

      	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_2");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  4");
        fprintf(geo_file,"%s\n","  5");
        fprintf(geo_file,"%s\n","  7");
        fprintf(geo_file,"%s\n","  6");
        fprintf(geo_file,"%s\n","  4");

       	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_3");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n","  4");
        fprintf(geo_file,"%s\n","  6");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n","  0");

      	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_4");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n","  5");
        fprintf(geo_file,"%s\n","  7");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n","  1");

      	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  4");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_5");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n","  6");
        fprintf(geo_file,"%s\n","  7");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n","  2");

      	fprintf(geo_file,"%s\n","#POLYLINE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  5");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_6");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $EPSILON");
        fprintf(geo_file,"%s\n","  0.000000e+000");
        fprintf(geo_file,"%s\n"," $MAT_GROUP");
        fprintf(geo_file,"%s\n","  -1");
        fprintf(geo_file,"%s\n"," $POINTS");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n","  4");
        fprintf(geo_file,"%s\n","  5");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n","  0");
       	
        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_1");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_1");

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  1");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_2");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_2");

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  2");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_3");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_3");

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  3");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_4");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_4");

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  4");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_5");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_5");

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $ID");
        fprintf(geo_file,"%s\n","  5");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  PLANE_6");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  PLANE_6");

        fprintf(geo_file,"%s\n","#STOP");
        fclose(geo_file);





  GEOLIB_Read_GeoLib((string)m_strFileNameBase);

  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".gli");

}

void CGSFormRightMeshing::OnBnClickedTetVolumeButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CGeoSysDoc *m_pDoc = GetDocument();

    if(m_frame->m_3dcontrol_msh_quality==1)
    {
        Get_MSH_for_Quality (0);
        m_frame->m_something_changed = 1;
        m_frame->m_3dcontrol_msh_quality=0;
    }
    else 
    {

   	CGS_OGL_Tetra_View dlg;
	if (dlg.DoModal() == IDOK) {
	m_frame->quality_range_min = dlg.m_min_quality_value;
	m_frame->quality_range_max = dlg.m_max_quality_value;
    }
    m_frame->m_3dcontrol_msh_quality=1;
  	Get_MSH_for_Quality (0);
	CalculateBasicTetrahedraData();
	CalculateTetrahedraVolumeQuality();
    m_frame->m_something_changed = 1;
    }
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedTetAngleButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CGeoSysDoc *m_pDoc = GetDocument();

    if(m_frame->m_3dcontrol_msh_quality==1)
    {
        Get_MSH_for_Quality (0);
        m_frame->m_something_changed = 1;
        m_frame->m_3dcontrol_msh_quality=0;
    }
    else 
    {

   	CGS_OGL_Tetra_View dlg;
	if (dlg.DoModal() == IDOK) {
	m_frame->quality_range_min = dlg.m_min_quality_value;
	m_frame->quality_range_max = dlg.m_max_quality_value;
    }
    m_frame->m_3dcontrol_msh_quality=1;
  	Get_MSH_for_Quality (0);
	CalculateBasicTetrahedraData();
	CalculateTetrahedraTriangleAngleQuality();
    m_frame->m_something_changed = 1;
    }
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

}

void CGSFormRightMeshing::OnBnClickedSetMeshdensity()
{
  UpdateData(TRUE);

	int i=0;
	vector<CGLPoint*> gli_points_vector;
	gli_points_vector = GetPointsVector();

    for (i=0;i<(long)gli_points_vector.size();i++)
    { 	
        gli_points_vector[i]->mesh_density =   m_mesh_density;
       
    }
}

void CGSFormRightMeshing::OnBnClickedDeleteDisplayedMesh()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysDoc *m_pDoc = GetDocument();
  //========================================================================
  int i=0;
  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      if (fem_msh_vector[i]->ele_display_mode == 1 ||
          fem_msh_vector[i]->nod_display_mode == 1)
      {
          fem_msh_vector.erase(fem_msh_vector.begin()+i);
          break;
      }
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSFormRightMeshing::OnBnClickedPatchindexForPlains()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysDoc *m_pDoc = GetDocument();
  int i=0, j=0, k=0;
  CGLPoint point;
  double dist;
  double tolerance = 0.001;
  double tri_point1[3],tri_point2[3],tri_point3[3],checkpoint[3];
  double tri_x[3],tri_y[3],tri_z[3];
  double min_mesh_dist=0.0;
  int pnt;
  int patch_index = -1;

  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Start Searching Plains:");

  //Loop over all meshes
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            fem_msh_vector[j]->ele_vector[i]->SetPatchIndex(patch_index);
        }
    //Loop over all edges
        for(i=0;i<(long)fem_msh_vector[j]->edge_vector.size();i++)
        {
       
            if (j==0 && i==0){
              min_mesh_dist = fem_msh_vector[j]->edge_vector[i]->Length();
            }
            else{
              if (min_mesh_dist  > fem_msh_vector[j]->edge_vector[i]->Length())
                  min_mesh_dist = fem_msh_vector[j]->edge_vector[i]->Length();
            }
        }
        tolerance = min_mesh_dist;
    }


  for(j=0;j<(long)fem_msh_vector.size();j++)
  {    
   if (j==0)
   {
     for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
     {
     if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 4)
	 {
         if (fem_msh_vector[j]->ele_vector[i]->GetPatchIndex() == -1)
         {
            pnt = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(0);
            tri_point1[0] = fem_msh_vector[j]->nod_vector[pnt]->X();
            tri_point1[1] = fem_msh_vector[j]->nod_vector[pnt]->Y();
            tri_point1[2] = fem_msh_vector[j]->nod_vector[pnt]->Z();
            pnt = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(1);
            tri_point2[0] = fem_msh_vector[j]->nod_vector[pnt]->X();
            tri_point2[1] = fem_msh_vector[j]->nod_vector[pnt]->Y();
            tri_point2[2] = fem_msh_vector[j]->nod_vector[pnt]->Z();
            pnt = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(2);
            tri_point3[0] = fem_msh_vector[j]->nod_vector[pnt]->X();
            tri_point3[1] = fem_msh_vector[j]->nod_vector[pnt]->Y();
            tri_point3[2] = fem_msh_vector[j]->nod_vector[pnt]->Z();

            tri_x[0]=tri_point1[0];
            tri_x[1]=tri_point2[0];
            tri_x[2]=tri_point3[0];
            tri_y[0]=tri_point1[1];
            tri_y[1]=tri_point2[1];
            tri_y[2]=tri_point3[1];
            tri_z[0]=tri_point1[2];
            tri_z[1]=tri_point2[2];
            tri_z[2]=tri_point3[2];

             CString item_name;
             item_name.Format(_T("%d"),fem_msh_vector[j]->ele_vector[i]->GetIndex());
             _tprintf(_T("%s"), (LPCTSTR) item_name);
             item_name = "Searching Plain for Element Index: (" + item_name + ")";
             pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)item_name);

            patch_index++;

       //Loop over all mesh nodes
        for(k=0;k<(long)fem_msh_vector[j]->nod_vector.size();k++)
        {
            point.x = fem_msh_vector[j]->nod_vector[k]->X();
            point.y = fem_msh_vector[j]->nod_vector[k]->Y();
            point.z = fem_msh_vector[j]->nod_vector[k]->Z();
            checkpoint[0] = fem_msh_vector[j]->nod_vector[k]->X();
            checkpoint[1] = fem_msh_vector[j]->nod_vector[k]->Y(); 
            checkpoint[2] = fem_msh_vector[j]->nod_vector[k]->Z();
            dist = MCalcDistancePointToPlane(checkpoint,tri_point1,tri_point2,tri_point3);
            if (i==0) fem_msh_vector[j]->nod_vector[k]->epsilon = dist;
            else
            {
                if (fem_msh_vector[j]->nod_vector[k]->epsilon > dist)
                    fem_msh_vector[j]->nod_vector[k]->epsilon = dist;
            }

                if (dist<=tolerance && dist>=-tolerance)
                {
                  fem_msh_vector[j]->nod_vector[k]->selected = 1;
                }
        }     

        // Loop over all mesh elements
        vec<long> node_index(20);

        for(k=0;k<(long)fem_msh_vector[j]->ele_vector.size();k++)
        {
            fem_msh_vector[j]->ele_vector[k]->GetNodeIndeces(node_index);

            if (fem_msh_vector[j]->ele_vector[k]->GetPatchIndex() == -1)
            {
            if (fem_msh_vector[j]->ele_vector[k]->GetElementType() == 3) /*Quad*/ 
            {
                if (fem_msh_vector[j]->nod_vector[node_index[0]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[1]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[2]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[3]]->selected == 1 )
                    fem_msh_vector[j]->ele_vector[k]->SetPatchIndex(patch_index);
                    //fem_msh_vector[j]->ele_vector[k]->selected = 1;   
            }

            if (fem_msh_vector[j]->ele_vector[k]->GetElementType() == 4) /*TRI*/ 
            {
                if (fem_msh_vector[j]->nod_vector[node_index[0]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[1]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[2]]->selected == 1 )
                    fem_msh_vector[j]->ele_vector[k]->SetPatchIndex(patch_index);
                    //fem_msh_vector[j]->ele_vector[k]->selected = 1;  
            }
            }
        }     



         
         }
  
     }
     }
   }
  }
 m_frame->dist_geo_object = 0.001;
 UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}


void CGSFormRightMeshing::OnBnClickedCombinePatchindex()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysDoc *m_pDoc = GetDocument();

  int i=0,j=0;
  int patch_index = -1;


    for (j=0; j<(long)fem_msh_vector.size(); j++)
    {
        for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
        {
            if (fem_msh_vector[j]->ele_vector[i]->matgroup_view == 1)
            {
                if (patch_index == -1)
                {
                    patch_index = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
                }
                else fem_msh_vector[j]->ele_vector[i]->SetPatchIndex(patch_index);
            }
        }
    }

 UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);

}

void CGSFormRightMeshing::OnBnClickedCompressPatchindex2()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysDoc *m_pDoc = GetDocument();

  int i=0,j=0,k=0;
  int patch_index = -1;
  int highest_mat_group_nb=0;
  int patch_used = 0;
  vector<long>used_patch;
  vector<long>new_patch;
  //vector<long>used_patch(highest_mat_group_nb);
  //vector<long>new_patch(highest_mat_group_nb);


    for (j=0; j<(long)fem_msh_vector.size(); j++)
    {
    if((int)fem_msh_vector[i]->ele_vector.size()>0)
    fem_msh_vector[j]->highest_mat_group_nb = fem_msh_vector[j]->ele_vector[0]->GetPatchIndex();
    for (i=0;i<(int)fem_msh_vector[j]->ele_vector.size();i++)
    {
        
        if (fem_msh_vector[j]->highest_mat_group_nb < fem_msh_vector[j]->ele_vector[i]->GetPatchIndex())
        fem_msh_vector[j]->highest_mat_group_nb = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
    }
    highest_mat_group_nb = fem_msh_vector[j]->highest_mat_group_nb +1;

    used_patch.resize(highest_mat_group_nb);
    new_patch.resize(highest_mat_group_nb);

    for (k=0;k<highest_mat_group_nb; k++)
    {
        patch_used = 0;

        for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
        { 
 
            if (fem_msh_vector[j]->ele_vector[i]->GetPatchIndex() == k)
            {
              patch_used = 1;
              break;
              //fem_msh_vector[j]->ele_vector[i]->SetPatchIndex(0);
            }
        }

        used_patch[k] = patch_used;

    }

        for (k=0;k<highest_mat_group_nb; k++)
        {
        patch_used = used_patch[k];

        if (used_patch[k]== 1)
        {
            patch_index++;
            new_patch[k] = patch_index;
        }
        else
        {
            new_patch[k]= -1;
        }
        }

        for (k=0;k<highest_mat_group_nb; k++)
        {
            patch_index = new_patch[k];
        }

        for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
        { 
            patch_index = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
            fem_msh_vector[j]->ele_vector[i]->SetPatchIndex(new_patch[patch_index]);
        }

    }

 used_patch.clear();
 new_patch.clear();

  //element_typ check;
  //fem_msh_vector[j]->face_vector[i]->GetNode(0)->X();

   UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);

}
