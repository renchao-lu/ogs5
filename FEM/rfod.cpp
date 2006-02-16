/**************************************************************************/
/* ROCKFLOW - Modul: rfod.c
                                                                          */
/* Aufgabe: Randbedingungen
   - Datenstrukturen und Definitionen fuer den Datentyp OBSERVATION_DATA
   - Verwaltung von OBSERVATION_DATA mit dem Datentyp LIST_OBSERVATION_DATA
   - Bereitstellung von Methoden fuer den Datentyp OBSERVATION_DATA
   - Schnittstellenfunktion zu Rockflow-Modelle
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Used tools */
#include "mathlib.h"
#include "geo_strings.h"

/* Used objects */
#include "nodes.h"
#include "gridadap.h"
#include "files.h"
#include "rf_tim_new.h"

/* Interface */
#include "rfod.h"


/* Definitionen */

  /* Liste von Randbedingungen */
static LIST_OBSERVATION_DATA *list_of_observation_data = NULL;

  /* Schluesselwoert */
char *keyword_observation_data = "OBSERVATION_DATA";


/* Default-Gruppen-Name */
static char DEFAULT_GROUP_NAME[]="ROCKFLOW";


/* Interne Lesefunktionen */
int FctObservationData(char *data, int found, FILE * f);




/* Nur fuer den Interface RF-OD */
static VARDMATRIX *od_matrix_values = NULL;
static VARDMATRIX *last_od_matrix_values = NULL;
static char **od_names = NULL;
static long od_names_number = 0;



/**************************************************************************/
/* ROCKFLOW - Funktion: create_observation_data
                                                                          */
/* Aufgabe:
   Konstruktor fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *create_observation_data(char *name)
{
  OBSERVATION_DATA *od;

  od = (OBSERVATION_DATA *) Malloc(sizeof(OBSERVATION_DATA));
  if ( od == NULL ) return NULL;


  od->name = (char *) Malloc((int)strlen(name)+1);
  if ( od->name == NULL ) {
         Free(od);
         return NULL;
  }
  strcpy(od->name,name);

  od->group_name=NULL;
  od->process_name=NULL; 
  od->iter_name=NULL; 

  od->type=0;
  od->time_radius=0.0;
  od->radius=MKleinsteZahl;

  od->curve=0;
  od->index=-1;

  od->node=-1;

  od->count_of_values=0;
  od->values=NULL;
  od->count_of_times=0;
  od->times=NULL;
  od->count_of_coeffs=0;
  od->coeffs=NULL;
  od->count_of_points=0;

  od->x=NULL;
  od->y=NULL;
  od->z=NULL;
  od->nodes=NULL;

  return od;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_observation_data
                                                                          */
/* Aufgabe:
   Destruktor fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des OD's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_observation_data(void *member)
{
  OBSERVATION_DATA *od=(OBSERVATION_DATA *)member;

  if ( od->name ) od->name=(char *)Free(od->name);
  if ( od->group_name ) od->group_name=(char *)Free(od->group_name);
  if ( od->process_name ) od->process_name=(char *)Free(od->process_name);
  if ( od->iter_name ) od->iter_name=(char *)Free(od->iter_name);
  if ( od->values ) od->values=(double *)Free(od->values);
  if ( od->times ) od->times=(double *)Free(od->times);
  if ( od->coeffs ) od->coeffs=(double *)Free(od->coeffs);
  if (od->nodes) od->nodes=(long *)Free(od->nodes);
  if ( od->x ) od->x=(double *)Free(od->x);
  if ( od->y ) od->y=(double *)Free(od->y);
  if ( od->z ) od->z=(double *)Free(od->z);
  if ( od ) od=(OBSERVATION_DATA *)Free(od);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data(OBSERVATION_DATA *od, \
                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( od->values ) od->values=(double *)Free(od->values);
  if ( od->times ) od->times=(double *)Free(od->times);
  if ( od->coeffs ) od->coeffs=(double *)Free(od->coeffs);

  od->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( od->values == NULL ) return NULL;

  od->times=(double *)Malloc(count_of_values*sizeof(double));
  if ( od->times == NULL ) { 
    if ( od->values ) od->values=(double *)Free(od->values);
    return NULL;
  }


  od->coeffs=(double *)Malloc(count_of_values*sizeof(double));
  if ( od->coeffs == NULL ) { 
    if ( od->times ) od->times=(double *)Free(od->times);
    if ( od->values ) od->values=(double *)Free(od->values);
    return NULL;
  }

  od->count_of_values=count_of_values;
  od->count_of_times=count_of_values;
  od->count_of_coeffs=count_of_values;
  return od;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data_values
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data_values(OBSERVATION_DATA *od, \
                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( od->values ) od->values=(double *)Free(od->values);

  od->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( od->values == NULL ) return NULL;

  od->count_of_values=count_of_values;
  return od;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data_times
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data_times(OBSERVATION_DATA *od, \
                                        long count_of_times)
{
  if (count_of_times <= 0) return NULL;

  if ( od->times ) od->times=(double *)Free(od->times);

  od->times=(double *)Malloc(count_of_times*sizeof(double));
  if ( od->times == NULL ) { 
    return NULL;
  }

  od->count_of_times=count_of_times;
  return od;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data_coeffs
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data_coeffs(OBSERVATION_DATA *od, \
                                        long count_of_coeffs)
{
  if (count_of_coeffs <= 0) return NULL;

  if ( od->coeffs ) od->coeffs=(double *)Free(od->coeffs);

  od->coeffs=(double *)Malloc(count_of_coeffs*sizeof(double));
  if ( od->coeffs == NULL ) { 
    return NULL;
  }

  od->count_of_coeffs=count_of_coeffs;
  return od;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: delete_observation_data
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Adresse des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *delete_observation_data(OBSERVATION_DATA *od)
{
  if ( od->values ) od->values=(double *)Free(od->values);
  if ( od->times ) od->times=(double *)Free(od->times);
  if ( od->coeffs ) od->coeffs=(double *)Free(od->coeffs);
  od->count_of_values=0;
  od->count_of_times=0;
  od->count_of_coeffs=0;

  return od;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data_coor
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data_coor(OBSERVATION_DATA *od, \
                                                                                        long count_of_points)
{
  if (count_of_points <= 0) return NULL;

  if (od->x) od->x=(double *)Free(od->x);
  if (od->y) od->y=(double *)Free(od->y);
  if (od->z) od->z=(double *)Free(od->z);

  od->x=(double *)Malloc(count_of_points*sizeof(double));
  if ( od->x == NULL ) return NULL;
  od->y=(double *)Malloc(count_of_points*sizeof(double));
  if ( od->y == NULL ) {
        od->x=(double *)Free(od->x);
        return NULL;
  }
  od->z=(double *)Malloc(count_of_points*sizeof(double));
  if ( od->z == NULL ) {
        od->x=(double *)Free(od->x);
        od->y=(double *)Free(od->y);
        return NULL;
  }

  od->count_of_points=count_of_points;
  return od;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_observation_data_coor
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Adresse des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *delete_observation_data_coor(OBSERVATION_DATA *od)
{
  if ( od->x ) od->x=(double *)Free(od->x);
  if ( od->y ) od->y=(double *)Free(od->y);
  if ( od->z ) od->z=(double *)Free(od->z);
  od->count_of_points=0;

  return od;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: init_observation_data_nodes
                                                                          */
