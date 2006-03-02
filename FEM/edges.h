/**************************************************************************/
/* ROCKFLOW - Modul: edges.h
                                                                          */
/* Aufgabe:
   Kantenverzeichnis
                                                                          */
/**************************************************************************/

#ifndef edges_INC

#define edges_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
typedef struct {
    long *nachbarn_2D;   /* zugehoerige 2D-Nachbarelemente */
    int anz_nachbarn_2D; /* Anzahl der zugehoerigen 2D-Nachbarelemente */
    long nachbar_1D;
    long knoten[3];
    long kinder[2];
    long vorgaenger;
} Kante;


extern int CreateEdgeList ( void );
  /* Erzeugt Datenstruktur fuer Kantenverzeichnis */
extern void DestroyEdgeList ( void );
  /* Entfernt Kantenverzeichnis aus Speicher */
extern void ConstructEdgeList ( void );
  /* Berechnet Kantenverzeichnis komplett neu */
extern void ConstructEdgeListFast ( void );
  /* Berechnet Kantenverzeichnis komplett neu */
extern Kante *NewEdge ( void );
  /* Erzeugt und initialisiert Kantenstruktur */
extern long AddEdge ( Kante *kante );
  /* Haengt Kante an Kantenverzeichnis an, Ergebnis ist Nummer der Kante */
extern void PlaceEdge ( Kante *kante, long pos );
  /* Haengt Kante an Kantenverzeichnis an Position pos an */
extern Kante *GetEdge ( long number );
  /* liefert Zeiger auf Kantenstruktur der Kante number */
extern int DeleteEdge ( long number );
  /* Loescht Kante mit der Nummer number */
extern long EdgeListSize (void);
  /* Liefert den hoechsten Index aller Kanten + 1 (bzw. 0 bei 0 Kanten) */
extern int CreateTwoEdges ( long vorgaenger );
  /* Erzeugt zwei neue Kanten als Kinder von vorgaenger und legt den
     neuen Mittelknoten an */
extern int ReAssign1DElementEdges ( long ele, long *kanten, int anz );
  /* Traegt das 1D-Element elem bei anz Kanten kanten aus; eine Kante ohne
     Nachbarn wird automatisch entfernt */
extern long* GetEdge2DElems ( long number, int *anzahl );
  /* Liefert zugehoerige 2D-Elemente und deren Anzahl */
extern void Assign2DElementEdges ( long number, long ele );
  /* Traegt 2D-Element ele bei Kante number ein */
extern int ReAssign2DElementEdges ( long ele, long *kanten, int anz );
  /* Traegt das 2D-Element ele bei anz Kanten kanten aus; eine Kante ohne
     Nachbarn wird automatisch entfernt */

/* Weitere externe Objekte */
extern long EdgeListLength;
  /* ANZAHL der aktuell gespeicherten Kanten */

extern long *EDGGetNodeVerticalEdges(long, int *);

#endif
