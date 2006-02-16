/**************************************************************************
 ROCKFLOW - Modul: cgs_mms.h

 Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul MMS.

**************************************************************************/

#ifndef cgs_mms_INC

#define cgs_mms_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */

extern void MakeGS_MMS(int phase, double *rechts,
                       char *bc_name,
                       char *source_mass_flow_name,
                       char *ss_volume_flow_name);
/* OK rf3604 */
extern void MakeGS_MMSV1(double *rechts, double *ergebnis_MMS, double dummy);
extern void MMSCalcSaturations(int timelevel);

/* Weitere externe Objekte */


#endif
