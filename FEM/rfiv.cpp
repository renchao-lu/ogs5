/**************************************************************************/
/* ROCKFLOW - Modul: rfiv.c
                                                                          */
/* Aufgabe: Verwaltung und Bearbeitung der inversen Variablen
   - Datenstrukturen und Definitionen fuer den Datentyp INVERSE_VARIABLE
   - Verwaltung von INVERSE_VARIABLE mit dem Datentyp LIST_INVERSE_VARIABLE
   - Bereitstellung von Methoden fuer den Datentyp INVERSE_VARIABLE
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
#include "geo_strings.h"

/* Used objects */
#include "rfdb.h"
#include "rf_tim_new.h"

/* Interface */
#include "rfiv.h"


/* Definitionen */

  /* Liste von inverse Variablen */
static LIST_INVERSE_VARIABLE *list_of_inverse_variable = NULL;

  /* Schluesselwoerter */
char *keyword_inverse_variable                         = "INVERSE_VARIABLE";

  /* Gruppen-Namen der IV's */
char *group_name_inverse_variable                      = "ROCKFLOW";


/* Interne Lesefunktionen */
int FctInverseVariable(char *data,FILE *f,char *iv_name,int index,char *string,int found);



/* Nur fuer den internen Zugriff */
static char **iv_names = NULL;
static long iv_names_number = 0;

static char **iv_group_names = NULL;
static long iv_group_names_number = 0;

static void **iv_items = NULL;
static long iv_items_number = 0;

static int  *iv_types = NULL;
static long iv_types_number = 0;


/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_variable
                                                                          */
/* Aufgabe:
   Konstruktor fuer INVERSE_VARIABLE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - Adresse des IV's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_VARIABLE *create_inverse_variable(char *name)
{
  INVERSE_VARIABLE *iv;

  iv = (INVERSE_VARIABLE *) Malloc(sizeof(INVERSE_VARIABLE));
  if ( iv == NULL ) return NULL;


  iv->name = (char *) Malloc((int)strlen(name)+1);
  if ( iv->name == NULL ) {
         Free(iv);
         return NULL;
  }
  strcpy(iv->name,name);

  iv->group_name=NULL;

  iv->type=0;

  iv->count_of_values=0;
  iv->values=NULL;

  return iv;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_variable
                                                                          */
/* Aufgabe:
   Destruktor fuer INVERSE_VARIABLE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des IV's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_inverse_variable(void *member)
{
  INVERSE_VARIABLE *iv=(INVERSE_VARIABLE *)member;

  if ( iv->name ) iv->name=(char *)Free(iv->name);
  if ( iv->group_name ) iv->group_name=(char *)Free(iv->group_name);
  if ( iv->values ) iv->values=(double *)Free(iv->values);

  if ( iv ) iv=(INVERSE_VARIABLE *)Free(iv);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_inverse_variable
                                                                          */
/* Aufgabe:
   Initialisator fuer INVERSE_VARIABLE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - Adresse des IV's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_VARIABLE *init_inverse_variable(INVERSE_VARIABLE *iv, \
                                                                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( iv->values ) iv->values=(double *)Free(iv->values);
  iv->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( iv->values == NULL ) return NULL;

  iv->count_of_values=count_of_values;
  return iv;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_inverse_variable
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Adresse des IV's.
                                                                          */
/* Ergebnis:
   - Adresse des IV's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_VARIABLE *delete_inverse_variable(INVERSE_VARIABLE *iv)
{
  if ( iv->values ) iv->values=(double *)Free(iv->values);
  iv->count_of_values=0;

  return iv;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_name
                                                                          */
/* Aufgabe:
   Liefert Name des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Name des IV's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_iv_name(INVERSE_VARIABLE *iv)
{
  if (iv->name)
    return iv->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_name
                                                                          */
/* Aufgabe:
   Setzt Name des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_name(INVERSE_VARIABLE *iv, char *name)
{
  if (!name) return;
  if(iv) {
    iv->name=(char *)Free(iv->name);
    iv->name = (char *) Malloc((int)strlen(name)+1);
    if ( iv->name == NULL ) return;
    strcpy(iv->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_group_name
                                                                          */
