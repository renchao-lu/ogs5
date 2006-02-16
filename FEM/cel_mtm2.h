/**************************************************************************/
/* ROCKFLOW - Modul: cel_MTM2.h

   Aufgabe: Mass Transport
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul MTM2.
   Abgeleitet von ATM

 */
/**************************************************************************/

#ifndef cel_MTM2_INC
#define cel_MTM2_INC
/* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "rf_pcs.h"



/* Initialisiert den Kernel */
//extern int InitializeKernel_MTM2(void);
/* Zerstoert den Kernel */
//extern int DestroyKernel_MTM2(void);

/* Zugriff auf Kernel-Parameter */
/*
extern int GetMethod_MTM2(void);
extern double GetTimeCollocationGlobal_MTM2(void);
extern double GetTimeCollocationCondBC_MTM2(void);
extern double GetTimeCollocationSource_MTM2(void);
extern double GetTimeCollocationUpwinding_MTM2(void);
*/
extern int GetArtificialDiffusion_MTM2(void);
extern double *GetArtificialDiffusionParam_MTM2(void);
/*
extern int GetUseLagrange_MTM2(void);
// extern int GetTransportInPhase_MTM2(void);
// extern int GetNonLinearCoupling_MTM2(void);
extern int GetPredictorMethod_MTM2(void);
extern double GetPredictorParam_MTM2(void);
extern int GetRelaxationMethod_MTM2(void);
extern double *GetRelaxationParam_MTM2(void);
extern int GetOscillationDampingMethod_MTM2(void);
extern double *GetOscillationDampingParam_MTM2(void);
*/
extern double MTM2_time_collocation_global; //SB

/* Berechnet und setzt die Peclet-Zahl und den Courant/Neumann-Zeitschritt des Elements number */
extern void CECalcPeclet_MTM2_NEW(int phase, int component, long index);

/* Berechnet und setzt die Elementmatrizen des Elements index */
extern void CECalcEleMatrix_MTM2_NEW( long index, long component, CRFProcess *m_pcs);

/* Daempfung */
extern void MTM2DampOscillations(int ndx1, int phase);
extern double MTM2NodeCalcLumpedMass(long knoten);

/* Weitere externe Objekte */
#define noEFFORT

#ifdef EFFORT
extern int MTM2_effort_index;
extern int MTM2_effort_total_index;
#endif



/* Deklarationen */
typedef struct {
    double *advectionmatrix;      /* Advektive Matrix des Elements */
    double *dispersionmatrix;     /* Dispersive Matrix des Elements */
    double *massmatrix;           /* Massenmatrix des Elements */
    double *decaymatrix;          /* Zerfallsmatrix des Elements */
	double *fluidcontentmatrix;   /* Matrix fuer Fluidvolumenaenderung des Elements */
} InternElDataMTM2;

/* Elementmatrizen einspeichern */
extern void MTM2SetElementAdvMatrixNew(long number, int mtm_pcs_number, double *matrix);
extern void MTM2SetElementDecayMatrixNew(long number, int mtm_pcs_number, double *matrix);
extern void MTM2SetElementDispMatrixNew(long number, int mtm_pcs_number, double *matrix);
extern void MTM2SetElementMassMatrixNew(long number,  int mtm_pcs_number, double *matrix);
extern void MTM2SetElementFluidContentMatrixNew(long number, int mtm_pcs_number, double *matrix);
/* Elementmatrizen holen */
extern double *MTM2GetElementAdvMatrixNew(long number,  int mtm_pcs_number);
extern double *MTM2GetElementDecayMatrixNew(long number,  int mtm_pcs_number);
extern double *MTM2GetElementDispMatrixNew(long number, int mtm_pcs_number);
extern double *MTM2GetElementMassMatrixNew(long number, int mtm_pcs_number);
extern double *MTM2GetElementFluidContentMatrixNew(long number, int mtm_pcs_number);

extern void *MTM2CreateELEMatricesPointer(void);
extern void *MTM2DestroyELEMatricesPointer(void *data);

/* liefert Phasensättigung */
double MTM2GetSaturation(int phase, long index, double r, double s, double t, double theta);

/* Todo - für automatische Zeitschrittsteuerung */
/*
extern void MTM2SetElementDtNew ( long number, int phase, double zeitschritt );
extern void MTM2SetElementPecletNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementPecletNumNew ( long number, int phase, int component);
extern void MTM2SetElementCourantNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementCourantNumNew ( long number, int phase, int component);
extern void MTM2SetElementDamkohlerNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementDamkohlerNumNew ( long number, int phase, int component);
*/

#endif
