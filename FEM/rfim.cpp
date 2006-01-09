/**************************************************************************/
/* ROCKFLOW - Modul: rfim.c
                                                                          */
/* Aufgabe: Inverse Methoden (Algorithmen)
   - Datenstrukturen und Definitionen fuer den Datentyp INVERSE_METHOD
   - Verwaltung von INVERSE_METHOD mit dem Datentyp LIST_INVERSE_METHOD
   - Bereitstellung von Methoden fuer den Datentyp INVERSE_METHOD
   - Schnittstellenfunktion zu Rockflow-Modelle
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
#include "rfiv.h"   
#include "rfii.h"     
#include "adaptiv.h"


/* Interface */
#include "rfim.h"


/* Definitionen */

  /* Liste der inversen Methoden */
static LIST_INVERSE_METHOD *list_of_inverse_method = NULL;

  /* Schluesselwoerter */
char *keyword_inverse_method                       = "INVERSE_METHOD";


  /* Namen der IM-Gruppen  */
char *group_name_inverse_method                    = "ROCKFLOW";


/* Default-Gruppen-Name */
static char DEFAULT_GROUP_NAME[]="ROCKFLOW";

/* Default-Name fuer alle Instanzen vom Typ ROCKFLOW_MODEL */
static char MASTER_DEFAULT_INVERSE_METHOD[]="MASTER_DEFAULT_INVERSE_METHOD";

/* Default-Name */
static char DEFAULT_NAME[]="DEFAULT";

/* Interne Lesefunktionen */
int FctInverseMethod(char *data, int found, FILE * f);



/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_method
                                                                          */
/* Aufgabe:
   Konstruktor fuer INVERSE_METHOD
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - Adresse des IM's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_METHOD *create_inverse_method(char *name)
{
  INVERSE_METHOD *im;

  im = (INVERSE_METHOD *) Malloc(sizeof(INVERSE_METHOD));
  if ( im == NULL ) return NULL;


  im->name = (char *) Malloc((int)strlen(name)+1);
  if ( im->name == NULL ) {
         Free(im);
         return NULL;
  }
  strcpy(im->name,name);

  im->group_name=NULL;
  im->time_loop_function=NULL;

  im->type=0;
  im->curve=0;

  im->lambda=0.0;
  im->second_maxiter=0;
  im->start_lambda=0.0;
  im->lambda_abs_eps=MKleinsteZahl;
  im->lambda_fac1=0.0;
  im->lambda_fac2=0.0;

  im->count_of_values=0;
  im->values=NULL;

  return im;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_method
                                                                          */
/* Aufgabe:
   Destruktor fuer INVERSE_METHOD
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des IM's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_inverse_method(void *member)
{
  INVERSE_METHOD *im=(INVERSE_METHOD *)member;

  if ( im->name ) im->name=(char *)Free(im->name);
  if ( im->group_name ) im->group_name=(char *)Free(im->group_name);
  if ( im->values ) im->values=(double *)Free(im->values);

  if ( im ) im=(INVERSE_METHOD *)Free(im);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_inverse_method
                                                                          */
/* Aufgabe:
   Initialisator fuer INVERSE_METHOD
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - Adresse des IM's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_METHOD *init_inverse_method(INVERSE_METHOD *im, \
                                                                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( im->values ) im->values=(double *)Free(im->values);
  im->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( im->values == NULL ) return NULL;

  im->count_of_values=count_of_values;
  return im;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_inverse_method
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Adresse des IM's.
                                                                          */
/* Ergebnis:
   - Adresse des IM's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_METHOD *delete_inverse_method(INVERSE_METHOD *im)
{
  if ( im->values ) im->values=(double *)Free(im->values);
  im->count_of_values=0;

  return im;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_name
                                                                          */
/* Aufgabe:
   Liefert Name des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Name des IM's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_im_name(INVERSE_METHOD *im)
{
  if (im->name)
    return im->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_name
                                                                          */
/* Aufgabe:
   Setzt Name des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E char *name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_name(INVERSE_METHOD *im, char *name)
{
  if (!name) return;
  if(im) {
    im->name=(char *)Free(im->name);
    im->name = (char *) Malloc((int)strlen(name)+1);
    if ( im->name == NULL ) return;
    strcpy(im->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_group_name
                                                                          */
/* Aufgabe:
   Liefert Name des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Name des IM's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_im_group_name(INVERSE_METHOD *im)
{
  if (im->group_name)
    return im->group_name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_group_name
                                                                          */
/* Aufgabe:
   Setzt Name des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E char *group_name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_group_name(INVERSE_METHOD *im, char *group_name)
{
  if (!group_name) return;
  if(im) {
    im->group_name=(char *)Free(im->group_name);
    im->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( im->group_name == NULL ) return;
    strcpy(im->group_name,group_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_type
                                                                          */
/* Aufgabe:
   Setzt Typ des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long type: Typ des Verfahrens 

                Line search methods
                type=0: none
                type=1: Bracket method (Two point search method)  
                type=2: Bracket method (Three point search method)  
                type=3: Golden section search method  
                type=4: Fibonacci section search method  
                type=5: Quadratic interpolation method  

                Gradient search methods
                type=6: steepest descent method  
                type=7: Conjugate gradient method  
                        Quasi-Newton methods
                type=8: DFP method (Davidon, Fletcher and Powell) 
                type=9: BFGS method (Broyton, Fletcher, Glodfarb and Shanno) 
                
                Newton methods
                type=10: Gauss-Newton method  
                type=11: Gauss-Newton-Levenberg-Marquardt method  
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_type(INVERSE_METHOD *im, long type)
{
  if (im) im->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_type
                                                                          */
/* Aufgabe:
   Liefert Typ des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_type(INVERSE_METHOD *im)
{
        return (im)?im->type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_time_loop_function
                                                                          */
/* Aufgabe:
   Liefert Zeitfunktion des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Name des IM's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
TimeLoopFunction get_im_time_loop_function(INVERSE_METHOD *im)
{
  if (im->time_loop_function) return im->time_loop_function;
  else return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_time_loop_function
                                                                          */
/* Aufgabe:
   Setzt Zeitfunktion des IM's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E TimeLoopFunction time_loop_function: Zeiger auf die Zeitfunktion
                                          des IM's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_time_loop_function(INVERSE_METHOD *im, TimeLoopFunction time_loop_function)
{
  if(im && time_loop_function) {
    im->time_loop_function=time_loop_function;
  }
  else im->time_loop_function = NULL;

  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_second_maxiter
                                                                          */
/* Aufgabe:
   Setzt (Marquardt) maximale Iterationszahl des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E maxiter: maximale Iteration des nichtlinearen iterativen Loesers.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_im_second_maxiter(INVERSE_METHOD *im, long second_maxiter)
{
  if (im) im->second_maxiter=second_maxiter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_maxiter
                                                                          */
/* Aufgabe:
   Liefert (Marquardt) maximale Iterationszahl des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - maximale Iterationszahl -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long get_im_second_maxiter(INVERSE_METHOD *im)
{
  return (im)?im->second_maxiter:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_lambda
                                                                          */
/* Aufgabe:
   Setzt Lambda-Startwert des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long lambda: Lambda-Startwert
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_im_start_lambda(INVERSE_METHOD *im, double start_lambda)
{
  if (im) im->start_lambda=start_lambda;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_start_lambda
                                                                          */
/* Aufgabe:
   Liefert Lambda-Startwert des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Verteilungslambda -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_im_start_lambda(INVERSE_METHOD *im)
{
        return (im)?im->start_lambda:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_lambda_abs_eps
                                                                          */
/* Aufgabe:
   Setzt (Lambda) absolute Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double lambda_abs_eps: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_im_lambda_abs_eps(INVERSE_METHOD *im, double lambda_abs_eps)
{
  if (im) im->lambda_abs_eps=lambda_abs_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_lambda_abs_eps
                                                                          */
/* Aufgabe:
   Liefert (Lambda) absolute Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_im_lambda_abs_eps(INVERSE_METHOD *im)
{
  return (im)?im->lambda_abs_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_lambda_fac1
                                                                          */
