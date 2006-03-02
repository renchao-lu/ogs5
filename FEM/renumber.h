/**************************************************************************/
/* ROCKFLOW - Modul: renumber.h
                                                                          */
/* Aufgabe:
   Knotenumnummerierer
   Ein Knotenumnummerierer erzeugt das globale long-Feld realnode, das
   die Originalnummern fuer die neuen Indizes enthaelt. Die Umkehrung,
   d.h. der neue Index bei bekannter Originalnummer wird in die
   Knotenliste unter current_number eingespeichert.
                                                                          */
/**************************************************************************/

#ifndef renumber_INC

#define renumber_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "prototyp.h"

/* Deklarationen */

/* Deklarationen fuer Schluesselwort #RENUMBER */
extern int FctRenumber(char *data, int found, FILE *f);

extern int umnummerierer;  /* Knotenumnummerierer */
extern VoidFuncVoid RenumberStart;  /* Funktionszeiger auf Umnummerierer-Startfunktion */
extern VoidFuncVoid RenumberNodes;  /* Funktionszeiger auf Umnummerierer */
extern VoidFuncVoid RenumberEnd;  /* Funktionszeiger auf Umnummerierer-Endfunktion */

void CompressStart ( void );
void CompressNodes ( void );
void CompressEnde ( void );
  /* einfacher Knotenumnummerierer, der lediglich die Loecher in der
     Knotenliste beseitigt */

void GibbsStart ( void );
void Gibbs ( void );
void GibbsEnde ( void );
  /* Knotenumnummerierer nach Gibbs, Poole, Stockmeyer */

void CuthillStart ( void );
void Cuthill ( void );
void CuthillEnde ( void );
  /* Knotenumnummerierer nach Cuthill-McKee */


/* Weitere externe Objekte */


#endif
