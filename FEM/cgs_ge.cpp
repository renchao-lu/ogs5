/**************************************************************************
   ROCKFLOW - Modul: cgs_ge.c
 
   Aufgabe:
   Aufstellen des Gesamtgleichungssystems fuer das Modul GE.

 
   Programmaenderungen:
   11/2000     C.Thorenz     Erste Version
   03/2003     RK            Quellcode bereinigt, Globalvariablen entfernt
    
 
**************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTCGS_GE


/* Intern benutzte Module/Objekte */
#include "int_ge.h"
#include "elements.h"
#include "nodes.h"
#include "mathlib.h"
#include "matrix.h"
#include "edges.h"
#include "plains.h"
#include "femlib.h"
#include "adaptiv.h"
#include "rf_num_new.h"
#include "rf_pcs.h" //OK_MOD"

/* Tools */
#include "tools.h"


/* Definitionen */
double GESourceSinkFunction(long knoten, int *gueltig, double val);


/**************************************************************************
   ROCKFLOW - Funktion: MakeGSxD

   Aufgabe:
    Gleichungssystem-Eintraege der Matrizen

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:
   11/2000     C.Thorenz     Erste Version

 **************************************************************************/
void MakeGSxD_GE(int feld, double *rechts)
{
  long nn, i, index, elanz;
  double left_matrix[64];
  double right_vector[8];

  feld=feld;

  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);

  /* Linke und rechte Seite aufbauen aus Elementmatrizen aller Elemente */
  elanz = ElListSize();
  for (index = 0; index < elanz; index++)
    {
      if (ElGetElement(index) != NULL)
        {                              /* wenn Element existiert */
          if (ElGetElementActiveState(index))
            {                          /* nur aktive Elemente */
              nn = ElNumberOfNodes[ElGetElementType(index) - 1];

              MNulleVec(right_vector, nn);

              if (GEGetElementElectricConductivityMatrix)
                for (i = 0; i < (nn * nn); i++)
                  left_matrix[i] = GEGetElementElectricConductivityMatrix(index)[i];

              /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
              IncorporateMatrix(index, left_matrix, right_vector, rechts);
            }                          /* if = nur aktive Elemente */
        }                              /* if = Element existiert */
    }                                  /* for = Schleife ueber die Elemente */
}



/**************************************************************************
   ROCKFLOW - Funktion: MakeGS_GE

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *rechts: rechte Seite GS (Vektor)

   Ergebnis:
   - void -

   Programmaenderungen:
   11/2000     C.Thorenz     Erste Version

 **************************************************************************/
void MakeGS_GE(int feld, double *rechts)
{
  MakeGSxD_GE(feld, rechts);
#ifdef TESTCGS_GE
  DisplayMsgLn("MakeGS vor Punktquellen");
#endif

  /* Punktquellen einarbeiten */
  //OK_ST sprintf(name, "%s%i", name_source_electric_current_dc, feld + 1);
  //OK_ST IncorporateSourceSinkEx(name, rechts, GESourceSinkFunction);

  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  /* Dirichlet-Randbedingungen eintragen */
  //OK_BC sprintf(name, "%s%i", name_boundary_condition_voltage_dc, feld + 1);
  //OK_BC IncorporateBoundaryConditions(name, rechts);

}

/**************************************************************************
   ROCKFLOW - Funktion:  GESourceSinkFunction

   Aufgabe:
   Umrechnen von Elektriker- in Physikervorzeichen ...

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   11/2000 C.Thorenz Erste Version

 **************************************************************************/
double GESourceSinkFunction(long knoten, int *gueltig, double val)
{

  knoten = knoten;

  *gueltig = 1;

  return -val;
}
