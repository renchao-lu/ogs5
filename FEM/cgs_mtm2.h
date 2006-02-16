/**************************************************************************/
/* ROCKFLOW - Modul: cgs_MTM2.h
 */
/* Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul MTM2.
 */
/**************************************************************************/
#ifndef cgs_MTM2_INC
#define cgs_MTM2_INC
  /* Schutz gegen mehrfaches Einfuegen */
#include "rf_pcs.h"
extern void MakeStatMat_MTM2(CRFProcess *m_pcs);
extern void MakeGS_MTM2_old(double *rechts, double *ergebnis, int component, CRFProcess *m_pcs);
extern void MakeGS_MTM2(double*, CRFProcess*);
extern void MTM2CalcSecondaryVariables(void);
#endif
