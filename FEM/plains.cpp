/**************************************************************************/
/* ROCKFLOW - Modul: plains.c
                                                                          */
/* Aufgabe:
   Flaechenverzeichnis
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
   10/2000     C.Thorenz  Warnungen beseitigt
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
      
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "plains.h"
#include "ptrarr.h"
  /* Wegen ConstructPlainList */
#include "elements.h"
#include "rf_pcs.h" //OK_MOD"
  /* Wegen ConstructPlainList und ReAssignXDElementPlains */
#include "nodes.h"
  /* Weil CreateFourPlains im Prinzip verfeinert und dabei neue
     Knoten erzeugt */


/* Interne (statische) Deklarationen */


/* Definitionen */
long PlainListLength;
  /* Anzahl der gespeicherten Flaechen, siehe Header */
int fk[6][4] = { { 0, 1, 2, 3 },
                 { 7, 6, 5, 4 },
                 { 1, 0, 4, 5 },
                 { 2, 1, 5, 6 },
                 { 3, 2, 6, 7 },
                 { 0, 3, 7, 4 } };
  /* Indizes der Flaechenknoten eines 3D-Elements (6 Flaechen mit je 4 Knoten) */
static ZeigerFeld *plainlist = NULL;
  /* Flaechenverzeichnis */
