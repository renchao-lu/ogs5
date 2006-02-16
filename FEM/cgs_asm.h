/**************************************************************************/
/* ROCKFLOW - Modul: cgs_asm.h
                                                                          */
/* Aufgabe:
   Aufstellen des Gleichungssystems fuer das Modul aSM.
                                                                          */
/**************************************************************************/

#ifndef cgs_asm_INC

#define cgs_asm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "rf_pcs.h"

/* Deklarationen */

extern void MakeStatMat_ASM ( void );
  /* berechnet Matrizen aller Elemente vor der Zeitschleife */

extern void MakeGS_ASM ( double *rechts, double *ergebnis,
                         double aktuelle_zeit );
  /* stellt Gesamtgleichungssystem fuer neue Loeser auf */
extern void MakeGS_ASM_NEW ( double *rechts, double *ergebnis,
                         double aktuelle_zeit );

extern void ASMCalcHeads(int phase, int timelevel);
extern void ASMCalcNodeFluxes(void);
extern void ASMCalcNodeWDepth(CRFProcess*);

extern void SMCalcElementMatrices(CRFProcess*);
extern void OFCalcElementMatrices(CRFProcess*);
extern void SMAssembleMatrix(double*,double*,double,CRFProcess*);
#endif
