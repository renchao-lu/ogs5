/**************************************************************************/
/* ROCKFLOW - Modul: int_mtm.c
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aTM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/* Programmaenderungen:
   12/1998     AH        Erste Version: Mehrkomponentenmodel
   03/1999     AH        Loesungs- Faellungsreaktionen
   12/1999     AH        Reaktiver Transport im Mehrkomponentenmodell
   03/2000     AH        Thermohaline Stroemungen.
                         Waermetransport und Zerfallskettenreaktionen in
                         Salzwasser.


   letzte Aenderung:    A.Habbar    29.03.1999
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "int_mtm.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/* Elementdaten-Schnittstelle */

VoidFuncLD MTMSetElementRetardFac;
VoidFuncLD MTMSetElementDt;
VoidFuncLD MTMSetElementPecletNum;
VoidFuncLDX MTMSetElementAdvMatrix;
VoidFuncLDX MTMSetElementDispMatrix;
VoidFuncLDX MTMSetElementMassMatrix;
VoidFuncLDX MTMSetElementSorptionMatrix;
VoidFuncLDX MTMSetElementSorptionIsothermMatrix;
VoidFuncLDX MTMSetElementDecayMatrix;
VoidFuncLDX MTMSetElementDissolutionMatrix;
VoidFuncLDX MTMSetElementDissolutionVector;
VoidFuncLDX MTMSetElementSorptionVector;
VoidFuncLDX MTMSetElementReactionVector;
VoidFuncLDX MTMSetElementReactionMatrix;
VoidFuncLDX MTMSetElementChainReactionVector;
VoidFuncLD MTMSetElementDDummy;

DoubleXFuncLong MTMGetElementAdvMatrix;
DoubleXFuncLong MTMGetElementDispMatrix;
DoubleXFuncLong MTMGetElementMassMatrix;
DoubleXFuncLong MTMGetElementSorptionMatrix;
DoubleXFuncLong MTMGetElementSorptionIsothermMatrix;
DoubleXFuncLong MTMGetElementDecayMatrix;
DoubleXFuncLong MTMGetElementDissolutionMatrix;
DoubleXFuncLong MTMGetElementDissolutionVector;
DoubleXFuncLong MTMGetElementSorptionVector;
DoubleXFuncLong MTMGetElementReactionVector;
DoubleXFuncLong MTMGetElementReactionMatrix;
DoubleXFuncLong MTMGetElementChainReactionVector;
DoubleFuncLong MTMGetElementRetardFac;
DoubleFuncLong MTMGetElementDt;
DoubleFuncLong MTMGetElementPecletNum;
DoubleFuncLong MTMGetElementDDummy;

VoidFuncLong MTMDeleteElementAdvMatrix;
VoidFuncLong MTMDeleteElementDispMatrix;
VoidFuncLong MTMDeleteElementMassMatrix;
VoidFuncLong MTMDeleteElementSorptionMatrix;
VoidFuncLong MTMDeleteElementSorptionIsothermMatrix;
VoidFuncLong MTMDeleteElementDecayMatrix;
VoidFuncLong MTMDeleteElementDissolutionMatrix;
VoidFuncLong MTMDeleteElementDissolutionVector;
VoidFuncLong MTMDeleteElementSorptionVector;
VoidFuncLong MTMDeleteElementReactionVector;
VoidFuncLong MTMDeleteElementReactionMatrix;
VoidFuncLong MTMDeleteElementChainReactionVector;

VoidFuncLID MTMSetElementFluidDensity;
DoubleFuncLI MTMGetElementFluidDensity;
VoidFuncLD MTMSetElementSoilPorosity;
DoubleFuncLong MTMGetElementSoilPorosity;

/* Knotendaten-Schnittstelle */
DoubleFuncLong MTMGetNodePress;
VoidFuncLD MTMSetNodePress;
DoubleFuncLI MTMGetNodeConc;
VoidFuncLID MTMSetNodeConc;
DoubleFuncLong MTMGetNodeConc0;
VoidFuncLD MTMSetNodeConc0;
DoubleFuncLong MTMGetNodeConc1;
VoidFuncLD MTMSetNodeConc1;
DoubleFuncLI MTMGetNodeSorb;
VoidFuncLID MTMSetNodeSorb;
DoubleFuncLong MTMGetNodeSorb0;
VoidFuncLD MTMSetNodeSorb0;
DoubleFuncLong MTMGetNodeSorb1;
VoidFuncLD MTMSetNodeSorb1;
DoubleFuncLong MTMGetNodeTemp0;
VoidFuncLD MTMSetNodeTemp0;
DoubleFuncLong MTMGetNodeTemp1;
VoidFuncLD MTMSetNodeTemp1;
