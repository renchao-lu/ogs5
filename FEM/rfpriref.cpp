/**************************************************************************/
/* ROCKFLOW - Modul: rfpriref.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp APRIORI_REFINE
   - Verwaltung von a priori Verfeinerungen mit dem Datentyp LIST_APRIORI_REFINE
                                                                          */
/* Programmaenderungen:
   08/1998    AH      Erste Version
   10/1999    AH      Anpassung
                      Ersetzen von ElPlaceNewElement() durch CreateElementTopology()
                      und NewNode() durch CreateNodeGeometry()
   07/2000    AH      Behandlung von degenerierten Elemente.
   10/2000    AH      Anisotropes Vorverfeinern (Neue Methoden 17 und 18)
                      Methode 6 geaendert (Lesen ueber Koordinaten)
   03/2003    RK      Quellcode bereinigt, Globalvariablen entfernt   
                         
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "elements.h"
#include "mathlib.h"
#include "rf_pcs.h" //OK_MOD"
#include "rfpriref.h"
//OK_IC #include "rfinitc.h"

// GeoLib
#include "geo_pnt.h"
#include "geo_strings.h"
#include "msh_elements_rfi.h"

/* Interne (statische) Deklarationen */


/* Definitionen */

static LIST_APRIORI_REFINE *list_of_refine_elements = NULL;
  /* Liste von Anfangszustaende */


#define APRIORIREFINE


/**************************************************************************/
/* ROCKFLOW - Funktion: create_apriori_refine
                                                                          */
/* Aufgabe:
   Konstruktor for APRIORI_REFINE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Adresse des AR's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1998    AH      Erste Version
   10/2001    AH      Anisotropes Vorverfeinern
                                                                          */
