/**************************************************************************/
/* ROCKFLOW - Modul: ptrarr.c
                                                                          */
/* Aufgabe:
   Dynamische Pointer-Felder (Bibliothek)
   Es wird ein Pointer-Feld erzeugt, das Zeiger auf beliebige
   Datenstrukturen verwaltet.
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann   Erste Version
   01/1994     MSR               Angepasst und erweitert fuer ROCKFLOW
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */

#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "ptrarr.h"


/* Interne (statische) Deklarationen */
int AllocPtrArray(ZeigerFeld *arr, long size);
void FreePtrArray(ZeigerFeld *arr);


/* Definitionen */

/**************************************************************************/
/* ROCKFLOW - Funktion: CreatePtrArray
                                                                          */
/* Aufgabe:
   Feld im dynamischen Speicher erzeugen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long size: Feldgroesse
   E long inc_size: Schrittweite bei automatischer Erweiterung des Feldes
                                                                          */
/* Ergebnis:
   Zeiger auf das Feld, bei Fehler NULL
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
ZeigerFeld *CreatePtrArray ( long size, long inc_size )
{
    ZeigerFeld *arr = (ZeigerFeld *) Malloc(sizeof(ZeigerFeld));

    if (arr == NULL)
        return NULL;

    if (!AllocPtrArray(arr, size)) {
        arr = (ZeigerFeld *) Free(arr);
        return NULL;
    }

    arr->inc_size = inc_size;

    return arr;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AllocPtrArray
                                                                          */
/* Aufgabe:
   Feldelemente im dynamischen Speicher allokieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
   E long size: einzurichtende Feldgroesse
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int AllocPtrArray ( ZeigerFeld *arr, long size )
{
    arr->elements = NULL;
    arr->len = 0l;
    arr->size = 0l;

    return ResizePtrArray(arr, size);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FreePtrArray
                                                                          */
/* Aufgabe:
   Feldelemente (Zeiger) im dynamischen Speicher freigeben;
   die Speicherbereiche der eigentlichen Daten muessen vorher schon
   freigegeben worden sein.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
void FreePtrArray ( ZeigerFeld *arr )
{
    arr->elements = (void **) Free(arr->elements);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyPtrArray
                                                                          */
/* Aufgabe:
   Feld im dynamischen Speicher freigeben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
void DestroyPtrArray ( ZeigerFeld *arr )
{
    FreePtrArray(arr);
    arr = (ZeigerFeld *) Free(arr);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ResizePtrArray
                                                                          */
/* Aufgabe:
   Feldgroesse aendern; wenn das Feld verkleinert wird, wird der nicht
   mehr benoetigte Speicher freigegeben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
   E long size: neue Feldgroesse
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
int ResizePtrArray ( ZeigerFeld *arr, long size )
{
    long i;
    void **elements;

    if (size <= 0l) {
        if (arr->elements != NULL)
            arr->elements = (void **) Free(arr->elements);
        arr->elements = NULL;
        arr->len = arr->size = 0l;
        return 1;
    }

    if (arr->elements == NULL)
        elements = (void **) Malloc(size * sizeof(void *));
    else
        elements = (void **) Realloc(arr->elements, size * sizeof(void *));
    if (elements == NULL)
        return 0;

    for (i = arr->size; i < size; i++)
        elements[i] = NULL;
    arr->elements = elements;
    if (arr->len > size)
        arr->len = size;
    arr->size = size;

    return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PtrArrayLength
                                                                          */
/* Aufgabe:
   Anzahl der Elemente des Feldes ermitteln incl. Loecher
   (hoechster Inex +1)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ZeigerFeld *arr: Pointer-Feld
                                                                          */
/* Ergebnis:
   Anzahl der Feldelemente
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
long PtrArrayLength ( ZeigerFeld *arr )
{
    return arr->len;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PtrArraySize
                                                                          */
/* Aufgabe:
   Feldgroesse ermitteln (ist groesser oder gleich der Elementanzahl)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ZeigerFeld *arr: Pointer-Feld
                                                                          */
/* Ergebnis:
   Groesse des Feldes
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
long PtrArraySize ( ZeigerFeld *arr )
{
    return arr->size;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetPtrArrayElement
                                                                          */
/* Aufgabe:
   Feldelement ermitteln
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ZeigerFeld *arr: Pointer-Feld
   E long i: Elementindex
                                                                          */
/* Ergebnis:
   void-Zeiger auf das Feldelement; nicht gefunden: NULL
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
void *GetPtrArrayElement ( ZeigerFeld *arr, long i )
{
    if (i < 0l || i >= arr->len)
        return NULL;
    return arr->elements[i];
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetPtrArrayElement
                                                                          */
/* Aufgabe:
   Feldelement setzen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
   E long i: Elementindex
   E void *ele: void-Zeiger auf neues Element
                                                                          */
/* Ergebnis:
   void-Zeiger auf das alte Feldelement; neu oder Fehler: NULL
                                                                          */
/* Programmaenderungen:
      1993     Thomas Hartmann        Erste Version
                                                                          */
/**************************************************************************/
void *SetPtrArrayElement ( ZeigerFeld *arr, long i, void *ele )
{
    void *oldele = NULL;

    if (i < 0l)
        return NULL;
    if (i >= arr->size) {
        long size = i + 1l;
        if (size < arr->size + arr->inc_size)
            size = arr->size + arr->inc_size;
        if (!ResizePtrArray(arr, size))
            return NULL;
    }
    if (i >= arr->len) {
        arr->len = i + 1l;
    } else
        oldele = arr->elements[i];
    arr->elements[i] = ele;
    return oldele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ShortenPtrArray
                                                                          */
/* Aufgabe:
   Anzahl der Feldelemente reduzieren, aber Speicher nicht freigeben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
   E long len: neue Feldelementanzahl
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1994     MSR           Erste Version
                                                                          */
/**************************************************************************/
void ShortenPtrArray ( ZeigerFeld *arr, long len )
{
    long i;

    if (len < arr->len) {
        if (len <= 0l)
            len = 0l;
        for (i = len; i < arr->len; i++)
            arr->elements[i] = NULL;
        arr->len = len;
    }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetLowestFree
                                                                          */
/* Aufgabe:
   Liefert niedigsten freien Feldindex
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
   E pos: Feldindex, ab dem aufwaerts nach freiem Element gesucht wird
                                                                          */
/* Ergebnis:
   Niedrigster freier Feldindex
                                                                          */
/* Programmaenderungen:
   02/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long GetLowestFree ( ZeigerFeld *arr, long pos )
{
  if (pos >= arr->len) return pos;
  while (arr->elements[pos]!=NULL) {
      pos++;
      if (pos >= arr->len) return pos;
  }
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CompressPtrArray
                                                                          */
/* Aufgabe:
   Komprimiert Pointer-Feld, indem alle Nullzeiger nach hinten sortiert
   werden und die Laenge angepasst wird.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ZeigerFeld *arr: Pointer-Feld
                                                                          */
/* Ergebnis:
   Anzahl der Feldelemente
                                                                          */
/* Programmaenderungen:
   02/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long CompressPtrArray ( ZeigerFeld *arr )
{
  long anz = 0;
  long i;
  for(i=0; i<arr->len; i++) {
      if (arr->elements[i] != NULL) {
          if (i != anz) {
              arr->elements[anz] = arr->elements[i];
              arr->elements[i] = NULL;
          }
          anz++;
      }
  }
  arr->len = anz;
  return anz;
}