/* Aufgabe:
   Setzt (Lambda)-Faktor 1 des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double lambda_fac1: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_im_lambda_fac1(INVERSE_METHOD *im, double lambda_fac1)
{
  if (im) im->lambda_fac1=lambda_fac1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_lambda_fac1
                                                                          */
/* Aufgabe:
   Liefert (Lambda)-Faktor 1 des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_im_lambda_fac1(INVERSE_METHOD *im)
{
  return (im)?im->lambda_fac1:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_lambda_fac2
                                                                          */
/* Aufgabe:
   Setzt (Lambda)-Faktor 2 des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double lambda_fac2: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void set_im_lambda_fac2(INVERSE_METHOD *im, double lambda_fac2)
{
  if (im) im->lambda_fac2=lambda_fac2;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_lambda_fac2
                                                                          */
/* Aufgabe:
   Liefert (Lambda)-Faktor 2 des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
double get_im_lambda_fac2(INVERSE_METHOD *im)
{
  return (im)?im->lambda_fac2:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_lambda
                                                                          */
/* Aufgabe:
   Setzt Radius des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long lambda: Verteilungslambda
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_lambda(INVERSE_METHOD *im, double lambda)
{
  if (im) im->lambda=lambda;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_lambda
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Verteilungslambda -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_lambda(INVERSE_METHOD *im)
{
        return (im)?im->lambda:-1;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_maxiter
                                                                          */
/* Aufgabe:
   Setzt maximale Iterationszahl des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E maxiter: maximale Iteration des nichtlinearen iterativen Loesers.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_maxiter(INVERSE_METHOD *im, long maxiter)
{
  if (im) im->maxiter=maxiter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_maxiter
                                                                          */
/* Aufgabe:
   Liefert maximale Iterationszahl des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - maximale Iterationszahl -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_maxiter(INVERSE_METHOD *im)
{
        return (im)?im->maxiter:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_criterium
                                                                          */
/* Aufgabe:
   Setzt Kriterium des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E long criterium: Kriterium (Konvergenztyp)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_criterium(INVERSE_METHOD *im, long criterium)
{
  if (im) im->criterium=criterium;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_criterium
                                                                          */
/* Aufgabe:
   Liefert Kriterium des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_criterium(INVERSE_METHOD *im)
{
        return (im)?im->criterium:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_norm
                                                                          */
/* Aufgabe:
   Setzt Norm (Konvergenznorm) des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
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
void set_im_norm(INVERSE_METHOD *im, int norm)
{
  if (im) im->norm=norm;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_norm
                                                                          */
/* Aufgabe:
   Liefert Norm (Konvergenznorm) des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_im_norm(INVERSE_METHOD *im)
{
  return (im)?im->norm:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_abs_eps
                                                                          */
/* Aufgabe:
   Setzt absolute Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double abs_eps: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_abs_eps(INVERSE_METHOD *im, double abs_eps)
{
  if (im) im->abs_eps=abs_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_abs_eps
                                                                          */
/* Aufgabe:
   Liefert absolute Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_abs_eps(INVERSE_METHOD *im)
{
        return (im)?im->abs_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_rel_eps
                                                                          */
/* Aufgabe:
   Setzt relative Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double rel_eps: relative Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_rel_eps(INVERSE_METHOD *im, double rel_eps)
{
  if (im) im->rel_eps=rel_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_rel_eps
                                                                          */
/* Aufgabe:
   Liefert relative Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_rel_eps(INVERSE_METHOD *im)
{
        return (im)?im->rel_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_var_eps
                                                                          */
/* Aufgabe:
   Setzt die variable Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
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
void set_im_var_eps(INVERSE_METHOD *im, double var_eps)
{
  if (im) im->var_eps=var_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_var_eps
                                                                          */
/* Aufgabe:
   Liefert die variable Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_var_eps(INVERSE_METHOD *im)
{
  return (im)?im->var_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_begin_eps
                                                                          */
/* Aufgabe:
   Setzt Anfangsgenauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
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
void set_im_begin_eps(INVERSE_METHOD *im, double begin_eps)
{
  if (im) im->begin_eps=begin_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_begin_eps
                                                                          */
/* Aufgabe:
   Liefert Anfangsgenauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_begin_eps(INVERSE_METHOD *im)
{
  return (im)?im->begin_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_end_eps
                                                                          */
/* Aufgabe:
   Setzt Endgenauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
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
void set_im_end_eps(INVERSE_METHOD *im, double end_eps)
{
  if (im) im->end_eps=end_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_end_eps
                                                                          */
/* Aufgabe:
   Liefert Endgenauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_end_eps(INVERSE_METHOD *im)
{
  return (im)?im->end_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_fac_eps
                                                                          */
/* Aufgabe:
   Setzt Faktor der variablen Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double fac_eps: Faktor der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_fac_eps(INVERSE_METHOD *im, double fac_eps)
{
  if (im) im->fac_eps=fac_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_fac_eps
                                                                          */
/* Aufgabe:
   Liefert Faktor der variablen Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_fac_eps(INVERSE_METHOD *im)
{
  return (im)?im->fac_eps:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_exp_eps
                                                                          */
/* Aufgabe:
   Setzt Exponenten der variablen Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
   E double exp_eps: Exponent der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_exp_eps(INVERSE_METHOD *im, double exp_eps)
{
  if (im) im->exp_eps=exp_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_exp_eps
                                                                          */
/* Aufgabe:
   Liefert Exponenten der variablen Genauigkeit des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf eine Instanz vom Typ
                                        INVERSE_METHOD.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_exp_eps(INVERSE_METHOD *im)
{
  return (im)?im->exp_eps:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_kind
                                                                          */
/* Aufgabe:
   Setzt Art der IM.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long kind: Art der inversen Methode                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_kind(INVERSE_METHOD *im, long kind)
{
  if (im) im->kind=kind;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_kind
                                                                          */
/* Aufgabe:
   Liefert Art der IM.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_kind(INVERSE_METHOD *im)
{
        return (im)?im->kind:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_curve
                                                                          */
/* Aufgabe:
   Setzt Kurve des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_curve(INVERSE_METHOD *im, long curve)
{
  if (im) im->curve=curve;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_curve
                                                                          */
/* Aufgabe:
   Liefert Kurve des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_curve(INVERSE_METHOD *im)
{
        return (im)?im->curve:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des IM's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_im_count_of_values(INVERSE_METHOD *im)
{
  return (im)?im->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_count_of_values(INVERSE_METHOD *im, long count_of_values)
{
  if (im) im->count_of_values=count_of_values;
  return;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: set_im_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_im_value(INVERSE_METHOD *im, long index, double value)
{
  if (im && im->values) {
    if ( (index<im->count_of_values) && (index>=0) )
          im->values[index]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (IM) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_im_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IM's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_im_value(INVERSE_METHOD *im, long index)
{
  if (im && im->values) {
    if ( (index<im->count_of_values) && (index>=0) )
          return im->values[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (IM) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (IM) (Returnwert -1)!");
        return -1.;
  }
}



/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_inverse_method_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von inversen Methoden;
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
LIST_INVERSE_METHOD *create_inverse_method_list (char *name)
{
  LIST_INVERSE_METHOD *list_of_inverse_method;

  list_of_inverse_method = (LIST_INVERSE_METHOD *) Malloc(sizeof(LIST_INVERSE_METHOD));
  if ( list_of_inverse_method == NULL ) return NULL;


  list_of_inverse_method->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_inverse_method->name == NULL ) {
         Free(list_of_inverse_method);
         return NULL;
  }
  strcpy(list_of_inverse_method->name,name);

  list_of_inverse_method->im_list=create_list();
  if ( list_of_inverse_method->im_list == NULL ) {
    Free(list_of_inverse_method->name);
        Free(list_of_inverse_method);
        return NULL;
  }

  list_of_inverse_method->names_of_inverse_method=NULL;
  list_of_inverse_method->count_of_inverse_method_name=0;

  return list_of_inverse_method;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_inverse_method_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der inversen Methoden aus dem Speicher
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
void destroy_inverse_method_list(void)
{
  if (list_of_inverse_method->name) list_of_inverse_method->name=(char *)Free(list_of_inverse_method->name);
  if(list_of_inverse_method->names_of_inverse_method) \
    list_of_inverse_method->names_of_inverse_method = \
      (char **)Free(list_of_inverse_method->names_of_inverse_method);

  if (list_of_inverse_method->im_list) {
    delete_list(list_of_inverse_method->im_list,destroy_inverse_method);
    list_of_inverse_method->im_list=destroy_list(list_of_inverse_method->im_list);
  }

  if (list_of_inverse_method) list_of_inverse_method=(LIST_INVERSE_METHOD *)Free(list_of_inverse_method);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_method_list_empty
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
int inverse_method_list_empty(void)
{
  if (!list_of_inverse_method) return 0;
  else if (!list_of_inverse_method->im_list) return 0;
  else return list_empty(list_of_inverse_method->im_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_im_init
                                                                          */
