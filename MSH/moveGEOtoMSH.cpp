#include "stdafx.h"
#include"moveGEOtoMSH.h"

#ifdef MFC
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#endif

#include "msh_lib.h"
#include "math.h"
// C/C++
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <fstream>
using namespace std;
// GEOLib
//#include "test.h"
// MSHLib
#include "msh_node.h"
#include "msh_gen_pris.h"
#include "msh_nodes_rfi.h"
//FEMLib
#include "rf_mmp_new.h"
#include "files.h"
#include "gs_project.h"
#include "mathlib.h"
#include "tools.h"
#ifdef MFC    //CC
#include "geosys.h"
#endif



/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
ToDo evtl. vector<CGLPoint>
08/2005 CC Modification: CGLPoint* e_pnt - Move from GeoLib to MshLib
**************************************************************************/
void MSHGetNodesClose(vector<long>&msh_point_vector,CGLPoint* e_pnt)
{
  long i;
  CGLPoint m_pnt;
  // Node loop
  for (i=0;i<NodeListSize();i++) {
    if (GetNode(i)==NULL) continue;
    m_pnt.x = GetNodeX(i);
    m_pnt.y = GetNodeY(i);
    m_pnt.z = GetNodeZ(i);
    if(e_pnt->PointDis(&m_pnt)<=(e_pnt->epsilon+MKleinsteZahl))
      msh_point_vector.push_back(i);
  }
}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
08/2004 OK Implementation based on GetXNodeNumberClose by AH
08/2005 CC Modification CGLPoint* - Move from GeoLib to MSHLib
**************************************************************************/
long MSHGetNodeNumberClose(double *distance, CGLPoint* e_pnt)
{
 long i;
 double dist,distmin;
 double p1[3];
 double p2[3];
 Knoten *node=NULL,*node_tmp;
 long msh_node_number;

 p1[0]=e_pnt->x; p1[1]=e_pnt->y, p1[2]=e_pnt->z;
 msh_node_number=-1;
 distmin=1.e+300;
 for (i=0;i<NodeListLength;i++) {
   if ( (node_tmp=GetNode(i)) != NULL) {
         p2[0]=GetNodeX(i);
         p2[1]=GetNodeY(i);
         p2[2]=GetNodeZ(i);
   }
   else continue;

   dist=EuklVek3dDist(p1,p2);
   if (dist < distmin) {
     distmin=dist;
         msh_node_number=i;
         node=node_tmp;
   }
 }
 *distance=distmin;

 return msh_node_number;
}
/*************************************************************************
  ROCKFLOW - Function: MSHGetNodesClose
  Task: Searching grid points which are close to a polyline
  Programming: 
   10/2002 OK Encapsulated from ExecuteSourceSinkMethod11 (CT)
   01/2003 OK Test
  last modified: 20.01.2003 OK
   08/2005 CC Modification Move from GeoLib to MSHLib
**************************************************************************/
long* MSHGetNodesClose(long *number_of_nodes, CGLPolyline* m_ply)
{
  long j,k,l;
  double pt1[3],line1[3],line2[3],pt0[3];
  double mult_eps = 1.0;
  double dist1p,dist2p,*length,laenge;
  long *nodes_all = NULL;
  long anz_relevant = 0;
  typedef struct {
     long knoten;
     long abschnitt;
     double laenge;
  } INFO;
  INFO *relevant=NULL;
  int weiter;
  double w1,w2;
  long knoten_help;
  double laenge_help;
  double gesamte_laenge = 0.;
  long polyline_point_vector_size;

  m_ply->sbuffer.clear();
  m_ply->ibuffer.clear();

if (m_ply) {

  length = (double*) Malloc(sizeof(double) *(long)m_ply->point_vector.size());

  pt0[0] = m_ply->point_vector[0]->x;
  pt0[1] = m_ply->point_vector[0]->y;
  pt0[2] = m_ply->point_vector[0]->z;

  /* */
  polyline_point_vector_size =(long)m_ply->point_vector.size();
  for (k=0;k<polyline_point_vector_size-1;k++) {
    line1[0] = m_ply->point_vector[k]->x;
    line1[1] = m_ply->point_vector[k]->y;
    line1[2] = m_ply->point_vector[k]->z;
    line2[0] = m_ply->point_vector[k+1]->x;
    line2[1] = m_ply->point_vector[k+1]->y;
    line2[2] = m_ply->point_vector[k+1]->z;
    length[k] = MCalcDistancePointToPoint(line2, line1);
    gesamte_laenge += length[k];
  }

  /* Wiederholen bis zumindest ein Knoten gefunden wurde */
 while(anz_relevant==0) {

   /* Schleife ueber alle Knoten */
  for (j=0;j<NodeListSize();j++) {
    if (GetNode(j)==NULL) continue;

    /* Schleife ueber alle Punkte des Polygonzuges */
	polyline_point_vector_size =(long)m_ply->point_vector.size();
    for (k=0;k<polyline_point_vector_size-1;k++) {

      /* ??? */
      pt1[0] = GetNodeX(j);
      pt1[1] = GetNodeY(j);
      pt1[2] = GetNodeZ(j);

      line1[0] = m_ply->point_vector[k]->x;
      line1[1] = m_ply->point_vector[k]->y;
      line1[2] = m_ply->point_vector[k]->z;
      line2[0] = m_ply->point_vector[k+1]->x;
      line2[1] = m_ply->point_vector[k+1]->y;
      line2[2] = m_ply->point_vector[k+1]->z;

      /* Ist der Knoten nah am Polygonabschnitt? */
      if ( MCalcDistancePointToLine(pt1,line1,line2) <= mult_eps*m_ply->epsilon ) {
        /* Im folgenden wird mit der Projektion weitergearbeitet */
        MCalcProjectionOfPointOnLine(pt1,line1,line2,pt1);
        /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
        dist1p = MCalcDistancePointToPoint(line1, pt1);
        /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
        dist2p = MCalcDistancePointToPoint(line2, pt1);
        /* Ist der Knoten innerhalb des Intervalls? */
        /* bis rf3807: if ((length[k] - dist1p - dist2p + MKleinsteZahl)/(length[k] + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl){ */
        if ((dist1p+dist2p-length[k]) <=  mult_eps*m_ply->epsilon ) {

          // For boundary conditions. WW
          m_ply->sbuffer.push_back(dist1p);
          m_ply->ibuffer.push_back(k);
          // ---------------------------

          anz_relevant++;
          /* Feld anpassen */
          nodes_all = (long *) Realloc(nodes_all,sizeof(long)*anz_relevant);
          relevant = (INFO *) Realloc(relevant, sizeof(INFO) * anz_relevant);
          /* Ablegen von Knotennummer und Position */
          nodes_all[anz_relevant-1] = j;
          /* Position ermitteln */
          laenge = 0.;
          for (l=0; l < k; l++)
            laenge += length[l];
          /* Ablegen von Knotennummer und Position */
          relevant[anz_relevant-1].knoten = j;
          relevant[anz_relevant-1].laenge = laenge + dist1p;
          /* Suche am Polygon abbrechen, naechster Knoten */
          k =(long)m_ply->point_vector.size();
        }
      } /* endif */
    } /* Ende Schleife ueber Polygonabschnitte */
  } /* Ende Schleife ueber Knoten */
  if(anz_relevant==0) mult_eps *=2.;
 } /* Ende Schleife Wiederholungen */

  if (mult_eps > 1.)
     cout << "!!! Epsilon increased in sources!" << endl;

  /* Schleife ueber alle Knoten; sortieren nach Reihenfolge auf dem Abschnitt (zyklisches Vertauschen, sehr lahm)*/
  do {
    weiter = 0;
    for (k=0;k<anz_relevant-1;k++) {
       w1=relevant[k].laenge;
       w2=relevant[k+1].laenge;
       if (w1>w2) { /* Die Eintraege vertauschen */
          knoten_help = relevant[k].knoten;
          laenge_help = relevant[k].laenge;
          relevant[k].knoten = relevant[k+1].knoten;
          relevant[k].laenge = relevant[k+1].laenge;
          relevant[k+1].knoten = knoten_help;
          relevant[k+1].laenge = laenge_help;
          weiter=1;
         }
      }
  } while (weiter);

  relevant = (INFO*) Free(relevant);
  *number_of_nodes = anz_relevant;

  } /* if (polyline) */

  return nodes_all;
}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
08/2005 CC Modification Move from GeoLib to MSHLib
**************************************************************************/
long* MSHGetNodesCloseSorted(long* number_of_nodes,CGLPolyline* m_ply)
{
  long i;
  double pt1[3],pt2[3];
  long *nodes_unsorted = NULL;
  long* nodes_sorted = NULL;
  //long number_of_nodes;
  //...............................................................
  // Auxillary node_value vector
  nodes_unsorted = MSHGetNodesClose(number_of_nodes,m_ply);
  // Sort by distance
  pt1[0] = m_ply->point_vector[0]->x;
  pt1[1] =  m_ply->point_vector[0]->y;
  pt1[2] =  m_ply->point_vector[0]->z;
  double *node_distances = new double[*number_of_nodes];
  for(i=0;i<*number_of_nodes;i++) {
    pt2[0] = GetNodeX(nodes_unsorted[i]);
    pt2[1] = GetNodeY(nodes_unsorted[i]);
    pt2[2] = GetNodeZ(nodes_unsorted[i]);
    node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
  }
  nodes_sorted = TOLSortNodes1(nodes_unsorted,node_distances,*number_of_nodes);
  // Release memory
  delete [] node_distances;
  return nodes_sorted;
//  delete [] nodes_unsorted;
}
/**************************************************************************
GeoLib-Method: MSHCreateLines
Task:
Programing:
04/2004 OK
08/2005 CC move from GeoLib to MshLib
**************************************************************************/
void MSHCreateLines(CGLPolyline* m_ply)
{
  long j;
  long *nodes = NULL;
  long no_msh_nodes = (long)m_ply->msh_nodes_vector.size();
  for (j=0;j<no_msh_nodes-1;j++) {
    nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[0]);
    nodes[0] = m_ply->msh_nodes_vector[j];
    nodes[1] = m_ply->msh_nodes_vector[j+1];
    if((nodes[0]>-1)&&(nodes[1]>-1)) {
//OK      ELECreateTopology(1,-1,0,ElementListLength);
      ElSetElementNodes(ElementListLength-1,nodes);
      ElSetElementGroupNumber(ElementListLength-1,m_ply->mat_group);
      anz_1D++;
    }
    else {
      nodes = (long *)Free(nodes);
      //AfxMessageBox("OnMshLibLineGen: Grid nodes not found");
      printf("OnMshLibLineGen: Grid nodes not found");
    }
  }
}

