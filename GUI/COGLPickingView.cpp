// OpenGLMDIView.cpp : implementation of the COGLPickingView class
//
// PCH taken from OGLView.cpp
#include "stdafx.h"
#include "math.h"
#include "OGLControl.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
// PCH 
#include <math.h>
#include "CGLBaseView.h"
#include "COGLPickingView.h"
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
#include "gs_graphics.h"
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
//FEM
#include "elements.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_fluid_momentum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef PCH
long view_counter=0;
long PointOnOff =0;
long PointNumberOnOff =0;
long DoublePointOnOff =0;
long NodeNumberOnOff =0;
long ElementNumberOnOff =0;
long PolylineNameOnOff =0;
#endif


/////////////////////////////////////////////////////////////////////////////
// COGLPickingView for PICKING by PCH

IMPLEMENT_DYNCREATE(COGLPickingView, CGLBaseView)

BEGIN_MESSAGE_MAP(COGLPickingView, CGLBaseView)
	//{{AFX_MSG_MAP(COGLPickingView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_SELECT, OnSelectInPicking)
	ON_COMMAND(ID_DESELECT, OnDeselectInPicking)
	ON_COMMAND(ID_SELECTALL, OnSelectAllInPicking)
	ON_COMMAND(ID_DESELECTALL, OnDeselectAllInPicking)
	ON_COMMAND(ID_PICKEDPROPERTY, OnPickedProperty)
	ON_COMMAND(ID_POLYLINE, OnPolylineDlg)
	ON_COMMAND(ID_SURFACE, OnSurfaceDlg)
	ON_COMMAND(ID_VOLUME, OnVolumeDlg)
    ON_COMMAND(IDC_BACK_COLOUR, OnBackColour)
	ON_UPDATE_COMMAND_UI(ID_SELECT, OnSelectInPickingUpdate)
	ON_UPDATE_COMMAND_UI(ID_DESELECT, OnDeselectInPickingUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COGLPickingView construction/destruction

GLUquadricObj *obj;

COGLPickingView::COGLPickingView()
{
    // Let's initialize two mesh stucture pointers
    m_pcs = NULL;
    m_msh = NULL;
    m_ele = NULL;
	// TODO: add construction code here
	X_Angle = 0.0;
	Y_Angle = 0.0;
	MouseDownPoint.x = 0;
	MouseDownPoint.y = 0;

    LengthOfSideOfelement = 0.0;
    WithNoZoomScale = 1.0;  // By changing this value, the initial zoom is set.
	// Set the zoomRatio initially
	theApp.zoomRatio = -4.f;	
	theApp.mouseX = theApp.mouseY = 0.0f;
    
    // Initialize scale factor
    ScaleFactor = longest_axis = 0.0;
  
	// Initialize ColorSwitch
	theApp.colorSwitch = TRUE;

	// Set rotate mode and none-editing mode as a default
    theApp.mMouseRotateMode = TRUE;
	theApp.mMouseZoomMode = FALSE;
	theApp.mMouseMoveMode = FALSE;
	theApp.mEditMode = FALSE;

	// Initialize SwitchManager
	for(int i=0; i<9; ++i)
		SwitchManager[i] = 0;
}

COGLPickingView::~COGLPickingView()
{
}


/////////////////////////////////////////////////////////////////////////////
// COGLPickingView diagnostics

#ifdef _DEBUG
void COGLPickingView::AssertValid() const
{
	CGLBaseView::AssertValid();
}

void COGLPickingView::Dump(CDumpContext& dc) const
{
	CGLBaseView::Dump(dc);
}


CGeoSysDoc* COGLPickingView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// COGLPickingView Overridables from CGLBaseView

void COGLPickingView::OnCreateGL()
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
	// prepare a bunch of line segments (carthesian axes arrows)
	StartStockDListDef();

	glBegin(GL_LINES);
		// yellow x axis arrow
		glColor3f(1.f,1.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);

		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.95f,0.05f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.95f,-0.05f,0.0f);

		glVertex3f(1.2f,-0.05f,0.0f);   // x letter for axis
		glVertex3f(1.15f,0.05f,0.0f);   // x letter for axis
		glVertex3f(1.2f,0.05f,0.0f);    // x letter for axis
		glVertex3f(1.15f,-0.05f,0.0f);  // x letter for axis
		// cyan y axis arrow
		glColor3f(0.f,1.f,1.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);

		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(0.05f,0.95f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(-0.05f,0.95f,0.0f);

		glVertex3f(0.0f,1.1f,0.0f);    // y letter for axis
		glVertex3f(0.0f,1.15f,0.0f);   // y letter for axis
		glVertex3f(0.0f,1.15f,0.0f);   // y letter for axis
		glVertex3f(0.05f,1.20f,0.0f);  // y letter for axis
		glVertex3f(0.0f,1.15f,0.0f);   // y letter for axis
		glVertex3f(-0.05f,1.20f,0.0f); // y letter for axis

		// magenta z axis arrow
		glColor3f(1.f,0.f,1.f);         // Color setting
		glVertex3f(0.0f,0.0f,0.0f);     // the Start point of z-axis line .jwy
		glVertex3f(0.0f,0.0f,1.0f);     // the End point of z-axis line .jwy
		glVertex3f(0.0f,0.0f,1.0f);     // the Start point of arrow line .jwy
		glVertex3f(0.0f,0.05f,0.95f);   // the End point of arrow line .jwy
		glVertex3f(0.0f,0.0f,1.0f);     // the Start point of arrow line .jwy
		glVertex3f(0.0f,-0.05f,0.95f);  // the End point of arrow line .jwy

		glVertex3f(-0.025f,0.0f,1.15f);   // z letter for axis .jwy
		glVertex3f(+0.025f,0.0f,1.15f);   // z letter for axis .jwy
		glVertex3f(+0.025f,0.0f,1.15f);   // z letter for axis .jwy
		glVertex3f(-0.025f,0.0f,1.10f);   // z letter for axis .jwy
		glVertex3f(-0.025f,0.0f,1.10f);   // z letter for axis .jwy
		glVertex3f(+0.025f,0.0f,1.10f);   // z letter for axis .jwy

	glEnd();
	EndStockListDef();
	
	// Chanhee introduced this for reference wireframe sphere
	obj = gluNewQuadric();
	gluQuadricDrawStyle(obj, GLU_LINE);

    // TK: Set the background color for the first view
	//SetClearCol(12632256);
    SetClearCol(0);
	
}

void COGLPickingView::OnDrawGL(void)
{
	// Chan-Hee

	// the couple glPush - glPop is necessary because the entire
	// scene is rotated of the given angle (which is absolute) at every redraw
	glPushMatrix();

	// rotate the objects of the given angle
	glRotated(X_Angle,1.0,0.0,0.0);
	glRotated(Y_Angle,0.0,0.0,1.0);  // Changed to make the X-Z plane on the screen easily

	// this should be self explanatory
	DrawStockDispLists();
	
	// Calling onSizeGL() in Base GLView
	CGLBaseView::OnSizeGL(-1, -1);

    AssiginSymbolLength();

	// None of the objects are checked for drawing.
	if(SomeSwitchOn() != 0)
		InitializeScalesForOpenGL();

	if(theApp.GLINodeSwitch == 1)
		DrawGLINode();
    
	if (theApp.PolylineSwitch == 1)
		DrawBoundary();
  
    if (theApp.SurfaceSwitch == 1)
		DrawSurface();
    
    if (theApp.VolumeSwitch == 1)
		DrawVolume();
    
	if(theApp.RFINodeSwitch == 1)
		DrawRFINode();
    
	if (theApp.ElementSwitch == 1)
		DrawElement();
    
	if (theApp.ReferenceSwitch == 1)
		DrawReference();
    
    // Draw Vectors
    if(theApp.VelocitySwitch == 1)
        DrawVectorOnNode();
     
    // Draw Particles
    if(theApp.ParticleSwitch == 1)
        DrawParticles();

	// Draw the vectors of crossroads
	if(theApp.CrossroadSwitch == 1)
		DrawVectorOnCrossroads();

	// For graphical debug
	if(theApp.GDebugSwitch == 1)
		DrawGDebug();

	glPopMatrix();
}

int COGLPickingView::SomeSwitchOn()
{
	int SumOfSwitchOns = 0;
	for(int i=0; i<9; ++i)
		SumOfSwitchOns += SwitchManager[i];

	return SumOfSwitchOns;
}

void COGLPickingView::TurnOnTheSwitches()
{
	if(SwitchManager[0] == 1)
	{
		SwitchManager[0] = 0;
		theApp.GLINodeSwitch = 1;
	}
    
	if (SwitchManager[1] == 1)
	{
		SwitchManager[1] = 0;
		theApp.PolylineSwitch = 1;
	}
  
    if (SwitchManager[2] == 1)
	{
		SwitchManager[2] = 0;
		theApp.SurfaceSwitch =1;
	}
    
    if (SwitchManager[3] == 1)
	{
		SwitchManager[3] = 0;
		theApp.VolumeSwitch = 1;
	}
    
	if(SwitchManager[4] == 1)
	{
		SwitchManager[4] = 0;
		theApp.RFINodeSwitch = 1;
	}
    
	if (SwitchManager[5] == 1)
	{
		SwitchManager[5] = 0;
		theApp.ElementSwitch = 1;
	}
    
	if (SwitchManager[6] == 1)
	{
		SwitchManager[6] = 0;
		theApp.ReferenceSwitch = 1;
	}
    
    // Draw Vectors
    if(SwitchManager[7] == 1)
	{
		SwitchManager[7] = 0;
        theApp.VelocitySwitch = 1;
	}
     
    // Draw Particles
    if(SwitchManager[8] == 1)
	{
		SwitchManager[8] = 0;
        theApp.ParticleSwitch = 1;
	}
}

void COGLPickingView::TurnOffTheSwitches()
{
	if(theApp.GLINodeSwitch == 1)
	{
		SwitchManager[0] = 1;
		theApp.GLINodeSwitch = 0;
	}
    
	if (theApp.PolylineSwitch == 1)
	{
		SwitchManager[1] = 1;
		theApp.PolylineSwitch = 0;
	}
  
    if (theApp.SurfaceSwitch == 1)
	{
		SwitchManager[2] = 1;
		theApp.SurfaceSwitch =0;
	}
    
    if (theApp.VolumeSwitch == 1)
	{
		SwitchManager[3] = 1;
		theApp.VolumeSwitch = 0;
	}
    
	if(theApp.RFINodeSwitch == 1)
	{
		SwitchManager[4] = 1;
		theApp.RFINodeSwitch = 0;
	}
    
	if (theApp.ElementSwitch == 1)
	{
		SwitchManager[5] = 1;
		theApp.ElementSwitch = 0;
	}
    
	if (theApp.ReferenceSwitch == 1)
	{
		SwitchManager[6] = 1;
		theApp.ReferenceSwitch = 0;
	}
    
    // Draw Vectors
    if(theApp.VelocitySwitch == 1)
	{
		SwitchManager[7] = 1;
        theApp.VelocitySwitch = 0;
	}
     
    // Draw Particles
    if(theApp.ParticleSwitch == 1)
	{
		SwitchManager[8] = 1;
        theApp.ParticleSwitch = 0;
	}
}

