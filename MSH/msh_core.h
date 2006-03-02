 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_core_INC
#define msh_core_INC
// C++
#include <string>
#include<iostream>
using namespace std;
//------------------------------------------------------------------------
namespace FiniteElement
{ 
  class CElement;  class CFiniteElementStd;  class CFiniteElementVec; 
  class ElementMatrix; class ElementMatrix_DM;
}
namespace Mesh_Group
{
 // Finite element object
 using FiniteElement::CElement;
 using FiniteElement::CFiniteElementStd;
 using FiniteElement::CFiniteElementVec;
 using FiniteElement::ElementMatrix;
 using FiniteElement::ElementMatrix_DM;

//------------------------------------------------------------------------
class CCore
{
   protected: // Properties
      long index;
      char boundary_type;
      bool mark; // e.g. marked to be refined or active 
      bool quadratic; // higher order
      string deli; // delimitor
      // Finite element objects
	  friend class FiniteElement::CElement;  
	  friend class FiniteElement::CFiniteElementStd; 
	  friend class FiniteElement::CFiniteElementVec; 
      friend class FiniteElement::ElementMatrix; 
	  friend class FiniteElement::ElementMatrix_DM;
   public: // Methods
      CCore(const int id);
      virtual ~CCore() {}
      // Operator
      virtual void operator = (CCore & g) {g=g;}
      virtual bool operator == (CCore & g) {g=g; return false;}
      // Get members
      long GetIndex() const {return index;} 
      bool GetMark() const {return mark;}
      bool Dirichlet() const  { if(boundary_type=='D') return true; else return false;}
      bool Neumann()   const  { if(boundary_type=='N') return true; else return false;}
      bool Cauchy ()   const  {  if(boundary_type=='C') return true; else return false;}
      bool onBoundary() const {  if(boundary_type=='B') return true; else return false; }
      bool Interior() const {  if(boundary_type=='I') return true; else return false; }
      // Set members
      void SetBoundaryType(const char type) {boundary_type = type;}
      void SetOrder(const bool order) {quadratic = order;}
      void SetMark(const bool state) {mark = state;}
      void SetIndex(const long lvalue){index=lvalue;} //OK
      // Output
      virtual void Write(ostream& os=cout) const {os<<endl;};
};

} // namespace Mesh_Group

#endif