/* Aufgabe:
   Liste der IM's initialisieren.
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
void list_im_init(void)
{
  if (list_of_inverse_method) list_current_init(list_of_inverse_method->im_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_im_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_im_next (void)
{
  return list_of_inverse_method->im_list ?
             get_list_next(list_of_inverse_method->im_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_inverse_method_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der inversen Methode mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ INVERSE_METHOD erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IM's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_inverse_method_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_inverse_method) return 0;
  for ( i=0; i<list_of_inverse_method->count_of_inverse_method_name; i++)
    if(strcmp(list_of_inverse_method->names_of_inverse_method[i],name) == 0) break;
  if (i < list_of_inverse_method->count_of_inverse_method_name) return i;

  list_of_inverse_method->names_of_inverse_method= \
          (char **) Realloc(list_of_inverse_method->names_of_inverse_method, \
          (list_of_inverse_method->count_of_inverse_method_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_inverse_method->names_of_inverse_method[i]=new_name;

  return ++(list_of_inverse_method->count_of_inverse_method_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_inverse_method_names
                                                                          */
/* Aufgabe:
   Alle inversen Methoden von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IM's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_inverse_method_names (void)
{
  long i;

  for (i=0; i<list_of_inverse_method->count_of_inverse_method_name; i++)
    if(list_of_inverse_method->names_of_inverse_method[i]) \
      list_of_inverse_method->names_of_inverse_method[i] = \
        (char *)Free(list_of_inverse_method->names_of_inverse_method[i]);

  return list_of_inverse_method->count_of_inverse_method_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_inverse_method_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ INVERSE_METHOD in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_inverse_method_list (INVERSE_METHOD *im)
{
  if(!list_of_inverse_method) return 0;
  list_of_inverse_method->count_of_inverse_method=append_list(list_of_inverse_method->im_list,(void *) im);
  return list_of_inverse_method->count_of_inverse_method;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_inverse_method_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ INVERSE_METHOD der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender INVERSE_METHOD-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
long remove_inverse_method_list (INVERSE_METHOD *im)
{
  list_of_inverse_method->count_of_inverse_method = \
      remove_list_member(list_of_inverse_method->im_list,(void *) im, destroy_inverse_method);
  return list_of_inverse_method->count_of_inverse_method;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: inverse_method_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines IM-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IM's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long inverse_method_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_inverse_method->count_of_inverse_method_name; i++)
    if(strcmp(list_of_inverse_method->names_of_inverse_method[i],name) == 0) break;
  if (i < list_of_inverse_method->count_of_inverse_method_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateInverseMethodList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von inversen Methoden (unbenannt);
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
void CreateInverseMethodList(void)
{
  list_of_inverse_method = create_inverse_method_list("INVERSE_METHOD");
  insert_inverse_method_list(create_inverse_method(MASTER_DEFAULT_INVERSE_METHOD));
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseMethodList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von inversen Methoden (unbenannt);
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
void DestroyInverseMethodList(void)
{
  undeclare_inverse_method_names();
  destroy_inverse_method_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InverseMethodListEmpty
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
int InverseMethodListEmpty(void)
{
  return inverse_method_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetInverseMethodGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INVERSE_METHOD
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INVERSE_METHOD *im -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INVERSE_METHOD
                                                                          */
/* Programmaenderungen:
   10/2001     OK         Erste Version
                                                                          */
/**************************************************************************/
INVERSE_METHOD *GetInverseMethodGroup(char *name,INVERSE_METHOD *im)
{
  if (!name) return NULL;
  if (!inverse_method_name_exist(name) ) return NULL;

  if(im==NULL) list_im_init();

  while ( (im=(INVERSE_METHOD *)get_list_im_next()) != NULL ) {
    if ( strcmp(StrUp(get_im_name(im)),StrUp(name))==0 ) break;
  }

  return im;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInverseMethodListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche INVERSE_METHOD-Objekte der Gruppe NAME
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
int DestroyInverseMethodListGroup(char *name)
{
  INVERSE_METHOD *im=NULL;

  if (!name) return -1;
  if (!inverse_method_name_exist(name) ) return -1;

  list_im_init();

  while ( (im=(INVERSE_METHOD *)get_list_im_next()) != NULL ) {
    if ( strcmp(StrUp(get_im_name(im)),StrUp(name))==0 )
        remove_inverse_method_list(im);
  }

  return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodNew
                                                                          */
/* Aufgabe:
   Fuehrt die inverse Methode mit dem Namen name aus.
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
int ExecuteInverseMethodNew (char *name, TimeLoopFunction time_loop_function)
{
  INVERSE_METHOD *im=NULL;

  /*if (!name) return 1;
  if (InverseMethodListEmpty()) return 1;
  if ( !inverse_method_name_exist(name) ) return 1;*/

  if (!name || InverseMethodListEmpty() || !inverse_method_name_exist(name)) {
    list_im_init();
    im=(INVERSE_METHOD *)get_list_im_next();
    set_im_time_loop_function(im,time_loop_function);
    ExecuteInverseMethodMethod0(im);
    return 1;
  }
  list_im_init();
  while ( (im=(INVERSE_METHOD *)get_list_im_next()) != NULL ) {
    if ( !get_im_group_name(im) ) continue;
    else if ( strcmp(StrUp(get_im_group_name(im)),StrUp(name)) ) continue;
    set_im_time_loop_function(im,time_loop_function);

    switch (get_im_type(im)) {
            case 0:
              ExecuteInverseMethodMethod0(im);
              break;
            case 1:
              ExecuteInverseMethodMethod1(im);
              break;
            case 2:
              ExecuteInverseMethodMethod2(im);
              break;
            case 3:
              ExecuteInverseMethodMethod3(im);
              break;
            case 4:
              ExecuteInverseMethodMethod4(im);
              break;
            case 5:
              ExecuteInverseMethodMethod5(im);
              break;
            case 10:
              ExecuteInverseMethodMethod10(im);
              break;
            case 11:
              ExecuteInverseMethodMethod11(im);
              break;
        } /* switch */

  } /* while */
  return 1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod0
                                                                          */
/* Aufgabe:
   Methode 1 
   Vorwärtsrechnen mit den Start-Variablen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod0 (INVERSE_METHOD *im)
{
  ExecuteForwardSimulation(im);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod1
                                                                          */
/* Aufgabe:
   Methode 1 : Bracket method (Two point search method)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod1 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  double new_value;

  ExecuteForwardSimulation(im);

    /*inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW");*/

      while ( inv_flag ) {
        /* Zeitschleife */
        SetAllMinInverseVariable();
        ExecuteForwardSimulation(im);
        inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value);

        /* Zeitschleife */
        SetAllMaxInverseVariable();
        ExecuteForwardSimulation(im);
        inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value);

        inv_flag=0;
      }

}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod2
                                                                          */
/* Aufgabe:
   Methode 2 : Bracket method (Three point search method)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod2 (INVERSE_METHOD *im)
{
  if (get_im_time_loop_function(im)) {
//OK_TIM    ConfigTime();
//OK_MOD    InitModel();
    get_im_time_loop_function(im)();
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod3
                                                                          */
