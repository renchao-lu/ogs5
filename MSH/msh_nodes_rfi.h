#ifndef msh_nodes_INC
#define msh_nodes_INC
// C++
#include <vector>
using namespace std;
// MSHLib
#include "elements.h"
#include "msh_lib.h"

class CMSHNodes
{
  private:
  public:
    //--------------------------------------------------------------------
    // Properties
     // GEO
    double x;
    double y;
    double z;
     // MSH
    long rfi_node_id;
	long nodenumber;
    long origin_rfi_node_number;
	long serialized_rfi_node_number;
    vector<long>ele_number_vector; //OK
    //vector<FiniteElement::CElement*>ele_vector_new; //OK
    vector<Element*>ele_vector; //OK
     // EQS
    long eqs_index; //OK
    bool selected; //OK

    //--------------------------------------------------------------------
    // Methods
    CMSHNodes(void);
    ~CMSHNodes(void);
};

extern void MSHDeleteElementFromList (long,long);
extern int GEOReadFile(char *dateiname);
extern void MSHSelectFreeSurfaceNodes (CFEMesh* m_msh);
extern long MSHGetNextNode(long, CFEMesh* m_msh);
extern double MSHGetNodeArea(long);
extern long* MSHGetNeighborNodes(long);
extern long* MSHGetNodesInColumn(long, int, CFEMesh* m_msh);
extern double MSHCalcTriangleArea3 (double*, double*, double*);
extern double* MSHGetGravityCenter(long);
extern long GEOIsNodeAMovingNode(long);
extern void MSHMoveNODUcFlow (CRFProcess*m_pcs);
extern void MSHDefineMobile (CRFProcess*m_pcs);
extern void MSHConstructVerticalEdgeListHex (void);
extern void MSHConstructVerticalEdgeListPrism (void);
extern void MSHConstructVerticalEdgeListRectangle (void);

typedef struct
{
  double x;
  double y;
  double z;
  double value;
  long number;
  double MeshDensity;
} RF_POINT;

#endif
