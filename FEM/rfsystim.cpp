/**************************************************************************/
/* ROCKFLOW - Modul: rfsystim.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp SYSTEM_TIME
   - Verwaltung von SYSTEM_TIME mit dem Datentyp LIST_SYSTEM_TIME
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
   01/2000     AH         StrUp funktion entfernt.
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
      


   last modified: RK 12.03.2003
                                                                          */
/**************************************************************************/

#include "stdafx.h"

/* Preprozessor-Definitionen */
#include "makros.h"

/* Used tools */
#include "mathlib.h"
#include "geo_strings.h"

/* Used objects */
#include "rf_pcs.h" //OK_MOD"

/* Interface */
#include "rfsystim.h"

//OK_IC #include "rfbc.h" /* DMATRIX ToDo OK 3261 */

  /* Liste von Randbedingungen */
static LIST_SYSTEM_TIME *list_of_system_time = NULL;

/* Nur fuer den Interface RF-BC */
#include "tools.h"
//static DMATRIX *st_matrix_values = NULL;
//static LMATRIX *st_matrix_iter = NULL;
static char **st_names = NULL;
static long st_names_number = 0;
/*static long st_cur_col = -1;*/



/* ================================================================ */
/* SYSTEM_TIME */
/* ================================================================ */



/* Systemnamen */
/*static char *sys_name_system_time=NULL;*/


/* Interne System-Funktionen */
TIME_T _get_system_time(void);



/**************************************************************************/
/* ROCKFLOW - Funktion: stop_system_time
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
TIME_T _get_system_time(void)
{
  sys_time_t t=0;
  time(&t);

  return (TIME_T)t;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: create_system_time
                                                                          */
/* Aufgabe:
   Konstruktor for SYSTEM_TIME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *create_system_time(char *name)
{
  SYSTEM_TIME *st;

  st = (SYSTEM_TIME *) Malloc(sizeof(SYSTEM_TIME));
  if ( st == NULL ) return NULL;


  st->name = (char *) Malloc((int)strlen(name)+1);
  if ( st->name == NULL ) {
         Free(st);
         return NULL;
  }
  strcpy(st->name,name);

  st->text=NULL;
  st->group_name=NULL;

  st->start_time=0;
  st->begin_time=0;
  st->current_time=0;
  st->stop_time=0;
  st->total_time=0;
  st->pause_time=0;
  st->time=0;
  st->running_flag=0;

  st->type=0;

  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_system_time
                                                                          */
/* Aufgabe:
   Destructor for SYSTEM_TIME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des ST's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_system_time(void *member)
{
  SYSTEM_TIME *st=(SYSTEM_TIME *)member;

  if (st->text) st->text=(char *)Free(st->text);
  if ( st->group_name ) st->group_name=(char *)Free(st->group_name);
  if ( st->name ) st->name=(char *)Free(st->name);

  if ( st ) st=(SYSTEM_TIME *)Free(st);

  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_system_time
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *delete_system_time(SYSTEM_TIME *st)
{
  st->start_time=0;
  st->begin_time=0;
  st->current_time=0;
  st->stop_time=0;
  st->total_time=0;
  st->time=0;
  st->running_flag=0;

  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: start_system_time
                                                                          */
/* Aufgabe:
   Systemzeit loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *start_system_time(SYSTEM_TIME *st)
{
  TIME_T t;

  t=_get_system_time();
  st->start_time=t;
  st->current_time=t;
  st->begin_time=t;
  st->time=0;
  st->running_flag=0;

  return st;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: run_system_time
                                                                          */
/* Aufgabe:
   Systemzeit loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *run_system_time(SYSTEM_TIME *st)
{
  TIME_T t;

  if (st->running_flag) return st;

  t=_get_system_time();
  st->current_time=t;
  st->begin_time=t;
  st->running_flag=1;

  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: stop_system_time
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *stop_system_time(SYSTEM_TIME *st)
{
  TIME_T t;

  if(!st->running_flag) return st;

  t=_get_system_time();
  st->stop_time=t;
  st->current_time=t;

  st->time += st->stop_time-st->begin_time;

  st->running_flag=0;

  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: print_system_time
                                                                          */
