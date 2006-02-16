/**************************************************************************
 ROCKFLOW - Modul: cel_mpc.c
 Task:
 Finite element matrices for RF-Kernel MPC

 Programming:
 11/2001 OK/JdJ  Implementation
 03/2003 RK  model substituted by process control parameters
 04/2003 JDJ MPCCalcCharacteristicNumbers
 04/2003 JDJ MPCCalculated ElementMatrix1D, dT/dt & grad T
 05/2003 OK  Access functions for element matrices
 05/2003 OK  MPCCreateELEMatricesPointer, MPCDestroyELEMatricesPointer
 06/2003 OK  Strain coupling matrices for quadrilaterals
 04/2004 JdJ 3-D Hexahedral matrices
 08/2004 OK  MFP implementation
 08/2004 OK  MMP implementation start
 11/2004 OK  NUM
**************************************************************************/
#include "stdafx.h"                    /* MFC */
// C++
#include <iostream>
/* Defines */
#define noTEST_CEL_MPC
#define CHARACTERISTIC_NUMBERS
/* Import */
#include "makros.h"
#include "rf_pcs.h" //OK_MOD"
 /* Objects */
#include "rf_num_new.h"
 /* Tools */
#include "mathlib.h"
#include "adaptiv.h"
 /* Kernel */
#include "int_mmp.h"
#include "cel_mpc.h"
 /* PCS */
#include "rf_vel_new.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "fem_ele_std.h"
using FiniteElement::CElement;
extern double gravity_constant;
/* Declarations */
int InitializeMatrixtype_MMP(char *name, int phase, int component);
int GetMatrixtype_MMP(char *name);
/* Kernel parameter */
double mpc_time_collocation_global = 1.;
int mpc_mass_lumping_method = 1;
double mpc_density;
double mpc_porosity;
double mpc_saturation;
double mpc_mass_fraction;
double mpc_diffusion_coefficient;
double mpc_permeability_ref;
double mpc_permeability_rel;
double mpc_viscosity;
double mpc_drho_dp;
double mpc_pressure;
double mpc_temperature;
double mpc_drho_dT;
// Calc element matrices
void MPCCalculateElementMatrix(long index, int phase, int comp, int matrixtyp,CRFProcess*);
void MPCCalcElementMatrix1D(long index, int phase, int comp, int matrixtyp,CRFProcess*);
void MPCCalcElementMatrix2DTriangle(long index, int phase, int comp, int matrixtyp,CRFProcess*);
void MPCCalcElementMatrix2D(long index,int phase,int comp,int matrixtyp,CRFProcess*);
void MPCCalcElementMatrix3DHexahedron(long index,int phase,int comp,int matrixtyp,CRFProcess*);
double MPCCalcCompressibilityNumber(long index,int phase,int comp,double *gp);
double MPCCalcNeumannNumber(long index,int phase,int comp,double *gp);
double MPCCalcStorativityNumber(long index,int phase,int comp,double *gp);
double MPCCalcDiffusivityNumber(long index,int phase,int comp,double *gp);
double MPCCalcReferenceNumber(long index,int phase,int comp,double *gp);
/* Element values */
VoidFuncLIID MPCSetElementCompressibilityNumber;
VoidFuncLIID MPCSetElementConductivityNumber;
VoidFuncLIID MPCSetElementDiffusivityNumber;

extern double MATGetComponentDiffusionCoefficient(long index,int phase,int comp);
static int mpc_pcs_number;

/*************************************************************************
  ROCKFLOW - Function: MPCCalculateElementMatrices
  Task: Calc element matrices
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void MPCCalculateElementMatrices(CRFProcess *m_pcs)
{
  /* Variablen */
  static long element;
  int phase,type; 
  int mpc_matrix_types=6;
  int mpc_component_this=-1;

#ifdef PCS_OBJECTS
  //mpc_component_this = PCSGetProcessNumber();
  mpc_component_this = m_pcs->pcs_number;
#endif 

  for (phase=0;phase<GetRFProcessNumPhases();phase++)
    for (type=0;type<mpc_matrix_types;type++)
      for (element=0;element<ElListSize();element++)
        if (ElGetElement(element)!=NULL)  /* Element existiert */
          if (ElGetElementActiveState(element)) /* aktives Element */
            MPCCalculateElementMatrix(element,phase,mpc_component_this,type,m_pcs);
}

/*
void MPCCalculateElementMatricesRichards(void)
{
  static long element;
  int phase,type; 
  int mpc_matrix_types=6;
  int mpc_component_this=-1;
  mpc_component_this = 1;
  for (phase=0;phase<GetRFProcessNumPhases();phase++)
    for (type=0;type<mpc_matrix_types;type++)
      for (element=0;element<ElListSize();element++)
        if (ElGetElement(element)!=NULL)
          if (ElGetElementActiveState(element))
            MPCCalculateElementMatrix(element,phase,mpc_component_this,type);
}
*/


/**************************************************************************
 ROCKFLOW - Funktion: MPCCalculateElementMatrix

 Aufgabe:
   Berechnet die angeforderte Matrix fuer ein Element

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index     : Elementnummer
     int matrix     : Nummer des Matrixtyps
     int phase      : Nummer der Phase

 Ergebnis:
   - void -


**************************************************************************/
void MPCCalculateElementMatrix(long index, int phase, int comp, int matrixtyp,CRFProcess*m_pcs)
{
#ifdef TEST_CEL_MPC
  DisplayMsgLn(""); 
  DisplayMsg("Element: ");    DisplayLong(index);
  DisplayMsg(" Phase: ");     DisplayLong(phase); 
  DisplayMsg(" Component: "); DisplayLong(comp); 
  DisplayMsg(" Type: ");      DisplayLong(matrixtyp);
#endif
  switch (ElGetElementType(index))
    {
    case 1:
      MPCCalcElementMatrix1D(index,phase,comp,matrixtyp,m_pcs);
      break;
    case 2:
      MPCCalcElementMatrix2D(index,phase,comp,matrixtyp,m_pcs);
      break;
    case 3:
      MPCCalcElementMatrix3DHexahedron(index,phase,comp,matrixtyp,m_pcs);
      break;
    case 4: /* 2-D linear triangle */
      MPCCalcElementMatrix2DTriangle(index,phase,comp,matrixtyp,m_pcs);
      break;
    }
}
/**************************************************************************
 ROCKFLOW - Funktion: MPCCalcElementMatrix1D

 Task: 
   Calculation of finite element matrices of a 1-D linear element of kernel MPC
   - hydraulic conductance matrix
   - hydraulic capacitance matrix
   - capillary forces
   - gravity forces
   - vapour diffusion term

 Parameters: (I: Input; R: Return; X: Both)
   I long index:    element number
   I int phase:     fluid phase 
   I int comp:      component number
   I int matrixtyp: matrix type

 Return:
   - void -

 Programming:
 11/2001   OK/JdJ   First implementation
 01/2003   OK/JdJ   dT/dt term
 01/2003   OK/JdJ   grad T term
 11/2003 OK new storage of element matrices
 04/2004 WW/JDJ Bugfix in Mat Properties (gp)
 08/2004 OK MFP implementation
 08/2004 OK MMP implementation
 11/2004 OK NUM
**************************************************************************/
void MPCCalcElementMatrix1D(long index, int phase, int comp, int matrixtyp,CRFProcess*m_pcs)
{
  static double *invjac, detjac;
  static double A, L, g, n, fkt, r;
  static long *element_nodes;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *diffusion_matrix;
  static double *storativity_pressure_matrix;
  static double *storativity_temperature_matrix;
  static double *diffusion_thermo_comp_matrix;
  static int nn=2;
  static int nn2;
  static double gp[3];
  static int nidx0,nidx1;
  //----------------------------------------------------------------------
  /* Jacobian for coordinate transformations */
   /* detjac = AL/2, invjac = 2/(AL) */
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  r=-1.0; /* full upwind */
  r=0.0;
  gp[0]=r; gp[1]=0.0; gp[2]=0.0;
  nn2=nn*nn;
  g = gravity_constant;
  mpc_time_collocation_global = 1.;
m_pcs=m_pcs;
  //----------------------------------------------------------------------
  // State variables
  char function_name[15];
  sprintf(function_name,"SATURATION%i",phase+1);
  mpc_saturation = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,function_name));
  nidx0 = PCSGetNODValueIndex("TEMPERATURE1",0);
  nidx1 = PCSGetNODValueIndex("TEMPERATURE1",1);
  mpc_temperature = (1.-mpc_time_collocation_global)*InterpolValue(index,nidx0,gp[0],gp[1],gp[2]) \
                  + mpc_time_collocation_global*InterpolValue(index,nidx1,gp[0],gp[1],gp[2]);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group]; // MMPGetGroup(index);
  n = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  double* permeability;
  permeability = m_mmp->PermeabilityTensor(index);
  mpc_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
            //porosity-permeability function     //MX, 10.2004
  if (m_mmp->porosity_model>=3 && m_mmp->permeability_porosity_model==2){              //porosity-permeability function     //MX, 10.2004
	double mpc_permeability_rel_porosity = m_mmp->PermeabilityPorosityFunction(index,gp,mpc_time_collocation_global);
	if (mpc_permeability_rel_porosity > 0.0)
	mpc_permeability_rel *= mpc_permeability_rel_porosity;
  }
  mpc_permeability_ref = permeability[0];
  A = m_mmp->geo_area; 
  L = 2. * detjac / A;
  //----------------------------------------------------------------------
  // MFP fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  m_mfp->mode = 0;
  mpc_density               = m_mfp->Density();
  mpc_viscosity             = m_mfp->Viscosity();
  mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  mpc_drho_dp               = m_mfp->drho_dp;
  mpc_diffusion_coefficient = m_mfp->diffusion;
