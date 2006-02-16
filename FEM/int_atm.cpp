/**************************************************************************
   ROCKFLOW - Modul: int_atm.c
 
   Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aTM zu den diesen Rechenkern benutzenden Modulen bereit.
  
   Programmaenderungen:
   07/1996     MSR        Erste Version
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
                          femlib.c (-> CalcElementJacobiMatrix)
                          Funktionszeiger bereitstellen
   25.07.1997  R.Kaiser   Funktionszeiger ...ElementJacobiMatrix
                          entfernt (-> intrface.c)
   02/2000     C.Thorenz  Erweitert
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
 
**************************************************************************/

#include "stdafx.h"             /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "int_atm.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/* Elementdaten-Schnittstelle */

/* Alte (einphasige) Zugriffe auf Matrizen */
VoidFuncLDX ATMSetElementAdvMatrix = NULL;
VoidFuncLDX ATMSetElementDispMatrix = NULL;
VoidFuncLDX ATMSetElementMassMatrix = NULL;

DoubleXFuncLong ATMGetElementAdvMatrix = NULL;
DoubleXFuncLong ATMGetElementFluidContentMatrix = NULL;
DoubleXFuncLong ATMGetElementDispMatrix = NULL;
DoubleXFuncLong ATMGetElementMassMatrix = NULL;

VoidFuncLong ATMDeleteElementAdvMatrix = NULL;
VoidFuncLong ATMDeleteElementFluidContentMatrix = NULL;
VoidFuncLong ATMDeleteElementDispMatrix = NULL;
VoidFuncLong ATMDeleteElementMassMatrix = NULL;

/* Neue (mehrphasige) Zugriffe auf Matrizen */
VoidFuncLIDX ATMSetElementAdvMatrixNew = NULL;
VoidFuncLIDX ATMSetElementFluidContentMatrixNew = NULL;
VoidFuncLIDX ATMSetElementDispMatrixNew = NULL;
VoidFuncLIDX ATMSetElementMassMatrixNew = NULL;

DoubleXFuncLI ATMGetElementAdvMatrixNew = NULL;
DoubleXFuncLI ATMGetElementFluidContentMatrixNew = NULL;
DoubleXFuncLI ATMGetElementDispMatrixNew = NULL;
DoubleXFuncLI ATMGetElementMassMatrixNew = NULL;


VoidFuncLI ATMDeleteElementAdvMatrixNew = NULL;
VoidFuncLI ATMDeleteElementFluidContentMatrixNew = NULL;
VoidFuncLI ATMDeleteElementDispMatrixNew = NULL;
VoidFuncLI ATMDeleteElementMassMatrixNew = NULL;


VoidFuncLD ATMSetElementRetardFac = NULL;
VoidFuncLD ATMSetElementDt = NULL;
VoidFuncLD ATMSetElementPecletNum = NULL;
VoidFuncLID ATMSetElementDtNew = NULL;
VoidFuncLID ATMSetElementPecletNumNew = NULL;

DoubleFuncLong ATMGetElementRetardFac = NULL;
DoubleFuncLong ATMGetElementDt = NULL;
DoubleFuncLI ATMGetElementDtNew = NULL;
DoubleFuncLong ATMGetElementPecletNum = NULL;
DoubleFuncLI ATMGetElementPecletNumNew = NULL;
DoubleFuncILDDDD ATMGetFluidContent = NULL;
DoubleFuncILLI ATMGetNodeFluidContent = NULL;


/* Knotendaten-Schnittstelle */
DoubleFuncLIII ATMGetNodeConc = NULL;
DoubleFuncLong ATMGetNodePress = NULL;
VoidFuncLD ATMSetNodePress = NULL;
DoubleFuncLong ATMGetNodeConc0 = NULL;
VoidFuncLD ATMSetNodeConc0 = NULL;
DoubleFuncLong ATMGetNodeConc1 = NULL;
VoidFuncLD ATMSetNodeConc1 = NULL;
DoubleFuncILLI ATMGetNodeFlux = NULL;
DoubleFuncILLI ATMGetNodeSatu = NULL;

/* Globaldaten-Schnittstelle */

IntFuncVoid ATMGetNumberOfFluidPhases = NULL;
DoubleFuncVoid ATMGetGlobalTheta = NULL;
VoidFuncDouble ATMSetGlobalTheta = NULL;