/* Aufgabe:
   Methode 2 : Golden section search method
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod3 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags;
  long iv_names_number=GetInverseVariableNamesNumber();
  char *iv_name;
  long i,j;
  double tau=0.618;
  double new_value1,new_value2;
  double new_value1_function,new_value2_function;
  long iter=0;
  long max_iter=get_im_maxiter(im);
  double eps=get_im_abs_eps(im);
  double lse,tlse,otlse=1.e+60;
  double *min_values,*max_values,*start_values,*coeff_values;
  double *old_values,*new_values,*eps_values,*dx;
  long *max_iter_values;
  long niv=GetInverseVariableNamesNumber();
  double feps, iter_abs=0.0;
  static int flag_text=0;
  char string[80];

  if (!flag_text) {
    PrintStartInverseText("INVERSE-ROCKFLOW");
    flag_text=1;
  }

  lse=0.0;

  min_values=(double *) Malloc(niv*sizeof(double));
  max_values=(double *) Malloc(niv*sizeof(double));
  start_values=(double *) Malloc(niv*sizeof(double));
  coeff_values=(double *) Malloc(niv*sizeof(double));
  new_values=(double *) Malloc(niv*sizeof(double));
  old_values=(double *) Malloc(niv*sizeof(double));
  dx=(double *) Malloc(niv*sizeof(double));
  eps_values=(double *) Malloc(niv*sizeof(double));
  max_iter_values=(long *) Malloc(niv*sizeof(long));

  /* -Informationen ueber Inverse-Variablen  holen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    GetMinInverseVariable(iv_name,&min_values[i]);
    GetMaxInverseVariable(iv_name,&max_values[i]);
    GetStartInverseVariable(iv_name,&start_values[i]);
    GetCoeffInverseVariable(iv_name,&coeff_values[i]);
    GetStartInverseVariable(iv_name,&old_values[i]);
    GetEpsInverseVariable(iv_name,&eps_values[i]);
    GetMaxiterInverseVariable(iv_name,&max_iter_values[i]);
  }


  /* Start-Werte als aktuelle Werte setzen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    new_values[i]=start_values[i];
    SetCurrentInverseVariable(iv_name,new_values[i]);
  }

  for (j=0; j<max_iter; j++) {

    inv_flags=1;
    tlse=0.0;

    /* Iteration schreiben */
    PrintCycleInverseIteration(j+1,"INVERSE-ROCKFLOW");

    for (i=0; i<iv_names_number; i++) {
      iter=0;
      /* Max- und Min-Werte neu setzen */
      iv_name=GetInverseVariableName(i);
      GetMinInverseVariable(iv_name,&min_values[i]);
      GetMaxInverseVariable(iv_name,&max_values[i]);
      PrintNameInverseVariable(iv_name,"INVERSE-ROCKFLOW");
      do {
        iv_name=GetInverseVariableName(i);

        /* Iteration schreiben */
        PrintStartInverseIteration(iter+1,"INVERSE-ROCKFLOW");

        new_value1=max_values[i]-tau*(max_values[i]-min_values[i]);
        new_value2=min_values[i]+tau*(max_values[i]-min_values[i]);


        /* 1. Wert setzen */
        if ( !SetCurrentInverseVariable(iv_name,new_value1) ) continue;
        /* Vorwaertssimaltion */
        ExecuteForwardSimulation(im);
        /* Iterationen pruefen */
        inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value1_function);
        inv_flags = inv_flags||inv_flag;


        /* 2. Wert setzen */
        if ( !SetCurrentInverseVariable(iv_name,new_value2) ) continue;
        /* Vorwaertssimaltion */
        ExecuteForwardSimulation(im);
        /* Iterationen pruefen */
        inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value2_function);
        inv_flags = inv_flags||inv_flag;

             
        if (new_value1_function < new_value2_function) {
          max_values[i]=new_value2;
        }
        else {
          min_values[i]=new_value1;
        }

        dx[i]=fabs(max_values[i]-min_values[i]);
        lse=(new_value1_function+new_value2_function)/2.;
        new_values[i]=(max_values[i]+min_values[i])/2.;
        /* Neuen Wert setzen */
        if ( !SetCurrentInverseVariable(iv_name,new_values[i]) ) continue;
        /* Mittelwert schreiben */
        PrintStartInverseVariable(iv_name,"INVERSE-ROCKFLOW");        
        if( dx[i] < eps_values[i]/pow(10.,(double)j) ) { 
        /*if( dx[i] < eps_values[i] ) { */
          DisplayMsgLn("");
          DisplayMsgLn("***********************************************");
          DisplayMsg("Inverse Problem: ");
          DisplayLong(iter);
          DisplayMsgLn(" iterations. ");
          DisplayMsgLn("***********************************************");
          DisplayMsgLn("");
          tlse+=lse;
          break;
        }
        iter++;
      } while (iter<max_iter_values[i]);
      if(iter>=max_iter_values[i]) { 
        DisplayMsgLn("");
        DisplayMsgLn("***********************************************");
        DisplayMsg("Inverse Problem: var : ");
        sprintf(string,"%s",iv_name);
        DisplayMsg(string);
        DisplayMsgLn("");
        DisplayMsg("Inverse Problem: iter = ");
        DisplayLong(iter);
        DisplayMsgLn("");
        DisplayMsgLn(" !!! Abbruch - maximale Iteration ist erreicht !!! ");
        DisplayMsgLn("***********************************************");
        DisplayMsgLn("");
        exit(1);
      }
    } /* for i */
    PrintStartInverseLSE(tlse,"INVERSE-ROCKFLOW");
    PrintStartInverseLF("INVERSE-ROCKFLOW");
    feps=MVekDist(old_values,new_values,niv);
    iter_abs=fabs(tlse-otlse);
    /*if(feps<eps && iter_abs<eps && j!=0) break;*/
    /*if((iter_abs<eps || tlse<eps) && j!=0) break;*/
    if(tlse<eps && j!=0) break;
    if(!inv_flags) break;
    for (i=0; i<niv; i++) {
      old_values[i]=new_values[i];
    }
    otlse=tlse;
  } /* for j  */

  min_values=(double *) Free(min_values);
  max_values=(double *) Free(max_values);
  start_values=(double *) Free(start_values);
  coeff_values=(double *) Free(coeff_values);
  new_values=(double *) Free(new_values);
  old_values=(double *) Free(old_values);
  dx=(double *) Free(dx);
  eps_values=(double *) Free(eps_values);
  max_iter_values=(long *) Free(max_iter_values);

  if(j>=max_iter) { 
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
    DisplayMsg("Inverse Problem: iter = ");
    DisplayLong(iter);
    DisplayMsgLn("");
    DisplayMsgLn(" !!! Abbruch - maximale Iteration ist erreicht !!! ");
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("");
    exit(1);
  }

}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod3V1
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod3V1 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags;
  long iv_names_number=GetInverseVariableNamesNumber();
  char *iv_name;
  long i,j;
  double min_value;
  double max_value;
  double tau=0.618;
  double new_value1=0.0,new_value2=0.0;
  double new_value1_function,new_value2_function;
  long iter=0;
  long max_iter=get_im_maxiter(im);
  double eps=get_im_abs_eps(im);
  double lse;
  static int flag_text=0;

  if (!flag_text) {
    PrintStartInverseText("INVERSE-ROCKFLOW");
    flag_text=1;
  }

  for (j=0; j<max_iter; j++) {

    inv_flags=1;
    lse=0.0;

    /* Iteration schreiben */
    PrintStartInverseIteration(j+1,"INVERSE-ROCKFLOW");

    for (i=0; i<iv_names_number; i++) {
      iv_name=GetInverseVariableName(i);

      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;
      new_value1=max_value-tau*(max_value-min_value);
      new_value2=min_value+tau*(max_value-min_value);

      /* Min-Wert setzen */
      if ( !SetCurrentInverseVariable(iv_name,new_value1) ) continue;

      /* Vorwaertssimaltion */
      ExecuteForwardSimulation(im);

      /* Modifizierte Parameter rausschreiben */
      /*PrintStartInverseVariable(iv_name,"INVERSE-ROCKFLOW");*/

      /* Iterationen pruefen */
      inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value1_function);
      inv_flags = inv_flags||inv_flag;

      /* Max-Wert setzen */
      if ( !SetCurrentInverseVariable(iv_name,new_value2) ) continue;

      /* Vorwaertssimaltion */
      ExecuteForwardSimulation(im);

      /* Modifizierte Parameter rausschreiben */
      /*PrintStartInverseVariable(iv_name,"INVERSE-ROCKFLOW");*/

      /* Iterationen pruefen */
      inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value2_function);
      inv_flags = inv_flags||inv_flag;

             
      if (new_value1_function < new_value2_function) {
        if ( !SetMinInverseVariable(iv_name,min_value) ) continue;
        if ( !SetMaxInverseVariable(iv_name,new_value2) ) continue;
      }
      else {
        if ( !SetMinInverseVariable(iv_name,new_value1) ) continue;
        if ( !SetMaxInverseVariable(iv_name,max_value) ) continue;
      }

      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;
      new_value1=max_value-tau*(max_value-min_value);
      new_value2=min_value+tau*(max_value-min_value);

      lse+=(new_value1_function+new_value2_function)/2.;

      /* Mittelwert schreiben */
      if ( !SetCurrentInverseVariable(iv_name,(new_value1+new_value2)/2.) ) continue;
      PrintStartInverseVariable(iv_name,"INVERSE-ROCKFLOW");
      iter++;
      if(fabs(new_value2-new_value1)<eps) { 
        DisplayMsgLn("");
        DisplayMsgLn("***********************************************");
        DisplayMsg("Inverse Problem: ");
        DisplayLong(iter);
        DisplayMsgLn(" iterations. ");
        DisplayMsgLn("***********************************************");
        DisplayMsgLn("");
        break;
      }
      if (iter>=max_iter) break;
    } /* for i */
    PrintStartInverseLSE(lse/iv_names_number,"INVERSE-ROCKFLOW");
    if(fabs(new_value2-new_value1)<eps) break;
    if(!inv_flags) break;


  } /* for j  */

}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod3V0
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod3V0 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags;
  long iv_names_number=GetInverseVariableNamesNumber();
  char *iv_name;
  long i;
  double last_start_value/*,start_value,start_value_function*/;
  double last_min_value,min_value/*,min_value_function*/;
  double last_max_value,max_value/*,max_value_function*/;
  double last_value/*,new_value*/;
  double tau=0.618;
  double new_value1,new_value2;
  double new_value1_function=0.0,new_value2_function=0.0;
  long iter;
  char *iter_name;

  for (i=0; i<iv_names_number; i++) {
    iv_name=GetInverseVariableName(i);
    iter=0;
    if ( !GetStartInverseVariable(iv_name,&last_start_value) ) continue;
    if ( !GetMinInverseVariable(iv_name,&last_min_value) ) continue;
    if ( !GetMaxInverseVariable(iv_name,&last_max_value) ) continue;

    new_value1=last_max_value-tau*(last_max_value-last_min_value);
    new_value2=last_min_value+tau*(last_max_value-last_min_value);
    last_value=last_start_value;

    inv_flags=1;
    do {
      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;


/*      if ( !SetCurrentInverseVariable(iv_name,min_value) ) continue;
      
      ExecuteForwardSimulation(im);
        inv_flag=ExecuteInverseIterationNew("CONC1","INVERSE-ROCKFLOW",&min_value_function);
        inv_flags = inv_flags||inv_flag;

      if ( !SetCurrentInverseVariable(iv_name,max_value) ) continue;
      ExecuteForwardSimulation(im);
        inv_flag=ExecuteInverseIterationNew("CONC1","INVERSE-ROCKFLOW",&max_value_function);
        inv_flags = inv_flags||inv_flag;
*/

      if ( !SetCurrentInverseVariable(iv_name,new_value1) ) continue;
      /* Vorwaertssimaltion */
      /*ExecuteForwardSimulation(im);*/
      if (get_im_time_loop_function(im)) {
        ExecuteForwardSimulation(im);
        iter_name=GetIterationNameObservationData(iv_name);
        inv_flag=ExecuteInverseIterationNew(iv_name,iv_name,"INVERSE-ROCKFLOW",&new_value1_function);
        inv_flags = inv_flags||inv_flag;
      }
      
      if ( !SetCurrentInverseVariable(iv_name,new_value2) ) continue;
      /* Vorwaertssimaltion */
      /*ExecuteForwardSimulation(im);*/
      if (get_im_time_loop_function(im)) {
        ExecuteForwardSimulation(im);
        iter_name=GetIterationNameObservationData(iv_name);
        inv_flag=ExecuteInverseIterationNew(iv_name,iv_name,"INVERSE-ROCKFLOW",&new_value2_function);
        inv_flags = inv_flags||inv_flag;
      }
       
      if (new_value1_function < new_value2_function) {
        if ( !SetMinInverseVariable(iv_name,min_value) ) continue;
        if ( !SetMaxInverseVariable(iv_name,new_value2) ) continue;
      }
      else {
        if ( !SetMinInverseVariable(iv_name,new_value1) ) continue;
        if ( !SetMaxInverseVariable(iv_name,max_value) ) continue;
      }

      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;
      new_value1=max_value-tau*(max_value-min_value);
      new_value2=min_value+tau*(max_value-min_value);
      iter++;
      if(fabs(new_value2-new_value1)<1.e-10) { 
        DisplayMsgLn("");
        DisplayMsgLn("***********************************************");
        DisplayMsg("Inverse Problem: ");
        DisplayLong(iter);
        DisplayMsgLn(" iterations. ");
        DisplayMsgLn("***********************************************");
        DisplayMsgLn("");
        break;
      }
    }
    while(inv_flags);


  }

}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod4
                                                                          */
