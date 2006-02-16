/**************************************************************************/
/* ROCKFLOW - Modul: cgs_atm.h
 */
/* Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul aTM.
 */
/**************************************************************************/

#ifndef cgs_atm_INC

#define cgs_atm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
extern void MakeStatMat_ATM(void);
  /* berechnet Matrizen aller Elemente vor der Zeitschleife */

extern void MakeGS_ATM(double *rechts, double *ergebnis, double zeit);
extern void MakeGS_ATM_New(double *rechts, double *ergebnis, double zeit, int phase, int anz_phasen, int component, double theta);
  /* stellt Gesamtgleichungssystem auf */
extern void CalcFlux_ATM(double *rechts, double *ergebnis, double *residuum, int phase, int component);

/* Weitere externe Objekte */


#endif
