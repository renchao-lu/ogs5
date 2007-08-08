// GSPropertyRightMesh.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
//OK#include "Msctf.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSPropertyRightMesh.h"
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
#include "gs_prisgen.h"
#include "msh_mesh.h"
#include "delaunay3D.h"
#include "delaunay2D.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"


// CGSPropertyRightMesh dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightMesh, CViewPropertyPage)
CGSPropertyRightMesh::CGSPropertyRightMesh()
	: CViewPropertyPage(CGSPropertyRightMesh::IDD)
{
    m_patch_index = 0;
    m_mesh_density = 0.0;
}

CGSPropertyRightMesh::~CGSPropertyRightMesh()
{
}

void CGSPropertyRightMesh::DoDataExchange(CDataExchange* pDX)
{
	CViewPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGSPropertyRightMesh)
	DDX_Control(pDX, IDC_LIST_MESH, m_List);
    DDX_Text(pDX, IDC_PATCH_INDEX, m_patch_index);
    DDX_Text(pDX, IDC_MESHDENSITY_INDEX, m_mesh_density);
    //}}AFX_DATA_MAP
	OnPaint();
}

void CGSPropertyRightMesh::OnPaint()
{
    //pDC = pDC;
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


BEGIN_MESSAGE_MAP(CGSPropertyRightMesh, CViewPropertyPage)
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
    ON_BN_CLICKED(IDC_MERGE_MESHES, OnBnClickedMergeMeshes)
    ON_BN_CLICKED(IDC_CHECKDOUBLENODES, OnBnClickedCheckdoublenodes)
    ON_BN_CLICKED(IDC_DELETEDOUBLENODES, OnBnClickedDeletedoublenodes)
    ON_BN_CLICKED(IDC_SPLITELEMENTS, OnBnClickedSplitelements)
    ON_BN_CLICKED(IDC_Y2Z, OnBnClickedY2z)
    ON_BN_CLICKED(IDC_X2Y, OnBnClickedX2y)
    ON_BN_CLICKED(IDC_X2Z, OnBnClickedX2z)
    ON_BN_CLICKED(IDC_MAPPING_RIGHT, OnBnClickedMappingRight)
    ON_BN_CLICKED(IDC_CREATE_ELLIPSOID, OnBnClickedCreateEllipsoid)
    ON_BN_CLICKED(IDC_NEW_MSH_FROM_MATMESH, OnBnClickedNewMeshfromMATMesh)
	ON_BN_CLICKED(IDC_MSH_TEST_DELAUNAY2DLIB, OnBnClickedMshTestDelaunay2dlib)

	ON_BN_CLICKED(IDC_MSH_TEST_DELAUNAY3DLIB, OnBnClickedMshTestDelaunay3DLib)
	ON_BN_CLICKED(IDC_CREATE_ELLIPSE2D, OnBnClickedCreateEllipse2d)
	ON_BN_CLICKED(IDC_MSHNODES_ALONG_POLYLINE, OnBnClickedMshnodesAlongPolyline)
    ON_BN_CLICKED(IDC_CREATE_LINES_ALONG_POLYLINE, OnBnClickedCreateLinesAlongPolyline)
END_MESSAGE_MAP()


// CGSPropertyRightMesh message handlers



// CGSPropertyRightMesh message handlers

void CGSPropertyRightMesh::OnBnClickedTri2priButton()
{
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
        MSH_OverWriteMSH((string)m_strFileNameMSH,meshtypechange.m_numberofprismlayers);
      }
    }
  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  CompleteMesh(); 
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
  OnPaint();
  // Add MSH member to GSP vector
  //GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".rfi");



}

void CGSPropertyRightMesh::OnBnClickedQuad2hexButton()
{
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
        MSH_OverWriteMSH((string)m_strFileNameMSH,meshtypechange.m_numberofprismlayers);
      }


    }

  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  CompleteMesh(); 
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
  OnPaint();
}


void CGSPropertyRightMesh::OnBnClickedHex2tetButton()
{
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
  CompleteMesh(); 
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
  OnPaint();
}

void CGSPropertyRightMesh::OnBnClickedQuad2triButton()
{
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  //========================================================================
  CFEMesh*m_msh = fem_msh_vector[0];

      UpdateData(TRUE);
      if(fem_msh_vector.size()==1)
      {	
          Create_Quads2Tri(m_msh);
          MSH_OverWriteMSH((string)m_strFileNameMSH,0);
      }
  //========================================================================

  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  FEMRead((string)m_strFileNameBase);
  CompleteMesh(); 
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
  OnPaint();
}


void CGSPropertyRightMesh::OnBnClickedStruc2dButton()
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
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
      double x=0.0,y=0.0,z=0.0, y2=0.0;
      double x_max = dlg.m_edit_x_max;
      double y_max = dlg.m_edit_y_max;
      double slope_x = dlg.slope_x;
      double slope_y = dlg.slope_y;
      long delta_x = dlg.m_delta_x;
      long delta_y = dlg.m_delta_y;
      double x_length = x_max - x_min;
      double y_length = y_max - y_min;
      double x_step = x_length/delta_x;
      double y_step = y_length/delta_y;
      long point1,point2,point3,point4,point5;
      int number_of_x_points=0;
      
      double z_origin    = origin;
      double z_origin_x  = z_origin + (slope_x*(x_max-x_min));
      double z_origin_y  = z_origin + (slope_y*(y_max-y_min));
      double z_origin_xy = z_origin + (slope_x*(x_max-x_min))+(slope_y*(y_max-y_min));

      //write gli file
       FILE *geo_file=NULL;
       geo_file = fopen(gli_file_name_const_char, "w+t");

      	fprintf(geo_file,"%s\n","#POINTS");
        fprintf(geo_file,"%i %lg %lg %lg\n",0, x_min, y_min, z_origin);
        fprintf(geo_file,"%i %lg %lg %lg\n",1, x_max, y_min, z_origin_x);
        fprintf(geo_file,"%i %lg %lg %lg\n",2, x_max, y_max, z_origin_xy);
        fprintf(geo_file,"%i %lg %lg %lg\n",3, x_min, y_max, z_origin_y);
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

    if (dlg.m_do_structured_triangle_mesh == FALSE && 
		dlg.m_do_structured_rectangle_mesh == FALSE &&
		dlg.m_do_structured_only_equi_triangle_mesh == FALSE) return;  

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
	    if (dlg.m_do_structured_only_equi_triangle_mesh == TRUE)
		{
		 x_step = (y_step/2)*(sqrt(3.0));
    	 number_of_x_points = ((int)((x_max-x_min)/x_step))+2;
		 if(number_of_x_points%2==0) number_of_x_points = number_of_x_points;
		 else number_of_x_points++;
         nNod = number_of_x_points* (delta_y +1);
		}
        fprintf(msh_file,"% ld\n",nNod);
    //write Geometry    
    m_ProgressBar.SetRange((short)0,(short)nNod+(short)nEle);
    m_ProgressBar.SetStep(1); 
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Creating Nodes");  
        int nnb=0;

    if (dlg.m_do_structured_triangle_mesh == TRUE || 
		dlg.m_do_structured_rectangle_mesh == TRUE)
	{
   	for(j=0 ;j<=delta_y;j++) 
    {
        y = y_min + (j*y_step);
        z_origin_y  = z_origin + (slope_y*y);
        for(i=0 ;i<=delta_x;i++) 
        {
        x = x_min + (i*x_step);
        z  = z_origin_y + (slope_x*x);
        fprintf( msh_file, "%d ", nnb);
        fprintf( msh_file, "%20.14f %20.14f %20.14f\n", x, y, z);
        nnb++;
        m_ProgressBar.StepIt();
	    }
    }
	}
    if (dlg.m_do_structured_only_equi_triangle_mesh == TRUE)
	{
   	for(j=0 ;j<=delta_y;j++) 
    {
        y = y_min + (j*y_step);
		y2 = y + (0.5*y_step);
        z_origin_y  = z_origin + (slope_y*y);
		x_step = (y_step/2)*(sqrt(3.0));
        for(i=0 ;i<number_of_x_points;i++) 
        {
		if(i%2==0)
		{
        x = x+(y_step/2)*(sqrt(3.0));
		if (i==0)x =  x_min;
        z  = z_origin_y + (slope_x*x);
        fprintf( msh_file, "%d ", nnb);
        fprintf( msh_file, "%20.14f %20.14f %20.14f\n", x, y, z);
        nnb++;
        x = x+(y_step/2)*(sqrt(3.0));
        z  = z_origin_y + (slope_x*x);
        fprintf( msh_file, "%d ", nnb);
        fprintf( msh_file, "%20.14f %20.14f %20.14f\n", x, y2, z);
        nnb++;
        m_ProgressBar.StepIt();
	    }
		}

    }
	}

    //Write Elements
	   	fprintf( msh_file, "%s\n", " $ELEMENTS");
        if (dlg.m_do_structured_rectangle_mesh == TRUE) nEle = delta_x * delta_y;
        if (dlg.m_do_structured_triangle_mesh == TRUE)  nEle = delta_x * delta_y * 2;
        if (dlg.m_do_structured_only_equi_triangle_mesh == TRUE)  nEle = ((number_of_x_points-1) *delta_y*2);
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
    if (dlg.m_do_structured_only_equi_triangle_mesh == TRUE)
    { 
    	for(j=0;j<(delta_y);j++) 
        {
             y_jump = number_of_x_points*j;

        for(i=0 ;i<=number_of_x_points-2;i++) 
        {
            point1 = y_jump+i;
            point2 = y_jump+i+1;
            point3 = y_jump+i+number_of_x_points;
            point4 = y_jump+i+2;
			point5 = y_jump+i+number_of_x_points+1;

            if(i%2==0)
			{
			fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point1, point2, point3);
            nel++;
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point2, point3, point5);
            nel++;
            m_ProgressBar.StepIt();
			}
			else
			{
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point1, point2, point5);
            nel++;
            fprintf( msh_file, "%d ", nel);
            fprintf( msh_file, " 0 -1 tri ");     
		    fprintf( msh_file, "%d  %d  %d\n",point1, point3, point5);
            nel++;
            m_ProgressBar.StepIt();
		    }

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
  CompleteMesh(); 

  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);
  OnPaint();
  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".gli");
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");
  }

}

