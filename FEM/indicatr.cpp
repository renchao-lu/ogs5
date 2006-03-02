/**************************************************************************/
/* ROCKFLOW - Modul: indicatr.c
                                                                          */
/* Aufgabe:
   Steuerfuntion fuer die Fehlerindikatoren.
   Fuer Stroemung und Transport stehen unterschiedliche Steuerfunktionen
   zur Verfuegung.
                                                                          */
/* Programmaenderungen:
   07/1997     R.Kaiser        Modul angelegt
   04.08.1998  R.Kaiser        Adaption fuer mehrere Loesungsgroessen
   03/2000     R.Kaiser        Adaption "Temperatur"
   07/2000     R.Kaiser        Ueberarbeitet
   08/2000     R.Kaiser        Verschiedene max. Verfeinerungstiefen
                               fuer die einzelnen Loesungsgroessen,
                               Zeitkurven fuer die Indikatoren
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTINDI

/* Header / Andere intern benutzte Module */
#include "adaptiv.h"
#include "elements.h"
#include "nodes.h"
#include "edges.h"
#include "plains.h"
#include "mathlib.h"
#include "tools.h"
#include "rf_tim_new.h"

/* Interne (statische) Deklarationen */


/* Definitionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator
                                                                          */
/* Aufgabe:
   Steuerfunktion fuer die Fehlerindikatoren

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   int fein: Iterationszaehler
                                                                          */
/* Ergebnis:
   Anzahl der zu veraendernden Elemente

                                                                          */
/* Programmaenderungen:
   07/1998   R.Kaiser  Erste Version
                                                                          */
