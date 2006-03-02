/**************************************************************************
   ROCKFLOW - Modul: relax.c

   Aufgabe:

   Stellt Funktionen zur Relaxation von iterativen Loesungen zur Verfuegung


   Programmaenderungen:
   1/2001      C. Thorenz       Erste Version
   03/2003     RK               Quellcode bereinigt, Globalvariablen entfernt   
      

 **************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"
#include "nodes.h"
#include "mathlib.h"
#include "tools.h"

typedef struct
  {
    char *name;
    double *old_iteration;
    double *very_old_iteration;
    long vector_length;
  }
RELAX;

static int relax_number_of_structures = 0;
static RELAX *relax_structures;

/**************************************************************************
 ROCKFLOW - Funktion: RelaxInit

 Aufgabe:
   Initialisiert die Relaxation fuer eine Groesse

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Name der Groesse

 Ergebnis:

 Programmaenderungen:
   1/2001   CT    Erste Version

**************************************************************************/
void RelaxInit(char *name)
{
  relax_number_of_structures++;
  relax_structures = (RELAX *) Realloc(relax_structures, relax_number_of_structures * sizeof(RELAX));
  relax_structures[relax_number_of_structures - 1].name = NULL;
  relax_structures[relax_number_of_structures - 1].old_iteration = NULL;
  relax_structures[relax_number_of_structures - 1].very_old_iteration = NULL;
  relax_structures[relax_number_of_structures - 1].vector_length = 0;
  relax_structures[relax_number_of_structures - 1].name = (char *) Malloc(((int)strlen(name) + 2) * sizeof(char));
  strcpy(relax_structures[relax_number_of_structures - 1].name, name);
}

/**************************************************************************
 ROCKFLOW - Funktion: RelaxDestroy

 Aufgabe:
   Zerstoert die Relaxation

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:

 Programmaenderungen:
   1/2001   CT    Erste Version

**************************************************************************/
void RelaxDestroy(void)
{
  int i;
  for (i = 0; i < relax_number_of_structures; i++)
    {
      relax_structures[i].name = (char *) Free(relax_structures[i].name);
      relax_structures[i].old_iteration = (double *) Free(relax_structures[i].old_iteration);
      relax_structures[i].very_old_iteration = (double *) Free(relax_structures[i].very_old_iteration);
      relax_structures[i].vector_length = 0;
    }
  relax_structures = (RELAX *) Free(relax_structures);
}