void COGLPickingView::DrawGLINode(void)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	DrawGLINodeScene(GL_RENDER) ;

	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseGLINode(mousePoint.x, mousePoint.y);
		
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseElement(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}

	// If nodes are selected, make'em green
	// Draw symbol for selection
	float centerX = 0., centerY = 0., centerZ =0.;
    
    for(int j=0; j < theApp->hitsGLINodeTotal; ++j)
	{
		glColor3f(0.0, 1.0, 0.0) ;
	
		x = (gli_points_vector[theApp->GLInodePickedTotal[j]]->x-x_mid)/ScaleFactor; 
        y = (gli_points_vector[theApp->GLInodePickedTotal[j]]->y-y_mid)/ScaleFactor;  
        z = (gli_points_vector[theApp->GLInodePickedTotal[j]]->z-z_mid)/ScaleFactor;

		centerX = x;
		centerY = y;
		centerZ = z;

		glBegin(GL_LINES);
			glVertex3f(centerX+symbolLengthX, centerY, centerZ);
			glVertex3f(centerX-symbolLengthX, centerY, centerZ);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(centerX, centerY+symbolLengthY, centerZ);
			glVertex3f(centerX, centerY-symbolLengthY, centerZ);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(centerX, centerY, centerZ+symbolLengthZ);
			glVertex3f(centerX, centerY, centerZ-symbolLengthZ);
		glEnd();
	}	
}

void COGLPickingView::DrawGLINodeScene(GLenum mode)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	glInitNames();
    
    // Number of points is taken from geo_pnt.h
	for(int i=0; i < (int)gli_points_vector.size(); ++i)
    {
        // Again, point id (index) is also taken from geo_pnt.h
        if (mode == GL_SELECT)
            glPushName(i); 
        
        glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;
			
        // Here I make tempary coordinates computed as OpenGL coordinates understand
		x = (gli_points_vector[i]->x-x_mid)/ScaleFactor; 
        y = (gli_points_vector[i]->y-y_mid)/ScaleFactor;  
        z = (gli_points_vector[i]->z-z_mid)/ScaleFactor;
        
		glBegin(GL_POINTS);
			glVertex3f( x, y, z);
		glEnd();

		glPopMatrix() ;
		glPopName();
    }
   
}


void COGLPickingView::DrawRFINode(void)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	DrawRFINodeScene(GL_RENDER) ;

	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseRFINode(mousePoint.x, mousePoint.y);
		
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseRFINode(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}

	// If nodes are selected, make'em green
	// But only if velocity vector is not checked out.
	// Draw symbol for selection
	if(theApp->VelocitySwitch != 1)
	{
		float centerX = 0., centerY = 0., centerZ =0.;
    
		for(int j=0; j < theApp->hitsRFINodeTotal; ++j)
		{
			glColor3f(0.0, 1.0, 0.0) ;
        
			// This is termperary measure only for single mesh cass
			m_msh = fem_msh_vector[0];

			double x, y, z;

			x = (m_msh->nod_vector[theApp->RFInodePickedTotal[j]]->X()-x_mid)/ScaleFactor; 
			y = (m_msh->nod_vector[theApp->RFInodePickedTotal[j]]->Y()-y_mid)/ScaleFactor;  
			z = (m_msh->nod_vector[theApp->RFInodePickedTotal[j]]->Z()-z_mid)/ScaleFactor;
		
			centerX = x; centerY = y; centerZ = z;

			glBegin(GL_LINES);
				glVertex3f(centerX+symbolLengthX, centerY, centerZ);
				glVertex3f(centerX-symbolLengthX, centerY, centerZ);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f(centerX, centerY+symbolLengthY, centerZ);
				glVertex3f(centerX, centerY-symbolLengthY, centerZ);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f(centerX, centerY, centerZ+symbolLengthZ);
				glVertex3f(centerX, centerY, centerZ-symbolLengthZ);
			glEnd();
		}	
	}
}
void COGLPickingView::DrawVectorOnCrossroads(void)
{
    double SquareOfMaximumOfMagnitudeOfVector = 0.0;

    SquareOfMaximumOfMagnitudeOfVector = GetSquareOfMaximumOfMagnitudeOfVector();
    
    // Open the gate to processes 
    m_pcs = PCSGet("FLUID_MOMENTUM");
    m_msh = m_pcs->m_msh;

	for(int i=0; i < theApp.hitsRFINodeTotal ; ++i)
	{
		// Find the index of crossroads
		int crossIndex = 0;
		for(int p=0; p< m_msh->fm_pcs->crossroads.size(); ++p)
		{
			if(theApp.RFInodePickedTotal[i] == m_msh->fm_pcs->crossroads[p]->Index)
				crossIndex = p;
		}
		
		// Looping one more which is times of the number of the planes around this crossroad.
		for(int p=0; p< m_msh->fm_pcs->crossroads[crossIndex]->numOfThePlanes; ++p)
		{
			double V[3], unit[3], glOrigin[3], glTheOtherPoint[3];

			for(int q=0; q<3; ++q)
				V[q] = m_msh->fm_pcs->crossroads[crossIndex]->plane[p].V[q];
			
			double SquareOfMagnitudeOfVector = 0.0;

			SquareOfMagnitudeOfVector = V[0]*V[0] + V[1]*V[1] + V[2]*V[2];

			unit[0] = V[0] / sqrt(SquareOfMagnitudeOfVector); 
			unit[1] = V[1] / sqrt(SquareOfMagnitudeOfVector); 
			unit[2] = V[2] / sqrt(SquareOfMagnitudeOfVector);
		
			// Here I compute the other point from the known magnitude and direction
			// This ratio is on the OpenGL scale
			double ratio = SquareOfMagnitudeOfVector / SquareOfMaximumOfMagnitudeOfVector * LengthOfSideOfelement;

			glOrigin[0] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->X()-x_mid)/ScaleFactor; 
			glOrigin[1] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->Y()-y_mid)/ScaleFactor;  
			glOrigin[2] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->Z()-z_mid)/ScaleFactor;

			glTheOtherPoint[0] = unit[0]*ratio + glOrigin[0];
			glTheOtherPoint[1] = unit[1]*ratio + glOrigin[1];
			glTheOtherPoint[2] = unit[2]*ratio + glOrigin[2];
        
			// Now draw the line of the vector
			glColor3f(0.0, 1.0, 1.0) ;  // Make the line cyan  

			glBegin(GL_LINES);
				//Draw the bottom triangle
				glVertex3f(glOrigin[0], glOrigin[1], glOrigin[2]);
				glVertex3f(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);
			glEnd();	
        
			//draw the wireframe sphere
			// Let's walk to the center of the sphere	
			glTranslatef(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);	
			gluSphere(obj, 0.05*LengthOfSideOfelement, 3, 3);
			// Let's walk back to where it was. 
			glTranslatef(-glTheOtherPoint[0], -glTheOtherPoint[1], -glTheOtherPoint[2]);
		}
	}
   
    Invalidate(TRUE);	 
}

void COGLPickingView::DrawGDebug(void)
{
	DrawGDebugScene(GL_RENDER);
}

void COGLPickingView::DrawVectorOnNode(void)
{
    double SquareOfMaximumOfMagnitudeOfVector = 0.0;

    SquareOfMaximumOfMagnitudeOfVector = GetSquareOfMaximumOfMagnitudeOfVector();
    
    // Open the gate to processes 
    m_pcs = PCSGet("FLUID_MOMENTUM");
    m_msh = m_pcs->m_msh;

	for(int i=0; i < theApp.hitsRFINodeTotal ; ++i)
	{
		double V[3], unit[3], glOrigin[3], glTheOtherPoint[3];
      
		for(int k=0; k<m_pcs->pcs_number_of_primary_nvals; ++k)
		{
			int idx = m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[k])+1;

			if(k == 0)
				V[0] = m_pcs->GetNodeValue(theApp.RFInodePickedTotal[i],idx);
            else if(k == 1)
				V[1] = m_pcs->GetNodeValue(theApp.RFInodePickedTotal[i],idx);
            else if(k == 2)
				V[2] = m_pcs->GetNodeValue(theApp.RFInodePickedTotal[i],idx);
		}
        double SquareOfMagnitudeOfVector = 0.0;

		SquareOfMagnitudeOfVector = V[0]*V[0] + V[1]*V[1] + V[2]*V[2];

		unit[0] = V[0] / sqrt(SquareOfMagnitudeOfVector); 
        unit[1] = V[1] / sqrt(SquareOfMagnitudeOfVector); 
        unit[2] = V[2] / sqrt(SquareOfMagnitudeOfVector);
		
        // Here I compute the other point from the known magnitude and direction
        // This ratio is on the OpenGL scale
        double ratio = SquareOfMagnitudeOfVector / SquareOfMaximumOfMagnitudeOfVector * LengthOfSideOfelement;

		glOrigin[0] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->X()-x_mid)/ScaleFactor; 
        glOrigin[1] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->Y()-y_mid)/ScaleFactor;  
        glOrigin[2] = (m_msh->nod_vector[theApp.RFInodePickedTotal[i]]->Z()-z_mid)/ScaleFactor;

		glTheOtherPoint[0] = unit[0]*ratio + glOrigin[0];
        glTheOtherPoint[1] = unit[1]*ratio + glOrigin[1];
        glTheOtherPoint[2] = unit[2]*ratio + glOrigin[2];
        
        // Now draw the line of the vector
        glColor3f(0.0, 1.0, 1.0) ;  // Make the line cyan  

		glBegin(GL_LINES);
			//Draw the bottom triangle
			glVertex3f(glOrigin[0], glOrigin[1], glOrigin[2]);
			glVertex3f(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);
		glEnd();	
        
        //draw the wireframe sphere
        // Let's walk to the center of the sphere	
		glTranslatef(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);	
		gluSphere(obj, 0.05*LengthOfSideOfelement, 3, 3);
        // Let's walk back to where it was. 
		glTranslatef(-glTheOtherPoint[0], -glTheOtherPoint[1], -glTheOtherPoint[2]);
	}   
    Invalidate(TRUE);	 
}
void COGLPickingView::DrawParticles(void)
{
	DrawParticleScene(GL_RENDER);

	if (theApp.mSelectMode == TRUE || theApp.mDeselectMode == TRUE)
	{
		mouseParticle(mousePoint.x, mousePoint.y);
		
		if(theApp.mContinuous == FALSE)
		{
			theApp.mSelectMode = FALSE;
			theApp.mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp.mSelectMode == TRUE || theApp.mDeselectMode == TRUE && theApp.mContinuous == TRUE )
	{
		mouseParticle(mousePoint.x, mousePoint.y);
		if(theApp.mContinuous == FALSE)
		{
			theApp.mSelectMode = FALSE;
			theApp.mDeselectMode = FALSE;
		}
	}

	// If nodes are selected, make'em green
	// Draw symbol for selection
	float centerX = 0., centerY = 0., centerZ =0.;
    
	for(int j=0; j < theApp.hitsParticleTotal; ++j)
	{
		glColor3f(0.0, 1.0, 0.0) ;
        
		// This is termperary measure only for single mesh cass
		m_msh = fem_msh_vector[0];

		double x, y, z;

		x = (m_msh->PT->X[theApp.ParticlePickedTotal[j]].Now.x-x_mid)/ScaleFactor; 
        y = (m_msh->PT->X[theApp.ParticlePickedTotal[j]].Now.y-y_mid)/ScaleFactor;  
        z = (m_msh->PT->X[theApp.ParticlePickedTotal[j]].Now.z-z_mid)/ScaleFactor;

		centerX = x; centerY = y; centerZ = z;

		glBegin(GL_LINES);
			glVertex3f(centerX+symbolLengthX, centerY, centerZ);
			glVertex3f(centerX-symbolLengthX, centerY, centerZ);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(centerX, centerY+symbolLengthY, centerZ);
			glVertex3f(centerX, centerY-symbolLengthY, centerZ);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(centerX, centerY, centerZ+symbolLengthZ);
			glVertex3f(centerX, centerY, centerZ-symbolLengthZ);
		glEnd();
	}	
}

double COGLPickingView::GetSquareOfMaximumOfMagnitudeOfVector(void)
{
    double MaximumOfSquareOfMagnitudeOfVector = 0.0;
    
    // Open the gate to processes 
    m_pcs = PCSGet("FLUID_MOMENTUM");
    m_msh = m_pcs->m_msh;
    
    for(int i=0; i < (int)m_msh->nod_vector.size() ; ++i)
	{
        double Vx = 0.0, Vy = 0.0, Vz = 0.0;
		for(int k=0; k<m_pcs->pcs_number_of_primary_nvals; ++k)
		{					
            int idx = m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[k])+1;
            
            if(k == 0)
                Vx = m_pcs->GetNodeValue(i,idx);
            else if(k == 1)
                Vy = m_pcs->GetNodeValue(i,idx);
            else if(k == 2)
                Vz = m_pcs->GetNodeValue(i,idx);
		}
        double SquareOfMagnitudeOfVector = 0.0;

        SquareOfMagnitudeOfVector = Vx*Vx + Vy*Vy + Vz*Vz;

        // Assign if SquareOfMagnitudeOfVector is bigger than MaximumOfSquareOfMagnitudeOfVector
        if (SquareOfMagnitudeOfVector > MaximumOfSquareOfMagnitudeOfVector)
            MaximumOfSquareOfMagnitudeOfVector =  SquareOfMagnitudeOfVector;   
	}    

    return MaximumOfSquareOfMagnitudeOfVector;
}


