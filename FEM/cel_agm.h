/**************************************************************************/
/* ROCKFLOW - Modul: cel_agm.h
                                                                          */
/* Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul aGM.
                                                                          */
/**************************************************************************/
#ifndef cel_agm_INC
#define cel_agm_INC
  /* Schutz gegen mehrfaches Einfuegen */

#include "rf_pcs.h"

typedef struct {
  double *conductancematrix; /* conductance matrix des Elements */
  double *capacitancematrix; /* capacitance matrix des Elements */
  double *gravityvector;     /* gravity vector des Elements */
  double *invjac;            /* Inverse Jakobi-Matrix im Zentrum des Elements */
} GMElementMatrices;

extern void GMSetElementCapacitanceMatrix(long number,double *matrix,int pcs_number);
extern double *GMGetElementCapacitanceMatrix(long number,int pcs_number);
extern void GMSetElementConductanceMatrix(long number,double *matrix,int pcs_number);
extern double *GMGetElementConductanceMatrix(long number,int pcs_number);
extern void GMSetElementGravityVector(long number,double *matrix,int pcs_number);
extern double *GMGetElementGravityVector(long number,int pcs_number);

extern void GMMakeElementEntry1D(long index,double *rechts,double *ergebnis, CRFProcess* m_pcs);
extern void GMMakeElementEntry2D(long index,double *rechts,double *ergebnis, CRFProcess* m_pcs);

extern void *GMCreateELEMatricesPointer(void);
extern void *GMDestroyELEMatricesPointer(void*);
extern void GMCalcElementMatrices(CRFProcess*);

#endif
