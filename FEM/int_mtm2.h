/**************************************************************************/
/* ROCKFLOW - Modul: int_MTM2.h
 */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MTM2 zu den diesen Rechenkern benutzenden Modulen bereit.

   letzte Aenderung: OK 31.10.97
 */
/**************************************************************************/

#ifndef int_MTM2_INC

#define int_MTM2_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_mtm2.h"
#include "cel_mtm2.h"

/* Knotendaten-Schnittstelle */
extern DoubleFuncLIII MTM2GetNodeConc;

extern DoubleFuncLong MTM2GetNodePress;
  /* Liefert Druck des Knotens number */
extern VoidFuncLD MTM2SetNodePress;
  /* Setzt Druck des Knotens number */
extern DoubleFuncLong MTM2GetNodeConc0;
  /* Liefert Konzentration 0 am Knoten number */
extern VoidFuncLD MTM2SetNodeConc0;
  /* Setzt Konzentration 0 am Knoten number */
extern DoubleFuncLong MTM2GetNodeConc1;
  /* Liefert Konzentration 1 am Knoten number */
extern VoidFuncLD MTM2SetNodeConc1;
  /* Setzt Konzentration 1 am Knoten number */
extern DoubleFuncILLI MTM2GetNodeFlux;
  /* Liefert Fluss der aktellen Phase  */
extern DoubleFuncILLI MTM2GetNodeSatu;
  /* Liefert Saettigung der aktellen Phase  */


/* Globaldaten-Schnittstelle */
// extern IntFuncVoid MTM2GetNumberOfFluidPhases;
  /* Liefert zurueck, wieviele Fluidphasen es gibt */
// extern IntFuncVoid MTM2GetTransportModelType;
  /* Liefert den Typ des Transportmodells: Stoff- oder Waermetransport */
/*extern DoubleFuncVoid MTM2GetGlobalAlpha0; */
  /* Liefert alpha0 */
/*extern VoidFuncDouble MTM2SetGlobalAlpha0; */
  /* Setzt alpha0 */
// extern DoubleFuncVoid MTM2GetGlobalTheta;
  /* Liefert theta */
// extern VoidFuncDouble MTM2SetGlobalTheta;
  /* Setzt theta */


/* Weitere externe Objekte */


#endif
