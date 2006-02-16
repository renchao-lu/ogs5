/**************************************************************************/
/* ROCKFLOW - Modul: rfiter.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp INVERSE_ITERATION
   - Verwaltung von INVERSE_ITERATION mit dem Datentyp LIST_INVERSE_ITERATION
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
   03/2003    RK      Quellcode bereinigt, Globalvariablen entfernt   
      
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Used tools */
#include "mathlib.h"
#include "geo_strings.h"

/* Used objects */
#include "rf_pcs.h" //OK_MOD"
#include "rfod.h"
#include "rfdb.h"

/* Interface */
#include "rfii.h"
#include "rfiv.h"

  /* Liste von Randbedingungen */
static LIST_INVERSE_ITERATION *list_of_inverse_iteration = NULL;

/* Nur fuer den Interface RF-BC */
//OK_IC #include "rfbc.h"
#include "tools.h"
static DMATRIX *ii_matrix_values = NULL;
static LMATRIX *ii_matrix_iter = NULL;
static char **ii_names = NULL;
static long ii_names_number = 0;
/*static long ii_cur_col = -1;*/



/* ================================================================ */
/* INVERSE_ITERATION */
/* ================================================================ */

  /* Schluesselwoert */
char *keyword_inverse_iteration = "INVERSE_ITERATION";


/* Default-Gruppen-Name */
static char DEFAULT_GROUP_NAME[]="ROCKFLOW";

/* Default-Name */
static char DEFAULT_NAME[]="DEFAULT";

/* Interne Deklarationen - Funktionen */



/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_iteration
                                                                          */
/* Aufgabe:
   Konstruktor fuer INVERSE_ITERATION
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - Adresse des II's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_ITERATION *create_inverse_iteration(char *name)
{
  INVERSE_ITERATION *ii;

  ii = (INVERSE_ITERATION *) Malloc(sizeof(INVERSE_ITERATION));
  if ( ii == NULL ) return NULL;


  ii->name = (char *) Malloc((int)strlen(name)+1);
  if ( ii->name == NULL ) {
         Free(ii);
         return NULL;
  }
  strcpy(ii->name,name);

  ii->group_name=NULL;
  ii->process_name=NULL;
  ii->type=0;

  ii->lse=0.0;

  ii->count_of_values=0;
  ii->values=NULL;
  ii->distribution_type=0;

  return ii;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_iteration
                                                                          */
/* Aufgabe:
   Destruktor fuer INVERSE_ITERATION
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des II's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_inverse_iteration(void *member)
{
  INVERSE_ITERATION *ii=(INVERSE_ITERATION *)member;

  if ( ii->name ) ii->name=(char *)Free(ii->name);
  if ( ii->group_name ) ii->group_name=(char *)Free(ii->group_name);
  if ( ii->process_name ) ii->process_name=(char *)Free(ii->process_name);
  if ( ii->values ) ii->values=(double *)Free(ii->values);
  if ( ii ) ii=(INVERSE_ITERATION *)Free(ii);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_inverse_iteration
                                                                          */
/* Aufgabe:
   Initialisator fuer INVERSE_ITERATION
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - Adresse des II's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_ITERATION *init_inverse_iteration(INVERSE_ITERATION *ii, \
                                                                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( ii->values ) ii->values=(double *)Free(ii->values);
  ii->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( ii->values == NULL ) return NULL;

  ii->count_of_values=count_of_values;
  return ii;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_inverse_iteration
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Adresse des II's.
                                                                          */
/* Ergebnis:
   - Adresse des II's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_ITERATION *delete_inverse_iteration(INVERSE_ITERATION *ii)
{
  if ( ii->values ) ii->values=(double *)Free(ii->values);
  ii->count_of_values=0;

  return ii;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_name
                                                                          */
/* Aufgabe:
   Liefert Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des II's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_ii_name(INVERSE_ITERATION *ii)
{
  if (ii->name)
    return ii->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_name
                                                                          */
/* Aufgabe:
   Setzt Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_name(INVERSE_ITERATION *ii, char *name)
{
  if (!name) return;
  if(ii) {
    ii->name=(char *)Free(ii->name);
    ii->name = (char *) Malloc((int)strlen(name)+1);
    if ( ii->name == NULL ) return;
    strcpy(ii->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_group_name
                                                                          */
/* Aufgabe:
   Liefert Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des II's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_ii_group_name(INVERSE_ITERATION *ii)
{
  if (ii->group_name)
    return ii->group_name;
  else
    return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_group_name
                                                                          */
/* Aufgabe:
   Setzt Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
   E char *group_name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_group_name(INVERSE_ITERATION *ii, char *group_name)
{
  if (!group_name) return;
  if(ii) {
    ii->group_name=(char *)Free(ii->group_name);
    ii->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( ii->group_name == NULL ) return;
    strcpy(ii->group_name,group_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_process_name
                                                                          */
/* Aufgabe:
   Liefert Prozess-Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des II's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_ii_process_name(INVERSE_ITERATION *ii)
{
  if (ii->process_name)
    return ii->process_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_process_name
                                                                          */
