// OGLView.cpp : OpenGL-View nur zusammen mit OGLEnabledView.cppp
//
#include "stdafx.h"
#include "math.h"
#include "OGLControl.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "OGLEnabledView.h"
#include "OGLView.h"
#include "OGLControl.h"
#include "OGLInfoDlg.h"
#include <glut.h>
#include "MainFrm.h"
#include "afxpriv.h"
#include <process.h>
#include "gs_project.h"
//DLG
#include "gs_tolerance.h"
#include "gs_ogl_tetra_view.h"
#include "gs_mesh_imp.h"
#include "GeoSysListView.h"
#include "GSForm3DLeft.h"
#include "gs_graphics.h"
#include "GSForm3DLeft.h"
#include "gs_structured_mesh.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_nodes_rfi.h"
#include "msh_elements_rfi.h"
#include "fem_ele.h"
#include "msh_lib.h"
#include "msh_quality.h"
#include "tricommon.h"
#include "msh_mesh.h"
using namespace Mesh_Group;

//FEM
#include "elements.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long view_counter=0;
long PointOnOff =0;
long PointNumberOnOff =0;
long DoublePointOnOff =0;
long NodeNumberOnOff =0;
long ElementNumberOnOff =0;
long PolylineNameOnOff =0;

/*----------------------------------------------------------------------*/
// Vectors for view
vector<CViewPoints*> view_points_vector;
CViewPoints *m_view_points = NULL;

vector<CViewLines*> view_lines_vector;
CViewLines *m_view_lines = NULL;

vector<CViewPolylines*> view_polylines_vector;
CViewPolylines *m_view_polylines = NULL;
CGLPolyline *ogl_polyline = NULL;
CGLPoint *m_polyline_points = NULL;
vector<CGLPolyline*>::iterator p = NULL;

vector<CViewSurfaces*> view_surfaces_vector;
CViewSurfaces *m_view_surfaces = NULL;

vector<CViewNodes*> view_nodes_vector;
CViewNodes *m_view_nodes = NULL;

vector<CViewElements*> view_elements_vector;
CViewElements *m_view_elements = NULL;

//OK vector<CMSHElements*>msh_elements_vector;

CPoint m_ptLast = CPoint(0,0);

//CVS TEST

/*----------------------------------------------------------------------*/
// CALLBACKS for nonconvex surface glu_view
#ifndef CALLBACK
#define CALLBACK
#endif

GLuint startList;

void CALLBACK beginCallback(GLenum which)
{
   glBegin(which);
}
void CALLBACK endCallback(void)
{
   glEnd();
}
void CALLBACK vertexCallback(GLvoid *vertex)
{
   const GLdouble *pointer;

   pointer = (GLdouble *) vertex;
   glColor3dv(pointer+3);
   glVertex3dv(pointer);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView
IMPLEMENT_DYNCREATE(COGLView, COGLEnabledView)

BEGIN_MESSAGE_MAP(COGLView, COGLEnabledView)
	//{{AFX_MSG_MAP(CGeoSysView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_HELP_OGLINFO, OnHelpOglinfo)
	ON_COMMAND(ID_ENVIR_FLAT, OnEnvirFlat)
	ON_COMMAND(ID_SHADING_COLOREDELEVATION, OnColoredElevation)
	ON_COMMAND(ID_ENVIR_LIGHTING, OnEnvirLighting)
	ON_COMMAND(ID_ENVIR_SMOOTH, OnEnvirSmooth)
    ON_COMMAND(ID_EXAMPLES_STARTEXAMPLES,OnStartOpenGLExamples )
    ON_COMMAND(ID_3DGLIVIEW_POINTS,On3DViewPoints )
    ON_COMMAND(ID_3DGLIVIEW_ADDPOINTS,On3DViewAddPoints )
	ON_COMMAND(ID_3DGLIVIEW_LINES,On3DViewLines )
	ON_COMMAND(ID_3DGLIVIEW_POLYLINES,On3DViewPolylines )
	ON_COMMAND(ID_3DGLIVIEW_SURFACES,On3DViewSurfaces )
	ON_COMMAND(ID_3DGLIVIEW_VOLUMES,On3DViewVolumes )
	ON_COMMAND(ID_3DGLIVIEW_POINTNUMBERS,On3DViewPointNumbers )
	ON_COMMAND(ID_3DGLIVIEW_DOUBLEPOINTS,On3DViewDoublePoints )    
	ON_COMMAND(ID_3DGLIVIEW_POLYLINENAMES,On3DViewPolylineNames)
	ON_COMMAND(ID_RFIVIEW_NODENUMBERS,On3DViewRFINodeNumbers )
	ON_COMMAND(ID_RFIVIEW_ELEMENTNUMBERS,On3DViewRFIElementNumbers )
	ON_COMMAND(ID_3DRFIVIEW_NODES,On3DViewRFINodes)
	ON_COMMAND(ID_3DRFIVIEW_ELEMENTS,On3DViewRFIElements)
	ON_COMMAND(ID_3DFEMVIEW_MODEL,On3DViewFEMModel)
    ON_UPDATE_COMMAND_UI(ID_3DGLIVIEW_POINTS, OnUpdate3DViewPoints )
	ON_UPDATE_COMMAND_UI(ID_3DGLIVIEW_POINTNUMBERS, OnUpdate3DViewPointNumbers )
    ON_UPDATE_COMMAND_UI(ID_3DGLIVIEW_DOUBLEPOINTS, OnUpdate3DViewDoublePoints )
    ON_UPDATE_COMMAND_UI(ID_3DGLIVIEW_POLYLINENAMES, OnUpdate3DViewPolylineNames )
	ON_UPDATE_COMMAND_UI(ID_RFIVIEW_NODENUMBERS,OnUpdate3DViewRFINodeNumbers)
	ON_UPDATE_COMMAND_UI(ID_RFIVIEW_ELEMENTNUMBERS,OnUpdate3DViewRFIElementNumbers)    
	ON_UPDATE_COMMAND_UI(ID_ENVIR_LIGHTING, OnUpdateEnvirLighting)
  	ON_UPDATE_COMMAND_UI(ID_SHADING_COLOREDELEVATION, OnUpdateColoredElevation)
	ON_UPDATE_COMMAND_UI(ID_ENVIR_FLAT, OnUpdateEnvirFlat)
	ON_UPDATE_COMMAND_UI(ID_ENVIR_SMOOTH, OnUpdateEnvirSmooth)
	ON_UPDATE_COMMAND_UI(ID_3DGLIVIEW_POLYLINENAMES, OnUpdate3DViewPolylineNames )	
	ON_COMMAND(IDC_BACK_COLOUR, OnBackColour)
	ON_COMMAND(ID_MESHTEST_AREA, OnMeshTest_Area)
	ON_COMMAND(ID_MESHTEST_AREA_GRAPH, OnMeshTest_Area_Graph)
	ON_COMMAND(ID_MESHTEST_ANGLE_GRAPH, OnMeshTest_Angle_Graph)
	ON_COMMAND(ID_MESHTEST_ANGLE, OnMeshTest_Angle)
	ON_COMMAND(ID_MESHTEST_LENGTH, OnMeshTest_Length)
	ON_COMMAND(ID_MESHTEST_LENGTH_GRAPH, OnMeshTest_Length_Graph)
	ON_COMMAND(ID_MESHTEST_TETRA_VOLUME, OnMeshTest_Tetra_Volume)
	ON_COMMAND(ID_MESHTEST_TETRA_VOLUME_GRAPH, OnMeshTest_Tetra_Volume_Graph)
	ON_COMMAND(ID_MESHTEST_TETRA_ANGLE, OnMeshTest_Tetra_Angle)
	ON_COMMAND(ID_MESHTEST_TETRA_TETRA_GRAPH, OnMeshTest_Tetra_Angle_Graph)
	ON_COMMAND(ID_MESHGENERATOR_GMSH_TRIANGULATION, OnMeshGenerator_GMSH_Tri_Mesh)
    ON_COMMAND(ID_MESHGENERATOR_DELAUNAY, OnMeshGenerator_Delaunay)   
	ON_COMMAND(ID_MSHLIB_MESHIMPROVEMENT,OnMeshImprovement)
    ON_COMMAND(ID_MESHGENERATOR_STRUCTURED2D,OnMeshStructured2d)

	ON_WM_KEYDOWN()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView construction/destruction

COGLView::COGLView():
sceneselect(0),
trackball((real).8,unitquaternion(DegToRad(45),Y_AXIS)*unitquaternion(DegToRad(-30),X_AXIS))
{
	trackball.SetColor(RGB(130,80,30));
	trackball.SetDrawConstraints();
    m_iDisplayMSHTri=0;
    m_iDisplayMSHLine = 0;
    m_bDisplayMSHPris = false;
	m_bDisplayMSHTet = false;
	m_bDisplayMSHQuad = false;
	m_bDisplayMSHHex = false;
    m_iDisplayIsosurfaces = 0;
    m_iDisplayIsolines = 0;
    m_iDisplayBC = 0;
    
}

COGLView::~COGLView()
{
	view_points_vector.clear();
	view_lines_vector.clear();
	view_polylines_vector.clear();
    view_nodes_vector.clear();
    view_elements_vector.clear();

	 InitializeOGLViewDataConstructs();

}

void COGLView::DoDataExchange(CDataExchange* pDX)
{
 	CView::DoDataExchange(pDX);
    On3DControl();
}

void COGLView:: On3DControl()
{   
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();   
    m_3dcontrol_points = mainframe->m_3dcontrol_points;
    m_3dcontrol_lines = mainframe->m_3dcontrol_lines;
    m_3dcontrol_polylines = mainframe->m_3dcontrol_polylines;
    m_3dcontrol_surfaces = mainframe->m_3dcontrol_surfaces;
    m_3dcontrol_volumes = mainframe->m_3dcontrol_volumes;  
    m_3dcontrol_nodes = mainframe->m_3dcontrol_nodes;
    m_3dcontrol_elements = mainframe->m_3dcontrol_elements;
    m_3dcontrol_matgroups = mainframe->m_3dcontrol_matgroups;   
    m_3dcontrol_double_points  = mainframe->m_3dcontrol_double_points;
    m_tolerancefactor = mainframe->m_tolerancefactor;
    m_image_distort_factor_x = mainframe->m_image_distort_factor_x;
    m_image_distort_factor_y = mainframe->m_image_distort_factor_y;
    m_image_distort_factor_z = mainframe->m_image_distort_factor_z;
    m_bounding_box = mainframe->m_bounding_box;
    m_3dcontrol_pcs =  mainframe->m_3dcontrol_pcs;
    m_3dcontrol_bc =  mainframe->m_3dcontrol_bc;
    m_pcs_name = mainframe->m_pcs_name;
    m_pcs_min = mainframe->m_pcs_min;
    m_pcs_max = mainframe->m_pcs_max;
    m_3dcontrol_sourcesink = mainframe->m_3dcontrol_sourcesink;
    m_3dcontrol_msh_quality = mainframe->m_3dcontrol_msh_quality;
    m_3dcontrol_mesh_quality_graph = mainframe->m_3dcontrol_mesh_quality_graph;
    quality_range_min = mainframe->quality_range_min;
    quality_range_max = mainframe->quality_range_max;
    m_selected_wire_frame = mainframe->m_selected_wire_frame;
    m_x_value_color = mainframe->m_x_value_color;
    m_y_value_color = mainframe->m_y_value_color;
    m_z_value_color = mainframe->m_z_value_color;
    m_3dcontrol_point_numbers = mainframe->m_3dcontrol_point_numbers;

    if (mainframe->m_something_changed == 1)
    {
        GetGLIPointsforView();
        //GetGLILinesforView();
        GetGLIPolylinesforView();
        GetGLISurfacesforView();
        //GetRFINodesforView();
        //GetRFIElementsforView();
        //GetRFIMinMaxPoints();
        GetMSHMinMax();

        mainframe->m_something_changed = 0;
     
        CGeoSysDoc* pdoc = GetDocument();
        pdoc->UpdateAllViews(this);
    }

    
    

}

/////////////////////////////////////////////////////////////////////////////
// COGLView diagnostics

#ifdef _DEBUG
void COGLView::AssertValid() const
{
	COGLEnabledView::AssertValid();
}

void COGLView::Dump(CDumpContext& dc) const
{
	COGLEnabledView::Dump(dc);
}

CGeoSysDoc* COGLView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView Overridables from CGLEnabledView

void COGLView::VideoMode(ColorsNumber& c, ZAccuracy& z, BOOL& dbuf)
{
// ask for 65355 colors...
	c=THOUSANDS;
// ...16 bit Z-buffer...
	z=NORMAL;
// ...double-buffering
	dbuf=TRUE;
}



void COGLView::OnCreateGL()
{

// perform hidden line/surface removal (enable Z-Buffering)
	glEnable(GL_DEPTH_TEST);
// set background color to black
	glClearColor(0.f,0.f,0.f,1.0f );
// set clear Z-Buffer value
	glClearDepth(1.0f);
// create and enable a white directional light
	float  color[]={1.f,1.f,1.f,1.f};// RGBA color spec
	glLightfv(GL_LIGHT0,GL_DIFFUSE,color);
	float  ambient[]={.3f,.3f,.3f,1.f};// RGBA color spec
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	float pos[]={1.f,1.f,1.f,0.f};
	glLightfv(GL_LIGHT0,GL_POSITION,pos);
	glEnable(GL_LIGHT0);
	
// prepare charset for text
	PrepareCharset2D("Arial",20);
// Set the background color for the first view
	//SetClearCol(12632256);//Grey
   	SetClearCol(00000000);//Black

}


void COGLView::OnDrawGL()
{
    UpdateData();   
    preRect = lpRect;         
    GetWindowRect(&lpRect);

    /*File handling*/ 
	CGeoSysDoc *doc = GetDocument();
	CGSProject prj;
    prj.ProjectName = doc->m_strGSPFilePathBase;


    // the couple glPush - glPop is necessary because the entire
// scene is rotated of the given angle (which is absolute) at every redraw
	glPushMatrix();
// apply trackball rotation
	trackball.IssueGLrotation();
// "save" the current transformation matrix for subsequent superimpose of axes
	glPushMatrix();
//Hier findet der Aufruf zur Darstellung statt:
// radius is for the examples: this should be self explanatory
	long i=0, j=0, k=0, l=0;
	float Red=0.0,Green=0.0,Blue=1.0;
    double value,value_min,value_max,value_norm;
    int mat_show_ok=0;
    points_vectorlength = (int)view_points_vector.size();
    lines_vectorlength = (int)view_lines_vector.size();
	polylines_vectorlength = (int)view_polylines_vector.size();
    surface_vectorlength = (int)view_surfaces_vector.size();
	rfi_nodes_vectorlength = (int)view_nodes_vector.size();
	rfi_elements_vectorlength = (int)view_elements_vector.size();
    CViewElements* m_ele = NULL;
	x_mid=y_mid=z_mid=x_total=y_total=z_total=0.0;
	ClipSize=1.0;
	GLUquadricObj *qobj =  gluNewQuadric();
	if (1==DocView_Interaction()) sceneselect = 0;
    
/*BoundingBox*/ 
    if (m_bounding_box == 1) {

        GetMSHMinMax();   
        ShowMeshBoundingBox();

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 

    }


/*POINTS*/ 
    if (m_3dcontrol_points == 1 && points_vectorlength != 0) {
         GetMSHMinMax();
		 if (points_vectorlength < 50)glPointSize(4.0);
		 else glPointSize(3.0);

         glBegin(GL_POINTS);/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/  
         j=0;
		 for (j=0;j<points_vectorlength;j++)
         { 
          if (gli_points_vector[j]->display_mode == 1)
          {
		   glColor3d(0.5,0.1,0.0);	  
		   glVertex3d(m_image_distort_factor_x*(-x_mid +  gli_points_vector[j]->x),m_image_distort_factor_y*(-y_mid +  gli_points_vector[j]->y),m_image_distort_factor_z*(-z_mid +  gli_points_vector[j]->z));   		   
          }
         }
    	 glEnd();
		 
         if (m_3dcontrol_point_numbers == 1)
         {
         char number[24];

         for (j=0;j<(int)gli_points_vector.size();j++)
         { 
          if (gli_points_vector[j]->display_mode == 1)
          {
		   glColor3f(1.0,1.0,1.0);
		   //glRasterPos3d(m_image_distort_factor_x*view_points_vector[j]->x,m_image_distort_factor_y*view_points_vector[j]->y,m_image_distort_factor_z*view_points_vector[j]->z);
           glRasterPos3d(m_image_distort_factor_x* (-x_mid +  gli_points_vector[j]->x),
                         m_image_distort_factor_y* (-y_mid +  gli_points_vector[j]->y),
                         m_image_distort_factor_z* (-z_mid +  gli_points_vector[j]->z));

           long point_number = gli_points_vector[j]->id;
		   sprintf(number,"%ld",point_number);
		   Text2D(number);		
          }
		 }
         }

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 
    }
/*POLYLINES*/ 
    if (m_3dcontrol_polylines == 1 && polylines_vectorlength != 0) { 
         glLineWidth(2);
         glColor3d(0.0,0.3,0.7);

		 for (j=0;j<polylines_vectorlength;j++)
         {
			 //glEnable(GL_BLEND);
	 		 glBegin(GL_LINE_STRIP);/*Linien lesen aus View-Linien-Vektor und darstellen*/
			 polylinepoints_vectorlength = (int)view_polylines_vector[j]->polyline_point_vector.size();
             for (k=0;k<polylinepoints_vectorlength;k++)
             {	     
    			glVertex3d(m_image_distort_factor_x*view_polylines_vector[j]->polyline_point_vector[k]->x,m_image_distort_factor_y*view_polylines_vector[j]->polyline_point_vector[k]->y,m_image_distort_factor_z*view_polylines_vector[j]->polyline_point_vector[k]->z);	
			 }
	 		 glEnd();
		 }
		 glLineWidth(1);

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 

    }
/*SURFACES*/ 
    if (m_3dcontrol_surfaces == 1 && surface_vectorlength != 0) {
       vector<GLdouble*> addGLdouble;
       //GetGLISurfacesforView();    
		 colorsignal = 1;
	 	 glEnable(GL_BLEND);
		 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		 //glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
         double col_val1, col_val2;
         surface_vectorlength = (int)view_surfaces_vector.size();

		 for (j=0;j<surface_vectorlength;j++)
         {
              col_val1=view_surfaces_vector[j]->surface_id;;
              col_val2 = (double)surface_vector.size();//CC
              value_norm = col_val1/col_val2;

                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			    glColor4d(Red,Green,Blue,0.7);

                GLUtesselator *tobj = gluNewTess();
                startList = glGenLists(1);
                gluTessCallback(tobj, GLU_TESS_VERTEX,
                                (GLvoid (CALLBACK*) ()) &glVertex3dv);
                gluTessCallback(tobj, GLU_TESS_BEGIN,
                                (GLvoid (CALLBACK*) ()) &beginCallback);
                gluTessCallback(tobj, GLU_TESS_END,
                                (GLvoid (CALLBACK*) ()) &endCallback);

                glNewList(startList, GL_COMPILE);
                //glShadeModel(GL_FLAT);
                gluTessBeginPolygon(tobj, NULL);
                    gluTessBeginContour(tobj);
                    GLdouble *v = NULL;
         
			 surfacepolyline_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector.size();
             for (k=0;k<surfacepolyline_vectorlength;k++)
             {	
                        surfacepolylinepoints_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector.size();		
                        for (l=0;l<surfacepolylinepoints_vectorlength-1;l++)
                        {
	                        point_id  = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->id;
	                        x = m_image_distort_factor_x*view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
	                        y = m_image_distort_factor_y*view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
	                        z = m_image_distort_factor_z*view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
	                        v = new GLdouble[ 3 ];
                            addGLdouble.push_back(v);
                            v[ 0 ] = x;
                            v[ 1 ] = y;
                            v[ 2 ] = z;
                            gluTessVertex( tobj, v, v ); 
                        }
    		 }
                gluTessEndContour(tobj);
                gluTessEndPolygon(tobj);
                glEndList();
                glCallList(startList);   
                glDeleteLists(startList,2);
                gluDeleteTess(tobj);

                for (l=0;l<(int)addGLdouble.size();l++)
                    delete addGLdouble[l];
                addGLdouble.clear();
			 

		 }
		 			 glDisable(GL_BLEND);

    /*TINS*/  
    CGLPoint tin_point1;
    CGLPoint tin_point2;
    CGLPoint tin_point3;
    GetMSHMinMax();
    GetMidPoint(); 

    Surface * m_surface = NULL;
    vector<Surface*>::const_iterator ps = surface_vector.begin();
	int tin_surface_vectorlength = (int)surface_vector.size();

    

	for (j=0;j<tin_surface_vectorlength ;j++)
    {
   	  m_surface = *ps;
	  CString Surface_Name = m_surface->name.data();

              col_val1=j;
              col_val2 = (double)surface_vector.size();//CC
              value_norm = col_val1/col_val2;
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			    glColor4d(Red,Green,Blue,0.7);

      if (m_surface->display_mode_3d == 1)
      {
        if (m_surface->TIN)
        {
           long nb_TIN_elements = (int)m_surface->TIN->Triangles.size();
           for(k=0;k<nb_TIN_elements;k++) 
           {
            tin_point1.x = -x_mid + m_surface->TIN->Triangles[k]->x[0];
            tin_point1.y = -y_mid + m_surface->TIN->Triangles[k]->y[0];
            tin_point1.z = -z_mid + m_surface->TIN->Triangles[k]->z[0];
            tin_point2.x = -x_mid + m_surface->TIN->Triangles[k]->x[1];
            tin_point2.y = -y_mid + m_surface->TIN->Triangles[k]->y[1];
            tin_point2.z = -z_mid + m_surface->TIN->Triangles[k]->z[1];
            tin_point3.x = -x_mid + m_surface->TIN->Triangles[k]->x[2];
            tin_point3.y = -y_mid + m_surface->TIN->Triangles[k]->y[2];
            tin_point3.z = -z_mid + m_surface->TIN->Triangles[k]->z[2];


            if (boundingbox == 1)
            {
             glPointSize(1.0);
             glBegin(GL_POINTS);
			 //glColor3d(0.3,0.6,0.0);
             glVertex3d(m_image_distort_factor_x*tin_point1.x,m_image_distort_factor_y*tin_point1.y,m_image_distort_factor_z*tin_point1.z);
             glVertex3d(m_image_distort_factor_x*tin_point2.x,m_image_distort_factor_y*tin_point2.y,m_image_distort_factor_z*tin_point2.z);
             glVertex3d(m_image_distort_factor_x*tin_point3.x,m_image_distort_factor_y*tin_point3.y,m_image_distort_factor_z*tin_point3.z);
             glEnd();
            }
            else
            {

			 glBegin(GL_LINE_LOOP);
			 glColor3d(Red,Green-0.1,Blue-0.1);
             glVertex3d(m_image_distort_factor_x*tin_point1.x,m_image_distort_factor_y*tin_point1.y,m_image_distort_factor_z*tin_point1.z);
             glVertex3d(m_image_distort_factor_x*tin_point2.x,m_image_distort_factor_y*tin_point2.y,m_image_distort_factor_z*tin_point2.z);
             glVertex3d(m_image_distort_factor_x*tin_point3.x,m_image_distort_factor_y*tin_point3.y,m_image_distort_factor_z*tin_point3.z);
             glEnd();

   			 glBegin(GL_TRIANGLES);
			 glColor3d(Red,Green,Blue);
             glVertex3d(m_image_distort_factor_x*tin_point1.x,m_image_distort_factor_y*tin_point1.y,m_image_distort_factor_z*tin_point1.z);
             glVertex3d(m_image_distort_factor_x*tin_point2.x,m_image_distort_factor_y*tin_point2.y,m_image_distort_factor_z*tin_point2.z);
             glVertex3d(m_image_distort_factor_x*tin_point3.x,m_image_distort_factor_y*tin_point3.y,m_image_distort_factor_z*tin_point3.z);
             glEnd();
            }

           }

        }
      }
      ++ps;
    }

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
   	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/    
    
    }

/*VOLUMES*/ 
    if (m_3dcontrol_volumes == 1 && volume_vectorlength != 0) { 
       //(m_3dcontrol_volumes == 1 && surface_vectorlength != 0)
     if (boundingbox == 1)
     {
        ShowMeshBoundingBox();         
     }      
     else     
     {
        double col_val1, col_val2;
        CGLVolume *gl_volume = NULL;
        Surface * gl_surface = NULL;
         vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC 
        vector<Surface*>::iterator p_sfc = NULL;
        volume_vectorlength  = (int)volume_vector.size();  //CC
            for(i=0;i<volume_vectorlength;i++)
            {
              /*---Farbpalette---START*/ 
              col_val1=i;
              col_val2=volume_vector.size();
              value_norm = col_val1/col_val2;
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glEnable(GL_BLEND);
        		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			    glColor4d(Red,Green,Blue,0.4);                             
              /*---Farbpalette-----END*/ 

                gl_volume = *p_vol;
                p_sfc = gl_volume->surface_vector.begin();               
                if (gl_volume->display_mode == 1)
                {       
                    CString volume_name = gl_volume->name.data(); 
                    while (p_sfc!= gl_volume->surface_vector.end())
                    {
                        int volumentyp = gl_volume->type;
                        int surfacelistsize = (int)gl_volume->surface_vector.size();

                        if (volumentyp == 1 && surfacelistsize == 2)
                        {

                        gl_surface = *p_sfc;
                        CString surface_name_of_volume = gl_surface->name.data();

                        
    /*TINS*/ 
    CGLPoint tin_point1;
    CGLPoint tin_point2;
    CGLPoint tin_point3;
	GetMSHMinMax();
    GetMidPoint(); 

    Surface * m_surface = NULL;
    vector<Surface*>::iterator ps = surface_vector.begin();//CC
	int vol_tin_surface_vectorlength = (int)surface_vector.size();

    

	for (j=0;j<vol_tin_surface_vectorlength ;j++)
    {
   	  m_surface = *ps;
	  CString Surface_Name = m_surface->name.data();
      if (Surface_Name == surface_name_of_volume)
      {
        if (m_surface->TIN)
        {
           long nb_TIN_elements = (int)m_surface->TIN->Triangles.size();
           for(k=0;k<nb_TIN_elements;k++) 
           {
            tin_point1.x = -x_mid + m_surface->TIN->Triangles[k]->x[0];
            tin_point1.y = -y_mid + m_surface->TIN->Triangles[k]->y[0];
            tin_point1.z = -z_mid + m_surface->TIN->Triangles[k]->z[0];
            tin_point2.x = -x_mid + m_surface->TIN->Triangles[k]->x[1];
            tin_point2.y = -y_mid + m_surface->TIN->Triangles[k]->y[1];
            tin_point2.z = -z_mid + m_surface->TIN->Triangles[k]->z[1];
            tin_point3.x = -x_mid + m_surface->TIN->Triangles[k]->x[2];
            tin_point3.y = -y_mid + m_surface->TIN->Triangles[k]->y[2];
            tin_point3.z = -z_mid + m_surface->TIN->Triangles[k]->z[2];

   			 glBegin(GL_TRIANGLES);
             glVertex3d(m_image_distort_factor_x*tin_point1.x,m_image_distort_factor_y*tin_point1.y,m_image_distort_factor_z*tin_point1.z);
             glVertex3d(m_image_distort_factor_x*tin_point2.x,m_image_distort_factor_y*tin_point2.y,m_image_distort_factor_z*tin_point2.z);
             glVertex3d(m_image_distort_factor_x*tin_point3.x,m_image_distort_factor_y*tin_point3.y,m_image_distort_factor_z*tin_point3.z);
             glEnd();
        }
      }
      }
      ++ps;
    }

                        ++p_sfc; 
//
 
//
                        }
                    }
                }
                ++p_vol;
            }
     }

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
   	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/    
    
    }

/*Boundary Conditions*/ 
    if (m_3dcontrol_bc == 1 ) { 

        CBoundaryConditionsGroup *m_bc_group = NULL;
        list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
        //GetMSHMinMax();
        GetMidPoint();
        while(p_bc_group!=bc_group_list.end()) {
            m_bc_group = *p_bc_group;
            if(m_bc_group->m_display_mode_bc == 1)
            {
            for(j=0;j<(long)fem_msh_vector.size();j++)
            { 
             if (m_bc_group->pcs_type_name.find(fem_msh_vector[j]->pcs_name.data())==0 || fem_msh_vector.size() == 1)
             {
             glPointSize(6.0);
			 glColor3d(1.0,0.0,0.0);
             glBegin(GL_POINTS);
             for (i=0;i<(long)m_bc_group->group_vector.size();i++) 
             {
             bc_x = -x_mid + fem_msh_vector[j]->nod_vector[m_bc_group->group_vector[i]->msh_node_number]->X();
             bc_y = -y_mid + fem_msh_vector[j]->nod_vector[m_bc_group->group_vector[i]->msh_node_number]->Y();
             bc_z = -z_mid + fem_msh_vector[j]->nod_vector[m_bc_group->group_vector[i]->msh_node_number]->Z();
             glVertex3d(m_image_distort_factor_x*bc_x,m_image_distort_factor_y*bc_y,m_image_distort_factor_z*bc_z);   		   
             }
             glEnd();                   
             }
            }
            }       
            ++p_bc_group;
        }
    }


/*NODES*/ 

  for(j=0;j<(long)fem_msh_vector.size();j++)
  {    
    if (fem_msh_vector[j]->nod_display_mode == 1)
    {
       if (boundingbox == 1)
       {
           ShowMeshBoundingBox();         
       }	
       else
       {
        GetMidPoint();
		/*MSH-Knoten lesen*/
		for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
         {
             if (fem_msh_vector[j]->nod_vector[i]->selected == 1){
                 glPointSize(8.0);
			     glColor3d(1.0,0.4,0.0);
             }
             else {
                 glPointSize(2.0);
			     glColor3d(0.0,0.4,0.0);
             }

             glBegin(GL_POINTS);
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (fem_msh_vector[j]->nod_vector[i]->X())),
                        m_image_distort_factor_y* (-y_mid +   (fem_msh_vector[j]->nod_vector[i]->Y())),
                        m_image_distort_factor_z* (-z_mid +   (fem_msh_vector[j]->nod_vector[i]->Z())));
             glEnd();
       }
		 }

		
         
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 	
    }
  }
