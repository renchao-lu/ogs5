/**************************************************************************
 ROCKFLOW - Modul: int_mmp.h

 Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MMP zu den diesen Rechenkern benutzenden Modulen bereit.

**************************************************************************/

#ifndef int_mmp_INC
#define int_mmp_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_mmp.h"
#include "cel_mmp.h"
#include "cvel.h"

/* Deklarationen */

/* Elementdaten-Schnittstelle */

/* Matrizen */
extern VoidFuncLIDX MMPSetElementConductivityMatrix;
extern VoidFuncLIDX MMPSetElementCapacitanceMatrix;
extern VoidFuncLIDX MMPSetElementCapillarityVector;
extern VoidFuncLIDX MMPSetElementGravityVector;
extern VoidFuncLDX MMPSetElementCouplingMatrixPU;

extern DoubleXFuncLI MMPGetElementConductivityMatrix;
extern DoubleXFuncLI MMPGetElementCapacitanceMatrix;
extern DoubleXFuncLI MMPGetElementCapillarityVector;
extern DoubleXFuncLI MMPGetElementGravityVector;
extern DoubleXFuncLong MMPGetElementSaturationMassMatrix;
extern DoubleXFuncLong MMPGetElementCouplingMatrixPU;

extern VoidFuncLI MMPDeleteElementConductivityMatrix;
extern VoidFuncLI MMPDeleteElementCapacitanceMatrix;
extern VoidFuncLI MMPDeleteElementCapillarityVector;
extern VoidFuncLI MMPDeleteElementGravityVector;


/* Elementwerte */


/* Knotendaten-Schnittstelle */
extern DoubleFuncILLI MMPGetNodePress;
extern DoubleFuncILLI MMPGetNodePressHigherOrderContinuum;
extern DoubleFuncILLI MMPGetNodePressLowerOrderContinuum;
extern DoubleFuncILLI MMPGetNodeFlux;
extern DoubleFuncILLI MMPGetNodeSatu;
extern VoidFuncILID MMPSetNodeFlux;
extern VoidFuncILID MMPSetNodePress;
extern DoubleFuncLong MMPGetNodeTemperature;

/* Globaldaten-Schnittstelle */

/* Materialbeziegungen */
extern DoubleFuncILLD MMPGetNodeDifferenceToReferencePressure;
extern DoubleFuncILDDDD MMPGetRelativePermeability;
extern DoubleFuncILLD MMPGetNodeRelativePermeability;
extern DoubleFuncILDDDD MMPGetSaturation;
extern DoubleFuncILDDDD MMPGetSaturationPressureDependency;
extern DoubleFuncLDDDD MMPGetStorativity;
extern DoubleFuncILLD MMPGetNodeCapillarPressure;

#endif