void CGSPropertyRightMesh::OnBnClickedTriDelaunayButton()
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
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
  CompleteMesh(); 
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);
  OnPaint();
  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");


}

void CGSPropertyRightMesh::OnBnClickedTriAreaiButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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

void CGSPropertyRightMesh::OnBnClickedTriAngleButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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

void CGSPropertyRightMesh::OnBnClickedTriLengthButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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

void CGSPropertyRightMesh::OnBnClickedTriQualityGraphButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    m_frame->OnViewOGLCreate();
}

void CGSPropertyRightMesh::OnBnClickedAutomaticMeshDensity()
{
    GEO_Get_Min_Max_Distance_of_polyline_neighbor_points();
    GEO_Copy_Min_Distance_Of_Neighbor_Points_To_Mesh_Density();
}

void CGSPropertyRightMesh::OnBnClickedGeometricMeshDensity2()
{
    GEO_Get_Min_Distance_of_neighbor_points();
    GEO_Copy_Min_Distance_Of_Neighbor_Points_To_Mesh_Density();
}

void CGSPropertyRightMesh::OnBnClickedGlobalSplitMeshDensity3()
{
    GEO_Mesh_Density_BiSect();
}

void CGSPropertyRightMesh::OnBnClickedStartGmshMeshing()
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
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGEO = m_strFileNameBase + "_temp.geo";
  CString m_strFileNameGMSH = m_strFileNameBase + "_temp.msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  string m_strExecuteGEO = "gmsh " + m_strFileNameGEO +" -2";
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
  /*GMSH EXE Path*/ 
   LPTSTR filename = new TCHAR[1024];
   GetModuleFileName(NULL,filename,1024);
   string m_strExecuteGEOonly = " " + m_strFileNameGEO;
   CFileFind finder;
   CString strWildcard(filename);
   CString LIB_PATH;
   CString Folder_Level_0 = strWildcard.Left(strWildcard.ReverseFind('\\'));
   CString Folder_Level_1 = Folder_Level_0.Left(Folder_Level_0.ReverseFind('\\'));
   CString Folder_Level_2 = Folder_Level_1.Left(Folder_Level_1.ReverseFind('\\'));
   Folder_Level_1 = Folder_Level_1 + "\\*.*";
   Folder_Level_2 = Folder_Level_2 + "\\*.*";  
   LIB_PATH = Folder_Level_0;
   m_strExecuteGEO = LIB_PATH + "\\gmsh" + m_strExecuteGEOonly.data();
   BOOL bWorking = finder.FindFile(Folder_Level_2);
   while (bWorking)
   {
      bWorking = finder.FindNextFile();
      if (finder.IsDots())
         continue;
      if (finder.IsDirectory())
      {
         CString str = finder.GetFilePath();        
         CString Folder = str.Right(str.GetLength()-str.ReverseFind('\\')-1);

         if(Folder.Compare("LIB")==0)
         {
           LIB_PATH = str;
           m_strExecuteGEO = LIB_PATH + "\\gmsh" + m_strExecuteGEOonly.data();
         }
      }
   }
   finder.Close();
  const char *m_strExecute=0;
  //TODO
  m_strExecuteGEO = "\"" + m_strExecuteGEO + "\"";
  m_strExecuteGEO = m_strExecuteGEO +" -2";
  m_strExecute =  m_strExecuteGEO.data();

  //UINT callback =WinExec(m_strExecute,SW_HIDE);

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
 OnPaint();
 m_ProgressBar.StepIt();
 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
 }
}

void CGSPropertyRightMesh::OnBnClickedMshnodesInPolygon()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Nodes:");
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase + "_temp";
  CString m_strFileNameGLI = m_strFileNameBase + ".gli";
  CString m_strFileNameGEO = m_strFileNameBase + ".geo";
  CString m_strFileNameTIN = m_strFileNameBase + ".tin";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameBase; 
  int i=0;

  for (i=0;i<(int)surface_vector.size();i++)
  {
      if (surface_vector[i]->display_mode_3d == 1)
      {
       if (surface_vector[i]->TIN)
       {
       /*TIN Search*/ 
       pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Nodes: TIN.......please wait");
       m_strFileNameTIN = m_pDoc->m_strGSPFilePath + surface_vector[i]->TIN->name.c_str();
       Select_Nodes_Elements_by_TINFile(m_strFileNameTIN);
       break;
       }
       else
       {
       /*Surface Meshing & Search*/ 
       pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Nodes:.......please wait");
       Mesh_Single_Surface(surface_vector[i]->name, file_name_const_char);
       GMSH2TIN(file_name_const_char);
       Select_Nodes_Elements_by_TINFile(file_name_const_char);
       }
       break;
      }
  }
       /*Node and Element Selection*/ 


 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");

 m_frame->dist_geo_object = 0.001;
 UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedClearSelection()
{
 Clear_Selected_Nodes_Elements();
 
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedMinMaxEdgeLength()
{
 int i=0, j=0;
 double edge_length;
 double min_edge_length=0.0;
 double max_edge_length=0.0;
 long node_id=0;
 long node_id_old=0;
 long ele_id=0;
 long ele_id_old=0;
 
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
              m_List.AddString(_T(">***********************************"));
              m_List.AddString(_T(">MSH data loaded"));
              m_List.AddString(_T(">***********************************"));
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

        /*FragmentedMesh Check*/ 
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
            node_id = fem_msh_vector[j]->nod_vector[i]->GetIndex();
            if(node_id>0 && node_id != node_id_old+1)
            {
                sprintf(string2add, "%lg",node_id);
                info_string = ">Gap after Node: " ;
                info_string.append(string2add);
                m_List.AddString(_T(info_string.data()));
            }
            node_id_old = fem_msh_vector[j]->nod_vector[i]->GetIndex();
        }
            if(node_id != i-1)
            {
                info_string = ">FRAGMENTED NODES!!!" ;
                m_List.AddString(_T(info_string.data()));
            }

        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            ele_id = fem_msh_vector[j]->ele_vector[i]->GetIndex();
            if(ele_id>0 && ele_id != ele_id_old+1)
            {
                sprintf(string2add, "%lg",ele_id);
                info_string = ">Gap after Element: " ;
                info_string.append(string2add);
                m_List.AddString(_T(info_string.data()));
            }
            ele_id_old = fem_msh_vector[j]->ele_vector[i]->GetIndex();
        }
            if(ele_id != i-1)
            {
                info_string = ">FRAGMENTED ELEMENTS!!!" ;
                m_List.AddString(_T(info_string.data()));
            }
 
    }

        m_List.AddString(_T(">***********************************"));
        


 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);

}

