/**************************************************************************/
/* ROCKFLOW - Modul: rfsystim.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps SYSTEM_TIME
   (Systemzeit).
                                                                          */
/**************************************************************************/


#ifndef rfsystim_INC

#define rfsystim_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


/* ================================================================ */
/* SYSTEM_TIME */
/* ================================================================ */


/* Typdeklaration TIME */
#ifndef _SYSTEM_TIME_DEFINED
  typedef double TIME_T;
  typedef time_t sys_time_t;
#define _SYSTEM_TIME_DEFINED
#endif


typedef struct {
    char *name;
    char *group_name;

    long id;
    char *text;

    TIME_T start_time;
    TIME_T begin_time;
    TIME_T current_time;
    TIME_T stop_time;
    TIME_T total_time;
    TIME_T pause_time;
    TIME_T time;
    int running_flag;

    long type;
} SYSTEM_TIME;



/* Erzeugt eine Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *create_system_time(char *name);
/* Zerstoert die Instanz vom Typ SYSTEM_TIME */
extern void destroy_system_time(void *member);

/* Loescht die Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *delete_system_time(SYSTEM_TIME *st);
/* Startet die Systemzeit der Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *start_system_time(SYSTEM_TIME *st);
/* Fuehrt die Systemzeit der Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *run_system_time(SYSTEM_TIME *st);
/* Stoppt die Systemzeit der Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *stop_system_time(SYSTEM_TIME *st);
/* Gibt die Systemzeit einer Instanz vom Typ SYSTEM_TIME aus */
extern SYSTEM_TIME *print_system_time(SYSTEM_TIME *st);

/* Berechnet die ausgelaufene Systemzeit der Instanz vom Typ SYSTEM_TIME */
extern SYSTEM_TIME *calculate_total_system_time(SYSTEM_TIME *st);

/* Setzt Name des ST's */
extern void set_st_name(SYSTEM_TIME *st, char *name);
/* Liefert Name des ST's */
extern char *get_st_name(SYSTEM_TIME *st);
/* Setzt Name des ST's */
extern void set_st_group_name(SYSTEM_TIME *st, char *group_name);
/* Liefert Name des ST's */
extern char *get_st_group_name(SYSTEM_TIME *st);
/* Setzt Text des ST's */
extern void set_st_text(SYSTEM_TIME *st, char *name);
/* Liefert Text des ST's */
extern char *get_st_text(SYSTEM_TIME *st);
/* Setzt id des ST's */
void set_st_id(SYSTEM_TIME *st, long id);
/* Liefert id des ST's */
long get_st_id(SYSTEM_TIME *st);

/* Setzt die Zeit der Gueltigkeit des ST's */
void set_st_time(SYSTEM_TIME *lsp, TIME_T time);
/* Liefert die Zeit der Gueltigkeit des ST's */
TIME_T get_st_time(SYSTEM_TIME *lsp);
/* Setzt Anfangssystemzeit des ST's */
void set_st_start_time(SYSTEM_TIME *st, TIME_T start_time);
/* Liefert id des ST's */
TIME_T get_st_start_time(SYSTEM_TIME *st);
/* Setzt Anfangssystemzeit des ST's */
void set_st_begin_time(SYSTEM_TIME *st, TIME_T begin_time);
/* Liefert id des ST's */
TIME_T get_st_begin_time(SYSTEM_TIME *st);


/* Setzt Typ des ST's */
void set_st_type(SYSTEM_TIME *st, long type);
/* Liefert Typ des ST's */
long get_st_type(SYSTEM_TIME *st);



/* =======================================================================*/
/* Verwaltung
                                                                          */
/* =======================================================================*/

typedef struct {
    char *name;
    LList *st_list;
    long count_of_system_time;
    char **names_of_system_time;
    long count_of_system_time_name;
    double *st_vector;
} LIST_SYSTEM_TIME;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_SYSTEM_TIME *create_system_time_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_system_time_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int system_time_list_empty(void);

/* Bekanntmachung der Iteration mit dem Name name. */
extern long declare_system_time_name (char *name);
/*   Alle Iterationen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_system_time_names (void);

/* Fuegt eine Instanz vom Typ SYSTEM_TIME in die Liste */
extern long insert_system_time_list (SYSTEM_TIME *st);

/* Prueft auf Existenz eines Namens */
long system_time_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateSystemTimeList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroySystemTimeList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int SystemTimeListEmpty(void);
  /* Spaltenindex der IP-Objekte mit dem Namen (name) */
long IndexSystemTime(char *name);


/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern SYSTEM_TIME *GetSystemTimeGroup(char *name,SYSTEM_TIME *st);
  /* Zerstoert saemtliche SYSTEM_TIME-Objekte der Gruppe NAME */
extern int DestroySystemTimeListGroup(char *name);


/* Methoden ausfuehren */


/* Zugriffsfunktion */
SYSTEM_TIME *GetSystemTime (char *name);

/* Hilfsfunktionenen */
double CalculateMaxSystemTime (char *group_name);

/* Ausgabefunktionen */
int StatisticsSystemTime(char *group_name);


/*========================================================================*/
/* Schnittstellenfunktionen zwischen RFSYSTIM und Rockflow                           */
/*========================================================================*/

/* ST-Objekt mit dem Namen (name) konfigurieren */
extern void ConfigSystemTime(char *group_name);

/* ST-Objekt mit dem Namen (name) konfigurieren */
extern int SetSystemTime(char *name,char *group_name,char *text, long *id);
/* ST-Objekt mit dem Namen (name) ausfuehren */
extern int RunSystemTime(char *name);
/* ST-Objekt mit dem Namen (name) stoppen */
extern int StopSystemTime(char *name);
/* ST-Objekt mit dem Namen (name) ausgeben */
extern int PrintSystemTime(char *name);


#endif

