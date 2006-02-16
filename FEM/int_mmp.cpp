/**************************************************************************
 ROCKFLOW - Modul: int_mmp.c

 Aufgabe:
 Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
 Rechenkerns MMP zu den diesen Rechenkern benutzenden Modulen bereit.

 Programmaenderungen:
 C. Thorenz          9/97
 05/1999   C.Thorenz   Umbau auf n-Phasen
 04/2002   OK          MMPGetNodeTemperature

**************************************************************************/

#include "stdafx.h"                    /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "int_mmp.h"
/* Elementdaten-Schnittstelle */
/* Matrizen */
VoidFuncLIDX MMPSetElementConductivityMatrix;
VoidFuncLIDX MMPSetElementCapacitanceMatrix;
VoidFuncLIDX MMPSetElementCapillarityVector;
VoidFuncLIDX MMPSetElementGravityVector;
VoidFuncLDX MMPSetElementCouplingMatrixPU;

DoubleXFuncLI MMPGetElementConductivityMatrix;
DoubleXFuncLI MMPGetElementCapacitanceMatrix;
DoubleXFuncLI MMPGetElementCapillarityVector;
DoubleXFuncLI MMPGetElementGravityVector;
DoubleXFuncLong MMPGetElementSaturationMassMatrix;
DoubleXFuncLong MMPGetElementCouplingMatrixPU;

VoidFuncLI MMPDeleteElementConductivityMatrix;
VoidFuncLI MMPDeleteElementCapacitanceMatrix;
VoidFuncLI MMPDeleteElementCapillarityVector;
VoidFuncLI MMPDeleteElementGravityVector;

/* Knotendaten-Schnittstelle */
DoubleFuncILLI MMPGetNodePress;
DoubleFuncILLI MMPGetNodePressHigherOrderContinuum;
DoubleFuncILLI MMPGetNodePressLowerOrderContinuum;
DoubleFuncILLI MMPGetNodeFlux;
DoubleFuncILLI MMPGetNodeSatu;
VoidFuncILID MMPSetNodeFlux;
VoidFuncILID MMPSetNodePress;
DoubleFuncLong MMPGetNodeTemperature;

/* Materialdaten */
DoubleFuncILLD MMPGetNodeDifferenceToReferencePressure;
DoubleFuncILDDDD MMPGetRelativePermeability;
DoubleFuncILLD MMPGetNodeRelativePermeability;
DoubleFuncILDDDD MMPGetSaturation;
DoubleFuncILDDDD MMPGetSaturationPressureDependency;
//DoubleFuncLDDDD MMPGetStorativity = (DoubleFuncLDDDD) GetSoilStorativity;
DoubleFuncILLD MMPGetNodeCapillarPressure;
