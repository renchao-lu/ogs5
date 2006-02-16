/**************************************************************************/
/* ROCKFLOW - Modul: int_asm.c
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aSM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   03/1997     R.Kaiser   Anpassung an das Stroemungsmodell
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
                          femlib.c (-> CalcElementJacobiMatrix)
                          Funktionszeiger bereitstellen
   25.07.1997  R.Kaiser   Funktionszeiger ...ElementJacobiMatrix
                          entfernt (-> intrface.c)

   letzte Aenderung    R.Kaiser   25.07.1997

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "int_asm.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/* Elementdaten-Schnittstelle */
VoidFuncLD ASMSetElementDt;
VoidFuncLDX ASMSetElementConductanceMatrix;
VoidFuncLDX ASMSetElementCapacitanceMatrix;
VoidFuncLDX ASMSetElementGravityVector;
VoidFuncLDX ASMSetElementStrainCouplingMatrixX;
VoidFuncLDX ASMSetElementStrainCouplingMatrixY;
VoidFuncLDX ASMSetElementCouplingMatrixPU;
/*VoidFuncLD ASMSetElementVolume;*/
VoidFuncLD ASMSetElementXVelo;
VoidFuncLD ASMSetElementYVelo;
VoidFuncLD ASMSetElementZVelo;
VoidFuncLD ASMSetElementXMiddle;
VoidFuncLD ASMSetElementYMiddle;
VoidFuncLD ASMSetElementZMiddle;
VoidFuncLD ASMSetElementXFlux;
VoidFuncLD ASMSetElementYFlux;
VoidFuncLD ASMSetElementZFlux;
VoidFuncLD ASMSetElementDDummy;
DoubleXFuncLong ASMGetElementConductanceMatrix;
DoubleXFuncLong ASMGetElementCapacitanceMatrix;
DoubleXFuncLong ASMGetElementGravityVector;
DoubleXFuncLong ASMGetElementStrainCouplingMatrixX; /*MK spaeter loeschen*/
DoubleXFuncLong ASMGetElementStrainCouplingMatrixY;
DoubleXFuncLong ASMGetElementCouplingMatrixPU;
DoubleXFuncLong ASMGetElementConductanceMatrixGroup;
DoubleXFuncLong ASMGetElementCapacitanceMatrixGroup;
DoubleXFuncLong ASMGetElementGravityVectorGroup;
DoubleFuncLong ASMGetElementDt;
/*DoubleFuncLong ASMGetElementVolume;*/
DoubleFuncLong ASMGetElementDDummy;
VoidFuncLong ASMDeleteElementConductanceMatrix;
VoidFuncLong ASMDeleteElementCapacitanceMatrix;
VoidFuncLong ASMDeleteElementGravityVector;

/* Knotendaten-Schnittstelle */
DoubleFuncLong ASMGetNodeHead0;
VoidFuncLD ASMSetNodeHead0;
DoubleFuncLong ASMGetNodePress0;
VoidFuncLD ASMSetNodePress0;
DoubleFuncLong ASMGetNodePress1;
VoidFuncLD ASMSetNodePress1;
VoidFuncILID ASMSetNodeFlux;
DoubleFuncILLI ASMGetNodeFlux;

/* Globaldaten-Schnittstelle */
IntFuncVoid ASMGetFlowModelType;
DoubleFuncVoid GetTheta_ASM;
VoidFuncDouble SetTheta_ASM;

