 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_elem_INC
#define msh_elem_INC
// C++
#include <string>
#include<iostream>
using namespace std;
// MSHLib
#include "msh_edge.h"
// PCSLib
namespace process{class CRFProcessDeformation;}
namespace Math_Group{class Matrix;}

class CRFProcess;
//------------------------------------------------------------------------
namespace Mesh_Group
{
 // Process
 using process::CRFProcessDeformation;
 using ::CRFProcess;
 using Math_Group::Matrix;
//------------------------------------------------------------------------
// Class definition
class CElem:public CCore
{
   private: // Members
      // ID
      int geo_type; // 1 Line, 2 Quad, 3 Hex, 4 Tri, 5 Tet, 6 Pris 
      CElem* owner;
      // Geometrical properties
      int ele_dim; // Dimension of element
      int nnodes;
      int nnodesHQ;
      vec<CNode*>nodes;
      int nedges;
      vec<CEdge*>edges;
      vec<int>edges_orientation;
      int nfaces;
      int no_faces_on_surface;
      int face_index; // Local face index for the instance for face
      double volume;
      double gravity_center[3];
      int grid_adaptation;  // Flag for grid adapting.
      int patch_index;
      double representative_length;//For stability calculations
      double courant;
      double neumann;	  // MSH topology
      double area;//Flux area
  public:
      double normal_vector[3]; //OK
  private:
	  // MSH topology
      Matrix* tranform_tensor;
      vec<CElem*>neighbors;
      //vec<CElem*> sons;
	  double angle[3];	// PCH, angle[0] rotation along y axis
						//	    angle[1] rotation along x' axis
						//		angle[2] translation along z'' axis.		
	  double MatT[9];
   private: // Methods
      int GetElementFaces1D(int *FaceNode);
      int GetElementFacesTri(const int Face, int *FaceNode);
      int GetElementFacesQuad(const int Face, int *FaceNode);
      int GetElementFacesHex(const int Face, int *FaceNode);
      int GetElementFacesTet(const int Face, int *FaceNode);
      int GetElementFacesPri(const int Face, int *FaceNode);
   private: // Friends
      friend class CFEMesh;
      // FEM
      friend class FiniteElement::CElement;  
      friend class FiniteElement::CFiniteElementStd; 
      friend class FiniteElement::CFiniteElementVec; 
      friend class FiniteElement::ElementMatrix; 
      friend class FiniteElement::ElementMatrix_DM;
      // PCS
      friend class process::CRFProcessDeformation;
      friend class ::CRFProcess;
  public: // Methods
      CElem();
      CElem(const int Index);
      CElem( const int Index, CElem* onwer, const int Face); // For Faces: Face, local face index
      ~CElem();
      //------------------------------------------------------------------
      // Geometry
      int GetDimension() const {return ele_dim;}
      double* GetGravityCenter() {return gravity_center;}
      int GetPatchIndex() const {return patch_index;} //MatGroup
      void SetPatchIndex(const int value) {patch_index = value;}
      void ComputeVolume();
      void SetFluxArea(double fluxarea) {area = fluxarea;}//CMCD for <3D elements with varying area
      double GetFluxArea() {return area;}//CMCD for <3D elements with varying area
	  double GetVolume() const {return volume;}
      void SetVolume(const double Vol) {volume = Vol;}
      void SetCourant(double Cour) {courant = Cour;}//CMCD
      double GetCourant() {return courant;}//CMCD
      void SetNeumann(double Neum) {neumann = Neum;}//CMCD
      double GetNeumann() {return neumann;}//CMCD
      double GetRepLength() {return representative_length;}//CMCD
      //------------------------------------------------------------------
      // ID
      int GetElementType() const {return geo_type;}
      void SetElementType(const int Type) {geo_type=Type;}
      void MarkingAll(bool makop); 
      string GetName() const;
      //------------------------------------------------------------------
      // Nodes
      vec<long>nodes_index;      
      void GetNodeIndeces(vec<long>&  node_index) const 
	    {for (int i=0; i< (int) nodes_index.Size();i++)
           node_index[i]= nodes_index[i];} 
      long GetNodeIndex(const int index) const  {return  nodes_index[index];} 
      void SetNodeIndex(const int index, const long g_index) {nodes_index[index]= g_index;} 
      void GetNodes(vec<CNode*>&ele_nodes)
        {for (int i=0; i< (int) nodes.Size();i++) ele_nodes[i]= nodes[i];}
      CNode* GetNode(const int index) { return nodes[index]; }
      void SetNodes(vec<CNode*>&  ele_nodes, const bool ReSize=false);
	  int GetNodesNumber_H() const  {return nnodesHQ;}
	  int GetNodesNumber(bool quad) const  {if(quad) return nnodesHQ; else return nnodes;}
	  int GetVertexNumber() const  {return nnodes;}
      void SetNodesNumber(int ivalue) {nnodes = ivalue;} //OK
      CElem* GetOwner() { return owner; }  //YD
      //------------------------------------------------------------------
      // Edges
      void GetEdges(vec<CEdge*>&  ele_edges) 
        {for (int i=0; i<nedges; i++) ele_edges[i]= edges[i];} 
      void SetEdges(vec<CEdge*>&  ele_edges) 
        {for (int i=0; i<nedges; i++) edges[i]= ele_edges[i];} 
      int FindFaceEdges(const int LocalFaceIndex, vec<CEdge*>&  face_edges);
      void SetEdgesOrientation(vec<int>&  ori_edg) 
         {for (int i=0; i<nedges; i++) edges_orientation[i]= ori_edg[i];} 
      void GetLocalIndicesOfEdgeNodes(const int Edge, int *EdgeNodes);
      int GetEdgesNumber() const{return nedges;}
      //------------------------------------------------------------------
      // Faces
      int GetFacesNumber() const {return nfaces;}
      void SetFace();
      void SetFace(CElem* onwer, const int Face);
	  int GetSurfaceFacesNumber() const {return no_faces_on_surface;} 
	  int GetLocalFaceIndex() const {return face_index;} 
      int GetFaceType();
      int GetElementFaceNodes(const int Face, int *FacesNode);
      //------------------------------------------------------------------
      // Neighbors
      void SetNeighbors(vec<CElem*>&  ele_neighbors) 
         { for (int i=0; i< nfaces;i++) neighbors[i] = ele_neighbors[i];}
      void SetNeighbor(const int LocalIndex, CElem* ele_neighbor) 
         { neighbors[LocalIndex] = ele_neighbor;}
      void GetNeighbors(vec<CElem*>&  ele_neighbors)  
         {for (int i=0; i< nfaces;i++) ele_neighbors[i]= neighbors[i];} 
      CElem* GetNeighbor(const int index) { return neighbors[index];} 

      //------------------------------------------------------------------
      // Coordinates transform
      void FillTransformMatrix();  
	  void FillTransformMatrix(int noneed);
	  double getTransformTensor(const int idx);
	  double GetAngle(const int i) const { return angle[i]; }	// PCH
	  void SetAngle(const int i, const double value) 
	  { 
		angle[i] = value; 
	  }	// PCH
      //------------------------------------------------------------------
      // I/O
      void Read(istream& is=cin, int fileType=0);
      void WriteIndex(ostream& os=cout) const;
      void WriteIndex_TEC(ostream& os=cout) const;
      void WriteAll(ostream& os=cout) const;
      void WriteNeighbors(ostream& os=cout) const;
      void Config(); //OK
      //------------------------------------------------------------------
      // MAT
      Vec mat_vector;  //OKWW
      int matgroup_view;//TK
      //------------------------------------------------------------------
      // Operator
      // virtual void operator = (const CElem& elem);
     //-------------------------------------------------------------------
     // DDC
      //------------------------------------------------------------------
      void FaceNormal(const int index0, const int index1, double*);   //YD
//     int domain; //OK
     long domain_nodes [8]; //OK, dynamisch
   public: //GUI control variables 
   int selected;
};

} // namespace Mesh_Group
#endif
