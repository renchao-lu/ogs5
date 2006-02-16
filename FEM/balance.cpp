/**************************************************************************
 ROCKFLOW - Modul: balance.c

 Aufgabe:
   Enthaelt die Methoden zur Bilanzierung von Fluessen

 Programmaenderungen:
   5/2001   CT   Erste Version

**************************************************************************/

/*========================================================================
 INCLUDES
========================================================================*/
#include "stdafx.h"		       /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"



#include "geo_strings.h"
#include "nodes.h"
#include "mathlib.h"

/* Header / Andere intern benutzte Module */
#include "balance.h"


/*========================================================================
 INTERNAL DECLARATIONS
========================================================================*/

static int balance_sets = 0;
static BALANCESET *bs_array = NULL;
static char *balance_file_name = NULL;
VoidFuncIntFileX BalanceWriteModelDataHeader = NULL;
VoidFuncIntFileXLX BalanceWriteModelData = NULL;


/* Functions ------------------------------------------------------------- */
BALANCESET *BalanceCreateObject(void);
GEOMETRY_OBJECT *BalanceCreateGeometryObject(BALANCESET * bs_set);
void BalanceDestroyAllGeometryObjects(BALANCESET * bs_set);
void BalanceMethod0(GEOMETRY_OBJECT * go, long *balance_nodes);
void BalanceMethod1(GEOMETRY_OBJECT * go, long *balance_nodes);
void BalanceMethod2(GEOMETRY_OBJECT * go, long *balance_nodes);



