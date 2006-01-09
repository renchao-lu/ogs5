/**************************************************************************/
/* ROCKFLOW - Modul: plains.h
                                                                          */
/* Aufgabe:
   Flaechenverzeichnis (nur fuer 3D)
                                                                          */
/**************************************************************************/

#ifndef plains_INC

#define plains_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
typedef struct {
    long nachbarn_1D[4];
    long nachbar_2D;
    long nachbarn_3D[2];
    long knoten[4];
    long kinder[4];
    long vorgaenger;
} Flaeche;


extern int CreatePlainList ( void );
  /* Erzeugt Datenstruktur fuer Flaechenverzeichnis */
extern void DestroyPlainList ( void );
  /* Entfernt Flaechenverzeichnis aus Speicher */
extern void ConstructPlainList ( void );
  /* Berechnet Flaechenverzeichnis komplett neu */
extern void ConstructPlainListFast ( void );
  /* Berechnet Flaechenverzeichnis komplett neu */
extern Flaeche *NewPlain ( void );
  /* Erzeugt und initialisiert Flaechenstruktur */
extern long AddPlain ( Flaeche *flaeche );
  /* Haengt Flaeche an Flaechenverzeichnis an, Ergebnis ist Nummer der Flaeche */
extern void PlacePlain ( Flaeche *flaeche, long pos );
  /* Haengt Flaeche an Flaechenverzeichnis an Position pos an */
extern Flaeche *GetPlain ( long number );
  /* liefert Zeiger auf Flaechenstruktur der Flaeche number */
extern int DeletePlain ( long number );
  /* Loescht Flaeche mit der Nummer number */
extern long PlainListSize (void);
  /* Liefert den hoechsten Index aller Flaechen + 1 (bzw. 0 bei 0 Flaechen) */
extern int ReAssign1DElementPlains ( long ele, long *flaechen, int anz );
  /* Traegt das 1D-Element elem bei anz Flaechen flaechen aus */
extern int ReAssign2DElementPlains ( long ele, long *flaechen, int anz );
  /* Traegt das 2D-Element elem bei anz Flaechen flaechen aus; eine Flaeche ohne
     2D- oder 3D-Nachbarn wird automatisch entfernt */
extern int ReAssign3DElementPlains ( long ele, long *flaechen, int anz );
  /* Traegt das 3D-Element elem bei anz Flaechen flaechen aus; eine Flaeche ohne
     2D- oder 3D-Nachbarn wird automatisch entfernt */

extern int PDrehsinn ( long *fknoten, long *eknoten, int fn );
  /* Ermittelt Drehsinn einer Flaeche zu einer Elementflaeche;
     0: positiv, 1: negativ */
extern int ComparePlains ( long *fknoten, long *eknoten, int fn );
  /* Bestimmt die Existenz einer angegebenen Flaeche als Elementflaeche;
     0: keine Elementflaeche, 1: positiv, 2: negativ */
extern int ComparePlains1D ( long *fknoten, long *eknoten );
  /* Bestimmt die Existenz einer angegebenen Flaeche als Nachbar eines
     1D-Elements;
     0..4: Index, wenn Flaeche angrenzt;
     -1: Flaeche grenzt nicht an */


/* Weitere externe Objekte */
extern long PlainListLength;
  /* ANZAHL der aktuell gespeicherten Flaechen */
extern int fk[6][4];
  /* Elementknotenindizes der 6 Elementflaechen */


#endif
