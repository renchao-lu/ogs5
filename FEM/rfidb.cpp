/**************************************************************************/
/* ROCKFLOW - Modul: rfidb.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp INPUT_DATABASE
   - Verwaltung von INPUT_DATABASE mit dem Datentyp LIST_INPUT_DATABASE
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
      

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

/* Interface */
#include "rfidb.h"



  /* Liste von Randbedingungen */
static LIST_INPUT_DATABASE *list_of_idb_database = NULL;



/* Nur fuer den internen Gebrauch */
static char **idb_names = NULL;
static long idb_names_number = 0;


/* Interne Daten */

/* ================================================================ */
/* Datentyp : INPUT_PROPERTIES_DATABASE                                                 */
/* ================================================================ */


typedef struct {
    char *sep_string;           /* Trenn-Zeichen beim Mehrfachvorkommen */
    char *comments;             /* Komentar-Zeichenkette */
    char *beginn_comments;      /* Komentar-Beginn-Zeichenkette fuer Blockkommentar */
    char *end_comments;         /* Komentar-End-Zeichenkette fuer Blockkommentar */
    char *beginn_section;       /* Block-Beginn-Zeichenkette fuer Blockkommentar */
    char *end_section;          /* Block-End-Zeichenkette fuer Blockkommentar */
    char *sep_cap;              /* Trenn-Zeichen beim Mehrfachvorkommen (Lesen) */
} INPUT_PROPERTIES_DATABASE;


/* Erzeugt eine Instanz vom Typ INPUT_PROPERTIES_DATABASE */
INPUT_PROPERTIES_DATABASE *create_ipdb_database(char *sep_string, char *comments, \
                                  char *beginn_comments, char *end_comments, \
                                  char *beginn_section, char *end_section, char *sep_cap);
/* Zerstoert die Instanz vom Typ INPUT_PROPERTIES_DATABASE */
void destroy_ipdb_database(void *member);

/* Interne Deklarationen */
INPUT_PROPERTIES_DATABASE *ipdb=NULL;



/* ================================================================ */
/* INPUT_PROPERTIES_DATABASE                                        */
/* ================================================================ */


/**************************************************************************/
/* ROCKFLOW - Funktion: create_ipdb_database
                                                                          */
/* Aufgabe:
   Konstruktor fuer den Datentyp INPUT_PROPERTIES_DATABASE.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *master_keyword: Zeiger auf das Schluesselwort des INP's.
                                                                          */
/* Ergebnis:
   - Adresse des IPDB's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INPUT_PROPERTIES_DATABASE *create_ipdb_database(char *sep_string, char *comments, \
                                  char *beginn_comments, char *end_comments, \
                                  char *beginn_section, char *end_section, char *sep_cap)
{
  INPUT_PROPERTIES_DATABASE *ipdb;

  ipdb = (INPUT_PROPERTIES_DATABASE *) Malloc(sizeof(INPUT_PROPERTIES_DATABASE));
  if ( ipdb == NULL ) return NULL;

  if (sep_string) {
    ipdb->sep_string = (char *) Malloc((int)strlen(sep_string)+1);
    if ( ipdb->sep_string == NULL ) {
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->sep_string,sep_string);
  }
  else {
    ipdb->sep_string = NULL;
  }

  if (comments) {
    ipdb->comments = (char *) Malloc((int)strlen(comments)+1);
    if ( ipdb->comments == NULL ) {
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->comments,comments);
  }
  else {
    ipdb->comments = NULL;
  }

  if (beginn_comments) {
    ipdb->beginn_comments = (char *) Malloc((int)strlen(beginn_comments)+1);
    if ( ipdb->beginn_comments == NULL ) {
      Free(ipdb->comments);
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->beginn_comments,beginn_comments);
  }
  else {
    ipdb->beginn_comments = NULL;
  }

  if (end_comments) {
    ipdb->end_comments = (char *) Malloc((int)strlen(end_comments)+1);
    if ( ipdb->end_comments == NULL ) {
      Free(ipdb->beginn_comments);
      Free(ipdb->comments);
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->end_comments,end_comments);
  }
  else {
     ipdb->end_comments = NULL;
  }

  if (beginn_section) {
    ipdb->beginn_section = (char *) Malloc((int)strlen(beginn_section)+1);
    if ( ipdb->beginn_section == NULL ) {
      Free(ipdb->end_comments);
      Free(ipdb->beginn_comments);
      Free(ipdb->comments);
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->beginn_section,beginn_section);
  }
  else {
    ipdb->beginn_section = NULL;
  }
  
  if (end_section) {
    ipdb->end_section = (char *) Malloc((int)strlen(end_section)+1);
    if ( ipdb->end_section == NULL ) {
      Free(ipdb->beginn_section);
      Free(ipdb->end_comments);
      Free(ipdb->beginn_comments);
      Free(ipdb->comments);
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->end_section,end_section);
  }
  else {
    ipdb->end_section = NULL;
  }
  
  if (sep_cap) {
    ipdb->sep_cap = (char *) Malloc((int)strlen(sep_cap)+1);
    if ( ipdb->sep_cap == NULL ) {
      Free(ipdb->end_section);
      Free(ipdb->beginn_section);
      Free(ipdb->end_comments);
      Free(ipdb->beginn_comments);
      Free(ipdb->comments);
      Free(ipdb->sep_string);
      Free(ipdb);
      return NULL;
    }
    strcpy(ipdb->sep_cap,sep_cap);
  }
  else {
    ipdb->sep_cap = NULL;
  }

  return ipdb;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_ipdb_database
                                                                          */
