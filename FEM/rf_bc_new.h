/**************************************************************************
FEMLib - Object: Boundary Conditions
Task: class implementation
Programing:
02/2004 OK Implementation
last modified
**************************************************************************/
#ifndef rf_bc_new_INC
#define rf_bc_new_INC
// C++ STL
#include <list>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
// GEOLib
#include "geo_ply.h"
// MSHLib
#include "msh_lib.h"
//#include "rf_ele_msh.h"

// PCSLib
#include "rf_pcs.h"
//========================================================================
using namespace Mesh_Group;
class CBoundaryCondition
{
  private:
    string tim_type_name; // Time function type
	int CurveIndex; // Time funtion index
    vector<int> PointsHaveDistribedBC;
    vector<double> DistribedBC;
    friend class CBoundaryConditionsGroup;
  public:
    // PCS
    string pcs_pv_name; //OK
    string pcs_type_name;
    string pcs_type_name_cond; //OK
    string pcs_pv_name_cond; //OK
    int pcs_number;
    // GEO
    int geo_type;
    string geo_type_name;
    string geo_name;
    // DIS
    int dis_type;
    string dis_type_name;
    string dis_file_name;
    vector<long>node_number_vector;
    vector<double>node_value_vector;
    vector<long>node_renumber_vector;
    long geo_node_number;
    double geo_node_value;
    string delimiter_type;
    double node_value_cond; //OK
    double condition; //OK
    string geo_node_substitute; //OK
    //FCT
    string fct_name;
    bool conditional;
    //DB
    string db_file_name;
    void ExecuteDataBasePolyline(CGLPolyline*);
    // MSH
    long msh_node_number;

    string msh_type_name; //OK4105

	CBoundaryCondition(void);
    ~CBoundaryCondition(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    CBoundaryCondition* Get(string,string);
    void SetDISType(void);
    void SetGEOType(void);
    void WriteTecplot(fstream*);
};

//========================================================================
class CBoundaryConditionNode //OK raus
{
  public:
    long geo_node_number;
    long msh_node_number;

    double node_value;
    int CurveIndex; // Time dependent function index
    //FCT
    int conditional; //OK
    CBoundaryConditionNode();
};

//========================================================================
// ToDo virtual function CNodeValueGroup
class CBoundaryConditionsGroup
{
  public:
    string group_name;
    string pcs_type_name; //OK
    string pcs_pv_name; //OK
    long msh_node_number_subst; //WW
    string fct_name; //OK
  public:
    vector<CBoundaryConditionNode*>group_vector;
    void Set(CRFProcess* m_pcs, const int ShiftInNodeVector, string this_pv_name="");
    CBoundaryConditionsGroup* Get(string);
	CBoundaryConditionsGroup(void);
    ~CBoundaryConditionsGroup(void);
    void WriteTecplot(void);
    CFEMesh* m_msh; //OK
    vector<CBoundaryCondition*>bc_group_vector; //OK
    double GetConditionalNODValue(int,CBoundaryCondition*); //OK

   public: // TK display
     int m_display_mode_bc;
};

//========================================================================
#define BC_FILE_EXTENSION ".bc"
extern list<CBoundaryConditionsGroup*>bc_group_list;
extern CBoundaryConditionsGroup* BCGetGroup(string pcs_type_name,string pcs_pv_name);
extern list<CBoundaryCondition*> bc_list;
extern bool BCRead(string);
extern void BCWrite(string);
extern void BCReadDataBase(string);
extern void BCExecuteDataBase(void);
extern void BCDelete();
extern void BCGroupDelete(string,string);

//ToDo
extern long IsNodeBoundaryCondition(char *name, long node);
extern void ScalingDirichletBoundaryConditions(const double factor);
extern int NodeSetBoundaryConditions(char*,int,string);

#endif
