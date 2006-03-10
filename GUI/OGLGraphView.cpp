// OGLGraphView.cpp : OpenGL-View nur zusammen mit OGLEnabledView.cppp
//

#include "stdafx.h"
#include "math.h"
#include "OGLControl.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "OGLEnabledView.h"
#include "OGLGraphView.h"
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


//vector<CMSHElements*>msh_elements_vector;

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView
IMPLEMENT_DYNCREATE(COGLGraphView, COGLEnabledView)

BEGIN_MESSAGE_MAP(COGLGraphView, COGLEnabledView)
	//{{AFX_MSG_MAP(CGeoSysView)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysView construction/destruction

COGLGraphView::COGLGraphView()
{
  x_graph_min = 0.0;
  x_graph_max = 1000.0;
  y_graph_min = 0.0;
  y_graph_max = 1000.0;
}

COGLGraphView::~COGLGraphView()
{

}

void COGLGraphView::DoDataExchange(CDataExchange* pDX)
{
 	CView::DoDataExchange(pDX);
    On3DControl();
    
}

void COGLGraphView:: On3DControl()
{   
    

}

/////////////////////////////////////////////////////////////////////////////
// COGLGraphView diagnostics

#ifdef _DEBUG
void COGLGraphView::AssertValid() const
{
	COGLEnabledView::AssertValid();
}

void COGLGraphView::Dump(CDumpContext& dc) const
{
	COGLEnabledView::Dump(dc);
}

CGeoSysDoc* COGLGraphView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeoSysDoc)));
	return (CGeoSysDoc*)m_pDocument;
}
#endif //_DEBUG





void COGLGraphView::OnCreateGL()
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
	SetClearCol(16777215);//white
    //SetClearCol(15624229);//Blue
}


void COGLGraphView::OnDrawGL()
{
    	glPushMatrix();
        double ClipSize = 1.0;
//
        GetGraphMinMax(); 
        DrawQualityFactor();
        DrawOGLGraphAxis(); 

        glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); 
        double x1 = -0.05;
        double x2 = 1.05;
        double y1 = -0.05;
        double y2 = (y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*20*((y_graph_max-y_graph_min)/10))+0.1;
        double z1 = -ClipSize*(1);
        double z2 = +ClipSize*(1);        
        glOrtho(x1,x2,y1,y2,z1,z2);
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
       	glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glPushAttrib(GL_ENABLE_BIT);
		glPopAttrib();
 
        preRect = lpRect;         
        GetWindowRect(&lpRect);
        if (preRect != lpRect) {
        Invalidate(TRUE);
        }
        CGeoSysDoc *m_pDoc = GetDocument();
        m_pDoc->UpdateAllViews(this);


 //CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

// Get the active MDI child window.
//CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

// or CMDIChildWnd *pChild = pFrame->MDIGetActive();

// Get the active view attached to the active MDI child
// window.
//COGLGraphView *pView = (COGLGraphView *) pChild->GetActiveView();


}


void COGLGraphView::SetClearCol(COLORREF rgb)
{
	float r=float(GetRValue(rgb))/255;
	float g=float(GetGValue(rgb))/255;
	float b=float(GetBValue(rgb))/255;
	BeginGLCommands();
	glClearColor(r,g,b,1.0f);
	EndGLCommands();
	Invalidate();	// force redraw
}


