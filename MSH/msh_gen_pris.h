#ifndef prisgen_INC

#define prisgen_INC
// FEM
#include "prototyp.h" // PROTOTYP:H MUSS GANZ OBEN STEHEN(STEHT VOR ALLEN ANDEREN INCLUDES)
#include "ptrarr.h"
#include "elements.h"
#include "nodes.h"
//MSHLib
#include "msh_mesh.h"
// C++ STL
#include <string>
#include <list>
#include <vector>
using namespace std;

/*---------------------------------------------------------------*/
class CMSHNodesPrisGen
{
  private:
  public:
    long rfi_node_id;
    double x;
    double y;
    double z;
    long tri_mesh_number;
	// constructor
    CMSHNodesPrisGen(void);
    // destructor
    ~CMSHNodesPrisGen(void);
};

/*---------------------------------------------------------------*/
class CMSHElementsPrisGen
{
  private:
  public:
    long rfi_element_id;
    long elementtype;
    long node1, node2, node3, node4, node5, node6, node7, node8;
	long layernumber;
    long materialnumber;
	// constructor
    CMSHElementsPrisGen(void);
    // destructor
    ~CMSHElementsPrisGen(void);
};
/*---------------------------------------------------------------*/

extern void Create_Quads2Tri(CFEMesh*m_msh);
extern void Create_Triangles2Prisms(long nb_prism_layers,double thickness_prism_layers,CFEMesh*m_msh);  
extern void Create_Quads2Hex(long nb_layers,double thickness_layers,CFEMesh*m_msh);
extern void MSH_OverWriteMSH (string m_filepath);
extern void MSH_Destroy_PrismDataConstructs(void);
extern void MSH_Tri_from_Quads();

extern vector<CMSHNodesPrisGen*>msh_pris_nodes_vector; //OK
extern vector<CMSHElementsPrisGen*> msh_pris_elements_vector; //OK

#endif
