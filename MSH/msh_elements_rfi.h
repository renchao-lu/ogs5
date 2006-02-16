#ifndef MshLib_INC
#define MshLib_INC
// C++ STL
#include <string>
#include <list>
#include <vector>
using namespace std;
// GEOLib
#include "geo_ply.h"
// MSHLib
#include "msh_gen_pris.h"
#include "msh_nodes_rfi.h"
// FEM
#include "prototyp.h" // PROTOTYP:H MUSS GANZ OBEN STEHEN(STEHT VOR ALLEN ANDEREN INCLUDES)
#include "ptrarr.h"
#include "elements.h"
#include "nodes.h"

/*---------------------------------------------------------------*/
class CMSHElements
{
  private:
  public:
    long element_id;
    long elementtype;
    long node1, node2, node3, node4, node5, node6, node7, node8;
	long layernumber;
    long materialnumber;
    long element_type;
    double x1,x2,x3,x4,x5,x6,x7,x8; //x1 = x-coordinate of node 1
    double y1,y2,y3,y4,y5,y6,y7,y8; // point coordinates
    double z1,z2,z3,z4,z5,z6,z7,z8;

  public: //mathematical constraints
    double a1,a2,a3,b1,b2,b3,c1,c2,c3; //Vektor coordinates
	double d1,d2,d3,e1,e2,e3,f1,f2,f3; //Vektor coordinates
    double a_quantum, b_quantum, c_quantum;
    double d_quantum, e_quantum, f_quantum;
	double cos_alpha, cos_beta, cos_gamma;
	double alpha_deg, beta_deg, gamma_deg;
	double alpha_rad, beta_rad, gamma_rad;
	double area, tetra_area;
	double tetra_tri_area_1, tetra_tri_area_2, tetra_tri_area_3, tetra_tri_area_4;
	double cos_alpha1, cos_beta1, cos_gamma1;
	double alpha_deg1, beta_deg1, gamma_deg1;
	double alpha_rad1, beta_rad1, gamma_rad1;
	double cos_alpha2, cos_beta2, cos_gamma2;
	double alpha_deg2, beta_deg2, gamma_deg2;
	double alpha_rad2, beta_rad2, gamma_rad2;
	double cos_alpha3, cos_beta3, cos_gamma3;
	double alpha_deg3, beta_deg3, gamma_deg3;
	double alpha_rad3, beta_rad3, gamma_rad3;
	double perimeter;
    double perimeter_tetra_tri_1, perimeter_tetra_tri_2, perimeter_tetra_tri_3, perimeter_tetra_tri_4;
    double tetra_volume;
	double circumsphere_radius;
	double circumcenter_x,circumcenter_y,circumcenter_z;
	bool element_marker;
  public:
    double quality_factor, area_quality, angle_quality, length_quality;
    double volume_quality;

	// constructor
    CMSHElements(void);
    // destructor
    ~CMSHElements(void);
};
/*---------------------------------------------------------------*/

extern vector<CMSHElements*>msh_elements_vector;
extern vector<CMSHNodes*>msh_nodes_vector;
extern vector<CMSHElements*> MSHGetElementVector(void);
extern vector<CMSHNodes*> MSHGetNodeVector(void);
extern void Get_Rockflow_RFI ();
extern void Get_Rockflow_RFI_from_File (string filename);
extern void Get_MSH_for_Quality (int meshnumber);

extern void MSH2RFI (char *filename, FILE *rfi_file);
extern int ReadRFInputFile ( char *datei, FILE *prot );
extern int ReadRFIFile(string g_strFileNameBase);
extern void Serialize_RFI (string filename);
extern bool msh_new;

extern void SearchElementsWithPointInCircumsphere(double x, double y, double z);
extern long PointInsideTriangleCircumsphere(double x, double y, double z, long element_id);
extern void TriCircumcenter3d(double a[3],double b[3],double c[3],double circumcenter[3]);
extern void ExecuteBowyerWatsonAlgorithm(double x, double y, double z, string projectname);
extern void RemoveNode(long nnr, string projectname);

extern void MSHOpen(string file_name_base);

#endif