void COGLGraphView::DrawOGLGraphAxis()
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
			 glColor3d(0.0,0.0,0.0);//Black
             //glColor3d(0.0,0.5,0.0);//Grey
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
			 glColor3d(0.0,0.0,0.0);//Black
             //glColor3d(0.0,0.5,0.0);//Grey
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
                glColor3d(0.0,0.0,0.0);//Black
                //glColor3d(0.7,0.7,0.7);//Grey
    			glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 4*((y_graph_max-y_graph_min)/10),0);
			    glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*16*((y_graph_max-y_graph_min)/10)),0);
				glEnd();

				glBegin(GL_LINES);
                glColor3d(0.0,0.0,0.0);//Black
                //glColor3d(0.7,0.7,0.7);//Grey
				glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			    glVertex3d(x_graph_min + (i+1)*(x_graph_max/20),y_graph_max + 3*((y_graph_max-y_graph_min)/10),0);
				glEnd();
				if (i==0){
				glBegin(GL_LINES);
                glColor3d(0.0,0.0,0.0);//Black
                //glColor3d(0.7,0.7,0.7);//Grey
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
             glColor3d(0.0,0.0,0.0);//Black
             //glColor3d(0.7,0.7,0.7);//Grey
			 glVertex3d(x_graph_min,y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max + 3.5*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_min,y_graph_max + 3.0*((y_graph_max-y_graph_min)/10),0);
			 glVertex3d(x_graph_max,y_graph_max + 3.0*((y_graph_max-y_graph_min)/10),0);
		     glEnd();

 			 glColor3d(0.0,0.0,0.0);
			 glRasterPos3d(x_graph_min,(y_graph_max + 4*((y_graph_max-y_graph_min)/10))+((norm_max_fac)*20*((y_graph_max-y_graph_min)/10)),0);
			 Text2D("Quality Histogram:");		

}

void COGLGraphView::DrawQualityFactor()
{
	int i=0;
	double qfactor;
	double norm_fac=0.0;
	long qiterator[20];
	long number_of_all_elements;
	double prozent;
	char txt[10];
	number_of_all_elements = (int)msh_elements_vector.size();
	//x_graph_min = 0.0;
	//x_graph_max = 1.5*y_graph_max;
	qiterator[0]=qiterator[1]=qiterator[2]=qiterator[3]=qiterator[4]=qiterator[5]=
    qiterator[6]=qiterator[7]=qiterator[8]=qiterator[9]=
	qiterator[10]=qiterator[11]=qiterator[12]=qiterator[13]=qiterator[14]=qiterator[15]=
    qiterator[16]=qiterator[17]=qiterator[18]=qiterator[19]=0;
	
	
	glBegin(GL_LINE_STRIP);		
	for (i=0;i<number_of_all_elements;i++)
	{ glColor3d(1.00-(msh_elements_vector[i]->quality_factor),msh_elements_vector[i]->quality_factor,0.0);
	  qfactor = msh_elements_vector[i]->quality_factor;
      glVertex3d((x_graph_max/number_of_all_elements)*i,msh_elements_vector[i]->quality_factor,0);
	  
	}
	glEnd();



			
	for (i=0;i<number_of_all_elements;i++)
	{ 
	  qfactor = msh_elements_vector[i]->quality_factor;
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
        glColor3d(0.0,0.0,0.0);//Black
        //glColor3d(0.6,0.6,0.6);//Grey

        glRasterPos3d(x_graph_min + (i)*(x_graph_max/20),y_graph_max + 3.05*((y_graph_max-y_graph_min)/10),0);
		Text2D(txt);	
	}

}

void COGLGraphView::GetGraphMinMax() 
{
	long number_of_all_elements = 0;
	int i=0;
	x_graph_min=y_graph_min=0.0;
	x_graph_max=y_graph_max=1.0;
	number_of_all_elements = (long)msh_elements_vector.size();  
		for (i=0;i<number_of_all_elements;i++)
		{
			if (msh_elements_vector[i]->quality_factor < y_graph_min) y_graph_min = msh_elements_vector[i]->quality_factor;
			if (msh_elements_vector[i]->quality_factor > y_graph_max) y_graph_max = msh_elements_vector[i]->quality_factor;
		}   
}

void COGLGraphView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
     pHint=pHint;//TK 
    pSender=pSender;//TK
  
    if(lHint==1 && init_view_check == 0) Invalidate();
    if(lHint==1)
    {
        init_view_check = 1;
    }
    else 
    {
        init_view_check = 0;
    }
}
