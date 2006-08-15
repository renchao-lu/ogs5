/**************************************************************************/
/* ROCKFLOW - Modul: rfidb.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps INPUT_DATABASE
   (Eingabe).
                                                                          */
/**************************************************************************/


#ifndef rfidb_INC

#define rfidb_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"
#include "rfstring.h"
#include "rfdb.h"

/* Prototypen */
typedef int (*Read_Function) (char *, int, FILE *);


/* ================================================================ */
/* Datentyp : INPUT_DATABASE                                                 */
/* ================================================================ */


typedef struct {
    char *master_keyword;         /* Master-Schluesselwort des Datums */ 
    /*char **keywords;*/              /* Schluesselwoerter */ 
    char *keyword;                 /* Schluesselwort */ 
    long count_of_keywords;       /* Anzahl der Schluesselwoerter */
    String *S;                    /* Inhalt als Zeichenkette */
    char *group_name;             /* Gruppenzughoerigkeit */
    int found;
    char *sep_string;           /* Trenn-Zeichen beim Mehrfachvorkommen */
    char *comments;             /* Komentar-Zeichen */
    char *beginn_comments;      /* Komentar-Beginn-Zeichen fuer Blockkommentar */
    char *end_comments;         /* Komentar-End-Zeichen fuer Blockkommentar */
    char *sep_cap;              /* Trenn-Zeichen beim Mehrfachvorkommen (Lesen) */

    Read_Function read_function;  /* Auswertefunktion */
    char *file_name;
    FILE *fp;
} INPUT_DATABASE;



/* Erzeugt eine Instanz vom Typ INPUT_DATABASE */
extern INPUT_DATABASE *create_idb_database(char *group_name, \
            char *master_keyword,  Read_Function read_function, \
            char *sep_string, char *comments, char *begin_comments, char *end_comments, char *sep_cap);
/* Zerstoert die Instanz vom Typ INPUT_DATABASE */
extern void destroy_idb_database(void *member);


/* Setzt Schluesselwort des INP's */
extern void set_idb_master_keyword(INPUT_DATABASE *idb, char *keyword);
/* Liefert Schluesselwort des INP's */
extern char *get_idb_master_keyword(INPUT_DATABASE *idb);
/* Setzt Schluesselwort des INP's */
extern void set_idb_keyword(INPUT_DATABASE *idb, char *keyword);
/* Liefert Schluesselwort des INP's */
extern char *get_idb_keyword(INPUT_DATABASE *idb);
/* Setzt Gruppenname des INP's */
extern void set_idb_group_name(INPUT_DATABASE *idb, char *group_name);
/* Liefert Gruppenname des INP's */
extern char *get_idb_group_name(INPUT_DATABASE *idb);
/* Setzt Found-Modus des INP's */
extern void set_idb_found(INPUT_DATABASE *idb, int found);
/* Liefert Found-Modus des INP's */
extern int get_idb_found(INPUT_DATABASE *idb);
/* Setzt Zeichenkette des INP's */
extern void set_idb_S(INPUT_DATABASE *idb, String *S);
/* Liefert Zeichenkette des INP's */
extern String *get_idb_S(INPUT_DATABASE *idb);
/* Setzt Kommentar-Zeichen des INP's */
extern void set_idb_sep_string(INPUT_DATABASE *idb, char *sep_string);
/* Liefert Kommentar-Zeichen des INP's */
extern char *get_idb_sep_string(INPUT_DATABASE *idb);
/* Setzt Kommentar-Zeichen des INP's */
extern void set_idb_comments(INPUT_DATABASE *idb, char *comments);
/* Liefert Kommentar-Zeichen des INP's */
extern char *get_idb_comments(INPUT_DATABASE *idb);
/* Setzt Kommentar-Beginn-Zeichen des INP's */
extern void set_idb_begin_comments(INPUT_DATABASE *idb, char *begin_comments);
/* Liefert Kommentar-Beginn-Zeichen des INP's */
extern char *get_idb_begin_comments(INPUT_DATABASE *idb);
/* Setzt Kommentar-End-Zeichen des INP's */
extern void set_idb_end_comments(INPUT_DATABASE *idb, char *end_comments);
/* Liefert Kommentar-End-Zeichen des INP's */
extern char *get_idb_end_comments(INPUT_DATABASE *idb);
/* Setzt Kommentar-Zeichen des INP's */
extern void set_idb_sep_cap(INPUT_DATABASE *idb, char *sep_cap);
/* Liefert Kommentar-Zeichen des INP's */
extern char *get_idb_sep_cap(INPUT_DATABASE *idb);

/* Setzt Auswertefunktion des INP's */
extern void set_idb_read_function(INPUT_DATABASE *idb, Read_Function read_function);
/* Liefert Auswertefunktion des INP's */
extern Read_Function get_idb_read_function(INPUT_DATABASE *idb);
/* Setzt Datei-Zeiger des INP's */
extern void set_idb_fp(INPUT_DATABASE *idb, FILE *fp);
/* Liefert Datei-Zeiger des INP's */
extern FILE *get_idb_fp(INPUT_DATABASE *idb);
/* Setzt Datei-Name des IDB's */
extern void set_idb_file_name(INPUT_DATABASE *idb, char *file_name);
/* Liefert Datei-Name des IDB's */
extern char *get_idb_file_name(INPUT_DATABASE *idb);

/* ================================================================ */
/* Datentyp : LIST_INPUT_DATABASE                                            */
/* ================================================================ */

typedef struct {
    char *name;
    LList *idb_list;
    long count_of_idb_database;
    char **names_of_idb_database;
    long count_of_idb_database_name;
    double *idb_vector;
} LIST_INPUT_DATABASE;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_INPUT_DATABASE *create_idb_database_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_idb_database_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int idb_database_list_empty(void);

/* Bekanntmachung der Iteration mit dem Name name. */
extern long declare_idb_database_name (char *name);
/*   Alle Iterationen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_idb_database_names (void);

/* Fuegt eine Instanz vom Typ INPUT_DATABASE in die Liste */
extern long insert_idb_database_list (INPUT_DATABASE *idb);

/* Prueft auf Existenz eines Namens */
long idb_database_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateInputDataBaseList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyInputDataBaseList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int InputDataBaseListEmpty(void);
  /* Spaltenindex der IP-Objekte mit dem Namen (name) */
long IndexInputDataBase(char *name);


/* IDB-Modell */

/* Alle IP-Objekte mit dem Namen (name) konfigurieren */
void SetInputDataBase(char *name);


/* Initialisierung */
INPUT_DATABASE *InitInputDataBase(char *group_name);
/* Lesen */
INPUT_DATABASE *ReadInputDataBase(char *group_name);
/* Ausfuehren */
INPUT_DATABASE *ExecuteInputDataBase(char *group_name);


/* Schnittstellenfunktion zu Rockflow */

extern int SetSectionInputDataBase(char *db_name, /* db_name == sys_name */ 
         char *master_keyword,  Read_Function read_function);

extern int SetSectionInputDataBaseEx(char *db_name, 
         char *master_keyword,  Read_Function read_function, 
         char *sep_string, char *comments, char *beginn_comments, char *end_comments, char *sep_cap);

extern int SetSectionPropertiesDataBase(char *sep_string, char *comments, \
                               char *beginn_comments, char *end_comments, \
                               char *beginn_section, char *end_section, char *sep_cap);

extern int InitInputFile(char *db_name);
extern int ExecuteInputFile(char *db_name);

#endif