/*ELEMENTS*/     

  for(j=0;j<(long)fem_msh_vector.size();j++)
  {    
    if (fem_msh_vector[j]->ele_display_mode == 1)
    {
       if (boundingbox == 1)
       {
           ShowMeshBoundingBox();         
       }	
       else
       {
        GetMidPoint();
        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
         {
         	 /*LINES = 1*/ 
             if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 1)
			 {
			 Draw_LineWireFrame(j, i);
			 }
			 /*RECTANGLES = 2*/ 
			 if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 2)
			 {
			 Draw_QuadWireFrame(j, i);
			 }
			 /*HEXAHEDRA = 3*/ 
			 if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 3)
			 {
			 Draw_HexWireFrame(j, i);
			 }
			 /*TRIANGLES = 4*/ 
			 if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 4)
			 {
			 Draw_TriWireFrame(j, i);
                if (fem_msh_vector[j]->ele_vector[i]->selected == 1){
                Draw_SelectedTriangles(j,i);
                }
     		 }
			 /*TETRAHEDRAS = 5*/ 
			 if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 5)
			 {
			 Draw_TetWireFrame(j, i);
			 }
			 /*PRISMS = 6*/ 
			 if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 6)
			 {
			 Draw_PrismWireFrame(j, i);
			 }

         
         }

       }
         
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 	
    }
  }

/*MAT_GROUPS of ELEMENTS*/     
  for(j=0;j<(long)fem_msh_vector.size();j++)
  {    
    if (fem_msh_vector[j]->ele_mat_display_mode == 1)
    {
       if (boundingbox == 1)
       {
           ShowMeshBoundingBox();         
       }	
       else
       {
        GetMidPoint();
        glDisable(GL_BLEND);
        value_min = 0;
        value_max = fem_msh_vector[j]->highest_mat_group_nb;   
        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
         if (fem_msh_vector[j]->ele_vector[i]->matgroup_view == 1)
         {
           switch(fem_msh_vector[j]->ele_vector[i]->GetElementType()){
            case 4: // triangle

             if (m_selected_wire_frame == 1)
             {
               Draw_TriWireFrame(j,i);
             }
              
	          glBegin(GL_TRIANGLES) ;
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
            }
                glEnd() ;

              break;

            case 2: // rectangle

             if (m_selected_wire_frame == 1)
             {
               Draw_QuadWireFrame(j,i);
             }

	          glBegin(GL_QUADS) ;
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
			  Red   =0.0;
			  Blue  =1.0;
			  Green =0.0;  
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
 		        glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));    
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));            
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));            
            }
              glEnd() ;
			  break;	

             case 5: // tetrahedra      

             if (m_selected_wire_frame == 1)
             {
               Draw_TetWireFrame(j,i);
             }

              glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
            }
                glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
            }
               glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI3*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
            }
                glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI4*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
                }
            else
            {
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
            }
                glEnd() ;

              break;

             case 6: // prism

             if (m_selected_wire_frame == 1)
             {
               Draw_PrismWireFrame(j,i);
             }

	          glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);	
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
            }
             glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;
 
                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
                }
            else
            {
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
            }
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD1*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
            }
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD2*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
            }
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD3*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
            }
              glEnd() ;

    		  break;	

             case 3: // Hexahedra

             if (m_selected_wire_frame == 1)
             {
               Draw_HexWireFrame(j,i);
             }

	          glBegin(GL_QUADS) ; /*Quad1*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
            }
             glEnd() ;

	          glBegin(GL_QUADS) ; /*Quad2*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
                glColor3f(Red,Green,Blue) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));    
                }
            else
            {
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));
            }
             glEnd() ;

              glBegin(GL_QUADS); /*QUAD3*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
    	      glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
            }
             glEnd() ;

              glBegin(GL_QUADS); /*QUAD4*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;

                  if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
            }
             glEnd() ;

              glBegin(GL_QUADS); /*QUAD5*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));
            }
             glEnd() ;

              glBegin(GL_QUADS); /*QUAD6*/ 
              value = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;

                if (m_x_value_color == 1 || m_y_value_color == 1 || m_z_value_color == 1 )
                {  
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
			
                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
            			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));

                            if (m_x_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X() - x_min) / (x_dist);
                            if (m_y_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y() - y_min) / (y_dist);
                            if (m_z_value_color == 1 )
                            value_norm = (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z() - z_min) / (z_dist);
                            Red =   Get_Red_Value(value_norm);
                            Green = Get_Green_Value(value_norm);
                            Blue =  Get_Blue_Value(value_norm);
                            glColor3f(Red,Green,Blue) ;
                            glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));    
                }
            else
            {
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));
            }
             glEnd() ;


    		  break;	


           }
         }
      
       }

       }
         
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 	
    }
  }

/*MATERIAL GROUPS*/ 
    if(m_3dcontrol_matgroups == 1 && rfi_elements_vectorlength != 0)
    {
     if (boundingbox == 1)
     {
        ShowMeshBoundingBox();         
     }      
     else     
     {
        value_min = 0;
        value_max = mmp_vector.size();        
		for (j=0;j<rfi_elements_vectorlength;j++){
          
             for (l=0;l<(int)material_groups_vector.size();l++)
             {
                 if (material_groups_vector[l]->materialnumber == ElGetElementGroupNumber(j) && material_groups_vector[l]->display_mode == 1){
                 mat_show_ok = 1;
                 break;
                 }
                 else mat_show_ok = 0;
             }
         if (mat_show_ok == 1) 
         {
          m_ele = view_elements_vector[j];
          switch(m_ele->element_type){
            case 4: // triangle
              
	          glBegin(GL_TRIANGLES) ;
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
              glEnd() ;

              break;

            case 2: // rectangle
	          glBegin(GL_QUADS) ;
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));            
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));            
              glEnd() ;
			  break;	

             case 5: // tetrahedra      
              glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI3*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI4*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
			    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
                glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
              glEnd() ;

              break;

             case 6: // prism

	          glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
			 glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD1*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD2*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD3*/ 
              value = ElGetElementGroupNumber(j);
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
             glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
              glEnd() ;

    		  break;	
           }
		  }
		}
        }


        /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 

    }
/*SOURCE SINK TERMS*/ 
    if(m_3dcontrol_sourcesink == 1)
    {
/*
      glPointSize(2.0);
	  glBegin(GL_POINTS);
	  for (k=0;k<(int)m_surface->nodes_coor_vector.size();k++)
	  {
           glColor3d(0.0,0.9,0.0);	 
		   bc_x = -x_mid + m_surface->nodes_coor_vector[k][0];
		   bc_y = -y_mid + m_surface->nodes_coor_vector[k][1];
		   bc_z = -z_mid + m_surface->nodes_coor_vector[k][2];
		   glVertex3d(m_image_distort_factor_x*bc_x,m_image_distort_factor_y*bc_y,m_image_distort_factor_z*bc_z);   		   
   	  }
      glEnd();
*/

         GetMSHMinMax();
         GetMidPoint(); 
         CSourceTerm* m_st = NULL;
		 for (j=0;j<(int)st_vector.size();j++)
         {
             m_st = st_vector[j];
             if (m_st->display_mode == 1)
             {
             if (m_st->geo_type_name.compare("LINE") == 0)
             {
                CGLLine* m_lin = NULL;
                m_lin = m_lin->GEOGetLine(m_st->geo_id);
                glColor3d(0.9,0.0,0.0);
                glPointSize(3.0);
	            glBegin(GL_POINTS);
	            for (k=0;k<(int)m_lin->nodes_coor_vector.size();k++)
	            {
		            bc_x = -x_mid + m_lin->nodes_coor_vector[k][0];
		            bc_y = -y_mid + m_lin->nodes_coor_vector[k][1];
		            bc_z = -z_mid + m_lin->nodes_coor_vector[k][2];
		            glVertex3d(m_image_distort_factor_x*bc_x,m_image_distort_factor_y*bc_y,m_image_distort_factor_z*bc_z);   		   
   	            }
                glEnd();
             }
             }
         }

         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 
    }


/*PCS-FEM-ISOSURFACES*/ 
    if(m_3dcontrol_pcs == 1 && rfi_elements_vectorlength != 0)
    {
     if (boundingbox == 1)
     {
        ShowMeshBoundingBox();         
     }      
     else     
     {
        value_min = m_pcs_min;
        value_max = m_pcs_max;        
		for (j=0;j<rfi_elements_vectorlength;j++){
          m_ele = view_elements_vector[j];
          switch(m_ele->element_type){
            case 4: // triangle
	          glBegin(GL_TRIANGLES) ;
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              glEnd() ;

              break;

            case 2: // rectangle
	          glBegin(GL_QUADS) ;
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
			  value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
              glColor3f(Red,Green,Blue) ;
  		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;
              glEnd() ;
			  break;	

             case 5: // tetrahedra
	          glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI3*/ 
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI4*/ 
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_y*m_ele->z2) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              glEnd() ;

              break;

             case 6: // prism

	          glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;
              value = GetNodeVal(m_ele->nodenumber5,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x5,m_image_distort_factor_y*m_ele->y5,m_image_distort_factor_z*m_ele->z5) ;
              value = GetNodeVal(m_ele->nodenumber6,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x6,m_image_distort_factor_y*m_ele->y6,m_image_distort_factor_z*m_ele->z6) ;
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD1*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              
              value = GetNodeVal(m_ele->nodenumber6,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x6,m_image_distort_factor_y*m_ele->y6,m_image_distort_factor_z*m_ele->z6) ;
			  
              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
              glColor3f(Red,Green,Blue) ;
  		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD2*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x1,m_image_distort_factor_y*m_ele->y1,m_image_distort_factor_z*m_ele->z1) ;
              
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              
              value = GetNodeVal(m_ele->nodenumber5,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x5,m_image_distort_factor_y*m_ele->y5,m_image_distort_factor_z*m_ele->z5) ;
			  
              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
              glColor3f(Red,Green,Blue) ;
  		      glVertex3f(m_image_distort_factor_x*m_ele->x4,m_image_distort_factor_y*m_ele->y4,m_image_distort_factor_z*m_ele->z4) ;
              glEnd() ;

              glBegin(GL_QUADS); /*QUAD3*/ 
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x3,m_image_distort_factor_y*m_ele->y3,m_image_distort_factor_z*m_ele->z3) ;
              
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x2,m_image_distort_factor_y*m_ele->y2,m_image_distort_factor_z*m_ele->z2) ;
              
              value = GetNodeVal(m_ele->nodenumber5,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_image_distort_factor_x*m_ele->x5,m_image_distort_factor_y*m_ele->y5,m_image_distort_factor_z*m_ele->z5) ;
			  
              value = GetNodeVal(m_ele->nodenumber6,PCSGetNODValueIndex((string)m_pcs_name,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
              glColor3f(Red,Green,Blue) ;
  		      glVertex3f(m_image_distort_factor_x*m_ele->x6,m_image_distort_factor_y*m_ele->y6,m_image_distort_factor_z*m_ele->z6) ;
              glEnd() ;

    		  break;	

		  }
		}
        }


        /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 

    }


