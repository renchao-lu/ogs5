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
  public: //OK
    vector<int> PointsHaveDistribedBC;
    vector<int> PointsCurvesIndex;
    vector<string> PointsFCTNames;
    vector<double> DistribedBC;
  private: //OK
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
	double periode_time_length, periode_phase_shift; // JOD
    string delimiter_type;
    double node_value_cond; //OK
    double condition; //OK
    string geo_node_substitute; //OK
    bool time_dep_interpol;
    // FCT
    string fct_name;
    bool conditional;
	bool periodic; // JOD
    // DB
    string db_file_name;
    void ExecuteDataBasePolyline(CGLPolyline*);
    void SurfaceIntepolation(CRFProcess* m_pcs, vector<long>&nodes_on_sfc, 
                                vector<double>&node_value_vector); //WW
    // MSH
    long msh_node_number;
    string msh_type_name; //OK4105
    // GUI
    bool display_mode;

	CBoundaryCondition(void);
    ~CBoundaryCondition(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
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
    long msh_node_number_subst; //WW

    double node_value;
    int CurveIndex; // Time dependent function index
    string pcs_pv_name; //YD/WW
    //
    string fct_name; //WW
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
    //WW vector<CBoundaryConditionNode*>group_vector;
    void Set(CRFProcess* m_pcs, const int ShiftInNodeVector, string this_pv_name="");
    CBoundaryConditionsGroup* Get(string);
	CBoundaryConditionsGroup(void);
    ~CBoundaryConditionsGroup(void);
    void WriteTecplot(void);
    CFEMesh* m_msh; //OK
    //WW vector<CBoundaryCondition*>bc_group_vector; //OK
    //WW double GetConditionalNODValue(int,CBoundaryCondition*); //OK
    int time_dep_bc;

   public: // TK display
     int m_display_mode_bc;
};

//========================================================================
#define BC_FILE_EXTENSION ".bc"
extern list<CBoundaryConditionsGroup*> bc_group_list;
extern CBoundaryConditionsGroup* BCGetGroup(string pcs_type_name,string pcs_pv_name);
extern list<CBoundaryCondition*> bc_list;
extern bool BCRead(string);
extern void BCWrite(string);
extern void BCReadDataBase(string);
extern void BCExecuteDataBase(void);
extern void BCDelete();
extern void BCGroupDelete(string,string);
extern void BCGroupDelete(void);
extern CBoundaryCondition* BCGet(string,string,string); //OK
extern CBoundaryCondition* BCGet(string); //OK

//ToDo
extern long IsNodeBoundaryCondition(char *name, long node);
extern void ScalingDirichletBoundaryConditions(const double factor);
//WW extern int NodeSetBoundaryConditions(char*,int,string);
extern int NodeSetBoundaryConditions(char *pv_name,int ndx,CRFProcess* m_pcs);

#endif