void CGSPropertyRightMesh::OnBnClickedSelectAll()
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
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedSetPatchIndex()
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
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedGmshMshImport()
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
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
	OnPaint();
    m_ProgressBar.StepIt();
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
    }
  }
}
void CGSPropertyRightMesh::OnBnClickedCreateGeoBoundingbox()
{
    //========================================================================
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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

void CGSPropertyRightMesh::OnBnClickedTetVolumeButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();

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

void CGSPropertyRightMesh::OnBnClickedTetAngleButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();

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

void CGSPropertyRightMesh::OnBnClickedSetMeshdensity()
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

void CGSPropertyRightMesh::OnBnClickedDeleteDisplayedMesh()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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
 OnPaint();
}

void CGSPropertyRightMesh::OnBnClickedPatchindexForPlains()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
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


void CGSPropertyRightMesh::OnBnClickedCombinePatchindex()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();

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

void CGSPropertyRightMesh::OnBnClickedCompressPatchindex2()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();

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

void CGSPropertyRightMesh::OnBnClickedMergeMeshes()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0, k=0;
  int mesh_counter = 0;
  int node_vec_size=0;
  int ele_vec_size=0;

  CFEMesh*m_msh = NULL;
  CFEMesh*m_msh_temp = NULL;
  CFEMesh*m_msh_o = NULL;
  CElem *m_ele = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      m_msh_o = fem_msh_vector[i];
      if (m_msh_o->ele_display_mode == 1 ||
          m_msh_o->nod_display_mode == 1)
      {
          /*copy first mesh*/ 
          if (mesh_counter == 0)
          {
          m_msh = new CFEMesh();
          m_msh->pcs_name = m_msh_o->pcs_name + "_test";
/* NW
          m_msh->nod_vector = m_msh_o->nod_vector;
*/
          m_msh->nod_vector.resize(m_msh_o->nod_vector.size());
          CNode *tmp_node;
          for (j=0;j<(long)m_msh_o->nod_vector.size();j++) 
          {
            tmp_node = new CNode(m_msh_o->nod_vector[j]->GetIndex(), m_msh_o->nod_vector[j]->X(), m_msh_o->nod_vector[j]->Y(), m_msh_o->nod_vector[j]->Z());
            m_msh->nod_vector[j] = tmp_node;
          }
/* NW
          m_msh->ele_vector = m_msh_o->ele_vector;
*/
          m_msh->ele_vector.resize(m_msh_o->ele_vector.size());
          CElem *tmp_elem;
          vec<CNode*> vec_nodes;
          for (j=0;j<(long)m_msh_o->ele_vector.size();j++) 
          {
            tmp_elem = new CElem(j, m_msh_o->ele_vector[j]);
            vec_nodes.resize(m_msh_o->ele_vector[j]->GetNodesNumber(false));
            for (k=0;k<(int)vec_nodes.Size();k++)
            {
              vec_nodes[k] = m_msh->nod_vector[m_msh_o->ele_vector[j]->GetNodeIndex(k)];
            }
            tmp_elem->SetNodes(vec_nodes);
            m_msh->ele_vector[j] = tmp_elem;
          }
          fem_msh_vector.push_back(m_msh);        
          m_msh_o->ele_display_mode = 0;
          m_msh_o->nod_display_mode = 0;
          mesh_counter++;
          }
      }
      if (m_msh_o->ele_display_mode == 1 ||
          m_msh_o->nod_display_mode == 1)
      {   
         /*add next mesh*/ 
          if (mesh_counter >= 1)
          {
          node_vec_size = (int)m_msh->nod_vector.size();
          ele_vec_size = (int)m_msh->ele_vector.size();
          m_msh_temp = new CFEMesh();
          m_msh_temp->pcs_name = m_msh_o->pcs_name + "_temp";
/* NW
          m_msh_temp->nod_vector = m_msh_o->nod_vector;
*/
          m_msh_temp->nod_vector.resize(m_msh_o->nod_vector.size());
          CNode *tmp_node;
          for (j=0;j<(long)m_msh_o->nod_vector.size();j++) 
          {
            tmp_node = new CNode(m_msh_o->nod_vector[j]->GetIndex(), m_msh_o->nod_vector[j]->X(), m_msh_o->nod_vector[j]->Y(), m_msh_o->nod_vector[j]->Z());
            m_msh_temp->nod_vector[j] = tmp_node;
          }
/* NW
          m_msh_temp->ele_vector = m_msh_o->ele_vector;
*/
          m_msh_temp->ele_vector.resize(m_msh_o->ele_vector.size());
          CElem *tmp_elem;
          vec<CNode*> vec_nodes;
          for (j=0;j<(long)m_msh_o->ele_vector.size();j++) 
          {
            tmp_elem = new CElem(j, m_msh_o->ele_vector[j]);
            vec_nodes.resize(m_msh_o->ele_vector[j]->GetNodesNumber(false));
            for (k=0;k<(int)vec_nodes.Size();k++)
            {
              vec_nodes[k] = m_msh_temp->nod_vector[m_msh_o->ele_vector[j]->GetNodeIndex(k)];
            }
            tmp_elem->SetNodes(vec_nodes);
            m_msh_temp->ele_vector[j] = tmp_elem;
          }

           for (j=0;j<(int)m_msh_temp->nod_vector.size();j++)
           m_msh_temp->nod_vector[j]->SetIndex(j + (int)m_msh->nod_vector.size());

           for (j=0;j<(int)m_msh_temp->ele_vector.size();j++)
           {
            m_ele =m_msh_o->ele_vector[j];
            m_msh_temp->ele_vector[j]->SetIndex(j + (int)m_msh->ele_vector.size());
            for (k=0;k<(int)m_ele->GetVertexNumber();k++)
                m_msh_temp->ele_vector[j]->SetNodeIndex(k,(int)m_msh->nod_vector.size() + m_ele->GetNodeIndex(k));
           }
           m_msh->nod_vector.resize(node_vec_size + m_msh_temp->nod_vector.size());
           for (j=0;j<(int)m_msh_temp->nod_vector.size();j++)
           m_msh->nod_vector[j+node_vec_size] = m_msh_temp->nod_vector[j];         
           m_msh->ele_vector.resize(ele_vec_size + m_msh_temp->ele_vector.size());
           for (j=0;j<(int)m_msh_temp->ele_vector.size();j++)
           m_msh->ele_vector[j+ele_vec_size] = m_msh_temp->ele_vector[j];  
         
          fem_msh_vector[i]->ele_display_mode = 0;
          fem_msh_vector[i]->nod_display_mode = 0;
          }

      }
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
 OnPaint();
}

