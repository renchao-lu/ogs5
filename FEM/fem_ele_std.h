/*
   Class element declaration
   class for finite element.
   Designed and programmed by WW/OK, 12/2004
*/

#ifndef fem_std_INC
#define fem_std_INC
#include "fem_ele.h"
// Problems
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "rf_msp_new.h"
#include "rf_out_new.h" //OK
#include "par_ddc.h" //OK

//-----------------------------------------------------
     // Process type
     //L: Liquid flow
     //U: Unconfined flow
     //G: Groundwater flow
     //T: Two-phase flow
     //C: Componental flow
     //H: heat transport
     //M: Mass transport
     //O: Overland flow
     //R: Richards flow
//F: Fluid momentum
//A: Gas flow
enum ProcessType { L, U, G, T, C, H, M, O, R, F, A };
//-----------------------------------------------------

namespace process {class CRFProcessDeformation;}
using SolidProp::CSolidProperties;

class CRFProcess;
namespace FiniteElement{
  using Math_Group::SymMatrix; 
  using Math_Group::Matrix;
  using Math_Group::Vec;
  using process::CRFProcessDeformation;
  using ::CRFProcess;
class CFiniteElementStd:public CElement
{
   public:  
     CFiniteElementStd(CRFProcess *Pcs, const int C_Sys_Flad, const int order=1);
     ~CFiniteElementStd();

     // Set material data
     void SetMaterial(const int phase=0);
     // Set memory for local matrices
     void SetMemory();

     // Set coupling information
     void ConfigureCoupling(CRFProcess* pcs, const int *Shift, bool dyn=false);

     // Element claculation
     // 1. Mass matrix
     void CalcMass();
     // 2. Lumped mass matrix
     void CalcLumpedMass();
     // 3. Laplace matrix
     void CalcLaplace();
     // 4. Gravity term
     void CalcGravity(); 
     // 5. Strain coupling matrix
     void CalcStrainCoupling(); 
     // 6. Thermal coupling
     void CalcRHS_by_ThermalDiffusion();
	   // 7. Advection matrix
	   void CalcAdvection();
	   // 8. Storage matrix
	   void CalcStorage();
	 // 9. Content matrix
	 void CalcContent();
     // Assembly
     void Assembly(); 
	 void Assembly(int dimension);	// PCH for Fluid Momentum
     void Cal_Velocity();

     // MB only temporary
     void Get_Matrix_Quad();
     void CalcNLTERMS(int nn, double* H, double* HaaOld, double power, double* swval, double* swold, double* krwval);
     void CalcCKWR(int nn, double* haa, double* z, double power, double* test, int* iups);
     CRFProcess *pcs;
     CSolidProperties *SolidProp;
     CFluidProperties *FluidProp;
     CFluidProperties *GasProp;
     CMediumProperties *MediaProp;
  private:  
     long index;
	 // Column index in the node value table
     int idx0, idx1, idxS, idx3; 
     int idx_pd;   //YD
     int phase; 
     int comp; // Component
	   int LocalShift; // For RHS
     // Danymic
     int idx_vel_disp[3], idx_pres;
     // Material properties
     double mat[9];
     bool heat_phase_change;
     process::CRFProcessDeformation *dm_pcs;
     ::CRFProcess *cpl_pcs; // Pointer to coupled process. WW  
     bool dynamic; 

     //-------------------------------------------------------
     // Auxillarary matrices
     Matrix *StiffMatrix;
     Matrix *AuxMatrix;
     Matrix *AuxMatrix1;
     // Gravity matrix;
     SymMatrix *GravityMatrix;

     // Auxillarary vectors for node values
     // Vector of local node values, e.g. pressure, temperature.
     // Assume maximium element nodes is 20
     double OldMatrix[64]; // For grid adapting     
  public: //OK
     double NodalVal0[20]; //?? NodalValueSaturation, NodalValueTemperature; ...
     double NodalVal[20];     
     double NodalVal1[20];
     double NodalVal2[20]; 
     double NodalVal3[20]; 
     double NodalVal4[20]; 
     double NodalValC[20]; 
     double NodalValC1[20]; 
     double NodalVal_Sat[20]; 
     double NodalVal_P[20]; 
  private: //OK
     // Gauss point value. Buffers.
     double TG, PG, Sw;    
     double edlluse[16];
     double edttuse[16];

     // Local matrices
     SymMatrix *Mass;
     Matrix *Laplace;
     Matrix *Advection; //SB4200
     Matrix *Storage; //SB4200
	 Matrix *Content; //SB4209
     Matrix *StrainCoupling;
     Vec       *RHS;      
     //-------------------------------------------------------
     // Primary as water head
     bool HEAD_Flag; 
     inline double CalCoefMass();
     inline void CalCoefLaplace(bool Gravity, int ip=0);
     inline double CalCoefAdvection(); //SB4200 OK/CMCD
     inline double CalCoefStorage(); //SB4200
	 inline double CalCoefContent();
     inline double CalCoefStrainCouping();
     inline void CalNodalEnthalpy();
     //-----------------------------------------------------
     // Process type
     //L: Liquid flow
     //U: Unconfined flow
     //G: Groundwater flow
     //T: Two-phase flow
     //C: Componental flow
     //H: heat transport
     //M: Mass transport
     //O: Liquid flow
     //R: Richards flow
     //A: Gas flow
	 //F: Fluid Momentum
     ProcessType PcsType;
     //-----------------------------------------------------
     // Local Assembly
     // Assembly of parabolic equation
     void AssembleParabolicEquation(); //OK4104
     void AssembleMixedHyperbolicParabolicEquation(); //SB4200
     void AssembleParabolicEquationNewton();
     inline void Assemble_strainCPL(); // Assembly of strain coupling
	 void AssembleMassMatrix(); // PCH
     // Assembly of RHS by Darcy's gravity term
     void Assemble_Gravity();
	 void AssembleRHS(int dimension); // PCH
     void Assemble_Transfer();
     bool check_matrices; //OK4104
   public:
     void AssembleParabolicEquationRHSVector(); //OK
     CPARDomain *m_dom; //OK
};


// Vector for storing element values
class ElementValue
{
  public:
    ElementValue(CRFProcess* m_pcs, CElem* ele);
    ~ElementValue(); 
    void getIPvalue_vec(const int IP, double * vec);
    Matrix Velocity;
      void GetEleVelocity(double * vec);
  private:
    // Friend class 
    friend class ::CRFProcess;
    friend class FiniteElement::CFiniteElementStd;
    // Process
    CRFProcess *pcs;
    friend class ::COutput; //OK
}; 
} // end namespace

/*------------------------------------------------------------------
   Finite element calculation for standard PDE. 
   12.12.2004 WW
------------------------------------------------------------------*/
#endif
