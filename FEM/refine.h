/**************************************************************************/
/* ROCKFLOW - Modul: refine.h
                                                                          */
/* Aufgabe:
   Verfeinerungs- / Vergroeberungsfunktionen
   Massgebend fuer die Verfeinerung ist der Zustand des
   "verfeinern" - Schalters in den Elementdaten.
   Aufgeteilt in refine1.c, refine2.c und refine3.c.
                                                                          */
/**************************************************************************/

#ifndef refine_INC

#define refine_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "adaptiv.h"
//OK_IC #include "rfbc.h"
//OK_IC #include "rfsousin.h"
#include "renumber.h"
  /* Funktionszeiger fuer Modellschnittstelle */

#define noTESTREF


/* Deklarationen zu refine1.c */
extern int NetRef1D_a ( void );
  /* einfache Verfeinerung/Vergroeberung nur fuer 1D-Elemente */

/* Deklarationen zu refine2.c */
extern int NetRef2D_a ( void );
  /* Verfeinern und Vergroebern fuer gekoppelte 1D- und 2D-Elemente */
extern void InterpolIrregNodeVals2D ( int nval0, int nval1 );
  /* Interpolieren der Knotenwerte */

/* Deklarationen zu refine3.c */
extern int NetRef3D_a ( void );
  /* Verfeinern und Vergroebern fuer gekoppelte 1D-, 2D- und 3D-Elemente */
extern void InterpolIrregNodeVals3D ( int nval0, int nval1 );
  /* Interpolieren der Knotenwerte */


/* Weitere externe Objekte */


#endif