/**************************************************************************/
APRIORI_REFINE *create_apriori_refine(char *name)
{
  APRIORI_REFINE *ar;

  ar = (APRIORI_REFINE *) Malloc(sizeof(APRIORI_REFINE));
  if ( ar == NULL ) return NULL;


  ar->name = (char *) Malloc((int)strlen(name)+1);
  if ( ar->name == NULL ) {
         Free(ar);
         return NULL;
  }
  strcpy(ar->name,name);

  ar->type=0;
  ar->mode=0;

  ar->start_element=-1;
  ar->end_element=-1;
  ar->step_elements=1;
  ar->count_of_values=0;
  ar->values=NULL;
  ar->count_of_points=0;
  ar->x=NULL;
  ar->y=NULL;
  ar->z=NULL;
  ar->refine_type=0;
  ar->radius=0;
  ar->distribution_type=0;

  ar->base_type=0;
  ar->base_value=0.0;
  return ar;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_apriori_refine
                                                                          */
/* Aufgabe:
   Destructor for APRIORI_REFINE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des AR's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_apriori_refine(void *member)
{
  APRIORI_REFINE *ar=(APRIORI_REFINE *)member;

  if ( ar->name ) ar->name=(char *)Free(ar->name);
  if ( ar->values ) ar->values=(double *)Free(ar->values);
  if ( ar->x ) ar->x=(double *)Free(ar->x);
  if ( ar->y ) ar->y=(double *)Free(ar->y);
  if ( ar->z ) ar->z=(double *)Free(ar->z);
  if ( ar ) ar=(APRIORI_REFINE *)Free(ar);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_apriori_refine
                                                                          */
/* Aufgabe:
   Initialisator for APRIORI_REFINE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Adresse des AR's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *init_apriori_refine(APRIORI_REFINE *ar, \
                                                                                        long count_of_values)
{
  if (count_of_values <= 0) return ar;

  if ( ar->values ) ar->values=(double *)Free(ar->values);

  ar->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( ar->values == NULL ) return NULL;

  ar->count_of_values=count_of_values;
  return ar;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_apriori_refine
                                                                          */
/* Aufgabe:
   Anfangszustand loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Adresse des AR's.
                                                                          */
/* Ergebnis:
   - Adresse des AR's -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *delete_apriori_refine(APRIORI_REFINE *ar)
{
  if ( ar->values ) ar->values=(double *)Free(ar->values);
  ar->count_of_values=0;

  return ar;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: init_apriori_refine_coor
                                                                          */
/* Aufgabe:
   Initialisator for APRIORI_REFINE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Adresse des AR's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *init_apriori_refine_coor(APRIORI_REFINE *ar, \
                                                                                        long count_of_points)
{
  if (count_of_points <= 0) return NULL;

  if (ar->x) ar->x=(double *)Free(ar->x);
  if (ar->y) ar->y=(double *)Free(ar->y);
  if (ar->z) ar->z=(double *)Free(ar->z);

  ar->x=(double *)Malloc(count_of_points*sizeof(double));
  if ( ar->x == NULL ) return NULL;
  ar->y=(double *)Malloc(count_of_points*sizeof(double));
  if ( ar->y == NULL ) {
        ar->x=(double *)Free(ar->x);
        return NULL;
  }
  ar->z=(double *)Malloc(count_of_points*sizeof(double));
  if ( ar->z == NULL ) {
        ar->x=(double *)Free(ar->x);
        ar->y=(double *)Free(ar->y);
        return NULL;
  }

  ar->count_of_points=count_of_points;
  return ar;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_apriori_refine_coor
                                                                          */
/* Aufgabe:
   Anfangszustand loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Adresse des AR's.
                                                                          */
/* Ergebnis:
   - Adresse des AR's -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *delete_apriori_refine_coor(APRIORI_REFINE *ar)
{
  if ( ar->x ) ar->x=(double *)Free(ar->x);
  if ( ar->y ) ar->y=(double *)Free(ar->y);
  if ( ar->z ) ar->z=(double *)Free(ar->z);
  ar->count_of_points=0;

  return ar;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: sread_apriori_refine
                                                                          */
/* Aufgabe:
   Anfangszustand aus einem String lesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Adresse des AR's.
   E char *string: Zeiger auf den String.
                                                                          */
/* Ergebnis:
   - Adresse des AR's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *sread_apriori_refine(APRIORI_REFINE *ar, char *string)
{
  long i;
  if (!ar->values) return NULL;

  if (sscanf(string,"%ld %ld %ld",&ar->start_element,&ar->end_element,&ar->step_elements) != 3) return NULL;

  for (i=0; i<ar->count_of_values; i++)
    if (sscanf(string,"%lf",&ar->values[i]) != 1) return NULL;

  return ar;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: fread_apriori_refine
                                                                          */
/* Aufgabe:
   Anfangszustand aus einer Datei lesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Adresse des AR's.
   E FILE *fp: Zeiger auf die Datei (vom Standardtyp FILE).
                                                                          */
/* Ergebnis:
   - Adresse des AR's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
APRIORI_REFINE *fread_apriori_refine(APRIORI_REFINE *ar, FILE *fp)
{
  long i;

  if (!ar->values) return NULL;

  if (fscanf(fp,"%ld %ld %ld",&ar->start_element,&ar->end_element,&ar->step_elements) != 3) return NULL;

  for (i=0; i<ar->count_of_values; i++)
    if (fscanf(fp,"%lf",&ar->values[i]) != 1) return NULL;

  return ar;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_name
                                                                          */
/* Aufgabe:
   Liefert Name des AR's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Name des AR's -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_ar_name(APRIORI_REFINE *ar)
{
  if (ar->name)
    return ar->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_name
                                                                          */
/* Aufgabe:
   Setzt Name des AR's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_name(APRIORI_REFINE *ar, char *name)
{
  if (!name) return;
  if(ar) {
    ar->name=(char *)Free(ar->name);
    ar->name = (char *) Malloc((int)strlen(name)+1);
    if ( ar->name == NULL ) return;
    strcpy(ar->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: apriori_refine_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines AR-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long apriori_refine_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_refine_elements->count_of_refine_elements_name; i++)
    if(strcmp(StrUp(list_of_refine_elements->names_of_refine_elements[i]),StrUp(name)) == 0) break;
  if (i < list_of_refine_elements->count_of_refine_elements_name) return ++i;
  else return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_type
                                                                          */
/* Aufgabe:
   Setzt Typ des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long start_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_type(APRIORI_REFINE *ar, long type)
{
  if (ar) ar->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_type
                                                                          */
/* Aufgabe:
   Liefert Typ des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_type(APRIORI_REFINE *ar)
{
        return (ar)?ar->type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_mode
                                                                          */
/* Aufgabe:
   Setzt Modus des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long mode: Modus (0: Ueberschreibmodus, 1: Einfuegemodus)                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_mode(APRIORI_REFINE *ar, long mode)
{
  if (ar) ar->mode=mode;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_mode
                                                                          */
/* Aufgabe:
   Liefert Modus des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Modus -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_mode(APRIORI_REFINE *ar)
{
        return (ar)?ar->mode:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_base_type
                                                                          */
/* Aufgabe:
   Setzt Basistyp des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long base_type: Entscheidet wie die Unterteilung erfolgt.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_ar_base_type(APRIORI_REFINE *ar, long base_type)
{
  if (ar) ar->base_type=base_type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_base_type
                                                                          */
/* Aufgabe:
   Liefert Basistyp des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long get_ar_base_type(APRIORI_REFINE *ar)
{
   return (ar)?ar->base_type:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_base_value
                                                                          */
/* Aufgabe:
   Setzt Basiswert des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E doubel base_value: Basiswert.
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_ar_base_value(APRIORI_REFINE *ar, double base_value)
{
  if (ar) ar->base_value=base_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_base_value
                                                                          */
/* Aufgabe:
   Liefert Basiswert des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_ar_base_value(APRIORI_REFINE *ar)
{
   return (ar)?ar->base_value:-1.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des AR's -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_count_of_values(APRIORI_REFINE *ar)
{
  return (ar)?ar->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_count_of_values(APRIORI_REFINE *ar, long count_of_values)
{
  if (ar) ar->count_of_values=count_of_values;
  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_start_element
                                                                          */
/* Aufgabe:
   Setzt Anfangsknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long start_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_start_element(APRIORI_REFINE *ar, long start_element)
{
  if (ar) ar->start_element=start_element;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_start_element
                                                                          */
/* Aufgabe:
   Liefert Anfangsknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Knotennummer des Anfangsknotens -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_start_element(APRIORI_REFINE *ar)
{
        return (ar)?ar->start_element:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_end_element
                                                                          */
/* Aufgabe:
   Setzt Endknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long end_element: Endknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_end_element(APRIORI_REFINE *ar, long end_element)
{
  if (ar) ar->end_element=end_element;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_end_element
                                                                          */
/* Aufgabe:
   Liefert Endknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Knotennummer des Endknotens -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_end_element(APRIORI_REFINE *ar)
{
        return (ar)?ar->end_element:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_step_elements
                                                                          */
/* Aufgabe:
   Setzt Schrittweite der Zwischenknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_step_elements(APRIORI_REFINE *ar, long step_elements)
{
  if (ar) ar->step_elements=step_elements;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_step_elements
                                                                          */
/* Aufgabe:
   Liefert Schrittweite der Zwischenknoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Knotenschrittweite -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_step_elements(APRIORI_REFINE *ar)
{
        return (ar)?ar->step_elements:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_value(APRIORI_REFINE *ar, long index, double value)
{
  if (ar && ar->values) {
    if ( (ar->count_of_values >= index-1) && (index >= 1) )
          ar->values[index-1]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ar_value(APRIORI_REFINE *ar, long index)
{
  double rv=-1.0;

  if (ar && ar->values) {
    if ( (ar->count_of_values >= index-1) && (index >= 1) )
          rv = ar->values[index-1];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) (Returnwert -1)!");
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (AR) (Returnwert -1)!");
  }

  return rv;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_refine_type
                                                                          */
/* Aufgabe:
   Setzt Verteilungstyp des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long refine_type: Verteilungstyp
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_refine_type(APRIORI_REFINE *ar, long refine_type)
{
  if (ar) ar->refine_type=refine_type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_refine_type
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Verteilungstyp -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_refine_type(APRIORI_REFINE *ar)
{
        return (ar)?ar->refine_type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_x
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
   E long xcoor: X-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_x(APRIORI_REFINE *ar, long index, double xcoor)
{
  if (ar && ar->x) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          ar->x[index-1]=xcoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_x
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - X-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ar_x(APRIORI_REFINE *ar, long index)
{
  double rv = -1.0;

  if (ar && ar->x) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          rv =  ar->x[index-1];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) (Returnwert -1)!");
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (AR) (Returnwert -1)!");
  }
  
  return rv;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_y
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
   E long xcoor: Y-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_y(APRIORI_REFINE *ar, long index, double ycoor)
{
  if (ar && ar->y) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          ar->y[index-1]=ycoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_y
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - Y-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ar_y(APRIORI_REFINE *ar, long index)
{
  double rv=-1.0;

  if (ar && ar->y) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          rv = ar->y[index-1];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) (Returnwert -1)!");
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (AR) (Returnwert -1)!");
  }

  return rv;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_z
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
   E long xcoor: X-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_z(APRIORI_REFINE *ar, long index, double zcoor)
{
  if (ar && ar->z) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          ar->z[index-1]=zcoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_z
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - X-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ar_z(APRIORI_REFINE *ar, long index)
{
  double rv = -1.0;

  if (ar && ar->z) {
    if ( (ar->count_of_points >= index-1) && (index >= 1) )
          rv = ar->z[index-1];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (AR) (Returnwert -1)!");
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (AR) (Returnwert -1)!");
  }

  return rv;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_radius
                                                                          */
/* Aufgabe:
   Setzt Radius des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long radius: Verteilungsradius
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_radius(APRIORI_REFINE *ar, double radius)
{
  if (ar) ar->radius=radius;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_radius
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Verteilungsradius -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ar_radius(APRIORI_REFINE *ar)
{
        return (ar)?ar->radius:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_distribution_type
                                                                          */
/* Aufgabe:
   Setzt Verteilungstyp des IC's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INITIAL_CONDITIONS *ic: Zeiger auf die Datenstruktur ic.
   E long distribution_type: Verteilungstyp
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_distribution_type(APRIORI_REFINE *ar, long distribution_type)
{
  if (ar) ar->distribution_type=distribution_type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ic_distribution_type
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des IC's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INITIAL_CONDITIONS *ic: Zeiger auf die Datenstruktur ic.
                                                                          */
/* Ergebnis:
   - Verteilungstyp -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_distribution_type(APRIORI_REFINE *ar)
{
        return (ar)?ar->distribution_type:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ar_level
                                                                          */
/* Aufgabe:
   Setzt Modus des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
   E long level: Modus (0: Ueberschreibmodus, 1: Einfuegemodus)                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ar_level(APRIORI_REFINE *ar, long level)
{
  if (ar) ar->level=level;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ar_level
                                                                          */
/* Aufgabe:
   Liefert Modus des AR's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - Modus -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ar_level(APRIORI_REFINE *ar)
{
        return (ar)?ar->level:-1;
}





/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_apriori_refine_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Anfangszustaende;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Listenname
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
LIST_APRIORI_REFINE *create_apriori_refine_list (char *name)
{
  LIST_APRIORI_REFINE *list_of_refine_elements;

  list_of_refine_elements = (LIST_APRIORI_REFINE *) Malloc(sizeof(LIST_APRIORI_REFINE));
  if ( list_of_refine_elements == NULL ) return NULL;


  list_of_refine_elements->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_refine_elements->name == NULL ) {
         Free(list_of_refine_elements);
         return NULL;
  }
  strcpy(list_of_refine_elements->name,name);

  list_of_refine_elements->ar_list=create_list();
  if ( list_of_refine_elements->ar_list == NULL ) {
    Free(list_of_refine_elements->name);
        Free(list_of_refine_elements);
        return NULL;
  }

  list_of_refine_elements->names_of_refine_elements=NULL;
  list_of_refine_elements->count_of_refine_elements_name=0;

  return list_of_refine_elements;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_apriori_refine_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der Anfangszustaende aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_apriori_refine_list(void)
{
  if (list_of_refine_elements->name) list_of_refine_elements->name=(char *)Free(list_of_refine_elements->name);
  if(list_of_refine_elements->names_of_refine_elements) \
    list_of_refine_elements->names_of_refine_elements = \
      (char **)Free(list_of_refine_elements->names_of_refine_elements);


  if (list_of_refine_elements->ar_list) {
    delete_list(list_of_refine_elements->ar_list,destroy_apriori_refine);
    list_of_refine_elements->ar_list=destroy_list(list_of_refine_elements->ar_list);
  }

  if (list_of_refine_elements) list_of_refine_elements=(LIST_APRIORI_REFINE *)Free(list_of_refine_elements);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: refine_elements_list_empty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 Liste --> leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int refine_elements_list_empty(void)
{
  if (!list_of_refine_elements) return 0;
  else if (!list_of_refine_elements->ar_list) return 0;
  else return list_empty(list_of_refine_elements->ar_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_ar_init
                                                                          */
/* Aufgabe:
   Liste der AR's initialisieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void list_ar_init(void)
{
  if (list_of_refine_elements) list_current_init(list_of_refine_elements->ar_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_ar_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_ar_next (void)
{
  return list_of_refine_elements->ar_list ?
             get_list_next(list_of_refine_elements->ar_list): NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_apriori_refine_list_item
                                                                          */
/* Aufgabe:
   Loescht eine Element vom Typ APRIORI_REFINE aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur,
                         die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl der verbleibenden Elemente
                                                                          */
/* Programmaenderungen:
   05/2000   OK   Erste Version
                                                                          */
/**************************************************************************/
long remove_apriori_refine_list_item (APRIORI_REFINE *ar)
{
  list_of_refine_elements->count_of_refine_elements = \
      remove_list_member(list_of_refine_elements->ar_list,(void *) ar, destroy_apriori_refine);
  return list_of_refine_elements->count_of_refine_elements;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: delete_apriori_refine_list
                                                                          */
/* Aufgabe:
   Entfernt Inhalt der Liste ohne diese zu zerstoeren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/2000   OK   Erste Version
                                                                          */
/**************************************************************************/
void delete_apriori_refine_list(void)
{
  APRIORI_REFINE *ar=NULL;

  list_ar_init();

  while ((ar=(APRIORI_REFINE *)get_list_ar_next()) != NULL ) {
      remove_apriori_refine_list_item(ar);
  }

}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_apriori_refine_name
                                                                          */
/* Aufgabe:
   Setzt den Anfangszustand in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen AR's_Namen -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_apriori_refine_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  for ( i=0; i<list_of_refine_elements->count_of_refine_elements_name; i++)
    if(strcmp(list_of_refine_elements->names_of_refine_elements[i],name) == 0) break;
  if (i < list_of_refine_elements->count_of_refine_elements_name) return i;

  list_of_refine_elements->names_of_refine_elements= \
          (char **) Realloc(list_of_refine_elements->names_of_refine_elements, \
          (list_of_refine_elements->count_of_refine_elements_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_refine_elements->names_of_refine_elements[i]=new_name;

  return ++(list_of_refine_elements->count_of_refine_elements_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_refine_elements_name
                                                                          */
/* Aufgabe:
   Alle Vorverfeinerer von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der entferneten AR's_Namen -
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_refine_elements_names (void)
{
  long i;

  for (i=0; i<list_of_refine_elements->count_of_refine_elements_name; i++)
    if(list_of_refine_elements->names_of_refine_elements[i]) \
      list_of_refine_elements->names_of_refine_elements[i] = \
        (char *)Free(list_of_refine_elements->names_of_refine_elements[i]);

  return list_of_refine_elements->count_of_refine_elements_name;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: insert_apriori_refine_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ APRIORI_REFINE in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_apriori_refine_list (APRIORI_REFINE *ar)
{
  list_of_refine_elements->count_of_refine_elements=append_list(list_of_refine_elements->ar_list,(void *) ar);
  return list_of_refine_elements->count_of_refine_elements;
}



/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateRefineElementList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Anfangszustaende (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void CreateRefineElementList(void)
{
  list_of_refine_elements = create_apriori_refine_list("APRIORI_REFINE");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyRefineElementList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von Anfangszustaende (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void DestroyRefineElementList(void)
{
  undeclare_refine_elements_names();
  destroy_apriori_refine_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: RefineElementListEmpty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 Liste --> leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int RefineElementListEmpty(void)
{
  return refine_elements_list_empty();
}





/**************************************************************************/
/* ROCKFLOW - Funktion: AprioriRefineElements
                                                                          */
/* Aufgabe:
   Setzt den Anfangszustand in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E APRIORI_REFINE *ar: Zeiger auf die Datenstruktur ar.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void AprioriRefineElements (char *name)
{
  APRIORI_REFINE *ar=NULL;

  if (!name) return;


  if (RefineElementListEmpty()) return;

  if ( !apriori_refine_name_exist(name) ) return;

#ifdef APRIORIREFINE
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("Vorverfeinerer:");
    DisplayMsg("Anzahl Elemente (Input) = ");
    DisplayLong(anz_1D+anz_2D+anz_3D);
    DisplayMsgLn("");
#endif

  list_ar_init();
  while ( (ar=(APRIORI_REFINE *)get_list_ar_next()) != NULL ) {
    if ( strcmp(StrUp(get_ar_name(ar)),StrUp(name)) ) continue;

    switch (get_ar_type(ar)) {
          case 0:
            APrioriRefineElementsLevelM0(ar);
            break;
          case 1:
            APrioriRefineElementsLevelM1(ar);
            break;
          case 2:
            APrioriRefineElementsLevelM2(ar);
          case 3:
            APrioriRefineElementsLevelM3(ar);
            break;
          case 4:
            APrioriRefineElementsLevelM4(ar);
            break;
          case 5:
            APrioriRefineElementsLevelM5(ar);
            break;
          case 6:
            APrioriRefineElementsLevelM6(ar);
            break;
          case 7:
            APrioriRefineElementsLevelM7(ar);
            break;
          case 8:
            APrioriRefineElementsLevelM8(ar);
            break;
          case 9:
            APrioriRefineElementsLevelM9(ar);
            break;
          case 10:
            APrioriRefineElementsLevelM10(ar);
            break;
          case 11:
            APrioriRefineElementsLevelM11(ar);
            break;
              case 12:
                APrioriRefineElementsLevelM12(ar);
                break;
              case 13:
                APrioriRefineElementsLevelM13(ar);
                break;
              case 14:
                APrioriRefineElementsLevelM14(ar);
                break;
              case 15:
                APrioriRefineElementsLevelM15(ar);
                break;
              case 16:
                APrioriRefineElementsLevelM16(ar);
                break;
              case 17:
                APrioriRefineElementsLevelM17(ar);
                break;
              case 18:
                APrioriRefineElementsLevelM18(ar);
                break;
        } /* switch */
  } /* while */

#ifdef APRIORIREFINE
    DisplayMsg("Anzahl Elemente (Output) = ");
    DisplayLong(anz_1D+anz_2D+anz_3D);
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
#endif

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM0
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM0 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursiv(ar,i,1,search); break;
/*Fehler*/        /* case 1: APrioriRefine1DElementLevelMode0(ar,i,search); break; */
          case 2: if (!TestIfElementTriangle(i))
                    APrioriRefine2DElementLevelAllModeRekursiv(ar,i,1,search);
                  else
                    APrioriRefine2DDElementLevelAllModeRekursiv(ar,i,1,search); break;
          case 3: APrioriRefine3DElementLevelAllModeRekursiv(ar,i,1,search); break;
        } /* switch */
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM1
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM1 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursivLeftRight(ar,i,1,search); break;
          case 2: APrioriRefine2DElementLevelAllModeRekursivT2(ar,i,1,search); break;
          /*case 3: APrioriRefine1DElementLevelAllModeRekursivLeftRight(ar,i,1,search); break;*/
        } /* switch */
  }

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM2
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM2 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursivNode(ar,i,1,search); break;
          case 2: APrioriRefine2DElementLevelAllModeRekursivNode(ar,i,1,search); break;
          case 3: APrioriRefine2DElementLevelAllModeRekursivNode(ar,i,1,search); break;
        } /* switch */
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM3
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM3 (APRIORI_REFINE *ar)
{
  long node;
  node = GetNodeNumberClose(get_ar_x(ar,1), get_ar_y(ar,1), get_ar_z(ar,1) );

  set_ar_start_element(ar,node);
  APrioriRefineElementsLevelM2(ar);

/*    case 1: APrioriRefine1DElementLevelAllModeRekursivNode(ar,node,1,search); break;*/
/*  APrioriRefine2DElementLevelAllModeRekursivNode(ar,node,1,search);*/

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM4
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM4 (APRIORI_REFINE *ar)
{
  long node=GetNodeNumberClose(get_ar_x(ar,1), get_ar_y(ar,1), \
                                                           get_ar_z(ar,1) );

  set_ar_start_element(ar,node);
  APrioriRefineElementsLevelM2(ar);
  APrioriDeleteAllElementNode(ar);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM5
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM5 (APRIORI_REFINE *ar)
{
  long i,list_length;

  list_length=NodeListLength;
  for (i=0;i<list_length;i++) {
    set_ar_start_element(ar,i);
    APrioriRefineElementsLevelM2(ar);
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM6
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM6 (APRIORI_REFINE *ar)
{
  int search=1;
  long numnode1=get_ar_start_element(ar);
  long numnode2=get_ar_end_element(ar);
  long i=get_ar_step_elements(ar);

  APrioriRefine2DElementLevelAllModeRekursivEdge(ar,i,numnode1,numnode2,1,search);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM7
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM7 (APRIORI_REFINE *ar)
{
  /* Variablen */
  /*long i,list_size;*/
  long element_type;
  int search=1;
  long element=get_ar_start_element(ar);

/*  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursiv(ar,i,1,search); break;
          case 2: APrioriRefine2DElementLevelAllModeRekursivNodeM2(ar,i,numnode,1,search); break;
        }
  } */

  switch (element_type=ElGetElementType(element)) {
    case 1: APrioriRefine1DElementLevelAllModeRekursiv(ar,element,1,search); break;
    case 2: APrioriRefine2DElementLevelAllModeRekursiv(ar,element,1,search); break;
  } /* switch */


  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM8
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM8 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
      case 2: APrioriRefine2DElementLevelAllModeRekursivT3 (ar,i,1,search); break;
        } /* switch */
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM9
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM9 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
      case 2: APrioriRefine2DElementLevelAllModeRekursivT4 (ar,i,1,search); break;
        } /* switch */
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM10
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM10 (APRIORI_REFINE *ar)
{
  /*double val;*/
  long j;
  double xp,yp,zp,dist;
  long node1,node2;
  double x1,y1,z1,x2,y2,z2,dist12/*,dv*/;
  double dp1[3], d12[3], dxp1[3];

  long node_list_length=NodeListLength;

  node1=GetNodeNumberClose((x1=get_ar_x(ar,1)), (y1=get_ar_y(ar,1)), \
                                                          (z1=get_ar_z(ar,1)) );
  if (node1<0) return;
  node2=GetNodeNumberClose((x2=get_ar_x(ar,2)), (y2=get_ar_y(ar,2)), \
                                                          (z2=get_ar_z(ar,2)) );
  if (node2<0) return;

  d12[0]=x2-x1;
  d12[1]=y2-y1;
  d12[2]=z2-z1;
  dist12=MBtrgVec(d12,3);
  if (dist12<0) return;

/*  dv=get_ar_value(ar,2)-get_ar_value(ar,1);*/

  for (j=0;j<node_list_length;j++) {
    xp=GetNodeX(j);
        yp=GetNodeY(j);
        zp=GetNodeZ(j);
        dp1[0]=xp-x1;
        dp1[1]=yp-y1;
        dp1[2]=zp-z1;
    dist=MBtrgVec(dp1,3);
        if (dist<0) return;
    M3KreuzProdukt(dp1,d12,dxp1);

    if ( MBtrgVec(dxp1,3)<MKleinsteZahl && dist<=(MBtrgVec(d12,3)+MKleinsteZahl) \
                        && MSkalarprodukt(dp1,d12,3)>=0 ) {
      set_ar_start_element(ar,j);
      APrioriRefineElementsLevelM2(ar);
        }
  } /* for */

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM11
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM11 (APRIORI_REFINE *ar)
{
  /*double val;*/
  long j;
  double xp,yp,zp,dist;
  long node;
  double x,y,z;
  double dp1[3];
  double radius=get_ar_radius(ar);
  long   type=get_ar_distribution_type(ar);

  long node_list_length=NodeListLength;

  node=GetNodeNumberClose((x=get_ar_x(ar,1)), (y=get_ar_y(ar,1)), \
                                                          (z=get_ar_z(ar,1)) );
  if (GetNode(node)==NULL) {
    DisplayErrorMsg("Fehler in APrioriRefineElementsLevelM11  --> Abbruch !!!");
    abort();
  }


  for (j=0;j<node_list_length;j++) {
    xp=GetNodeX(j);
        yp=GetNodeY(j);
        zp=GetNodeZ(j);
        dp1[0]=xp-x;
        dp1[1]=yp-y;
        dp1[2]=zp-z;
    dist=MBtrgVec(dp1,3);
        if (dist<0) return;
    if (0<=type && type<=2) dist=Vek3dDistCoor(x,y,z,xp,yp,zp,type);
        else dist=-1.;
    if (dist<0) continue;
    if (dist<radius) {
      set_ar_start_element(ar,j);
      APrioriRefineElementsLevelM2(ar);
        }
        else continue;
  } /* for j */

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM12
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM12 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,list_size,element_type;
  int search=1;
  long start_elements=0;
  list_size=ElListSize();
  for (i=start_elements;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
      case 2: APrioriRefine2DElementLevelAllModeRekursivT5 (ar,i,1,search); break;
        } /* switch */
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM13
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM13 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long numele=get_ar_start_element(ar);
  long i,list_size;
  int search=1;
  long start_elements=0;

  list_size=ElListSize();
  for (i=start_elements;i<list_size;i++) {
    if ((numele==ElGetElementStartNumber(i)) && (ElGetElementType(i)==2)) {
      APrioriRefine2DElementLevelAllModeRekursivT5(ar,i,1,search); break;
      }
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM14
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM14 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long i,list_size,element_type,level;
  int search=1;
  long start_elements=0;

  set_ar_level(ar,1);
  for (level=1; level<=apriori_level; level++) {
    list_size=ElListSize();
    for (i=start_elements;i<list_size;i++) {
      switch (element_type=ElGetElementType(i)) {
        case 2: APrioriRefine2DElementLevelAllModeRekursivT5 (ar,i,1,search); break;
          } /* switch */
    } /* for i */
  } /* for level */

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM15
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM15 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long numele=get_ar_start_element(ar);
  long i,list_size,level;
  int search=1;
  long start_elements=0;

  set_ar_level(ar,1);
  for (level=1; level<=apriori_level; level++) {
    list_size=ElListSize();
    for (i=start_elements;i<list_size;i++) {
      if ((numele==ElGetElementStartNumber(i)) && (ElGetElementType(i)==2)) {
        APrioriRefine2DElementLevelAllModeRekursivT5 (ar,i,1,search);
          } /* switch */
    } /* for i */
  } /* for level */

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM16
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM16 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long i,j,k,list_size,element_type;
  int search=1,found;

  double xp,yp,zp,dist;
  long node1,node2;
  double x1,y1,z1,x2,y2,z2,dist12;
  double dp1[3], d12[3], dxp1[3];
  long *nodes;
  Knoten *node[4];
  long start_elements=0;

  list_size=ElListSize();
  for (i=start_elements;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
      case 2: {
  found=0;
  node1=GetNodeNumberClose((x1=get_ar_x(ar,1)), (y1=get_ar_y(ar,1)), \
                                                          (z1=get_ar_z(ar,1)) );
  if (node1<0) return;
  node2=GetNodeNumberClose((x2=get_ar_x(ar,2)), (y2=get_ar_y(ar,2)), \
                                                          (z2=get_ar_z(ar,2)) );
  if (node2<0) return;


  d12[0]=x2-x1;
  d12[1]=y2-y1;
  d12[2]=z2-z1;
  dist12=MBtrgVec(d12,3);
  if (dist12<0) return;



  nodes = ElGetElementNodes(i);
  for (k=0; k<4; k++) {
    node[k]= GetNode(nodes[k]);
  }

  for (j=0;j<4;j++) {
    xp=node[j]->x;
        yp=node[j]->y;
        zp=node[j]->z;
        dp1[0]=xp-x1;
        dp1[1]=yp-y1;
        dp1[2]=zp-z1;
    dist=MBtrgVec(dp1,3);
        if (dist<0) return;
    M3KreuzProdukt(dp1,d12,dxp1);

    if ( MBtrgVec(dxp1,3)<MKleinsteZahl && dist<=(MBtrgVec(d12,3)+MKleinsteZahl) \
                        && MSkalarprodukt(dp1,d12,3)>=0 ) {
       found=1;
        }
  } /* for */

      if (found) APrioriRefine2DElementLevelAllModeRekursivT5 (ar,i,1,search); break;
      }
        } /* switch */
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM17
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM17 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long list_size,element_type;
  int search=1;
  long numnode1=get_ar_start_element(ar);
  long numnode2=get_ar_end_element(ar);
  long i=get_ar_step_elements(ar);

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotrop(ar,i,numnode1,numnode2,1,search); break;
          case 2: APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop(ar,i,numnode1,numnode2,1,search); break;
          case 3: break;
        } /* switch */
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefineElementsLevelM18
                                                                          */
/* Aufgabe:
   Verfeinert alle Elemente a priori bis zu einem Level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void APrioriRefineElementsLevelM18 (APRIORI_REFINE *ar)
{
  /* Variablen */
  long list_size,element_type;
  int search=1;
  long numnode1=get_ar_start_element(ar);
  long numnode2=get_ar_end_element(ar);
  long i=get_ar_step_elements(ar);
  long ni=get_ar_level(ar);
  

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
    switch (element_type=ElGetElementType(i)) {
          case 1: APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotropI(ar,i,numnode1,numnode2,ni,search); break;
          case 2: APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotropI(ar,i,numnode1,numnode2,ni,search); break;
          case 3: break;
        } /* switch */
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivLeftRight
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivLeftRight (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long pos;

  if (level>apriori_level || level<=0) return i;

  pos=i;
  pos=APrioriRefine1DElementLevelAllModeRekursivLeft(ar,pos,level,search);
  pos=APrioriRefine1DElementLevelAllModeRekursivRight(ar,pos,++level,search);

  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivLeft
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivLeft (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;
  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long number=-1;
  double x,y,z,dist=0.0;
  long group;

  if (level>apriori_level) return i;

      if((element_type=ElGetElementType(i))!=1) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          n1=nodes[0];
          n2=nodes[1];
      node1 = GetNode(n1);
      node2 = GetNode(n2);
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;
          x=(node1->x+node2->x)/2.0;
          y=(node1->y+node2->y)/2.0;
          z=(node1->z+node2->z)/2.0;

          old_node=n1;
                  node=NULL;
          if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
          if( node != NULL && dist<MKleinsteZahl) {
            new_node=number;
          }
                  else {
                node=CreateNodeGeometry();
                /*node=NewNode();*/
            node->x=x;
            node->y=y;
            node->z=z;
            new_node=AddNode(node);
          }

          /* Elemente Erzeugen */
          if (mode==0) {
        pos=i;
        ElDeleteElement (pos);
        CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;
          }

      ElSetElementStartNumber(pos,start_number);


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node;
            nodes[1]=new_node;
        old_node=new_node;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);
        /*if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);*/

                i=APrioriRefine1DElementLevelAllModeRekursivLeft(ar,pos,++level,search);
                --level;

          /* Elemente Erzeugen */
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;

        ElSetElementStartNumber(pos,start_number);


            /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node;
            nodes[1]=n2;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        ElSetElementActiveState(pos,1);
        /*if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);*/


  return pos;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivRight
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivRight (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;
  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long number=-1;
  double x,y,z,dist=0.0;
  long group;

  if (level>apriori_level) return i;

      if((element_type=ElGetElementType(i))!=1) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          n1=nodes[0];
          n2=nodes[1];
      node1 = GetNode(n1);
      node2 = GetNode(n2);
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;
          x=(node1->x+node2->x)/2.0;
          y=(node1->y+node2->y)/2.0;
          z=(node1->z+node2->z)/2.0;

      /* Aenderung bzgl. Left */
          old_node=n2;

          node=NULL;
          if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
          if( node != NULL && dist<MKleinsteZahl) {
            new_node=number;
          }
                  else {
                node=CreateNodeGeometry();
                /*node=NewNode();*/
            node->x=x;
            node->y=y;
            node->z=z;
            new_node=AddNode(node);
          }

          /* Elemente Erzeugen */
          if (mode==0) {
        pos=i;
        ElDeleteElement (pos);
        CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;
          }

      ElSetElementStartNumber(pos,start_number);


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                nodes[0]=old_node;
            nodes[1]=new_node;
        old_node=new_node;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        /*if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);*/

                i=APrioriRefine1DElementLevelAllModeRekursivLeft(ar,pos,++level,search);
                --level;

          /* Elemente Erzeugen */
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;

        ElSetElementStartNumber(pos,start_number);

            /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node;
      /* Aenderung bzgl. Left */
            nodes[1]=n1;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        ElSetElementActiveState(pos,1);
        /*if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);*/


  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivLeft
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivLeft (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4],new_node[5];

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j,k,kk,jj;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

          nnod=1;
          nele=nnod+1;


      /* Elemente */
      for (j=1;j<=nele*nele-1;j++) {

            if ((mode==0) && j == 1) {
          pos=i;
          ElDeleteElement (pos);
          CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        ElSetElementStartNumber(pos,start_number);


      /* Knoten Erzeugen */
          x[0]=y[0]=z[0]=0.0;
          for (k=0; k<4; k++) {
                if (k==3) kk=0;
                else kk=k+1;
        x[k+1]=((node[kk])->x+(node[k])->x)/2.0;
        y[k+1]=((node[kk])->y+(node[k])->y)/2.0;
        z[k+1]=((node[kk])->z+(node[k])->z)/2.0;
                x[0]=x[0]+x[k+1];
                y[0]=y[0]+y[k+1];
                z[0]=z[0]+z[k+1];
      }
          x[0]=x[0]/4.0;
          y[0]=y[0]/4.0;
          z[0]=z[0]/4.0;

          /* Knoten gegebenfalls erzeugen */
      for (k=0;k<=nele*nele;k++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[k],y[k],z[k],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[k]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[k];
          nnode->y=y[k];
          nnode->z=z[k];
          new_node[k]=AddNode(nnode);
                }
          }


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==1) jj=4;
            else jj=j-1;
            nodes[0]=old_node[j-1];
            nodes[1]=new_node[j];
            nodes[2]=new_node[0];
            nodes[3]=new_node[jj];

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

                APrioriRefine2DElementLevelAllModeRekursiv(ar,pos,++level,search);
                --level;
          }

      /* Elemente */
      for (j=3;j<=nele*nele;j++) {

            if ((mode==0) && j == 1) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        ElSetElementStartNumber(pos,start_number);


      /* Knoten Erzeugen */
          x[0]=y[0]=z[0]=0.0;
          for (k=0; k<4; k++) {
                if (k==3) kk=0;
                else kk=k+1;
        x[k+1]=((node[kk])->x+(node[k])->x)/2.0;
        y[k+1]=((node[kk])->y+(node[k])->y)/2.0;
        z[k+1]=((node[kk])->z+(node[k])->z)/2.0;
                x[0]=x[0]+x[k+1];
                y[0]=y[0]+y[k+1];
                z[0]=z[0]+z[k+1];
      }
          x[0]=x[0]/4.0;
          y[0]=y[0]/4.0;
          z[0]=z[0]/4.0;

          /* Knoten gegebenfalls erzeugen */
      for (k=0;k<=nele*nele;k++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[k],y[k],z[k],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[k]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[k];
          nnode->y=y[k];
          nnode->z=z[k];
          new_node[k]=AddNode(nnode);
                }
          }


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==1) jj=4;
            else jj=j-1;
            nodes[0]=old_node[j-1];
            nodes[1]=new_node[j];
            nodes[2]=new_node[0];
            nodes[3]=new_node[jj];

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);
          }


  return pos;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursiv
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version  !!! noch nicht fertig
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;
  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j;
  long number=-1;
  double x,y,z,dist=0.0;
  long group;


  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=1) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          n1=nodes[0];
          n2=nodes[1];
      node1 = GetNode(n1);
      node2 = GetNode(n2);
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;
          x=(node1->x+node2->x)/2.0;
          y=(node1->y+node2->y)/2.0;
          z=(node1->z+node2->z)/2.0;

          old_node=n1;
      for (j=1;j<=nele;j++) {
            if ((j%2)!=0) {
                  node=NULL;
          if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
          if( node != NULL && dist<MKleinsteZahl) {
            new_node=number;
          }
                  else {
                node=CreateNodeGeometry();
                /*node=NewNode();*/
            node->x=x;
            node->y=y;
            node->z=z;
            new_node=AddNode(node);
          }
                }
            else {
          new_node=n2;
                }

          /* Elemente Erzeugen */
          if ((mode==0) && j == 1) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;
          }

      ElSetElementStartNumber(pos,start_number);

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node;
            nodes[1]=new_node;
        old_node=new_node;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

                APrioriRefine1DElementLevelAllModeRekursiv(ar,pos,++level,search);
                --level;
          }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursiv
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4],new_node[5];

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j,k,kk,jj;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;
  long group;


  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;

          x[0]=y[0]=z[0]=0.0;
          for (k=0; k<4; k++) {
                if (k==3) kk=0;
                else kk=k+1;
        x[k+1]=((node[kk])->x+(node[k])->x)/2.0;
        y[k+1]=((node[kk])->y+(node[k])->y)/2.0;
        z[k+1]=((node[kk])->z+(node[k])->z)/2.0;
                x[0]=x[0]+x[k+1];
                y[0]=y[0]+y[k+1];
                z[0]=z[0]+z[k+1];
      }
          x[0]=x[0]/4.0;
          y[0]=y[0]/4.0;
          z[0]=z[0]/4.0;

          /* Knoten gegebenfalls erzeugen */
      for (j=0;j<=nele*nele;j++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[j],y[j],z[j],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[j]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[j];
          nnode->y=y[j];
          nnode->z=z[j];
          new_node[j]=AddNode(nnode);
                }
          }

      /* Elemente */
      for (j=1;j<=nele*nele;j++) {
          if ((mode==0) && j == 1) {
        pos=i;
        //OK3809 ElDeleteElement (pos);
        ELEDestroyTopology(pos);
        //CreateElementTopology(element_type,pred,0,pos);
        ELECreateTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
          }

      ElSetElementStartNumber(pos,start_number);


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==1) jj=4;
            else jj=j-1;
            nodes[0]=old_node[j-1];
            nodes[1]=new_node[j];
            nodes[2]=new_node[0];
            nodes[3]=new_node[jj];

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

                APrioriRefine2DElementLevelAllModeRekursiv(ar,pos,++level,search);
                --level;
          }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DDElementLevelAllModeRekursiv
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D (degeneriertes) Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/2000    AH      Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DDElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4],new_node[5];

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j,k,kk,jj;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;
  long group;
  long old_d2d_node[3];
  int dnode;
  Knoten *d2d_node[3];


  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
      pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      dnode=TestIfElementTriangle(i);
      kk=0;
      nodes = ElGetElementNodes(i);
      for (k=0; k<4; k++) {
        old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
        if (!(dnode==(k+1)))
        old_d2d_node[kk++]=nodes[k];
      }
      pred=ElGetElementPred(i);

      for (k=0; k<3; k++) {
        d2d_node[k]= GetNode(old_d2d_node[k]);
      }

      /* Knoten Erzeugen */
      nnod=1;
      nele=nnod+1;

      x[0]=y[0]=z[0]=0.0;
      for (k=0; k<3; k++) {
        if (k==2) kk=0;
        else kk=k+1;
        x[k+1]=((d2d_node[kk])->x+(d2d_node[k])->x)/2.0;
        y[k+1]=((d2d_node[kk])->y+(d2d_node[k])->y)/2.0;
        z[k+1]=((d2d_node[kk])->z+(d2d_node[k])->z)/2.0;
        x[0]=x[0]+x[k+1];
        y[0]=y[0]+y[k+1];
        z[0]=z[0]+z[k+1];
      }
      x[0]=x[0]/3.0;
      y[0]=y[0]/3.0;
      z[0]=z[0]/3.0;

          /* Knoten gegebenfalls erzeugen */
      for (j=0;j<=nele*nele-1;j++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[j],y[j],z[j],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[j]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[j];
          nnode->y=y[j];
          nnode->z=z[j];
          new_node[j]=AddNode(nnode);
                }
          }

      /* Elemente */
      for (j=1;j<=nele*nele-1;j++) {
          if ((mode==0) && j == 1) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
          }

      ElSetElementStartNumber(pos,start_number);


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==1) jj=3;
            else jj=j-1;
            nodes[0]=old_node[j-1];
            nodes[1]=new_node[j];
            nodes[2]=new_node[0];
            nodes[3]=new_node[jj];

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

                APrioriRefine2DElementLevelAllModeRekursiv(ar,pos,++level,search);
                --level;
          }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine3DElementLevelAllModeRekursiv
                                                                          */