/* Aufgabe:
   Liefert Name des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Name des IV's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_iv_group_name(INVERSE_VARIABLE *iv)
{
  if (iv->group_name)
    return iv->group_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_group_name
                                                                          */
/* Aufgabe:
   Setzt Name des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E char *group_name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_group_name(INVERSE_VARIABLE *iv, char *group_name)
{
  if (!group_name) return;
  if(iv) {
    iv->group_name=(char *)Free(iv->group_name);
    iv->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( iv->group_name == NULL ) return;
    strcpy(iv->group_name,group_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_type
                                                                          */
/* Aufgabe:
   Setzt Typ des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_type(INVERSE_VARIABLE *iv, long type)
{
  if (iv) iv->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_type
                                                                          */
/* Aufgabe:
   Liefert Typ des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_iv_type(INVERSE_VARIABLE *iv)
{
        return (iv)?iv->type:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_ptr
                                                                          */
/* Aufgabe:
   Setzt Element-Zeiger des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E char *name: Dateizeiger des IV's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int set_iv_ptr(INVERSE_VARIABLE *iv, void *ptr)
{
  if(iv && ptr) {
    iv->ptr = ptr;
    return 1;
  }
  else return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_ptr
                                                                          */
/* Aufgabe:
   Liefert Element-Zeiger des IV's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - Dateizeiger des IV's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_iv_ptr(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->ptr:NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_start_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E double start_value: Anfangswert der Inverse-Variable

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_iv_start_value(INVERSE_VARIABLE *iv, double start_value)
{
  if(iv) iv->start_value=start_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_start_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_iv_start_value(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->start_value:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_min_value
                                                                          */
/* Aufgabe:
   Setzt Minimum-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E double min_value: Minimum-Wert der Inverse-Variable

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_iv_min_value(INVERSE_VARIABLE *iv, double min_value)
{
  if(iv) iv->min_value=min_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_min_value
                                                                          */
/* Aufgabe:
   Liefert Minimum-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_iv_min_value(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->min_value:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_max_value
                                                                          */
/* Aufgabe:
   Setzt Maximum-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E double max_value: Maximum-Wert der Inverse-Variable

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_iv_max_value(INVERSE_VARIABLE *iv, double max_value)
{
  if(iv) iv->max_value=max_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_max_value
                                                                          */
/* Aufgabe:
   Liefert Maximum-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_iv_max_value(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->max_value:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_coeff_value
                                                                          */
/* Aufgabe:
   Setzt Koeffizienten-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E double coeff_value: Koeffizienten-Wert der Inverse-Variable
                         (Gewichtsfaktor)

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_iv_coeff_value(INVERSE_VARIABLE *iv, double coeff_value)
{
  if(iv) iv->coeff_value=coeff_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_coeff_value
                                                                          */
/* Aufgabe:
   Liefert Koeffizienten-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_iv_coeff_value(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->coeff_value:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_eps_value
                                                                          */
/* Aufgabe:
   Setzt Koeffizienten-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
   E double eps_value: Abbruchschranke-Wert der Inverse-Variable

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_iv_eps_value(INVERSE_VARIABLE *iv, double eps_value)
{
  if(iv) iv->eps_value=eps_value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_eps_value
                                                                          */
/* Aufgabe:
   Liefert Abbruchschranke-Wert des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf eine Instanz vom Typ INVERSE_VARIABLE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_iv_eps_value(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->eps_value:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_max_iter
                                                                          */
/* Aufgabe:
   Setzt maximale Iteration des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E long max_iter: maximale Iteration                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_max_iter(INVERSE_VARIABLE *iv, long max_iter)
{
  if (iv) iv->max_iter=max_iter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_max_iter
                                                                          */
/* Aufgabe:
   Liefert maximale Iteration des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_iv_max_iter(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->max_iter:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_kind
                                                                          */
/* Aufgabe:
   Setzt Art des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E int kind: Art der inversen Variable
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_kind(INVERSE_VARIABLE *iv, int kind)
{
  if (iv) iv->kind=kind;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_kind
                                                                          */
/* Aufgabe:
   Liefert Art des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_iv_kind(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->kind:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_curve
                                                                          */
/* Aufgabe:
   Setzt Kurve des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E int curve: Kurve                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_curve(INVERSE_VARIABLE *iv, int curve)
{
  if (iv) iv->curve=curve;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_curve
                                                                          */
