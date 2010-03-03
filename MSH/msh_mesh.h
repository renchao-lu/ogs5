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
#include "Configure.h"
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
//
#ifdef NEW_EQS    //1.11.2007 WW
namespace Math_Group { class SparseTable;}
using Math_Group::SparseTable;
#endif
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
	//int max_ele_dim; // For overland flow and channel flow only
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
    bool has_multi_dim_ele;
    int max_ele_dim; 
    int map_counter;  //21.01.2009 WW
    bool mapping_check;  //23.01.2009 WW
    // Sparse graph of this mesh. 1.11.2007 WW
#ifdef NEW_EQS
    SparseTable *sparse_graph;  
    SparseTable *sparse_graph_H; // For high order interpolation  
#endif
    // LINE
    void CheckMarkedEdgesOnPolyLine(CGLPolyline*m_polyline, vector<long> &ele_vector_at_ply); //NW
    void CreateLineElementsFromMarkedEdges(CFEMesh*m_msh_ply, vector<long> &ele_vector_at_ply); //NW
    bool HasSameCoordinatesNode(CNode* nod, long &node_no); //NW

 public:	
    string pcs_name;
    string geo_name; //MB
    string geo_type_name; //OK10_4310
    int ele_type;
    bool selected;
    int no_msh_layer; //OK
    int nr,ns; //OK
    double z_min,z_max; //OK
    int max_mmp_groups; //OKCC
//    int mat_group; //OK
    double min_edge_length; //TK
    double max_edge_length; //TK
    long msh_no_line;
    long msh_no_quad;
    long msh_no_hexs;
    long msh_no_tris;
    long msh_no_tets;
    long msh_no_pris;
    int msh_max_dim;
    bool cross_section;
  //3D View Control
    //
    int ele_display_mode;
    int nod_display_mode;
    int patch_display_mode;
    int ele_mat_display_mode;
    int highest_mat_group_nb;
    bool m_bCheckMSH; //OK

#ifdef MFC //FS//WW
    double *n_area_val; 
#endif

#ifdef RANDOM_WALK
    RandomWalk* PT; // PCH
#endif
    CFluidMomentum* fm_pcs; // by PCH
    CFEMesh(void);
    ~CFEMesh(void);
    ios::pos_type Read(ifstream*);
#ifdef USE_TOKENBUF
    int Read(TokenBuf* tokenbuf);
#endif
    ios::pos_type ReadBIN(ifstream*); //OK
    void Write(fstream*);
    void WriteBIN(fstream*,fstream*); //OK
    ios::pos_type GMSReadTIN(ifstream*);
    // 
    void ConstructGrid();
    void GenerateHighOrderNodes();
    //
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
    void InitialNodesNumber()//WW
      { NodesNumber_Quadratic = NodesNumber_Linear = (long)nod_vector.size();}
  
	//....................................................................
    // Coordinate system
    int GetCoordinateFlag () const {return coordinate_system;}
    void FillTransformMatrix();  
	//....................................................................
    void GetNODOnGEO(string,string,vector<long>&); //OK
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
    void GetELEOnPLY(CGLPolyline*,vector<long>&); //OK
    // GEO-SFC
    void GetNODOnSFC(Surface*,vector<long>&);
    void GetNODOnSFC_PLY(Surface*,vector<long>&);
    void GetNODOnSFC_PLY_XY(Surface*m_sfc, vector<long>& msh_nod_vector, bool givenNodesOnSurface=false); // givenNodeOnSurface by WW
    void GetNODOnSFC_TIN(Surface*,vector<long>&);
    void GetNodesOnCylindricalSurface(Surface*m_sfc, vector<long>& NodesS);
    void GetNODOnSFC_Vertical(Surface*,vector<long>&);
    void CreateQuadELEFromSFC(Surface*);
    void CopySelectedNodes(vector<long>&msh_nod_vector); //TK
    void GetELEOnSFC(Surface*,vector<long>&); //OK
    void GetELEOnSFC_TIN(Surface*,vector<long>&); //OK
    void CreateLineELEFromSFC(); //OK
	void GetNODOnSFC_PLY_Z(Surface*,vector<long>&); // 02.2009/OK
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
    //OK411 void SetNOD2ELETopology();
    // ELE-NOD relations
    void SetELE2NODTopology();
    // LINE->LINE
    void AppendLineELE();
    // TRI->PRIS
    void CreatePriELEFromTri(int,double);
#ifdef MFC
    void LayerMapping(const char *dateiname, const int NLayers,\
            const int row, const int DataType, int integ, int infil_integ); //19.01.2009. WW
    void LayerMapping_Check(const char *dateiname, const int NLayers, int integ); //19.01.2009. WW
    inline void WriteCurve2RFD(const int NLayers, const char *dateiname);
#endif 
    // TRI->LINE
    void CreateLineELEFromTri(); //OK
    void CreateLineELEFromTriELE(); //OK
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
    void ConnectedNodes(bool quadratic); //OK
    void ConnectedElements2Node(bool quadratic=false); //WW
    void CreateSurfaceTINfromTri(Surface*); //OK
    void CreateLayerSurfaceTINsfromPris(Surface*); //OK
    // MAT
    vector<string>mat_names_vector; //OK
    void DefineMobileNodes(CRFProcess*); //OK
    void FaceNormal(); // YD
    void SetELENormalVectors(); //OK4310
    void SetNODPatchAreas(); //OK4310
    void SetNetworkIntersectionNodes(); //OK4319->PCH
#ifdef NEW_EQS   // 1.11.2007 WW     
    // Compute the graph of the sparse matrix related to this mesh. 1.11.2007 WW
    void CreateSparseTable();  
    // Get the sparse graph   1.11.2007 WW
    SparseTable *GetSparseTable(bool quad = false) 
      const { if(!quad) return sparse_graph; else return sparse_graph_H;}
#endif
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

 // Okayama LIBS and DLL'S Entrance here!!!
#ifdef MFC
  extern "C" __declspec (dllexport) void TETGEN(void*,void*,void*,void*, void*,void*,void*,void*,void*,void*);
#endif

#endif
