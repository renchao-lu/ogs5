/**************************************************************************
FEMLib - Object: MAT-FP
Task: class implementation
Programing:
08/2004 OK Implementation
last modified:
**************************************************************************/
#ifndef rf_mfp_new_INC
#define rf_mfp_new_INC
// C++ STL
using namespace std;
// GEOLib
// FEMLib
/*---------------------------------------------------------------*/
//namespace FiniteElement {class CElement;}
//using FiniteElement::CElement;
namespace FiniteElement {class CFiniteElementStd;}
using FiniteElement::CFiniteElementStd;
class CFluidProperties
{
  private:
    // State variables
    double primary_variable[10]; //WW
    double primary_variable_t0[10];//CMCD
    double primary_variable_t1[10];//CMCD
    // 
    bool cal_gravity; //YD/WW
    double molar_mass; 
    // FEM
    friend class  FiniteElement::CFiniteElementStd; //WW
    // PCS
    CRFProcess *m_pcs; //OK4704
  public:
    string name;
    int phase;
    // FEM
    CFiniteElementStd *Fem_Ele_Std;
    long node; //OK4704
    // Density
    int density_model;
    double rho_0;
    double drho_dp;
    double drho_dT;
    double drho_dC;
    string rho_fct_name;
    // Viscosity
    int viscosity_model;
    double viscosity;
    double my_0;
    double dmy_dp;
    double dmy_dT;
    double dmy_dC;
    string my_fct_name;
    // Thermal properties
    double specific_heat_capacity;
    string heat_capacity_fct_name;
    int heat_conductivity_model;
    double heat_conductivity;
    string heat_conductivity_fct_name;
    double temperature_buffer; //YD, shifted to public JOD
	int heat_capacity_model;  //YD, shifted to public JOD 
    // Electrical properties
    int electric_conductivity_model;
    int electric_conductivity_num_val;
    double *electric_conductivity_val; 
    // Chemical properties
	string dif_fct_name;
	int diffusion_model; /* SB:p2 */
	double diffusion; /*SB:2p */
    // State variables
    double p_0;
	double T_0;
	double C_0;
    double Z;
    double T_Latent1, T_Latent2, latent_heat; 
    int heat_phase_change_curve; 
    // IO
    string file_base_name;
    int mode;
    // PCS  YD
    vector<string>density_pcs_name_vector;
    vector<string>viscosity_pcs_name_vector;
    vector<string>specific_heat_capacity_pcs_name_vector;
    vector<string>heat_conductivity_pcs_name_vector;
    vector<string>enthalpy_pcs_name_vector;
    // DAT
    //string dat_type_name;
    //string dat_name;
    // FCT
    string fct_pcs_name;
    string fct_name;
    //
    CFluidProperties(void);
    ~CFluidProperties(void);
    ios::pos_type Read(ifstream*);
    void Write(ofstream*);
    void CalPrimaryVariable(vector<string>& pcs_name_vector);
    // Add an argument: double* variables = NULL. 28.05.2008 WW
    double Density(double *variables = NULL); 
    double Viscosity(double *variables = NULL); //OK4709
    double SpecificHeatCapacity();
	double PhaseChange(); // JOD 
    double HeatConductivity();
    double CalcEnthalpy(double temperature);
    double Enthalpy(int,double);
    double EnthalpyPhase(long,int,double*,double);
    double MassFraction(long number,int comp,double*gp,double theta, CFiniteElementStd* assem=NULL);
    double InternalEnergy(long number,double*gp,double theta);
    double DensityTemperatureDependence(long number,int comp,double*gp,double theta);
    double vaporDensity(const double T); //WW
    double vaporDensity_derivative(const double T); //WW
    bool CheckGravityCalculation() const {return cal_gravity;}
    int GetHeatCapacityModel() const {return heat_capacity_model;}//YD
  private:
    double GasViscosity_Reichenberg_1971(double,double);
	double MATCalcFluidDensityMethod8(double p, double T, double C);
    double LiquidViscosity_Yaws_1976(double);
    double LiquidViscosity_Marsily_1986(double);
    double LiquidViscosity_NN(double,double);
    double LiquidViscosity_CMCD(double p,double T,double C);
	double MATCalcHeatConductivityMethod2(double p, double T, double C);
	double MATCalcFluidHeatCapacityMethod2(double p, double T, double C);
};

extern vector<CFluidProperties*>mfp_vector;
extern bool MFPRead(string);
extern void MFPWrite(string);
#define MFP_FILE_EXTENSION ".mfp"
extern double MFPCalcVapourPressure(double);
extern double MFPCalcFluidsHeatCapacity(CFiniteElementStd* assem=NULL); //WW
extern double MFPCalcFluidsHeatConductivity(long index,double*gp,double theta, CFiniteElementStd* assem=NULL);
extern void MFPDelete();
extern CFluidProperties* MFPGet(string);    //OK/YD
double MFPGetNodeValue(long,string); //OK
#endif
