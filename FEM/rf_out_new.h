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
using namespace std;
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

  public:
    // ID 
    string ID; //OK4709
    string file_base_name;
    string delimiter_type;
    // PCS
    string primary_variable;
    string pcs_type_name;
    int pcs_vector_number;//CC
    string pcs_pv_name;//CMCD
    CRFProcess* GetPCS(const string&); //OK
    CRFProcess* GetPCS_ELE(const string&); //OK
    CRFProcess* m_pcs; //OK
    // NOD values
    string nod_value_name;
    vector<string>nod_value_vector;
    void GetNodeIndexVector(vector<int>&); //OK
    void SetNODFluxAtPLY(); //OK
    // ELE values
    string ele_value_name;
    vector<string>ele_value_vector;
    void GetELEValuesIndexVector(vector<int>&); //OK
    // GEO
    long geo_node_number;
    string geo_name;
    int geo_type;
    string geo_type_name;
    // MSH
    long msh_node_number;
    string msh_type_name; //OK
    CFEMesh* GetMSH(); //OK
    // TIM
    string tim_type_name;
    vector<double>time_vector;
    double time;
    int nSteps; // After each nSteps, make output
    // DAT
    string dat_type_name;
    vector<double*>out_line_vector;
	bool INITIAL_CONDITIONS;
    bool selected;
    // DIS
    string dis_type_name; //OK
	// MAT values 
    vector<string>mmp_value_vector; //OK
    vector<string>mfp_value_vector; //OK
    // PCON values
    string pcon_value_name;
    vector<string>pcon_value_vector;
    // File status
    bool new_file_opened;   //WW
    //--------------------------------------------------------------------
	COutput(void);
    ~COutput(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    int GetPointClose(CGLPoint);
    void WriteTimeCurveData(fstream &);
    void WriteTimeCurveHeader(fstream &);
    void NODWriteDOMDataTEC();
    void WriteTECHeader(fstream& ,int,string);
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
    void WriteVTKHeader(fstream&);
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
extern vector<COutput*>out_vector;
extern bool OUTRead(string);
extern void OUTWrite(string);
#define OUT_FILE_EXTENSION ".out"
extern void OUTData(double, const int step);
extern void OUTDelete();
extern COutput* OUTGet(string);
extern void OUTCheck(void); // new SB

#endif
