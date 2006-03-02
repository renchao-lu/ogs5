/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C++
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_elements_rfi.h"
// PCSLib

/**************************************************************************
GeoLib-Method:
Task:
Programing:
04/2005 OK Implementation based on algorithm originally by CT
**************************************************************************/
void CFEMesh::SetLINPointsClose(CGLLine*m_lin)
{
  m_lin = m_lin;
//WWW
#ifdef TODO 
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
  CMSHNodes* m_nod = NULL;
  //----------------------------------------------------------------------
  // Tests
  if(m_lin->point1<0)
    return;
  if(m_lin->point2<0)
    return;
  //----------------------------------------------------------------------
  // Initializations
  anz_relevant = 0;
  m_lin->msh_nodes = NULL;
  CGLPoint *m_point=NULL;
  m_point = m_point->Get(m_lin->point1);
  line1[0] = m_point->x;
  line1[1] = m_point->y;
  line1[2] = m_point->z;
  m_point = m_point->Get(m_lin->point2);
  line2[0] = m_point->x;
  line2[1] = m_point->y;
  line2[2] = m_point->z;
  length = MCalcDistancePointToPoint(line2,line1);
  //----------------------------------------------------------------------
  // Repeat untill at least one node is found
  while(anz_relevant==0) {
    // NOD list loop
    for (j=0;j<(long)nod_vector.size();j++) {
      m_nod = nod_vector[j];
      pt1[0] = m_nod->x;
      pt1[1] = m_nod->y;
      pt1[2] = m_nod->z;
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
  m_point = m_point->Get(m_lin->point1);
  pt1[0] = m_point->x;
  pt1[1] = m_point->y;
  pt1[2] = m_point->z;
  dist = (double*) Malloc(sizeof(double)*m_lin->no_msh_nodes);
  for(j=0;j<m_lin->no_msh_nodes;j++) {
    m_nod = nod_vector[m_lin->msh_nodes[j]];
    pt2[0] = m_nod->x; 
    pt2[1] = m_nod->y; 
    pt2[2] = m_nod->z; 
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

///WWW
#endif 
}