/* Aufgabe:
   Initialisator fuer OBSERVATION_DATA
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *init_observation_data_nodes(OBSERVATION_DATA *od, \
                                                                                        long count_of_points)
{
  if (count_of_points <= 0) return NULL;

  if (od->nodes) od->nodes=(long *)Free(od->nodes);

  od->nodes=(long *)Malloc(count_of_points*sizeof(long));
  if ( od->nodes == NULL ) return NULL;

  od->count_of_points=count_of_points;
  return od;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_observation_data_nodes
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Adresse des OD's.
                                                                          */
/* Ergebnis:
   - Adresse des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *delete_observation_data_nodes(OBSERVATION_DATA *od)
{
  if ( od->nodes ) od->nodes=(long *)Free(od->nodes);

  return od;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_name
                                                                          */
/* Aufgabe:
   Liefert Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Name des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_od_name(OBSERVATION_DATA *od)
{
  if (od->name)
    return od->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_name
                                                                          */
/* Aufgabe:
   Setzt Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_name(OBSERVATION_DATA *od, char *name)
{
  if (!name) return;
  if(od) {
    od->name=(char *)Free(od->name);
    od->name = (char *) Malloc((int)strlen(name)+1);
    if ( od->name == NULL ) return;
    strcpy(od->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_group_name
                                                                          */
/* Aufgabe:
   Liefert Gruppen-Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Name des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_od_group_name(OBSERVATION_DATA *od)
{
  if (od->group_name)
    return od->group_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_group_name
                                                                          */
/* Aufgabe:
   Setzt Gruppen-Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E char *group_name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_group_name(OBSERVATION_DATA *od, char *group_name)
{
  if (!group_name) return;
  if(od) {
    od->group_name=(char *)Free(od->group_name);
    od->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( od->group_name == NULL ) return;
    strcpy(od->group_name,group_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_process_name
                                                                          */
/* Aufgabe:
   Liefert Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Name des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_od_process_name(OBSERVATION_DATA *od)
{
  if (od->process_name)
    return od->process_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_process_name
                                                                          */
/* Aufgabe:
   Setzt Name des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E char *process_name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_process_name(OBSERVATION_DATA *od, char *process_name)
{
  if (!process_name) return;
  if(od) {
    od->process_name=(char *)Free(od->process_name);
    od->process_name = (char *) Malloc((int)strlen(process_name)+1);
    if ( od->process_name == NULL ) return;
    strcpy(od->process_name,process_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_iter_name
                                                                          */
/* Aufgabe:
   Liefert Iterationsname des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Name des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_od_iter_name(OBSERVATION_DATA *od)
{
  if (od->iter_name)
    return od->iter_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_iter_name
                                                                          */
/* Aufgabe:
   Setzt Iterationsname des OD's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E char *iter_name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_iter_name(OBSERVATION_DATA *od, char *iter_name)
{
  if (!iter_name) return;
  if(od) {
    od->iter_name=(char *)Free(od->iter_name);
    od->iter_name = (char *) Malloc((int)strlen(iter_name)+1);
    if ( od->iter_name == NULL ) return;
    strcpy(od->iter_name,iter_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_type
                                                                          */
/* Aufgabe:
   Setzt Typ des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long type: Typ (Eingabemöglichkeit)                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_od_type(OBSERVATION_DATA *od, long type)
{
  if (od) od->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_type
                                                                          */
