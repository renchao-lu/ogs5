/**************************************************************************/
/* ROCKFLOW - Modul: rfdb.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp DATABASE
   - Verwaltung von DATABASE mit dem Datentyp LIST_DATABASE
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Interface */
#include "rfdb.h"

#include "geo_strings.h"

  /* Liste von Randbedingungen */
static LIST_DATABASE *list_of_db = NULL;




/* ================================================================ */
/* DATABASE */
/* ================================================================ */



/**************************************************************************/
/* ROCKFLOW - Funktion: CreateDataBase
                                                                          */
/* Aufgabe:
   DATABASE-Konstruktor
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Name (ID) des DB's.
                                                                          */
/* Ergebnis:
   - Adresse des DB's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *CreateDataBase(char *name, char *file_name, char *prot_name)
{
  DATABASE *db;

  db = (DATABASE *) Malloc(sizeof(DATABASE));
  if ( db == NULL ) return NULL;

  db->name = (char *) Malloc((int)strlen(name)+1);
  if ( db->name == NULL ) {
    Free(db);
    return NULL;
  }
  strcpy(db->name,name);

  db->file_name = (char *) Malloc((int)strlen(file_name)+1);
  if ( db->file_name == NULL ) {
    Free(db->name);
    Free(db);
    return NULL;
  }
  strcpy(db->file_name,file_name);

  db->prot_name = (char *) Malloc((int)strlen(prot_name)+1);
  if ( db->prot_name == NULL ) {
    Free(db->file_name);
    Free(db->name);
    Free(db);
    return NULL;
  }
  strcpy(db->prot_name,prot_name);

  db->S=String_create();
  if ( db->S == NULL ) {
    Free(db->prot_name);
    Free(db->file_name);
    Free(db->name);
    Free(db);
    return NULL;
  }

  db->fp = NULL;
  db->pp = NULL;
  db->group_name = NULL;
  db->max_size=-1;
  db->save_mode=0;

  return db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyDataBase
                                                                          */
/* Aufgabe:
   DATABASE-Destruktor
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf eine Instanz vom Typ DADATBASE 
                   (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void DestroyDataBase(void *member)
{
  DATABASE *db=(DATABASE *)member;

  if ( db->S != NULL ) {
    db->S=String_destroy(db->S);
  }
  if ( db->fp ) fclose(db->fp);
  if ( db->prot_name ) db->prot_name=(char *)Free(db->prot_name);
  if ( db->file_name ) db->file_name=(char *)Free(db->file_name);
  if ( db->group_name ) db->group_name=(char *)Free(db->group_name);
  if ( db->name ) db->name=(char *)Free(db->name);
  if ( db ) db=(DATABASE *)Free(db);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DeleteDataBase
                                                                          */
/* Aufgabe:
   Inhalt einer Datenbank Loeschen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf eine Instanz vom Typ DADATBASE 
                   (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void DeleteDataBase(DATABASE *db)
{
  CloseFileDataBase(db);
  CloseProtDataBase(db);

  if ( db->S != NULL ) {
    db->S=String_destroy(db->S);
  }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetNameDataBase
                                                                          */
/* Aufgabe:
   Setzt Namen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Zeiger auf den Namen des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetNameDataBase(DATABASE *db, char *name)
{
  if (!name) return;
  if(db) {
    db->name=(char *)Free(db->name);
    db->name = (char *) Malloc((int)strlen(name)+1);
    if ( db->name == NULL ) return;
    strcpy(db->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetNameDataBase
                                                                          */
/* Aufgabe:
   Liefert Namen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Namen des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetNameDataBase(DATABASE *db)
{
  return (db && db->name)?db->name:NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetGroupNameDataBase
                                                                          */
/* Aufgabe:
   Setzt Gruppennamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Zeiger auf den Namen des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetGroupNameDataBase(DATABASE *db, char *group_name)
{
  if (!group_name) return;
  if(db) {
    db->group_name=(char *)Free(db->group_name);
    db->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( db->group_name == NULL ) return;
    strcpy(db->group_name,group_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetGroupNameDataBase
                                                                          */
/* Aufgabe:
   Liefert Gruppennamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Namen des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetGroupNameDataBase(DATABASE *db)
{
  return (db && db->group_name)?db->group_name:NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: SetFileNameDataBase
                                                                          */