/*DoublePoints*/ 
    if (m_3dcontrol_double_points == 1) 
    {
         DisplayDoublePoints();
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 


    }

/*MESH QUALITY*/ 
    if(m_3dcontrol_msh_quality == 1 && msh_elements_vector.size()!= 0 )
    {   
       if (boundingbox == 1)
       {
           ShowMeshBoundingBox();         
       }	
       else
       {
		for (j=0;j<(int)msh_elements_vector.size();j++)
         {
			 /*TRIANGLES = 4*/ 
			 if (msh_elements_vector[j]->element_type == 4)
			 {
			 glColor3d(1.00-(msh_elements_vector[j]->quality_factor),msh_elements_vector[j]->quality_factor,0.0);
			 //glColor3d(1-msh_elements_vector[j]->quality_factor,msh_elements_vector[j]->quality_factor,0.0);
             glEnable(GL_BLEND);
			 glBegin(GL_TRIANGLES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
		     glEnd();     	
     		 }
			 /*TETRAHEDRAS = 5*/ 
			 if (msh_elements_vector[j]->element_type == 5)
			 {
	 	 	 glEnable(GL_BLEND);
			 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			 if (msh_elements_vector[j]->quality_factor >= quality_range_min && 
				 msh_elements_vector[j]->quality_factor <= quality_range_max)
			 {
 			 glColor4d(1.00-(msh_elements_vector[j]->quality_factor),
				       0.0,msh_elements_vector[j]->quality_factor,0.9); //==high 1=no transparenz

    		 glBegin(GL_TRIANGLES); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
		     glEnd();
			 glBegin(GL_TRIANGLES); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
		     glEnd();
 			 glBegin(GL_TRIANGLES); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
		     glEnd();
  			 glBegin(GL_TRIANGLES); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
		     glEnd();

			 glColor3d(0.0,0.0,0.0);
			 glBegin(GL_LINE_LOOP);
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
		     glEnd();
			 glBegin(GL_LINE_LOOP); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
		     glEnd();
 			 glBegin(GL_LINE_LOOP); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x3)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y3)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z3)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
		     glEnd();
  			 glBegin(GL_LINE_LOOP); 
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x1)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y1)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z1)));
			 glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x2)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y2)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z2)));
             glVertex3d(m_image_distort_factor_x* (-x_mid +   (msh_elements_vector[j]->x4)),m_image_distort_factor_y* (-y_mid +   (msh_elements_vector[j]->y4)),m_image_distort_factor_z* (-z_mid +   (msh_elements_vector[j]->z4)));
		     glEnd();

			 }
			 }

		 } 

       }	
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
         if (preRect != lpRect) Invalidate(TRUE);
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 


    




    }



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
             Half Static Display: Without Zoom but with Rotation
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	GetMSHMinMax();
    GetMidPoint();
    trackball.DrawBall();