/* Aufgabe:
   Systemzeit ausgeben.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *print_system_time(SYSTEM_TIME *st)
{
  char string[256];

  if (st->name) {
    sprintf (string,"%s: %s %e s\n",st->name,st->text,st->time);
    DisplayMsgLn(string);
  }
  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: calculate_total_system_time
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Adresse des ST's.
                                                                          */
/* Ergebnis:
   - Adresse des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *calculate_total_system_time(SYSTEM_TIME *st)
{
  st->total_time += st->time;

  return st;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_name
                                                                          */
/* Aufgabe:
   Setzt Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
   E char *name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_name(SYSTEM_TIME *st, char *name)
{
  if (!name) return;
  if(st) {
    st->name=(char *)Free(st->name);
    st->name = (char *) Malloc((int)strlen(name)+1);
    if ( st->name == NULL ) return;
    strcpy(st->name,name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_name
                                                                          */
/* Aufgabe:
   Liefert Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_st_name(SYSTEM_TIME *st)
{
  if (st->name)
    return st->name;
  else
        return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_group_name
                                                                          */
/* Aufgabe:
   Setzt Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
   E char *group_name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_group_name(SYSTEM_TIME *st, char *group_name)
{
  if (!group_name) return;
  if(st) {
    st->group_name=(char *)Free(st->group_name);
    st->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( st->group_name == NULL ) return;
    strcpy(st->group_name,group_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_group_name
                                                                          */
/* Aufgabe:
   Liefert Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_st_group_name(SYSTEM_TIME *st)
{
  if (st->group_name) return st->group_name;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_text
                                                                          */
/* Aufgabe:
   Setzt Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
   E char *text: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_text(SYSTEM_TIME *st, char *text)
{
  if (!text) return;
  if(st) {
    st->text=(char *)Free(st->text);
    st->text = (char *) Malloc((int)strlen(text)+1);
    if ( st->text == NULL ) return;
    strcpy(st->text,text);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_text
                                                                          */
/* Aufgabe:
   Liefert Name des ST's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des ST's -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_st_text(SYSTEM_TIME *st)
{
  if (st->text) return st->text;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_id
                                                                          */
/* Aufgabe:
   Setzt Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_id(SYSTEM_TIME *st, long id)
{
  if (st) st->id=id;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_id
                                                                          */
/* Aufgabe:
   Liefert Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_st_id(SYSTEM_TIME *st)
{
  return (st)? st->id:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_time
                                                                          */
/* Aufgabe:
   Setzt die Zeit der Gueltigkeit des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf eine Instanz vom Typ
                                        SYSTEM_TIME.
   E double time: Zeit der Gueltigkeit des Loesers
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_time(SYSTEM_TIME *st, TIME_T time)
{
  if (st) st->time=time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_time
                                                                          */
/* Aufgabe:
   Liefert die Zeit der Gueltigkeit des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf eine Instanz vom Typ
                                        SYSTEM_TIME.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
TIME_T get_st_time(SYSTEM_TIME *st)
{
  return (st)?st->time:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_start_time
                                                                          */
/* Aufgabe:
   Setzt Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
   E TIME_T start_time: Anfangssystemzeit starten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_start_time(SYSTEM_TIME *st, TIME_T start_time)
{
  if (st) st->start_time=start_time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_start_time
                                                                          */
/* Aufgabe:
   Liefert Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
TIME_T get_st_start_time(SYSTEM_TIME *st)
{
  return (st)? st->start_time:0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_begin_time
                                                                          */
/* Aufgabe:
   Setzt Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_begin_time(SYSTEM_TIME *st, TIME_T begin_time)
{
  if (st) st->begin_time=begin_time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_begin_time
                                                                          */
/* Aufgabe:
   Liefert Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur st.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
TIME_T get_st_begin_time(SYSTEM_TIME *st)
{
  return (st)? st->begin_time:0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_st_type
                                                                          */
/* Aufgabe:
   Setzt Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_st_type(SYSTEM_TIME *st, long type)
{
  if (st) st->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_st_type
                                                                          */