/* Aufgabe:
   Setzt Dateinamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Zeiger auf den Dateinamen des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetFileNameDataBase(DATABASE *db, char *file_name)
{
  if (!file_name) return;
  if(db) {
    db->file_name=(char *)Free(db->file_name);
    db->file_name = (char *) Malloc((int)strlen(file_name)+1);
    if ( db->file_name == NULL ) return;
    strcpy(db->file_name,file_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetFileNameDataBase
                                                                          */
/* Aufgabe:
   Liefert Dateinamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Namen des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetFileNameDataBase(DATABASE *db)
{
/*  return (db && db->file_name)?db->file_name:NULL;*/

  return (db && db->file_name)? db->file_name: 
         (db && db->name)? db->name: NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: SetFilePointerDataBase
                                                                          */
/* Aufgabe:
   Setzt Dateizeiger des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Dateizeiger des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetFilePointerDataBase(DATABASE *db, FILE *fp)
{
  if (!fp) return;
  if(db) {
    if (db->fp) CloseFileDataBase(db);
    db->fp = fp;
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetFilePointerDataBase
                                                                          */
/* Aufgabe:
   Liefert Dateizeiger des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Dateizeiger des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
FILE *GetFilePointerDataBase(DATABASE *db)
{
  return (db && db->fp)?db->fp:NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetMaxSizeDataBase
                                                                          */
/* Aufgabe:
   Setzt Datenbank-Groesse des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E long max_size: maximale Datenbank-Groesse des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetMaxSizeDataBase(DATABASE *db, long max_size)
{
  if (db) db->max_size = max_size;
  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetMaxSizeDataBase
                                                                          */
/* Aufgabe:
   Liefert Datenbank-Groesse des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - maximale Datenbank-Groesse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long GetMaxSizeDataBase(DATABASE *db)
{
  return (db)?db->max_size:0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetSaveModeDataBase
                                                                          */
/* Aufgabe:
   Setzt Speicherungsmodus des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E int save_mode: Speicherungsmodus des DB's.
                    0: ascii   1: binary
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetSaveModeDataBase(DATABASE *db, int save_mode)
{
  if (db) db->save_mode = save_mode;
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetSaveModeDataBase
                                                                          */
/* Aufgabe:
   Liefert Speicherungsmodus des DB's

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Speicherungsmodus des DB's, 0: ascii  1: binary -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int GetSaveModeDataBase(DATABASE *db)
{
  return (db)?db->save_mode:0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetProtNameDataBase
                                                                          */
/* Aufgabe:
   Setzt Protokoll-Dateinamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Zeiger auf den Protokoll-Dateinamen des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetProtNameDataBase(DATABASE *db, char *prot_name)
{
  if (!prot_name) return;
  if(db) {
    db->prot_name=(char *)Free(db->prot_name);
    db->prot_name = (char *) Malloc((int)strlen(prot_name)+1);
    if ( db->prot_name == NULL ) return;
    strcpy(db->prot_name,prot_name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetProtNameDataBase
                                                                          */
/* Aufgabe:
   Liefert Protokoll-Dateinamen des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Namen des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *GetProtNameDataBase(DATABASE *db)
{
/*  return (db && db->prot_name)?db->prot_name:NULL;*/

  return (db && db->prot_name)? db->prot_name: 
         (db && db->name)? db->name: NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: SetProtPointerDataBase
                                                                          */
/* Aufgabe:
   Setzt Protokoll-Dateizeiger des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
   E char *name: Dateizeiger des DB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetProtPointerDataBase(DATABASE *db, FILE *pp)
{
  if (!pp) return;
  if(db) {
    if (db->pp) CloseProtDataBase(db);
    db->pp = pp;
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetProtPointerDataBase
                                                                          */
/* Aufgabe:
   Liefert Dateizeiger des DB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Dateizeiger des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
FILE *GetProtPointerDataBase(DATABASE *db)
{
  return (db && db->pp)?db->pp:NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: OpenFileDataBase
                                                                          */
