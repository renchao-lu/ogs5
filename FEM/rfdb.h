/**************************************************************************/
/* ROCKFLOW - Modul: rfdb.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps DATABASE

                                                                          */
/**************************************************************************/


#ifndef rfdb_INC

#define rfdb_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"
#include "rfstring.h"


/* ================================================================ */
/* Datentyp : DATABASE                                                 */
/* ================================================================ */


typedef struct {
    char *name;
    String *S;
    FILE *fp;
    char *file_name;
    char *group_name;
    long max_size;
    int save_mode;

    FILE *pp;
    char *prot_name;

} DATABASE;



/* Erzeugt eine Instanz vom Typ DATABASE */
extern DATABASE *CreateDataBase(char *name, char *file_name, char *prot_name);
/* Zerstoert eine Instanz vom Typ DATABASE */
extern void DestroyDataBase(void *member);
/* Loescht den Inhalt einer Instanz vom Typ DATABASE */
extern void DeleteDataBase(DATABASE *db);


/* Setzt Namen (ID) des DB's */
extern void SetNameDataBase(DATABASE *db, char *name);
/* Liefert Namen (ID) des DB's */
extern char *GetNameDataBase(DATABASE *db);
/* Setzt Gruppennamen des DB's */
extern void SetGroupNameDataBase(DATABASE *db, char *group_name);
/* Liefert Gruppennamen des DB's */
extern char *GetGroupNameDataBase(DATABASE *db);
/* Setzt Datenamen des DB's */
extern void SetFileNameDataBase(DATABASE *db, char *group_name);
/* Liefert Datenamen des DB's */
extern char *GetFileNameDataBase(DATABASE *db);
/* Setzt Dateizeiger des DB's */
extern void SetFilePointeDataBase(DATABASE *db, FILE *fp);
/* Liefert Dateizeiger des DB's */
extern FILE *GetFilePointerDataBase(DATABASE *db);
/* Setzt Datenbank-Groesse des DB's */
extern void SetMaxSizeDataBase(DATABASE *db, long max_size);
/* Liefert Datenbank-Groesse des DB's */
extern long GetMaxSizeDataBase(DATABASE *db);
/* Setzt Speicherungsmodus des DB's */
extern void SetSaveModeDataBase(DATABASE *db, int save_mode);
/* Liefert Speicherungsmodus des DB's */
extern int GetSaveModeDataBase(DATABASE *db);
/* Setzt Protokoll-Datenamen des DB's */
extern void SetProtNameDataBase(DATABASE *db, char *group_name);
/* Liefert Protokoll-Datenamen des DB's */
extern char *GetProtNameDataBase(DATABASE *db);
/* Setzt Protokoll-Dateizeiger des DB's */
extern void SetProtPointeDataBase(DATABASE *db, FILE *pp);
/* Liefert Protokoll-Dateizeiger des DB's */
extern FILE *GetProtPointerDataBase(DATABASE *db);

/* Eroeffnet eine Datenbank vom Typ DATABASE */
extern DATABASE *OpenFileDataBase(DATABASE *db);
/* Schliesst eine Datenbank vom Typ DATABASE */
extern DATABASE *CloseFileDataBase(DATABASE *db);
/* Lese-Zeiger auf Datenbank-Anfang positionieren */
extern DATABASE *RewindFileDataBase(DATABASE *db);
/* Datenbank lesen */
extern DATABASE *ReadFileDataBase(char *db_name);

/* Eroeffnet die Protokoll-Datei einer Datenbank vom Typ DATABASE */
extern DATABASE *OpenProtDataBase(DATABASE *db);
/* Schliesst die Protokoll-Datei einer Datenbank vom Typ DATABASE */
extern DATABASE *CloseProtDataBase(DATABASE *db);
/* Lese-Zeiger auf Protokoll-Datei-Anfang  einer Datenbank positionieren */
extern DATABASE *RewindProtDataBase(DATABASE *db);


/* ================================================================ */
/* Datentyp : LIST_DATABASE                                         */
/* ================================================================ */


typedef struct {
    char *name;
    List *db_list;
    long count_of_db;
    char **names_of_db;
    long count_of_db_names;
} LIST_DATABASE;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_DATABASE *create_db_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_db_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int db_list_empty(void);

/* Bekanntmachung der Iteration mit dem Name name. */
extern long declare_db_name (char *name);
/*   Alle Iterationen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_db_names (void);

/* Fuegt eine Instanz vom Typ DATABASE in die Liste */
extern long insert_db_list (DATABASE *db);

/* Prueft auf Existenz eines Namens */
long db_name_exist (char *name);


/* ================================================================ */
/* Interface                                                        */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateDataBaseList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyDataBaseList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int DataBaseListEmpty(void);


/* DP-Modell */

/* Schnittstellenfunktionen zwischen DB und IDB */
int IDB_ReadDataBase (char *db_name, char *master_keyword, String *S, \
                 char **keywords, long number_of_keywords,
                 char *sep_string, char *comments, char *beginn_comments,char *end_comments, char *sep_kap);


/* DB-Objekt mit dem Namen (db_name) setzen */
extern DATABASE *SetDataBase(char *db_name, char *inp_dat,  char *out_dat);
/* liefert den Zeiger auf die Datenbank mit dem Namen db_name */ 
extern DATABASE *GetDataBase (char *db_name);
/* Datenbank auf Existenz pruefen */
extern DATABASE *ExistDataBase(char *db_name);
/* Eroeffnet eine Datenbank vom Typ DATABASE */
extern DATABASE *OpenDataBase(char *db_name);
/* Schliesst eine Datenbank vom Typ DATABASE */
extern DATABASE *CloseDataBase(char *db_name);
/* Lese-Zeiger auf Datenbank-Anfang positionieren */
extern DATABASE *RewindDataBase(char *db_name);
/* Datenbank lesen */
DATABASE *ReadDataBase(char *db_name);

/* Liefert Zeiger auf einen Protokoll-Datei-Zeiger
   (Wegen Kompatibilitaet nur). */
FILE *DB_GetProtPointerDataBase (char *db_name);


#endif

