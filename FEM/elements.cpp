/**************************************************************************
 ROCKFLOW - Modul: elements.c
                                                                          
 Aufgabe:
   Datenstruktur des Elementverzeichnisses und zugehoerige Funktionen
                                                                          
 Programmaenderungen:
   07/1996   MSR  Erste Version
   07/1997   RK   Korrekturen und Aenderungen aus dem aTM uebertragen
   08/1998   AH   Listentypen: Listen, Baeumen etc.
   08/1998   AH   Korrekturen: CreateElementPrototypes, ElDestroyElementList
   08/1998   RK   Funktionen fuer Adaption
   02/1999   AH   Einfuehrung von Gruppennummer (group_number)
   02/1999   CT   Korrekturen fuer herausgeloeste Materialgruppen
   03/1999   CT   anz_matxx, start_mat entfernt
   10/1999   AH   Anpassung (DestroyInternElementData)
   11/1999   RK   Kapselung Gitteradaption
   02/2000   CT   Warnung bei kleinen Elementen
   07/2000   OK   Erweiterungen fuer Dreieckselemente
   10/2000   CT   Erweiterungen fuer Mehrphasen
   11/2000   OK   Erweiterungen fuer Tetraeder und Prismen
   10/2001   AH   Inverses Modellieren
                  InitModelElementData
   11/2001   AH   Warnung entfernt
   01/2002   MK   Generate_Quadratic_Elements        
   09/2002   WW   Record stresses and effetive plastic strain at 
                  Gauss points      
   03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt           
   05/2003   OK   ELEGetElementMatrices, ELECreateElementMatricesPointer
                  ELECreateElementData
   07/2003   WW   Modified Generate2D_9N_Elements () to a general 2D high order 
                  grid generator
   08/2004   WW   1. New high order element generator (the old stuff is removed)
                  2. Element faces and edges  
**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"
#define CRITICAL_ELEMENT_VOLUME MKleinsteZahl

#ifndef  ENABLE_ADT

/* Objekte */
#include "elements.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "gridadap.h" /* anz_adapt */

#ifdef PCS_OBJECTS
#include "rf_pcs.h"
#endif

/* Tools */
#include "mathlib.h" /* MKleinsteZahl */
#include "ptrarr.h"
#include "tools.h"   /* ConstructElemsToNodesList */
#include "femlib.h"

/* Definitionen */
long ElementListLength;
  /* ANZAHL der aktuell gespeicherten Elemente, siehe Header */

int ElNumberOfNodes[6] = { 2, 4, 8, 3, 4, 6 }; /* OK rf3424 */
  /* Anzahl der Knoten eines Elements, siehe Header */
int ElNumberOfEdges[6] = { 1, 4, 0, 3, 6, 9 };
  /* Anzahl der Kanten eines Elements, siehe Header */
int ElNumberOfPlains[6] = { 0, 1, 6, 1, 4, 6 };
  /* Anzahl der Flaechen eines Elements (1D bei 3D-Netz: n), siehe Header */
int ElNumberOfChilds[6] = { 2, 4, 8, 0, 0, 0 };
  /* Anzahl der Kinder eines Elements, siehe Header */

static ZeigerFeld *elementlist;
  /* Elementverzeichnis */
static long lowest_free_element;
  /* niedrigste freie Elementnummer */

 /* Zaehler fuer Elementgruppen */
long start_new_elems;
long anz_1D = 0l;
long anz_2D = 0l;
long anz_3D = 0l;

long *ActiveElements = NULL; /* Feld in dem die Nummern der aktiven Elemente stehen */
long anz_active_elements = 0l;
long anz_active_1D = 0l;
long anz_active_2D = 0l;
long anz_active_3D = 0l;

long start_new_edges;
long start_new_plains;
int max_dim = 0;

long HighOrder = 0;

int *EleNeighborsOfNode = NULL;

VoidXFuncVoid InitInternElementData=NULL;
VoidXFuncVoidX DestroyInternElementData=NULL;
VoidXFuncVoidX ELEDestroyElementMatrices=NULL;
VoidFuncLDXD SetElementJacobiMatrix;
DoubleXFuncLDX GetElementJacobiMatrix;

#ifdef PCS_OBJECTS
extern VoidXFuncVoidX PCSDestroyELEMatrices[PCS_NUMBER_MAX];
#endif

/**************************************************************************/
/* ROCKFLOW - Funktion: ElCreateElementList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Elementverzeichnis; das erste Element hat spaeter
   die Nummer 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElCreateElementList (void)
{
 if(!elementlist) {
  ElementListLength = 0l;
  lowest_free_element = 0l;
  elementlist = CreatePtrArray(ELEM_START_SIZE,ELEM_INC_SIZE);
 }
  return !(elementlist == NULL);
}


/**************************************************************************
 ROCKFLOW - Funktion: ElDestroyElementList
                                                                          
 Aufgabe:
   Entfernt komplettes Elementverzeichnis aus dem Speicher.
   Dies ist die einzige Moeglichkeit, Materialdaten regulaer freizugeben,
   da auf einen Materialdatensatz mehrere Zeiger zeigen koennen.
                                                                          
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          
 Ergebnis:
   - void -
                                                                          
 Programmaenderungen:
   03/1994     MSR        Erste Version
   08/1998     AH         Anpassung an die Aenderungen in Materialdaten
   9/2000      CT         Mehrphasen fuer Geschwindigkeiten
   11/2002     OK         execute only if elementlist exists
   06/2004     WW         Delete neighbors if they exist

**************************************************************************/
void ElDestroyElementList (void)
{
  if(elementlist) {
  long i, phases = 1;
  int j;
  long l = PtrArrayLength(elementlist);
  int no_processes =(int)pcs_vector.size();

  if (GetRFProcessNumPhases()>phases) phases = GetRFProcessNumPhases();

  for (i = 0l; i < l; i++) {
    if (elementlist->elements[i]!=NULL) {
      ((Element *)elementlist->elements[i])->elementknoten = (long *)Free(((Element *)elementlist->elements[i])->elementknoten);
      ((Element *)elementlist->elements[i])->ele_edge_nodes2d = (long *)Free(((Element *)elementlist->elements[i])->ele_edge_nodes2d);       
      ((Element *)elementlist->elements[i])->kanten = (long *)Free(((Element *)elementlist->elements[i])->kanten);
      if(!((Element *)elementlist->elements[i])->Neighbors)
      {
          delete ((Element *)elementlist->elements[i])->Neighbors;
          ((Element *)elementlist->elements[i])->Neighbors = NULL;
      }

      ((Element *)elementlist->elements[i])->kinder = (long *)Free(((Element *)elementlist->elements[i])->kinder);
      if(DestroyInternElementData)
        ((Element *)elementlist->elements[i])->eval_intern = DestroyInternElementData(((Element *)elementlist->elements[i])->eval_intern);
      ((Element *)elementlist->elements[i])->eval = (double *)Free(((Element *)elementlist->elements[i])->eval);
      ((Element *)elementlist->elements[i])->global_verf = (int *)Free(((Element *)elementlist->elements[i])->global_verf);

	  if(((Element *)elementlist->elements[i])->gauss_velo !=NULL) //SB:3912
      for (j = 0; j < phases; j++)
        ((Element *)elementlist->elements[i])->gauss_velo[j] = (double *)Free(((Element *)elementlist->elements[i])->gauss_velo[j]);
      ((Element *)elementlist->elements[i])->gauss_velo = (double **)Free(((Element *)elementlist->elements[i])->gauss_velo);

       for(j=0;j<no_processes;j++) {
          ((Element *)elementlist->elements[i])->element_matrices[j] = \
              PCSDestroyELEMatrices[j](((Element *)elementlist->elements[i])->element_matrices[j]);
       }

       if(((Element *)elementlist->elements[i])->invjac)
         ((Element *)elementlist->elements[i])->invjac = \
           (double *)Free(((Element *)elementlist->elements[i])->invjac);
       //WW
       if(((Element *)elementlist->elements[i])->element_midpoint)
         ((Element *)elementlist->elements[i])->element_midpoint = \
           (double *)Free(((Element *)elementlist->elements[i])->element_midpoint);
       if(((Element *)elementlist->elements[i])->hetfields)
         ((Element *)elementlist->elements[i])->hetfields = \
           (double *)Free(((Element *)elementlist->elements[i])->hetfields);

        elementlist->elements[i] = Free((Element *) elementlist->elements[i]);
      }
  }
  ResizePtrArray(elementlist,0l);                 /* evtl. ueberfluessig */
  ElementListLength = 0l;
  lowest_free_element = 0l;
  DestroyPtrArray(elementlist);
  elementlist = NULL; //OK

  /* OK rf3424 */
  anz_1D = 0l;
  anz_2D = 0l;
  anz_3D = 0l;

 } /* if(elementlist) */

}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElShortenElementList
                                                                          */
/* Aufgabe:
   Entfernt alle Elemente ab number (einschl.) aus dem Elementverzeichnis,
   bzw. laesst nur number Elemente uebrig. Materialdaten werden nicht
   entfernt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer, ab der geloescht werden soll.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
    9/2000     CT         Mehrphasen fuer Geschwindigkeiten

                                                                          */