void CGSPropertyRightMesh::OnBnClickedCheckdoublenodes()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0, k=0;
  double search_tolerance=0;
  double node_distance=0;
  CString iteration_name;
  CString iteration_max;

  CFEMesh*m_msh_temp = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      if (fem_msh_vector[i]->ele_display_mode == 1 ||
          fem_msh_vector[i]->nod_display_mode == 1)
      {   
          m_msh_temp = new CFEMesh();
          m_msh_temp->pcs_name = "double_node_check";
          m_msh_temp->nod_vector = fem_msh_vector[i]->nod_vector;
          m_msh_temp->ele_vector = fem_msh_vector[i]->ele_vector;
          m_msh_temp->edge_vector = fem_msh_vector[i]->edge_vector;

          GetMinMaxEdgeLength(m_msh_temp);
          search_tolerance = m_msh_temp->min_edge_length;

           for (j=0;j<(int)fem_msh_vector[i]->nod_vector.size();j++)
           {
             iteration_max.Format(_T("%d"),(int)fem_msh_vector[i]->nod_vector.size());
             _tprintf(_T("%s"), (LPCTSTR) iteration_max);
             iteration_name.Format(_T("%d"),j+1);
             _tprintf(_T("%s"), (LPCTSTR) iteration_name);
             iteration_name = "Searching Double Nodes: " + iteration_name + " from " + iteration_max;
             pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)iteration_name);

             for (k=j+1;k<(int)m_msh_temp->nod_vector.size();k++)
             {
			    node_distance =    EuklVek3dDistCoor ( 
                                    fem_msh_vector[i]->nod_vector[j]->X(),
                                    fem_msh_vector[i]->nod_vector[j]->Y(),
                                    fem_msh_vector[i]->nod_vector[j]->Z(),
                                    m_msh_temp->nod_vector[k]->X(),
                                    m_msh_temp->nod_vector[k]->Y(),
                                    m_msh_temp->nod_vector[k]->Z());
                 
                if (node_distance < search_tolerance)
                {
                    fem_msh_vector[i]->nod_vector[k]->selected = 1;
                }
             }
           }       
      }
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Double Nodes: Finished");  

  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::GetMinMaxEdgeLength(CFEMesh*m_msh)
{
 int i=0, j=0;
 double edge_length;
 double min_edge_length=0.0;
 double max_edge_length=0.0;
    
        for(i=0;i<(long)m_msh->edge_vector.size();i++)
        {
            edge_length = m_msh->edge_vector[i]->Length();

            if (j==0 && i==0)
            {
              min_edge_length = edge_length;
              max_edge_length = edge_length;
            }
            else
            {
              if (min_edge_length > edge_length)min_edge_length = edge_length;
              if (max_edge_length < edge_length)max_edge_length = edge_length;
            }
        }            
        m_msh->min_edge_length = min_edge_length;
        m_msh->max_edge_length = max_edge_length;
}

