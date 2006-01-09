/**************************************************************************/
/* ROCKFLOW - Modul: int_htm.c
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns HTM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/* Programmaenderungen:
   03/2000      RK        Erste Version


                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
#include "makros.h"
#include "int_htm.h"
/* Elementdaten-Schnittstelle */
/*
VoidFuncLDX HTMSetElementHeatCapacitanceMatrix;
VoidFuncLDX HTMSetElementHeatAdvectionMatrix;
VoidFuncLDX HTMSetElementHeatDiffusionMatrix;
DoubleXFuncLong HTMGetElementHeatCapacitanceMatrix;
DoubleXFuncLong HTMGetElementHeatAdvectionMatrix;
DoubleXFuncLong HTMGetElementHeatDiffusionMatrix;
VoidFuncLong HTMDeleteElementHeatCapacitanceMatrix;
VoidFuncLong HTMDeleteElementHeatAdvectionMatrix;
VoidFuncLong HTMDeleteElementHeatDiffusionMatrix;
DoubleXFuncLong HTMGetElementHeatCapacitanceMatrixGroup;
DoubleXFuncLong HTMGetElementHeatAdvectionMatrixGroup;
DoubleXFuncLong HTMGetElementHeatDiffusionMatrixGroup;
VoidFuncLD HTMSetElementDt;
DoubleFuncLong HTMGetElementDt;
VoidFuncLD HTMSetElementPecletNum;
VoidFuncLD HTMSetElementDDummy;
DoubleFuncLong HTMGetElementPecletNum;
DoubleFuncLong HTMGetElementDDummy;
*/
/* Knotendaten-Schnittstelle */
/*
DoubleFuncILDDDD HTMGetFluidContent=NULL;
DoubleFuncLong HTMGetNodeTemp0;
VoidFuncLD HTMSetNodeTemp0;
DoubleFuncLong HTMGetNodeTemp1;
VoidFuncLD HTMSetNodeTemp1;
DoubleFuncILDDDD HTMGetSaturationGP;
DoubleFuncILLI HTMGetNodeFlux = NULL;
*/