/* Aufgabe:
   Setzt Prozess-Name des II's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
   E char *process_name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_process_name(INVERSE_ITERATION *ii, char *process_name)
{
  if (!process_name) return;
  if(ii) {
    ii->process_name=(char *)Free(ii->process_name);
    ii->process_name = (char *) Malloc((int)strlen(process_name)+1);
    if ( ii->process_name == NULL ) return;
    strcpy(ii->process_name,process_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_type
                                                                          */
/* Aufgabe:
   Setzt Typ des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_type(INVERSE_ITERATION *ii, long type)
{
  if (ii) ii->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_type
                                                                          */
/* Aufgabe:
   Liefert Typ des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_type(INVERSE_ITERATION *ii)
{
        return (ii)?ii->type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_lse
                                                                          */
/* Aufgabe:
   Setzt least square error des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double lse: Minimales Fehler-Quadrat.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_lse(INVERSE_ITERATION *ii, double lse)
{
  if (ii) ii->lse=lse;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_lse
                                                                          */
/* Aufgabe:
   Liefert least square error des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_lse(INVERSE_ITERATION *ii)
{
  return (ii)?ii->lse:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_maxiter
                                                                          */
/* Aufgabe:
   Setzt maximale Iterationszahl des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E maxiter: maximale Iteration des nichtlinearen iterativen Loesers.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_maxiter(INVERSE_ITERATION *ii, long maxiter)
{
  if (ii) ii->maxiter=maxiter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_maxiter
                                                                          */
/* Aufgabe:
   Liefert maximale Iterationszahl des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - maximale Iterationszahl -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_maxiter(INVERSE_ITERATION *ii)
{
        return (ii)?ii->maxiter:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_criterium
                                                                          */
/* Aufgabe:
   Setzt Kriterium des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E long criterium: Kriterium (Konvergenztyp)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_criterium(INVERSE_ITERATION *ii, long criterium)
{
  if (ii) ii->criterium=criterium;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_criterium
                                                                          */
/* Aufgabe:
   Liefert Kriterium des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_criterium(INVERSE_ITERATION *ii)
{
        return (ii)?ii->criterium:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_norm
                                                                          */
/* Aufgabe:
   Setzt Norm (Konvergenznorm) des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E int norm : Norm 
                0: unendliche Norm
                1: Eins-Norm
                2: Euklidische Norm
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_norm(INVERSE_ITERATION *ii, int norm)
{
  if (ii) ii->norm=norm;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_norm
                                                                          */
/* Aufgabe:
   Liefert Norm (Konvergenznorm) des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_ii_norm(INVERSE_ITERATION *ii)
{
  return (ii)?ii->norm:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_abs_eps
                                                                          */
/* Aufgabe:
   Setzt absolute Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double abs_eps: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_abs_eps(INVERSE_ITERATION *ii, double abs_eps)
{
  if (ii) ii->abs_eps=abs_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_abs_eps
                                                                          */
/* Aufgabe:
   Liefert absolute Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_abs_eps(INVERSE_ITERATION *ii)
{
        return (ii)?ii->abs_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_rel_eps
                                                                          */
/* Aufgabe:
   Setzt relative Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double rel_eps: relative Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_rel_eps(INVERSE_ITERATION *ii, double rel_eps)
{
  if (ii) ii->rel_eps=rel_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_rel_eps
                                                                          */
/* Aufgabe:
   Liefert relative Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_rel_eps(INVERSE_ITERATION *ii)
{
        return (ii)?ii->rel_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_rel_cg_eps
                                                                          */
/* Aufgabe:
   Setzt die Anfangsgenauigkeit des II's (fuer variable Genauigkeit).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double rel_cg_eps: Anfangsgenauigkeit der Iteration (fuer variable Genauigkeit)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_rel_cg_eps(INVERSE_ITERATION *ii, double rel_cg_eps)
{
  if (ii) ii->rel_cg_eps=rel_cg_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_rel_cg_eps
                                                                          */
/* Aufgabe:
   Liefert die Anfangsgenauigkeit des II's (fuer variable Genauigkeit).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_rel_cg_eps(INVERSE_ITERATION *ii)
{
        return (ii)?ii->rel_cg_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_theta
                                                                          */
/* Aufgabe:
   Setzt Gewichtsfaktor des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double theta: Gewichtsfaktor fuer nichtlineare iterative Loeser.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_theta(INVERSE_ITERATION *ii, double theta)
{
  if (ii) ii->theta=theta;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_theta
                                                                          */
/* Aufgabe:
   Liefert Gewichtsfaktor des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_theta(INVERSE_ITERATION *ii)
{
        return (ii)?ii->theta:-1.;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_time
                                                                          */
/* Aufgabe:
   Setzt die Zeit der Gueltigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double time: Zeit der Gueltigkeit des Loesers
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_time(INVERSE_ITERATION *ii, double time)
{
  if (ii) ii->time=time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_time
                                                                          */