/* Aufgabe:
   Liefert Typ des ST's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_st_type(SYSTEM_TIME *st)
{
        return (st)?st->type:-1;
}



/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_system_time_list
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
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
LIST_SYSTEM_TIME *create_system_time_list (char *name)
{
  LIST_SYSTEM_TIME *list_of_system_time;

  list_of_system_time = (LIST_SYSTEM_TIME *) Malloc(sizeof(LIST_SYSTEM_TIME));
  if ( list_of_system_time == NULL ) return NULL;


  list_of_system_time->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_system_time->name == NULL ) {
         Free(list_of_system_time);
         return NULL;
  }
  strcpy(list_of_system_time->name,name);

  list_of_system_time->st_list=create_list();
  if ( list_of_system_time->st_list == NULL ) {
    Free(list_of_system_time->name);
        Free(list_of_system_time);
        return NULL;
  }

  list_of_system_time->names_of_system_time=NULL;
  list_of_system_time->count_of_system_time_name=0;

  return list_of_system_time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_system_time_list
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
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_system_time_list(void)
{
  if (list_of_system_time->name) list_of_system_time->name=(char *)Free(list_of_system_time->name);
  if(list_of_system_time->names_of_system_time) \
    list_of_system_time->names_of_system_time = \
      (char **)Free(list_of_system_time->names_of_system_time);

  if (list_of_system_time->st_list) {
    delete_list(list_of_system_time->st_list,destroy_system_time);
    list_of_system_time->st_list=destroy_list(list_of_system_time->st_list);
  }

  if (list_of_system_time) list_of_system_time=(LIST_SYSTEM_TIME *)Free(list_of_system_time);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: system_time_list_empty
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
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int system_time_list_empty(void)
{
  if (!list_of_system_time) return 0;
  else if (!list_of_system_time->st_list) return 0;
  else return list_empty(list_of_system_time->st_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_st_init
                                                                          */
/* Aufgabe:
   Liste der ST's initialisieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void list_st_init(void)
{
  if (list_of_system_time) list_current_init(list_of_system_time->st_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_st_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_st_next (void)
{
  return list_of_system_time->st_list ?
             get_list_next(list_of_system_time->st_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_system_time_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ SYSTEM_TIME erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen ST's_Namen -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_system_time_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_system_time) return 0;
  for ( i=0; i<list_of_system_time->count_of_system_time_name; i++)
    if(strcmp(list_of_system_time->names_of_system_time[i],name) == 0) break;
  if (i < list_of_system_time->count_of_system_time_name) return i;

  list_of_system_time->names_of_system_time= \
          (char **) Realloc(list_of_system_time->names_of_system_time, \
          (list_of_system_time->count_of_system_time_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_system_time->names_of_system_time[i]=new_name;

  return ++(list_of_system_time->count_of_system_time_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_system_time_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen ST's_Namen -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_system_time_names (void)
{
  long i;

  for (i=0; i<list_of_system_time->count_of_system_time_name; i++)
    if(list_of_system_time->names_of_system_time[i]) \
      list_of_system_time->names_of_system_time[i] = \
        (char *)Free(list_of_system_time->names_of_system_time[i]);

  return list_of_system_time->count_of_system_time_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_system_time_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ SYSTEM_TIME in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_system_time_list (SYSTEM_TIME *st)
{
  if(!list_of_system_time) return 0;
  list_of_system_time->count_of_system_time=append_list(list_of_system_time->st_list,(void *) st);
  return list_of_system_time->count_of_system_time;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_system_time_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ SYSTEM_TIME der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender SYSTEM_TIME-Objekte
                                                                          */
/* Programmaenderungen:
   10/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
long remove_system_time_list (SYSTEM_TIME *st)
{
  list_of_system_time->count_of_system_time = \
      remove_list_member(list_of_system_time->st_list,(void *) st, destroy_system_time);
  return list_of_system_time->count_of_system_time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: system_time_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
   11/2002   OK   test if list_of_system_time exists
                                                                          */