/* Aufgabe:
   Methode 4 : Fibonacci section search method 
   Noch nicht fertig!!!
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod4 (INVERSE_METHOD *im)
{
  /* Vorwaertssimaltion */
  ExecuteForwardSimulation(im);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod5
                                                                          */
/* Aufgabe:
   Methode 5 : Quadratic interpolation method
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod5 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags;
  long iv_names_number=GetInverseVariableNamesNumber();
  char *iv_name;
  long i;
  double last_start_value,start_value,start_value_function=0.0;
  double last_min_value,min_value,min_value_function=0.0;
  double last_max_value,max_value,max_value_function=0.0;
  double last_value,new_value;
  long iter;

  for (i=0; i<iv_names_number; i++) {
    iv_name=GetInverseVariableName(i);
    iter=0;

    if ( !GetStartInverseVariable(iv_name,&last_start_value) ) continue;
    if ( !GetMinInverseVariable(iv_name,&last_min_value) ) continue;
    if ( !GetMaxInverseVariable(iv_name,&last_max_value) ) continue;

    start_value=(last_max_value-last_min_value)/2.0;
    if ( !GetStartInverseVariable(iv_name,&start_value) ) continue;
    last_value=start_value;

    inv_flags=1;
    do {
      if ( !GetStartInverseVariable(iv_name,&start_value) ) continue;
      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;

      if ( !SetCurrentInverseVariable(iv_name,start_value) ) continue;
      if (get_im_time_loop_function(im)) {
        /* Vorwaertssimaltion */
        ExecuteForwardSimulation(im);
        inv_flag=ExecuteInverseIterationNew(iv_name,"CONC1","INVERSE-ROCKFLOW",&start_value_function);
        inv_flags = inv_flags||inv_flag;
      }
      
   
      if ( !SetCurrentInverseVariable(iv_name,min_value) ) continue;
      if (get_im_time_loop_function(im)) {
        /* Vorwaertssimaltion */
        ExecuteForwardSimulation(im);
        inv_flag=ExecuteInverseIterationNew(iv_name,"CONC1","INVERSE-ROCKFLOW",&min_value_function);
        inv_flags = inv_flags||inv_flag;
      }

      if ( !SetCurrentInverseVariable(iv_name,max_value) ) continue;
      if (get_im_time_loop_function(im)) {
        /* Vorwaertssimaltion */
        ExecuteForwardSimulation(im);
        inv_flag=ExecuteInverseIterationNew(iv_name,"CONC1","INVERSE-ROCKFLOW",&max_value_function);
        inv_flags = inv_flags||inv_flag;
      }


      if ( !InverseMethod5(min_value,min_value_function, max_value,max_value_function,
                           start_value,start_value_function, &new_value) ) {
        printf("\n     !!!!! Error !!!!!!");
        continue;
      }
       
      if (new_value < start_value) {
        if ( !SetCurrentInverseVariable(iv_name,new_value) ) continue;
        if ( !SetMinInverseVariable(iv_name,min_value) ) continue;
        if ( !SetMaxInverseVariable(iv_name,start_value) ) continue;
        last_value=start_value;
        start_value=new_value;
      }
      else {
        if ( !SetCurrentInverseVariable(iv_name,new_value) ) continue;
        if ( !SetMinInverseVariable(iv_name,start_value) ) continue;
        if ( !SetMaxInverseVariable(iv_name,max_value) ) continue;
        last_value=start_value;
        start_value=new_value;
      }

      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;
      start_value=(max_value-min_value)/2.0;
      if ( !GetStartInverseVariable(iv_name,&start_value) ) continue;


      if ( !GetMinInverseVariable(iv_name,&min_value) ) continue;
      if ( !GetMaxInverseVariable(iv_name,&max_value) ) continue;
      iter++;
      if(fabs(max_value-min_value)<1.e-10) { 
        DisplayMsgLn("");
        DisplayMsgLn("***********************************************");
        DisplayMsg("Inverse Problem: ");
        DisplayLong(iter);
        DisplayMsgLn(" iterations. ");
        DisplayMsgLn("***********************************************");
        DisplayMsgLn("");
        break;
      }


    }
    while(inv_flags);
  }

}