if((phase==0)&&(comp==1))
  mpc_drho_dT               = m_mfp->DensityTemperatureDependence(index,comp,gp,mpc_time_collocation_global);
  mpc_diffusion_coefficient = 1e-6;
  /*==========================================================================*/
  /* Calculation of element matrices */
  switch (matrixtyp)
    {
    /*------------------------------------------------------------------------*/
    case 0: /* K_component^phase - Conductivity matrix */
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      if (!conductance_matrix) {
        conductance_matrix = (double *) Malloc(nn2 * sizeof(double));
	  }
      /* Upwinding fuer Permeabilitaetsmatrizen */
       /* if ((mmp_upwind_method == 1) || (mmp_upwind_method == 2)) */
      /* Maximum Mobility Upwinding */
       /* if (mmp_upwind_method == 3) */
      /* fkt = rho^p*X_k^p * k*kr^p/mue^p */
      fkt = mpc_permeability_ref * mpc_permeability_rel * mpc_density * mpc_mass_fraction / mpc_viscosity;
#ifdef CHARACTERISTIC_NUMBERS
      fkt = MPCCalcNeumannNumber(index,phase,comp,gp);
#endif
      /* calculate: grad(phi)* fkt *grad(ome) */
      fkt *= A / L;
      conductance_matrix[0] = fkt;
      conductance_matrix[1] = -fkt;
      conductance_matrix[2] = -fkt;
      conductance_matrix[3] = fkt;
      /* store element matrix */
      MPCSetELEConductivityMatrix(index,phase,conductance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(conductance_matrix,nn,nn,"MPCCalculateElementMatrix1D: KP_k^p");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 1: /* C_component^phase - Capacitance matrix */
      capacitance_matrix = MPCGetELECapacitanceMatrix(index,phase);
	  if (!capacitance_matrix) {
        capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
	  }
      /* fkt = n*rho^phase*X_comp^phase */
      fkt = mpc_porosity * mpc_density * mpc_mass_fraction;
#ifdef CHARACTERISTIC_NUMBERS
      fkt = MPCCalcStorativityNumber(index,phase,comp,gp);
#endif
      /* N^T* fkt *N */
      fkt *= A * L / 6.;
      if (mpc_mass_lumping_method)
        {
          capacitance_matrix[0] = 3. * fkt;
          capacitance_matrix[1] = 0.;
          capacitance_matrix[2] = 0.;
          capacitance_matrix[3] = 3. * fkt;
        }
      else
        {
          capacitance_matrix[0] = 2. * fkt;
          capacitance_matrix[1] = 1. * fkt;
          capacitance_matrix[2] = 1. * fkt;
          capacitance_matrix[3] = 2. * fkt;
        }
      MPCSetELECapacitanceMatrix(index,phase,capacitance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(capacitance_matrix,nn,nn,"MPCCalculateElementMatrix1D: CS_k^p");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 2: /* g_comp^phase - gravity vector */
      gravity_vector = MPCGetELEGravityVector(index,phase);
	  if (!gravity_vector) {
        gravity_vector = (double *) Malloc(nn2*sizeof(double));
	  }
	  /* Fuer erstes Integral: grad(phi)*kr*k/mue*rho*g */
      fkt = mpc_permeability_ref \
          * mpc_permeability_rel \
          / mpc_viscosity \
          * mpc_density \
          * mpc_mass_fraction;
      fkt *= g \
          * ((GetNode(element_nodes[1])) -> z - (GetNode(element_nodes[0])) -> z);
      fkt *= A / L;
      gravity_vector[0] = -fkt;
      gravity_vector[1] = fkt;
      MPCSetELEGravityVector(index,phase,gravity_vector);
#ifdef TEST_CEL_MPC
      MZeigVec(gravity_vector,nn, "MPCCalculateElementMatrix1D: gravity vector");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 3: /* storativity pressure: compressibility */
      storativity_pressure_matrix = MPCGetELEStorativityPressureMatrix(index,phase);
	  if (!storativity_pressure_matrix) {
        storativity_pressure_matrix = (double *) Malloc(nn2* sizeof(double));
	  }
      /* fkt = n*S^p*X_k^p*drho^p/dp^p */
      fkt = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
#ifdef CHARACTERISTIC_NUMBERS
      fkt = MPCCalcCompressibilityNumber(index,phase,comp,gp);
#endif
      /* N^T* fkt *N */
      fkt *= A * L / 6.;
      if (mpc_mass_lumping_method)
        {
          storativity_pressure_matrix[0] = 3. * fkt;
          storativity_pressure_matrix[1] = 0.;
          storativity_pressure_matrix[2] = 0.;
          storativity_pressure_matrix[3] = 3. * fkt;
        }
      else
        {
          storativity_pressure_matrix[0] = 2. * fkt;
          storativity_pressure_matrix[1] = 1. * fkt;
          storativity_pressure_matrix[2] = 1. * fkt;
          storativity_pressure_matrix[3] = 2. * fkt;
        }
      MPCSetELEStorativityPressureMatrix(index,phase,storativity_pressure_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(storativity_pressure_matrix,nn,nn,"MPCCalculateElementMatrix1D: CP_k^p");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 4: /* Component diffusion matrix */
      diffusion_matrix = MPCGetELEDiffusionMatrix(index,phase);
	  if (!diffusion_matrix) {
        diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
	  }
      /* fkt= n*S^p*rho^p*X_k^p*D_k^p */
      fkt = mpc_porosity * mpc_saturation * mpc_density * mpc_mass_fraction * mpc_diffusion_coefficient;
#ifdef CHARACTERISTIC_NUMBERS
      fkt = MPCCalcDiffusivityNumber(index,phase,comp,gp);
#endif
      /* grad(phi)^T* fkt *grad(ome) */
      fkt *= A / L;
      diffusion_matrix[0] = fkt;
      diffusion_matrix[1] = -fkt;
      diffusion_matrix[2] = -fkt;
      diffusion_matrix[3] = fkt;
      /* store element matrix */
      MPCSetELEDiffusionMatrix(index,phase,diffusion_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(diffusion_matrix,nn,nn,"MPCCalculateElementMatrix1D: K_k^p");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 5: /* Dilatation coupling matrix */
      break;
    /*------------------------------------------------------------------------*/
    case 6: /* storativity temperature: thermal expansion matrix C_T^phase_comp*/
if((phase==0)&&(comp==1)) {
//OKmemory
      storativity_temperature_matrix = MPCGetELEStorativityTemperatureMatrix(index,phase);
	  if (!storativity_temperature_matrix) {
        storativity_temperature_matrix = (double *) Malloc(nn2* sizeof(double));
	  }
      /* fkt = n*S^g * drho/dT */
      fkt = mpc_porosity * mpc_saturation * mpc_drho_dT;
#ifdef CHARACTERISTIC_NUMBERS
/*      fkt = MPCCalcCompressibilityNumber(index,phase,comp,gp); */
#endif
      /* N^T* fkt *N */
      fkt *= A * L / 6.;
      if (mpc_mass_lumping_method)
        {
          storativity_temperature_matrix[0] = 3. * fkt;
          storativity_temperature_matrix[1] = 0.;
          storativity_temperature_matrix[2] = 0.;
          storativity_temperature_matrix[3] = 3. * fkt;
        }
      else
        {
          storativity_temperature_matrix[0] = 2. * fkt;
          storativity_temperature_matrix[1] = 1. * fkt;
          storativity_temperature_matrix[2] = 1. * fkt;
          storativity_temperature_matrix[3] = 2. * fkt;
        }
      MPCSetELEStorativityTemperatureMatrix(index,phase,storativity_temperature_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(storativity_temperature_matrix,nn,nn,"MPCCalculateElementMatrix1D: CT_k^p");
#endif
}
      break;
    /*------------------------------------------------------------------------*/
    case 7: /* Thermo-componental diffusion matrix - KT_k^p*/
if((phase==0)&&(comp==1)) {
      diffusion_thermo_comp_matrix = MPCGetELEThermoCompDiffusionMatrix(index,phase);
	  if (!diffusion_thermo_comp_matrix) {
        diffusion_thermo_comp_matrix = (double *) Malloc(nn2 * sizeof(double));
	  }
      /* fkt= n*S^p*rho^p*X_k^p*DT_k^p */
      fkt = mpc_porosity * mpc_saturation * mpc_density * mpc_mass_fraction * mpc_diffusion_coefficient;
#ifdef CHARACTERISTIC_NUMBERS
      /* fkt = MPCCalcDiffusivityNumber(index,phase,comp,gp); */
#endif
      /* grad(phi)^T* fkt *grad(ome) */
      fkt *= A / L;
      diffusion_thermo_comp_matrix[0] = fkt;
      diffusion_thermo_comp_matrix[1] = -fkt;
      diffusion_thermo_comp_matrix[2] = -fkt;
      diffusion_thermo_comp_matrix[3] = fkt;
      /* store element matrix */
      MPCSetELEThermoCompDiffusionMatrix(index,phase,diffusion_thermo_comp_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(diffusion_thermo_comp_matrix,nn,nn,"MPCCalculateElementMatrix1D: KT_k^p");
#endif
      break;
}
    default:
      /* Fehlermeldung */
      DisplayMsgLn("MPCCalculateElementMatrix1D: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCCalcElementMatrix2DTriangle

 Task: 
   Calculation of finite element matrices 
   of a 2-D linear triangular element of kernel MPC
   - hydraulic conductance matrix
   - hydraulic capacitance matrix
   - capillary forces
   - gravity forces
   - vapour diffusion term

 Parameters: (I: Input; R: Return; X: Both)
   I long index:    element number
   I int phase:     fluid phase 
   I int comp:      component number
   I int matrixtyp: matrix type

 Return:
   - void -

 Programming:
 02/2002   OK/JdJ   First implementation (based on CalcEle2DTriangle_ASM)
 11/2003 OK new storage of element matrices
 03/2004 WW Coupling matrix

**************************************************************************/
void MPCCalcElementMatrix2DTriangle(long index,int phase,int comp,int matrixtyp,CRFProcess*m_pcs)
{
  static int i;
  /* Geometry */
  static long *element_nodes;
  static double volume, fkt, A, area;
  static double r[3];
  static double x[3],y[3],x_gp,y_gp,x_mid,y_mid;
  static double fac_geo;
  static int nn=3;
  static int nn2;
  /* Materials */
  static double k_xx, k_yy, k_zz;
  static double d_xx, d_yy;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *storativity_pressure_matrix;
  // static double *capillarity_matrix;
  static double *diffusion_matrix;
  static double fac_mat;
  /*from comparison with 1-d*/
  static double *perm;
  /* DM -> SM - Kopplungsgroessen */
  //  static double invjac[4];
  //  static double *coupling_matrix_u_x, *coupling_matrix_u_y;
  //  static double g_ome_xy_t[12]; 
  //  static double coord[12]; 
  //  static double ome[3];
//TEST  const int nd = 6;
  nn2=nn*nn;
  /*--------------------------------------------------------------------------*/
  /* Element-Geometrie */
   /* Element-Koordinaten im lokalen System (x',y')==(a,b) */
  /*  Calc2DElementCoordinatesTriangle(index,x,y); */
  element_nodes = ElGetElementNodes(index);
  double gp[3]={0.,0.,0.};
  x_mid=0.0;
  y_mid=0.0;
  for (i=0;i<nn;i++) {
    x[i]=GetNodeX(element_nodes[i]);
    y[i]=GetNodeY(element_nodes[i]);
    x_mid +=x[i];
    y_mid +=y[i];
  }
  x_mid /= 3.;
  y_mid /= 3.;
  x_gp = x_mid;
  y_gp = y_mid;
  r[0]=x_mid;
  r[1]=y_mid;
  r[2]=0.0;
  mpc_time_collocation_global = 1.0;
m_pcs=m_pcs;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  A = m_mmp->geo_area;
  volume = fabs(ElGetElementVolume(index));
  area = volume/A;
  element_nodes = ElGetElementNodes(index);
  perm = m_mmp->PermeabilityTensor(index);
  k_xx = perm[0];
  k_yy = perm[3];
  k_zz = perm[3]; //???
  mpc_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  mpc_density               = m_mfp->Density();
  mpc_viscosity             = m_mfp->Viscosity();
  mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  mpc_drho_dp               = m_mfp->drho_dp;
  mpc_diffusion_coefficient = m_mfp->diffusion;
if((phase==0)&&(comp==1))
  mpc_drho_dT               = m_mfp->DensityTemperatureDependence(index,comp,gp,mpc_time_collocation_global);
  mpc_diffusion_coefficient = 1e-6;
  //----------------------------------------------------------------------
  //OK mpc_saturation = MMax(0., MPCGetSaturation(phase,index,0.,0.,0.,mpc_time_collocation_global));
  char fct_name[15];
  sprintf(fct_name,"SATURATION%i",phase+1);
  mpc_saturation = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
  /*==========================================================================*/
  /* Calculation of element matrices */
  switch (matrixtyp){
    /*------------------------------------------------------------------------*/
    case 0: /* K_component^phase - Conductivity matrix */
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(nn2 * sizeof(double));
      /* Calculate: grad(phi)* k*kr/mue*rho^l*X_w^l *grad(ome) */
      fac_mat = mpc_permeability_rel * mpc_density * mpc_mass_fraction / mpc_viscosity;
       /* 3x3 matrix */
      conductance_matrix[0] = k_xx*(y[1]-y[2])*(y[1]-y[2]) + k_yy*(x[2]-x[1])*(x[2]-x[1]);
      conductance_matrix[1] = k_xx*(y[1]-y[2])*(y[2]-y[0]) + k_yy*(x[2]-x[1])*(x[0]-x[2]);
      conductance_matrix[2] = k_xx*(y[1]-y[2])*(y[0]-y[1]) + k_yy*(x[2]-x[1])*(x[1]-x[0]);
      conductance_matrix[3] = k_xx*(y[2]-y[0])*(y[1]-y[2]) + k_yy*(x[0]-x[2])*(x[2]-x[1]);
      conductance_matrix[4] = k_xx*(y[2]-y[0])*(y[2]-y[0]) + k_yy*(x[0]-x[2])*(x[0]-x[2]);
      conductance_matrix[5] = k_xx*(y[2]-y[0])*(y[0]-y[1]) + k_yy*(x[0]-x[2])*(x[1]-x[0]);
      conductance_matrix[6] = k_xx*(y[0]-y[1])*(y[1]-y[2]) + k_yy*(x[1]-x[0])*(x[2]-x[1]);
      conductance_matrix[7] = k_xx*(y[0]-y[1])*(y[2]-y[0]) + k_yy*(x[1]-x[0])*(x[0]-x[2]);
      conductance_matrix[8] = k_xx*(y[0]-y[1])*(y[0]-y[1]) + k_yy*(x[1]-x[0])*(x[1]-x[0]);
       /* Volumetric element */
      fac_geo = 1./(4.*area);
#ifdef CHARACTERISTIC_NUMBERS
	  if (fabs(k_xx - k_yy)< MKleinsteZahl){
      fac_mat = MPCCalcNeumannNumber(index,phase,comp,r);
      conductance_matrix[0] = (y[1]-y[2])*(y[1]-y[2]) + (x[2]-x[1])*(x[2]-x[1]);
      conductance_matrix[1] = (y[1]-y[2])*(y[2]-y[0]) + (x[2]-x[1])*(x[0]-x[2]);
      conductance_matrix[2] = (y[1]-y[2])*(y[0]-y[1]) + (x[2]-x[1])*(x[1]-x[0]);
      conductance_matrix[3] = (y[2]-y[0])*(y[1]-y[2]) + (x[0]-x[2])*(x[2]-x[1]);
      conductance_matrix[4] = (y[2]-y[0])*(y[2]-y[0]) + (x[0]-x[2])*(x[0]-x[2]);
      conductance_matrix[5] = (y[2]-y[0])*(y[0]-y[1]) + (x[0]-x[2])*(x[1]-x[0]);
      conductance_matrix[6] = (y[0]-y[1])*(y[1]-y[2]) + (x[1]-x[0])*(x[2]-x[1]);
      conductance_matrix[7] = (y[0]-y[1])*(y[2]-y[0]) + (x[1]-x[0])*(x[0]-x[2]);
      conductance_matrix[8] = (y[0]-y[1])*(y[0]-y[1]) + (x[1]-x[0])*(x[1]-x[0]);
      }
	  else{
	  DisplayMsgLn("kxx not = kyy, disable characteristic numbers!");
            abort();
	  }
#endif
      for (i=0;i<nn2;i++) {
        conductance_matrix[i] *= fac_mat * fac_geo;
      }
      /* Store element matrix */
      MPCSetELEConductivityMatrix(index,phase,conductance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(conductance_matrix,nn,nn,"MPCCalcElementMatrix2DTriangle: conductivity matrix");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 1: /* C_component^phase - Capacitance matrix */
      capacitance_matrix = MPCGetELECapacitanceMatrix(index,phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
      /* Calculate: grad(phi)* k*kr/mue*rho^l*X_w^l *grad(ome) */
      fac_mat = mpc_porosity * mpc_density * mpc_mass_fraction;
#ifdef CHARACTERISTIC_NUMBERS
      fac_mat = MPCCalcStorativityNumber(index,phase,comp,r);
#endif
       /* 3x3 matrix */
      capacitance_matrix[0] = 2.0;
      capacitance_matrix[1] = 1.0;
      capacitance_matrix[2] = 1.0;
      capacitance_matrix[3] = 1.0;
      capacitance_matrix[4] = 2.0;
      capacitance_matrix[5] = 1.0;
      capacitance_matrix[6] = 1.0;
      capacitance_matrix[7] = 1.0;
      capacitance_matrix[8] = 2.0;
       /* Volumetric element */
      fac_geo = (area)/(12.);
      for (i=0;i<nn2;i++) {
        capacitance_matrix[i] *= fac_mat * fac_geo;
      }
      /* Store element matrix */
      MPCSetELECapacitanceMatrix(index,phase,capacitance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(capacitance_matrix,nn,nn,"MPCCalcElementMatrix2DTriangle: capacitance matrix");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 2: /* g_comp^phase - gravity vector */
      gravity_vector = MPCGetELEGravityVector(index,phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(nn*sizeof(double));
      /* Fuer erstes Integral: grad(phi)*kr*k/mue*rho*g */
      fkt = gravity_constant * k_zz \
          * mpc_density \
          * mpc_mass_fraction \
          * mpc_permeability_rel \
          / mpc_viscosity \
          * mpc_density;
       /* Volumetric element */
      fac_geo = 1./(2.*area);
      fkt *= fac_geo;
      gravity_vector[0] = fkt*(GetNode(element_nodes[2])->z - GetNode(element_nodes[1])->z );
      gravity_vector[1] = fkt*(GetNode(element_nodes[0])->z - GetNode(element_nodes[2])->z );
      gravity_vector[2] = fkt*(GetNode(element_nodes[1])->z - GetNode(element_nodes[0])->z );
      MPCSetELEGravityVector(index,phase,gravity_vector);
#ifdef TEST_CEL_MPC
      MZeigVec(gravity_vector,nn, "MPCCalcElementMatrix2DTriangle: gravity vector");
#endif
      break;
    /*------------------------------------------------------------------------*/
     case 3: /* storativity pressure: compressibility */
   /* if (phase==1) {   JDJ: Other element types do not have this if*/
      storativity_pressure_matrix = MPCGetELEStorativityPressureMatrix(index,phase);
      if (!storativity_pressure_matrix)
        storativity_pressure_matrix = (double *) Malloc(nn2* sizeof(double));
      /* fac_mat = n*S^p*X_k^p*drho^p/dp^p */
      fac_mat = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
#ifdef CHARACTERISTIC_NUMBERS
      fac_mat = MPCCalcCompressibilityNumber(index,phase,comp,r);
#endif
       /* 3x3 matrix */
      storativity_pressure_matrix[0] = 2.0;
      storativity_pressure_matrix[1] = 1.0;
      storativity_pressure_matrix[2] = 1.0;
      storativity_pressure_matrix[3] = 1.0;
      storativity_pressure_matrix[4] = 2.0;
      storativity_pressure_matrix[5] = 1.0;
      storativity_pressure_matrix[6] = 1.0;
      storativity_pressure_matrix[7] = 1.0;
      storativity_pressure_matrix[8] = 2.0;
       /* Volumetric element */
      fac_geo = (area)/(12.);
      for (i=0;i<nn2;i++) {
       storativity_pressure_matrix[i] *= fac_mat * fac_geo;
      }
      /* Store element matrix */
      MPCSetELEStorativityPressureMatrix(index,phase,storativity_pressure_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(storativity_pressure_matrix,nn,nn,"MPCCalcElementMatrix2DTriangle: storativity_pressure matrix");
#endif
	  //  } /* if(phase==1) */
      break;
    /*------------------------------------------------------------------------*/
    case 4: /* Component diffusion matrix */
      diffusion_matrix = MPCGetELEDiffusionMatrix(index,phase);
      if (!diffusion_matrix)
        diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
      /* Calculate: grad(phi)* n*S^g*rho^g*X_w^g*D^g *grad(ome) */
      fac_mat = mpc_porosity * mpc_saturation * mpc_density * mpc_mass_fraction;
       /* 3x3 matrix */
      //SB:2p mpc_diffusion_coefficient = CalcTracerDiffusionCoefficient(index, phase ,comp);
	  mpc_diffusion_coefficient = m_mfp->diffusion; //MATGetPhaseDiffusion(phase);
      d_xx = d_yy = mpc_diffusion_coefficient;
      diffusion_matrix[0] = d_xx*(y[1]-y[2])*(y[1]-y[2]) + d_yy*(x[2]-x[1])*(x[2]-x[1]);
      diffusion_matrix[1] = d_xx*(y[1]-y[2])*(y[2]-y[0]) + d_yy*(x[2]-x[1])*(x[0]-x[2]);
      diffusion_matrix[2] = d_xx*(y[1]-y[2])*(y[0]-y[1]) + d_yy*(x[2]-x[1])*(x[1]-x[0]);
      diffusion_matrix[3] = d_xx*(y[2]-y[0])*(y[1]-y[2]) + d_yy*(x[0]-x[2])*(x[2]-x[1]);
      diffusion_matrix[4] = d_xx*(y[2]-y[0])*(y[2]-y[0]) + d_yy*(x[0]-x[2])*(x[0]-x[2]);
      diffusion_matrix[5] = d_xx*(y[2]-y[0])*(y[0]-y[1]) + d_yy*(x[0]-x[2])*(x[1]-x[0]);
      diffusion_matrix[6] = d_xx*(y[0]-y[1])*(y[1]-y[2]) + d_yy*(x[1]-x[0])*(x[2]-x[1]);
      diffusion_matrix[7] = d_xx*(y[0]-y[1])*(y[2]-y[0]) + d_yy*(x[1]-x[0])*(x[0]-x[2]);
      diffusion_matrix[8] = d_xx*(y[0]-y[1])*(y[0]-y[1]) + d_yy*(x[1]-x[0])*(x[1]-x[0]);
#ifdef CHARACTERISTIC_NUMBERS
	  if (fabs(d_xx -d_yy)< MKleinsteZahl){
      fac_mat = MPCCalcDiffusivityNumber(index,phase,comp,r);
      diffusion_matrix[0] = (y[1]-y[2])*(y[1]-y[2]) + (x[2]-x[1])*(x[2]-x[1]);
      diffusion_matrix[1] = (y[1]-y[2])*(y[2]-y[0]) + (x[2]-x[1])*(x[0]-x[2]);
      diffusion_matrix[2] = (y[1]-y[2])*(y[0]-y[1]) + (x[2]-x[1])*(x[1]-x[0]);
      diffusion_matrix[3] = (y[2]-y[0])*(y[1]-y[2]) + (x[0]-x[2])*(x[2]-x[1]);
      diffusion_matrix[4] = (y[2]-y[0])*(y[2]-y[0]) + (x[0]-x[2])*(x[0]-x[2]);
      diffusion_matrix[5] = (y[2]-y[0])*(y[0]-y[1]) + (x[0]-x[2])*(x[1]-x[0]);
      diffusion_matrix[6] = (y[0]-y[1])*(y[1]-y[2]) + (x[1]-x[0])*(x[2]-x[1]);
      diffusion_matrix[7] = (y[0]-y[1])*(y[2]-y[0]) + (x[1]-x[0])*(x[0]-x[2]);
      diffusion_matrix[8] = (y[0]-y[1])*(y[0]-y[1]) + (x[1]-x[0])*(x[1]-x[0]);
      }
      else{
	  DisplayMsgLn("dxx not = dyy, disable characteristic numbers!");
            abort();
	  }
#endif
       /* Volumetric element */
      fac_geo = 1./(4.*area);
      for (i=0;i<nn2;i++) {
        diffusion_matrix[i] *= fac_mat * fac_geo;
      }
      /* Store element matrix */
      MPCSetELEDiffusionMatrix(index,phase,diffusion_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(diffusion_matrix,nn,nn,"MPCCalcElementMatrix2DTriangle: diffusion_matrix");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 5: /* Dilatation coupling matrix */
    /*----------------------------------------------------------------*/
    /* coupling matrices WW*/
    if(GetRFProcessProcessing("SD")) {//WW
/*
      Init_Quadratic_Elements();
      coupling_matrix_u_x = MPCGetELEStrainCouplingMatrixX(index,phase);
      if (!coupling_matrix_u_x)
        coupling_matrix_u_x = (double *) Malloc(nn*9* sizeof(double));
      coupling_matrix_u_y = MPCGetELEStrainCouplingMatrixY(index,phase);
      if (!coupling_matrix_u_y)
        coupling_matrix_u_y = (double *) Malloc(nn*nd* sizeof(double));
      // Initialisieren 
      MNulleMat(coupling_matrix_u_x,nn,nd);
      MNulleMat(coupling_matrix_u_y,nn,nd);

      elem_dm->ConfigElement(index, 2);
      elem_dm->ComputeStrainCouplingMatrix(0, coupling_matrix_u_x);
      elem_dm->ComputeStrainCouplingMatrix(1, coupling_matrix_u_y);

      gp[0]=0.0; gp[1]=0.0; gp[2]=0.0; 
      fkt = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
      fkt *= m_mfp->Density();

      for (i=0;i<(nn*nd);i++) {
        coupling_matrix_u_x[i] *= area*fkt;
        coupling_matrix_u_y[i] *= area*fkt;
      }
 
       MPCSetELEStrainCouplingMatrixX(index,phase,coupling_matrix_u_x);
       MPCSetELEStrainCouplingMatrixY(index,phase,coupling_matrix_u_y);
       Init_Linear_Elements();
*/
    } /* deformation */
      break;
    default:
      /* Fehlermeldung */
      DisplayMsgLn("CalcEle2DTriangle_MPC: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}
/**************************************************************************
 ROCKFLOW - Funktion: MPCCalcElementMatrix2D

 Task: 
   Calculation of finite element matrices 
   of a 2-D linear triangular element of kernel MPC
   - hydraulic conductance matrix
   - hydraulic capacitance matrix
   - capillary forces
   - gravity forces
   - vapour diffusion term

 Parameters: (I: Input; R: Return; X: Both)
   I long index:    element number
   I int phase:     fluid phase 
   I int comp:      component number
   I int matrixtyp: matrix type

 Return:
   - void -

 Programming:
 02/2002 OK Implementation based on CalcEle2D_MMP (CT)
 06/2003 JDJ/OK Bugfix: gp[0]=r; gp[1]=s; gp[2]=0.0;
 06/2003 OK DM Coupling terms
 11/2003 OK new storage of element matrices
 03/2004 WW Improve the efficiency
            Remove a bug on DM coupling matrix  
**************************************************************************/
void MPCCalcElementMatrix2D(long index,int phase,int comp,int matrixtyp,CRFProcess*m_pcs)
{
  static double *invjac, detjac;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  //  static double *capillarity_matrix;
  static double *storativity_pressure_matrix;
  double *mpc_diffusion_matrix=NULL;
  static double *gravity_vector;
  static long *element_nodes;
  static double elevation[8];
  static double g;
  double Var;
  static double tkt[9], kp[9], rho_n[4];
  static int anzgp;
  static long i, j, k, l;
  static int ii, jj,kk, ll;
  const int dim = 2;
//TEST  const int nd = 9;
  static double r, s, r_upw, s_upw; //, fkt;
  static double zwi[16];
  static double zwa[16];
  static double phi[16];
  static double grad_N[16];
  static double fac_geo;
  /* Materials */
  static double kr, kr_max,  rho, mue, n,  area;
  static double fac_mat;
  static double alpha[2], scale;        /* SUPG */
  static double v[2], v_rs[2];
  static double jacobi[4];
  //  static double *coupling_matrix_u_x, *coupling_matrix_u_y;
  //  static long kl;
/* DM -> SM - DM coupling */
//  static double invjac_9N[4],invjac_t_9N[4],detjac_9N;
//  static double g_ome_t_9N[18];  
//  static double g_ome_xy_t_9N[18]; 
  //--------------------------------------------------------------------
  // NUM
  int mpc_upwind_method = 0;
  double mpc_collocation_upwind = 0.5;
  double mpc_upwind_parameter = 0.0;
  int mpc_element_integration_method_maximum = 0;
  int mpc_element_integration_method_rel_perm = 0;
  int mpc_element_integration_method_viscosity = 0;
  int mpc_element_integration_method_density = 0;
  mpc_time_collocation_global = 1.;
  int nn=4;
  int nn2=16;
  anzgp = m_pcs->m_num->ele_gauss_points;
  double gp[3]={0.,0.,0.};
  g = gravity_constant;
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  //--------------------------------------------------------------------
  // State variables
  char fct_name[15];
  sprintf(fct_name,"SATURATION%i",phase+1);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  n = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  area = m_mmp->geo_area;
  double* mpc_permeability = NULL;
  mpc_permeability = m_mmp->PermeabilityTensor(index);
  mpc_permeability_ref = mpc_permeability[0];
  kp[0] = mpc_permeability[0];
  kp[1] = mpc_permeability[1];
  kp[2] = mpc_permeability[2];
  kp[3] = mpc_permeability[3];
  mpc_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
  if (m_mmp->porosity_model>=3 && m_mmp->permeability_porosity_model==2){              //porosity-permeability function     //MX, 10.2004
	double mpc_permeability_rel_porosity = m_mmp->PermeabilityPorosityFunction(index,gp,mpc_time_collocation_global);
	if (mpc_permeability_rel_porosity > 0.0)
	mpc_permeability_rel *= mpc_permeability_rel_porosity;
  }
  //----------------------------------------------------------------------
  // MFP fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  mpc_density               = m_mfp->Density();
  mpc_viscosity             = m_mfp->Viscosity();
  mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  mpc_drho_dp               = m_mfp->drho_dp;
  mpc_diffusion_coefficient = m_mfp->diffusion;
if((phase==0)&&(comp==1))
  mpc_drho_dT               = m_mfp->DensityTemperatureDependence(index,comp,gp,mpc_time_collocation_global);
  mpc_diffusion_coefficient = 1e-6;
  //----------------------------------------------------------------------
  switch (matrixtyp)
    {
    /*========================================================================*/
    case 0:
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      if (!conductance_matrix)
          conductance_matrix = (double *) Malloc(nn2 * sizeof(double));
      /* Initialisieren */
      MNulleMat(conductance_matrix,nn,nn);
      /*----------------------------------------------------------------------*/
      /* Upwinding fuer Permeabilitaetsmatrizen */
        /* Initialisieren */
      MNulleVec(alpha, 2);
      r_upw = s_upw = 0.;
      if ((mpc_upwind_method == 1) || (mpc_upwind_method == 2))
        {
          //CalcVelo2Drs(phase, index, mpc_collocation_upwind, 0., 0., 0., v);  MB
          VELCalcGaussQuadLocal(phase, index, gp, m_pcs, v);
      
          /* Geschwindigkeitstransformation: a,b -> r,s */
          Calc2DElementJacobiMatrix(index, 0., 0., invjac, &detjac);
          MKopierVec(invjac, jacobi, 4);
          M2InvertiereUndTransponiere(jacobi);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jacobi-Matrix */
          MMultMatVec(jacobi, 2, 2, v, 2, v_rs, 2);
          if (MBtrgVec(v_rs, 2) > MKleinsteZahl)
            {
              /* Upwind-Faktoren */
              for (l = 0; l < 2; l++)
                alpha[l] = -mpc_upwind_parameter * v_rs[l] / (MBtrgVec(v_rs, 2) + MKleinsteZahl);
            }
          if (mpc_upwind_method == 1)
            {
              /* Verschiebungen der Gausspunkte auf Element begrenzen */
              scale = 1.;
              if (fabs(alpha[0]) > 1.)
                scale = MMin(scale, 1. / fabs(alpha[0]));
              if (fabs(alpha[1]) > 1.)
                scale = MMin(scale, 1. / fabs(alpha[1]));
              r_upw = scale * alpha[0];
              s_upw = scale * alpha[1];
            }
          if (mpc_upwind_method == 2)
            {
              /* Verschiebungen auf -1<x<1 begrenzen */
              r_upw = MRange(-1., alpha[0], 1.);
              s_upw = MRange(-1., alpha[1], 1.);
            }
        }
      /* Maximum Mobility Upwinding */
      if (mpc_upwind_method == 3)
        {
          kr_max = 0.;
          for (i = 0; i < 4; i++)
            kr_max = MMax(kr_max, MMPGetNodeRelativePermeability(phase, index, element_nodes[i],mpc_collocation_upwind));
          kr = kr_max;
        }
      m_mfp->mode = 1;
      for (l = 0; l < nn; l++)
        rho_n[l] = m_mfp->Density();
//OK_MMP        rho_n[l] = MATCalcFluidDensityNode(phase,element_nodes[l],mpc_time_collocation_global);
      m_mfp->mode = 0;
      /* Schleife ueber GaussPunkte */
      for (i = 0; i < anzgp; i++)
        {
          for (j = 0; j < anzgp; j++)
            {
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
              /* Leitfaehigkeitsmatrix ohne Zeitanteile */
              /* Integration ueber Gausspunkte */
              if (mpc_element_integration_method_maximum)
                {
                  gp[0] = r_upw = r;
                  gp[1] = s_upw = s;

                  if (mpc_upwind_method == 1)
                    {
                      /* Verschiebungen der Gausspunkte auf Element begrenzen */
                      scale = 1.;
                      if (fabs(r + alpha[0]) > 1.)
                        scale = MMin(scale, (1. - fabs(r)) / fabs(alpha[0]));
                      if (fabs(s + alpha[1]) > 1.)
                        scale = MMin(scale, (1. - fabs(s)) / fabs(alpha[1]));
                      gp[0] = r_upw = r + scale * alpha[0];
                      gp[1] = s_upw = s + scale * alpha[1];
                    }
                  if (mpc_upwind_method == 2)
                    {
                      /* Verschiebungen auf -1<x<1 begrenzen */
                      gp[0] = r_upw = MRange(-1., r + alpha[0], 1.);
                      gp[1] = s_upw = MRange(-1., s + alpha[1], 1.);
                    }

                  if (mpc_element_integration_method_rel_perm)
                    kr = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
                  if (mpc_element_integration_method_viscosity)
                    mue = m_mfp->Viscosity();
                  if (mpc_element_integration_method_density)
                    rho = m_mfp->Density();
                  if (mpc_upwind_method == 3)
                    kr = kr_max;
                }
      /* Calculate: grad(phi)* k*kr/mue*rho^l*X_w^l *grad(ome) */
	  gp[0]=r; gp[1]=s; gp[2]=0.0; /*JDJ 10.6.03 war gp[3]*/
#ifdef CHARACTERISTIC_NUMBERS
      fac_mat = MPCCalcNeumannNumber(index,phase,comp,gp);
	  fac_mat /= kp[0];
#else
      fac_mat = mpc_permeability_rel * mpc_density * mpc_mass_fraction / mpc_viscosity;
#endif
              /* J^-1 */
              Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
              /* Faktor */
              fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
              /* tkmyt = J^-1^T * (k) * (J^-1)*/
              MMultMatMat(kp, 2, 2, invjac, 2, 2, zwa, 2, 2);
              MTranspoMat(invjac, 2, 2, zwi);
              MMultMatMat(zwi, 2, 2, zwa, 2, 2, tkt, 2, 2);
			  /* Ansatz- und Wichtungsfunktionen holen */
              // grad(phi) 
              MGradPhi2D(grad_N, r, s);     /* 2 Zeilen 4 Spalten */
			  // WW
              for(ii=0; ii<nn; ii++)
			  {
                 for(jj=0; jj<nn; jj++)
				 {
                    Var = 0.0;
				    for(kk=0; kk<dim; kk++)
					{
     			       for(ll=0; ll<dim; ll++)
				       {
                           Var += grad_N[kk*nn+ii]*tkt[dim*kk+ll]*grad_N[ll*nn+jj];
				       }
					}
                    conductance_matrix[ii*nn+jj] += Var*fac_geo * fac_mat* area;
				 }
			  }
           }                          /* Ende der Schleife ueber GaussPunkte */
        }                              /* Ende der Schleife ueber GaussPunkte */
      MPCSetELEConductivityMatrix(index,phase,conductance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(conductance_matrix,nn,nn,"MPCCalcElementMatrix2D: conductance_matrix");
#endif
      break;
    /*========================================================================*/
    case 1:
      /* Kompressibilitaet und Speicherterme */
      capacitance_matrix = MPCGetELECapacitanceMatrix(index,phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
      /* Initialisieren */
      MNulleMat(capacitance_matrix, nn, nn);
      for (i = 0; i < anzgp; i++)
        {
          for (j = 0; j < anzgp; j++)
            {
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
              gp[0]=r; gp[1]=s; gp[2]=0.0; /*JDJ 10.6.03 */

              /* Berechnen der Element-Jacobi-Matrix [J_3D],
                 ihrer Determinante det[J_3D] sowie Inversen [J_3D^-1]
                 in den Gauss-Punkten */
              Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);

              /* fkt = (1/rho*n*d_rho/d_p*S + Se*S + n*dS/dp)  Elementweise konstante Werte ausserhalb der Schleife */
			  /* // WW
              fkt = (n * GetFluidDensityPressureDependency(phase)
//                    / GetFluidDensity(phase, index, r, s, 0., mpc_time_collocation_global)
                    / MATCalcFluidDensity(phase,index,gp,mpc_time_collocation_global,0,NULL)
                    * MMax(0., MPCGetSaturation(phase, index, r, s, 0., mpc_time_collocation_global))
                    + S
                    * MMax(0., MPCGetSaturation(phase, index, r, s, 0., mpc_time_collocation_global))
                  );
               */
#ifdef CHARACTERISTIC_NUMBERS
      fac_mat = MPCCalcStorativityNumber(index,phase,comp,gp);
#else
      /* Calculate: n * rho^p X_k^p  */
      mpc_density               = m_mfp->Density();
      mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
      fac_mat = mpc_porosity * mpc_density * mpc_mass_fraction;
#endif
              /* Gauss-Faktoren g_i * g_j  * g_k * det[J_2D] */
              fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
              /* Ansatzfunktion im Gauss-Punkt omega==N */
              MOmega2D(phi, r, s);
              if (mpc_mass_lumping_method)
                {
                  // Wenn die Speicherung per ML-FEM
                  //   behandelt wird, wird nur die Diagonale gesetzt 
                  for (k = 0; k < nn; k++)
                    capacitance_matrix[k * nn+k] += phi[k]*  fac_mat * fac_geo*area;
                }
              else
                {
                   //WW
                   for(ii=0; ii<nn; ii++)
			       {
                      for(jj=0; jj<nn; jj++)
				      {
                          capacitance_matrix[ii*nn+jj] 
							 += phi[ii]*phi[jj]* fac_mat * fac_geo* area;
 				      }
         		  }
              }
            }
        }                              /* Ende der Schleife ueber die Gauss-Punkte */    
      /* Store element matrix */
      MPCSetELECapacitanceMatrix(index,phase,capacitance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(capacitance_matrix,nn,nn,"MPCCalcElementMatrix2D: capacitance matrix");
#endif
      break;
    //--------------------------------------------------------------------
    case 2:
      /* Gravitationsvektor mit elementgemittelter Dichte ! */
      gravity_vector = MPCGetELEGravityVector(index,phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(nn2*sizeof(double));
      /*                   
        OK hier ist noch was faul
        gravity_vector = (double *) Malloc(nn*sizeof(double));
      */
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      /* Initialisieren */
      MNulleVec(gravity_vector, nn);
      /* Schwerkraftterm berechnen (gravity vector) */
      for (l=0;l<nn;l++)
        elevation[l] = ((GetNode(element_nodes[l]))->y) * mpc_density * g; //WW
//        elevation[l] = ((GetNode(element_nodes[l]))->z) * mpc_density * g;
      MMultMatVec(conductance_matrix, nn, nn, elevation, nn, gravity_vector, nn);
      MPCSetELEGravityVector(index,phase,gravity_vector);
#ifdef TEST_CEL_MPC
      MZeigVec(gravity_vector,nn, "MPCCalcElementMatrix2D: gravity vector");
#endif
      break;
    //--------------------------------------------------------------------
    case 3:
      storativity_pressure_matrix = MPCGetELEStorativityPressureMatrix(index,phase);
      if (!storativity_pressure_matrix)
        storativity_pressure_matrix = (double *) Malloc(nn2* sizeof(double));
      /* Initialisieren */
      MNulleMat(storativity_pressure_matrix,nn,nn);
      /* in CGS */
      /*-----------------------------------------------------*/
      for (i = 0; i < anzgp; i++) {
        for (j = 0; j < anzgp; j++) {
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
		  gp[0]=r; gp[1]=s; gp[2]=0.0; /*JDJ 10.6.03*/
          /*gp[0]=r; gp[2]=s; gp[3]=0.0;*/ /*3824*/
          Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
#ifdef CHARACTERISTIC_NUMBERS
          fac_mat = MPCCalcCompressibilityNumber(index,phase,comp,gp);
#else		  
		  /* fkt = n*S^p*X_k^p*drho^p/dp^p */
          mpc_saturation = MMax(0., MPCGetSaturation(phase,index,r,s,0.,mpc_time_collocation_global));
          mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
          fac_mat = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
#endif
          /* Gauss-Faktoren g_i * g_j  * g_k * det[J_2D] */
          fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
          /* Ansatzfunktion im Gauss-Punkt omega==N */
          MOmega2D(phi, r, s);
          if (mpc_mass_lumping_method) {
            // Wenn die Speicherung per ML-FEM behandelt wird, wird nur die Diagonale gesetzt 
            for (k = 0; k < nn; k++)
              storativity_pressure_matrix[k * nn+k] += phi[k] * fac_mat * fac_geo *area;
          }
          else {
             //WW
             for(ii=0; ii<nn; ii++)
             {
                for(jj=0; jj<nn; jj++)
                    storativity_pressure_matrix[ii*nn+jj] 
					  += phi[ii]*phi[jj]* fac_mat * fac_geo* area;
    		  }

           }
        }
      }                              /* Ende der Schleife ueber die Gauss-Punkte */
      /* Store element matrix */
      MPCSetELEStorativityPressureMatrix(index,phase,storativity_pressure_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(storativity_pressure_matrix,nn,nn,"MPCCalcElementMatrix2D: storativity_pressure matrix");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 4: /* Component diffusion matrix */
      mpc_diffusion_matrix = MPCGetELEDiffusionMatrix(index,phase);
      if (!mpc_diffusion_matrix)
        mpc_diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
      // Initialisieren . WW
      MNulleMat(mpc_diffusion_matrix,nn,nn);
      for (i = 0; i < anzgp; i++)
        {
          for (j = 0; j < anzgp; j++)
            {
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
              gp[0]=r; gp[1]=s; gp[2]=0.0;
       /* Materials */
#ifdef CHARACTERISTIC_NUMBERS
      fac_mat = MPCCalcDiffusivityNumber(index,phase,comp,gp); 
      kp[1] = kp[2] = 0.0;
      kp[0] = 1.0;
      kp[3] = 1.0;
#else
      mpc_saturation = MMax(0., MPCGetSaturation(phase,index,gp[0],gp[1],0., \
		  mpc_time_collocation_global));
      mpc_density = m_mfp->Density();
      //JDJ mpc_diffusion_coefficient = MATGetComponentDiffusionCoefficient(index,phase,comp);
//SB - removed tracer properties	  mpc_diffusion_coefficient = CalcTracerDiffusionCoefficient(index, phase ,comp);
	  mpc_diffusion_coefficient = m_mfp->diffusion; //SB replaced
      fac_mat = mpc_porosity * mpc_saturation * mpc_density;
#endif
      /* Calculate: grad(phi) * D^g * grad(ome) */
              /* J^-1 */
              Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
              /* Faktor */
              fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
              /* tkmyt = (J^-1)^T * (k) * (J^-1) */
              MMultMatMat(kp, 2, 2, invjac, 2, 2, zwa, 2, 2);
              MTranspoMat(invjac, 2, 2, zwi);
              MMultMatMat(zwi, 2, 2, zwa, 2, 2, tkt, 2, 2);
              // grad(phi) 
              MGradPhi2D(grad_N, r, s);     /* 2 Zeilen 4 Spalten */
			  // WW
              for(ii=0; ii<nn; ii++)
			  {
                 for(jj=0; jj<nn; jj++)					  
				 {
                    Var = 0.0;
				    for(kk=0; kk<dim; kk++)
					{
     			       for(ll=0; ll<dim; ll++)
				       {
                           Var += grad_N[kk*nn+ii]*tkt[dim*kk+ll]*grad_N[ll*nn+jj];
				       }
					}
                    mpc_diffusion_matrix[ii*nn+jj] += Var * fac_geo * fac_mat* area;
				 }
			  }
           }                          /* Ende der Schleife ueber GaussPunkte */
        }                              /* Ende der Schleife ueber GaussPunkte */
      MPCSetELEDiffusionMatrix(index,phase,mpc_diffusion_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(mpc_diffusion_matrix,nn,nn,"MPCCalcElementMatrix2D: diffusion matrix");
#endif
      break;
    /*------------------------------------------------------------------------*/
    case 5: // Strain coupling matrix
    ///WW TEST
    if(phase==1) {
    if(GetRFProcessProcessing("SD")) {
/*
      Init_Quadratic_Elements();
      coupling_matrix_u_x = MPCGetELEStrainCouplingMatrixX(index,phase);
      if (!coupling_matrix_u_x)
        coupling_matrix_u_x = (double *) Malloc(nn*nd* sizeof(double));
      coupling_matrix_u_y = MPCGetELEStrainCouplingMatrixY(index,phase);
      if (!coupling_matrix_u_y)
        coupling_matrix_u_y = (double *) Malloc(nn*nd* sizeof(double));
      MNulleMat(coupling_matrix_u_x,nn,nd);
      MNulleMat(coupling_matrix_u_y,nn,nd);
      elem_dm->ConfigElement(index, 2);
      elem_dm->ComputeStrainCouplingMatrix(0, coupling_matrix_u_x);
      elem_dm->ComputeStrainCouplingMatrix(1, coupling_matrix_u_y);

      gp[0]=0.0; gp[1]=0.0; gp[2]=0.0; 
      fkt = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
      fkt *= m_mfp->Density();

      for (i=0;i<(nn*nd);i++) {
        coupling_matrix_u_x[i] *= area*fkt;
        coupling_matrix_u_y[i] *= area*fkt;
      }
      MPCSetELEStrainCouplingMatrixX(index,phase,coupling_matrix_u_x);
      MPCSetELEStrainCouplingMatrixY(index,phase,coupling_matrix_u_y);
#ifdef TEST_CEL_MPC
      MZeigMat(coupling_matrix_u_x,nn,nn, "MPCStrainCouplingMatrixX");
      MZeigMat(coupling_matrix_u_y,nn,nn, "MPCStrainCouplingMatrixY");
#endif
      Init_Linear_Elements();
*/
    }
    }
    break;
    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}
/**************************************************************************
 ROCKFLOW - Funktion: MPCCalcElementMatrix3DHexahedron

 Task: 
   Calculation of finite element matrices 
   of a 3-D linear hexahedron element of kernel MPC
   - hydraulic conductance matrix
   - hydraulic capacitance matrix
   - capillary forces
   - gravity forces
   - vapour diffusion term

 Parameters: (I: Input; R: Return; X: Both)
   I long index:    element number
   I int phase:     fluid phase 
   I int comp:      component number
   I int matrixtyp: matrix type

 Return:
   - void -

 Programming:
 02/2002   OK   Implementation based on CalcEle2D_MMP (CT)
 06/2003 OK Bugfix: gp[0]=r; gp[1]=s; gp[2]=0.0;
04/2004 JDJ Bugfix and re-implementation based on ASM
12/2004 JDJ Bugfix after version 4 implementations
**************************************************************************/
void MPCCalcElementMatrix3DHexahedron(long index,int phase,int comp,int matrixtyp,CRFProcess*m_pcs)
{
  /* Counter */
  static long i, j, k, l, m;
  int ii,jj,kk,ll;
  /* Matrices */
  static double *conductance_matrix;
  static double *mpc_capacitance_matrix;
  static double *storativity_pressure_matrix;
  static double *mpc_diffusion_matrix;
  static double gravity_matrix[64];
  static double *coupling_matrix_u_x, *coupling_matrix_u_y, *coupling_matrix_u_z;
  //  static double *coupling_matrix_pu;
  /* Vectors */
  static double *gravity_vector;
  /* Tensors */
  static double tkt[9], kp[9], dif[9];
  /* NUMerics */
  static int anzgp;
  static double r,s,t;
  //  static double phi[9];
  // static double phi_t[9];
  //  static double grad_phi[24];
  //  static double grad_phi_t[24];
  // static double grad_ome[24];
  static double ome[24], g_ome_t_20N[24], g_ome_xyz_t_20N[24];
  static double invjac_t_20N[9];
  /* NOD, ELE */
  static long *element_nodes;
  /* GEOmetry */
  static double *invjac, detjac, detjac_20N;
  static double fac_geo;
  /* AUXiliary */
  static double elevation[8];
  static double zwi[64];
  static double zwa[64];
  static long   kl;
   double Var;
  /* MATerials */
   // static double k_xx,k_yy,k_zz, 
  static double n,*perm;
  static double fac_mat;
  static double g;
  /*--------------------------------------------------------------------------*/
  /* NUMerics */
  double gp[3]={0.,0.,0.};
  int nn=8;
  int nn2=64;
  int dim = 3;
  anzgp = m_pcs->m_num->ele_gauss_points;
  mpc_time_collocation_global = 1.;
  /*--------------------------------------------------------------------------*/
  /* GEOmetry */
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  g = gravity_constant;
if(phase==1)
  phase = phase;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  n = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  perm = m_mmp->PermeabilityTensor(index);
  kp[0] = perm[0];
  kp[4] = perm[4];
  kp[8] = perm[8];
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  m_mfp->mode = 0; //JDJ 20.12.2004
  mpc_density               = m_mfp->Density();
  mpc_viscosity             = m_mfp->Viscosity();
  mpc_mass_fraction         = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  mpc_drho_dp               = m_mfp->drho_dp;
  mpc_diffusion_coefficient = m_mfp->diffusion;
if((phase==0)&&(comp==1))
  mpc_drho_dT               = m_mfp->DensityTemperatureDependence(index,comp,gp,mpc_time_collocation_global);
  mpc_diffusion_coefficient = 1e-6;
  //--------------------------------------------------------------------
  // State variables
  char fct_name[15];
  sprintf(fct_name,"SATURATION%i",phase+1);
  static double fkt;
  /****************************************************************************/
  switch (matrixtyp)
    {
    /*========================================================================*/
    case 0:
      /*----------------------------------------------------------------------*/
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(nn2 * sizeof(double));
      MNulleMat(conductance_matrix,nn,nn);
      /*----------------------------------------------------------------------*/
      /* Schleife ueber GaussPunkte */
      for (i=0;i<anzgp;i++)
        for (j=0;j<anzgp;j++)
          for (k=0;k<anzgp;k++) {
            r = MXPGaussPkt(anzgp,i);
            s = MXPGaussPkt(anzgp,j);
            t = MXPGaussPkt(anzgp,k);
            /* Integration ueber Gausspunkte */
            /*------------------------------------------------------------------*/
            /* MATerials */
            /* fac_mat = k*kr/mue*rho^l*X_w^l */
			MNulleMat(kp, 3, 3);
			kp[0] = perm[0]; //assignment JDJ 20.12.04
            kp[4] = perm[4];
            kp[8] = perm[8];
            gp[0]=r; gp[1]=s; gp[2]=t; //JDJ 20.12.04
#ifdef noCHARACTERISTIC_NUMBERS
            gp[0]=r; gp[1]=s; gp[2]=t;
            mpc_density          = m_mfp->Density();
            mpc_viscosity        = m_mfp->Viscosity();
            mpc_mass_fraction    = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
            mpc_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
            fac_mat = mpc_permeability_rel * mpc_density * mpc_mass_fraction / mpc_viscosity;
#endif
#ifdef CHARACTERISTIC_NUMBERS
            fac_mat = MPCCalcNeumannNumber(index,phase,comp,gp);
            fac_mat /= kp[0];
#endif
	      /* J^-1 */
              Calc3DElementJacobiMatrix(index, r, s,t, invjac, &detjac);
              /* Faktor */
              fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k)* detjac;
              /* tkmyt = J^-1 * (k) * (J^-1)T */
              MMultMatMat(kp, dim, dim, invjac, dim, dim, zwa, dim, dim);
              MTranspoMat(invjac, dim, dim, zwi);
              MMultMatMat(zwi, dim, dim, zwa, dim, dim, tkt, dim, dim);
			  /* Ansatz- und Wichtungsfunktionen holen */
              // grad(phi) 
              MGradPhi3D(ome, r, s, t);     /* 2 Zeilen 4 Spalten */
			  // WW
              for(ii=0; ii<nn; ii++)
			  {
                 for(jj=0; jj<nn; jj++)
				 {
                    Var = 0.0;
				    for(kk=0; kk<dim; kk++)
					{
     			       for(ll=0; ll<dim; ll++)
				       {
                           Var += ome[kk*nn+ii]*tkt[dim*kk+ll]*ome[ll*nn+jj];
				       }
					}
                    conductance_matrix[ii*nn+jj] += Var*fac_geo * fac_mat;
				 }
			  }
      }
      /* Schleife ueber GaussPunkte */
      /*----------------------------------------------------------------------*/
      MPCSetELEConductivityMatrix(index,phase,conductance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(conductance_matrix,nn,nn,"MPCCalcElementMatrix3D: conductance_matrix");
#endif
      break;
    /*========================================================================*/
    case 1:
      /* Kompressibilitaet und Speicherterme */
      mpc_capacitance_matrix = MPCGetELECapacitanceMatrix(index,phase);
      if (!mpc_capacitance_matrix)
        mpc_capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
      MNulleMat(mpc_capacitance_matrix,nn,nn);
      /*----------------------------------------------------------------------*/
      /* Schleife ueber GaussPunkte */
      for (i=0;i<anzgp;i++)
        for (j=0;j<anzgp;j++)
          for (k=0;k<anzgp;k++){
            r = MXPGaussPkt(anzgp,i);
            s = MXPGaussPkt(anzgp,j);
            t = MXPGaussPkt(anzgp,k);
            gp[0]=r; gp[1]=s; gp[2]=t;
             /* [J_3D^-1], det[J_3D] */
            Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
             /* n*rho^p*X_k^p  */
#ifdef noCHARACTERISTIC_NUMBERS
            mpc_porosity      = m_mmp->Porosity(index,gp,theta);
            mpc_density       = m_mfp->Density();
            mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
            fac_mat = mpc_porosity * mpc_density * mpc_mass_fraction;
#endif
#ifdef CHARACTERISTIC_NUMBERS
            fac_mat = MPCCalcStorativityNumber(index,phase,comp,gp);
#endif
             /* g_i*g_j*g_k * det[J_3D] */
            fac_geo = MXPGaussFkt(anzgp,i) * MXPGaussFkt(anzgp,j) * MXPGaussFkt(anzgp,k) \
                      * detjac;
/*
            // Normale FEM 
            // Wichtungsfunktionen im Gauss-Punkt phi==N 
            MPhi3D(phi,r,s,t);
            // Ansatzfunktion im Gauss-Punkt omega==N 
            MOmega3D(ome,r,s,t);
            // Berechnen phi * omega 
            MMultVecVec(phi,nn,ome,nn,zwi,nn,nn);
            for (l=0;l<nn2;l++)
              mpc_capacitance_matrix[l] += (zwi[l] * fac_mat * fac_geo); */
			// Ansatzfunktion im Gauss-Punkt omega==N
              MOmega3D(ome, r, s, t);
			//WW
              for(ii=0; ii<nn; ii++)
              {
                 for(jj=0; jj<nn; jj++)
	             {
                     mpc_capacitance_matrix[ii*nn+jj]+= ome[ii]*ome[jj]* fac_mat * fac_geo;
 				 }
         	  }
      } /* Ende der Schleife ueber die Gauss-Punkte */
      /* Store element matrix */
      MPCSetELECapacitanceMatrix(index,phase,mpc_capacitance_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(mpc_capacitance_matrix,nn,nn,"MPCCalcElementMatrix3D: capacitance matrix");
#endif
      break;
 /*========================================================================*/
    case 2:
	 /* Gravitationsvektor mit elementgemittelter Dichte ? */
      /* Gravitationsvektor mit elementgemittelter Dichte ! */
      gravity_vector = MPCGetELEGravityVector(index,phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(nn*sizeof(double));
      conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      if (!conductance_matrix){
//OK???          MPCCalcElementMatrix3DHexahedron(index,phase,comp,0);
        conductance_matrix = MPCGetELEConductivityMatrix(index,phase);
      }
      MNulleMat(gravity_matrix, 8, 8);
      MNulleVec(gravity_vector, 8);
	  r=s=t=0.0;
      /* Schwerkraftterm berechnen (gravity vector) */
	  gp[0]=r; gp[1]=s; gp[2]=t;
      mpc_density = m_mfp->Density();
      for (l=0;l<8;l++)
        elevation[l] = ((GetNode(element_nodes[l]))->z) * mpc_density * g;
      MMultMatVec(conductance_matrix, 8, 8, elevation, 8, gravity_vector, 8);
      MPCSetELEGravityVector(index,phase,gravity_vector);
#ifdef TEST_CEL_MPC
      MZeigVec(gravity_vector,nn, "MPCCalcElementMatrix3D: gravity vector");
#endif
	   break;
    /*========================================================================*/
    case 3:
      storativity_pressure_matrix = MPCGetELEStorativityPressureMatrix(index,phase);
      if (!storativity_pressure_matrix)
        storativity_pressure_matrix = (double *) Malloc(nn2* sizeof(double));
      /* Initialisieren */
      MNulleMat(storativity_pressure_matrix,nn,nn);
      /* in CGS */
      /*-----------------------------------------------------*/
      for (i = 0; i < anzgp; i++)
        for (j = 0; j < anzgp; j++)
          for (k = 0; k < anzgp; k++){
            r = MXPGaussPkt(anzgp, i);
            s = MXPGaussPkt(anzgp, j);
		    t = MXPGaussPkt(anzgp, k);
            gp[0]=r; gp[1]=s; gp[2]=t;
            Calc3DElementJacobiMatrix(index, r, s,t, invjac, &detjac);
            /* fkt = n*S^p*X_k^p*drho^p/dp^p */
#ifdef noCHARACTERISTIC_NUMBERS
            mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
            mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
            mpc_drho_dp = m_mfp->drho_dp;
            mpc_saturation = MMax(0.,MPCGetSaturation(phase,index,0.,0.,0.,mpc_time_collocation_global));
            fac_mat = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
#endif
#ifdef CHARACTERISTIC_NUMBERS
            fac_mat = MPCCalcCompressibilityNumber(index,phase,comp,gp);
#endif
            /* Gauss-Faktoren g_i * g_j  * g_k * det[J_2D] */
            fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * detjac;
            /* Ansatzfunktion im Gauss-Punkt omega==N */
            MOmega3D(ome, r, s, t);
/*
            // Normale FEM 
            //Wichtungsfunktionen im Gauss-Punkt phi==N 
            MPhi3D(phi, r, s, t);
            // Berechnen phi * omega 
            MMultVecVec(phi, 8, ome, 8, zwi, 8, 8);
            for (l = 0; l < nn2; l++)
              storativity_pressure_matrix[l] += zwi[l] * fac_mat * fac_geo;*/
			//WW
            for(ii=0; ii<nn; ii++)
            {
              for(jj=0; jj<nn; jj++)
                storativity_pressure_matrix[ii*nn+jj] += ome[ii]*ome[jj]* fac_mat * fac_geo;
			}
      }                              /* Ende der Schleife ueber die Gauss-Punkte */
      MPCSetELEStorativityPressureMatrix(index,phase,storativity_pressure_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(storativity_pressure_matrix,nn,nn,"MPCCalcElementMatrix3D: storativity_pressure_matrix");
#endif
      break;
    /*========================================================================*/
    case 4: /* Component diffusion matrix */
      mpc_diffusion_matrix = MPCGetELEDiffusionMatrix(index,phase);
      if (!mpc_diffusion_matrix)
        mpc_diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
	  MNulleMat(mpc_diffusion_matrix,nn,nn);
      for (i = 0; i < anzgp; i++)
          for (j = 0; j < anzgp; j++)
            for (k = 0; k < anzgp; k++){
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
			  t = MXPGaussPkt(anzgp, k);
              gp[0]=r; gp[1]=s; gp[2]=t;
       /* Materials */
#ifdef noCHARACTERISTIC_NUMBERS
        mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
        mpc_saturation = MMax(0., MPCGetSaturation(phase,index,gp[0],gp[1],gp[2],mpc_time_collocation_global));
        mpc_density = m_mfp->Density();
        fac_mat = mpc_porosity * mpc_saturation * mpc_density;
#endif
#ifdef CHARACTERISTIC_NUMBERS
             fac_mat = MPCCalcDiffusivityNumber(index,phase,comp,gp);
			 //fac_mat /= m_mfp->diffusion; //MATGetPhaseDiffusion(phase);
#endif
	        MNulleMat(dif, 3, 3);
            dif[0] = 1.; //MATGetPhaseDiffusion(phase);
            dif[4] = 1.; //MATGetPhaseDiffusion(phase);
            dif[8] = 1.; //MATGetPhaseDiffusion(phase);
			dif[1] = dif[2] = dif[3]= dif[5]=dif[6]=dif[7]=0.0;
/* // JDJ alignment to 1 and 2 D matrices
            //Kopie ASM
			Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
                // Faktor
                fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * \
                    MXPGaussFkt(anzgp, k) * detjac;

                // tkmyt = (J^-1)T * (K/my) * J^-1
                MMultMatMat(dif, 3, 3, invjac, 3, 3, zwa, 3, 3);
                MTranspoMat(invjac, 3, 3, zwi);
                MMultMatMat(zwi, 3, 3, zwa, 3, 3, tkmyt, 3, 3);
                // conductance matrix und gravity matrix
                //grad(phi)T
                MGradPhi3D(zwa, r, s, t);       // 3 Zeilen 8 Spalten
                MTranspoMat(zwa, 3, 8, zwi);    // jetzt: 8 Zeilen 3 Spalten
                // grad(phi)T * tkmyt
                MMultMatMat(zwi, 8, 3, tkmyt, 3, 3, zwa, 8, 3);
                //grad(omega)
                MGradOmega3D(ome, r, s, t);     // 3 Zeilen 8 Spalten 
                // grad(phi)T * tkmyt * grad(omega) 
                MMultMatMat(zwa, 8, 3, ome, 3, 8, zwi, 8, 8);
              for (l=0;l<nn2;l++)
                  mpc_diffusion_matrix[l] = (zwi[l] * fac_geo * fac_mat);*/
			   /* Calculate: grad(phi) * D^g * grad(ome) */
              /* J^-1 */
              Calc3DElementJacobiMatrix(index, r, s,t, invjac, &detjac);
              /* Faktor */
              fac_geo = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * detjac;
              /* tkmyt = J^-1 * (k) * (J^-1)T */
              MMultMatMat(dif, dim, dim, invjac, dim, dim, zwa, dim, dim);
              MTranspoMat(invjac, dim, dim, zwi);
              MMultMatMat(zwi, dim, dim, zwa, dim, dim, tkt, dim, dim);
              // grad(phi)T 
              MGradPhi3D(ome, r, s, t); 
  			  // WW
              for(ii=0; ii<nn; ii++)
			  {
                 for(jj=0; jj<nn; jj++)					  
				 {
                    Var = 0.0;
				    for(kk=0; kk<dim; kk++)
					{
     			       for(ll=0; ll<dim; ll++)
				       {
                           Var += ome[kk*nn+ii]*tkt[dim*kk+ll]*ome[ll*nn+jj];
				       }
					}
                    mpc_diffusion_matrix[ii*nn+jj] += Var * fac_geo * fac_mat;
				 }
			  }
                                    
       } /* Ende der Schleife ueber GaussPunkte */
      MPCSetELEDiffusionMatrix(index,phase,mpc_diffusion_matrix);
#ifdef TEST_CEL_MPC
      MZeigMat(mpc_diffusion_matrix,nn,nn,"MPCCalcElementMatrix3D: mpc_diffusion_matrix");
#endif
      break;
	  /*The following has to be checked either by WW or OK JDJ 6.3.2004*/
    //--------------------------------------------------------------------
    case 5:                            /* JDJ 3909okfjdj5 */
      if(GetRFProcessProcessing("SD")) {
      Init_Quadratic_Elements();
      /* Dilatation coupling matrix */
      coupling_matrix_u_x = MPCGetELEStrainCouplingMatrixX(index,phase);
      if (!coupling_matrix_u_x)
        coupling_matrix_u_x = (double *) Malloc(nn*20* sizeof(double));
      coupling_matrix_u_y = MPCGetELEStrainCouplingMatrixY(index,phase);
      if (!coupling_matrix_u_y)
        coupling_matrix_u_y = (double *) Malloc(nn*20* sizeof(double));
      coupling_matrix_u_z = MPCGetELEStrainCouplingMatrixY(index,phase);
      if (!coupling_matrix_u_z)
        coupling_matrix_u_z = (double *) Malloc(nn*20* sizeof(double));
      MNulleMat(coupling_matrix_u_x,nn,20);
      MNulleMat(coupling_matrix_u_y,nn,20);
      MNulleMat(coupling_matrix_u_z,nn,20);
      for (i = 0; i < anzgp; i++)
          for (j = 0; j < anzgp; j++)
            for (k = 0; k < anzgp; k++){
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
              t = MXPGaussPkt(anzgp, k);
              Calc3DElementJacobiMatrix_20N(index,r,s,t,invjac_t_20N,&detjac_20N);
              MGradOmega3D_20N(g_ome_t_20N,r,s,t); /* grad(ome)^T - 3 Zeilen 20 Spalten */
              /* J^-T * grad(ome) */
              MMultMatMat(invjac_t_20N,3,3,g_ome_t_20N,3,20,g_ome_xyz_t_20N,3,20);
              /* Faktor g_i * g_j  * g_k * det[J_2D] */
              fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j)* MXPGaussFkt(anzgp, k) * detjac;
              /* fkt *= S^p * rho^p */
              //OK fkt *= MPCGetSaturation(phase, index, r, s, t, mpc_time_collocation_global);
              mpc_saturation = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
              fkt *= mpc_saturation;
              fkt *= m_mfp->Density();
              for (l=0;l<9;l++)      /* 9 Zeilen  */
                for (m=0;m<20;m++) {    /* 20 Spalten */
                   kl = (20*l)+m;      
                    coupling_matrix_u_x[kl] += ome[l] * g_ome_xyz_t_20N[m]   * fkt;  
                    coupling_matrix_u_y[kl] += ome[l] * g_ome_xyz_t_20N[20+m] * fkt;
                    coupling_matrix_u_z[kl] += ome[l] * g_ome_xyz_t_20N[2*20+m] * fkt;  
                }
        }                              /* Ende der Schleife ueber die Gauss-Punkte */
        MPCSetELEStrainCouplingMatrixX(index,phase,coupling_matrix_u_x);
        MPCSetELEStrainCouplingMatrixY(index,phase,coupling_matrix_u_y);
        MPCSetELEStrainCouplingMatrixZ(index,phase,coupling_matrix_u_z);
#ifdef TEST_CEL_MPC
      MZeigMat(coupling_matrix_u_x,nn,nn, "MPCStrainCouplingMatrixX");
      MZeigMat(coupling_matrix_u_y,nn,nn, "MPCStrainCouplingMatrixY");
      MZeigMat(coupling_matrix_u_z,nn,nn, "MPCStrainCouplingMatrixZ");
#endif
        Init_Linear_Elements();
   }
      break;
    /*========================================================================*/
    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mpc.c: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK  Implementation
04/2003 JDJ Prohibit div by 0
08/2004 OK  MFP
**************************************************************************/
void MPCCalcCharacteristicNumbers(long index,int phase,int comp)
{
  static int i;
  static double pressure_average,saturation_average;
  static int timelevel;
  static double compressibility_number,neumann_number;
  static double diffusivity_number;
  static double dt,dx;
  static double *invjac, detjac;
  static double A, L, r;
  static long *element_nodes;
  static int nn=2;
  static double compressibility_number0,neumann_number0;
  static double compressibility_number1,neumann_number1;
  double mpc_dS_dpc;
  double gp[3];
  //----------------------------------------------------------------------
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcCharacteristicNumbers: no time discretization data !" << endl;
  dt = m_tim->CalcTimeStep();
  //----------------------------------------------------------------------
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  A = m_mmp->geo_area;
  L = 2. * detjac / A;
  dx = L;
  r=0.0; /* full upwind */
  gp[0]=r, gp[1]=0.0, gp[2]=0.0;
  //----------------------------------------------------------------------
  timelevel=1;
  pressure_average = saturation_average = 0.0;
  for (i=0;i<nn;i++) {
    pressure_average   += MPCGetNodePressure(element_nodes[i],phase,timelevel);
    saturation_average += MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  pressure_average   /= nn;
  saturation_average /= nn;
  /*--------------------------------------------------------------------------*/
  /* Materials */
  /*--------------------------------------------------------------------------*/
  /* Compressibility number */
  compressibility_number = MPCCalcCompressibilityNumber(index,phase,comp,gp);
  MPCSetElementCompressibilityNumber(index,phase,comp,compressibility_number);
  phase=0;
  compressibility_number0 = MPCCalcCompressibilityNumber(index,phase,comp,gp);
  phase=1;
  compressibility_number1 = MPCCalcCompressibilityNumber(index,phase,comp,gp);
  /*--------------------------------------------------------------------------*/
  /* Neumann number */
  neumann_number = MPCCalcNeumannNumber(index,phase,comp,gp);
  neumann_number *= dt/(dx*dx);
  MPCSetElementConductivityNumber(index,phase,comp,neumann_number);
  phase=0;
  neumann_number0 = MPCCalcNeumannNumber(index,phase,comp,gp);
  phase=1;
  neumann_number1 = MPCCalcNeumannNumber(index,phase,comp,gp);
  neumann_number = (neumann_number0+neumann_number1)/(compressibility_number0+compressibility_number1);
  neumann_number *= dt/(dx*dx);
  MPCSetElementConductivityNumber(index,0,comp,neumann_number);
  MPCSetElementConductivityNumber(index,1,comp,neumann_number);
  /*--------------------------------------------------------------------------*/
  /* Neumann number / Richards model */
if((GetRFProcessFlowModel()==10)||(GetRFProcessFlowModel()==11)) {
  mpc_dS_dpc = MMPGetSaturationPressureDependency(phase,index,0.,0.,0.,mpc_time_collocation_global);
  neumann_number = mpc_permeability_rel * mpc_permeability_ref \
                      / (mpc_viscosity * mpc_porosity) \
                      / mpc_dS_dpc \
                      * dt / (dx*dx);
  MPCSetElementConductivityNumber(index,phase,comp,neumann_number);
}
  /*--------------------------------------------------------------------------*/
  /* Diffusivity number */
  diffusivity_number = MPCCalcStorativityNumber(index,phase,comp,gp);
  MPCSetElementDiffusivityNumber(index,phase,comp,diffusivity_number);
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK Implementation
08/2004 OK MFP
**************************************************************************/
double MPCCalcCompressibilityNumber(long index,int phase,int comp,double *gp)
{
  double term1, term2;
  double compressibility_number;
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  if(!m_mfp)
    return -1.;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  //----------------------------------------------------------------------
  /* phase=gas, term1 = n*S^p*X_k^p*drho^p/dp^p */
  //OK mpc_saturation = MMax(0.,MPCGetSaturation(phase,index,gp[0],gp[1],gp[2],mpc_time_collocation_global));
  char fct_name[15];
  sprintf(fct_name,"SATURATION%i",phase+1);
  mpc_saturation = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
  mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  mpc_drho_dp = m_mfp->drho_dp;
  term1 = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
  /* ------------------------------------------------------------------------ */
  /* reference number */
  term2 = MPCCalcReferenceNumber(index,phase,comp,gp);
  /* ------------------------------------------------------------------------ */
  if(fabs(term2)< MKleinsteZahl){//WW
    compressibility_number = 0.0;
  }
  else
    compressibility_number = term1 / term2;
  return compressibility_number;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK Implementation
08/2004 OK MFP
**************************************************************************/
double MPCCalcNeumannNumber(long index,int phase,int comp,double *gp)
{
  double term1, term2;
  double* permeability;
  double neumann_number;
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  //if(!m_mfp) return -1.;
  mpc_viscosity = m_mfp->Viscosity();
  mpc_density = m_mfp->Density();
  mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  permeability = m_mmp->PermeabilityTensor(index);
  mpc_permeability_ref = permeability[0];
  mpc_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mpc_time_collocation_global,phase);
  //----------------------------------------------------------------------
  /* term1 = rho^p*X_k^p * k*kr^p/mue^p */
  term1 = mpc_permeability_ref * mpc_permeability_rel * mpc_density * mpc_mass_fraction / mpc_viscosity;
  /* ------------------------------------------------------------------------ */
  /* reference number */
  term2 = MPCCalcReferenceNumber(index,phase,comp,gp);
  /* ------------------------------------------------------------------------ */
  
  if(fabs(term2)< MKleinsteZahl){
    neumann_number = 0.0;
  }
  else
  neumann_number = term1 / term2;
  /* ------------------------------------------------------------------------ */
  return neumann_number;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK Implementation
08/2004 OK MFP
**************************************************************************/
double MPCCalcStorativityNumber(long index,int phase,int comp,double *gp)
{
  double term1, term2;
  double storativity_number;
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  if(!m_mfp)
    return -1.;
  mpc_density = m_mfp->Density();
  mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global); 
  //----------------------------------------------------------------------
  /* term1 = n*rho^phase*X_comp^phase */
  term1 = mpc_porosity * mpc_density * mpc_mass_fraction;
  if(phase==0) {
    term1 *= -1.0;
  }
  /* ------------------------------------------------------------------------ */
  /* reference number */
  term2 = MPCCalcReferenceNumber(index,phase,comp,gp);
  /* ------------------------------------------------------------------------ */
  
  if(fabs(term2)< MKleinsteZahl ){
    storativity_number = 0.0;
  }
  else
  storativity_number = term1 / term2;
  /* ------------------------------------------------------------------------ */
  return storativity_number;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK Implementation
08/2004 OK MFP
**************************************************************************/
double MPCCalcDiffusivityNumber(long index,int phase,int comp,double *gp)
{
  double term1, term2;
  double diffusivity_number;
  int nidx1=-1;
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  if(!m_mfp)
    return -1.;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  //----------------------------------------------------------------------
  /* fkt = n*S^p*rho^p*X_k^p*D_k^p */
  //OK mpc_saturation = MMax(0.,MPCGetSaturation(phase,index,gp[0],gp[1],gp[2],mpc_time_collocation_global));
  char fct_name[15];
  sprintf(fct_name,"SATURATION%i",phase+1);
  mpc_saturation = MMax(0.,PCSGetELEValue(index,gp,mpc_time_collocation_global,fct_name));
  if(phase==0)
    nidx1 = PCSGetNODValueIndex("SATURATION1",1);
  else if(phase==1)
    nidx1 = PCSGetNODValueIndex("SATURATION2",1);
  mpc_saturation = InterpolValue(index,nidx1,gp[0],gp[1],gp[2]); //GetNodeVal(index,nidx1);
  mpc_density = m_mfp->Density();
  mpc_mass_fraction = m_mfp->MassFraction(index,comp,gp,mpc_time_collocation_global);
  //SB:2p mpc_diffusion_coefficient = CalcTracerDiffusionCoefficient(index, phase ,comp);
  mpc_diffusion_coefficient = m_mfp->diffusion; //MATGetPhaseDiffusion(phase);
  term1 = mpc_porosity * mpc_saturation * mpc_density * mpc_mass_fraction * mpc_diffusion_coefficient;
  /* ------------------------------------------------------------------------ */
  /* reference number */
  term2 = MPCCalcReferenceNumber(index,phase,comp,gp);  //2.1e-6;
  /* ------------------------------------------------------------------------ */
  
  if(fabs(term2)< MKleinsteZahl){
    diffusivity_number = 0.0;
  }
  else
  diffusivity_number = term1 / term2;
  /* ------------------------------------------------------------------------ */
  return diffusivity_number;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2002 OK Implementation
08/2004 OK MFP
**************************************************************************/
double MPCCalcReferenceNumber(long index,int phase,int comp,double *gp)
{
  double reference_number;
  int this_phase=-1, this_comp=-1;
if(phase==1)
  phase=phase;  
  //----------------------------------------------------------------------
  // Fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  if(!m_mfp)
    return -1.;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  mpc_porosity = m_mmp->Porosity(index,gp,mpc_time_collocation_global);
  //----------------------------------------------------------------------
if(comp==0) {
  this_phase = 0, this_comp = 0;
}
else if(comp==1) {
  this_phase = 1, this_comp = 1;
}
  //----------------------------------------------------------------------
  // n*S^p*X_k^p*drho^p/dp^p
  double mpc_saturation = 1.0;
/*???
  mpc_saturation = MMax(0., MPCGetSaturation(this_phase,index,\
                                             gp[0],gp[1],gp[2],\
                                             mpc_time_collocation_global));
*/
  mpc_mass_fraction = mfp_vector[this_phase]->MassFraction(index,this_comp,gp,mpc_time_collocation_global);
                     //m_mfp->MassFraction(index,this_comp,gp,mpc_time_collocation_global);
  mpc_drho_dp = mfp_vector[this_phase]->drho_dp; //m_mfp->drho_dp;
  reference_number = mpc_porosity * mpc_saturation * mpc_mass_fraction * mpc_drho_dp;
  //----------------------------------------------------------------------
  return reference_number;
}

/*** KERNEL MPC ***********************************************************/
/*------------------------------------------------------------------------*/
/* ElementCapacitanceMatrix */
void MPCSetELECapacitanceMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_capacitance_matrix_phase[phase] = matrix;
}
double *MPCGetELECapacitanceMatrix(long number,int phase)
{
 return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_capacitance_matrix_phase[phase];
}
/*------------------------------------------------------------------------*/
/* ElementConductanceMatrix */
void MPCSetELEConductivityMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_conductivity_matrix_phase[phase] = matrix;
}
double *MPCGetELEConductivityMatrix(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_conductivity_matrix_phase[phase];
}
/*------------------------------------------------------------------------*/
/* ElementGravityVector */
void MPCSetELEGravityVector(long number, int phase,double *vec)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_gravity_vector_phase[phase] = vec;
}
double *MPCGetELEGravityVector(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_gravity_vector_phase[phase];
}
/*------------------------------------------------------------------------*/
/* ElementStorativityPressureMatrix */
double *MPCGetELEStorativityPressureMatrix(long number,int phase)
{
  return ((MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number))->mpc_capillarity_matrix[phase];
}
void MPCSetELEStorativityPressureMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_capillarity_matrix[phase] = matrix;
}
/*------------------------------------------------------------------------*/
/* ElementStorativityTemperatureMatrix */
double *MPCGetELEStorativityTemperatureMatrix(long number,int phase)
{
  return ((MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number))->mpc_storativity_temperature_matrix[phase];
}
void MPCSetELEStorativityTemperatureMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_storativity_temperature_matrix[phase] = matrix;
}
/*------------------------------------------------------------------------*/
/* ElementDiffusionMatrix */
void MPCSetELEDiffusionMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_diffusion_matrix[phase] = matrix;
}
double *MPCGetELEDiffusionMatrix(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_diffusion_matrix[phase];
}
/*------------------------------------------------------------------------*/
/* ElementThermoComponentalDiffusionMatrix */
void MPCSetELEThermoCompDiffusionMatrix(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_diffusion_thermo_comp_matrix[phase] = matrix;
}
double *MPCGetELEThermoCompDiffusionMatrix(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_diffusion_thermo_comp_matrix[phase];
}
/*------------------------------------------------------------------------*/
/* ElementStrainCouplingMatrixX */
void MPCSetELEStrainCouplingMatrixX(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_strain_coupling_matrix_x[phase] = matrix;
}
double *MPCGetELEStrainCouplingMatrixX(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_strain_coupling_matrix_x[phase];
}
/*------------------------------------------------------------------------*/
/* ElementStrainCouplingMatrixY */
void MPCSetELEStrainCouplingMatrixY(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_strain_coupling_matrix_y[phase] = matrix;
}
double *MPCGetELEStrainCouplingMatrixY(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_strain_coupling_matrix_y[phase];
}

/*------------------------------------------------------------------------*/
/* ElementStrainCouplingMatrixZ */
void MPCSetELEStrainCouplingMatrixZ(long number,int phase,double *matrix)
{
  static MPCElementMatrices *data;
  data = (MPCElementMatrices *) ELEGetElementMatrices(number,mpc_pcs_number);
  data->mpc_strain_coupling_matrix_z[phase] = matrix;
}
double *MPCGetELEStrainCouplingMatrixZ(long number,int phase)
{
  return ((MPCElementMatrices*)ELEGetElementMatrices(number,mpc_pcs_number))->mpc_strain_coupling_matrix_z[phase];
}
/*************************************************************************
ROCKFLOW - Function: MPCCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *MPCCreateELEMatricesPointer(void)
{ 
  MPCElementMatrices *data = NULL;
  int i;
  data = (MPCElementMatrices *) Malloc(sizeof(MPCElementMatrices));
  data->mpc_conductivity_matrix_phase = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_capacitance_matrix_phase = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_gravity_vector_phase = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_diffusion_matrix = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_capillarity_matrix = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_strain_coupling_matrix_x = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_strain_coupling_matrix_y = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  data->mpc_strain_coupling_matrix_z = \
       (double **) Malloc(sizeof(double *) * GetRFProcessNumPhases());
  for (i=0;i<GetRFProcessNumPhases();i++) {
    data->mpc_conductivity_matrix_phase[i] = NULL;
    data->mpc_capacitance_matrix_phase[i] = NULL;
    data->mpc_gravity_vector_phase[i] = NULL;
    data->mpc_diffusion_matrix[i] = NULL;
    data->mpc_capillarity_matrix[i] = NULL;
    data->mpc_strain_coupling_matrix_x[i] = NULL;
    data->mpc_strain_coupling_matrix_y[i] = NULL;
    data->mpc_strain_coupling_matrix_z[i] = NULL;
  }
  return (void *) data;
}

/*************************************************************************
ROCKFLOW - Function: MPCCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *MPCDestroyELEMatricesPointer(void *data)
{
  int i;
  for (i=0;i<GetRFProcessNumPhases();i++) {
    if (((MPCElementMatrices *) data)->mpc_diffusion_matrix[i])
        ((MPCElementMatrices *) data)->mpc_diffusion_matrix[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_diffusion_matrix[i]);
    if (((MPCElementMatrices *) data)->mpc_capillarity_matrix[i])
        ((MPCElementMatrices *) data)->mpc_capillarity_matrix[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_capillarity_matrix[i]);
    if (((MPCElementMatrices *) data)->mpc_gravity_vector_phase[i])
        ((MPCElementMatrices *) data)->mpc_gravity_vector_phase[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_gravity_vector_phase[i]);
    if (((MPCElementMatrices *) data)->mpc_capacitance_matrix_phase[i])
        ((MPCElementMatrices *) data)->mpc_capacitance_matrix_phase[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_capacitance_matrix_phase[i]);
    if (((MPCElementMatrices *) data)->mpc_conductivity_matrix_phase[i])
        ((MPCElementMatrices *) data)->mpc_conductivity_matrix_phase[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_conductivity_matrix_phase[i]);
    if (((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_x[i])
        ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_x[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_x[i]);
    if (((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_y[i])
        ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_y[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_y[i]);
    if (((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_z[i])
        ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_z[i] = \
         (double *) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_z[i]);
  }
  ((MPCElementMatrices *) data)->mpc_diffusion_matrix = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_diffusion_matrix);
  ((MPCElementMatrices *) data)->mpc_capillarity_matrix = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_capillarity_matrix);
  ((MPCElementMatrices *) data)->mpc_gravity_vector_phase = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_gravity_vector_phase);
  ((MPCElementMatrices *) data)->mpc_capacitance_matrix_phase = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_capacitance_matrix_phase);
  ((MPCElementMatrices *) data)->mpc_conductivity_matrix_phase = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_conductivity_matrix_phase);
  ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_x = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_x);
  ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_y = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_y);
  ((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_z = \
                     (double **) Free(((MPCElementMatrices *) data)->mpc_strain_coupling_matrix_z);

  data = (void*) Free(data);
  return data;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
double MPCGetNodePressure(long node,int phase,int timelevel)
{
  double pressure = 1e5;
  switch(phase){
    case 0:
      pressure = PCSGetNODValue(node,"PRESSURE1",timelevel);
      break;
    case 1:
      pressure = PCSGetNODValue(node,"PRESSURE2",timelevel);
      break;
  }
  return pressure;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
double MPCGetNodeSaturation(long node,int phase,int timelevel)
{
  double saturation = 0.0;
  switch(phase){
    case 0:
      saturation = PCSGetNODValue(node,"SATURATION1",timelevel);
      break;
    case 1:
      saturation = PCSGetNODValue(node,"SATURATION2",timelevel);
      break;
  }
  return saturation;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
double MPCGetTemperatureNode(long node,int comp,int timelevel)
{
  double temperature;
  comp=comp;
  temperature = PCSGetNODValue(node,"TEMPERATURE1",timelevel);
  return temperature;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
double MPCGetNodeMassFraction(long node,int comp,int timelevel)
{
  double mass_fraction = 0.0;
  switch(comp){
    case 0:
      mass_fraction = PCSGetNODValue(node,"MASS_FRACTION1",timelevel);
      break;
    case 1:
      mass_fraction = PCSGetNODValue(node,"MASS_FRACTION2",timelevel);
      break;
  }
  return mass_fraction;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
double MPCGetNodeCapillaryPressure(long node,int timelevel)
{
  return PCSGetNODValue(node,"PRESSURE_CAP",timelevel);
}