//-----------------
inline double dotProduction(const double *x1,  const double *x2, const double *x0)
{
    return  (x1[0]-x0[0])*(x2[0]-x0[0])
           +(x1[1]-x0[1])*(x2[1]-x0[1])
           +(x1[2]-x0[2])*(x2[2]-x0[2]);
}

//WW
inline double Distance(const double *x1, const double *x2)
{
     return sqrt((x1[0]-x2[0])*(x1[0]-x2[0])
                +(x1[1]-x2[1])*(x1[1]-x2[1])
                +(x1[2]-x2[2])*(x1[2]-x2[2]));

}
/**************************************************************************
GeoLib-Method: MSHCreateLines
Task:
Programing:
04/2004 WW
08/2005 CC move from geolib to mshlib
**************************************************************************/
long *GetNodesOnArc(long &NumNodes, CGLPolyline* m_ply)  
{
   long i;

   //Obtain fem node for groupvector
   CGLPoint *CGPa=NULL, *CGPb=NULL, *CGPc=NULL;
   const long nNodes = NodeListSize();   
   const int SizeCGLPoint = (int)m_ply->point_vector.size();
   double r1, r2,a0,a1,a2;
   const double Tol = 1.0e-6;
   double xa[3],xb[3],xc[3], xn[3];

   vector<long> nodesOnArc;
   long* NodesOnArc=NULL;

   if(SizeCGLPoint>3)
   {
       cout<<"More than 3 points that an arc needed"<<endl;
       abort();
   }
   else if (SizeCGLPoint==0)
   {
       cout<<"No point are given for this arc"<<endl;
       abort();      
   }
   CGPa = m_ply->point_vector[0];
   CGPb = m_ply->point_vector[2];
   CGPc = m_ply->point_vector[1];
   xa[0] = CGPa->x;
   xa[1] = CGPa->y;
   xa[2] = CGPa->z;
   xb[0] = CGPb->x;
   xb[1] = CGPb->y;
   xb[2] = CGPb->z;
   xc[0] = CGPc->x;
   xc[1] = CGPc->y;
   xc[2] = CGPc->z;

   r1 = Distance(xa,xc);
   r2 = Distance(xb,xc);


   if(fabs(r1-r2)>m_ply->epsilon)
   {
       cout<<"Start point and end point do not have identical "
       <<"distance to the center of the arc"<<endl;
       abort();
   }
   a0 = acos(dotProduction(xa,xb, xc)/(r1*r1));
    
   // Check nodes by comparing distance 
   for(i=0; i<nNodes; i++)
   {        
      xn[0] = GetNodeX(i);
      xn[1] = GetNodeY(i);
      xn[2] = GetNodeZ(i);
      r2 =  Distance(xn,xc);
      if(fabs(r2-r1)<m_ply->epsilon)
      {
         if(a0<Tol) // Closed arc
             nodesOnArc.push_back(i);
         else
         {
            a1 = acos(dotProduction(xa,xn, xc)/(r1*r1));
            a2 = acos(dotProduction(xb,xn, xc)/(r1*r1));
            if(fabs(a1+a2-a0)<Tol)  
               nodesOnArc.push_back(i);
         }
      }
   }

   NumNodes = (int)nodesOnArc.size();
   // to fit the vector, nodes, in rf_bc_new
   NodesOnArc = (long *) Malloc(sizeof(long)*NumNodes);
   for(i=0; i<NumNodes; i++)
     NodesOnArc[i] = nodesOnArc[i];
   nodesOnArc.clear();
   
   // return
   return NodesOnArc;
}
/**************************************************************************
GeoLib-Method: GetPointsIn
Task: 
Programing:
01/2004 OK Implementation
08/2005 CC Modification Move from Geolib to Mshlib
**************************************************************************/
long* GetPointsIn(Surface* m_sfc,long* number_of_nodes)
{
  long *nodes = NULL;
  long i;
  double *xp=NULL,*yp=NULL,*zp=NULL;
  long anz_relevant = 0;
  CGLPoint m_pnt;
  /* Inside polygon */
  if(!m_sfc->polygon_point_vector.empty()) {
    xp = (double*) Malloc(((long)m_sfc->polygon_point_vector.size())*sizeof(double));
    yp = (double*) Malloc(((long)m_sfc->polygon_point_vector.size())*sizeof(double));
    zp = (double*) Malloc(((long)m_sfc->polygon_point_vector.size())*sizeof(double));
    long polygon_point_vector_length = (long)m_sfc->polygon_point_vector.size();
    for(i=0;i<polygon_point_vector_length;i++) {
      xp[i] = m_sfc->polygon_point_vector[i]->x;
      yp[i] = m_sfc->polygon_point_vector[i]->y;
      zp[i] = m_sfc->polygon_point_vector[i]->z;
    }

    /*-----------------------------------------------------------------*/
    for(i=0;i<NodeListSize();i++) {
      if (GetNode(i)==NULL) continue;
      m_pnt.x = GetNodeX(i);
      m_pnt.y = GetNodeY(i);
      m_pnt.z = GetNodeZ(i);
      if(m_pnt.IsInsidePolygonPlain(
                                   xp,yp,zp,\
                                   (long)m_sfc->polygon_point_vector.size())) {
        anz_relevant++;
        /* Feld anpassen */
        nodes = (long *) Realloc(nodes,sizeof(long)*anz_relevant);
        /* Ablegen von Knotennummer und Position */
        nodes[anz_relevant-1] = i;
      }  /* end if inside polygon */
    } /* end for node loop */
    /*-------------------------------------------------------------------*/
  }
  // Destructions
   // nodes extern
  xp = (double*) Free(xp);
  yp = (double*) Free(yp);
  zp = (double*) Free(zp);
   //
  *number_of_nodes = anz_relevant;
  return nodes;
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
01/2005 OK TIN case
08/2005 CC move from Geolib to Mshlib
last modification:
**************************************************************************/
vector<long> GetMSHNodesClose(Surface* m_sfc)
{
  long i,j;
  long no_nodes = 0;
  long *nodes_array = NULL;
  long no_points;
  CGLPoint m_node;
  CGLPolyline* m_polyline = NULL;
  CGLPolyline* m_polyline1 = NULL;
  CGLPolyline* m_polyline2 = NULL;
  vector<long>nodes_vector;
  double xp[3],yp[3],zp[3];
  CTriangle *m_triangle = NULL;
  long no_triangles;

  //list<CGLPolyline*>::const_iterator p = m_sfc->polyline_of_surface_list.begin();
vector<CGLPolyline*>::iterator p = m_sfc->polyline_of_surface_vector.begin();
  nodes_vector.clear();

  switch(m_sfc->type) {
    //--------------------------------------------------------------------
    case 0: // surface polygon
      nodes_array = GetPointsIn(m_sfc,&no_nodes);//change this into m_sfc todo
      for(i=0;i<no_nodes;i++)
        nodes_vector.push_back(nodes_array[i]);
      break;
    //--------------------------------------------------------------------
    case 1: // TIN
//OK41
      no_triangles = (long)m_sfc->TIN->Triangles.size();
      for(i=0;i<no_triangles;i++){
        m_triangle = m_sfc->TIN->Triangles[i];
        xp[0] = m_triangle->x[0];
        yp[0] = m_triangle->y[0];
        zp[0] = m_triangle->z[0];
        xp[1] = m_triangle->x[1];
        yp[1] = m_triangle->y[1];
        zp[1] = m_triangle->z[1];
        xp[2] = m_triangle->x[2];
        yp[2] = m_triangle->y[2];
        zp[2] = m_triangle->z[2];
        for(j=0;j<NodeListSize();j++){
          m_node.x = GetNodeX(j);
          m_node.y = GetNodeY(j);
          m_node.z = GetNodeZ(j);
          if(m_node.IsInsideTriangle(xp,yp,zp)){
            nodes_vector.push_back(j);
          }
        }
      }
      break;
    //--------------------------------------------------------------------
    case 2: // 2 vertical polylines //OK
      // .................................................................
      // nodes close to first polyline 
      p = m_sfc->polyline_of_surface_vector.begin();
      while(p!=m_sfc->polyline_of_surface_vector.end()) {
        m_polyline = *p;
        nodes_array = MSHGetNodesClose(&no_nodes,m_polyline);//CC
        break;
      }
      // .....................................................................
      // using triangles
      p = m_sfc->polyline_of_surface_vector.begin();
      while(p!=m_sfc->polyline_of_surface_vector.end()) {
        m_polyline1 = *p; 
        ++p;
        m_polyline2 = *p;
        break;
      }
      no_points = (long)m_polyline1->point_vector.size();

      for(j=0;j<no_nodes;j++) {
        m_node.x = GetNodeX(nodes_array[j]);
        m_node.y = GetNodeY(nodes_array[j]);
        m_node.z = GetNodeZ(nodes_array[j]);
        for(i=0;i<no_points-1;i++) {
          // first triangle of quad
          xp[0] = m_polyline1->point_vector[i]->x; 
          yp[0] = m_polyline1->point_vector[i]->y; 
          zp[0] = m_polyline1->point_vector[i]->z; 
          xp[1] = m_polyline1->point_vector[i+1]->x; 
          yp[1] = m_polyline1->point_vector[i+1]->y; 
          zp[1] = m_polyline1->point_vector[i+1]->z; 
          xp[2] = m_polyline2->point_vector[i]->x; 
          yp[2] = m_polyline2->point_vector[i]->y; 
          zp[2] = m_polyline2->point_vector[i]->z; 
          if(m_node.IsInsideTriangle(xp,yp,zp)) {
            nodes_vector.push_back(nodes_array[j]);
          }
          // second triangle of quad
          xp[0] = m_polyline2->point_vector[i]->x; 
          yp[0] = m_polyline2->point_vector[i]->y; 
          zp[0] = m_polyline2->point_vector[i]->z; 
          xp[1] = m_polyline2->point_vector[i+1]->x; 
          yp[1] = m_polyline2->point_vector[i+1]->y; 
          zp[1] = m_polyline2->point_vector[i+1]->z; 
          xp[2] = m_polyline1->point_vector[i+1]->x; 
          yp[2] = m_polyline1->point_vector[i+1]->y; 
          zp[2] = m_polyline1->point_vector[i+1]->z; 
          if(m_node.IsInsideTriangle(xp,yp,zp)) {
            nodes_vector.push_back(nodes_array[j]);
          }
        } // no_points
      } // no_nodes

      break;
  }
  return nodes_vector;
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
03/2004 OK Implementation
last modification:08/2005 CC move
**************************************************************************/
void CreateTINfromMSH(Surface* m_sfc,long i)
{
  int j;
  CGLPoint m_point;
  CTriangle *m_triangle;
  long *nodes = NULL;

   if(ElGetElementType(i)==4) { // use only triangles
    // calc element mid point
    nodes = ElGetElementNodes(i);
    m_point.x = 0.0;
    m_point.y = 0.0;
    m_point.z = 0.0;
    for(j=0;j<3;j++) {
      m_point.x += GetNodeX(nodes[j]);
      m_point.y += GetNodeY(nodes[j]);
      m_point.z += GetNodeZ(nodes[j]);
    }
    m_point.x *= Mdrittel;
    m_point.y *= Mdrittel;
    m_point.z *= Mdrittel;
    if(IsPointInSurface(m_sfc,&m_point)) {//CC
      m_triangle = new CTriangle;
      m_triangle->number = (long)m_sfc->TIN->Triangles.size();
      for(j=0;j<3;j++) {
        m_triangle->x[j] = GetNodeX(nodes[j]);
        m_triangle->y[j] = GetNodeY(nodes[j]);
        m_triangle->z[j] = GetNodeZ(nodes[j]);
      }
      m_sfc->TIN->Triangles.push_back(m_triangle);
    }    
   } // Triangles
}

/**************************************************************************
  ROCKFLOW - Modul: attach(const char *dateiname, const int NNodes)
                                                                          
  Aufgabe:
   Moves Z-Values of nodes according to a specified surface in 
   filename.dat file. Works for regular grids only!!!
   Eg.: 

   const char *dateiname     :   file name
   const int NLayers         :   Number of layers in the
   const int row             :   row number of the row to be mapped
                                                                          
   Programmaenderungen:
   10/2003     WW/MB     Erste Version
   03/2004 OK extension for TINs
   08/2005 CC move from Geolib to mshlib                                                                        
***************************************************************************/
void MapTIN(Surface* m_sfc, string map_file_name)
{
  FILE *f;
  fpos_t pos;

  char *s;  /* gelesene Zeile */
  int i,j,k;
  int ncols = 0;
  int nrows = 0;
 
  int nx, ny;
  int NPoints = 0;
//  int count = 0;

  double x,y, z;

  double MinX = 1.0e+10; 
  double MinY = 1.0e+10; 
  double MaxX = -1.0e+10; 
  double MaxY = -1.0e+10; 
  double dx =  1.0e+10;
  double dy =  1.0e+10;

  double xi =  0.0;
  double eta =  0.0;
   
  double locX[4];
  double locY[4];
  double locH0[4];

  double* GridX =NULL;  
  double* GridY =NULL;  
  double* H0=NULL;
  double **H=NULL;
   
  double ome[4];

  const int NNodes=NodeListSize();
  int NNodesPerRow = 0;

 // const char* xxxxx;
  char charbuff[41];

  //-----------------------------------------------------------------------
  // Open grid file
  if ((f = fopen(map_file_name.c_str(),"rb"))==NULL)  {
      DisplayErrorMsg("Fehler: Cannot open .dat file. It may not exist !");
      abort();
  }
  s = (char *) Malloc(MAX_ZEILE);
  //-----------------------------------------------------------------------
  // File type
  int pos_ext = (int)map_file_name.find_first_of(".");
  string extension = map_file_name.assign(map_file_name,pos_ext,string::npos);
  int DataType=0;
  if(extension.compare(".dat")==0)DataType = 1;
  if(extension.compare(".asc")==0)DataType = 2;
  switch(DataType)
  {
    //.....................................................................
    case 1: // SURFER file
       /* Count number of points of the grid */
       if( fgetpos(f, &pos ) != 0 )  {
         perror( "fgetpos error" );
         abort();
       }
       else  { 
         NPoints=-1;
         while(!feof(f))    {
           fgets(s,MAX_ZEILE,f);
           NPoints++;      
         }
       }

       rewind(f);
       fsetpos(f, &pos ); 

       /* Allocate memory for grid and the specified surface*/
       GridX = (double *) Malloc(NPoints * sizeof(double));
       GridY = (double *) Malloc(NPoints * sizeof(double));
       H0 = (double *) Malloc(NPoints * sizeof(double));

       /* Read surface data and determine the range*/
       for(i=0; i<NPoints; i++)   {
         if(fgets(s,MAX_ZEILE,f)==NULL)	 {
           //DisplayErrorMsg("Error: End of .grd file reached ");
#ifdef MFC
           AfxMessageBox("1");
#endif
           abort();
         }

         if (sscanf(s," %lf %lf %lf  ", &(GridX[i]),&(GridY[i]),&(H0[i]))!=3) 	   {
           //DisplayErrorMsg("Error: More than one integal in .grd file!");
#ifdef MFC
           AfxMessageBox("2");
#endif
           abort();
         }

       }

	   for(i=0; i<NPoints; i++)   {
          // Determine the range of the grid  
          if(GridX[i]>=MaxX) MaxX = GridX[i];   
          if(GridY[i]>=MaxY) MaxY = GridY[i];   
          if(GridX[i]<=MinX) MinX = GridX[i];   
          if(GridY[i]<=MinY) MinY = GridY[i];
	 
          if(i>0&&(fabs(GridX[i]-GridX[i-1])> MKleinsteZahl)) 
          if(dx>=fabs(GridX[i]-GridX[i-1])) dx = fabs(GridX[i]-GridX[i-1]);
          if(i>0&&(fabs(GridY[i]-GridY[i-1])< MKleinsteZahl)) 
	      if(dy>=fabs(GridY[i]-GridY[i-1])) dy = fabs(GridY[i]-GridY[i-1]);
       } 
	   break;
    //.....................................................................
	 case 2:
       ncols = 0;
       nrows = 0;
       
	   double x0, y0, z0;

       fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %i  ", charbuff, &ncols);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d ", charbuff, &nrows);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &x0);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &y0);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &dx);
	   dy = dx;

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &z0);

       MinX = x0;
       MaxX = x0+dx*ncols;
       MinY = y0;
       MaxY = y0+dy*nrows;

       // Allocate memory for grid and the specified surface
       H = new double*[nrows];
	   for(i=0; i<nrows; i++)  H[i] = new double[ncols];

	   // Compute the grid points:
	   for(i=0; i<nrows; i++) 
	   {          
    	  for(j=0; j<ncols; j++)
		  {
             fscanf(f,"%lf", &H[i][j]);
		  } 
	   } 
	   //
       break; 
  }

  int NLayers = 0;
  NNodesPerRow = NNodes / (NLayers+1); 
  // 1. Compute the height of points to be attached
  long no_surface_triangles = (long)m_sfc->TIN->Triangles.size();
  for(i=0;i<no_surface_triangles;i++) {
    for(j=0;j<3;j++) {
      //if(i >= (row-1) * NNodesPerRow  &&  i <= (row * NNodesPerRow) -1 ) {           
        x = m_sfc->TIN->Triangles[i]->x[j];
        y = m_sfc->TIN->Triangles[i]->y[j];

        if(x<MinX||x>MaxX||y<MinY||y>MaxY)   {
          //DisplayErrorMsg("Error: The X_Y range of the specified surface is too small !");
#ifdef MFC
          AfxMessageBox("3");
#endif
          abort();
	    }
	    nx = (int)((x-MinX)/dx);	
        ny = (int)((y-MinY)/dy);
        
        if(nx*dx+MinX>=x)  nx -= 1;
        if(ny*dy+MinY>=y)  ny -= 1;
	    if(nx>ncols) nx = ncols-2;
	    if(ny>nrows) ny = nrows-2;
        if(nx<0) nx = 0;
        if(ny<0) ny = 0;
         
	    locX[0] = MinX+nx*dx;
	    locY[0] = MinY+ny*dy;
	    locH0[0] = H[ny][nx];

        locX[1] = MinX+(nx+1)*dx;
	    locY[1] = MinY+ny*dy;
	    locH0[1] = H[ny+1][nx];

	    locX[2] = MinX+(nx+1)*dx;
	    locY[2] = MinY+(ny+1)*dy;
	    locH0[2] = H[ny+1][nx+1];

	    locX[3] = MinX+nx*dx;
	    locY[3] = MinY+(ny+1)*dy;
	    locH0[3] = H[ny][nx+1];

        // Interpolate 
        xi =  2.0*(x-0.5*(locX[0]+locX[1]))/dx;             
        eta =  2.0*(y-0.5*(locY[1]+locY[2]))/dy;              
        MPhi2D(ome, xi, eta);   
        
        z=0.0;
        for(k=0;k<4;k++) 
          z += ome[k]*locH0[k]; 
        m_sfc->TIN->Triangles[i]->z[j] = z;
      //}
    }
  }

  /* Release memory*/
  if(GridX) GridX  = (double*) Free(GridX);
  if(GridY) GridY  = (double*) Free(GridY);
  if(H0) H0 = (double*) Free(H0);
  if(s) s = (char *)Free(s);
  fclose(f);
}
/**************************************************************************
GeoLib-Method: 
Task: Geometric / topological method
Programing:
01/2005 OK Implementation
01/2005 CC add variable :long i for the function
08/2005 CC move from Geolib to Mshlib
**************************************************************************/
void CreateLayerSurfacesTINs(Surface* m_mfc,long i)
{
  double pt0[3],pt1[3];
  long* nodes = NULL;
  double eps = 1.;
  int k;
  Surface* m_sfc = NULL;
  CTriangle *m_triangle = NULL;
  //CTIN *m_TIN = NULL;
  string sfc_layer_name = m_mfc->name + "_layer_";
  string sfc_layer_name_this;
  //---------------------------------------------------------------------
 /* if(!TIN){
    AfxMessageBox("No surface TIN");
    return;
  }*/
  //---------------------------------------------------------------------
  // Determine number of layers
  int no_layers = (int)(msh_no_pris/msh_no_tris);
  //---------------------------------------------------------------------
  char layer_number[3];

  //---------------------------------------------------------------------
  // Collect MSH triangles of TIN surface
  //long no_triangles = (long)TIN->Triangles.size();
  //CC 01/2005

    // Element mid point of TIN element
    pt0[0] = 0.0;
    pt0[1] = 0.0;
    for(k=0;k<3;k++) {
      pt0[0] += m_mfc->TIN->Triangles[i]->x[k];
      pt0[1] += m_mfc->TIN->Triangles[i]->y[k];
    }
    pt0[0] *= Mdrittel;
    pt0[1] *= Mdrittel;
    pt0[2] = 0.0;
    for(long j=0;j<msh_no_tris;j++) {
      if(ElGetElementType(j)==4){ // triangles
        nodes = ElGetElementNodes(j);
        pt1[0] = 0.0;
        pt1[1] = 0.0;
        for(k=0;k<3;k++) {
          pt1[0] += GetNodeX(nodes[k]);
          pt1[1] += GetNodeY(nodes[k]);
        }
        pt1[0] *= Mdrittel;
        pt1[1] *= Mdrittel;
        pt1[2] = 0.0;
        double dist = MCalcDistancePointToPoint(pt0,pt1);
        // MSH triangle found
        if(dist<eps){
          for(int l=1;l<no_layers+1;l++){
            nodes = ElGetElementNodes(j+l*msh_no_tris);
            m_triangle = new CTriangle;
            for(k=0;k<3;k++) {
              m_triangle->number = m_mfc->TIN->Triangles[i]->number;
              m_triangle->x[k] = m_mfc->TIN->Triangles[i]->x[k];
              m_triangle->y[k] = m_mfc->TIN->Triangles[i]->y[k];
              m_triangle->z[k] = GetNodeZ(nodes[k+3]);
              
            }
            sprintf(layer_number,"%i",l);
            sfc_layer_name_this = sfc_layer_name + layer_number;            
            m_sfc = GEOGetSFCByName(sfc_layer_name_this);//CC
            
            if(m_sfc)
              m_sfc->TIN->Triangles.push_back(m_triangle);
          }
        }
      }
    }
 
}
/**************************************************************************
GeoLib-Method: 
Task: MSH nodes close (eps) to a plane surface defined by 3 points
Programing:
08/2004 OK based ExecuteBoundaryConditionsMethod7(BOUNDARY_CONDITIONS *bc) by AH
last modification:
**************************************************************************/
void GetMSHNodesCloseAH(Surface* m_mfc,vector<long>&sfc_points)
{
  long j;
  double xp, yp, zp, dist;
  double x1, y1, z1, x2, y2, z2, x3, y3, z3, dist12, dist13;
  double dp1[3], d12[3], d13[3], dn[3], p1[3], p[3];
  double h, bn;

  double radius = -1.;
  double d_eps = 10. * MKleinsteZahl;
  if (radius < 0.0)
    radius = d_eps;
  //----------------------------------------------------------------------
  x1 = m_mfc->polygon_point_vector[0]->x;
  y1 = m_mfc->polygon_point_vector[0]->y;
  z1 = m_mfc->polygon_point_vector[0]->z;
  x2 = m_mfc->polygon_point_vector[1]->x;
  y2 = m_mfc->polygon_point_vector[1]->y;
  z2 = m_mfc->polygon_point_vector[1]->z;
  x3 = m_mfc->polygon_point_vector[2]->x;
  y3 = m_mfc->polygon_point_vector[2]->y;
  z3 = m_mfc->polygon_point_vector[2]->z;
  //----------------------------------------------------------------------
  d12[0] = x2 - x1;
  d12[1] = y2 - y1;
  d12[2] = z2 - z1;
  dist12 = MBtrgVec(d12, 3);
  if (dist12 < 0)
    return;
  d13[0] = x3 - x1;
  d13[1] = y3 - y1;
  d13[2] = z3 - z1;
  dist13 = MBtrgVec(d13, 3);
  if (dist13 < 0)
    return;
  //----------------------------------------------------------------------
  M3KreuzProdukt(d12, d13, dn);
  if ((bn = MBtrgVec(dn, 3)) < d_eps)
    return;
  else
    for (j = 0; j < 3; j++)
      dn[j] /= bn;

  p1[0] = x1;
  p1[1] = y1;
  p1[2] = z1;
  h = MSkalarprodukt(p1, dn, 3);
  if (h < 0)
    {
      h *= -1.;
      for (j = 0; j < 3; j++)
        dn[j] *= -1.;
    }
  //----------------------------------------------------------------------
  for (j = 0; j < NodeListSize(); j++)
    {
      if (GetNode(j) == NULL)
        continue;
      p[0] = xp = GetNodeX(j);
      p[1] = yp = GetNodeY(j);
      p[2] = zp = GetNodeZ(j);
      dp1[0] = xp - x1;
      dp1[1] = yp - y1;
      dp1[2] = zp - z1;
      dist = MBtrgVec(dp1, 3);
      if (dist < 0)
        return;

      if (fabs(MSkalarprodukt(dp1, dn, 3)) < d_eps || \
          fabs(MSkalarprodukt(p, dn, 3) - h) <= radius){
        sfc_points.push_back(j);
      }
    } 

  return;
}
/**************************************************************************
GeoLib-Method: 
Task: Get nodes on plane surface by comparing the area of polygon computed
      by triangles, which are formed by node and the gravity center 
      with edges of polygon, respectively  
Programing:
09/2004 WW Implementation
last modification:08/2005 CC 
**************************************************************************/
void GetMSHNodesOnSurface(Surface* m_mfc,vector<long>& NodesS)
{
  long i,j,k;
  int nPointsPly = 0;
  const int nNodes = NodeListSize();   
  double gC[3],p1[3],p2[3];
  double Area1, Area2;
  double Tol = 1.0e-9;

  CGLPolyline* m_polyline = NULL;

  //list<CGLPolyline*>::const_iterator p = m_mfc->polyline_of_surface_vector.begin();
vector<CGLPolyline*>::iterator p = m_mfc->polyline_of_surface_vector.begin();
  NodesS.clear();

  // nodes close to first polyline 
  p = m_mfc->polyline_of_surface_vector.begin();
  while(p!=m_mfc->polyline_of_surface_vector.end()) {
     m_polyline = *p;
     // Grativity center of this polygon
     for(i=0; i<3; i++) gC[i] = 0.0;
     nPointsPly = (int)m_polyline->point_vector.size();
     for(i=0; i<nPointsPly; i++)
     { 
         gC[0] += m_polyline->point_vector[i]->x;
         gC[1] += m_polyline->point_vector[i]->y;
         gC[2] += m_polyline->point_vector[i]->z;
     } 
     for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;

     // Area of this polygon by the grativity center
     Area1 = 0.0;
     for(i=0; i<nPointsPly; i++)
     { 
         p1[0] = m_polyline->point_vector[i]->x;
         p1[1] = m_polyline->point_vector[i]->y;
         p1[2] = m_polyline->point_vector[i]->z;
         if(i<nPointsPly-1)
         {
            p2[0] = m_polyline->point_vector[i+1]->x;
            p2[1] = m_polyline->point_vector[i+1]->y;
            p2[2] = m_polyline->point_vector[i+1]->z;
         }
         else
         {
            p2[0] = m_polyline->point_vector[0]->x;
            p2[1] = m_polyline->point_vector[0]->y;
            p2[2] = m_polyline->point_vector[0]->z;
         }
         Area1 += fabs(MSHCalcTriangleArea3 (p1, gC, p2));
     } 

     // Check nodes by comparing area 
     for(j=0; j<nNodes; j++)
     {        
        Area2 = 0.0;
        gC[0] = GetNodeX(j);
        gC[1] = GetNodeY(j);
        gC[2] = GetNodeZ(j);

        for(i=0; i<nPointsPly; i++)
        { 
            p1[0] = m_polyline->point_vector[i]->x;
            p1[1] = m_polyline->point_vector[i]->y;
            p1[2] = m_polyline->point_vector[i]->z;
            
            k = i+1;
            if(i==nPointsPly-1)
               k = 0;
            p2[0] = m_polyline->point_vector[k]->x;
            p2[1] = m_polyline->point_vector[k]->y;
            p2[2] = m_polyline->point_vector[k]->z;

           Area2 += fabs(MSHCalcTriangleArea3 (p1, gC, p2));
        }
        if(fabs(Area1-Area2)<Tol) 
            NodesS.push_back(j);
     } 
      
     p++;
  }
}
/**************************************************************************
GeoLib-Method: 
Task: Get nodes on plane surface by comparing the area of polygon computed
      by triangles, which are formed by node and the gravity center 
      with edges of polygon, respectively  
Programing:
09/2004 WW Implementation
last modification:
**************************************************************************/
void GetMSHNodesOnSurfaceXY(Surface* m_mfc,vector<long>& NodesS)
{
  long i,j,k;
  int nPointsPly = 0;
  const int nNodes = NodeListSize();   
  double gC[3],p1[3],p2[3];
  double Area1, Area2;
  double Tol = 1.0e-9;

  CGLPolyline* m_polyline = NULL;

  //list<CGLPolyline*>::const_iterator p = m_mfc->polyline_of_surface_list.begin();
vector<CGLPolyline*>::iterator p = m_mfc->polyline_of_surface_vector.begin();
  NodesS.clear();

  // nodes close to first polyline 
  p = m_mfc->polyline_of_surface_vector.begin();
  while(p!=m_mfc->polyline_of_surface_vector.end()) {
     m_polyline = *p;
     // Grativity center of this polygon
     for(i=0; i<3; i++) gC[i] = 0.0;
     nPointsPly = (int)m_polyline->point_vector.size();
     for(i=0; i<nPointsPly; i++)
     { 
         gC[0] += m_polyline->point_vector[i]->x;
         gC[1] += m_polyline->point_vector[i]->y;
         //gC[2] += m_polyline->point_vector[i]->z;
		 gC[2] += 0.0;
     } 
     for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;

     // Area of this polygon by the grativity center
     Area1 = 0.0;
     for(i=0; i<nPointsPly; i++)
     { 
         p1[0] = m_polyline->point_vector[i]->x;
         p1[1] = m_polyline->point_vector[i]->y;
         //p1[2] = m_polyline->point_vector[i]->z;
		 p1[2] = 0.0;
         if(i<nPointsPly-1)
         {
            p2[0] = m_polyline->point_vector[i+1]->x;
            p2[1] = m_polyline->point_vector[i+1]->y;
            //p2[2] = m_polyline->point_vector[i+1]->z;
			p2[2] = 0.0;
         }
         else
         {
            p2[0] = m_polyline->point_vector[0]->x;
            p2[1] = m_polyline->point_vector[0]->y;
            //p2[2] = m_polyline->point_vector[0]->z;
			p2[2] = 0.0;
         }
         Area1 += fabs(MSHCalcTriangleArea3 (p1, gC, p2));
     } 

     // Check nodes by comparing area 
     for(j=0; j<nNodes; j++)
     {        
        Area2 = 0.0;
        gC[0] = GetNodeX(j);
        gC[1] = GetNodeY(j);
        //gC[2] = GetNodeZ(j);
		gC[2] = 0.0;


        for(i=0; i<nPointsPly; i++)
        { 
            p1[0] = m_polyline->point_vector[i]->x;
            p1[1] = m_polyline->point_vector[i]->y;
            //p1[2] = m_polyline->point_vector[i]->z;
            p1[2] = 0.0;
            
            k = i+1;
            if(i==nPointsPly-1)
               k = 0;
            p2[0] = m_polyline->point_vector[k]->x;
            p2[1] = m_polyline->point_vector[k]->y;
            //p2[2] = m_polyline->point_vector[k]->z;
            p2[2] = 0.0;
            Area2 += fabs(MSHCalcTriangleArea3 (p1, gC, p2));
        }
        if(fabs(Area1-Area2)<Tol) 
            NodesS.push_back(j);
     } 
      
     p++;
  }
}
/**************************************************************************
GeoLib-Method: 
Task: Get nodes on cylindrical surface by comparing the area of 
      triangles form by nodes and two axis points
Programing:
10/2004 WW Implementation
last modification:08/2005 CC move from geolib to mshlib
**************************************************************************/
void GetMSHNodesOnCylindricalSurface(Surface* m_mfc,vector<long>& NodesS)
{
  long j;
  const int nNodes = NodeListSize();   
  double gC[3],p1[3],p2[3];
  double dist, R, dc1, dc2;

  NodesS.clear();

  p1[0] = m_mfc->polygon_point_vector[0]->x;
  p1[1] = m_mfc->polygon_point_vector[0]->y;
  p1[2] = m_mfc->polygon_point_vector[0]->z;

  p2[0] = m_mfc->polygon_point_vector[1]->x;
  p2[1] = m_mfc->polygon_point_vector[1]->y;
  p2[2] = m_mfc->polygon_point_vector[1]->z;

  dist = sqrt( (p1[0]-p2[0])*(p1[0]-p2[0])
                    +(p1[1]-p2[1])*(p1[1]-p2[1])
                    +(p1[2]-p2[2])*(p1[2]-p2[2]));

  // Check nodes by comparing area 
  for(j=0; j<nNodes; j++)
  {        
     gC[0] = GetNodeX(j);
     gC[1] = GetNodeY(j);
     gC[2] = GetNodeZ(j);

     dc1 =  (p2[0]-p1[0])*(gC[0]-p1[0])
           +(p2[1]-p1[1])*(gC[1]-p1[1])
           +(p2[2]-p1[2])*(gC[2]-p1[2]); 
     dc2 =  (p2[0]-p1[0])*(gC[0]-p2[0])
           +(p2[1]-p1[1])*(gC[1]-p2[1])
           +(p2[2]-p1[2])*(gC[2]-p2[2]); 
     if(dc1<0.0) continue;
     if(dc2>0.0) continue;

     R = 2.0*fabs(MSHCalcTriangleArea3 (p1, gC, p2))/dist;

     if(fabs(R-m_mfc->Radius)<m_mfc->epsilon) 
         NodesS.push_back(j);
  } 
      
}
/**************************************************************************
GeoLib-Method: GetPointsInVolume
Task: 
Programing:
01/2004 MB Vorbereitung 
09/2005 CC move from Geolib to Mshlib?? m_vol no reference??
**************************************************************************/
long* GetPointsInVolume(CGLVolume* m_vol,long* number_of_nodes)
{
m_vol = m_vol; //OK
  long *nodes = NULL;
  long i;
  long anz_relevant = 0;
 
  for(i=0;i<NodeListLength;i++){
    /* Feld anpassen */
    anz_relevant++;    
    nodes = (long *) Realloc(nodes,sizeof(long)*anz_relevant);
    /* Ablegen von Knotennummer und Position */
    nodes[anz_relevant-1] = i;
  }

  *number_of_nodes = anz_relevant;
  return nodes;
}

