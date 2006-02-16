/**************************************************************************
   ROCKFLOW - Modul: cel_atm.h
 
   Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul aTM.

   letzte Aenderung: RK 03/2003
 
**************************************************************************/

#ifndef cel_atm_INC
#define cel_atm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */

/* Initialisiert den Kernel */
extern int InitializeKernel_ATM(void);


/* Zugriff auf Kernel-Parameter */
extern int GetMethod_ATM(void);
extern double GetTimeCollocationGlobal_ATM(void);
extern double GetTimeCollocationCondBC_ATM(void);
extern double GetTimeCollocationSource_ATM(void);
extern double GetTimeCollocationUpwinding_ATM(void);
extern int GetArtificialDiffusion_ATM(void);
extern double *GetArtificialDiffusionParam_ATM(void);
extern int GetUseLagrange_ATM(void);
extern int GetTransportInPhase_ATM(void);
extern int GetNonLinearCoupling_ATM(void);
extern int GetPredictorMethod_ATM(void);
extern double GetPredictorParam_ATM(void);
extern int GetRelaxationMethod_ATM(void);
extern double *GetRelaxationParam_ATM(void);
extern int GetOscillationDampingMethod_ATM(void);
extern double *GetOscillationDampingParam_ATM(void);


  /* berechnet stationaere Matrizenanteile eines Elements */
extern void StatMat_ATM(long);

  /* berechnet instationaere Matrizenanteile eines Elements */
extern void MakeMat_ATM(long);

  /* Berechnet und setzt den Retardationsfaktor des Elements number */
extern void CECalcRetard_ATM(long number);

  /* Berechnet und setzt die Peclet-Zahl und den Courant/Neumann-Zeitschritt
     des Elements number */
extern void CECalcPeclet_ATM(long number);
extern void CECalcPeclet_ATM_NEW(int phase, int component, long index);

  /* Berechnet und setzt die Elementmatrizen des Elements index */
extern void CECalcEleMatrix_ATM(long index);
extern void CECalcEleMatrix_ATM_NEW(int phase, int component, long index);
extern void ConditionalRebuildMatrices_ATM(int phase, int comp);
extern void BuildMatrices_ATM(int phase, int comp);

/* Daempfung */
extern void ATMDampOscillations(int ndx1, int phase);
extern double ATMNodeCalcLumpedMass(long knoten);


/* Weitere externe Objekte */
#define noEFFORT

#ifdef EFFORT
extern int atm_effort_index;
extern int atm_effort_total_index;
#endif

#endif
