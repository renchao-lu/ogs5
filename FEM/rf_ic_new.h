/**************************************************************************
FEMLib - Object: Initial Conditions IC
Task: class implementation
Programing:
08/2004 OK Implementation
last modified
**************************************************************************/
#ifndef rf_ic_new_INC
#define rf_ic_new_INC

#define IC_FILE_EXTENSION ".ic"

// C++ STL
#include <fstream>
#include <string>
#include <vector>
using namespace std;
#include "rf_node.h"
#include "rf_pcs.h"
/*---------------------------------------------------------------*/
class CInitialCondition
{
  private:
   int SubNumber; //WW
   vector<int> subdom_index; //WW
   vector<double> subdom_ic; //WW
   // Coefficents for linear distribution function
   // f = a0+b0*x+c0*y+d0*z
   double *a0, *b0, *c0, *d0; //WW
  public:
    // PCS
    string pcs_type_name; //OK
    string pcs_pv_name; //OK
    CRFProcess* m_pcs; //OK
    // GEO
    string geo_type_name;
    int geo_type;
    int GetNumDom() const {return (int)subdom_index.size();} //WW
    int GetDomain(const int dom_index) const {return subdom_index[dom_index];} //WW
    string geo_name;
	//int mat_type; //MX
    // DIS
    string dis_type_name;
    vector<CNodeValue*>node_value_vector;
    void SetDomain(int);
    void SetPolyline(int);
    double DistributionFuntion(const int dom_i, const double x, const double y, const double z) //WW
          { return a0[dom_i]+b0[dom_i]*x+c0[dom_i]*y+d0[dom_i]*z;}
    void SetPoint(int);  //MX
	//void SetMaterialDomain(int); //MX
    string delimiter_type;
	double gradient_ref_depth;
	double gradient_ref_depth_value;
	double gradient_ref_depth_gradient;
    string rfr_file_name; //OK
    CInitialCondition(void);
    ~CInitialCondition(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    void Set(int);
    void SetEle(int);  //MX
    void SetDomainEle(int); //MX
    CFEMesh* m_msh;
};

class CInitialConditionGroup
{
  public:
    string pcs_type_name; //OK
    string pcs_pv_name; //OK
    vector<CNodeValue*>group_vector;
  public:
    void Set();
};

extern vector<CInitialConditionGroup*>ic_group_vector;
extern vector<CInitialCondition*>ic_vector;
extern bool ICRead(string);
extern void ICWrite(string);
extern void ICDelete();
extern CInitialCondition* ICGet(string); //OK

#endif