/**************************************************************************/
/* ROCKFLOW - Funktion: InverseMethod5
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int InverseMethod5 (double a, double fa, double b, double fb, double c, double fc, 
                     double *d)
{
  double s=(b-a)/2;
  
  if ( fabs(s-c) <= MKleinsteZahl) return 0;
  if ( fabs(2.*(fb-2.*fc+fa)) <= MKleinsteZahl) return 0;

  /**d = fabs(c + s*(fa-fb) / (2.*(fb-2.*fc+fa)));*/

  *d = c - 0.5*( (b-a)*(b-a)*(fb-fc) - (b-c)*(b-c)*(fb-fa) ) \
              /( (b-a)*(fb-fc) - (b-c)*(fb-fa) ) ;

  return 1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod10
                                                                          */
/* Aufgabe:
   Methode 10 : Gauss-Newton method  
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod10 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags=0;
  char *iv_name;
  char *obs_name;
  long i,j,l;
  double new_value1_function,new_value2_function;
  long iter=0;
  long max_iter=get_im_maxiter(im);
  double eps=get_im_abs_eps(im);
  double *gnmat=NULL,*vecb=NULL,*vecx=NULL;
  double *mat=NULL,*tmat=NULL,*vecf;
  double *min_values,*max_values,*start_values,*new_values,*coeff_values;
  double *dx;
  long niv=GetInverseVariableNamesNumber();
  long nobs=GetObservationDataNamesNumber();
  double lse, feps, iter_abs=0.0;
  static int flag_text=0;

  if (niv <= 0) {
    /* Vorwaertssimaltion */
    ExecuteForwardSimulation(im);
    return;
  }

  if (!flag_text) {
    PrintStartInverseText("INVERSE-ROCKFLOW");
    flag_text=1;
  }

  lse=0.0;



  /* Speicher holen */
  mat=(double *) Malloc(nobs*niv*sizeof(double));
  tmat=(double *) Malloc(niv*nobs*sizeof(double));
  vecf=(double *) Malloc(nobs*sizeof(double));

  gnmat=(double *) Malloc(niv*niv*sizeof(double));
  vecb=(double *) Malloc(niv*sizeof(double));
  vecx=(double *) Malloc(niv*sizeof(double));

  min_values=(double *) Malloc(niv*sizeof(double));
  max_values=(double *) Malloc(niv*sizeof(double));
  start_values=(double *) Malloc(niv*sizeof(double));
  coeff_values=(double *) Malloc(niv*sizeof(double));
  new_values=(double *) Malloc(niv*sizeof(double));
  dx=(double *) Malloc(niv*sizeof(double));


  /* -Informationen ueber Inverse-Variablen  holen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    GetMinInverseVariable(iv_name,&min_values[i]);
    GetMaxInverseVariable(iv_name,&max_values[i]);
    GetStartInverseVariable(iv_name,&start_values[i]);
    GetCoeffInverseVariable(iv_name,&coeff_values[i]);
  }


  /* Start-Werte als aktuelle Werte setzen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    new_values[i]=start_values[i];
    SetCurrentInverseVariable(iv_name,new_values[i]);
  }


  /* Berechnung mit Start-Werte */
  /* Vorwaertssimaltion */
  ExecuteForwardSimulation(im);

  /* Iterationen pruefen */
  inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value1_function);
  inv_flags = inv_flags||inv_flag;
  /* Noch zu ergaenzen: Hier moeglicheweise Abbruch */
  if ( new_value1_function < 1.e-12 ) {
    /* Gauss-Newton-Iteration schreiben */
    PrintStartInverseIteration(iter,"INVERSE-ROCKFLOW");
    PrintAllStartInverseVariable("INVERSE-ROCKFLOW");
    PrintStartInverseLSE(new_value1_function,"INVERSE-ROCKFLOW");
    return;
  }

  /* modellspezifische Ergebnisse fuer Inverses Modellieren extra speichern */
  CopyToLastAllModelObservationData();

  iter=0;
  do {

    for (i=0; i<niv; i++) {

      /* Gestoerten Wert setzen */
      for (l=0; l<niv; l++) {
        iv_name=GetInverseVariableName(l);
        new_values[l]=start_values[l];
      }
      iv_name=GetInverseVariableName(i);
      dx[i]=coeff_values[i]*start_values[i];
      /*dx[i]=coeff_values[i];*/
      new_values[i]=start_values[i]+dx[i];
      for (l=0; l<niv; l++) {
        iv_name=GetInverseVariableName(l);
        SetCurrentInverseVariable(iv_name,new_values[l]);
      }

      /* Vorwaertssimaltion */
      ExecuteForwardSimulation(im);

      for (j=0; j<nobs; j++) {
        obs_name=GetObservationDataName(j);
        /* Aufbau der Sensiviaetsmatrix */
        CalculateSensitivityParameterInverseIteration(obs_name,dx[i], \
                                                   &mat[i*nobs+j]);
      }
    } /* for i */

    for (j=0; j<nobs; j++) {
      obs_name=GetObservationDataName(j);
      /* Aufbau des Sensiviaetsvektors */
      CalculateRightSideSensitivityParameterInverseIteration(obs_name,&vecf[j]);
    }


    /* Aufbau der Gaus-Newton-Matrix */
    MTranspoMat(mat,nobs,niv,tmat);
    MMultMatMat(tmat,niv,nobs, mat,nobs,niv, gnmat,niv,niv);
    /* Aufbau der Gaus-Newton-Rechte-Seite */
    MMultMatVec(tmat,niv,nobs, vecf,nobs,vecb,niv);       
    /* Loesen des Gaus-Newton-GLS */
    Gauss(gnmat,vecb,vecx ,niv);


    /* Bereich der neuen Werte pruefen und als aktuelle Werte setzen */
    for (i=0; i<niv; i++) {
      iv_name=GetInverseVariableName(i);
      new_values[i]=start_values[i]+vecx[i];
      if(new_values[i]<min_values[i]) new_values[i]=min_values[i];
      if(new_values[i]>max_values[i]) new_values[i]=max_values[i];
      SetCurrentInverseVariable(iv_name,new_values[i]);
    }


    /* Berechnung mit Verbesserten Werte */
    /* Vorwaertssimaltion */
    ExecuteForwardSimulation(im);

    /* Iterationen pruefen */
    inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value2_function);
    inv_flags = inv_flags||inv_flag;
    if(!inv_flags) break;

    /* modellspezifische Ergebnisse fuer Inverses Modellieren extra speichern */
    CopyToLastAllModelObservationData();

    feps=fabs(new_value2_function-new_value1_function);

    /* Gauss-Newton-Iteration schreiben */
    PrintStartInverseIteration(iter,"INVERSE-ROCKFLOW");
    PrintAllStartInverseVariable("INVERSE-ROCKFLOW");
    PrintStartInverseLSE(new_value2_function,"INVERSE-ROCKFLOW");

    iter++;
    iter_abs=fabs(new_value2_function-new_value1_function);
    new_value1_function=new_value2_function;
    /* Gestoerten Wert setzen */
    for (l=0; l<niv; l++) {
      start_values[l]=new_values[l];
    }
    if (iter<=1) continue;
  }
  while (/*iter_abs>=eps &&*/ new_value2_function>eps && iter<=max_iter);


  /* Speicherfreigabe */
  mat=(double *) Free(mat);
  tmat=(double *) Free(tmat);
  vecf=(double *) Free(vecf);

  gnmat=(double *) Free(gnmat);
  vecb=(double *) Free(vecb);
  vecx=(double *) Free(vecx);

  min_values=(double *) Free(min_values);
  max_values=(double *) Free(max_values);
  start_values=(double *) Free(start_values);
  coeff_values=(double *) Free(coeff_values);
  new_values=(double *) Free(new_values);
  dx=(double *) Free(dx);

  if(iter<=max_iter) { 
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
    DisplayMsg("Inverses Problem: ");
    DisplayLong(iter);
    DisplayMsgLn(" iterations. ");
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("");
  }
  else { 
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
    DisplayMsg("Inverses Problem: ");
    DisplayLong(iter);
    DisplayMsgLn(" iterations. ");
    DisplayMsgLn(" !!! Maximale Itartionszahl ist ueberschritten worden !!!. ");
    DisplayMsgLn(" Programmabruch. ");
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("");
    exit(1);
  }


}