/* Aufgabe:
   Verfeinert ein 3D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998   AH   Erste Version   Noch Fehlerhaft !!!
   11/2000   OK   Bugfix
                                                                          */
/**************************************************************************/
long APrioriRefine3DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);
  long group;

  Knoten *node[8],*nnode;
  long *nodes;
  /*long old_node[8],new_node[8],enew_node[12];*/
  long all_nodes[27];
  /*long volume_nodes[8],plains_nodes[6],edges_nodes[12],all_nodes[27];*/

  long nele;
  long pred;
  long element_type;
  long pos;
  long j,k,jj=0;
  long number=-1;
  double x[9],y[9],z[9],dist=0.0;
  double m[3];
  double xx[12],yy[12],zz[12];

  long VNumber[]={0,6,8,2,18,24,26,20};
  long PNumber[]={13,4,22,12,14,10,16};
  long ENumber[]={1,3,7,5,9,15,17,11,21,25,23,19};


  if (level>apriori_level) return 0;

  for (k=0; k<27; k++) all_nodes[k]=0;

  if((element_type=ElGetElementType(i))!=3) return 0;

  pos=i;
  ElSetElementActiveState(pos,0);
  group=ElGetElementGroupNumber(pos);

  nodes = ElGetElementNodes(i);
  for (k=0; k<8; k++) {
      all_nodes[VNumber[k]]=/*volume_nodes[k]=*/nodes[k];
      node[k]= GetNode(nodes[k]);
  }
  pred=ElGetElementPred(i);

  GetMiddelXPoint(node,m,8l);
  x[0]=m[0]; y[0]=m[1]; z[0]=m[2];

  {
      GetMiddelXPoint2D(node[0],node[1],node[2],node[3],m);
      x[1]=m[0]; y[1]=m[1]; z[1]=m[2];

      GetMiddelXPoint2D(node[4],node[5],node[6],node[7],m);
      x[2]=m[0]; y[2]=m[1]; z[2]=m[2];

      GetMiddelXPoint2D(node[0],node[4],node[5],node[1],m);
      x[3]=m[0]; y[3]=m[1]; z[3]=m[2];

      GetMiddelXPoint2D(node[3],node[7],node[6],node[2],m);
      x[4]=m[0]; y[4]=m[1]; z[4]=m[2];

      GetMiddelXPoint2D(node[0],node[3],node[7],node[4],m);
      x[5]=m[0]; y[5]=m[1]; z[5]=m[2];

      GetMiddelXPoint2D(node[1],node[2],node[6],node[5],m);
      x[6]=m[0]; y[6]=m[1]; z[6]=m[2];
  }

  GetMiddelXPoint1D(node[0],node[3],m);
  xx[0]=m[0]; yy[0]=m[1]; zz[0]=m[2];

  GetMiddelXPoint1D(node[0],node[1],m);
  xx[1]=m[0]; yy[1]=m[1]; zz[1]=m[2];

  GetMiddelXPoint1D(node[1],node[2],m);
  xx[2]=m[0]; yy[2]=m[1]; zz[2]=m[2];

  GetMiddelXPoint1D(node[2],node[3],m);
  xx[3]=m[0]; yy[3]=m[1]; zz[3]=m[2];

  GetMiddelXPoint1D(node[0],node[4],m);
  xx[4]=m[0]; yy[4]=m[1]; zz[4]=m[2];

  GetMiddelXPoint1D(node[1],node[5],m);
  xx[5]=m[0]; yy[5]=m[1]; zz[5]=m[2];

  GetMiddelXPoint1D(node[2],node[6],m);
  xx[6]=m[0]; yy[6]=m[1]; zz[6]=m[2];

  GetMiddelXPoint1D(node[3],node[7],m);
  xx[7]=m[0]; yy[7]=m[1]; zz[7]=m[2];

  GetMiddelXPoint1D(node[4],node[5],m);
  xx[8]=m[0]; yy[8]=m[1]; zz[8]=m[2];

  GetMiddelXPoint1D(node[5],node[6],m);
  xx[9]=m[0]; yy[9]=m[1]; zz[9]=m[2];

  GetMiddelXPoint1D(node[6],node[7],m);
  xx[10]=m[0]; yy[10]=m[1]; zz[10]=m[2];

  GetMiddelXPoint1D(node[4],node[7],m);
  xx[11]=m[0]; yy[11]=m[1]; zz[11]=m[2];


  /* Knoten gegebenfalls erzeugen */
  for (k=0; k<7; k++) {
      nnode=NULL;
      if(search && k!=0) nnode=GetXNodeNumberClose(x[k],y[k],z[k],&number,&dist);
      if( nnode != NULL && dist<MKleinsteZahl) {
          all_nodes[PNumber[k]]=/*new_node[k]=*/number;
      }
      else {
          nnode=CreateNodeGeometry();
          /*nnode=NewNode();*/
          nnode->x=x[k];
          nnode->y=y[k];
          nnode->z=z[k];
          all_nodes[PNumber[k]]=/*new_node[k]=*/AddNode(nnode);
      }
  }


  for (k=0; k<12; k++) {
      nnode=NULL;
      if(search) nnode=GetXNodeNumberClose(xx[k],yy[k],zz[k],&number,&dist);
      if( nnode != NULL && dist<MKleinsteZahl) {
          all_nodes[ENumber[k]]=/*enew_node[k]=*/number;
      }
      else {
          nnode=CreateNodeGeometry();
          /*nnode=NewNode();*/
          nnode->x=xx[k];
          nnode->y=yy[k];
          nnode->z=zz[k];
          all_nodes[ENumber[k]]=/*enew_node[k]=*/AddNode(nnode);
      }
  }


  /* Elemente */
  nele=2;
  for (j=1;j<=nele*nele*nele;j++) {
      if ((mode==0) && j == 1) {
          pos=i;
          ElDeleteElement (pos);
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
      }
      else {
          pos=ElListSize();
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
          anz_3D++;
		  msh_no_hexs++;
      }

      ElSetElementStartNumber(pos,start_number);


      /* Elementknoten */
      /*nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[--element_type]);*/
      nodes = (long *) Malloc(sizeof(long)*16);

      if(j<=2) jj=j;
      else if (j<=4) jj=j+1;
      else if (j<=6) jj=j+5;
      else if (j<=8) jj=j+6;

      nodes[0]=all_nodes[jj-1];
      nodes[1]=all_nodes[jj+2];
      nodes[2]=all_nodes[jj+3];
      nodes[3]=all_nodes[jj];
      nodes[4]=all_nodes[jj+8];
      nodes[5]=all_nodes[jj+11];
      nodes[6]=all_nodes[jj+12];
      nodes[7]=all_nodes[jj+9];

      /* Elementknoten eintragen */
      ElSetElementNodes(pos,nodes);
      ElSetElementGroupNumber(pos,group);

      if (level==apriori_level) ElSetElementActiveState(pos,1);
      else ElSetElementActiveState(pos,0);

      APrioriRefine3DElementLevelAllModeRekursiv(ar,pos,++level,search);
      --level;

  }

  return pos;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivNode
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivNode (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  double m[3];
  long nm;
  long knode=0,knext=0,kprev=0;
  long found=0;
  long group;


 if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=1) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<2; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<2; k++) {
                  if (old_node[k]==start_number) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=1; else kprev=k-1;
              if (k==1) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

      /* Mittleren Knoten erzeugen */

      GetMiddelXPoint1D(node[0],node[1],m);
          x[0]=m[0]; y[0]=m[1]; z[0]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[0],y[0],z[0],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[0];
          nnode->y=y[0];
          nnode->z=z[0];
          nm=AddNode(nnode);
                }


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[kprev];
            nodes[1]=nm;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_1D++;
			  msh_no_line++;
                }
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=old_node[knode];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine1DElementLevelAllModeRekursivNode(ar,pos,++level,search);
                --level;

  return pos;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivNode
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivNode (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  double m[3];
  long n1,n2,nm;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<4; k++) {
                  if (old_node[k]==start_number) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      /* Mittleren Knoten erzeugen */
      GetMiddelXPoint2D(node[0],node[1],node[2],node[3],m);
          x[0]=m[0]; y[0]=m[1]; z[0]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[0],y[0],z[0],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[0];
          nnode->y=y[0];
          nnode->z=z[0];
          nm=AddNode(nnode);
                }


      GetMiddelXPoint1D(node[knode],node[knext],m);
          x[1]=m[0]; y[1]=m[1]; z[1]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[1];
          nnode->y=y[1];
          nnode->z=z[1];
          n1=AddNode(nnode);
                }

          GetMiddelXPoint1D(node[kprev],node[knode],m);
          x[2]=m[0]; y[2]=m[1]; z[2]=m[2];
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[2];
          nnode->y=y[2];
          nnode->z=z[2];
          n2=AddNode(nnode);
                }

                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=n1;
            nodes[2]=old_node[knext];
            nodes[3]=old_node[kother];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=old_node[kother];
            nodes[2]=old_node[kprev];
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[knode];
            nodes[1]=n1;
            nodes[2]=nm;
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivNode(ar,pos,++level,search);
                --level;

  return pos;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivNodeM2
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivNodeM2 (APRIORI_REFINE *ar, long i, long numnode, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  double m[3];
  long n1,n2,nm,n11,n22;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      /* Mittleren Knoten erzeugen */
      GetMiddelXPoint2D(node[0],node[1],node[2],node[3],m);
          x[0]=m[0]; y[0]=m[1]; z[0]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[0],y[0],z[0],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[0];
          nnode->y=y[0];
          nnode->z=z[0];
          nm=AddNode(nnode);
                }


      GetMiddelXPoint1D(node[knode],node[knext],m);
          x[1]=m[0]; y[1]=m[1]; z[1]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[1];
          nnode->y=y[1];
          nnode->z=z[1];
          n1=AddNode(nnode);
                }

          GetMiddelXPoint1D(node[kprev],node[knode],m);
          x[2]=m[0]; y[2]=m[1]; z[2]=m[2];
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[2];
          nnode->y=y[2];
          nnode->z=z[2];
          n2=AddNode(nnode);
                }


      GetMiddelXPoint1D(node[knext],node[kother],m);
          x[3]=m[0]; y[3]=m[1]; z[3]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[3],y[3],z[3],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n22=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[3];
          nnode->y=y[3];
          nnode->z=z[3];
          n22=AddNode(nnode);
                }

          GetMiddelXPoint1D(node[kother],node[kprev],m);
          x[4]=m[0]; y[4]=m[1]; z[4]=m[2];
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[4],y[4],z[4],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n11=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[4];
          nnode->y=y[4];
          nnode->z=z[4];
          n11=AddNode(nnode);
                }


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=n1;
            nodes[2]=old_node[knext];
            nodes[3]=n22;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        APrioriRefine2DElementLevelAllModeRekursivEdge(ar,pos,old_node[knext], \
                                                                                                          n22,++level,search);
                --level;

                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=n11;
            nodes[2]=old_node[kprev];
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        APrioriRefine2DElementLevelAllModeRekursivEdge(ar,pos,n11,old_node[kprev], \
                                                                                                          ++level,search);
                --level;


            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[knode];
            nodes[1]=n1;
            nodes[2]=nm;
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm;
            nodes[1]=n22;
            nodes[2]=old_node[kother];
            nodes[3]=n11;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);




                APrioriRefine2DElementLevelAllModeRekursivNodeM2(ar,pos,nm,++level,search);
                --level;

  return pos;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivEdge
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivEdge (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                                                         long numnode2, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  double m[3];
  long nm1,nm2;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode1) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      if (old_node[knext] != numnode2) return 0;


      GetMiddelXPoint1D(node[knode],node[kprev],m);
          x[1]=m[0]; y[1]=m[1]; z[1]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[1];
          nnode->y=y[1];
          nnode->z=z[1];
          nm1=AddNode(nnode);
                }

          GetMiddelXPoint1D(node[knext],node[kother],m);
          x[2]=m[0]; y[2]=m[1]; z[2]=m[2];
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[2];
          nnode->y=y[2];
          nnode->z=z[2];
          nm2=AddNode(nnode);
                }



                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm1;
            nodes[1]=nm2;
            nodes[2]=old_node[kother];
            nodes[3]=old_node[kprev];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[knode];
            nodes[1]=old_node[knext];
            nodes[2]=nm2;
            nodes[3]=nm1;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivEdge(ar,pos,numnode1,numnode2,++level,search);
                --level;

  return pos;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivEdgeNew
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivEdgeNew (APRIORI_REFINE *ar, long i, long numnode1, \
                                                        long numnode2, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);
  long base_type=get_ar_base_type(ar);
  double base_value=get_ar_base_value(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  long nm1,nm2;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;
  double div=0.0;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode1) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

        if (old_node[knext] != numnode2) return 0;

        if (base_type == 0) {
          div=base_value;   
        }
        else if (base_type == 1) {
          div=pow(base_value,(double)level);   
        }
        else if (base_type >=2) {
          div=pow(base_value,(double)base_type);   
        }
        else return -1;
        
        if (fabs(div)<MKleinsteZahl) return -1;

        x[1]=node[knode]->x+(node[kprev]->x-node[knode]->x)/div;
        y[1]=node[knode]->y+(node[kprev]->y-node[knode]->y)/div;
        z[1]=node[knode]->z+(node[kprev]->z-node[knode]->z)/div;

        nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[1];
          nnode->y=y[1];
          nnode->z=z[1];
          nm1=AddNode(nnode);
                }

        x[2]=node[knext]->x+(node[kother]->x-node[knext]->x)/div;
        y[2]=node[knext]->y+(node[kother]->y-node[knext]->y)/div;
        z[2]=node[knext]->z+(node[kother]->z-node[knext]->z)/div;

        nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[2];
          nnode->y=y[2];
          nnode->z=z[2];
          nm2=AddNode(nnode);
                }



                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm1;
            nodes[1]=nm2;
            nodes[2]=old_node[kother];
            nodes[3]=old_node[kprev];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[knode];
            nodes[1]=old_node[knext];
            nodes[2]=nm2;
            nodes[3]=nm1;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivEdgeNew(ar,pos,numnode1,numnode2,++level,search);
                --level;

  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                                                         long numnode2, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  double m[3];
  long nm1,nm2;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);


          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode1) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      if (old_node[knext] != numnode2) return 0;


      GetMiddelXPoint1D(node[knode],node[knext],m);
          x[1]=m[0]; y[1]=m[1]; z[1]=m[2];

            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[1];
          nnode->y=y[1];
          nnode->z=z[1];
          nm1=AddNode(nnode);
                }

          GetMiddelXPoint1D(node[kprev],node[kother],m);
          x[2]=m[0]; y[2]=m[1]; z[2]=m[2];
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          nm2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[2];
          nnode->y=y[2];
          nnode->z=z[2];
          nm2=AddNode(nnode);
                }



                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node[knode];
            nodes[1]=nm1;
            nodes[2]=nm2;
            nodes[3]=old_node[kprev];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

                APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop(ar,pos,numnode1,nm1,++level,search);
                --level;

            if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=nm1;
            nodes[1]=old_node[knext];
            nodes[2]=old_node[kother];
            nodes[3]=nm2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop(ar,pos,nm1,numnode2,++level,search);
                --level;

  return pos;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotropI
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotropI (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                        long numnode2, long ni, int search)
{
  /* Variablen */
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;

  long nm1,nm2;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long group;
  long j,nnod,nele;

  if (ni<2) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
      pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
      for (k=0; k<4; k++) {
        old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
      }
      pred=ElGetElementPred(i);


      for (k=0; k<4; k++) {
        if (old_node[k]==numnode1) {
          found=1;
          knode=k;
          if (k==0) kprev=3; else kprev=k-1;
          if (k==3) knext=0; else knext=k+1;
        }
      }

      if (!found) return -1;

      for (k=0; k<4; k++)
        if(k!=knode && k!=kprev && k!=knext) kother=k;

      if (old_node[knext] != numnode2) return 0;

      /* Knoten Erzeugen */
      for (j=1;j<=ni;j++) {
        if (j!=ni){
          nnod=ni-1;
          nele=nnod+1;
          x[1]=node[knode]->x+j*(node[knext]->x-node[knode]->x)/nele;
          y[1]=node[knode]->y+j*(node[knext]->y-node[knode]->y)/nele;
          z[1]=node[knode]->z+j*(node[knext]->z-node[knode]->z)/nele;
          nnode=NULL;
          if(search) nnode=GetXNodeNumberClose(x[1],y[1],z[1],&number,&dist);
          if( nnode != NULL && dist<MKleinsteZahl) {
            nm1=number;
          }
          else {
            nnode=CreateNodeGeometry();
            /*nnode=NewNode();*/
            nnode->x=x[1];
            nnode->y=y[1];
            nnode->z=z[1];
            nm1=AddNode(nnode);
          }
          x[2]=node[kprev]->x+j*(node[kother]->x-node[kprev]->x)/nele;
          y[2]=node[kprev]->y+j*(node[kother]->y-node[kprev]->y)/nele;
          z[2]=node[kprev]->z+j*(node[kother]->z-node[kprev]->z)/nele;
          nnode=NULL;
          if(search) nnode=GetXNodeNumberClose(x[2],y[2],z[2],&number,&dist);
          if( nnode != NULL && dist<MKleinsteZahl) {
            nm2=number;
          }
          else {
            nnode=CreateNodeGeometry();
            /*nnode=NewNode();*/
            nnode->x=x[2];
            nnode->y=y[2];
            nnode->z=z[2];
            nm2=AddNode(nnode);
          }
        }
        else {
          nm1=old_node[knext];
          nm2=old_node[kother];
        }

        if (mode==0 && j==1) {
          pos=i;
          ElDeleteElement (pos);
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
        }
        else {
          pos=ElListSize();
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
          anz_2D++;
		  msh_no_quad++;
        }
        /*ElSetElementStartNumber(pos,start_number);*/

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
        nodes[0]=old_node[knode];
        nodes[1]=nm1;
        nodes[2]=nm2;
        nodes[3]=old_node[kprev];
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        old_node[knode]=nm1;
        old_node[kprev]=nm2;
      }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivAnisotrop
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version !!! noch nicht fertig
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotrop (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;
  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j;
  long number=-1;
  double x,y,z,dist=0.0;
  long found=0;
  long group;


  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=1) return 0;
      pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
      n1=nodes[0];
      n2=nodes[1];
      if ( n1==numnode1 && n2==numnode2 ) {
        found=1;
        n1=numnode1;
        n2=numnode2;
      }
      else if ( n1==numnode2 && n2==numnode1 ) {
        found=1;
        n1=numnode2;
        n2=numnode1;
      }

      if (!found) return -1;

      node1 = GetNode(n1);
      node2 = GetNode(n2);
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
      nnod=1;
      nele=nnod+1;
      x=(node1->x+node2->x)/2.0;
      y=(node1->y+node2->y)/2.0;
      z=(node1->z+node2->z)/2.0;

      old_node=n1;
      for (j=1;j<=nele;j++) {
        if ((j%2)!=0) {
          node=NULL;
          if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
          if( node != NULL && dist<MKleinsteZahl) {
            new_node=number;
          }
          else {
            node=CreateNodeGeometry();
            /*node=NewNode();*/
            node->x=x;
            node->y=y;
            node->z=z;
            new_node=AddNode(node);
          }
        }
        else {
          new_node=n2;
        }

        /* Elemente Erzeugen */
        if ((mode==0) && j == 1) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_1D++;
			msh_no_line++;
          }

      ElSetElementStartNumber(pos,start_number);

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
            nodes[0]=old_node;
            nodes[1]=new_node;
        old_node=new_node;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

                APrioriRefine1DElementLevelAllModeRekursiv(ar,pos,++level,search);
                --level;
          }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelAllModeRekursivAnisotropI
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version !!! noch nicht fertig
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotropI (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long ni, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;
  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j;
  long number=-1;
  double x,y,z,dist=0.0;
  long found=0;
  long group;


  if (ni<2) return 0;

      if((element_type=ElGetElementType(i))!=1) return 0;
      pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
      n1=nodes[0];
      n2=nodes[1];
      if ( n1==numnode1 && n2==numnode2 ) {
        found=1;
        n1=numnode1;
        n2=numnode2;
      }
      else if ( n1==numnode2 && n2==numnode1 ) {
        found=1;
        n1=numnode2;
        n2=numnode1;
      }

      if (!found) return -1;

      node1 = GetNode(n1);
      node2 = GetNode(n2);
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
      old_node=n1;
      for (j=1;j<=ni;j++) {
        if (j!=ni){
          nnod=ni-1;
          nele=nnod+1;
          x=node1->x+j*(node2->x-node1->x)/nele;
          y=node1->y+j*(node2->y-node1->y)/nele;
          z=node1->z+j*(node2->z-node1->z)/nele;
          node=NULL;
          if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
          if( node != NULL && dist<MKleinsteZahl) {
            new_node=number;
          }
          else {
            node=CreateNodeGeometry();
            /*node=NewNode();*/
            node->x=x;
            node->y=y;
            node->z=z;
            new_node=AddNode(node);
          }
        }
        else {
          new_node=n2;
        }


        /* Elemente Erzeugen */
        if ((mode==0) && j == 1) {
          pos=i;
          ElDeleteElement (pos);
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
        }
        else {
          pos=ElListSize();
          CreateElementTopology(element_type,pred,0,pos);
          /*ElPlaceNewElement(element_type,pred,0,pos);*/
          anz_1D++;
		  msh_no_line++;
        }

        ElSetElementStartNumber(pos,start_number);

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
        nodes[0]=old_node;
        nodes[1]=new_node;
        old_node=new_node;

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        ElSetElementActiveState(pos,1);
      }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivT2
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivT2 (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4],new_node[5];

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j,k,kk,jj;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;

          x[0]=y[0]=z[0]=0.0;
          for (k=0; k<4; k++) {
                if (k==3) kk=0;
                else kk=k+1;
        x[k+1]=((node[kk])->x+(node[k])->x)/2.0;
        y[k+1]=((node[kk])->y+(node[k])->y)/2.0;
        z[k+1]=((node[kk])->z+(node[k])->z)/2.0;
                x[0]=x[0]+x[k+1];
                y[0]=y[0]+y[k+1];
                z[0]=z[0]+z[k+1];
      }
          x[0]=x[0]/4.0;
          y[0]=y[0]/4.0;
          z[0]=z[0]/4.0;

          /* Knoten gegebenfalls erzeugen */
      for (j=0;j<=nele*nele;j++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[j],y[j],z[j],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[j]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[j];
          nnode->y=y[j];
          nnode->z=z[j];
          new_node[j]=AddNode(nnode);
                }
          }

      /* Elemente */
      for (j=1;j<=nele*nele;j++) {
          if ((mode==0) && j == 1) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
          }

        ElSetElementStartNumber(pos,start_number);
        ElSetElementGroupNumber(pos,group);


        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==1) jj=4;
            else jj=j-1;
            nodes[0]=old_node[j-1];
            nodes[1]=new_node[j];
            nodes[2]=new_node[0];
            nodes[3]=new_node[jj];

        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

          APrioriRefine2DElementLevelAllModeRekursivNodeM2(ar,pos,new_node[0],++level,search);
          --level;

          }


  return pos;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivT5
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivT5 (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
/*  long start_number=get_ar_start_element(ar);*/
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);
  long element_start_number=ElGetElementStartNumber(i);
  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4],new_node[5];

  long nnod,nele;
  long pred;
  long element_type;
  long pos;
  long j,k,kk,jj,jjj;
  long number=-1;
  double x[5],y[5],z[5],dist=0.0;
  double m[3];
  long group;

  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      ElSetElementActiveState(pos,0);
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

      /* Knoten Erzeugen */
          nnod=1;
          nele=nnod+1;

      /* Mittelknoten berechnen */
      GetMiddelXPoint2D(node[0],node[1],node[2],node[3],m);
          x[0]=m[0]; y[0]=m[1]; z[0]=m[2];

          for (k=0; k<4; k++) {
                if (k==3) kk=0;
                else kk=k+1;
        x[k+1]=((node[k])->x+x[0])/2.0;
        y[k+1]=((node[k])->y+y[0])/2.0;
        z[k+1]=((node[k])->z+z[0])/2.0;
      }

          /* Knoten erzeugen */
      for (j=1;j<=nele*nele;j++) {
            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(x[j],y[j],z[j],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          new_node[j]=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=x[j];
          nnode->y=y[j];
          nnode->z=z[j];
          new_node[j]=AddNode(nnode);
                }
          }

      /* Mittleren Element erzeugen */
          if (mode == 0) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
          }

      ElSetElementStartNumber(pos,element_start_number);

      /* Elementknoten */
      nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
          nodes[0]=new_node[1];
          nodes[1]=new_node[2];
          nodes[2]=new_node[3];
          nodes[3]=new_node[4];

      /* Elementknoten eintragen */
      ElSetElementNodes(pos,nodes);
      ElSetElementActiveState(pos,1);
      ElSetElementGroupNumber(pos,group);


      /* Elemente */
      for (j=1;j<=nele*nele;j++) {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        ElSetElementStartNumber(pos,element_start_number);

        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
                if (j==4) jj=0;
            else jj=j;
                if (j==4) jjj=1;
            else jjj=j+1;
            nodes[0]=old_node[j-1];
            nodes[1]=old_node[jj];
            nodes[2]=new_node[jjj];
            nodes[3]=new_node[j];

        /* Elementknoten und -gruppe eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);

        if (level==apriori_level) ElSetElementActiveState(pos,1);
        else ElSetElementActiveState(pos,0);

        APrioriRefine2DElementLevelAllModeRekursivEdge(ar,pos,old_node[j-1],old_node[jj], \
                                                                                                          ++level,search);
                --level;

          }


  return pos;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriDeleteAllElementNode
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void APrioriDeleteAllElementNode (APRIORI_REFINE *ar)
{
/* Noch nicht fertig */
  ar = ar;
  return;
#ifdef dkfjggdshgsdfglshgfh

  long start_number=get_ar_start_element(ar);
  long i,list_size,element_type;
  int found=0;
  long k;
  long *nodes;

  list_size=ElListSize();
  for (i=0;i<list_size;i++) {
      if((element_type=ElGetElementType(i))!=2) return; /*  Zuerst nur fuer 2D */

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                  if (nodes[k]==start_number) {
                          found=1;
                  }
          }

          if (!found) continue;
          else ElDeleteElement(i);
  }

  if (found) DeleteNode(start_number);

 return;
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine1DElementLevelMode0
                                                                          */
