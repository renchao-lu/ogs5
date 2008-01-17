/**************************************************************************
FEMLib - Object: Source Terms ST
Task: class implementation
Programing:
01/2003 OK Implementation
last modified
**************************************************************************/
#ifndef rf_st_new_INC
#define rf_st_new_INC
// C++ STL
#include <list>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
#include "rf_node.h"
#include "geo_ply.h"

typedef struct {
  vector<double>value_reference; 
  vector<double>last_source_value; 
  //double value_store[10][5000];
  double** value_store;//[PCS_NUMBER_MAX*2]First ref no processes(two fields per process..time, value), second ref no values
} NODE_HISTORY;

//extern vector<NODE_HISTORY*>node_history_vector;//CMCD
//========================================================================
class CSourceTerm
{
  public: //OK
    int CurveIndex; // Time funtion index
    vector<int> PointsHaveDistribedBC;
    vector<double> DistribedBC;
    vector<double> DistBC_KRiverBed;
    vector<double> DistBC_WRiverBed;
    vector<double> DistBC_TRiverBed;
    vector<double> DistBC_BRiverBed;
	
	vector<double> node_value_vectorA; // JOD
    vector<double> node_value_vectorB;
    vector<double> node_value_vectorC;
    vector<double> node_value_vectorD;
    vector<double> node_value_vectorE;    	
    vector<double> node_value_vectorArea;


