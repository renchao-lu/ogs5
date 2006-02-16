/**************************************************************************/
/* ROCKFLOW - Modul: rfstring.c
 */
/* Aufgabe: Zeichenketten bearbeiten
 */
/* Programmaenderungen:
   10/2001    AH      Erste Version
    3/2002    CT      Korrektur in String_read_section_file_print_mode
 */
/**************************************************************************/

#include "stdafx.h"                    /* MFC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <ctype.h>*/
/*#include <sys/types.h>*/
#include <sys/stat.h>

#include "makros.h"
#include "rfstring.h"
#include "geo_strings.h"
#include "display.h"



/* =========  File: Basis-Funktionen  ========= */

/* -------------------------------------------------------------- */
int str_len(char *string)
{
  if (string) {
    return (int)strlen(string);
  }
  else return 0;
}
/* -------------------------------------------------------------- */

char *str_cpy(char *destination, char *source)
{
  if (destination && source) {
    return strcpy(destination, source);
  }
  else return destination;
}

/* ------------------------------------------------------------------ */
/* Datei eroeffnen                                                    */
/* ------------------------------------------------------------------ */
FILE *open_file( char *name, char *Attribut )
{
  FILE *fp;

  if((fp=fopen(name,Attribut))==NULL) {
    printf("\nFehler beim Eroeffnen der Datei %s !!!\n",name);
    return NULL;
  }
  rewind(fp);

  return fp;
}
/* ------------------------------------------------------------------ */


/* -------------------------------------------------------------- */
/* file Laenge holen
								  */
/* -------------------------------------------------------------- */
int get_file_length(char *file_name, long *length)
{
  struct stat status;

  /*#ifndef S_IFMT
  #define S_IFMT _IFMT
  #endif

  #ifndef S_IFDIR
  #define S_IFDIR _IFDIR
  #endif*/

  *length=0;
  if (stat(file_name,&status) == -1) return 0;
  /*if ( (status.st_mode & S_IFMT) == S_IFDIR) return 0;*/
  else {
    *length=(long )status.st_size;
  }
  return 1;
}
/* -------------------------------------------------------------- */


/* =========  String: Basis-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* string ausgeben
								  */
