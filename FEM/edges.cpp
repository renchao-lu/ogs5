/**************************************************************************/
/* ROCKFLOW - Modul: edges.c
                                                                          */
/* Aufgabe:
   Kantenverzeichnis
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
   07/2002     RK         Einführung eines Kantenmittelknotens
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "adaptiv.h"
#include "gridadap.h"
#include "ptrarr.h"
#include "elements.h"
  /* Wegen ConstructEdgeList */
#include "nodes.h"
  /* Weil CreateTwoEdges im Prinzip verfeinert und dabei einen neuen
     Knoten erzeugt */


/* Interne (statische) Deklarationen */


/* Definitionen */
long EdgeListLength;
  /* Anzahl der gespeicherten Kanten, siehe Header */
static ZeigerFeld *edgelist = NULL;
  /* Kantenverzeichnis */
static long lowest_free_edge;
  /* niedrigste freie Kantennummer */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateEdgeList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Kantenverzeichnis; die erste Kante hat spaeter den
   Index 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
int CreateEdgeList ( void )
{
  EdgeListLength = 0l;
  lowest_free_edge = 0l;
  edgelist = CreatePtrArray(EDGE_START_SIZE,EDGE_INC_SIZE);
  return !(edgelist == NULL);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyEdgeList
                                                                          */
/* Aufgabe:
   Entfernt komplettes Kantenverzeichnis aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void DestroyEdgeList ( void )
{
  long i;
  long l = PtrArrayLength(edgelist);
  for (i = 0l; i < l; i++) {
      if (edgelist->elements[i]!=NULL) {
          ((Kante *) edgelist->elements[i])->nachbarn_2D =
                    (long *)Free(((Kante *) edgelist->elements[i])->nachbarn_2D);
          edgelist->elements[i] = Free((Kante *) edgelist->elements[i]);
      }
  }
  ResizePtrArray(edgelist,0l);
  EdgeListLength = 0l;
  lowest_free_edge = 0l;
  DestroyPtrArray(edgelist);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructEdgeList
                                                                          */
/* Aufgabe:
   Berechnet Kantenverzeichnis komplett neu (es sollte gerade vorher
   neu angelegt (initialisiert) worden sein. Es werden nur die
   Ausgangselemente eingetragen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)
   01.07.2002  RK         Kantenmittelknoten                       

   letzte Aenderung:      R.Kaiser    01.07.2002
                                                                          */
/**************************************************************************/
void ConstructEdgeList ( void )
{
  long i, j, k, l;  /* Laufvariablen */
  int typ;  /* Elementtyp - 1 */
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *kanten = NULL;  /* Zeiger auf Elementkanten */
  Kante *kante = NULL;  /* Zeiger auf Kante */
  /* Schleife ueber alle Ausgangselemente */
  for (i=0;i<start_new_elems;i++) {
      typ = ElGetElementType(i) - 1;
      knoten = ElGetElementNodes(i);
      kanten = (long *) Malloc(ElNumberOfEdges[typ]*sizeof(long));
      for (j=0l;j<ElNumberOfEdges[typ];j++) {  /* Schleife ueber alle Kanten */
          k = 0l;
          l = EdgeListLength;
          while (k<l) {  /* Kante suchen */
              kante = GetEdge(k);
              if (((kante->knoten[0]==knoten[j]) && (kante->knoten[1]==knoten[(j+1)%4])) ||
                  ((kante->knoten[1]==knoten[j]) && (kante->knoten[0]==knoten[(j+1)%4]))    )
                  l = -20l;  /* Kante gefunden, Abbruchkriterium */
              else
                  k++;
          }
          if (l==EdgeListLength) {  /* Kante existiert noch nicht */
              k = AddEdge(kante=NewEdge());  /* Kante erzeugen */
              kante->knoten[0] = knoten[j];
              kante->knoten[1] = knoten[(j+1)%4];
              kante->knoten[2] = -1;
              if (typ)   /* 2D-Element ( oder 3D ... ) */
                  Assign2DElementEdges(k,i);
              else /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          else {  /* Kante existiert bereits mit Index k */
              if (typ)    /* 2D-Element ( oder 3D ... ) */
                  Assign2DElementEdges(k,i);
              else  /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          /* Kante bei Element eintragen */
          kanten[j] = k;
      }
      /* Kantenverweise bei Element i eintragen */
      ElSetElementEdges(i,kanten);
  }
  start_new_edges = EdgeListLength;
    /* Alle Knoten und Elemente sollten jetzt im Kantenverzeichnis
       vermerkt sein */
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructEdgeListFast
                                                                          */
/* Aufgabe:
   Berechnet Kantenverzeichnis komplett neu (es sollte gerade vorher
   neu angelegt (initialisiert) worden sein. Es werden nur die
   Ausgangselemente eingetragen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   
   TODO !!!!!
                                                                          */
/**************************************************************************/
void ConstructEdgeListFast ( void )
{
  ConstructEdgeList();
}

/**************************************************************************/
/* ROCKFLOW - Funktion: NewEdge
                                                                          */
/* Aufgabe:
   Erzeugt neue initialisierte Kantenstruktur
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   Zeiger auf neue Kantenstruktur
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)
   01.07.2002  RK         Kantenmittelknoten                       
                          

   letzte Aenderung:      R.Kaiser      01.07.2002
                                                                          */
/**************************************************************************/
Kante *NewEdge ( void )
{
  static Kante *k;
  k = (Kante *) Malloc(sizeof(Kante));
  k->nachbar_1D = -1l;
  k->nachbarn_2D = NULL;
  k->anz_nachbarn_2D= 0;
  k->knoten[0] = k->knoten[1] = k->knoten[2] = k->kinder[0] = k->kinder[1] = -1l;
  k->vorgaenger = -1l;
  return k;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AddEdge
                                                                          */
/* Aufgabe:
   Haengt Kante an Kantenverzeichnis an Position lowest_free_edge an;
   die erste Kante bekommt die Nummer 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Kante *node: Zeiger auf Kantendatenstruktur, die angehaengt
                  werden soll.
                                                                          */
/* Ergebnis:
   Kantennummer der neuen Kante
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
long AddEdge ( Kante *kante )
{
  static long pos;
  pos = lowest_free_edge;
  SetPtrArrayElement(edgelist, lowest_free_edge, (void *) kante);
  lowest_free_edge = GetLowestFree(edgelist, (lowest_free_edge+1));
  EdgeListLength++;
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PlaceEdge
                                                                          */
/* Aufgabe:
   Haengt Kante an Kantenverzeichnis an Position pos an
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Kante *node: Zeiger auf Kantendatenstruktur, die angehaengt
                  werden soll.
   E long pos: Kantennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void PlaceEdge ( Kante *kante, long pos )
{
#ifdef ERROR_CONTROL
    if (GetEdge(pos)!=NULL) {
        DisplayErrorMsg("PlaceEdge: Kante existiert bereits !!!");
        abort();
    }
#endif
  SetPtrArrayElement(edgelist, pos, (void *) kante);
  lowest_free_edge = GetLowestFree(edgelist, lowest_free_edge);
  EdgeListLength++;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetEdge
                                                                          */
/* Aufgabe:
   Liefert Kantendaten der Kante number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Kantennummer, deren Daten gelesen werden sollen
                                                                          */
/* Ergebnis:
   Zeiger auf gesuchte Kantendatenstruktur, wenn nicht gefunden: NULL
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
Kante *GetEdge ( long number )
{
  /* return (Kante *) edgelist->elements[number]; */
  /* besser, aber wahrscheinlich langsamer: */
  return (Kante *) GetPtrArrayElement(edgelist, number);
  /* (hier wuerde auch die Feldgroesse ueberprueft werden!) */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DeleteEdge
                                                                          */
/* Aufgabe:
   Loescht Kante aus Kantenverzeichnis an Position number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer der Kante, die geloescht werden soll.
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   01.07.2002  RK         Kantenmittelknoten
                                                                          */
/**************************************************************************/
int DeleteEdge ( long number )
{
  static Kante *k;
  static Kante *kk;
  k = GetEdge(number);
#ifdef ERROR_CONTROL
  if (k!=NULL) {
      /* Kinder ueberpruefen */
      if ((k->kinder[0]>=0l) || (k->kinder[1]>=0l)) {
          DisplayErrorMsg("Fehler bei DeleteEdge - Kante hat Kind(er) !!!");
          return 0;
      }
#endif
      /* Kante bei Vorgaenger austragen */
      if (k->vorgaenger>=0l) {
          kk = GetEdge(k->vorgaenger);
          /* hier waere noch eine Fehlerabfrage moeglich */
          if (number==kk->kinder[0])  /* number ist kind[0] */
              kk->kinder[0] = -1l;
          else  /* number ist kind[1] */
              kk->kinder[1] = -1l;
          kk->knoten[2] = -1l;    
      }
      /* Kante loeschen */
      ((Kante *) edgelist->elements[number])->nachbarn_2D =
               (long *)Free(((Kante *) edgelist->elements[number])->nachbarn_2D);
      edgelist->elements[number] = Free((Kante *) edgelist->elements[number]);
      EdgeListLength--;
      if (lowest_free_edge > number)
          lowest_free_edge = number;
      return 1;
#ifdef ERROR_CONTROL
  }
  else {
      DisplayErrorMsg("Fehler bei DeleteEdge - Kante exist. nicht !!!");
      return 0;
  }
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: EdgeListSize
                                                                          */
/* Aufgabe:
   Liefert den hoechsten Index aller Kanten + 1 (Anzahl der Eintraege im
   Pointer-Feld)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
long EdgeListSize (void)
{
  return PtrArrayLength(edgelist);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateTwoEdges
                                                                          */
/* Aufgabe:
   Erzeugt zwei neue Kanten als Kinder von vorgaenger und legt den
   neuen Mittelknoten an. (Die Funktion ist also eigentlich eine
   Verfeinerungsfunktion)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long vorgaenger: Nummer der Kante, die verfeinert werden soll
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
   07/2002     RK         Kantenmittelknoten
                                                                          */
/**************************************************************************/
int CreateTwoEdges ( long vorgaenger )
{
  static Kante *k;
  static Knoten *knoten;
  static long knoten_nr;
  static Kante *k1;
  static Kante *k2;
  static long k1n, k2n;
  static Knoten *kno1;
  static Knoten *kno2;
  /* Vorgaengerkante holen */
  k = GetEdge(vorgaenger);
#ifdef ERROR_CONTROL
    /* Abbruch, wenn schon Kinder existieren */
    if ((k->kinder[0]>=0l) || (k->kinder[1]>=0l)) {
        DisplayErrorMsg("Fehler bei CreateTwoEdges !!!");
        return 0;
    }
#endif
  /* neue Kanten erzeugen und eintragen */
  k1n = AddEdge(k1=NewEdge());
  k2n = AddEdge(k2=NewEdge());
  /* neuen Knoten erzeugen und eintragen */
  knoten_nr = AddNode(knoten=NewNode());
  /* Kantendaten Vorgaenger eintragen */
  k->kinder[0] = k1n;
  k->kinder[1] = k2n;
  k->knoten[2] = knoten_nr; 
  /* Kantendaten Kinder eintragen */
  k1->vorgaenger = vorgaenger;
  k1->knoten[0] = k->knoten[0];
  k1->knoten[1] = knoten_nr;
  k2->vorgaenger = vorgaenger;
  k2->knoten[0] = knoten_nr;
  k2->knoten[1] = k->knoten[1];
  /* Eckknoten holen */
  kno1 = GetNode(k->knoten[0]);
  kno2 = GetNode(k->knoten[1]);
  /* Knotendaten eintragen */
  knoten->x = ( kno1->x + kno2->x ) * 0.5;
  knoten->y = ( kno1->y + kno2->y ) * 0.5;
  knoten->z = ( kno1->z + kno2->z ) * 0.5;
  /* modellspezifische Knotendaten verfeinern */
  RefineEdgeValues(knoten,kno1,kno2);
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign1DElementEdges
                                                                          */
/* Aufgabe:
   Traegt das 1D-Element elem bei anz Kanten kanten aus; eine Kante ohne
   Nachbarn wird automatisch entfernt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *kanten: Zeiger auf zugehoerigen Kantennummern
   E int anzahl: Anzahl der Kanten
                                                                          */
/* Ergebnis:
   0 bei Fehler (Kanten waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)

   letzte Aenderung:    R.Kaiser    31.10.1997
                                                                          */
/**************************************************************************/
int ReAssign1DElementEdges ( long ele, long *kanten, int anz )
{
  static long ele2; /* Dummy */
  static int i;
  static Kante *k;

  ele2 = ele; /* Dummy, keine Funktion !!! */

  for (i=0;i<anz;i++) {
      k = GetEdge(kanten[i]);
#ifdef ERROR_CONTROL
        if (k==NULL) {
            DisplayErrorMsg("Fehler bei ReAssign1DElementEdges !!!");
            return 0;
        }
        /* hier koennte man auch noch eine Fehlerabfrage einbauen, ob das
           Element ueberhaupt eingetragen ist. */
#endif
      k->nachbar_1D = -1l;
      /* evtl. Kante loeschen */
      if (k->nachbarn_2D == NULL) {
          /* Kante hat keine Nachbarn mehr -> loeschen
             (Sie sollte auch keine Kinder mehr haben!) */
          DeleteEdge(kanten[i]);
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetEdge2DElems
                                                                          */
/* Aufgabe:
   Liefert zugehoerige 2D-Elemente und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Kantennummer
   R int *anzahl: Anzahl der zugehoerigen Elemente
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Elemente
                                                                          */
/* Programmaenderungen:
   31.10.1997     R.Kaiser        Erste Version
                                                                          */
/**************************************************************************/
long* GetEdge2DElems ( long number, int *anzahl )
{
  *anzahl = ((Kante *) edgelist->elements[number])->anz_nachbarn_2D;
  return ((Kante *) edgelist->elements[number])->nachbarn_2D;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign2DElementEdges
                                                                          */
/* Aufgabe:
   Traegt 2D-Element ele bei Kante number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Kantennummer
   E long ele: einzutragende Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   31.10.1997     R.Kaiser        Erste Version
                                                                          */
/**************************************************************************/
void Assign2DElementEdges ( long number, long ele )
{
  static int i;
  i = ++((Kante *) edgelist->elements[number])->anz_nachbarn_2D;
  ((Kante *) edgelist->elements[number])->nachbarn_2D =              \
    (long *)Realloc(((Kante *) edgelist->elements[number])->nachbarn_2D,(i*sizeof(long)));
  (((Kante *) edgelist->elements[number])->nachbarn_2D)[i-1] = ele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign2DElementEdges
                                                                          */
/* Aufgabe:
   Traegt das 2D-Element elem bei anz Kanten kanten aus; eine Kante ohne
   Nachbarn wird automatisch entfernt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *kanten: Zeiger auf zugehoerigen Kantennummern
   E int anz: Anzahl der Kanten
                                                                          */
/* Ergebnis:
   0 bei Fehler (Kanten waren nicht eingetragen, ele nicht gefunden),
                 sonst 1
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)

   letzte Aenderung:      R.Kaiser     31.10.1997
                                                                          */
/**************************************************************************/
int ReAssign2DElementEdges ( long ele, long *kanten, int anz )
{
  static int anzele;
  static Kante *k;
  static int i, j, n;
  for (i=0;i<anz;i++) {
    k = GetEdge(kanten[i]);
#ifdef ERROR_CONTROL
      if (k==NULL) {
        DisplayErrorMsg("Fehler bei ReAssign2DElementEdges !!!");
        return 0;
      }
#endif
    anzele = ((Kante *) edgelist->elements[kanten[i]])->anz_nachbarn_2D;
    /* alle Eintraege ueberpruefen */
    j = 0;
    while (((((Kante *) edgelist->elements[kanten[i]])->nachbarn_2D)[j]!=ele) && (j<anzele))
      j++;
#ifdef ERROR_CONTROL
    if (j==anzele)  /* nicht gefunden */
      return 0;
#endif
    /* einer weniger */
    anzele = --((Kante *) edgelist->elements[kanten[i]])->anz_nachbarn_2D;
    /* Umspeichern */
    for (n=j;n<anzele;n++)
      (((Kante *) edgelist->elements[kanten[i]])->nachbarn_2D)[n] =     \
      (((Kante *) edgelist->elements[kanten[i]])->nachbarn_2D)[n+1];
    /* Speicher aendern */
    ((Kante *) edgelist->elements[kanten[i]])->nachbarn_2D =              \
    (long *)Realloc(((Kante *) edgelist->elements[kanten[i]])->nachbarn_2D,(anzele*sizeof(long)));

    if ((anzele == 0) && (k->nachbar_1D == -1l)) {
      /* evtl. Kante loeschen */
      /* Kante hat keine Nachbarn mehr -> loeschen
        (Sie sollte auch keine Kinder mehr haben!) */
      DeleteEdge(kanten[i]);
    }
  }
  return 1;
}

/**************************************************************************/
/* ROCKFLOW - Function: EDGGetNodeVerticalEdges
                                                                          */
/* Task:
   Searching for all vertical edges of a node. 
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
 
   Input:      node                                                       */
                               
/* Return:     *anz_kanten: number of edges
               long *  pointer on field with edges,  Spaeter freigeben!!! */
                                                                          
/* Programming:
   03/2003     MB         Implementation   (based on LAGGetNode2DEdges)
02/2005 MB QUAD, PRIS and HEX                                                                          */
/**************************************************************************/
long *EDGGetNodeVerticalEdges(long node, int *anz_kanten)
{
  /* Liefert alle Kanten an einem Knoten, aber nicht die irregulaeren Elternkanten */
  int anz_elem = 0;
  int i, j, k;
  int dim = 0;
  int ElementType;
  long *alle_kanten = NULL;
  long *kanten;
  long *elemente = NULL;
  Kante *kante;

  switch (max_dim) {
    case 0: 
      break;
    case 1:
      elemente = GetNode2DElems(node, &anz_elem);
      break;
    case 2:
      elemente = GetNode3DElems(node, &anz_elem);
      break;
  }

  *anz_kanten = 0;

  /* Alle Kanten holen, die an diesen Elementen haengen */
  alle_kanten = (long *) Malloc(4 * anz_elem * sizeof(long));
  alle_kanten[0] = -1;

  for (i = 0; i < anz_elem; i++)  {
    if (ElGetElementActiveState(elemente[i]))  {
      ElementType = ElGetElementType(i);
      switch (ElementType) {
        case 2:
          dim = 2;
          break;
        case 3:  //Hex
          dim = 4;  
          break;
        case 6:  //Pris
          dim = 3;
          break;
      }
      kanten = ElGetElementEdges(elemente[i]);
      for (j = 0; j < dim ; j++) {
        for (k = 0; ((k < *anz_kanten) && (alle_kanten[k] != kanten[j])); k++) {
        };
        if (k == *anz_kanten)  {
          kante = GetEdge(kanten[j]);
          if (((kante -> knoten[0] == node)|| (kante -> knoten[1] == node))
                                        && (kante -> kinder[0] == -1))  {
            /* Alle Kanten aussortieren, die nicht an diesem Knoten sind */
            /* Alle Kanten aussortieren, die Kinder haben */
            alle_kanten[*anz_kanten] = kanten[j];
            (*anz_kanten)++;
          } /* endif */
        }   /* endif */
      }     /* endfor */
    }       /* endif */
  }         /* endfor */
 return alle_kanten;

}

