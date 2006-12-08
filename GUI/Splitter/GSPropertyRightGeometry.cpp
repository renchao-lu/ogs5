// GSPropertyRightGeometry.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "GSPropertyRightGeometry.h"
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
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"


// CGSPropertyRightGeometry dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightGeometry, CViewPropertyPage)
CGSPropertyRightGeometry::CGSPropertyRightGeometry()
	: CViewPropertyPage(CGSPropertyRightGeometry::IDD)
{
    m_tolerancefactor=0.000;
    m_polyline_min_seg_length = 0.000;
    m_polyline_max_seg_length = 1.000;
    m_polyline_smaller_seg_length_def = 0.000;

}

CGSPropertyRightGeometry::~CGSPropertyRightGeometry()
{
}

void CGSPropertyRightGeometry::DoDataExchange(CDataExchange* pDX)
{
	CViewPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGSPropertyRightGeometry)
    DDX_Control(pDX, IDC_LIST_MESH, m_List);
    DDX_Text(pDX,IDC_EDIT_TOLERANCE, m_tolerancefactor);
    DDX_Text(pDX, IDC_EDIT_MIN_POLYLENGTH, m_polyline_min_seg_length);
    DDX_Text(pDX, IDC_EDIT_MAX_POLYLENGTH, m_polyline_max_seg_length);
    DDX_Text(pDX, IDC_EDIT_SMALER_POLYLENGTH_DEF, m_polyline_smaller_seg_length_def);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGSPropertyRightGeometry, CViewPropertyPage)
    ON_BN_CLICKED(IDC_CHECK_DOUBLE_POINTS, OnBnClickedCheckDoublePoints)
    ON_BN_CLICKED(IDC_DELETE_DOUBLE_POINTS_BUTTON, OnBnClickedDeleteDoublePointsButton)
    ON_BN_CLICKED(IDC_ADD_POLYGON_BUTTON, OnBnClickedAddPolygonButton)
    ON_BN_CLICKED(IDC_ADD_NEXT_POLYGON_BUTTON, OnBnClickedAddNextPolygonButton)
    ON_BN_CLICKED(IDC_GET_MINMAXSEGLENGTH_BUTTON, OnBnClickedGetMinmaxseglengthButton)
    ON_BN_CLICKED(IDC_SET_MINMAXSEGLENGTH_BUTTON, OnBnClickedSetMinmaxseglengthButton)
    ON_BN_CLICKED(IDC_RELOAD_GEO_BUTTON, OnBnClickedReloadGeoButton)
    ON_BN_CLICKED(IDC_GET_SEGLENGTH_SMALLER_DEF, OnBnClickedGetSeglengthSmallerDef)
	ON_BN_CLICKED(IDC_GEO_INFO, OnBnClickedGeoInfo)
END_MESSAGE_MAP()


// CGSPropertyRightGeometry message handlers

void CGSPropertyRightGeometry::OnDataChange()
{
    	if (!UpdateData())
		return;

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();    
    mainframe->m_3dcontrol_double_points = m_3dcontrol_double_points;
    mainframe->m_tolerancefactor = m_tolerancefactor;
    mainframe->m_something_changed = 1;
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    m_pDoc->UpdateAllViews(NULL);
}

void CGSPropertyRightGeometry::OnBnClickedCheckDoublePoints()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  if(double_points_button_check_on==1)double_points_button_check_on=0;
  else double_points_button_check_on=1;

  if(double_points_button_check_on==1)
  {
    UpdateData(TRUE);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing point numbers");
    // TODO: GEO_Serialize_Point_Numbers();
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching double points");
    GEO_Search_DoublePoints(m_tolerancefactor);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");
    m_3dcontrol_double_points = 1;
  }
  else
  {
    m_3dcontrol_double_points = 0;
  }
  GEOSurfaceTopology();
  GEOCreateSurfacePointVector(); //OK
  OnDataChange();
}