void CGSPropertyRightMesh::OnBnClickedDeletedoublenodes()
{ 
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0, k=0;
  int index;
  double search_tolerance=0;
  double node_distance=0;
  CString iteration_name;
  CString iteration_max;

  
  CFEMesh*m_msh_temp = NULL;
  CFEMesh*m_msh_serial = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      if (fem_msh_vector[i]->ele_display_mode == 1 ||
          fem_msh_vector[i]->nod_display_mode == 1)
      {   
          m_msh_temp = new CFEMesh();
          m_msh_temp->pcs_name = "double_node_check";
          m_msh_temp->nod_vector = fem_msh_vector[i]->nod_vector;
          m_msh_temp->ele_vector = fem_msh_vector[i]->ele_vector;
          m_msh_temp->edge_vector = fem_msh_vector[i]->edge_vector;

          if (m_msh_temp->edge_vector.size() > 0) //NW
          {
            GetMinMaxEdgeLength(m_msh_temp);
            search_tolerance = m_msh_temp->min_edge_length;
          } else {
            search_tolerance = 1.0e-10;
          }

           /*Check&Mark*/ 
           for (j=0;j<(int)fem_msh_vector[i]->nod_vector.size();j++)
           {
             iteration_max.Format(_T("%d"),(int)fem_msh_vector[i]->nod_vector.size());
             _tprintf(_T("%s"), (LPCTSTR) iteration_max);
             iteration_name.Format(_T("%d"),j+1);
             _tprintf(_T("%s"), (LPCTSTR) iteration_name);
             iteration_name = "Searching Double Nodes: " + iteration_name + " from " + iteration_max;
             pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)iteration_name);

             for (k=j+1;k<(int)m_msh_temp->nod_vector.size();k++)
             {
                if (m_msh_temp->nod_vector[k]->selected != 1)
                {
			    node_distance =    EuklVek3dDistCoor ( 
                                    fem_msh_vector[i]->nod_vector[j]->X(),
                                    fem_msh_vector[i]->nod_vector[j]->Y(),
                                    fem_msh_vector[i]->nod_vector[j]->Z(),
                                    m_msh_temp->nod_vector[k]->X(),
                                    m_msh_temp->nod_vector[k]->Y(),
                                    m_msh_temp->nod_vector[k]->Z());
                 
                if (node_distance < search_tolerance)
                {
                    fem_msh_vector[i]->nod_vector[k]->selected = 1;
                    m_msh_temp->nod_vector[k]->selected = 1;
                    index = fem_msh_vector[i]->nod_vector[j]->GetIndex();
                    m_msh_temp->nod_vector[k]->SetIndex(index);
                }
                }
             }
           }
           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Double Nodes: Finished");  

           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing Mesh: Start");  
           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing Mesh: sorting node numbers.........................please wait");
             /*Serialize Nodenumbers*/ 
             int counter=0;
             m_msh_serial = new CFEMesh();
             m_msh_serial->pcs_name = "node_serial";
             m_msh_serial->nod_vector = m_msh_temp->nod_vector;
             m_msh_serial->ele_vector = m_msh_temp->ele_vector;

             for (k=0;k<(int)m_msh_serial->nod_vector.size();k++)
             {                
                if (m_msh_serial->nod_vector[k]->selected != 1)
                {
                  m_msh_serial->nod_vector[k]->SetIndex(counter);
                  counter++;
                }        
             }
             for (k=0;k<(int)m_msh_temp->nod_vector.size();k++)
             {                
                if (m_msh_temp->nod_vector[k]->selected != 1)
                  m_msh_temp->nod_vector[k]->SetIndex(m_msh_serial->nod_vector[k]->GetIndex());
                else
                  m_msh_temp->nod_vector[k]->SetIndex(m_msh_serial->nod_vector[m_msh_temp->nod_vector[k]->GetIndex()]->GetIndex());
             }
           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing Mesh: sorting element numbers.........................please wait");


           /*Correct ElementNode Numbers*/ 
           long N0=0;
           long NN0=0;

           for (j=0;j<(int)fem_msh_vector[i]->ele_vector.size();j++)
           {
             for (k=0;k<(int)fem_msh_vector[i]->ele_vector[j]->GetVertexNumber();k++)
             {
                N0 = fem_msh_vector[i]->ele_vector[j]->GetNodeIndex(k);
                NN0 = m_msh_temp->nod_vector[N0]->GetIndex();
                m_msh_temp->ele_vector[j]->SetNodeIndex(k,NN0);
             }

           }
           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Deleting Nodes....................please wait");
           /*Delete DoubleNodes*/ 
           for (j=0;j<(int)m_msh_temp->nod_vector.size();j++)
           {
               if(m_msh_temp->nod_vector[j]->selected == 1)
               {
                   m_msh_temp->nod_vector.erase( m_msh_temp->nod_vector.begin()+j);
                   j--;
               }
           }
           pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Deleting Double Nodes: Finished");  

          fem_msh_vector[i]->nod_vector = m_msh_temp->nod_vector;
          fem_msh_vector[i]->ele_vector = m_msh_temp->ele_vector;

      }


  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedSplitelements()
{
  //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"EdgeFileGeneration: Start");  
   clock_t start, finish;
   double  duration;
   long hours=0,minutes=0,seconds=0;
   start = clock();
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameEdge = m_strFileNameBase + "_eges.txt";
  CString m_strFileNameNodes = m_strFileNameBase + "_nodes.txt";
  CString m_strFileNameElements = m_strFileNameBase + "_elements.txt";
  CString m_strFileNameNewMSH = m_strFileNameBase + "_refined.txt";

  const char *file_name_const_char = 0;
  FILE *node_file=NULL;
  file_name_const_char = m_strFileNameNodes;
  node_file = fopen(file_name_const_char, "w+t");
  FILE *ele_file=NULL;
  file_name_const_char = m_strFileNameElements;
  ele_file = fopen(file_name_const_char, "w+t");

  FILE *edge_file=NULL;
  file_name_const_char = m_strFileNameEdge;
  edge_file = fopen(file_name_const_char, "w+t");
  FILE *new_mesh_file=NULL;
  file_name_const_char = m_strFileNameNewMSH;
  new_mesh_file = fopen(file_name_const_char, "w+t");

  //========================================================================
  int i=0, j=0, k=0, m=0;
  CFEMesh*m_msh = NULL;
  CNode *m_node = NULL;
  CElem *m_ele = NULL;
  CEdge *m_edge = NULL;
  vec<CNode*> e_nodes(3);    
  vec<CNode*> control_edge_nodes(3);
  vector<int> new_edge_nodes;
  int point_counter=0;
  int element_counter=0;
  double x_midpoint=0.0,y_midpoint=0.0,z_midpoint=0.0;
  int nNod=0;
  char input_text[1024];
  long id;
  double x, y, z;
  int ok=0;
  CString iteration_name;
  CString iteration_max;
  CString duration_name;


  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      new_edge_nodes.resize(0);
      m_msh = fem_msh_vector[i];
      if (m_msh->ele_display_mode == 1 || m_msh->nod_display_mode == 1)
      { 
        /*Read&Write Edges + Midpointnumber + New Edges****************/ 
        fprintf(edge_file,"%s\n","#Edges");
        new_edge_nodes.resize((int)m_msh->edge_vector.size());
        for (j=0;j<(int)m_msh->edge_vector.size();j++)
        {
            new_edge_nodes[j] = (int)m_msh->nod_vector.size()+j;
            m_edge = m_msh->edge_vector[j];
            m_edge->GetNodes(e_nodes);

            fprintf(edge_file,"%i %i %i\n",e_nodes[0]->GetIndex(), 
                                             e_nodes[1]->GetIndex(),
                                               new_edge_nodes[j]);
        }

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Analyzing Nodes........");  
        /*Read&Write Nodes*********************************************/ 
        fprintf(node_file,"%s\n","#EdgeNodes");
        for (j=0;j<(int)m_msh->nod_vector.size();j++)
        {
            m_node = m_msh->nod_vector[j];
            fprintf(node_file,"%i ",m_node->GetIndex());
            fprintf(node_file,"%20.14f %20.14f %20.14f\n",m_node->X(),
                                                          m_node->Y(),
                                                          m_node->Z());

        }
        nNod=j;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Adding Nodes........");  
        /*Add Points toEdges*********************************************/ 
        fprintf(node_file,"%s\n","#Additional_Edge_Nodes");
        point_counter = (int)m_msh->nod_vector.size();
        for (j=0;j<(int)m_msh->edge_vector.size();j++)
        {
            m_edge = m_msh->edge_vector[j];
            m_edge->GetNodes(e_nodes);
            fprintf(node_file,"%i ",point_counter);      
            x_midpoint = (e_nodes[0]->X() + e_nodes[1]->X())/2;
            y_midpoint = (e_nodes[0]->Y() + e_nodes[1]->Y())/2;
            z_midpoint = (e_nodes[0]->Z() + e_nodes[1]->Z())/2;
            fprintf(node_file,"%20.14f %20.14f %20.14f\n",x_midpoint,y_midpoint,z_midpoint);
            point_counter++;
        }
        nNod = point_counter;
        fclose (node_file);
        fclose(edge_file);

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Analyzing Elements........");  
  finish = clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;

  /*Read&Write Elements*********************************************/ 
        file_name_const_char = m_strFileNameEdge;
        edge_file = fopen(file_name_const_char, "rt");
        int tri_point0,tri_point1,tri_point2;
        int edge0_point0,edge0_point1;
        int edge1_point0,edge1_point1;
        int edge2_point0,edge2_point1;
        long edge_nodes[6];
        int patch_index;
        long hit = 0;

        vector<long*> nodesofnewedges;
        long* nodes_info;
        vector<long*> used_edges;


        for (k=0;k<(int)m_msh->edge_vector.size();k++)
        {
            nodes_info = new long[4];
             m_edge = m_msh->edge_vector[k];
             m_edge->GetNodes(e_nodes);
             nodes_info[0] = e_nodes[0]->GetIndex();
             nodes_info[1] = e_nodes[1]->GetIndex();
             nodes_info[2] = new_edge_nodes[k];
             nodes_info[3] = 0;
            nodesofnewedges.push_back(nodes_info);
        }

        fprintf(ele_file,"%s\n","#Elements");
        for (j=0;j<(int)m_msh->ele_vector.size();j++)
        { 
             iteration_max.Format(_T("%d"),(int)fem_msh_vector[i]->ele_vector.size());
             _tprintf(_T("%s"), (LPCTSTR) iteration_max);
             iteration_name.Format(_T("%d"),j+1);
             _tprintf(_T("%s"), (LPCTSTR) iteration_name);
             iteration_name = "Checking to split element number: " + iteration_name + " from " + iteration_max;
           
             finish = clock();
             duration = (double)(finish - start) / CLOCKS_PER_SEC;
             duration_name.Format(_T("%5.0f"),duration);
             _tprintf(_T("%s"), (LPCTSTR) duration_name);
             iteration_name = iteration_name + "..................elapsed time: " + duration_name + " s";            
             pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)iteration_name);

           m_ele =m_msh->ele_vector[j];
           tri_point0 = m_ele->GetNodeIndex(0);
           tri_point1 = m_ele->GetNodeIndex(1);
           tri_point2 = m_ele->GetNodeIndex(2);
           patch_index = m_ele->GetPatchIndex();
           hit = 0;
           for (k=0;k<(int)nodesofnewedges.size();k++)
           {
             nodes_info = nodesofnewedges[k];
              if (tri_point0 == nodes_info[0] && tri_point1 == nodes_info[1]){
                 edge_nodes[0] = nodes_info[0];
                 edge_nodes[1] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
              if (tri_point1 == nodes_info[0] && tri_point2 == nodes_info[1]){
                 edge_nodes[2] = nodes_info[0];
                 edge_nodes[3] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
              if (tri_point2 == nodes_info[0] && tri_point0 == nodes_info[1]){
                 edge_nodes[4] = nodes_info[0];
                 edge_nodes[5] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
              if (tri_point0 == nodes_info[1] && tri_point1 == nodes_info[0]){
                 edge_nodes[0] = nodes_info[1];
                 edge_nodes[1] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
              if (tri_point1 == nodes_info[1] && tri_point2 == nodes_info[0]){
                 edge_nodes[2] = nodes_info[1];
                 edge_nodes[3] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
              if (tri_point2 == nodes_info[1] && tri_point0 == nodes_info[0]){
                 edge_nodes[4] = nodes_info[1];
                 edge_nodes[5] = nodes_info[2];
                 nodes_info[3] = nodes_info[3]+1;
                 nodesofnewedges[k] = nodes_info;
                 hit++;
              }
 
              if(nodes_info[3]>=2)
              {
                 nodesofnewedges.erase(nodesofnewedges.begin()+k);
                 used_edges.push_back(nodes_info);
                 k--;
              }
              if(hit==3){
                  hit=0;
                  break;
              }

              if (hit<3 && k==(int)nodesofnewedges.size()-1)
              {
                 for (m=0;m<(int)used_edges.size();m++)
                 {
                    if (tri_point0 == nodes_info[0] && tri_point1 == nodes_info[1]){
                        edge_nodes[0] = nodes_info[0];
                        edge_nodes[1] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }
                    if (tri_point1 == nodes_info[0] && tri_point2 == nodes_info[1]){
                        edge_nodes[2] = nodes_info[0];
                        edge_nodes[3] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }
                    if (tri_point2 == nodes_info[0] && tri_point0 == nodes_info[1]){
                        edge_nodes[4] = nodes_info[0];
                        edge_nodes[5] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }
                    if (tri_point0 == nodes_info[1] && tri_point1 == nodes_info[0]){
                        edge_nodes[0] = nodes_info[1];
                        edge_nodes[1] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }
                    if (tri_point1 == nodes_info[1] && tri_point2 == nodes_info[0]){
                        edge_nodes[2] = nodes_info[1];
                        edge_nodes[3] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }
                    if (tri_point2 == nodes_info[1] && tri_point0 == nodes_info[0]){
                        edge_nodes[4] = nodes_info[1];
                        edge_nodes[5] = nodes_info[2];
                        nodes_info[3] = nodes_info[3]+1;
                        used_edges[m] = nodes_info;
                        hit++;
                    }        
                    if(hit==3){
                        hit=0;
                        break;
                    }
                 }       
              }
           }

           fprintf(ele_file,"%i %i %i %i %i %i %i %i %i %i\n",tri_point0,tri_point1,tri_point2,
                                                    edge_nodes[0],edge_nodes[1],
                                                    edge_nodes[2],edge_nodes[3],
                                                    edge_nodes[4],edge_nodes[5],
                                                    patch_index);       
        }

        fprintf(ele_file,"%s\n","#STOP");
        fclose(ele_file);
        fclose(edge_file);

        for (m=0;m<(int)used_edges.size();m++)
        delete used_edges[m];
        used_edges.clear();
        for (k=0;k<(int)nodesofnewedges.size();k++)
        delete nodesofnewedges[k];
        nodesofnewedges.clear();

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Creating new external mesh........");  
 
        /*Create New Elements*********************************************/ 
        file_name_const_char = m_strFileNameNodes;
        node_file = fopen(file_name_const_char, "rt");
        file_name_const_char = m_strFileNameElements;
        ele_file = fopen(file_name_const_char, "rt");

        //write MSH Head
	   	fprintf( new_mesh_file, "%s\n", "#FEM_MSH");
        //write PCS Type
        fprintf( new_mesh_file, "%s\n", " $PCS_TYPE");
	   	fprintf( new_mesh_file, "%s\n", m_msh->pcs_name.data()); 
        //Write Nodes
	   	fprintf( new_mesh_file, "%s\n", " $NODES");
        fprintf(new_mesh_file,"% ld\n",nNod);
        //write Geometry    
        while (!feof(node_file))
        {
          fgets(input_text,1024,node_file);
          ok= sscanf(input_text,"%i %lg %lg %lg", &id,&x,&y,&z);
          if (ok==4) fprintf(new_mesh_file,"%i %20.14f %20.14f %20.14f\n",id, x,y,z);
        }



        //write Elements
        fprintf( new_mesh_file, "%s\n", " $ELEMENTS");
        fprintf( new_mesh_file, "%i\n", 4*(int)m_msh->ele_vector.size());
        //write Topology
        while (!feof(ele_file))
        {
          fgets(input_text,1024,ele_file);
          ok= sscanf(input_text,"%i %i %i %i %i %i %i %i %i %i", &tri_point0,&tri_point1,&tri_point2,
                                                  &edge0_point0,&edge0_point1,
                                                  &edge1_point0,&edge1_point1,
                                                  &edge2_point0,&edge2_point1,
                                                  &patch_index);
          if (ok==10)
          {
          fprintf(new_mesh_file,"%i %i ",element_counter, patch_index);
          fprintf( new_mesh_file, "%s", " tri  "); 
          fprintf(new_mesh_file,"%i %i %i\n",edge0_point0,edge0_point1,edge2_point1);
          element_counter++;
          fprintf(new_mesh_file,"%i %i ",element_counter, patch_index);
          fprintf( new_mesh_file, "%s", " tri  ");
          fprintf(new_mesh_file,"%i %i %i\n",edge0_point1,edge1_point0,edge1_point1);
          element_counter++;
          fprintf(new_mesh_file,"%i %i ",element_counter, patch_index);
          fprintf( new_mesh_file, "%s", " tri  "); 
          fprintf(new_mesh_file,"%i %i %i\n",edge1_point1,edge2_point0,edge2_point1);
          element_counter++;
          fprintf(new_mesh_file,"%i %i ",element_counter, patch_index);
          fprintf( new_mesh_file, "%s", " tri  ");
          fprintf(new_mesh_file,"%i %i %i\n",edge0_point1,edge1_point1,edge2_point1);
          element_counter++;
          }
        }

        fprintf( new_mesh_file, "%s\n", "#STOP");
        fclose(ele_file);
        fclose(node_file);

        break;
      }

  }

             finish = clock();
             duration = (double)(finish - start) / CLOCKS_PER_SEC;

             hours = (long)duration /3600;
             minutes = (long)duration /60;
             seconds = (long)duration-(hours*3600)-(minutes*60);
             duration_name.Format(_T("%d"),hours);
             _tprintf(_T("%s"), (LPCTSTR) duration_name);
             iteration_name = "New external mesh is ready!  (elapsed time: " + duration_name + "h:";
             duration_name.Format(_T("%d"),minutes);
             _tprintf(_T("%s"), (LPCTSTR) duration_name);
             iteration_name = iteration_name + duration_name + "min:";
             duration_name.Format(_T("%d"),seconds);
             _tprintf(_T("%s"), (LPCTSTR) duration_name);
             iteration_name = iteration_name + duration_name + "sec)";
             pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)iteration_name);

  //======================================================================== 
  fclose(new_mesh_file);
  CString exe_call;
  //exe_call = "notepad.exe " + m_strFileNameEdge;
  //WinExec(exe_call, SW_SHOW);
  exe_call = "notepad.exe " + m_strFileNameNewMSH;
  WinExec(exe_call, SW_SHOW);
  //exe_call = "notepad.exe " + m_strFileNameNodes;
  //WinExec(exe_call, SW_SHOW);
  //exe_call = "notepad.exe " + m_strFileNameElements;
  //WinExec(exe_call, SW_SHOW);
  remove(m_strFileNameEdge);
  remove(m_strFileNameNodes);
  remove(m_strFileNameElements);
  
}

