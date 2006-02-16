/**************************************************************************/
/* ROCKFLOW - Modul: pst_flow.h
                                                                          */
/* Aufgabe:
   Funktionen zur Berechnung der Geschwindigkeiten in den Gausspunkten
   fuer das Modul aSM.
                                                                          */
/**************************************************************************/

#ifndef pst_flow_INC

#define pst_flow_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
extern void CalcMassFlux_ASM ( FILE *f);
  /* Berechnet und setzt den Massenstrom usw. des
     Elements number */

/* Weitere externe Objekte */


#endif