/* Aufgabe:
   Liefert Typ des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long get_od_type(OBSERVATION_DATA *od)
{
        return (od)?od->type:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_lse
                                                                          */
/* Aufgabe:
   Setzt least square error des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf eine Instanz vom Typ
                                        OBSERVATION_DATA.
   E double lse: Minimales Fehler-Quadrat.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_lse(OBSERVATION_DATA *od, double lse)
{
  if (od) od->lse=lse;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_lse
                                                                          */
/* Aufgabe:
   Liefert least square error des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf eine Instanz vom Typ
                                        OBSERVATION_DATA.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_lse(OBSERVATION_DATA *od)
{
  return (od)?od->lse:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_time_radius
                                                                          */
/* Aufgabe:
   Setzt Zeitradius des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E double time_radius: Verteilungsradius
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_od_time_radius(OBSERVATION_DATA *od, double time_radius)
{
  if (od) od->time_radius=time_radius;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_time_radius
                                                                          */
/* Aufgabe:
   Liefert Zeitradius des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Verteilungsradius -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_od_time_radius(OBSERVATION_DATA *od)
{
  return (od)?od->time_radius:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_curve
                                                                          */
/* Aufgabe:
   Setzt Kurve des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long curve: Kurve des OD's                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_curve(OBSERVATION_DATA *od, long curve)
{
  if (od) od->curve=curve;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_curve
                                                                          */
/* Aufgabe:
   Liefert Kurve des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_curve(OBSERVATION_DATA *od)
{
        return (od)?od->curve:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_index
                                                                          */
/* Aufgabe:
   Setzt Index des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_index(OBSERVATION_DATA *od, int index)
{
  if (od) od->index=index;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_index
                                                                          */
/* Aufgabe:
   Liefert Index des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_od_index(OBSERVATION_DATA *od)
{
        return (od)?od->index:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_count_of_values(OBSERVATION_DATA *od)
{
  return (od)?od->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_count_of_values(OBSERVATION_DATA *od, long count_of_values)
{
  if (od) od->count_of_values=count_of_values;
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_count_of_times
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_count_of_times(OBSERVATION_DATA *od)
{
  return (od)?od->count_of_times:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_count_of_times
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_count_of_times(OBSERVATION_DATA *od, long count_of_times)
{
  if (od) od->count_of_times=count_of_times;
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_count_of_coeffs
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des OD's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_count_of_coeffs(OBSERVATION_DATA *od)
{
  return (od)?od->count_of_coeffs:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_count_of_coeffs
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_count_of_coeffs(OBSERVATION_DATA *od, long count_of_coeffs)
{
  if (od) od->count_of_coeffs=count_of_coeffs;
  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_node
                                                                          */
/* Aufgabe:
   Setzt Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long node: Knotennummer                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_node(OBSERVATION_DATA *od, long node)
{
  if (od) od->node=node;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_node
                                                                          */
/* Aufgabe:
   Liefert Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Knotennummer -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_node(OBSERVATION_DATA *od)
{
        return (od)?od->node:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_value(OBSERVATION_DATA *od, long index, double value)
{
  if (od && od->values) {
    if ( (index<od->count_of_values) && (index>=0) )
          od->values[index]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_value(OBSERVATION_DATA *od, long index)
{
  if (od && od->values) {
    if ( (index<od->count_of_values) && (index>=0) )
          return od->values[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1.;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_values
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf Feldwerte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *get_od_values(OBSERVATION_DATA *od, long *count_of_values)
{
  if (od && od->values) {
    *count_of_values=od->count_of_values;
    return od->values;
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert NULL)!");
    return NULL;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_coeffs
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf Feldwerte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *get_od_coeffs(OBSERVATION_DATA *od, long *count_of_values)
{
  if (od && od->coeffs) {
    *count_of_values=od->count_of_coeffs;
    return od->coeffs;
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert NULL)!");
    return NULL;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_times
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf Feldwerte des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *get_od_times(OBSERVATION_DATA *od, long *count_of_times)
{
  double *rv=NULL;

  if (od && od->times) {
    *count_of_times=od->count_of_times;
    rv = od->times;
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert NULL)!");
  }

  return rv;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_time
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_time(OBSERVATION_DATA *od, long index, double time)
{
  if (od && od->times) {
    if ( (index<od->count_of_values) && (index>=0) )
          od->times[index]=time;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_time
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_time(OBSERVATION_DATA *od, long index)
{
  double rv=-1.0;

  if (od && od->times) {
    if ( (index<od->count_of_values) && (index>=0) )
          rv = od->times[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
  }

  return rv;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_coeff
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_coeff(OBSERVATION_DATA *od, long index, double coeff)
{
  if (od && od->coeffs) {
    if ( (index<od->count_of_values) && (index>=0) )
          od->coeffs[index]=coeff;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_coeff
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_coeff(OBSERVATION_DATA *od, long index)
{
  if (od && od->coeffs) {
    if ( (index<od->count_of_values) && (index>=0) )
          return od->coeffs[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1.;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_x
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
   E long xcoor: X-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_x(OBSERVATION_DATA *od, long index, double xcoor)
{
  if (od && od->x) {
    if ( (index<od->count_of_points) && (index>=0) )
          od->x[index]=xcoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_x
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - X-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_x(OBSERVATION_DATA *od, long index)
{
  if (od && od->x) {
    if ( (index<od->count_of_points) && (index>=0) )
          return od->x[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1.;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_y
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
   E long xcoor: Y-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_y(OBSERVATION_DATA *od, long index, double ycoor)
{
  if (od && od->y) {
    if ( (index<od->count_of_points) && (index>=0) )
          od->y[index]=ycoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_y
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - Y-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_y(OBSERVATION_DATA *od, long index)
{
  if (od && od->y) {
    if ( (index<od->count_of_points) && (index>=0) )
          return od->y[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1.;
  }
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_z
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
   E long xcoor: X-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_z(OBSERVATION_DATA *od, long index, double zcoor)
{
  if (od && od->z) {
    if ( (index<od->count_of_points) && (index>=0) )
          od->z[index]=zcoor;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_z
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - X-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_z(OBSERVATION_DATA *od, long index)
{
  if (od && od->z) {
    if ( (index<od->count_of_points) && (index>=0) )
          return od->z[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1.;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_radius
                                                                          */
