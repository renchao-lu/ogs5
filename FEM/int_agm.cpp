/**************************************************************************/
/* ROCKFLOW - Modul: int_agm.c
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
        Rechenkerns aGM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   04/1997     OK         Anpassung an das Gasmodell
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
                          femlib.c (-> CalcElementJacobiMatrix)
                          Funktionszeiger bereitstellen
   25.07.1997  R.Kaiser   Funktionszeiger ...ElementJacobiMatrix
                          entfernt (-> intrface.c)

   letzte Aenderung    R.Kaiser   25.06.1997
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "int_agm.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/* Elementdaten-Schnittstelle */

VoidFuncLDX AGMSetElementConductanceMatrix;
VoidFuncLDX AGMSetElementCapacitanceMatrix;
VoidFuncLDX AGMSetElementGravityVector;
VoidFuncLD AGMSetElementDDummy;
VoidFuncLD AGMSetElementXVelo;
VoidFuncLD AGMSetElementYVelo;
VoidFuncLD AGMSetElementZVelo;
VoidFuncLD AGMSetElementXMiddle;
VoidFuncLD AGMSetElementYMiddle;
VoidFuncLD AGMSetElementZMiddle;
VoidFuncLD AGMSetElementXFlux;
VoidFuncLD AGMSetElementYFlux;
VoidFuncLD AGMSetElementZFlux;
DoubleXFuncLong AGMGetElementConductanceMatrix;
DoubleXFuncLong AGMGetElementCapacitanceMatrix;
DoubleXFuncLong AGMGetElementGravityVector;
DoubleFuncLong AGMGetElementDt;
DoubleFuncLong AGMGetElementDDummy;
VoidFuncLong AGMDeleteElementConductanceMatrix;
VoidFuncLong AGMDeleteElementCapacitanceMatrix;
VoidFuncLong AGMDeleteElementGravityVector;

/* Knotendaten-Schnittstelle */
DoubleFuncLong AGMGetNodePress0;
VoidFuncLD AGMSetNodePress0;
DoubleFuncLong AGMGetNodePress1;
VoidFuncLD AGMSetNodePress1;

/* Globaldaten-Schnittstelle */
VoidFuncDouble AGMSetGravityAcceleration;
DoubleFuncVoid AGMGetGravityAcceleration;
VoidFuncDouble AGMSetReferenceTemperature;
DoubleFuncVoid AGMGetReferenceTemperature;
VoidFuncDouble AGMSetReferencePressure;
DoubleFuncVoid AGMGetReferencePressure;
VoidFuncDouble AGMSetPermeability;
DoubleFuncVoid AGMGetPermeability;
VoidFuncDouble AGMSetTheta;
DoubleFuncVoid AGMGetTheta;
VoidFuncDouble AGMSetConvergenceCriterion;
DoubleFuncVoid AGMGetConvergenceCriterion;
VoidFuncInt AGMSetMaximumIterationNumber;
IntFuncVoid AGMGetMaximumIterationNumber;

