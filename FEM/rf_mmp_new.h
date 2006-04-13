/**************************************************************************
FEMLib-Object: MAT-MP 
Task: MediumProperties
Programing:
01/2004 OK Implementation
**************************************************************************/

#ifndef rfmat_mp_new_INC

#define rfmat_mp_new_INC
  /* Schutz gegen mehrfaches Einfuegen */

// C++ STL
#include <list>
#include <string>
#include <vector>
#include <fstream>
using namespace std;
// PCSLib
#include "rf_pcs.h"

using namespace std;
namespace FiniteElement {class CFiniteElementStd;}
using FiniteElement::CFiniteElementStd;
class CMediumProperties
{
 public:
    CFiniteElementStd *Fem_Ele_Std;
 private:
    // Data base
	void SetMediumPropertiesDefaultsClay(void);//CMCD 9/2004 GeoSys 4
	void SetMediumPropertiesDefaultsSilt(void);//CMCD 9/2004 GeoSys 4
	void SetMediumPropertiesDefaultsSand(void);//CMCD 9/2004 GeoSys 4
	void SetMediumPropertiesDefaultsGravel(void);//CMCD 9/2004 GeoSys 4
	void SetMediumPropertiesDefaultsCrystalline(void);//CMCD 9/2004 GeoSys 4
	void SetMediumPropertiesDefaultsBordenAquifer(void);//CMCD 9/2004 GeoSys 4
    // Porosity
  double PorosityEffectiveStress(long,double );//CMCD 9/2004 GeoSys 4
  double PorosityVolumetricFreeSwellingConstantIonicstrength(long,double,double);
  double PorosityEffectiveConstrainedSwelling(long,double,double, double*); //MX 1/2005
	double PorosityVolumetricFreeSwelling(long,double,double); //MX 1/2005
	double PorosityEffectiveConstrainedSwellingConstantIonicStrength(long,double,double, double*); //MX 1/2005
    // Permeability
	double PermeabilityPressureFunctionMethod1(long ,double );//CMCD 9/2004 GeoSys 4
	double PermeabilityPressureFunctionMethod2(long ,double );//CMCD 9/2004 GeoSys 4
	double PermeabilityPressureFunctionMethod3(long ,double );//CMCD 9/2004 GeoSys 4
	double PermeabilityPressureFunctionMethod4(long ,double, double );//CMCD 9/2004 GeoSys 4
    friend class CMediumPropertiesGroup;
 public:
  //-------------------------------------------
  // Methods
  CMediumProperties(void);  // constructor
  ~CMediumProperties(void); // destructor
  CMediumProperties* Get(string);
  CMediumProperties* GetDB(string);
  CMediumProperties* GetByGroupNumber(int);
  void Set(string,string,double);
  void SetDB(string,string,double);
  int GetPropertyType(string);
  ios::pos_type Read(ifstream*);
  void Write(fstream*);
  void WriteTecplot(string);
	double* PermeabilityTensor(long index);
	double Porosity(CFiniteElementStd* assem=NULL);//CMCD 9/2004 GeoSys 4
	double TortuosityFunction(long number,double*gp,double theta, CFiniteElementStd* assem=NULL);//CMCD 9/2004 GeoSys 4
	double NonlinearFlowFunction(long number, double *gp, double theta);//CMCD 9/2004 GeoSys 4
	double PermeabilityPressureFunction(long index,double *gp,double theta);//CMCD 9/2004 GeoSys 4
  double PermeabilitySaturationFunction(long number,double*gp,double theta,int phase);//CMCD 9/2004 GeoSys 4
	double PermeabilityPorosityFunction(long index,double *gp,double theta);//CMCD 9/2004 GeoSys 4
  double CapillaryPressureFunction(long number,double*gp,double theta,int phase,double saturation);//CMCD 9/2004 GeoSys 4
  double StorageFunction(long number,double*gp,double theta);//CMCD 9/2004 GeoSys 4
  double HeatCapacity(long number, double*gp,double theta, CFiniteElementStd* assem=NULL);
  double* HeatConductivityTensor(int number); //MX
  double* HeatDispersionTensorNew(int ip);//CMCD
	double* MassDispersionTensor(long number,double*gp,double theta,long component); //SB:GS4
	double* MassDispersionTensorNew(int ip); //CMCD
	double Density(long number,double*gp,double theta); //OK
  double SaturationCapillaryPressureFunction(long number,double*gp,double theta,int phase); //OK4104
  double SaturationPressureDependency(long number,double*gp,double theta); //OK4104
	double SaturationCapillaryPressureFunction(const double capillary_pressure, const int phase); //WW
  double SaturationPressureDependency(const double saturation,double theta); // WW
  double PermeabilitySaturationFunction(const double Saturation, int phase); //WW
	double PorosityVolumetricChemicalReaction(long);  //MX 1/2005
	double Porosity(long number,double*gp,double theta);//CMCD 9/2004 GeoSys 4
    void SetDistributedELEProperties(string); //OK
    void WriteTecplotDistributedProperties(); //OK
  //-------------------------------------------
  // Properties
    // PCS
    string pcs_type_name;   //YD
  CRFProcess*m_pcs; //OK
  vector<string>pcs_name_vector;
	vector<string>porosity_pcs_name_vector;
  //....................................................................
  //GEO
  string geo_type_name;
  string geo_name;
  vector<string>geo_name_vector; //OK
  int geo_dimension;
	double geo_area;
    string geo_area_file; //OK
    CFEMesh* m_msh; //OK
    //....................................................................
	double density;
  string name;
  int number;
  int porosity_model; // porosity
	int porosity_curve;
	double porosity_model_values[15];
  double porosity;
  string porosity_file; //OK/MB
	int tortuosity_model;
	double tortuosity_model_values[10];
	double tortuosity;
	int flowlinearity_model;
	double flowlinearity_model_values[10];
	int storage_model; // storativity
  double storage_model_values[10];
  double storage;
  int conductivity_model;
  double conductivity;
  int unconfined_flow_group;
  int permeability_model; // permeability
  double permeability;
  double permeability_tensor[9];
  string permeability_tensor_type_name;
  int permeability_tensor_type;
	int permeability_pressure_model;
	double permeability_pressure_model_values[10];
	double permeability_pressure_rel;
  int permeability_saturation_model[3];
  double permeability_saturation;
	string permeability_file; //SB //OK/MB string permeability_dis_type_file; 
  int capillary_pressure_model;
  double capillary_pressure;
	int permeability_porosity_model;
	double permeability_porosity_model_values[10];
  double storativity;
  double saturation_res[3]; // saturation max 3 phases
  double saturation_max[3];
  double saturation_eff[3];
  double saturation_exp[3];
  double heat_capacity; // thermal properties
	int mass_dispersion_model;
	double mass_dispersion_longitudinal;
	double mass_dispersion_transverse;
	int heat_dispersion_model;
	double heat_dispersion_longitudinal;
	double heat_dispersion_transverse;
  double heat_conductivity_tensor[9];
  int fct_number; // functions
  double permeability_saturation_model_values[2];
  double capillary_pressure_model_values[1];
	int heat_diffusion_model;
  //aux
  int m_color[3];
  bool selected;
  int mode;
	string het_file_name; //SB
  // surface water
  double friction_coefficient;  // MB
  double friction_model;
  double channel_width;
  double argument; //OK
  //Dual Richards transfer coefficient  YD
  double transfer_coefficient;
  double unsaturated_hydraulic_conductivity;
  double specific_storage;
};

 class CMediumPropertiesGroup  //YD
{
  public:
    CMediumPropertiesGroup() {OrigSize=0;} 
    void Set(CRFProcess* m_pcs);
    string pcs_name;
    string pcs_type_name; 
    CFEMesh* m_msh;
    vector<CMediumProperties*>mmp_group_vector; 
  private:
    int OrigSize;  // For excavation simulation. 
}; 

extern CMediumPropertiesGroup* MMPGetGroup(string pcs_type_name); //YD
extern list<CMediumPropertiesGroup*>mmp_group_list;   //YD
extern void MMPGroupDelete(string pcs_type_name);

extern vector<CMediumProperties*>mmp_vector;
extern void MATLoadDB(string);
extern list<string> keywd_list; //keyword-referenzliste "kw"
extern void comp_keywd_list(string);
extern void read_keywd_list(void);
extern list<string>mat_name_list;

extern void MMPWrite(string);
extern bool MMPRead(string);
extern void MMPWriteTecplot(string);
extern void MMPDelete();
extern CMediumProperties* MMPGet(string);
extern void MMP2PCSRelation(CRFProcess*);
extern void GetHeterogeneousFields(); //SB
extern long GetNearestHetVal2(long EleIndex, CFEMesh *m_msh, vector <double> xvals,  vector <double> yvals,  vector <double> zvals,  vector <double> mmpvals);
double GetAverageHetVal2(long EleIndex, CFEMesh *m_msh, vector <double> xvals,  vector <double> yvals,  vector <double> zvals,  vector <double> mmpvals);
#define MMP_FILE_EXTENSION ".mmp"

#endif