/*
             //X-Achse 
 			 glBegin(GL_LINES); 
             glColor3d(0.0,1.0,0.0);
			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex2d( trackball.center.x+x_dist/2,trackball.center.y);            
			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex2d( trackball.center.x-x_dist/2,trackball.center.y);            
             glEnd(); 	
             //Y-Achse 
 			 glBegin(GL_LINES); 
             glColor3d(0.0,0.0,1.0);
			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex2d( trackball.center.x,trackball.center.y+y_dist/2);
   			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex2d( trackball.center.x,trackball.center.y-y_dist/2);            
             glEnd(); 	
             //Z-Achse 
 			 glBegin(GL_LINES); 
             glColor3d(1.0,0.0,0.0);
			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex3d( trackball.center.x,trackball.center.y,z_dist/2);            
			 glVertex2d( trackball.center.x,trackball.center.y);            
             glVertex3d( trackball.center.x,trackball.center.y,-z_dist/2);            
             glEnd(); */	
  			


    trackball.DrawBall();

    
	//glViewport(0,0,m_ClientRect.right/5,m_ClientRect.bottom/5);
	glPopMatrix();
	glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		DrawStockDispLists();
	glPopAttrib();
	//glViewport(0,0,m_ClientRect.right,m_ClientRect.bottom);   
	glPopMatrix();
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
             Static Display: Without Rotation or Zoom
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*PCS LEGEND*/         
        if(m_3dcontrol_pcs == 1)
		{
			 static char number[10];
             value_min = m_pcs_min;
             value_max = m_pcs_max; 
             double legendewert;

			 glPushMatrix();
			 glMatrixMode(GL_PROJECTION);

			 /*Titel*/ 
			 glColor3d(0.0,0.0,1.0);
  			 glRasterPos2d((0.00*dist/scale_x)-trans_x,(-0.97*dist/scale_y)-trans_y);            
			 Text2D(m_pcs_name);

			 
			 /*Zeitschrittanzeige*/ 
			 glColor3d(0.0,0.0,0.0);
   			 glRasterPos2d(( 0.40*dist/scale_x)-trans_x,( 0.85*dist/scale_y)-trans_y);            
			 Text2D("TIME:");
   			 glRasterPos2d(( 0.65*dist/scale_x)-trans_x,( 0.85*dist/scale_y)-trans_y);            
             sprintf(number,"%lg",aktuelle_zeit);          
			 Text2D(number);

			 /*Legende Rahmen und Unterteilung*/ 
 			 glBegin(GL_LINE_LOOP); 
             glColor3d(0.0,0.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);            
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);              
        	 glEnd(); 	
  			 glBegin(GL_LINES); 
             glColor3d(0.0,0.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
   			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
   			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
			 glEnd(); 	

	
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,(-0.98*dist/scale_y)-trans_y);            
             legendewert = value_min;
			 sprintf(number,"%lg",legendewert);          
			 Text2D(number);
			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,(-0.48*dist/scale_y)-trans_y);          
    		 legendewert = value_min+((value_max-value_min)*1/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);		
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.02*dist/scale_y)-trans_y);            
    		 legendewert = value_min+((value_max-value_min)*2/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.52*dist/scale_y)-trans_y);            
    		 legendewert = value_min+((value_max-value_min)*3/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.92*dist/scale_y)-trans_y);            
    		 legendewert = value_max;
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
             
			 /*Legende Farben*/ 
		     glBegin(GL_QUADS); 
             /*1.Unit:*/ 
			 glColor3d(0.0,0.0,1.0);
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);
             glColor3d(0.0,1.0,1.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             /*2.Unit:*/ 
			 glColor3d(0.0,1.0,1.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glColor3d(0.0,1.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.00*dist/scale_y)-trans_y);            
             /*3.Unit:*/ 
             glColor3d(0.0,1.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glColor3d(1.0,1.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             /*4.Unit:*/ 
             glColor3d(1.0,1.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glColor3d(1.0,0.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
			 glEnd(); 	
			 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
			 glPushAttrib(GL_ENABLE_BIT);
			 glPopAttrib();
			 view_counter++;
        }

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               END OF DRAWING
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    //
	switch(sceneselect)
	{
	case 0://Keine Ansicht
	     
        		break;

	case 1: //GLI-Points	 
		 if (points_vectorlength < 50)
		 { /*DATA INPUT*/	
         GLU_BEGIN;/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/ 
		 for (j=0;j<points_vectorlength;j++)
         { 
	        GLU_VERTEX;
			//glEnable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
			glColor3d(0.0,0.0,0.0);
            glTranslated(view_points_vector[j]->x,view_points_vector[j]->y,view_points_vector[j]->z);   		   
			gluQuadricDrawStyle( qobj, GLU_FILL);
			gluQuadricNormals( qobj, GLU_SMOOTH );
			gluSphere( qobj, (max-min)/400, 9, 9); 
            glTranslated((view_points_vector[j]->x)*(-1),(view_points_vector[j]->y)*(-1),(view_points_vector[j]->z)*(-1));   		   
		 }
    	 GLU_END;
		 }

		 else
		 { /*DATA INPUT*/	
		 glPointSize(2.0);
         glBegin(GL_POINTS);/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/  
         j=0;
		 for (j=0;j<points_vectorlength;j++)
         { 
		   glColor3d(0.0,0.0,0.0);	  
		   glVertex3d(view_points_vector[j]->x,view_points_vector[j]->y,view_points_vector[j]->z);   		   
		 }
    	 glEnd();
		 }
		  	
         /*DISPLAY*/ 
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (PointOnOff==1) DisplayPoints();
 		 if (DoublePointOnOff==1) DisplayDoublePoints();
         if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translatations-Steuerung*/ 
		break;
	case 2://GLI-Lines           
   		 glBegin(GL_LINES);/*Linien lesen aus View-Linien-Vektor und darstellen*/
		 for (j=0;j<lines_vectorlength;j++)
         {
			 if (colorsignal == 1){
				 colorsignal = 0;
				 glColor3d(1.0,0.0,0.0);
			 }
			 else {
				 colorsignal = 1;
				 glColor3d(0.0,0.0,1.0);
			 }

			 glVertex3d(view_lines_vector[j]->x1,view_lines_vector[j]->y1,view_lines_vector[j]->z1);            
			 glVertex3d(view_lines_vector[j]->x2,view_lines_vector[j]->y2,view_lines_vector[j]->z2);
     	 }
		 glEnd();
		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)),\
				     -ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)),\
					 -ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (PointOnOff==1) DisplayPoints();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
         if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		 break;
	case 3://GLI-Polylines 
		 colorsignal = 0;
		 for (j=0;j<polylines_vectorlength;j++)
         {
			 //glEnable(GL_BLEND);
			 colorsignal++;
			 if (colorsignal == 10)colorsignal=1;
	 		 glBegin(GL_LINE_STRIP);/*Linien lesen aus View-Linien-Vektor und darstellen*/
			 polylinepoints_vectorlength = (int)view_polylines_vector[j]->polyline_point_vector.size();
             for (k=0;k<polylinepoints_vectorlength;k++)
             {
			    switch (colorsignal)
				{
					case 0: glColor4d(1.0,0.0,0.0,0.3);
						break;
					case 1: glColor4d(0.0,1.0,0.0,0.3);
						break;
					case 2: glColor4d(0.0,0.0,1.0,0.3);
						break;
					case 3: glColor4d(1.0,1.0,0.0,0.3);
						break;
					case 4: glColor4d(0.0,1.0,1.0,0.3);
						break;
					case 5: glColor4d(1.0,0.0,1.0,0.3);
						break;
					case 6: glColor4d(0.5,0.0,0.0,0.3);
						break;
					case 7: glColor4d(0.0,0.5,0.0,0.3);
						break;
					case 8: glColor4d(0.0,0.0,0.5,0.3);
						break;
					case 9: glColor4d(0.0,0.5,0.5,0.3);
						break;
					default: glColor4d(1.0,1.0,1.0,0.3);
						break;
				}
    			glVertex3d(view_polylines_vector[j]->polyline_point_vector[k]->x,view_polylines_vector[j]->polyline_point_vector[k]->y,view_polylines_vector[j]->polyline_point_vector[k]->z);	
			 }
	 		 glEnd();
		 }
		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointOnOff==1) DisplayPoints();
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		break;
	case 4://GLI-Surfaces
		 colorsignal = 1;
	 	 glEnable(GL_BLEND);
		 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		 //glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);


		 for (j=0;j<surface_vectorlength;j++)
         {
		     colorsignal++;
			 if (colorsignal == 10)colorsignal=1;
			 
			 surfacepolyline_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector.size();
             for (k=0;k<surfacepolyline_vectorlength;k++)
             {
			    switch (colorsignal)
				{
					case 0: glColor4d(1.0,0.0,0.0,0.7);
						break;
					case 1: glColor4d(0.0,1.0,0.0,0.7);
						break;
					case 2: glColor4d(0.0,0.0,1.0,0.7);
						break;
					case 3: glColor4d(1.0,1.0,0.0,0.7);
						break;
					case 4: glColor4d(0.0,1.0,1.0,0.7);
						break;
					case 5: glColor4d(1.0,0.0,1.0,0.7);
						break;
					case 6: glColor4d(0.5,0.0,0.0,0.7);
						break;
					case 7: glColor4d(0.0,0.5,0.0,0.7);
						break;
					case 8: glColor4d(0.0,0.0,0.5,0.7);
						break;
					case 9: glColor4d(0.0,0.5,0.5,0.7);
						break;
					default:glColor4d(1.0,1.0,1.0,0.7);
						break;
				}
			
				surfacepolylinepoints_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector.size();		
                for (l=0;l<surfacepolylinepoints_vectorlength;l++)
				{
                    if (l<surfacepolylinepoints_vectorlength-1){
					glBegin(GL_TRIANGLES);/*Linien lesen aus View-Linien-Vektor und darstellen*/
					point_id  = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->id;//CC
					x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					glVertex3d(x,y,z);
					l++;
					x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					glVertex3d(x,y,z);
					l--;
					x = view_surfaces_vector[j]->surface_midpoint_x;
					y = view_surfaces_vector[j]->surface_midpoint_y;
					z = view_surfaces_vector[j]->surface_midpoint_z;
					glVertex3d(x,y,z);
        	 		glEnd();
                    }
                    if (l==surfacepolylinepoints_vectorlength-1){
					glBegin(GL_TRIANGLES);/*Linien lesen aus View-Linien-Vektor und darstellen*/
					point_id  = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->id;//CC
					x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					glVertex3d(x,y,z);
					l++;
					x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->x;
					y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->y;
					z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->z;    			
					glVertex3d(x,y,z);
					l--;
					x = view_surfaces_vector[j]->surface_midpoint_x;
					y = view_surfaces_vector[j]->surface_midpoint_y;
					z = view_surfaces_vector[j]->surface_midpoint_z;
					glVertex3d(x,y,z);
        	 		glEnd();
                    }

				}
    		 }
			 

		 }
		 			 glDisable(GL_BLEND);

		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
   	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 }
		 if (PointOnOff==1) DisplayPoints();
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		break;
	case 5://GLI-Volumes
		break;

	case 10://RFI-Nodes 
		if (rfi_nodes_vectorlength < 200) glPointSize(4.0); 
        else glPointSize(1.0);
		glBegin(GL_POINTS);/*MSH-Knoten lesen*/
		for (j=0;j<rfi_nodes_vectorlength;j++)
         {
			 glColor3d(0.0,0.0,1.0);
			 glVertex3d(view_nodes_vector[j]->x,view_nodes_vector[j]->y,view_nodes_vector[j]->z);
		 }

 		 if (rfi_nodes_vectorlength==0)
		 {
           glColor3f(1.0,1.0,1.0);
		   glRasterPos3d(x_mid,y_mid,z_mid);
		   Text2D("NO MSH-Nodes !");		   

		 }


		 glEnd();
		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);		 
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist ),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointOnOff==1) DisplayPoints();
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		break;

	
	

	case 11://RFI-Elements
		for (j=0;j<rfi_elements_vectorlength;j++)
         {
			 /*LINES = 1*/ 
			 if (view_elements_vector[j]->element_type == 1)
			 {
			 glBegin(GL_LINES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(1.0,1.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
		     glEnd();
			 }
			 /*RECTANGLES = 2*/ 
			 if (view_elements_vector[j]->element_type == 2)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(1.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);            
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);            
			 glEnd();
			 }
			 /*HEXAHEDRA = 3*/ 
			 if (view_elements_vector[j]->element_type == 3)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(1.0,1.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
             glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);    	     
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);
			 glEnd();
			 }
			 /*TRIANGLES = 4*/ 
			 if (view_elements_vector[j]->element_type == 4)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(1.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
     		 }
			 /*TETRAHEDRAS = 5*/ 
			 if (view_elements_vector[j]->element_type == 5)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,1.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
 			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
  			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
		     glEnd();
			 }
			 /*PRISMS = 6*/ 
			 if (view_elements_vector[j]->element_type == 6)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.3,1.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 }
		 } 
		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointOnOff==1) DisplayPoints();
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
             ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
         
		break;
	
		case 12://RFI-ElementQuality		
		
		for (j=0;j<rfi_elements_vectorlength;j++)
         {
			 /*LINES = 1*/ 
			 if (view_elements_vector[j]->element_type == 1)
			 {
			 glBegin(GL_LINES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
		     glEnd();
			 }
			 /*RECTANGLES = 2*/ 
			 if (view_elements_vector[j]->element_type == 2)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);            
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);            
			 glEnd();
			 }
			 /*HEXAHEDRA = 3*/ 
			 if (view_elements_vector[j]->element_type == 3)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
             glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);    	     
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);
			 glEnd();
			 }
			 /*TRIANGLES = 4*/ 
			 if (view_elements_vector[j]->element_type == 4)
			 {
			 //glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 /*glColor3d(0.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();		*/

			 glColor3d(1.00-(view_elements_vector[j]->quality_factor),view_elements_vector[j]->quality_factor,0.0);
			 //glColor3d(1-view_elements_vector[j]->quality_factor,view_elements_vector[j]->quality_factor,0.0);

			 glEnable(GL_BLEND);
			 glBegin(GL_TRIANGLES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();     	
     		 }
			 /*TETRAHEDRAS = 5*/ 
			 if (view_elements_vector[j]->element_type == 5)
			 {
	 	 	 glEnable(GL_BLEND);
			 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			 if (view_elements_vector[j]->quality_factor >= quality_range_min && 
				 view_elements_vector[j]->quality_factor <= quality_range_max)
			 {
 			 glColor4d(1.00-(view_elements_vector[j]->quality_factor),
				       0.0,view_elements_vector[j]->quality_factor,0.9); //==high 1=no transparenz

    		 glBegin(GL_TRIANGLES); 
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
			 glBegin(GL_TRIANGLES); 
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
 			 glBegin(GL_TRIANGLES); 
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
  			 glBegin(GL_TRIANGLES); 
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
		     glEnd();

			 glColor3d(0.0,0.0,0.0);
			 glBegin(GL_LINE_LOOP);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
			 glBegin(GL_LINE_LOOP); 
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
		     glEnd();
 			 glBegin(GL_LINE_LOOP); 
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
		     glEnd();
  			 glBegin(GL_LINE_LOOP); 
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
		     glEnd();

			 }
			 }

			 /*PRISMS = 6*/ 
			 if (view_elements_vector[j]->element_type == 6)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 }
		 } 
		   
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointOnOff==1) DisplayPoints();
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		break;
	
		case 20://Graph		
			 DrawOGLGraph();

		 if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x= 0;
			 trans_y= 0;
             scale_x=0.5;
			 scale_y=0.5;
			 scale_z=0.5;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);	 
			 glOrtho(-ClipSize*(fabs(x_graph_min))-((x_graph_max-x_graph_min)/100),+ClipSize*0.5*(fabs(x_graph_max))+((x_graph_max-x_graph_min)/100),\
				     -ClipSize*(fabs(y_graph_min))-((y_graph_max-y_graph_min)/50),+ClipSize*(fabs(y_graph_max))+((y_graph_max-y_graph_min)/2.5),\
					 -ClipSize*(1),+ClipSize*(1));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO
             glTranslated(trans_x,trans_y/2,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/
		break;
	case 13://FEM-VIEW		
		// OK/RN/TK 	
        int i=0, j=0, k=0;
        value_min = m_dUmin;
		value_max = m_dUmax;
		CGraphics m_graphics;

        if (m_iDisplayBC) 
        {
            CGLPoint *m_point=NULL;
            CGLPolyline *m_polyline=NULL;
            Surface *m_surface=NULL;
            int xpixel(double);
            int ypixel(double);
            CBoundaryCondition *m_bc = NULL;
            list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
              while(p_bc!=bc_list.end()) 
              {
                m_bc = *p_bc;
                /*POINTS-BC*/ 
                if(m_bc->geo_type_name.compare("POINT")==0) 
                {                   
                    m_point = GEOGetPointByName(m_bc->geo_name);//CC
                }
                /*POLYLINES-BC*/ 
                if(m_bc->geo_type_name.compare("POLYLINE")==0) 
                {
                    m_polyline = GEOGetPLYByName(m_bc->geo_name);//CC
                    CString namePLY_BC = m_polyline->name.data();
                    if(m_polyline) 
                    {   glPointSize(5);
                        glBegin(GL_POINTS) ;              
                        glColor3f(0,0,0) ;  
		                glVertex3d(0.0,0.0,0.0) ;
                        glEnd() ;
                        
                        glColor3f(1,0,0) ;  
                        polylines_vectorlength = (int)view_polylines_vector.size();
		                for (j=0;j<polylines_vectorlength;j++)
                        {
                            
			                polylinepoints_vectorlength = (int)view_polylines_vector[j]->polyline_point_vector.size();
                            CString namePLY = view_polylines_vector[j]->polylinename;
                            CString pcs_name = m_bc->pcs_type_name.data();
                            if (namePLY == namePLY_BC && m_strQuantityName == pcs_name)
                            {
                                glBegin(GL_LINE_STRIP);
                                for (k=0;k<polylinepoints_vectorlength;k++)
                                {
                                 glVertex3d(view_polylines_vector[j]->polyline_point_vector[k]->x,view_polylines_vector[j]->polyline_point_vector[k]->y,view_polylines_vector[j]->polyline_point_vector[k]->z);                                   
			                    }
                                glEnd();
                            }
                        }
                    }
                }

                /*SURFACES-BC*/ 
                CString nameSFC_BC, pcs_name, nameSFC ;


                if(m_bc->geo_type_name.compare("SURFACE")==0) 
                {  
                   m_surface = GEOGetSFCByName(m_bc->geo_name);//CC
                   nameSFC_BC = m_surface->name.data();
                   pcs_name = m_bc->pcs_type_name.data();
                    vector<long>bc_nodes_vector;
                   if(m_surface)
                   { 
                    if (m_strQuantityName == pcs_name)
                    {    
                    m_surface = NULL;
                    m_surface = GEOGetSFCByName(m_bc->geo_name);//CC
                    GetMSHNodesOnSurface(m_surface,bc_nodes_vector);//CC
                    //bc_nodes_vector = m_surface->GetMSHNodesClose();
                    long nodes_vector_length = (int)bc_nodes_vector.size();
                      for(i=0;i<nodes_vector_length;i++){
                         long wert = bc_nodes_vector[i];
                         glPointSize(4);
                         glBegin(GL_POINTS);/*MSH-Knoten lesen*/
               			 glColor3d(0.0,0.0,1.0);
			             glVertex3d(view_nodes_vector[wert]->x,view_nodes_vector[wert]->y,view_nodes_vector[wert]->z);
                         glEnd();
                      }
                     bc_nodes_vector.clear();
                    }
                   }

                }
                if(m_bc->geo_type_name.compare("SURFACE")==0) 
                {   
                    
                    m_surface = GEOGetSFCByName(m_bc->geo_name);//CC
                    nameSFC_BC = m_surface->name.data();
                    if(m_surface) 
                    {
                    vector<CGLPolyline*>::iterator p_sfc_ply = m_surface->polyline_of_surface_vector.begin();
                    while(p_sfc_ply!=m_surface->polyline_of_surface_vector.end()) 
                    {
                        m_polyline = *p_sfc_ply; 
                        if(m_polyline) {
                   	 	 glEnable(GL_BLEND);
		                 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                         glColor4d(1.0,0.0,0.0,0.3);
                         //surface_vectorlength = view_surfaces_vector.size();
		                
                        for (j=0;j<surface_vectorlength;j++)
                        {              			 
			                surfacepolyline_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector.size();
                            nameSFC = view_surfaces_vector[j]->surfacename.data();
                            pcs_name = m_bc->pcs_type_name.data();
                            if (nameSFC == nameSFC_BC && m_strQuantityName == pcs_name){           
                            for (k=0;k<surfacepolyline_vectorlength;k++)
                            {			                               			
				                surfacepolylinepoints_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector.size();		
                                for (l=0;l<surfacepolylinepoints_vectorlength;l++)
				                {
                                    if (l<surfacepolylinepoints_vectorlength-1){
					                glBegin(GL_TRIANGLES);/*Linien lesen aus View-Linien-Vektor und darstellen*/
					                point_id  = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->id;//CC
					                x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					                y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					                z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					                glVertex3d(x,y,z);
					                l++;
					                x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					                y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					                z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					                glVertex3d(x,y,z);
					                l--;
					                x = view_surfaces_vector[j]->surface_midpoint_x;
					                y = view_surfaces_vector[j]->surface_midpoint_y;
					                z = view_surfaces_vector[j]->surface_midpoint_z;
					                glVertex3d(x,y,z);
        	 		                glEnd();
                                    }
                                    if (l==surfacepolylinepoints_vectorlength-1){
					                glBegin(GL_TRIANGLES);/*Linien lesen aus View-Linien-Vektor und darstellen*/
					                point_id  = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->id;//CC
					                x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->x;
					                y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->y;
					                z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l]->z;    			
					                glVertex3d(x,y,z);
					                l++;
					                x = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->x;
					                y = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->y;
					                z = view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[0]->z;    			
					                glVertex3d(x,y,z);
					                l--;
					                x = view_surfaces_vector[j]->surface_midpoint_x;
					                y = view_surfaces_vector[j]->surface_midpoint_y;
					                z = view_surfaces_vector[j]->surface_midpoint_z;
					                glVertex3d(x,y,z);
        	 		                glEnd();
                                    }
                                }
                            }
                            }
                        }
                        }
                          glDisable(GL_BLEND);
                          ++p_sfc_ply;
                    }
                    }
                }
                ++p_bc;
              }    
        }

		if(m_iDisplayIsolines)
		{


		}
		if(m_iDisplayIsosurfaces)
		{
		for (j=0;j<rfi_elements_vectorlength;j++){
          m_ele = view_elements_vector[j];
          switch(m_ele->element_type){
            case 4: // triangle
	          glBegin(GL_TRIANGLES) ;
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x1,m_ele->y1,m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x2,m_ele->y2,m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x3,m_ele->y3,m_ele->z3) ;
              glEnd() ;

              break;

            case 2: // rectangle
	          glBegin(GL_QUADS) ;
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_ele->x1,m_ele->y1,m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
			  if (value_norm<0.25){
				  Red   =0.0;
				  Blue  =1.0;
				  Green =value_norm*4;
			  }
			  if (value_norm>=0.25 && value_norm<0.5){
				  Red   =0.0;
				  Green =1.0;
				  Blue  =1.0-((value_norm-0.25)*4);
			  }
			  if (value_norm>=0.5 && value_norm<0.75){
				  Blue  =0.0;
				  Green =1.0;
				  Red   = (value_norm-0.5)*4;
			  }
		      if (value_norm>=0.75){
				  Blue  =0.0;
				  Red   =1.0;
				  Green = 1.0-((value_norm-0.75)*4);
			  }	 	 
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_ele->x2,m_ele->y2,m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
		      glVertex3f(m_ele->x3,m_ele->y3,m_ele->z3) ;
			  value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor4f(Red,Green,Blue,0.3) ;
              glColor3f(Red,Green,Blue) ;
  		      glVertex3f(m_ele->x4,m_ele->y4,m_ele->z4) ;
              glEnd() ;
			  break;	

             case 5: // tetrahedra
	          glBegin(GL_TRIANGLES) ; /*TRI1*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x1,m_ele->y1,m_ele->z1) ;
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x2,m_ele->y2,m_ele->z2) ;
              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x3,m_ele->y3,m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI2*/ 
              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x1,m_ele->y1,m_ele->z1) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x4,m_ele->y4,m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x3,m_ele->y3,m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI3*/ 
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x2,m_ele->y2,m_ele->z2) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x4,m_ele->y4,m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber3,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x3,m_ele->y3,m_ele->z3) ;
              glEnd() ;

	          glBegin(GL_TRIANGLES) ; /*TRI4*/ 
              value = GetNodeVal(m_ele->nodenumber2,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
              glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x2,m_ele->y2,m_ele->z2) ;

              value = GetNodeVal(m_ele->nodenumber4,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
		      glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x4,m_ele->y4,m_ele->z4) ;

              value = GetNodeVal(m_ele->nodenumber1,PCSGetNODValueIndex((string)m_strQuantityName,1));
              value_norm = (value-value_min)/(value_max-value_min);
                Red =   Get_Red_Value(value_norm);
                Green = Get_Green_Value(value_norm);
                Blue =  Get_Blue_Value(value_norm);
			  glColor3f(Red,Green,Blue) ;
		      glVertex3f(m_ele->x1,m_ele->y1,m_ele->z1) ;
              glEnd() ;

              break;

		  }
		}
		}
        if (m_iDisplayMSH)
        {
        //----------------------------------------------------------------
		for (j=0;j<rfi_elements_vectorlength;j++)
         {
			 /*LINES = 1*/ 
		 if(m_iDisplayMSHLine)
		 {

			 if (view_elements_vector[j]->element_type == 1)
			 {
			 glBegin(GL_LINES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.5,0.5,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
		     glEnd();
			 }
		 }
		 if(m_bDisplayMSHQuad)
		 {
			 /*RECTANGLES = 2*/ 
			 if (view_elements_vector[j]->element_type == 2)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.5,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,(view_elements_vector[j]->z1));
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,(view_elements_vector[j]->z2));
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,(view_elements_vector[j]->z3));            
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,(view_elements_vector[j]->z4));            
			 glEnd();
			 }
		 }
		 if(m_bDisplayMSHHex)
		 {
			 /*HEXAHEDRA = 3*/ 
			 if (view_elements_vector[j]->element_type == 3)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.5,0.0,0.5);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
             glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);    	     
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x7,view_elements_vector[j]->y7,view_elements_vector[j]->z7);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x8,view_elements_vector[j]->y8,view_elements_vector[j]->z8);
			 glEnd();
			 }
		 }
		 if(m_iDisplayMSHTri)
		 {
			 /*TRIANGLES = 4*/ 
			 if (view_elements_vector[j]->element_type == 4)
			 {
			 glBegin(GL_LINE_LOOP); // Linien lesen aus View-Linien-Vektor und darstellen
			 glColor3d(0.0,0.0,0.5);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
     		 }
		 }
		if(m_bDisplayMSHTet)
		{
			 /*TETRAHEDRAS = 5*/ 
			 if (view_elements_vector[j]->element_type == 5)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.0,0.5,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
 			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
		     glEnd();
  			 glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
		     glEnd();
			 }
		}
		
		if(m_bDisplayMSHPris)
		{
			 /*PRISMS = 6*/ 
			 if(m_bDisplayMSHPris){
			 if (view_elements_vector[j]->element_type == 6)
			 {
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glColor3d(0.5,0.0,0.0);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
     	     glEnd();
			 glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x1,view_elements_vector[j]->y1,view_elements_vector[j]->z1);
			 glVertex3d(view_elements_vector[j]->x4,view_elements_vector[j]->y4,view_elements_vector[j]->z4);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x2,view_elements_vector[j]->y2,view_elements_vector[j]->z2);
			 glVertex3d(view_elements_vector[j]->x5,view_elements_vector[j]->y5,view_elements_vector[j]->z5);
			 glEnd();
			 glBegin(GL_LINES);
             glVertex3d(view_elements_vector[j]->x3,view_elements_vector[j]->y3,view_elements_vector[j]->z3);
			 glVertex3d(view_elements_vector[j]->x6,view_elements_vector[j]->y6,view_elements_vector[j]->z6);
			 glEnd();
			 }
		 } 
        }
		 }
         }
        //}
         if (view_counter==NULL) /*First View*/ 
		 {
			 trans_x=trans_y=trans_z=0.0;
             scale_x=scale_y=scale_z=0.9;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);
	         glOrtho(-ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)),\
				     -ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)),\
					 -ClipSize*(fabs(min)+fabs(max)),+ClipSize*(fabs(min)+fabs(max)));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 view_counter++;
		 } 
		 if (PointNumberOnOff==1) DisplayPointNumbers();
		 if (DoublePointOnOff==1) DisplayDoublePoints();
		 if (PolylineNameOnOff==1) DisplayPolylineNames();
		 if (NodeNumberOnOff==1) DisplayNodeNumbers();
		 if (ElementNumberOnOff==1) DisplayElementNumbers();
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/ 
		break;
    };

	switch(sceneselect) //Statisch aber mitrotierend
	{
	case 0: //nicht belegt
		break;

	default:
    //case 100:
		     glPushMatrix();
			 glMatrixMode(GL_PROJECTION);
	
			 glBegin(GL_LINES); 
             glColor3d(1.0,1.0,1.0);
             /*x-Achse*/ 
			 glVertex3d((-0.80*dist/scale_x),( 0.00*dist/scale_y),( 0.00*dist/scale_y));
			 glVertex3d(( 0.80*dist/scale_x),( 0.00*dist/scale_y),( 0.00*dist/scale_y));
             /*y-Achse*/ 
			 glVertex3d(( 0.00*dist/scale_x),(-0.80*dist/scale_y),( 0.00*dist/scale_y));
			 glVertex3d(( 0.00*dist/scale_x),( 0.80*dist/scale_y),( 0.00*dist/scale_y));
             /*z-Achse*/ 
			 glVertex3d(( 0.00*dist/scale_x),( 0.00*dist/scale_y),(-0.80*dist/scale_y));
			 glVertex3d(( 0.00*dist/scale_x),( 0.00*dist/scale_y),( 0.80*dist/scale_y));
             glEnd(); 

 			 glBegin(GL_TRIANGLES); 
             glColor3d(1.0,1.0,1.0);
             /*x-Achse*/ 
			 glVertex3d(( 0.80*dist/scale_x),( 0.00*dist/scale_y),( 0.00*dist/scale_y));
             glVertex3d(( 0.7*dist/scale_x),( 0.015*dist/scale_y),( 0.00*dist/scale_y));
			 glVertex3d(( 0.70*dist/scale_x),(-0.015*dist/scale_y),( 0.00*dist/scale_y));
			 /*y-Achse*/ 
			 glVertex3d(( 0.00*dist/scale_x),( 0.80*dist/scale_y),( 0.00*dist/scale_y));
			 glVertex3d(( 0.015*dist/scale_x),( 0.70*dist/scale_y),( 0.00*dist/scale_y));
			 glVertex3d((-0.015*dist/scale_x),( 0.70*dist/scale_y),( 0.00*dist/scale_y));
			 /*z-Achse*/ 
			 glVertex3d(( 0.00*dist/scale_x),( 0.00*dist/scale_y),( 0.80*dist/scale_y));
			 glVertex3d(( 0.015*dist/scale_x),( 0.00*dist/scale_y),( 0.70*dist/scale_y));
			 glVertex3d((-0.015*dist/scale_x),( 0.00*dist/scale_y),( 0.70*dist/scale_y));
             glEnd(); 
         
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();

 			 glPushAttrib(GL_ENABLE_BIT);
			 glPopAttrib();
			 view_counter++;
			 ZoomAndMove(1);

		break;
	};

    //Aktivieren falls alter view style:
    /*trackball.DrawBall();
	glPopMatrix();
	glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		DrawStockDispLists();
	glPopAttrib();
	glPopMatrix();
    */

	switch(sceneselect) //Statisch
	{

	case 13: //Legende

        if(m_iDisplayIsosurfaces)
		{
			 static char number[10];
			 double value_max = m_dUmax;
			 double value_min = m_dUmin;
			 double legendewert;

			 glPushMatrix();
			 glMatrixMode(GL_PROJECTION);

			 /*Titel*/ 
			 glColor3d(0.0,0.0,1.0);
  			 glRasterPos2d((0.00*dist/scale_x)-trans_x,(-0.97*dist/scale_y)-trans_y);            
			 Text2D(m_strQuantityName);

			 
			 /*Zeitschrittanzeige*/ 
			 glColor3d(0.0,0.0,0.0);
   			 glRasterPos2d(( 0.40*dist/scale_x)-trans_x,( 0.85*dist/scale_y)-trans_y);            
			 Text2D("TIME:");
   			 glRasterPos2d(( 0.65*dist/scale_x)-trans_x,( 0.85*dist/scale_y)-trans_y);            
             sprintf(number,"%lg",aktuelle_zeit);          
			 Text2D(number);

			 /*Legende Rahmen und Unterteilung*/ 
 			 glBegin(GL_LINE_LOOP); 
             glColor3d(0.0,0.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);            
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);              
        	 glEnd(); 	
  			 glBegin(GL_LINES); 
             glColor3d(0.0,0.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
   			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
   			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
			 glEnd(); 	

	
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,(-0.98*dist/scale_y)-trans_y);            
             legendewert = value_min;
			 sprintf(number,"%lg",legendewert);          
			 Text2D(number);
			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,(-0.48*dist/scale_y)-trans_y);          
    		 legendewert = value_min+((value_max-value_min)*1/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);		
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.02*dist/scale_y)-trans_y);            
    		 legendewert = value_min+((value_max-value_min)*2/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.52*dist/scale_y)-trans_y);            
    		 legendewert = value_min+((value_max-value_min)*3/4);
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
   			 glRasterPos2d((-0.945*dist/scale_x)-trans_x,( 0.92*dist/scale_y)-trans_y);            
    		 legendewert = value_max;
			 sprintf(number,"%lg",legendewert);
			 Text2D(number);
             
			 /*Legende Farben*/ 
		     glBegin(GL_QUADS); 
             /*1.Unit:*/ 
			 glColor3d(0.0,0.0,1.0);
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-1.00*dist/scale_y)-trans_y);
             glColor3d(0.0,1.0,1.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             /*2.Unit:*/ 
			 glColor3d(0.0,1.0,1.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.50*dist/scale_y)-trans_y);            
             glColor3d(0.0,1.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,(-0.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,(-0.00*dist/scale_y)-trans_y);            
             /*3.Unit:*/ 
             glColor3d(0.0,1.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.00*dist/scale_y)-trans_y);            
             glColor3d(1.0,1.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             /*4.Unit:*/ 
             glColor3d(1.0,1.0,0.0);
			 glVertex2d((-0.95*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glVertex2d((-1.00*dist/scale_x)-trans_x,( 0.50*dist/scale_y)-trans_y);            
             glColor3d(1.0,0.0,0.0);
			 glVertex2d((-1.00*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
             glVertex2d((-0.95*dist/scale_x)-trans_x,( 1.00*dist/scale_y)-trans_y);            
			 glEnd(); 	
			 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
			 glPushAttrib(GL_ENABLE_BIT);
			 glPopAttrib();
			 view_counter++;
        }


        if (m_iDisplayBC) 
        {
			 glPushMatrix();
			 glMatrixMode(GL_PROJECTION);

			 /*Titel*/ 
			 glColor3d(1.0,0.0,0.0);
  			 glRasterPos2d((0.00*dist/scale_x)-trans_x,(-0.97*dist/scale_y)-trans_y);            
			 Text2D(m_strQuantityName);

             
             glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
			 glPushAttrib(GL_ENABLE_BIT);
			 glPopAttrib();
			 view_counter++;

        }
       break;
	};
}

/////////////////////////////////////////////////////////////////////////////
// COGLView message handlers

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(nFlags & MK_CONTROL) trackball.UseConstraints(CAMERA_AXES);
	else if(nFlags & MK_SHIFT) trackball.UseConstraints(BODY_AXES);
		 else trackball.UseConstraints(NO_AXES);
// remember where we clicked
	MouseDownPoint=point;
	trackball.MouseDown(point);
// capture mouse movements even outside window borders
	SetCapture();
// activate bounding box
    boundingbox = 1;
// redraw the view	//
	Invalidate(TRUE);

}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(nFlags & MK_CONTROL) trackball.UseConstraints(CAMERA_AXES);
	else if(nFlags & MK_SHIFT) trackball.UseConstraints(BODY_AXES);
		 else trackball.UseConstraints(NO_AXES);
// remember where we clicked
	MouseDownPoint=point;
	trackball.MouseDown(point);
// capture mouse movements even outside window borders
	SetCapture();
// activate bounding box
    boundingbox = 1;
// redraw the view	//
	Invalidate(TRUE);
}

void COGLView::OnRButtonUp(UINT nFlags, CPoint point) 
{
// forget where we clicked
	MouseDownPoint=CPoint(0,0);
	if(nFlags)trackball.MouseUp(point);
// release mouse capture
	ReleaseCapture();
// deactivate bounding box
    boundingbox = 0;
// redraw the view	//
	Invalidate(TRUE);
}