/* Aufgabe:
   Verfeinert ein 1D Element a priori bis zum Level level.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine1DElementLevelMode0 (APRIORI_REFINE *ar, long i, int search)
{
  /* Variablen */
  long start_number=get_ar_start_element(ar);
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);

  Knoten *node,*node1,*node2;

  long *nodes;
  long n1,n2;
  long old_node,new_node;

  long nnod,nele;
  long pred;
  double dx,dy,dz;
  long element_type;
  long pos;

  long j;
  long node_counter,element_counter;
  double x,y,z;
  long number=-1;
  double dist=0.0;
  long group;

  if(apriori_level<=0) return 0;

  node_counter=0;
  element_counter=0;

    if((element_type=ElGetElementType(i))!=1) return 0;

    pos=i;
    group=ElGetElementGroupNumber(pos);

    nodes = ElGetElementNodes(i);
    n1=nodes[0];
        n2=nodes[1];
    node1 = GetNode(n1);
    node2 = GetNode(n2);
    pred=ElGetElementPred(i);

    /* Knoten Erzeugen */
        nnod=(long)(pow(2.,(int)apriori_level)-1);
        nele=nnod+1;
        dx=(node2->x-node1->x)/(double)(nele);
        dy=(node2->y-node1->y)/(double)(nele);
        dz=(node2->z-node1->z)/(double)(nele);
        old_node=n1;

    for (j=1;j<=nele;j++) {
      x=node1->x+j*dx;
      y=node1->y+j*dy;
      z=node1->z+j*dz;

          node=NULL;
      if(search) node=GetXNodeNumberClose(x,y,z,&number,&dist);
      if( node != NULL && dist<MKleinsteZahl) {
        new_node=number;
          }
          else {
                node=CreateNodeGeometry();
                /*node=NewNode();*/
        node->x=x;
        node->y=y;
        node->z=dz;
        new_node=AddNode(node);
            node_counter++;
          }

          /* Elemente Erzeugen */
          if ((mode==0) && j == 1) {
        pos=i;
        ElDeleteElement (pos);
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
      else {
                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
          }
          anz_1D++;
		  msh_no_line++;
          element_counter++;

      ElSetElementStartNumber(pos,start_number);

      /* Elementknoten */
      nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type]);
          nodes[0]=old_node;
          nodes[1]=new_node;
      old_node=new_node;

      /* Elementknoten eintragen */
      ElSetElementNodes(pos,nodes);
      ElSetElementGroupNumber(pos,group);
          /*ElSetElementAMGLevel(pos,0);*/
    }

  return 1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivT3
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivT3 (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);
  double val=get_ar_value(ar,1);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double dist=0.0;

  double m[3],t1[3],t2[3];
  long n1,n2,n3,n4;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long numnode;
  long group;


  if (level>apriori_level) return 0;

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

      numnode=old_node[0];

          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      /* Knoten erzeugen */

      TangUnitVectorXPoint(node[knode],node[knext],t1);
      TangUnitVectorXPoint(node[knode],node[kprev],t2);
      GetXPoint(node[knode],t1,val,t2,val,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n1=AddNode(nnode);
                }


      TangUnitVectorXPoint(node[knext],node[kother],t1);
      TangUnitVectorXPoint(node[knext],node[knode],t2);
      GetXPoint(node[knext],t1,val,t2,val,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n2=AddNode(nnode);
                }


      TangUnitVectorXPoint(node[kother],node[kprev],t1);
      TangUnitVectorXPoint(node[kother],node[knext],t2);
      GetXPoint(node[kother],t1,val,t2,val,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n3=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n3=AddNode(nnode);
                }



      TangUnitVectorXPoint(node[kprev],node[knode],t1);
      TangUnitVectorXPoint(node[kprev],node[kother],t2);
      GetXPoint(node[kprev],t1,val,t2,val,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n4=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n4=AddNode(nnode);
                }



                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[0];
            nodes[1]=old_node[1];
            nodes[2]=n2;
            nodes[3]=n1;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[1];
            nodes[1]=old_node[2];
            nodes[2]=n3;
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[2];
            nodes[1]=old_node[3];
            nodes[2]=n4;
            nodes[3]=n3;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[3];
            nodes[1]=old_node[0];
            nodes[2]=n1;
            nodes[3]=n4;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=n1;
            nodes[1]=n2;
            nodes[2]=n3;
            nodes[3]=n4;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivT3(ar,pos,++level,search);
                --level;


  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: APrioriRefine2DElementLevelAllModeRekursivT4
                                                                          */