/* Aufgabe:
   Destruktor von einer Instanz vom Typ INPUT_PROPERTIES_DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf eine Instanz vom Typ INPUT_PROPERTIES_DATABASE 
                   (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_ipdb_database(void *member)
{
  INPUT_PROPERTIES_DATABASE *ipdb=(INPUT_PROPERTIES_DATABASE *)member;

  if ( ipdb->end_section ) ipdb->end_section=(char *)Free(ipdb->end_section);
  if ( ipdb->beginn_section ) ipdb->beginn_section=(char *)Free(ipdb->beginn_section);
  if ( ipdb->end_comments ) ipdb->end_comments=(char *)Free(ipdb->end_comments);
  if ( ipdb->beginn_comments ) ipdb->beginn_comments=(char *)Free(ipdb->beginn_comments);
  if ( ipdb->comments ) ipdb->comments=(char *)Free(ipdb->comments);
  if ( ipdb->sep_string ) ipdb->sep_string=(char *)Free(ipdb->sep_string);
  if ( ipdb->sep_cap ) ipdb->sep_cap=(char *)Free(ipdb->sep_cap);

  if ( ipdb ) ipdb=(INPUT_PROPERTIES_DATABASE *)Free(ipdb);

  return;
}



/* ================================================================ */
/* INPUT_DATABASE */
/* ================================================================ */


/**************************************************************************/
/* ROCKFLOW - Funktion: create_idb_database
                                                                          */
/* Aufgabe:
   Konstruktor for INPUT_DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *master_keyword: Zeiger auf das Schluesselwort des INP's.
                                                                          */