/**************************************************************************
 ROCKFLOW - Funktion: RelaxIterationChanges

 Aufgabe:
   Fuehrt Relaxation aus

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   E char *name: Name der Groesse
   X double *ergebnis: Vektor mit Knotendaten in interner Nummerierung
   E int iter: Nummer des aktuellen Iterationsschritts
   E int ndx0: Knotenindex fuer die alte Zeitebene der Groesse
   E int ndx1: Knotenindex fuer die neue Zeitebene der Groesse
   E int methode: Relaxationsmethode
   E double *param: Relaxationsparameter


 Ergebnis:

 Programmaenderungen:
   1/2001   CT    Erste Version

**************************************************************************/
void RelaxIterationChanges(char *name, double *ergebnis, int iterationsschritt, int ndx0, int ndx1, int methode, double *param)
{
  static double stress = 1., stress_old = 1.;
  double *old, *very_old, change;
  long oscillation = 0, convergence = 0, node;
  int i, curr;
  
  ndx0=ndx0;

  for (i = 0; i < relax_number_of_structures; i++)
    if (!strcmp(relax_structures[i].name, name))
      break;

  if (i < relax_number_of_structures)
    {
      curr = i;
    }
  else
    {
      DisplayMsgLn("Error in RelaxIterationChanges!");
      abort();
    }

  old = relax_structures[curr].old_iteration;
  very_old = relax_structures[curr].very_old_iteration;


  switch (methode)
    {
    default:
    case 0:
      break;

    case 1:
      /* feste Daempfung */
      for (i = 0l; i < NodeListLength; i++)
        ergebnis[i] = (1. - param[0]) * ergebnis[i] + param[0] * GetNodeVal(NodeNumber[i], ndx1);

      break;

    case 2:
      /* Oszillationen des Feldes untersuchen */
      change = 0.;
      oscillation = 1;
      convergence = 1;

      if (NodeListLength != relax_structures[curr].vector_length)
        {
          relax_structures[curr].old_iteration = (double *) Realloc(relax_structures[curr].old_iteration, NodeListLength * sizeof(double));
          relax_structures[curr].very_old_iteration = (double *) Realloc(relax_structures[curr].very_old_iteration, NodeListLength * sizeof(double));
          relax_structures[curr].vector_length = NodeListLength;
          old = relax_structures[curr].old_iteration;
          very_old = relax_structures[curr].very_old_iteration;
          for (i = 0l; i < NodeListLength; i++)
            {
              very_old[i] = old[i] = ergebnis[i];
              ergebnis[i] = stress_old * ergebnis[i] + (1. - stress_old) * GetNodeVal(NodeNumber[i], ndx1);
            }
          break;
        }

      /* Die mittlere Veraenderung bestimmen */
      for (i = 0l; i < NodeListLength; i++)
        change += fabs(ergebnis[i] - GetNodeVal(NodeNumber[i], ndx1));
      change /= (double) NodeListLength;

      if (iterationsschritt <= 2)
        {
          stress = stress_old;
        }
      else
        {
          /* Oszillierende Knotenwerte suchen */
          for (i = 0l; i < NodeListLength; i++)
            {
              /* Veraendert sich der Knotenwert noch ? */
              if (fabs(ergebnis[i] - GetNodeVal(NodeNumber[i], ndx1)) > change * param[0])
                {
                  /* Oszilliert der Knotenwert? */
                  if ((Signum(ergebnis[i] - old[i]) != Signum(old[i] - very_old[i])) && (fabs(ergebnis[i] - old[i]) / (MKleinsteZahl + fabs(old[i] - very_old[i])) > 0.5))
                    oscillation += 1;
                  else
                    convergence += 1;
                }
            }
          stress *= pow((1. + param[1]) - (double) oscillation / (double) (oscillation + convergence), param[2]);
          stress = max(min(stress, 1.), 1. - param[3]);
        }

      for (i = 0l; i < NodeListLength; i++)
        {
          very_old[i] = old[i];
          old[i] = ergebnis[i];
        }

      if (iterationsschritt == 3)
        stress_old = stress;

      for (i = 0l; i < NodeListLength; i++)
        ergebnis[i] = stress * ergebnis[i] + (1. - stress) * GetNodeVal(NodeNumber[i], ndx1);

      break;

    case 3:
      /* Oszillationen eines Knotens als Massstab nehmen */
      change = 0.;

      if (NodeListLength != relax_structures[curr].vector_length)
        {
          relax_structures[curr].old_iteration = (double *) Realloc(relax_structures[curr].old_iteration, NodeListLength * sizeof(double));
          relax_structures[curr].very_old_iteration = (double *) Realloc(relax_structures[curr].very_old_iteration, NodeListLength * sizeof(double));
          relax_structures[curr].vector_length = NodeListLength;
          old = relax_structures[curr].old_iteration;
          very_old = relax_structures[curr].very_old_iteration;
          for (i = 0l; i < NodeListLength; i++)
            {
              very_old[i] = old[i] = ergebnis[i];
              ergebnis[i] = stress_old * ergebnis[i] + (1. - stress_old) * GetNodeVal(NodeNumber[i], ndx1);
            }
          break;
        }

      /* Die mittlere Veraenderung bestimmen */
      for (i = 0l; i < NodeListLength; i++)
        {
          change += fabs(ergebnis[i] - GetNodeVal(NodeNumber[i], ndx1));
        }
      change /= (double) NodeListLength;

      if (iterationsschritt <= 2)
        {
          stress = stress_old;
        }
      else
        {
          /* Oszillierenden Knotenwert suchen */
          node = -1;
          for (i = 0l; i < NodeListLength; i++)
            {
              /* Veraendert sich der Knotenwert noch ? */
              if (fabs(ergebnis[i] - GetNodeVal(NodeNumber[i], ndx1)) > change * param[0])
                {
                  /* Oszilliert der Knotenwert? */
                  if (Signum(ergebnis[i] - old[i]) != Signum(old[i] - very_old[i]))
                    {
                      if (node == -1)
                        node = i;
                      if (fabs(ergebnis[i] - old[i]) > fabs(ergebnis[node] - old[node]))
                        node = i;
                    }
                }
            }

          if (node != -1)
            {
              stress = fabs(very_old[node] - old[node]) / (fabs(ergebnis[node] - old[node]) + MKleinsteZahl) * param[1];
              stress = MMax(MMin(1., stress), param[1]);
            }
        }

      for (i = 0l; i < NodeListLength; i++)
        {
          very_old[i] = old[i];
          old[i] = ergebnis[i];
        }

      if (iterationsschritt == 3)
        stress_old = stress;

      for (i = 0l; i < NodeListLength; i++)
        ergebnis[i] = stress * ergebnis[i] + (1. - stress) * GetNodeVal(NodeNumber[i], ndx1);

      break;
    }
}