// This function is very problematic. Either this should be improved or
// never be used. Serious memory leaking.
char* COGLPickingView::string2CharArrary(string aString)
{
	char *charArrary = NULL;

	charArrary = (char *)malloc((aString.size()+1) * sizeof (char));
	for(int p=0; p < (int)aString.size(); ++p)
	{
		charArrary[p] = aString[p];
	}
	charArrary[p] = '\0';

	return charArrary;
}

void COGLPickingView::DrawRFINodeScene(GLenum mode)
{
    // This is termperary measure only for single mesh cass
    m_msh = fem_msh_vector[0];

	glInitNames();
    
    for(int i=0; i < (int)m_msh->nod_vector.size() ; ++i)
    {
        // Again, point id (index) is also taken from geo_pnt.h
        if (mode == GL_SELECT)
            glPushName(i); 
        
        glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;   
		
        x = (m_msh->nod_vector[i]->X()-x_mid)/ScaleFactor; 
        y = (m_msh->nod_vector[i]->Y()-y_mid)/ScaleFactor;  
        z = (m_msh->nod_vector[i]->Z()-z_mid)/ScaleFactor;

		glBegin(GL_POINTS);
			glVertex3f( x, y, z);
		glEnd();

		glPopMatrix() ;
		glPopName();
    }
   
}

void COGLPickingView::DrawParticleScene(GLenum mode)
{
    m_msh = fem_msh_vector[0];

    glInitNames();
    
    for(int i=0; i < m_msh->PT->numOfParticles; ++i)
    {
        // Again, point id (index) is also taken from geo_pnt.h
        if (mode == GL_SELECT)
            glPushName(i); 
        
        glPushMatrix();

		//color the object in red
		glColor3f(1.0, 0.0, 0.0) ;   

		x = (m_msh->PT->X[i].Now.x-x_mid)/ScaleFactor; 
        y = (m_msh->PT->X[i].Now.y-y_mid)/ScaleFactor;  
        z = (m_msh->PT->X[i].Now.z-z_mid)/ScaleFactor;
        
		glBegin(GL_POINTS);
			glVertex3f( x, y, z);
		glEnd();

		glPopMatrix() ;
		glPopName();
    }
}

void COGLPickingView::DrawGDebugScene(GLenum mode)
{
	// Open the gate to processes 
    m_pcs = PCSGet("FLUID_MOMENTUM");
	m_msh = m_pcs->m_msh;
  
	double SquareOfMaximumOfMagnitudeOfVector = 0.0;
    SquareOfMaximumOfMagnitudeOfVector = GetSquareOfMaximumOfMagnitudeOfVector();

    glInitNames();

	for(int i=0; i < theApp.hitsElementTotal; ++i)
	{
		m_ele = m_msh->ele_vector[theApp.elementPickedTotal[i]];
		/*TRIANGLES = 4*/ 
		if (m_ele->GetElementType() == 4)
		{
			double N1[3], N2[3], N3[3];
			double XYx[3], XYy[3], XYz[3];
			double x[3], y[3], z[3];

			N1[0] = m_msh->nod_vector[m_ele->GetNodeIndex(0)]->X();
			N1[1] = m_msh->nod_vector[m_ele->GetNodeIndex(0)]->Y();
			N1[2] = m_msh->nod_vector[m_ele->GetNodeIndex(0)]->Z();
			N2[0] = m_msh->nod_vector[m_ele->GetNodeIndex(1)]->X();
			N2[1] = m_msh->nod_vector[m_ele->GetNodeIndex(1)]->Y();
			N2[2] = m_msh->nod_vector[m_ele->GetNodeIndex(1)]->Z();
			N3[0] = m_msh->nod_vector[m_ele->GetNodeIndex(2)]->X();
			N3[1] = m_msh->nod_vector[m_ele->GetNodeIndex(2)]->Y();
			N3[2] = m_msh->nod_vector[m_ele->GetNodeIndex(2)]->Z();
			m_msh->PT->ToTheXYPlane(m_ele, N1);	
			m_msh->PT->ToTheXYPlane(m_ele, N2);
			m_msh->PT->ToTheXYPlane(m_ele, N3);
			
			x[0] = (N1[0]-x_mid)/ScaleFactor; x[1] = (N2[0]-x_mid)/ScaleFactor; x[2] = (N3[0]-x_mid)/ScaleFactor;
			y[0] = (N1[1]-y_mid)/ScaleFactor; y[1] = (N2[1]-y_mid)/ScaleFactor; y[2] = (N3[1]-y_mid)/ScaleFactor;
			z[0] = (N1[2]-z_mid)/ScaleFactor; z[1] = (N2[2]-z_mid)/ScaleFactor; z[2] = (N3[2]-z_mid)/ScaleFactor;
       
			glBegin(GL_LINE_LOOP);
			for(int j=0;j<3;++j)
				glVertex3f(x[j], y[j], z[j]);
			glEnd();
		}

		// Draw the velocity vector transformed.
		for(int j=0; j<m_ele->GetEdgesNumber(); ++j)
		{
			double V[3], unit[3], glOrigin[3], glTheOtherPoint[3], Pxy[3];

			// If this node is not a crossroad
			if(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->crossroad != 1)
			{
				for(int k=0; k<m_pcs->pcs_number_of_primary_nvals; ++k)
				{
					int idx = m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[k])+1;

					if(k == 0)
						V[0] = m_pcs->GetNodeValue(m_ele->GetNodeIndex(j),idx);
					else if(k == 1)
						V[1] = m_pcs->GetNodeValue(m_ele->GetNodeIndex(j),idx);
					else if(k == 2)
						V[2] = m_pcs->GetNodeValue(m_ele->GetNodeIndex(j),idx);
				}
			}
			else	// if this is a crossroad,
			{
				// Find the crossroad index
				CrossRoad* crossroad = NULL;
				for(int k=0; k< m_msh->fm_pcs->crossroads.size(); ++k)
				{
					if( m_msh->fm_pcs->crossroads[k]->Index == m_ele->GetNodeIndex(j) )
						crossroad = m_msh->fm_pcs->crossroads[k];
				}

				if(crossroad)
				{
				}
				else	// Failed to find the crossroad although it is a crossroad
					abort();	

				// Find the velocity of the crossroad associated with the connected planes.
				for(int k=0; k< crossroad->numOfThePlanes; ++k)
				{
					// I am going to check the normal vector of the element and the connected plane.
					double tolerance = 1e-10;
					double E[3], P[3];
					for(int p=0; p<3; ++p)
					{
						E[p] = m_ele->getTransformTensor(6+p); 
						P[p] = crossroad->plane[k].norm[p];
					}
					
					double same = (E[0]-P[0])*(E[0]-P[0]) + (E[1]-P[1])*(E[1]-P[1]) + (E[2]-P[2])*(E[2]-P[2]);

					if(same < tolerance)
					{
						for(int p=0; p<3; ++p)
							V[p] = crossroad->plane[k].V[p];	
					}
				}
			}
			
			m_msh->PT->ToTheXYPlane(m_ele, V);
			double SquareOfMagnitudeOfVector = 0.0;
			SquareOfMagnitudeOfVector = V[0]*V[0] + V[1]*V[1] + V[2]*V[2];

			unit[0] = V[0] / sqrt(SquareOfMagnitudeOfVector); 
			unit[1] = V[1] / sqrt(SquareOfMagnitudeOfVector); 
			unit[2] = V[2] / sqrt(SquareOfMagnitudeOfVector);
		
			// Here I compute the other point from the known magnitude and direction
			// This ratio is on the OpenGL scale
			double ratio = SquareOfMagnitudeOfVector / SquareOfMaximumOfMagnitudeOfVector * LengthOfSideOfelement;

			Pxy[0] = m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X();
			Pxy[1] = m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y();
			Pxy[2] = m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z();
			m_msh->PT->ToTheXYPlane(m_ele, Pxy);
			glOrigin[0] = (Pxy[0]-x_mid)/ScaleFactor; 
			glOrigin[1] = (Pxy[1]-y_mid)/ScaleFactor;  
			glOrigin[2] = (Pxy[2]-z_mid)/ScaleFactor;

			glTheOtherPoint[0] = unit[0]*ratio + glOrigin[0];
			glTheOtherPoint[1] = unit[1]*ratio + glOrigin[1];
			glTheOtherPoint[2] = unit[2]*ratio + glOrigin[2];
        
			// Now draw the line of the vector
			glColor3f(0.0, 1.0, 1.0) ;  // Make the line cyan  

			glBegin(GL_LINES);
				//Draw the bottom triangle
				glVertex3f(glOrigin[0], glOrigin[1], glOrigin[2]);
				glVertex3f(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);
			glEnd();	
        
			//draw the wireframe sphere
			// Let's walk to the center of the sphere	
			glTranslatef(glTheOtherPoint[0], glTheOtherPoint[1], glTheOtherPoint[2]);	
			gluSphere(obj, 0.05*LengthOfSideOfelement, 3, 3);
			// Let's walk back to where it was. 
			glTranslatef(-glTheOtherPoint[0], -glTheOtherPoint[1], -glTheOtherPoint[2]);
		}
	}
}


void COGLPickingView::DrawElement()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();


    for(int k=0; k < theApp->hitsElementTotal; ++k)
	{
		if(theApp->colorSwitch == TRUE )
			glColor3f(0.0, 1.0, 0.0) ;
		else
			glColor3f(1.0, 0.0, 0.0) ;
		
		// If elements are selected, make'em green
		drawHittedUnitElement(theApp->elementPickedTotal[k]);
	}


    drawElementScene(GL_RENDER) ;


	
	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseElement(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseElement(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}

}

