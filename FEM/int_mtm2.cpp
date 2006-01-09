/**************************************************************************/
/* ROCKFLOW - Modul: int_MTM2.c
 */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MTM2 zu den diesen Rechenkern benutzenden Modulen bereit.
 */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
   femlib.c (-> CalcElementJacobiMatrix)
   Funktionszeiger bereitstellen
   25.07.1997  R.Kaiser   Funktionszeiger ...ElementJacobiMatrix
   entfernt (-> intrface.c)
   02/2000   C.Thorenz    Erweitert
 */
/**************************************************************************/

#include "stdafx.h"             /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
/* Header / Andere intern benutzte Module */
 #include "int_mtm2.h"
/* Knotendaten-Schnittstelle */
DoubleFuncLIII MTM2GetNodeConc = NULL;
DoubleFuncLong MTM2GetNodePress = NULL;
VoidFuncLD MTM2SetNodePress = NULL;
DoubleFuncLong MTM2GetNodeConc0 = NULL;
VoidFuncLD MTM2SetNodeConc0 = NULL;
DoubleFuncLong MTM2GetNodeConc1 = NULL;
VoidFuncLD MTM2SetNodeConc1 = NULL;
DoubleFuncILLI MTM2GetNodeFlux = NULL;
DoubleFuncILLI MTM2GetNodeSatu = NULL;