/* Aufgabe:
   Liefert Kurve des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_iv_curve(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->curve:-1;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des IV's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_iv_count_of_values(INVERSE_VARIABLE *iv)
{
  return (iv)?iv->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_count_of_values(INVERSE_VARIABLE *iv, long count_of_values)
{
  if (iv) iv->count_of_values=count_of_values;
  return;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: set_iv_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_iv_value(INVERSE_VARIABLE *iv, long index, double value)
{
  if (iv && iv->values) {
    if ( (index<iv->count_of_values) && (index>=0) )
          iv->values[index]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (IV) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_iv_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IV's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_iv_value(INVERSE_VARIABLE *iv, long index)
{
  if (iv && iv->values) {
    if ( (index<iv->count_of_values) && (index>=0) )
          return iv->values[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (IV) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (IV) (Returnwert -1)!");
        return -1.;
  }
}





/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_variable_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von inversen Variablen;
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
LIST_INVERSE_VARIABLE *create_inverse_variable_list (char *name)
{
  LIST_INVERSE_VARIABLE *list_of_inverse_variable;

  list_of_inverse_variable = (LIST_INVERSE_VARIABLE *) Malloc(sizeof(LIST_INVERSE_VARIABLE));
  if ( list_of_inverse_variable == NULL ) return NULL;


  list_of_inverse_variable->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_inverse_variable->name == NULL ) {
         Free(list_of_inverse_variable);
         return NULL;
  }
  strcpy(list_of_inverse_variable->name,name);

  list_of_inverse_variable->iv_list=create_list();
  if ( list_of_inverse_variable->iv_list == NULL ) {
    Free(list_of_inverse_variable->name);
        Free(list_of_inverse_variable);
        return NULL;
  }

  list_of_inverse_variable->names_of_inverse_variable=NULL;
  list_of_inverse_variable->count_of_inverse_variable_name=0;

  return list_of_inverse_variable;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_variable_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der inversen Variablen aus dem Speicher
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
void destroy_inverse_variable_list(void)
{
  if (list_of_inverse_variable->name) list_of_inverse_variable->name=(char *)Free(list_of_inverse_variable->name);
  if(list_of_inverse_variable->names_of_inverse_variable) \
    list_of_inverse_variable->names_of_inverse_variable = \
      (char **)Free(list_of_inverse_variable->names_of_inverse_variable);

  if (list_of_inverse_variable->iv_list) {
    delete_list(list_of_inverse_variable->iv_list,destroy_inverse_variable);
    list_of_inverse_variable->iv_list=destroy_list(list_of_inverse_variable->iv_list);
  }

  if (list_of_inverse_variable) list_of_inverse_variable=(LIST_INVERSE_VARIABLE *)Free(list_of_inverse_variable);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_variable_list_empty
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
int inverse_variable_list_empty(void)
{
  if (!list_of_inverse_variable) return 0;
  else if (!list_of_inverse_variable->iv_list) return 0;
  else return list_empty(list_of_inverse_variable->iv_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_iv_init
                                                                          */
/* Aufgabe:
   Liste der IV's initialisieren.
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
void list_iv_init(void)
{
  if (list_of_inverse_variable) list_current_init(list_of_inverse_variable->iv_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_iv_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur iv.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_iv_next (void)
{
  return list_of_inverse_variable->iv_list ?
             get_list_next(list_of_inverse_variable->iv_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_inverse_variable_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ INVERSE_VARIABLE erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IV's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_inverse_variable_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_inverse_variable) return 0;
  for ( i=0; i<list_of_inverse_variable->count_of_inverse_variable_name; i++)
    if(strcmp(list_of_inverse_variable->names_of_inverse_variable[i],name) == 0) break;
  if (i < list_of_inverse_variable->count_of_inverse_variable_name) return i;

  list_of_inverse_variable->names_of_inverse_variable= \
          (char **) Realloc(list_of_inverse_variable->names_of_inverse_variable, \
          (list_of_inverse_variable->count_of_inverse_variable_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_inverse_variable->names_of_inverse_variable[i]=new_name;

  return ++(list_of_inverse_variable->count_of_inverse_variable_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_inverse_variable_names
                                                                          */