void COGLPickingView::drawHittedUnitElement(int index)
{
    // This is termperary measure only for single mesh cass
    m_msh = fem_msh_vector[0];
	m_ele = m_msh->ele_vector[index];
	
    /*LINES = 1*/ 
    if (m_ele->GetElementType() == 1)
	{
		int numOfNodeInElement = 2;

        double x[2], y[2], z[2];

        for(int j=0;j< numOfNodeInElement;++j)
		{
            // Convert coordinate to OpenGL
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}
	    	
		// Draw a line
		glBegin(GL_LINES);
			//Draw the line
			glVertex3f(x[0], y[0], z[0]);
			glVertex3f(x[1], y[1], z[1]);
		glEnd();	
    }
    /*RECTANGLES = 2*/ 
	if (m_ele->GetElementType() == 2)
	{
        double x[4], y[4], z[4];
        for(int j=0; j<4; ++j)
		{
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}
       
        glBegin(GL_LINE_LOOP);
		for(int j=0;j<4;++j)
            glVertex3f(x[j], y[j], z[j]);
	    glEnd();
    }
    /*HEXAHEDRA = 3*/ 
	if (m_ele->GetElementType() == 3)
	{
        double x[8], y[8], z[8];
        for(int j=0; j<8; ++j)
		{
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}

		//Draw both top and bottom plane
		glBegin(GL_LINE_LOOP);
			for(int j=0;j<4;++j)
				glVertex3f(x[j], y[j], z[j]);
		glEnd();

		glBegin(GL_LINE_LOOP);
			for(int j=0;j<4;++j)
				glVertex3f(x[j+4], y[j+4], z[j+4]);
		glEnd();

		//Draw four side lines
		glBegin(GL_LINES);
			for(int j=0;j<4;j++)
			{
				// Line from local node 1 to local node 
				glVertex3f(x[j], y[j], z[j]);
				glVertex3f(x[j+4], y[j+4], z[j+4]);
			}
		glEnd();	
    }
    /*TRIANGLES = 4*/ 
	if (m_ele->GetElementType() == 4)
	{
		double x[3], y[3], z[3];
        for(int j=0; j<3; ++j)
		{
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}
       
        glBegin(GL_LINE_LOOP);
		for(int j=0;j<3;++j)
			glVertex3f(x[j], y[j], z[j]);
	    glEnd();
    }
    /*TETRAHEDRAS = 5*/ 
	if (m_ele->GetElementType() == 5)
	{
		int numOfNodeInElement = 4;     
        double x[4], y[4], z[4];
        
        for(int j=0;j< numOfNodeInElement;++j)
		{
            // Convert coordinate to OpenGL
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}
	    	
		// Draw all the edges
		glBegin(GL_LINES);
			for(int j=0;j< 3;j++)
			{
				//Draw the bottom triangle
				glVertex3f(x[j], y[j], z[j]);
				if(j!=2)
					glVertex3f(x[j+1], y[j+1], z[j+1]);
				else
					glVertex3f(x[0], y[0], z[0]);

				// Draw the other three lines
				glVertex3f(x[3], y[3], z[3]);
				glVertex3f(x[j], y[j], z[j]);
			}
		glEnd();	
    }
    /*PRISMS = 6*/ 
	if (m_ele->GetElementType() == 6)
	{
		int numOfNodeInElement = 6;

        double x[6], y[6], z[6];
        for(int j=0; j<numOfNodeInElement; ++j)
		{
			x[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			y[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			z[j] = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
		}

		//Draw both top and bottom plane
		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(x[j], y[j], z[j]);
		glEnd();

		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(x[j+numOfNodeInElement/2], y[j+numOfNodeInElement/2], z[j+numOfNodeInElement/2]);
		glEnd();

		//Draw four side lines
		glBegin(GL_LINES);
			for(int j=0;j<numOfNodeInElement/2;j++)
			{
				// Line from local node 1 to local node 
				glVertex3f(x[j], y[j], z[j]);
				glVertex3f(x[j+numOfNodeInElement/2], y[j+numOfNodeInElement/2], z[j+numOfNodeInElement/2]);
			}
		glEnd();	
    }
}




void COGLPickingView::drawUnitElement(int i)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    m_msh = fem_msh_vector[0];      // This is because FEM is not executed.
	m_ele = m_msh->ele_vector[i];

	drawHittedUnitElement(i);
    

/* This center symbol is too costly.

    // Compute the center of an element
    int numOfNodeInElement = 0;

    //LINES = 1 
    if (m_ele->GetElementType() == 1)
	{
        numOfNodeInElement = 2;
  
        CGLPoint gl[2];
        
        for(int j=0;j< numOfNodeInElement;++j)
	    {
            // Convert coordinate to OpenGL
            gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());
	    	
            centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
	    }
	    centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement; 
    }
    //RECTANGLES = 2 
	if (m_ele->GetElementType() == 2)
	{
        numOfNodeInElement = 4;     
        CGLPoint gl[4];
        
        for(int j=0;j< numOfNodeInElement;++j)
	    {
            // Convert coordinate to OpenGL
            gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());
	    	
            centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
	    }
	    centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement; 
    }
    //HEXAHEDRA = 3 
	if (m_ele->GetElementType() == 3)
	{
        numOfNodeInElement = 8;
        CGLPoint gl[8];
        for(int j=0; j< numOfNodeInElement; ++j)
		{
            gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());

			centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
		}
		centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement;

		//Draw both top and bottom plane
		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
		glEnd();

		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(gl[j+numOfNodeInElement/2].x, gl[j+numOfNodeInElement/2].y, gl[j+numOfNodeInElement/2].z);
		glEnd();

		//Draw four side lines
		glBegin(GL_LINES);
			for(int j=0;j<numOfNodeInElement/2;j++)
			{
				// Line from local node 1 to local node 
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
				glVertex3f(gl[j+numOfNodeInElement/2].x, gl[j+numOfNodeInElement/2].y, gl[j+numOfNodeInElement/2].z);
			}
		glEnd();	
    }
    //TRIANGLES = 4 
	if (m_ele->GetElementType() == 4)
	{
        numOfNodeInElement = 3;    
        CGLPoint gl[3];
        
        for(int j=0;j< numOfNodeInElement;++j)
	    {
            // Convert coordinate to OpenGL
			gl[j].x = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X()-x_mid)/ScaleFactor;
			gl[j].y = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y()-y_mid)/ScaleFactor;
			gl[j].z = (m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z()-z_mid)/ScaleFactor;
	    	
            centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
	    }
	    centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement; 
    }
    //TETRAHEDRAS = 5
	if (m_ele->GetElementType() == 5)
	{
		numOfNodeInElement = 4;   
        CGLPoint gl[4];
        
        for(int j=0;j< numOfNodeInElement;++j)
	    {
            // Convert coordinate to OpenGL
            gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());
	    	
            centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
	    }
	    centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement; 

		// Draw all the edges
		glBegin(GL_LINES);
			for(int j=0;j< 3;j++)
			{
				//Draw the bottom triangle
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
				if(j!=2)
					glVertex3f(gl[j+1].x, gl[j+1].y, gl[j+1].z);
				else
					glVertex3f(gl[0].x, gl[0].y, gl[0].z);

				// Draw the other three lines
				glVertex3f(gl[3].x, gl[3].y, gl[3].z);
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
			}
		glEnd();	
    }
    //PRISMS = 6 
	if (m_ele->GetElementType() == 6)
	{
        numOfNodeInElement = 6;
        CGLPoint gl[6];
        for(int j=0; j< numOfNodeInElement; ++j)
		{
            gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());

			centerX += gl[j].x;
	    	centerY += gl[j].y;
	    	centerZ += gl[j].z;
		}
		centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement;

		//Draw both top and bottom plane
		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
		glEnd();

		glBegin(GL_LINE_LOOP);
			for(int j=0;j<numOfNodeInElement/2;++j)
				glVertex3f(gl[j+numOfNodeInElement/2].x, gl[j+numOfNodeInElement/2].y, gl[j+numOfNodeInElement/2].z);
		glEnd();

		//Draw four side lines
		glBegin(GL_LINES);
			for(int j=0;j<numOfNodeInElement/2;j++)
			{
				// Line from local node 1 to local node 
				glVertex3f(gl[j].x, gl[j].y, gl[j].z);
				glVertex3f(gl[j+numOfNodeInElement/2].x, gl[j+numOfNodeInElement/2].y, gl[j+numOfNodeInElement/2].z);
			}
		glEnd();	
    }
	
    glBegin(GL_LINES);
		glVertex3f(centerX+symbolLengthX, centerY, centerZ);
		glVertex3f(centerX-symbolLengthX, centerY, centerZ);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(centerX, centerY+symbolLengthY, centerZ);
		glVertex3f(centerX, centerY-symbolLengthY, centerZ);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(centerX, centerY, centerZ+symbolLengthZ);
		glVertex3f(centerX, centerY, centerZ-symbolLengthZ);
	glEnd();

*/

}




void COGLPickingView::drawElementScene(GLenum mode)
{
	// This is termperary measure only for single mesh cass
    m_msh = fem_msh_vector[0];
	
	glInitNames();

    for (int i = 0 ; i < (int)m_msh->ele_vector.size(); i++)
	{	
		if (mode == GL_SELECT)
			glPushName(i);
		
		glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;

		drawUnitElement(i);

		glPopMatrix() ;
		glPopName();

	}
}

void COGLPickingView::DrawBoundary()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    CGLPolyline* thisPolyline = NULL;
    // Getting the number of polylines
    //GEOPolylinesNumber();//CC

    for(int k=0; k < theApp->hitsPolylineTotal; ++k)
    {
        // Get the polyline by index to take care of.
        thisPolyline = polyline_vector[theApp->polylinePickedTotal[k]];//CC
        int howManyPointInThisPolyline = (int)thisPolyline->point_vector.size();

        // If boundarys are selected, make'em green
		glColor3f(0.0, 1.0, 0.0) ;
			
        //draw the object
        glBegin(GL_LINE_LOOP);
            for(int j=0; j< howManyPointInThisPolyline; ++j)
            {
                CGLPoint real, gl;

                real.x = thisPolyline->point_vector[j]->x;
                real.y = thisPolyline->point_vector[j]->y;
                real.z = thisPolyline->point_vector[j]->z;

/*  BY OK, The lines can be on when gli points and rfi points are different.
                double distance;
                long msh_node_number = real.MSHGetNodeNumberClose(&distance);
                real.x = GetNodeX(msh_node_number); real.y = GetNodeY(msh_node_number); real.z = GetNodeZ(msh_node_number);      
*/

                gl = ConvertScaleToOpenGL(real);

                glVertex3f(gl.x, gl.y, gl.z);     
            }
		glEnd();	
    }

    drawBoundaryScene(GL_RENDER) ;

	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseBoundary(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseBoundary(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
}



void COGLPickingView::drawBoundaryScene(GLenum mode)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	int i;
	
	glInitNames();
    
    // Getting the number of polylines
    int numberOfPolylines = (int)polyline_vector.size();//CC

    for (i = 0 ; i < numberOfPolylines ; i++)
	{	
		if (mode == GL_SELECT)
			glPushName(i);
		
		glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;

		//draw the object
		drawUnitBoundary(i);

		glPopMatrix() ;
		glPopName();

	}
}

void COGLPickingView::drawUnitBoundary(int i)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    CGLPolyline* thisPolyline = NULL;
    glBegin(GL_LINE_LOOP);
        // Get the polyline by index to take care of.
       thisPolyline = polyline_vector[i];
        int howManyPointInThisPolyline = (int)thisPolyline->point_vector.size();
        for(int j=0; j< howManyPointInThisPolyline; ++j)
            {
                CGLPoint real, gl;

                real.x = thisPolyline->point_vector[j]->x;
                real.y = thisPolyline->point_vector[j]->y;
                real.z = thisPolyline->point_vector[j]->z;

                gl = ConvertScaleToOpenGL(real);

                glVertex3f(gl.x, gl.y, gl.z);     
            }
	glEnd();
}