static long lowest_free_plain;
  /* niedrigste freie Flaechennummer */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreatePlainList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Flaechenverzeichnis; die erste Flaeche hat spaeter den
   Index 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int CreatePlainList ( void )
{
  PlainListLength = 0l;
  lowest_free_plain = 0l;
  plainlist = CreatePtrArray(EDGE_START_SIZE,EDGE_INC_SIZE);
  return !(plainlist == NULL);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyPlainList
                                                                          */
/* Aufgabe:
   Entfernt komplettes Flaechenverzeichnis aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void DestroyPlainList ( void )
{
  long i;
  long l = PtrArrayLength(plainlist);
  for (i = 0l; i < l; i++) {
      if (plainlist->elements[i]!=NULL) {
          plainlist->elements[i] = Free((Flaeche *) plainlist->elements[i]);
      }
  }
  ResizePtrArray(plainlist,0l);
  PlainListLength = 0l;
  lowest_free_plain = 0l;
  DestroyPtrArray(plainlist);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructPlainList
                                                                          */
/* Aufgabe:
   Berechnet Flaechenverzeichnis komplett neu (es sollte gerade vorher
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
   02/1996     MSR        Erste Version
   02.04.1998  R.Kaiser   Kopplung beliebiger Elementdim. in 3D
                                                                          */
/**************************************************************************/
void ConstructPlainList ( void )
{
  long i, j, k, l, ii;  /* Laufvariablen */
  int typ;  /* Elementtyp - 1 */
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *flaechen = NULL;  /* Zeiger auf Elementflaechen */
  Flaeche *flaeche = NULL;  /* Zeiger auf Flaeche */
  /* Schleife ueber alle Ausgangselemente (3D) */
  for (i=0;i<start_new_elems;i++) {
      if (ElGetElementType(i)==3) {
          knoten = ElGetElementNodes(i);
          flaechen = (long *) Malloc(6*sizeof(long));
          for (j=0l;j<6l;j++) {  /* Schleife ueber alle Flaechen */
              k = 0l;
              l = PlainListLength;
              while (k<l) {  /* Flaeche suchen */
                  flaeche = GetPlain(k);
                  if (ComparePlains(flaeche->knoten,knoten,j))
                      l = -20l;  /* Flaeche gefunden, Abbruchkriterium */
                  else
                      k++;
              }
              if (l==PlainListLength) {  /* Flaeche existiert noch nicht */
                  k = AddPlain(flaeche=NewPlain());  /* Flaeche erzeugen */
                  /* Bei neuer Flaeche trage ich mich immer positiv drehend ein */
                  for (ii=0l;ii<4l;ii++)
                      flaeche->knoten[ii] = knoten[fk[j][ii]];
                  flaeche->nachbarn_3D[0] = i;
              }
              else {  /* Flaeche existiert bereits mit Index k */
                  flaeche->nachbarn_3D[1] = i;
              }
              /* Flaeche bei Element eintragen */
              flaechen[j] = k;
          }
          /* Flaechenverweise bei Element i eintragen */
          ElSetElementPlains(i,flaechen,-1);
      }
  }
  /* Schleife ueber alle Ausgangselemente (1D und 2D) */
  for (i=0;i<start_new_elems;i++) {
      if ((typ = ElGetElementType(i) - 1)<2) {
          knoten = ElGetElementNodes(i);
          if (typ) {  /* 2D */
              flaechen = (long *) Malloc(sizeof(long));
              k = 0l;
              l = PlainListLength;
              while (k<l) {  /* Flaeche suchen */
                  flaeche = GetPlain(k);
                  if (ComparePlains(flaeche->knoten,knoten,-1))
                      l = -20l;  /* Flaeche gefunden, Abbruchkriterium */
                  else
                      k++;
              }

                if (l==PlainListLength) {  /* Flaeche existiert noch nicht */
                  k = AddPlain(flaeche=NewPlain());  /* Flaeche erzeugen */
                  /* Bei neuer Flaeche trage ich mich immer positiv drehend ein */
                  for (ii=0l;ii<4l;ii++)
                    flaeche->knoten[ii] = knoten[ii];
                  flaeche->nachbar_2D = i;
                }

               /* Flaeche existiert mit Index k */
              flaeche->nachbar_2D = i;
              /* Flaeche bei Element eintragen */
              flaechen[0] = k;
              l = -1l;
          }
          else { /* 1D */
              l = 0l;
              flaechen = NULL;
              for (k=0l;k<PlainListLength;k++) {  /* Schleife ueber alle Flaechen */
                  flaeche = GetPlain(k);
                  if ((ii = ComparePlains1D(flaeche->knoten,knoten))>=0) {
                      /* Flaeche gefunden */
                      l++;
                      flaechen = (long *) Realloc(flaechen,l*sizeof(long));
                      flaechen[l-1] = k;
                      flaeche->nachbarn_1D[ii] = i;
                  }
              }

          }
          /* Flaechenverweise bei Element i eintragen */
          ElSetElementPlains(i,flaechen,(int) l);
      }
  }
  start_new_plains = PlainListLength;
    /* Alle Knoten und Elemente sollten jetzt im Flaechenverzeichnis
       vermerkt sein */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructPlainListFast
                                                                          */
/* Aufgabe:
   Berechnet Flaechenverzeichnis komplett neu (es sollte gerade vorher
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
   08/2000     RK         Erste Version
   10/2000     C.Thorenz  Warnungen beseitigt

                                                                          */
/**************************************************************************/
void ConstructPlainListFast ( void )
{
  int anzahl_elemente = 0;
  long output1=0, output2=0;
  int np=-1, typ; /* Anzahl Elementflächen */
  int j, ii, jj, kk; /* Laufvariablen */
  int plain_exist = 0;
  int mem = 0;
  long i, k=-1, l; /* Laufvariablen */
  long node1=-1, node=-1;
  long *nodeplains1, *nodeplains, *plains1;
  int anzahl_nodeplains1, anzahl_nodeplains;
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *flaechen = NULL;  /* Zeiger auf Elementflaechen */
  Flaeche *flaeche = NULL;  /* Zeiger auf Flaeche */

  if (GetRFControlGridAdapt() == 3) {
    DisplayMsgLn("");
    DisplayMsg("Aufbau Flaechenverzeichnis:");
    DisplayMsgLn("");
  }


  /* Schleife ueber alle Ausgangselemente (3D und 2D) */
  for (i=0;i<start_new_elems;i++) {
    typ = ElGetElementType(i);
    if (typ > 1) {
      anzahl_elemente++;
      knoten = ElGetElementNodes(i);
      if (typ == 2) np=1;
      if (typ == 3) np=6;
      flaechen = (long *) Malloc(np*sizeof(long));

      for (j=0;j<np;j++) {  /* Schleife ueber alle Elementflaechen */
        plain_exist = 0;

        if (typ == 2) node1 = knoten[0];
        if (typ == 3) node1 = knoten[fk[j][0]];

        nodeplains1 = GetNodePlains(node1,&anzahl_nodeplains1);

        plains1 = (long *) Malloc(anzahl_nodeplains1*sizeof(long));

        for (ii=0;ii<anzahl_nodeplains1;ii++) {
          plains1[ii] = nodeplains1[ii];
        }

        for (jj=1;jj<4;jj++) {
          if (typ == 2) node = knoten[jj];
          if (typ == 3) node = knoten[fk[j][jj]];

          nodeplains = GetNodePlains(node,&anzahl_nodeplains);

          if (anzahl_nodeplains1 != 0) {
            if (anzahl_nodeplains != 0) {
              for (ii=0;ii<anzahl_nodeplains1;ii++) {
                mem = 0;
                for (kk=0;kk<anzahl_nodeplains;kk++) {
                  if (plains1[ii] == nodeplains[kk]) {
                    mem = 1;
                  }
                }
                if (mem == 0) plains1[ii] = -1;
              }
            }
            else {
              for (ii=0;ii<anzahl_nodeplains1;ii++)
                plains1[ii] = -1;

              break;
            }
          }
        }

        if (anzahl_nodeplains1 != 0) {
          for (ii=0;ii<anzahl_nodeplains1;ii++) {
            if (plains1[ii] != -1) {
              k = plains1[ii];
              plain_exist = 1;
            }
          }
        }

        plains1 = (long *)Free(plains1);

        if (!plain_exist) {  /* Flaeche existiert noch nicht */
          k = AddPlain(flaeche=NewPlain());  /* Flaeche erzeugen */
          /* Bei neuer Flaeche trage ich mich immer positiv drehend ein */
          for (ii=0;ii<4;ii++) {
            if (typ == 2) flaeche->knoten[ii] = knoten[ii];
            if (typ == 3) flaeche->knoten[ii] = knoten[fk[j][ii]];
          }

          /* Flaeche bei zugehoerigen Knoten Eintragen */
          for (ii=0;ii<4;ii++)
            AssignPlain(flaeche->knoten[ii],k);

          if (typ == 2) flaeche->nachbar_2D = i;
          if (typ == 3) flaeche->nachbarn_3D[0] = i;
        }
        if (plain_exist) {  /* Flaeche existiert bereits mit Index k */
          if (typ == 2) flaeche->nachbar_2D = i;
          if (typ == 3) {
            if (flaeche->nachbarn_3D[0] == -1l) flaeche->nachbarn_3D[0] = i;
            else  flaeche->nachbarn_3D[1] = i;
          }
        }
        /* Flaeche bei Element eintragen */
        flaechen[j] = k;
      }
      /* Flaechenverweise bei Element i eintragen */
      ElSetElementPlains(i,flaechen,-1);

      /* Ausgabe */
      if (GetRFControlGridAdapt() == 3) {
        output1 = 100 * anzahl_elemente / (anz_2D + anz_3D);
        if ((output1 != output2) && (output1%10 == 0) ) {
          DisplayLong(output1); DisplayMsg(" % der Elemente bearbeitet");
          DisplayMsgLn("");
        }
        output2 = output1;
      }

    }
  }


  /* Schleife ueber alle Ausgangselemente (1D) */
  /* Der Algorithmus fuer 1D-Elemente muss noch geprueft werden !!!! */

  for (i=0;i<start_new_elems;i++) {
    if (ElGetElementType(i) == 1) {
      knoten = ElGetElementNodes(i);
      l = 0l;
      flaechen = NULL;

      node1 = knoten[0];
      nodeplains1 = GetNodePlains(node1,&anzahl_nodeplains1);

      plains1 = (long *) Malloc(anzahl_nodeplains1*sizeof(long));

      for (ii=0;ii<anzahl_nodeplains1;ii++) {
        plains1[ii] = nodeplains1[ii];
      }

      node = knoten[1];
      nodeplains = GetNodePlains(node,&anzahl_nodeplains);

      if (anzahl_nodeplains1 != 0) {
        if (anzahl_nodeplains != 0) {
          for (ii=0;ii<anzahl_nodeplains1;ii++) {
            mem = 0;
            for (kk=0;kk<anzahl_nodeplains;kk++) {
              if (plains1[ii] == nodeplains[kk]) {
                mem = 1;
              }
            }
            if (mem == 0) plains1[ii] = -1;
          }
        }
        else {
          for (ii=0;ii<anzahl_nodeplains1;ii++)
            plains1[ii] = -1;

          break;
        }
      }

      if (anzahl_nodeplains1 != 0) {
        for (ii=0;ii<anzahl_nodeplains1;ii++) {
          if (plains1[ii] != -1) {
            flaeche = GetPlain(plains1[ii]);

            if ((jj = ComparePlains1D(flaeche->knoten,knoten))>=0) {
              /* Flaeche gefunden */

              l++;
              flaechen = (long *) Realloc(flaechen,l*sizeof(long));
              flaechen[l-1] = plains1[ii];
              flaeche->nachbarn_1D[jj] = i;
            }
          }
        }
      }

      plains1 = (long *)Free(plains1);

      /* Flaechenverweise bei Element i eintragen */
      ElSetElementPlains(i,flaechen,(int) l);
    }
  }

  start_new_plains = PlainListLength;

  /* Alle Knoten und Elemente sollten jetzt im Flaechenverzeichnis
     vermerkt sein */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: NewPlain
                                                                          */
/* Aufgabe:
   Erzeugt neue initialisierte Flaechenstruktur
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   Zeiger auf neue Flaechenstruktur
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
Flaeche *NewPlain ( void )
{
  static Flaeche *k;
  k = (Flaeche *) Malloc(sizeof(Flaeche));
  k->nachbarn_1D[0] = k->nachbarn_1D[1] = k->nachbarn_1D[2] = k->nachbarn_1D[3] =
    k->nachbarn_3D[0] = k->nachbarn_3D[1] = k->nachbar_2D =
    k->knoten[0] = k->knoten[1] = k->kinder[0] = k->kinder[1] =
    k->knoten[2] = k->knoten[3] = k->kinder[2] = k->kinder[3] =
    k->vorgaenger = -1l;
  return k;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AddPlain
                                                                          */
/* Aufgabe:
   Haengt Flaeche an Flaechenverzeichnis an Position lowest_free_plain an;
   die erste Flaeche bekommt die Nummer 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Flaeche *node: Zeiger auf Flaechendatenstruktur, die angehaengt
                  werden soll.
                                                                          */
/* Ergebnis:
   Flaechennummer der neuen Flaeche
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
long AddPlain ( Flaeche *flaeche )
{
  static long pos;
  pos = lowest_free_plain;
  SetPtrArrayElement(plainlist, lowest_free_plain, (void *) flaeche);
  lowest_free_plain = GetLowestFree(plainlist, (lowest_free_plain+1));
  PlainListLength++;
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PlacePlain
                                                                          */
/* Aufgabe:
   Haengt Flaeche an Flaechenverzeichnis an Position pos an
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Flaeche *node: Zeiger auf Flaechendatenstruktur, die angehaengt
                  werden soll.
   E long pos: Flaechennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void PlacePlain ( Flaeche *flaeche, long pos )
{
#ifdef ERROR_CONTROL
    if (GetPlain(pos)!=NULL) {
        DisplayErrorMsg("PlacePlain: Flaeche existiert bereits !!!");
        abort();
    }
#endif
  SetPtrArrayElement(plainlist, pos, (void *) flaeche);
  lowest_free_plain = GetLowestFree(plainlist, lowest_free_plain);
  PlainListLength++;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetPlain
                                                                          */
/* Aufgabe:
   Liefert Flaechendaten der Flaeche number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Flaechennummer, deren Daten gelesen werden sollen
                                                                          */
/* Ergebnis:
   Zeiger auf gesuchte Flaechendatenstruktur, wenn nicht gefunden: NULL
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
Flaeche *GetPlain ( long number )
{
  /* return (Flaeche *) plainlist->elements[number]; */
  /* besser, aber wahrscheinlich langsamer: */
  return (Flaeche *) GetPtrArrayElement(plainlist, number);
  /* (hier wuerde auch die Feldgroesse ueberprueft werden!) */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DeletePlain
                                                                          */
/* Aufgabe:
   Loescht Flaeche aus Flaechenverzeichnis an Position number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer der Flaeche, die geloescht werden soll.
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int DeletePlain ( long number )
{
  static Flaeche *k;
  static Flaeche *kk;
  k = GetPlain(number);
#ifdef ERROR_CONTROL
  if (k!=NULL) {
      /* Kinder ueberpruefen */
      if ((k->kinder[0]>=0l) || (k->kinder[1]>=0l) || (k->kinder[2]>=0l) || (k->kinder[3]>=0l)) {
          DisplayErrorMsg("Fehler bei DeletePlain - Flaeche hat Kind(er) !!!");
          return 0;
      }
#endif
      /* Flaeche bei Vorgaenger austragen */
      if (k->vorgaenger>=0l) {
          kk = GetPlain(k->vorgaenger);
          /* hier waere noch eine Fehlerabfrage moeglich */
          if (number==kk->kinder[0])  /* number ist kind[0] */
              kk->kinder[0] = -1l;
          else if (number==kk->kinder[1])  /* number ist kind[1] */
              kk->kinder[1] = -1l;
          else if (number==kk->kinder[2])  /* number ist kind[2] */
              kk->kinder[2] = -1l;
          else if (number==kk->kinder[3])  /* number ist kind[3] */
              kk->kinder[3] = -1l;
      }
      /* Flaeche loeschen */
      plainlist->elements[number] = Free((Flaeche *) plainlist->elements[number]);
      PlainListLength--;
      if (lowest_free_plain > number)
          lowest_free_plain = number;
      return 1;
#ifdef ERROR_CONTROL
  }
  else {
      DisplayErrorMsg("Fehler bei DeletePlain - Flaeche exist. nicht !!!");
      return 0;
  }
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PlainListSize
                                                                          */
/* Aufgabe:
   Liefert den hoechsten Index aller Flaechen + 1 (Anzahl der Eintraege im
   Pointer-Feld)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
long PlainListSize (void)
{
  return PtrArrayLength(plainlist);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign1DElementPlains
                                                                          */
/* Aufgabe:
   Traegt das 1D-Element elem bei anz Flaechen flaechen aus; eine Flaeche
   ohne Nachbarn wird automatisch entfernt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *flaechen: Zeiger auf zugehoerige Flaechennummern
   E int anzahl: Anzahl der Flaechen
                                                                          */
/* Ergebnis:
   0 bei Fehler (Flaechen waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
   04/1996     MSR        Flaeche muss bei 1D-El. nicht existieren
                                                                          */
/**************************************************************************/
int ReAssign1DElementPlains ( long ele, long *flaechen, int anz )
{
  static int i;
  static Flaeche *k;
  for (i=0;i<anz;i++) {
      if (flaechen[i]>=0l) {  /* sonst gibt's die Flaeche gar nicht mehr */
          k = GetPlain(flaechen[i]);
#ifdef ERROR_CONTROL
            if (k==NULL) {
                DisplayErrorMsg("Fehler bei ReAssign1DElementPlains !!!");
                return 0;
            }
            /* hier koennte man auch noch eine Fehlerabfrage einbauen, ob das
               Element ueberhaupt eingetragen ist. */
#endif
          if (ele==k->nachbarn_1D[0]) {  /* ele ist nachbar[0] */
              k->nachbarn_1D[0] = -1l;
          }
          else if (ele==k->nachbarn_1D[1]) {  /* ele ist nachbar[1] */
              k->nachbarn_1D[1] = -1l;
          }
          else if (ele==k->nachbarn_1D[2]) {  /* ele ist nachbar[2] */
              k->nachbarn_1D[2] = -1l;
          }
#ifdef ERROR_CONTROL
            else if (ele==k->nachbarn_1D[3]) {  /* ele ist nachbar[3] */
#else
            else {  /* ele ist nachbar[3] */
#endif
                k->nachbarn_1D[3] = -1l;
            }
#ifdef ERROR_CONTROL
            else {
                DisplayErrorMsg("ReAssign1DElementPlains: Element nicht eingetragen !");
                abort();
            }
#endif
          /* evtl. Flaeche loeschen --> inzw. unnoetig
          if ((k->nachbar_2D==-1l) &&
              (k->nachbarn_3D[0]==-1l) && (k->nachbarn_3D[1]==-1l) &&
              (k->nachbarn_1D[0]==-1l) && (k->nachbarn_1D[1]==-1l) &&
              (k->nachbarn_1D[2]==-1l) && (k->nachbarn_1D[3]==-1l) ) {
                 Flaeche hat keine Nachbarn mehr -> loeschen
                 (Sie sollte auch keine Kinder mehr haben!)
              DeletePlain(flaechen[i]);
          } */
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign2DElementPlains
                                                                          */
/* Aufgabe:
   Traegt das 2D-Element elem bei anz Flaechen flaechen aus; eine Flaeche
   ohne Nachbarn wird automatisch entfernt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *flaechen: Zeiger auf zugehoerigen Flaechennummern
   E int anzahl: Anzahl der Flaechen
                                                                          */
/* Ergebnis:
   0 bei Fehler (Flaechen waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
   04/1996     MSR        Flaeche muss bei 1D-El. nicht existieren
                                                                          */
/**************************************************************************/
int ReAssign2DElementPlains ( long ele, long *flaechen, int anz )
{
  static long dummy;
  static int i, j, ii;
  static Flaeche *k;
  static long *p;

  dummy = ele;

  for (i=0;i<anz;i++) {
      k = GetPlain(flaechen[i]);
#ifdef ERROR_CONTROL
        if (k==NULL) {
            DisplayErrorMsg("Fehler bei ReAssign2DElementPlains !!!");
            return 0;
        }
        /* hier koennte man auch noch eine Fehlerabfrage einbauen, ob das
           Element ueberhaupt eingetragen ist. */
#endif
      k->nachbar_2D = -1l;
      /* evtl. Flaeche loeschen */
      if ((k->nachbarn_3D[0]==-1l) && (k->nachbarn_3D[1]==-1l)) {
          /* Flaeche auch bei 1D-Elementen austragen und anschliessend
             loeschen */
          for (j=0;j<4;j++) {  /* Schleife ueber die 1D-Nachbarn */
              if (k->nachbarn_1D[j]>=0l) {  /* 1D-Nachbar eingetragen */
                  p = ElGetElementPlains(k->nachbarn_1D[j]);
                  ii = 0;
                  while (
#ifdef ERROR_CONTROL
                          (ii<ElGetElementPlainsNumber(k->nachbarn_1D[j])) &&
#endif
                          (p[ii]!=flaechen[i]) )
                      ii++;
#ifdef ERROR_CONTROL
                      if (ii==ElGetElementPlainsNumber(k->nachbarn_1D[j])) {
                          DisplayErrorMsg("!!! Fehler in ReAssign2DElementPlains !");
                          abort();
                      }
#endif
                  p[ii] = -1l;
              }
          }
          /* Flaeche hat keine Nachbarn mehr -> loeschen
             (Sie sollte auch keine Kinder mehr haben!) */
          DeletePlain(flaechen[i]);
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign3DElementPlains
                                                                          */
/* Aufgabe:
   Traegt das 3D-Element elem bei anz Flaechen flaechen aus; eine Flaeche
   ohne Nachbarn wird automatisch entfernt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *flaechen: Zeiger auf zugehoerigen Flaechennummern
   E int anzahl: Anzahl der Flaechen
                                                                          */
/* Ergebnis:
   0 bei Fehler (Flaechen waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
   04/1996     MSR        Flaeche muss bei 1D-El. nicht existieren
                                                                          */
/**************************************************************************/
int ReAssign3DElementPlains ( long ele, long *flaechen, int anz )
{
  static int i, j, ii, jj;
  static Flaeche *k;
  static long *p;
  for (i=0;i<anz;i++) {
      k = GetPlain(flaechen[i]);
#ifdef ERROR_CONTROL
        if (k==NULL) {
            DisplayErrorMsg("Fehler bei ReAssign3DElementPlains !!!");
            return 0;
        }
        /* hier koennte man auch noch eine Fehlerabfrage einbauen, ob das
           Element ueberhaupt eingetragen ist. */
#endif
      if (ele==k->nachbarn_3D[0]) {  /* ele ist nachbar[0] */
          k->nachbarn_3D[0] = -1l;
          jj = 1;
      }
      else {  /* ele ist nachbar[1] */
          k->nachbarn_3D[1] = -1l;
          jj = 0;
      }
      /* evtl. Flaeche loeschen */
      if ((k->nachbarn_3D[jj]==-1l) && (k->nachbar_2D==-1l)) {
          /* Flaeche auch bei 1D-Elementen austragen und anschliessend
             loeschen */
          for (j=0;j<4;j++) {  /* Schleife ueber die 1D-Nachbarn */
              if (k->nachbarn_1D[j]>=0l) {  /* 1D-Nachbar eingetragen */
                  p = ElGetElementPlains(k->nachbarn_1D[j]);
                  ii = 0;
                  while (
#ifdef ERROR_CONTROL
                          (ii<ElGetElementPlainsNumber(k->nachbarn_1D[j])) &&
#endif
                          (p[ii]!=flaechen[i]) )
                      ii++;
#ifdef ERROR_CONTROL
                      if (ii==ElGetElementPlainsNumber(k->nachbarn_1D[j])) {
                          DisplayErrorMsg("!!! Fehler in ReAssign3DElementPlains !");
                          abort();
                      }
#endif
                  p[ii] = -1l;
              }
          }
          /* Flaeche hat keine Nachbarn mehr -> loeschen
             (Sie sollte auch keine Kinder mehr haben!) */
          DeletePlain(flaechen[i]);
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PDrehsinn
                                                                          */
/* Aufgabe:
   Bestimmt den Drehsinn der Flaechenknoten einer Flaeche zu denen einer
   Elementflaeche.
   Schnellere Version von ComparePlains3D; Existenz vorausgesetzt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long *fknoten: Zeiger auf Feld mit den 4 Flaechenknoten
   E long *eknoten: Zeiger auf Feld mit den 8 Elementknoten
   E int fn: Nummer der zu vergleichenden Flaeche
                                                                          */
/* Ergebnis:
   0, wenn Drehsinn identisch
   1, wenn Drehsinn entgegengesetzt
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
   02/2004	   TK		  Neue Platzierung von abort()
                                                                          */
/**************************************************************************/
int PDrehsinn ( long *fknoten, long *eknoten, int fn )
{
  static long ek[4];
  static int i;
  int ok=1;
  /* Knoten der Elementflaeche ermitteln */
  for (i=0;i<4;i++)
      ek[i] = eknoten[fk[fn][i]];
  /* Einen uebereinstimmenden Knoten suchen */
  i = 0;
  ok=i+1;
  while (
#ifdef ERROR_CONTROL
          (i<4) &&
#endif
          (ek[0]!=fknoten[i])
        )
      i++;
#ifdef ERROR_CONTROL
  if (i==4) {
      DisplayErrorMsg("!!! PDrehsinn: Fehler - Knoten nicht gefunden !!!");
      abort();
  }
#endif
  if (ek[1]==fknoten[(i+1)%4])
      return 0;  /* Drehsinn positiv */
#ifdef ERROR_CONTROL
  else if (ek[/*3*/1]==fknoten[(i+3)%4])
      return 1;  /* Drehsinn negativ */
  else {
      DisplayErrorMsg("!!! PDrehsinn: Fehler - 2. Knoten nicht gefunden !!!");
      ok=0;
  }
  if (ok==0) abort();
  else return 0;  /* damit keine warning kommt */ 
#else
  else
      return 1;  /* Drehsinn negativ */
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ComparePlains
                                                                          */
/* Aufgabe:
   Bestimmt die Existenz einer angegebenen Flaeche als Elementflaeche.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long *fknoten: Zeiger auf Feld mit den 4 Flaechenknoten
   E long *eknoten: Zeiger auf Feld mit den 8 bzw. 4 Elementknoten
   E int fn: Nummer der zu vergleichenden Flaeche (3D-Element);
             <0, wenn Flaechenknoten bei 2D-Element direkt angegeben
                                                                          */
/* Ergebnis:
   0, wenn Flaeche keine Elementflaeche
   1, wenn Flaeche Elementflaeche, Drehsinn positiv
   2, wenn Flaeche Elementflaeche, Drehsinn negativ
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ComparePlains ( long *fknoten, long *eknoten, int fn )
{
  static long ek[4];
  static int i;
  /* Knoten der Elementflaeche ermitteln */
  if (fn<0) {  /* 2D */
      for (i=0;i<4;i++)
          ek[i] = eknoten[i];
  }
  else {  /* 3D */
      for (i=0;i<4;i++)
          ek[i] = eknoten[fk[fn][i]];
  }
  /* Einen uebereinstimmenden Knoten suchen */
  i = 0;
  while ( (i<4) && (ek[0]!=fknoten[i]) )
      i++;
  if (i==4)
      return 0;
  if ( (ek[1]==fknoten[(i+1)%4]) && (ek[2]==fknoten[(i+2)%4]) &&
       (ek[3]==fknoten[(i+3)%4]) )
      return 1;  /* Drehsinn positiv */
  else if ( (ek[1]==fknoten[(i+3)%4]) && (ek[2]==fknoten[(i+2)%4]) &&
            (ek[3]==fknoten[(i+1)%4]) )
      return 2;  /* Drehsinn negativ */
  else
      return 0;  /* Flaechen nicht identisch */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ComparePlains1D
                                                                          */
/* Aufgabe:
   Bestimmt die Existenz einer angegebenen Flaeche als Nachbar eines
   1D-Elements.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long *fknoten: Zeiger auf Feld mit den 4 Flaechenknoten
   E long *eknoten: Zeiger auf Feld mit den 2 Elementknoten
                                                                          */
/* Ergebnis:
   0..4: Index, wenn Flaeche angrenzt
   -1: Flaeche grenzt nicht an
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ComparePlains1D ( long *fknoten, long *eknoten )
{
  static int i;
  /* Einen uebereinstimmenden Knoten suchen */
  i = 0;
  while ( (i<4) && (eknoten[0]!=fknoten[i]) )
      i++;
  if (i==4)
      return -1;
  if (eknoten[1]==fknoten[(i+1)%4])
      return i;
  else if (eknoten[1]==fknoten[(i+3)%4])
      return ((i+3)%4);
  else
      return -1;  /* Kante liegt nicht an */
}