void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{  
// forget where we clicked
	MouseDownPoint=CPoint(0,0);
	if(nFlags)trackball.MouseUp(point);
// release mouse capture
	ReleaseCapture();
// deactivate bounding box
    boundingbox = 0;
// redraw the view	//
	Invalidate(TRUE);
}

void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
 dx = point.x - m_ptLast.x;
 dy = point.y - m_ptLast.y;
 m_ptLast = point;

 if ((nFlags & MK_LBUTTON) && (!(nFlags & MK_RBUTTON)))
 {
 	if(nFlags)	trackball.MouseMove(point);
	// check if we have captured the mouse
	if (GetCapture()==this)
	{
	// increment the object rotation angles
	//		X_Angle+=double(point.y-MouseDownPoint.y)/3.6;
	//		Y_Angle+=double(point.x-MouseDownPoint.x)/3.6;
	// redraw the view
		Invalidate(TRUE);
	// remember the mouse point
	//		MouseDownPoint=point;
	}
 }

if ((nFlags & MK_RBUTTON)&& (!(nFlags & MK_LBUTTON))) 
{
   if (dy<0 && m_ptLast.y!=0.0) OnKeyDown(VK_SUBTRACT, NULL, nFlags); 
   if (dy>0 && m_ptLast.y!=0.0) OnKeyDown(VK_ADD, NULL, nFlags); 
}

 if ((nFlags & MK_RBUTTON) && (nFlags & MK_LBUTTON)) 
 {
   if (dy!=0 && m_ptLast.y!=0.0) OnKeyDown(VK_LEFT, NULL, nFlags);
 }

}

BOOL COGLView::OnMouseWheel(UINT nFlags,short zDelta,CPoint point)
{
   point=point;
   dy=zDelta/100;
   if (dy<0) OnKeyDown(VK_SUBTRACT, NULL, nFlags); 
   if (dy>0) OnKeyDown(VK_ADD, NULL, nFlags); 

   return FALSE;
}

void COGLView::OnHelpOglinfo() 
{
// istantiate dialog
	COGLInfoDlg dlg;
// retrieve infos
	GLInfoStruct info;
	info=GetInformation();
// prepare dialog
	dlg.m_accel=info.acceleration;
	dlg.m_extensions=info.glextensions;
	dlg.m_rend=info.renderer;
	dlg.m_vendor=info.vendor;
	dlg.m_version=info.glversion;
	dlg.m_gluext=info.gluextensions;
	dlg.m_gluver=info.gluversion;
	dlg.m_detailstext.Format("Color buffer: %i bit red, %i bit green, %i bit blue, %i bit alpha.\nDepth buffer: %i bit.\nStencil buffer: %i bit.",info.red_bits,info.green_bits,info.blue_bits,info.alpha_bits,info.depth_bits,info.stencil_bits);
// convert the spaces in newlines (in extensions fields) 
	int pos=0;
	while ( (pos=dlg.m_extensions.Find(" "))!=-1 )
	{
		// substitute space with LF
		dlg.m_extensions.SetAt(pos,'\n');
		// insert a CR before LF
		dlg.m_extensions.Insert(pos,'\r');
	}
	pos=0;
	while ( (pos=dlg.m_gluext.Find(" "))!=-1 )
	{
		// substitute space with LF
		dlg.m_gluext.SetAt(pos,'\n');
		// insert a CR before LF
		dlg.m_gluext.Insert(pos,'\r');
	}
// show dialog
	dlg.DoModal();	
}

void COGLView::OnEnvirFlat() 
{
	BeginGLCommands();
// set flat shading
	glShadeModel(GL_FLAT);
	EndGLCommands();
// redraw
	Invalidate(TRUE);
}


void COGLView::OnColoredElevation() 
{
	if(m_z_value_color==1) m_z_value_color=0;
	else m_z_value_color=1;
	Invalidate(TRUE);


}

void COGLView::OnEnvirLighting() 
{
	//double light1 [4]= { 1, 1, 1, 1};
	
	//const GLfloat* angle = 180.0;
 
	BeginGLCommands();
	// toggle lighting
	if(glIsEnabled(GL_LIGHTING)) glDisable(GL_LIGHTING);
	else {
    //glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
    glEnable(GL_LIGHTING);
	}
	EndGLCommands();




// redraw
	Invalidate(TRUE);
}

void COGLView::OnEnvirSmooth() 
{
	BeginGLCommands();
// set smooth shading
	glShadeModel(GL_SMOOTH);
	EndGLCommands();
// redraw
	Invalidate(TRUE);
}

void COGLView::OnUpdateEnvirLighting(CCmdUI* pCmdUI) 
{
	BeginGLCommands();
// update menu check
	pCmdUI->SetCheck(glIsEnabled(GL_LIGHTING));
	EndGLCommands();
}

void COGLView::OnUpdateColoredElevation(CCmdUI* pCmdUI) 
{
	if (m_z_value_color==1) pCmdUI->SetCheck(1);
    else pCmdUI->SetCheck(0);
}

void COGLView::OnUpdateEnvirFlat(CCmdUI* pCmdUI) 
{
	BeginGLCommands();
// check shading mode
	GLint val;
	glGetIntegerv(GL_SHADE_MODEL,&val);
// set check accordingly
	pCmdUI->SetCheck(val==GL_FLAT);
	EndGLCommands();
}

void COGLView::OnUpdateEnvirSmooth(CCmdUI* pCmdUI) 
{
	BeginGLCommands();
// check shading mode
	GLint val;
	glGetIntegerv(GL_SHADE_MODEL,&val);
// set check accordingly
	pCmdUI->SetCheck(val==GL_SMOOTH);
	EndGLCommands();
}


void COGLView::OnUpdate3DViewPoints(CCmdUI* pCmdUI) 
{
	if (PointOnOff==1) pCmdUI->SetCheck(1);
	if (PointOnOff==0) pCmdUI->SetCheck(0);
}


void COGLView::OnUpdate3DViewPointNumbers(CCmdUI* pCmdUI) 
{
	if (PointNumberOnOff==1) pCmdUI->SetCheck(1);
	if (PointNumberOnOff==0) pCmdUI->SetCheck(0);
}

void COGLView::OnUpdate3DViewDoublePoints(CCmdUI* pCmdUI) 
{
	if (DoublePointOnOff==1) pCmdUI->SetCheck(1);
	if (DoublePointOnOff==0) pCmdUI->SetCheck(0);
}


void COGLView::OnUpdate3DViewPolylineNames(CCmdUI* pCmdUI) 
{
	if (PolylineNameOnOff==1) pCmdUI->SetCheck(1);
	if (PolylineNameOnOff==0) pCmdUI->SetCheck(0);

}



void COGLView::OnUpdate3DViewRFINodeNumbers(CCmdUI* pCmdUI) 
{
	if (NodeNumberOnOff==1) pCmdUI->SetCheck(1);
	if (NodeNumberOnOff==0) pCmdUI->SetCheck(0);
}

void COGLView::OnUpdate3DViewRFIElementNumbers(CCmdUI* pCmdUI) 
{
	if (ElementNumberOnOff==1) pCmdUI->SetCheck(1);
	if (ElementNumberOnOff==0) pCmdUI->SetCheck(0);
}


void COGLView::OnSizeGL(int cx, int cy) 
{
	//afxDump<<"client area: "<<cx<<" x "<<cy<<"\n";
	trackball.ClientAreaResize(CRect(0,0,cx,cy));
	COGLEnabledView::OnSizeGL(cx,cy);
}

void COGLView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar==VK_UP) zoom_control = 2;
	if (nChar==VK_DOWN)zoom_control = 1;
	if (nChar==VK_RIGHT)zoom_control = 4;
    if (nChar==VK_LEFT)	zoom_control = 3;
	if (nChar==VK_ADD || nChar==VK_OEM_PLUS)zoom_control = 5;
	if (nChar==VK_SUBTRACT || nChar==VK_OEM_MINUS)zoom_control = 6;
    if (nChar==VK_NUMPAD0)zoom_control = 7;
    if (nChar==VK_HOME)zoom_control = 8;


	if(nChar==VK_SPACE) trackball.ToggleMethod();
	trackball.Key(nChar);
	COGLEnabledView::OnKeyDown(nChar, nRepCnt, nFlags); 
	Invalidate(TRUE);
}
void COGLView::OnStartOpenGLExamples() 
{
	//CGLEnabledView::OnDraw(CDC* pDC)
	sceneselect=100;
    Invalidate(TRUE);
}


void COGLView::On3DViewPoints() 
{
	GetGLIPointsforView();
	sceneselect=1;
    Invalidate(TRUE);
}

void COGLView::On3DViewAddPoints() 
{
	GetGLIPointsforView();
	if(PointOnOff==1) PointOnOff=0;
	else PointOnOff=1;
	Invalidate(TRUE);
}

void COGLView::On3DViewLines() 
{
    GetGLILinesforView();
	sceneselect=2;
    Invalidate(TRUE);
}
void COGLView::On3DViewPolylines() 
{
	GetGLIPolylinesforView();
	sceneselect=3;
    Invalidate(TRUE);
}
void COGLView::On3DViewSurfaces() 
{
	GetGLISurfacesforView();
	sceneselect=4;
    Invalidate(TRUE);
}
void COGLView::On3DViewVolumes() 
{
	GetGLIPointsforView();
	sceneselect=0;
    Invalidate(TRUE);
}

void COGLView::On3DViewPointNumbers() 
{
 	if(PointNumberOnOff==1) PointNumberOnOff=0;
	else PointNumberOnOff=1;
	Invalidate(TRUE);
}

void COGLView::On3DViewDoublePoints() 
{
	if(DoublePointOnOff==1) {
		DoublePointOnOff=0;
	}
	else{

		CGS_Tolerance m_tolerance;
	    m_tolerance.DoModal();
		DoublePointOnOff=1;
	}
	Invalidate(TRUE);	
}


void COGLView::On3DViewPolylineNames() 
{
 	if(PolylineNameOnOff==1) PolylineNameOnOff=0;
	else PolylineNameOnOff=1;
	Invalidate(TRUE);
}


void COGLView::On3DViewRFINodes() 
{
	GetRFINodesforView();
	sceneselect=10;
    Invalidate(TRUE);
}

void COGLView::On3DViewRFIElements() 
{
  if(ElListSize()>0){ //OK
	GetRFIElementsforView();
  }
	sceneselect=11;
    graph_view_onoff = 0;
    Invalidate(TRUE);
}

void COGLView::On3DViewFEMModel() 
{
	//ON_COMMAND(ID_FILE_CLOSE,NULL);
	//CDocument::OnFileClose
    //PostMessage(WM_COMMAND, ID_FILE_CLOSE);

  CGraphics m_graphics;
  m_graphics.m_iDisplayMSH = m_iDisplayMSH;
  m_graphics.m_bDisplayMSHQuad = m_bDisplayMSHQuad;
  m_graphics.m_bDisplayMSHTet = m_bDisplayMSHTet;
  m_graphics.m_bDisplayMSHPris=m_bDisplayMSHPris;
  m_graphics.m_iDisplayMSHTri=m_iDisplayMSHTri;
  m_graphics.m_iDisplayMSHLine=m_iDisplayMSHLine;
  m_graphics.m_iDisplayIsosurfaces = m_iDisplayIsosurfaces;
  m_graphics.m_iDisplayIsolines = m_iDisplayIsolines;
  m_graphics.m_iDisplayBC = m_iDisplayBC;
  m_graphics.m_strQuantityName = m_strQuantityName; 
  m_graphics.DoModal();


  m_iDisplayMSH = m_graphics.m_iDisplayMSH;
  m_bDisplayMSHQuad = m_graphics.m_bDisplayMSHQuad;
  m_bDisplayMSHTet = m_graphics.m_bDisplayMSHTet;
  m_bDisplayMSHPris = m_graphics.m_bDisplayMSHPris;
  m_iDisplayMSHTri=m_graphics.m_iDisplayMSHTri;
  m_iDisplayMSHLine = m_graphics.m_iDisplayMSHLine;
  m_iDisplayIsosurfaces = m_graphics.m_iDisplayIsosurfaces; 
  m_iDisplayIsolines = m_graphics.m_iDisplayIsolines; 
  m_iDisplayBC = m_graphics.m_iDisplayBC;
  
  
  m_strQuantityName = m_graphics.m_strQuantityName; //Achtung: Daten vorhanden auch wenn sie im Debugger nicht angezeigt werden!!!
  m_dUmin = m_graphics.m_dUmin;
  m_dUmax = m_graphics.m_dUmax;

 
   GetGLIPolylinesforView();
   GetGLISurfacesforView();
   GetRFINodesforView();
   GetRFIElementsforView();
    sceneselect=13;
    graph_view_onoff = 0;
    Invalidate(TRUE);
}

void COGLView::On3DViewRFINodeNumbers() 
{
 	if(NodeNumberOnOff==1) NodeNumberOnOff=0;
	else NodeNumberOnOff=1;
	Invalidate(TRUE);
}

void COGLView::On3DViewRFIElementNumbers() 
{
 	if(ElementNumberOnOff==1) ElementNumberOnOff=0;
	else ElementNumberOnOff=1;
	Invalidate(TRUE);
}



int COGLView::ZoomAndMove(int switcher) 
{
			 ClipSize=1.00;
 if (switcher==1)
 {
         if (zoom_control==1) /*Move UP (not used for mouse move)*/ 
		 {
             scale_x =  scale_x - 0.000;
			 scale_y =  scale_y - 0.000;
			 scale_z =  scale_z - 0.000;
			 trans_x +=  dx*dist/100;
			 trans_y +=  dy*dist/100;
     		 trans_z +=  trans_z;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();
			 current_x = trackball.currentQuat.v.vec[0];
			 current_y = trackball.currentQuat.v.vec[1];
			 current_z = trackball.currentQuat.v.vec[2];
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);
			 glTranslated(-trans_x,-trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
			 
		 } 

		 if (zoom_control==2) /*Move DOWN (not used for mouse move)*/ 
		 {
             scale_x =  scale_x - 0.000;
			 scale_y =  scale_y - 0.000;
			 scale_z =  scale_z - 0.000;
			 trans_x +=  dx*dist/100;
			 trans_y +=  dy*dist/100;
     		 trans_z +=  trans_z;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();
			 current_x = trackball.currentQuat.v.vec[0];
			 current_y = trackball.currentQuat.v.vec[1];
			 current_z = trackball.currentQuat.v.vec[2];
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);
			 glTranslated(-trans_x,-trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
		 } 

         if (zoom_control==3) /*Move Right.....general Translation!!!*/ 
		 {
             scale_x =  scale_x - 0.000;
			 scale_y =  scale_y - 0.000;
			 scale_z =  scale_z - 0.000;
			 trans_x +=  dx*dist/(100*scale_x);
			 trans_y +=  -dy*dist/(100*scale_y);
     		 trans_z +=  trans_z;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();
			 current_x = trackball.currentQuat.v.vec[0];
			 current_y = trackball.currentQuat.v.vec[1];
			 current_z = trackball.currentQuat.v.vec[2];
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);	 
			 glTranslated(trans_x,trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
		 } 
 
		 if (zoom_control==4) /*Move Left (not used for mouse move)*/ 
		 {
             scale_x =  scale_x - 0.000;
			 scale_y =  scale_y - 0.000;
			 scale_z =  scale_z - 0.000;
			 trans_x +=  dx*50;
			 trans_y +=  -dy*50;
     		 trans_z +=  trans_z;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();
			 current_x = trackball.currentQuat.v.vec[0];
			 current_y = trackball.currentQuat.v.vec[1];
			 current_z = trackball.currentQuat.v.vec[2];
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);	 
			 glTranslated(trans_x,trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
		 }  	
		 
		 if (zoom_control==5) /*Zoom IN*/ 
		 {
             scale_x +=  0.01 + dy/25;
			 scale_y +=  0.01 + dy/25;
			 scale_z +=  0.00;
			 trans_x =  trans_x - 0.00;
			 trans_z =  trans_z - 0.00;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();           
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);		 
			 glTranslated(trans_x,trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
		 } 
 
         if (zoom_control==6) /*Zoom OUT*/ 
		 {
			 
             scale_x -=  0.01 - dy/25;
			 scale_y -=  0.01 - dy/25;
			 //scale_x =  scale_x - 0.05;
			 //scale_y =  scale_y - 0.05;
			 scale_z -=  0.025;
			 trans_x =  trans_x - 0.00;
			 trans_y =  trans_y - 0.00;
			 trans_z =  trans_z - 0.00;
     		 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();           
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);
			 glTranslated(trans_x,trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
			 zoom_control=0;
             return 1;
		 } 

		 if (zoom_control==7) /*Automatic Rotation*/ 
		 {
			 int t=0;
			 for (t=0;t<1500000;t++)// Schleife für zeitliche Skalierung
			 {
             scale_x =  scale_x - 0.00;
			 scale_y =  scale_y - 0.00;
			 scale_z =  scale_z - 0.00;
			 trans_x =  trans_x - 0.00;
			 trans_y =  trans_y - 0.00;
			 trans_z =  trans_z - 0.00;
			 }
     		 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity();			 
			 trackball.Key(VK_NUMPAD6);
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 glScaled(scale_x,scale_y,scale_z);
			 glTranslated(((x_max+y_max)/2)*trans_x,0.00,0.00);
			 glTranslated(0.00,((x_max+y_max)/2)*trans_y,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     Invalidate(TRUE);
             return 1;
		 } 

		 if (zoom_control==8) /*Back to first view*/ 
		 {

			 trans_x=trans_y=0.0;
             scale_x=0.5;
			 scale_y=0.5;
			 scale_z=0.5;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);         
	         glOrtho(-ClipSize*(dist),+ClipSize*(dist),\
				     -ClipSize*(dist),+ClipSize*(dist),\
					 -ClipSize*(dist),+ClipSize*(dist));   		           
			 if (graph_view_onoff == 1)
			 {
			 trans_x= 0;
			 trans_y= 0;
             scale_x=0.5;
			 scale_y=0.5;
			 scale_z=0.5;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);	 
			 glOrtho(-ClipSize*(fabs(x_graph_min))-((x_graph_max-x_graph_min)/100),+ClipSize*0.5*(fabs(x_graph_max))+((x_graph_max-x_graph_min)/100),\
				     -ClipSize*(fabs(y_graph_min))-((y_graph_max-y_graph_min)/50),+ClipSize*(fabs(y_graph_max))+((y_graph_max-y_graph_min)/2.5),\
					 -ClipSize*(1),+ClipSize*(1));   		           
             glTranslated(trans_x,trans_y/2,0.00);		 
			 }
			 
			 //((x_graph_max-x_graph_min)/50)
 	       
			 glScaled(scale_x,scale_y,scale_z);//TODO		 
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
			 zoom_control=0;
 		     Invalidate(TRUE);
			 view_counter++;
             return 1;
		 } 
        return 0;
 }
  else return 0;
 }

void COGLView::GetMidPoint() 
{
  x_mid = (x_min + x_max)/2;
  y_mid = (y_min + y_max)/2;
  z_mid = (z_min + z_max)/2;
}