/* Aufgabe:
   Liefert die Zeit der Gueltigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_time(INVERSE_ITERATION *ii)
{
        return (ii)?ii->time:-1.;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_kind
                                                                          */
/* Aufgabe:
   Setzt Art der Zeitgueltigkeit des LSP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E long kind: Art der Zeitgueltigkeit
                0  : immer gueltig
                1  : gueltig in einem Zeitintervall
                2  : gueltig fuer einen bestimmten Zeitpunkt
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_kind(INVERSE_ITERATION *ii, long kind)
{
  if (ii) ii->kind=kind;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_kind
                                                                          */
/* Aufgabe:
   Liefert Art der Zeitgueltigkeit des LSP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_kind(INVERSE_ITERATION *ii)
{
        return (ii)?ii->kind:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des II's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_count_of_values(INVERSE_ITERATION *ii)
{
  return (ii)?ii->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_count_of_values(INVERSE_ITERATION *ii, long count_of_values)
{
  if (ii) ii->count_of_values=count_of_values;
  return;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_value(INVERSE_ITERATION *ii, long index, double value)
{
  if (ii && ii->values) {
    if ( (index<ii->count_of_values) && (index>=0) )
          ii->values[index]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (BC) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_value(INVERSE_ITERATION *ii, long index)
{
  if (ii && ii->values) {
    if ( (index<ii->count_of_values) && (index>=0) )
          return ii->values[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (BC) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (BC) (Returnwert -1)!");
        return -1.;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_distribution_type
                                                                          */
/* Aufgabe:
   Setzt Verteilungstyp des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E long refine_type: Verteilungstyp
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_distribution_type(INVERSE_ITERATION *ii, long distribution_type)
{
  if (ii) ii->distribution_type=distribution_type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_distribution_type
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - Verteilungstyp -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_distribution_type(INVERSE_ITERATION *ii)
{
        return (ii)?ii->distribution_type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_level
                                                                          */
/* Aufgabe:
   Setzt Modus des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E long level: Level
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_level(INVERSE_ITERATION *ii, long level)
{
  if (ii) ii->level=level;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_level
                                                                          */
/* Aufgabe:
   Liefert Modus des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - Modus -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ii_level(INVERSE_ITERATION *ii)
{
        return (ii)?ii->level:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_var_eps
                                                                          */
/* Aufgabe:
   Setzt die variable Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double var_eps: Variable Genauigkeit oder variable Abbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_var_eps(INVERSE_ITERATION *ii, double var_eps)
{
  if (ii) ii->var_eps=var_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_var_eps
                                                                          */
/* Aufgabe:
   Liefert die variable Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_var_eps(INVERSE_ITERATION *ii)
{
  return (ii)?ii->var_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_begin_eps
                                                                          */
/* Aufgabe:
   Setzt Anfangsgenauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double begin_eps: Anfangsgenauigkeit oder Anfangsabbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_begin_eps(INVERSE_ITERATION *ii, double begin_eps)
{
  if (ii) ii->begin_eps=begin_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_begin_eps
                                                                          */
/* Aufgabe:
   Liefert Anfangsgenauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_begin_eps(INVERSE_ITERATION *ii)
{
  return (ii)?ii->begin_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_end_eps
                                                                          */
/* Aufgabe:
   Setzt Endgenauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double end_eps: Endgenauigkeit oder Endabbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_end_eps(INVERSE_ITERATION *ii, double end_eps)
{
  if (ii) ii->end_eps=end_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_end_eps
                                                                          */
/* Aufgabe:
   Liefert Endgenauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_end_eps(INVERSE_ITERATION *ii)
{
  return (ii)?ii->end_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_fac_eps
                                                                          */
/* Aufgabe:
   Setzt Faktor der variablen Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double fac_eps: Faktor der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_fac_eps(INVERSE_ITERATION *ii, double fac_eps)
{
  if (ii) ii->fac_eps=fac_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_fac_eps
                                                                          */
/* Aufgabe:
   Liefert Faktor der variablen Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_fac_eps(INVERSE_ITERATION *ii)
{
  return (ii)?ii->fac_eps:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ii_exp_eps
                                                                          */
/* Aufgabe:
   Setzt Exponenten der variablen Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
   E double exp_eps: Exponent der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ii_exp_eps(INVERSE_ITERATION *ii, double exp_eps)
{
  if (ii) ii->exp_eps=exp_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ii_exp_eps
                                                                          */
