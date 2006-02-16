/**************************************************************************
 ROCKFLOW - Modul: cgs_mpc.h

 Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul MPC.

**************************************************************************/

#ifndef cgs_mpc_INC
#define cgs_mpc_INC
  /* Schutz gegen mehrfaches Einfuegen */

extern void MPCAssembleSystemMatrix(int phase,double *rechts);
extern void MPCCalcSecondaryVariables(void);
extern void MPCCalcSecondaryVariablesRichards(void);
extern void MPCSetICRichards(int);
extern void MPCSetIC(int);

#endif