void COGLPickingView::DrawSurface()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp(); 

    for(int k=0; k < theApp->hitsSurfaceTotal; ++k)
    {
        // Get the surface by index to take care of;
        Surface* thisSurface = surface_vector[theApp->surfacePickedTotal[k]];//CC
        int numberOfPointsInThisSurface = (int)thisSurface->polygon_point_vector.size();

        // If boundarys are selected, make'em green
		glColor3f(0.0, 1.0, 0.0) ;
			
        //draw the object
        glBegin(GL_POLYGON);
  //      glBegin(GL_LINE_LOOP);
            for(int j=0; j< numberOfPointsInThisSurface; ++j)
            {
                CGLPoint real, gl;

                real.x = thisSurface->polygon_point_vector[j]->x;
                real.y = thisSurface->polygon_point_vector[j]->y;
                real.z = thisSurface->polygon_point_vector[j]->z;

                gl = ConvertScaleToOpenGL(real);

                glVertex3f(gl.x, gl.y, gl.z);  
            }
		glEnd();	
    }

    drawSurfaceScene(GL_RENDER) ;

	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseSurface(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseSurface(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
}

void COGLPickingView::drawSurfaceScene(GLenum mode)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	int i;
	
	glInitNames();
    
    // Getting the number of polylines
    int numberOfSurfaces =  (int)surface_vector.size();//CC

    for (i = 0 ; i < numberOfSurfaces ; i++)
	{	
		if (mode == GL_SELECT)
			glPushName(i);
		
		glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;

		//draw the object
		drawUnitSurface(i);

		glPopMatrix() ;
		glPopName();

	}
}

void COGLPickingView::drawUnitSurface(int i)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    
    glBegin(GL_POLYGON);
    Surface* thisSurface=NULL;
//    glBegin(GL_LINE_LOOP);
        // Get the surface by index to take care of;
        thisSurface = surface_vector[i];//CC
        int numberOfPointsInThisSurface = (int)thisSurface->polygon_point_vector.size();

        for(int j=0; j< numberOfPointsInThisSurface; ++j)
            {
                CGLPoint real, gl;

                real.x = thisSurface->polygon_point_vector[j]->x;
                real.y = thisSurface->polygon_point_vector[j]->y;
                real.z = thisSurface->polygon_point_vector[j]->z;

                gl = ConvertScaleToOpenGL(real);

                glVertex3f(gl.x, gl.y, gl.z);     
            }
	glEnd();
}

void COGLPickingView::DrawVolume()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp(); 
    CGLVolume* thisVolume = NULL;
    Surface* thisSurface = NULL;

    for(int k=0; k < theApp->hitsVolumeTotal; ++k)
    {
        // Get the volume by index to take care of;
        thisVolume = GetVolume(theApp->volumePickedTotal[k]);
		int numberOfSurfacesInThisVolume = (int)thisVolume->surface_vector.size();
		

        for(int i=0; i< numberOfSurfacesInThisVolume;++i)
        {
            // Get the surface by index to take care of;
            thisSurface = surface_vector[i];//CC
            int numberOfPointsInThisSurface = (int)thisSurface->polygon_point_vector.size();
   
            // If boundarys are selected, make'em green
		    glColor3f(0.0, 1.0, 0.0) ;
			
            //draw the object
            glBegin(GL_POLYGON);
                for(int j=0; j< numberOfPointsInThisSurface; ++j)
                {
                    CGLPoint real, gl;

                    real.x = thisSurface->polygon_point_vector[j]->x;
                    real.y = thisSurface->polygon_point_vector[j]->y;
                    real.z = thisSurface->polygon_point_vector[j]->z;

                    gl = ConvertScaleToOpenGL(real);

                    glVertex3f(gl.x, gl.y, gl.z);  
                }
		    glEnd();	
        }       
    }

    drawVolumeScene(GL_RENDER) ;

	if (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE)
	{
		mouseVolume(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
	// Added for continuous picking
	else if(theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE && theApp->mContinuous == TRUE )
	{
		mouseVolume(mousePoint.x, mousePoint.y);
		if(theApp->mContinuous == FALSE)
		{
			theApp->mSelectMode = FALSE;
			theApp->mDeselectMode = FALSE;
		}
	}
}



void COGLPickingView::drawVolumeScene(GLenum mode)
{
	// Getting the number of polylines
    int numberOfVolumes =  (int)volume_vector.size();

    for (int i = 0 ; i < numberOfVolumes ; i++)
	{	
		if (mode == GL_SELECT)
			glPushName(i);
		
		glPushMatrix();

		//color the object
		glColor3f(1.0, 1.0, 1.0) ;

		//draw the object
		drawUnitVolume(i);

		glPopMatrix() ;
		glPopName();

	}
}

void COGLPickingView::drawUnitVolume(int i)
{
	// Get the volume by index to take care of;
    CGLVolume* thisVolume = NULL;
    Surface* thisSurface = NULL;

    thisVolume = GetVolume(i);
	int numberOfSurfacesInThisVolume = (int)thisVolume->surface_vector.size();

    for(int p=0; p< numberOfSurfacesInThisVolume;++p)
    {
        // Get the surface by index to take care of;
        thisSurface = surface_vector[p];//CC
        int numberOfPointsInThisSurface = (int)thisSurface->polygon_point_vector.size();
   
        //draw the object
        glBegin(GL_POLYGON);
            for(int j=0; j< numberOfPointsInThisSurface; ++j)
            {
                CGLPoint real, gl;

                real.x = thisSurface->polygon_point_vector[j]->x;
                real.y = thisSurface->polygon_point_vector[j]->y;
                real.z = thisSurface->polygon_point_vector[j]->z;

                gl = ConvertScaleToOpenGL(real);

                glVertex3f(gl.x, gl.y, gl.z);  
            }
        glEnd();	
    }       
}


void COGLPickingView::DrawPostprocess()
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Compute color
	//double color;
    double Cmax;

	Cmax = computeCmax();

#ifdef PCH
	for (int i = 0 ; i < theApp->mFEM.numOfElement ; i++)
	{	
		//color the object
		glColor3f(1.0, 1.0, 1.0) ;

		if ( (theApp->layerAllSwitch) == TRUE )
			drawColoredElement(i);

		else if ( theApp->mFEM.Element[i].layerNo == theApp->layerNo )
			drawColoredElement(i);
	}
#endif


}

void COGLPickingView::DrawReference(void)
{
#ifdef PCH

	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	if (theApp->LineSwitch == 1)
	{	
		glColor3f(1.0, 0.0, 0.0) ;

		glBegin(GL_LINE_LOOP);
			glVertex3f(theApp->cuttingPlane.One.One.x(), theApp->cuttingPlane.One.One.y(), theApp->cuttingPlane.One.One.z());
			glVertex3f(theApp->cuttingPlane.Two.One.x(), theApp->cuttingPlane.Two.One.y(), theApp->cuttingPlane.Two.One.z());
			glVertex3f(theApp->cuttingPlane.Three.One.x(), theApp->cuttingPlane.Three.One.y(), theApp->cuttingPlane.Three.One.z());
			glVertex3f(theApp->cuttingPlane.Four.One.x(), theApp->cuttingPlane.Four.One.y(), theApp->cuttingPlane.Four.One.z());
		glEnd();
	}
	
	if (theApp->SphereSwitch == 1)
	{
		drawReferenceScene(GL_RENDER);
	}

	if (theApp->DiskSwitch == 1)
	{
		drawReferenceScene(GL_RENDER);
	}

	if( theApp->AngleSwitch == 1)
	{
		glColor3f(1.0, 0.0, 0.0) ;

		glBegin(GL_LINES);
			glVertex3f(theApp->Aone.x(), theApp->Aone.y(), theApp->Aone.z());
			glVertex3f(theApp->Atwo.x(), theApp->Atwo.y(), theApp->Atwo.z());
			glVertex3f(theApp->Atwo.x(), theApp->Atwo.y(), theApp->Atwo.z());
			glVertex3f(theApp->Athree.x(), theApp->Athree.y(), theApp->Athree.z());
		glEnd();
	}
#endif

}

void COGLPickingView::drawReferenceScene(GLenum mode)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	
	int i=0;//TK

	glInitNames();
	
	if (mode == GL_SELECT)
		glPushName(i);
		
	glPushMatrix();

	glColor3f(1.0, 0.0, 0.0) ;


#ifdef PCH
	if(theApp->SphereSwitch == 1)
	{
		//draw the wireframe sphere
		glTranslatef(theApp->sphereCenter.x(), theApp->sphereCenter.y(), theApp->sphereCenter.z());	
		gluSphere(obj, theApp->sphereRadius, 24, 24); 
	}

	if(theApp->DiskSwitch == 1)
	{
		//draw the disk
		glTranslatef(theApp->sphereCenter.x(), theApp->sphereCenter.y(), theApp->sphereCenter.z());	
		gluDisk(obj, 0.0f, theApp->sphereRadius, 36, 1); 
	}
#endif


	glPopMatrix() ;
	glPopName();
}

void COGLPickingView::drawColoredElement(int index)
{
    index=index;//TK
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	
#ifdef PCH
	//Draw both top and bottom plane
	glBegin(GL_QUADS);
		// 1st plane
		for(int j=0;j<4;++j)
			glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j]].x(),
				theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j]].y(),
				theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j]].z());
		// 2nd plane
		for(j=0;j<4;++j)
			glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j+4]].x(),
				theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j+4]].y(),
				theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[j+4]].z());	
		// 3rd plane
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].z());

		// 4th plane
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].z());
		
		// 5th plane
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[3]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[2]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[6]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[7]].z());

		// 6th plane
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[0]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[1]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[5]].z());
		glVertex3f(theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].x(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].y(),
			theApp->mFEM.Vertex[theApp->mFEM.Element[index].GlobalNo[4]].z());


	glEnd();	
#endif
	
}


double COGLPickingView::computeCmax(void)
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	double Cmax = -1000.;

#ifdef PCH
	for(int i=0; i < theApp->mFEM.numOfElement; ++i)
		if(Cmax < theApp->mFEM.Element[i].Conc)
			Cmax = theApp->mFEM.Element[i].Conc;
#endif

	return Cmax;

}

/////////////////////////////////////////////////////////////////////////////
// COGLPickingView message handlers

void COGLPickingView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Turn off the switches for rotation
	if (theApp->mMouseRotateMode == TRUE)
		TurnOffTheSwitches();

	if (theApp->mMouseRotateMode == TRUE && theApp->mMouseZoomMode == FALSE &&
		theApp->mMouseMoveMode == FALSE )
	{
		if ((nFlags & MK_LBUTTON) && (!(nFlags & MK_RBUTTON)))
		{
		// remember where we clicked
		MouseDownPoint=point;
		// capture mouse movements even outside window borders
		SetCapture();
		Invalidate(TRUE);
		}
	}
	else if(theApp->mMouseZoomMode == TRUE && theApp->mMouseRotateMode == FALSE &&
		theApp->mMouseMoveMode == FALSE) 
	{
		// remember where we clicked
		MouseDownPoint=point;
		// capture mouse movements even outside window borders
		SetCapture();
		Invalidate(TRUE);
	}
	else if(theApp->mMouseZoomMode == FALSE && theApp->mMouseRotateMode == FALSE &&
		theApp->mMouseMoveMode == TRUE )
	{
		// remember where we clicked
		MouseDownPoint=point;
		// capture mouse movements even outside window borders
		SetCapture();
		Invalidate(TRUE);
	}
	

	// Adding selection mode
	else if ( (theApp->mSelectMode == TRUE || theApp->mDeselectMode == TRUE )
		&& theApp->mMouseRotateMode == FALSE && theApp->mMouseZoomMode == FALSE &&
		theApp->mMouseMoveMode == FALSE )
	{
		SetCapture();
		mousePoint = point;
		// redraw the view
		Invalidate(TRUE);	
	}
	
}