/**************************************************************************/
void ElShortenElementList ( long number )
{
  long i, phases = 1;
  int j;
  long l = PtrArrayLength(elementlist);

  if (GetRFProcessNumPhases()>phases) phases = GetRFProcessNumPhases();

  for (i = number; i < l; i++) {
      if (elementlist->elements[i]!=NULL) {
          ((Element *)elementlist->elements[i])->elementknoten = (long *)Free(((Element *)elementlist->elements[i])->elementknoten);
          ((Element *)elementlist->elements[i])->ele_edge_nodes2d = (long *)Free(((Element *)elementlist->elements[i])->ele_edge_nodes2d);         
          ((Element *)elementlist->elements[i])->kanten = (long *)Free(((Element *)elementlist->elements[i])->kanten);
          ((Element *)elementlist->elements[i])->kinder = (long *)Free(((Element *)elementlist->elements[i])->kinder);
          ((Element *)elementlist->elements[i])->eval_intern = DestroyInternElementData(((Element *)elementlist->elements[i])->eval_intern);
          ((Element *)elementlist->elements[i])->eval = (double *)Free(((Element *)elementlist->elements[i])->eval);
          ((Element *)elementlist->elements[i])->global_verf = (int *)Free(((Element *)elementlist->elements[i])->global_verf);

          for (j = 0; j < phases; j++)
              ((Element *)elementlist->elements[i])->gauss_velo[j] = (double *)Free(((Element *)elementlist->elements[i])->gauss_velo[j]);
          ((Element *)elementlist->elements[i])->gauss_velo = (double **)Free(((Element *)elementlist->elements[i])->gauss_velo);


          elementlist->elements[i] = Free((Element *) elementlist->elements[i]);
          ElementListLength--;
      }
  }
  ShortenPtrArray(elementlist, number);
  lowest_free_element = GetLowestFree(elementlist,0);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElCompressElementList
                                                                          */
/* Aufgabe:
   Komprimiert Elementverzeichnis (entfernt Nullzeiger)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElCompressElementList ( void )
{
  ElementListLength = CompressPtrArray(elementlist);
  lowest_free_element = ElementListLength;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElListSize
                                                                          */
/* Aufgabe:
   Liefert die hoechste Nummer aller Elemente + 1 (Anzahl der Eintraege im
   Pointer-Feld)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   11/2002   OK   test if elementlist exists
                                                                          */
/**************************************************************************/
long ElListSize (void)
{
 if(elementlist)
  return PtrArrayLength(elementlist);
 else
  return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElCreateElement
                                                                          */
/* Aufgabe:
   Erzeugt leeres aktives Element der Dimension elementtyp und haengt es an
   das Elementverzeichnis an der Position lowest_free_element an. Wenn der
   Vorgaenger >=0 ist, wird die Materialgruppe uebernommen. Das erste
   Element bekommt die Nummer 0. Alle sonstigen Zeiger und Werte werden
   initialisiert.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int elementtyp:  Dimension des zu erzeugenden Elements
   E long vorgaenger: Vorgaengerelement des zu erzeugenden Elements
   E int level: Verfeinerungslevel des zu erzeugenden Elements;
                wird -1 angegeben, wird der Level des Vorgaengers
                geerbt und um 1 erhoeht.
                                                                          */
/* Ergebnis:
   Elementnummer des neuen Elements
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   28.02.1999  C.Thorenz  Verschmelzen mit ElPlaceNewElement
                                                                          */
/**************************************************************************/
long ElCreateElement ( int elementtyp, long vorgaenger, int level )
{
  static long pos;
  pos = GetLowestFree(elementlist, lowest_free_element);
  CreateElementTopology( elementtyp, vorgaenger, level , pos);
  return pos;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: CreateElementTopology
                                                                          */
/* Aufgabe:
   Erzeugt leeres aktives Element der Dimension elementtyp und traegt es
   an Position pos im Elementverzeichnis ein. Der Materialzeiger wird nicht
   uebernommen. Das erste Element bekommt die Nummer 0. Alle sonstigen
   Zeiger und Werte werden initialisiert.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int elementtyp:  Dimension des zu erzeugenden Elements
   E long vorgaenger: Vorgaengerelement des zu erzeugenden Elements
   E int level: Verfeinerungslevel des zu erzeugenden Elements
   E long pos: Elementnummer des zu erzeugenden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
   11/1998     AH         element_start_number
   02/1999     AH         group_number
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   28.02.1999  C.Thorenz  Verschmelzen mit ElCreateElement
    2.03.1999  C.Thorenz  Korrektur
   05.11.1999  R.Kaiser   Kapselung Gitteradaption
    9/2000     CT         Speicher fuer Geschwindigkeiten nur einmal holen -> schneller
                          Umbenannt in  CreateElementTopology (war redundant)
   04/2002     OK         Tetraeder
                                                                          */
/**************************************************************************/
void CreateElementTopology ( int elementtyp, long vorgaenger, int level, long pos )
{
  static int i, phases=1, anz, NumGauss;

  static Element *elem;

  elem = (Element *) Malloc(sizeof(Element));

  if (vorgaenger >= 0) {   /* CT */
      elem->group_number = ElGetElementGroupNumber(vorgaenger);
      if (level==-1)
          elem->level = ((Element *)elementlist->elements[vorgaenger])->level + 1;
      else
          elem->level = /* 0 msr 0596 */ level;
  }  else  {
      elem->group_number = -1;
      if (level==-1)
          elem->level = 0;
      else
          elem->level = /* 0 msr 0596 */ level;
  }

  elem->element_start_number = -1;  /* ah */
  elem->elementtyp = elementtyp;
  elem->aktiv = 1;
  elem->count_verf = 0;
  elem->count_vergr = 0;
  elem->verfeinern = 0;
  elem->elementknoten = NULL;
  elem->ele_edge_nodes2d = NULL;
  elem->anz_ele_edge_nodes2d = 4;
  elem->vorgaenger = vorgaenger;
  elem->kanten = NULL;
  elem->anz_flaechen = ElNumberOfPlains[elementtyp-1];
  elem->kinder = NULL;
  if(InitInternElementData)
    elem->eval_intern = InitInternElementData();
  else //OK
    elem->eval_intern = NULL;
  elem->eval = (double *)Malloc(anz_eval*sizeof(double));
  for (i=0;i<anz_eval;i++)
      elem->eval[i] = eval_data[i].vorgabe;

  elem->global_verf = (int *)Malloc(AdaptGetTotalNumInd()*sizeof(int));
  for (i=0;i<AdaptGetTotalNumInd();i++)
      elem->global_verf[i] = 0;

  /* Speicher fuer Geschw. in Gausspunkten */
  anz = -1;

  NumGauss = GetNumericsGaussPoints(elementtyp);
  switch (elementtyp) {
     case 1:
       anz = 3;
       break;
     case 2:
       anz = 2 * NumGauss * NumGauss;
       break;
     case 3:
       anz = 3 * NumGauss * NumGauss * NumGauss;
       break;
     case 4:
       anz = 3;
       NumGauss = 3;
       break;
     case 5: /* Tetraeder */
       anz = 3;
       break;
     case 6: /* Prism MB */
       //anz = 3;
       anz = 3 * 2 * 3; //3D * 2(Linear) / 3(Triangle) 
       NumGauss = 18;
       break;
     default:
    DisplayMsgLn("Unsupported element-type in CreateElementTopology");
    exit(1);
  }

  if (GetRFProcessNumPhases() > phases) phases = GetRFProcessNumPhases();

  elem->gauss_velo = (double **) Malloc(phases * sizeof(double *));
  for (i=0;i<phases;i++)
      elem->gauss_velo[i] = (double *) Malloc(anz * sizeof(double));


  elem->art_diff = 0.0;
  elem->verf = 0;
  elem->dirty_bit = 65535;
  elem->volume = 0.0;


  elem->invjac = NULL;

//OKnew
  for(i=0;i<PCS_NUMBER_MAX;i++)
    elem->element_matrices[i] = NULL;

  elem->Neighbors = NULL; //SB/MB
  elem->element_midpoint = NULL; //SB
  elem->hetfields = NULL; //SB
  
  /*----------------------------------------------------------------------------*/
#ifdef ERROR_CONTROL
    if (ElGetElement(pos)!=NULL) {
        DisplayErrorMsg("ElPlaceNewElement: Element existiert bereits !!!");
        abort();
    }
#endif
  SetPtrArrayElement(elementlist, pos, (void *) elem);
  lowest_free_element = GetLowestFree(elementlist, lowest_free_element);
  ElementListLength++;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElDeleteElement
                                                                          */
/* Aufgabe:
   Loescht Element aus Elementverzeichnis an Position number.
   Materialdaten werden nicht geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Elements, das geloescht werden soll.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994   MSR  Erste Version (basierend auf atm)
   10/1999   AH   Anpassung von DestroyInternElementData
   07/2000   OK   Weitere Abfrage vor Freigabe der Element-Matrizen
    9/2000   CT   Mehrphasen fuer Geschwindigkeiten
                                                                          */
/**************************************************************************/
void ElDeleteElement ( long number )
{
  int j, phases=1;

  if (GetRFProcessNumPhases()>phases) phases=GetRFProcessNumPhases();

  if (elementlist->elements[number]!=NULL) {
      /* Element-Topologie */
      ((Element *)elementlist->elements[number])->elementknoten = (long *)Free(((Element *)elementlist->elements[number])->elementknoten);
      ((Element *)elementlist->elements[number])->ele_edge_nodes2d = (long *)Free(((Element *)elementlist->elements[number])->ele_edge_nodes2d); 
      ((Element *)elementlist->elements[number])->kanten = (long *)Free(((Element *)elementlist->elements[number])->kanten);
      ((Element *)elementlist->elements[number])->kinder = (long *)Free(((Element *)elementlist->elements[number])->kinder);
      ((Element *)elementlist->elements[number])->global_verf = (int *)Free(((Element *)elementlist->elements[number])->global_verf);
      /* Element-Matrizen */
      /* if (DestroyInternElementData) */
      if ( (DestroyInternElementData) && (((Element *)elementlist->elements[number])->eval_intern) )
        ((Element *)elementlist->elements[number])->eval_intern = DestroyInternElementData(((Element *)elementlist->elements[number])->eval_intern);
      /* Element-Daten */
      ((Element *)elementlist->elements[number])->eval = (double *)Free(((Element *)elementlist->elements[number])->eval);

	  if(((Element *)elementlist->elements[number])->gauss_velo){ //SB3910TK2
	  for (j = 0; j < phases; j++){
		  if(((Element *)elementlist->elements[number])->gauss_velo)  //SB3909
         ((Element *)elementlist->elements[number])->gauss_velo[j] = (double *)Free(((Element *)elementlist->elements[number])->gauss_velo[j]);
	  }
      ((Element *)elementlist->elements[number])->gauss_velo = (double **)Free(((Element *)elementlist->elements[number])->gauss_velo);
	  }

      elementlist->elements[number] = Free((Element *) elementlist->elements[number]);
      ElementListLength--;
      if (lowest_free_element > number)
          lowest_free_element = number;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElement
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf das ganze Element, sollte moeglichst nicht oder nur
   zu Testzwecken (Existenz) benutzt werden.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
Element *ElGetElement ( long number )
{
  return (Element *) GetPtrArrayElement(elementlist,number);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementStartNumber
                                                                          */
/* Aufgabe:
   Setzt die Startnummer des Elements number.
   Diese Funktion sollte nur einmal aufgerufen werden (eben gleich nach
   dem Einlesen des Startnetzes)
   Diese Nummer sollte beim Kreieren des Objektes (Element) als Parameter
   uebergeben werden.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer (Position im Feld)
   E long start_number: Startnummer im Ausgangsnetz
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementStartNumber ( long number, long element_start_number )
{
  ((Element *)elementlist->elements[number])->element_start_number = element_start_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementGroupNumber
                                                                          */
/* Aufgabe:
   Setzt die Gruppennummer des Elements number.
   Diese Funktion sollte nur einmal aufgerufen werden (eben gleich nach
   dem Einlesen des Startnetzes).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer (Position im Feld)
   E long group_number: Gruppennummer im Ausgangsnetz
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementGroupNumber ( long number, long group_number )
{
  ((Element *)elementlist->elements[number])->group_number = group_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementActiveState
                                                                          */
/* Aufgabe:
   Setzt den Aktiv-Status des Elements number
   (1: aktiv; 0: nicht aktiv)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int aktiv: zu setzender Status
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementActiveState ( long number, int aktiv )
{
  ((Element *)elementlist->elements[number])->aktiv = aktiv;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementDirtyBit
                                                                          */
/* Aufgabe:
   Setzt den Dirty-Status des Elements number
   Wird benutzt fuer den bedingten Wiederaufbau von Matritzen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int bit: zu setzender Status
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementDirtyBit ( long number, int bit )
{
  ((Element *)elementlist->elements[number])->dirty_bit = bit;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementRefineState
                                                                          */
/* Aufgabe:
   Setzt den Verfeinerungs-Status des Elements number
   (0: nichts veraendern, 1: verfeinern, -1: vergroebern)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int vergroebern: zu setzender Status
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementRefineState ( long number, int verfeinern )
{
  ((Element *)elementlist->elements[number])->verfeinern = verfeinern;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementNodes
                                                                          */
/* Aufgabe:
   Setzt die Eckknoten des Elements number. Ein evtl. bestehendes Feld
   wird geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *knoten: Feld mit den Eckknoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Anzahl ausgebaut
                                                                          */
/**************************************************************************/
void ElSetElementNodes ( long number, long *knoten )
{
  ((Element *)elementlist->elements[number])->elementknoten =
    (long *)Free(((Element *)elementlist->elements[number])->elementknoten);
  ((Element *)elementlist->elements[number])->elementknoten = knoten;
}


/**************************************************************************
 ROCKFLOW - Funktion: ElSetElementNeignbors
                                                                          
 Aufgabe:
   Set neighbors of an element
                                                                          
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *Neighbor: Element indeces of this element
                                                                          
 Ergebnis:
   - void -
                                                                          
 Programmaenderungen:
   07/2004     WW        Erste Version
                                                                          
**************************************************************************/
void ElSetElementNeighbors ( long number, long *Neighbor )
{
  ((Element *)elementlist->elements[number])->Neighbors =
    (long *)Free(((Element *)elementlist->elements[number])->Neighbors);
  ((Element *)elementlist->elements[number])->Neighbors = Neighbor;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementEdges
                                                                          */
/* Aufgabe:
   Setzt die Kanten des Elements number. Ein evtl. bestehendes Feld
   wird geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *kanten: Feld mit den Kanten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Anzahl ausgebaut, Nachbarn -> Kanten
                                                                          */
/**************************************************************************/
void ElSetElementEdges ( long number, long *kanten )
{
  ((Element *)elementlist->elements[number])->kanten =
    (long *)Free(((Element *)elementlist->elements[number])->kanten);
  ((Element *)elementlist->elements[number])->kanten = kanten;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementPlains
                                                                          */
/* Aufgabe:
   Setzt die Flaechen des Elements number. Ein evtl. bestehendes Feld
   wird geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *flaechen: Feld mit den Flaechen
   E int anz: Anzahl der Flaechen; anz<0: keine Aenderung der
              Standardeinstellung
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementPlains ( long number, long *flaechen, int anz )
{
  ((Element *)elementlist->elements[number])->kanten =
    (long *)Free(((Element *)elementlist->elements[number])->kanten);
  ((Element *)elementlist->elements[number])->kanten = flaechen;
  if (anz>=0)
      ((Element *)elementlist->elements[number])->anz_flaechen = anz;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementChilds
                                                                          */
/* Aufgabe:
   Setzt die Kinder des Elements number. Ein evtl. bestehendes Feld
   wird geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *kinder: Feld mit den Kindern
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Anzahl ausgebaut
                                                                          */
/**************************************************************************/
void ElSetElementChilds ( long number, long *kinder )
{
  ((Element *)elementlist->elements[number])->kinder =
    (long *)Free(((Element *)elementlist->elements[number])->kinder);
  ((Element *)elementlist->elements[number])->kinder = kinder;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementVal
                                                                          */
/* Aufgabe:
   Setzt einen Elementwert des Elements number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int idx: Index des Elementwertes
   E double val: Wert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementVal ( long number, int idx, double val )
{
  ((Element *)elementlist->elements[number])->eval[idx] = val;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementData
                                                                          */
/* Aufgabe:
   Setzt die Internen Elementdaten des Elements number.
   Wenn schon ein Datensatz existierte, wird er zuvor geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X void *data: Zeiger auf die Daten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementData ( long number, void *data )
{
  ((Element *)elementlist->elements[number])->eval_intern =
    DestroyInternElementData(((Element *)elementlist->elements[number])->eval_intern);
  ((Element *)elementlist->elements[number])->eval_intern = data;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetVelocity
                                                                          */
/* Aufgabe:
   Setzt die Geschwindigkeiten in den Gausspunkten des Elements number fuer
   eine Phase.

   */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X double *gauss_velo: Zeiger auf die Geschwindigkeiten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1994     MSR        Erste Version
   06/1999     OK         Gauss-Punkte setzen raus
   09/2000     CT         Mehrphasen, Speicherallokierung weg -> schneller

                                                                          */
/**************************************************************************/
void ElSetVelocityNew ( long number, int phase, double *gauss_velo )
{
  ((Element *)elementlist->elements[number])->gauss_velo[phase] = gauss_velo;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementStartNumber
                                                                          */
/* Aufgabe:
   Liefert die Nummer im Startnetz des Elements number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long ElGetElementStartNumber ( long number )
{
  return ((Element *)elementlist->elements[number])->element_start_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementGroupNumber
                                                                          */
/* Aufgabe:
   Liefert die Gruppennummer des Elements number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer (Position im Feld)
                                                                          */
/* Ergebnis:
   - Gruppennummer -
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long ElGetElementGroupNumber ( long number )
{
  return ((Element *)elementlist->elements[number])->group_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementType
                                                                          */
/* Aufgabe:
   Liefert den Typ (1D, 2D oder 3D) des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElGetElementType ( long number )
{
  return ((Element *)elementlist->elements[number])->elementtyp;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementActiveState
                                                                          */
/* Aufgabe:
   Liefert den Aktiv-Status des Elements number
   (0: nicht aktiv, 1: aktiv)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElGetElementActiveState ( long number )
{
  return ((Element *)elementlist->elements[number])->aktiv;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementRefineState
                                                                          */
/* Aufgabe:
   Liefert den Verfeinerungs-Status des Elements number
   (0: nichts veraendern, 1: verfeinern, -1: vergroebern)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElGetElementRefineState ( long number )
{
  return ((Element *)elementlist->elements[number])->verfeinern;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementLevel
                                                                          */
/* Aufgabe:
   Liefert den Verfeinerungs-Level des Elements number
   (0: noch nicht verfeinert; sonst Anzahl der bisherigen Verfeinerungen)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElGetElementLevel ( long number )
{
  return ((Element *)elementlist->elements[number])->level;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementNodes
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Eckknoten des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long *ElGetElementNodes ( long number )
{
  return ((Element *)elementlist->elements[number])->elementknoten;
}

/**************************************************************************
 ROCKFLOW - Funktion: ElGetElementNeighbors
                                                                          
 Aufgabe:
   Get indeces of neighbo elements of this element
                                                                          
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          
 Ergebnis:
   s.o.
                                                                          
 Programmaenderungen:
   07/2004     WW        Erste Version
                                                                          
*************************************************************************/
long *ElGetElementNeighbors ( long number )
{
  return ((Element *)elementlist->elements[number])->Neighbors;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetEdgeNodes2D
                                                                          */
/* Aufgabe:
   Liefert für jedes Element die Kantenknoten (hängende Knoten) und deren 
   Anzahl (default: 4). Besitzt eine Kante keinen hängenden Knoten, 
   dann -1.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   R int *anzahl: Anzahl der Kantenknoten (default: 4)
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Kantenknoten
                                                                          */
/* Programmaenderungen:
   07/2002     RK        Erste Version
                                                                          */
/**************************************************************************/
long *ElGetEdgeNodes2D ( long number, int *anzahl )
{
  *anzahl = ((Element *) elementlist->elements[number])->anz_ele_edge_nodes2d;
  return ((Element *) elementlist->elements[number])->ele_edge_nodes2d;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetEdgeNodes2D
                                                                          */
/* Aufgabe:
   Setzt die Kantenknoten des Elements number. Ein evtl. bestehendes Feld
   wird geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   X long *edge_nodes: Feld mit den Kantenknoten
   E int anz: Anzahl der Kantenknoten; anz<0: keine Aenderung der
              Standardeinstellung
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/2002     RK        Erste Version
                                                                          */
/**************************************************************************/                      
void ElSetEdgeNodes2D ( long number, long *edge_nodes, int anz )
{
  ((Element *)elementlist->elements[number])->ele_edge_nodes2d =
    (long *)Free(((Element *)elementlist->elements[number])->ele_edge_nodes2d);
  ((Element *)elementlist->elements[number])->ele_edge_nodes2d = edge_nodes;
  if (anz>=0)
      ((Element *)elementlist->elements[number])->anz_ele_edge_nodes2d = anz;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementPred
                                                                          */
/* Aufgabe:
   Liefert die Nummer des Vorgaengerelementes des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long ElGetElementPred ( long number )
{
  return ((Element *)elementlist->elements[number])->vorgaenger;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementEdges
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Kanten des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Nachbarn -> Kanten
                                                                          */
/**************************************************************************/
long *ElGetElementEdges ( long number )
{
  return ((Element *)elementlist->elements[number])->kanten;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementPlains
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Flaechen des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
long *ElGetElementPlains ( long number )
{
#ifdef ERROR_CONTROL
    if (elementlist->elements[number]==NULL) {
        DisplayErrorMsg("!!! ElGetElementPlains: Element existiert nicht!");
        abort();
    }
#endif
  return ((Element *)elementlist->elements[number])->kanten;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementChilds
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Kinder des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long *ElGetElementChilds ( long number )
{
  return ((Element *)elementlist->elements[number])->kinder;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementData
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die internen Elementdaten des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void *ElGetElementData ( long number )
{
  return ((Element *)elementlist->elements[number])->eval_intern;
}

/*************************************************************************
  ROCKFLOW - Function: ELEGetElementMatrices
  Task: Access to element matrices
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void *ELEGetElementMatrices(long number,int process)
{
  //int process = 0;
#ifdef PCS_OBJECTS
  //process = PCSGetProcessNumber();
#endif
  return ((Element *)elementlist->elements[number])->element_matrices[process];
}

void *ELEGetElementMatricesPCS(long number,int process)
{
  return ((Element *)elementlist->elements[number])->element_matrices[process];
}

/*************************************************************************
  ROCKFLOW - Function: ELECreateElementMatricesPointer
  Task: Create element matrices pointer
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void ELECreateElementMatricesPointer(long number,int process)
{
  static Element *elem;
  elem = ElGetElement(number);
  elem->element_matrices[process] = InitInternElementData();
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementVal
                                                                          */
/* Aufgabe:
   Liefert einen Elementwert des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int idx: Index des Elementwertes
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
double ElGetElementVal ( long number, int idx )
{
  return ((Element *)elementlist->elements[number])->eval[idx];
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetVelocity
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Geschwindigkeiten in den Gausspunkten des
   Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   08/1994     MSR        Erste Version
    9/2000     CT         Mehrphasen fuer Geschwindigkeiten

                                                                          */
/**************************************************************************/
double *ElGetVelocity ( long number )
{
  return ElGetVelocityNew (number, 0);

}
/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetVelocityNew
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die Geschwindigkeiten in den Gausspunkten des
   Elements number fuer eine Phase
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int phase: Phase
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
    9/2000     CT         Mehrphasen fuer Geschwindigkeiten
                                                                          */
/**************************************************************************/
double *ElGetVelocityNew ( long number, int phase)
{
  return ((Element *)elementlist->elements[number])->gauss_velo[phase];
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetArtDiff
                                                                          */
/* Aufgabe:
   Liefert kuensliche Diffusion des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   09/1996     cb          Erste Version
                                                                          */
/**************************************************************************/
double ElGetArtDiff ( long number )
{
  return ((Element *)elementlist->elements[number])->art_diff;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetVerf
                                                                          */
/* Aufgabe:
   Liefert Merker des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/1996      cb          Erste Version
                                                                          */
/**************************************************************************/
int ElGetVerf ( long number )
{
  return ((Element *)elementlist->elements[number])->verf;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetVerfCount
                                                                          */
/* Aufgabe:
   Liefert Anzahl der Parameter, die das Element fuer die Verfeinerung
   markiert haben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1998      R.Kaiser          Erste Version
                                                                          */
/**************************************************************************/
int ElGetVerfCount ( long number )
{
  return ((Element *)elementlist->elements[number])->count_verf;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetVergrCount
                                                                          */
/* Aufgabe:
   Liefert Anzahl der Parameter, die das Element fuer die Verfeinerung
   markiert haben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1998      R.Kaiser          Erste Version
                                                                          */
/**************************************************************************/
int ElGetVergrCount ( long number )
{
  return ((Element *)elementlist->elements[number])->count_vergr;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetGlobalVerf
                                                                          */
/* Aufgabe:
   Liefert Merker des Elements number (fuer verf_index)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int verf_index: Verfeinerungsindex
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1998     R.Kaiser        Erste Version
                                                                          */
/**************************************************************************/
int ElGetGlobalVerf ( long number, int verf_index )
{
  return ((Element *)elementlist->elements[number])->global_verf[verf_index];
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementDirtyBit
                                                                          */
/* Aufgabe:
   Liefert den Dirty-Status des Elements number
   Wird benutzt fuer den bedingten Wiederaufbau von Matritzen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   R int bit: gesetzter Status
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/

int ElGetElementDirtyBit ( long number )
{
  return ((Element *)elementlist->elements[number])->dirty_bit;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetArtDiff
                                                                          */
/* Aufgabe:
   Setzt kuenstliche Diffusion des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
   E double wert: Double-Wert
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   09/1996     cb         Erste Version
                                                                          */
/**************************************************************************/
void ElSetArtDiff ( long number, double wert )
{
  ((Element *)elementlist->elements[number])->art_diff = wert;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetVerf
                                                                          */
/* Aufgabe:
   Setzt Merker des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
   E int wert: Int-Wert
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/1996     cb         Erste Version
                                                                          */
/**************************************************************************/
void ElSetVerf ( long number, int wert )
{
  ((Element *)elementlist->elements[number])->verf = wert;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetVerfCount
                                                                          */
/* Aufgabe:
   Setzt Anzahl der Parameter, die das Element number fuer die
   Verfeinerung markiert haben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
   E int wert: Int-Wert
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1998     R.Kaiser         Erste Version
                                                                          */
/**************************************************************************/
void ElSetVerfCount ( long number, int wert )
{
  ((Element *)elementlist->elements[number])->count_verf = wert;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetVergrCount
                                                                          */
/* Aufgabe:
   Setzt Anzahl der Parameter, die das Element number fuer die
   Verfeinerung markiert haben
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
   E int wert: Int-Wert
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1998     R.Kaiser         Erste Version
                                                                          */
/**************************************************************************/
void ElSetVergrCount ( long number, int wert )
{
  ((Element *)elementlist->elements[number])->count_vergr = wert;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetGlobalVerf
                                                                          */
/* Aufgabe:
   Setzt Merker des Elements number (fuer verf_index)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
   E int verf_index: Verfeinerungsindex
   E int val: Wert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1998     R.Kaiser        Erste Version
                                                                          */
/**************************************************************************/
void ElSetGlobalVerf ( long number, int verf_index, int val )
{
  ((Element *)elementlist->elements[number])->global_verf[verf_index] = val;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElSetElementLevel
                                                                          */
/* Aufgabe:
   Setzt Verfeinerungslevel des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementindex
   E int wert: Int-Wert
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   04/1998     R.Kaiser         Erste Version
                                                                          */
/**************************************************************************/
void ElSetElementLevel ( long number, int wert )
{
  ((Element *)elementlist->elements[number])->level = wert;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementNodesNumber
                                                                          */
/* Aufgabe:
   Liefert die Anzahl der Eckknoten des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Zugriff auf globales Feld
                                                                          */
/**************************************************************************/
int ElGetElementNodesNumber ( long number )
{
  return ElNumberOfNodes[((Element *)elementlist->elements[number])->elementtyp-1];
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementEdgesNumber
                                                                          */
/* Aufgabe:
   Liefert Anzahl der Kanten des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Zugriff auf globales Feld, Nachbarn -> Kanten
                                                                          */
/**************************************************************************/
int ElGetElementEdgesNumber ( long number )
{
  return ElNumberOfEdges[((Element *)elementlist->elements[number])->elementtyp-1];
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementPlainsNumber
                                                                          */
/* Aufgabe:
   Liefert Anzahl der Flaechen des Elements number
   Bei 3D-Netzen existiert fuer 1D-Elemente eine eigene
   Strukturkomponente !!!
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ElGetElementPlainsNumber ( long number )
{
  return ((Element *)elementlist->elements[number])->anz_flaechen;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElGetElementChildsNumber
                                                                          */
/* Aufgabe:
   Liefert Anzahl der Kinder des Elements number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   06/1995     MSR        Zugriff auf globales Feld
                                                                          */
/**************************************************************************/
int ElGetElementChildsNumber ( long number )
{
  return ElNumberOfChilds[((Element *)elementlist->elements[number])->elementtyp-1];
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ElDeleteElementChilds
                                                                          */
/* Aufgabe:
   Loescht die Kindereintragungen des Elements number und erdet den
   Zeiger.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElDeleteElementChilds ( long number )
{
  ((Element *)elementlist->elements[number])->kinder = (long *)Free(((Element *)elementlist->elements[number])->kinder);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ElDeleteElementGaussVeloPhase
                                                                          */
/* Aufgabe:
   Loescht den Eintrag der Phasen-Geschwindigkeiten 
   in den Gausspunkten des Elements number 
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ElDeleteElementGaussVeloPhase (long number,int phase)
{
  ((Element *)elementlist->elements[number])->gauss_velo[phase]
    = (double *)Free(((Element *)elementlist->elements[number])->gauss_velo[phase]);
}


/*----------------------------------------------------------------------------*/
/* Geometry and topology */

void ElSetElementVolume(long number, double value)
{
    /* //WW
    if (value < CRITICAL_ELEMENT_VOLUME) {
        DisplayMsgLn("");
        DisplayMsg("Element ");
        DisplayLong(number);
        DisplayMsg(" hat ein Volumen von ");
        DisplayDouble(value, 0, 0);
        DisplayMsgLn("");
    }
*/
  ((Element *)elementlist->elements[number])->volume = value;

}

double ElGetElementVolume ( long number )
{
  return ((Element *)elementlist->elements[number])->volume;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: CreateModelElementData
                                                                          */
/* Aufgabe:
   Speicher fuer Elementmatrizen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
   07/2000   OK   Erweiterung fuer mehrfachen Aufruf
   11/2001   AH   Initialsierung der Daten erst in InitModelElementData
                                                                          */
/**************************************************************************/
void CreateModelElementData ( long number )
{
  /*static int i;*/
  static Element *elem;

  elem = ElGetElement(number);
  if(!elem->eval_intern) //OK
    elem->eval_intern = InitInternElementData();
  if(!(elem->eval)) /* OK rf3419 */
      elem->eval = (double *)Malloc(anz_eval*sizeof(double));
  /*for (i=0;i<anz_eval;i++)
      elem->eval[i] = eval_data[i].vorgabe;*/
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyModelElementData
                                                                          */
/* Aufgabe:
   Speicher fuer Elementmatrizen freigeben.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void DestroyModelElementData ( long number )
{
  static Element *elem;

  elem = ElGetElement(number);
  if (elem && elem->eval) elem->eval=(double *)Free(elem->eval);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitModelElementData
                                                                          */
/* Aufgabe:
   Speicher fuer Elementmatrizen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
   07/2000   OK   Erweiterung fuer mehrfachen Aufruf
   10/2001   AH   CreateModelElementData in InitModelElementData 
                  umbenannt    ah inv
                  Keine Initialisierung der internen Modelldaten:
                  InitInternElementData() ausgeklammert
                  Keine Speicher-Anfrage
                                                                          */
/**************************************************************************/
void InitModelElementData ( long number )
{
  static int i;
  static Element *elem;

  elem = ElGetElement(number);
  /*if(!(elem->eval))*/ /* OK rf3419 */
      /*elem->eval = (double *)Malloc(anz_eval*sizeof(double));*/

  if(elem->eval)
    for (i=0;i<anz_eval;i++)
      elem->eval[i] = eval_data[i].vorgabe;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigTopology
                                                                          */
/* Aufgabe:
   Konfigurationen fuer toplogische Objekte:
   - maximale Elementdimension ermitteln
   - Element-zu-Knoten-Verzeichnis aufstellen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   12/1999   OK   Implementierung
   04/2002   OK   Erweiterung für neue Elemente
04/2005 OK MSH
                                                                          */
/**************************************************************************/
void ConfigTopology(void)
{
  long i;
  int ele_dim=0;
  //----------------------------------------------------------------------
  // maximale Elementdimension bestimmen: z.B. fuer Umnummerierer
  for (i = 0; i < ElListSize(); i++) {
    if (ElGetElement(i) != NULL) {
      switch(ElGetElementType(i)) {
        case 1: ele_dim = 0; break; /* 1-D bar element */
        case 2: ele_dim = 1; break; /* 2-D quadrilateral element */
        case 3: ele_dim = 2; break; /* 3-D hexahedral element */
        case 4: ele_dim = 1; break; /* 2-D triagular element */
        case 5: ele_dim = 2; break; /* 3-D tetrahedral element */
        case 6: ele_dim = 2; break; /* 3-D prismatic element */
        /* besser ELEGetElementDimension(i) */
      }
    max_dim = max(max_dim,ele_dim);
    }
  }
  //----------------------------------------------------------------------
   /* Geometrie - Topologie */
     /* Kanten- bzw. Flaechenverzeichnis aufbauen */
/*
    ConstructRelationships();
    ConstructStatusEntries();
*/
  //----------------------------------------------------------------------
  // Element-zu-Knoten-Verzeichnis aufstellen (u.a. fuer Cuthill benoetigt !!!)
   ConstructElemsToNodesList();  /* auch bei Restart o.k. */
}


//WW
void Init_Quadratic_Elements (void)
{
    HighOrder = 2;
    NodeListLength = NodeListLength_H;

    if(max_dim==0) 
    {
        DisplayMsg("Init_Quadratic_Elements: 2D or 3D elements only!!!");
        DisplayMsg("No RFI Data");
        //OK abort();
    } 
}
//WW
void Init_Linear_Elements (void)
{
    HighOrder = 1;
    NodeListLength = NodeListLength_L;

    if(max_dim==0) 
    {
        DisplayMsg("Init_Quadratic_Elements: 2D or 3D elements only!!!");
        DisplayMsg("No RFI Data");
        //OK abort();
    } 
}

/*****************************************************/
/*  Function: GetLowOrderNodeNumber
    Means as the name.
    08/2003  WW 
*/  
long GetLowOrderNodeNumber(void)
{
  return NodeListLength_L;
  /*
  long NodeListSize_Low=0; 
  const long NodeListSize_Current=NodeListSize(); 
  Init_Linear_Elements();
  NodeListSize_Low = NodeListSize(); 
  if(NodeListSize_Current!=NodeListSize_Low)
    Init_Quadratic_Elements();
  return NodeListSize_Low; 
  */
}
/*****************************************************/
/*  Function: GetHighOrderNodeNumber
    Means as the name.
    08/2003  WW 
*/  

long GetHighOrderNodeNumber(void)
{
  return NodeListLength_H;
  /*
  long NodeListSize_High=0; 
  const long NodeListSize_Current=NodeListSize(); 
  Init_Quadratic_Elements();
  NodeListSize_High = NodeListSize(); 
  if(NodeListSize_Current!=NodeListSize_High)
    Init_Linear_Elements();
  return NodeListSize_High; 
  */
}


/**************************************************************************
  GEOSYS - Funktion: Generate_Quadratic_Elements
                                                                          
 Task:
   As the function name
 Progamming:                                                                         
   09/2004   WW   
**************************************************************************/
void Generate_Quadratic_Elements()
{
    long i, NodeIndex=0;
    long *element_nodes_L; // Low order
    long *element_nodes_H; // High order
    long *node_nodes1, *node_nodes2; // New nodes associate to a vertex
    static Knoten *node;

    int EdgeVetex[2];   
    int j,k,l, nL, nH, nEdges, kk;
    int nNodesVet1, nNodesVet2;
    bool node_exist;

    if(max_dim==0) 
    {
        DisplayMsg("Init_Quadratic_Elements: 2D or 3D elements only!!!");
        DisplayMsg("No RFI data"); //OK
        //OK abort();
        return;
    } 

    NodeListLength_L = NodeListLength;

    for (i = 0; i < ElListSize(); i++)
    {
       if (ElGetElement(i) != NULL)     // Element existiert 
         if (ElGetElementActiveState(i)) // aktives Element 
         {
             nL = NumbersOfElementNode(i);
             nH = NumbersOfElementNodeHQ(i);
             nEdges = NumbersOfElementEdge(i);
             element_nodes_L = ElGetElementNodes(i);
             element_nodes_H = (long *) Malloc(nH*sizeof(long)); 
             
             // Create new node by edges
             kk = nL-1;
             for(j=0; j<nL; j++) 
                element_nodes_H[j] = element_nodes_L[j];

             for(j=0; j<nEdges; j++) 
             {
                GetElementEdge(i, j, EdgeVetex);
                node_nodes1 = GetNodeNodes(element_nodes_L[EdgeVetex[0]],&nNodesVet1);
                node_nodes2 = GetNodeNodes(element_nodes_L[EdgeVetex[1]],&nNodesVet2);
                
                // Check if this edge is done
                node_exist = false;
                for (k=0;k<nNodesVet1;k++) 
                {
                   for (l=0;l<nNodesVet2;l++) 
                   {
                      if (node_nodes1[k] == node_nodes2[l])
                      { 
                         node_exist = true;
                         NodeIndex = node_nodes1[k];
                         break;
                      }
                   }
                   if (node_exist)  break;
                }
                
                if(!node_exist)
                {
                 
                    NodeIndex = AddNode(node=CreateNodeGeometry());
                    // Add information to edge verteces
                    AssignNodeNode(element_nodes_L[EdgeVetex[0]],NodeIndex);
                    AssignNodeNode(element_nodes_L[EdgeVetex[1]],NodeIndex);

                    // 
                    node->x = ( (GetNode(element_nodes_L[EdgeVetex[0]]))->x 
                              + (GetNode(element_nodes_L[EdgeVetex[1]]))->x )* 0.5;
                    node->y = ( (GetNode(element_nodes_L[EdgeVetex[0]]))->y 
                              + (GetNode(element_nodes_L[EdgeVetex[1]]))->y )* 0.5;
                    node->z = ( (GetNode(element_nodes_L[EdgeVetex[0]]))->z 
                              + (GetNode(element_nodes_L[EdgeVetex[1]]))->z )* 0.5;
                }
                kk++;
                element_nodes_H[kk] = NodeIndex;
             } // Edge
             if(ElGetElementType(i)==2) // Quadralateral
             {
                kk++;
                NodeIndex = AddNode(node=CreateNodeGeometry());
                //
                node->x = 0.0;
                node->y = 0.0;
                node->z = 0.0;
                for (k=0;k<nL;k++) 
                {
                   node->x += (GetNode(element_nodes_L[k]))->x;
                   node->y += (GetNode(element_nodes_L[k]))->y;
                   node->z += (GetNode(element_nodes_L[k]))->z;
                }
                node->x /= (double)nL;
                node->y /= (double)nL;
                node->z /= (double)nL;

                element_nodes_H[kk] = NodeIndex;
             }

             // Update element nodes
             ElSetElementNodes(i,element_nodes_H);
             element_nodes_H = NULL;
         }
    }

    NodeListLength_H = NodeListLength;    
    
    Init_Linear_Elements();
}


/* Listen: Listen, Baeume, etc. */


/* SB: Calculate and Set Element Midpoints */
void ELECalcEleMidPoint(void){

int i;
static double middel_coord[3];
 
  for (i = 0; i < ElListSize(); i++)
	if (ElGetElement(i) != NULL) {
		CalculateMiddelPointElement(i, middel_coord);
		ELESetEleMidPoint(i,middel_coord);
	}
}

/* SB: */
void ELESetEleMidPoint(int i, double *middel_coord){
	int j;
	static Element *elem;

	elem = ElGetElement(i);
	if(elem->element_midpoint == NULL)
		elem->element_midpoint = (double *) Malloc(3 * sizeof(double));
	for(j=0;j<3;j++)
		elem->element_midpoint[j] = middel_coord[j];
}

double ELEGetEleMidPoint(int i, int j){
	return ElGetElement(i)->element_midpoint[j];
}

void ELEPrintEleMidPoint(void){
	int i, j;
	static double coord;
	FILE *outfile;

	outfile = fopen("elementmidpoints","w");

	for (i = 0; i < ElListSize(); i++)
		if (ElGetElement(i) != NULL) {
			fprintf(outfile," %i ",i);
			for(j=0;j<3;j++){
				coord = ELEGetEleMidPoint(i,j);
				fprintf(outfile," %f ",coord);
			}
			fprintf(outfile,"\n");
		}
	fclose(outfile);
}
void ELESetHetFieldValues(int i, int nof, double *values){
	int j;
	static Element *elem;

	elem = ElGetElement(i);
	if(elem == NULL)
		DisplayMsgLn(" ERROR! Element not found ");
	if(elem->hetfields == NULL)
		elem->hetfields = (double *) Malloc(nof * sizeof(double));
	for(j=0;j<nof;j++)
		elem->hetfields[j] = values[j];
}

double ELEGetHetFieldValue(int i, int j){
	return ElGetElement(i)->hetfields[j];
}

/*************************************************************************
  ROCKFLOW - Function: GEOSetELEJacobianMatrix
  Task: Access functions to Jacobian matrices
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void GEOSetELEJacobianMatrix(long number,double *invjac,double detjac)
{
  static Element *element;
  element = ElGetElement(number);
  //element->invjac = (double *)Free(element->invjac);
  element->invjac = invjac;
  element->detjac = detjac;
}

/*************************************************************************
  ROCKFLOW - Function: GEOGetELEJacobianMatrix
  Task: Access functions to Jacobian matrices
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
double *GEOGetELEJacobianMatrix(long number,double *detjac)
{
  static Element *element;
  element = ElGetElement(number);
  *detjac = element->detjac;
  return element->invjac;
}


/*************************************************************************
  ROCKFLOW - Function: ELEConfigJacobianMatrix
  Task: Access functions to Jacobian matrices
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void ELEConfigJacobianMatrix(void)
{
  SetElementJacobiMatrix = GEOSetELEJacobianMatrix;
  GetElementJacobiMatrix = GEOGetELEJacobianMatrix;
}


/**************************************************************************
ROCKFLOW - Funktion: ELEListExists
Task: Test if list exists
Programing:
11/2003 OK Implementation
**************************************************************************/
int ELEListExists(void)
{
  if(!elementlist)
    return 0;
  else
    return 1;
}


/**************************************************************************
GeoSys-FEM-Method: ELECreateTopology
Task: create element topology
Programing:
12/2003 OK Implementation based on CreateElementTopology
**************************************************************************/
void ELECreateTopology ( int elementtyp, long vorgaenger, int level, long pos )
{
  int i;
  static Element *elem;

  elem = (Element *) Malloc(sizeof(Element));

  if (vorgaenger >= 0) {   /* CT */
      elem->group_number = ElGetElementGroupNumber(vorgaenger);
      if (level==-1)
          elem->level = ((Element *)elementlist->elements[vorgaenger])->level + 1;
      else
          elem->level = /* 0 msr 0596 */ level;
  }  else  {
      elem->group_number = -1;
      if (level==-1)
          elem->level = 0;
      else
          elem->level = /* 0 msr 0596 */ level;
  }

  elem->element_start_number = -1;  /* ah */
  elem->elementtyp = elementtyp;
  elem->aktiv = 1;
  elem->count_verf = 0;
  elem->count_vergr = 0;
  elem->verfeinern = 0;
  elem->elementknoten = NULL;
  elem->ele_edge_nodes2d = NULL;
  elem->Neighbors = NULL;
  elem->anz_ele_edge_nodes2d = 4;
  elem->vorgaenger = vorgaenger;
  elem->kanten = NULL;
  elem->anz_flaechen = ElNumberOfPlains[elementtyp-1];
  elem->kinder = NULL;

  elem->global_verf = (int *)Malloc(AdaptGetTotalNumInd()*sizeof(int));
  for (i=0;i<AdaptGetTotalNumInd();i++)
      elem->global_verf[i] = 0;

  elem->art_diff = 0.0;
  elem->verf = 0;
  elem->dirty_bit = 65535;
  elem->volume = 0.0;

  elem->invjac = NULL;
  elem->eval = NULL;
  elem->element_midpoint = NULL; //SB
  elem->hetfields = NULL;
  elem->gauss_velo = NULL;

  elem->eval_intern = NULL; //SB:3912
#ifdef ERROR_CONTROL
    if (ElGetElement(pos)!=NULL) {
        DisplayErrorMsg("ElPlaceNewElement: Element existiert bereits !!!");
        abort();
    }
#endif

  SetPtrArrayElement(elementlist, pos, (void *) elem);
  lowest_free_element = GetLowestFree(elementlist, lowest_free_element);
  ElementListLength++;
}


/**************************************************************************
GeoSys-FEM-Method: ELECreateMatricesPointer
Task: create element matrices pointer
Programing:
12/2003 OK Implementation based on CreateElementTopology
**************************************************************************/
void ELECreateMatricesPointer(long number)
{
  Element *elem=NULL;
  int i;

  elem = ElGetElement(number);
  if(InitInternElementData)
    elem->eval_intern = InitInternElementData();
  else //OK
    elem->eval_intern = NULL;

//OKnew
  for(i=0;i<PCS_NUMBER_MAX;i++)
    elem->element_matrices[i] = NULL;
}

/**************************************************************************
GeoSys-FEM-Method: ELECreateValues
Task: create element values
Programing:
12/2003 OK Implementation based on CreateElementTopology
**************************************************************************/
//void ELECreateValues (long number)
/*************************************************************************
  ROCKFLOW - Function: ELECreateElementData
  Task: Create element data
  Programming: 05/2003 OK Implementation
  last modified:
**************************************************************************/
void ELECreateElementData(long number)
{
  static int i;
  static Element *elem;

  elem = ElGetElement(number);
  if(!(elem->eval))
    elem->eval = (double *)Malloc(anz_eval*sizeof(double));
  else 
    elem->eval = (double *)Realloc(elem->eval,anz_eval*sizeof(double));
  for (i=0;i<anz_eval;i++)
    elem->eval[i] = eval_data[i].vorgabe;
}

/**************************************************************************
GeoSys-FEM-Method: ELECreateGPPointer
Task: create element Gauss point values
Programing:
12/2003 OK Implementation based on CreateElementTopology (WW)
06/2004 WW Gauss value of stresses are moved to pcs_dm.h/cpp
**************************************************************************/
void ELECreateGPValues (long number)
{
  Element *elem=NULL;
  int NumGauss,anz,i, k;

  elem = ElGetElement(number);

  /* Speicher fuer Geschw. in Gausspunkten */
  anz = -1;

  NumGauss = GetNumericsGaussPoints(elem->elementtyp);
  switch (elem->elementtyp) {
     case 1:
       anz = 3;
       break;
     case 2:
       anz = 2 * NumGauss * NumGauss;
       break;
     case 3:
       anz = 3 * NumGauss * NumGauss * NumGauss;
       break;
     case 4:
       anz = 3;
       break;
     case 5: /* Tetraeder */
       anz = 3;
       break;
     case 6: /* Prism MB */
       //anz = 3;
       anz = 3 * 2 * 3; //3D * 2(Linear) / 3(Triangle) 
       NumGauss = 18;
       break;
     default:
    DisplayMsgLn("Unsupported element-type in CreateElementTopology");
    exit(1);
  }

  /*---------------------------------------------------------------------------------*/
  /* Velocities at Gauss points*/
  if(!elem->gauss_velo)
    elem->gauss_velo = (double **) Malloc(GetRFProcessNumPhases() * sizeof(double *));
  for (i=0;i<GetRFProcessNumPhases();i++) {
    //if(!elem->gauss_velo[i]) ? where to initialize
      elem->gauss_velo[i] = (double *) Malloc(anz * sizeof(double));
      // Initialization WW
      for(k=0; k<anz; k++) elem->gauss_velo[i][k] = 0.0;           
  }
}

/**************************************************************************
GeoSys-FEM-Method: ELEDestroyTopology
Task:
Programing:
12/2003 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDestroyTopology (long number)
{
  if (elementlist->elements[number]!=NULL) {
    ((Element *)elementlist->elements[number])->elementknoten = (long *)Free(((Element *)elementlist->elements[number])->elementknoten);
    ((Element *)elementlist->elements[number])->ele_edge_nodes2d = (long *)Free(((Element *)elementlist->elements[number])->ele_edge_nodes2d); 
    ((Element *)elementlist->elements[number])->kanten = (long *)Free(((Element *)elementlist->elements[number])->kanten);
    ((Element *)elementlist->elements[number])->kinder = (long *)Free(((Element *)elementlist->elements[number])->kinder);
    ((Element *)elementlist->elements[number])->global_verf = (int *)Free(((Element *)elementlist->elements[number])->global_verf);
    elementlist->elements[number] = Free((Element *) elementlist->elements[number]);
    ElementListLength--;
    if (lowest_free_element > number)
      lowest_free_element = number;
  }
}

/**************************************************************************
GeoSys-FEM-Method: ELEDestroyMatrices
Task:
Programing:
12/2003 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDestroyMatrices(long i)
{
  int j;

  if(DestroyInternElementData)
    ((Element *)elementlist->elements[i])->eval_intern = \
      DestroyInternElementData(((Element *)elementlist->elements[i])->eval_intern);

  int no_processes =(int)pcs_vector.size();
  for(j=0;j<no_processes;j++) {
    if(PCSDestroyELEMatrices[j]) //OK
      ((Element*)elementlist->elements[i])->element_matrices[j] = \
        PCSDestroyELEMatrices[j](((Element *)elementlist->elements[i])->element_matrices[j]);
  }
  if(((Element*)elementlist->elements[i])->invjac)
    ((Element*)elementlist->elements[i])->invjac = \
      (double *)Free(((Element *)elementlist->elements[i])->invjac);
}

/**************************************************************************
GeoSys-FEM-Method: ELEDestroyValues
Task:
Programing:
12/2003 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDestroyValues(long i)
{
  ((Element *)elementlist->elements[i])->eval = (double *)Free(((Element *)elementlist->elements[i])->eval);
}

/**************************************************************************
GeoSys-FEM-Method: ELEDestroyGPValues
Task:
Programing:
12/2003 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDestroyGPValues(long i)
{
  int j;
  /* Velocites at Gauss points */
  if(((Element *)elementlist->elements[i])->gauss_velo) {
    for (j=0;j<GetRFProcessNumPhases();j++)
      ((Element *)elementlist->elements[i])->gauss_velo[j] = (double *)Free(((Element *)elementlist->elements[i])->gauss_velo[j]);
    ((Element *)elementlist->elements[i])->gauss_velo = (double **)Free(((Element *)elementlist->elements[i])->gauss_velo);
  }
}

/**************************************************************************
GeoSys-FEM-Method: ELEDestroyPtrArray
Task:
Programing:
12/2003 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDestroyPtrArray (void)
{
  long i,l;
  if(ELEListExists())
  {
    l = PtrArrayLength(elementlist);
    for (i=0l;i<l;i++) {
      if (elementlist->elements[i]!=NULL) {
        ELEDestroyMatrices(i);
        ELEDestroyValues(i);
        ELEDestroyGPValues(i);
        ELEDestroyTopology(i);
      }
    }
  }

  ResizePtrArray(elementlist,0l);                 /* evtl. ueberfluessig */
  ElementListLength = 0l;
  lowest_free_element = 0l;
  DestroyPtrArray(elementlist);
  elementlist = NULL; //OK

  /* OK rf3424 */
  anz_1D = 0l;
  anz_2D = 0l;
  anz_3D = 0l;

}
/**************************************************************************
GeoSys-FEM-Method: ElementDimension
Task: Get the space dimenson of an element
Programing:
06/2004 WW  
**************************************************************************/
int ElementDimension(const long Index)
{
	int ele_dim = 1;
    switch(ElGetElementType(Index))
	{
	   case 1: ele_dim = 1; break; // 1-D bar element 
       case 2: ele_dim = 2; break; // 2-D quadrilateral element 
       case 3: ele_dim = 3; break; // 3-D hexahedral element 
       case 4: ele_dim = 2; break; // 2-D triagular element 
       case 5: ele_dim = 3; break; // 3-D tetrahedral element 
       case 6: ele_dim = 4; break; // 3-D prismatic element 
	}
    return ele_dim; 
}

/**************************************************************************
GeoSys-FEM-Method: ELEDeleteElementFromList
Task:
Programing:
09/2002 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDeleteElementFromList(long element_start,long element_end)
{
  long i,j,k,l;
  int k0;
  long *element_nodes;
  long array_length=0l;
  long this_node;
  long *array=NULL;
  int delete_this_node=0;  
  long old_node_array_length;
  long *node_table;
  long *new_element_nodes;
  Element *element=NULL;
  long *node_array_delete=NULL;
  long number_of_nodes_delete;
  int nn;

  /* 1 Array of nodes belonging to element to be deleted */
  /* 1 Delete elements from element array */
  k0 = ElNumberOfNodes[ElGetElementType(element_start)-1];
  for (i=element_start;i<element_end+1;i++) {
    element_nodes = ElGetElementNodes(i);
    k = ElNumberOfNodes[ElGetElementType(i)-1];
    array_length += k;
    array = (long*) Realloc(array,array_length*sizeof(long));
    for (j=0;j<k;j++)
      array[array_length+j-k0]=element_nodes[j];
    ELEDestroyTopology(i);
    //ElDeleteElement(i);
  }

  /* 2 Reduce and renumber element array */
  ElCompressElementList();
  for (i=0;i<ElListSize();i++) {
    element = ElGetElement(i);
    ElSetElementStartNumber(i,i);
  }

  /* 3 Search for unconnected nodes */
  old_node_array_length = NodeListLength;
  l=0;
  for (i=0;i<array_length;i++) {
    this_node = array[i];
    for (j=0;j<ElListSize();j++) {
      element_nodes = ElGetElementNodes(j);
      for (k=0;k<ElNumberOfNodes[ElGetElementType(j)-1];k++) {
        delete_this_node = 1;
        if (this_node==element_nodes[k]) {
          delete_this_node = 0;
          break;
        }
      if(!delete_this_node) break;
      }
    if(!delete_this_node) break;
    }
    if(GetNode(this_node)&&delete_this_node) {
      node_array_delete = (long*) Realloc(node_array_delete,++l*sizeof(long));
      node_array_delete[l] = this_node;
      NODDeleteNodeGeometric(this_node);
    }
  }
  number_of_nodes_delete = l;

/*
  new_node_array_length = old_node_array_length - number_of_nodes_delete;
  for (i=0;i<number_of_nodes_delete;i++) {
    if (node_array_delete[i]>new_node_array_length) {
      node_array_available[j] = node_array_delete[i];
    }
  }
*/

  /* 4 Renumber nodes */
  j=0;
  node_table = (long *) Malloc(NodeListSize()*sizeof(long));
  for (i=0;i<NodeListSize();i++) {
    if (GetNode(i)!=NULL) {
      node_table[i] = j;
      j++;
    }
  }

  /* 5 New element nodes */
  for (i=0;i<ElListSize();i++) {
    nn = ElNumberOfNodes[ElGetElementType(i)-1];
    element_nodes = ElGetElementNodes(i);
    new_element_nodes = (long*) Malloc(nn*sizeof(long));
    for (j=0;j<nn;j++) {
      new_element_nodes[j] = node_table[element_nodes[j]];
    }
    ElSetElementNodes(i,new_element_nodes);
    new_element_nodes = ElGetElementNodes(i);
  }

  CompressNodeList();

  array = (long*) Free(array);
  node_table = (long*) Free(node_table);

}


/**************************************************************************
GeoSys-FEM-Method: ELEDelete
Task:
Programing:
09/2002 OK Implementation based on ElDestroyList
**************************************************************************/
void ELEDelete(long index)
{
  if(ELEListExists()) {
    if (elementlist->elements[index]!=NULL) {
      ELEDestroyMatrices(index);
      ELEDestroyValues(index);
      ELEDestroyGPValues(index);
      ELEDestroyTopology(index);
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2000 AH Aus CalcMassFluxXX uebernommen und abgespeckt.
11/2004 OK Triangles
last modification:
**************************************************************************/
int CalculateMiddelPointElement(long index, double *coord)
{
    /* Laufvariablen und Hilfsfelder */
    static int i;
    static double vf[12];       /* Vektorfeld */
    static long *nodes;
    static double trans[6];     /* Drehmatrix */

    static double X1, X2, Y1, Y2, Z1, Z2;
    static double koma[24];
    static Knoten *node;

    switch (ElGetElementType(index)) {
    case 1:{
            nodes = ElGetElementNodes(index);
            coord[0] = 0.5 * (GetNodeX(nodes[0]) + GetNodeX(nodes[1]));
            coord[1] = 0.5 * (GetNodeY(nodes[0]) + GetNodeY(nodes[1]));
            coord[2] = 0.5 * (GetNodeZ(nodes[0]) + GetNodeZ(nodes[1]));
            break;
        }
    case 2:{
            /* Koordinatentransformatiomsmatrix [2D_T_3D] und
               Knotenkoordinatenmatrix [A_3D]^T berechnen */
            CalcTransformationsMatrix(index, vf, trans);
            /* Berechnung des Elementmittelpunktes */
            coord[0] = (vf[3] + vf[0] + vf[9] + vf[6]) / 4;
            coord[1] = (vf[4] + vf[1] + vf[10] + vf[7]) / 4;
            coord[2] = (vf[5] + vf[2] + vf[11] + vf[8]) / 4;

            break;
        }
    case 3:{
            nodes = ElGetElementNodes(index);
            for (i = 0; i < 8; i++) {
                node = GetNode(nodes[i]);
                koma[i * 3] = node->x;
                koma[i * 3 + 1] = node->y;
                koma[i * 3 + 2] = node->z;
            }

            vf[0] = koma[0] + 0.5 * (koma[3] - koma[0]);
            vf[1] = koma[1] + 0.5 * (koma[4] - koma[1]);
            vf[2] = koma[2] + 0.5 * (koma[5] - koma[2]);
            vf[3] = koma[6] + 0.5 * (koma[9] - koma[6]);
            vf[4] = koma[7] + 0.5 * (koma[10] - koma[7]);
            vf[5] = koma[8] + 0.5 * (koma[11] - koma[8]);

            vf[6] = koma[12] + 0.5 * (koma[15] - koma[12]);
            vf[7] = koma[13] + 0.5 * (koma[16] - koma[13]);
            vf[8] = koma[14] + 0.5 * (koma[17] - koma[14]);
            vf[9] = koma[18] + 0.5 * (koma[21] - koma[18]);
            vf[10] = koma[19] + 0.5 * (koma[22] - koma[19]);
            vf[11] = koma[20] + 0.5 * (koma[23] - koma[20]);


            /* Berechnung des Mittelpunkte der Elementflaechen */
            X1 = vf[0] + 0.5 * (vf[3] - vf[0]);
            Y1 = vf[1] + 0.5 * (vf[4] - vf[1]);
            Z1 = vf[2] + 0.5 * (vf[5] - vf[2]);
            X2 = vf[6] + 0.5 * (vf[9] - vf[6]);
            Y2 = vf[7] + 0.5 * (vf[10] - vf[7]);
            Z2 = vf[8] + 0.5 * (vf[11] - vf[8]);

            /* Berechnung des Elementmittelpunktes */
            coord[0] = X1 + 0.5 * (X2 - X1);
            coord[1] = Y1 + 0.5 * (Y2 - Y1);
            coord[2] = Z1 + 0.5 * (Z2 - Z1);
            break;
        }
      case 4:
            nodes = ElGetElementNodes(index);
            coord[0] = (GetNodeX(nodes[0])+GetNodeX(nodes[1])+GetNodeX(nodes[2]))/3.;
            coord[1] = (GetNodeY(nodes[0])+GetNodeY(nodes[1])+GetNodeY(nodes[2]))/3.;
            coord[2] = (GetNodeZ(nodes[0])+GetNodeZ(nodes[1])+GetNodeZ(nodes[2]))/3.;
            break;
      case 6:
            nodes = ElGetElementNodes(index);
            coord[0] = (GetNodeX(nodes[0])+GetNodeX(nodes[1])+GetNodeX(nodes[2])+\
                        GetNodeX(nodes[3])+GetNodeX(nodes[4])+GetNodeX(nodes[5]))/6.;
            coord[1] = (GetNodeY(nodes[0])+GetNodeY(nodes[1])+GetNodeY(nodes[2])+\
                        GetNodeY(nodes[3])+GetNodeY(nodes[4])+GetNodeY(nodes[5]))/6.;
            coord[2] = (GetNodeZ(nodes[0])+GetNodeZ(nodes[1])+GetNodeZ(nodes[2])+\
                        GetNodeZ(nodes[3])+GetNodeZ(nodes[4])+GetNodeZ(nodes[5]))/6.;
            break;
    }                           /* switch */
    return 1;
}

/**************************************************************************
GeoSys-FEM-Method: NumbersOfElementNode
Task: Get number of nodes of an element node (Geometry)
Programing:
06/2004 WW  
**************************************************************************/
int NumbersOfElementNode(const long Index)
{
	int n_nodes = 1;
    switch(ElGetElementType(Index))
	{
	   case 1: n_nodes = 2; break; // 1-D bar element 
       case 2: n_nodes = 4; break; // 2-D quadrilateral element 
       case 3: n_nodes = 8; break; // 3-D hexahedral element 
       case 4: n_nodes = 3; break; // 2-D triagular element 
       case 5: n_nodes = 4; break; // 3-D tetrahedral element 
       case 6: n_nodes = 6; break; // 3-D prismatic element 
	}
    return n_nodes; 
}

/**************************************************************************
GeoSys-FEM-Method: NumbersOfElementNodeHQ
Task: Get number of nodes of an element node (High order)
Programing:
06/2004 WW  
**************************************************************************/
int NumbersOfElementNodeHQ(const long Index)
{
	int n_nodes = 1;
    switch(ElGetElementType(Index))
	{
	   case 1: n_nodes = 3; break; // 1-D bar element 
       case 2: n_nodes = 9; break; // 2-D quadrilateral element 
       case 3: n_nodes = 20; break; // 3-D hexahedral element 
       case 4: n_nodes = 6; break; // 2-D triagular element 
       case 5: n_nodes = 10; break; // 3-D tetrahedral element 
       case 6: n_nodes = 15; break; // 3-D prismatic element 
	}
    return n_nodes; 
}
/**************************************************************************
GeoSys-FEM-Method: NumbersOfElementEdge
Task: Get number of nodes of an element node (Geometry)
Programing:
09/2004 WW  
**************************************************************************/
int NumbersOfElementEdge(const long Index)
{
	int edges = 1;
    switch(ElGetElementType(Index))
	{
	   case 1: edges = 0; break; // 1-D bar element 
       case 2: edges = 4; break; // 2-D quadrilateral element 
       case 3: edges = 12; break; // 3-D hexahedral element 
       case 4: edges = 3; break; // 2-D triagular element 
       case 5: edges = 6; break; // 3-D tetrahedral element 
       case 6: edges = 9; break; // 3-D prismatic element 
	}
    return edges; 
}
/**************************************************************************
GeoSys-FEM-Method: GetElementEdge
Task:  Get local egde node indeces 
Programing:
09/2004 WW  
**************************************************************************/
void GetElementEdge(const long EleIndex, const int Edge, int *EdgeNodes)
{
    switch(ElGetElementType(EleIndex))
	{
       case 1: 
           break; // 1-D bar element 
       case 2: // 2-D quadrilateral element 
          EdgeNodes[0] = Edge;
          EdgeNodes[1] = (Edge+1)%4;
          break;             
       case 3: // 3-D hexahedral element
          if(Edge<8)
          {
             EdgeNodes[0] = Edge;
             EdgeNodes[1] = (Edge+1)%4+4*(int)(Edge/4);
          }
          else 
          {
             EdgeNodes[0] = Edge%4;
             EdgeNodes[1] = Edge%4+4;
          }
          break;  
       case 4:  // 2-D triagular element 
          EdgeNodes[0] = Edge;
          EdgeNodes[1] = (Edge+1)%3;
          break;
       case 5:  // 3-D tetrahedra
          if(Edge<3)
          {
             EdgeNodes[0] = Edge;
             EdgeNodes[1] = (Edge+1)%3;
          }
          else
          {
             EdgeNodes[0] = 3;
             EdgeNodes[1] = (Edge+1)%3;
          }

          break;  
       case 6: // 3-D prismatic element
          if(Edge<6)
          {
             EdgeNodes[0] = Edge;
             EdgeNodes[1] = (Edge+1)%3+3*(int)(Edge/3);
          }
          else 
          {
             EdgeNodes[0] = Edge%3;
             EdgeNodes[1] = Edge%3+3;
          }
          break;  
	}
}

/**************************************************************************
GeoSys-FEM-Method: NumbersOfElementNeighbors
Task: Get number of nodes of an element faces (Geometry)
Programing:
06/2004 WW  
**************************************************************************/
int NumbersOfElementFaces(const long Index)
{
	int n_nei = 1;
    switch(ElGetElementType(Index))
	{
	   case 1: n_nei = 2; break; // 1-D bar element 
       case 2: n_nei = 4; break; // 2-D quadrilateral element 
       case 3: n_nei = 6; break; // 3-D hexahedral element 
       case 4: n_nei = 3; break; // 2-D triagular element 
       case 5: n_nei = 4; break; // 3-D tetrahedral element 
       case 6: n_nei = 5; break; // 3-D prismatic element 
	}
    return n_nei; 
}



/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodes
Task: Get local indeces of an element face nodes
Return: number of nodes of a face
Programing:
06/2004 WW  
**************************************************************************/
int GetElementFaces1D(int *FaceNode)
{
    FaceNode[0] = 0;
    FaceNode[1] = 0;
    return 2;
}
/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodesTri
Task: Get local indeces of a traingle element face nodes
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes
Return: number of nodes of a face
Programing:
09/2004 WW  
**************************************************************************/
int GetElementFacesTri(const int Face, const int order, int *FaceNode)
{
    if(order==1)
    {
       FaceNode[0] = Face;
       FaceNode[1] = (Face+1)%3;
       return 2;
    }
    else
    {
       FaceNode[0] = Face;
       FaceNode[1] = Face+3;
       FaceNode[2] = (Face+1)%3;
       return 3;
    }
}

/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodesQuad
Task: Get local indeces of a quadralateral element face nodes
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes
Return: number of nodes of a face
Programing:
09/2004 WW  
**************************************************************************/
int GetElementFacesQuad(const int Face, const int order, int *FaceNode)
{
    if(order==1)
    {
       FaceNode[0] = Face;
       FaceNode[1] = (Face+1)%4;
       return 2;
    }
    else
    {
       FaceNode[0] = Face;
       FaceNode[1] = Face+4;
       FaceNode[2] = (Face+1)%4;
       return 3;
    }
}

/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodesHex
Task: Get local indeces of a hexahedra element face nodes
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes
Return: number of nodes of a face
Programing:
09/2004 WW  
**************************************************************************/
int GetElementFacesHex(const int Face, const int order, int *FaceNode)
{
   int nn=4, k = 0;
   if(order==2) nn = 8;
   switch(Face)
   {
      case 0:
         for(k=0; k<4; k++)
            FaceNode[k] = k;
         if(order==2)
         {
            for(k=0; k<4; k++)
                FaceNode[k+4] = k+8;
         }
         break;
      case 1:
         for(k=0; k<4; k++)
            FaceNode[k] = k+4;
         if(order==2)
         {
            for(k=0; k<4; k++)
                FaceNode[k+4] = k+12;
         }
         break;
      case 2:
         FaceNode[0] = 0;
         FaceNode[1] = 4;
         FaceNode[2] = 5;
         FaceNode[3] = 1;
         if(order==2)
         {
            FaceNode[4] = 16;
            FaceNode[5] = 12;
            FaceNode[6] = 17;
            FaceNode[7] = 8;
         }
         break;
      case 3:
         FaceNode[0] = 1;
         FaceNode[1] = 5;
         FaceNode[2] = 6;
         FaceNode[3] = 2;
         if(order==2)
         {
            FaceNode[4] = 17;
            FaceNode[5] = 13;
            FaceNode[6] = 18;
            FaceNode[7] = 9;
         }
         break;
      case 4:
         FaceNode[0] = 2;
         FaceNode[1] = 6;
         FaceNode[2] = 7;
         FaceNode[3] = 3;
         if(order==2)
         {
            FaceNode[4] = 18;
            FaceNode[5] = 14;
            FaceNode[6] = 19;
            FaceNode[7] = 10;
         }
         break;
      case 5:
         FaceNode[0] = 0;
         FaceNode[1] = 3;
         FaceNode[2] = 7;
         FaceNode[3] = 4;
         if(order==2)
         {
            FaceNode[4] = 11;
            FaceNode[5] = 19;
            FaceNode[6] = 15;
            FaceNode[7] = 16;
         }
         break;
   }
   return nn;
}


/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodesTet
Task: Get local indeces of a Tedrahedra element face nodes
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes
Return: number of nodes of a face
Programing:
09/2004 WW  
**************************************************************************/
int GetElementFacesTet(const int Face, const int order, int *FaceNode)
{
   int nn=3;
   if(order==2) nn =6;
   switch(Face)
   {
      case 0:
         FaceNode[0] = 1;
         FaceNode[1] = 2;
         FaceNode[2] = 3;
         if(order==2)
         {
            FaceNode[3] = 5 ;
            FaceNode[4] = 8;
            FaceNode[5] = 7;
         }
         break;
      case 1:
         FaceNode[0] = 3;
         FaceNode[1] = 2;
         FaceNode[2] = 0;
         if(order==2)
         {
            FaceNode[3] = 8 ;
            FaceNode[4] = 6;
            FaceNode[5] = 9;
         }
         break;
      case 2:
         FaceNode[0] = 1;
         FaceNode[1] = 3;
         FaceNode[2] = 0;
         if(order==2)
         {
            FaceNode[3] = 7 ;
            FaceNode[4] = 9;
            FaceNode[5] = 4;
         }
         break;
      case 3:
         FaceNode[0] = 0;
         FaceNode[1] = 2;
         FaceNode[2] = 1;
         if(order==2)
         {
            FaceNode[3] = 6 ;
            FaceNode[4] = 5;
            FaceNode[5] = 4;
         }
         break;

   }
   return nn;
}


/**************************************************************************
GeoSys-FEM-Method: GetElementFaceNodesPri
Task: Get local indeces of a prismal element face nodes
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes
Return: number of nodes of a face
Programing:
09/2004 WW  
**************************************************************************/
int GetElementFacesPri(const int Face, const int order, int *FaceNode)
{
   int nn=3, k = 0;
   switch(Face)
   {
      case 0:
         nn = 3;
         for(k=0; k<3; k++)
            FaceNode[k] = k;
         if(order==2)
         {
            for(k=0; k<3; k++)
                FaceNode[k+3] = k+6;
            nn = 6;
         }
         break;
      case 1:
         for(k=0; k<3; k++)
            FaceNode[k] = k+3;
         nn = 3;
         if(order==2)
         {
            for(k=0; k<3; k++)
                FaceNode[k+3] = k+9;
            nn = 6;
         }
         break;
      case 2:
         FaceNode[0] = 1;
         FaceNode[1] = 2;
         FaceNode[2] = 5;
         FaceNode[3] = 4;
         nn = 4;
         if(order==2)
         {
            FaceNode[4] = 7 ;
            FaceNode[5] = 14;
            FaceNode[6] = 10;
            FaceNode[7] = 13;
            nn = 8;
         }
         break;
      case 3:
         FaceNode[0] = 5;
         FaceNode[1] = 2;
         FaceNode[2] = 0;
         FaceNode[3] = 3;
         nn = 4;
         if(order==2)
         {
            FaceNode[4] = 14 ;
            FaceNode[5] =  8;
            FaceNode[6] = 12;
            FaceNode[7] = 10;
            nn = 8;
         }
         break;
      case 4:
         FaceNode[0] = 0;
         FaceNode[1] = 1;
         FaceNode[2] = 4;
         FaceNode[3] = 3;
         nn = 4;
         if(order==2)
         {
            FaceNode[4] = 6 ;
            FaceNode[5] = 13;
            FaceNode[6] = 9;
            FaceNode[7] = 12;
            nn = 8;
         }
         break;

   }
   return nn;
}



/**************************************************************************
GeoSys-FEM-Method: GetElementFaces
Task: set element faces (Geometry)
Augs.:
        const int Face :  Local index of element face 
        const int order:  1 Linear. 2, quadratic
        int *FaceNode  :  Local index of face nodes

Programing:
09/2004 WW  
**************************************************************************/
int GetElementFaceNodes(const long Index, const int Face, 
                              int *FacesNode, const int order)
{
    switch(ElGetElementType(Index))
	{
	   case 1:  // 1-D bar element
           return GetElementFaces1D(FacesNode);
           break;          
       case 2: // 2-D quadrilateral element
           return GetElementFacesQuad(Face, order, FacesNode);
           break;           
       case 3: // 3-D hexahedral element 
           return GetElementFacesHex(Face, order, FacesNode);
           break;           
       case 4:  // 2-D triagular element 
           return GetElementFacesTri(Face, order, FacesNode);
           break;           
       case 5:  // 3-D tetrahedral element 
           return GetElementFacesTet(Face, order, FacesNode);
           break;           
       case 6: 
           return GetElementFacesPri(Face, order, FacesNode);
           break; // 3-D prismatic element 
	}
    return 0;
}

/**************************************************************************
GeoSys-FEM-Method: ComputeNeighborsOfElement
Task: Compute neighbors of each element in element list
Programing:
09/2004 WW  
03/2005 WW  
06/2005 OK not completed
**************************************************************************/
void ComputeNeighborsOfElement(void)
{
   bool done;
   int k, l, m, n, nf0, nf1;
   long i, j;
   int n_faces0, n_faces1,  f_index;
   long *Neighbours0, *Neighbours1;
   long *EleNodes0, *EleNodes1;
   int FNodes0[8], FNodes1[8];
   // Allocate memory
   /*
  if(ElListSize()>1e2){ //OKToDo
    cout << "ComputeNeighborsOfElement: not completed" << endl;
    return;
  }*/
   for (i = 0; i < ElListSize(); i++)  
   {
      if (ElGetElement(i) != NULL)     // Element existiert 
        if (ElGetElementActiveState(i))
        {
            n_faces0 = NumbersOfElementFaces(i);
            Neighbours0 = new  long[n_faces0];  
            for(k=0; k<n_faces0; k++)  Neighbours0[k] = -1;           
            ElSetElementNeighbors(i, Neighbours0);

        }
   }	

   // Get neighbors of each element
   for (i = 0; i < ElListSize(); i++)  
   {
      if (ElGetElement(i) != NULL)     // Element existiert 
        if (ElGetElementActiveState(i))
        {
           n_faces0 = NumbersOfElementFaces(i);
           Neighbours0 = ElGetElementNeighbors(i);
           EleNodes0 = ElGetElementNodes(i);

           // Compare faces by comparison of the sum of face node indeces
           for(k=0; k<n_faces0; k++)
           {
               if(Neighbours0[k]==-1)
               {                      
                   done = false;
                   nf0 = GetElementFaceNodes(i, k, FNodes0); 
                   for (j = 0; j < ElListSize(); j++)  
                   {
                      if(i==j) continue;
                      if (ElGetElement(j) != NULL)     // Element existiert 
                        if (ElGetElementActiveState(j))
                        {
                            n_faces1 = NumbersOfElementFaces(j);
                            Neighbours1 = ElGetElementNeighbors(j);
                            EleNodes1 = ElGetElementNodes(j);
                                                         
                            // Compare faces
							/*
                            f_index = 0;
                            for(m=0; m<n_faces1; m++)
                            {
                               if(Neighbours1[m]>-1) f_index++;
							}
							//If all neighbours are found
							if(f_index==n_faces1) continue;
							*/
                            for(m=0; m<n_faces1; m++)
                            {
								/*   
                                if(Neighbours1[m]==i) 
								{
                                    Neighbours0[k] = j;
                                    done = true;
                                    break;                                                     
								}
								*/
                                nf1 = GetElementFaceNodes(j, m, FNodes1); 
                                if(nf0!=nf1) continue;

                                // Summarize the face node index
                                f_index = 0;
                                for(l=0; l<nf0; l++) 
                                {
                                    for(n=0; n<nf1; n++) 
                                    {
                                       if(EleNodes0[FNodes0[l]]==EleNodes1[FNodes1[n]])
                                       { f_index++; break;  }
                                    }
                                }   
                                if(f_index==nf0)
                                {
                                     Neighbours0[k] = j;
                                     Neighbours1[m] = i;  
                                     done = true;
                                     break;                 
                                }
                            }
                            if(done) break;                                               
                        }
                   }
               }
           } // End if done
        }
   }       
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalculateSimpleMiddelPointElement
 */
/* Aufgabe:
   Berechnung des Elementmittelpunktes
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   09/2004  CMCD Intro GeoSys 4
   Takes the average middle point of all element types, stores it in *coord.
   
 */
/**************************************************************************/
void CalculateSimpleMiddelPointElement(long index, double *coord)
{
	int nn,i;
	long* element_nodes = NULL;
	double xnodes[8],ynodes[8],znodes[8];
	double x_mid, y_mid, z_mid;

			nn = ElNumberOfNodes[ElGetElementType(index) - 1];
			element_nodes = ElGetElementNodes(index);
 		
			x_mid = 0.0;
			y_mid = 0.0;
			z_mid = 0.0;
        
			for (i=0;i<nn;i++)
			{
				znodes[i]=GetNodeZ(element_nodes[i]);
				ynodes[i]=GetNodeY(element_nodes[i]);
				xnodes[i]=GetNodeX(element_nodes[i]);
			}

			for (i=0;i<nn;i++)
			{
				x_mid += xnodes[i];
				y_mid += ynodes[i];
				z_mid += znodes[i];
			}
		
			x_mid = x_mid/(double)nn;
			y_mid = y_mid/(double)nn;
			z_mid = z_mid/(double)nn;

			coord[0]=x_mid;
			coord[1]=y_mid;
			coord[2]=z_mid;
}




/*************************************************************************/
//ROCKFLOW - Funktion: CalculateStabilityCriteria

// Aufgabe:
// Calculates the stability criteria for heat flow.

// Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
// E long number: Index des Elements, Diffusion Coefficient, Velcoity in Element, Time Step

// Ergebnis:
// - void -

// Programmaenderungen:
// 09/2004  CMCD Intro GeoSys 4
/**************************************************************************/
void CalculateStabilityCriteria(long index, double D, double vg, double dt)
{
    Element *elem = NULL;
    elem = ElGetElement(index);
    double ideal_char_length;
	double ideal_time_step_dif;
	double ideal_time_step_vel;
	double max_time_step_dif;
	double max_time_step_vel;
    
	//Courant <= 1 // Neumann <= 0.5 // Peclet <= 2// 
	elem->Courant_number = (vg * dt)/elem->Element_Char_Length;
    elem->Neumann_number = 0.5 * (elem->Element_Char_Length*elem->Element_Char_Length) / D;
    elem->Peclet_number = (vg * elem->Element_Char_Length)/D;
    
    //Calculate ideal stability conditions 
    ideal_char_length = D*2./vg;//Peclet
    ideal_time_step_dif = 0.5*ideal_char_length*ideal_char_length/D;//Neumann
    ideal_time_step_vel = ideal_char_length/vg;//Courant  

	//Look at the maximum values possible
	max_time_step_dif = 0.5*elem->Element_Char_Length*elem->Element_Char_Length/D;//Neumann
    max_time_step_vel = elem->Element_Char_Length/vg;//Courant  
	elem->TimestepCourant = max_time_step_vel;//Save the time step in the courant number for the loop_pcs Checkcourant()
}


/**************************************************************************
GeoSys-FEM-Method: 
Task: 
Programing:
10/2004   
**************************************************************************/
int ELENoNodes(long index)
{
  int no_nodes=0;
  switch(ElGetElementType(index)){
    case 1: no_nodes = 2; break;
    case 2: no_nodes = 4; break;
    case 3: no_nodes = 8; break;
    case 4: no_nodes = 3; break;
    case 5: no_nodes = 4; break;
    case 6: no_nodes = 6; break;
  }
  return no_nodes;
}

/**************************************************************************
GeoSys-FEM-Method: 
Task: 
Programing:
10/2004   
**************************************************************************/
double ELEGetValue(long index, string pcs_pv_name,int timelevel)
{
  double ele_value = 0.;
  int nidx = PCSGetNODValueIndex(pcs_pv_name,timelevel);
  long* nodes = NULL;
  nodes = ElGetElementNodes(index);
  int no_nodes = ELENoNodes(index);
  int i;
  for(i=0;i<no_nodes;i++){
    ele_value += GetNodeVal(nodes[i],nidx);
  }
  ele_value /= (double)no_nodes;
  return ele_value;
}

#else
/* OK: bis Version rf 3.2.28 gespeichert */
#endif