/* Aufgabe:
   Setzt Radius des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long radius: Verteilungsradius
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_radius(OBSERVATION_DATA *od, double radius)
{
  if (od) od->radius=radius;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_radius
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - Verteilungsradius -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_od_radius(OBSERVATION_DATA *od)
{
        return (od)?od->radius:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_od_nodes
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
   E long xcoor: X-Koordinate des Punktes mit dem Index index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_od_nodes(OBSERVATION_DATA *od, long index, long node)
{
  if (od && od->nodes) {
    if ( (index<od->count_of_points) && (index>=0) )
          od->nodes[index]=node;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_od_nodes
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
   E long index: Index des Punktes
                                                                          */
/* Ergebnis:
   - X-Koordinate des Punktes -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_od_nodes(OBSERVATION_DATA *od, long index)
{
  if (od && od->nodes) {
    if ( (index<od->count_of_points) && (index>=0) )
          return od->nodes[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (OD) (Returnwert -1)!");
          return -1;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (OD) (Returnwert -1)!");
        return -1;
  }
}





/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_observation_data_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Randbedingungen;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Listenname
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
LIST_OBSERVATION_DATA *create_observation_data_list (char *name)
{
  LIST_OBSERVATION_DATA *list_of_observation_data;

  list_of_observation_data = (LIST_OBSERVATION_DATA *) Malloc(sizeof(LIST_OBSERVATION_DATA));
  if ( list_of_observation_data == NULL ) return NULL;


  list_of_observation_data->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_observation_data->name == NULL ) {
         Free(list_of_observation_data);
         return NULL;
  }
  strcpy(list_of_observation_data->name,name);

  list_of_observation_data->od_list=create_list();
  if ( list_of_observation_data->od_list == NULL ) {
    Free(list_of_observation_data->name);
        Free(list_of_observation_data);
        return NULL;
  }

  list_of_observation_data->names_of_observation_data=NULL;
  list_of_observation_data->count_of_observation_data_name=0;

  return list_of_observation_data;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_observation_data_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der Randbedingungen aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_observation_data_list(void)
{
  if (list_of_observation_data->name) list_of_observation_data->name=(char *)Free(list_of_observation_data->name);
  if(list_of_observation_data->names_of_observation_data) \
    list_of_observation_data->names_of_observation_data = \
      (char **)Free(list_of_observation_data->names_of_observation_data);

  if (list_of_observation_data->od_list) {
    delete_list(list_of_observation_data->od_list,destroy_observation_data);
    list_of_observation_data->od_list=destroy_list(list_of_observation_data->od_list);
  }

  if (list_of_observation_data) list_of_observation_data=(LIST_OBSERVATION_DATA *)Free(list_of_observation_data);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: observation_data_list_empty
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
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int observation_data_list_empty(void)
{
  if (!list_of_observation_data) return 0;
  else if (!list_of_observation_data->od_list) return 0;
  else return list_empty(list_of_observation_data->od_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_od_init
                                                                          */