void COGLPickingView::mouseGLINode(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		DrawGLINodeScene(GL_SELECT) ;
		hitsGLINode = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processhitsGLINode(hitsGLINode, selectBuf) ;
	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseRFINode(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		DrawRFINodeScene(GL_SELECT) ;
		hitsRFINode = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processhitsRFINode(hitsRFINode, selectBuf) ;
	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseParticle(int x, int y)
{
	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp.mouseX, theApp.mouseY,theApp.zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		DrawParticleScene(GL_SELECT) ;
		hitsParticle = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processhitsParticle(hitsParticle, selectBuf) ;
	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseElement(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		drawElementScene(GL_SELECT) ;
		hitsElement = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processHitsElement(hitsElement, selectBuf) ;

	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseBoundary(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		drawBoundaryScene(GL_SELECT) ;
		hitsBoundary = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processHitsBoundary(hitsBoundary, selectBuf) ;

	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseSurface(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		drawSurfaceScene(GL_SELECT) ;
		hitsSurface = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processHitsSurface(hitsSurface, selectBuf) ;

	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::mouseVolume(int x, int y)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	GLuint selectBuf[BUFSIZE];
	GLint viewport[4];
	GLfloat ratio;

	glSelectBuffer(BUFSIZE, selectBuf) ;
	glGetIntegerv(GL_VIEWPORT,viewport) ;

	ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];

	glMatrixMode(GL_PROJECTION) ;

	glPushMatrix() ;
		glRenderMode(GL_SELECT) ;
		glLoadIdentity() ;

		gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 10.0,10.0,viewport) ;
		gluPerspective(40.0,ratio,0.1f, 10.0f);
		glTranslatef(theApp->mouseX, theApp->mouseY,theApp->zoomRatio);

		glMatrixMode(GL_MODELVIEW) ;
		drawVolumeScene(GL_SELECT) ;
		hitsVolume = glRenderMode(GL_RENDER) ;
		glMatrixMode(GL_PROJECTION) ;
	glPopMatrix() ;

	processHitsVolume(hitsVolume, selectBuf) ;

	glMatrixMode(GL_MODELVIEW) ;
	
}

void COGLPickingView::processhitsGLINode(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked nodes
	theApp->GLInodePicked = (int *) realloc(theApp->GLInodePicked, hits * sizeof(int));
	theApp->GLInodePickedTotal = (int *) realloc(theApp->GLInodePickedTotal, (theApp->hitsGLINodeTotal + hits)* sizeof(int));

	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->GLInodePickedTotal[theApp->hitsGLINodeTotal + i] = buffer[i*4+3];
					//	theApp->GLInodePicked[i] = buffer[i*4+3];
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsGLINodeTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsGLINodeTotal; ++i)
			for(int j=i+1; j < theApp->hitsGLINodeTotal; ++j)
				if(theApp->GLInodePickedTotal[i] == theApp->GLInodePickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsGLINodeTotal - 1); ++k)
						theApp->GLInodePickedTotal[k] = theApp->GLInodePickedTotal[k+1];
				
					--theApp->hitsGLINodeTotal;
				}	
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->GLInodePicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsGLINodeTotal; ++i)
				if (theApp->GLInodePickedTotal[i] == theApp->GLInodePicked[j])
				{
					for(int k = i; k < (theApp->hitsGLINodeTotal - 1); ++k)
						theApp->GLInodePickedTotal[k] = theApp->GLInodePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsGLINodeTotal;
				}		
	}

}


void COGLPickingView::processhitsRFINode(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked nodes
	theApp->RFInodePicked = (int *) realloc(theApp->RFInodePicked, hits * sizeof(int));
	theApp->RFInodePickedTotal = (int *) realloc(theApp->RFInodePickedTotal, (theApp->hitsRFINodeTotal + hits)* sizeof(int));

	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->RFInodePickedTotal[theApp->hitsRFINodeTotal + i] = buffer[i*4+3];
					//	theApp->GLInodePicked[i] = buffer[i*4+3];
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsRFINodeTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsRFINodeTotal; ++i)
			for(int j=i+1; j < theApp->hitsRFINodeTotal; ++j)
				if(theApp->RFInodePickedTotal[i] == theApp->RFInodePickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsRFINodeTotal - 1); ++k)
						theApp->RFInodePickedTotal[k] = theApp->RFInodePickedTotal[k+1];
				
					--theApp->hitsRFINodeTotal;
				}	
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->RFInodePicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsRFINodeTotal; ++i)
				if (theApp->RFInodePickedTotal[i] == theApp->RFInodePicked[j])
				{
					for(int k = i; k < (theApp->hitsRFINodeTotal - 1); ++k)
						theApp->RFInodePickedTotal[k] = theApp->RFInodePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsRFINodeTotal;
				}		
	}

}

void COGLPickingView::processhitsParticle(GLint hits, GLuint buffer[])
{
	// Dynamic memory allocation for the picked nodes
	theApp.ParticlePicked = (int *) realloc(theApp.ParticlePicked, hits * sizeof(int));
	theApp.ParticlePickedTotal = (int *) realloc(theApp.ParticlePickedTotal, (theApp.hitsParticleTotal + hits)* sizeof(int));

	if(theApp.mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp.ParticlePickedTotal[theApp.hitsParticleTotal + i] = buffer[i*4+3];
		
		// Store the tentative hits to the hitstotal to keep adding
		theApp.hitsParticleTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp.hitsParticleTotal; ++i)
			for(int j=i+1; j < theApp.hitsParticleTotal; ++j)
				if(theApp.ParticlePickedTotal[i] == theApp.ParticlePickedTotal[j])
				{
					for(int k = j; k < (theApp.hitsParticleTotal - 1); ++k)
						theApp.ParticlePickedTotal[k] = theApp.ParticlePickedTotal[k+1];
				
					--theApp.hitsParticleTotal;
				}	
	}
	else if(theApp.mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp.ParticlePicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp.hitsParticleTotal; ++i)
				if (theApp.ParticlePickedTotal[i] == theApp.ParticlePicked[j])
				{
					for(int k = i; k < (theApp.hitsParticleTotal - 1); ++k)
						theApp.ParticlePickedTotal[k] = theApp.ParticlePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp.hitsParticleTotal;
				}		
	}

}

void COGLPickingView::processHitsElement(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked nodes
	theApp->elementPicked = (int *) realloc(theApp->elementPicked, hits * sizeof(int));
	theApp->elementPickedTotal = (int *) realloc(theApp->elementPickedTotal, (theApp->hitsElementTotal + hits)* sizeof(int));

	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
		{
			theApp->elementPickedTotal[theApp->hitsElementTotal + i] = theApp->elementPicked[i] = buffer[i*4+3];		
		}
	
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsElementTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsElementTotal; ++i)
			for(int j=i+1; j < theApp->hitsElementTotal; ++j)
				if(theApp->elementPickedTotal[i] == theApp->elementPickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsElementTotal - 1); ++k)
						theApp->elementPickedTotal[k] = theApp->elementPickedTotal[k+1];
				
					--theApp->hitsElementTotal;
				}	

		theApp->elementPickedTotal = (int *) realloc(theApp->elementPickedTotal, theApp->hitsElementTotal* sizeof(int));
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		
		for(int i=0; i < hits; ++i)
			theApp->elementPicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsElementTotal; ++i)
				if (theApp->elementPickedTotal[i] == theApp->elementPicked[j])
				{
					for(int k = i; k < (theApp->hitsElementTotal - 1); ++k)
						theApp->elementPickedTotal[k] = theApp->elementPickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsElementTotal;
				}

		// Dynamic memory allocation for the picked nodes
		theApp->elementPickedTotal = (int *) realloc(theApp->elementPickedTotal, theApp->hitsElementTotal* sizeof(int));  
	}
}

void COGLPickingView::processHitsBoundary(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked nodes
	theApp->boundaryPicked = (int *) realloc(theApp->boundaryPicked, hits * sizeof(int));
	theApp->polylinePickedTotal = (int *) realloc(theApp->polylinePickedTotal, (theApp->hitsPolylineTotal + hits)* sizeof(int));	
	
	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->polylinePickedTotal[theApp->hitsPolylineTotal + i] =
						theApp->boundaryPicked[i] = buffer[i*4+3];
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsPolylineTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsPolylineTotal; ++i)
			for(int j=i+1; j < theApp->hitsPolylineTotal; ++j)
				if(theApp->polylinePickedTotal[i] == theApp->polylinePickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsPolylineTotal - 1); ++k)
						theApp->polylinePickedTotal[k] = theApp->polylinePickedTotal[k+1];
				
					--theApp->hitsPolylineTotal;
				}	
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->boundaryPicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsPolylineTotal; ++i)
				if (theApp->polylinePickedTotal[i] == theApp->boundaryPicked[j])
				{
					for(int k = i; k < (theApp->hitsPolylineTotal - 1); ++k)
						theApp->polylinePickedTotal[k] = theApp->polylinePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsPolylineTotal;
				}
	}
}


void COGLPickingView::processHitsSurface(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked nodes
	theApp->surfacePicked = (int *) realloc(theApp->surfacePicked, hits * sizeof(int));
	theApp->surfacePickedTotal = (int *) realloc(theApp->surfacePickedTotal, (theApp->hitsSurfaceTotal + hits)* sizeof(int));	
	
	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->surfacePickedTotal[theApp->hitsSurfaceTotal + i] =
						theApp->surfacePicked[i] = buffer[i*4+3];
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsSurfaceTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsSurfaceTotal; ++i)
			for(int j=i+1; j < theApp->hitsSurfaceTotal; ++j)
				if(theApp->surfacePickedTotal[i] == theApp->surfacePickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsSurfaceTotal - 1); ++k)
						theApp->surfacePickedTotal[k] = theApp->surfacePickedTotal[k+1];
				
					--theApp->hitsSurfaceTotal;
				}	
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->surfacePicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsSurfaceTotal; ++i)
				if (theApp->surfacePickedTotal[i] == theApp->surfacePicked[j])
				{
					for(int k = i; k < (theApp->hitsSurfaceTotal - 1); ++k)
						theApp->surfacePickedTotal[k] = theApp->surfacePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsSurfaceTotal;
				}
	}
}


void COGLPickingView::processHitsVolume(GLint hits, GLuint buffer[])
{
	
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// Dynamic memory allocation for the picked volumes
	theApp->volumePicked = (int *) realloc(theApp->volumePicked, hits * sizeof(int));
	theApp->volumePickedTotal = (int *) realloc(theApp->volumePickedTotal, (theApp->hitsVolumeTotal + hits)* sizeof(int));	
	
	if(theApp->mSelectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->volumePickedTotal[theApp->hitsVolumeTotal + i] =
						theApp->volumePicked[i] = buffer[i*4+3];
		// Store the tentative hits to the hitstotal to keep adding
		theApp->hitsVolumeTotal += hits;

		// Eliminate the duplicates
		for(i=0; i < theApp->hitsVolumeTotal; ++i)
			for(int j=i+1; j < theApp->hitsVolumeTotal; ++j)
				if(theApp->volumePickedTotal[i] == theApp->volumePickedTotal[j])
				{
					for(int k = j; k < (theApp->hitsVolumeTotal - 1); ++k)
						theApp->volumePickedTotal[k] = theApp->volumePickedTotal[k+1];
				
					--theApp->hitsVolumeTotal;
				}	
	}
	else if(theApp->mDeselectMode == TRUE)
	{
		for(int i=0; i < hits; ++i)
			theApp->volumePicked[i] = buffer[i*4+3];
		
		// Search the same node and delete it
		for(int j=0; j < hits; ++j)
			for(i=0; i < theApp->hitsVolumeTotal; ++i)
				if (theApp->volumePickedTotal[i] == theApp->volumePicked[j])
				{
					for(int k = i; k < (theApp->hitsVolumeTotal - 1); ++k)
						theApp->volumePickedTotal[k] = theApp->volumePickedTotal[k+1];
					
					// Adjust the total number of node selected
					--theApp->hitsVolumeTotal;
				}
	}
}

void COGLPickingView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Turn on when the rotation is over.
	if (theApp.mMouseRotateMode == TRUE)
		TurnOnTheSwitches();

    nFlags=nFlags;//TK
    point=point;//TK
	// forget where we clicked
	MouseDownPoint=CPoint(0,0);
	// release mouse capture
	ReleaseCapture();
	// Important this will show the selection right away
	Invalidate(TRUE);

}