void CGSPropertyRightGeometry::OnBnClickedDeleteDoublePointsButton()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  UpdateData(TRUE);

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing point numbers");
  GEO_Serialize_Point_Numbers();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching double points");
  GEO_Search_DoublePoints(m_tolerancefactor);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Deleting double points");
  GEO_Delete_DoublePoints();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");

  //writing and reading of a temp file to ensure gloabal loading and updating
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  string m_strFileNameGEO_temp = m_strFileNameBase + "_temp";
  GEOWrite(m_strFileNameGEO_temp);
  GEOLIB_Read_GeoLib(m_strFileNameGEO_temp);
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO_temp.data();
  remove(file_name_const_char);
 
  OnDataChange();
  m_frame->m_rebuild_formtree = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

}

void CGSPropertyRightGeometry::OnBnClickedAddPolygonButton()
{
  //========================================================================
  //File handling
  //========================================================================
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  m_pDoc->OnRemoveGEO();
  m_pDoc->OnRemoveMSH();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  CString m_strFileNameGMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  //========================================================================

  const char *gli_file_name_const_char = 0;
  const char *rfi_file_name_const_char = 0;
  gli_file_name_const_char = m_strFileNameGEO;
  rfi_file_name_const_char = m_strFileNameRFI;

  FILE *plg_file_orig=NULL;
  FILE *geo_file=NULL;
  geo_file = fopen(gli_file_name_const_char, "w+t");
  char input_text[1024];
  int ok=1;
  int i=0;
  long id=0;
  double x=0.0,y=0.0,z=0.0;

  CFileDialog fileDlg(TRUE, "plg", NULL, OFN_ENABLESIZING," POLYGON File (*.plg)|*.plg|| ");
  if(fileDlg.DoModal()==IDOK){

    plg_file_orig = fopen(fileDlg.GetPathName(),"rt"); 

    if (fileDlg.GetFileExt() == "plg")
    {
      fgets(input_text,1024,plg_file_orig);

      //FORMAT: 
      //POINTS 
      //ID X Y Z
      //POLYGONS
      //ID point_numbers

      if (!strncmp(input_text, "POINTS",6))
      {
       fprintf(geo_file,"%s\n","#POINTS");
       while (!feof(plg_file_orig)){
           fgets(input_text,1024,plg_file_orig);
           if (!strncmp(input_text, "PLG",3))break;
           else
           {
                sscanf(input_text,"%i %lg %lg %lg", &id, &x, &y, &z);
                fprintf(geo_file,"%i %lg %lg %lg\n",id, x, y, z);
           }
           //ok = sscanf(input_text,"%i %lg %lg %lg", &id, &x, &y, &z);
           //fprintf(geo_file,"%s", input_text);
           
       }
       

        rewind (plg_file_orig);
        int plg_start=0;
        int first_hit=0;
        int first_point=0;
        char prename[1024];

        while (!feof(plg_file_orig)){
            fgets(input_text,1024,plg_file_orig);
        if (!strncmp(input_text, "PLG",3))
        {
            if (first_hit > 0)
            {
             fprintf(geo_file," %i\n",first_point);

                /*SFC*/ 
                fprintf(geo_file,"%s\n","#SURFACE");
                fprintf(geo_file,"%s\n"," $NAME");
                fprintf(geo_file,"   %s\n",prename);
                fprintf(geo_file,"%s\n"," $TYPE 0");
                fprintf(geo_file,"%s\n","  0");
                fprintf(geo_file,"%s\n"," $POLYLINES");
                fprintf(geo_file,"   %s\n",prename);

            }

            plg_start++;
            first_hit=-1;
            first_point=-1;
            strcpy(prename,input_text);
            fprintf(geo_file,"%s\n","#POLYLINE");
            fprintf(geo_file,"%s\n"," $NAME");
            fprintf(geo_file,"%s\n", input_text);
            fprintf(geo_file,"%s\n"," $TYPE 0");
            fprintf(geo_file,"%s\n","  0");
            fprintf(geo_file,"%s\n"," $EPSILON");
            fprintf(geo_file,"%s\n","  0.000000e+000");
            fprintf(geo_file,"%s\n"," $MAT_GROUP");
            fprintf(geo_file,"%s\n","  -1");
            fprintf(geo_file,"%s\n"," $POINTS");
        }
        if (plg_start>0 && strncmp(input_text, "PLG",3))
        {
             ok = sscanf(input_text,"%i", &id);
             fprintf(geo_file," %i\n",id);
             if (first_hit == -1) first_point = id;
             first_hit++;
        }
       }   

      
        fprintf(geo_file,"%s\n","#STOP");
      }


      //FORMAT: x,y,z in order
      else
      {
      rewind (plg_file_orig);
      
      if(plg_file_orig)
      {

       fprintf(geo_file,"%s\n","#POINTS");

       while (!feof(plg_file_orig)&& ok>0){

           id++;
           fgets(input_text,1024,plg_file_orig);
           ok = sscanf(input_text,"%lg %lg %lg", &x, &y, &z);
           fprintf(geo_file,"%i %lg %lg %lg\n",id, x, y, z);
       }
       
       /*fprintf(geo_file,"%s\n","#LINES");
       for (i=0;i<id-1;i++)
       {
          fprintf(geo_file,"%i %i %i\n",i+1, i+1, i+2);
       }
          fprintf(geo_file,"%i %i %i\n",i+1, i+1, i-i+1);
       */
   
       fprintf(geo_file,"%s\n","#POLYLINE");
       fprintf(geo_file,"%s\n"," $NAME");
       fprintf(geo_file,"%s\n","  Polygon_01");
       fprintf(geo_file,"%s\n"," $TYPE 0");
       fprintf(geo_file,"%s\n","  0");
       fprintf(geo_file,"%s\n"," $EPSILON");
       fprintf(geo_file,"%s\n","  0.000000e+000");
       fprintf(geo_file,"%s\n"," $MAT_GROUP");
       fprintf(geo_file,"%s\n","  -1");
       fprintf(geo_file,"%s\n"," $POINTS");

       for (i=0;i<id;i++)
       {
          fprintf(geo_file," %i\n",i+1);
       }
          fprintf(geo_file," %i\n",i-i+1);

        fprintf(geo_file,"%s\n","#SURFACE");
        fprintf(geo_file,"%s\n"," $NAME");
        fprintf(geo_file,"%s\n","  Polygon_01");
        fprintf(geo_file,"%s\n"," $TYPE 0");
        fprintf(geo_file,"%s\n","  0");
        fprintf(geo_file,"%s\n"," $POLYLINES");
        fprintf(geo_file,"%s\n","  Polygon_01");
        fprintf(geo_file,"%s\n","#STOP");
        
      }
    }
    }
  fclose(geo_file);
  GEOLIB_Read_GeoLib((string)m_strFileNameBase);

  OnDataChange();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".gli");
  

  }
}
void CGSPropertyRightGeometry::OnBnClickedAddNextPolygonButton()
{
  /*FILE *plg_file_orig=NULL;
  char input_text[1024];
  long id=0;
  double x=0.0,y=0.0,z=0.0;
  long pointsvectorsize,polylinesvectorsize,surfacesvectorsize;

  CGLPoint *m_point = NULL;
  CGLPoint *m_polyline_start_point = NULL;
  vector<CGLPoint*> gli_points_vector;
  gli_points_vector = GEOLIB_GetGLIPoints_Vector();
  pointsvectorsize =(long)gli_points_vector.size();
 
  CGLPolyline *gl_polyline = NULL;
  list<CGLPolyline*> polyline_list;
  polyline_list = gl_polyline->GEOGetPolylines();
  polylinesvectorsize =(long)polyline_list.size();
  //list<CGLPolyline*>::const_iterator p = polyline_list.begin();
  sprintf(input_text,"%s%d","Polygon_",polylinesvectorsize+1);
  //gl_polyline = gl_polyline->GEOCreatePolyline(input_text);
  //gl_polyline->number_this = polylinesvectorsize+1;
  //gs_polyline = gs_polyline->Create(c_string_ply);//CC
  gs_polyline = new CGLPolyline;
  gs_polyline->name = c_string_ply;
  polyline_vector.push_back(gs_polyline);
  gs_polyline->id = size;

  Surface *gl_surface = NULL;
  list<Surface*> surface_list;
  surface_list = gl_surface->GEOGetSurfaces();
  surfacesvectorsize = (long)surface_list.size();
  //list<Surface*>::const_iterator ps = surface_list.begin();
  sprintf(input_text,"%s%d","Polygon_",surfacesvectorsize+1);
  gl_surface = gl_surface->GEOCreateSurface(input_text);
  gl_surface->number_this = surfacesvectorsize+1;

  CFileDialog fileDlg(TRUE, "plg", NULL, OFN_ENABLESIZING," POLYGON File (*.plg)|*.plg|| ");
  if(fileDlg.DoModal()==IDOK){
    if (fileDlg.GetFileExt() == "plg")
    {
      plg_file_orig = fopen(fileDlg.GetPathName(),"rt"); 
      if(plg_file_orig)
      {
       while (!feof(plg_file_orig)){

           id++;
           fgets(input_text,1024,plg_file_orig);
           sscanf(input_text,"%lg %lg %lg", &x, &y, &z);

           m_point = new CGLPoint;
           m_point->id = pointsvectorsize+id;
           m_point->x = x;
           m_point->y = y;
           m_point->z = z;           
           gli_points_vector.push_back(m_point);

           gl_polyline->point_list.push_back(m_point);
           gl_polyline->point_vector.push_back(m_point);
          
           if (id==1)m_polyline_start_point = m_point;
       } 
           gl_polyline->point_list.push_back(m_polyline_start_point);
           gl_polyline->point_vector.push_back(m_polyline_start_point);
           gl_surface->polyline_of_surface_list.push_back(gl_polyline);
      }
    }

  GEOLIB_SetGLIPoints_Vector(gli_points_vector);

  OnDataChange();
  CGeoSysDoc *m_pDoc = GetDocument();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

  }*/
}

