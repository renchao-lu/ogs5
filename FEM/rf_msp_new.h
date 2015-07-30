/**************************************************************************
FEMLib - Object: MAT-SP
Task: class implementation
Programing:
08/2004 WW Implementation
last modified:
**************************************************************************/
#ifndef rf_msp_new_INC
#define rf_msp_new_INC

// C++ STL
//#include <fstream>
//#include <string>
//#include <vector>

#define MSP_FILE_EXTENSION ".msp"

namespace FiniteElement
{
class element;
class CFiniteElementVec;
class CFiniteElementStd;
class ElementValue_DM;
}

namespace Math_Group
{
class Matrix;
}
namespace process
{
class CRFProcessDeformation;
}

namespace SolidProp
{
class CSolidProperties
{
    public:
        //
        CSolidProperties();
        ~CSolidProperties();

        std::ios::pos_type Read(std::ifstream*);
        //CMCD
        FiniteElement::CFiniteElementStd *Fem_Ele_Std;
        std::string name;
        // IO
        std::string file_base_name;
        // Output
        void Write(std::fstream*);
        //CMCD
        void CalPrimaryVariable(std::vector<std::string>& pcs_name_vector);

        //-------------------------------------------------------------
        // Access to data
        //-------------------------------------------------------------
        // 1. Density
        double Density(double refence = 0.0);
        // 2. Thermal
        double Heat_Capacity(double refence = 0.0);
        // Boiling model
        double Heat_Capacity(double temperature, double porosity, double Sat);
        int GetCapacityModel() const
        {
            return Capacity_mode;
        }
        int GetConductModel() const
        {
            return Conductivity_mode;
        }
        bool CheckTemperature_in_PhaseChange(const double T0, const double T1);
        double Enthalpy(double temperature, const double latent_factor );
        double Heat_Conductivity(double refence = 0.0);
        void HeatConductivityTensor(const int dim, double* tensor, int group);

#ifdef RFW_FRACTURE
        double Youngs_Modulus(CElem* elem, double refence = 0.0);
        //RFW, for fracture calc
        double Get_Youngs_Min_Aperture(CElem *elem);
#else
        double Youngs_Modulus(double refence = 0.0);
#endif
        void SetYoungsModulus(const double El)
        {
            (*data_Youngs)(0)=El;
        }
        double Poisson_Ratio() const
        {
            return PoissonRatio;
        }
        void CalcYoungs_SVV(const double strain_v);
        double Thermal_Expansion() const
        {
            return ThermalExpansion;
        }
        // 4. Plasticity
        int Plastictity() const
        {
            return Plasticity_type;
        }
        double GetPlasticParameter(const int index)
        {
            return (*data_Plasticity)(index);
        }
        // 5. Creep
        int CreepModel() const
        {
            return Creep_mode;
        }
        double GetCreepParameter(const int index)
        {
            return (*data_Creep)(index);
        }
        // Initilize density
        void NullDensity();