void COGLPickingView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    nFlags=nFlags;//TK

	if (theApp->mMouseRotateMode == TRUE && theApp->mMouseZoomMode == FALSE &&
		theApp->mMouseMoveMode == FALSE )
	{
		// check if we have captured the mouse
		if (GetCapture()==this)
		{
			// increment the object rotation angles
			X_Angle+=double(point.y-MouseDownPoint.y)/3.6;
			Y_Angle+=double(point.x-MouseDownPoint.x)/3.6;
			// redraw the view
			Invalidate(TRUE);
			// remember the mouse point
			MouseDownPoint=point;
		}
	}
	
	else if(theApp->mMouseZoomMode == TRUE && theApp->mMouseRotateMode == FALSE &&
		theApp->mMouseMoveMode == FALSE)
	{
		// check if we have captured the mouse
		if (GetCapture()==this)
		{
			// increment the object rotation angles
			if(-MouseDownPoint.x / 100. > -6.) // -6.0 is empirical number obtained by trial and error
				theApp->zoomRatio = -MouseDownPoint.x / 100.;
			
			// redraw the view
			Invalidate(TRUE);
			// remember the mouse point	
			MouseDownPoint=point;	
		}	
	}
	else if(theApp->mMouseZoomMode == FALSE && theApp->mMouseRotateMode == FALSE &&
		theApp->mMouseMoveMode == TRUE )
	{
		// check if we have captured the mouse
		if (GetCapture()==this)
		{
			// increment the object rotation angles
			theApp->mouseY = - double(point.y-MouseDownPoint.y)/100.;
			theApp->mouseX = double(point.x-MouseDownPoint.x)/100.;
			// redraw the view
			Invalidate(TRUE);
		}
	}
}

void COGLPickingView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    nFlags=nFlags;//TK

	// remember where we clicked
	MouseDownPoint=point;
	// capture mouse movements even outside window borders
	SetCapture();
	Invalidate(TRUE);

	theApp.mMouseRotateMode = FALSE;
	theApp.mMouseZoomMode = FALSE;
	theApp.mMouseMoveMode = TRUE;

	theApp.mEditMode = FALSE;

	OnStopPicking();
}

void COGLPickingView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    nFlags=nFlags;//TK
    point=point;//TK
	// forget where we clicked
	MouseDownPoint=CPoint(0,0);
	// release mouse capture
	ReleaseCapture();
	// Important this will show the selection right away
	Invalidate(TRUE);

	theApp.mMouseRotateMode = TRUE;
	theApp.mMouseZoomMode = FALSE;
	theApp.mMouseMoveMode = FALSE;

	theApp.mEditMode = FALSE;
}

void COGLPickingView::OnMButtonDown(UINT nFlags, CPoint point) 
{
    nFlags=nFlags;//TK
    point=point;//TK
	OriginalView();

	theApp.mMouseRotateMode = TRUE;
	theApp.mMouseZoomMode = FALSE;
	theApp.mMouseMoveMode = FALSE;

	theApp.mEditMode = FALSE;
}

void COGLPickingView::OnMButtonUp(UINT nFlags, CPoint point) 
{
    nFlags=nFlags;//TK
    point=point;//TK
}

BOOL COGLPickingView::OnMouseWheel(UINT nFlags,short zDelta,CPoint point)
{
    nFlags=nFlags;//TK
    point=point;//TK

	// Turn off the switches for rotation
//	TurnOffTheSwitches();

	// do rolled out stuff here
	theApp.zoomRatio += -(zDelta/120.0)*0.2;

	// redraw the view
	Invalidate(TRUE);

	return TRUE;
}

void COGLPickingView::OriginalView()
{
	COGLPickingView::COGLPickingView();

	// redraw the view
	Invalidate(TRUE);
}