/* Aufgabe:
   Eroeffnet die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *OpenFileDataBase(DATABASE *db)
{
  char *name=GetFileNameDataBase(db);
  FILE *fp=GetFilePointerDataBase(db);
  static char *open_mode="r";
  int save_mode=GetSaveModeDataBase(db);

  if (save_mode == 1) return NULL;  /* binary mode: noch nicht implementiert */ 

  if (name && !fp) {
    if ((fp = fopen(name,open_mode))==NULL) {
      /*fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);*/
      /*exit(1);*/
      return NULL;
    }
    else SetFilePointerDataBase(db,fp);
  }
  else if (name && fp) {
    CloseFileDataBase(db);
    if ((fp = fopen(name,open_mode))==NULL) {
      /*fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);*/
      /*exit(1);*/
      return NULL;
    }
    else SetFilePointerDataBase(db,fp);
  }
  else {
    /*fprintf(stderr,"Warnung: Datenbank besitzt keinen Identifikator !");*/
    /*return NULL;*/
    return NULL;
  }

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CloseFileDataBase
                                                                          */
/* Aufgabe:
   Schliesst die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *CloseFileDataBase(DATABASE *db)
{
  if ( db->fp ) fclose(db->fp); db->fp=NULL;

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RewindFileDataBase
                                                                          */
/* Aufgabe:
   Lese-Zeiger auf Datenbank-Anfang positionieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *RewindFileDataBase(DATABASE *db)
{
  CloseFileDataBase(db);
  OpenFileDataBase(db);

  return db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: OpenProtDataBase
                                                                          */
/* Aufgabe:
   Eroeffnet die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *OpenProtDataBase(DATABASE *db)
{
  char *name=GetProtNameDataBase(db);
  FILE *pp=GetProtPointerDataBase(db);
  static char *open_mode="wa";
  int save_mode=GetSaveModeDataBase(db);

  if (save_mode == 1) return NULL;  /* binary mode: noch nicht implementiert */ 

  if (name && !pp) {
    if ((pp = fopen(name,open_mode))==NULL) {
      /*fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);*/
      /*exit(1);*/
      return NULL;
    }
    else SetProtPointerDataBase(db,pp);
  }
  else if (name && pp) {
    CloseProtDataBase(db);
    if ((pp = fopen(name,open_mode))==NULL) {
      /*fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);*/
      /*exit(1);*/
      return NULL;
    }
    else SetProtPointerDataBase(db,pp);
  }
  else {
    /*fprintf(stderr,"Warnung: Datenbank besitzt keinen Identifikator !");*/
    return NULL;
  }

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CloseProtDataBase
                                                                          */
/* Aufgabe:
   Schliesst die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *CloseProtDataBase(DATABASE *db)
{
  if ( db->pp ) fclose(db->pp); db->pp=NULL;

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RewindProtDataBase
                                                                          */
/* Aufgabe:
   Lese-Zeiger auf Datenbank-Anfang positionieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf eine Instanz vom Typ DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *RewindProtDataBase(DATABASE *db)
{
  CloseProtDataBase(db);
  OpenProtDataBase(db);

  return db;
}




/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_db_list
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
LIST_DATABASE *create_db_list (char *name)
{
  LIST_DATABASE *list_of_db;

  list_of_db = (LIST_DATABASE *) Malloc(sizeof(LIST_DATABASE));
  if ( list_of_db == NULL ) return NULL;


  list_of_db->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_db->name == NULL ) {
         Free(list_of_db);
         return NULL;
  }
  strcpy(list_of_db->name,name);

  list_of_db->db_list=create_list();
  if ( list_of_db->db_list == NULL ) {
    Free(list_of_db->name);
        Free(list_of_db);
        return NULL;
  }

  list_of_db->names_of_db=NULL;
  list_of_db->count_of_db_names=0;

  return list_of_db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_db_list
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
void destroy_db_list(void)
{
  if (list_of_db->name) list_of_db->name=(char *)Free(list_of_db->name);
  if(list_of_db->names_of_db) \
    list_of_db->names_of_db = \
      (char **)Free(list_of_db->names_of_db);

  if (list_of_db->db_list) {
    delete_list(list_of_db->db_list,DestroyDataBase);
    list_of_db->db_list=destroy_list(list_of_db->db_list);
  }

  if (list_of_db) list_of_db=(LIST_DATABASE *)Free(list_of_db);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: db_list_empty
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
int db_list_empty(void)
{
  if (!list_of_db) return 0;
  else if (!list_of_db->db_list) return 0;
  else return list_empty(list_of_db->db_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_db_init
                                                                          */
