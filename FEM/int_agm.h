/**************************************************************************/
/* ROCKFLOW - Modul: int_agm.h
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aGM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/* Programmaenderungen:
      27.06.97       R.Kaiser              letzte Aenderung
                                                                          */
/**************************************************************************/

#ifndef int_agm_INC

#define int_agm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_agm.h"
#include "cel_agm.h"


/* Deklarationen */


/* Elementdaten-Schnittstelle */

extern VoidFuncLDX AGMSetElementConductanceMatrix;
  /* Setzt die conductance matrix des Elements number */
extern VoidFuncLDX AGMSetElementCapacitanceMatrix;
  /* Setzt die capacitance matrix des Elements number */
extern VoidFuncLDX AGMSetElementGravityVector;
  /* Setzt den gravity vector des Elements number */
extern VoidFuncLD AGMSetElementDDummy;
  /* Setzt Double-Wert des Elements number */
extern DoubleXFuncLong AGMGetElementConductanceMatrix;
  /* Liefert die conductance matrix des Elements number */
extern DoubleXFuncLong AGMGetElementCapacitanceMatrix;
  /* Liefert die capacitance matrix des Elements number */
extern DoubleXFuncLong AGMGetElementGravityVector;
  /* Liefert den gravity vector des Elements number */
extern DoubleFuncLong AGMGetElementDt;
  /* Liefert Courant-Zeitschritt des Elements number */
extern DoubleFuncLong AGMGetElementDDummy;
  /* Liefert Double-Wert des Elements number */
extern VoidFuncLong AGMDeleteElementConductanceMatrix;
  /* Erdet den conductance matrix-Zeiger des Elements number */
extern VoidFuncLong AGMDeleteElementCapacitanceMatrix;
  /* Erdet den capacitance matrix-Zeiger des Elements number */
extern VoidFuncLong AGMDeleteElementGravityVector;
  /* Erdet den gravity vector-Zeiger des Elements number */

extern VoidFuncLD AGMSetElementXVelo;
  /* Setzt x-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD AGMSetElementYVelo;
  /* Setzt y-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD AGMSetElementZVelo;
  /* Setzt z-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD AGMSetElementXMiddle;
  /* Setzt x-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD AGMSetElementYMiddle;
  /* Setzt y-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD AGMSetElementZMiddle;
  /* Setzt z-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD AGMSetElementXFlux;
  /* Setzt x-Komponente des Massenstroms des Elements number */
extern VoidFuncLD AGMSetElementYFlux;
  /* Setzt y-Komponente des Massenstroms des Elements number */
extern VoidFuncLD AGMSetElementZFlux;
  /* Setzt z-Komponente des Mittelstroms des Elements number */


/* Knotendaten-Schnittstelle */
extern DoubleFuncLong AGMGetNodePress0;
  /* Liefert Druck 0 am Knoten number */
extern VoidFuncLD AGMSetNodePress0;
  /* Setzt Druck 0 am Knoten number */
extern DoubleFuncLong AGMGetNodePress1;
  /* Liefert Druck 1 am Knoten number */
extern VoidFuncLD AGMSetNodePress1;
  /* Setzt Druck 1 am Knoten number */

/* Globaldaten-Schnittstelle */
extern VoidFuncDouble AGMSetGravityAcceleration;
extern DoubleFuncVoid AGMGetGravityAcceleration;
extern VoidFuncDouble AGMSetReferenceTemperature;
extern DoubleFuncVoid AGMGetReferenceTemperature;
extern VoidFuncDouble AGMSetReferencePressure;
extern DoubleFuncVoid AGMGetReferencePressure;
extern VoidFuncDouble AGMSetPermeability;
extern DoubleFuncVoid AGMGetPermeability;
extern VoidFuncDouble AGMSetTheta;
extern DoubleFuncVoid AGMGetTheta;
extern VoidFuncDouble AGMSetConvergenceCriterion;
extern DoubleFuncVoid AGMGetConvergenceCriterion;
extern VoidFuncInt AGMSetMaximumIterationNumber;
extern IntFuncVoid AGMGetMaximumIterationNumber;


/* Weitere externe Objekte */


#endif
