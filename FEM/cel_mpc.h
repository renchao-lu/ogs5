/**************************************************************************
 ROCKFLOW - Modul: cel_mpc.h

 Task:
 Interface of FEM-Kernels MPC
 Programming:
 05/2003 OK MPCElementMatrices
 **************************************************************************/
#ifndef cel_mpc_INC
#define cel_mpc_INC
  /* Schutz gegen mehrfaches Einfuegen */
#include "rf_pcs.h"
extern void MPCSetELEConductivityMatrix(long number,int phase,double *matrix);
extern double *MPCGetELEConductivityMatrix(long number,int phase);
extern void MPCSetELECapacitanceMatrix(long number,int phase,double *matrix);
extern double *MPCGetELECapacitanceMatrix(long number,int phase);
extern void MPCSetELEGravityVector(long number, int phase,double *vec);
extern double *MPCGetELEGravityVector(long number,int phase);
extern double *MPCGetELEStorativityPressureMatrix(long number,int phase);
extern void MPCSetELEStorativityPressureMatrix(long number,int phase,double *matrix);
extern void MPCSetELEDiffusionMatrix(long number,int phase,double *matrix);
extern double *MPCGetELEDiffusionMatrix(long number,int phase);
extern void MPCSetELEStrainCouplingMatrixX(long number,int phase,double *matrix);
extern double *MPCGetELEStrainCouplingMatrixX(long number,int phase);
extern void MPCSetELEStrainCouplingMatrixY(long number,int phase,double *matrix);
extern double *MPCGetELEStrainCouplingMatrixY(long number,int phase);
extern void MPCSetELEStrainCouplingMatrixZ(long number,int phase,double *matrix);
extern double *MPCGetELEStrainCouplingMatrixZ(long number,int phase);
extern double *MPCGetELEStorativityTemperatureMatrix(long number,int phase);
extern void MPCSetELEStorativityTemperatureMatrix(long number,int phase,double *matrix);
extern void MPCSetELEThermoCompDiffusionMatrix(long number,int phase,double *matrix);
extern double *MPCGetELEThermoCompDiffusionMatrix(long number,int phase);

extern void MPCCalculateElementMatrices(CRFProcess*);
extern void *MPCCreateELEMatricesPointer(void);
extern void *MPCDestroyELEMatricesPointer(void *data);
extern void MPCCalculateElementMatricesRichards(void);

extern void *MPCCreateELEMatricesPointer(void);
extern void *MPCDestroyELEMatricesPointer(void *data);
/* element matrices */
typedef struct {
    double **mpc_conductivity_matrix_phase; /* Leitfaehigkeitsmatrizen fuer alle beweglichen Phasen */
    double **mpc_capacitance_matrix_phase;  /* Kompressibiliaetsmatrizen fuer alle beweglichen Phasen */
    double **mpc_capillarity_matrix;        /* Kapillardruckvektoren fuer alle beweglichen Phasen */
    double **mpc_diffusion_matrix;          /* diffusion matrix */
    double **mpc_gravity_vector_phase;      /* gravity vector des Elements */
    double **mpc_strain_coupling_matrix_x;
    double **mpc_strain_coupling_matrix_y;
    double **mpc_strain_coupling_matrix_z;
	double **mpc_storativity_temperature_matrix;
	double **mpc_diffusion_thermo_comp_matrix;
} MPCElementMatrices;
/* Node values */
extern double MPCGetNodePressure(long node,int phase,int timelevel);
extern double MPCGetNodeMassFraction(long node,int comp,int timelevel);
extern double MPCGetNodeSaturation(long node,int phase,int timelevel);
extern double MPCGetTemperatureNode(long node,int comp,int timelevel);
extern double MPCGetNodeCapillaryPressure(long node,int timelevel);
/* Element values */
extern VoidFuncLIID MPCSetElementCompressibilityNumber;
extern VoidFuncLIID MPCSetElementConductivityNumber;
extern VoidFuncLIID MPCSetElementDiffusivityNumber;
#endif