        //-------------------------------------------------------------
        // Manipulators of data
        //-------------------------------------------------------------
        // 1. Elasticity
        void Calculate_Lame_Constant();
        // For thermal elastic model
        void ElasticConsitutive(const int Dimension, Math_Group::Matrix* D_e) const;
        // For transverse isotropic linear elasticity: UJG 24.11.2009
        void ElasticConstitutiveTransverseIsotropic(const int Dimension);
        Math_Group::Matrix* getD_tran() const
        {
            return D_tran;
        }
        void CalculateTransformMatrixFromNormalVector(const int Dimension);
        double E_Function(int dim, const FiniteElement::ElementValue_DM *ele_val, int ngp);//WX:06.2012
        // 2. Plasticity
        // 2.1 Drucker-Prager
        double GetAngleCoefficent_DP(const double Angle);
        double GetYieldCoefficent_DP(const double Angle);
        void CalulateCoefficent_DP();
        bool StressIntegrationDP(const int GPiGPj, const FiniteElement::ElementValue_DM* ele_val,
                                 double* TryStress, double& dPhi, const int Update);
        void ConsistentTangentialDP(Math_Group::Matrix* Dep, const double dPhi, const int Dim);
        bool DirectStressIntegrationDP(const int GPiGPj,
                                       const FiniteElement::ElementValue_DM* ele_val,
                                       double* TryStress,
                                       const int Update);
        int DirectStressIntegrationDPwithTension(const int GPiGPj,
                Math_Group::Matrix* De,
                const FiniteElement::ElementValue_DM* ele_val,
                double* TryStress,
                const int Update,
                double &mm);                            //WX
        void TangentialDP(Math_Group::Matrix* Dep);
        void TangentialDP2(Math_Group::Matrix* Dep); //WX
        void TangentialDPwithTension(Math_Group::Matrix* Dep, double mm); //WX
        void TangentialDPwithTensionCorner(Math_Group::Matrix* Dep, double mm); //WX
        // 2.2 Single yield surface model
        void dF_dNStress(double* dFdS, const double* DevS, const double* S_Invariants,
                         const double* MatN1, const int LengthStrs);
        void dF_dStress(double* dFdS, const double* RotV, const double* S_Invariants,
                        const double* MatN1, const int LengthStrs);
        void dF_dMat(double* dFdM, const double* S_Invariants, const double* MatN1);
        void dG_dNStress(double* dGdS, const double* DevS, const double* S_Invariants,
                         const double* MatN1, const int LengthStrs);
        void dG__dNStress_dNStress(const double* DevS, const double* S_Invariants,
                                   const double* MatN1, const int LengthStrs );
        void dG__dStress_dStress(const double* DevS, const double* RotV,
                                 const double* S_Invariants, const double* MatN1,
                                 const int LengthStrs);
        void dG_dSTress_dMat(const double* DevS, const double* S_Invariants,
                             const double* MatN1, const int LengthStrs);
        void dfun2(const double* DevS, const double* RotV, const double* S_Invariants,
                   const double* MatN1, const int LengthStrs);

        int CalStress_and_TangentialMatrix_SYS(const int GPiGPj, const FiniteElement::ElementValue_DM* ele_val,
                                               const Math_Group::Matrix* De,  Math_Group::Matrix* D_ep, double* dStress,
                                               const int Update);
        // 2.2 Cam-clay model
        void CalStress_and_TangentialMatrix_CC(const int GPiGPj,
                                               const FiniteElement::ElementValue_DM* ele_val,
                                               double* dStrain,
                                               Math_Group::Matrix* Dep,
                                               const int Update);
        // Substep integration. 16.05.2008 WW
        void CalStress_and_TangentialMatrix_CC_SubStep(const int GPiGPj,
                const FiniteElement::ElementValue_DM* ele_val,
                double* dStrain,
                Math_Group::Matrix* Dep,
                const int Update);
        // Parameter function for thermal elatic model. Last modifed on 15.03.2008 //WW
        double TEPSwellingParameter(const double mean_stress);
        void TEPSwellingParameter_kis(const double suction);
        // Strain inrement by creep
        void AddStain_by_Creep(const int ns,
                               double* stress_n,
                               double* dstrain,
                               double temperature = 0.0);
        void AddStain_by_HL_ODS(const FiniteElement::ElementValue_DM* ele_val,
                                double* stress_n,
                                double* dstrain,
                                double temperature = 30);
        void CleanTrBuffer_HL_ODS();
        void AccumulateEtr_HL_ODS(const FiniteElement::ElementValue_DM* ele_val, const int nGS);

        void CalculateCoefficent_MOHR(double ep, double scalar_comp, double scalar_tens);
        void CalPrinStrs(double* stresses, double* prin_stresses, int Size);
        void CalPrinDir(double* prin_str, double* stress, double* v, int Size);
        void CalPrinStrDir(double* stress, double* prin_str, double* prin_dir, int Dim);
        void CalTransMatrixA(double* v, Math_Group::Matrix* A, int Size);
        int DirectStressIntegrationMOHR(const int GPiGPj, FiniteElement::ElementValue_DM *ele_val,
                                        double *TryStress, const int Update, Math_Group::Matrix *Dep, int itesteps );
        int StressIntegrationMOHR_Aniso(const int GPiGPj, const FiniteElement::ElementValue_DM *ele_val,
                                        double *TryStress, const int Update, Math_Group::Matrix *Dep);//WX:12.2011 aniso mohr
        double CalAnisoPara(double *Stress, double *MicroStruTensor);//WX:12.2011 cal. aniso parameter
        int MohrCheckFailure(double* NormStr, int &failurestate, int Size);
        void TangentialMohrShear(Math_Group::Matrix* Dep);
        void TangentialMohrTension(Math_Group::Matrix* Dep);
        void Cal_Inv_Matrix(int Size, Math_Group::Matrix* MatrixA, Math_Group::Matrix* xx);
        double CalVarP(double* vec1, double* vec2, double* sigma_B, double* sigma_l);
        double CalVar_t(double* vecl,
                        double* veclg,
                        Math_Group::Matrix* D,
                        double* sigma_B,
                        double* sigma_l,
                        int Size);
        void CalDep_l(double* vecl, double* veclg, Math_Group::Matrix* D, Math_Group::Matrix* Dep_l, double fkt);
        void VecCrossProduct(double* vec1, double* vec2, double* result_vec);

