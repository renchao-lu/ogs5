 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_edge_INC
#define msh_edge_INC
// MSHLib
#include "msh_node.h"
//------------------------------------------------------------------------
namespace Mesh_Group
{
//------------------------------------------------------------------------
// Class definition
class CEdge:public CCore
{
   private: // Members
      vec<CNode*>  nodes_of_edges;
      friend class CElem;
	  friend class FiniteElement::CElement;  
	  friend class FiniteElement::CFiniteElementStd; 
	  friend class FiniteElement::CFiniteElementVec; 
      friend class FiniteElement::ElementMatrix; 
	  friend class FiniteElement::ElementMatrix_DM;
   public: // Methods
      CEdge(const int Index, bool quadr=false);
      ~CEdge(); 
      // Get functions
      void GetNodes( vec<CNode*>& Nodes) 
        { for(int i=0; i<3; i++)  Nodes[i] = nodes_of_edges[i]; }			
      CNode* GetNode(const int l_index) {return nodes_of_edges[l_index];} 
      double Length();
      // Set functions
      void SetNodes( vec<CNode*>& Nodes)
        { for(int i=0; i<3; i++)  nodes_of_edges[i] = Nodes[i]; }
      // Operator
      void operator = (CEdge& edg);
      bool operator == (CEdge& edg);
      // Output
      void Write(ostream& osm=cout) const;
};

} // namespace Mesh_Group
#endif
