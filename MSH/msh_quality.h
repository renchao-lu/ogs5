#ifndef meshtest_INC

#define meshtets_INC
// FEM
#include "prototyp.h" // PROTOTYP:H MUSS GANZ OBEN STEHEN(STEHT VOR ALLEN ANDEREN INCLUDES)
#include "elements.h"
#include "nodes.h"

#include "msh_elements_rfi.h"

// C++ STL
#include <string>
#include <list>
#include <vector>
using namespace std;


/*---------------------------------------------------------------*/
class CMSHNodesTest
{
  private:
  public:
    long rfi_node_id;
    double x;
    double y;
    double z;
    long tri_mesh_number;
	// constructor
    CMSHNodesTest(void);
    // destructor
    ~CMSHNodesTest(void);
};

/*---------------------------------------------------------------*/
class CMSHElementsTest
{
  private:
  public:
    long rfi_element_id;
    long elementtype;
    long node1, node2, node3, node4, node5, node6, node7, node8;
	long layernumber;
    long materialnumber;
	// constructor
    CMSHElementsTest(void);
    // destructor
    ~CMSHElementsTest(void);
};
/*---------------------------------------------------------------*/




extern void CalculateBasicTriangleData();  
extern void CalculateTriangleAreaQuality();
extern void CalculateTriangleAngleQuality();
extern void CalculateTriangleLengthQuality();
extern void CalculateBasicTetrahedraData();  
extern void CalculateTetrahedraVolumeQuality();
extern void CalculateTetrahedraTriangleAngleQuality();
#endif
