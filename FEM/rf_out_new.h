/**************************************************************************
FEMLib - Object: OUT
Task: class implementation
Programing:
06/2004 OK Implementation
last modified:
**************************************************************************/
#ifndef rf_out_new_INC
#define rf_out_new_INC
// C++ STL
#include <list>
#include <fstream>
#include <string>
#include <vector>

// GEOLib
#include "geo_pnt.h"
// FEMLib
#include "rf_pcs.h"
#include <sstream>        // for istringstream (ME)
#include "vtk.h"
/*---------------------------------------------------------------*/
namespace Mesh_Group{class CFEMesh;}
using Mesh_Group::CFEMesh;
class COutput
{
  private:
    double out_amplifier; //WW to amplify output
    friend void OUTData(double, const int step);
    inline void WriteELEVelocity(iostream &vtk_file); //WW/OK
    CFEMesh* m_msh;
    CVTK* vtk;
	// GEO
	/**
	 * index of geometric object (GEOLIB::Point, GEOLIB::Polyline, ...)
	 */
	size_t _geo_obj_idx; // TF 05/2010
	/**
	 * the id of the geometric object as string REMOVE CANDIDATE
	 */
	std::string geo_name; // TF 05/2010

  public:
	  /**
	   * scaling factor for values
	   * @param amplifier - a double value for scaling data
	   */
	  void setAmplifier (double amplifier) { out_amplifier = amplifier; }
    // ID
    std::string ID; //OK4709
    std::string file_base_name;
    // PCS
    std::string primary_variable;
    std::string pcs_type_name;
    int pcs_vector_number;//CC
    std::string pcs_pv_name;//CMCD
    CRFProcess* GetPCS(const std::string&); //OK
    CRFProcess* GetPCS_ELE(const std::string&); //OK
    CRFProcess* m_pcs; //OK
    // NOD values
    std::string nod_value_name;
    std::vector<std::string>nod_value_vector;
    void GetNodeIndexVector(std::vector<int>&); //OK
    void SetNODFluxAtPLY(); //OK
    // ELE values
    std::string ele_value_name;
    std::vector<std::string>ele_value_vector;
    void GetELEValuesIndexVector(std::vector<int>&); //OK
	// RWPT values
	std::string rwpt_value_name;
	std::vector<std::string>rwpt_value_vector;
    // GEO
    long geo_node_number;
    int geo_type;
    std::string geo_type_name;
	/**
	 * ToDo remove after transition to new GEOLIB - REMOVE CANDIDATE
	 * getGeoName returns a string used as id for geometric entity
	 * @return the value of attribute geo_name in case of
	 * geo_type_name == POLYLINE or geo_type_name = SURFACE
	 * If geo_type_name == POINT the id of the point is returned.
	 */
	const std::string& getGeoName(); // TF 05/2010

    // MSH
    long msh_node_number;
    std::string msh_type_name; //OK
    CFEMesh* GetMSH(); //OK
    // TIM
    std::string tim_type_name;
    std::vector<double>time_vector;
	std::vector<double>rwpt_time_vector; //JTARON, needed because outputs are treated differently in RWPT
    double time;
    int nSteps; // After each nSteps, make output
    // DAT
    std::string dat_type_name;
	std::string matlab_delim; //JTARON .. make matlab outputs same as tecplot, just need to comment the header
    std::vector<double*>out_line_vector;
    // DIS
    std::string dis_type_name; //OK
	// MAT values
    std::vector<std::string>mmp_value_vector; //OK
    std::vector<std::string>mfp_value_vector; //OK
    // PCON values
    std::string pcon_value_name;
    std::vector<std::string>pcon_value_vector;
    // File status
    bool new_file_opened;   //WW
    //--------------------------------------------------------------------
	COutput(void);
    ~COutput(void);

    /**
     * read from file stream
     * @param in input file stream
     * @param geo_obj object of class GEOObjects that manages the geometric entities
     * @param unique_name the name of the project to access the right geometric entities
     * @return the new position in the stream after reading
     */
    ios::pos_type Read(std::ifstream* in, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_name);

    void Write(fstream*);
    int GetPointClose(CGLPoint);
    void WriteTimeCurveData(fstream &);
    void WriteTimeCurveHeader(fstream &);
    void NODWriteDOMDataTEC();
    void WriteTECHeader(fstream& ,int,std::string);
    void WriteTECNodeData(fstream&);
    void WriteTECElementData(fstream&  ,int);
    double NODWritePLYDataTEC(int);
    void NODWritePNTDataTEC(double,int);
    void ELEWriteDOMDataTEC();
    void WriteELEValuesTECHeader(fstream&);
    void WriteELEValuesTECData(fstream&);
    void NODWriteSFCDataTEC(int);
    void NODWriteSFCAverageDataTEC(double,int); //OK
    void WriteDataVTK(int); //GK
    void WriteVTKHeader(fstream&,int);
    void WriteVTKNodeData(fstream&);
    void WriteVTKElementData(fstream&);
    void WriteVTKValues(fstream&);
    void WriteRFO(); //OK
    void WriteRFOHeader(fstream&); //OK
    void WriteRFONodes(fstream&); //OK
    void WriteRFOElements(fstream&); //OK
    void WriteRFOValues(fstream&); //OK
    void NODWriteLAYDataTEC(int); //OK
    void ELEWriteSFC_TEC(); //OK
    void ELEWriteSFC_TECHeader(fstream&); //OK
    void ELEWriteSFC_TECData(fstream&); //OK
    void CalcELEFluxes(); //OK
    void ELEWritePLY_TEC(); //OK
    void ELEWritePLY_TECHeader(fstream&); //OK
    void ELEWritePLY_TECData(fstream&); //OK
    void TIMValue_TEC(double); //OK
    double NODFlux(long); //OK
	void PCONWriteDOMDataTEC(); //MX
	void WriteTECNodePCONData(fstream &);  //MX
};
extern std::vector<COutput*>out_vector;

/**
 * read file that stores information about output
 * @param file_base_name base file name (without extension)
 * @param geo_obj object of class GEOObjects managing the geometric entities
 * @param unique_name unique name to access the geometric entities in geo_obj
 * @return true if file reading was successful, else false
 */
bool OUTRead(const std::string& file_base_name, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_name);

extern void OUTWrite(std::string);
#define OUT_FILE_EXTENSION ".out"
extern void OUTData(double, const int step);
extern void OUTDelete();
extern COutput* OUTGet(std::string);
extern void OUTCheck(void); // new SB
extern COutput* OUTGetRWPT(std::string); //JTARON

#endif
