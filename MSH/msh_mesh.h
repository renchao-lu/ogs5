/**
 * \file msh_mesh.h
 */


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

/** depreciated includes of GEOLib */
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_vol.h"

// GEOLIB
#include "Point.h"
#include "Polyline.h"

// MSHLib
#include "msh_elem.h"

#ifdef RANDOM_WALK
class RandomWalk;
#endif

class CFluidMomentum;

#ifdef NEW_EQS    //1.11.2007 WW
namespace Math_Group { class SparseTable;}
using Math_Group::SparseTable;
#endif
//------------------------------------------------------------------------
namespace Mesh_Group
{
//------------------------------------------------------------------------
// Class definition
class CFEMesh {
public:
	std::string pcs_name;
	std::string geo_name; //MB
	std::string geo_type_name; //OK10_4310
	int ele_type;
	bool selected;
	int no_msh_layer; //OK
	int nr, ns; //OK
	double z_min, z_max; //OK
	int max_mmp_groups; //OKCC

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

#ifdef RANDOM_WALK
	RandomWalk* PT; // PCH
#endif

	CFluidMomentum* fm_pcs; // by PCH
	CFEMesh(void);
	~CFEMesh(void);

	ios::pos_type Read(std::ifstream*);

#ifdef USE_TOKENBUF
	int Read(TokenBuf* tokenbuf);
#endif

	ios::pos_type ReadBIN(std::ifstream*); //OK
	void Write(fstream*);
	void WriteBIN(fstream*, fstream*); //OK
	ios::pos_type GMSReadTIN(std::ifstream*);
	//
	void ConstructGrid();
	void GenerateHighOrderNodes();
	//
	void RenumberNodesForGlobalAssembly();
	// For number of nodes
	int GetMaxElementDim() {
		return max_ele_dim;
	}
	void SwitchOnQuadraticNodes(bool quad) {
		useQuadratic = quad;
	}
	bool getOrder() const {
		return useQuadratic;
	}
	bool isAxisymmetry() const {
		return axisymmetry;
	}
	// Get number of nodes
	long GetNodesNumber(const bool quadr)//CMCD int to long
	{
		if (quadr)
			return NodesNumber_Quadratic;
		else
			return NodesNumber_Linear;
	}

	long NodesInUsage() const {
		if (useQuadratic)
			return NodesNumber_Quadratic;
		else
			return NodesNumber_Linear;
	}

	void InitialNodesNumber()//WW
	{
		NodesNumber_Quadratic = NodesNumber_Linear = (long) nod_vector.size();
	}

	//....................................................................
	// Coordinate system
	int GetCoordinateFlag() const {
		return coordinate_system;
	}
	void FillTransformMatrix();
	//....................................................................
	void GetNODOnGEO(const std::string&, const std::string&, std::vector<long>&); //OK
	// GEO-PNT
	/** depreciated method */
	long GetNODOnPNT(CGLPoint*);
	/** depreciated method */
	long GetNearestELEOnPNT(CGLPoint*);

	// GEO-PLY
	/** depreciated method */
	void GetNODOnPLY(CGLPolyline*, std::vector<long>&);
	/** depreciated method */
	void GetNodesOnArc(CGLPolyline*m_ply, std::vector<long>&msh_nod_vector);

	/** depreciated method - method uses old Polyline class  */
	void GetNODOnPLY_XY(CGLPolyline*m_ply, std::vector<long>&msh_nod_vector);

	void CreateLineELEFromPLY(CGLPolyline*, int, CFEMesh*);
	void CreateLineELEFromPLY(CGLPolyline*);
	void CreateLayerPolylines(CGLPolyline*); //OK
	void GetELEOnPLY(CGLPolyline*, std::vector<long>&); //OK
	// GEO-SFC
	void GetNODOnSFC(Surface*, std::vector<long>&);
	void GetNODOnSFC_PLY(Surface*, std::vector<long>&);
	void GetNODOnSFC_PLY_XY(Surface*m_sfc, std::vector<long>& msh_nod_vector,
			bool givenNodesOnSurface = false); // givenNodeOnSurface by WW
	void GetNODOnSFC_TIN(Surface*, std::vector<long>&);
	void GetNodesOnCylindricalSurface(Surface*m_sfc, std::vector<long>& NodesS);
	void GetNODOnSFC_Vertical(Surface*, std::vector<long>&);
	void CreateQuadELEFromSFC(Surface*);
	void CopySelectedNodes(std::vector<long>&msh_nod_vector); //TK
	void GetELEOnSFC(Surface*, std::vector<long>&); //OK
	void GetELEOnSFC_TIN(Surface*, std::vector<long>&); //OK
	void CreateLineELEFromSFC(); //OK
	void GetNODOnSFC_PLY_Z(Surface*, std::vector<long>&); // 02.2009/OK
	// GEO-VOL
	void ELEVolumes(); //OK