/* Aufgabe:
   Liste der DB's initialisieren.
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
void list_db_init(void)
{
  if (list_of_db) list_current_init(list_of_db->db_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_db_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_db_next (void)
{
  return list_of_db->db_list ?
             get_list_next(list_of_db->db_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_db_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ DATABASE erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des DB's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen DB's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_db_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_db) return 0;
  for ( i=0; i<list_of_db->count_of_db_names; i++)
    if(strcmp(list_of_db->names_of_db[i],name) == 0) break;
  if (i < list_of_db->count_of_db_names) return i;

  list_of_db->names_of_db= \
          (char **) Realloc(list_of_db->names_of_db, \
          (list_of_db->count_of_db_names+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_db->names_of_db[i]=new_name;

  return ++(list_of_db->count_of_db_names);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_db_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen DB's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_db_names (void)
{
  long i;

  for (i=0; i<list_of_db->count_of_db_names; i++)
    if(list_of_db->names_of_db[i]) \
      list_of_db->names_of_db[i] = \
        (char *)Free(list_of_db->names_of_db[i]);

  return list_of_db->count_of_db_names;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_db_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ DATABASE in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_db_list (DATABASE *db)
{
  if(!list_of_db) return 0;
  list_of_db->count_of_db=append_list(list_of_db->db_list,(void *) db);
  return list_of_db->count_of_db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_db_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ DATABASE der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender DATABASE-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long remove_db_list (DATABASE *db)
{
  list_of_db->count_of_db = \
      remove_list_member(list_of_db->db_list,(void *) db, DestroyDataBase);
  return list_of_db->count_of_db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: db_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des DB's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long db_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_db->count_of_db_names; i++)
    if(strcmp(list_of_db->names_of_db[i],name) == 0) break;
  if (i < list_of_db->count_of_db_names) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateDataBaseList
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
void CreateDataBaseList(void)
{
  list_of_db = create_db_list("DATABASE_LIST");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyDataBaseList
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
void DestroyDataBaseList(void)
{
  undeclare_db_names();
  destroy_db_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DataBaseListEmpty
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
int DataBaseListEmpty(void)
{
  return db_list_empty();
}


/*========================================================================*/
/* Schnittstellenfunktionen zwischen DB und IDB                           */
/*========================================================================*/


/**************************************************************************/
/* ROCKFLOW - Funktion: IDB_ReadDataBase
                                                                          */
/* Aufgabe:
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version
                                                                          */
/**************************************************************************/
int IDB_ReadDataBase (char *db_name, char *master_keyword, String *S, \
                 char **keywords, long number_of_keywords,
                 char *sep_string, char *comments, char *beginn_comments,char *end_comments, char *sep_kap)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return 0;

  if ( db->S ) {
    if (number_of_keywords) 
      String_read_Keyword(S,db->S,master_keyword,keywords,number_of_keywords,\
                 sep_string,comments,beginn_comments,end_comments,sep_kap);
    else 
      String_read_keyword(S,db->S,master_keyword,'{','}');
  }

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: IDB_ReadDataBase
                                                                          */
/* Aufgabe:
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   10/2001     AH        Erste Version (noch nicht fertig !!!)
                         Ausgabe soll in einem String erfolgen
                                                                          */
/**************************************************************************/
int IDB_WriteDataBase (char *db_name, char *master_keyword, String *S, \
                 char **keywords, long number_of_keywords,
                 char *sep_string, char *comments, char *beginn_comments,char *end_comments, char *sep_kap)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return 0;

  if ( db->S ) {
    if (number_of_keywords) 
      String_read_Keyword(S,db->S,master_keyword,keywords,number_of_keywords,\
                 sep_string,comments,beginn_comments,end_comments,sep_kap);
    else 
      String_read_keyword(S,db->S,master_keyword,'{','}');
  }

  return 1;
}



/*========================================================================*/
/* Schnittstellenfunktionen zwischen DB und Rockflow                           */
/*========================================================================*/


/**************************************************************************/
/* ROCKFLOW - Funktion: SetDataBase
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E DATABASE *db: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *SetDataBase(char *db_name, char *inp_dat,  char *out_dat)
{
  DATABASE *db=NULL;

  db=CreateDataBase(db_name,inp_dat,out_dat); 
  if (db) insert_db_list(db);

  if (db) return db;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: OpenDataBase
                                                                          */