/* Aufgabe:
   Liste der OD's initialisieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void list_od_init(void)
{
  if (list_of_observation_data) list_current_init(list_of_observation_data->od_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_od_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_od_next (void)
{
  return list_of_observation_data->od_list ?
             get_list_next(list_of_observation_data->od_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_observation_data_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ OBSERVATION_DATA erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen OD's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_observation_data_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_observation_data) return 0;
  for ( i=0; i<list_of_observation_data->count_of_observation_data_name; i++)
    if(strcmp(list_of_observation_data->names_of_observation_data[i],name) == 0) break;
  if (i < list_of_observation_data->count_of_observation_data_name) return i;

  list_of_observation_data->names_of_observation_data= \
          (char **) Realloc(list_of_observation_data->names_of_observation_data, \
          (list_of_observation_data->count_of_observation_data_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_observation_data->names_of_observation_data[i]=new_name;

  return ++(list_of_observation_data->count_of_observation_data_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_observation_data_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen OD's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_observation_data_names (void)
{
  long i;

  for (i=0; i<list_of_observation_data->count_of_observation_data_name; i++)
    if(list_of_observation_data->names_of_observation_data[i]) \
      list_of_observation_data->names_of_observation_data[i] = \
        (char *)Free(list_of_observation_data->names_of_observation_data[i]);

  return list_of_observation_data->count_of_observation_data_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_observation_data_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ OBSERVATION_DATA in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_observation_data_list (OBSERVATION_DATA *od)
{
  if(!list_of_observation_data) return 0;
  list_of_observation_data->count_of_observation_data=append_list(list_of_observation_data->od_list,(void *) od);
  return list_of_observation_data->count_of_observation_data;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_observation_data_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ OBSERVATION_DATA der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender OBSERVATION_DATA-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
long remove_observation_data_list (OBSERVATION_DATA *od)
{
  list_of_observation_data->count_of_observation_data = \
      remove_list_member(list_of_observation_data->od_list,(void *) od, destroy_observation_data);
  return list_of_observation_data->count_of_observation_data;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: observation_data_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines OD-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long observation_data_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_observation_data->count_of_observation_data_name; i++)
    if(strcmp(list_of_observation_data->names_of_observation_data[i],name) == 0) break;
  if (i < list_of_observation_data->count_of_observation_data_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateObservationDataList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Randbedingungen (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void CreateObservationDataList(void)
{
  list_of_observation_data = create_observation_data_list("OBSERVATION_DATA");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyObservationDataList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von Randbedingungen (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void DestroyObservationDataList(void)
{
  long i;

  if (od_matrix_values) DestroyVarDoubleMatrix(od_matrix_values);
  if (last_od_matrix_values) DestroyVarDoubleMatrix(last_od_matrix_values);

  /* ah zv */
  for(i=0; i<od_names_number; i++)
    if (od_names[i]) Free(od_names[i]);
  if (od_names) od_names=(char **)Free(od_names);
  od_names_number=0;

  undeclare_observation_data_names();
  destroy_observation_data_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ObservationDataListEmpty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 --> Liste leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int ObservationDataListEmpty(void)
{
  return observation_data_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetObservationData
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetObservationData (char *name)
{
  char *new_name;

  if (!name) return;

  od_names=(char **) Realloc(od_names,(od_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  od_names[od_names_number]=new_name;
  od_names[++od_names_number]=NULL;

  return;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: IndexObservationData
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines OD-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des OD's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long IndexObservationData(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<od_names_number; i++)
    if(strcmp(od_names[i],name) == 0) break;
  if (i < od_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetObservationDataGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp OBSERVATION_DATA
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: OBSERVATION_DATA *od -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp OBSERVATION_DATA
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
OBSERVATION_DATA *GetObservationDataGroup(char *name,OBSERVATION_DATA *od)
{
  if (!name) return NULL;
  if (!observation_data_name_exist(name) ) return NULL;

  if(od==NULL) list_od_init();

  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name))==0 ) break;
  }

  return od;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyObservationDataListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche OBSERVATION_DATA-Objekte der Gruppe NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name - Gruppenname
                                                                          */
/* Ergebnis:
   Kontrollflag
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
int DestroyObservationDataListGroup(char *name)
{
  OBSERVATION_DATA *od=NULL;

  if (!name) return -1;
  if (!observation_data_name_exist(name) ) return -1;

  list_od_init();

  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name))==0 )
        remove_observation_data_list(od);
  }

  return 0;
}




/*  OD-RF */


/**************************************************************************/
/* ROCKFLOW - Funktion: GetObservationDataNamesNumber
                                                                          */
/* Aufgabe:
   Liefert die Anzahl der gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -

                                                                          */
/* Ergebnis:
   - siehe oben -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long GetObservationDataNamesNumber (void)
{
  return od_names_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetObservationDataName
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetObservationDataName (long i)
{
  return (od_names[i])?od_names[i]:NULL;
}



/*------------------------------------------------------------------------*/
/* Lesefunktionen */



/**************************************************************************/
/* ROCKFLOW - Funktion: FctObservationData
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #OBSERVATION_DATA gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.
   #OBSERVATION_DATA: Iterations-Parameter fuer alle Modelle.
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
   10/2001     AH      erste Version
                                                                          */
/**************************************************************************/
int FctObservationData(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    char od_name[255]="NONE";
    double d;
    long l;
    int i=0;
    OBSERVATION_DATA *od=NULL;
    char *name;
    char *process_name;
    char *iter_name;
    int dec=1;
    long np;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #OBSERVATION_DATA");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {           /* OBSERVATION_DATA nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f, "; Schluesselwort: #");
      FilePrintString(f, keyword_observation_data);
      LineFeed(f);
#endif
    } /* found */
    else {                /* ITERATION_PRESSURE gefunden */
      FilePrintString(f, "#");
      FilePrintString(f, keyword_observation_data);
      LineFeed(f);
#ifdef EXT_RFD
      FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein");
      LineFeed(f);
#endif
        while (StrTestHash(&data[p], &pos)) {
          LineFeed(f);
          while (StrTestLong(&data[p+=pos])) {
            if ( (od=create_observation_data(od_name)) == NULL ) {
              ok = 0;
              break;
            }
            else {
              set_od_group_name(od,DEFAULT_GROUP_NAME);                            
              ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
              set_od_type(od,l);
            }


                if (get_od_type(od)==0) {
                        StrReadString (&name,&data[p+=pos],f,TFString,&pos);
                        set_od_name(od,name);                            
                        name=(char *)Free(name);
                        StrReadString (&process_name,&data[p+=pos],f,TFString,&pos);
                        set_od_process_name(od,process_name);                            
                        process_name=(char *)Free(process_name);
                        StrReadString (&iter_name,&data[p+=pos],f,TFString,&pos);
                        set_od_iter_name(od,iter_name);                            
                        iter_name=(char *)Free(iter_name);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_od_node(od,l);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        np=l;
                        init_observation_data(od,np);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time_radius(od,d);
                        LineFeed(f);
                        for (i=0; i<np; i++) {
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_time(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_value(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_coeff(od,i,d);
                            LineFeed(f);
                        }
                }
                else if (get_od_type(od)==1) {
                        StrReadString (&process_name,&data[p+=pos],f,TFString,&pos);
                        set_od_process_name(od,process_name);                            
                        process_name=(char *)Free(process_name);
                        StrReadString (&iter_name,&data[p+=pos],f,TFString,&pos);
                        set_od_iter_name(od,iter_name);                            
                        iter_name=(char *)Free(iter_name);
                        init_observation_data_coor(od,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_x(od,0,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_y(od,0,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_z(od,0,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_radius(od,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        np=l;
                        init_observation_data(od,np);
                        LineFeed(f);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time_radius(od,d);
                        for (i=0; i<np; i++) {
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_time(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_value(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_coeff(od,i,d);
                            LineFeed(f);
                        }
                }
                else if (get_od_type(od)==2) {
                        StrReadString (&process_name,&data[p+=pos],f,TFString,&pos);
                        set_od_process_name(od,process_name);                            
                        process_name=(char *)Free(process_name);
                        StrReadString (&iter_name,&data[p+=pos],f,TFString,&pos);
                        set_od_iter_name(od,iter_name);                            
                        iter_name=(char *)Free(iter_name);
                        init_observation_data_times(od,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time(od,0,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        np=l;
                        init_observation_data_values(od,np);
                        init_observation_data_coeffs(od,np);
                        init_observation_data_nodes(od,np);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time_radius(od,d);
                        LineFeed(f);
                        for (i=0; i<np; i++) {
                            ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                            set_od_nodes(od,i,l);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_value(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_coeff(od,i,d);
                            LineFeed(f);
                        }
                }
                else if (get_od_type(od)==3) {
                        StrReadString (&process_name,&data[p+=pos],f,TFString,&pos);
                        set_od_process_name(od,process_name);                            
                        process_name=(char *)Free(process_name);
                        init_observation_data_times(od,1);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time(od,0,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        np=l;
                        init_observation_data_coor(od,np);
                        init_observation_data_values(od,np);
                        init_observation_data_coeffs(od,np);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_radius(od,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_od_time_radius(od,d);
                        LineFeed(f);
                        for (i=0; i<np; i++) {
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_x(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_y(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_z(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_value(od,i,d);
                            ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                            set_od_coeff(od,i,d);
                            LineFeed(f);
                        }
                    }
                }

            if (od) {
              if (dec) declare_observation_data_name(DEFAULT_GROUP_NAME);
              else dec=0;
              insert_observation_data_list(od);
            }
            LineFeed(f);
          } /* while long */
        }  /* while hash */
#ifdef EXT_RFD
        FilePrintString(f,"; Es folgen beliebig viele Bloecke mit mindestens 3 Werten."); LineFeed(f);
        FilePrintString(f,"; - Typ (type) [>=0;0]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 0: Einzelknotenzuweisung"); LineFeed(f);
        FilePrintString(f,"; - Knotennummer (node) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenwert (value) [;1]"); LineFeed(f);
        LineFeed(f);
        FilePrintString(f,";   Typ = 1: Einzelknotenzuweisung"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten x (*x) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten y (*y) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenkoordinaten z (*z) [>=0;-1]"); LineFeed(f);
        FilePrintString(f,"; - Umgebungsradius (radius) [;1]"); LineFeed(f);
        FilePrintString(f,"; - Knotenwert (value) [;1]"); LineFeed(f);
        LineFeed(f);
#endif
    return ok;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: SetAllObservationData
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E OBSERVATION_DATA *od: Zeiger auf die Datenstruktur od.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetAllObservationData (void)
{
  OBSERVATION_DATA *od;
  char *name;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( (name=get_od_name(od)) != NULL ) SetObservationData(name);
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllObservationData
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void InitAllObservationData (void)
{
  OBSERVATION_DATA *od;
  long ncol=od_names_number;
  long j,col,row=0;
  char *name;

  if (ncol<=0) return;

  if (od_matrix_values) DestroyVarDoubleMatrix(od_matrix_values);
  od_matrix_values=CreateVarDoubleMatrix(ncol);
  if (last_od_matrix_values) DestroyVarDoubleMatrix(last_od_matrix_values);
  last_od_matrix_values=CreateVarDoubleMatrix(ncol);


  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    col=get_od_count_of_values(od);
    if ( (name=get_od_name(od)) != NULL ) {
      row=IndexObservationData(name);
      InitVarDoubleMatrix(od_matrix_values,row,col);
      InitVarDoubleMatrix(last_od_matrix_values,row,col);
      for (j=0; j<col; j++) {
        od_matrix_values->m[row][j]=0.0;
        last_od_matrix_values->m[row][j]=0.0;
      }
    }
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllObservationData
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void ConfigAllObservationData (void)
{
  OBSERVATION_DATA *od;
  char *process_name;
  int j,k;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    set_od_index(od,-1);
    if ( (process_name=get_od_process_name(od)) != NULL ) {
      for (j=0;j<danz_n;j++) {
        for (k=0;k<datafield_n[j].transfer;k++)
          if ( strcmp(StrUp(process_name),StrUp(datafield_n[j].name))==0 ) { 
            set_od_index(od,j+k);
          }
      }
    }
  }
 
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SaveAllModelObservationData
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void SaveAllModelObservationData (void)
{
  OBSERVATION_DATA *od;
  char *name;
  long j;
  long node;
  double d;
  long n,col;
  int save_flag;
  long row;


  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( (name=get_od_name(od))!=NULL && get_od_index(od)>=0) {
      if (get_od_type(od)!=0) continue;
      node=get_od_node(od);
      if (GetNode(node)==NULL) {
        DisplayErrorMsg("Fehler in SaveAllModelObservationData !!!");
        DisplayErrorMsg("Knoten exisiert nicht --> Abbruch !!!");
        abort();
      }
      row=IndexObservationData(name);
      save_flag=0;
      n=get_od_count_of_values(od);
      col=-1;
      for (j=0;j<n;j++) {
        if ( fabs(aktuelle_zeit-get_od_time(od,j)) < fabs(get_od_time_radius(od)) ) {
          col=j;
          save_flag=1;
          break;
        }
      }
      
      if (save_flag && col>=0) { 
        j=get_od_index(od); 
        d=GetNodeVal(node,datafield_n[j].idx_program);
        od_matrix_values->m[row][col]=d;
      }
    }
  }
 
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SaveToLastAllModelObservationData
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version (unbenutzt)
                                                                          */
/**************************************************************************/
void SaveToLastAllModelObservationData (void)
{
  OBSERVATION_DATA *od;
  char *name;
  long j;
  long node;
  double d;
  long n,col;
  int save_flag;
  long row;


  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( (name=get_od_name(od))!=NULL && get_od_index(od)>=0) {
      if (get_od_type(od)!=0) continue;
      node=get_od_node(od);
      if (GetNode(node)==NULL) {
        DisplayErrorMsg("Fehler in SaveAllModelObservationData --> Abbruch !!!");
        abort();
      }
      row=IndexObservationData(name);
      save_flag=0;
      n=get_od_count_of_values(od);
      col=-1;
      for (j=0;j<n;j++) {
        if ( fabs(aktuelle_zeit-get_od_time(od,j)) < fabs(get_od_time_radius(od)) ) {
          col=j;
          save_flag=1;
          break;
        }
      }
      
      if (save_flag && col>=0) { 
        j=get_od_index(od); 
        d=GetNodeVal(node,datafield_n[j].idx_program);
        last_od_matrix_values->m[row][col]=d;
      }
    }
  }
 
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CopyToLastAllModelObservationData
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void CopyToLastAllModelObservationData (void)
{
  if (od_matrix_values && last_od_matrix_values)
    CopyVarDoubleMatrix(last_od_matrix_values,od_matrix_values);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetTimesObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *GetTimesObservationData (char *name, long *number_of_times)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    if ( get_od_index(od) >= 0) {
      return get_od_times(od,number_of_times);
    }
  }

  return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetValuesObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *GetValuesObservationData (char *name, long *number_of_values)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    if ( get_od_index(od) >= 0) {
      return get_od_values(od,number_of_values);
    }
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetCoeffsObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *GetCoeffsObservationData (char *name, long *number_of_values)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    if ( get_od_index(od) >= 0) {
      return get_od_coeffs(od,number_of_values);
    }
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetModelValuesObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *GetModelValuesObservationData (char *name, long *number_of_values)
{
  OBSERVATION_DATA *od;
  long row=IndexObservationData(name);

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    if ( get_od_index(od) >= 0) {
      *number_of_values=od_matrix_values->col[row];
      return od_matrix_values->m[row];
    }
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetLastModelValuesObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double *GetLastModelValuesObservationData (char *name, long *number_of_values)
{
  OBSERVATION_DATA *od;
  long row=IndexObservationData(name);

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    if ( get_od_index(od) >= 0) {
      *number_of_values=last_od_matrix_values->col[row];
      return last_od_matrix_values->m[row];
    }
  }

  return NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetIterationNameObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetIterationNameObservationData (char *name)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    return get_od_iter_name(od);
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SaveLeastSquareErrorObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int SaveLeastSquareErrorObservationData (char *name, double lse)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    set_od_lse(od,lse);
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetLeastSquareErrorObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetLeastSquareErrorObservationData (char *name, double *lse)
{
  OBSERVATION_DATA *od;

  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    if ( strcmp(StrUp(get_od_name(od)),StrUp(name)) ) continue;
    *lse=get_od_lse(od);
    return 1;
  }

  return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetAllLeastSquareErrorObservationData
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die OD.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long i: Index in der Namensliste.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetAllLeastSquareErrorObservationData(double *lse)
{
  OBSERVATION_DATA *od;

  *lse=0.0;
  list_od_init();
  while ( (od=(OBSERVATION_DATA *)get_list_od_next()) != NULL ) {
    *lse+=get_od_lse(od);
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: WriteBothValuesObservationData
                                                                          */
/* Aufgabe:
   Liefert Wert der Randbedingung am Knoten mit dem index i.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
int WriteBothValuesObservationData (char *name, FILE *f)
{
  int i,ret=0;
  double *od_times,*od_values,*mod_values;
  long number_of_times,number_of_values;

  if (!name) return 0;

  od_times=GetTimesObservationData(name,&number_of_times);
  od_values=GetValuesObservationData(name,&number_of_values);
  mod_values=GetModelValuesObservationData(name,&number_of_values);

  if (od_values && mod_values) {
    LineFeed(f);
    FilePrintString(f,";   Time         Observed         Calculated"); 
    LineFeed(f);
    for (i=0; i<number_of_values; i++) {
      fprintf(f," %e ",od_times[i]);
      fprintf(f," %e ",od_values[i]);
      fprintf(f," %e ",mod_values[i]);
      LineFeed(f);
    }
  }
    
  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalculateSensitivityParameterInverseIteration
                                                                          */
/* Aufgabe:
   Liefert Wert der Randbedingung am Knoten mit dem index i.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
int CalculateSensitivityParameterInverseIteration(char *name, double dx, double *dfdx)
{
  double *mod_values,*last_mod_values;
  long number_of_values;
  long j;

  if (!name) return 0;
  if (fabs(dx)<MKleinsteZahl) return 0;

  mod_values=GetModelValuesObservationData(name,&number_of_values);
  last_mod_values=GetLastModelValuesObservationData(name,&number_of_values);

  if (mod_values && last_mod_values) {
    *dfdx=0.0;
    for (j=0; j<number_of_values; j++) {
      *dfdx+=mod_values[j]-last_mod_values[j];
    }
    *dfdx/=dx;
    return 1;
  }

  return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalculateRightSideSensitivityParameterInverseIteration
                                                                          */
/* Aufgabe:
   Liefert Wert der Randbedingung am Knoten mit dem index i.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
int CalculateRightSideSensitivityParameterInverseIteration(char *name, double *dfdx)
{
  double *od_values,*last_mod_values;
  long number_of_values;
  long j;

  if (!name) return 0;

  od_values=GetValuesObservationData(name,&number_of_values);
  last_mod_values=GetLastModelValuesObservationData(name,&number_of_values);

  if (od_values && last_mod_values) {
    *dfdx=0.0;
    for (j=0; j<number_of_values; j++) {
      (*dfdx)+=od_values[j]-last_mod_values[j];
    }
    return 1;
  }

  return 0;
}



/* Tools */

/**************************************************************************/
/* ROCKFLOW - Funktion: CreateVarDoubleMatrix
                                                                          */
/* Aufgabe:
   Konstruktor fuer VARDMATRIX
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long row: Anzahl der Zeilen.
                                                                          */
/* Ergebnis:
   - Adresse der Matrix -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
VARDMATRIX *CreateVarDoubleMatrix(long row)
{
  VARDMATRIX *vdm;
  register long i;

  vdm = (VARDMATRIX *) Malloc(sizeof(VARDMATRIX));
  if ( vdm == NULL ) return NULL;

  vdm->m = (double **) Malloc(row*sizeof(double *));
  if (vdm->m == NULL) return NULL;

  for (i=0; i<row; i++) vdm->m[i] = NULL;
  vdm->row=row;

  vdm->col=(long *) Malloc(row*sizeof(long));
  for (i=0; i<row; i++) vdm->col[i] = 0;

  return vdm;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitVarDoubleMatrix
                                                                          */
/* Aufgabe:
   Konstruktor fuer VARDMATRIX
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long row: Zeile.
   E long col: Spaltendimension.
                                                                          */
/* Ergebnis:
   - Adresse der Matrix -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
VARDMATRIX *InitVarDoubleMatrix(VARDMATRIX *vdm, long row, long col)
{
  if ( vdm==NULL || vdm->m==NULL ) return NULL;

  if (vdm->m[row] != NULL) Free(vdm->m[row]);
  vdm->m[row] = (double *) Malloc(col*sizeof(double));
  if (vdm->m[row] == NULL) { DestroyVarDoubleMatrix (vdm); return NULL; }
  vdm->col[row] = col;

  return vdm;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyVarDoubleMatrix
                                                                          */
/* Aufgabe:
   Destruktor fuer VARDMATRIX
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DAMTRIX dm: Instanz vom Typ VARDMATRIX.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void DestroyVarDoubleMatrix(VARDMATRIX *vdm)
{
  register long i;

  if ( vdm->col ) vdm->col=(long *)Free(vdm->col);
  for (i=0; i<vdm->row; i++)
    if ( vdm->m[i] ) vdm->m[i]=(double *)Free(vdm->m[i]);
  if ( vdm->m ) vdm->m=(double **)Free(vdm->m);
  if ( vdm ) vdm=(VARDMATRIX *)Free(vdm);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CopyVarDoubleMatrix
                                                                          */
/* Aufgabe:
   Konstruktor fuer VARDMATRIX
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long row: Anzeilen der Zeilen.
                                                                          */
/* Ergebnis:
   - Adresse der Matrix -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
VARDMATRIX *CopyVarDoubleMatrix(VARDMATRIX *dest, VARDMATRIX *src)
{
  register long i,j;

  if ( src==NULL || src->m==NULL ) return NULL;
  for (i=0; i<src->row; i++) {
    if ( (src->m[i]) == NULL ) return NULL;
  }

  if ( dest==NULL || dest->m==NULL ) return NULL;
  for (i=0; i<dest->row; i++) {
    if ( (dest->m[i]) == NULL ) return NULL;
  }

  for (i=0; i<src->row; i++) {
     dest->col[i] = src->col[i];
     for (j=0; j<src->col[i]; j++) {
       dest->m[i][j]=src->m[i][j];
     }
  }

  return dest;
}