void CGSPropertyRightGeometry::OnBnClickedGetMinmaxseglengthButton()
{
  m_polyline_min_seg_length = GEO_Get_Min_PolySeg_length();
  m_polyline_max_seg_length = GEO_Get_Max_PolySeg_length();
  UpdateData(FALSE);
}

void CGSPropertyRightGeometry::OnBnClickedSetMinmaxseglengthButton()
{
  //CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //CGeoSysDoc *m_pDoc = GetDocument();
  

  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing point numbers");
  GEO_Serialize_Point_Numbers();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");
 
  UpdateData(TRUE);

  if (m_polyline_max_seg_length-m_polyline_min_seg_length > 0.0)
  {
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Setting segment length");
  GEO_Set_Poly_Seg_Length(m_polyline_min_seg_length,m_polyline_max_seg_length);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Serializing");
  GEO_Serialize_Point_Numbers();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Searching double points");
  GEO_Search_DoublePoints(0);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Merging");
  GEO_Delete_DoublePoints();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");
  }
  else AfxMessageBox("Segment length < 0.000 !!! ");

  m_polyline_min_seg_length = GEO_Get_Min_PolySeg_length();
  m_polyline_max_seg_length = GEO_Get_Max_PolySeg_length();
  UpdateData(FALSE);

  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  GSPWriteData();
  GEOLIB_Read_GeoLib((string)m_strFileNameBase);

  OnDataChange();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);

  
  
}