/* Ergebnis:
   - Adresse des INP's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *create_idb_database(char *group_name, 
         char *master_keyword,  Read_Function read_function, 
         char *sep_string, char *comments, char *beginn_comments, char *end_comments, char *sep_cap)
{
  INPUT_DATABASE *idb;

  idb = (INPUT_DATABASE *) Malloc(sizeof(INPUT_DATABASE));
  if ( idb == NULL ) return NULL;


  idb->master_keyword = (char *) Malloc((int)strlen(master_keyword)+1);
  if ( idb->master_keyword == NULL ) {
    Free(idb);
    return NULL;
  }
  strcpy(idb->master_keyword,master_keyword);

  idb->group_name = (char *) Malloc((int)strlen(group_name)+1);
  if ( idb->group_name == NULL ) {
    Free(idb->master_keyword);
    Free(idb);
    return NULL;
  }
  strcpy(idb->group_name,group_name);

  if (sep_string) {
    idb->sep_string = (char *) Malloc((int)strlen(sep_string)+1);
    if ( idb->sep_string == NULL ) {
      Free(idb->group_name);
      Free(idb->master_keyword);
      Free(idb);
      return NULL;
    }
    strcpy(idb->sep_string,sep_string);
  }
  else {
    idb->sep_string = NULL;
  }

  if (comments) {
    idb->comments = (char *) Malloc((int)strlen(comments)+1);
    if ( idb->comments == NULL ) {
      Free(idb->sep_string);
      Free(idb->group_name);
      Free(idb->master_keyword);
      Free(idb);
      return NULL;
    }
    strcpy(idb->comments,comments);
  }
  else {
    idb->comments = NULL;
  }

  if (beginn_comments) {
    idb->beginn_comments = (char *) Malloc((int)strlen(beginn_comments)+1);
    if ( idb->beginn_comments == NULL ) {
      Free(idb->comments);
      Free(idb->sep_string);
      Free(idb->group_name);
      Free(idb->master_keyword);
      Free(idb);
      return NULL;
    }
    strcpy(idb->beginn_comments,beginn_comments);
  }
  else {
    idb->beginn_comments = NULL;
  }

  if (end_comments) {
    idb->end_comments = (char *) Malloc((int)strlen(end_comments)+1);
    if ( idb->end_comments == NULL ) {
      Free(idb->beginn_comments);
      Free(idb->comments);
      Free(idb->sep_string);
      Free(idb->group_name);
      Free(idb->master_keyword);
      Free(idb);
      return NULL;
    }
    strcpy(idb->end_comments,end_comments);
  }
  else {
     idb->end_comments = NULL;
  }

  if (sep_cap) {
    idb->sep_cap = (char *) Malloc((int)strlen(sep_cap)+1);
    if ( idb->sep_cap == NULL ) {
      Free(idb->end_comments);
      Free(idb->beginn_comments);
      Free(idb->comments);
      Free(idb->sep_string);
      Free(idb->group_name);
      Free(idb->master_keyword);
      Free(idb);
      return NULL;
    }
    strcpy(idb->sep_cap,sep_cap);
  }
  else {
     idb->end_comments = NULL;
  }

  idb->S=String_create();
  if ( idb->S == NULL ) {
    Free(idb->sep_cap);
    Free(idb->end_comments);
    Free(idb->beginn_comments);
    Free(idb->comments);
    Free(idb->sep_string);
    Free(idb->group_name);
    Free(idb->master_keyword);
    Free(idb);
    return NULL;
  }
  
  if ( read_function ) idb->read_function=read_function;
  else idb->read_function=NULL;
  idb->keyword=NULL;
  idb->file_name=NULL;
  idb->fp=NULL;

  return idb;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_idb_database
                                                                          */
/* Aufgabe:
   Destructor for INPUT_DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf eine Instanz vom Typ INPUT_DATABASE 
                   (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_idb_database(void *member)
{
  INPUT_DATABASE *idb=(INPUT_DATABASE *)member;

  if ( idb->fp ) fclose(idb->fp); idb->fp=NULL;
  if ( idb->read_function ) idb->read_function=NULL;
  if ( idb->S != NULL ) {
    idb->S=String_destroy(idb->S);
  }

  if ( idb->file_name ) idb->file_name=(char *)Free(idb->file_name);
  if ( idb->keyword ) idb->keyword=(char *)Free(idb->keyword);
  if ( idb->group_name ) idb->group_name=(char *)Free(idb->group_name);
  if ( idb->master_keyword ) idb->master_keyword=(char *)Free(idb->master_keyword);

  if ( idb->sep_string ) idb->sep_string=(char *)Free(idb->sep_string);
  if ( idb->comments ) idb->comments=(char *)Free(idb->comments);
  if ( idb->beginn_comments ) idb->beginn_comments=(char *)Free(idb->beginn_comments);
  if ( idb->end_comments ) idb->end_comments=(char *)Free(idb->end_comments);
  if ( idb->sep_cap ) idb->sep_cap=(char *)Free(idb->sep_cap);

  if ( idb ) idb=(INPUT_DATABASE *)Free(idb);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: open_idb_database
                                                                          */
/* Aufgabe:
   Eroeffnet die Instanz vom Typ INPUT_DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des IDB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *open_idb_database(INPUT_DATABASE *idb)
{
  char *name=get_idb_file_name(idb);
  FILE *fp=NULL;
  static char *first_open_mode="a";
  static char *next_open_mode="a";

  if (get_idb_file_name(idb) && !get_idb_fp(idb)) {
    if ((fp = fopen(name,first_open_mode))==NULL) {
      /*DisplayErrorMsg("Fehler: Datei %s konnte nicht geoeffnet werden !");*/
      fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);
      return NULL;
    }
    else set_idb_fp(idb,fp);
  }
  else if (get_idb_file_name(idb) && get_idb_fp(idb)) {
    if ((fp = fopen(name,next_open_mode))==NULL) {
      /*DisplayErrorMsg("Fehler: Datei %s konnte nicht geoeffnet werden !");*/
      fprintf(stderr,"Fehler: Datei %s konnte nicht geoeffnet werden !",name);
      return NULL;
    }
    else set_idb_fp(idb,fp);
  }

  return idb;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: close_idb_database
                                                                          */