    vector<double*> normal2surface; //WW
    vector<double*>pnt_parameter_vector; //OK
    vector<long>element_st_vector;   //YD
  private: 
    CGLPolyline *plyST; //OK ???
    friend class CSourceTermGroup;
  public:
    // PCS
    //string primary_variable;
    string pcs_pv_name; //OK
    string pcs_type_name;
    CRFProcess* m_pcs; //OKCMCD
    int pcs_number;
    string pcs_type_name_cond; //OK
    string pcs_pv_name_cond; //OK
	// fluid process coupling JOD
	double coup_leakance, rill_height; 
	double sorptivity, constant, rainfall, rainfall_duration, moistureDeficit;
    bool conditional, area_assembly;
    bool river;
  	bool COUPLING_SWITCH;
	// overland flow  JOD
	double normaldepth_slope;
    bool critical_depth, channel;
	double channel_width;
	// GEO
    string geo_prop_name;
    long geo_node_number;
    double geo_node_value; //Maxium three points
    string geo_type_name;
    long msh_node_number;
    int geo_id; //OK
    // DIS
    string dis_type_name;
    int component;
    int geo_type;
    int dis_type;
    double dis_prop[3];
    double *nodes;
    vector<long>node_number_vector;
    vector<double>node_value_vector;
    vector<long>node_renumber_vector;
    string delimiter_type;
    string geo_name;
    //FCT
    string fct_name;
    // TIM
    int curve;
    string tim_type_name;
    // GUI
    bool display_mode;
    //Analytical term for matrix diffusion
    bool analytical;
    int analytical_material_group;
    long resolution;
    double st_area;
    double analytical_diffusion;
    long number_of_terms;
    int no_an_sol;
    long max_no_terms;
    double factor;
    double analytical_porosity;
    double analytical_tortousity;
    double analytical_linear_sorption_Kd;
    double analytical_matrix_density;
    // MSH    
    string msh_type_name; //OK4310
    //--------------------------------------------------------------------
    // Methods
	CSourceTerm(void);
    ~CSourceTerm(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    void SetDISType(void);
    void SetGEOType(void);
    void EdgeIntegration(CFEMesh* m_msh, vector<long>&nodes_on_ply, 
                        vector<double>&node_value_vector);
    void FaceIntegration(CFEMesh* m_msh, vector<long>&nodes_on_sfc, 
                        vector<double>&node_value_vector);
    void DomainIntegration(CFEMesh* m_msh, vector<long>&nodes_in_dom, 
                        vector<double>&node_value_vector);
    // Set Members. WW
    void SetPolyline(CGLPolyline *Polyline) {plyST = Polyline;}  //OK ???

    void SetNOD2MSHNOD(vector<long>&nodes, vector<long>&conditional_nodes);

    double GetNodeLastValue (long n, int idx);
    void SetNodePastValue ( long n, int idx, int pos, double value);//CMCD
    void SetNodeLastValue (long n, int idx, double value);
    double GetNodePastValue(long,int,int);//CMCD
    double GetNodePastValueReference ( long n, int idx );//CMCD
    void CreateHistoryNodeMemory(NODE_HISTORY* nh );//CMCD
    void DeleteHistoryNodeMemory();//CMCD
    void SetSurfaceNodeVectorConditional(vector<long>&sfc_nod_vector, vector<long>&sfc_nod_vector_cond);

    void SetPolylineRiverNodeValueVectors(CGLPolyline* m_ply, int number_of_nodes);
   //void SetPolylineNodeValueVectors(CGLPolyline* m_ply, vector<long>& ply_nod_vector);
   void InterpolatePolylineRiverNodeValueVector(CGLPolyline* m_ply, vector<double>& Distribed, vector<double>& ply_nod_vector);
   void SetNodeValues(vector<long>&nodes, vector<long>&nodes_cond, vector<double>&node_values, const int ShiftInNodeVector);
   void SetNOD(); 
   ios::pos_type ReadDistributionType(ifstream *st_file);
   ios::pos_type ReadGeoType(ifstream *st_file);
};

//========================================================================
class CSourceTermGroup
{
  public:
    CSourceTermGroup() {} //WW
    CSourceTermGroup* Get(string);
    void Set(CRFProcess* m_pcs, const int ShiftInNodeVector, string this_pv_name="");
//WW    vector<CNodeValue*>group_vector;
    string pcs_name;
    string pcs_type_name; //OK
    string pcs_pv_name; //OK
    CFEMesh* m_msh;
    CFEMesh* m_msh_cond;
//WW    vector<CSourceTerm*>st_group_vector; //OK
    //WW double GetConditionalNODValue(int,CSourceTerm*); //OK
    //WW double GetRiverNODValue(int,CSourceTerm*, long msh_node); //MB
    //WW double GetCriticalDepthNODValue(int,CSourceTerm*, long msh_node); //MB
    //WW double GetNormalDepthNODValue(int,CSourceTerm*, long msh_node); //MB JOD
	 //WW Changed from the above
//    double GetAnalyticalSolution(CSourceTerm *m_st,long node_number, string process);//CMCD
    // TRANSFER OF DUAL RICHARDS
    string fct_name; //YD
  private:
	void SetPNT(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector); // JOD
	void SetLIN(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
    void SetPLY(CSourceTerm* m_st, const int ShiftInNodeVector); //OK
	void SetDMN(CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
	void SetSFC(CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
	void SetPolylineNodeVector(CGLPolyline* m_ply, vector<long>&ply_nod_vector); // JOD
	void SetPolylineNodeVectorConditional(CSourceTerm* m_st, CGLPolyline* m_ply, 
		vector<long>&ply_nod_vector, vector<long>&ply_nod_vector_cond);
    void SetPolylineNodeValueVector(CSourceTerm* m_st, CGLPolyline* m_ply, vector<long>&ply_nod_vector, vector<long>&ply_nod_vector_cond
										  , vector<double>&ply_nod_val_vector);
    void SetSurfaceNodeVector(Surface* m_sfc, vector<long>&sfc_nod_vector); // JOD
	void SetSurfaceNodeValueVector( CSourceTerm* m_st, Surface* m_sfc, vector<long>&sfc_nod_vector, vector<double>&sfc_nod_val_vector);
    void AreaAssembly(CSourceTerm* m_st,  vector<long>&ply_nod_vector_cond, 
									vector<double>& ply_nod_val_vector);
};

extern CSourceTermGroup* STGetGroup(string pcs_type_name,string pcs_pv_name);
extern list<CSourceTermGroup*> st_group_list;
extern bool STRead(string);
extern void STWrite(string);
#define ST_FILE_EXTENSION ".st"
extern void STCreateFromPNT();
extern vector<CSourceTerm*> st_vector;
extern void STDelete();
void STCreateFromLIN(vector<CGLLine*>);
CSourceTerm* STGet(string);
extern void STGroupDelete(string pcs_type_name,string pcs_pv_name);
extern void STGroupsDelete(void);//Haibing;
extern long aktueller_zeitschritt;
extern double aktuelle_zeit;
extern vector<string>analytical_processes;
extern CSourceTerm* STGet(string,string,string); //OK

// WW moved here
extern  double GetAnalyticalSolution(long node_number, CSourceTerm *m_st);//CMCD, WW
extern  void GetRiverNODValue(double& value, CNodeValue* cnodev,CSourceTerm* m_st, long msh_node);
extern	double GetConditionalNODValue(CSourceTerm* m_st, CNodeValue* cnodev); 
extern  void GetCriticalDepthNODValue(double& value, CSourceTerm*, long msh_node); //MB
extern  void GetNormalDepthNODValue(double& value, CSourceTerm*, long msh_node); //MB JOD
extern  void GetCouplingNODValue(double& value, CSourceTerm* m_st, CNodeValue* cnodev, long msh_node); // JOD
extern  void GetCouplingNODValueNewton(double& value, CSourceTerm* m_st, CNodeValue* cnodev, long msh_node); // JOD
extern  void GetCouplingNODValuePicard(double& value, CSourceTerm* m_st, CNodeValue* cnodev, long msh_node); // JOD
extern  void GetCouplingNODValueMixed(double& value, CSourceTerm* m_st, CNodeValue* cnodev, long msh_node); // JOD
extern  double GetRelativeCouplingPermeability(CRFProcess* m_pcs, double head, double rillDepth, long msh_node); // JOD
extern  void GetPhilipNODValue(double& value,CSourceTerm* m_st); // JOD
extern  void GetGreenAmptNODValue(double& value, CSourceTerm* m_st, long msh_node); // JOD
extern  void GetNODValue(double& value, CNodeValue* cnodev,CSourceTerm* m_st, long msh_node); // JOD
#endif