/**************************************************************************/
long system_time_name_exist (char *name)
{
  long i;

  if (!name) return 0;
  if (!list_of_system_time) return 0;

  for ( i=0; i<list_of_system_time->count_of_system_time_name; i++)
    if(strcmp(list_of_system_time->names_of_system_time[i],name) == 0) break;
  if (i < list_of_system_time->count_of_system_time_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateSystemTimeList
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
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void CreateSystemTimeList(void)
{
  list_of_system_time = create_system_time_list("SYSTEM_TIME");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroySystemTimeList
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
   10/1999    AH      Erste Version
   10/2000    AH      st_names konvertieren und nullen. ah rfm
                      st_names_number zu Null setzen.

                                                                          */
/**************************************************************************/
void DestroySystemTimeList(void)
{
  long i;

  //OK_BC if (st_matrix_values) DestroyDoubleMatrix(st_matrix_values);
  //OK_BC if (st_matrix_iter) DestroyLongMatrix(st_matrix_iter);
  /* ah zv */
  for(i=0; i<st_names_number; i++)
    if (st_names[i]) Free(st_names[i]);
  if (st_names) st_names=(char **)Free(st_names);
  st_names_number=0;

  undeclare_system_time_names();
  destroy_system_time_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SystemTimeListEmpty
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
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int SystemTimeListEmpty(void)
{
  return system_time_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigSystemTime
                                                                          */
/* Aufgabe:
   Konfiguriert den Systemzeit-Gruppennamen. Es wird nicht geprueft ob
   den Namen schon existiert.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void ConfigSystemTime(char *name)
{
  char *new_name;

  st_names=(char **) Realloc(st_names,(st_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  st_names[st_names_number]=new_name;
  st_names[++st_names_number]=NULL;

  return;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: IndexSystemTime
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des ST's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long IndexSystemTime(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<st_names_number; i++)
    if(strcmp(st_names[i],name) == 0) break;
  if (i < st_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetSystemTimeGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp SYSTEM_TIME
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: SYSTEM_TIME *st -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp SYSTEM_TIME
                                                                          */
/* Programmaenderungen:
   10/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *GetSystemTimeGroup(char *name,SYSTEM_TIME *st)
{
  if (!name) return NULL;
  if (!system_time_name_exist(name) ) return NULL;

  if(st==NULL) list_st_init();

  while ( (st=(SYSTEM_TIME *)get_list_st_next()) != NULL ) {
    if ( strcmp(get_st_name(st),name)==0 ) break;
  }

  return st;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroySystemTimeListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche SYSTEM_TIME-Objekte der Gruppe NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name - Gruppenname
                                                                          */
/* Ergebnis:
   Kontrollflag
                                                                          */
/* Programmaenderungen:
   10/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
int DestroySystemTimeListGroup(char *name)
{
  SYSTEM_TIME *st=NULL;

  if (!name) return -1;
  if (!system_time_name_exist(name) ) return -1;

  list_st_init();

  while ( (st=(SYSTEM_TIME *)get_list_st_next()) != NULL ) {
    if ( strcmp(get_st_name(st),name)==0 )
        remove_system_time_list(st);
  }

  return 0;
}





/*========================================================================*/
/* Extras                                                                 */
/*========================================================================*/


/**************************************************************************/
/* ROCKFLOW - Funktion: GetSystemTime
                                                                          */
/* Aufgabe:
   Liefert die linearen Loeser-Eigenschaften fuer die Gruppe name.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
SYSTEM_TIME *GetSystemTime (char *name)
{
  SYSTEM_TIME *st=NULL;

  if (!name) return NULL;
  if (SystemTimeListEmpty()) return NULL;

  list_st_init();
  while ( (st=(SYSTEM_TIME *)get_list_st_next()) != NULL ) {
    if ( strcmp(get_st_name(st),name)==0 )
    return st;
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalculateMaxSystemTime
                                                                          */
/* Aufgabe:
   Liefert die linearen Loeser-Eigenschaften fuer die Gruppe name.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double CalculateMaxSystemTime (char *group_name)
{
  SYSTEM_TIME *st=NULL;
  TIME_T time=0,max_time=0;
  double MaxTime=0;

  if (!group_name) return 0.;
  if (SystemTimeListEmpty()) return 0.;

  list_st_init();
  while ( (st=(SYSTEM_TIME *)get_list_st_next()) != NULL ) {
    if ( strcmp(get_st_group_name(st),group_name)==0 ) {
      time=get_st_time(st);
      if (time > max_time) max_time=time;
    }
  }
  MaxTime=(double)max_time;

  return MaxTime;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StatisticsSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int StatisticsSystemTime(char *group_name)
{
  SYSTEM_TIME *st=NULL;
  TIME_T t=0;
  double MaxTime=0.;
  static char string[1000];

  if (!group_name) return 0;
  if(IndexSystemTime(group_name) < 0) return 0;

  MaxTime=CalculateMaxSystemTime(group_name);

  if (fabs(MaxTime)< MKleinsteZahl) return 0;

  DisplayMsgLn("");
  sprintf (string,"************* Summary of the times needed in the system *************");
  DisplayMsgLn(string);
  sprintf (string,"                          %s",group_name);
  DisplayMsgLn(string);
  sprintf (string,"--------------------------------------------------------------");
  DisplayMsgLn(string);
  list_st_init();
  while ( (st=(SYSTEM_TIME *)get_list_st_next()) != NULL ) {
    if ( strcmp(get_st_group_name(st),group_name)==0 ) {
      t=get_st_time(st);
      /*sprintf (string,"|%-9s | %-21s | %10.0f s | %6.2f ",st->name,st->text,(double)t,(double)t/MaxTime*100.);*/
      sprintf (string,"|%-33s | %10.0f s | %6.2f ",st->text,(double)t,(double)t/MaxTime*100.);
      strcat (string,"% |");
      DisplayMsgLn(string);
      sprintf (string,"--------------------------------------------------------------");
      DisplayMsgLn(string);
    }
  }
  sprintf (string,"**************************************************************");
  DisplayMsgLn(string);

  return 1;
}



/*========================================================================*/
/* Schnittstellenfunktionen zwischen RFSYSTIM und Rockflow                           */
/*========================================================================*/


/**************************************************************************/
/* ROCKFLOW - Funktion: SetSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int SetSystemTime(char *name, char *group_name, char *text, long *id)
{
  SYSTEM_TIME *st=NULL;

  if ( system_time_name_exist(name) ){
    /*printf("!!! timer %s already exist !!!",name);*/
    return 0;
  }

  st=create_system_time(name);
  if (st) {
    set_st_text(st,text);
    set_st_group_name(st,group_name);
    *id=insert_system_time_list(st);
    set_st_id(st,*id);
    declare_system_time_name(name);
  }

  if (st) return 1;
  else return 0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: StartSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int StartSystemTime(char *name)
{
  SYSTEM_TIME *st=NULL;

  if (!st) return 0;

  if ( !system_time_name_exist(name) ){
    printf("!!! timer %s not exist !!!",name);
    return 0;
  }

  if( (st=GetSystemTime(name)) != NULL) start_system_time(st);
  else return 0;

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RunSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int RunSystemTime(char *name)
{
  SYSTEM_TIME *st=NULL;

  if (!name) return 0;

  if ( !system_time_name_exist(name) ){
    printf("!!! timer %s not exist !!!",name);
    return 0;
  }

  if( (st=GetSystemTime(name)) != NULL) run_system_time(st);
  else return 0;

  return 1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: StopSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int StopSystemTime(char *name)
{
  SYSTEM_TIME *st=NULL;
  /*TIME_T t=0;*/

  if (!name) return 0;

  if ( !system_time_name_exist(name) ){
    printf("!!! timer %s not exist !!!",name);
    return 0;
  }

  if( (st=GetSystemTime(name)) != NULL) stop_system_time(st);
  else return 0;

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PrintSystemTime
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E SYSTEM_TIME *st: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int PrintSystemTime(char *name)
{
  SYSTEM_TIME *st=NULL;
  /*TIME_T t=0;*/

  if (!name) return 0;

  if ( !system_time_name_exist(name) ){
    printf("!!! timer %s not exist !!!",name);
    return 0;
  }

  if( (st=GetSystemTime(name)) != NULL) print_system_time(st);
  else return 0;

  return 1;
}