/* Aufgabe:
   Alle inversen Variablen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IV's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_inverse_variable_names (void)
{
  long i;

  for (i=0; i<list_of_inverse_variable->count_of_inverse_variable_name; i++)
    if(list_of_inverse_variable->names_of_inverse_variable[i]) \
      list_of_inverse_variable->names_of_inverse_variable[i] = \
        (char *)Free(list_of_inverse_variable->names_of_inverse_variable[i]);

  return list_of_inverse_variable->count_of_inverse_variable_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_inverse_variable_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ INVERSE_VARIABLE in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_inverse_variable_list (INVERSE_VARIABLE *iv)
{
  if(!list_of_inverse_variable) return 0;
  list_of_inverse_variable->count_of_inverse_variable=append_list(list_of_inverse_variable->iv_list,(void *) iv);
  return list_of_inverse_variable->count_of_inverse_variable;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_inverse_variable_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ INVERSE_VARIABLE der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender INVERSE_VARIABLE-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
long remove_inverse_variable_list (INVERSE_VARIABLE *iv)
{
  list_of_inverse_variable->count_of_inverse_variable = \
      remove_list_member(list_of_inverse_variable->iv_list,(void *) iv, destroy_inverse_variable);
  return list_of_inverse_variable->count_of_inverse_variable;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_variable_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines IV-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long inverse_variable_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_inverse_variable->count_of_inverse_variable_name; i++)
    if(strcmp(list_of_inverse_variable->names_of_inverse_variable[i],name) == 0) break;
  if (i < list_of_inverse_variable->count_of_inverse_variable_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateInverseVariableList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von inversen Variablen (unbenannt);
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
void CreateInverseVariableList(void)
{
  list_of_inverse_variable = create_inverse_variable_list("INVERSE_VARIABLE");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseVariableList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von inversen Variablen (unbenannt);
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
void DestroyInverseVariableList(void)
{
  long i;


  for(i=0; i<iv_group_names_number; i++)
    if (iv_group_names[i]) Free(iv_group_names[i]);
  if (iv_group_names) iv_group_names=(char **)Free(iv_group_names);
  iv_group_names_number=0;

  for(i=0; i<iv_names_number; i++)
    if (iv_names[i]) Free(iv_names[i]);
  if (iv_names) iv_names=(char **)Free(iv_names);
  iv_names_number=0;

  if (iv_items) iv_items=(void **)Free(iv_items);
  iv_items_number=0;

  if (iv_types) iv_types=(int *)Free(iv_types);
  iv_types_number=0;

  undeclare_inverse_variable_names();
  destroy_inverse_variable_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InverseVariableListEmpty
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
int InverseVariableListEmpty(void)
{
  return inverse_variable_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetInverseVariable(char *name, char *group_name,  int type, void *item)
{
  char *new_name;
  char *new_group_name;
  char string[81];

  if (!name || !item) { 

    DisplayMsgLn("Error - Inverse Problem !!!");
    DisplayMsgLn("Error - variable name or reference not defined !!!");
    if (group_name != NULL) {
      sprintf(string,"\n%s%s","group name : ",group_name);
    }
    else {
      sprintf(string,"%s","Error - group name : not defined");
    }
    DisplayMsgLn(string);
    exit(1);
  }

  iv_names=(char **) Realloc(iv_names,(iv_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  iv_names[iv_names_number]=new_name;
  iv_names[++iv_names_number]=NULL;

  iv_group_names=(char **) Realloc(iv_group_names,(iv_group_names_number+2)*sizeof(char **) );
  new_group_name=(char *)Malloc((int)strlen(group_name)+1);
  strcpy(new_group_name,group_name);
  iv_group_names[iv_group_names_number]=new_group_name;
  iv_group_names[++iv_group_names_number]=NULL;

  iv_items=(void **) Realloc(iv_items,(iv_items_number+2)*sizeof(void **) );
  iv_items[iv_items_number]=item;
  iv_items[++iv_items_number]=NULL;

  iv_types=(int *) Realloc(iv_types,(iv_types_number+2)*sizeof(int *) );
  iv_types[iv_types_number]=type;
  iv_types[++iv_types_number]=-1;

}




/**************************************************************************/
/* ROCKFLOW - Funktion: IndexInverseVariable
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines IV-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IV's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long IndexInverseVariable(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<iv_names_number; i++)
    if(strcmp(iv_names[i],name) == 0) break;
  if (i < iv_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetInverseVariableGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INVERSE_VARIABLE
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INVERSE_VARIABLE *iv -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INVERSE_VARIABLE
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_VARIABLE *GetInverseVariableGroup(char *name,INVERSE_VARIABLE *iv)
{
  if (!name) return NULL;
  if (!inverse_variable_name_exist(name) ) return NULL;

  if(iv==NULL) list_iv_init();

  while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
    if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) break;
  }

  return iv;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseVariableListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche INVERSE_VARIABLE-Objekte der Gruppe NAME
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
int DestroyInverseVariableListGroup(char *name)
{
  INVERSE_VARIABLE *iv=NULL;

  if (!name) return -1;
  if (!inverse_variable_name_exist(name) ) return -1;

  list_iv_init();

  while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
    if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 )
        remove_inverse_variable_list(iv);
  }

  return 0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetInverseVariableNamesNumber
                                                                          */
