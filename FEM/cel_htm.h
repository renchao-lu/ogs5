/**************************************************************************
   ROCKFLOW - Modul: cel_htm.h
 
   Aufgabe:
   Schnittstelle des FEM-Kernels HTM

   letzte Aenderung: OK 11/2003
 
**************************************************************************/
#ifndef cel_htm_INC
#define cel_htm_INC
#include "rf_pcs.h"
extern void ConditionalRebuildMatrices_HTM(int phase, int comp);
extern void *HTMCreateELEMatricesPointer(void);
extern void *HTMDestroyELEMatricesPointer(void*);
extern void CalcElementMatrices_HTM(long);
extern void CECalcPeclet_HTM(long);
extern void HTMCalcElementMatrices(CRFProcess*);
extern void HTMCalcElementMatrix(long index,CRFProcess*);
extern void HTMSetElementHeatCapacitanceMatrix(long number,double*matrix,int pcs_number);
extern double *HTMGetElementHeatCapacitanceMatrix(long number,int pcs_number);
extern void HTMSetElementHeatDiffusionMatrix(long number,double *matrix,int pcs_number);
extern double *HTMGetElementHeatDiffusionMatrix(long number,int pcs_number);
extern void HTMSetElementHeatAdvectionMatrix(long number,double *matrix,int pcs_number);
extern double *HTMGetElementHeatAdvectionMatrix(long number,int pcs_number);
/* Element matrices */
typedef struct {
  double *heatcapacitancematrix;
  double *heatadvectionmatrix;
  double *heatdispersionmatrix;
} HTMElementMatrices;
/* Kernel initialisieren und zerstoeren */
extern int InitializeKernel_HTM(void);
/* Zugriff auf Kernel-Parameter */
extern int GetMethod_HTM(void);
extern double GetTimeCollocationGlobal_HTM(void);
extern int GetNonLinearCoupling_HTM(void);
extern int GetPredictorMethod_HTM(void);
extern double GetPredictorParam_HTM(void);
extern int GetRelaxationMethod_HTM(void);
extern double *GetRelaxationParam_HTM(void);
/* Daempfung */
extern void HTMDampOscillations(int ndx1);
extern double HTMNodeCalcLumpedMass(long knoten);
#endif
