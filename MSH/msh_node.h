 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_node_INC
#define msh_node_INC
// C++
#include <string>
#include <vector>
#include "matrix_class.h"
// MSHLib
#include "msh_core.h"
//------------------------------------------------------------------------
namespace Mesh_Group
{
 using Math_Group::SymMatrix;
 using Math_Group::vec;
 using Math_Group::Vec;

//------------------------------------------------------------------------
// Class definition
class CNode:public CCore
{
   private: // Members
      long eqs_index; // renumber
      double coordinate[3];
      friend class CEdge;
      friend class CElem;
      // FEM
      friend class FiniteElement::CElement;  
      friend class FiniteElement::CFiniteElementStd; 
      friend class FiniteElement::CFiniteElementVec; 
      friend class FiniteElement::ElementMatrix; 
      friend class FiniteElement::ElementMatrix_DM;
   public:
      double epsilon;
      long interior_test;
      int crossroad;	// PCH: Make theses privates can be done later on.
      int free_surface; //MB ??? mobile
      vector<long> connected_elements;
      vector<long> connected_nodes; //OK
      // The vector to store the representive element index.
      // This can be used to extract the norm of the plane that the element lies on.
      // Establishing this vector is done in the Fluid Momentum
      // since this is bounded by velocity.
      vector<long> connected_planes;	// PCH 
      vector<long>  m5_index; //WW
      CNode(const int Index):CCore(Index), eqs_index(-1) {}
      CNode(const int Index, const double x, const double y, const double z=0.0);
      ~CNode() {}
      // Operator
      void operator = (const CNode& n);
      bool operator == (const CNode & n);
      // Get functions
      double X() const {return coordinate[0];}
      double Y() const {return coordinate[1];}
      double Z() const {return coordinate[2];}
      void Coordinates(double *xyz) const
      {  
        for(int i=0; i<3; i++)  xyz[i] = coordinate[i];
      } 
	  int  GetEquationIndex() const { return eqs_index;}
      // Set functions
      void SetX(const double argX)  { coordinate[0] = argX;}
      void SetY(const double argY)  { coordinate[1] = argY;}
      void SetZ(const double argZ)  { coordinate[2] = argZ;}
      void SetCoordinates(const double* argCoord); 
	  void SetEquationIndex(const long eqIndex) {eqs_index = eqIndex;} //
      // Output
      void Write(ostream& os=cout) const;
      //GUI control variables 
      int selected;
      double patch_area; //OK4310
};

} // namespace Mesh_Group
#endif