/**************************************************************************
GeoLib-Method: GEOGetVolume
Task: Get element nodes in a material domain
Programing:
10/2004 WW Implementation
**************************************************************************/
void GEOGetNodesInMaterialDomain(const int MatIndex, vector<long>& Nodes)
{
   long index, *element_nodes;
   int i, j, Size, nn, order = 2;
   const int L_Nodes = GetLowOrderNodeNumber();   
   bool exist;
   if(L_Nodes==NodeListSize()) order = 1;
   if(L_Nodes==0) order = 1;

   Nodes.resize(0);
   nn = 0;
   for (index=0;index<ElListSize();index++)
   {
      if (ElGetElement(index)!=NULL)
      {  // Eelement exist
         if (ElGetElementActiveState(index))
         {  // Element active
            if(order==1) nn = NumbersOfElementNode(index);
            if(order==2) nn = NumbersOfElementNodeHQ(index);
            if(ElGetElementGroupNumber(index)==MatIndex)
            {
               Size = (int)Nodes.size();
               element_nodes = ElGetElementNodes(index); 
               for(i=0; i<nn; i++)
               {
                   exist = false;
                   for(j=0; j<Size; j++)
                   {
                      if(element_nodes[i]==Nodes[j])
                      {
                         exist = true;
                         break;
                      }
                   }
                   if(!exist) Nodes.push_back(element_nodes[i]);                  
               }
            }
         }
      }
   }
}
/**************************************************************************
GeoLib-Method: GEOGetVolume
Task: Get element nodes in a material domain
Programing:
10/2004 WW Implementation
**************************************************************************/
using Mesh_Group::CElem;
void GEOGetNodesInMaterialDomain(CFEMesh* m_msh, const int MatIndex, vector<long>& Nodes, bool Order)
{
  int i, nn;
  long e, j, Size;
  CElem* elem = NULL;
  bool exist;
  nn =0;
  Size = 0;
  Nodes.resize(0);
  for (e = 0; e < (long)m_msh->ele_vector.size(); e++)
  {
    elem = m_msh->ele_vector[e];
    if (elem->GetMark()) // Marked for use
    {
       nn = elem->GetNodesNumber(Order); 
	   if(elem->GetPatchIndex()==MatIndex)
	   {
           Size = (int)Nodes.size();
           for(i=0; i<nn; i++)
           {
              exist = false;
              for(j=0; j<Size; j++)
              {
				 if(elem->GetNodeIndex(i)==Nodes[j])
                 {
                    exist = true;
                    break;
                 }
              }
              if(!exist) Nodes.push_back(elem->GetNodeIndex(i));                  
           }	   
	   } 
    }  // if
  } //For
}
/**************************************************************************
GeoLib-Method:
Task:
Programing:
01/2004 OK Implementation based on algorithm originally by CT
09/2005 CC Move from geolib to mshlib
**************************************************************************/
void SetRFIPointsClose(CGLLine* m_lin)
{
  long j;
  double pt1[3],pt2[3],line1[3],line2[3];
  double mult_eps = 1.0;
  double dist1p,dist2p,length;
  long anz_relevant;
  double *dist;
  typedef struct {
     long knoten;
     long abschnitt;
     double laenge;
  } INFO;
  INFO *relevant=NULL;
  long knoten_help;
  double laenge_help;
  double w1,w2;
  int weiter;
  //----------------------------------------------------------------------
  // Tests
  if(!ELEListExists()) {
    return;
  }
  if(m_lin->point1<0)
    return;
  if(m_lin->point2<0)
    return;
  //----------------------------------------------------------------------
  // Initializations
  anz_relevant = 0;
  m_lin->msh_nodes = NULL;
  CGLPoint *m_point=NULL;
  m_point = GEOGetPointById(m_lin->point1);//CC
  line1[0] = m_point->x;
  line1[1] = m_point->y;
  line1[2] = m_point->z;
  m_point = GEOGetPointById(m_lin->point2);//CC
  line2[0] = m_point->x;
  line2[1] = m_point->y;
  line2[2] = m_point->z;
  length = MCalcDistancePointToPoint(line2,line1);
  //----------------------------------------------------------------------
  // Repeat untill at least one node is found
  while(anz_relevant==0) {
    // NOD list loop
    for (j=0;j<NodeListSize();j++) {
      if (GetNode(j)==NULL) continue;
      pt1[0] = GetNodeX(j);
      pt1[1] = GetNodeY(j);
      pt1[2] = GetNodeZ(j);
      // Is MSH point near to line
      if ( MCalcDistancePointToLine(pt1,line1,line2) <= mult_eps*m_lin->epsilon ) {
        // Calc projection of pt1 to line and use this in the following
        MCalcProjectionOfPointOnLine(pt1,line1,line2,pt1);
        // Abstand des Punktes zum ersten Punkt des Polygonabschnitts
        dist1p = MCalcDistancePointToPoint(line1, pt1);
        // Abstand des Punktes zum zweiten Punkt des Polygonabschnitts
        dist2p = MCalcDistancePointToPoint(line2, pt1);
        // Ist der Knoten innerhalb des Intervalls?
        if ((dist1p+dist2p-length) <=  mult_eps*m_lin->epsilon ) {
          anz_relevant++;
          // Feld anpassen
          m_lin->msh_nodes = (long *) Realloc(m_lin->msh_nodes,sizeof(long)*anz_relevant);
          relevant = (INFO *) Realloc(relevant, sizeof(INFO)*anz_relevant);
          // Ablegen von Knotennummer und Position
          m_lin->msh_nodes[anz_relevant-1] = j;
        }
      } // endif
    } // Ende Schleife ueber Knoten
    if(anz_relevant==0) mult_eps *=2.;
  } // Ende Schleife Wiederholungen
  if (mult_eps > 1.)
    printf("Warning: Epsilon increased in CGLLine::SetRFIPointsClose.");
  m_lin->no_msh_nodes = anz_relevant;
  //----------------------------------------------------------------------
  // Sort MSH nodes, beginning from first line point
  //......................................................................
  // Calc distances from first line point
  m_point = GEOGetPointById(m_lin->point1);//CC
  pt1[0] = m_point->x;
  pt1[1] = m_point->y;
  pt1[2] = m_point->z;
  dist = (double*) Malloc(sizeof(double)*m_lin->no_msh_nodes);
  for(j=0;j<m_lin->no_msh_nodes;j++) {
    pt2[0] = GetNodeX(m_lin->msh_nodes[j]);  
    pt2[1] = GetNodeY(m_lin->msh_nodes[j]);  
    pt2[2] = GetNodeZ(m_lin->msh_nodes[j]);  
    dist[j] = MCalcDistancePointToPoint(pt1,pt2);
  }
  //......................................................................
  // Sorting by pair switching
  do {
    weiter = 0;
    for (j=0;j<m_lin->no_msh_nodes-1;j++) {
       w1=dist[j];
       w2=dist[j+1];
       if (w1>w2) { /* Die Eintraege vertauschen */
          knoten_help = m_lin->msh_nodes[j];
          laenge_help = dist[j];
          m_lin->msh_nodes[j] = m_lin->msh_nodes[j+1];
          dist[j] = dist[j+1];
          m_lin->msh_nodes[j+1] = knoten_help;
          dist[j+1] = laenge_help;
          weiter=1;
         }
      }
  } while (weiter);
  //----------------------------------------------------------------------
  // Destructions
  dist = (double*) Free(dist);
  relevant = (INFO*) Free(relevant);
}
/**************************************************************************
GeoLib-Method: CreateMSHLines
Task:
Programing:
01/2004 OK
**************************************************************************/
void CreateMSHLines(CGLLine* m_lin)
{
  long j;
  long *nodes = NULL;
  for (j=0;j<m_lin->no_msh_nodes-1;j++) {
    nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[0]);
    nodes[0] = m_lin->msh_nodes[j];
    nodes[1] = m_lin->msh_nodes[j+1];
    if((nodes[0]>-1)&&(nodes[1]>-1)) {
//OK      ELECreateTopology(1,-1,0,ElementListLength);
      ElSetElementNodes(ElementListLength-1,nodes);
      ElSetElementGroupNumber(ElementListLength-1,m_lin->mat_group);
      anz_1D++;
    }
    else {
      nodes = (long *)Free(nodes);
      //AfxMessageBox("OnMshLibLineGen: Grid nodes not found");
      printf("OnMshLibLineGen: Grid nodes not found");
    }
  }
}
/**************************************************************************
GeoSys-GUI Function
Programing:
01/2004 OK Implementation
09/2005 CC Modification No MFC function
**************************************************************************/
bool IsPointInSurface(Surface* m_fsc, CGLPoint *m_point)
{
  bool ok = false;
  m_point = m_point;
  if(!m_fsc)
    return false; //OK
 #ifdef MFC
  long i;
  int j;
  CPoint this_point;
  CGLPoint *m_sfc_point;

  POINT m_arrPoint[1024] ;
  long number_of_surface_polygon_points = (long)m_fsc->polygon_point_vector.size();
  long number_of_TIN_elements = 0;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    m_point->x_pix = theApp->g_graphics_modeless_dlg->xpixel(m_point->x);
    m_point->y_pix = theApp->g_graphics_modeless_dlg->ypixel(m_point->y);
  }
  if(m_fsc->TIN)
    number_of_TIN_elements = (long)m_fsc->TIN->Triangles.size();
  //-----------------------------------------------------------------------
  // surface polygon
  if(number_of_surface_polygon_points>0) {
    for(i=0;i<number_of_surface_polygon_points;i++) 
    {
      m_sfc_point = m_fsc->polygon_point_vector[i];
      if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
      {
        m_arrPoint[i].x = theApp->g_graphics_modeless_dlg->xpixel(m_sfc_point->x);//CC
        m_arrPoint[i].y = theApp->g_graphics_modeless_dlg->ypixel(m_sfc_point->y);//CC
      }
    }
    CRgn surface_polygon;
    surface_polygon.CreatePolygonRgn(&m_arrPoint[0],(int)number_of_surface_polygon_points,WINDING);
    this_point.x = m_point->x_pix;
    this_point.y = m_point->y_pix;
    if (surface_polygon.PtInRegion(this_point)) {
      ok = true;
    }
    DeleteObject(surface_polygon);
  }
  //-----------------------------------------------------------------------
  // TIN
  else if(number_of_TIN_elements>0) {
    for(i=0;i<number_of_TIN_elements;i++) {
     
      for(j=0;j<3;j++) {
         m_arrPoint[j].x = (long)m_fsc->TIN->Triangles[i]->x[j];
         m_arrPoint[j].y = (long)m_fsc->TIN->Triangles[i]->y[j];
      }
      CRgn surface_polygon;
      surface_polygon.CreatePolygonRgn(&m_arrPoint[0],3,WINDING);
      this_point.x = (long)m_point->x;
      this_point.y = (long)m_point->y;
      if (surface_polygon.PtInRegion(this_point)) {
        ok = true;
        DeleteObject(surface_polygon);
        break;
      }
      DeleteObject(surface_polygon);
    }
  }
  //-----------------------------------------------------------------------
  DeleteObject(m_arrPoint);