void COGLView::GetMinMaxPoints() 
{
	int k=0;
    int j=0;
	vector<CGLPoint*> gli_points_vector_view;
    gli_points_vector_view.clear();
    gli_points_vector_view = GetPointsVector();//CC
    points_vectorlength = (int)gli_points_vector_view.size();
    /*GLI-POINTS*/ 
	if (gli_points_vector_view.size() != NULL)
	{
	x_min = gli_points_vector_view[0]->x;
	x_max = gli_points_vector_view[0]->x;
	y_min = gli_points_vector_view[0]->y;
	y_max = gli_points_vector_view[0]->y;
	z_min = gli_points_vector_view[0]->z;
	z_max = gli_points_vector_view[0]->z;  

    for (k=0;k<points_vectorlength;k++)
    {
	   x_count1 = gli_points_vector_view[k]->x;
	   y_count1 = gli_points_vector_view[k]->y;
	   z_count1 = gli_points_vector_view[k]->z;

          if (x_count1 < x_min) x_min = x_count1;
          if (x_count1 > x_max) x_max = x_count1;
		  if (y_count1 < y_min) y_min = y_count1;
          if (y_count1 > y_max) y_max = y_count1;
		  if (z_count1 < z_min) z_min = z_count1;
          if (z_count1 > z_max) z_max = z_count1;
	}
	}

    else
    {
        x_min = 0.0;
        x_max = 0.0;
        y_min = 0.0;
        y_max = 0.0;
        z_min = 0.0;
        z_max = 0.0;
    }
    /*GLI-Surfaces*/ 
    Surface * m_surface = NULL;
    vector<Surface*>::iterator ps = surface_vector.begin();
	surface_vectorlength = (long)surface_vector.size();//CC

	for (j=0;j<surface_vectorlength ;j++)
    {
   	  m_surface = *ps;
	  CString Surface_Name = m_surface->name.data();

        if (m_surface->TIN)
        {
           long nb_TIN_elements = (int)m_surface->TIN->Triangles.size();
           for(k=0;k<nb_TIN_elements;k++) 
           {
            x_count1 = m_surface->TIN->Triangles[k]->x[0];
            y_count1 = m_surface->TIN->Triangles[k]->y[0];
            z_count1 = m_surface->TIN->Triangles[k]->z[0];
            if (x_count1 < x_min) x_min = x_count1;
            if (x_count1 > x_max) x_max = x_count1;
		    if (y_count1 < y_min) y_min = y_count1;
            if (y_count1 > y_max) y_max = y_count1;
		    if (z_count1 < z_min) z_min = z_count1;
            if (z_count1 > z_max) z_max = z_count1;
            x_count1 = m_surface->TIN->Triangles[k]->x[1];
            y_count1 = m_surface->TIN->Triangles[k]->y[1];
            z_count1 = m_surface->TIN->Triangles[k]->z[1];
            if (x_count1 < x_min) x_min = x_count1;
            if (x_count1 > x_max) x_max = x_count1;
		    if (y_count1 < y_min) y_min = y_count1;
            if (y_count1 > y_max) y_max = y_count1;
		    if (z_count1 < z_min) z_min = z_count1;
            if (z_count1 > z_max) z_max = z_count1;
            x_count1 = m_surface->TIN->Triangles[k]->x[2];
            y_count1 = m_surface->TIN->Triangles[k]->y[2];
            z_count1 = m_surface->TIN->Triangles[k]->z[2];
            if (x_count1 < x_min) x_min = x_count1;
            if (x_count1 > x_max) x_max = x_count1;
		    if (y_count1 < y_min) y_min = y_count1;
            if (y_count1 > y_max) y_max = y_count1;
		    if (z_count1 < z_min) z_min = z_count1;
            if (z_count1 > z_max) z_max = z_count1;
           }
      }
      ++ps;
    }

    /*Auswertung*/ 
	//if (z_min==0.0) z_min = 0.1;
    //if (z_max==0.0) z_max = 0.1;

    if (x_min <= y_min && x_min <= z_min ) min = x_min;
	if (y_min <= x_min && y_min <= z_min ) min = y_min;
	if (z_min <= x_min && z_min <= y_min ) min = z_min;
	if (x_max >= y_max && x_max >= z_max ) max = x_max;
	if (y_max >= x_max && y_max >= z_max ) max = y_max;
	if (z_max >= x_max && z_max >= y_max ) max = z_max;

	x_dist = x_max-x_min;
    y_dist = y_max-y_min;
    z_dist = z_max-z_min;
    dist = x_dist;
    if (y_dist > dist) dist = y_dist;
    if (z_dist > dist) dist = z_dist;


}	

void COGLView::DisplayPoints() 
{	
    static char number[10];
	int j=0;
	GLUquadricObj *qobj =  gluNewQuadric();
	vector<CGLPoint*> gli_points_vector_view;
    gli_points_vector_view.clear();
    gli_points_vector_view = GetPointsVector();//CC
    points_vectorlength = (int)gli_points_vector_view.size();
    GetMSHMinMax();
    GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 

		 if (points_vectorlength < 1000)
		 { /*DATA INPUT*/	
         GLU_BEGIN;/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/ 
		 for (j=0;j<points_vectorlength;j++)
         { 
	        GLU_VERTEX;
			//glEnable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
			glColor3d(0.0,0.0,1.0);
            glTranslated(view_points_vector[j]->x,view_points_vector[j]->y,view_points_vector[j]->z);   		   
			gluQuadricDrawStyle( qobj, GLU_FILL);
			gluQuadricNormals( qobj, GLU_SMOOTH );
			gluSphere( qobj, (max-min)/400, 6, 6); 
            glTranslated((view_points_vector[j]->x)*(-1),(view_points_vector[j]->y)*(-1),(view_points_vector[j]->z)*(-1));   		   
		 }
    	 GLU_END;
		 }

		 else
		 { /*DATA INPUT*/	
		 glPointSize(2.0);
         glBegin(GL_POINTS);/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/  
		 for (j=0;j<points_vectorlength;j++)
         { 
		   glColor3d(0.0,0.0,1.0);	  
		   glVertex3d(view_points_vector[j]->x,view_points_vector[j]->y,view_points_vector[j]->z);   		   
		 }
    	 glEnd();
		 }	
}

void COGLView::DisplayPointNumbers() 
{
    static char number[10];
	int j=0;
	vector<CGLPoint*> gli_points_vector_view;
    gli_points_vector_view.clear();
    gli_points_vector_view = GetPointsVector();//CC
    points_vectorlength = (int)gli_points_vector_view.size();
    GetMSHMinMax();
    GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 

		 for (j=0;j<points_vectorlength;j++)
         { 
		   glColor3f(1.0,1.0,1.0);
		   glRasterPos3d(-x_mid + gli_points_vector_view[j]->x,-y_mid + gli_points_vector_view[j]->y,-z_mid + gli_points_vector_view[j]->z);
		   long point_number = gli_points_vector_view[j]->id;//CC
		   sprintf(number,"%ld",point_number);
		   Text2D(number);		
		 }
}

void COGLView::DisplayDoublePoints() 
{
	static char number[10];
	int j=0;
	long nbdp=0;
    GEO_Search_DoublePoints(m_tolerancefactor);
    points_vectorlength = (int)gli_points_vector.size();
    GetMSHMinMax();
    GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 

         glColor3f(1.0,0.0,0.0);
         glRasterPos3d(0.0,0.0,0.0);
         Text2D("Double Nodes");
         glPointSize(8.0);
         glBegin(GL_POINTS);/*Points direkt lesen aus GLI-Points-Vektor und darstellen*/  
		 for (j=0;j<points_vectorlength;j++)
         { 
           nbdp =  gli_points_vector[j]->number_of_doubled_points;
		   if (nbdp>0)
		   {
             glVertex3d(m_image_distort_factor_x*(-x_mid + gli_points_vector[j]->x),m_image_distort_factor_y*(-y_mid + gli_points_vector[j]->y),m_image_distort_factor_z*(-z_mid + gli_points_vector[j]->z));
		   }
		 }
    	 glEnd();
}

void COGLView::DisplayPolylineNames() 
{
	int j=0, k=0;
	double x=0.0,y=0.0,z=0.0;
    double x_total=0.0,y_total=0.0,z_total=0.0;
	double x_Name=0.0,y_Name=0.0,z_Name=0.0;
	polylines_vectorlength = (int)view_polylines_vector.size();

		 for (j=0;j<polylines_vectorlength;j++)
         {
			 polylinepoints_vectorlength = (int)view_polylines_vector[j]->polyline_point_vector.size();
             for (k=0;k<polylinepoints_vectorlength;k++)
             {
				 x = view_polylines_vector[j]->polyline_point_vector[k]->x;
				 y = view_polylines_vector[j]->polyline_point_vector[k]->y;
				 z = view_polylines_vector[j]->polyline_point_vector[k]->z;               
				 x_total = x_total + x;
				 y_total = y_total + y;
				 z_total = z_total + z;
			 }
			 x_Name = x_total/polylinepoints_vectorlength;
			 y_Name = y_total/polylinepoints_vectorlength;
			 z_Name = z_total/polylinepoints_vectorlength;

			 glColor3f(1.0,1.0,1.0);
		     glRasterPos3d(x_Name,y_Name,z_Name);
			 Text2D(view_polylines_vector[j]->polylinename);
			 x=y=z=x_total=y_total=z_total=x_Name=y_Name=z_Name=0.0;
		 }

}


void COGLView::DisplayNodeNumbers() 
{
  static char number[20];
  int j=0;
  long nodelistlength =  (int)view_nodes_vector.size();
  for (j=0;j<nodelistlength;j++)
  {
    glColor3f(1.0,1.0,1.0);
		   glRasterPos3d(view_nodes_vector[j]->x,view_nodes_vector[j]->y,view_nodes_vector[j]->z);
		   long node_number = view_nodes_vector[j]->nodenumber;
		   sprintf(number,"%ld",node_number);
		   Text2D(number);		
  }
}
void COGLView::DisplayElementNumbers() 
{
    static char number[10];
	int j=0;
    long elements_vectorlength = (int)view_elements_vector.size();
    long element_type=0;
	double x_pos, y_pos, z_pos = 0.0;
    for (j=0;j<elements_vectorlength;j++)
    { 
	  element_type = view_elements_vector[j]->element_type;
	  glColor3f(1.0,1.0,1.0);
      /*LINES = 1*/ 
      if (element_type == 1)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2)/2;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2)/2;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2)/2;
  	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }
	  /*RECTANGLES = 2*/ 
	  if (element_type == 2)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2 + view_elements_vector[j]->x3 + view_elements_vector[j]->x4)/4;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2 + view_elements_vector[j]->y3 + view_elements_vector[j]->y4)/4;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2 + view_elements_vector[j]->z3 + view_elements_vector[j]->z4)/4;
  	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }
	  /*HEXAHEDRA = 3*/ 
      if (element_type == 3)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2 + view_elements_vector[j]->x3 + view_elements_vector[j]->x4 + view_elements_vector[j]->x5 + view_elements_vector[j]->x6 + view_elements_vector[j]->x7 + view_elements_vector[j]->x8)/8;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2 + view_elements_vector[j]->y3 + view_elements_vector[j]->y4 + view_elements_vector[j]->y5 + view_elements_vector[j]->y6 + view_elements_vector[j]->y7 + view_elements_vector[j]->y8)/8;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2 + view_elements_vector[j]->z3 + view_elements_vector[j]->z4 + view_elements_vector[j]->z5 + view_elements_vector[j]->z6 + view_elements_vector[j]->z7 + view_elements_vector[j]->z8)/8;
  	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }
	  /*TRIANGLES = 4*/ 
      if (element_type == 4)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2 + view_elements_vector[j]->x3)/3;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2 + view_elements_vector[j]->y3)/3;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2 + view_elements_vector[j]->z3)/3;
  	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }
	  /*TETRAHEDRAS = 5*/ 
      if (element_type == 5)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2 + view_elements_vector[j]->x3 + view_elements_vector[j]->x4)/4;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2 + view_elements_vector[j]->y3 + view_elements_vector[j]->y4)/4;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2 + view_elements_vector[j]->z3 + view_elements_vector[j]->z4)/4;
  	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }
	  /*PRISMS = 6*/ 
      if (element_type == 6)
	  {	   
       x_pos = (view_elements_vector[j]->x1 + view_elements_vector[j]->x2 + view_elements_vector[j]->x3 + view_elements_vector[j]->x4 + view_elements_vector[j]->x5 + view_elements_vector[j]->x6)/6;
       y_pos = (view_elements_vector[j]->y1 + view_elements_vector[j]->y2 + view_elements_vector[j]->y3 + view_elements_vector[j]->y4 + view_elements_vector[j]->y5 + view_elements_vector[j]->y6)/6;
  	   z_pos = (view_elements_vector[j]->z1 + view_elements_vector[j]->z2 + view_elements_vector[j]->z3 + view_elements_vector[j]->z4 + view_elements_vector[j]->z5 + view_elements_vector[j]->z6)/6;
	   glRasterPos3d(x_pos,y_pos,z_pos);
	   long element_number = view_elements_vector[j]->elementnumber;
	   sprintf(number,"%ld",element_number);
	   Text2D(number);		
	  }	 
  }
}

void COGLView::GetGLIPointsforView() 
{
	int j=0;
    long size;
    points_vectorlength = (int)gli_points_vector.size();

    size = (int)view_points_vector.size();
    if (size > 0)
    {
	    for (j=0;j<size;j++)
        {
            delete view_points_vector[j];
        }
        view_points_vector.clear();
    }

    GetMSHMinMax();
    GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 
	for (j=0;j<points_vectorlength;j++)
    {
        if (gli_points_vector[j]->display_mode == 1)
        {
        m_view_points = new CViewPoints;
	    m_view_points->pointnumber = gli_points_vector[j]->id;//CC
	    m_view_points->meshdensity= gli_points_vector[j]->mesh_density;
	    m_view_points->x = -x_mid +  gli_points_vector[j]->x;
        m_view_points->y = -y_mid +  gli_points_vector[j]->y;
	    m_view_points->z = -z_mid +  gli_points_vector[j]->z;
        view_points_vector.push_back(m_view_points);
        }
    }
} 

void COGLView::GetGLILinesforView() 
{
    int j=0;
	int k=0;
	int hit=0;
	long point1, point2;
    long size;

    
    size = (int)view_lines_vector.size();
    if (size > 0)
    {
	    for (j=0;j<size;j++)
        {
            delete view_lines_vector[j];
        }
        view_lines_vector.clear();
    }
    view_lines_vector.clear();

    points_vectorlength = (int)gli_points_vector.size();
    lines_vectorlength = (int)gli_file_lines_vector.size();
    GetMSHMinMax();
    GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 
	for (j=0;j<lines_vectorlength;j++)
    {
     if (gli_file_lines_vector[j]->display_mode == 1)
     {
	 point1 = gli_file_lines_vector[j]->point1;
     point2 = gli_file_lines_vector[j]->point2;
     hit = 0;
	 m_view_lines = new CViewLines;
	 m_view_lines->linenumber = gli_file_lines_vector[j]->gli_line_id;
	 m_view_lines->pointnumber1 = gli_file_lines_vector[j]->point1;
     m_view_lines->pointnumber2 = gli_file_lines_vector[j]->point2;
     //TODO
	 for (k=0;k<points_vectorlength;k++)
	 {
       if (point1 == gli_points_vector[k]->id)//CC
	   {
		m_view_lines->x1 = -x_mid + gli_points_vector[k]->x;
		m_view_lines->y1 = -y_mid + gli_points_vector[k]->y;
		m_view_lines->z1 = -z_mid + gli_points_vector[k]->z;
		hit++;
	   }
	 
	   if (point2 == gli_points_vector[k]->id)//CC
	   {
         m_view_lines->x2 = -x_mid + gli_points_vector[k]->x;
		 m_view_lines->y2 = -y_mid + gli_points_vector[k]->y;
		 m_view_lines->z2 = -z_mid + gli_points_vector[k]->z;
		 hit++;
	   }
	   if (hit == 2) break;
	   
	 }
     view_lines_vector.push_back(m_view_lines);
     }
    }
} 

void COGLView::GetGLIPolylinesforView() 
{
    int j=0, k=0;
    int size;
    p = polyline_vector.begin();//CC 08/2005
    CString Name;
    polylines_vectorlength = (int)polyline_vector.size();//CC 08/2005
    size = (int)view_polylines_vector.size();
    if (size > 0)
    {
	    for (j=0;j<size;j++)
        {
            polylinepoints_vectorlength = (int)view_polylines_vector[j]->polyline_point_vector.size();
	     	for (k=0;k<polylinepoints_vectorlength;k++)
            {
               delete view_polylines_vector[j]->polyline_point_vector[k];
            }
            view_polylines_vector[j]->polyline_point_vector.clear();
            delete view_polylines_vector[j];
        }
        view_polylines_vector.clear();
    }

    size = (int)view_polylines_vector.size();


    GetMSHMinMax();
    GetMidPoint(); 
	 
	for (j=0;j<polylines_vectorlength;j++)
    { 
		ogl_polyline = *p;
		Name = ogl_polyline->name.data();
		polylinepoints_vectorlength = (int)ogl_polyline->point_vector.size();
        if (ogl_polyline->display_mode == 1)
        {
		m_view_polylines = new CViewPolylines;
		m_view_polylines->polylinename = ogl_polyline->name.data();
		m_view_polylines->polyline_id = j;
        CString ply_file_name = ogl_polyline->ply_file_name.data();
        /*$POINTS*/ 
		for (k=0;k<polylinepoints_vectorlength;k++)
        {
			m_polyline_points = new CGLPoint;
			m_polyline_points->x = -x_mid + ogl_polyline->point_vector[k]->x;
			m_polyline_points->y = -y_mid + ogl_polyline->point_vector[k]->y;
			m_polyline_points->z = -z_mid + ogl_polyline->point_vector[k]->z;
			m_view_polylines->polyline_point_vector.push_back(m_polyline_points);
        }
		view_polylines_vector.push_back(m_view_polylines);
        }
        ++p;
	}

} 

void COGLView::GetGLISurfacesforView() 
{

	int j=0, k=0, l=0;
    int size=0;
	double x_middle=0.0, y_middle=0.0, z_middle=0.0;
    int pre_number=-1;
	string Surface_Name;
    CString Polyline_Name;
	CString SurfacePolyline_Name;
	CGLPolyline *sfc_polyline = NULL;
	CViewPolylines *m_surface_polyline = NULL;
	Surface * m_surface = NULL;
	CGLPoint *poly_points0 = NULL;
	CGLPoint *poly_view_points = NULL;



	vector<Surface*>::iterator ps = surface_vector.begin();//CC
	polylines_vectorlength = (int)polyline_vector.size();//CC 08/2005
	surface_vectorlength = (long)surface_vector.size();//CC
	GetMSHMinMax();
    GetMidPoint(); 

    GEOSurfaceTopology();
    GEOCreateSurfacePointVector(); //OK

    size = (int)view_surfaces_vector.size();
    if (size > 0)
    {
	    for (j=0;j<size;j++)
        {
            surfacepolyline_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector.size();
	     	for (k=0;k<surfacepolyline_vectorlength;k++)
            {
                    surfacepolylinepoints_vectorlength = (int)view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector.size();
	     	        for (l=0;l<surfacepolylinepoints_vectorlength;l++)
                    {
                        delete view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector[l];
                    }
                    view_surfaces_vector[j]->surface_polyline_vector[k]->polyline_point_vector.clear();
               delete view_surfaces_vector[j]->surface_polyline_vector[k];
            }
            view_surfaces_vector[j]->surface_polyline_vector.clear();
           delete view_surfaces_vector[j];
        }
        view_surfaces_vector.clear();    
    }


	for (j=0;j<surface_vectorlength ;j++)
    {

	  m_surface = *ps;
	  m_surface->epsilon;
	  Surface_Name = m_surface->name;

      if (m_surface->display_mode_3d == 1)
      {

		m_view_surfaces = new CViewSurfaces;
		m_view_surfaces->surfacename = m_surface->name;
		m_view_surfaces->surface_id = j;

        /*$POINTS*/ 
		//list<CGLPolyline*>::const_iterator pp = m_surface->polyline_of_surface_list.begin();//CC
        vector<CGLPolyline*>::iterator pp = m_surface->polyline_of_surface_vector.begin();
		surfacepolyline_vectorlength = (int)m_surface->polyline_of_surface_vector.size();
		for (k=0;k<surfacepolyline_vectorlength;k++)
        {
		    sfc_polyline = *pp;
            m_surface_polyline = new CViewPolylines;
			m_surface_polyline->polylinename = sfc_polyline->name.data();
			m_surface_polyline->polyline_id = k;

			vector<CGLPoint*>::iterator pl = sfc_polyline->point_vector.begin();//CC
			vector<CGLPoint*>::iterator pl0 = sfc_polyline->point_vector.begin();//CC
			vector<CGLPoint*>::const_iterator plv = m_surface_polyline->polyline_point_vector.begin();
			surfacepolylinepoints_vectorlength = (int)sfc_polyline->point_vector.size();
			x_total=y_total=z_total=0.0;
			for (l=0;l<surfacepolylinepoints_vectorlength;l++)
			{
                poly_points0 = *pl0;
             	x = -x_mid + poly_points0->x;
				y = -y_mid + poly_points0->y;
				z = -z_mid + poly_points0->z;
				x_total = x_total +x;
				y_total = y_total +y;
				z_total = z_total +z;
				++pl0;
			}
				x_middle = x_total/surfacepolylinepoints_vectorlength;
				y_middle = y_total/surfacepolylinepoints_vectorlength;
				z_middle =	z_total/surfacepolylinepoints_vectorlength;

            if (k==0)
            {
			for (l=0;l<surfacepolylinepoints_vectorlength;l++)
			{
				poly_view_points = new CGLPoint;
                poly_view_points->id = sfc_polyline->point_vector[l]->id;		
				poly_view_points->x = -x_mid + sfc_polyline->point_vector[l]->x;
				poly_view_points->y = -y_mid + sfc_polyline->point_vector[l]->y;
				poly_view_points->z = -z_mid + sfc_polyline->point_vector[l]->z;
				m_surface_polyline->polyline_point_vector.push_back(poly_view_points);
                pre_number = sfc_polyline->point_vector[l]->id;
			}
            }

            else
            {
                if (pre_number != sfc_polyline->point_vector[0]->id)
                {
       			    for (l=0;l<surfacepolylinepoints_vectorlength;l++)
    			    {  
				    poly_view_points = new CGLPoint;
				    poly_view_points->id = sfc_polyline->point_vector[surfacepolylinepoints_vectorlength-1-l]->id;
				    poly_view_points->x = -x_mid + sfc_polyline->point_vector[surfacepolylinepoints_vectorlength-1-l]->x;
				    poly_view_points->y = -y_mid + sfc_polyline->point_vector[surfacepolylinepoints_vectorlength-1-l]->y;
				    poly_view_points->z = -z_mid + sfc_polyline->point_vector[surfacepolylinepoints_vectorlength-1-l]->z;
				    m_surface_polyline->polyline_point_vector.push_back(poly_view_points);
                    pre_number = sfc_polyline->point_vector[surfacepolylinepoints_vectorlength-1-l]->id;
                    }
                }
                else
                {
       			    for (l=0;l<surfacepolylinepoints_vectorlength;l++)
    			    {  
				    poly_view_points = new CGLPoint;
				    poly_view_points->id = sfc_polyline->point_vector[l]->id;
				    poly_view_points->x = -x_mid + sfc_polyline->point_vector[l]->x;
				    poly_view_points->y = -y_mid + sfc_polyline->point_vector[l]->y;
				    poly_view_points->z = -z_mid + sfc_polyline->point_vector[l]->z;
				    m_surface_polyline->polyline_point_vector.push_back(poly_view_points);
                    pre_number = sfc_polyline->point_vector[l]->id;
                    }
                }
            }
		m_view_surfaces->surface_polyline_vector.push_back(m_surface_polyline);
			++pp;
		}
		m_view_surfaces->surface_midpoint_x =x_middle;
		m_view_surfaces->surface_midpoint_y =y_middle;
		m_view_surfaces->surface_midpoint_z =z_middle;
		view_surfaces_vector.push_back(m_view_surfaces);
      }
      ++ps;
	}
} 

