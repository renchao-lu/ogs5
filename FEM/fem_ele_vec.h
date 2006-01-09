/*
   Class element declaration
   class for finite element.
   Designed and programmed by Wenqing Wang, 06/2004
*/
#ifndef fem_dm_INC
#define fem_dm_INC
#include "fem_ele.h"
#include "matrix_class.h"
// Material properties
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"

namespace SolidProp {class CSolidProperties;} 
namespace process{class CRFProcessDeformation;}
namespace Mesh_Group{class CElem;}
namespace FiniteElement{

    using SolidProp::CSolidProperties;
    using Math_Group::Matrix;
    using Math_Group::SymMatrix;
    using Math_Group::Vec;
    using process::CRFProcessDeformation;
	using Mesh_Group::CElem; 

// Vector for storing element values
class ElementValue_DM
{
  public:
      ElementValue_DM(CElem* ele);
      ~ElementValue_DM(); 
  private:
    // Friend class 
    friend class SolidProp::CSolidProperties;
    friend class process::CRFProcessDeformation;
    friend class CFiniteElementVec;

    Matrix *Stress0; // Initial stress
    Matrix *Stress;
    Matrix *pStrain;
    // Preconsolidation pressure
    Matrix *prep0;               
    Matrix *e_i;    // Void ratio        
    // Variables of single yield surface model   
    Matrix *xi;     // Rotational hardening variables     
    Matrix *MatP;   // Material parameters 

    // Discontinuity
	double disp_j;
	double tract_j;
    bool Localized;
    Matrix *NodesOnPath;
    double *orientation;    

}; 

// Derived element for deformation caculation
class CFiniteElementVec:public CElement 
{
  public:
     CFiniteElementVec (process::CRFProcessDeformation *dm_pcs, const int C_Sys_Flad, const int order=2); 
     ~CFiniteElementVec ();
	 
     // Set memory for local matrices
     void SetMemory();
	 
	 // Compute the local finite element matrices and vectors    
     void LocalAssembly(const long elementIndex, const int update);
	 // Assemble local matrics and vectors to the global system    
     bool GlobalAssembly();

	 // Compute strains     
     void ComputeStrain();

     // Set material data
     void SetMaterial();

	 // Get strain
	 double *GetStrain() const {return dstrain; }

     //----------- Enhanced element -----------------------
     // Geometry related
     bool LocalAssembly_CheckLocalization(const long index);
     bool LocalAssembly_TraceDiscontinuity(const long index);
     int IntersectionPoint(const int O_edge, 
                           const double *NodeA, double *NodeB);
     //----------- End of enhanced element ----------------

  private:  
   
     process::CRFProcessDeformation *pcs;
     
	 int ns;  // Number of stresses components
     double Radius; // For axisymmetrical problems
    
     // Flow coupling
     int Flow_Type;

     // Primary value indeces
     // Column index in the node value table
     int idx_P, idx_P0, idx_P1, idx_P1_0, idx_P2;
	 int idx_T0,idx_T1;
     int idx_S0, idx_S; 
     int idx_pls;  
     // Displacement column indeces in the node value table
     int Idx_Stress[6];
     int Idx_Strain[6];
  
     // B matrix
     Matrix *B_matrix;
     Matrix *B_matrix_T;

	 //------ Material -------
     CSolidProperties* smat;
     CFluidProperties *m_mfp; // Fluid coupling
     // Medium property
     CMediumProperties *m_mmp; // Fluid coupling
     double CalDensity();
 

    // Elastic constitutive matrix
	 Matrix *De;
	 // Consistent tangential matrix
	 Matrix *ConsistDep;
	 
     // Local matricies and vectors 
	 Matrix *AuxMatrix;
	 Matrix *Stiffness;
     Matrix *PressureC;
     SymMatrix *Mass; // For dynamic analysis
     Vec *RHS;

	 //  Stresses:
	 //  s11, s22, s33, s12, s13, s23
     double *dstress;
     //  Straines:
	 //  s11, s22, s33, s12, s13, s23
     double *dstrain;

     // Results, displacements
     //  u_x1, u_x2, u_x3, ..., u_xn,
     //  u_y1, u_y2, u_y3, ..., u_yn,
     //  u_z1, u_z2, u_z3, ..., u_zn
	 double *Disp;

	 // Temperatures of nodes
	 double *Temp;
     double S_Water; 


     //Element value
     ElementValue_DM *eleV_DM;

     //------ Enhanced element ------
	 // Jump flag of element nodes
     bool *NodesInJumpedA;
	 // Regular enhanced strain matrix 
     Matrix *Ge ;
	 // Singular enhanced strain matrix 
     Matrix *Pe;	 
	 // Additional node. Normally, the gravity center
	 double X0[3];
	 // Normal to the discontinuity surface
     double n_jump[3];
	 // principle stresses
     double pr_stress[3];
	 // Compute principle stresses
     double ComputePrincipleStresses(const double *Stresses);
	 // Compute principle stresses
     double ComputeJumpDirectionAngle(const double *Mat);
     //------ End of enhanced element ------



	 // For axisymmetrical problems   
     void ComputeRadius();

	 // Form B matric     
     void setB_Matrix(const int LocalIndex);
	 // Form the tanspose of B matric     
     void setTransB_Matrix(const int LocalIndex);
	 //
	 void ComputeMatrix_RHS(const double fkt, const Matrix *p_D);


	 // Temporarily used variables  
     // 1. For extropolating gauss value to node
     int GetLocalIndex(const int gp_r, const int gp_s, int gp_t);
     double *Sxx, *Syy, *Szz, *Sxy, *Sxz, *Syz, *pstr;
	 double *tolStrain;
     // 2. For enhanced strain approach
     Matrix *BDG, *PDB, *DtD, *PeDe ; // For enhanced strain element

     /// Extropolation
	 bool RecordGuassStrain(const int gp, const int gp_r, 
		                    const int gp_s, int gp_t);
     void ExtropolateGuassStrain();
     void ExtropolateGuassStress(const int index);


   	 // Compute the local finite element matrices     
     void LocalAssembly_continuum(const long elementIndex, const int update);
	 void LocalAssembly_EnhancedStrain(const long index, const int update);

     // Assembly local stiffness matrix
     void GlobalAssembly_Stiffness();
     void GlobalAssembly_RHS();

     //----------- Enhanced element ----------------
     void CheckNodesInJumpedDomain();
	 // Compute the regular enhanced strain matrix 
     void ComputeRESM(const double *tangJump = NULL);
	 // Compute the singular enhanced strain matrix 
     void ComputeSESM(const double *tangJump = NULL);

     friend class process::CRFProcessDeformation;

     // Auxillarary vector
     double AuxNodal[8];
     double AuxNodal_S0[8];
     double AuxNodal_S[8];
     double AuxNodal1[60];

     // Dynamic
     // Damping parameters
     bool dynamic;
     int Idx_Vel[3];
     double beta2, bbeta1;
     // Auxillarary vector
     Vec *dAcceleration; 
     void ComputeMass();
};


} // end namespace
extern std::vector<FiniteElement::ElementValue_DM*> ele_value_dm;

#endif

