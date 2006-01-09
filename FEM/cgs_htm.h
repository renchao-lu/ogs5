/**************************************************************************/
/* ROCKFLOW - Modul: cgs_htm.h
                                                                          */
/* Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul HTM.
                                                                          */
/**************************************************************************/
#ifndef cgs_htm_INC
#define cgs_htm_INC
extern void CalcFlux_HTM(double *rechts, double *ergebnis);
extern void HTMAssembleMatrix(CRFProcess*);
#endif