        bool GetBoolExcavated()
        {
            return excavated;   //WX:01.2013
        }
        //Solid Chemical Reactive System
        void SetSolidReactiveSystemProperties();

        //Set value for solid reactive system - TN
        void setSolidReactiveSystem (FiniteElement::SolidReactiveSystem reactive_system);

    private:
        // Material parameters
        double PoissonRatio;
        int Youngs_mode;
        int excavation;                          //12.2009. WW
        bool excavated;                       //12.2009. To be ..... WW
        Math_Group::Matrix* data_Youngs;
        double ThermalExpansion;
        //
        double s_tol;                         //16.06.2008 WW
        double f_tol;                         //16.06.2008 WW
        double biot_const;
        int bishop_model;                     //05.2011 WX
        double bishop_model_value;            //05.2011 WX
        double threshold_dev_str;             //12.2012 WX
        double grav_const;                    //WW
        Math_Group::Matrix* data_Density;
        //
        Math_Group::Matrix* data_Capacity;
        Math_Group::Matrix* data_Conductivity;
        //
        Math_Group::Matrix* data_Plasticity;
        Math_Group::Matrix* data_Creep;
        //
        int Density_mode;
        //
        int Capacity_mode;
        int Conductivity_mode;
        double T_0;
        int Plasticity_type;
        double primary_variable[10];             //CMCD
        double primary_variable_t0[10];          //CMCD
        double primary_variable_t1[10];          //CMCD
        // Creep property
        // 1. Stationary Norton model
        int Creep_mode;
        //
        bool axisymmetry;

        int mode;                                //CMCD
        // Swelling pressure
        int SwellingPressureType;
        double Max_SwellingPressure;
        //
        std::string CurveVariable_Conductivity;
        int CurveVariableType_Conductivity;
        // Secondary data
        // Elasticity
        double E;                                // Youngs moduls calculated from data_Youngs
        double Lambda;
        double G;                             // Shear stress modulus
        double K;                             // Bulk modulus
        double Ks;                            //WX solid Bulk modulus
        int E_Function_Model;                 //WX:06.2012. E depends on stress
        double E_Function_Model_Value[3];     //WX:06.2012
        int Time_Dependent_E_nv_mode;//WX:01.2013. E nv is changed with time
        int Time_Dependent_E_nv_value[5];//WX:01.2013

        // Rotation matrices and their transpose: UJG 25.11.2009
        Math_Group::Matrix* Crotm;                        // If this is needed by permaebility calculation, we keep it. Otherwise remove it. (To do, UJG/WW)
        Math_Group::Matrix* D_tran;

        // Plasticity
        // 1. Drucker-Prager
        double Al;
        double Xi;
        double Y0;
        double BetaN;
        double Hard;
        double Hard_Loc;
        double tension;     //WX:08.2010 Tension strength

        //4. Mohr-Coulomb	//WX: 11.2010. Mohr-Coulomb model
        double Ntheta;
        double Nphi;
        double csn;

        // Plasticity
        double dl2;
        // 2. Single yield surface
        Math_Group::Matrix *d2G_dSdS;
        Math_Group::Matrix *d2G_dSdM;
        Math_Group::Matrix *LocalJacobi;                     // To store local Jacobi matrix
        Math_Group::Matrix *inv_Jac;                         // To store the inverse of the  Jacobi matrix
        Math_Group::Matrix *sumA_Matrix;
        double *rhs_l;                           // To store local unknowns of 15
        double *x_l;                             // To store local unknowns of 15
        int  *Li;
        void AllocateMemoryforSYS();
        void ResizeMatricesSYS(const int Dim);

