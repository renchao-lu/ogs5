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
   public:
	  vector<long>connected_elements;	// PCH
   private: // Members
      vec<CNode*>  nodes_of_edges;
	  int joint;	// PCH
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
	  int GetJoint() const { return joint;}	// PCH
      // Set functions
      void SetNodes( vec<CNode*>& Nodes)
        { for(int i=0; i<3; i++)  nodes_of_edges[i] = Nodes[i]; }
	  void SetJoint(const int i) { joint = i; }	// PCH
      // Operator
      void operator = (CEdge& edg);
      bool operator == (CEdge& edg);
      // Output
      void Write(ostream& osm=cout) const;
      //
      void SetNormalVector(double *ele_normal_vector, double *normal_vector); //OK
      void GetEdgeVector(double *edge_vector); //OK
      void GetEdgeMidPoint(double *edge_vector); //OK
};

} // namespace Mesh_Group
#endif