/* Aufgabe:
   Eroeffnet die Instanz vom Typ INPUT_DATABASE
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Adresse des IDB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *close_idb_database(INPUT_DATABASE *idb)
{
  if ( idb->fp ) fclose(idb->fp); idb->fp=NULL;

  return idb;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_master_keyword
                                                                          */
/* Aufgabe:
   Setzt Master-Schluesselwort des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *master_keyword: Zeiger auf das Master-Schluesselwort des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_master_keyword(INPUT_DATABASE *idb, char *master_keyword)
{
  if (!master_keyword) return;
  if(idb) {
    idb->master_keyword=(char *)Free(idb->master_keyword);
    idb->master_keyword = (char *) Malloc((int)strlen(master_keyword)+1);
    if ( idb->master_keyword == NULL ) return;
    strcpy(idb->master_keyword,master_keyword);
    return;
  }
  else return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_master_keyword
                                                                          */
/* Aufgabe:
   Liefert Master-Schluesselwort des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Schluesselwort des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_master_keyword(INPUT_DATABASE *idb)
{
  if (idb->master_keyword) return idb->master_keyword;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_keyword
                                                                          */
/* Aufgabe:
   Setzt Schluesselwort des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *keyword: Zeiger auf das Schluesselwort des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_keyword(INPUT_DATABASE *idb, char *keyword)
{
  if (!keyword) return;
  if(idb) {
    idb->keyword=(char *)Free(idb->keyword);
    idb->keyword = (char *) Malloc((int)strlen(keyword)+1);
    if ( idb->keyword == NULL ) return;
    strcpy(idb->keyword,keyword);
    return;
  }
  else return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_keyword
                                                                          */
/* Aufgabe:
   Liefert Schluesselwort des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Schluesselwort des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_keyword(INPUT_DATABASE *idb)
{
  if (idb->keyword)
    return idb->keyword;
  else
    return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_group_name
                                                                          */
/* Aufgabe:
   Setzt Name des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *group_name: Zeiger auf den Namen des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_group_name(INPUT_DATABASE *idb, char *group_name)
{
  if (!group_name) return;
  if(idb) {
    idb->group_name=(char *)Free(idb->group_name);
    idb->group_name = (char *) Malloc((int)strlen(group_name)+1);
    if ( idb->group_name == NULL ) return;
    strcpy(idb->group_name,group_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_group_name
                                                                          */
/* Aufgabe:
   Liefert Name des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Name des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_group_name(INPUT_DATABASE *idb)
{
  if (idb->group_name)
    return idb->group_name;
  else
    return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_keyword
                                                                          */
/* Aufgabe:
   Setzt Found-Modus des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E int found: Found-Modus des Schluesselworts des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_found(INPUT_DATABASE *idb, int found)
{
  if(idb) idb->found=found;
  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_keyword
                                                                          */
/* Aufgabe:
   Liefert Found-Modus des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_idb_found(INPUT_DATABASE *idb)
{
  if (idb) return idb->found;
  else return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_S
                                                                          */
/* Aufgabe:
   Setzt Name des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *S: Zeiger auf den Namen des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_S(INPUT_DATABASE *idb, String *S)
{
  if (!idb || !S) return;
  
  idb->S=String_create_String(S);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_S
                                                                          */
/* Aufgabe:
   Liefert Name des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Name des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
String *get_idb_S(INPUT_DATABASE *idb)
{
  if (idb->S) return idb->S;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_sep_string
                                                                          */
/* Aufgabe:
   Setzt Trenn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *sep_string: zeiger auf die Trenn-Zeichenkette des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_sep_string(INPUT_DATABASE *idb, char *sep_string)
{
  if(idb) {
    idb->sep_string=(char *)Free(idb->sep_string);
    idb->sep_string = (char *) Malloc((int)strlen(sep_string)+1);
    if ( idb->sep_string == NULL ) return;
    strcpy(idb->sep_string,sep_string);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_sep_string
                                                                          */