/* Aufgabe:
   Verfeinert ein 2D Element a priori bis zum Level level.
   Rekursiver Algorithmus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long level: Vorverfeinerungslevel fueralle Elemente.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long APrioriRefine2DElementLevelAllModeRekursivT4 (APRIORI_REFINE *ar, long i, long level, int search)
{
  /* Variablen */
  long apriori_level=get_ar_level(ar);
  long mode=get_ar_mode(ar);
  double val=get_ar_value(ar,1);

  Knoten *node[4],*nnode;
  long *nodes;
  long old_node[4];

  long pred;
  long element_type;
  long pos;
  long k;
  long number=-1;
  double dist=0.0;

  double m[3],t1[3],t2[3];
  long n1,n2,n3,n4;
  long knode=0,knext=0,kprev=0,kother=0;
  long found=0;
  long numnode;
  double val1;
  long group;

  if (level>apriori_level) return 0;

  val1=val*pow(2.,(double)(level-1));

      if((element_type=ElGetElementType(i))!=2) return 0;
          pos=i;
      /*ElSetElementActiveState(pos,0);*/
      group=ElGetElementGroupNumber(pos);

      nodes = ElGetElementNodes(i);
          for (k=0; k<4; k++) {
                old_node[k]=nodes[k];
        node[k]= GetNode(old_node[k]);
          }
      pred=ElGetElementPred(i);

      numnode=old_node[0];

          for (k=0; k<4; k++) {
                  if (old_node[k]==numnode) {
                          found=1;
                          knode=k;
                          if (k==0) kprev=3; else kprev=k-1;
              if (k==3) knext=0; else knext=k+1;
                  }
          }

          if (!found) return -1;

          for (k=0; k<4; k++)
                if(k!=knode && k!=kprev && k!=knext) kother=k;

      /* Knoten erzeugen */

      TangUnitVectorXPoint(node[knode],node[knext],t1);
      TangUnitVectorXPoint(node[knode],node[kprev],t2);
      GetXPoint(node[knode],t1,val1,t2,val1,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n1=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n1=AddNode(nnode);
                }


      TangUnitVectorXPoint(node[knext],node[kother],t1);
      TangUnitVectorXPoint(node[knext],node[knode],t2);
      GetXPoint(node[knext],t1,val1,t2,val1,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n2=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n2=AddNode(nnode);
                }


      TangUnitVectorXPoint(node[kother],node[kprev],t1);
      TangUnitVectorXPoint(node[kother],node[knext],t2);
      GetXPoint(node[kother],t1,val1,t2,val1,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n3=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n3=AddNode(nnode);
                }



      TangUnitVectorXPoint(node[kprev],node[knode],t1);
      TangUnitVectorXPoint(node[kprev],node[kother],t2);
      GetXPoint(node[kprev],t1,val1,t2,val1,m);


            nnode=NULL;
        if(search) nnode=GetXNodeNumberClose(m[0],m[1],m[2],&number,&dist);
        if( nnode != NULL && dist<MKleinsteZahl) {
          n4=number;
                }
            else {
              nnode=CreateNodeGeometry();
              /*nnode=NewNode();*/
          nnode->x=m[0];
          nnode->y=m[1];
          nnode->z=m[2];
          n4=AddNode(nnode);
                }



                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[0];
            nodes[1]=old_node[1];
            nodes[2]=n2;
            nodes[3]=n1;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[1];
            nodes[1]=old_node[2];
            nodes[2]=n3;
            nodes[3]=n2;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[2];
            nodes[1]=old_node[3];
            nodes[2]=n4;
            nodes[3]=n3;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                pos=ElListSize();
                CreateElementTopology(element_type,pred,0,pos);
                /*ElPlaceNewElement(element_type,pred,0,pos);*/
            anz_2D++;
			msh_no_quad++;
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=old_node[3];
            nodes[1]=old_node[0];
            nodes[2]=n1;
            nodes[3]=n4;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                if (mode==0) {
          pos=i;
          ElDeleteElement (pos);
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
                }
        else {
                  pos=ElListSize();
                  CreateElementTopology(element_type,pred,0,pos);
                  /*ElPlaceNewElement(element_type,pred,0,pos);*/
              anz_2D++;
			  msh_no_quad++;
                }
        /*ElSetElementStartNumber(pos,start_number);*/
        /* Elementknoten */
        nodes = (long *) Malloc(sizeof(long)*ElNumberOfNodes[element_type-1]);
            nodes[0]=n1;
            nodes[1]=n2;
            nodes[2]=n3;
            nodes[3]=n4;
        /* Elementknoten eintragen */
        ElSetElementNodes(pos,nodes);
        ElSetElementGroupNumber(pos,group);


                APrioriRefine2DElementLevelAllModeRekursivT4(ar,pos,++level,search);
                --level;


  return pos;
}