void CGSPropertyRightMesh::OnBnClickedY2z()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0;
  double y,z;
  CFEMesh*m_msh = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      m_msh = fem_msh_vector[i];
      if (m_msh->ele_display_mode == 1 ||
          m_msh->nod_display_mode == 1)
      {
          for (j=0;j<(int)m_msh->nod_vector.size();j++)
          {
              y = m_msh->nod_vector[j]->Y();
              z = m_msh->nod_vector[j]->Z();
              m_msh->nod_vector[j]->SetY(z);
              m_msh->nod_vector[j]->SetZ(y);
          }         
      }
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedX2y()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0;
  double x,y;
  CFEMesh*m_msh = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      m_msh = fem_msh_vector[i];
      if (m_msh->ele_display_mode == 1 ||
          m_msh->nod_display_mode == 1)
      {
          for (j=0;j<(int)m_msh->nod_vector.size();j++)
          {
              x = m_msh->nod_vector[j]->X();
              y = m_msh->nod_vector[j]->Y();
              m_msh->nod_vector[j]->SetX(y);
              m_msh->nod_vector[j]->SetY(x);
          }         
      }
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedX2z()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0;
  double x,z;
  CFEMesh*m_msh = NULL;

  for (i=0;i<(int)fem_msh_vector.size();i++)
  {
      m_msh = fem_msh_vector[i];
      if (m_msh->ele_display_mode == 1 ||
          m_msh->nod_display_mode == 1)
      {
          for (j=0;j<(int)m_msh->nod_vector.size();j++)
          {
              x = m_msh->nod_vector[j]->X();
              z = m_msh->nod_vector[j]->Z();
              m_msh->nod_vector[j]->SetX(z);
              m_msh->nod_vector[j]->SetZ(x);
          }         
      }
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}