void CGSPropertyRightGeometry::OnBnClickedReloadGeoButton()
{
  CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
  CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
  CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
  CString m_strFileNameBase = m_pDoc->m_strGSPFilePathBase;
  CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CString m_strFileNameGEO = m_strFileNameBase + ".gli";
  //GSPWriteData();
  GEOLIB_Read_GeoLib((string)m_strFileNameBase);
  m_polyline_min_seg_length = GEO_Get_Min_PolySeg_length();
  m_polyline_max_seg_length = GEO_Get_Max_PolySeg_length();
  UpdateData(FALSE);
  OnDataChange();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;
  m_pDoc->UpdateAllViews(NULL);
  Invalidate(TRUE);
}

void CGSPropertyRightGeometry::OnBnClickedGetSeglengthSmallerDef()
{
     UpdateData(TRUE);

	int j=0, l=0;
	long pointsvectorsize, polylinesvectorsize;
	long number_of_polylinepoints;
	long check_point;
    double x1=0.0,y1=0.0,z1=0.0,seg_length=0.0;
    double x2=0.0,y2=0.0,z2=0.0;
	vector<CGLPoint*> gli_points_vector;
	gli_points_vector = GetPointsVector();
    pointsvectorsize =(long)gli_points_vector.size();
	CGLPolyline *gl_polyline = NULL;
    vector<CGLPolyline*> polyline_vector;
	polyline_vector = GetPolylineVector();
	polylinesvectorsize =(long)polyline_vector.size();
	vector<CGLPolyline*>::const_iterator p = polyline_vector.begin();

	string Name;
    
   	for (l=0;l<polylinesvectorsize;l++)
    { 	
		gl_polyline = *p;
		Name = gl_polyline->name;
        gl_polyline->min_plg_Dis = m_polyline_smaller_seg_length_def;
		number_of_polylinepoints = (long)gl_polyline->point_vector.size();
		for (j=0;j<number_of_polylinepoints;j++)
		{ 
          gl_polyline->point_vector[j]->plg_hightlight_seg = 0;
        }           
        for (j=0;j<number_of_polylinepoints-1;j++)
		{ 
            check_point = gl_polyline->point_vector[j]->id;
            x1 = gl_polyline->point_vector[j]->x;
            y1 = gl_polyline->point_vector[j]->y;
            z1 = gl_polyline->point_vector[j]->z;

            check_point = gl_polyline->point_vector[j+1]->id;
            x2 = gl_polyline->point_vector[j+1]->x;
            y2 = gl_polyline->point_vector[j+1]->y;
            z2 = gl_polyline->point_vector[j+1]->z;

            seg_length = EuklVek3dDistCoor ( x1, y1, z1, x2, y2, z2 ); 

            if (seg_length <  m_polyline_smaller_seg_length_def)
            {
                gl_polyline->point_vector[j]->plg_hightlight_seg = 1;    
                gl_polyline->point_vector[j+1]->plg_hightlight_seg = 1;
            }
		}
		++p;
	}
 CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

void CGSPropertyRightGeometry::OnBnClickedGeoInfo()
{
  int i=0;
  string info_string;
  char numberstring [56];
  int openPLY=0, closedPLY=0;

  m_List.ResetContent();
  m_List.AddString(_T(">***********************************"));
  if ((int)gli_points_vector.size() > 0)
  {
   m_List.AddString(_T(">GEO data loaded"));
   m_List.AddString(_T(">***********************************"));
   /*Handle Point Information here:*/ 
   info_string = ">Number of Points: " ;
   sprintf(numberstring, "%i",(int)gli_points_vector.size());
   info_string.append(numberstring);
   m_List.AddString(_T(info_string.data()));
   /*Handle Polyline Information here:*/ 
   info_string = ">Number of Polylines: " ;
   sprintf(numberstring, "%i",(int)polyline_vector.size());
   info_string.append(numberstring);
   m_List.AddString(_T(info_string.data()));

   for (i=0;i<(int)polyline_vector.size();i++)
   { 
	   if (polyline_vector[i]->point_vector[0] == polyline_vector[i]->point_vector[polyline_vector[i]->point_vector.size()-1])
       closedPLY++;
       else
	   openPLY++;
   }
   info_string = ">  open | closed: " ;
   sprintf(numberstring, "%i",openPLY);
   info_string.append(numberstring);
   info_string.append(" | ");
   sprintf(numberstring, "%i",closedPLY++);
   info_string.append(numberstring);
   m_List.AddString(_T(info_string.data()));
   /*Handle Surface Information here:*/ 
   info_string = ">Number of Surfaces: " ;
   sprintf(numberstring, "%i",(int)surface_vector.size());
   info_string.append(numberstring);
   m_List.AddString(_T(info_string.data()));
   /*Handle Volume Information here:*/ 

  }


  /*End of GEO DATA handle*/ 
  else 
  {
  m_List.AddString(_T(">GEO data not loaded"));
  m_List.AddString(_T(">***********************************"));
  }

  m_List.AddString(_T(">"));
  m_List.AddString(_T(">***********Info End***************"));

 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);

}


