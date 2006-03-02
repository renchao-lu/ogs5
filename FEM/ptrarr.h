/**************************************************************************/
/* ROCKFLOW - Modul: ptrarr.h
                                                                          */
/* Aufgabe:
   Dynamische Pointer-Felder (Bibliothek)
                                                                          */
/**************************************************************************/

#ifndef ptrarr_INC

#define ptrarr_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
typedef struct {                /* Dynamisches Feld von Pointern */
    void **elements;            /* Feld mit den Pointern */
    long len;                   /* Anzahl der Elemente */
    long size;                  /* Groesse des Feldes */
    long inc_size;              /* Schrittweite bei automatischer Erweiterung
                                   des Feldes */
    } ZeigerFeld;

extern ZeigerFeld *CreatePtrArray(long size, long inc_size);
  /* Erzeugt Pointer-Feld */
extern void DestroyPtrArray(ZeigerFeld *arr);
  /* Zerstoert Pointer-Feld */
extern int ResizePtrArray(ZeigerFeld *arr, long size);
  /* veraendert die Groesse des Feldes */
extern long PtrArrayLength(ZeigerFeld *arr);
  /* liefert die Anzahl der Feldelemente incl. Loecher (hoechster Index + 1 ) */
extern long PtrArraySize(ZeigerFeld *arr);
  /* liefert die Groesse des Feldes (belegter Speicher) */
extern void *GetPtrArrayElement(ZeigerFeld *arr, long i);
  /* liest ein Feldelement */
extern void *SetPtrArrayElement(ZeigerFeld *arr, long i, void *ele);
  /* setzt ein Feldelement */
extern void ShortenPtrArray(ZeigerFeld *arr, long len);
  /* reduziert die Elementanzahl, ohne den Speicher freizugeben */
extern long GetLowestFree(ZeigerFeld *arr, long pos);
  /* Liefert niedrigsten freien Feldindex >=pos */
extern long CompressPtrArray(ZeigerFeld *arr);
  /* Entfernt Nullzeiger aus dem Feld, Ergebnis ist Anzahl der Elemente */


/* Weitere externe Objekte */


#endif