/* Aufgabe:
   Liefert Exponenten der variablen Genauigkeit des II's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf eine Instanz vom Typ
                                        INVERSE_ITERATION.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ii_exp_eps(INVERSE_ITERATION *ii)
{
  return (ii)?ii->exp_eps:0.0;
}


/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_iteration_list
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
LIST_INVERSE_ITERATION *create_inverse_iteration_list (char *name)
{
  LIST_INVERSE_ITERATION *list_of_inverse_iteration;

  list_of_inverse_iteration = (LIST_INVERSE_ITERATION *) Malloc(sizeof(LIST_INVERSE_ITERATION));
  if ( list_of_inverse_iteration == NULL ) return NULL;


  list_of_inverse_iteration->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_inverse_iteration->name == NULL ) {
         Free(list_of_inverse_iteration);
         return NULL;
  }
  strcpy(list_of_inverse_iteration->name,name);

  list_of_inverse_iteration->ii_list=create_list();
  if ( list_of_inverse_iteration->ii_list == NULL ) {
    Free(list_of_inverse_iteration->name);
        Free(list_of_inverse_iteration);
        return NULL;
  }

  list_of_inverse_iteration->names_of_inverse_iteration=NULL;
  list_of_inverse_iteration->count_of_inverse_iteration_name=0;

  return list_of_inverse_iteration;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_iteration_list
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
void destroy_inverse_iteration_list(void)
{
  if (list_of_inverse_iteration->name) list_of_inverse_iteration->name=(char *)Free(list_of_inverse_iteration->name);
  if(list_of_inverse_iteration->names_of_inverse_iteration) \
    list_of_inverse_iteration->names_of_inverse_iteration = \
      (char **)Free(list_of_inverse_iteration->names_of_inverse_iteration);

  if (list_of_inverse_iteration->ii_list) {
    delete_list(list_of_inverse_iteration->ii_list,destroy_inverse_iteration);
    list_of_inverse_iteration->ii_list=destroy_list(list_of_inverse_iteration->ii_list);
  }

  if (list_of_inverse_iteration) list_of_inverse_iteration=(LIST_INVERSE_ITERATION *)Free(list_of_inverse_iteration);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_iteration_list_empty
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
int inverse_iteration_list_empty(void)
{
  if (!list_of_inverse_iteration) return 0;
  else if (!list_of_inverse_iteration->ii_list) return 0;
  else return list_empty(list_of_inverse_iteration->ii_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_ii_init
                                                                          */
/* Aufgabe:
   Liste der II's initialisieren.
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
void list_ii_init(void)
{
  if (list_of_inverse_iteration) list_current_init(list_of_inverse_iteration->ii_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_ii_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_ii_next (void)
{
  return list_of_inverse_iteration->ii_list ?
             get_list_next(list_of_inverse_iteration->ii_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_inverse_iteration_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ INVERSE_ITERATION erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen II's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_inverse_iteration_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_inverse_iteration) return 0;
  for ( i=0; i<list_of_inverse_iteration->count_of_inverse_iteration_name; i++)
    if(strcmp(list_of_inverse_iteration->names_of_inverse_iteration[i],name) == 0) break;
  if (i < list_of_inverse_iteration->count_of_inverse_iteration_name) return i;

  list_of_inverse_iteration->names_of_inverse_iteration= \
          (char **) Realloc(list_of_inverse_iteration->names_of_inverse_iteration, \
          (list_of_inverse_iteration->count_of_inverse_iteration_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_inverse_iteration->names_of_inverse_iteration[i]=new_name;

  return ++(list_of_inverse_iteration->count_of_inverse_iteration_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_inverse_iteration_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen II's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_inverse_iteration_names (void)
{
  long i;

  for (i=0; i<list_of_inverse_iteration->count_of_inverse_iteration_name; i++)
    if(list_of_inverse_iteration->names_of_inverse_iteration[i]) \
      list_of_inverse_iteration->names_of_inverse_iteration[i] = \
        (char *)Free(list_of_inverse_iteration->names_of_inverse_iteration[i]);

  return list_of_inverse_iteration->count_of_inverse_iteration_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_inverse_iteration_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ INVERSE_ITERATION in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_inverse_iteration_list (INVERSE_ITERATION *ii)
{
  if(!list_of_inverse_iteration) return 0;
  list_of_inverse_iteration->count_of_inverse_iteration=append_list(list_of_inverse_iteration->ii_list,(void *) ii);
  return list_of_inverse_iteration->count_of_inverse_iteration;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_inverse_iteration_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ INVERSE_ITERATION der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender INVERSE_ITERATION-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001    OK      Erste Version
                                                                          */