/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteInverseMethodMethod11
                                                                          */
/* Aufgabe:
   Methode 11 : Gauss-Newton-Levenberg-Marquardt method   
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void ExecuteInverseMethodMethod11 (INVERSE_METHOD *im)
{
  int inv_flag=0;
  int inv_flags=0;
  char *iv_name;
  char *obs_name;
  long i,j,l;
  double new_value1_function,new_value2_function;
  long iter=0,lm_iter=0;
  long max_iter=get_im_maxiter(im);
  long lm_max_iter=get_im_second_maxiter(im);
  double fac1=get_im_lambda_fac1(im);
  double fac2=get_im_lambda_fac2(im);
  double eps=get_im_abs_eps(im);
  double start_lambda=get_im_start_lambda(im); 
  /*double lambda_eps=get_im_lambda_abs_eps(im);*/
  double *gnmat=NULL,*vecb=NULL,*vecx=NULL;
  double *ognmat=NULL,*ovecb=NULL;
  double *mat=NULL,*tmat=NULL,*vecf;
  double *min_values,*max_values,*start_values,*new_values,*coeff_values;
  double *dx;
  long niv=GetInverseVariableNamesNumber();
  long nobs=GetObservationDataNamesNumber();
  double lse, feps, iter_abs=0.0, lambda=0.0;
  static int flag_text=0;

  if (!flag_text) {
    PrintStartInverseText("INVERSE-ROCKFLOW");
    flag_text=1;
  }

  lse=0.0;



  /* Speicher holen */
  mat=(double *) Malloc(nobs*niv*sizeof(double));
  tmat=(double *) Malloc(niv*nobs*sizeof(double));
  vecf=(double *) Malloc(nobs*sizeof(double));

  gnmat=(double *) Malloc(niv*niv*sizeof(double));
  vecb=(double *) Malloc(niv*sizeof(double));
  vecx=(double *) Malloc(niv*sizeof(double));

  ognmat=(double *) Malloc(niv*niv*sizeof(double));
  ovecb=(double *) Malloc(niv*sizeof(double));

  min_values=(double *) Malloc(niv*sizeof(double));
  max_values=(double *) Malloc(niv*sizeof(double));
  start_values=(double *) Malloc(niv*sizeof(double));
  coeff_values=(double *) Malloc(niv*sizeof(double));
  new_values=(double *) Malloc(niv*sizeof(double));
  dx=(double *) Malloc(niv*sizeof(double));


  /* -Informationen ueber Inverse-Variablen  holen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    GetMinInverseVariable(iv_name,&min_values[i]);
    GetMaxInverseVariable(iv_name,&max_values[i]);
    GetStartInverseVariable(iv_name,&start_values[i]);
    GetCoeffInverseVariable(iv_name,&coeff_values[i]);
  }


  /* Start-Werte als aktuelle Werte setzen */
  for (i=0; i<niv; i++) {
    iv_name=GetInverseVariableName(i);
    new_values[i]=start_values[i];
    SetCurrentInverseVariable(iv_name,new_values[i]);
  }


  /* Berechnung mit Start-Werte */
  /* Zeitschleife */
  /* Vorwaertssimaltion */
  ExecuteForwardSimulation(im);
  /* Iterationen pruefen */
  inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value1_function);
  inv_flags = inv_flags||inv_flag;
  /* Noch zu ergaenzen: Hier moeglicheweise Abbruch */
  if ( new_value1_function < 1.e-9 ) {
    /* Gauss-Newton-Iteration schreiben */
    PrintStartInverseIteration(iter,"INVERSE-ROCKFLOW");
    PrintAllStartInverseVariable("INVERSE-ROCKFLOW");
    PrintStartInverseLSE(new_value1_function,"INVERSE-ROCKFLOW");
    return;
  }

  /* modellspezifische Ergebnisse fuer Inverses Modellieren extra speichern */
  CopyToLastAllModelObservationData();

  iter=0;
  do {

    for (i=0; i<niv; i++) {

      /* Gestoerten Wert setzen */
      for (l=0; l<niv; l++) {
        iv_name=GetInverseVariableName(l);
        new_values[l]=start_values[l];
      }
      iv_name=GetInverseVariableName(i);
      dx[i]=coeff_values[i]*start_values[i];
      /*dx[i]=coeff_values[i];*/
      new_values[i]=start_values[i]+dx[i];
      for (l=0; l<niv; l++) {
        iv_name=GetInverseVariableName(l);
        SetCurrentInverseVariable(iv_name,new_values[l]);
      }

      /* Vorwaertssimaltion */
      ExecuteForwardSimulation(im);

      for (j=0; j<nobs; j++) {
        obs_name=GetObservationDataName(j);
        /* Aufbau der Sensiviaetsmatrix */
        CalculateSensitivityParameterInverseIteration(obs_name,dx[i], \
                                                   &mat[i*nobs+j]);
      }
    } /* for i */

    for (j=0; j<nobs; j++) {
      obs_name=GetObservationDataName(j);
      /* Aufbau des Sensiviaetsvektors */
      CalculateRightSideSensitivityParameterInverseIteration(obs_name,&vecf[j]);
    }


    /* Aufbau der Gaus-Newton-Matrix */
    MTranspoMat(mat,nobs,niv,tmat);
    MMultMatMat(tmat,niv,nobs,mat,nobs,niv, ognmat,niv,niv);
    /* Aufbau der Gaus-Newton-Rechte-Seite */
    MMultMatVec(tmat,niv,nobs,vecf,nobs,ovecb,niv);       



    /* Levenberg-Marquardt Iterationszahl */
    lm_iter=0;

    /* Iteration Levenberg-Marquardt */
    do 
    {
      /* Matrix gnmat und Vektor vecb Kopieren */
      MKopierMat(ognmat,gnmat,niv,niv);
      MKopierVec(ovecb,vecb,niv);

      /* Levenberg-Marquardt Streckungsfaktor */
      for (i=0; i<niv; i++) {
        gnmat[i*niv+i] += lambda;
      }

      /* Loesen des Gaus-Newton-Levenberg-Marquardt-GLS */
      Gauss(gnmat,vecb,vecx ,niv);

      /* Bereich der neuen Werte pruefen und als aktuelle Werte setzen */
      for (i=0; i<niv; i++) {
        iv_name=GetInverseVariableName(i);
        new_values[i]=start_values[i]+vecx[i];
        if(new_values[i]<min_values[i]) new_values[i]=min_values[i];
        if(new_values[i]>max_values[i]) new_values[i]=max_values[i];
        SetCurrentInverseVariable(iv_name,new_values[i]);
      }

      /* Berechnung mit Verbesserten Werte */
      /* Vorwaertssimaltion */
      ExecuteForwardSimulation(im);

      /* Iterationen pruefen */
      inv_flag=ExecuteAllInverseIteration("INVERSE-ROCKFLOW",&new_value2_function);
      inv_flags = inv_flags||inv_flag;
      if(!inv_flags) break;

      /* modellspezifische Ergebnisse fuer Inverses Modellieren extra speichern */
      CopyToLastAllModelObservationData();

      feps=fabs(new_value2_function-new_value1_function);

      /* Neues Lambda berechnen und Levenberg-Marquardt-Iterationszahl erhoehen */
      /*if ( feps >= fabs(lambda_eps) && new_value2_function>new_value1_function ) {*/
      if ( new_value2_function>new_value1_function ) {
        lambda=lm_iter*fac1*start_lambda+fac2;
        lm_iter++;
      }
      else break;

    } /* Iteration Levenberg-Marquardt */
    while ( /*(feps>=fabs(lambda_eps)) &&*/ (lm_iter<=lm_max_iter) );


    /* Iteration Levenberg-Marquardt auf max. Iteration pruefen */
    if(lm_iter>lm_max_iter) { 
      DisplayMsgLn("");
      DisplayMsgLn("***********************************************");
      DisplayMsg("Inverses Problem: ");
      DisplayLong(lm_iter);
      DisplayMsgLn(" iterations. ");
      DisplayMsgLn(" !!! Levenberg-Marquardt Maximale Itartionszahl ");
      DisplayMsgLn(" !!! ist ueberschritten worden. ");
      DisplayMsgLn(" Programmabruch. ");
      DisplayMsgLn("***********************************************");
      DisplayMsgLn("");
      exit(1);
    }

    /* Iteration Levenberg-Marquardt ausgeben */
    /*if (feps<fabs(lambda_eps)) {
      PrintStartInverseIteration(lm_iter,"INVERSE-ROCKFLOW");
      PrintAllStartInverseVariable("INVERSE-ROCKFLOW");
      PrintStartInverseLSE(new_value2_function,"INVERSE-ROCKFLOW");
    }*/

    /* Gauss-Newton-Iteration schreiben */
    PrintStartInverseIteration(iter,"INVERSE-ROCKFLOW");
    PrintAllStartInverseVariable("INVERSE-ROCKFLOW");
    PrintStartInverseLSE(new_value2_function,"INVERSE-ROCKFLOW");

    iter++;
    iter_abs=fabs(new_value2_function-new_value1_function);
    new_value1_function=new_value2_function;
    /* Gestoerten Wert setzen */
    for (l=0; l<niv; l++) {
      start_values[l]=new_values[l];
    }
    if (iter<=1) continue;
  }
  while (iter_abs>=eps && iter<=max_iter);


  /* Speicherfreigabe */
  mat=(double *) Free(mat);
  tmat=(double *) Free(tmat);
  vecf=(double *) Free(vecf);

  gnmat=(double *) Free(gnmat);
  ognmat=(double *) Free(ognmat);
  vecb=(double *) Free(vecb);
  ovecb=(double *) Free(ovecb);
  vecx=(double *) Free(vecx);

  min_values=(double *) Free(min_values);
  max_values=(double *) Free(max_values);
  start_values=(double *) Free(start_values);
  coeff_values=(double *) Free(coeff_values);
  new_values=(double *) Free(new_values);
  dx=(double *) Free(dx);

  if(iter<=max_iter) { 
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
    DisplayMsg("Inverses Problem: ");
    DisplayLong(iter);
    DisplayMsgLn(" iterations. ");
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("");
  }
  else { 
    DisplayMsgLn("");
    DisplayMsgLn("***********************************************");
    DisplayMsg("Inverses Problem: ");
    DisplayLong(iter);
    DisplayMsgLn(" iterations. ");
    DisplayMsgLn(" !!! Maximale Itartionszahl ist ueberschritten worden !!!. ");
    DisplayMsgLn(" Programmabruch. ");
    DisplayMsgLn("***********************************************");
    DisplayMsgLn("");
    exit(1);
  }


}