/**************************************************************************/
long Indicator (int iteration)
{
  static int j;
  static long number_ref;
  static int num_ref_quantity;
  static int counter_indicator;

  counter_indicator = 0;
  number_ref = 0;

  for (num_ref_quantity=0;num_ref_quantity<AdaptGetNumAdaptQuantities();num_ref_quantity++)  {

    if (num_ref_quantity == 0)
      InitMemoryIndicator(iteration);

    if (AdaptGetNumberInd(num_ref_quantity) > 0) {
      for (j=0;j<AdaptGetNumberInd(num_ref_quantity);j++) {
        number_ref += Indicatr(iteration,num_ref_quantity,j,counter_indicator);
        counter_indicator++;

      }
    }

  }

  return number_ref;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicatr
                                                                          */
/* Aufgabe:
   Steuerfunktion fuer die Fehlerindikatoren aus indicat1 (heuristisch)
   und indicat2 (analytisch) (Transport)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   int iteration: Iterationszaehler
                                                                          */
/* Ergebnis:
   Anzahl der zu veraendernden Elemente
                                                                          */
/* Programmaenderungen:
   03/1996      cb         Erste Version (aTM)
   09/1996      cb         Fehler-Summen (aTM)
   01.07.1997   R.Kaiser   Rockflow-Anpassung (aTM -> ROCKFLOW)
                                                                          */
/**************************************************************************/
long Indicatr (int iteration, int num_ref_quantity, int num_indicator, int counter_indicator)
{
  static int grid_adaptation,j,typ,AD;
  static long i;
  static long active_element;
  static double error, average_error;
  static double sum_error[3][2];
  static double max_error[3][2];
  static double min_error[3][2];
  static double scale_value[3][2];
  static long num_elements_typ_AD[3][2];
  static int refine_state;
  static long anz_verf;   /* Ergebnis: Anzahl der zu verfeinernden Elemente */
  static long anz_fein,anz_grob;
  static long vorgaenger;
  static int transport;
  static int active_indicator;
  static int valid_refine;
  static int valid_coarse;
  static int interp_method=0;
  static long curve_refine, curve_coarse;
  static double refine_param, coarse_param;

  iteration=iteration;

  grid_adaptation = 1; /* Gitteradaption */
  anz_fein = anz_grob = 0l;
  transport = 0;
  error = -1;
  valid_refine = 1;
  valid_coarse = 1;

  if ((AdaptGetRefQuantity(num_ref_quantity) != 0) &&
      (AdaptGetRefQuantity(num_ref_quantity) != 3))
    transport = 1;


  for (i=0;i<3;i++)
    for (j=0;j<2;j++) {
      sum_error[i][j] = 0.0;
      max_error[i][j] = 0.0;
      min_error[i][j] = 1.e+10;
      scale_value[i][j] = 0.0;
      num_elements_typ_AD[i][j] = 0l;

    }

  for (i=0;i<anz_active_elements;i++) {  /* Schleife ueber alle aktiven Elemente */
    active_element = ActiveElements[i];
    typ = ElGetElementType(active_element) - 1; /* Elementtyp */

    if ((transport) && (AdaptGetMethodInd(num_ref_quantity) == 1)) {
      if (ADAPGetElementPecletNum(active_element) > 1.99)
        AD = 1;  /* Advektion */
      else
        AD = 0;  /* Diffusion */
    }
    else AD = 0;

    if (AdaptGetRefInd(num_ref_quantity,num_indicator,typ,AD) != 0) {
      error = AdaptGetIndValue(typ,AD,active_element,num_ref_quantity,num_indicator);

      sum_error[typ][AD] += error;
      max_error[typ][AD] = max(max_error[typ][AD],error);
      min_error[typ][AD] = min(min_error[typ][AD],error);
      ++num_elements_typ_AD[typ][AD];
    }

    ADAPSetElementDDummy(active_element,error);
  }

  if ((sum_error[0][0] <= 0.0) && (sum_error[0][1] <= 0.0) && (sum_error[1][0] <= 0.0) &&
      (sum_error[1][1] <= 0.0) && (sum_error[2][0] <= 0.0) && (sum_error[2][1] <= 0.0))
     grid_adaptation = 0; /* keine Gitteradaption */


  if (grid_adaptation) {

    for (i=0;i<3;i++)
      for (j=0;j<2;j++) {
        if (num_elements_typ_AD[i][j]>0l) {
          average_error = sum_error[i][j]/(double)num_elements_typ_AD[i][j];


          if (AdaptGetOutputAdaptation() == 0) {
            if (aktueller_zeitschritt==1l) {
              if ((transport) && (AdaptGetMethodInd(num_ref_quantity) == 1)) {
                if (j==0) DisplayMsg("Diffusion:");
                else DisplayMsg("Advektion:");
                DisplayMsgLn("");
              }

              DisplayMsg(" Maximalwert="); DisplayDouble(max_error[i][j],0,0);
              DisplayMsg(" Mittelwert="); DisplayDouble(average_error,0,0);
              DisplayMsgLn("");
            }
          }

          if ((AdaptGetOutputAdaptation() == 1) || (AdaptGetOutputAdaptation() == 2)) {

            DisplayMsgLn("");


            if (AdaptGetRefQuantity(num_ref_quantity) == 0) DisplayMsg("PRESSURE_");
            if (AdaptGetRefQuantity(num_ref_quantity) == 1) DisplayMsg("COMPONENT_");
            if (AdaptGetRefQuantity(num_ref_quantity) == 2) DisplayMsg("TEMPERATURE_");
            if (AdaptGetRefQuantity(num_ref_quantity) == 3) DisplayMsg("SATURATION_");

            if (i==0) DisplayMsg("1D");
            if (i==1) DisplayMsg("2D");
            if (i==2) DisplayMsg("3D");

            DisplayMsg("  ("); DisplayLong(AdaptGetNumPhaseComponent(num_ref_quantity)+1);
            DisplayMsg(")");

            DisplayMsgLn("");
            if ((transport) && (AdaptGetMethodInd(num_ref_quantity) == 1)) {
              if (j==0) DisplayMsg("Diffusion:");
              else DisplayMsg("Advektion:");
              DisplayMsgLn("");
            }


            if (AdaptGetMethodInd(num_ref_quantity) == 3) {  /* Zeitkurven */
              curve_refine = AdaptGetCurveInd(num_ref_quantity,num_indicator,i,j,0);
              GetCurveValue(curve_refine,interp_method,aktuelle_zeit,&valid_refine);

              curve_coarse = AdaptGetCurveInd(num_ref_quantity,num_indicator,i,j,1);
              GetCurveValue(curve_coarse,interp_method,aktuelle_zeit,&valid_coarse);

            }


            if (AdaptGetRefInd(num_ref_quantity,num_indicator,i,j) == 1)
              DisplayMsg(" Differenzen-Indikator");
            if (AdaptGetRefInd(num_ref_quantity,num_indicator,i,j) == 2)
              DisplayMsg(" Gradienten-Indikator");
            if (AdaptGetRefInd(num_ref_quantity,num_indicator,i,j) == 3)
              DisplayMsg(" Sprung-Indikator");
            DisplayMsgLn("");

            if (!valid_refine && valid_coarse) {
              DisplayMsg(" Verfeinerung deaktiviert !!");
              DisplayMsgLn("");
            }

            if (!valid_coarse && valid_refine) {
              DisplayMsg(" Vergroeberung deaktiviert !!");
              DisplayMsgLn("");
            }

            if (!valid_refine && !valid_coarse) {
              DisplayMsg(" Indikator deaktiviert !!");
              DisplayMsgLn("");
            }


            if (valid_refine || valid_coarse) {
              DisplayMsg(" Maximalwert="); DisplayDouble(max_error[i][j],0,0);
              DisplayMsg(" Minimalwert="); DisplayDouble(min_error[i][j],0,0);
              DisplayMsgLn("");
              DisplayMsg(" Mittelwert="); DisplayDouble(average_error,0,0);
              DisplayMsgLn("");
            }

            DisplayMsgLn("");


          }


          switch (AdaptGetRefTol(num_ref_quantity,num_indicator)) {

            case 0:  /* Absolut-Wert */
              scale_value[i][j] = 1.0;
              break;

            case 1:  /* Skalierung am Mittelwert */
              scale_value[i][j] = max(MKleinsteZahl,average_error);
              break;

            case 2:  /* Skalierung am Maximalwert */
              scale_value[i][j] = max(MKleinsteZahl,max_error[i][j]);
              break;

          }
        }

      }


    /* Markieren der Elemente ... */
    for (i=0;i<anz_active_elements;i++) {  /* Schleife ueber alle aktiven Elemente */
      active_element = ActiveElements[i];
      typ = ElGetElementType(active_element) - 1;   /* Elementtyp */

      if (transport && (AdaptGetMethodInd(num_ref_quantity) == 1)){
        if (ADAPGetElementPecletNum(active_element) > 1.99)
          AD = 1;  /* Advektion */
        else
          AD = 0;  /* Diffusion */
      }
      else AD = 0;

      active_indicator = 1;
      refine_state = 0;

      /* ... infolge kuenstlicher Diffusion, Johnson-Indikator */
      if (transport && (AdaptGetRefInd(num_ref_quantity,num_indicator,typ,AD) == 103) &&
          (ElGetArtDiff(active_element) > MKleinsteZahl)) {
        if ((ElGetElementLevel(active_element)+1) >= AdaptGetMaxRefLevel()) {
          /* kuenstliche Diffusion zuruecksetzen */
          ElSetArtDiff(active_element,0.0);
        }

        refine_state = 1;  /* muss verfeinert werden ! */
        ElSetVerf(active_element,refine_state);  /* wird verfeinert */

        ElSetGlobalVerf(active_element,counter_indicator,1);
        ElSetVerfCount(active_element,ElGetVerfCount(active_element)+1);
      }
      /* ... infolge vorgegebener Schranken */
      else {
        /* OK error = ADAPGetElementDDummy(active_element) / scale_value[typ][AD]; */
        error = ADAPGetElementDDummy(active_element) / (scale_value[typ][AD]+MKleinsteZahl);


        if (AdaptGetMethodInd(num_ref_quantity) == 3) {  /* Zeitkurven */
          refine_param = AdaptGetRefParam(num_ref_quantity,num_indicator,typ,AD,0);
          curve_refine = AdaptGetCurveInd(num_ref_quantity,num_indicator,typ,AD,0);
          refine_param = GetCurveValue(curve_refine,interp_method,aktuelle_zeit,&valid_refine) * refine_param;

          coarse_param = AdaptGetRefParam(num_ref_quantity,num_indicator,typ,AD,1);
          curve_coarse = AdaptGetCurveInd(num_ref_quantity,num_indicator,typ,AD,1);
          coarse_param = GetCurveValue(curve_coarse,interp_method,aktuelle_zeit,&valid_coarse) * coarse_param;

        }
        else {
          refine_param = AdaptGetRefParam(num_ref_quantity,num_indicator,typ,AD,0);
          coarse_param = AdaptGetRefParam(num_ref_quantity,num_indicator,typ,AD,1);
        }


        if ((error >= refine_param) && (valid_refine == 1)) {
          if (AdaptGetMethodInd(num_ref_quantity) == 3) {  /* unterschiedliche Verfeinerungstiefen */
            if (ElGetElementLevel(active_element) < AdaptGetQuantityMaxRefLevel(num_ref_quantity,typ)) {
              refine_state = 1;  /* muss verfeinert werden ! */
              ElSetVerf(active_element,refine_state);  /* wird verfeinert */

              ElSetGlobalVerf(active_element,counter_indicator,1);
              ElSetVerfCount(active_element,ElGetVerfCount(active_element)+1);

            }
          }
          else {
              refine_state = 1;  /* muss verfeinert werden ! */
              ElSetVerf(active_element,refine_state);  /* wird verfeinert */

              ElSetGlobalVerf(active_element,counter_indicator,1);
              ElSetVerfCount(active_element,ElGetVerfCount(active_element)+1);
          }

        }

        if ( (error < coarse_param) &&
                  (coarse_param > 0.0) && (valid_coarse == 1) &&
                  ( ((vorgaenger=ElGetElementPred(active_element))>=0) &&
                    (ElGetVerf(vorgaenger) == 0) ) ) {

          /* in diesem It.schritt noch nicht verfeinert */
          if (ElGetGlobalVerf(vorgaenger,counter_indicator) == 1)
            ElSetVergrCount(active_element,ElGetVergrCount(active_element)+1);

          if (ElGetVerfCount(vorgaenger) == ElGetVergrCount(active_element)) {
            refine_state = -1;  /* kann vergroebert werden ! */



            /* Elemente hoeherer Dimension */
            CoarsenNeighbourElements(active_element);
          }


        }

        if ( (!valid_coarse) && (!valid_refine) &&
                  ( ((vorgaenger=ElGetElementPred(active_element))>=0) &&
                    (ElGetVerf(vorgaenger) == 0) ) ) {


          /* in diesem It.schritt noch nicht verfeinert */
          if (ElGetGlobalVerf(vorgaenger,counter_indicator) == 1)
            ElSetVergrCount(active_element,ElGetVergrCount(active_element)+1);

          if (ElGetVerfCount(vorgaenger) == ElGetVergrCount(active_element)) {
            refine_state = -1;  /* kann vergroebert werden ! */

            /* Elemente hoeherer und geringerer Dimension */
            CoarsenAllNeighbourElements(active_element);
          }

        }


      }

      if (AdaptGetMethodInd(num_ref_quantity) == 3)
         if (ElGetElementLevel(active_element) > AdaptGetQuantityMaxRefLevel(num_ref_quantity,typ))
           active_indicator = 0;

      if (AdaptGetRefInd(num_ref_quantity,num_indicator,typ,AD) == 0)
        active_indicator = 0;

      if (active_indicator == 1) {
        if (ElGetElementRefineState(active_element) == 0) {
          ElSetElementRefineState(active_element,refine_state);
          if (refine_state == 1) anz_fein++;
          if (refine_state == -1)
            anz_grob++;
        }
        if (ElGetElementRefineState(active_element) == 1)
          ElSetElementRefineState(active_element,1);

        if (ElGetElementRefineState(active_element) == -1) {
          if (refine_state == 1) {
            ElSetElementRefineState(active_element,refine_state);
            anz_fein++;
            anz_grob--;
          }
          if (refine_state == 0) {
            ElSetElementRefineState(active_element,refine_state);
            anz_grob--;
          }
          if (refine_state == -1)
            ElSetElementRefineState(active_element,refine_state);
        }
      }



    }
  }

  anz_verf = anz_fein + anz_grob;

  return anz_verf;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitMemoryIndicator
                                                                          */
/* Aufgabe: Initialisiert 'globale Merker' fuer die Indikatoren

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int num_ref_quantity: Nummer Verfeinerungsgroesse
   E int counter_indicator: Nummer Indikator
   E int iteration: Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/2000      RK         Erste Version
                                                                          */
/**************************************************************************/
void InitMemoryIndicator (int iteration)
{
  static int i, j;

  if (!iteration) {
    for (i=0;i<ElListSize();i++)
      if (ElGetElement(i)!=NULL) {
        ElSetVerf(i,0); /* alle! Merker auf 0 */
      }
  }
  for (i=0;i<ElListSize();i++)
    if (ElGetElement(i)!=NULL)
      if (ElGetElementActiveState(i)) {  /* aktives Element */
        ElSetElementRefineState(i,0);
        ElSetVerfCount(i,0);
        ElSetVergrCount(i,0);
        for (j=0;j<AdaptGetTotalNumInd();j++)
          ElSetGlobalVerf(i,j,0);
      }

}


/**************************************************************************/
/* ROCKFLOW - Funktion: CoarsenNeighbourElements
                                                                          */
/* Aufgabe:
   Markiert Nachbarelemente hoeherer Dimension als
   Element 'index' mit -1 (vergroebern)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: mit -1 (vergroebern) markiertes Element
                                                                          */
/* Ergebnis:
   Mit -1 markierte Nachbarelemente hoeherer Dimension
                                                                          */
/* Programmaenderungen:
   05/2000      RK         Erste Version
   08/2000      RK         Fehler korrigiert !!
                                                                          */
/**************************************************************************/
void CoarsenNeighbourElements (long index)
{
  static int m, j, i;
  static int typ;
  static int anz;
  static long h; /* Hilfsvariable */
  static Kante *k;
  static Flaeche *f;
  static long *elem;
  static long *kinder;
  static long *kinder2;
  static long vorgaenger;

  typ = ElGetElementType(index)-1;  /* Elementtyp */


  /* Elemente hoeherer Dimension */
  if (max_dim>typ) {
    if (max_dim==1) {
      vorgaenger=ElGetElementPred(index);
      kinder = ElGetElementChilds(vorgaenger);
      for (i=0;i<2;i++) {
        if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
          k=GetEdge(ElGetElementEdges(vorgaenger)[0]);
          elem = GetEdge2DElems(ElGetElementEdges(vorgaenger)[0],&anz);
          for (m=0;m<anz;m++) {
            kinder2 = ElGetElementChilds(elem[m]);
            for (j=0;j<4;j++) {
              if ((ElGetElementRefineState(kinder2[j])==0) &&
                   ElGetElementActiveState(kinder2[j]))
                ElSetElementRefineState(kinder2[j],-1);
            }
          }
        }
      }
   }
   else
      if (typ==0) {
        vorgaenger=ElGetElementPred(index);
        kinder = ElGetElementChilds(vorgaenger);
        for (i=0;i<2;i++) {
          if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
            h=ElGetElementPlainsNumber(vorgaenger);

            for (j=0;j<h;j++) {
              f=GetPlain(ElGetElementPlains(vorgaenger)[j]);
              if (f->nachbar_2D>=0l) {
                kinder2 = ElGetElementChilds(f->nachbar_2D);
                for (m=0;m<4;m++) {
                  if (ElGetElementRefineState(kinder2[m])==0 &&
                      ElGetElementActiveState(kinder2[m]))
                    ElSetElementRefineState(kinder2[m],-1);
                }
              }
            }
          }
        }
      }
      else {
        vorgaenger=ElGetElementPred(index);
        kinder = ElGetElementChilds(vorgaenger);
        for (i=0;i<4;i++) {
          if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
            f=GetPlain(ElGetElementPlains(vorgaenger)[0]);
            if (f->nachbarn_3D[0]>=0l) {
              kinder2 = ElGetElementChilds(f->nachbarn_3D[0]);
              for (j=0;j<8;j++) {
                if (ElGetElementRefineState(kinder2[j])==0 &&
                    ElGetElementActiveState(kinder2[j]))
                  ElSetElementRefineState(kinder2[j],-1);
              }
            }
            if (f->nachbarn_3D[1]>=0l) {
              kinder2 = ElGetElementChilds(f->nachbarn_3D[1]);
              for (j=0;j<8;j++) {
                if (ElGetElementRefineState(kinder2[j])==0 &&
                    ElGetElementActiveState(kinder2[j]))
                  ElSetElementRefineState(kinder2[j],-1);
              }
            }
          }
        }
      }
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CoarsenAllNeighbourElements
                                                                          */
/* Aufgabe:
   Markiert Nachbarelemente hoeherer und geringerer Dimension als
   Element 'index' mit -1 (vergroebern)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: mit -1 (vergroebern) markiertes Element
                                                                          */
/* Ergebnis:
   Mit -1 markierte Nachbarelemente hoeherer und geringerer
   Dimension
                                                                          */
/* Programmaenderungen:
   08/2000      RK         Erste Version
                                                                          */
/**************************************************************************/
void CoarsenAllNeighbourElements (long index)
{
  static int m, j, i;
  static int typ;
  static long h; /* Hilfsvariable */
  static Kante *k;
  static Flaeche *f;
  static long *elem;
  static int anz;
  static long *kinder;
  static long *kinder2;
  static long vorgaenger;

  typ = ElGetElementType(index)-1; /* Elementtyp */


  if (max_dim > 0) {
    switch (typ) {
      case 0:
        if (max_dim == 1) {
          vorgaenger=ElGetElementPred(index);
          kinder = ElGetElementChilds(vorgaenger);
          for (i=0;i<2;i++) {
            if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
              k=GetEdge(ElGetElementEdges(vorgaenger)[0]);
              elem = GetEdge2DElems(ElGetElementEdges(vorgaenger)[0],&anz);
              for (m=0;m<anz;m++) {
                kinder2 = ElGetElementChilds(elem[m]);
                for (j=0;j<4;j++) {
                  if (ElGetElementRefineState(kinder2[j])==0
                   && ElGetElementActiveState(kinder2[j]))
                    ElSetElementRefineState(kinder2[j],-1);
                }
              }
            }
          }
        }

        if (max_dim == 2) {
          vorgaenger=ElGetElementPred(index);
          kinder = ElGetElementChilds(vorgaenger);
          for (i=0;i<2;i++) {
            if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
              h=ElGetElementPlainsNumber(vorgaenger);

              for (j=0;j<h;j++) {
                f=GetPlain(ElGetElementPlains(vorgaenger)[j]);
                if (f->nachbar_2D>=0l) {
                  kinder2 = ElGetElementChilds(f->nachbar_2D);
                  for (m=0;m<4;m++) {
                    if (ElGetElementRefineState(kinder2[m])==0
                     && ElGetElementActiveState(kinder2[m]))
                      ElSetElementRefineState(kinder2[m],-1);
                  }
                }
                if (f->nachbarn_3D[0]>=0l) {
                  kinder2 = ElGetElementChilds(f->nachbarn_3D[0]);
                  for (m=0;m<8;m++) {
                    if (ElGetElementRefineState(kinder2[m])==0
                     && ElGetElementActiveState(kinder2[m]))
                      ElSetElementRefineState(kinder2[m],-1);
                  }
                }
                if (f->nachbarn_3D[1]>=0l) {
                  kinder2 = ElGetElementChilds(f->nachbarn_3D[1]);
                  for (m=0;m<8;m++) {
                    if (ElGetElementRefineState(kinder2[m])==0
                     && ElGetElementActiveState(kinder2[m]))
                      ElSetElementRefineState(kinder2[m],-1);
                  }
                }
              }
            }
          }
        }
        break;
      case 1:
        if (max_dim == 1) {
          for (j=0;j<4;j++) {
            k=GetEdge(ElGetElementEdges(index)[j]);
            if ((k->nachbar_1D>=0l) && (ElGetElementRefineState(k->nachbar_1D)==0)
             && ElGetElementActiveState(k->nachbar_1D)) {
              ElSetElementRefineState(k->nachbar_1D,-1);
              }
          }
        }
        if (max_dim == 2) {
          vorgaenger=ElGetElementPred(index);
          kinder = ElGetElementChilds(vorgaenger);
          for (i=0;i<4;i++) {
            if ((kinder[i] != index) && (ElGetElementRefineState(kinder[i]) == -1)) {
              f=GetPlain(ElGetElementPlains(vorgaenger)[0]);
              if (f->nachbarn_3D[0]>=0l) {
                kinder2 = ElGetElementChilds(f->nachbarn_3D[0]);
                for (j=0;j<8;j++) {
                  if (ElGetElementRefineState(kinder2[j])==0
                   && ElGetElementActiveState(kinder2[j]))
                    ElSetElementRefineState(kinder2[j],-1);
                }
              }
              if (f->nachbarn_3D[1]>=0l) {
                kinder2 = ElGetElementChilds(f->nachbarn_3D[1]);
                for (j=0;j<8;j++) {
                  if (ElGetElementRefineState(kinder2[j])==0
                   && ElGetElementActiveState(kinder2[j]))
                    ElSetElementRefineState(kinder2[j],-1);
                }
              }
            }
          }
        }
        break;
      case 2:
        for (j=0;j<6;j++) {
          f=GetPlain(ElGetElementPlains(index)[j]);
          if ((f->nachbar_2D>=0l) && (ElGetElementRefineState(f->nachbar_2D)==0)
           && ElGetElementActiveState(f->nachbar_2D))
            ElSetElementRefineState(f->nachbar_2D,-1);
        }
        break;
    }
  }
}
