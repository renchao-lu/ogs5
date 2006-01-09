/**************************************************************************
   ROCKFLOW - Modul: cel_ge.h
 
   Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer das Modul aSM.

   letzte Aenderung: RK 03/2003
 
**************************************************************************/

#ifndef cel_ge_INC
#define cel_ge_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
/* Kernel initialisieren und zerstoeren */
extern int InitializeKernel_GE(void);
extern int GetMethod_GE(void);

/* berechnet stationaere Matrizenanteile eines Elements */
extern void CalcElementMatrix_GE(long index);


/* Weitere externe Objekte */
#endif