/* -------------------------------------------------------------- */
int string_printf(char *string)
{
  if ( (printf("%s",string)) == EOF ) return 0;
  else return 1;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* string Laenge aktualisieren
								  */
/* -------------------------------------------------------------- */
int string_length(char *string, long *length)
{
  *length=(int)strlen(string);
  if ( *length < 0) return 0;
  else return 1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* strings addieren (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
char *string_cat(char *string1, char *string2)
{
  char *buffer;

  buffer=(char *) Malloc((int)strlen(string1)+(int)strlen(string2)+1);
  if ( buffer == NULL ) {
     printf("\nNicht genug Speicher um einen neuen string zu erzeugen (string_cat)\n");
     return NULL;
  }
  strcpy(buffer,string1);
  strcat(buffer,string2);

  return buffer;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* string2 an Position pos vom string1 einfuegen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
char *string_insert_pos(char **string1, char *string2, long pos)
{
  char *buffer;
  char *buf;
  /*char *buf_end;*/
  long l1,l2,l;

  if (string1 == NULL) return NULL;

  if (*string1) l1=(int)strlen((*string1));
  else l1=0;
  if (string2) l2=(int)strlen((string2));
  else l2=0;

  l=l1+l2+1;


  buffer=(char *) Malloc(l);
  if ( buffer == NULL ) {
     printf("\nNicht genug Speicher um einen neuen string zu erzeugen (string_insert_pos)\n");
     return NULL;
  }
  buffer[l-1]='\0';

  if (pos > l1) pos=l1;

  buf=buffer;
/*  buf_end=buf+(int)strlen((*string1))+(int)strlen(string2);*/

  if (l1 > 0) strncpy(buffer,(*string1),pos);
  if (l1 > 0) buffer[pos]='\0';
  else buffer[0]='\0';
  if (l1>0) buf=(*string1)+pos;
  strcat(buffer,string2);
  if (l1>0) strcat(buffer,buf);

  /*if ( (*string1) ) Free(*string1);*/
  l=(int)strlen(buffer)+1;
  *string1=(char *)Realloc(*string1,l);
  strcpy(*string1,buffer);
  /**string1[l-1]='\0';*/
  Free(buffer);

  return *string1;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* string destroy
								  */
/* -------------------------------------------------------------- */
int string_free(char **string)
{
  if (*string) {
    Free(*string);
    *string=NULL;
    return 1;
  }
  else return 0;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette1 in Zeichenkette2 suchen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
int string_search_V0(char *string1, char *string2, char **pp)
{
  char c;
  char *buf,*buf_end;
  int l,found=0;
   
  buf=string1;
  buf_end=buf+(int)strlen(string1);

  while ( (c=*buf++) != '\0' )  /* Fuehrende Blanks uebergehen */
    if ( c == ' ' || c =='\t' || c =='\n') {
	  continue;
    }
    else break;
  buf--;

  l=(int)strlen(string2);
  while (buf && buf<buf_end) {
    if ( (strncmp(buf,string2,l)) == 0) found=1;
	if (found) break;
	buf++;
  }
 if (found) *pp=buf; else *pp=NULL;

  return (found)? 1: 0;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette1 in Zeichenkette2 suchen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
int string_search(char *string1, char *string2, char **pp)
{
  int found=0;
  *pp=strstr(string1,string2);
  if (*pp) found=1;
  return found;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Feld von Zeichenketten in Zeichenkette suchen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
char *string_array_search(char *string, char *string_array[], int n)
{
  char *buf=NULL;
  int i;
    
  for (i=0; i<n; i++) {
	if (string_array[i]) {
      buf=strstr(string,string_array[i]);
	  if (buf) break;
	}	  
  }

  if (!buf) return NULL;

  return buf;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette1 in Zeichenkette2 suchen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
int string_ncopie(char *string1, char *string2, long n)
{
  int found=0;
  char *p;
  p=strncpy(string1,string2,n);
  if (p) found=1;
  return found;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* string2 an Position pos vom string1 einfuegen (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
char *string_replace_pos(char **string, char *string1, char *string2, long pos)
{
  char *buffer;
  char *buf;
  /*char *buf_end;*/
  long l1,l2,l,ll;
  char **pp;
  char *p;
  long nc;

  if (string == NULL) return NULL;
  if (pos < 0) pos=0;

  /**pp=&(*string[pos]);*/
  p=*string+pos;
  pp=&p;
  /*if (!string_search(&*string[pos], string1, pp)) return NULL ;*/
  if (!string_search(*string+pos, string1, pp)) return NULL ;

  if (*string) l=(int)strlen((*string));
  else l=0;
  if (string1) l1=(int)strlen((string1));
  else l1=0;
  if (string2) l2=(int)strlen((string2));
  else l2=0;

  ll=l+l2-l1+1;
  if (ll<0) return NULL;

  buffer=(char *) Malloc(ll);
  if ( buffer == NULL ) {
     printf("\nNicht genug Speicher um einen neuen string zu erzeugen (string_insert_pos)\n");
     return NULL;
  }
  buffer[ll-1]='\0';

  if (pos > ll) pos=ll;

/*  buf_end=buf+(int)strlen((*string1))+(int)strlen(string2);*/
  nc=(long)(*pp-*string);
  if (ll > 0) strncpy(buffer,(*string),(int)(nc));
  if (ll > 0) buffer[nc]='\0';
  else buffer[0]='\0';
  strcat(buffer,string2);
  buffer[nc+l2]='\0';
  buf=*string;
  buf+=nc+l1;
  strcat(buffer,buf);
  buffer[ll-1]='\0';
  /*if ( (*string) ) Free(*string);*/
  /**string=buffer;*/
  *string=(char *)Realloc(*string,(int)strlen(buffer));
  strcpy(*string,buffer);
  Free(buffer);

  buf=*string;
  buf+=nc+l2;

  return buf;
}
/* -------------------------------------------------------------- */

/* =========  String-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* String(string) erzeugen
												  */
/* -------------------------------------------------------------- */
String *String_create_string(char *string)
{
  String *S;

  S = (String *) Malloc(sizeof(String));
  if ( S == NULL ) {
     printf("\nNicht genug Speicher um einen neuen String zu erzeugen (String_create_string)\n");
     return NULL;
  }

  S->string=NULL;
  S->length=0;
  S->p=NULL;

  if (string_insert_pos(&(S->string),string,0) == NULL) {
     Free(S);
     printf("\nNicht genug Speicher um einen neuen string zu erzeugen (String_create_string)\n");
     return NULL;
  }

  String_update_length(S);
  S->string[S->length]='\0';
  S->p=S->string;

  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String(void) erzeugen
								                                  */
/* -------------------------------------------------------------- */
String *String_create(void)
{
  return String_create_string("");
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String(char) erzeugen
								  */
/* -------------------------------------------------------------- */
String *String_create_char(char c)
{
  char *s=" ";
  s[0]=c;
  return String_create_string(s);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String(String) erzeugen
												  */
/* -------------------------------------------------------------- */
String *String_create_String(String *S)
{
  return String_create_string(S->string);
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String-Destruktor
								  */
/* -------------------------------------------------------------- */
String *String_destroy(String *S)
{

  if ( S != NULL ) {
    if ( S->string != NULL ) {
      string_free(&(S->string));
    }
    Free(S);
    S=NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator initialisieren
												  */
/* -------------------------------------------------------------- */
String *String_iterator_init(String *S)
{
  if (S) S->p=S->string;

  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator Vorwaerts bewegen
												  */
/* -------------------------------------------------------------- */
String *String_iterator_next(String *S)
{
  if (S) {
	if(*(S->p) == '\0') return NULL;
	else S->p++;
	if (*(S->p) == '\0') return NULL;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator Rueckwaerts bewegen
												  */
/* -------------------------------------------------------------- */
String *String_iterator_preview(String *S)
{
  if (S) {
	if(S->p == S->string) return NULL;
	else S->p--;
	if (S->p == S->string) return NULL;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Iterator an Position pos setzen
												  */
/* -------------------------------------------------------------- */
long String_iterator_set_pos(String *S, long pos)
{
  long save=-1;
  if (S) {
    save=String_iterator_get_pos(S);
    if ( pos<0 ) pos=0;
    if ( pos>String_get_length(S) ) pos=String_get_length(S);
	S->p = &S->string[pos];
  }
  return save;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Liefert die Position des Iterators
												  */
/* -------------------------------------------------------------- */
long String_iterator_get_pos(String *S)
{
  long pos=-1;
  if (S) {
    pos=(long)(S->p - S->string);
  }

  return pos;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Iterator an erste Position pos setzen
												  */
/* -------------------------------------------------------------- */
String *String_iterator_set_first(String *S)
{
  if (S) {
	S->p = S->string;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator an letzte Position pos setzen
												  */
/* -------------------------------------------------------------- */
String *String_iterator_set_last(String *S)
{
  if (S) {
	S->p = &S->string[String_get_length(S)-1];
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Iterator Vorwaerts bewegen (zv: binaer NULL konnte nicht verglichen werden !!) 
												  */
/* -------------------------------------------------------------- */
String *String_iterator_delete_next_blanks(String *S)
{
  char *ptr,c;
  int found=0;

  if (S) {
	if(*(S->p) == '\0') return NULL;
	ptr=S->p;
    while ( (c=*ptr) != '\0' )  /* Fuehrende Blanks uebergehen */
      if ( c == ' ' || c =='\t' || c =='\n') {
        found=1;
        ptr++;
	    continue;
	  }
    else break;

	S->p=ptr;
	if (*(S->p) == '\0') return NULL;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Iterator Vorwaerts bewegen (zv: binaer NULL konnte nicht 
   verglichen werden !!) 
												  */
/* -------------------------------------------------------------- */
String *String_iterator_next_word_new(String *S)
{
  char *ptr,c;
  int found=0;

  if (S) {
	/*if(*(S->p) == '\0') return NULL;
	if(!S->p) return NULL;*/

	if(*(S->p) == '\0') return NULL;
	ptr=S->p;
	if(*ptr == '\0') return NULL;
    while ( (c=*ptr) != '\0' )  /* Fuehrende Blanks uebergehen */
      if ( c == ' ' || c =='\t' || c =='\n') {
        ptr++;
        found=1;
	    continue;
	  }
    else break;
	S->p=ptr;
	if (*(S->p) == '\0') return NULL;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator Vorwaerts bewegen
												  */
/* -------------------------------------------------------------- */
String *String_iterator_next_word(String *S)
{
  char *ptr,c;
  if (S) {
	if(*(S->p) == '\0') return NULL;
	ptr=S->p;
    while ( (c=*ptr++) != '\0' )  /* Fuehrende Blanks uebergehen */
      if ( c == ' ' || c =='\t' || c =='\n') {
	    continue;
	  }
    else break;
    ptr--;
    while ( (c=*ptr++) != '\0' )  /* Word uebergehen */
      if ( c == ' ' || c =='\t' || c =='\n') {
	    break;
	  }
    else continue;
    ptr--;
    while ( (c=*ptr++) != '\0' )  /* Blanks uebergehen */
      if ( c == ' ' || c =='\t' || c =='\n') {
	    continue;
	  }
    else break;
    ptr--;

    
	S->p=ptr;
	if (*(S->p) == '\0') return NULL;
    return S;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Abfrage ob Iterator an letzte Position ist
												  */
/* -------------------------------------------------------------- */
String *String_iterator_end(String *S)
{
  if (S) {
	if (S->p == S->string+String_get_length(S)) return S;    
  }
  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Abfrage ob Iterator an erste Position ist
												  */
/* -------------------------------------------------------------- */
String *String_iterator_begin(String *S)
{
  if (S) {
	if (S->p == S->string) return S;
  }
  return NULL;
}
/* -------------------------------------------------------------- */




/* -------------------------------------------------------------- */
/* String-Laenge
								  */
/* -------------------------------------------------------------- */
long String_get_length(String *S)
{
  if (S) return S->length;
  else {
     printf("\nString existiert nicht (String_get_length)\n");
     return 0;
  }
}
/* -------------------------------------------------------------- */




/* -------------------------------------------------------------- */
/* Zeichenkette string an Position pos in String S einfuegen
								  */
/* -------------------------------------------------------------- */
String *String_insert_string_pos(String *S, char *string, long pos)
{
  if (string_insert_pos(&(S->string),string,pos) == NULL) return NULL;
  else {
    String_update_length(S);
    return S;
  }
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String S2 an Position pos in String S1 einfuegen
								  */
/* -------------------------------------------------------------- */
String *String_insert_pos(String *S1, String *S2, long pos)
{
  if (String_insert_string_pos(S1,S2->string,pos) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichen  c an Position pos in String S einfuegen (zv: Return-Wert)
   und Resultat liefern.
								  */
/* -------------------------------------------------------------- */
String *String_insert_char_pos(String *S, char c, long pos)
{
  char s[2];
  s[0]=c;
  s[1]='\0';

  if (String_insert_string_pos(S,s,pos) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string in String S anhaengen
								  */
/* -------------------------------------------------------------- */
String *String_append_string(String *S, char *string)
{
  String *rv = NULL;
  if (S && string) {
    if (String_insert_string_pos(S,string,String_get_length(S)) != NULL) rv = S;
  }
  return rv;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String S2 in String S1 anhaengen
								  */
/* -------------------------------------------------------------- */
String *String_append(String *S1, String *S2)
{
  if (String_append_string(S1,S2->string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S anhaengen
								  */
/* -------------------------------------------------------------- */
String *String_append_char(String *S, char c)
{
  String *rv = NULL;
  char s[2];
  s[0]=c;
  s[1]='\0';
  if (String_append_string(S,s) != NULL) rv = S;
  return rv;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string in String S suchen (ti S->p++)
								  */
/* -------------------------------------------------------------- */
int String_search_string(String *S, char *string)
{
  int found=0;
  char **pp=&S->p;
  char *ppp=S->p;
  /*if (S->p != S->string) S->p++;*/   /* tc */
  found=string_search(S->p,string,pp);
  if (found) S->p=*pp;
  else S->p=ppp;
  return found;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* String S2 in String S1 suchen
								  */
/* -------------------------------------------------------------- */
int String_search(String *S1, String *S2)
{
  return String_search_string(S1,S2->string);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* char c in String S suchen
								  */
/* -------------------------------------------------------------- */
int String_search_char(String *S, char c)
{
  char s[2];
  s[0]=c;
  s[1]='\0';
  return String_search_string(S,s);
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string in String S copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_copie_string(String *S, char *string)
{
  string_free(&S->string);
  if (String_insert_string_pos(S,string,0) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_copie(String *S1, String *S2)
{
  if (String_copie_string(S1,S2->string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_copie_char(String *S, char c)
{
  char s[2];
  s[0]=c;
  s[1]='\0';
  if (String_copie_string(S,s) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string in String S copieren (Zuweisung) noch nicht fertig !!!
								  */
/* -------------------------------------------------------------- */
String *String_ncopie_string(String *S, char *string, long n)
{
  n=0; /* ´Dummy-Funktion : Warnung entfernen */
  string_free(&S->string);
  if (String_insert_string_pos(S,string,0) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_ncopie(String *S1, String *S2, long n)
{
  n=0; /* ´Dummy-Funktion : Warnung entfernen */
  if (String_copie_string(S1,S2->string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_ncopie_char(String *S, char c, long n)
{
  char s[2];
  s[0]=c;
  s[1]='\0';
  n=0; /* ´Dummy-Funktion : Warnung entfernen */
  if (String_copie_string(S,s) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string1 in String S durch string2 ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_replace_string(String *S, char *string1, char *string2)
{
  char *ptr;
  if ( (ptr=string_replace_pos(&S->string,string1,string2,String_iterator_get_pos(S))) == NULL) return NULL;
  else {
    S->p=ptr;
    String_update_length(S);
	/*String_iterator_next_word(S);*/
    return S;
  }
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette string in String S2 durch string in String S durch string2 ersetzen
   String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_replace(String *S1, String *S2, char *string)
{
  if (String_replace_string(S1,S2->string,string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S durch char cc ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_replace_char(String *S, char c, char cc)
{
  char s[2],sc[2];
  s[0]=c;
  s[1]='\0';
  sc[0]=cc;
  sc[1]='\0';

  if (String_replace_string(S,s,sc) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string1 in String S durch string2 ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_delete_string(String *S, char *string)
{
  if (String_replace_string(S,string,"") == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette string in String S2 durch string in String S durch string2 ersetzen
   String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_delete(String *S1, String *S2)
{
  if (String_delete_string(S1,S2->string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S durch char cc ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_delete_char(String *S, char c)
{
  char s[2];
  s[0]=c;
  s[1]='\0';

  if (String_delete_string(S,s) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette string1 in String S durch string2 ersetzen (zv: Rueckgabewert=Anzahl)
								  */
/* -------------------------------------------------------------- */
String *String_replace_all_string(String *S, char *string1, char *string2)
{
  for(;;) {   /* pos=get_iterator_position */
    /*if (string_replace_pos(&S->string,string1,string2,0) == NULL) break;*/ /* error */
    if (String_replace_string(S,string1,string2) == NULL) break;
    else {
      String_update_length(S);
   }
  }
  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette string in String S2 durch string in String S durch string2 ersetzen
   String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_replace_all(String *S1, String *S2, char *string)
{
  if (String_replace_all_string(S1,S2->string,string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S durch char cc ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_replace_char_all(String *S, char c, char cc)
{
  char s[2],sc[2];
  s[0]=c;
  s[1]='\0';
  sc[0]=cc;
  sc[1]='\0';

  if (String_replace_all_string(S,s,sc) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Zeichenkette string1 in String S durch string2 ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_delete_all_string(String *S, char *string)
{
  if (String_replace_all_string(S,string,"") == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Zeichenkette string in String S2 durch string in String S durch string2 ersetzen
   String S2 in String S1 copieren (Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_delete_all(String *S1, String *S2)
{
  if (String_delete_all_string(S1,S2->string) == NULL) return NULL;
  else return S1;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* char c in String S durch char cc ersetzen
								  */
/* -------------------------------------------------------------- */
String *String_delete_all_char(String *S, char c)
{
  char s[2];
  s[0]=c;
  s[1]='\0';

  if (String_delete_all_string(S,s) == NULL) return NULL;
  else return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String-Laenge aktualisieren
								  */
/* -------------------------------------------------------------- */
String *String_update_length(String *S)
{

  if (S) {
      if ( !string_length(S->string,&(S->length)) ) {
	printf("\nFehler beim aktualisieren der String-Laenge (String_update_length)\n");
	return NULL;
      }
  }
  else {
     printf("\nString existiert nicht (String_update_length)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_sread(String *S, char *stream, char char_end)
{
  char c;
  char *buf,*buf_end;
  long n=0;

  buf=stream;
  buf_end=buf+(int)strlen(stream);

  if (S) {
    while ( (c=*buf++) != char_end  && buf < buf_end)
    {
      n++;
      String_append_char(S,c);
    }
  }
  else {
     printf("\nString existiert nicht (String_sread)\n");
     return n;
  }

  return n;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_Sread(String *Destination, String *Source, char char_end)
{
  char c;
  char *buf,*buf_end;
  long n=0;

  if (!Source) return 0;  

  buf=Source->p;
  buf_end=Source->string+(int)strlen(Source->string);

  if (Destination) {
    while ( (c=*buf++) != char_end  && buf < buf_end)
    {
      n++;
      String_append_char(Destination,c);
    }
  }
  else {
     printf("\nString existiert nicht (String_Sread)\n");
     return n;
  }

  return n;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_nSread(String *Destination, String *Source, long nchar)
{
  char c;
  char *buf,*buf_end;
  long n=0;

  if (!Source) return 0;  

  buf=Source->p;
  buf_end=Source->string+(int)strlen(Source->string);

  if (Destination) {
    while ( n<nchar && (c=*buf++) != '\0' && buf < buf_end)
    {
      n++;
      String_append_char(Destination,c);
    }
  }
  else {
     printf("\nString existiert nicht (String_Sread)\n");
     return n;
  }

  return n;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_pos_nSread(String *Destination, String *Source, long pos, long nchar)
{
  char c;
  char *buf,*buf_end;
  long n=0;

  if (!Source) return 0;  

  buf=Source->p;
  buf_end=Source->string+(int)strlen(Source->string);

  if (pos)
    while ( pos-- && *buf++ && buf < buf_end);

  if (Destination) {
    while ( n<nchar && (c=*buf++) != '\0' && buf < buf_end)
    {
      n++;
      String_append_char(Destination,c);
    }
  }
  else {
     printf("\nString existiert nicht (String_Sread)\n");
     return n;
  }

  return n;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_read_keyword(String *SC, String *S, char *keyword, char begin_char, char end_char)
{
  long i=0;
  String_iterator_init(S);
  while ( String_search_string(S,keyword) ) {
	  i++;
	  String_iterator_next_word(S);
	  String_Sread(SC,S,begin_char);
	  String_iterator_next_word(S);
	  String_Sread(SC,S,end_char);
	  String_iterator_next_word(S);
  }
  
  return i;
}


/* -------------------------------------------------------------- */
/* String aus einer Zeichenkette lesen
								  */
/* -------------------------------------------------------------- */
long String_read_Keyword(String *SC, String *S_source, char *keyword, \
                  char **keywords, long number_of_keywords, \
                  char *sep_string, char *comments, char *beginn_comments, char *end_comments, char *sep_kap)
{
  long i=0,j,save,pos,last;
  String *S_tmp=String_create();
  String *S_tmp0=String_create();
  String *S=String_create();
  /*char *ptr;*/
  int found=0;

  String_copie(S,S_source);
  /*String_replace_all_string(S,"#"," ");*/

  /* Block-Komentare aus S entfernen. Ergebnis liegt in S_tmp0 */
  String_iterator_init(S);
  do 
  {
    last=pos=String_iterator_get_pos(S);
    if ( String_search_string(S,beginn_comments) ) {
      save=String_iterator_get_pos(S);
      if (String_search_string(S,end_comments) ) {
        String_iterator_next_word(S);
        pos=String_iterator_get_pos(S);
      }
      else { 
        String_iterator_next_word(S);
        pos=String_iterator_get_pos(S);
      }
    }
    else {
      save=pos=String_get_length(S);
    }
    String_iterator_set_pos(S,last);
	String_nSread(S_tmp0,S,save-last);
    String_iterator_set_pos(S,pos);
  } while ( String_iterator_next_word_new(S) );


  /* Zeilen-Komentare aus S_tmp0 entfernen. Ergebnis liegt in S_tmp */
  String_iterator_init(S_tmp0);
  do 
  {
    last=pos=String_iterator_get_pos(S_tmp0);
    if ( String_search_string(S_tmp0,comments) ) {
      save=String_iterator_get_pos(S_tmp0);
      if (String_search_string(S_tmp0,"\n") ) {
        pos=String_iterator_get_pos(S_tmp0);
      }
      else { 
        String_iterator_next_word(S);
        pos=String_iterator_get_pos(S_tmp0);
      }
    }
    else {
      save=pos=String_get_length(S_tmp0);
    }
    String_iterator_set_pos(S_tmp0,last);
	String_nSread(S_tmp,S_tmp0,save-last);
    String_iterator_set_pos(S_tmp0,pos);
  } while ( String_iterator_next_word_new(S_tmp0) );


  String_iterator_init(S_tmp);
  while ( String_search_string(S_tmp,keyword) ) {    
    /*ptr=S_tmp->p+(int)strlen(keyword);
    if ( (*(ptr) == ' ') || (*(ptr) == '\t') || (*(ptr) == '\n') ) ok=1;
    else continue; */
	i++;
	String_append_string(SC,sep_kap); /* Hier besteht die Moeglichkeit */
	String_append_string(SC," ");
	String_iterator_next_word(S_tmp);    /* den Zaehler hinzufuegen um den */
    save=String_iterator_get_pos(S_tmp); /* den Abschnitt zu identifizieren */
    found=0;
    for (j=0; j<number_of_keywords; j++) {
      String_iterator_set_pos(S_tmp,save);
      if( String_search_string(S_tmp,keywords[j]) ) {
        /*ptr=S_tmp->p+(int)strlen(keyword);
        if ( (*(ptr) == ' ') || (*(ptr) == '\t') || (*(ptr) == '\n') ) ok=1;
        else break; */
        found=1;
        /*if ( strcmp(StrUp(keyword),StrUp(keywords[j]))==0 ) {
	      String_append_string(SC,sep_string);
        }*/
        break;
      }
    }
    if (found) {
      pos=String_iterator_get_pos(S_tmp);
      String_iterator_set_pos(S_tmp,save);
      if (sep_string) pos--;
	  String_nSread(SC,S_tmp,pos-save);
      String_iterator_set_pos(S_tmp,pos);
    }
    else String_iterator_set_pos(S_tmp,save);
  }

  /*printf("\nKeyword = %s\n",keyword);*/
  /*String_printf(SC);*/

  String_destroy(S_tmp);
  String_destroy(S_tmp0);
  String_destroy(S);

  return i;
}


/* -------------------------------------------------------------- */
/* Double-Wert aus String lesen
								  */
/* -------------------------------------------------------------- */
double String_sread_double ( String *S )
{
  double x;
  int n;
  char c;

  if ( sscanf(S->p,"%lf%n",&x,&n) <= 0 ) {
     printf("\nFehler: double nicht gelesen (String_sread_double)\n");
     return 0.;
  }
  else {
    S->p+=n;
	while ( c=*S->p++, c == ' ' || c == '\n' || c == '\t')
		continue;
    S->p--;
    return x;
  }
}


/* -------------------------------------------------------------- */
/* Int-Wert aus String lesen
								  */
/* -------------------------------------------------------------- */
int String_sread_int ( String *S )
{
  int x;
  int n;

  if ( sscanf(S->p,"%d%n",&x,&n) <= 0 ) {
     printf("\nFehler: int nicht gelesen (String_sread_int)\n");
     return 0;
  }
  else {
    S->p+=n;
    return x;
  }
}


/* -------------------------------------------------------------- */
/* long-Wert aus String lesen
								  */
/* -------------------------------------------------------------- */
long String_sread_long ( String *S )
{
  long x;
  int n;

  if ( sscanf(S->p,"%ld%n",&x,&n) <= 0 ) {
     printf("\nFehler: long nicht gelesen (String_sread_long)\n");
     return 0;
  }
  else {
    S->p+=n;
    return x;
  }
}



/* -------------------------------------------------------------- */
/* String aus String lesen
								  */
/* -------------------------------------------------------------- */
char *String_sread_string ( String *S, char string[] )
{
  int n;

  if (!string) return NULL;

  if ( (sscanf(S->p,"%s%n",string,&n)) <= 0 ) {
     printf("\nFehler: string nicht gelesen (String_sread_string)\n");
     return NULL;
  }
  else {
    S->p+=n;
    return string;
  }
}



/* -------------------------------------------------------------- */
/* String aus String lesen
								  */
/* -------------------------------------------------------------- */
char *String_sread_string_end_char ( String *S, char string[], char char_end )
{
  char c;
  char *buf,*buf_end;
  long n=0;

  if (S) {
    buf=S->p;
    buf_end=S->string+String_get_length(S);
    while ( (c=*buf++) != char_end  && buf < buf_end)
    {
      string[n]=c;
      n++;
    }
	string[n]='\0';
	S->p+=n;
	return string;
  }
  else {
     printf("\nString existiert nicht (String_sread_string_end_char)\n");
     return NULL;
  }
}




/* -------------------------------------------------------------- */
/* String ausgeben
								  */
/* -------------------------------------------------------------- */
String *String_printf(String *S)
{
  if (S) {
    if ( (printf("%s",S->string)) == EOF ) {
       printf("\nString konnte nicht ausgegeben werden !!! (String_printf)\n");
       S=String_destroy(S);
    }
  }
  else {
     printf("\nString existiert nicht (String_printf)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String ausgeben
								  */
/* -------------------------------------------------------------- */
String *String_putc(String *S, FILE *stream)
{
  long i=0;

  if (S) {
    while ( S->string[i] && i<String_get_length(S) )
    if ( (putc(S->string[i++],stream)) == EOF ) {
       printf("\nString konnte nicht ausgegeben werden !!! (String_putc)\n");
       S=String_destroy(S);
    }
  }
  else {
     printf("\nString existiert nicht (String_putc)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* String ausgeben
								  */
/* -------------------------------------------------------------- */
String *String_putchar(String *S)
{
  return String_putc(S,stdout);
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* String ausgeben
								  */
/* -------------------------------------------------------------- */
String *String_write_line(String *S)
{
  S=String_putc(S,stdout);
  printf("\n");
  return S;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* String lesen (Benutzen als Zuweisung)
								  */
/* -------------------------------------------------------------- */
String *String_getc(String *S, FILE *stream, char char_end)
{
  char c;
  char buffer[1024];
  char *buf,*buf_end;

  buf=buffer;
  buf_end=buf+1023;

  if (S) {
    while ( (c=(char)getc(stream)) != EOF && c != char_end && buf < buf_end )
    {
      *buf++=c;
    }
    *buf='\0';
    S=String_destroy(S);
    S=String_create_string(buffer);
    if ( buf == buf_end ) {  /* zv Realloc */
      printf("\nWarnung : Zeile zu gross !!! (String_getc)\n");
      printf("\n"); String_putchar(S); printf("\n");
      if ( c != EOF && c != char_end )
      while ( (c=(char)getc(stream)) != EOF && c != char_end );
    }
  }
  else {
     printf("\nString existiert nicht (String_getc)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* String lesen
								  */
/* -------------------------------------------------------------- */
String *String_read(String *S, FILE *stream, char char_end)
{
  char c;

  if (S) {
    while ( (c=(char)getc(stream)) != EOF && c != char_end )
    {
      String_append_char(S,c);
    }
  }
  else {
     printf("\nString existiert nicht (String_read)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String lesen
								  */
/* -------------------------------------------------------------- */
String *String_getchar(String *S)
{
  return String_getc(S,stdin,'\n');
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String lesen (Benutzen als Zuweisung) !!!
								  */
/* -------------------------------------------------------------- */
String *String_read_line(String *S)
{
  return String_getc(S,stdin,'\n');
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* String aus Datei lesen
								  */
/* -------------------------------------------------------------- */
String *String_fread_line(String *S, FILE *stream)
{
  return String_getc(S,stream,'\n');
}
/* -------------------------------------------------------------- */


/* ah db */


/* -------------------------------------------------------------- */
/* string mit einer laenge length  allokieren (Basisfunktion)
								  */
/* -------------------------------------------------------------- */
char *string_alloc(long length)
{
  char *buffer;
  long i;

  buffer=(char *) Malloc(length+1);
  if ( buffer == NULL ) {
     printf("\nNicht genug Speicher um einen neuen string zu erzeugen (string_alloc)\n");
     return NULL;
  }
  for (i=0; i<length; i++) buffer[i]=' ';
  buffer[length]='\0';

  return buffer;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Komplette Datei in String lesen
								  */
/* -------------------------------------------------------------- */
String *String_read_file_print_mode(String *S, char *file_name, int mode)
{
  FILE *stream;
  char c;
  long length,i=0;
  char *buf,*buf_end;

  mode=0; /* ´Dummy-Funktion : Warnung entfernen */

  stream=open_file(file_name,"r");
  if (!stream) return NULL;

  if (S) {
    if ( !get_file_length(file_name,&length) ) {
      printf("\n!!! File laenge konnte nicht ermittelt werden (String_read_file)\n");
      return NULL;
    }
    if (S->string) string_free(&(S->string));
    /*printf("\n!!! File laenge = %ld\n",length);*/

    S->string=string_alloc(length);
    /*printf("\n strlen liefert %ld \n",(int)strlen(S->string));*/
    if ( !S->string ) {
      printf("\n!!! File laenge konnte nicht ermittelt werden (String_read_file)\n");
      return NULL;
    }
    buf=S->string;
    buf_end=buf+length;

    i=0;
    while ( !feof(stream) && (c=(char)getc(stream)) != EOF  && buf < buf_end )
    {
      if ( isprint(c) || isspace(c)) S->string[i++]=c;
      else S->string[i++]=' ';
      buf++;
    }
    S->string[length]='\0';
    String_update_length(S);
  }
  else {
     printf("\nString existiert nicht (String_read)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Abschnitt mit Laenge max_size aus Datei in String lesen
								  */
/* -------------------------------------------------------------- */
String *String_read_section_file_print_mode(String *S, char *file_name, int mode, long max_size)
{
  FILE *stream;
  char c;
  long length,i=0;
  char *buf,*buf_end;

  mode=0;
  /*if (!mode) {*/
    stream=open_file(file_name,"r");
    if (!stream) return NULL;
  /*}*/

  if (S) {
    if ( !get_file_length(file_name,&length) ) {
      printf("\n!!! File laenge konnte nicht ermittelt werden (String_read_file)\n");
      return NULL;
    }
    if (S->string) string_free(&(S->string));
    /*printf("\n!!! File laenge = %i\n",length);*/

    if (length > max_size) length=max_size;

    S->string=string_alloc(length);
    printf("\n strlen liefert %ld \n",(long)(int)strlen(S->string));
    if ( !S->string ) {
      printf("\n!!! File laenge konnte nicht ermittelt werden (String_read_file)\n");
      return NULL;
    }
    buf=S->string;
    buf_end=buf+length;

    i=0;
    while ( !feof(stream) && (c=(char)getc(stream)) != EOF  && buf < buf_end )
    {
      if ( isprint(c) || isspace(c)) S->string[i++]=c;
      else S->string[i++]=' ';
      buf++;
    }
    S->string[length]='\0';
    String_update_length(S);
  }
  else {
     printf("\nString existiert nicht (String_read)\n");
     return NULL;
  }

  return S;
}
/* -------------------------------------------------------------- */

/**************************************************************************/
/* ROCKFLOW - Funktion: GetLineFromFile1
                                      */
/* Aufgabe:
   Liest aus dem Eingabefile *ein die nächste Zeile
   Fängt die Zeile mit ";" an oder ist sie leer, wird sie ausgelassen
   Rückgabe ist ist ein string mit dem Zeileninhalt ab dem ersten Nicht-Leerzeichen 
   bis zum ersten Auftreten des Kommentartzeichens ";"
                                            */
/* Programmaenderungen:
    05/2004     SB  First Version
                                   */
/**************************************************************************/


string GetLineFromFile2(ifstream *ein,bool*keyword,bool*subkeyword,ios::pos_type position)
{
  string line, zeile = "";
  int fertig=0, i, j;
  char zeile1[MAX_ZEILE];
  keyword = false;
  subkeyword = false;
  //----------------------------------------------------------------------
  while(fertig<1){
    position = ein->tellg();
    if(ein->getline(zeile1,MAX_ZEILE)){				//Zeile lesen
	  line = zeile1;							//character in string umwandeln
	  i = (int) line.find_first_not_of(" ",0);		//Anfängliche Leerzeichen überlesen, i=Position des ersten Nichtleerzeichens im string
	  j = (int) line.find(";",i) ;					//Nach Kommentarzeichen ; suchen. j = Position des Kommentarzeichens, j=-1 wenn es keines gibt.
	  if(j!=i)fertig = 1;						//Wenn das erste nicht-leerzeichen ein Kommentarzeichen ist, zeile überlesen. Sonst ist das eine Datenzeile
	  if((i != -1))
		zeile = line.substr(i,j-i);   //Ab erstem nicht-Leerzeichen bis Kommentarzeichen rauskopieren in neuen substring, falls Zeile nicht leer ist
    }
    else{//end of file found
       fertig=1;
    }
  }// end while(...)
  //----------------------------------------------------------------------
  // Check keywords
  if(zeile.find("$")!=string::npos)
    *subkeyword = true;
  if(zeile.find("#")!=string::npos)
    *keyword = true;
  //----------------------------------------------------------------------
  return zeile;
}