void COGLPickingView::OnFileSaveMesh() 
{

#ifdef PCH

	// TODO: Add your command handler code here

	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	// FileDialog Open
	static char BASED_CODE szFilter[] = "Mesh Configuration File (*.msh)|*.msh|All Files (*.*)|*.*||";
	CFileDialog pFlg(FALSE, "slt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL );

	
	if(pFlg.DoModal() == IDOK)
	{	
		FILE *Mesh;
		Mesh = fopen(pFlg.GetPathName(), "w");

		// Mesh Configuration initially
		fprintf(Mesh,"%.20e\t%.20e\t%.20e\t%d\t%d\t%d\t\n\n",
			theApp->mFEM.Xscale, theApp->mFEM.Yscale, theApp->mFEM.Zscale,
			theApp->mFEM.nx, theApp->mFEM.ny, theApp->mFEM.nz);

		// Number of nodes, elements, and boundaries
		fprintf(Mesh,"%d\t%d\t%d\t\n\n",
			theApp->mFEM.numOfNode, theApp->mFEM.numOfElement, theApp->mFEM.numOfBoundary);

		// Node info stored here
		for(int i=0; i < theApp->mFEM.numOfNode; ++i)
			fprintf(Mesh,"%d\t%.20e\t%.30f\t%.20e\t%.20e\t%d\t%d\t%d\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%d\t%.20e\t%d\t%.20e\t%.20e\t\n",
			theApp->mFEM.Vertex[i].id(), theApp->mFEM.Vertex[i].layerNo(),
			theApp->mFEM.Vertex[i].x(), theApp->mFEM.Vertex[i].y(), 
			theApp->mFEM.Vertex[i].z(), theApp->mFEM.Vertex[i].elementNumber(), 
			theApp->mFEM.Vertex[i].boudarySwitch(), theApp->mFEM.Vertex[i].survivalScore,
			theApp->mFEM.Vertex[i].HeadInitial, theApp->mFEM.Vertex[i].HeadNow,
			theApp->mFEM.Vertex[i].ConcInitial, theApp->mFEM.Vertex[i].ConcNow,
			theApp->mFEM.Vertex[i].TypeIFlow, theApp->mFEM.Vertex[i].TypeIHYes,
			theApp->mFEM.Vertex[i].TypeI, theApp->mFEM.Vertex[i].TypeICYes,
			theApp->mFEM.Vertex[i].q, theApp->mFEM.Vertex[i].Cinput);
		fprintf(Mesh, "\n");
		

		// Element info stored here
		for(i=0; i < theApp->mFEM.numOfElement; ++i)
		{
			fprintf(Mesh,"%d\t%d\t", i, theApp->mFEM.Element[i].layerNo);

			for(int j=0; j < 8; ++j)
				fprintf(Mesh,"%d\t", theApp->mFEM.Element[i].GlobalNo[j]);

			fprintf(Mesh,"%d\t%d\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%.20e\t%d\t%.20e\t%d\t%.20e\t%.20e\t%d\t\n", theApp->mFEM.Element[i].deadSwitch,
				theApp->mFEM.Element[i].numOfBoundariesInElement, theApp->mFEM.Element[i].mParameter.alpha,
				theApp->mFEM.Element[i].mParameter.beta, theApp->mFEM.Element[i].mParameter.Kxx,
				theApp->mFEM.Element[i].mParameter.Kyy, theApp->mFEM.Element[i].mParameter.Kzz,
				theApp->mFEM.Element[i].mParameter.Vx, theApp->mFEM.Element[i].mParameter.Vy, 
				theApp->mFEM.Element[i].mParameter.Vz, theApp->mFEM.Element[i].mParameter.R,
				theApp->mFEM.Element[i].mParameter.Ss, theApp->mFEM.Element[i].mParameter.Galpha, 
				theApp->mFEM.Element[i].mParameter.n, theApp->mFEM.Element[i].mParameter.Sr, 
				theApp->mFEM.Element[i].mParameter.pumpingRate, theApp->mFEM.Element[i].mParameter.pumpingRateYes, 
				theApp->mFEM.Element[i].mParameter.pumpingConc, theApp->mFEM.Element[i].mParameter.pumpingConcYes, 
				theApp->mFEM.Element[i].mParameter.Cnow, theApp->mFEM.Element[i].mParameter.density,
				theApp->mFEM.Element[i].IrregularSwitch);	
		}
		fprintf(Mesh, "\n");

		// Boundary info stored here
		for(i=0; i < theApp->mFEM.numOfBoundary; ++i)
		{
			fprintf(Mesh, "%d\t%d\t", i, theApp->mFEM.Boundary[i].layerNo);

			for(int j=0; j < 4; ++j)
				fprintf(Mesh,"%d\t", theApp->mFEM.Boundary[i].GlobalNo[j]);

			fprintf(Mesh, "%d\t%d\t%d\t%d\t%e\t%d\t%.20e\t%d\t\n", theApp->mFEM.Boundary[i].element,
				theApp->mFEM.Boundary[i].deadSwitch, theApp->mFEM.Boundary[i].face,
				theApp->mFEM.Boundary[i].boundaryTypeFlow, theApp->mFEM.Boundary[i].valueFlow,
				theApp->mFEM.Boundary[i].boundaryType, theApp->mFEM.Boundary[i].value,
				theApp->mFEM.Boundary[i].seaSide);
		}
		fprintf(Mesh, "\n");

		// Just added to start a simulationa at the intermediate time
		fprintf(Mesh,"%.20e\n", theApp->mFEM.Tcurrent);

		fflush(Mesh);
		fclose(Mesh);
	}

#endif
	
}




void COGLPickingView::OnFileSaveResult()
{
	//CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

#ifdef PCH
	// FileDialog Open
	static char BASED_CODE szFilter[] = "Mesh Configuration File (*.rst)|*.rst|All Files (*.*)|*.*||";
	CFileDialog pFlg(FALSE, "slt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL );

	
	if(pFlg.DoModal() == IDOK)
	{	
		FILE *Mesh;
		Mesh = fopen(pFlg.GetPathName(), "w");

		

		// Node info stored here
		for(int i=0; i < theApp->mFEM.numOfNode; ++i)
			fprintf(Mesh,"%d\t%e\t%e\t%e\t%e\t%d\t%d\t%d\t%e\t%e\t%e\t%e\t%e\t%e\t\n",
			theApp->mFEM.Vertex[i].id(), theApp->mFEM.Vertex[i].layerNo(),
			theApp->mFEM.ScaleConversion(theApp->mFEM.Vertex[i].pos()).x(), theApp->mFEM.ScaleConversion(theApp->mFEM.Vertex[i].pos()).y(), 
			theApp->mFEM.ScaleConversion(theApp->mFEM.Vertex[i].pos()).z(), theApp->mFEM.Vertex[i].elementNumber(), 
			theApp->mFEM.Vertex[i].boudarySwitch(), theApp->mFEM.Vertex[i].survivalScore,
			theApp->mFEM.Vertex[i].HeadInitial, theApp->mFEM.Vertex[i].HeadNow,
			theApp->mFEM.Vertex[i].ConcInitial, theApp->mFEM.Vertex[i].ConcNow,
			theApp->mFEM.Vertex[i].TypeIFlow, theApp->mFEM.Vertex[i].TypeI);
		fprintf(Mesh, "\n");
		

		fflush(Mesh);
		fclose(Mesh);
	}
#endif
}


void COGLPickingView::callToRedraw(void)
{
	Invalidate(TRUE);
}

void COGLPickingView::GetRFIMinMaxPoints()
{

    m_msh = fem_msh_vector[0];

    int k=0;
    long nodelistlength =  (long)m_msh->nod_vector.size();

    GetMinMaxPoints();

	if (nodelistlength != NULL)
	{

    for (k=0;k<nodelistlength;k++)
    {
	   x_count1 = m_msh->nod_vector[k]->X();
	   y_count1 = m_msh->nod_vector[k]->Y();
	   z_count1 = m_msh->nod_vector[k]->Z();

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

void COGLPickingView::GetMSHMinMax()
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

void COGLPickingView::GetMinMaxPoints() 
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

void COGLPickingView::GetMidPoint() 
{
  x_mid = (x_min + x_max)/2;
  y_mid = (y_min + y_max)/2;
  z_mid = (z_min + z_max)/2;
}

void COGLPickingView::InitializeScalesForOpenGL()
{
    // Catch some range
	GetMSHMinMax(); 

    // Converting scale   
    // First, obtain the longest axis.
    double xAxis = x_dist; double yAxis = y_dist; double zAxis = z_dist;

	// Select the longest axis 
	if (xAxis < yAxis) {
		if (zAxis < yAxis) longest_axis = yAxis;
		else longest_axis = zAxis;
	}
	else {
		if (zAxis < xAxis) longest_axis = xAxis;
		else longest_axis = zAxis;
	}

	ScaleFactor = longest_axis / 2.0*WithNoZoomScale;
}

CGLPoint COGLPickingView::ConvertScaleToOpenGL(CGLPoint real)
{
    CGLPoint gl;

    // Converting scale   
    gl.x = (real.x - x_mid)  / ScaleFactor; gl.y = (real.y - y_mid) / ScaleFactor; gl.z = (real.z - z_mid) / ScaleFactor;
    
    return gl;
}

CGLPoint COGLPickingView::ConvertScaleToOpenGL(double x, double y, double z)
{
    CGLPoint gl;

    // Converting scale   
    gl.x = (x - x_mid)  / ScaleFactor; gl.y = (y - y_mid) / ScaleFactor; gl.z = (z - z_mid) / ScaleFactor;

    return gl;
}

int COGLPickingView::numberOfGLIPoints()
{
	return (int)gli_points_vector_view.size();
}

CGLPoint COGLPickingView::GetGLIPointByIndex(int i)
{
	CGLPoint AGLIPoint;

	AGLIPoint.x = gli_points_vector[i]->x;
	AGLIPoint.y = gli_points_vector[i]->y;
	AGLIPoint.z = gli_points_vector[i]->z;

	return AGLIPoint;
}

void COGLPickingView::OnSelectInPicking()
{
	theApp.mEditMode = TRUE;
	theApp.mMouseRotateMode = FALSE;

	if(theApp.mEditMode == TRUE)
	{
		theApp.mSelectMode = TRUE;
		theApp.mDeselectMode = FALSE;
		theApp.mMouseRotateMode = FALSE;	

		// Toggle the icon select and deselect buttons
		bSelectEnabled = FALSE;
		bDeselectEnabled = TRUE;
	}

	theApp.mContinuous = TRUE;
}


void COGLPickingView::OnDeselectInPicking()
{
    theApp.mEditMode = TRUE;
	theApp.mMouseRotateMode = FALSE;

	if(theApp.mEditMode == TRUE)
	{
		theApp.mDeselectMode = TRUE;
		theApp.mSelectMode = FALSE;
		theApp.mMouseRotateMode = FALSE;	

		// Toggle the icon select and deselect buttons
		bDeselectEnabled = FALSE;
		bSelectEnabled = TRUE;
	}

	theApp.mContinuous = TRUE;
}

void COGLPickingView::OnSelectAllInPicking()
{
	UpdateData(TRUE);
	if(theApp.ElementSwitch == 1)
	{
		theApp.hitsElementTotal = 0;
        // This is termperary measure only for single mesh cass
        m_msh = fem_msh_vector[0];
		for(int i=0; i< (int)m_msh->ele_vector.size() ; ++i)
		{
			++theApp.hitsElementTotal;
			theApp.elementPickedTotal = (int *)realloc(theApp.elementPickedTotal, theApp.hitsElementTotal*sizeof(int));
			theApp.elementPickedTotal[theApp.hitsElementTotal-1] = i;
		}
	}
	if(theApp.RFINodeSwitch == 1)
	{
		theApp.hitsRFINodeTotal = 0;

		// get the number of gli points from COGLPickingView
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
        pView=pView;//TK

        // This is termperary measure only for single mesh cass
        m_msh = fem_msh_vector[0];
        for(int i=0; i < (int)m_msh->nod_vector.size() ; ++i)        
		{
			++theApp.hitsRFINodeTotal;
			theApp.RFInodePickedTotal = (int *)realloc(theApp.RFInodePickedTotal, theApp.hitsRFINodeTotal*sizeof(int));
			theApp.RFInodePickedTotal[theApp.hitsRFINodeTotal-1] = i;
		}
	}
	if(theApp.ParticleSwitch == 1)
	{
		theApp.hitsParticleTotal = 0;

		// get the number of gli points from COGLPickingView
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
        pView=pView;//TK

        // This is termperary measure only for single mesh cass
        m_msh = fem_msh_vector[0];
		for(int i=0; i < m_msh->PT->numOfParticles ; ++i)        
		{
			++theApp.hitsParticleTotal;
			theApp.ParticlePickedTotal = (int *)realloc(theApp.ParticlePickedTotal, theApp.hitsParticleTotal*sizeof(int));
			theApp.ParticlePickedTotal[theApp.hitsParticleTotal-1] = i;
		}
	}
	if(theApp.PolylineSwitch == 1)
	{
		theApp.hitsPolylineTotal = 0;

        // Getting the number of polylines
        int numberOfPolylines = (int)polyline_vector.size();//CC

        for(int i=0; i< numberOfPolylines; ++i)
		{
			++theApp.hitsPolylineTotal;
			theApp.polylinePickedTotal = (int *)realloc(theApp.polylinePickedTotal, theApp.hitsPolylineTotal*sizeof(int));
			theApp.polylinePickedTotal[theApp.hitsPolylineTotal-1] = i;
		}
	}
	if(theApp.GLINodeSwitch == 1)
	{
		theApp.hitsGLINodeTotal = 0;

		// get the number of gli points from COGLPickingView
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		
		int numberOfGLIPoints = pView->numberOfGLIPoints();

		for(int i=0; i < numberOfGLIPoints; ++i)
		{
			++theApp.hitsGLINodeTotal;
			theApp.GLInodePickedTotal = (int *)realloc(theApp.GLInodePickedTotal, theApp.hitsGLINodeTotal*sizeof(int));
			theApp.GLInodePickedTotal[theApp.hitsGLINodeTotal-1] = i;
		}
	}
    if(theApp.SurfaceSwitch == 1)
	{
		theApp.hitsSurfaceTotal = 0;

        // Getting the number of polylines
        int numberOfSurfaces =  (int)surface_vector.size();//CC

		for(int i=0; i< numberOfSurfaces; ++i)
		{
			++theApp.hitsSurfaceTotal;
			theApp.surfacePickedTotal = (int *)realloc(theApp.surfacePickedTotal, theApp.hitsSurfaceTotal*sizeof(int));
			theApp.surfacePickedTotal[theApp.hitsSurfaceTotal-1] = i;
		}
	}

	// Do some real time selection in the list
	if(theApp.pPolyline->GetSafeHwnd() != NULL)
	{
		theApp.pPolyline->OnUpdatePointListPicked();
		theApp.pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp.pSurface->GetSafeHwnd() != NULL)
	{
		theApp.pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp.pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp.pVolume->GetSafeHwnd() != NULL)
	{
		theApp.pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	// Redraw
	Invalidate();
}

void COGLPickingView::OnDeselectAllInPicking()
{
	UpdateData(TRUE);

	if(theApp.RFINodeSwitch == 1)
	{
		theApp.hitsRFINodeTotal = 0;
	}
	if(theApp.ElementSwitch == 1)
	{
		theApp.hitsElementTotal = 0;
	}
	if(theApp.GLINodeSwitch == 1)
	{
		theApp.hitsGLINodeTotal = 0;
	}
	if(theApp.PolylineSwitch == 1)
	{
		theApp.hitsPolylineTotal = 0;
	}
    if(theApp.SurfaceSwitch == 1)
	{
		theApp.hitsSurfaceTotal = 0;
	}
	if(theApp.VolumeSwitch == 1)
	{
		theApp.hitsVolumeTotal = 0;
	}
	if(theApp.ParticleSwitch == 1)
	{
		theApp.hitsParticleTotal = 0;
	}
	

	// Do some real time selection in the list
	if(theApp.pPolyline->GetSafeHwnd() != NULL)
	{
		theApp.pPolyline->OnUpdatePointListPicked();
		theApp.pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp.pSurface->GetSafeHwnd() != NULL)
	{
		theApp.pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp.pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp.pVolume->GetSafeHwnd() != NULL)
	{
		theApp.pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	// Redraw
	Invalidate();
}

void COGLPickingView::OnStopPicking()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    // Turn off continuous picking
	theApp->mContinuous = FALSE;

	// These handles for next picking
	theApp->mSelectMode = FALSE;
	theApp->mDeselectMode = FALSE;
	
	// Turn back on the icon select and deselect buttons
	bDeselectEnabled = TRUE;
	bSelectEnabled = TRUE;

	// Do some real time selection in the list
	if(theApp->pPolyline->GetSafeHwnd() != NULL)
	{
		theApp->pPolyline->OnUpdatePointListPicked();
		theApp->pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp->pSurface->GetSafeHwnd() != NULL)
	{
		theApp->pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp->pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp->pVolume->GetSafeHwnd() != NULL)
	{
		theApp->pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	// redraw the view
	Invalidate(TRUE);
}

void COGLPickingView::OnSelectInPickingUpdate(CCmdUI *pCmdUI)
{
	if (!bSelectEnabled) pCmdUI->Enable(FALSE);	
}

void COGLPickingView::OnDeselectInPickingUpdate(CCmdUI *pCmdUI)
{
	if (!bDeselectEnabled) pCmdUI->Enable(FALSE);
}

void COGLPickingView::OnPickedProperty()
{
	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

	mainframe->OnPickedProperty();
}

void COGLPickingView::OnPolylineDlg()
{
	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

	mainframe->OnPolylineDlg();
}

void COGLPickingView::OnSurfaceDlg()
{
	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

	mainframe->OnSurfaceDlg();
}

void COGLPickingView::OnVolumeDlg()
{
	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();

	mainframe->OnVolumeDlg();
}

void COGLPickingView::OnBackColour()
{
	CColorDialog dlg;
	if (dlg.DoModal()!=IDOK)
		return;

	m_ClearCol=dlg.GetColor();	// for drawing button
	SetClearCol(m_ClearCol);
}

void COGLPickingView::SetClearCol(COLORREF rgb)
{
	float r=float(GetRValue(rgb))/255;
	float g=float(GetGValue(rgb))/255;
	float b=float(GetBValue(rgb))/255;
	BeginGLCommands();
	glClearColor(r,g,b,1.0f);
	EndGLCommands();
	Invalidate();	// force redraw
}
void COGLPickingView::AssiginSymbolLength()
{
    // This is termperary measure only for single mesh cass

    if (fem_msh_vector.size()>0) //TK: 21.11.05
    {
    m_msh = fem_msh_vector[0];
	m_ele = m_msh->ele_vector[0];
    
    CGLPoint gl[2];

    for(int j=0;j< 2;++j)
        // Convert coordinate to OpenGL
        gl[j] = ConvertScaleToOpenGL(m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X(), 
                m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y(), m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z());
	  
    // Solve for the distance between node0 and node 1 in element
    double distance = sqrt( (gl[0].x - gl[1].x)*(gl[0].x - gl[1].x) + (gl[0].y - gl[1].y)*(gl[0].y - gl[1].y)
                    + (gl[0].z - gl[1].z)*(gl[0].z - gl[1].z) );  	
 
    symbolLengthY = symbolLengthZ = symbolLengthX = 0.2 * distance;

    LengthOfSideOfelement = distance;   // This is in OpenGL scale.
    }
}
