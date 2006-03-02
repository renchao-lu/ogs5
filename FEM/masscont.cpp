/**************************************************************************/
/* ROCKFLOW - Modul: masscont.c
                                                                          */
/* Aufgabe:
   Massenkontinuitaet sicherstellen, gehoert direkt zu refine.c
                                                                          */
/* Programmaenderungen:
   08/1995     cb         Erste Version
   04.08.1998  R.Kaiser   Adaption fuer meherer Loesungsgroessen
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "adaptiv.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
//OK_IC #include "rfbc.h"

#define noTESTMASS

/* Interne (statische) Deklarationen */
void MassCont_fast ( long ele, int verf_index, double *zwischenspeicher );

void MassCont_1D_light ( long ele, int verf_index, double *zwischenspeicher );
void MassCont_3D2D1D_light ( long ele, int verf_index, double *zwischenspeicher );

/* Definitionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteMassCont
                                                                          */
/* Aufgabe:
   Massenerhaltung beim Vergroebern sicherstellen,
   Aufruf vor der Vergroeberung !
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
            */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   16.07.1999     R.Kaiser         Erste Version
                                                                          */
/**************************************************************************/
void ExecuteMassCont( long ele, int num_ref_quantity, double *zwischenspeicher )
{
  switch (AdaptGetCoarseCorrect(num_ref_quantity)) {

    case 1: /* schnelle Massenerhaltung */
      MassCont_fast(ele, num_ref_quantity, zwischenspeicher);
      break;

    case 2: /* gleichmaessige Massenerhaltung */
      if (max_dim == 0) /* maximal 1D-Elemente */
        MassCont_1D_light(ele, num_ref_quantity, zwischenspeicher);
      else
        MassCont_3D2D1D_light(ele, num_ref_quantity, zwischenspeicher);
      break;
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: MassCont_fast
                                                                          */
/* Aufgabe:
   Massenerhaltung beim Vergroebern sicherstellen (3D),
   Aufruf vor der Vergroeberung !
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: Elementnummer des zu vergroebernden Elements
   E int num_ref_quantity: Nummer der Adaptionsgroesse
   E double *zwischenspeicher


                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/1995     msr         Erste Version
   12/1995     cb          Aenderung (ohne Kanteninkremente)
   05/1996     cb          zweite Version
                                                                          */
/**************************************************************************/
void MassCont_fast ( long ele, int num_ref_quantity, double *zwischenspeicher )
{
  static long *kinder;
  static long *knoten;
  static Knoten *k[8]; /* max. 8 Knoten */
  static Knoten *kindknoten;
  static double inkrement,m;
  static int i,jj,typ,nn;
  static int phase;
  static int component;
  static int idx[2];

  idx[0] = AdaptGetRefNval0(num_ref_quantity);
  idx[1] = AdaptGetRefNval1(num_ref_quantity);

  typ = ElGetElementType(ele)-1;
  nn = ElNumberOfNodes[typ];
  knoten = ElGetElementNodes(ele);  /* Elementknotennummern holen */
  kinder = ElGetElementChilds(ele);  /* Elementkindernummern holen */
  for (i=0;i<nn;i++)  /* Eckknoten von ele holen */
      k[i] = GetNode(knoten[i]);

  for (jj=0;jj<2;jj++) {

  /* Mittelwert aus Eckknoten bestimmen */
  m=0.0;
  for (i=0;i<nn;i++)
    m += k[i]->nval[idx[jj]];
  m /= (double)nn;
  /* Wert des Mittelknotens */
  if (typ == 0) /* 1D: Knoten 1 von Kind 0 von ele */
    kindknoten = GetNode(ElGetElementNodes(kinder[0])[1]);
  else if (typ == 1) /* 2D: Knoten 2 von z.B. Kind 0 von ele */
    kindknoten = GetNode(ElGetElementNodes(kinder[0])[2]);
  else /* 3D: Knoten 6 von z.B. Kind 0 von ele */
    kindknoten = GetNode(ElGetElementNodes(kinder[0])[6]);
  /* Inkrement aufgrund Mittelknoten ausrechnen */
  inkrement = ( kindknoten->nval[idx[jj]] - m ) / (double)nn;
  /* Eckknoten inkrementieren */

  char primary_variable_name[256]; //OK_BC

  if (AdaptGetRefQuantity(num_ref_quantity) == 0) {
    phase = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;
    sprintf(primary_variable_name,"%s%d","PRESSURE",phase); //OK_BC 
    for (i=0;i<nn;i++)
//OK_IC      if (IsNodeBoundaryCondition(primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }

  if (AdaptGetRefQuantity(num_ref_quantity)== 1) {
    component = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;
    for (i=0;i<nn;i++)
      sprintf(primary_variable_name,"%s%d","BOUND_CONC",component);
//OK_IC      if (IsNodeBoundaryCondition (primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }

  if (AdaptGetRefQuantity(num_ref_quantity)== 2) {
    for (i=0;i<nn;i++)
      sprintf(primary_variable_name,"%s%d","TEMPERATURE",1);
//OK_IC      if (IsNodeBoundaryCondition (primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }


  if (AdaptGetRefQuantity(num_ref_quantity)== 3) {
    phase = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;
    for (i=0;i<nn;i++)
      sprintf(primary_variable_name,"%s%d","SATURATION_COND",phase); 
//OK_IC      if (IsNodeBoundaryCondition (primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }
  if (AdaptGetRefQuantity(num_ref_quantity)== 4) {
    component = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;
    for (i=0;i<nn;i++)
      sprintf(primary_variable_name,"%s%d","BOUND_SORB",component); 
//OK_IC      if (IsNodeBoundaryCondition (primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }
  if (AdaptGetRefQuantity(num_ref_quantity)== 5) {
    component = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;
    for (i=0;i<nn;i++)
      sprintf(primary_variable_name,"%s%d","BOUND_SOLU",component); 
//OK_IC      if (IsNodeBoundaryCondition (primary_variable_name,knoten[i]) == 0) {
        if (jj==0)
          zwischenspeicher[knoten[i]] += inkrement;
        if (jj==1)
          zwischenspeicher[2*knoten[i]] += inkrement;
//OK_IC      }
  }
  }

}



/**************************************************************************/
/* ROCKFLOW - Funktion: MassCont_1D_light
                                                                          */
/* Aufgabe:
   Massenerhaltung beim Vergroebern sicherstellen (1D),
   Aufruf vor der Vergroeberung !
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)


                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1995     cb         Erste Version
                                                                          */
/**************************************************************************/
void MassCont_1D_light ( long ele, int num_ref_quantity, double *zwischenspeicher )
{
zwischenspeicher = zwischenspeicher;
  /* Variablen */
  static int jj;
  static long *elekno; /* Elementknoten von ele */
  //  static long *elems;
  static long *kinder;
  static double vol1,vol2,conc;
  static int idx[2];
  static int phase_component;

  idx[0] = AdaptGetRefNval0(num_ref_quantity);
  idx[1] = AdaptGetRefNval1(num_ref_quantity);

  phase_component = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;

  /* Element-Daten */
  kinder = ElGetElementChilds(ele);
  elekno = ElGetElementNodes(ele);

  for (jj=0;jj<2;jj++) {

  conc = 0.;
  /* Massendifferenz */
  conc = GetNodeVal(ElGetElementNodes(kinder[0])[1],idx[jj]);
  conc -= 0.5 * (GetNodeVal(elekno[0],idx[jj]) + GetNodeVal(elekno[1],idx[jj]));
/*  vol1 = conc * ADAPGetElementVolume(kinder[0]); OK rf3230 */
  vol1 = conc * ElGetElementVolume(kinder[0]);

  vol2 = 0.0;
/* OK_BC
  for (i=0;i<2;i++)
    if (((AdaptGetRefQuantity(num_ref_quantity) == 0) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_FLUID_PHASE(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 1) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_TRACER_COMPONENT(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 3) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SATURATION(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 4) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SORBED_COMPONENT(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 5) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SOLUTE_COMPONENT(phase_component),elekno[i]) == 0))){ // keine RB
      elems = GetNode1DElems(elekno[i],&anz); // Elemente am Knoten
      for (j=0;j<anz;j++)
        if (kinder[0] == elems[j] || kinder[1] == elems[j])
//          vol2 += ADAPGetElementVolume(elems[j]); OK rf3230
          vol2 += ElGetElementVolume(elems[j]);

        else
//         vol2 += 0.5 * ADAPGetElementVolume(elems[j]); OK rf3230
          vol2 += 0.5 * ElGetElementVolume(elems[j]);
*/    

  /* Konzentrationsinkremente */
  if (vol2 > MKleinsteZahl)
    conc = vol1 / vol2;
  else
    conc = 0.0;

/* OK_BC
  for (i=0;i<2;i++)
    if (((AdaptGetRefQuantity(num_ref_quantity) == 0) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_FLUID_PHASE(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 1) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_TRACER_COMPONENT(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 3) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SATURATION(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 4) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SORBED_COMPONENT(phase_component),elekno[i]) == 0)) ||
        ((AdaptGetRefQuantity(num_ref_quantity) == 5) &&
         (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SOLUTE_COMPONENT(phase_component),elekno[i]) == 0))){ // keine RB

#ifdef TESTMASS
      DisplayMsg("Inkrement["); DisplayLong(elekno[i]); DisplayMsg("] = ");
      DisplayDouble(conc,0,0); DisplayMsgLn("");
#endif
      // Konzentrationsinkrement in Zwischenspeicher eintragen

      if (jj==0)
        zwischenspeicher[elekno[i]] += conc;
      if (jj==1)
        zwischenspeicher[2*elekno[i]] += conc;
    }
OK */
  }
}




/**************************************************************************/
/* ROCKFLOW - Funktion: MassCont_3D2D1D_light
                                                                          */
/* Aufgabe:
   Massenerhaltung beim Vergroebern sicherstellen,
   Aufruf vor der Vergroeberung !
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)


                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1996     cb         Erste Version
                                                                          */
/**************************************************************************/
void MassCont_3D2D1D_light ( long ele, int num_ref_quantity, double *zwischenspeicher )
{
  /* Strukturen */
  typedef struct {           /* lokale Element-Datenstruktur */
    long nummer;             /* globale Element-Nummer */
    long *knoten;            /* lokale Knoten-Nummern des Elements */
  } LokElement;

  typedef struct {           /* lokale Knoten-Datenstruktur */
    long nummer;             /* globale Knoten-Nummer */
    int kennung;             /* Kennung des Knotens */
    long *nachbar;           /* Nachbarn des ggf. irreg. Knotens */
  } LokKnoten;

  /* Variablen */
  static LokElement *lokele;
  static LokKnoten *lokkno;
  static double *mass;
  static long *kinder;
  static long dim,mittelknoten,patchele;
  static long *elekno;
  static int typ,anz,zahl,nn,nc,level;
  static long *pele;
  static long i,j,k,l,m,jj;
  static LokKnoten *lok;
  static long anzele,anzkno;
  static double *glslinks;
  static double *glsrechts;
  static double *zwi;
  static double fakl,sum1,sum2,sum3;
#ifdef TESTMASS
  static double zwa[8],zwo[8];
#endif
  static int idx[2];
  static int phase_component;

  idx[0] = AdaptGetRefNval0(num_ref_quantity);
  idx[1] = AdaptGetRefNval1(num_ref_quantity);
  phase_component = AdaptGetNumPhaseComponent(num_ref_quantity) + 1;


  /* Patch um das Element ele */
  typ = ElGetElementType(ele)-1;
  nn = ElNumberOfNodes[typ];
  nc = ElNumberOfChilds[typ];
  level = ElGetElementLevel(ele);

  /* lokales Elementfeld */
  anzele = 0l;
  dim = nc+1;
  lokele = (LokElement *) Malloc(dim*sizeof(LokElement));
  for (i=0;i<dim;i++)
    lokele[i].knoten = (long *) Malloc(nn*sizeof(long));

  /* Patchelemente eintragen */
  /* zuerst Vorgaengerelement */
  lokele[0].nummer = ele;
  ADAPMakeMat(lokele[0].nummer); /* u.U. Aenderung der Massen-Matrix */
  /* dann Elementkinder */
  kinder = ElGetElementChilds(ele);
  for (i=0;i<nc;i++)
    lokele[i+1].nummer = kinder[i];
  anzele = nc+1;
  /* jetzt alle anderen Patchelemente */
  elekno = ElGetElementNodes(ele);
  for (i=0;i<nn;i++) {
    pele = GetNodeXDElems[typ](elekno[i],&anz);
    for (j=0;j<anz;j++) {
      /* nur Elemente mit Level <= level(ele) oder */
      /* evt. Inkonsistenz des Netzes durch vorherige Vergroeberung */
      if (ElGetElementLevel(pele[j])>level || \
          ElGetElementRefineState(pele[j])==4) {
        patchele = ElGetElementPred(pele[j]);
          ADAPMakeMat(patchele); /* u.U. Aenderung der Massen-Matrix */
        /* printf("1.Vorgaenger von %ld bei Status 4=%ld\n",pele[j],patchele); */
      }
      else
        patchele = pele[j];
      for (k=0;k<anzele;k++)
        if (patchele == lokele[k].nummer)
          break;
      if (k == anzele) {
        if (anzele == dim) {
          lokele = (LokElement *)Realloc(lokele,(++dim*sizeof(LokElement)));
          lokele[anzele].knoten = (long *) Malloc(nn*sizeof(long));
        }
        lokele[anzele++].nummer = patchele; /* Patchelemente von ele */
      }
    }
  }

  /* lokales Knotenfeld */
  anzkno = 0l;
  dim = (long)pow(3.0,(double)(typ+1));
  lokkno = (LokKnoten *) Malloc(dim*sizeof(LokKnoten));

  /* Patchknoten eintragen */
  for(i=0;i<anzele;i++) {
    elekno = ElGetElementNodes(lokele[i].nummer); /* Knoten am Patchelement */
    for (j=0;j<nn;j++) {
      for (k=0;k<anzkno;k++)
        if (elekno[j] == lokkno[k].nummer)
          break;
      if (k == anzkno) {
        if (anzkno == dim)
          lokkno = (LokKnoten *)Realloc(lokkno,(++dim*sizeof(LokKnoten)));
        lokkno[anzkno++].nummer = elekno[j];
      }
      lokele[i].knoten[j] = k; /* lokale Knotennummer */
    }
  }

  /* Kennungen 0: freier Knoten
               1: Randbed.-Knoten
               2: zu eliminierender Kantenknoten (links)
              -2:                                (rechts)
               3: zu eliminierender Flaechenknoten (links)
              -3:                                  (rechts) (Mittelknoten 2D)
              -4: zu eliminierender Volumenknoten (rechts) (Mittelknoten 3D) */
  for (i=0;i<anzkno;i++)
    lokkno[i].kennung = 1; /* Randbed.-Knoten */
  for (i=1;i<(nc+1);i++) /* Kindelemente */
    for (j=0;j<nn;j++) { /* Knoten am Kindelement */
      lok = &lokkno[lokele[i].knoten[j]];
/* OK_BC
      if (((AdaptGetRefQuantity(num_ref_quantity) == 0) &&
           (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_FLUID_PHASE(phase_component),lok->nummer) == 0)) ||
          ((AdaptGetRefQuantity(num_ref_quantity) == 1) &&
           (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_TRACER_COMPONENT(phase_component),lok->nummer) == 0)) ||
         ((AdaptGetRefQuantity(num_ref_quantity) == 3) &&
           (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SATURATION(phase_component),lok->nummer) == 0)) ||
         ((AdaptGetRefQuantity(num_ref_quantity) == 4) &&
           (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SORBED_COMPONENT(phase_component),lok->nummer) == 0)) ||
         ((AdaptGetRefQuantity(num_ref_quantity) == 5) &&
           (IsNodeBoundaryCondition (DEFAULT_NAME_BOUNDARY_CONDITION_SOLUTE_COMPONENT(phase_component),lok->nummer) == 0))) OK */ { // keine RB

        /* 1D: nicht konsistente Nummerierung der Kindelemente */
        if ((max_dim < 2) && (typ == 0) && (i == nc)) {
            if (j == 0) {
              lok->kennung = -2; /* Mittelknoten */
            }
            else {
              lok->kennung = 0; /* freier Knoten */
            }
        }
        else {
          if (j == 0) {
            lok->kennung = 0; /* freier Knoten */
          }
          else if (j == 1 || j == 3 || j == 4) {
            if (GetNodeState(lok->nummer) == -1) /* regulaer wird ... */
              lok->kennung = +2; /* ... irreg. Kantenknoten */
            else
              lok->kennung = -2;
          }
          else if (j == 2 || j == 5 || j == 7) {
            if ((typ == 2) && (GetNodeState(lok->nummer) == -1)) /* regulaer wird ... */
              lok->kennung = +3; /* ... irreg. Flaechenknoten */
            else
              lok->kennung = -3;
          }
          else if (j == 6) {
            lok->kennung = -4; /* irreg. Volumenknoten (nur 3D) */
          }
        }
      }
    }
  /* Kontrolle aller Knoten an Randelementen */
  for (i=(nc+1);i<anzele;i++) /* Randelemente */
    for (j=0;j<nn;j++) { /* Knoten am Randelement */
      lok = &lokkno[lokele[i].knoten[j]];
      if (lok->kennung < 0) /* lokale irr.Kno. */
        lok->kennung = -lok->kennung;
    }
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung == 1) {
      if (GetNodeState(lokkno[i].nummer) == -2)  /* irreg. Knoten */
        lokkno[i].kennung = +2; /* irreg. Kantenknoten */
      if (GetNodeState(lokkno[i].nummer) == -4) /* nur 3D */
        lokkno[i].kennung = +3; /* irreg. Flaechenknoten */
    }
  /* Nachbarn zu den irregulaeren Knoten */
  for (i=0;i<anzkno;i++)
    if (abs(lokkno[i].kennung) > 1) {
      if (abs(lokkno[i].kennung) == 2) /* irreg. Kantenknoten */
        zahl = 2;
      else if (abs(lokkno[i].kennung) == 3) /* irreg. Flaechenknoten */
        zahl = 4;
      else if (lokkno[i].kennung == -4) /* irreg. Volumenknoten */
        zahl = 8;
      elekno = GetNodeXDElems[typ](lokkno[i].nummer,&anz);
      lokkno[i].nachbar = (long *) Malloc(zahl*sizeof(long));
      for (j=0;j<zahl;j++) {
        lokkno[i].nachbar[j] = -1;
        l = ElGetElementNodes(elekno[j])[0];
        if (max_dim<2 && typ==0 && l==lokkno[i].nummer)
          l = ElGetElementNodes(elekno[j])[1];
        for (k=0;k<anzkno;k++)
          if (lokkno[k].nummer == l) {
            lokkno[i].nachbar[j] = k; /* lokale Nachbar-Nummer */
            break;
          }
        if (lokkno[i].nachbar[j] == -1) { /* irreg. Knoten auf Ecke */
          /* printf("irreg. Knoten auf Ecke=%ld\n",lokkno[i].nummer); */
          lokkno[i].kennung = 1;
          lokkno[i].nachbar = (long *)Free(lokkno[i].nachbar);
          break;
        }
      }
    }

  /* Ueberpruefung der irreg. Nachbarknoten */
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung == +2 || lokkno[i].kennung == +3) {
      /* anhaengende Elemente evt. eintragen */
      pele = GetNodeXDElems[typ](lokkno[i].nummer,&anz);
      for (j=0;j<anz;j++) {
        if (ElGetElementLevel(pele[j])>level || \
            ElGetElementRefineState(pele[j])==4) {
          patchele = ElGetElementPred(pele[j]);
          ADAPMakeMat(patchele);
      /* printf("2.Vorgaenger von %ld bei Status 4=%ld\n",pele[j],patchele); */
        }
        else
          patchele = pele[j];
        for (k=0;k<anzele;k++)
          if (patchele == lokele[k].nummer)
            break;
        if (k == anzele) {
          lokele = (LokElement *)Realloc(lokele,(++anzele*sizeof(LokElement)));
          lokele[k].knoten = (long *) Malloc(nn*sizeof(long));
           /* Patchelemente von ele */
          lokele[k].nummer = patchele;
          /* Knoten eintragen */
          elekno = ElGetElementNodes(lokele[k].nummer);
          for (l=0;l<nn;l++) {
            for (m=0;m<anzkno;m++)
              if (elekno[l] == lokkno[m].nummer)
                break;
            if (m == anzkno) {
              lokkno = (LokKnoten *)Realloc(lokkno,(++anzkno*sizeof(LokKnoten)));
              lokkno[m].nummer = elekno[l];
              lokkno[m].kennung = 1;
            }
            lokele[k].knoten[l] = m; /* lokale Knotennummer */
          }
        }
      }
    }

  for (jj=0;jj<2;jj++) {
  /* Speicheranforderung fuer glslinks, glsrechts und zwi */
  glslinks = (double *) Malloc(sizeof(double)*anzkno*anzkno);
  glsrechts = (double *) Malloc(sizeof(double)*anzkno);
  zwi = (double *) Malloc(sizeof(double)*anzkno);

  /* Initialisierung */
  MNulleMat(glslinks,anzkno,anzkno);

  /* feines System: Einspeichern in links */
  for (i=1;i<anzele;i++) { /* Schleife ueber alle Elemente (ohne Vorgaenger) */
    if (AdaptGetRefQuantity(num_ref_quantity) == 0)
      mass = ADAPGetElementCapacitanceMatrix(lokele[i].nummer);
    if (AdaptGetRefQuantity(num_ref_quantity) == 1)
      mass = ADAPGetElementMassMatrix(lokele[i].nummer);
    for (j=0;j<nn;j++)
      for (k=0;k<nn;k++) /* zeilenweise */
        glslinks[lokele[i].knoten[j]*anzkno+lokele[i].knoten[k]] += mass[j*nn+k];
  }
  /* rechte Seite des Gls. */
  MNulleVec(zwi,anzkno);
  for (i=0;i<anzkno;i++)
    zwi[i] = GetNodeVal(lokkno[i].nummer,idx[jj]); /* Erg. aus letztem ZS */
  MMultMatVec(glslinks,anzkno,anzkno,zwi,anzkno,glsrechts,anzkno);

  /* Massenkontrolle bzw. Gesamtmasse feines System */
  sum1 = 0.0;
  for (i=0;i<anzkno;i++)
    sum1 += glsrechts[i];

  /* grobes System: Einspeichern des Vorgaengers in links */
  if (AdaptGetRefQuantity(num_ref_quantity) == 0)
    mass = ADAPGetElementCapacitanceMatrix(lokele[0].nummer);
  if (AdaptGetRefQuantity(num_ref_quantity) == 1)
    mass = ADAPGetElementMassMatrix(lokele[0].nummer);
  for (j=0;j<nn;j++)
    for (k=0;k<nn;k++) /* zeilenweise */
      glslinks[lokele[0].knoten[j]*anzkno+lokele[0].knoten[k]] += mass[j*nn+k];
  /* Loeschen der Kind-Elemente */
  for (i=1;i<(nc+1);i++) {  /* Schleife ueber die Kind-Elemente */
    if (AdaptGetRefQuantity(num_ref_quantity) == 0)
      mass = ADAPGetElementCapacitanceMatrix(lokele[i].nummer);
    if (AdaptGetRefQuantity(num_ref_quantity) == 1)
      mass = ADAPGetElementMassMatrix(lokele[i].nummer);
    /* linke Seite des Gls. */
    for (j=0;j<nn;j++)
      for (k=0;k<nn;k++) /* zeilenweise */
        glslinks[lokele[i].knoten[j]*anzkno+lokele[i].knoten[k]] -= mass[j*nn+k];
  }
  /* rechte Seite */
  MNulleVec(zwi,anzkno);
  for (i=0;i<anzkno;i++)
    if (abs(lokkno[i].kennung) < 2)
      zwi[i] = GetNodeVal(lokkno[i].nummer,idx[jj]); /* Erg. aus letztem ZS */
  /* irreg. Knoten berechnen */
  for (i=0;i<anzkno;i++) {
    zahl = 0;
    if (lokkno[i].kennung == +2) {
      fakl = 0.5;
      zahl = 2;
    }
    else if (lokkno[i].kennung == +3) {
      fakl = 0.25;
      zahl = 4;
    }
    /* Ergebnisse der Nachbarknoten */
    for (j=0;j<zahl;j++)
      zwi[i] += fakl * zwi[lokkno[i].nachbar[j]];
  }
  MMultMatVec(glslinks,anzkno,anzkno,zwi,anzkno,glsrechts,anzkno);

  /* Gesamtmasse grobes System */
  sum2 = 0.0;
  for (i=0;i<anzkno;i++)
    sum2 += glsrechts[i];

  /* Differenz = Verlust bzw. Gewinn ... */
  sum3 = sum1 - sum2;

  /* ... im groben System ausgleichen; dazu zuerst ... */
  /* ... irregulaere Knoten eliminieren und dann ... */
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung > 1) {
      zahl = 0;
      if (lokkno[i].kennung == +2) {
        /* irreg. Kantenknoten (Kennung +2) links eliminieren */
        fakl = 0.5;
        zahl = 2;
      }
      else if (lokkno[i].kennung == +3) {
        /* irreg. Flaechenknoten (Kennung +3) links eliminieren */
        fakl = 0.25;
        zahl = 4;
      }
      /* i. Zeile teilen und auf die Nachbarzeilen addieren */
      for (j=0;j<zahl;j++)
        for (k=0;k<anzkno;k++)
          glslinks[lokkno[i].nachbar[j]*anzkno+k] += fakl * glslinks[i*anzkno+k];
      /* i. Spalte teilen und auf die Nachbarspalten addieren */
      for (j=0;j<zahl;j++)
        for (k=0;k<anzkno;k++)
          glslinks[k*anzkno+lokkno[i].nachbar[j]] += fakl * glslinks[k*anzkno+i];
      /* Zeile und Spalte streichen */
      for (j=0;j<anzkno;j++) {
        glslinks[i*anzkno+j] = 0.0;
        glslinks[j*anzkno+i] = 0.0;
      }
    }

  /* ... Randbedingungen ... */
  /* ... auf Mittelknoten (Massenerhaltung) addieren und den dann ... */
  if (typ == 0)
    mittelknoten = lokele[1].knoten[1];
  else if (typ == 1)
    mittelknoten = lokele[1].knoten[2];
  else if (typ == 2)
    mittelknoten = lokele[1].knoten[6];
  zahl = 0;
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung == 1) {
      /* Zeile auf Mittelknoten addieren und streichen */
      for (j=0;j<anzkno;j++) {
        glslinks[mittelknoten*anzkno+j] += glslinks[i*anzkno+j];
        glslinks[i*anzkno+j] = 0.0;
      }
      /* Spalte streichen */
      for (j=0;j<anzkno;j++)
        glslinks[j*anzkno+i] = 0.0;
    }
    else if (lokkno[i].kennung == 0)
      /* Anzahl freier Knoten ermitteln */
      zahl++;
  /* ... (noch mal) eliminieren */
  /* Zeile teilen und auf die (reg.) Nachbarzeilen addieren */
  for (j=0;j<nn;j++) /* Schleife ueber Anzahl Nachbarn */
    if (lokkno[lokkno[mittelknoten].nachbar[j]].kennung == 0)
      for (k=0;k<anzkno;k++)
        glslinks[lokkno[mittelknoten].nachbar[j]*anzkno+k] += glslinks[mittelknoten*anzkno+k] / (double)zahl;
  /* Spalte teilen und auf die Nachbarspalten addieren */
  for (j=0;j<nn;j++)
    if (lokkno[lokkno[mittelknoten].nachbar[j]].kennung == 0)
      for (k=0;k<anzkno;k++)
        glslinks[k*anzkno+lokkno[mittelknoten].nachbar[j]] += glslinks[k*anzkno+mittelknoten] / (double)zahl;
  /* Zeile und Spalte streichen */
  for (j=0;j<anzkno;j++) {
    glslinks[mittelknoten*anzkno+j] = 0.0;
    glslinks[j*anzkno+mittelknoten] = 0.0;
  }

  /* Matrix-Summe nur der freien Knoten */
  sum2 = 0.0;
  for (i=0;i<(anzkno*anzkno);i++)
    sum2 += glslinks[i];

  /* Inkrement */
  if (fabs(sum2) > MKleinsteZahl)
    sum3 /= sum2;
  else
    sum3 = 0.0;

#ifdef TESTMASS
  MNulleVec(zwi,anzkno);
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung == 0) /* freie Knoten */
      zwi[i] = GetNodeVal(lokkno[i].nummer,idx[jj]) + sum3;
    else if (lokkno[i].kennung == 1) /* RB-Knoten */
      zwi[i] = GetNodeVal(lokkno[i].nummer,idx[jj]);

  /* Knoten im Nachlauf berechnen */
  for (i=0;i<anzkno;i++) {
    zahl = 0;
    if (lokkno[i].kennung == +2) {
      fakl = 0.5;
      zahl = 2;
    }
    else if (lokkno[i].kennung == +3) {
      fakl = 0.25;
      zahl = 4;
    }
    /* Ergebnisse der Nachbarknoten */
    for (j=0;j<zahl;j++)
      zwi[i] += fakl * zwi[lokkno[i].nachbar[j]];
  }

  /* Massenkontrolle */
  sum2 = 0.0;
  /* Vorgaenger */
  if (AdaptGetRefQuantity(num_ref_quantity) == 0)
    mass = ADAPGetElementCapacitanceMatrix(lokele[0].nummer);
  if (AdaptGetRefQuantity(num_ref_quantity) == 1)
    mass = ADAPGetElementMassMatrix(lokele[0].nummer);
  for (j=0;j<nn;j++)
    zwo[j] = zwi[lokele[0].knoten[j]];
  MMultMatVec(mass,nn,nn,zwo,nn,zwa,nn);
  for (j=0;j<nn;j++)
    sum2 += zwa[j];
  /* alle anderen */
  for (i=(nc+1);i<anzele;i++) {
    if (AdaptGetRefQuantity(num_ref_quantity) == 0)
      mass = ADAPGetElementCapacitanceMatrix(lokele[i].nummer);
    if (AdaptGetRefQuantity(num_ref_quantity) == 1)
      mass = ADAPGetElementMassMatrix(lokele[i].nummer);
    for (j=0;j<nn;j++)
      zwo[j] = zwi[lokele[i].knoten[j]];
    MMultMatVec(mass,nn,nn,zwo,nn,zwa,nn);
    for (j=0;j<nn;j++)
      sum2 += zwa[j];
  }
  if (fabs(sum1-sum2)>1.e-8) {
    DisplayMsg("Kontrolle: "); DisplayDouble(sum1,0,0);
    DisplayMsg(" - "); DisplayDouble(sum2,0,0);DisplayMsg(" = ");
    DisplayDouble((sum1-sum2),0,0); DisplayMsgLn("");
  }
#endif

  /* Inkremente eintragen */
  /* freie Knoten (Kennung 0) */
  for (i=0;i<anzkno;i++)
    if (lokkno[i].kennung == 0) {
#ifdef TESTMASS
      DisplayLong(i); DisplayMsg(": Inkrement["); DisplayLong(lokkno[i].nummer);
      DisplayMsg("] = "); DisplayDouble(sum3,0,0); DisplayMsgLn("");
#endif
      /* Inkrement in Zwischenspeicher */
      if (jj==0)
        zwischenspeicher[lokkno[i].nummer] += sum3;
      if (jj==1)
        zwischenspeicher[2*lokkno[i].nummer] += sum3;


    }

  /* Speicher wieder freigeben */
  for (i=0;i<anzele;i++)
    lokele[i].knoten = (long *)Free(lokele[i].knoten);
  lokele = (LokElement *)Free(lokele);
  for (i=0;i<anzkno;i++)
    if (abs(lokkno[i].kennung) > 1) /* irreg. Knoten */
      lokkno[i].nachbar = (long *)Free(lokkno[i].nachbar);
  lokkno = (LokKnoten *)Free(lokkno);
  glslinks = (double *)Free (glslinks);
  glsrechts = (double *)Free (glsrechts);
  zwi = (double *)Free (zwi);
  }
}

