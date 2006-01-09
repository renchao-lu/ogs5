/**************************************************************************/
/* ATM - Modul: testvar.h
                                                                          */
/* Aufgabe:
   Enthaelt alle Variablen-Testfunktionen zum Ueberpruefen
   und evtl. Korrigieren von aus der Eingabedatei eingelesenen Werten.
   Wird von files1.c benutzt.

   Letzte Aenderung: OK 18.09.1997
                     AH 24.02.1998
                                                                          */
/**************************************************************************/

#ifndef testvar_INC

#define testvar_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include <stdio.h>


/* Deklarationen der Variablen-Korrekturfunktionen */
  /* zu ANAV */
extern int TFAnav (int *x, FILE *f);
extern int TFAnav0 (long *x, FILE *f);
extern int TFAnavx (double *x, FILE *f);
  /* zu IC_FLOW, IC_SATURATION, IC_TRANSPORT */
extern int TFSatuvorgabe (double *x, FILE *f);
extern int TFKonzvorgabe (double *x, FILE *f);
  /* zu MATERIAL_1D, MATERIAL_2D, MATERIAL_3D */
extern int TFPorositaet ( double *x, FILE *f );
extern int TFVerteilungskoeff ( double *x, FILE *f );
extern int TFFelsdichte ( double *x, FILE *f );
extern int TFTortuositaet ( double *x, FILE *f );
extern int TFDispersionsmodell ( int *x, FILE *f );
extern int TFDisp1 ( double *x, FILE *f );
extern int TFDisp2 ( double *x, FILE *f );
extern int TFDisp3 ( double *x, FILE *f );
extern int TFArea ( double *x, FILE *f );
extern int TFSpeicherkoeff ( double *x, FILE *f );
extern int TFMaterialmodell ( int *x, FILE *f );
  /* Testfunktionen zu Schluesselwort XD_GROUNDWATERFLOW_ELEMENT */
extern int TFNonlinearflowelement ( int *x, FILE *f );
extern int TFAlpha ( double *x, FILE *f );
  /* zu TRANSPORT_COMPONENT1 */
extern int TFHalbwertszeit ( double *x, FILE *f );

  /* C1.7 Material Properties */
extern int TFGravityAcceleration ( double *x, FILE *f );
extern int TFReferenceTemperature ( double *x, FILE *f );
extern int TFReferencePressure ( double *x, FILE *f );


/* Weitere externe Objekte */


#endif