/* Aufgabe:
   Liefert die Anzahl der gesetzten Namen fuer die IV.
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
long GetInverseVariableNamesNumber (void)
{
  return iv_names_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetInverseVariableName
                                                                          */
/* Aufgabe:
   Liefert einen gesetzten Namen fuer die IV.
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
char *GetInverseVariableName (long i)
{
  return (iv_names[i])?iv_names[i]:NULL;
}



/*------------------------------------------------------------------------*/
/* Lesefunktionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctInverseVariableNew
                                                                          */
/* Aufgabe:
   Lesefunktion fuer inverse Variablen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data       : Zeiger auf die aus der Datei eingelesenen Zeichen
   E FILE *f          : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
int FctInverseVariableNew ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char iv_name_this[81];
    INVERSE_VARIABLE *iv = NULL;
    int counter;
    char keyword[80];
    int index=0;
    char *name;

#ifdef TESTFILES
    char protocol[80];
#endif

    counter = 0;


    /* Schleife ueber alle Phasen bzw. Komponenten */
    while (StrTestHash(&data[p],&pos)||(found==2)) {

        /* Gruppen-Name generieren */
        if(index==0)
            sprintf(iv_name_this,"%s%d",group_name_inverse_variable,++counter);
        else if (index>0) /* bestimmte Phasen/Komponenten */
            strcpy(iv_name_this,group_name_inverse_variable);

        if(found==1) {
            /* IV-Gruppen-Name in die Namensliste einfuegen */
            declare_inverse_variable_name(iv_name_this);
        }
        else if (found==2) {
            /* Pruefen, ob IV-Objekt-Gruppe existiert */
            if(!inverse_variable_name_exist(iv_name_this)) break;
        }

        /* Schluesselwort schreiben */
        sprintf(keyword,"%s%s","#",keyword_inverse_variable);
        FilePrintString(f,keyword);
        LineFeed(f);

#ifdef TESTFILES
        sprintf(protocol,"%s%s","Eingabedatenbank: Schluesselwort",keyword);
        DisplayMsgLn(protocol);
#endif

        /* Schleife ueber alle IV-Objekte der Gruppe */
        while (StrTestLong(&data[p+=pos])||(found==2)) {

            if(found==1) {
                iv=create_inverse_variable(iv_name_this);
            }
            else if(found==2) {
                iv=GetInverseVariableGroup(iv_name_this,iv);
            }

            if(iv==NULL) {
                ok = 0;
                break;
            }
            else {
                if(found==1) {
                    ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
                    set_iv_type(iv,l);
                }
                else if (found==2) {
                    fprintf(f," %ld ",get_iv_type(iv));
                }
                if (get_iv_type(iv)==0) {
                    if(found==1) {
                        StrReadString (&name,&data[p+=pos],f,TFString,&pos);
                        set_iv_name(iv,name);
                        name=(char *)Free(name);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_iv_start_value(iv,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_iv_min_value(iv,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_iv_max_value(iv,d);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_iv_coeff_value(iv,d);
                        ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
                        set_iv_max_iter(iv,l);
                        ok = (StrReadDouble(&d,&data[p+=pos],f,TFDouble,&pos) && ok);
                        set_iv_eps_value(iv,d);
                    }
                    else if (found==2) {
                        fprintf(f," %e ",get_iv_start_value(iv));
                        fprintf(f," %e ",get_iv_min_value(iv));
                        fprintf(f," %e ",get_iv_max_value(iv));
                        fprintf(f," %e ",get_iv_coeff_value(iv));
                        fprintf(f," %ld ",get_iv_max_iter(iv));
                        fprintf(f," %e ",get_iv_eps_value(iv));
                    }
                }

                if(found==1) {
                    if (iv) {
                        insert_inverse_variable_list(iv);
                    }
                }
                else if (found==2) {
                }
                LineFeed(f);
            }
        }


#ifdef EXT_RFD
        FilePrintString(f,"; - Typ (type) [>=0;0]"); LineFeed(f);
        LineFeed(f);
#endif
    }
    return ok;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void InitAllInverseVariable(void)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;

  if (iv_names_number == 0) return;

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      set_iv_type(iv,iv_types[i]);
      set_iv_ptr(iv,iv_items[i]);
      set_iv_group_name(iv,iv_group_names[i]);
      i++; 
      continue;
    }
    else i++;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetCurrentInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int SetCurrentInverseVariable(char *name, double current_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  char *iv_name;
  
  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Aktueller-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      *ptr_d=current_value;
      break;
    }
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetCurrentInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetCurrentInverseVariable(char *name, double *current_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Aktueller Wert holen */
      ptr_d=(double *)get_iv_ptr(iv);
      *current_value=*ptr_d;
      break;
    }
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetAllStartInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetAllStartInverseVariable(void)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double start_value;

  if (iv_names_number == 0) return;

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      start_value=get_iv_start_value(iv);
      *ptr_d=start_value;
      i++; 
      continue;
    }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetAllInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetAllInverseVariable(void)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double start_value;

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      start_value=get_iv_start_value(iv);
      *ptr_d=start_value;
      i++; 
      continue;
    }
  }
}




