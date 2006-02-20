 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_mesh_INC
#define msh_mesh_INC
// C++
#include <string>
// GEOLib
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_vol.h"
// MSHLib
#include "msh_elem.h"
#ifdef RANDOM_WALK
class RandomWalk;
#endif
class CFluidMomentum;
//------------------------------------------------------------------------
namespace Mesh_Group
{
//------------------------------------------------------------------------
// Class definition
class CFEMesh
{
 private:

    // Finite element objects
    friend class FiniteElement::CElement;  
    friend class FiniteElement::CFiniteElementStd; 
    friend class FiniteElement::CFiniteElementVec; 
    friend class FiniteElement::ElementMatrix; 
    friend class FiniteElement::ElementMatrix_DM;     
    // Process friends
    friend class process::CRFProcessDeformation;
    friend class ::CRFProcess;

    long NodesNumber_Linear;
    long NodesNumber_Quadratic;
	int max_ele_dim; // For overland flow and channel flow only
    bool useQuadratic;
    bool axisymmetry;
    // Coordinate indicator
	// 1:  X component only
	// 12: Y component only
	// 13: Z component only
	// 2:  X, Y component
	// 23:  X, Z component
	// 3:  X, Y, Z component
    int coordinate_system;
 public:	 
    string pcs_name;
    string geo_name; //MB
    int ele_type;
    bool selected;
    int no_msh_layer; //OK
    int nr,ns; //OK
    double z_min,z_max; //OK
    int max_mmp_groups; //OKCC
//    int mat_group; //OK
    double min_edge_length; //TK
    double max_edge_length; //TK
    long no_line;
    long no_quad;
    long no_hexs;
    long no_tris;
    long no_tets;
    long no_pris;
  //3D View Control
  public:
    int ele_display_mode;
    int nod_display_mode;
    int ele_mat_display_mode;
    int highest_mat_group_nb;

#ifdef RANDOM_WALK
    RandomWalk* PT; // PCH
#endif
    CFluidMomentum* fm_pcs; // by PCH
    CFEMesh(void);
    ~CFEMesh(void);
    ios::pos_type Read(ifstream*);
    ios::pos_type ReadBIN(ifstream*); //OK
    void Write(fstream*);
    void WriteBIN(fstream*,fstream*); //OK
    ios::pos_type GMSReadTIN(ifstream*);

    void ConstructGrid( const bool quadratic=false);
    void RenumberNodesForGlobalAssembly();
    // For number of nodes
	int GetMaxElementDim() {return max_ele_dim;}
    void SwitchOnQuadraticNodes(bool quad) { useQuadratic = quad;}
    bool getOrder() const {return useQuadratic;}
    bool isAxisymmetry() const {return axisymmetry;}
    // Get number of nodes
    long GetNodesNumber(const bool quadr)//CMCD int to long 
      {if(quadr) return NodesNumber_Quadratic; else return  NodesNumber_Linear;}
    long NodesInUsage() const 
      {if (useQuadratic) return NodesNumber_Quadratic;
         else return NodesNumber_Linear;}
  
	//....................................................................
    // Coordinate system
    int GetCoordinateFlag () const {return coordinate_system;}
    void FillTransformMatrix();  
	//....................................................................
    // GEO-PNT
    long GetNODOnPNT(CGLPoint*);
    long GetNearestELEOnPNT(CGLPoint*);
    // GEO-LIN
    void SetLINPointsClose(CGLLine*);
    void CreateLineELEFromLIN(CGLLine*);
    // GEO-PLY
    void GetNODOnPLY(CGLPolyline*,vector<long>&);
    void GetNodesOnArc(CGLPolyline*m_ply, vector<long>&msh_nod_vector);  
    void GetNODOnPLY_XY(CGLPolyline*m_ply,vector<long>&msh_nod_vector);
    void CreateLineELEFromPLY(CGLPolyline*,int,CFEMesh*);
    void CreateLineELEFromPLY(CGLPolyline*);
    void CreateLayerPolylines(CGLPolyline*); //OK
    // GEO-SFC
    void GetNODOnSFC(Surface*,vector<long>&);
    void GetNODOnSFC_PLY(Surface*,vector<long>&);
    void GetNODOnSFC_PLY_XY(Surface*,vector<long>&);
    void GetNODOnSFC_TIN(Surface*,vector<long>&);
    void GetNodesOnCylindricalSurface(Surface*m_sfc, vector<long>& NodesS);
    void GetNODOnSFC_Vertical(Surface*,vector<long>&);
    void CreateQuadELEFromSFC(Surface*);
    void CopySelectedNodes(vector<long>&msh_nod_vector); //TK
    void GetELEOnSFC(Surface*,vector<long>&); //OK
    void GetELEOnSFC_TIN(Surface*,vector<long>&); //OK
    // GEO-VOL
    void ELEVolumes(); //OK
    //....................................................................
    // QUAD->HEX
    void CreateHexELEFromQuad(int,double);
    // QUAD->LINE
    void CreateLineELEFromQuad(int,double,int);
    void SetActiveElements(vector<long>&);
    void SetMSHPart(vector<long>&,long); //MB
    bool NodeExists(long node);
    // NOD-ELE relations
    void SetNOD2ELETopology();
    // ELE-NOD relations
    void SetELE2NODTopology();
    // LINE->LINE
    void AppendLineELE();
    // TRI->PRIS
    void CreatePriELEFromTri(int,double);
    // All nodes
    vector<Mesh_Group::CNode*> nod_vector;
    // All edges
    vector<Mesh_Group::CEdge*> edge_vector;
    // All surface feces
    vector<Mesh_Group::CElem*> face_vector;
    // All surface nomal
    vector<double*> face_normal;        //YD
    // All elements 
    vector<Mesh_Group::CElem*> ele_vector;
    // Nodes in usage
    // To record eqs_index->global node index
    vector<long> Eqs2Global_NodeIndex;
    void PrismRefine(const int Layer, const int subdivision); //OK
    void EdgeLengthMinMax(); //OK
    void SetMATGroupFromVOLLayer(CGLVolume*); //OK
    void SetMATGroupsFromVOLLayer(); //OK
    void ConnectedNodes(); //OK
    void CreateSurfaceTINfromTri(Surface*); //OK
    void CreateLayerSurfaceTINsfromPris(Surface*); //OK
    // MAT
    vector<string>mat_names_vector; //OK
    void DefineMobileNodes(CRFProcess*); //OK
    void FaceNormal(); // YD
};

} // namespace Mesh_Group

using Mesh_Group::CFEMesh;

extern int COOD;
extern long msh_no_line;
extern long msh_no_quad;
extern long msh_no_hexs;
extern long msh_no_tris;
extern long msh_no_tets;
extern long msh_no_pris;

#endif