/*------------------------------------------------------------------------*/
/* Lesefunktionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctInverseMethod
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #INVERSE_METHOD gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.
   #INVERSE_METHOD: Iterations-Parameter fuer alle Modelle.
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
int FctInverseMethod(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    double d;
    long l;
    int i=0,k=0;
    INVERSE_METHOD *im;
    int dec=1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #INVERSE_METHOD");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {           /* INVERSE_METHOD nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f, "; Schluesselwort: #");
      FilePrintString(f, keyword_inverse_method);
      LineFeed(f);
#endif
      if ( (im=create_inverse_method(DEFAULT_GROUP_NAME)) == NULL ) {
        ok = 0;
      }
      else {
        set_im_name(im,DEFAULT_NAME);      /* ID */
        set_im_type(im,0);      /* Standard */
        set_im_maxiter(im,1000);/* Max. Anzahl an Iterationen */
        set_im_criterium(im,2); /* Kriterium */
        set_im_norm(im,k);
        set_im_abs_eps(im,1.e-6);      /* abs. Fehlerschranke */
        set_im_rel_eps(im,1.e-3);      /* rel. Fehlerschranke */
        set_im_kind(im,0);      /* Immer gueltig */
        /* IM's-Name deklarieren */
        declare_inverse_method_name(DEFAULT_GROUP_NAME);
        /* Objekt in der Liste einfuegen */
        insert_inverse_method_list(im);
      }
    } /* found */
    else {                /* ITERATION_PRESSURE gefunden */
      FilePrintString(f, "#");
      FilePrintString(f, keyword_inverse_method);
      LineFeed(f);
#ifdef EXT_RFD
      FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein");
      LineFeed(f);
#endif
        while (StrTestHash(&data[p], &pos)) {
          i++;
          LineFeed(f);
          while (StrTestLong(&data[p+=pos])) {
            if ( (im=create_inverse_method("NONE")) == NULL ) {
              ok = 0;
              break;
            }
            else {
              set_im_group_name(im,DEFAULT_GROUP_NAME);                            
              ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
              set_im_type(im,l);
            }

            if ( get_im_type(im) == 0 || get_im_type(im) == 1 ||
                 get_im_type(im) == 2 || get_im_type(im) == 3 ||
                 get_im_type(im) == 4 || get_im_type(im) == 5 ||
                 get_im_type(im) == 10 ) {
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_im_maxiter(im,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_abs_eps(im,d);      /* abs. Fehlerschranke */
                LineFeed(f);
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, abs error tolerance, time control"); 
        LineFeed(f);
#endif
            }
            else if ( get_im_type(im) == 11 ) {
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_im_maxiter(im,l);      
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_abs_eps(im,d);     
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_im_second_maxiter(im,l);     
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_lambda_abs_eps(im,d);      
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_start_lambda(im,d);      
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_lambda_fac1(im,d);      
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_im_lambda_fac2(im,d);      
                LineFeed(f);
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, abs error tolerance, time control"); 
        LineFeed(f);
#endif
        }



            if (im) {
              if (dec) declare_inverse_method_name(DEFAULT_GROUP_NAME);
              else dec=0;
              insert_inverse_method_list(im);
            }
            LineFeed(f);
          } /* while long */
        }  /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Inverse-Methode (INVERSE_METHOD) [0..2;0]");
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
/* ROCKFLOW - Funktion: ConfigInverseMethod
                                                                          */
/* Aufgabe:
   Konfigurationen der inversen Methoden (IM):
   - Namen fuer IM Objekte fuer alle Feldgroessen
     (entsprechend der Anzahl von Phasen und Komponenten)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   10/2001    AH     Erste Version
                                                                          */
/**************************************************************************/
void ConfigInverseMethod(void)
{
  /*--------------------------------------------------------------------*/
  /* Alle noetigen Inverse-Objekte Konfigurieren */
  InitAllInverseVariable();
  SetAllStartInverseVariable();

  SetAllObservationData();
  ConfigAllObservationData();        
  InitAllObservationData();

  ConfigInverseIteration("ROCKFLOW");
  InitAllInverseIteration();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteForwardSimulation
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INVERSE_METHOD *im: Zeiger auf die Datenstruktur im.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteForwardSimulation (INVERSE_METHOD *im)
{
  static int first = 1;
  if (get_im_time_loop_function(im)) {
    if (GetRFControlGridAdapt() && !first) {
      DestroyAllNewElementsGridAdaptation();
      ReinitGridAdaptation();
      first = 0;
    }
//OK_TIM    ConfigTime();
//OK_MOD    InitModel();
    get_im_time_loop_function(im)();
  }
}
