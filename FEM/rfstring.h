/**************************************************************************/
/* Header-Datei: rfstring.h                                               */
/*                                                                        */
/* Aufgabe:                                                               */
/* String Bearbeitung                                                     */
/*                                                                        */
/*		                                                                  */
/**************************************************************************/
#ifndef rfstring_INC
#define rfstring_INC
#include <fstream>
#include <sstream>
using namespace std;
#include "geo_strings.h"
//#include "test.h"
  /* Schutz gegen mehrfaches Einfuegen */
/* =========  File: Basis-Funktionen  ========= */
/* verbesserte string funktionen */
int str_len(char *string);
char *str_cpy(char *destination, char *source);
/* Datei eroeffnen */
FILE *open_file( char *name, char *Attribut );
/* file Laenge ermitteln */
int get_file_length(char *file_name, long *length);
/* =========  String: Basis-Funktionen  ========= */
/* string ausgeben */
int string_printf(char *string);
/* string Laenge aktualisieren */
int string_length(char *string, long *length);
/* string2 addieren an Position pos vom string1 (Basisfunktion) */
char *string_insert_pos(char **string1, char *string2, long pos);
/* string freigeben */
int string_free(char **string);
/* Zeichenkette2 in Zeichenkette1 suchen */
int string_search(char *string1, char *string2, char **pp);
/* string mit einer laenge length llokieren */
char *string_alloc(long length);
/* =========  String-Datentyp  ========= */
/* Datentyp String */
typedef struct {
  char *string;
  long length;
  char *p;
} String;
/* =========  String-Funktionen  ========= */
/* String-Konstruktoren */
String *String_create_string(char *string);
String *String_create(void);
String *String_create_char(char c);
String *String_create_String(String *S);
/* String-Destruktor */
String *String_destroy(String *string);
/* Iterator */
String *String_iterator_init(String *S);
String *String_iterator_next(String *S);
String *String_iterator_preview(String *S);
long String_iterator_set_pos(String *S, long pos);
long String_iterator_get_pos(String *S);
String *String_iterator_set_first(String *S);
String *String_iterator_set_last(String *S);
String *String_iterator_next_word(String *S);
String *String_iterator_next_word_new(String *S);
String *String_iterator_delete_next_blanks(String *S);
String *String_iterator_begin(String *S);
String *String_iterator_end(String *S);
/* String-Laenge */
long String_get_length(String *string);
/* String-Laenge aktualisieren */
String *String_update_length(String *S);
/* String ausgeben */
String *String_printf(String *S);
/* String einfuegen */
String *String_insert_string_pos(String *S, char *string, long pos);
String *String_insert_pos(String *S1, String *S2, long pos);
String *String_insert_char_pos(String *S, char c, long pos);
/* String anhaengen */
String *String_append_string(String *S, char *string);
String *String_append(String *S1, String *S2);
String *String_append_char(String *S, char c);
/* String suchen */
int String_search_string(String *S, char *string);
int String_search(String *S1, String *S2);
int String_search_char(String *S, char c);
/* String kopieren */
String * String_copie_string(String *S, char *string);
String * String_copie(String *S1, String *S2);
String * String_copie_char(String *S, char c);
/* String ersetzen */
String *String_replace_string(String *S, char *string1, char *string2);
String *String_replace(String *S1, String *S2, char *string);
String *String_replace_char(String *S, char c, char cc);
/* String loeschen */
String * String_delete_string(String *S, char *string);
String * String_delete(String *S1, String *S2);
String * String_delete_char(String *S, char c);
/* String ersetzen */
String *String_replace_all_string(String *S, char *string1, char *string2);
String *String_replace_all(String *S1, String *S2, char *string);
String *String_replace_all_char(String *S, char c, char cc);
/* String loeschen */
String * String_delete_all_string(String *S, char *string);
String * String_delete_all(String *S1, String *S2);
String * String_delete_all_char(String *S, char c);
/* Double-Wert aus String lesen */
double String_sread_double ( String *S );
/* Int-Wert aus String lesen */
int String_sread_int ( String *S );
/* long-Wert aus String lesen */
long String_sread_long ( String *S );
/* Zeichenkette aus String lesen */
char *String_sread_string ( String *S, char *string );
/* String ausgeben */
String *String_printf(String *S);
String *String_putchar(String *S);
String *String_write_line(String *S);
String *String_putc(String *S, FILE *stream);
/* String lesen */
String *String_getc(String *S, FILE *stream, char char_end);
String *String_getchar(String *S);
String *String_read_line(String *S);
String *String_read(String *S, FILE *stream, char char_end);
/* String lesen */
long String_sread(String *S, char *stream, char char_end);
long String_read_keyword(String *SC, String *S, char *keyword, char begin_char, char end_char);
long String_read_Keyword(String *SC, String *S_source, char *keyword, \
                  char **keywords, long number_of_keywords, \
                  char *sep_string, char *comments, char *beginn_comments, char *end_comments, char *sep_kap);
char *String_sread_string_end_char ( String *S, char string[], char char_end );
long String_Sread(String *Destination, String *Source, char char_end);
long String_nSread(String *Destination, String *Source, long nchar);
long String_pos_nSread(String *Destination, String *Source, long pos, long nchar);
/* Komplette Datei in String lesen */
String *String_read_file_print_mode(String *S, char *file_name, int mode);
/* Abschnitt mit Laenge max_size aus Datei in String lesen */
String *String_read_section_file_print_mode(String *S, char *file_name, int mode, long max_size);
//------------------------------------------------------------------------
extern string GetLineFromFile2(ifstream *ein,bool*keyword,bool*subkeyword,ios::pos_type position);
#endif   /* rfstring_INC */
