/**************************************************************************
 ROCKFLOW - Modul: cel_mms.h

 Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul MMS.

 05/2003 OK MMSElementMatrices
   
 letzte Aenderung: RK 03/2003  

**************************************************************************/
#ifndef cel_mms_INC
#define cel_mms_INC
#include "rf_pcs.h"
/* Initialisiert Kernel-Daten */
extern int InitializeKernel_MMS(void);
/* Zerstoert Kernel-Daten */
/* Zugriff auf Kernelparameter */
extern double GetTimeCollocationGlobal_MMS(void);
extern double GetTimeCollocationCondBC_MMS(void);
extern double GetTimeCollocationSource_MMS(void);
extern double GetTimeCollocationUpwinding_MMS(void);
extern double GetTimeCollocationOpenBoundary_MMS(void);
extern int GetMethod_MMS(void);
extern int GetOtherSatuCalcMethod_MMS(void);
extern int GetNonLinearCoupling_MMS(void);
extern int GetPredictorMethod_MMS(void);
extern double GetPredictorParam_MMS(void);
extern int GetRelaxationMethod_MMS(void);
extern double *GetRelaxationParam_MMS(void);

/* Deklarationen */
extern void MakeMat_MMS(long);
  /* Berechnet und setzt die Elementmatrizen des Elements number */
extern void MakeStatMat_MMS(void);
  /* berechnet Matrizen aller Elemente vor der Zeitschleife */

/* Daempfung */
extern void MMSDampOscillations(int ndx1);
extern double MMSNodeCalcLumpedMass(long knoten);

/* Element matrices */
typedef struct {
  double *saturation_mass_matrix;     /* Massenmatrix des Elements fuer die Saettigung, z.Zt. */
} MMSElementMatrices;

extern double *MMSGetElementSaturationMassMatrix_MMS(long number);
extern void MMSSetElementSaturationMassMatrix_MMS(long number, double *matrix);
extern double *MMSGetElementCapillarityVector_MMS(long number,int phase);
extern double *MMSGetElementCapacitanceMatrix_MMS(long number,int phase);
extern double *MMSGetElementGravityVector_MMS(long number,int phase);
extern double *MMSGetElementConductivityMatrixPhase_MMS(long number, int phase);
extern void *MMSDestroyELEMatricesPointer(void *data);
extern void *MMSCreateELEMatricesPointer(void);
extern void MMSCalcElementMatrices(CRFProcess*);

#endif
