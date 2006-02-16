/**************************************************************************/
/* ROCKFLOW - Modul: cgs_agm.h
                                                                          */
/* Aufgabe:
   Aufstellen des Gleichungssystems fuer das Modul aGM.
                                                                          */
/**************************************************************************/
#ifndef cgs_agm_INC
#define cgs_agm_INC
  /* Schutz gegen mehrfaches Einfuegen */
#include "rf_pcs.h"
void GMAssembleMatrix(double *rechts,double *ergebnis,double aktuelle_zeit,CRFProcess *m_pcs);
#endif