/**************************************************************************/
long remove_inverse_iteration_list (INVERSE_ITERATION *ii)
{
  list_of_inverse_iteration->count_of_inverse_iteration = \
      remove_list_member(list_of_inverse_iteration->ii_list,(void *) ii, destroy_inverse_iteration);
  return list_of_inverse_iteration->count_of_inverse_iteration;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_iteration_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long inverse_iteration_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_inverse_iteration->count_of_inverse_iteration_name; i++)
    if(strcmp(list_of_inverse_iteration->names_of_inverse_iteration[i],name) == 0) break;
  if (i < list_of_inverse_iteration->count_of_inverse_iteration_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateInverseIterationList
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
void CreateInverseIterationList(void)
{
  list_of_inverse_iteration = create_inverse_iteration_list("INVERSE_ITERATION");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseIterationList
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
void DestroyInverseIterationList(void)
{
  long i;

  //OK_BC if (ii_matrix_values) DestroyDoubleMatrix(ii_matrix_values);
  //OK_BC if (ii_matrix_iter) DestroyLongMatrix(ii_matrix_iter);

  /* ah zv */
  for(i=0; i<ii_names_number; i++)
    if (ii_names[i]) Free(ii_names[i]);
  if (ii_names) Free(ii_names);

  undeclare_inverse_iteration_names();
  destroy_inverse_iteration_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InverseIterationListEmpty
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
int InverseIterationListEmpty(void)
{
  return inverse_iteration_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetInverseIteration
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetInverseIteration (char *name)
{
  char *new_name;

  ii_names=(char **) Realloc(ii_names,(ii_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  ii_names[ii_names_number]=new_name;
  ii_names[++ii_names_number]=NULL;

  return;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: IndexInverseIteration
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des II's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long IndexInverseIteration(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<ii_names_number; i++)
    if(strcmp(ii_names[i],name) == 0) break;
  if (i < ii_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetInverseIterationGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INVERSE_ITERATION
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INVERSE_ITERATION *ii -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INVERSE_ITERATION
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_ITERATION *GetInverseIterationGroup(char *name,INVERSE_ITERATION *ii)
{
  if (!name) return NULL;
  /*if (!inverse_iteration_name_exist(name) ) return NULL;*/

  if(ii==NULL) list_ii_init();

  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name))==0 ) break;
  }

  return ii;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseIterationListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche INVERSE_ITERATION-Objekte der Gruppe NAME
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
int DestroyInverseIterationListGroup(char *name)
{
  INVERSE_ITERATION *ii=NULL;

  if (!name) return -1;
  /*if (!inverse_iteration_name_exist(name) ) return -1;*/

  list_ii_init();

  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name))==0 )
        remove_inverse_iteration_list(ii);
  }

  return 0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIteration
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
int ExecuteInverseIteration (char *name, double *current, double *last,
                                double *ref, long dim)
{
  INVERSE_ITERATION *ii=NULL;
  int ret=0,found=0;
  double cur_eps=0.0,last_eps=0.0;
  long col;
  long cur_iter,max_iter;
  double base=10.0;

  if (!name) { return 0; }
  if (InverseIterationListEmpty()) { return 0; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return ret;

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
    found=0;
    switch (get_ii_type(ii)) {
          case 0:
            found=1;
            ret=!ExecuteInverseIterationMethod0(ii,current,last,dim,&cur_eps);
            break;
          case 1:
            found=1;
            ret=!ExecuteInverseIterationMethod1(ii,current,last,ref,dim,&cur_eps);
            break;
          case 2:
            found=1;
            if(ii_matrix_iter->m[0][col] == 0) set_ii_var_eps(ii,get_ii_begin_eps(ii));
            else {
              set_ii_var_eps(ii,get_ii_var_eps(ii)*get_ii_fac_eps(ii)*pow(base,get_ii_exp_eps(ii)));
            }
            ret=!ExecuteInverseIterationMethod2(ii,current,last,ref,dim,&cur_eps);
            break;
        } /* switch */
        if (found) {
          ii_matrix_iter->m[0][col]++;
          cur_iter=ii_matrix_iter->m[0][col];
          last_eps=ii_matrix_values->m[0][col];
          ii_matrix_values->m[0][col]=cur_eps;
          if (fabs(last_eps)<MKleinsteZahl) ii_matrix_values->m[1][col]=cur_eps;
          else ii_matrix_values->m[1][col]=cur_eps/last_eps;
          max_iter=get_ii_maxiter(ii);
          if (cur_iter>max_iter && get_ii_kind(ii) == 0) {
            DisplayErrorMsg("Maximale Anzahl an Modell-Iterationen erreicht.");
            ret=0;
          }
          return ret;            
        }          
  } /* while */
  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetIterationNumberInverseIteration
                                                                          */
/* Aufgabe:
   Liefert Anzahl der ausgefuehrten Iterationen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Identifikatorname des Prozesses.
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                      Funktion noch nicht abgeschlossen (max_num_iter)
                                                                          */
/**************************************************************************/
long GetIterationNumberInverseIteration (char *name)
{
  INVERSE_ITERATION *ii=NULL;
  int found=0;
  long col;
  long max_num_iter=0,num_iter=0;

  if (!name) { return 0; }
  if (InverseIterationListEmpty()) { return 0; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return found;

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
    found=0;
    switch (get_ii_type(ii)) {
          case 0:
          case 1:
          case 2:
            found=1;
            break;
        } /* switch */
        if (found) {
          num_iter=ii_matrix_iter->m[0][col];
          max_num_iter=max(num_iter,max_num_iter);
        }          
  } /* while */
  return max_num_iter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetConvergenceRateInverseIteration
                                                                          */
/* Aufgabe:
   Liefert die Konvergenzrate der ausgefuehrten Iteration.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Identifikatorname des Prozesses.
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version
                         Funktion noch nicht abgeschlossen (max_cur_eps)
                                                                          */
/**************************************************************************/
double GetConvergenceRateInverseIteration (char *name)
{
  INVERSE_ITERATION *ii=NULL;
  int found=0;
  double max_cur_eps=0.0,cur_eps=0.0;
  long col;

  if (!name) { return 0.; }
  if (InverseIterationListEmpty()) { return 0.; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0.; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return 0.0;

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
    switch (get_ii_type(ii)) {
          case 0:
          case 1:
          case 2:
             cur_eps=ii_matrix_values->m[0][col];
             max_cur_eps=max(cur_eps,max_cur_eps);
             found=1;
             break;
    } /* switch */
  } /* while */
  if (found) return max_cur_eps;
  else return -1.;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitInverseIteration
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
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
void InitInverseIteration (char *name)
{
  long i,n=2;
  long col=IndexInverseIteration(name);

  if (col<0) return;

  for (i=0; i<n; i++) {
    ii_matrix_values->m[i][col]=0.0;
    ii_matrix_iter->m[i][col]=0;
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllInverseIteration
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
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
void InitAllInverseIteration (void)
{
  long i,j,n;
  long ncol=ii_names_number;

  if (ncol<=0) return;

  n=2;
  //OK_BC if (ii_matrix_values) DestroyDoubleMatrix(ii_matrix_values);
  //OK_BC ii_matrix_values=CreateDoubleMatrix(n,ncol);
  //OK_BC if (ii_matrix_iter) DestroyLongMatrix(ii_matrix_iter);
  //OK_BC ii_matrix_iter=CreateLongMatrix(n,ncol);
  for (i=0; i<n; i++)
    for (j=0; j<ncol; j++) {
      ii_matrix_values->m[i][j]=0.0;
      ii_matrix_iter->m[i][j]=0;
    }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIterationMethod0
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int ExecuteInverseIterationMethod0 (INVERSE_ITERATION *ii, double *current, double *last,
                                       long dim, double *cur_eps)
{
  double eps=get_ii_abs_eps(ii);
  double error=MVekDist(current,last,dim);

  *cur_eps=error;
  if (*cur_eps <= eps) return 1;
  else return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIterationMethod1
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int ExecuteInverseIterationMethod1 (INVERSE_ITERATION *ii, 
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps)
{
  double eps=get_ii_rel_eps(ii);
  double error=MVekDist(current,last,dim);
  double d=MBtrgVec(ref,dim);

  if (fabs(d )<MKleinsteZahl) *cur_eps=error;
  else *cur_eps=error/d;
  if (*cur_eps <= eps) return 1;
  else return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIterationMethod2
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_ITERATION *ii: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version  Noch zu ergaenzen !!!
                                                                          */
/**************************************************************************/
int ExecuteInverseIterationMethod2 (INVERSE_ITERATION *ii, 
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps)
{
  double eps=get_ii_var_eps(ii);
  double error=MVekDist(current,last,dim);
  double d=MBtrgVec(ref,dim);

  if (fabs(d)< MKleinsteZahl) *cur_eps=error;
  else *cur_eps=error/d;
  if (*cur_eps <= eps) return 1;
  else return 0;
}


/*------------------------------------------------------------------------*/
/* Lesefunktionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctInverseIteration
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #INVERSE_ITERATION gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.
   #INVERSE_ITERATION: Iterations-Parameter fuer alle Modelle.
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
int FctInverseIteration(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    double d;
    long l;
    int i=0,k=0;
    INVERSE_ITERATION *ii;
    char *name;
    int dec=1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #INVERSE_ITERATION");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {           /* INVERSE_ITERATION nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f, "; Schluesselwort: #");
      FilePrintString(f, keyword_inverse_iteration);
      LineFeed(f);
#endif
      if ( (ii=create_inverse_iteration(DEFAULT_GROUP_NAME)) == NULL ) {
        ok = 0;
      }
      else {
        set_ii_name(ii,DEFAULT_NAME);      /* ID */
        set_ii_type(ii,0);      /* Standard-Iteration */
        set_ii_maxiter(ii,1000);/* Max. Anzahl an Iterationen */
        set_ii_criterium(ii,2); /* Kriterium */
        set_ii_norm(ii,k);
        set_ii_abs_eps(ii,1.e-6);      /* abs. Fehlerschranke */
        set_ii_rel_eps(ii,1.e-3);      /* rel. Fehlerschranke */
        set_ii_rel_cg_eps(ii,1.e-3);   /* var. Fehlerschranke */
        set_ii_time(ii,-1.);           /* Gueltigkeitszeit */
        set_ii_kind(ii,0);      /* Immer gueltig */
        /* II's-Name deklarieren */
        declare_inverse_iteration_name(DEFAULT_GROUP_NAME);
        /* II-Objekt in der Liste einfuegen */
        insert_inverse_iteration_list(ii);
      }
    } /* found */
    else {                /* ITERATION_PRESSURE gefunden */
      FilePrintString(f, "#");
      FilePrintString(f, keyword_inverse_iteration);
      LineFeed(f);
#ifdef EXT_RFD
      FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein");
      LineFeed(f);
#endif
        while (StrTestHash(&data[p], &pos)) {
          i++;
          LineFeed(f);
          while (StrTestLong(&data[p+=pos])) {
            if ( (ii=create_inverse_iteration("NONE")) == NULL ) {
              ok = 0;
              break;
            }
            else {
              set_ii_group_name(ii,DEFAULT_GROUP_NAME);                            
              ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
              set_ii_type(ii,l);
            }

            if ( get_ii_type(ii) == 0 ) {
                StrReadString (&name,&data[p+=pos],f,TFString,&pos);
                set_ii_name(ii,name);                            
                name=(char *)Free(name);
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ii_norm(ii,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_maxiter(ii,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_abs_eps(ii,d);      /* abs. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_kind(ii,l);         /* Immer gueltig */
                LineFeed(f);
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, abs error tolerance, time control"); 
        LineFeed(f);
#endif
        }

        if ( get_ii_type(ii) == 1 ) {
                StrReadString (&name,&data[p+=pos],f,TFString,&pos);
                set_ii_name(ii,name);                            
                name=(char *)Free(name);
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ii_norm(ii,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_maxiter(ii,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_rel_eps(ii,d);      /* rel. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_kind(ii,l);         /* Immer gueltig */
                LineFeed(f);
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, rel error tolerance, time control"); 
        LineFeed(f);
#endif
        }

        if ( get_ii_type(ii) == 2 ) {
                StrReadString (&name,&data[p+=pos],f,TFString,&pos);
                set_ii_name(ii,name);                            
                name=(char *)Free(name);
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ii_norm(ii,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_maxiter(ii,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_begin_eps(ii,d);    /* Anfangsfehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_end_eps(ii,d);      /* Endfehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_fac_eps(ii,d);      /* Faktor fuer die var. Fehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ii_exp_eps(ii,d);      /* Exponenet fuer die var. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ii_kind(ii,l);         /* Immer gueltig */
                LineFeed(f);
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, eps start, eps final, eps fac, eps exp, time control"); 
        LineFeed(f);
#endif
        }

            if (ii) {
              if (dec) declare_inverse_iteration_name(DEFAULT_GROUP_NAME);
              else dec=0;
              insert_inverse_iteration_list(ii);
            }
            LineFeed(f);
          } /* while long */
        }  /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Inverse-Iterationen (INVERSE_ITERATION) [0..2;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: Standard; 1: ; 2:");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen: [>=0;..]");
            LineFeed(f);
#endif
   }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIteration
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
int ExecuteInverseMethod (char *name, double *current, double *last,
                          double *ref, long dim)
{
  INVERSE_ITERATION *ii=NULL;
  int ret=0,found=0;
  double cur_eps=0.0,last_eps=0.0;
  long col;
  long cur_iter,max_iter;
  double base=10.0;

  if (!name) { return 0; }
  if (InverseIterationListEmpty()) { return 0; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return ret;

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
    found=0;
    switch (get_ii_type(ii)) {
          case 0:
            found=1;
            ret=!ExecuteInverseIterationMethod0(ii,current,last,dim,&cur_eps);
            break;
          case 1:
            found=1;
            ret=!ExecuteInverseIterationMethod1(ii,current,last,ref,dim,&cur_eps);
            break;
          case 2:
            found=1;
            if(ii_matrix_iter->m[0][col] == 0) set_ii_var_eps(ii,get_ii_begin_eps(ii));
            else {
              set_ii_var_eps(ii,get_ii_var_eps(ii)*get_ii_fac_eps(ii)*pow(base,get_ii_exp_eps(ii)));
            }
            ret=!ExecuteInverseIterationMethod2(ii,current,last,ref,dim,&cur_eps);
            break;
        } /* switch */
        if (found) {
          ii_matrix_iter->m[0][col]++;
          cur_iter=ii_matrix_iter->m[0][col];
          last_eps=ii_matrix_values->m[0][col];
          ii_matrix_values->m[0][col]=cur_eps;
          if (fabs(last_eps)< MKleinsteZahl) ii_matrix_values->m[1][col]=cur_eps;
          else ii_matrix_values->m[1][col]=cur_eps/last_eps;
          max_iter=get_ii_maxiter(ii);
          if (cur_iter>max_iter && get_ii_kind(ii) == 0) {
            DisplayErrorMsg("Maximale Anzahl an Modell-Iterationen erreicht.");
            ret=0;
          }
          return ret;            
        }          
  } /* while */
  return ret;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteAllInverseIteration
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
int ExecuteAllInverseIteration (char *db_name, double *lse)
{
  int i,ret=0;
  char *name=NULL;
  char *iter_name;
  double *od_values,*coeffs_values,*mod_values;
  long number_of_values;
  FILE *f=DB_GetProtPointerDataBase(db_name);
  long j,n;
  double w2,edw2=0.0;
  int test=0;

  n=GetObservationDataNamesNumber();
  *lse=0.0;
  for (i=0; i<n; i++) {
    name=GetObservationDataName(i);
    /*WriteBothValuesObservationData(name,f);*/

    od_values=GetValuesObservationData(name,&number_of_values);
    mod_values=GetModelValuesObservationData(name,&number_of_values);
    coeffs_values=GetCoeffsObservationData(name,&number_of_values);

    for (j=0; j<number_of_values; j++) {
      w2=coeffs_values[j]*coeffs_values[j];
      edw2+=1./w2;
      /*(*lse)+=(od_values[j]-mod_values[j])*(od_values[j]-mod_values[j])/w2;*/
      (*lse)+=(od_values[j]-mod_values[j])*(od_values[j]-mod_values[j]);
    }
    /**lse/=edw2;*/

    /*InitInverseIteration(name);*/
    iter_name=GetIterationNameObservationData(name);
    ret=ExecuteInverseMethod(iter_name,od_values,mod_values,od_values,number_of_values);
    PrintInverseIteration(iter_name,name);
  }
  (*lse)=sqrt(*lse);
  if (test) {
    fprintf(f," Least square error = %e ",*lse);
    fprintf(f," %e ",*lse);
    LineFeed(f);
    fflush(f);
  }
  SaveLeastSquareErrorObservationData(name,*lse);


  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: PrintInverseIteration
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
int PrintInverseIteration (char *name, char *text)
{
  int ret=1;
  long col;
  static long cur_iter=0;
  static double cur_eps=0.0;

  if (!name) { return ret; }
  if (InverseIterationListEmpty()) { return ret; }
  /*if ( !inverse_iteration_name_exist(name) ) { return ret; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return ret;

  DisplayMsgLn("");
  cur_iter=0;
  cur_eps=0.0;
  cur_iter=GetIterationNumberInverseIteration(name);
  cur_eps=GetConvergenceRateInverseIteration(name);
  DisplayMsg(text);
  DisplayMsg(":  Iteration-Nr. = ");
  DisplayLong((long) cur_iter);
  DisplayMsg("   Convergence rate = ");
  DisplayDouble((double) cur_eps, 0, 0);
  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetPocessNameInverseIteration
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
char *GetPocessNameInverseIteration(char *name)
{
  INVERSE_ITERATION *ii=NULL;
  long col;

  if (!name) { return NULL; }
  if (InverseIterationListEmpty()) { return NULL; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0; }*/

  col=IndexInverseIteration(name);
  if (col < 0) return NULL;

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
    return get_ii_process_name(ii);
  } /* while */

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseIteration
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
int ExecuteInverseIterationNew (char *iv_name, char *name, char *db_name, double *lse)
{
  INVERSE_ITERATION *ii=NULL;
  int ret=0;
  double *od_values,*coeffs_values,*mod_values;
  long number_of_values;
  long j;
  double w2,edw2;
  /*int i; */
  /*long col;*/
  FILE *f=DB_GetProtPointerDataBase(db_name);
  double iv_value;

  if (!f)  return 0;

  if (!name) { return 0; }
  if (InverseIterationListEmpty()) { return 0; }
  /*if ( !inverse_iteration_name_exist(name) ) { return 0; }*/

  /*col=IndexInverseIteration(name);
  if (col < 0) return ret;*/

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_name(ii)),StrUp(name)) ) continue;
      /*name=GetObservationDataName(i);*/
      WriteBothValuesObservationData(name,f);
      if ( !GetStartInverseVariable(iv_name,&iv_value) ) continue;
      fprintf(f," Modified parameter: %s = %e",iv_name,iv_value);
      LineFeed(f);   

      od_values=GetValuesObservationData(name,&number_of_values);
      coeffs_values=GetCoeffsObservationData(name,&number_of_values);
      mod_values=GetModelValuesObservationData(name,&number_of_values);

      *lse=0.0; edw2=0.0;
      for (j=0; j<number_of_values; j++) {
        w2=coeffs_values[j]*coeffs_values[j];
        edw2+=1./w2;
        *lse+=(od_values[j]-mod_values[j])*(od_values[j]-mod_values[j])/w2;
      }
      *lse/=edw2;
      /**lse=sqrt(*lse);*/
      /*fprintf(f," Least square error = %e ",(*lse));*/
      fprintf(f," %e ",(*lse));
      /*LineFeed(f);   */
      fflush(f);

      InitInverseIteration(name);
      ret=ExecuteInverseMethod(name,od_values,mod_values,od_values,number_of_values);
      PrintInverseIteration(name,GetPocessNameInverseIteration(name));
      return ret;            
  } /* while */
  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigInverseIteration
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
int ConfigInverseIteration (char *name)
{
  INVERSE_ITERATION *ii=NULL;

  if (!name) { return 0; }
  if (InverseIterationListEmpty()) { return 0; }
  if ( !inverse_iteration_name_exist(name) ) { return 0; }

  list_ii_init();
  while ( (ii=(INVERSE_ITERATION *)get_list_ii_next()) != NULL ) {
    if ( strcmp(StrUp(get_ii_group_name(ii)),StrUp(name)) ) continue;
    SetInverseIteration(get_ii_name(ii));
  } /* while */

  return 1;

}