void COGLView::GetMSHMinMax() 
{
	int j=0,i=0;

    GetMinMaxPoints();

  for(j=0;j<(long)fem_msh_vector.size();j++)
  {
     for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
     {
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
	//if (z_min==0.0) z_min = 0.1; TODO
    //if (z_max==0.0) z_max = 0.1;

	if (x_min <= y_min && x_min <= z_min ) min = x_min;
	if (y_min <= x_min && y_min <= z_min ) min = y_min;
	if (z_min <= x_min && z_min <= y_min ) min = z_min;
	if (x_max >= y_max && x_max >= z_max ) max = x_max;
	if (y_max >= x_max && y_max >= z_max ) max = y_max;
	if (z_max >= x_max && z_max >= y_max ) max = z_max;
	}

	x_dist_mesh = x_dist = x_max-x_min;
    y_dist_mesh = y_dist = y_max-y_min;
    z_dist_mesh = z_dist = z_max-z_min;
    dist = x_dist;
    if (y_dist > dist) dist = y_dist;
    if (z_dist > dist) dist = z_dist;

    GetMidPoint();
}

void COGLView::GetRFINodesforView() 
{
  int i=0;
  long nodelistlength=0;
  int size;
  size = (int)view_nodes_vector.size();
      for (i=0;i<size;i++)
      {
         delete view_nodes_vector[i];
      }
      view_nodes_vector.clear();
  
  if(NODListExists()&& m_3dcontrol_nodes == 1){	  
	nodelistlength  =  NodeListSize ();
	GetMSHMinMax(); /*Lokalisierung der transformierten Min-Max-Koordinaten*/ 
	GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 
	for (i=0;i<nodelistlength;i++)
	{
		m_view_nodes = new CViewNodes;
		m_view_nodes->nodenumber = i;
		m_view_nodes->x =  -x_mid +   GetNodeX (i);
		m_view_nodes->y =  -y_mid +   GetNodeY (i);
		m_view_nodes->z =  -z_mid +   GetNodeZ (i);
		view_nodes_vector.push_back(m_view_nodes);  
	}
  }
} 
void COGLView::GetRFIElementsforView() 
{
  static long *element_nodes;
  int i=0;
  long elementlistlength=0;
  
  int size = (int)view_elements_vector.size();
      for (i=0;i<size;i++)
      {
         delete view_elements_vector[i];
      }
      view_elements_vector.clear();

  if(ELEListExists()&& (m_3dcontrol_elements == 1 || m_3dcontrol_pcs ==1 || m_3dcontrol_matgroups == 1 || m_3dcontrol_msh_quality == 1)) {
		elementlistlength =  ElListSize ();
		GetMSHMinMax(); /*Lokalisierung der transformierten Min-Max-Koordinaten*/ 
		GetMidPoint(); /*Mittelpunkt für Koordinatentranformation auf 0/0/0*/ 
		for (i=0;i<elementlistlength;i++)
		{
			/*LINES = 1*/ 
			if (ElGetElementType(i) == 1)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			view_elements_vector.push_back(m_view_elements); 
			}
			/*RECTANGLES = 2*/ 
			if (ElGetElementType(i) == 2)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->nodenumber3 = element_nodes[2];
 			m_view_elements->nodenumber4 = element_nodes[3];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			m_view_elements->x3 =  -x_mid + GetNodeX (element_nodes[2]);
			m_view_elements->y3 =  -y_mid + GetNodeY (element_nodes[2]);
			m_view_elements->z3 =  -z_mid + GetNodeZ (element_nodes[2]);
 			m_view_elements->x4 =  -x_mid + GetNodeX (element_nodes[3]);
			m_view_elements->y4 =  -y_mid + GetNodeY (element_nodes[3]);
			m_view_elements->z4 =  -z_mid + GetNodeZ (element_nodes[3]);
			view_elements_vector.push_back(m_view_elements); 
			}
			/*HEXAHEDRA = 3*/ 
			if (ElGetElementType(i) == 3)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->nodenumber3 = element_nodes[2];
 			m_view_elements->nodenumber4 = element_nodes[3];
			m_view_elements->nodenumber5 = element_nodes[4];
			m_view_elements->nodenumber6 = element_nodes[5];
			m_view_elements->nodenumber7 = element_nodes[6];
			m_view_elements->nodenumber8 = element_nodes[7];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			m_view_elements->x3 =  -x_mid + GetNodeX (element_nodes[2]);
			m_view_elements->y3 =  -y_mid + GetNodeY (element_nodes[2]);
			m_view_elements->z3 =  -z_mid + GetNodeZ (element_nodes[2]);
 			m_view_elements->x4 =  -x_mid + GetNodeX (element_nodes[3]);
			m_view_elements->y4 =  -y_mid + GetNodeY (element_nodes[3]);
			m_view_elements->z4 =  -z_mid + GetNodeZ (element_nodes[3]);
			m_view_elements->x5 =  -x_mid + GetNodeX (element_nodes[4]);
			m_view_elements->y5 =  -y_mid + GetNodeY (element_nodes[4]);
			m_view_elements->z5 =  -z_mid + GetNodeZ (element_nodes[4]);
			m_view_elements->x6 =  -x_mid + GetNodeX (element_nodes[5]);
			m_view_elements->y6 =  -y_mid + GetNodeY (element_nodes[5]);
			m_view_elements->z6 =  -z_mid + GetNodeZ (element_nodes[5]);
			m_view_elements->x7 =  -x_mid + GetNodeX (element_nodes[6]);
			m_view_elements->y7 =  -y_mid + GetNodeY (element_nodes[6]);
			m_view_elements->z7 =  -z_mid + GetNodeZ (element_nodes[6]);
			m_view_elements->x8 =  -x_mid + GetNodeX (element_nodes[7]);
			m_view_elements->y8 =  -y_mid + GetNodeY (element_nodes[7]);
			m_view_elements->z8 =  -z_mid + GetNodeZ (element_nodes[7]);
			view_elements_vector.push_back(m_view_elements); 
			}
			/*TRIANGLES = 4*/ 
			if (ElGetElementType(i) == 4)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->nodenumber3 = element_nodes[2];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			m_view_elements->x3 =  -x_mid + GetNodeX (element_nodes[2]);
			m_view_elements->y3 =  -y_mid + GetNodeY (element_nodes[2]);
			m_view_elements->z3 =  -z_mid + GetNodeZ (element_nodes[2]);
			view_elements_vector.push_back(m_view_elements); 
			}
			/*TETRAHEDRAS = 5*/ 
			if (ElGetElementType(i) == 5)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->nodenumber3 = element_nodes[2];
 			m_view_elements->nodenumber4 = element_nodes[3];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			m_view_elements->x3 =  -x_mid + GetNodeX (element_nodes[2]);
			m_view_elements->y3 =  -y_mid + GetNodeY (element_nodes[2]);
			m_view_elements->z3 =  -z_mid + GetNodeZ (element_nodes[2]);
 			m_view_elements->x4 =  -x_mid + GetNodeX (element_nodes[3]);
			m_view_elements->y4 =  -y_mid + GetNodeY (element_nodes[3]);
			m_view_elements->z4 =  -z_mid + GetNodeZ (element_nodes[3]);
			view_elements_vector.push_back(m_view_elements); 
			}
			/*PRISMS = 6*/ 
			if (ElGetElementType(i) == 6)
			{
			element_nodes = ElGetElementNodes(i);
			m_view_elements = new CViewElements;     
			m_view_elements->elementnumber = i;
			m_view_elements->element_type = ElGetElementType(i);
			m_view_elements->nodenumber1 = element_nodes[0];
			m_view_elements->nodenumber2 = element_nodes[1];
			m_view_elements->nodenumber3 = element_nodes[2];
 			m_view_elements->nodenumber4 = element_nodes[3];
			m_view_elements->nodenumber5 = element_nodes[4];
			m_view_elements->nodenumber6 = element_nodes[5];
			m_view_elements->materialnumber = ElGetElementGroupNumber(i);
			m_view_elements->x1 =  -x_mid + GetNodeX (element_nodes[0]);
			m_view_elements->y1 =  -y_mid + GetNodeY (element_nodes[0]);
			m_view_elements->z1 =  -z_mid + GetNodeZ (element_nodes[0]);
			m_view_elements->x2 =  -x_mid + GetNodeX (element_nodes[1]);
			m_view_elements->y2 =  -y_mid + GetNodeY (element_nodes[1]);
			m_view_elements->z2 =  -z_mid + GetNodeZ (element_nodes[1]);
			m_view_elements->x3 =  -x_mid + GetNodeX (element_nodes[2]);
			m_view_elements->y3 =  -y_mid + GetNodeY (element_nodes[2]);
			m_view_elements->z3 =  -z_mid + GetNodeZ (element_nodes[2]);
 			m_view_elements->x4 =  -x_mid + GetNodeX (element_nodes[3]);
			m_view_elements->y4 =  -y_mid + GetNodeY (element_nodes[3]);
			m_view_elements->z4 =  -z_mid + GetNodeZ (element_nodes[3]);
			m_view_elements->x5 =  -x_mid + GetNodeX (element_nodes[4]);
			m_view_elements->y5 =  -y_mid + GetNodeY (element_nodes[4]);
			m_view_elements->z5 =  -z_mid + GetNodeZ (element_nodes[4]);
			m_view_elements->x6 =  -x_mid + GetNodeX (element_nodes[5]);
			m_view_elements->y6 =  -y_mid + GetNodeY (element_nodes[5]);
			m_view_elements->z6 =  -z_mid + GetNodeZ (element_nodes[5]);
			view_elements_vector.push_back(m_view_elements); 

			} 
		}
	}
	else
	{
		view_elements_vector.clear();
	}
} 

void COGLView::GetRFIMinMaxPoints() 
{
	int k=0;
    long nodelistlength =  NodeListSize ();

    GetMinMaxPoints();

	if (nodelistlength != NULL)
	{
	/*x_min = GetNodeX (k);
	x_max = GetNodeX (k);
	y_min = GetNodeY (k);
	y_max = GetNodeY (k);
	z_min = GetNodeZ (k);
	z_max = GetNodeZ (k);  */ 

    for (k=0;k<nodelistlength;k++)
    {
	   x_count1 = GetNodeX (k);
	   y_count1 = GetNodeY (k);
	   z_count1 = GetNodeZ (k);

          if (x_count1 < x_min) x_min = x_count1;
          if (x_count1 > x_max) x_max = x_count1;
		  if (y_count1 < y_min) y_min = y_count1;
          if (y_count1 > y_max) y_max = y_count1;
		  if (z_count1 < z_min) z_min = z_count1;
          if (z_count1 > z_max) z_max = z_count1;
	}   
	if (z_min==0.0) z_min = 0.1;
    if (z_max==0.0) z_max = 0.1;

	if (x_min <= y_min && x_min <= z_min ) min = x_min;
	if (y_min <= x_min && y_min <= z_min ) min = y_min;
	if (z_min <= x_min && z_min <= y_min ) min = z_min;
	if (x_max >= y_max && x_max >= z_max ) max = x_max;
	if (y_max >= x_max && y_max >= z_max ) max = y_max;
	if (z_max >= x_max && z_max >= y_max ) max = z_max;
	}

	x_dist_mesh = x_dist = x_max-x_min;
    y_dist_mesh = y_dist = y_max-y_min;
    z_dist_mesh = z_dist = z_max-z_min;
    dist = x_dist;
    if (y_dist > dist) dist = y_dist;
    if (z_dist > dist) dist = z_dist;
}	

void COGLView::InitializeOGLViewDataConstructs() 
{
 scale_x=scale_y=scale_z = 0.0;
 trans_x=trans_y=trans_z = 0.0;
 x= y= z = 0.0;
 max= min = 0.0;
 current_x=current_y=current_z = 0.0;
 x_min=x_max=y_min=y_max=z_min=z_max = 0.0;
 x_mid=y_mid=z_mid=x_total=y_total=z_total = 0.0;
 x_count1=x_count2=y_count1=y_count2=z_count1=z_count2 = 0.0;
 points_vectorlength= lines_vectorlength = 0;
 point1= point2 = 0;
 zoom_control = 0;
 view_counter=NULL;
}	



void COGLView::OnBackColour()
{
	CColorDialog dlg;
	if (dlg.DoModal()!=IDOK)
		return;

	m_ClearCol=dlg.GetColor();	// for drawing button
	SetClearCol(m_ClearCol);
}

void COGLView::SetClearCol(COLORREF rgb)
{
	float r=float(GetRValue(rgb))/255;
	float g=float(GetGValue(rgb))/255;
	float b=float(GetBValue(rgb))/255;
	BeginGLCommands();
	glClearColor(r,g,b,1.0f);
	EndGLCommands();
	Invalidate();	// force redraw
}



