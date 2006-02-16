/**************************************************************************/
/* ROCKFLOW - Modul: int_ge.h
 */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aSM zu den diesen Rechenkern benutzenden Modulen bereit.
 */
/**************************************************************************/

#ifndef int_ge_INC
#define int_ge_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_ge.h"
#include "cel_ge.h"
#include "cvel.h"
#include "pst_flow.h"

/* Deklarationen */

/* Elementdaten-Schnittstelle */
extern VoidFuncLDX GESetElementElectricConductivityMatrix;
  /* Setzt die conductance matrix des Elements number */
extern DoubleXFuncLong GEGetElementElectricConductivityMatrix;
  /* Liefert die conductance matrix des Elements number */

/* Knotendaten-Schnittstelle */

/* Globaldaten-Schnittstelle */

/* Weitere externe Objekte */
#endif
