/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#include "stdafx.h" // MFC
// C++
#include <cmath>
// MSHLib
#include "msh_edge.h"
//========================================================================
namespace Mesh_Group
{
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
CEdge::CEdge(const int Index, bool quadr)
   :CCore(Index)
{
   quadratic = quadr;
   index = Index;
   // Assume that each edge has three nodes
   nodes_of_edges.resize(3);
   for(int i=0; i<3; i++)
     nodes_of_edges[i] = NULL;	 
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
CEdge::~CEdge()
{
   nodes_of_edges.resize(0);
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CEdge::operator = (CEdge& ed)
{
   boundary_type = ed.boundary_type; 
   index = ed.index;
   mark = ed.mark;
   for(int i=0; i<nodes_of_edges.Size(); i++)
      nodes_of_edges[i] = ed.nodes_of_edges[i]; 
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
double CEdge::Length()
{
    double dx, dy, dz;
    dx = nodes_of_edges[1]->X()-nodes_of_edges[0]->X();
    dy = nodes_of_edges[1]->Y()-nodes_of_edges[0]->Y();
    dz = nodes_of_edges[1]->Z()-nodes_of_edges[0]->Z();
    return sqrt(dx*dx+dy*dy+dz*dz);
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
bool CEdge::operator == (CEdge& ed)
{
   int identical;
 
   // Compare two ends
   identical=0;
   for(int i=0; i<2; i++)
   {
      if(nodes_of_edges[i] == ed.nodes_of_edges[i])
        identical++;
   }
   if(identical==2)
	   return true;

   identical=0;
   for(int i=0; i<2; i++)
   {
      if(nodes_of_edges[1-i] == ed.nodes_of_edges[i])
        identical++;
   }
   if(identical==2)
	   return true;

   return false;
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CEdge::Write(ostream& osm) const
{
    osm<<"Edge: "<< index<<endl;
   	for(int i=0; i<nodes_of_edges.Size(); i++)
   	{
       osm<<"Node: "<< i<<endl;
       nodes_of_edges[i]->Write(osm);        
    }
   	osm<<endl;
}

} // namespace Mesh_Group
//========================================================================