void CGSPropertyRightMesh::OnBnClickedCreateEllipsoid()
{
 int i=0, j=0;
 double x_mid=0,y_mid=0,z_mid=0;
 double x_elepsoid_min=0,y_elepsoid_min=0,z_elepsoid_min=0;
 double x_elepsoid_max=0,y_elepsoid_max=0,z_elepsoid_max=0;
 double x_elepsoid_dist=0,y_elepsoid_dist=0,z_elepsoid_dist=0;
 double a=0,b=0,c=0;
 double x=0,y=0,z=0,r=0;
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
      if (fem_msh_vector[j]->ele_display_mode == 1 || fem_msh_vector[j]->nod_display_mode == 1)
      { 
        /*Calculate Max&Midpoints*/ 
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
            if(i==0)
            {
            x_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->X();
            y_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Y();
            z_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Z();
            x_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->X();
            y_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Y();
            z_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Z();
            }
            else
            {
            if (x_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->X())
                x_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->X();
            if (y_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->Y())
                y_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Y();
            if (z_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->Z())
                z_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Z();
            if (x_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->X())
                x_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->X();
            if (y_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->Y())
                y_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Y();
            if (z_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->Z())
                z_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Z();
            }
            x_mid = x_mid + fem_msh_vector[j]->nod_vector[i]->X();
            y_mid = y_mid + fem_msh_vector[j]->nod_vector[i]->Y();
            z_mid = z_mid + fem_msh_vector[j]->nod_vector[i]->Z();
        }     
        x_mid = x_mid/(long)fem_msh_vector[j]->nod_vector.size();
        y_mid = y_mid/(long)fem_msh_vector[j]->nod_vector.size();
        z_mid = z_mid/(long)fem_msh_vector[j]->nod_vector.size();
        x_elepsoid_dist = x_elepsoid_max - x_mid;
        y_elepsoid_dist = y_elepsoid_max - y_mid;
        z_elepsoid_dist = z_elepsoid_max - z_mid;


        /*Select Points inside triangle*/ 
        a = x_elepsoid_dist;
        b = y_elepsoid_dist;
        c = z_elepsoid_dist;
        r = 0.0;
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
          /*Zylinder*/ /*
          x = fem_msh_vector[j]->nod_vector[i]->X()-x_mid;
          y = fem_msh_vector[j]->nod_vector[i]->Y()-y_mid;
          r = ((x*x)/(a*a))+((y*y)/(b*b))+((z*z)/(c*c));
          if (r <= 1) fem_msh_vector[j]->nod_vector[i]->selected = 1;
          */

          x = fem_msh_vector[j]->nod_vector[i]->X()-x_mid;
          y = fem_msh_vector[j]->nod_vector[i]->Y()-y_mid;
          z = fem_msh_vector[j]->nod_vector[i]->Z()-z_mid;
          r = ((x*x)/(a*a))+((y*y)/(b*b))+((z*z)/(c*c));
          if (r <= 1) fem_msh_vector[j]->nod_vector[i]->selected = 1;

        }     

        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            double* xyz;
            xyz = fem_msh_vector[j]->ele_vector[i]->GetGravityCenter();
            x = xyz[0]-x_mid;
            y = xyz[1]-y_mid;
            z = xyz[2]-z_mid;
            r = ((x*x)/(a*a))+((y*y)/(b*b))+((z*z)/(c*c));
            if (r <= 1) fem_msh_vector[j]->ele_vector[i]->selected = 1;
        }     
      }
    }
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedNewMeshfromMATMesh()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  //========================================================================
  int i=0,j=0;
  int k=0;
  int number_meshes=(int)fem_msh_vector.size();
  int mesh_counter =0;
  int pos = 0;
  CFEMesh*m_msh = NULL;
  //CElem* elem = NULL;
  //CNode* node = NULL;

  for (i=0;i<number_meshes;i++)
  {
    for (j=0;j<(int)fem_msh_vector[i]->ele_vector.size();j++)
    {
        if (fem_msh_vector[i]->ele_vector[j]->matgroup_view == 1)
        {
          if (mesh_counter == 0)
          {
          m_msh = new CFEMesh();
          m_msh->pcs_name = "Shape01";
          m_msh->nod_vector = fem_msh_vector[i]->nod_vector;
          m_msh->ele_vector = fem_msh_vector[i]->ele_vector;
          fem_msh_vector.push_back(m_msh);        
          mesh_counter++;
          if(j>0)
          {
          fem_msh_vector[number_meshes]->ele_vector.erase(fem_msh_vector[number_meshes]->ele_vector.begin(),
                                                          fem_msh_vector[number_meshes]->ele_vector.begin()+j);
          pos=j;
          }
          }
        }
        if (fem_msh_vector[i]->ele_vector[j]->matgroup_view != 1 && mesh_counter > 0)
        {
        fem_msh_vector[number_meshes]->ele_vector.erase(fem_msh_vector[number_meshes]->ele_vector.begin()+j-pos);
        pos++;
   
        }
    }

    int nn=0, node_index=0, new_node_index=0;
    int node_counter=0;
    if (mesh_counter > 0)
    {
        for (j=0;j<(int)fem_msh_vector[number_meshes]->ele_vector.size();j++)
        {
          nn = fem_msh_vector[number_meshes]->ele_vector[j]->GetNodesNumber(false);
          for(k=0; k<nn; k++)
          {
            node_index = fem_msh_vector[number_meshes]->ele_vector[j]->GetNodeIndex(k);
            fem_msh_vector[number_meshes]->nod_vector[node_index]->selected = 1;
          }
        }
        for (j=0;j<(int)fem_msh_vector[number_meshes]->nod_vector.size();j++)
        {
         if (fem_msh_vector[number_meshes]->nod_vector[j]->selected != 1)
             fem_msh_vector[number_meshes]->nod_vector[j]->selected = -1;
         else
         {
          fem_msh_vector[number_meshes]->nod_vector[j]->selected = node_counter;
          node_counter++;
         }
        }
        //
        for (j=0;j<(int)fem_msh_vector[number_meshes]->ele_vector.size();j++)
        {
          nn = fem_msh_vector[number_meshes]->ele_vector[j]->GetNodesNumber(false);
          for(k=0; k<nn; k++)
          {
            node_index = fem_msh_vector[number_meshes]->ele_vector[j]->GetNodeIndex(k);
            new_node_index = fem_msh_vector[number_meshes]->nod_vector[node_index]->selected;
            //fem_msh_vector[number_meshes]->ele_vector[j]->SetNodeIndex(k,new_node_index);
            fem_msh_vector[number_meshes]->ele_vector[j]->nodes_index[k]=new_node_index;
          }
        }
        //
        for (j=0;j<(int)fem_msh_vector[number_meshes]->nod_vector.size();j++)
        {
         if (fem_msh_vector[number_meshes]->nod_vector[j]->selected == -1)
         {
             fem_msh_vector[number_meshes]->nod_vector.erase(fem_msh_vector[number_meshes]->nod_vector.begin()+j);
             j--;
         }
        }
        for (j=0;j<(int)fem_msh_vector[number_meshes]->nod_vector.size();j++)
        {
            fem_msh_vector[number_meshes]->nod_vector[j]->SetIndex((long)fem_msh_vector[number_meshes]->nod_vector[j]->selected);
         fem_msh_vector[number_meshes]->nod_vector[j]->selected = 0;
        }


    }
    //TODO: keep the original mesh
      fem_msh_vector.erase(fem_msh_vector.begin()+i);
  }
  
  //========================================================================
 m_frame->m_something_changed = 1;
 m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
 OnPaint();
}
void CGSPropertyRightMesh::OnBnClickedMappingRight()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CPrisGenMap m_mapping;
  m_mapping.m_strFileNameBase = m_pDoc->m_strGSPFileBase;
  m_mapping.DoModal();
}



void CGSPropertyRightMesh::OnBnClickedMshTestDelaunay3DLib()
{
  //========================================================================
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNamePoints = m_strFileNameBase + ".pnt";
  CString m_strFileNameTetMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *d3dgeo_file_name_const_char = 0;
  const char *d3dmsh_file_name_const_char = 0;
  d3dgeo_file_name_const_char = m_strFileNamePoints;
  d3dmsh_file_name_const_char = m_strFileNameTetMSH;
  FILE *d3d_input_file = NULL;
  d3d_input_file = fopen(d3dgeo_file_name_const_char, "w+t"); 
  char input_file [1024];
  char output_file [1024];
  strcpy(input_file,d3dgeo_file_name_const_char);
  strcpy(output_file,d3dmsh_file_name_const_char);
  //========================================================================

  int i=0;
  vector<CGLPoint*> gli_points_vector;
  gli_points_vector = GetPointsVector();
  fprintf( d3d_input_file, "%d\n", (int)gli_points_vector.size());
  for (i=0;i<(long)gli_points_vector.size();i++)  { 	
   fprintf(d3d_input_file,"%20.14f %20.14f %20.14f \n",gli_points_vector[i]->x,  gli_points_vector[i]->y,gli_points_vector[i]->z);
  }
   fclose(d3d_input_file);
   pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Starting 3D Delaunay ");  
   ExecuteDelaunay3D(input_file, output_file);

  FEMRead((string)m_strFileNameBase);
  CompleteMesh(); 

  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);
  OnPaint();
  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");


  remove(d3dgeo_file_name_const_char);
}
void CGSPropertyRightMesh::OnBnClickedMshTestDelaunay2dlib()
{
  //========================================================================
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameTetMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";

  const char *d3dmsh_file_name_const_char = 0;
  d3dmsh_file_name_const_char = m_strFileNameTetMSH;
  char output_file [1024];
  strcpy(output_file,d3dmsh_file_name_const_char);
  //========================================================================

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

  if (ready2mesh == 0) {
	  return;
  }

  CFEMesh*m_msh = NULL;
  m_msh = new CFEMesh();

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Starting 2D Delaunay");  
  int errorCode = ExecuteDelaunay2D(output_file);	/* Meshing */
  if (errorCode != 0) {
	  char buff[128];
	  sprintf(buff, "ERROR 2D Delaunay - ERROR CODE = %d", errorCode);
	  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)buff);  
	  return;
  }

  FEMRead((string)m_strFileNameBase);
  CompleteMesh(); 

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".msh");

  // Update View
  m_frame->m_something_changed = 1;
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);
  OnPaint();

}