/**************************************************************************/
/* ROCKFLOW - Funktion: SetAllMinInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetAllMinInverseVariable(void)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double min_value;

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      min_value=get_iv_min_value(iv);
      *ptr_d=min_value;
      i++; 
      continue;
    }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetStartInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int SetStartInverseVariable(char *name, double start_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  char *iv_name;
  
  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      set_iv_start_value(iv,start_value);
      *ptr_d=start_value;
      i++; 
      continue;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetStartInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetStartInverseVariable(char *name, double *start_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      *start_value=get_iv_start_value(iv);
      *start_value=*ptr_d;
      i++; 
      continue;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetMinInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int SetMinInverseVariable(char *name, double min_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;
  
  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Min-Wert setzen */
      set_iv_min_value(iv,min_value);
      i++; 
      continue;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMinInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMinInverseVariable(char *name, double *min_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Liefert Min-Wert */
      *min_value=get_iv_min_value(iv);
      i++; 
      break;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetAllMaxInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetAllMaxInverseVariable(void)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double max_value;

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert setzen */
      ptr_d=(double *)get_iv_ptr(iv);
      max_value=get_iv_max_value(iv);
      *ptr_d=max_value;
      i++; 
      continue;
    }
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetMaxInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int SetMaxInverseVariable(char *name, double max_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;
  
  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Max-Wert setzen */
      set_iv_max_value(iv,max_value);
      i++; 
      continue;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetMaxInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMaxInverseVariable(char *name, double *max_value)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Liefert Max-Wert */
      *max_value=get_iv_max_value(iv);
      i++; 
      continue;
    }
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetCoeffInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetCoeffInverseVariable(char *name, double *coeff)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Liefert Max-Wert */
      *coeff=get_iv_coeff_value(iv);
      i++; 
      continue;
    }
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetEpsInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetEpsInverseVariable(char *name, double *eps)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Liefert Max-Wert */
      *eps=get_iv_eps_value(iv);
      i++; 
      continue;
    }
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetMaxiterInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetMaxiterInverseVariable(char *name, long *max_iter)
{
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  char *iv_name;

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Liefert Max-Wert */
      *max_iter=get_iv_max_iter(iv);
      i++; 
      continue;
    }
  }

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintAllStartInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintAllStartInverseVariable(char *db_name)
{
  char *name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double start_value;
  FILE *f=DB_GetProtPointerDataBase(db_name);

  i=0;
  while( (name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert holen */
      ptr_d=(double *)get_iv_ptr(iv);
      /*start_value=get_iv_start_value(iv);*/
      start_value=*ptr_d;
      fprintf(f," %e ",start_value);
      /*fprintf(f," Modified parameter: %s = %e",name,start_value);*/
      /*LineFeed(f);*/
      i++; 
      continue;
    }
  }
  fflush(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintStartInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintStartInverseVariable(char *name, char *db_name)
{
  char *iv_name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  double *ptr_d;
  double start_value;
  FILE *f=DB_GetProtPointerDataBase(db_name);

  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Start-Wert holen */
      ptr_d=(double *)get_iv_ptr(iv);
      /*start_value=get_iv_start_value(iv);*/
      start_value=*ptr_d;
      /*fprintf(f," Modified parameter: %s = %e",name,start_value);*/
      fprintf(f," %e",start_value);
      /*LineFeed(f);*/
      i++; 
      break;
    }
  }
  fflush(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintStartInverseIteration
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintStartInverseIteration(long i, char *db_name)
{
  FILE *f=DB_GetProtPointerDataBase(db_name);

  LineFeed(f);
  fprintf(f," %ld ",i);
  fflush(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintCycleInverseIteration
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintCycleInverseIteration(long i, char *db_name)
{
  FILE *f=DB_GetProtPointerDataBase(db_name);

  fprintf(f," %ld. cycle",i);
  fflush(f);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: PrintNameInverseVariable
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintNameInverseVariable(char *name, char *db_name)
{
  FILE *f=DB_GetProtPointerDataBase(db_name);

  LineFeed(f);
  fprintf(f," %s",name);
  fflush(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintStartInverseLSE
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintStartInverseLSE(double lse, char *db_name)
{
  FILE *f=DB_GetProtPointerDataBase(db_name);

  LineFeed(f);
  fprintf(f," %e ",lse);
  fflush(f);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: PrintStartInverseLF
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintStartInverseLF(char *db_name)
{
  FILE *f=DB_GetProtPointerDataBase(db_name);

  LineFeed(f);
  fflush(f);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: PrintStartInverseText
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_VARIABLE *iv: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void PrintStartInverseText(char *db_name)
{
  char *iv_name;
  int i,found;
  INVERSE_VARIABLE *iv=NULL;
  FILE *f=DB_GetProtPointerDataBase(db_name);

  static char string[1024];
  static char tmp[81];
  long iter=100;

  if (iv_names_number == 0) return;

  sprintf(string,"VARIABLES =");
  strcat(string," \"");
  sprintf(tmp," %s ","Iteration");
  strcat(string,tmp);
  strcat(string,"\"");
  i=0;
  while( (iv_name=GetInverseVariableName(i)) != NULL ) {
    found=0;
    list_iv_init();
    while ( (iv=(INVERSE_VARIABLE *)get_list_iv_next()) != NULL ) {
      if ( strcmp(StrUp(get_iv_name(iv)),StrUp(iv_name))==0 ) {
        found=1;
        break;
      }
    }
 
    if (found) { 
      /* Parametervariable ausgeben */
      strcat(string," \"");
      strcat(string,iv_name);
      strcat(string,"\"");
      /*fprintf(f," %s",iv_name);*/
      /*LineFeed(f);*/
    }
    i++; 
  }
  strcat(string," \"");
  sprintf(tmp," %s ","Least square error");
  strcat(string,tmp);
  strcat(string,"\"");
  fprintf(f," %s",string);
  LineFeed(f);

  sprintf(tmp,"%e",aktuelle_zeit);  
  sprintf(string,"ZONE T=\"TIME=");
  strcat(string,tmp);
  strcat(string," s\"");
  strcat(string," , ");
  sprintf(tmp,"I=%ld, J=1, K=1, F=POINT\n",iter);
  strcat(string,tmp);

  fprintf(f," %s",string);
  fflush(f);
}

