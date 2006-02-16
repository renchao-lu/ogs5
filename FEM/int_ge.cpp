/**************************************************************************/
/* ROCKFLOW - Modul: int_ge.c
 */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns GE zu den diesen Rechenkern benutzenden Modulen bereit.
 */
/* Programmaenderungen:
 */
/**************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "int_ge.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/* Elementdaten-Schnittstelle */
VoidFuncLDX GESetElementElectricConductivityMatrix = NULL;
DoubleXFuncLong GEGetElementElectricConductivityMatrix = NULL;

/* Knotendaten-Schnittstelle */

/* Globaldaten-Schnittstelle */
