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

// new GEOLIB
#include "GEOObjects.h"

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
	// GEO
	/**
	 * index of geometric object (GEOLIB::Point, GEOLIB::Polyline, ...)
	 */
	size_t _geo_obj_idx; // TF 18.05.2010
	/**
	 * the id of the geometric object as string REMOVE CANDIDATE
	 */
	std::string geo_name; // TF 05/2010

	std::string tim_type_name; // Time function type
	std::string fname; //27.02.2009. WW
	int CurveIndex; // Time funtion index
public:
	/**
	 * retrieves the index of the geometric object
	 * @return the index in the vector storing the geometric entities
	 */
	size_t getGeoObjIdx() const; // TF 18.05.2010
	/**
	 * ToDo remove after transition to new GEOLIB - REMOVE CANDIDATE
	 * getGeoName returns a string used as id for geometric entity
	 * @return the value of attribute geo_name in case of
	 * geo_type_name == POLYLINE or geo_type_name = SURFACE
	 * If geo_type_name == POINT the id of the point is returned.
	 */
	const std::string& getGeoName(); // TF 05/2010

	/**
	 * set the file name attribute - REMOVE CANDIDATE
	 * @param name
	 */
	void setFileName (const std::string &name); // TF 05/2010
	std::string getFileName () const { return fname; } // TF 05/2010
	void setCurveIndex (int curve_index) { CurveIndex = curve_index; } // TF 05/2010
	int getCurveIndex () const { return CurveIndex; }; // TF 05/2010
	void setTimTypeName (const std::string &name) { tim_type_name = name;} // TF 05/2010
	std::string getTimTypeName () const { return tim_type_name; } // TF 05/2010


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
    double node_value_cond; //OK
    double condition; //OK
    double epsilon; //NW. temporally set here for surfarce interpolation
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

	CBoundaryCondition();
    ~CBoundaryCondition();
    /**
     * reads a boundary condition from stream
     * @param in input file stream for reading
     * @param geo_obj pointer to the geometric object manager
     * @param unique_fname the project name
     * @return the position in the stream after the boundary condition
     */
    ios::pos_type Read(std::ifstream* in, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_fname); // TF
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

/**
 * read boundary conditions from file
 * @param file_base_name the base name of the file (without extension)
 * @param geo_obj the geometric object managing geometric entities
 * @param unique_name the (unique) name of the project
 * @return false, if the file can not opened, else true
 */
bool BCRead (std::string file_base_name, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_name);

extern void BCWrite(std::string);
extern void BCDelete();
extern void BCGroupDelete(std::string,std::string);
extern void BCGroupDelete(void);
extern CBoundaryCondition* BCGet(const std::string&,const std::string&,const std::string&); //OK
extern CBoundaryCondition* BCGet(std::string); //OK

//ToDo
extern long IsNodeBoundaryCondition(char *name, long node);
extern void ScalingDirichletBoundaryConditions(const double factor);

#endif