#endif
  return ok;
}
/**************************************************************************
GeoLib-Method:
Task: 
Programing:
03/2004 OK Implementation
01/2005 OK 2nd version
08/2005 CC Modification
**************************************************************************/
void MSHCreateLayerPolylines(CGLPolyline* m_ply)
{
    long *nodes = NULL;
  long no_nodes;
  long i,j;
  double dist;
  double x,y,x1,y1;
  CGLPoint *m_point = NULL;
CGLPolyline* m_polyline = NULL;
string ply_path;
 CGSProject* m_gsp = GSPGetMember("gli");//CC
 if(m_gsp)
string ply_path = m_gsp->path;
  //---------------------------------------------------------------------
  //
  nodes = MSHGetNodesCloseXY(&no_nodes,m_ply); //OK41
  //---------------------------------------------------------------------
  // Determine number of layers
  int no_layers = 0;
  x1 = GetNodeX(nodes[0]);
  y1 = GetNodeY(nodes[0]);
  for(j=0;j<NodeListLength;j++) {
    x = GetNodeX(j); // ToDo NodeNumber[j]
    y = GetNodeY(j);
    dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    if(dist<m_ply->epsilon) {
      no_layers++;
    }
  }
  long nodes_per_layer = (long)(NodeListSize()/no_layers);
  long no_polyline_nodes = (long)(no_nodes/no_layers);
  //---------------------------------------------------------------------
  // Create layer polylines
  char layer_number[10];

    m_polyline = new CGLPolyline;
    sprintf(layer_number,"%ld",0L);
    m_polyline->name = m_ply->name + "_L" + layer_number;
    m_polyline->type = 22;
    m_polyline->data_type = 1;
    for(i=0;i<no_polyline_nodes;i++) {
      m_point = new CGLPoint;
      m_point->x = GetNodeX(nodes[i]); // ToDo NodeNumber[j]
      m_point->y = GetNodeY(nodes[i]); // ToDo NodeNumber[j]
      m_point->z = GetNodeZ(nodes[i]); // ToDo NodeNumber[j]
      m_polyline->point_vector.push_back(m_point);
    }
    m_polyline->SortPointVectorByDistance();
    polyline_vector.push_back(m_polyline); //CC
    m_polyline->WritePointVector(m_ply->name);//CC
    m_polyline->WriteTecplot(ply_path); //OK41 CC change

  for(j=1;j<no_layers;j++) {
    m_polyline = new CGLPolyline;
    sprintf(layer_number,"%ld",j);
    m_polyline->name = m_ply->name + "_L" + layer_number;
    m_polyline->type = 22;
    m_polyline->data_type = 1;
    for(i=0;i<no_polyline_nodes;i++) {
      m_point = new CGLPoint;
      m_point->x = GetNodeX(nodes[i]+j*nodes_per_layer); // ToDo NodeNumber[j]
      m_point->y = GetNodeY(nodes[i]+j*nodes_per_layer); // ToDo NodeNumber[j]
      m_point->z = GetNodeZ(nodes[i]+j*nodes_per_layer); // ToDo NodeNumber[j]
      m_polyline->point_vector.push_back(m_point);
    }
    m_polyline->SortPointVectorByDistance();
    polyline_vector.push_back(m_polyline); //CC
    m_polyline->WritePointVector(m_ply->name);
    m_polyline->WriteTecplot(ply_path); //CC8888
  }
}
#ifdef iduwhed
/**************************************************************************
GeoLib-Method:
Task: 
Programing:
03/2004 OK Implementation
08/2005 CC Modification
**************************************************************************/
void MSHCreateLayerPolylines(CGLPolyline* m_ply)
{
  long *nodes = NULL;
  long no_nodes;
  long **matrix = NULL;
  int row=0;
  long i,j;
  double dist;
  double x,y,x1,y1;
  CGLPoint *m_point = NULL;
CGSProject* m_gsp = GSPGetMember("gli");//CC
 if(m_gsp)
string ply_path = m_gsp->path;
  //---------------------------------------------------------------------
  // Node columns
//OK41
  nodes = MSHGetNodesCloseXYSorted(&no_nodes,m_ply); 
//------------------------------------------
  // Determine number of layers
  int no_layers = 0;
  x1 = GetNodeX(nodes[0]);
  y1 = GetNodeY(nodes[0]);
  for(j=0;j<NodeListLength;j++) {
    x = GetNodeX(j); // ToDo NodeNumber[j]
    y = GetNodeY(j);
    dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    if(dist<m_ply->epsilon) {
      no_layers++;
    }
  }
  CString m_strMessage;
  m_strMessage.Format("Number of layers: %i",no_layers);
  AfxMessageBox(m_strMessage);
//OK41
  ///matrix = new long*[no_nodes];
  long nodes_per_layer = (long)(no_nodes/no_layers);
  matrix = (long**) Malloc(nodes_per_layer*sizeof(long));
  for(i=0;i<nodes_per_layer;i++) {
    x1 = GetNodeX(nodes[i]);
    y1 = GetNodeY(nodes[i]);
    row=0;
    matrix[i] = NULL;
    for(j=0;j<NodeListLength;j++) {
      x = GetNodeX(j); // ToDo NodeNumber[j]
      y = GetNodeY(j);
      dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
      if(dist<m_ply->epsilon) {
        row++;
        matrix[i] = (long*) Realloc(matrix[i],row*sizeof(long));
        matrix[i][row-1] = j; // ToDo NodeNumber[j];
      }
    }
  }
  //---------------------------------------------------------------------
  // Create layer polylines
  char layer_number[10];
  for(i=0;i<no_layers;i++) {
    m_polyline = new CGLPolyline;
    sprintf(layer_number,"%ld",j);
    m_polyline->name = name + "_L" + layer_number;
    m_polyline->type = 22;
    m_polyline->data_type = 1;
    for(j=0;j<nodes_per_layer;j++) {
      m_point = new CGLPoint;
      m_point->x = GetNodeX(matrix[j][i]); // ToDo NodeNumber[j]
      m_point->y = GetNodeY(matrix[j][i]); // ToDo NodeNumber[j]
      m_point->z = GetNodeZ(matrix[j][i]); // ToDo NodeNumber[j]
      m_polyline->point_vector.push_back(m_point);
    }
    polyline_vector.push_back(m_polyline); //CC
    m_polyline->WritePointVector(name);//CC
 }
  //-----------------------------------------------------------------------
  // Memory
  for(i=0;i<nodes_per_layer;i++) {
    matrix[i] = (long*) Free(matrix[i]);
  }
  matrix = (long**) Free(matrix);
}
#endif