        // Direct stress integration for Drucker-Prager
        double *devS;
        double *dFds;
        double *dGds;
        double *D_dFds;
        double *D_dGds;
        double *dFtds;                           //WX: 08.2010
        double *dGtds;                           //WX: 08.2010
        //Direct stress integration for Mohr-Coulomb.	WX:08.11.2010.
        Math_Group::Matrix *TransMatrixA;
        Math_Group::Matrix *TransMatrixA_T;
        Math_Group::Matrix *Inv_TransA;
        Math_Group::Matrix *Inv_TransA_T;
        Math_Group::Matrix *TmpDe;
        Math_Group::Matrix *Inv_De;
        Math_Group::Matrix *TMatrix;
        Math_Group::Matrix *TmpMatrix;
        Math_Group::Matrix *TmpMatrix2;
        Math_Group::Matrix *Dep_l;
        Math_Group::Matrix *dDep_l;
        Math_Group::Matrix *dGds_dFds;
        Math_Group::Matrix *ConstitutiveMatrix;
        Math_Group::Matrix *TransMicroStru;//WX: 11.2011 micro structure tensor transform matrix
        Math_Group::Matrix *TransMicroStru_T;
        Math_Group::Matrix *TransMicroStru_TInv;
        double *MicroStruTensor;//WX: 11.2011
        double *comp_para;//WX:12.2011
        double *tens_para;

        //WX:09.2011
        double *Bedding_Norm;//Norm vector of bedding plane, used for anisotropic elasto-plasticity
        int bedding_fric_curve;
        int bedding_uc_curve;
        int bedding_tens_curve;
        int bedding_uc_curve_order;
        int bedding_tens_curve_order;
        bool Plasticity_Bedding;
        void CalTransMatrixMicroStru(Math_Group::Matrix *A, double *v);

        //5. Hoek-Brown WX
        double HoekB_a;
        double HoekB_s;
        double HoekB_mb;
        double HoekB_sigci;
        double HoekB_tens;
        double HoekB_cohe;
        void CalculateCoefficent_HOEKBROWN();
        int StressIntegrationHoekBrown(const int GPiGPj, const FiniteElement::ElementValue_DM* ele_val,
                                       double* TryStress, const int Update, Math_Group::Matrix* Dep );
        void TangentialHoekBrown(Math_Group::Matrix* Dep);

        //WW
        std::vector<std::string>  capacity_pcs_name_vector;
        //WW
        std::vector<std::string>  conductivity_pcs_name_vector;

        // Mini linear solver
        void Gauss_Elimination(const int DimE, Math_Group::Matrix& AA, int* L,  double* xx);
        void Gauss_Back(const int DimE, Math_Group::Matrix& AA, double* rhs, int* L, double* xx);
        // Thermal properties
        int thermal_conductivity_tensor_type;
        int thermal_conductivity_tensor_dim;
        double thermal_conductivity_tensor[9];
        std::string thermal_conductivity_tensor_type_name;
        // Handles. May be used by GUI
        std::string solid_name;

        //Solid reactive system properties - TN
        std::string reaction_system;
        double lower_solid_density_limit;
        double upper_solid_density_limit;
        double reaction_enthalpy;
        double reaction_entropy;
        double non_reactive_solid_volume_fraction;
        double non_reactive_solid_density;

        double specific_heat_source;

        //-------------------------------------------------------------
        // Numeric
        double CalulateValue(const Math_Group::Matrix *data, const double x) const;
        double Kronecker(const int ii, const int jj);

        //Get solid reactive system - TN
        FiniteElement::SolidReactiveSystem getSolidReactiveSystem () const;

        // Friends that can access to this data explicitly
        friend bool MSPRead(std::string file_base_name);
        friend void MSPWrite(std::string);

        FiniteElement::SolidReactiveSystem _reactive_system;

        friend class FiniteElement::CFiniteElementVec;
        friend class FiniteElement::CFiniteElementStd;
        friend class FiniteElement::ElementValue;
        friend class FiniteElement::ElementValue_DM;
        friend class process::CRFProcessDeformation;
        friend class CMediumProperties;
        friend class ::CRFProcess;

};

}                                                 // end namespace
extern std::vector<SolidProp::CSolidProperties*> msp_vector;
extern bool MSPRead(std::string file_base_name);
extern void MSPWrite(std::string);
extern void MSPDelete();
//OK
extern std::vector<std::string> msp_key_word_vector;
extern void MSPStandardKeywords();                //OK
//OK
extern SolidProp::CSolidProperties* MSPGet(std::string);

extern double StressNorm(const double *s, const int Dim);
extern double TensorMutiplication2(const double *s1, const double *s2, const int Dim);
extern double TensorMutiplication3(const double *s1, const double *s2, const double *s3, const int Dim);
extern double DeviatoricStress(double *Stress);
#endif
