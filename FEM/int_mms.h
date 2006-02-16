/**************************************************************************
 ROCKFLOW - Modul: int_mms.h

 Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MMS zu den diesen Rechenkern benutzenden Modulen bereit.

**************************************************************************/

#ifndef int_mms_INC

#define int_mms_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_mms.h"
#include "cel_mms.h"

/* Deklarationen */

/* Elementdaten-Schnittstelle */
/* Matrizen */
extern VoidFuncLIDX MMSSetElementConductivityMatrix;
extern VoidFuncLIDX MMSSetElementCapacitanceMatrix;
extern VoidFuncLIDX MMSSetElementCapillarityVector;
extern VoidFuncLIDX MMSSetElementGravityVector;
extern VoidFuncLDX MMSSetElementSaturationMassMatrix;

extern DoubleXFuncLI MMSGetElementConductivityMatrix;
extern DoubleXFuncLI MMSGetElementCapacitanceMatrix;
extern DoubleXFuncLI MMSGetElementCapillarityVector;
extern DoubleXFuncLI MMSGetElementGravityVector;
extern DoubleXFuncLong MMSGetElementSaturationMassMatrix;

extern VoidFuncLI MMSDeleteElementConductivityMatrix;
extern VoidFuncLI MMSDeleteElementCapacitanceMatrix;
extern VoidFuncLI MMSDeleteElementCapillarityVector;
extern VoidFuncLI MMSDeleteElementGravityVector;
extern VoidFuncLong MMSDeleteElementSaturationMassMatrix;

/* Knotendaten-Schnittstelle */
extern DoubleFuncILLI MMSGetNodePress;
extern DoubleFuncILLI MMSGetNodeFlux;
extern DoubleFuncILLI MMSGetNodeSatu;
extern VoidFuncILID MMSSetNodeFlux;
extern VoidFuncILID MMSSetNodeSatu;
extern DoubleFuncLong MMSGetNodeTemperature;

/* Globaldaten-Schnittstelle */
extern VoidFuncInt MMSSetCalcSaturationForPhase;
extern IntFuncVoid MMSGetCalcSaturationForPhase;
extern IntFuncVoid MMSGetNumberOfPhases;
extern DoubleFuncILDDDD MMSGetSaturationPressureDependency;
extern DoubleFuncILLD MMSGetNodeSaturationPressureDependency;

#endif