/**************************************************************************
GeoLibGUI-Method: 
Task: 
Programing:
OK first version
07/2005 CC Implementation mark the selected point
08/2005 CC Modification
**************************************************************************/
long* MSHGetNodesCloseXY(long *number_of_nodes, CGLPolyline* m_ply)
{
  long j,k,l;
  double pt1[3],line1[3],line2[3],pt0[3];
  double mult_eps = 1.0;
  double dist1p,dist2p,*length,laenge;
  long *nodes_all = NULL;
  long anz_relevant = 0;
  typedef struct {
     long knoten;
     long abschnitt;
     double laenge;
  } INFO;
  INFO *relevant=NULL;
  int weiter;
  double w1,w2;
  long knoten_help;
  double laenge_help;
  double gesamte_laenge = 0.;
  long polyline_point_vector_size;

if (m_ply&&(m_ply->point_vector.size()>0)) {

  length = (double*) Malloc(sizeof(double) *(long)m_ply->point_vector.size());

  pt0[0] = m_ply->point_vector[0]->x;
  pt0[1] = m_ply->point_vector[0]->y;
  pt0[2] = 0.0;

  /* */
  polyline_point_vector_size =(long)m_ply->point_vector.size();
  for (k=0;k<polyline_point_vector_size-1;k++) {
    line1[0] = m_ply->point_vector[k]->x;
    line1[1] = m_ply->point_vector[k]->y;
    line1[2] = 0.0;
    line2[0] = m_ply->point_vector[k+1]->x;
    line2[1] = m_ply->point_vector[k+1]->y;
    line2[2] = 0.0;
    length[k] = MCalcDistancePointToPoint(line2, line1);
    gesamte_laenge += length[k];
  }

  /* Wiederholen bis zumindest ein Knoten gefunden wurde */
 while(anz_relevant==0) {


//----------------------------------//CC

  for (j=0;j<NodeListSize();j++) {
    if (GetNode(j)==NULL) continue;

    /* Schleife ueber alle Punkte des Polygonzuges */
	polyline_point_vector_size =(long)m_ply->point_vector.size();
    for (k=0;k<polyline_point_vector_size-1;k++) {

      /* ??? */
      pt1[0] = GetNodeX(j);
      pt1[1] = GetNodeY(j);
      pt1[2] = 0.0;

      line1[0] = m_ply->point_vector[k]->x;
      line1[1] = m_ply->point_vector[k]->y;
      line1[2] = 0.0;
      line2[0] = m_ply->point_vector[k+1]->x;
      line2[1] = m_ply->point_vector[k+1]->y;
      line2[2] = 0.0;

      /* Ist der Knoten nah am Polygonabschnitt? */
      if ( MCalcDistancePointToLine(pt1,line1,line2) <= mult_eps*m_ply->epsilon ) {
        /* Im folgenden wird mit der Projektion weitergearbeitet */
        MCalcProjectionOfPointOnLine(pt1,line1,line2,pt1);
        /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
        dist1p = MCalcDistancePointToPoint(line1, pt1);
        /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
        dist2p = MCalcDistancePointToPoint(line2, pt1);
        /* Ist der Knoten innerhalb des Intervalls? */
        /* bis rf3807: if ((length[k] - dist1p - dist2p + MKleinsteZahl)/(length[k] + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl){ */
        if ((dist1p+dist2p-length[k]) <=  mult_eps*m_ply->epsilon ) {
          anz_relevant++;
          /* Feld anpassen */
          nodes_all = (long *) Realloc(nodes_all,sizeof(long)*anz_relevant);
          relevant = (INFO *) Realloc(relevant, sizeof(INFO) * anz_relevant);
          /* Ablegen von Knotennummer und Position */
          nodes_all[anz_relevant-1] = j;
          /* Position ermitteln */
          laenge = 0.;
          for (l=0; l < k; l++)
            laenge += length[l];
          /* Ablegen von Knotennummer und Position */
          relevant[anz_relevant-1].knoten = j;
          relevant[anz_relevant-1].laenge = laenge + dist1p;
          /* Suche am Polygon abbrechen, naechster Knoten */
          k =(long)m_ply->point_vector.size();
        }
      } /* endif */
    } /* Ende Schleife ueber Polygonabschnitte */
  } /* Ende Schleife ueber Knoten */
  if(anz_relevant==0) mult_eps *=2.;
 } /* Ende Schleife Wiederholungen */

  if (mult_eps > 1.)
     cout << "!!! Epsilon increased in sources!" << endl;

  /* Schleife ueber alle Knoten; sortieren nach Reihenfolge auf dem Abschnitt (zyklisches Vertauschen, sehr lahm)*/
  do {
    weiter = 0;
    for (k=0;k<anz_relevant-1;k++) {
       w1=relevant[k].laenge;
       w2=relevant[k+1].laenge;
       if (w1>w2) { /* Die Eintraege vertauschen */
          knoten_help = relevant[k].knoten;
          laenge_help = relevant[k].laenge;
          relevant[k].knoten = relevant[k+1].knoten;
          relevant[k].laenge = relevant[k+1].laenge;
          relevant[k+1].knoten = knoten_help;
          relevant[k+1].laenge = laenge_help;
          weiter=1;
         }
      }
  } while (weiter);
  relevant = (INFO*) Free(relevant);
  *number_of_nodes = anz_relevant;

  } /* if (polyline) */

  return nodes_all;
}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
01/2005 OK Implementation
08/2005 CC
**************************************************************************/
long* MSHGetNodesCloseXYSorted(long* number_of_nodes, CGLPolyline* m_ply)
{
  long i;
  double pt1[3],pt2[3];
  long *nodes_unsorted = NULL;
  long* nodes_sorted = NULL;
  //long number_of_nodes;
  //...............................................................
  // Auxillary node_value vector
  nodes_unsorted = MSHGetNodesCloseXY(number_of_nodes,m_ply);
  // Sort by distance
  pt1[0] = m_ply->point_vector[0]->x;
  pt1[1] = m_ply->point_vector[0]->y;
  pt1[2] = m_ply->point_vector[0]->z;
  double *node_distances = new double[*number_of_nodes];
  for(i=0;i<*number_of_nodes;i++) {
    pt2[0] = GetNodeX(nodes_unsorted[i]);
    pt2[1] = GetNodeY(nodes_unsorted[i]);
    pt2[2] = GetNodeZ(nodes_unsorted[i]);
    node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
  }
  nodes_sorted = TOLSortNodes1(nodes_unsorted,node_distances,*number_of_nodes);
  // Release memory
  delete [] node_distances;
  return nodes_sorted;
//  delete [] nodes_unsorted;
}