/* Aufgabe:
   Eroeffnet die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *OpenDataBase(char *db_name)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return 0;

  CloseFileDataBase(db);
  if (!OpenFileDataBase(db)) {
    CloseProtDataBase(db);
    return NULL;
  }
  OpenProtDataBase(db); /* Protokolldatei oeffnen */

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CloseDataBase
                                                                          */
/* Aufgabe:
   Schliesst die Instanz vom Typ DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *CloseDataBase(char *db_name)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return NULL;

  CloseFileDataBase(db);
  CloseProtDataBase(db);

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RewindDataBase
                                                                          */
/* Aufgabe:
   Lese-Zeiger auf Datenbank-Anfang positionieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *RewindDataBase(char *db_name)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return NULL;

  CloseFileDataBase(db);
  OpenFileDataBase(db);

  CloseProtDataBase(db);
  OpenProtDataBase(db);

  return db;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReadDataBase
                                                                          */
/* Aufgabe:
   Datenbank lesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *ReadDataBase(char *db_name)
{
  char *file_name=NULL;
  int mode=0;
  long max_size=0;
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return NULL;

/*
  if ( !GetFilePointerDataBase(db) ) {
    OpenDataBase(db);
    mode=1;
  }
*/
  file_name=GetFileNameDataBase(db);
  max_size=GetMaxSizeDataBase(db);

  if (max_size<0) 
    String_read_file_print_mode(db->S,file_name,mode);
  else 
    String_read_section_file_print_mode(db->S,file_name,mode,max_size);

  return db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExistDataBase
                                                                          */
/* Aufgabe:
   Datenbank auf Existenz pruefen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version  (zv) param: besser Name 
                          Funktion gehoert zur Verwaltung
                                                                          */
/**************************************************************************/
DATABASE *ExistDataBase(char *db_name)
{
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return NULL;

  return (DATABASE *)GetFilePointerDataBase(db);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ReadFileDataBase
                                                                          */
/* Aufgabe:
   Datenbank lesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *db_name: Zeiger auf den Datenbanknamen.
                                                                          */
/* Ergebnis:
   - Adresse des DB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *ReadFileDataBase(char *db_name)
{
  char *file_name=NULL;
  int mode=0;
  long max_size=0;
  DATABASE *db=NULL;

  db=GetDataBase (db_name);
  if (!db) return NULL;

  file_name=GetFileNameDataBase(db);
  max_size=GetMaxSizeDataBase(db);

/*
  if ( !GetFilePointerDataBase(db) ) {
    OpenDataBase(db);
    mode=1;
  }
*/
  if (max_size<0) 
    String_read_file_print_mode(db->S,file_name,mode);
  else 
    String_read_section_file_print_mode(db->S,file_name,mode,max_size);

  return db;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetDataBase
                                                                          */
/* Aufgabe:
   Liefert die linearen Loeser-Eigenschaften fuer die Gruppe name.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LINEAR_SOLVER_PROPERTIES *lsp: Zeiger auf die Datenstruktur ls.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
DATABASE *GetDataBase (char *db_name)
{
  DATABASE *db=NULL;

  if (!db_name) return NULL;
  if (DataBaseListEmpty()) return NULL;
  
  list_db_init();
  while ( (db=(DATABASE *)get_list_db_next()) != NULL ) {
    if ( strcmp(StrUp(GetNameDataBase(db)),StrUp(db_name))==0 ) 
    return db;
  }

  return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DB_GetProtPointerDataBase
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf einen Protokoll-Datei-Zeiger
   (Wegen Kompatibilitaet nur).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LINEAR_SOLVER_PROPERTIES *lsp: Zeiger auf die Datenstruktur ls.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version 
                                                                          */
/**************************************************************************/
FILE *DB_GetProtPointerDataBase (char *db_name)
{
  DATABASE *db=NULL;

  if (!db_name) return NULL;
  if (DataBaseListEmpty()) return NULL;
  
  list_db_init();
  while ( (db=(DATABASE *)get_list_db_next()) != NULL ) {
    if ( strcmp(StrUp(GetNameDataBase(db)),StrUp(db_name))==0 ) 
    return GetProtPointerDataBase(db);
  }

  return NULL;
}