void COGLView::OnMeshTest_Area()
{
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleAreaQuality();
	GetRFIElementsforView();
	sceneselect=12;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Tetra_Volume()
{
	CGS_OGL_Tetra_View dlg;
	if (dlg.DoModal() == IDOK) {
	quality_range_min = dlg.m_min_quality_value;
	quality_range_max = dlg.m_max_quality_value;
	Get_Rockflow_RFI();
	CalculateBasicTetrahedraData();
	CalculateTetrahedraVolumeQuality();
	GetRFIElementsforView();
	sceneselect=12;
    Invalidate(TRUE);
	}
}

void COGLView::OnMeshTest_Tetra_Angle()
{
	CGS_OGL_Tetra_View dlg;
	if (dlg.DoModal() == IDOK) {
	quality_range_min = dlg.m_min_quality_value;
	quality_range_max = dlg.m_max_quality_value;
	Get_Rockflow_RFI();
	CalculateBasicTetrahedraData();
	CalculateTetrahedraTriangleAngleQuality();
	GetRFIElementsforView();
	sceneselect=12;
    Invalidate(TRUE);
	}
}


void COGLView::OnMeshTest_Angle()
{
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleAngleQuality();
	GetRFIElementsforView();
	sceneselect=12;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Length()
{
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleLengthQuality();
	GetRFIElementsforView();
	sceneselect=12;
    Invalidate(TRUE);

}


void COGLView::OnMeshTest_Area_Graph()
{
	view_counter=NULL;
	graph_view_onoff = 1;
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleAreaQuality();
	GetRFIElementsforView();
	sceneselect=20;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Angle_Graph()
{
	view_counter=NULL;
	graph_view_onoff = 1;
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleAngleQuality();
	GetRFIElementsforView();
	sceneselect=20;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Length_Graph()
{
	view_counter=NULL;
	graph_view_onoff = 1;
	Get_Rockflow_RFI();
	CalculateBasicTriangleData();
	CalculateTriangleLengthQuality();
	GetRFIElementsforView();
	sceneselect=20;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Tetra_Volume_Graph()
{
	view_counter=NULL;
	graph_view_onoff = 1;
	Get_Rockflow_RFI();
	CalculateBasicTetrahedraData();
	CalculateTetrahedraVolumeQuality();
	GetRFIElementsforView();
	sceneselect=20;
    Invalidate(TRUE);
}

void COGLView::OnMeshTest_Tetra_Angle_Graph()
{
	view_counter=NULL;
	graph_view_onoff = 1;
	Get_Rockflow_RFI();
	CalculateBasicTetrahedraData();
	CalculateTetrahedraTriangleAngleQuality();
	GetRFIElementsforView();
	sceneselect=20;
    Invalidate(TRUE);
}

void COGLView::DrawOGLGraphAxis()
{
  

			 glBegin(GL_LINES); /*X-Achse*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(x_graph_min,y_graph_min,0);
			 glVertex3d(x_graph_max,y_graph_min,0);
		     glEnd();

 			 glBegin(GL_LINES); /*Y-Achse*/
			 glColor3d(0.0,0.0,0.0);
             glVertex3d(x_graph_min,y_graph_min,0);
			 glVertex3d(x_graph_min,y_graph_max,0);
		     glEnd();

             glBegin(GL_LINES); /*X-Hilfslinien*/
			 glColor3d(0.0,0.5,0.0);
             glVertex3d(x_graph_min,y_graph_max,0);
			 glVertex3d(x_graph_max,y_graph_max,0);
             glVertex3d(x_graph_min,y_graph_max - 1*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 1*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 2*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 2*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 3*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 3*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 4*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 4*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 5*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 5*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 6*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 6*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 7*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 7*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 8*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 8*((y_graph_max-y_graph_min)/10),0);
             glVertex3d(x_graph_min,y_graph_max - 9*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max - 9*((y_graph_max-y_graph_min)/10),0);
		     glEnd();

 			 glBegin(GL_LINES); /*Y-Hilfslinien*/
			 glColor3d(0.0,0.5,0.0);
             glVertex3d(x_graph_min,y_graph_min,0);
			 glVertex3d(x_graph_min,y_graph_max,0);       
			 glVertex3d(1*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(1*((x_graph_max-x_graph_min)/10),y_graph_max,0);
 			 glVertex3d(2*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(2*((x_graph_max-x_graph_min)/10),y_graph_max,0);
			 glVertex3d(3*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(3*((x_graph_max-x_graph_min)/10),y_graph_max,0);
 			 glVertex3d(4*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(4*((x_graph_max-x_graph_min)/10),y_graph_max,0);
			 glVertex3d(5*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(5*((x_graph_max-x_graph_min)/10),y_graph_max,0);
 			 glVertex3d(6*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(6*((x_graph_max-x_graph_min)/10),y_graph_max,0);
			 glVertex3d(7*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(7*((x_graph_max-x_graph_min)/10),y_graph_max,0);
 			 glVertex3d(8*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(8*((x_graph_max-x_graph_min)/10),y_graph_max,0);
			 glVertex3d(9*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(9*((x_graph_max-x_graph_min)/10),y_graph_max,0);
 			 glVertex3d(10*((x_graph_max-x_graph_min)/10),y_graph_min,0);
			 glVertex3d(10*((x_graph_max-x_graph_min)/10),y_graph_max,0);
		     glEnd();

			 glColor3d(0.0,0.0,0.0);
			 glRasterPos3d(x_graph_min,y_graph_max + 0.2*((y_graph_max-y_graph_min)/10),0);
			 Text2D("Quality graph for all points:");		

			 char min_txt[56];
			 char max_txt[56];
		     sprintf(min_txt,"%g",y_graph_min);
			 sprintf(max_txt,"%g",y_graph_max);
			 glRasterPos3d(x_graph_max,y_graph_min,0);
			 Text2D(min_txt);				 
			 glRasterPos3d(x_graph_max,y_graph_max,0);
 			 Text2D(max_txt);		

 			 glRasterPos3d(0,0,0);
 			 Text2D("o");		


//Histogramm

			 int i=0;
			 double j=0.0;
			 char txt[4];
			 double scale_nb = 0.0;

 			 glBegin(GL_LINES); /*X-Achse*/
			 glColor3d(0.0,0.0,0.0);

			 glVertex3d(x_graph_min,y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
		     glEnd();

 			 glBegin(GL_LINES); /*Y-Achse*/
			 glColor3d(0.0,0.0,0.0);
			 glVertex3d(x_graph_min,y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_min,(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*16*((y_graph_max-y_graph_min)/10)),0);
		     glEnd();

				for (i=0;i<20;i++)
				{
				glBegin(GL_LINES);
				glColor3d(0.7,0.7,0.7);
				glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
			    glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*16*((y_graph_max-y_graph_min)/10)),0);
				glEnd();

				glBegin(GL_LINES);
				glColor3d(0.7,0.7,0.7);
				glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			    glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 3*((y_graph_max-y_graph_min)/10),0);
				glEnd();
				if (i==0){
				glBegin(GL_LINES);
				glColor3d(0.7,0.7,0.7);
				glVertex3d(x_graph_min + (i)*(x_graph_max/20),y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			    glVertex3d(x_graph_min + (i)*(x_graph_max/20),y_graph_max + 3*((y_graph_max-y_graph_min)/10),0);
				glEnd();
				}
                j++;
				scale_nb = j/20.0;
				sprintf(txt,"%3.2f",scale_nb);
				glColor3d(0.0,0.0,0.0);
				glRasterPos3d(x_graph_min + (i)*(x_graph_max/20),y_graph_max + 3.65*((y_graph_max-y_graph_min)/10),0);
				Text2D(txt);	
				}

    		 glBegin(GL_LINES); /*X-Achse für Kästchen*/
			 glColor3d(0.7,0.7,0.7);
			 glVertex3d(x_graph_min,y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_min,y_graph_max + 3.0*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max + 3.0*((y_graph_max-y_graph_min)/10),0);
		     glEnd();

 			 glColor3d(0.0,0.0,0.0);
			 glRasterPos3d(x_graph_min,(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*20*((y_graph_max-y_graph_min)/10)),0);
			 Text2D("Quality Histogram:");		

}

void COGLView::DrawOGLGraph()
{
    CGeoSysDoc *m_pDoc = GetDocument();
    m_pDoc->OnOpenOGL();

    // the couple glPush - glPop is necessary because the entire
// scene is rotated of the given angle (which is absolute) at every redraw
	glPushMatrix();
// apply trackball rotation
	trackball.IssueGLrotation();
// "save" the current transformation matrix for subsequent superimpose of axes
	glPushMatrix();
//Hier findet der Aufruf zur Darstellung statt:
// radius is for the examples: this should be self explanatory



	DrawQualityFactor();
	DrawOGLGraphAxis();

    
   

       
			 trans_x= 0;
			 trans_y= 0;
             scale_x=0.5;
			 scale_y=0.5;
			 scale_z=0.5;
			 glPushMatrix();
		     glMatrixMode(GL_PROJECTION);
		     glLoadIdentity(); 
			 trackball.SetCamera(view_counter);
			 trackball.currentQuat=unitquaternion(0,X_AXIS);	 
			 glOrtho(-ClipSize*(fabs(x_graph_min))-((x_graph_max-x_graph_min)/100),+ClipSize*0.5*(fabs(x_graph_max))+((x_graph_max-x_graph_min)/100),\
				     -ClipSize*(fabs(y_graph_min))-((y_graph_max-y_graph_min)/50),+ClipSize*(fabs(y_graph_max))+((y_graph_max-y_graph_min)/2.5),\
					 -ClipSize*(1),+ClipSize*(1));   		           
			 glScaled(scale_x,scale_y,scale_z);//TODO
             glTranslated(trans_x,trans_y/2,0.00);
			 glMatrixMode(GL_MODELVIEW);
		     glPopMatrix();
		     //Invalidate(TRUE);
		
		 ZoomAndMove(1);/*Skalierungs- und Translations-Steuerung*/

    trackball.DrawBall();
	glPopMatrix();
	glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		DrawStockDispLists();
	glPopAttrib();
	glPopMatrix();
    

}
			 
void COGLView::DrawQualityFactor()
{
	int i=0;
	double qfactor;
	double norm_fac=0.0;
	long qiterator[20];
	long number_of_all_elements;
	double prozent;
	char txt[10];
	number_of_all_elements = (int)view_elements_vector.size();
	x_graph_min = 0.0;
	x_graph_max = 1.5*y_graph_max;
	qiterator[0]=qiterator[1]=qiterator[2]=qiterator[3]=qiterator[4]=qiterator[5]=
    qiterator[6]=qiterator[7]=qiterator[8]=qiterator[9]=
	qiterator[10]=qiterator[11]=qiterator[12]=qiterator[13]=qiterator[14]=qiterator[15]=
    qiterator[16]=qiterator[17]=qiterator[18]=qiterator[19]=0;
	
	
	glBegin(GL_LINE_STRIP);		
	for (i=0;i<number_of_all_elements;i++)
	{ glColor3d(1.00-(view_elements_vector[i]->quality_factor),view_elements_vector[i]->quality_factor,0.0);
	  qfactor = view_elements_vector[i]->quality_factor;
      glVertex3d((x_graph_max/number_of_all_elements)*i,view_elements_vector[i]->quality_factor,0);
	  
	}
	glEnd();



			
	for (i=0;i<number_of_all_elements;i++)
	{ 
	  qfactor = view_elements_vector[i]->quality_factor;
	  if (0.00 <= qfactor && qfactor < 0.05) qiterator[0]=qiterator[0]+1;
  	  if (0.05 <= qfactor && qfactor < 0.10) qiterator[1]=qiterator[1]+1;
  	  if (0.10 <= qfactor && qfactor < 0.15) qiterator[2]=qiterator[2]+1;
  	  if (0.15 <= qfactor && qfactor < 0.20) qiterator[3]=qiterator[3]+1;
	  if (0.20 <= qfactor && qfactor < 0.25) qiterator[4]=qiterator[4]+1;
	  if (0.25 <= qfactor && qfactor < 0.30) qiterator[5]=qiterator[5]+1;
	  if (0.30 <= qfactor && qfactor < 0.35) qiterator[6]=qiterator[6]+1;
	  if (0.35 <= qfactor && qfactor < 0.40) qiterator[7]=qiterator[7]+1;
	  if (0.40 <= qfactor && qfactor < 0.45) qiterator[8]=qiterator[8]+1;
	  if (0.45 <= qfactor && qfactor < 0.50) qiterator[9]=qiterator[9]+1;
	  if (0.50 <= qfactor && qfactor < 0.55) qiterator[10]=qiterator[10]+1;
  	  if (0.55 <= qfactor && qfactor < 0.60) qiterator[11]=qiterator[11]+1;
  	  if (0.60 <= qfactor && qfactor < 0.65) qiterator[12]=qiterator[12]+1;
  	  if (0.65 <= qfactor && qfactor < 0.70) qiterator[13]=qiterator[13]+1;
	  if (0.70 <= qfactor && qfactor < 0.75) qiterator[14]=qiterator[14]+1;
	  if (0.75 <= qfactor && qfactor < 0.80) qiterator[15]=qiterator[15]+1;
	  if (0.80 <= qfactor && qfactor < 0.85) qiterator[16]=qiterator[16]+1;
	  if (0.85 <= qfactor && qfactor < 0.90) qiterator[17]=qiterator[17]+1;
	  if (0.90 <= qfactor && qfactor < 0.95) qiterator[18]=qiterator[18]+1;
	  if (0.95 <= qfactor && qfactor < 1.00) qiterator[19]=qiterator[19]+1;

	}
	for (i=0;i<20;i++)
	{
	  norm_fac =  qiterator[i]*1.0/number_of_all_elements;
	  if (norm_fac > norm_max_fac)norm_max_fac = norm_fac;
	  glBegin(GL_QUADS);
	  glColor3d(0.0,0.99,0.0);
	  glVertex3d(x_graph_min + i*(x_graph_max/20),y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
      glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
      if (i==1 || i==3 || i==5 || i==7 || i==9 || i==11 || i==13 || i==15 || i==17 || i==19) glColor3d(0.0,0.99,0.0);
	  else glColor3d(0.0,0.7,0.0);		
	  glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_fac)*16*((y_graph_max-y_graph_min)/10)),0);
	  glVertex3d(x_graph_min + i*(x_graph_max/20),(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_fac)*16*((y_graph_max-y_graph_min)/10)),0);	  
	  glEnd();

		prozent = 100.0*qiterator[i]/number_of_all_elements;
		sprintf(txt,"%3.1lf",prozent);
		glColor3d(0.6,0.6,0.6);
		glRasterPos3d(x_graph_min + (i)*(x_graph_max/20),y_graph_max + 3.05*((y_graph_max-y_graph_min)/10),0);
		Text2D(txt);	
	}

}

			 
void COGLView::OnMeshGenerator_GMSH_Tri_Mesh()
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
  CString m_strFileNameRFI = m_strFileNameBase + ".rfi";
  CString m_strFileNameGEO = m_strFileNameBase + ".geo";
  CString m_strFileNameGMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Generation of GMSH input data");
  GEO_Write_GMSH_Input_File(file_name_const_char);/*GLI2GMSH*/ 
  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Meshing");
  CString m_strExecute = "..\\LIB\\gmsh " + m_strFileNameGEO +" -2";
  system(m_strExecute); /*Meshing*/ 
  m_ProgressBar.StepIt();
  FILE *rfi_file=NULL;
  file_name_const_char = m_strFileNameRFI;
  rfi_file = fopen(file_name_const_char, "w+t");
  file_name_const_char = m_strFileNameGMSH;
  char file_name__char [1024];
  strcpy(file_name__char,file_name_const_char);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: MSH2RFI Converter");
  MSH2RFI (file_name__char, rfi_file);/*MSH2RFI*/ 
  m_ProgressBar.StepIt();
  file_name_const_char = m_strFileNameRFI;
  fclose(rfi_file);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Serialize RFI");
  Serialize_RFI (file_name_const_char);/*Serialisierung*/  
  m_ProgressBar.StepIt();
  //remove(m_strFileNameGEO);
  remove(m_strFileNameGMSH);
  file_name_const_char = m_strFileNameBase;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Load Mesh");
  MSHOpen(file_name_const_char);/*OpenRFI*/ 
  m_ProgressBar.StepIt();
  //
 
 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Load View Data");
 MSHOpen((string)m_strFileNameBase);/*OpenRFI*/ 
 GetRFINodesforView();
 GetRFIElementsforView();
 Invalidate(TRUE);
 m_ProgressBar.StepIt();
 pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
}

void COGLView::OnMeshStructured2d()
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

  //write rfi file
  	static char kopf[81];
	long output_nr=0,output_geom=0;
	char rf_version_string[16];
    int i=0, nEle=0, nNod=0;

       FILE *rfi_file=NULL;
       rfi_file = fopen(rfi_file_name_const_char, "w+t");

   	/*Lesen der Versionsnummer und loeschen der Punkte aus dem Versions-String */ 
	strcpy(rf_version_string,ROCKFLOW_VERSION);
	int j = 0;
	for (i=0;((i<(int)(int)strlen(rf_version_string))&&(rf_version_string[i]));i++) 
	{
		if (rf_version_string[i]!='.')
			rf_version_string[i-j] = rf_version_string[i];
			else
			j++;
	}   
	rf_version_string[i-j] = 0;

    //write RFI Head
		for (i=0;i<80;i++)
			kopf[i] = ' ';
		kopf[80] = '\0';
		sprintf(kopf," 0 0 %ld %ld %#g 0 %s",output_nr,output_geom,0.0,rf_version_string);
		for (i=0;i<80;i++)
			if ((kopf[i]==' ') || (kopf[i]=='\0'))
				kopf[i] = '#';
		if (fprintf(rfi_file,"%s\n",kopf)!=81) {
			DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Kopf) !!");
		}

	//Schreibe 2.Zeile
		nNod = (delta_x +1)* (delta_y +1);
        if (dlg.m_do_structured_rectangle_mesh == TRUE) nEle = delta_x * delta_y;
        if (dlg.m_do_structured_triangle_mesh == TRUE)  nEle = delta_x * delta_y * 2;
        if (dlg.m_do_structured_triangle_mesh == FALSE && dlg.m_do_structured_rectangle_mesh == FALSE) return;  
		fprintf(rfi_file,"0 ");
		fprintf(rfi_file,"%ld ",nNod);
		fprintf(rfi_file,"%ld \n",nEle);


        m_ProgressBar.SetRange(0,nNod+nEle);
	    m_ProgressBar.SetStep(1); 

    //write Geometry
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Creating Nodes");  
        int nnb=0;
   	for(j=0 ;j<=delta_y;j++) 
    {
        y = y_min + (j*y_step);
        for(i=0 ;i<=delta_x;i++) 
        {
        x = x_min + (i*x_step);
        fprintf( rfi_file, "%d ", nnb);
        fprintf( rfi_file, "%20.14f %20.14f %20.14f\n", x, y, z);
        nnb++;
        m_ProgressBar.StepIt();
	    }
    }

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
            fprintf( rfi_file, "%d ", nel);
            fprintf( rfi_file, " 0 -1 quad ");     
		    fprintf( rfi_file, "%d  %d  %d  %d\n",point1, point2, point3, point4 );
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
            fprintf( rfi_file, "%d ", nel);
            fprintf( rfi_file, " 0 -1 tri ");     
		    fprintf( rfi_file, "%d  %d  %d\n",point1, point2, point3);
            nel++;
            fprintf( rfi_file, "%d ", nel);
            fprintf( rfi_file, " 0 -1 tri ");     
		    fprintf( rfi_file, "%d  %d  %d\n",point1, point3, point4);
            nel++;
            m_ProgressBar.StepIt();
	    }
        }

    }

       fclose(rfi_file);



  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Finish....Load Data");  

  GEOLIB_Read_GeoLib((string)m_strFileNameBase);
  MSHOpen((string)m_strFileNameBase);

  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".gli");
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".rfi");
  }
}


void COGLView::OnMeshGenerator_Delaunay()
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
  CString m_strFileNameGMSH = m_strFileNameBase + ".msh";
  CString m_strFileNameGSP = m_strFileNameBase + ".gsp";
  const char *file_name_const_char = 0;
  file_name_const_char = m_strFileNameGEO;
  //========================================================================

  const char *gli_file_name_const_char = 0;
  const char *rfi_file_name_const_char = 0;
  gli_file_name_const_char = m_strFileNameGEO;
  rfi_file_name_const_char = m_strFileNameRFI;

  bLoadStart_TRI( (char*)gli_file_name_const_char, (char*)rfi_file_name_const_char, 0 );

  m_ProgressBar.StepIt();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"MESH GENERATION: Finish....Load Data");  

  MSHOpen((string)m_strFileNameBase);
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_pDoc->UpdateAllViews(NULL);
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)" ");  
  Invalidate(TRUE);

  // Add MSH member to GSP vector
  GSPAddMember((string)m_pDoc->m_strGSPFileBase + ".rfi");


}


void COGLView::OnMeshImprovement()
{
	Cgs_mesh_imp mshimp;
	mshimp.DoModal();
	CGeoSysDoc *m_pDoc = GetDocument();
	CString m_strFileNameBase = m_pDoc->m_strGSPFileBase;
	MSHOpen((string)m_strFileNameBase);
	GetRFINodesforView();
	GetRFIElementsforView();
	Invalidate(TRUE);

}

/*************************************************************************
==========================================================================
    DRAWING FUNCTIONS:
==========================================================================
*************************************************************************/

void COGLView::ShowMeshBoundingBox() 
{
         glDisable(GL_BLEND);
         glBegin(GL_LINES);
         glColor3d(0.0,0.0,0.8);
         //glColor3d(0.0,0.0,0.0);//black box
         //glColor4d(1.0,1.0,1.0,0.2);
         
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));

        
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));

         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));

         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(-x_dist_mesh/2),m_image_distort_factor_y*(y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(-z_dist_mesh/2));
         glVertex3d(m_image_distort_factor_x*(x_dist_mesh/2),m_image_distort_factor_y*(-y_dist_mesh/2),m_image_distort_factor_z*(z_dist_mesh/2));

         glEnd();
}


void COGLView::Draw_LineWireFrame(int msh_vector_position, int element_vector_position) 
{
    /*LINES = 1*/ 
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINES); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.5,0.5,0.0);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),
            m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),
            m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),
            m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),
            m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
	glEnd();
}

void COGLView::Draw_QuadWireFrame(int msh_vector_position, int element_vector_position) 
{
    /*RECTANGLES = 2*/ 
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.0,0.0,0.5);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));            
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));            
	glEnd();
}

void COGLView::Draw_HexWireFrame(int msh_vector_position, int element_vector_position) 
{
	/*HEXAHEDRA = 3*/ 
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.5,0.5,0.0);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
    glEnd();
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));    	     
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(6)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(7)->Z())));
	glEnd();
}

void COGLView::Draw_TriWireFrame(int msh_vector_position, int element_vector_position) 
{
	/*TRIANGLES = 4*/ 
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.0,0.5,0.0);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glEnd();
}

void COGLView::Draw_TetWireFrame(int msh_vector_position, int element_vector_position) 
{
    /*TETRAHEDRAS = 5*/ 
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.0,0.0,0.5);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glEnd();
	glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glEnd();
 	glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glEnd();
  	glBegin(GL_LINE_STRIP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glEnd();
}

void COGLView::Draw_PrismWireFrame(int msh_vector_position, int element_vector_position) 
{
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glColor3d(0.0,0.3,1.0);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
    glEnd();
	glBegin(GL_LINE_LOOP); /*Linien lesen aus View-Linien-Vektor und darstellen*/
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(3)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(4)->Z())));
	glEnd();
	glBegin(GL_LINES);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(5)->Z())));
	glEnd();
}

void COGLView::Draw_SelectedTriangles(int msh_vector_position, int element_vector_position) 
{
    int j = msh_vector_position;
    int i = element_vector_position;
	glBegin(GL_TRIANGLES); 
    glColor3d(0.2,0.1,0.0);
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(0)->Z())));
	glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(1)->Z())));
    glVertex3d(m_image_distort_factor_x*(-x_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->X())),m_image_distort_factor_y*(-y_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Y())),m_image_distort_factor_z*(-z_mid +   (fem_msh_vector[j]->ele_vector[i]->GetNode(2)->Z())));
    glEnd();
}

double COGLView::Get_Red_Value(double value_norm) 
{
   	float Red=0.0;
	if (value_norm<0.25)                    Red   =0.0;
	if (value_norm>=0.25 && value_norm<0.5) Red   =0.0;
	if (value_norm>=0.5 && value_norm<0.75) Red   = (value_norm-0.5)*4;
	if (value_norm>=0.75)                   Red   =1.0;
    return Red;
}

double COGLView::Get_Green_Value(double value_norm) 
{
	float Green=0.0;
	if (value_norm<0.25)                    Green =value_norm*4;
	if (value_norm>=0.25 && value_norm<0.5) Green =1.0;
	if (value_norm>=0.5 && value_norm<0.75) Green =1.0;
	if (value_norm>=0.75)                   Green = 1.0-((value_norm-0.75)*4);
    return Green;
}

double COGLView::Get_Blue_Value(double value_norm) 
{
	float Blue=1.0;
	if (value_norm<0.25)                    Blue  =1.0;
	if (value_norm>=0.25 && value_norm<0.5) Blue  =1.0-((value_norm-0.25)*4);
	if (value_norm>=0.5 && value_norm<0.75) Blue  =0.0;
	if (value_norm>=0.75)                   Blue  =0.0;
    return Blue;
}