/**************************************************************************
 ROCKFLOW - Funktion: FctBalance

 Aufgabe:
   Konstruiert BALANCE Objekte aus dem RFD-File

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei

 Ergebnis:
   0 bei Fehler, sonst 1

 Programmaenderungen:
   5/2001   CT    Erste Version
**************************************************************************/
int FctBalance(char *data, int found, FILE * f)
{
  int ok = 1, i;
  int pos = 2, p = 0, p_sub = 0, unknown_subkeyword = 0;
  int ivalue = 0;
  int beginn = 0;
  char *name = NULL, *sub = NULL;
  BALANCESET *bs = NULL;
  GEOMETRY_OBJECT *go = NULL;

  name = (char *) Malloc(((int)strlen(data) + 2) * sizeof(char));
  sub = (char *) Malloc(((int)strlen(data) + 2) * sizeof(char));

  LineFeed(f);

#ifdef TESTFILES
  DisplayMsg("Eingabedatenbank, Schluesselwort #BALANCE ");
  DisplayMsgLn(variable_name);
#endif

  if (!found)
    {				       /* #BALANCE nicht gefunden */
    }
  else
    {				       /* #BALANCE  gefunden */
      FilePrintString(f, "#BALANCE");
      LineFeed(f);

#ifdef EXT_RFD
      FilePrintString(f, "; Keyword: #BALANCE");
      LineFeed(f);
      FilePrintString(f, "; Specifies objects for balances");
      LineFeed(f);
#endif


      if (found == 1)
	{
	  /* Schleife ueber wiederholte Schluesselworte */
	  while (StrTestHash(&data[p], &pos))
	    {
              p += pos;
  
	      /* Erzeugen eines neuen Objekts */
	      bs = BalanceCreateObject();

	      /* Schleife ueber Sub-Keywords */
	      while (StrReadSubKeyword(sub, data, p, &beginn, &p))
		{

		  /* Lesen des Sub-Keywords */
		  ok = StrReadStr(name, sub, f, TFString, &p_sub) && ok;
		  unknown_subkeyword = 1;
		  pos = 0;

		  /* Auswerten des Sub-Keywords */
		  if (!strcmp(name, "$GEOMETRY_OBJECT"))
		    {

#ifdef EXT_RFD
		      LineFeed(f);
		      FilePrintString(f, "; - Geometrie-Objekte ($GEOMETRY_OBJECTS) [0,1,2]");
		      LineFeed(f);
		      FilePrintString(f, ";   0: Single point: x,y,z,epsilon");
		      LineFeed(f);
		      FilePrintString(f, ";   1: Open polygon: number_ofpoints, x0,y0,z0,...,xn,yn,zn,epsilon");
		      LineFeed(f);
		      FilePrintString(f, ";   2: Area bounded by four points on plain: x0,y0,z0,...,x3,y3,z3,epsilon");
		      LineFeed(f);
#else
		      FilePrintString(f, "; geometry objects");
		      LineFeed(f);
#endif

		      unknown_subkeyword = 0;

		      while (StrTestInt(&sub[p_sub += pos]))
			{
			  go = BalanceCreateGeometryObject(bs);

			  ok = (StrReadInt(&ivalue, &sub[p_sub], f, TFInt, &pos) && ok);
			  go -> method = ivalue;

			  switch (ivalue)
			    {
			    default:
			      DisplayMsgLn("#BALANCE");
			      DisplayMsgLn(" $GEOMETRY_OBJECT: Unknown method!");
			      break;

			    case 0:
			      go -> number_coordinates = 1;
			      go -> x = (double *) Malloc(sizeof(double));
			      go -> y = (double *) Malloc(sizeof(double));
			      go -> z = (double *) Malloc(sizeof(double));
			      ok = (StrReadDouble(go -> x, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      ok = (StrReadDouble(go -> y, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      ok = (StrReadDouble(go -> z, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      ok = (StrReadDouble(&go -> epsilon, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      break;

			    case 1:
			      ok = (StrReadInt(&go -> number_coordinates, &sub[p_sub += pos], f, TFInt, &pos) && ok);
			      go -> x = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      go -> y = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      go -> z = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      for (i = 0; i < go -> number_coordinates; i++)
				{
				  ok = (StrReadDouble(&go -> x[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				  ok = (StrReadDouble(&go -> y[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				  ok = (StrReadDouble(&go -> z[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				}
			      ok = (StrReadDouble(&go -> epsilon, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      break;

			    case 2:
			      go -> number_coordinates = 4;
			      go -> x = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      go -> y = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      go -> z = (double *) Malloc(go -> number_coordinates * sizeof(double));
			      for (i = 0; i < go -> number_coordinates; i++)
				{
				  ok = (StrReadDouble(&go -> x[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				  ok = (StrReadDouble(&go -> y[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				  ok = (StrReadDouble(&go -> z[i], &sub[p_sub += pos], f, TFDouble, &pos) && ok);
				}
			      ok = (StrReadDouble(&go -> epsilon, &sub[p_sub += pos], f, TFDouble, &pos) && ok);
			      break;
			    }	       /* endswitch */
			}	       /* endwhile */
		    }		       /* endwhile */
		}		       /* endif */

	      if (unknown_subkeyword)
		{
		  FilePrintString(f, "* !!! Error: Unknown Subkeyword! $");
		  FilePrintString(f, name);
		  LineFeed(f);
		  DisplayMsgLn("* !!! Keyword #BALANCE");
		  DisplayMsg("* !!! Error: Unknown Subkeyword! ");
		  DisplayMsgLn(name);
		  exit(1);
		}
	      p_sub += pos;
	      pos = 0;
	    }			       /* Endwhile */
	}			       /* endif found==1 */


      /* Sub-Keywords ausgeben */
      if (found == 2)
	{
	}


      /* Ende der Leseroutine */
    }				       /* endif "found" */

  name = (char *) Free(name);
  sub = (char *) Free(sub);

  return ok;
}




/**************************************************************************
 ROCKFLOW - Funktion: BalanceCreateObject

 Aufgabe:
   Konstruktion von Balance-Objekten

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   BALANCESET: Zeiger auf Objekt

 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
BALANCESET *BalanceCreateObject(void)
{
  balance_sets++;

  if (!balance_sets)
    bs_array = (BALANCESET *) Malloc(sizeof(BALANCESET));
  else
    bs_array = (BALANCESET *) Realloc(bs_array, (balance_sets * sizeof(BALANCESET)));

  bs_array[balance_sets - 1].method = 0;
  bs_array[balance_sets - 1].number_geometry_objects = 0;
  bs_array[balance_sets - 1].geometry_objects = NULL;

  return &bs_array[balance_sets - 1];
}




/*************************************************************************
 ROCKFLOW - Funktion: BalanceInitAllObjects

 Aufgabe:
   Initialisierung aller BALANCE Objekte

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char * : Name der RockFlow-Eingabedatei ohne Extension

 Ergebnis:
   - void -

 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

*************************************************************************/
void BalanceInitAllObjects(char *dateiname)
{
  int i;
  char name[256];
  FILE *file;

  /* Dateiname fuer spaetere Nutzung speichern */
  balance_file_name = (char *) Malloc((int)strlen(dateiname) + 1);
  sprintf(balance_file_name, "%s", dateiname);

  if (BalanceWriteModelDataHeader)
    {
      for (i = 0; i < balance_sets; i++)
	{
	  sprintf(name, "%s.B%i", balance_file_name, i);
	  file = fopen(name, "w");
	  if (file)
	    {
              /* Schreiben des Headers, bisher gibt es nur Methode 0 */ 
              BalanceWriteModelDataHeader(bs_array[i].method, file);

	      fclose(file);
	    }
	}
    }
}



/*************************************************************************
 ROCKFLOW - Funktion: BalanceDestroyAllObjects

 Aufgabe:
   Destruktion aller BALANCE Objekte

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -

 Ergebnis:
   - void -

 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

*************************************************************************/
void BalanceDestroyAllObjects(void)
{
  int i;
  for (i = 0; i < balance_sets; i++)
    {
      BalanceDestroyAllGeometryObjects(&bs_array[i]);
    }

  bs_array = (BALANCESET *) Free(bs_array);
  balance_sets = 0;

  balance_file_name = (char *) Free(balance_file_name);
}





/**************************************************************************
 ROCKFLOW - Funktion: BalanceCreateGeometryObject

 Aufgabe:
   Konstruktion von Geometry-Objekten in Balance-Objekten

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: BALANCESET *bs_set    Zeiger auf das aktuelle Balance-Objekt

 Ergebnis:
   GEOMETRY_OBJECT: Zeiger auf Geometrie-Objekt

 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
GEOMETRY_OBJECT *BalanceCreateGeometryObject(BALANCESET * bs_set)
{
  bs_set -> number_geometry_objects++;

  if (!bs_set -> geometry_objects)
    bs_set -> geometry_objects = (GEOMETRY_OBJECT *) Malloc(sizeof(GEOMETRY_OBJECT));
  else
    bs_set -> geometry_objects = (GEOMETRY_OBJECT *) Realloc(bs_set -> geometry_objects, (bs_set -> number_geometry_objects * sizeof(GEOMETRY_OBJECT)));

  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].method = -1;
  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].number_coordinates = 0;
  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].x = NULL;
  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].y = NULL;
  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].z = NULL;
  bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1].epsilon = -1.;

  return &bs_set -> geometry_objects[bs_set -> number_geometry_objects - 1];
}


/**************************************************************************
 ROCKFLOW - Funktion: BalanceDestroyAllGeometryObjects

 Aufgabe:
   Destruktion aller Geometry-Objekte in einem Balance-Objekt

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: BALANCESET *bs_set    Zeiger auf das aktuelle Balance-Objekt

 Ergebnis:
   
 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
void BalanceDestroyAllGeometryObjects(BALANCESET * bs_set)
{
  int i;

  for (i = 0; i < bs_set -> number_geometry_objects; i++)
    {
      bs_set -> geometry_objects[i].x = (double *) Free(bs_set -> geometry_objects[i].x);
      bs_set -> geometry_objects[i].y = (double *) Free(bs_set -> geometry_objects[i].y);
      bs_set -> geometry_objects[i].z = (double *) Free(bs_set -> geometry_objects[i].z);
    }

  bs_set -> geometry_objects = (GEOMETRY_OBJECT *) Free(bs_set -> geometry_objects);
}




/**************************************************************************
 ROCKFLOW - Funktion: BalanceOverAllGeometryObjects

 Aufgabe:
   Ermittelt Bilanzen fuer alle Geometry-Objekte

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   
 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version 

**************************************************************************/
void BalanceOverAllGeometryObjects(void)
{
  long i, j;
  long *balance_nodes;
  char name[256];
  FILE *file;

  if (BalanceWriteModelData)
    {

      balance_nodes = (long *) Malloc(sizeof(long) * NodeListSize());
      for (i = 0; i < balance_sets; i++)
	{
	  for (j = 0; j < NodeListSize(); j++)
	    balance_nodes[j] = 0;

	  for (j = 0; j < bs_array[i].number_geometry_objects; j++)
	    {
	      switch (bs_array[i].geometry_objects[j].method)
		{
		case 0:
		  BalanceMethod0(&bs_array[i].geometry_objects[j], balance_nodes);
		  break;
		case 1:
		  BalanceMethod1(&bs_array[i].geometry_objects[j], balance_nodes);
		  break;
		case 2:
		  BalanceMethod2(&bs_array[i].geometry_objects[j], balance_nodes);
		  break;
		}
	    }

	  sprintf(name, "%s.B%i", balance_file_name, (int) i);
	  file = fopen(name, "a");
	  if (file)
	    {
	      BalanceWriteModelData(bs_array[i].method, file, balance_nodes);
	      fclose(file);
	    }
	}

      balance_nodes = (long *) Free(balance_nodes);
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: BalanceMethod0

 Aufgabe:
   Ermittelt zugehoerige Knoten um einen Punkt

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: GEOMETRY_OBJECT * go: Pointer auf betrachtetes Geometrieobjekt
   R: long *balance_nodes: Array mit 0/1 fuer die zu betrachtenden Knoten

 Ergebnis:
   
 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
void BalanceMethod0(GEOMETRY_OBJECT * go, long *balance_nodes)
{

  long i;
  double node_coor[3];
  double point_coor[3];

  point_coor[0] = go -> x[0];
  point_coor[1] = go -> y[0];
  point_coor[2] = go -> z[0];

  for (i = 0; i < NodeListSize(); i++)
    {
      if (GetNode(i))
	{
	  node_coor[0] = GetNodeX(i);
	  node_coor[1] = GetNodeY(i);
	  node_coor[2] = GetNodeZ(i);

	  if (MCalcDistancePointToPoint(point_coor, node_coor) <= go -> epsilon)
	    balance_nodes[i] = 1;
	}
    }
}

/**************************************************************************
 ROCKFLOW - Funktion: BalanceMethod1

 Aufgabe:
   Ermittelt zugehoerige Knoten an einem Polygonzug

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: GEOMETRY_OBJECT * go: Pointer auf betrachtetes Geometrieobjekt
   R: long *balance_nodes: Array mit 0/1 fuer die zu betrachtenden Knoten

 Ergebnis:
   
 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
void BalanceMethod1(GEOMETRY_OBJECT * go, long *balance_nodes)
{
  long node, i;
  double dist12, dist1p, dist2p; 
  double node_coor[3];
  double point_coor0[3];
  double point_coor1[3];
  double pt1[3];

  for (node = 0; node < NodeListSize(); node++)
    {
      if (GetNode(node))
         {
          node_coor[0] = GetNodeX(node);
	  node_coor[1] = GetNodeY(node);
	  node_coor[2] = GetNodeZ(node);

         /* Schleife ueber alle Punkte des Poygonzuges */
         for (i = 0; i < go -> number_coordinates - 1; i++)
          {

          point_coor0[0] = go -> x[i];
          point_coor0[1] = go -> y[i];
          point_coor0[2] = go -> z[i];

          point_coor1[0] = go -> x[i+1];
          point_coor1[1] = go -> y[i+1];
          point_coor1[2] = go -> z[i+1];

          /* Ist der Knoten nah am Polygonabschnitt? */
          if (MCalcDistancePointToLine(node_coor, point_coor0, point_coor1) <= go->epsilon )
           {
              /* Im folgenden wird mit der Projektion weitergearbeitet */
              MCalcProjectionOfPointOnLine(node_coor, point_coor0, point_coor1, pt1);

              /* Laenge des Polygonabschnitts */
              dist12 = MCalcDistancePointToPoint(point_coor1, point_coor0);

              /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
              dist1p = MCalcDistancePointToPoint(point_coor0, pt1);
              /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
              dist2p = MCalcDistancePointToPoint(point_coor1, pt1);

              /* Ist der Knoten innerhalb des Intervalls? */
              if ((dist12 - dist1p - dist2p + MKleinsteZahl) / (dist12 + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl)
                {
        	    balance_nodes[node] = 1;
                }
            }                          /* endif */
        }                              /* for */
      }                                  /* endif */
    }                                  /* for */

  return;
}



/**************************************************************************
 ROCKFLOW - Funktion: BalanceMethod2

 Aufgabe:
   Ermittelt zugehoerige Knoten in einem konvexen Viereck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: GEOMETRY_OBJECT * go: Pointer auf betrachtetes Geometrieobjekt
   R: long *balance_nodes: Array mit 0/1 fuer die zu betrachtenden Knoten

 Ergebnis:
   
 Programmaenderungen:
   5/2001     C. Thorenz        Erste Version

**************************************************************************/
void BalanceMethod2(GEOMETRY_OBJECT * go, long *balance_nodes)
{
  long node, i;
  double dist12, dist1p, dist2p; 
  double node_coor[3];
  double point_coor0[3];
  double point_coor1[3];
  double pt1[3];

  for (node = 0; node < NodeListSize(); node++)
    {
      if (GetNode(node))
         {
          node_coor[0] = GetNodeX(node);
	  node_coor[1] = GetNodeY(node);
	  node_coor[2] = GetNodeZ(node);

         /* Schleife ueber alle Punkte des Poygonzuges */
         for (i = 0; i < go -> number_coordinates - 1; i++)
          {

          point_coor0[0] = go -> x[i];
          point_coor0[1] = go -> y[i];
          point_coor0[2] = go -> z[i];

          point_coor1[0] = go -> x[i+1];
          point_coor1[1] = go -> y[i+1];
          point_coor1[2] = go -> z[i+1];

          /* Ist der Knoten nah am Polygonabschnitt? */
          if (MCalcDistancePointToLine(node_coor, point_coor0, point_coor1) <= go->epsilon )
           {
              /* Im folgenden wird mit der Projektion weitergearbeitet */
              MCalcProjectionOfPointOnLine(node_coor, point_coor0, point_coor1, pt1);

              /* Laenge des Polygonabschnitts */
              dist12 = MCalcDistancePointToPoint(point_coor1, point_coor0);

              /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
              dist1p = MCalcDistancePointToPoint(point_coor0, pt1);
              /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
              dist2p = MCalcDistancePointToPoint(point_coor1, pt1);

              /* Ist der Knoten innerhalb des Intervalls? */
              if ((dist12 - dist1p - dist2p + MKleinsteZahl) / (dist12 + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl)
                {
        	    balance_nodes[node] = 1;
                }
            }                          /* endif */
        }                              /* for */
      }                                  /* endif */
    }                                  /* for */

  return;

}