/* Tools */


/**************************************************************************/
/* ROCKFLOW - Funktion: TestIfElementTriangle
                                                                          */
/* Aufgabe:
   Testet ein Viereckelement ueber seine Topologie, ob es um ein
   degeneriertes Element handelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   07/2000    AH      Erste Version
                                                                          */
/**************************************************************************/
int TestIfElementTriangle (long index)
{
 static long *nodes;

 if (ElGetElementType(index)!=2) return 0;

 nodes = ElGetElementNodes(index);
 if ( (nodes[0]==nodes[1]) && (nodes[2]!=nodes[3])) return 1;
 if ( (nodes[1]==nodes[2]) && (nodes[3]!=nodes[0])) return 2;
 if ( (nodes[2]==nodes[3]) && (nodes[0]!=nodes[1])) return 3;
 if ( (nodes[3]==nodes[0]) && (nodes[1]!=nodes[2])) return 4;

 return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetXNodeNumberClose
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
Knoten *GetXNodeNumberClose (double xcoor, double ycoor, double zcoor, \
                                                         long *number, double *distance)
{
 long i;
 double dist,distmin;
 double x[3];
 double y[3];
 Knoten *node=NULL,*node_tmp;

 x[0]=xcoor; x[1]=ycoor, x[2]=zcoor;
 *number=-1;
 distmin=1.e+300;
 for (i=0;i<NodeListLength;i++) {
   if ( (node_tmp=GetNode(i)) != NULL) {
         y[0]=GetNodeX(i);
         y[1]=GetNodeY(i);
         y[2]=GetNodeZ(i);
   }
   else continue;

   dist=EuklVek3dDist(x,y);
   if (dist < distmin) {
     distmin=dist;
         *number=i;
         node=node_tmp;
   }
 }
 *distance=distmin;

 return node;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelPoint
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelPoint (double *x, double *y, double *z, double *m, long np)
{
 long i;

 if (np <= 0) return 0;

 m[0]=x[0];
 m[1]=y[0];
 m[2]=z[0];
 for (i=1;i<np;i++) {
   m[0]=m[0]+x[i];
   m[1]=m[1]+y[i];
   m[2]=m[2]+z[i];
 }
 m[0]=m[0]/(double)np;
 m[1]=m[1]/(double)np;
 m[2]=m[2]/(double)np;

 return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelXPoint
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelXPoint (Knoten *node[], double *m, long np)
{
 long i;

 if (np <= 0) return 0;

 m[0]=(node[0])->x;
 m[1]=(node[0])->y;
 m[2]=(node[0])->z;
 for (i=1;i<np;i++) {
   m[0]=m[0]+(node[i])->x;
   m[1]=m[1]+(node[i])->y;
   m[2]=m[2]+(node[i])->z;
 }
 m[0]=m[0]/(double)np;
 m[1]=m[1]/(double)np;
 m[2]=m[2]/(double)np;

 return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelPoint1D
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelPoint1D (double *p1, double *p2, double *m, long dim)
{
 long i;

 if (dim <=0 ) return 0;

 for (i=0;i<dim;i++)
   m[i]=(p1[i]+p2[i])/2.;

 return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelXPoint1D
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelXPoint1D (Knoten *node1, Knoten *node2, double *m)
{
  m[0]=(node1->x+node2->x)/2.0;
  m[1]=(node1->y+node2->y)/2.0;
  m[2]=(node1->z+node2->z)/2.0;

 return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelPoint2D
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelPoint2D (double *p1, double *p2, double *p3, double *p4, \
                                          double *m, long dim)
{
 long i;

 if (dim <=0 ) return 0;

 for (i=0;i<dim;i++)
   m[i]=(p1[i]+p2[i]+p3[i]+p4[i])/4.;

 return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelXPoint2D
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelXPoint2D (Knoten *node1, Knoten *node2, Knoten *node3, Knoten *node4, \
                                          double *m)
{
  m[0]=(node1->x+node2->x+node3->x+node4->x)/4.0;
  m[1]=(node1->y+node2->y+node3->y+node4->y)/4.0;
  m[2]=(node1->z+node2->z+node3->z+node4->z)/4.0;

 return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMiddelPoint3D
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMiddelPoint3D (double *p1, double *p2, double *p3, double *p4, \
                                          double *p5, double *p6, double *p7, double *p8, \
                                          double *pm, long dim)
{
 long i;

 if (dim <=0 ) return 0;

 for (i=0;i<dim;i++)
   pm[i]=(p1[i]+p2[i]+p3[i]+p4[i]+p5[i]+p6[i]+p7[i]+p8[i])/8.;

 return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: TangUnitVectorXPoint
                                                                          */
/* Aufgabe:
   Ermittelt den tangentiellen Einheitsvektor, der durch die Punkte
   P1 und P2 geht..
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 1 im Erfolgsfall, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int TangUnitVectorXPoint(Knoten *node1, Knoten *node2, double *t)
{
  double dist;

  t[0]=node2->x-node1->x;
  t[1]=node2->y-node1->y;
  t[2]=node2->z-node1->z;

  dist=MBtrgVec(t,3);
  if (dist<= 0.0) return 0;

  t[0]=t[0]/dist;
  t[1]=t[1]/dist;
  t[2]=t[2]/dist;

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetXPoint
                                                                          */
/* Aufgabe:
   Ermittelt den tangentiellen Einheitsvektor, der durch die Punkte
   P1 und P2 geht..
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 1 im Erfolgsfall, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetXPoint(Knoten *node1, double *t1, double dist1, \
                          double *t2, double dist2, double *p)
{

  p[0]=node1->x+dist1*t1[0]+dist2*t2[0];
  p[1]=node1->y+dist1*t1[1]+dist2*t2[1];
  p[2]=node1->z+dist1*t1[2]+dist2*t2[2];

  return 1;
}




/* Lesefunktion */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAprioriRefineElement
                                      */
/* Aufgabe:
   Liest die zu dem Schluesselwort APRIORI_REFINE_ELEMENT gehoerigen
   Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   APRIORI_REFINE_ELEMENT: Elementvorverfeinerer
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   08/1998    AH      Erste Version
   07/2000    OK      Protokoll fuer found==2
   10/2000    AH      Anisotropes Vorverfeinern Methoden 17 und 18 
                      Methode 6 geaendert (Lesen ueber Koordinaten)
                                      */
/**************************************************************************/
int FctAprioriRefineElement ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ar_name[]="APRIORI_REFINE_ELEMENT";
    APRIORI_REFINE *ar=NULL;
    APRIORI_REFINE *ar_prior=NULL;

/* AR's-Name in der Namensliste einfuegen */
    declare_apriori_refine_name(ar_name);

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort APRIORI_REFINE_ELEMENT");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.?.?  Elementvorverfeinerer ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f,"*** C?.?.?  Elementvorverfeinerer ***");
        LineFeed(f);
        FilePrintString(f,"; Schluesselwort: #APRIORI_REFINE_ELEMENT (Anfangsbedingungen)");
        LineFeed(f);
#endif
    }
    else {   /* Schluesselwort gefunden */
        if(found==1) {
            FilePrintString(f,"#APRIORI_REFINE_ELEMENT");
            LineFeed(f);
        }
        else if(found==2) {
            if(!RefineElementListEmpty()) {
                FilePrintString(f,"#APRIORI_REFINE_ELEMENT");
                LineFeed(f);
            }
            list_ar_init();
        }

#ifdef EXT_RFD
        FilePrintString(f,"; Schluesselwort: #APRIORI_REFINE_ELEMENT (Anfangsbedingungen)"); LineFeed(f);
        FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann ohne"); LineFeed(f);
        FilePrintString(f,"; Elementvorverfeinerer gearbeitet."); LineFeed(f);
#endif

        while (StrTestLong(&data[p+=pos])||(found==2)) {

            if(found==1) {
                ar=create_apriori_refine(ar_name);
                if(ar==NULL) break;
            }
            else if (found==2) {
                ar=(APRIORI_REFINE *)get_list_ar_next();
                if(ar==ar_prior) return ok;
                ar_prior = ar;
            }

            if(ar==NULL) {
                break;
            }
            else {
                if(found==1) {
                    ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
                    set_ar_type(ar,l);
                }
                else if (found==2) {
                    fprintf(f," %ld ",get_ar_type(ar));
                }

                if (get_ar_type(ar)==0) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                        fprintf(f," %ld ",get_ar_level(ar));
                    }
                }
                else if (get_ar_type(ar)==1) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==2) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==3) {
                    if(found==1) {
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==4) {
                    if(found==1) {
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==5) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                        }
                else if (get_ar_type(ar)==6) {
                    if(found==1) {
                        init_apriori_refine_coor(ar,2);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,2,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        /*ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_base_type(ar,l);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_base_value(ar,d);*/
                    }
                    else if (found==2) {
                    }
                }
                /*else if (get_ar_type(ar)==6) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_end_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_step_elements(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }*/
                else if (get_ar_type(ar)==7) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==8) {
                    if(found==1) {
                        init_apriori_refine(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_value(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==9) {
                    if(found==1) {
                        init_apriori_refine(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_value(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==10) {
                    if(found==1) {
                        init_apriori_refine(ar,1);
                        init_apriori_refine_coor(ar,2);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,2,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==11) {
                    if(found==1) {
                        init_apriori_refine(ar,1);
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_radius(ar,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_distribution_type(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==12) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==13) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==14) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==15) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==16) {
                    if(found==1) {
                        init_apriori_refine_coor(ar,2);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,2,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==17) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_end_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }
                else if (get_ar_type(ar)==18) {
                    if(found==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_end_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                    }
                    else if (found==2) {
                    }
                }

                if (ar) insert_apriori_refine_list (ar);
            } /* else */
            LineFeed(f);
        } /* while */
#ifdef EXT_RFD
        FilePrintString(f,"; Es folgen beliebig viele Bloecke mit mindestens 2 Werten."); LineFeed(f);
        FilePrintString(f,"; - Vorverfeinerer-Typ (type) [>=0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 0: Gleichmaessig Vorverfeinern"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 1: Ungleichmaessig Vorverfeinern"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 2: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,"; - Knotenknummer (start_element) [>=0;-1]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 3: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Quelle/Senke) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 4: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,";            Der Knoten wird geloescht (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Quelle/Senke) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 5: Vorverfeinern um allen Knoten"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Knoten) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 6: Vorverfeinern entlang einer Kante"); LineFeed(f);
        FilePrintString(f,";            (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Elementnummer (2D) (start_element) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotennummer 1 [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotennummer 2 [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 7: Vorverfeinern einzelnes Elements"); LineFeed(f);
        FilePrintString(f,";            (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Elementnummer  (start_element) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 8: Matrixdiffusion (Fall 1)"); LineFeed(f);
        FilePrintString(f,"; - Anfangstiefe (value) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 9: Matrixdiffusion (Fall 2)"); LineFeed(f);
        FilePrintString(f,"; - Anfangstiefe (value) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 10: Vorverfeinern um allen Knoten, die sich auf einer Linie befinden"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x1 (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y1 (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z1 (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x2 (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y2 (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z2 (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente im Gebiet) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 11: Vorverfeinern um alle Knoten in einem Gebiet"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Radius (radius) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Gebietstyp  (distribution_type) [0<=...<=2;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente im Gebiet) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
#endif
    }
  return ok;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: FctAprioriRefineElement
                                      */
/* Aufgabe:
   Liest die zu dem Schluesselwort APRIORI_REFINE_ELEMENT gehoerigen
   Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   APRIORI_REFINE_ELEMENT: Elementvorverfeinerer
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   08/1998    AH         Erste Version
                                      */
/**************************************************************************/
int FctAprioriRefineElementV0 ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
        char ar_name[]="APRIORI_REFINE_ELEMENT";
    APRIORI_REFINE *ar;

/* AR's-Name in der Namensliste einfuegen */
    declare_apriori_refine_name(ar_name);

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort APRIORI_REFINE_ELEMENT");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.?.?  Elementvorverfeinerer ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f,"*** C?.?.?  Elementvorverfeinerer ***");
        LineFeed(f);
        FilePrintString(f,"; Schluesselwort: #APRIORI_REFINE_ELEMENT (Anfangsbedingungen)");
        LineFeed(f);
#endif
    }
    else {   /* Schluesselwort gefunden */
        FilePrintString(f,"#APRIORI_REFINE_ELEMENT");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f,"; Schluesselwort: #APRIORI_REFINE_ELEMENT (Anfangsbedingungen)"); LineFeed(f);
        FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann ohne"); LineFeed(f);
        FilePrintString(f,"; Elementvorverfeinerer gearbeitet."); LineFeed(f);
#endif
        while (StrTestLong(&data[p+=pos])) {

            if ( (ar=create_apriori_refine(ar_name)) == NULL ) {
                                ok = 0;
                                break;
                        }
                        else {
                          ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
                          set_ar_type(ar,l);
                        }

                        if (get_ar_type(ar)==0) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==1) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==2) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==3) {
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==4) {
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==5) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==6) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_end_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_step_elements(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==7) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==8) {
                        init_apriori_refine(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_value(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==9) {
                        init_apriori_refine(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_value(ar,1,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==10) {
                        init_apriori_refine(ar,1);
                        init_apriori_refine_coor(ar,2);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,2,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==11) {
                        init_apriori_refine(ar,1);
                        init_apriori_refine_coor(ar,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_radius(ar,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_distribution_type(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==12) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==13) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==14) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==15) {
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_start_element(ar,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }
                        else if (get_ar_type(ar)==16) {
                        init_apriori_refine_coor(ar,2);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,1,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_x(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_y(ar,2,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_ar_z(ar,2,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_ar_level(ar,l);
                        }

                        if (ar) insert_apriori_refine_list (ar);
            LineFeed(f);
        }
#ifdef EXT_RFD
        FilePrintString(f,"; Es folgen beliebig viele Bloecke mit mindestens 2 Werten."); LineFeed(f);
        FilePrintString(f,"; - Vorverfeinerer-Typ (type) [>=0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 0: Gleichmaessig Vorverfeinern"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 1: Ungleichmaessig Vorverfeinern"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 2: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,"; - Knotenknummer (start_element) [>=0;-1]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 3: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Quelle/Senke) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 4: Vorverfeinern um eine Quelle/Senke"); LineFeed(f);
        FilePrintString(f,";            Der Knoten wird geloescht (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Quelle/Senke) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 5: Vorverfeinern um allen Knoten"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente um die Knoten) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 6: Vorverfeinern entlang einer Kante"); LineFeed(f);
        FilePrintString(f,";            (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Elementnummer (2D) (start_element) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotennummer 1 [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotennummer 2 [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 7: Vorverfeinern einzelnes Elements"); LineFeed(f);
        FilePrintString(f,";            (noch nicht benutzen!!)"); LineFeed(f);
        FilePrintString(f,"; - Elementnummer  (start_element) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 8: Matrixdiffusion (Fall 1)"); LineFeed(f);
        FilePrintString(f,"; - Anfangstiefe (value) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 9: Matrixdiffusion (Fall 2)"); LineFeed(f);
        FilePrintString(f,"; - Anfangstiefe (value) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 10: Vorverfeinern um allen Knoten, die sich auf einer Linie befinden"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x1 (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y1 (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z1 (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x2 (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y2 (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z2 (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente im Gebiet) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 11: Vorverfeinern um alle Knoten in einem Gebiet"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Koordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Radius (radius) [>=0.;0.]"); LineFeed(f);
        FilePrintString(f,"; - Gebietstyp  (distribution_type) [0<=...<=2;-1]"); LineFeed(f);
        FilePrintString(f,"; - Level (fuer alle Elemente im Gebiet) (level) [>0;0]"); LineFeed(f);
        LineFeed(f);
#endif
    }
  return ok;
}

/**************************************************************************
ROCKFLOW - Funktion: 
Task: Test if list exists
Programing:
09/2003   OK   Implementation
**************************************************************************/
int REFListExists(void)
{
  if(!list_of_refine_elements)
    return 0;
  else
    return 1;
}




