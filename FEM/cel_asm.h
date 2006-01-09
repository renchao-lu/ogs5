/**************************************************************************/
/* ROCKFLOW - Modul: cel_asm.h
                                                                          */
/* Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer das Modul aSM.
   05/2003 OK SMElementMatrices (PCS element matrices)
                                                                          */
/**************************************************************************/

#ifndef cel_asm_INC

#define cel_asm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "rf_pcs.h"

/* Deklarationen */
  /* berechnet stationaere Matrizenanteile eines Elements */
extern void StatMat_ASM ( long index);
  /* berechnet instationaere Matrizenanteile eines Elements */
extern void CalcElementMatrix_ASM ( long index );
extern void CalcElementMatrix_ASM_New(long index,\
                               double *capacitance_matrix_cgs,\
                               double *conductance_matrix_cgs,\
                               double *gravity_vector_cgs);

extern void CalcEle1D_ASM (long index,double *matrix1,double *matrix2,double *matrix3,CRFProcess*);
extern void CalcEle2D_ASM (long index,double *matrix1,double *matrix2,double *matrix3,CRFProcess*);
extern void CalcEle3D_ASM (long index,double *matrix1,double *matrix2,double *matrix3,CRFProcess*);
extern void CalcEle2DTriangle_ASM(long,double*,double*,double*,CRFProcess*);
extern void CalcEle2DTriangle_OF(long,double*,double*,double*,CRFProcess*);

extern void CalcEle2DQuad_OF(long,double*,double*,double*,CRFProcess*);
extern void CalcEle3DTetraeder_ASM(long,CRFProcess*);
extern void CalcEle3DPrism_ASM_num(long,double*,double*,double*,CRFProcess*);
extern void CalcEle3DPrism_ASM_ana(long,double*,double*,double*,CRFProcess*);

/* Element matrices */
typedef struct {
  double *conductancematrix;    /* conductance matrix des Elements */
  double *capacitancematrix;    /* capacitance matrix des Elements */
  double *gravityvector;        /* gravity vector des Elements */
  double *strain_coupling_matrix_x;
  double *strain_coupling_matrix_y;
  double *strain_coupling_matrix_z;
} SMElementMatrices;

extern void *SMCreateELEMatricesPointer(void);
extern void *SMDestroyELEMatricesPointer(void *data);

extern void SMSetElementCapacitanceMatrix(long number,double *matrix,int);
extern double *SMGetElementCapacitanceMatrix(long number,int);
extern void SMSetElementConductanceMatrix(long number,double *matrix,int);
extern double *SMGetElementConductanceMatrix(long number,int);
extern void SMSetElementGravityVector(long number,double *matrix,int);
extern double *SMGetElementGravityVector(long number,int);
extern void SMSetElementStrainCouplingMatrixX(long number,double *matrix,int);
extern void SMSetElementStrainCouplingMatrixY(long number,double *matrix,int);
extern void SMSetElementStrainCouplingMatrixZ(long number,double *matrix,int);
extern double *SMGetElementStrainCouplingMatrixX(long number,int);
extern double *SMGetElementStrainCouplingMatrixY(long number,int);
extern double *SMGetElementStrainCouplingMatrixZ(long number,int);

extern void SMCalcElementMatrix(long,CRFProcess*);
extern void OFCalcElementMatrix(long,CRFProcess*);

#endif