	/**
	 * GetNODOnPNT searchs the nearest node to the geometric point
	 * */
	long GetNODOnPNT(const GEOLIB::Point* const pnt);
	/**
	 * GetNearestELEOnPNT searchs the nearest element (gravity center)
	 * to the geometric point
	 * */
	long GetNearestELEOnPNT(const GEOLIB::Point* const pnt);

	/**
	 * GetNODOnPLY searchs the nearest node to the Polyline
	 * */
	void GetNODOnPLY(const GEOLIB::Polyline* ply, std::vector<size_t>& msh_nod_vector);

	/**
	 * circle arc described by the middle point m, the arc start point a and the arc end point b
	 * if the angle is to small (a == b) then all mesh nodes within the annulus defined by
	 * the inner radius \f$ \|(a-m)- \| - min_edge_length \f$ and the outer radius
	 * \f$\|(a-m)- \| + min_edge_length \f$ are pushed in msh_nod_vector
	 */
	void GetNodesOnArc(const GEOLIB::Point* a, const GEOLIB::Point* m,
			const GEOLIB::Point* b, std::vector<size_t>& msh_nod_vector);

	//....................................................................
	// QUAD->HEX
	void CreateHexELEFromQuad(int, double);
	// QUAD->LINE
	void CreateLineELEFromQuad(int, double, int);
	void SetActiveElements(std::vector<long>&);
	void SetMSHPart(std::vector<long>&, long); //MB
	bool NodeExists(long node);
	// NOD-ELE relations
	//OK411 void SetNOD2ELETopology();
	// ELE-NOD relations
	void SetELE2NODTopology();
	// LINE->LINE
	void AppendLineELE();
	// TRI->PRIS
	void CreatePriELEFromTri(int, double);
#ifdef MFC
	void LayerMapping(const char *dateiname, const int NLayers,
			const int row, const int DataType, int integ, int infil_integ); //19.01.2009. WW
	void LayerMapping_Check(const char *dateiname, const int NLayers, int integ); //19.01.2009. WW
	inline void WriteCurve2RFD(const int NLayers, const char *dateiname);
#endif
	// TRI->LINE
	void CreateLineELEFromTri(); //OK
	void CreateLineELEFromTriELE(); //OK
	// All nodes
	std::vector<Mesh_Group::CNode*> nod_vector;
	// All edges
	std::vector<Mesh_Group::CEdge*> edge_vector;
	// All surface feces
	std::vector<Mesh_Group::CElem*> face_vector;
	// All surface nomal
	std::vector<double*> face_normal; //YD
	// All elements
	std::vector<Mesh_Group::CElem*> ele_vector;
	// Nodes in usage
	// To record eqs_index->global node index
	std::vector<long> Eqs2Global_NodeIndex;
	void PrismRefine(const int Layer, const int subdivision); //OK
//	void EdgeLengthMinMax(); //OK
	void SetMATGroupFromVOLLayer(CGLVolume*); //OK
	void SetMATGroupsFromVOLLayer(); //OK
	void ConnectedNodes(bool quadratic); //OK
	void ConnectedElements2Node(bool quadratic = false); //WW
	void CreateSurfaceTINfromTri(Surface*); //OK
	void CreateLayerSurfaceTINsfromPris(Surface*); //OK
	// MAT
	std::vector<std::string> mat_names_vector; //OK
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
	const {if(!quad) return sparse_graph; else return sparse_graph_H;}
#endif

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
	int map_counter; //21.01.2009 WW
	bool mapping_check; //23.01.2009 WW

	double min_edge_length; //TK
	double max_edge_length; //TK

	// Sparse graph of this mesh. 1.11.2007 WW
#ifdef NEW_EQS
	SparseTable *sparse_graph;
	SparseTable *sparse_graph_H; // For high order interpolation
#endif
	// LINE
	void CheckMarkedEdgesOnPolyLine(CGLPolyline*m_polyline,
			std::vector<long> &ele_vector_at_ply); //NW
	void CreateLineElementsFromMarkedEdges(CFEMesh*m_msh_ply,
			std::vector<long> &ele_vector_at_ply); //NW
	bool HasSameCoordinatesNode(CNode* nod, long &node_no); //NW

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
