/**************************************************************************
FEMLib - Object: Boundary Conditions
Task: class implementation
Programing:
02/2004 OK Implementation
last modified
**************************************************************************/
#ifndef rf_bc_new_INC
#define rf_bc_new_INC

#include <list>
#include <fstream>
#include <string>
#include <vector>

// GEOLib
#include "geo_ply.h"
// MSHLib
#include "msh_lib.h"
// PCSLib
#include "rf_pcs.h"

using namespace Mesh_Group;

class CBoundaryCondition
{
  private:
    std::string tim_type_name; // Time function type
    std::string fname; //27.02.2009. WW
	int CurveIndex; // Time funtion index
  public: //OK
    std::vector<int> PointsHaveDistribedBC;
    std::vector<int> PointsCurvesIndex;
    std::vector<std::string> PointsFCTNames;
    std::vector<double> DistribedBC;
  private: //OK
    friend class CBoundaryConditionsGroup;
  public:
    // PCS
    std::string pcs_pv_name; //OK
    std::string pcs_type_name;
    std::string pcs_type_name_cond; //OK
    std::string pcs_pv_name_cond; //OK
    int pcs_number;
    // GEO
    int geo_type;
    std::string geo_type_name;
    std::string geo_name;
    // DIS
    int dis_type;
    std::string dis_type_name;
    std::string dis_file_name;
    std::vector<long>node_number_vector;
    std::vector<double>node_value_vector;
    std::vector<long>node_renumber_vector;
    long geo_node_number;
    double geo_node_value;
	double periode_time_length, periode_phase_shift; // JOD
    std::string delimiter_type;
    double node_value_cond; //OK
    double condition; //OK
    std::string geo_node_substitute; //OK
    bool time_dep_interpol;
    // FCT
    std::string fct_name;
    bool conditional;
	bool periodic; // JOD
    // DB
    std::string db_file_name;
    void ExecuteDataBasePolyline(CGLPolyline*);
    void SurfaceIntepolation(CRFProcess* m_pcs, std::vector<long>&nodes_on_sfc,
                                std::vector<double>&node_value_vector); //WW
    inline void DirectAssign(const long ShiftInNodeVector);  //27.02.2009. WW
    inline void PatchAssign(const long ShiftInNodeVector);  //19.03.2009. WW
    // MSH
    long msh_node_number;
    std::string msh_type_name; //OK4105
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

class CBoundaryConditionNode //OK raus
{
  public:
    long geo_node_number;
    long msh_node_number;
    long msh_node_number_subst; //WW

    double node_value;
    int CurveIndex; // Time dependent function index
    std::string pcs_pv_name; //YD/WW
    //
    std::string fct_name; //WW
    //FCT
    int conditional; //OK
    CBoundaryConditionNode();
};

//========================================================================
// ToDo virtual function CNodeValueGroup
class CBoundaryConditionsGroup
{
  public:
    std::string group_name;
    std::string pcs_type_name; //OK
    std::string pcs_pv_name; //OK
    long msh_node_number_subst; //WW
    std::string fct_name; //OK
  public:
    //WW std::vector<CBoundaryConditionNode*>group_vector;
    void Set(CRFProcess* m_pcs, const int ShiftInNodeVector, std::string this_pv_name="");
    CBoundaryConditionsGroup* Get(std::string);
	CBoundaryConditionsGroup(void);
    ~CBoundaryConditionsGroup(void);
    void WriteTecplot(void);
    CFEMesh* m_msh; //OK
    //WW std::vector<CBoundaryCondition*>bc_group_vector; //OK
    //WW double GetConditionalNODValue(int,CBoundaryCondition*); //OK
    int time_dep_bc;

   public: // TK display
     int m_display_mode_bc;
};

//========================================================================
#define BC_FILE_EXTENSION ".bc"
extern list<CBoundaryConditionsGroup*> bc_group_list;
extern CBoundaryConditionsGroup* BCGetGroup(std::string pcs_type_name,std::string pcs_pv_name);
extern list<CBoundaryCondition*> bc_list;
extern bool BCRead(std::string);
extern void BCWrite(std::string);
extern void BCReadDataBase(std::string);
extern void BCExecuteDataBase(void);
extern void BCDelete();
extern void BCGroupDelete(std::string,std::string);
extern void BCGroupDelete(void);
extern CBoundaryCondition* BCGet(std::string,std::string,std::string); //OK
extern CBoundaryCondition* BCGet(std::string); //OK

//ToDo
extern long IsNodeBoundaryCondition(char *name, long node);
extern void ScalingDirichletBoundaryConditions(const double factor);

#endif
