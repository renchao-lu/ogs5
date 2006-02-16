/**************************************************************************/
/* ROCKFLOW - Modul: masscont.h
                                                                          */
/* Aufgabe:
   Massenkontinuitaet sicherstellen (gehoert direkt zu refine.h)
                                                                          */
/**************************************************************************/

#ifndef masscont_INC

#define masscont_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */



  /* Massenerhaltung */
extern void ExecuteMassCont( long ele, int num_ref_quantity, double *zwischenspeicher );



/* Weitere externe Objekte */


#endif