void CGSPropertyRightMesh::OnBnClickedCreateEllipse2d()
{
 int i=0, j=0;
 double x_mid=0,y_mid=0,z_mid=0;
 double x_elepsoid_min=0,y_elepsoid_min=0,z_elepsoid_min=0;
 double x_elepsoid_max=0,y_elepsoid_max=0,z_elepsoid_max=0;
 double x_elepsoid_dist=0,y_elepsoid_dist=0,z_elepsoid_dist=0;
 double a=0,b=0,c=0;
 double x=0,y=0,z=0,r=0;
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
      if (fem_msh_vector[j]->ele_display_mode == 1 || fem_msh_vector[j]->nod_display_mode == 1)
      { 
        /*Calculate Max&Midpoints*/ 
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
            if(i==0)
            {
            x_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->X();
            y_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Y();
            z_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Z();
            x_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->X();
            y_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Y();
            z_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Z();
            }
            else
            {
            if (x_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->X())
                x_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->X();
            if (y_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->Y())
                y_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Y();
            if (z_elepsoid_min > fem_msh_vector[j]->nod_vector[i]->Z())
                z_elepsoid_min = fem_msh_vector[j]->nod_vector[i]->Z();
            if (x_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->X())
                x_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->X();
            if (y_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->Y())
                y_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Y();
            if (z_elepsoid_max < fem_msh_vector[j]->nod_vector[i]->Z())
                z_elepsoid_max = fem_msh_vector[j]->nod_vector[i]->Z();
            }
            x_mid = x_mid + fem_msh_vector[j]->nod_vector[i]->X();
            y_mid = y_mid + fem_msh_vector[j]->nod_vector[i]->Y();
            z_mid = z_mid + fem_msh_vector[j]->nod_vector[i]->Z();
        }     
        x_mid = x_mid/(long)fem_msh_vector[j]->nod_vector.size();
        y_mid = y_mid/(long)fem_msh_vector[j]->nod_vector.size();
        z_mid = z_mid/(long)fem_msh_vector[j]->nod_vector.size();
        x_elepsoid_dist = x_elepsoid_max - x_mid;
        y_elepsoid_dist = y_elepsoid_max - y_mid;
        z_elepsoid_dist = z_elepsoid_max - z_mid;


        /*Select Points inside triangle*/ 
        a = x_elepsoid_dist;
        b = y_elepsoid_dist;
        c = z_elepsoid_dist;
        r = 0.0;
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
          x = fem_msh_vector[j]->nod_vector[i]->X()-x_mid;
          y = fem_msh_vector[j]->nod_vector[i]->Y()-y_mid;
          r = ((x*x)/(a*a))+((y*y)/(b*b));
          if (r <= 1) fem_msh_vector[j]->nod_vector[i]->selected = 1;

        }     

        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            double* xyz;
            xyz = fem_msh_vector[j]->ele_vector[i]->GetGravityCenter();
            x = xyz[0]-x_mid;
            y = xyz[1]-y_mid;
            z = xyz[2]-z_mid;
            r = ((x*x)/(a*a))+((y*y)/(b*b));
            if (r <= 1) fem_msh_vector[j]->ele_vector[i]->selected = 1;
        }     
      }
    }
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSPropertyRightMesh::OnBnClickedMshnodesAlongPolyline()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Nodes:");
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase + "_temp";
  CString m_strFileNameGLI = m_strFileNameBase + ".gli";
  CString m_strFileNameGEO = m_strFileNameBase + ".geo";
  CString m_strFileNameTIN = m_strFileNameBase + ".tin";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameBase; 
  int i=0, j=0;
  double checkpoint[3];
  double tolerance = 0.001;
  double min_mesh_dist = 0.0;
  int tri_node[3];
  double tri_ele_coor[3][3]; 

  vector<CGLPolyline*>::iterator p = polyline_vector.begin();  CGLPolyline *polyline;
  for (i=0;i<(int)(int)polyline_vector.size();i++)
  {
	  polyline = *p;
      if (polyline->display_mode == 1)
      {
       /*Polyline Search*/ 
       pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching Nodes:.......please wait");


 //Loop over all meshes
    for(j=0;j<(long)fem_msh_vector.size()-1;j++)
    {
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
    //Loop over all mesh nodes
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
            checkpoint[0] = fem_msh_vector[j]->nod_vector[i]->X();
            checkpoint[1] = fem_msh_vector[j]->nod_vector[i]->Y(); 
            checkpoint[2] = fem_msh_vector[j]->nod_vector[i]->Z();
        }

    //Loop over all mesh elements
		for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
          if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 4) /*TRI*/ 
          {
		   tri_node[0] = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(0);
		   tri_node[1] = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(1);
		   tri_node[2] = fem_msh_vector[j]->ele_vector[i]->GetNodeIndex(2);
           tri_ele_coor[0][0] = fem_msh_vector[j]->nod_vector[tri_node[0]]->X();
		   tri_ele_coor[0][1] = fem_msh_vector[j]->nod_vector[tri_node[0]]->Y();
		   tri_ele_coor[0][2] = fem_msh_vector[j]->nod_vector[tri_node[0]]->Z();
           tri_ele_coor[1][0] = fem_msh_vector[j]->nod_vector[tri_node[1]]->X();
		   tri_ele_coor[1][1] = fem_msh_vector[j]->nod_vector[tri_node[1]]->Y();
		   tri_ele_coor[1][2] = fem_msh_vector[j]->nod_vector[tri_node[1]]->Z();
           tri_ele_coor[2][0] = fem_msh_vector[j]->nod_vector[tri_node[2]]->X();
		   tri_ele_coor[2][1] = fem_msh_vector[j]->nod_vector[tri_node[2]]->Y();
		   tri_ele_coor[2][2] = fem_msh_vector[j]->nod_vector[tri_node[2]]->Z();

		   /*Calculate Intersection Points*/ 
		   polyline->point_vector.size();


		  }
		}

    }




	   break;
      }

  }
       /*Node and Element Selection*/ 


 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");

 m_frame->dist_geo_object = 0.001;
 UpdateData(TRUE);
 m_frame->m_something_changed = 1;
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

/**************************************************************************
Task:   Create line elements along the polylines using existing edges
Programing:
07/2007 NW implementation
**************************************************************************/
void CGSPropertyRightMesh::OnBnClickedCreateLinesAlongPolyline()
{
  //========================================================================
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameTetMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  //========================================================================

  if (fem_msh_vector.size() == 0) {
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"BASE MESH NOT FOUND.");  
    return;
  }

  //========================================================================
  //Make a list of the user-specified polylines 
  vector<CGLPolyline*> vct_target_polylines;
  for (int i=0;i<(int)polyline_vector.size();i++)
  {
    if (polyline_vector[i]->display_mode == 1)
    {
      vct_target_polylines.push_back(polyline_vector[i]);
    }
  }
  if (vct_target_polylines.size() == 0) {
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"CHECKED POLYLINE NOT FOUND.");  
    return;
  }

  //========================================================================
  //Create Line Elements
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Creating Line Elements");  
  ////DEBUG CODE: Remove extra mesh
  //while (fem_msh_vector.size() > 1) {
  //  fem_msh_vector.pop_back();
  //}
  //// DEBUG CODE: unhighlight all elements
  //for(int i=0;i<(long)fem_msh_vector[0]->ele_vector.size();i++) {
  //  fem_msh_vector[0]->ele_vector[i]->selected = 0;
  //}

  CFEMesh* m_msh_ply = new CFEMesh();
  m_msh_ply->pcs_name = "POLYLINES";

  CGLPolyline* m_ply = NULL;
  for (int i=0;i<(int)vct_target_polylines.size();i++)
  {
    m_ply = vct_target_polylines[i];
    fem_msh_vector[0]->CreateLineELEFromPLY(m_ply, 4, m_msh_ply);
  }
  if (m_msh_ply->ele_vector.size() > 0) {
    fem_msh_vector.push_back(m_msh_ply);
  } else {
    delete m_msh_ply;
  }

  //========================================================================
  //MSHWrite((string)m_strFileNameBase);
  //FEMRead((string)m_strFileNameBase);
  //CompleteMesh(); 

  // Update View
  m_frame->m_something_changed = 1;
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);
  OnPaint();
}
