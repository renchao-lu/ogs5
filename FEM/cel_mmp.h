/**************************************************************************
 ROCKFLOW - Modul: cel_mmp.h

 Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer das Modul MMP.
 05/2003 OK MMPElementMatrices

*************************************************************************/
#ifndef cel_mmp_INC
#define cel_mmp_INC
#include "rf_pcs.h"
/* Initialisiert den Kernel */
extern int InitializeKernel_MMP(void);
/* Initialisiert die durch ein Modell benutzten Matrizentypen */
extern int InitializeMatrixtype_MMP(char *name, int phase, int component);
/* Zerstoert den Kernel */
extern int DestroyKernel_MMP(void);
/* Gibt die Anzahl der registrierten Matrizentypen zurueck */
extern int GetNumberOfRegisteredMatrixtypes_MMP(void);
/* Geben Kernelkonfigurationsdaten zurueck */
extern double GetTimeCollocationGlobal_MMP(void);
extern double GetTimeCollocationCondBC_MMP(void);
extern double GetTimeCollocationSource_MMP(void);
extern double GetTimeCollocationUpwinding_MMP(void);
extern double GetTimeCollocationOpenBoundary_MMP(void);
extern int GetValuesExtractMethod_MMP(void);
extern int GetMethod_MMP(void);
extern int GetNonLinearCoupling_MMP(void);
extern int GetPredictorMethod_MMP(void);
extern double GetPredictorParam_MMP(void);
extern int GetRelaxationMethod_MMP(void);
extern double *GetRelaxationParam_MMP(void);
extern int GetReferencePhase_MMP(void);

/* Bedingter Matrix-Wiederaufbau */
extern void ConditionalRebuildMatrices_MMP(int phase);

/* Berechnet alle Elementmatrizen neu */
extern void BuildElementMatrices_MMP(long index);

/* Berechnet Elemenmatrizen fuer Richardsansatz neu */
extern void RebuildMatricesRichards_MMP(void);

/* Matrizenanteile eines Elements */
extern void CalcElementMatrix_MMP(long index, int matrixtyp, int phase,CRFProcess*);
extern void CalcEle1D_MMP(long index, int matrixtyp, int phase,CRFProcess*);
extern void CalcEle2D_MMP(long index, int matrixtyp, int phase,CRFProcess*);
extern void CalcEle3D_MMP(long index, int matrixtyp, int phase,CRFProcess*);
extern void MMPCalcElementMatrices2DTriangle(long index, int phase, int matrixtyp,CRFProcess*);

/* Weitere externe Objekte */
#define noEFFORT

#ifdef EFFORT
extern int mmp_effort_index;
extern int mmp_effort_total_index;
#endif

/* Element matrices */
typedef struct {
  double **mmp_conductivity_matrix_phase; /* Leitfaehigkeitsmatrizen fuer alle beweglichen Phasen */
  double **mmp_capacitance_matrix_phase;  /* Kompressibiliaetsmatrizen fuer alle beweglichen Phasen */
  double **mmp_gravity_vector_phase;      /* gravity vector des Elements */
  double **mmp_capillarity_vector_phase;  /* Kapillardruckvektoren fuer alle beweglichen Phasen */
  double **mmp_capillarity_matrix;  /* Kapillardruckvektoren fuer alle beweglichen Phasen */
  double **mmp_strain_coupling_matrix_x;
  double **mmp_strain_coupling_matrix_y;
  double **mmp_strain_coupling_matrix_z;
} MMPElementMatrices;

extern void MMPSetElementCapacitanceMatrixPhase_MMP(long number, int phase, double *matrix);
extern double *MMPGetElementCapacitanceMatrixPhase_MMP(long number, int phase);
extern void MMPSetElementConductivityMatrixPhase_MMP(long number, int phase, double *matrix);
extern double *MMPGetElementConductivityMatrixPhase_MMP(long number, int phase);
extern void MMPSetElementGravityVectorPhase_MMP(long number, int phase, double *vec);
extern double *MMPGetElementGravityVectorPhase_MMP(long number, int phase);
extern double *MMPGetElementCapillarityVectorPhase_MMP(long number, int phase);
extern void MMPSetElementCapillarityVectorPhase_MMP(long number, int phase, double *vec);

extern void MMPSetElementStrainCouplingMatrixX(long number, int phase, double *matrix);
extern double *MMPGetElementStrainCouplingMatrixX(long number,int phase);
extern void MMPSetElementStrainCouplingMatrixY(long number, int phase, double *matrix);
extern double *MMPGetElementStrainCouplingMatrixY(long number, int phase);
extern void MMPSetElementStrainCouplingMatrixZ(long number, int phase, double *matrix);
extern double *MMPGetElementStrainCouplingMatrixZ(long number, int phase);
extern void MMPCalcElementMatrices(CRFProcess*) ;
extern void *MMPDestroyELEMatricesPointer(void *data);
extern void *MMPCreateELEMatricesPointer(void);
extern void MMPCalcSecondaryVariables(void);
extern void MMPCalcElementMatricesTypes(CRFProcess*m_pcs);

#endif
