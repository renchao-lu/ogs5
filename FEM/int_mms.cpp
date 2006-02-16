/**************************************************************************
   ROCKFLOW - Modul: int_mms.c

   Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MMS zu den diesen Rechenkern benutzenden Modulen bereit.

   Programmaenderungen:

   C. Thorenz          9/97
   C. Thorenz          3/98
   C. Thorenz          4/99
   C. Thorenz          5/99
   04/2002   OK   MMSGetNodeTemperature
**************************************************************************/

#include "stdafx.h"             /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "int_mms.h"

/* Interne (statische) Deklarationen */

/* Definitionen */

VoidFuncLIDX MMSSetElementConductivityMatrix;
VoidFuncLIDX MMSSetElementCapacitanceMatrix;
VoidFuncLIDX MMSSetElementCapillarityVector;
VoidFuncLIDX MMSSetElementGravityVector;
VoidFuncLDX MMSSetElementSaturationMassMatrix;

DoubleXFuncLI MMSGetElementConductivityMatrix;
DoubleXFuncLI MMSGetElementCapacitanceMatrix;
DoubleXFuncLI MMSGetElementCapillarityVector;
DoubleXFuncLI MMSGetElementGravityVector;
DoubleXFuncLong MMSGetElementSaturationMassMatrix;

VoidFuncLI MMSDeleteElementConductivityMatrix;
VoidFuncLI MMSDeleteElementCapacitanceMatrix;
VoidFuncLI MMSDeleteElementCapillarityVector;
VoidFuncLI MMSDeleteElementGravityVector;
VoidFuncLong MMSDeleteElementSaturationMassMatrix;

/* Knotendaten-Schnittstelle */
DoubleFuncILLI MMSGetNodePress;
DoubleFuncILLI MMSGetNodeFlux;
DoubleFuncILLI MMSGetNodeSatu;
VoidFuncILID MMSSetNodeFlux;
VoidFuncILID MMSSetNodeSatu;
DoubleFuncLong MMSGetNodeTemperature;

/* Globaldaten-Schnittstelle */
VoidFuncInt MMSSetCalcSaturationForPhase;
IntFuncVoid MMSGetCalcSaturationForPhase;
IntFuncVoid MMSGetNumberOfPhases;
DoubleFuncILDDDD MMSGetSaturationPressureDependency;
DoubleFuncILLD MMSGetNodeSaturationPressureDependency;