/* Aufgabe:
   Liefert Trenn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Trenn-Zeichen des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_sep_string(INPUT_DATABASE *idb)
{
  if (idb->sep_string) return idb->sep_string;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_comments
                                                                          */
/* Aufgabe:
   Setzt Kommentar-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *comments: zeiger auf die Kommentar-Zeichenkette des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version zv im Fall comments==NULL
                                                                          */
/**************************************************************************/
void set_idb_comments(INPUT_DATABASE *idb, char *comments)
{
  if(idb) {
    idb->comments=(char *)Free(idb->comments);
    idb->comments = (char *) Malloc((int)strlen(comments)+1);
    if ( idb->comments == NULL ) return;
    strcpy(idb->comments,comments);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_comments
                                                                          */
/* Aufgabe:
   Liefert Kommentar-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Kommentar-Zeichen des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_comments(INPUT_DATABASE *idb)
{
  if (idb->comments) return idb->comments;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_beginn_comments
                                                                          */
/* Aufgabe:
   Setzt Kommentar-Beginn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *beginn_comments: zeiger auf die Kommentar-Beginn-Zeichenkette
                            des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version zv im Fall comments==NULL
                                                                          */
/**************************************************************************/
void set_idb_beginn_comments(INPUT_DATABASE *idb, char *beginn_comments)
{
  if(idb) {
    idb->beginn_comments=(char *)Free(idb->beginn_comments);
    idb->beginn_comments = (char *) Malloc((int)strlen(beginn_comments)+1);
    if ( idb->beginn_comments == NULL ) return;
    strcpy(idb->beginn_comments,beginn_comments);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_beginn_comments
                                                                          */
/* Aufgabe:
   Liefert Kommentar-Beginn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Kommentar-Beginn-Zeichen des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_beginn_comments(INPUT_DATABASE *idb)
{
  if (idb->beginn_comments) return idb->beginn_comments;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_end_comments
                                                                          */
/* Aufgabe:
   Setzt Kommentar-End-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *end_comments: zeiger auf die Kommentar-End-Zeichenkette
                         des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH          Erste Version
                                                                          */
/**************************************************************************/
void set_idb_end_comments(INPUT_DATABASE *idb, char *end_comments)
{
  if(idb) {
    idb->end_comments=(char *)Free(idb->end_comments);
    idb->end_comments = (char *) Malloc((int)strlen(end_comments)+1);
    if ( idb->end_comments == NULL ) return;
    strcpy(idb->end_comments,end_comments);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_end_comments
                                                                          */
/* Aufgabe:
   Liefert Kommentar-End-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Kommentar-End-Zeichen des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_end_comments(INPUT_DATABASE *idb)
{
  if (idb->end_comments) return idb->end_comments;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_sep_cap
                                                                          */
/* Aufgabe:
   Setzt Trenn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *sep_cap: zeiger auf die Trenn-Zeichenkette des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_sep_cap(INPUT_DATABASE *idb, char *sep_cap)
{
  if(idb) {
    idb->sep_cap=(char *)Free(idb->sep_cap);
    idb->sep_cap = (char *) Malloc((int)strlen(sep_cap)+1);
    if ( idb->sep_cap == NULL ) return;
    strcpy(idb->sep_cap,sep_cap);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_sep_cap
                                                                          */
/* Aufgabe:
   Liefert Trenn-Zeichen des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Trenn-Zeichen des INP's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_sep_cap(INPUT_DATABASE *idb)
{
  if (idb->sep_cap) return idb->sep_cap;
  else return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_read_function
                                                                          */
/* Aufgabe:
   Setzt Auswertefuntion des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E IntFuncCXIXFileX read_function: Funtionzeiger auf die Auswertefuntion
                                     des INP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH          Erste Version
                                                                          */
/**************************************************************************/
void set_idb_read_function(INPUT_DATABASE *idb, Read_Function read_function)
{
  if(idb) {
    idb->read_function=read_function;
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_read_function
                                                                          */
/* Aufgabe:
   Liefert die Auswertefuntion des INP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
Read_Function get_idb_read_function(INPUT_DATABASE *idb)
{
  if (idb->read_function) return idb->read_function;
  else return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_file_name
                                                                          */
/* Aufgabe:
   Setzt Datei-Name des IDB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E char *file_name: Zeiger auf den Datei-Namen des IDB's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_idb_file_name(INPUT_DATABASE *idb, char *file_name)
{
  if (!file_name) return;
  if(idb) {
    idb->file_name=(char *)Free(idb->file_name);
    idb->file_name = (char *) Malloc((int)strlen(file_name)+1);
    if ( idb->file_name == NULL ) return;
    strcpy(idb->file_name,file_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_file_name
                                                                          */
/* Aufgabe:
   Liefert Datei-Name des IDB's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - Datei-Name des IDB's -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_idb_file_name(INPUT_DATABASE *idb)
{
  if (idb->file_name) return idb->file_name;
  else return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_idb_fp
                                                                          */
/* Aufgabe:
   Setzt Datei-Zeiger des IDB's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
   E FILE *fp: Datei-Zeiger                                                                           */

/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH          Erste Version
                                                                          */
/**************************************************************************/
void set_idb_fp(INPUT_DATABASE *idb, FILE *fp)
{
  if (idb) idb->fp=fp;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_idb_fp
                                                                          */
/* Aufgabe:
   Liefert Datei-Zeiger des IDB's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf eine Instanz vom Typ INPUT_DATABASE.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   10/2001     AH          Erste Version
                                                                          */
/**************************************************************************/
FILE *get_idb_fp(INPUT_DATABASE *idb)
{
  return (idb)?idb->fp:NULL;
}




/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_idb_database_list
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
LIST_INPUT_DATABASE *create_idb_database_list (char *name)
{
  LIST_INPUT_DATABASE *list_of_idb_database;

  list_of_idb_database = (LIST_INPUT_DATABASE *) Malloc(sizeof(LIST_INPUT_DATABASE));
  if ( list_of_idb_database == NULL ) return NULL;


  list_of_idb_database->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_idb_database->name == NULL ) {
         Free(list_of_idb_database);
         return NULL;
  }
  strcpy(list_of_idb_database->name,name);

  list_of_idb_database->idb_list=create_list();
  if ( list_of_idb_database->idb_list == NULL ) {
    Free(list_of_idb_database->name);
        Free(list_of_idb_database);
        return NULL;
  }

  list_of_idb_database->names_of_idb_database=NULL;
  list_of_idb_database->count_of_idb_database_name=0;

  return list_of_idb_database;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_idb_database_list
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
void destroy_idb_database_list(void)
{
  if (list_of_idb_database->name) list_of_idb_database->name=(char *)Free(list_of_idb_database->name);
  if(list_of_idb_database->names_of_idb_database) \
    list_of_idb_database->names_of_idb_database = \
      (char **)Free(list_of_idb_database->names_of_idb_database);

  if (list_of_idb_database->idb_list) {
    delete_list(list_of_idb_database->idb_list,destroy_idb_database);
    list_of_idb_database->idb_list=destroy_list(list_of_idb_database->idb_list);
  }

  if (list_of_idb_database) list_of_idb_database=(LIST_INPUT_DATABASE *)Free(list_of_idb_database);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: idb_database_list_empty
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
int idb_database_list_empty(void)
{
  if (!list_of_idb_database) return 0;
  else if (!list_of_idb_database->idb_list) return 0;
  else return list_empty(list_of_idb_database->idb_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_idb_init
                                                                          */
/* Aufgabe:
   Liste der INP's initialisieren.
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
void list_idb_init(void)
{
  if (list_of_idb_database) list_current_init(list_of_idb_database->idb_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_idb_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_idb_next (void)
{
  return list_of_idb_database->idb_list ?
             get_list_next(list_of_idb_database->idb_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_idb_database_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ INPUT_DATABASE erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des INP's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen INP's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_idb_database_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_idb_database) return 0;
  for ( i=0; i<list_of_idb_database->count_of_idb_database_name; i++)
    if(strcmp(list_of_idb_database->names_of_idb_database[i],name) == 0) break;
  if (i < list_of_idb_database->count_of_idb_database_name) return i;

  list_of_idb_database->names_of_idb_database= \
          (char **) Realloc(list_of_idb_database->names_of_idb_database, \
          (list_of_idb_database->count_of_idb_database_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_idb_database->names_of_idb_database[i]=new_name;

  return ++(list_of_idb_database->count_of_idb_database_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_idb_database_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen INP's_Namen -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_idb_database_names (void)
{
  long i;

  for (i=0; i<list_of_idb_database->count_of_idb_database_name; i++)
    if(list_of_idb_database->names_of_idb_database[i]) \
      list_of_idb_database->names_of_idb_database[i] = \
        (char *)Free(list_of_idb_database->names_of_idb_database[i]);

  return list_of_idb_database->count_of_idb_database_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_idb_database_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ INPUT_DATABASE in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_idb_database_list (INPUT_DATABASE *idb)
{
  if(!list_of_idb_database) return 0;
  list_of_idb_database->count_of_idb_database=append_list(list_of_idb_database->idb_list,(void *) idb);
  return list_of_idb_database->count_of_idb_database;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_idb_database_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ INPUT_DATABASE der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender INPUT_DATABASE-Objekte
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long remove_idb_database_list (INPUT_DATABASE *idb)
{
  list_of_idb_database->count_of_idb_database = \
      remove_list_member(list_of_idb_database->idb_list,(void *) idb, destroy_idb_database);
  return list_of_idb_database->count_of_idb_database;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: idb_database_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines IDB-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des INP's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
long idb_database_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_idb_database->count_of_idb_database_name; i++)
    if(strcmp(list_of_idb_database->names_of_idb_database[i],name) == 0) break;
  if (i < list_of_idb_database->count_of_idb_database_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateInputDataBaseList
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
void CreateInputDataBaseList(void)
{
  list_of_idb_database = create_idb_database_list("INPUT_DATABASE");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyInputDataBaseList
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
void DestroyInputDataBaseList(void)
{
  long i;

  for(i=0; i<idb_names_number; i++)
    if (idb_names[i]) Free(idb_names[i]);
  if (idb_names) Free(idb_names);
  if (ipdb) destroy_ipdb_database(ipdb);
  ipdb=NULL;

  undeclare_idb_database_names();
  destroy_idb_database_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InputDataBaseListEmpty
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
int InputDataBaseListEmpty(void)
{
  return idb_database_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetInputDataBase
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E INPUT_DATABASE *idb: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetInputDataBase (char *name)
{
  char *new_name;

  idb_names=(char **) Realloc(idb_names,(idb_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  idb_names[idb_names_number]=new_name;
  idb_names[++idb_names_number]=NULL;

  return;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: IndexInputDataBase
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines IDB-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des INP's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
long IndexInputDataBase(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<idb_names_number; i++)
    if(strcmp(idb_names[i],name) == 0) break;
  if (i < idb_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: InitInputDataBase
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INPUT_DATABASE
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INPUT_DATABASE *idb -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INPUT_DATABASE
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *InitInputDataBase(char *db_name)
{
  INPUT_DATABASE *idb;

  if (!db_name) return NULL;

  /*if (!idb_database_name_exist(db_name) ) return NULL;*/

  idb=NULL;
  list_idb_init();
  while ( (idb=(INPUT_DATABASE *)get_list_idb_next()) != NULL ) {
      set_idb_found(idb,0);
  }

  idb=NULL;
  list_idb_init();
  while ( (idb=(INPUT_DATABASE *)get_list_idb_next()) != NULL ) {
    if ( strcmp(StrUp(get_idb_group_name(idb)),StrUp(db_name))==0 ) {
      set_idb_found(idb,1);
      set_idb_file_name(idb,db_name);
      SetInputDataBase(get_idb_master_keyword(idb));
    }      
  }

  return idb;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ReadInputDataBase
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INPUT_DATABASE
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INPUT_DATABASE *idb -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INPUT_DATABASE
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *ReadInputDataBase(char *db_name)
{
  INPUT_DATABASE *idb;

  if (!db_name) return NULL;

  /*if (!idb_database_name_exist(db_name) ) return NULL;*/

  idb=NULL;
  list_idb_init();
  while ( (idb=(INPUT_DATABASE *)get_list_idb_next()) != NULL ) {
    /*IDB_ReadDataBase(db_name,get_idb_master_keyword(idb),get_idb_S(idb),NULL,0);*/
    IDB_ReadDataBase(db_name,get_idb_master_keyword(idb),get_idb_S(idb), \
            idb_names,idb_names_number, get_idb_sep_string(idb), \
            get_idb_comments(idb),get_idb_beginn_comments(idb),get_idb_end_comments(idb),get_idb_sep_cap(idb));
  }

  return idb;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ReadInputDataBase
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp INPUT_DATABASE
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: INPUT_DATABASE *idb -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp INPUT_DATABASE
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
INPUT_DATABASE *ExecuteInputDataBase(char *db_name)
{
  INPUT_DATABASE *idb;

  if (!db_name) return NULL;


  /*if (!idb_database_name_exist(db_name) ) return NULL;*/

  idb=NULL;
  list_idb_init();
  while ( (idb=(INPUT_DATABASE *)get_list_idb_next()) != NULL ) {
    /*open_idb_database(idb);*/
    if (get_idb_read_function(idb))
      get_idb_read_function(idb)(get_idb_S(idb)->string,get_idb_found(idb),DB_GetProtPointerDataBase(db_name));
    /*close_idb_database(idb);*/
  }

  return idb;
}


/*------------------------------------------------------------------------*/
/* Schnittstellenfunktionen zu Rockflow                                   */
/*------------------------------------------------------------------------*/

/**************************************************************************/
/* ROCKFLOW - Funktion: SetSectionInputDataBase
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                      */
/**************************************************************************/
int SetSectionInputDataBase(char *db_name, 
                            char *master_keyword,  Read_Function read_function)
{
  return SetSectionInputDataBaseEx(db_name,master_keyword,read_function,NULL,NULL,NULL,NULL,NULL);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: SetSectionInputDataBaseEx
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                      */
/**************************************************************************/
int SetSectionInputDataBaseEx(char *db_name, 
         char *master_keyword,  Read_Function read_function, 
         char *sep_string, char *comments, char *beginn_comments, char *end_comments, char *sep_cap)
{
  INPUT_DATABASE *idb=NULL;
  static int i=0;
  static char *sep_string_new;           /* Trenn-Zeichen beim Mehrfachvorkommen */
  static char *comments_new;             /* Komentar-Zeichen */
  static char *beginn_comments_new;      /* Komentar-Beginn-Zeichen fuer Blockkommentar */
  static char *end_comments_new;         /* Komentar-End-Zeichen fuer Blockkommentar */
  /*static char *beginn_section_new;*/      /* Komentar-Beginn-Zeichen fuer Blockkommentar */
  /*static char *end_section_new;*/         /* Komentar-End-Zeichen fuer Blockkommentar */
  static char *sep_cap_new;             /* Trenn-Zeichen beim Mehrfachvorkommen (Lesen) */

  if(sep_string) sep_string_new=sep_string; else sep_string_new=ipdb->sep_string;
  if(comments) comments_new=comments; else comments_new=ipdb->comments;
  if(beginn_comments) beginn_comments_new=comments; else beginn_comments_new=ipdb->beginn_comments;
  if(end_comments) end_comments_new=end_comments; else end_comments_new=ipdb->end_comments;
  /*if(beginn_section) beginn_section_new=section; else beginn_section_new=ipdb->beginn_section;
  if(end_section) end_section_new=end_section; else end_section_new=ipdb->end_section;*/
  if(sep_cap) sep_cap_new=sep_cap; else sep_cap_new=ipdb->sep_cap;

  if ((idb=create_idb_database(db_name,master_keyword,read_function, \
              sep_string_new,comments_new,beginn_comments_new,end_comments_new,sep_cap_new) ) != NULL) {
    insert_idb_database_list(idb);
    i++;
  }

  if (idb) return i;
  else return -i;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetSectionPropertiesDataBase
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                      */
/**************************************************************************/
int SetSectionPropertiesDataBase(char *sep_string, char *comments, \
                               char *beginn_comments, char *end_comments, \
                               char *beginn_section, char *end_section, char *sep_cap)
{
  static int i=0;

  if (ipdb) destroy_ipdb_database(ipdb);
  ipdb=NULL;

  i++;
  ipdb=create_ipdb_database(sep_string,comments,beginn_comments, \
                            end_comments,beginn_section,end_section,sep_cap);

  if (ipdb) return i;
  else return -i;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: InitInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
int InitInputFile(char *db_name)
{
  InitInputDataBase(db_name);
  ReadInputDataBase(db_name);

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecureInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
int ExecuteInputFile(char *db_name)
{ /* ah ze */
  /*InitInputDataBase(db_name);
  ReadInputDataBase(db_name);*/
  ExecuteInputDataBase(db_name);
  return 1;
}
