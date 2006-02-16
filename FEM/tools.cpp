/**************************************************************************/
/* ROCKFLOW - Modul: tools.c
                                                                          */
/* Aufgabe:
   verschiedene Funktionen, die von verschiedenen Modulen gebraucht
   werden und keine Adaptivitaet voraussetzen (sie aber teilweise
   unterstuetzen)
                                                                          */
/* Programmaenderungen:
   08/1996     MSR/cb        Erste Version
   01.07.1997  R.Kaiser      Korrekturen und Aenderungen aus dem aTM
                             uebertragen
   10/1999     AH            Systemzeit
   01/2000     AH            in GetCurveValue: Konstante Kurve beruecksichtigt
   02/2000     CT            Funktion P0260 von Rainer, CalcIterationError veraendert
   10/2001     AH            Abfrage if (!kurven) in DestroyFunctionsData.
   03/2003     RK            Quellcode bereinigt, Globalvariablen entfernt   
      
                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
#define noTESTTOOLS
/* Header / Andere intern benutzte Module */
#include "tools.h"
#include "elements.h"
#include "nodes.h"
#include "edges.h"
#include "mathlib.h"
#include "femlib.h"
#include "intrface.h"
#include "rfsystim.h"
#include "rf_tim_new.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
/* Tools */
// GEOLib
#include "geo_pnt.h"
#include "geo_strings.h"

// MSHLib
#include "msh_elem.h"
#include "msh_lib.h"
using namespace std;
/* Interne (statische) Deklarationen */

void Restart1DElementNodes ( long number );
void Restart2DElementNodes ( long number );
void Restart3DElementNodes ( long number );

// int GetLineFromFile(char *, ifstream *);

 /* #CURVES */
Kurven *kurven = NULL;
int anz_kurven = 0;

 /* Schluesselwort #FRACTURE_APERTURE_DISTRIBUTION */
double *fracture_aperture_array  = NULL;
hetfields *hf = NULL;
long fracture_aperture_anz = 0l;

#define TIMER_CEN_LIST "CEN_LIST"


/* Tools */

/* Definitionen */

/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructElemsToNodesList
                                                                          */
/* Aufgabe:
   Erzeugt fuer alle Ausgangselemente die Element-zu-Knoten-Liste
   Bei Restart wird sie fuer alle Elemente nachberechnet.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   05/1996     MSR        Restart
   10/1999     AH         Systemzeit
   04/2003     MB         case4/case5
                                                                          */
/**************************************************************************/
void ConstructElemsToNodesList ( void )
{
  long i;
  static long cen_list_id_timer=-1;

  /* Systemzeit fuer Element-zu-Knoten-Liste setzen und ausfuehren */
  SetSystemTime(TIMER_CEN_LIST,"ROCKFLOW","ROCKFLOW: ElemsToNodesList",&cen_list_id_timer);
  RunSystemTime(TIMER_CEN_LIST);

  for (i=0;i<start_new_elems;i++) {
      switch (ElGetElementType(i)) {
          case 1: Assign1DElementNodes(i,ElGetElementNodes(i),ElGetElementNodesNumber(i));
                  break;
          case 2: Assign2DElementNodes(i,ElGetElementNodes(i),ElGetElementNodesNumber(i));
                  break;
          case 3: Assign3DElementNodes(i,ElGetElementNodes(i),ElGetElementNodesNumber(i));
                  break;
          case 4: Assign2DElementNodes(i,ElGetElementNodes(i),ElGetElementNodesNumber(i));
                  break;
          /* prism */
          case 6: Assign3DElementNodes(i,ElGetElementNodes(i),ElGetElementNodesNumber(i));
                  break;
      }
  }
  if (ElListSize()>start_new_elems) {  /* Restart */
      /* Bei Restart muss der Aufbau der Liste nachvollzogen werden, damit
         Geschwister direkt nacheinander eingetragen werden
         -->InterpolIrregNodeConcs etc. */
      for (i=0;i<start_new_elems;i++) {
          switch (ElGetElementType(i)) {
              case 1: Restart1DElementNodes(i);
                      break;
              case 2: Restart2DElementNodes(i);
                      break;
              case 3: Restart3DElementNodes(i);
                      break;
          }
      }
  }

  /* Systemzeit fuer Element-zu-Knoten-Liste anhalten */
  StopSystemTime(TIMER_CEN_LIST);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Restart1DElementNodes
                                                                          */
/* Aufgabe: Passt die Element-zu-Knoten-Liste fuer number und seine
            Kinder an.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Restart1DElementNodes ( long number )
{
  long *kinder = NULL;
  if ((kinder=ElGetElementChilds(number))!=NULL) {
      int i;
      ReAssign1DElementNodes(number,ElGetElementNodes(number),ElNumberOfNodes[0]);
      for (i=0;i<ElNumberOfChilds[0];i++)
          Assign1DElementNodes(kinder[i],ElGetElementNodes(kinder[i]),ElNumberOfNodes[0]);
      for (i=0;i<ElNumberOfChilds[0];i++)
          Restart1DElementNodes(kinder[i]);
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Restart2DElementNodes
                                                                          */
/* Aufgabe: Passt die Element-zu-Knoten-Liste fuer number und seine
            Kinder an.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Restart2DElementNodes ( long number )
{
  long *kinder = NULL;
  if ((kinder=ElGetElementChilds(number))!=NULL) {
      int i;
      ReAssign2DElementNodes(number,ElGetElementNodes(number),ElNumberOfNodes[1]);
      for (i=0;i<ElNumberOfChilds[1];i++)
          Assign2DElementNodes(kinder[i],ElGetElementNodes(kinder[i]),ElNumberOfNodes[1]);
      for (i=0;i<ElNumberOfChilds[1];i++)
          Restart2DElementNodes(kinder[i]);
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Restart3DElementNodes
                                                                          */
/* Aufgabe: Passt die Element-zu-Knoten-Liste fuer number und seine
            Kinder an.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Restart3DElementNodes ( long number )
{
  long *kinder = NULL;
  if ((kinder=ElGetElementChilds(number))!=NULL) {
      int i;
      ReAssign3DElementNodes(number,ElGetElementNodes(number),ElNumberOfNodes[2]);
      for (i=0;i<ElNumberOfChilds[2];i++)
          Assign3DElementNodes(kinder[i],ElGetElementNodes(kinder[i]),ElNumberOfNodes[2]);
      for (i=0;i<ElNumberOfChilds[2];i++)
          Restart3DElementNodes(kinder[i]);
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: PresetErgebnis
                                                                          */
/* Aufgabe:
   Belegt den Loesungsvektor des Gesamtgleichungssystems entweder
   mit 0.0 oder mit den Ergebnissen des letzten Zeitschritts vor
   (abhaengig von NULLE_ERGEBNIS).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X double *ergebnis: Ergebnisvektor
   E int nidx: Knotenergebnis-Index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void PresetErgebnis ( double *ergebnis, int nidx )
{
#ifdef NULLE_ERGEBNIS
    MNulleVec(ergebnis,NodeListLength);
#else
    register long i;
    for(i=0l;i<NodeListLength;i++)
        ergebnis[i]=GetNodeVal(NodeNumber[i],nidx);
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StartTimeStep
                                                                          */
/* Aufgabe:
   Minimalen Anfangszeitschritt berechnen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int dtindex: Index des Elementdatums Courant- bzw. Neumann-Zeitschritt
                                                                          */
/* Ergebnis:
   - double -
                                                                          */
/* Programmaenderungen:
   01/1996     cb         Erste Version
                                                                          */
/**************************************************************************/
double StartTimeStep ( int dtindex )
{
  static long index, l;
  static double dtmin;
  /* Vorschlag zum Anfangszeitschritt */
  dtmin=1.0/MKleinsteZahl;
  l = ElListSize();
  for (index=0;index<l;index++) /* Elemente */
    if (ElGetElement(index)!=NULL)
      if (ElGetElementActiveState(index))
        dtmin=min(ElGetElementVal(index,dtindex),dtmin);
  return dtmin;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FindFrontElement
                                                                          */
/* Aufgabe:
   Massgebendes (Front-) Element (einschl. Zeitschritt) fuer automatische
   Zeitschritt-Steuerung finden. Verfahren: Element mit steilstem
   Gradienten und(!) kleinstem Zeitschritt ist massgebendes Element.

   Voraussetzung: Benoetigte Parameter muessen entsprechend vorhanden
                  sein.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X double *mindt: ??? (msr)
   E int nidx: Index des massgebenden Knotenwertes
   E int dtindex: Elementdatenindex des Courant- bzw. Neumann-Zeitschritts
   E DoubleXFuncLDX ElGetElJacobi: Funktionszeiger auf Funktion, die
                  Jacobi-Matrix und Determinante wie bei aTM liefert
                                                                          */
/* Ergebnis:
   Elementnummer des massgebenden Elements
                                                                          */
/* Programmaenderungen:
   06/1995     cb         Erste Version
   11/1995     msr        static und ohne malloc
   08/1996     cb         neu (einfacher + schneller)
                                                                          */
/**************************************************************************/
long FindFrontElement ( double *mindt, int nidx, int dtindex,
                        DoubleXFuncLDX ElGetElJacobi )
{
  /* Variablen */
  static int et,nn;
  static long elanz,i,j,ele,mele;
  static double maxgc,gc,eledt,detjac,mini;
  static long *kn;
  static double kc[8]; /* max. 1x8-Vektor */
  static double *invjac;  /* max. 3x3-Matrix */
  static double grad[24];  /* max. 3x8-Matrix */
  static double zwi[3];

  /* Initialisieren */
  maxgc = MKleinsteZahl;
  *mindt = mini = 1.0/MKleinsteZahl;
  ele = mele = -1l;

  elanz = ElListSize();
  /* Schleife ueber alle aktiven Elemente */
  for (j=0;j<elanz;j++)
      if (ElGetElement(j)!=NULL)
          if (ElGetElementActiveState(j)) {
              et = ElGetElementType(j);
              nn = ElNumberOfNodes[et-1];
              kn = ElGetElementNodes(j);
              for (i=0;i<nn;i++)
                kc[i] = GetNodeVal(kn[i],nidx);
              /* Gradient der Ansatzfunktionen im Schwerpunkt */
              invjac = ElGetElJacobi(j,&detjac);
              switch(et){
                  case 1: /* 1D-Elemente */
                          /* cb: CalcJac1D(j,invjac,&detjac); */
                          gc = 0.5 * fabs(kc[1] - kc[0]) * MBtrgVec(invjac,3l);
                          break;
                  case 2: /* 2D-Elemente */
                          /* (J^-1)T * grad(omega) * c */
                          MGradOmega2D(grad,0.0,0.0);
                          MMultMatVec(grad,2,4,kc,4,zwi,2);
                          /* cb: CalcJac2D(j,0.0,0.0,invjac,&detjac); */
                          MMultVecMat(zwi,2,invjac,2,2,grad,2);
                          gc = MBtrgVec(grad,2l);
                          break;
                  case 3: /* 3D-Elemente */
                          /* J^-1 * grad(omega) * c */
                          MGradOmega3D(grad,0.0,0.0,0.0);
                          MMultMatVec(grad,3,8,kc,8,zwi,3);
                          /* cb: CalcJac3D(j,0.0,0.0,0.0,invjac,&detjac); */
                          MMultMatVec(invjac,3,3,zwi,3,grad,3);
                          gc = MBtrgVec(grad,3l);
                          break;
              } /* switch */
              eledt = ElGetElementVal(j,dtindex);
              if ((gc > maxgc) && (eledt < *mindt)) {
                  maxgc = gc;
                  *mindt = eledt;
                  ele = j;
              }
              if (eledt < mini) {
                  mini = eledt;
                  mele = j;
              }
          }
  if (ele < 0l) {
      /* Front-Element nicht gefunden */
      DisplayErrorMsg("Front-Element nicht gefunden");
      ele = mele;
      *mindt = mini;
  }
#ifdef TESTTOOLS
    DisplayMsg("Massgebendes Element:");
    DisplayLong(ele); DisplayMsg(" (Gradient="); DisplayDouble(maxgc,0,0);
    DisplayMsg(") + zugeh. dt="); DisplayDouble(*mindt,0,0);
    DisplayMsgLn("");
#endif

  return ele;
}

/**************************************************************************
   ROCKFLOW - Funktion: CalcCourantTimeStep

   Aufgabe:
   Berechnet den Courant- (Cr=1) des angegebenen 1D-, 2D- oder 3D-Elements
   dt_Courant = ds / v

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
   E long ndx   : Speicherplatz fuer die Konzentration in der Knotenstruktur
   E double dc  : Grenzwert fuer Abweichungen der Knoten-Konzentration,
                  damit Element ueberhaupt bearbeitet wird. Fuer dc<0 wird
                  eine Projektion des Geschwindigkeitsvektors auf den
                  Konzentrationsgradienten gemacht und der Geschwindigkeitsvektors
                  dann auf die LÇnge dieser Projektion gebracht. Nur 2D.
                  v_neu=(v*grad(c))/(|v|*|grad(c)|)*v

   Ergebnis:
   double dt  : Courant-Zeitschritt

   Programmaenderungen:
   7/1997 C.Thorenz, basiert auf Teilen von "FindFrontElement"


**************************************************************************/
double CalcCourantTimeStep ( long index, long ndx, double dc )
{
 static int et,anz,i,j;
 static long k,anzgp,nn;
 static long *kn;
 static double *velovec,*invjac;
 static double vs[3],dx[3],dr[3],zwi[9],kc[8],grad[8];
 static double v,ds,el_dt,detjac,a,b,c,fkt,gc,c_min,c_max;
 static double deltax,deltay,deltaz;
 static double porosity;

 CMediumProperties *m_mat_mp = NULL;
 long group = ElGetElementGroupNumber(index);
 m_mat_mp = mmp_vector[group];
 porosity = m_mat_mp->Porosity(index,NULL,1.0);
 /* Betrag der mittleren Geschwindigkeit */
 et = ElGetElementType(index);
 if (et == 2)
   anz = 2;
 else
   anz = 3;
 velovec = ElGetVelocity(index);
/* anzgp = (long)pow((double)ElGetGPNum(index),(double)et);*/
 anzgp = (long)pow((double)GetNumericsGaussPoints(ElGetElementType(index)),(double)et);

 for (j=0;j<anz;j++) {
   vs[j] = 0.0;
   for (k=0l;k<anzgp;k++)
     vs[j] += velovec[anz*k+j] / porosity;
   vs[j] /= (double)anzgp;
 }


/*  nn=pow(2,et); */
 nn = ElNumberOfNodes[et-1];

 invjac = GetElementJacobiMatrix(index,&detjac);
 kn = ElGetElementNodes(index);
 c_min=1./MKleinsteZahl;
 c_max=-c_min;

 for (i=0;i<nn;i++) {
   kc[i] = GetNodeVal(kn[i],ndx);
   c_min=min(c_min,kc[i]);
   c_max=max(c_max,kc[i]);
 }

 /* Wenn keine nennenswerten Konzentrationsunterschiede vorhanden sind, wird das
    Element nicht beruecksichtigt */

 if (dc>0.) {
   if ((c_max-c_min)<fabs(dc)) return -1.;
 } else {
   /* OK if ((c_max-c_min)/(c_max)<fabs(dc)) return -1.; */
   if ((c_max-c_min)/(c_max+MKleinsteZahl)<fabs(dc)) return -1.;
 }

 /* durchstroemte Elementlaenge */
 switch (et) {
   case 1:
     deltax=(GetNode(kn[0])->x)-(GetNode(kn[1])->x);
     deltay=(GetNode(kn[0])->y)-(GetNode(kn[1])->y);
     deltaz=(GetNode(kn[0])->z)-(GetNode(kn[1])->z);
     ds = sqrt(pow(deltax,2.)+pow(deltay,2.)+pow(deltaz,2.));
     v  = sqrt(pow(vs[0],2.)+pow(vs[1],2.)+pow(vs[2],2.));
     break;
   case 2:

     if (dc<0.) {
       /* Geschwindigkeitsvektors wird reduziert auf Laenge der
          Projektion des Geschwindigkeitsvektors auf die Richtung von grad(c) */

       /* v_neu=(v*grad(c))/(|v|*|grad(c)|)*v  */

       MGradOmega2D(grad,0.0,0.0);
       MMultMatVec(grad,2,4,kc,4,zwi,2);
       /* cb: CalcJac2D(j,0.0,0.0,invjac,&detjac); */
       MMultVecMat(zwi,2,invjac,2,2,grad,2);
       /* in den ersten zwei Eintraegen von "grad" steht jetzt
         (hoffentlich) der Gradient von c im A-B Koor.system */

       /* gc= |v|*|grad(c)| */
       gc = sqrt(vs[0]*vs[0]+vs[1]*vs[1]) * sqrt(grad[0]*grad[0]+grad[1]*grad[1]);

       /* v= v*grad(c) */
       v =  vs[0]*grad[0] + vs[1]*grad[1] ;

       /* v= v*grad(c)/(|v|*|grad(c)|) */
       v = -v/max(gc,MKleinsteZahl);

       vs[0] = v*vs[0];
       vs[1] = v*vs[1];
     }

     v = MBtrgVec(vs,anz);

     if (v>MKleinsteZahl) {
       a = fabs(vs[0]);
       b = fabs(vs[1]);
       fkt = max(a,b);
       dr[0] = 2.0 * a / fkt;
       dr[1] = 2.0 * b / fkt;
     }
     else { /* reine Diffusion */
       dr[0] = 2.0;
       dr[1] = 0.0;
     }
     /* dr(lokal) = J * dx(global) */
     invjac = GetElementJacobiMatrix(index,&detjac);
     for (j=0;j<4;j++)
       zwi[j]=invjac[j];
     M2InvertiereUndTransponiere(zwi);   /* MK_bitte_pruefen!!!: Transponierte der */ /* Jakobi-Matrix */
     MMultMatVec(zwi,2,2,dr,2,dx,2);
     ds = MBtrgVec(dx,2);
     /* printf("dx=%e %e ds=%e\n",dx[0],dx[1],ds); */
     break;

  case 3:

      a = fabs(vs[0]);
      b = fabs(vs[1]);
      c = fabs(vs[2]);
      fkt = max(max(a,b),c);
      dr[0] = 2.0 * a / fkt;
      dr[1] = 2.0 * b / fkt;
      dr[2] = 2.0 * c / fkt;
     /* dr(lokal) = J * dx(global) */
     invjac = GetElementJacobiMatrix(index,&detjac);
     for (j=0;j<9;j++)
       zwi[j]=invjac[j];
     M3Invertiere(zwi); /* Jakobi-Matrix */
     MMultMatVec(zwi,3,3,dr,3,dx,3);
     ds = MBtrgVec(dx,3);
     /* printf("dx=%e %e %e ds=%e\n",dx[0],dx[1],dx[2],ds); */
     break;
 }

  el_dt=ds/max(v,MKleinsteZahl); /* Courant */

  return el_dt;

}






/**************************************************************************
   ROCKFLOW - Funktion: CalcSystemCourantTimeStep

   Aufgabe:
   Berechnet den minimalen Courantzeitschritt (Cr=1) fuer alle aktiven Elemente
   dt_Courant = ds / v

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ndx   : Speicherplatz fuer die Konzentration in der Knotenstruktur
   E double dc  : Grenzwert fuer Abweichungen der Knoten-Konzentration,
                  damit Element ueberhaupt bearbeitet wird.

   Ergebnis:
   double dt  : Courant-Zeitschritt

   Programmaenderungen:
   7/1997 C.Thorenz

**************************************************************************/
double CalcSystemCourantTimeStep ( long ndx, double acknowledge )
{
#define DebugCourant

  static long i;
  static double dt,dt_zwischen;
#ifdef DebugCourant
  static long massgebendes_Element;
#endif
  dt_zwischen=-1.;
  for (i=0;i<ElListSize();i++)
    if (ElGetElement(i)!=NULL)  /* Element existiert */
      if (ElGetElementActiveState(i)) {  /* aktives Element */
        dt=CalcCourantTimeStep(i,ndx,acknowledge);
        if (dt>0.) {
          if ((dt<dt_zwischen)||(dt_zwischen<0.)) {
            dt_zwischen=dt;
#ifdef DebugCourant
              massgebendes_Element=i;
#endif
          }
        }
      }
  dt=dt_zwischen;
#ifdef DebugCourant
    if (dt>0.) {
      DisplayMsg("Massgebendes Element Courant:");
      DisplayLong(massgebendes_Element);
      DisplayMsgLn("");
    }
#endif
  return dt;
}

/**************************************************************************
   ROCKFLOW - Funktion: GetSystemCourantNeumannTimeStep

   Aufgabe:
   Holt das Minimum von Courant- und Neumannzeitschritt fuer alle
   aktiven Elemente.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int ndx   : Speicherplatz fuer die Konzentration in der Knotenstruktur
   E int dtindex : Speicherplatz fuer Zeitschritt in der Elementstruktur
   E double dc  : Grenzwert fuer Abweichungen der Knoten-Konzentration,
                  damit Element ueberhaupt bearbeitet wird.
                  Wenn dc<0, dann wird mit den vorhandenen Konzentrationen normiert
   Ergebnis:
   double dt  : Zeitschritt

   Programmaenderungen:
   9/1998 C.Thorenz

**************************************************************************/
double GetSystemCourantNeumannTimeStep ( long ndx, int dtindex, double acknowledge )
{
#define DebugCourantNeumann
  static long i,j,nn,*kn,et;
  static double dt,dt_zwischen,c_min,c_max;

#ifdef DebugCourantNeumann
  static long massgebendes_Element;
#endif
  dt_zwischen=-1.;
  for (i=0;i<ElListSize();i++)
    if (ElGetElement(i)!=NULL)  /* Element existiert */
      if (ElGetElementActiveState(i)) {  /* aktives Element */
        dt = ElGetElementVal(i, dtindex);
        if (dt>0.) {
          if ((dt<dt_zwischen)||(dt_zwischen<0.)) {
            et = ElGetElementType(i);
            nn = ElNumberOfNodes[et-1];
            kn = ElGetElementNodes(i);
            c_min=1./MKleinsteZahl;
            c_max=-c_min;

            for (j=0;j<nn;j++) {
              c_min=min(c_min,GetNodeVal(kn[j],ndx));
              c_max=max(c_max,GetNodeVal(kn[j],ndx));
            }

 /* Wenn keine nennenswerten Konzentrationsunterschiede vorhanden sind, wird das
    Element nicht beruecksichtigt */

            if(acknowledge>0.) {
             if ((c_max-c_min)>=fabs(acknowledge)) {
              dt_zwischen=dt;
#ifdef DebugCourantNeumann
                massgebendes_Element=i;
#endif
             }
            } else {
             if ((c_max-c_min)/(c_max+MKleinsteZahl)>=fabs(acknowledge)) {
              dt_zwischen=dt;
#ifdef DebugCourantNeumann
                massgebendes_Element=i;
#endif
             }
            }
          }
        }
      }
  dt=dt_zwischen;
#ifdef DebugCourantNeumann
    if (dt>0.) {
      DisplayMsg("Massgebendes Element CourantNeumann:");
      DisplayLong(massgebendes_Element);
      DisplayMsg(" dt=");
      DisplayDouble(dt,0,0);
      DisplayMsgLn("");
    }
#endif
  return dt;
}



/**************************************************************************
   ROCKFLOW - Funktion: TestElementDirtyness

   Aufgabe:
   Untersucht, ob sich bestimmte Groessen in den Knoten eines
   Elements veraendert haben.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index : Elementnummer
   E long ndx1  : Speicherplatz fuer den ersten Wert des Vergleichpaares in der Knotenstruktur
   E long ndx2  : Speicherplatz fuer den zweiten Wert des Vergleichpaares in der Knotenstruktur
   E double dx  : Grenzwert fuer Abweichungen der Knoten-Konzentration,
                  damit Element als "dirty" bezeichnet wird.
                  dx < 0.  : Normieren mit Knotenwerten

   Ergebnis:
   int dirty    : Dirtyness

   Programmaenderungen:
   11/1997 C.Thorenz, erste Version

**************************************************************************/
int TestElementDirtyness ( long index, long ndx1, long ndx2, double acknowledge)
{

  static long i,anz_knoten;
  static long *knoten;
  static double a,b,c,d;

  d=fabs(acknowledge);

  if (d < MKleinsteZahl) return 1;

  knoten = ElGetElementNodes(index);
  anz_knoten = 1;
  for (i=1;i<=ElGetElementType(index);i++) anz_knoten*=2;

  for (i=0;i<anz_knoten;i++) {
    if (acknowledge < 0.) {
      a=(GetNodeVal(knoten[i],ndx1)-GetNodeVal(knoten[i],ndx2));
      b=0.5*(GetNodeVal(knoten[i],ndx1)+GetNodeVal(knoten[i],ndx2));
      c=fabs(a/(b+MKleinsteZahl));
    } else {
      a=(GetNodeVal(knoten[i],ndx1)-GetNodeVal(knoten[i],ndx2));
      c=fabs(a);
    }
    if(c > d) return 1;
  }
  return 0;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: Signum
                                                                          */
/* Aufgabe:
   Gibt abhaengig vom Vorzeichen -1,0,1 zurueck
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double zahl
                                                                          */
/* Ergebnis:
   vorzeichen
                                                                          */
/* Programmaenderungen:
   1/1998     C.Thorenz  Erste Version                                                                          */
/**************************************************************************/
int Signum(double x)
{
    if (x > 0.)
        return 1;
    if (fabs(x)< MKleinsteZahl)
        return 0;
    if (x < 0.)
        return -1;
    return 0;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: InterpolateElementNodesValues
                                      */
/* Aufgabe:
   Bildet den arithmetischen Mittel einer Elementgroesse durch Interpolation
   der zugehoerigen Knotenwerte.
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E long idx  : Index der Variable in der internen Datensturktur
                                      */
/* Ergebnis:
   - double - Mittelwert im Element (arithmetischer Mittel)
                                      */
/* Programmaenderungen:
   09/1997       ah         Erste Version

   letzte Aenderung:     A.Habbar     18.09.1997
                                      */
/**************************************************************************/
double InterpolateElementNodesValues ( long index, long idx )
{
  static long i;
  static int nn;
  static long *element_nodes;
  static double val;

  val=0.;
  if (ElGetElement(index)!=NULL)   /* wenn Element existiert */
     if (ElGetElementActiveState(index)){  /* nur aktive Elemente */
        nn = ElGetElementNodesNumber(index);
        element_nodes = ElGetElementNodes(index);
        for (i=0;i<nn;i++) val +=GetNodeVal(element_nodes[i],idx);
        val /= (double)nn;
        element_nodes = NULL;
     }
  return val;

}



/**************************************************************************
 ROCKFLOW - Funktion: GetCurveValue

 Aufgabe:
   Liefert Wert aus einer Kurve fuer angegebenen Punkt.
   Liegt der Punkt ausserhalb des durch die Kurve beschriebenen
   Bereichs, wird der letzte bzw. erste Wert zurueckgeliefert und
   der Flag fuer die Gueltigkeit auf 0 gesetzt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int kurve: Kurvennummer, >= 0
   E int methode  : Interpolationsmethode
   E double punkt: Punkt
   R int gueltig: Flag fuer die Gueltigkeit des zurueckgelieferten Wertes

 Ergebnis:
   s.o.

 Programmaenderungen:
   Basiert auf "GetRBZValue"

   04/1999     C.Thorenz     Gueltigkeit und Methode eingefuehrt
   09/2000     C.Thorenz     Fehlermeldung bei falscher Kurvennummer

**************************************************************************/
double GetCurveValue(int kurve, int methode, double punkt, int *gueltig)
{
    static long anz;
    register long i;
    static StuetzStellen *s;

    if (kurve==0) {*gueltig = 1; return 1.0; }

#ifdef ERROR_CONTROL
    if((kurve<0)||(kurve>=anz_kurven)) {
        DisplayMsgLn("");
        DisplayMsg("PANIC! Curve ");
        DisplayLong(kurve);
        DisplayMsgLn(" is requested but not defined!");
        abort();
    }
#endif

    anz = kurven[kurve].anz_stuetzstellen;
    s = kurven[kurve].stuetzstellen;
    *gueltig = 1;
    i = 1l;

    if (punkt < s[0].punkt) {
        *gueltig = 0;
        return s[0].wert;
    }
    if (punkt > s[anz - 1l].punkt) {
        *gueltig = 0;
        return s[anz - 1l].wert;
    }

    /* Suchen der Stuetzstelle. Vorraussetzung: Zeitpunkte aufsteigend geordnet */
    while (punkt > s[i].punkt)
        i++;

    switch (methode) {
    default:
    case 0:
        /* Lineare Interpolation */
        return (s[i - 1].wert +
                (s[i].wert - s[i - 1l].wert) / (s[i].punkt - s[i - 1l].punkt) *
                (punkt - s[i - 1l].punkt));
    }
}

/**************************************************************************
 ROCKFLOW - Funktion: GetCurveValueInverse

 Aufgabe:
   Liefert zu einem Wert aus einer Kurve den zugehoerigen Punkt.
   Liegt der Punkt ausserhalb des durch die Kurve beschriebenen
   Bereichs, wird der letzte bzw. erste Wert zurueckgeliefert und
   der Flag fuer die Gueltigkeit auf 0 gesetzt.

   Kurven muessen streng monoton fallend oder steigend sein.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int kurve: Kurvennummer, >= 0
   E int methode  : Interpolationsmethode
   E double wert: Wert
   R int gueltig: Flag fuer die Gueltigkeit des zurueckgelieferten Wertes

 Ergebnis:
   s.o.

 Programmaenderungen:
   Basiert auf "GetRBZValue"

   12/1999     C. Thorenz Erste Version

**************************************************************************/
double GetCurveValueInverse(int kurve, int methode, double wert, int *gueltig)
{
    static long anz;
    register long i;
    static StuetzStellen *s;

#ifdef ERROR_CONTROL
    if((kurve<0)||(kurve>=anz_kurven)) {
        DisplayMsgLn("");
        DisplayMsg("PANIC! Curve ");
        DisplayLong(kurve);
        DisplayMsgLn(" is requested but not defined!");
        abort();
    }
#endif

    anz = kurven[kurve].anz_stuetzstellen;
    s = kurven[kurve].stuetzstellen;
    *gueltig = 1;
    i = 1l;

    if (s[0].wert < s[anz - 1l].wert) {
        /* Monoton steigend */
        if (wert < s[0].wert) {
            *gueltig = 0;
            return s[0].punkt;
        }
        if (wert > s[anz - 1].wert) {
            *gueltig = 0;
            return s[anz - 1].punkt;
        }
        /* Suchen der Stuetzstelle. Vorraussetzung: Zeitpunkte aufsteigend geordnet */
        while (wert > s[i].wert) i++;
    } else {
        /* Monoton fallend */
        if (wert > s[0].wert) {
            *gueltig = 0;
            return s[0].punkt;
        }
        if (wert < s[anz - 1].wert) {
            *gueltig = 0;
            return s[anz - 1].punkt;
        }
        /* Suchen der Stuetzstelle. Vorraussetzung: Zeitpunkte aufsteigend geordnet */
        while (wert < s[i].wert) i++;
    }

    switch (methode) {
    default:
    case 0:
        /* Lineare Interpolation */
        return (s[i - 1].punkt +
                (s[i].punkt - s[i - 1l].punkt) / (s[i].wert - s[i - 1l].wert) *
                (wert - s[i - 1l].wert));
    }
}




/**************************************************************************
 ROCKFLOW - Funktion: GetCurveDerivative

 Aufgabe:
   Liefert die Ableitung zu einem Punkt auf einer Kurve.
   Liegt der Punkt ausserhalb des durch die Kurve beschriebenen
   Bereichs, wird die letzte bzw. erste moegliche Ableitung
   zurueckgeliefert und der Flag fuer die Gueltigkeit auf 0 gesetzt.

   Die Ableitung kann ueber mehrere Methoden bestimmt werden:
     0: Stueckweise konstant
     1: Gleitend

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int kurve: Kurvennummer, >= 0
   E int methode  : Ableitungsmethode
   E double punkt: Punkt
   R int gueltig: Flag fuer die Gueltigkeit des zurueckgelieferten Wertes

 Ergebnis:
   s.o.

 Programmaenderungen:

   3/2002   C. Thorenz Erste Version
**************************************************************************/
double GetCurveDerivative(int kurve, int methode, double punkt, int *gueltig)
{
    static long anz;
    register long i;
    static StuetzStellen *s;
    static double w, s1, s2; 

    if (kurve==0) {*gueltig = 1; return 1.0; }

#ifdef ERROR_CONTROL
    if((kurve<0)||(kurve>=anz_kurven)) {
        DisplayMsgLn("");
        DisplayMsg("PANIC! Curve ");
        DisplayLong(kurve);
        DisplayMsgLn(" is requested but not defined!");
        abort();
    }
#endif

    anz = kurven[kurve].anz_stuetzstellen;
    s = kurven[kurve].stuetzstellen;
    *gueltig = 1;
    i = 1l;

    if (punkt < s[0].punkt) {
        *gueltig = 0;
        i = 1;
        punkt = s[0].punkt; 
    } else if (punkt > s[anz - 1l].punkt) {
        *gueltig = 0;
        i = anz-1;
        punkt = s[anz-1].punkt; 
    } else {
        /* Suchen der Stuetzstelle. Vorraussetzung: Zeitpunkte aufsteigend geordnet */
        while (punkt > s[i].punkt) i++;
    }

    switch (methode) {
    default:
    case 0:
        /* Stueckweise konstant */
        if(fabs(s[i].punkt-s[i-1].punkt) > DBL_MIN)
           return (s[i].wert-s[i-1].wert)/(s[i].punkt-s[i-1].punkt);
        else
           return Signum(s[i+1].wert-s[i].wert)/DBL_MIN;
    case 1:
        /* Gleitend */
        if ((i>1)&&(i<anz-2)) {
          s1 = (0.5*s[i].wert-0.5*s[i-2].wert)/ 
               (0.5*s[i].punkt-0.5*s[i-2].punkt);

          s2 = (0.5*s[i+1].wert-0.5*s[i-1].wert)/ 
               (0.5*s[i+1].punkt-0.5*s[i-1].punkt);

          w  = (punkt-s[i-1].punkt)/(s[i].punkt-s[i-1].punkt);
 
          return (1.-w) * s1 + w * s2;
        } else {
          /* Stueckweise konstant */
          if(fabs(s[i].punkt-s[i-1].punkt) > DBL_MIN)
             return (s[i].wert-s[i-1].wert)/(s[i].punkt-s[i-1].punkt);
          else
             return Signum(s[i+1].wert-s[i].wert)/DBL_MIN;
        } 
    }
}







/**************************************************************************/
/* ROCKFLOW - Funktion: CreateFunctionsData
                                                                          */
/* Aufgabe:
   Speicherbelegungen und Initialisierungen fuer Functions-Daten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
int CreateFunctionsData(void)
{
    kurven = NULL;
    anz_kurven = 0;

    return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyFunctionsData
                                                                          */
/* Aufgabe:
   Speicherfreigaben fuer Functions-Daten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1999    OK      Implementierung
   10/2001    AH      Abfrage: if (!kurven)
                                                                          */
/**************************************************************************/
void DestroyFunctionsData(void)
{
  int i;

  if (!kurven) return;

  for (i=0;i<anz_kurven;i++)
      kurven[i].stuetzstellen = (StuetzStellen *) Free(kurven[i].stuetzstellen);
  kurven = (Kurven *) Free(kurven);
    /* sollte bereits in ConstructStatusEntries geschehen sein */

  fracture_aperture_array = (double *) Free(fracture_aperture_array);

}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctCurves
                                      */
/* Aufgabe:
   Liest die zu dem Schluesselwort CURVES gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   CURVES: Kurven fuer funktionale Zusammenhaenge
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   07/1996     RK         Rockflow-Anpassung
   12/1999     OK         Protokoll für RF-Shell

   letzte Aenderung: OK 05.05.2000 Bugfix
                                       */
/**************************************************************************/
int FctCurves(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    StuetzStellen *stuetz;
    long anz;
    double d1, d2;
    int curve_counter = 0;
    int i;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort CURVES");
#endif

    LineFeed(f);
    FilePrintString(f, "; 9 Functions ----------------------------------------------------------");
    LineFeed(f);

    /* Erste Konstante 1-Zeitkurve einfuegen (Index 0) - muss vorhanden sein TODO */
    if ( (found==0)||(found==1) ) {
        anz_kurven = 1;
        stuetz = (StuetzStellen *) Malloc(sizeof(StuetzStellen));
        stuetz[0].punkt = 1.0;
        stuetz[0].wert = 1.0;
        kurven = (Kurven *) Malloc(sizeof(Kurven));
        kurven[anz_kurven - 1].anz_stuetzstellen = 1;
        kurven[anz_kurven - 1].stuetzstellen = stuetz;
    }
    /* einlesen */
    if (!found) {
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #CURVES (z.B. Zeitkurven)");
        LineFeed(f);
#endif
    } else {
        /* CURVES gefunden */
        while (StrTestHash(&data[p],&pos)||(found==2)) {
            /* Pruef-Funktion*/
            if ( (found==2)&&(curve_counter>=anz_kurven) ) {
                break;
            }
            FilePrintString(f, "#CURVES");
            LineFeed(f);
#ifdef EXT_RFD
            FilePrintString(f, "; Schluesselwort: #CURVES (z.B. fuer Zeitkurven)");
            LineFeed(f);
            FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein.");
            LineFeed(f);
            FilePrintString(f, "; Es folgen beliebig viele Abschnitte, die jeweils eine eigene Kurve");
            LineFeed(f);
            FilePrintString(f, "; darstellen und jeweils mit dem Schluesselwort #CURVES eingeleitet werden");
            LineFeed(f);
            FilePrintString(f, "; muessen. Jede Dieser Kurven hat folgendes Aussehen:");
            LineFeed(f);
            FilePrintString(f, ";   Eine Kurve setzt sich aus Stuetzstellen und Werten an den Stuetzstellen");
            LineFeed(f);
            FilePrintString(f, ";   zusammen:");
            LineFeed(f);
            FilePrintString(f, ";   Es folgen beliebig viele Bloecke mit je 2 Werten.");
            LineFeed(f);
            FilePrintString(f, ";   - Stuetzstelle [double]");
            LineFeed(f);
            FilePrintString(f, ";   - Funktionswert an der Stuetzstelle [double]");
            LineFeed(f);
#endif
            if (found==1) {
                stuetz = NULL;
                anz = 0l;
                while (StrTestDouble(&data[p += pos])) {
                    ok = (StrReadDouble(&d1, &data[p], f, TFDouble, &pos) && ok);
                    ok = (StrReadDouble(&d2, &data[p += pos], f, TFDouble, &pos) && ok);
                    LineFeed(f);
                    anz++;
                    stuetz = (StuetzStellen *) Realloc(stuetz, (anz * sizeof(StuetzStellen)));
                    stuetz[anz - 1].punkt = d1;
                    stuetz[anz - 1].wert = d2;
                }
                if (anz >= 1l) {
                    /* gueltige Zeitkurve, d.h. mind. 1 gueltige Stuetzstelle */
                    anz_kurven++;
                    kurven = (Kurven *) Realloc(kurven, (anz_kurven * sizeof(Kurven)));
                    kurven[anz_kurven - 1].anz_stuetzstellen = anz;
                    kurven[anz_kurven - 1].stuetzstellen = stuetz;
                } else {
                    FilePrintString(f, "* vorhergehende Zeitkurve unzulaessig, ignoriert !");
                    LineFeed(f);
                    stuetz = (StuetzStellen *) Free(stuetz);
                   /* stuetz = Free(stuetz); MFC */
                }
            }
            else if (found==2) {
              if(curve_counter>0) { /* Dummy-Kurve nicht ausgeben */
                stuetz = kurven[curve_counter].stuetzstellen;
                for(i=0;i<kurven[curve_counter].anz_stuetzstellen;i++) {
                    fprintf(f," %e ",stuetz[i].punkt);
                    fprintf(f," %e ",stuetz[i].wert);
                    LineFeed(f);
                }
                stuetz = NULL;
              }
            }
        curve_counter++;
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctFractureApertureDistribution
                                      */
/* Aufgabe:
   Liest die zu dem Schluesselwort FRACTURE_APERTURE_DISTRIBUTION
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                      */
/* Ergebnis:
    0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
    09/1997     O.Kolditz
                                      */
/**************************************************************************/
int FctFractureApertureDistribution(char *data, int found, FILE * f)
{
    int pos = 0;
    int p = 0;
    int ok = 1;
    double d;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort FRACTURE_APERTURE_DISTRIBUTION");
#endif
#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #FRACTURE_APERTURE_DISTRIBUTION");
        LineFeed(f);
#endif
    } else {
        /* FRACTURE_APERTURE_DISTRIBUTION gefunden */
        FilePrintString(f, "#FRACTURE_APERTURE_DISTRIBUTION");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #FRACTURE_APERTURE_DISTRIBUTION");
        LineFeed(f);
        FilePrintString(f, "; - Anzahl von Werten [anz_b>=0]");
        LineFeed(f);
        FilePrintString(f, "; - anz_b Kluftoeffnungsweiten");
        LineFeed(f);
#endif
/*      ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
      LineFeed(f);
      p += pos; */
        do {
            ok = (StrReadDouble(&d, &data[p], f, TFDouble, &pos) && ok);
            LineFeed(f);
            fracture_aperture_anz++;
            fracture_aperture_array = \
                (double *) Realloc(fracture_aperture_array, (fracture_aperture_anz * sizeof(double)));
            fracture_aperture_array[fracture_aperture_anz - 1] = d;
        } while (StrTestDouble(&data[p += pos]));
        LineFeed(f);

    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: StrCmp
                                                                          */
/* Aufgabe:
   Vergleicht zwei Zeichenketten, wobei ein sicheres Handling von
   Konstanten gewaehrleistet wird. Es wird zwischen Klein- und
   Grossbuchstaben nicht unterschieden.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X char *string1:
   X char *string2:
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   01/2000     AH        Erste Version
                                                                          */
/**************************************************************************/
int StrCmp ( char *string1, char *string2 )
{
  int l1 = (int)strlen(string1);
  int l2 = (int)strlen(string2);
  char *copy_string1=NULL;
  char *copy_string2=NULL;
  int cmp=1;

  copy_string1 = (char *) Malloc(l1+1);
  if ( copy_string1 == NULL ) {
    printf ("\n!!! Nicht genug Speicher um eine neue Zeichenkette zu allokieren !!!");
    return cmp;
  }
  copy_string2 = (char *) Malloc(l2+1);
  if ( copy_string2 == NULL ) {
    copy_string1=(char *)Free(copy_string1);
    printf ("\n!!! Nicht genug Speicher um eine neue Zeichenkette zu allokieren !!!");
    return cmp;
  }


  strcpy(copy_string1,string1);
  strcpy(copy_string2,string2);
  cmp=strcmp(StrUp(copy_string1),StrUp(copy_string2));

  copy_string1=(char *)Free(copy_string1);
  copy_string2=(char *)Free(copy_string2);

  return cmp;
}


/**************************************************************************
   ROCKFLOW - Funktion: P0260

   Aufgabe:
        Spalten einer (long) Matrix aufsteigend sortieren (durch Mischen)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
       ma, na: Groesse, Spalten hintereinander gespeichert (wie FTN)
       iz1,iz2: (0 - ma-1) erste und letzte Zeile, nach denen sortiert wird

   Ergebnis:

   Programmaenderungen:
       R. Ratke, Jan. 2000

**************************************************************************/
void P0260 (long imat[], long ma, long na, long iz1, long iz2)

{long  idz=1, is=0, js=na;         /* Inkrement Sortierzeilen, Spaltenanfaenge */
 long  i, k, j1, j2, j3, j4, la, lb, ls=1, ka, kb, ns;
 long *sort; sort =(long *) Malloc(2*na*sizeof(long));
    for (i=0; i<na; i++) {sort[i]=ma*i;} /* rel. Spaltenanfaenge des Originals */
    if (iz2<iz1) idz=-1;

/* Sortieren mit MixSort, aufsteigend */
    do
    {  kb=0;         /* Index alte Folge */
       ns=0;         /* Anzahl neu sortierter Spalten insges. */

       do
       {  ka=kb;            /* 1. sortiertes Stueck, Puffer "A" */
          la=min(ls,na-ka); /* Laenge */
          kb +=la;          /* 2. Teilstueck, Puffer "B" */
          lb=min(ls,na-kb);

Next_item:
          if (la<1) goto Rest_B;   /* evtl. noch Rest aus "B" */
          if (lb<1) goto Aus_A;

          i=iz1-idz;
          do
          { i +=idz; k=imat[i+sort[is+ka]]-imat[i+sort[is+kb]];
            if (k<0) goto Aus_A; if (k>0) goto Aus_B;
          } while (i!=iz2);        /* ueber die Sortierzeilen */

Aus_A:    sort[js+ns]=sort[is+ka]; /* aus Puffer "A" */
          ka++; la--; ns++;
          goto Next_item;

Rest_B:   if (lb<1) goto Alles_leer;
Aus_B:    sort[js+ns]=sort[is+kb];    /* aus Puffer "B" */
          kb++; lb--; ns++;
          goto Next_item;

Alles_leer: i=i;
       } while (ns<na);        /* noch neue Folgenpaare */
       ls +=ls;                /* Durchgang fertig, doppelte Sortierlaenge */
       is=js; js=na-is;        /* neue Reihenfolge ab is, Indextausch */
    } while (ls<na);   /* nochmal mischen */

/* Matrix in neue Reihenfolge bringen */
    for (k=0; k<na; k++)     /* alle Spalten */
    {  sort[k]=sort[is+k]/ma;/* Sortierfolge, ohne Faktor ma */
       sort[na+k]=k;         /* Spaltennummer */
    }

    j2=0;                   /* Zielindex */
    for (k=0; k<na; k++)    /* neue Spalten ab j2 */
    { j1 =sort[na+sort[k]]; /* gesuchte Spalte */
      j3 =j1*ma;            /* Spaltenanfang */
      if (j3==j2) goto Next_Spal;

      for (i=0;i<ma;i++)    /* Gesamte Spalte tauschen */
      {  j4 =imat[j3+i];  imat[j3+i]=imat[j2+i];  imat[j2+i]=j4;
      }
      j3= sort[na+k];       /* alter Inhalt Sp. k */
      sort[na+j1]=j3;       /* steht nun in Sp. j1 */
      sort[na+j3]=j1;       /* wenn j3 gesucht wird --> in j1 */
Next_Spal:
        j2 +=ma;
      }
      Free (sort);
      return;
} /* END P0260 */

/**************************************************************************
   ROCKFLOW - Funktion: P0265

   Aufgabe:
      Vektor ivec der Laenge n aufsteigend sortieren (durch Mischen)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
        X: long *ivec:  Vektor der Laenge n aufsteigend sortieren (durch Mischen)
        E: double *rvec:  Sortierkriterium
        E: long n: Vektorlaenge
        X: long *sort: Hilfsvektor der Laenge 2*n, initialisiert, wird nicht freigegeben

   Ergebnis:

   Programmaenderungen:
       R. Ratke, Jan. 2000

**************************************************************************/

void P0265 (long ivec[], double rvec[], long n, long *sort)
{long  is=0, js=n;         /* Indizes in sort */
 long  i, j, k, la, lb, ls=32, ka, kb, ns;
 if (sort==NULL) sort =(long *) Malloc(2*n*sizeof(long));
    for (i=0; i<n; i++) {sort[i]=i;} /* Originalnumerierung */

/* Vorsortieren mit Bubble-Sort: Stuecke der Laenge ls*/
    if (ls>n) ls=n;
    for (i=0; i<n; i=ns)
    {  ns=i+ls; if (ns>n) ns=n;
       for (j=i; j<(ns-1); j++)
       {  kb=sort[j];
          for (k=j+1; k<ns; k++)
          {  if (rvec[sort[k]] < rvec[kb])
             {  kb=sort[k]; sort[k]=sort[j]; sort[j]=kb;
    }  }  }  }

/* Sortieren mit MixSort, aufsteigend */
    while (ls<n)
    {  kb=0;         /* Index alte Folge */
       ns=0;         /* Anzahl neu sortierter Elemente insges. */

       do
       {  ka=kb;           /* 1. sortiertes Stueck, Puffer "A" */
          la=min(ls,n-ka); /* Laenge */
          kb +=la;         /* 2. Teilstueck, Puffer "B" */
          lb=min(ls,n-kb);

Next_item:
          if (la<1) goto Rest_B;   /* evtl. noch Rest aus "B" */
          if (lb<1) goto Aus_A;

          if (rvec[sort[is+ka]] > rvec[sort[is+kb]]) goto Aus_B;

Aus_A:    sort[js+ns]=sort[is+ka]; /* aus Puffer "A" */
          ka++; la--; ns++;
          goto Next_item;

Rest_B:   if (lb<1) goto Alles_leer;
Aus_B:    sort[js+ns]=sort[is+kb]; /* aus Puffer "B" */
          kb++; lb--; ns++;
          goto Next_item;


Alles_leer: n=n;
       } while (ns<n); /* noch neue Folgenpaare */
       ls +=ls;        /* Durchgang fertig, doppelte Sortierlaenge */
       is=js; js=n-is; /* neue Reihenfolge ab is, Indextausch */
    } /*  while (ls<n) nochmal mischen */

/* Vektor in neue Reihenfolge bringen */
    for (k=0; k<n; k++)    /* alle Spalten */
    {  sort[k]=sort[is+k]; /* Sortierfolge, ohne Faktor ma */
       sort[n+k]=k;        /* Zeiger auf Element */
    }

    for (k=0; k<n; k++)    /* Zielindex */
    {  i =sort[n+sort[k]]; /* dort steht jetzt die Quelle */
       if (i!=k)
       {  j =ivec[i];    ivec[i] =ivec[k];  ivec[k]=j; /* tauschen */
          j =sort[n+k];  sort[n+i] =j;   sort[n+j] =i; /* Verweis berichtigen */
    }  }
      return;
} /* END P0265 */






/**************************************************************************
   ROCKFLOW - Funktion: DampOscillations

   Aufgabe:
   
   Daempft Oszillationen eines Knotenfeldes.
    - begrenzt den Wertebereich
    - begrenzt die Differenz im Element
   
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
        E: int ndx1: Knotenindex der betrachteten Groesse
        E: int oscil_damp_method: Art der Daempfung
        E: double *oscil_damp_parameter: Parameter
        E: double (*NodeCalcLumpedMass) (long)): Funktion um die gelumpte 
           Massen-/Speichermatrix fuer einen Knoten zu holen.

   Ergebnis:

   - long -  Anzahl der ausgefuehrten Operationen

   Programmaenderungen:
   07/2002   CT   Erste Version

  **************************************************************************/
long DampOscillations(int ndx1, int oscil_damp_method, double *oscil_damp_parameter, double (*NodeCalcLumpedMass) (long))
{
  long i, j, nn, go_on, node, *nodes, *edges;
  double val_min, val_max, val_min_max, val_max_diff, val_soll, val, m1, m2, val_max_neu, val_min_neu;
  double val_min_abs, val_max_abs, val_min_ele, val_max_ele; 
  long val_min_node, val_max_node, damping=0;
  int anz_edge;
  double mass_sum, ml, sum_dist=0.;

  /* Vorbelegen der Steuerparameter */
  switch(oscil_damp_method) {
    default:
      DisplayMsg("!!!   Error: Unknown damping method.");
      DisplayMsgLn("!!!  Terminating.");
      exit(1);
    case 0:
      val_min=-1.e99;
      val_max=1.e99;
      val_max_diff=1.e99;
      break;
    case 1:
      val_min=oscil_damp_parameter[0];
      val_max=oscil_damp_parameter[1];
      val_max_diff=1.e99;
      break;
    case 2:
      val_min=-1.e99;
      val_max=1.e99;
      val_max_diff=oscil_damp_parameter[0];
      break;
    case 3:
      val_min=-1.e99;
      val_max=1.e99;
      val_max_diff=oscil_damp_parameter[0];
      break;
    case 4:
      val_min=oscil_damp_parameter[0];
      val_max=oscil_damp_parameter[1];
      val_max_diff=oscil_damp_parameter[2];
      break;
    case 5:
      val_min=oscil_damp_parameter[0];
      val_max=oscil_damp_parameter[1];
      val_max_diff=oscil_damp_parameter[2];
      break;
  }


  do {
    go_on = 0; 

    if (oscil_damp_method==0)
         /* Nix tun */ 
         return 0;

    if ((oscil_damp_method==1)||(oscil_damp_method==4)||(oscil_damp_method==5)) {
        /* Min-Max-Begrenzung der Werte */

        for (i = 0; i < NodeListLength; i++) {
          node = NodeNumber[i];
          val = GetNodeVal(node, ndx1);
          val_soll=val;
   
          /* Liegt der Wert ausserhalb des MinMax-Intervalls? */
          if ((val < val_min)||(val > val_max)) {
            /* Alle Konten an diesem Knoten holen */
            edges = GetNodeEdges(node, &anz_edge);
            /* Speicher fuer alle Knoten an den Kanten */
            nodes = (long *) Malloc(anz_edge*sizeof(long));

            go_on = 1; 
             
            if (val < val_min) val_soll = val_min;
            if (val > val_max) val_soll = val_max;
      
            /* Alle Nachbarknoten holen */
            for (j = 0; j < anz_edge; j++) {
              if(GetEdge(edges[j])->knoten[0]!=node) nodes[j]=GetEdge(edges[j])->knoten[0]; 
              if(GetEdge(edges[j])->knoten[1]!=node) nodes[j]=GetEdge(edges[j])->knoten[1]; 
            }

            /* Massenspeicherung der Knoten bestimmen */
            ml = mass_sum = NodeCalcLumpedMass(node);
            for (j = 0; j < anz_edge; j++) 
              mass_sum += NodeCalcLumpedMass(nodes[j]);
     
            /* Umverteilen der Masse */
            for (j = 0; j < anz_edge; j++) 
              SetNodeVal(nodes[j], ndx1, GetNodeVal(nodes[j], ndx1)+ml/mass_sum*(val_soll+val));

            /* Setzen des Sollwertes */
            SetNodeVal(node, ndx1, val_soll);
 
            /* Protokollieren der Aenderung */
            sum_dist += fabs(val_soll - val) * ml;
            damping++;

            /* Speicher freigeben */
            nodes = (long *) Free(nodes);
          }
        }
     }

     if ((oscil_damp_method==2)||(oscil_damp_method==3)||(oscil_damp_method==4)||(oscil_damp_method==5)) {
        /* Begrenzung der (relativen) Differenz im Element */
        val_min_max = 1.;

        if ((oscil_damp_method==3)||(oscil_damp_method==5)) {
          /* Relativ zur maximalen Differenz */
          val_min_abs =  1.e99;
          val_max_abs = -1.e99;

          /* Feststellen der Maximaldifferenz im System */
          for (i = 0; i < NodeListLength; i++) {
             val = GetNodeVal(NodeNumber[i], ndx1);
             if(val<val_min_abs) val_min_abs = val; 
             if(val>val_max_abs) val_max_abs = val;
          }   
          val_min_max = val_max_abs-val_min_abs;
        }

        for (i = 0; i < anz_active_elements; i++) {
           /* Anzahl der Knoten am Element */
           nn = ElNumberOfNodes[ElGetElementType(ActiveElements[i]) - 1];
           /* Alle Knoten am Element */
           nodes = ElGetElementNodes(ActiveElements[i]);

           val_min_ele =  1.e99;
           val_max_ele = -1.e99;
           val_min_node = -1; 
           val_max_node = -1; 

           /* Bestimmen der Knoten mit den Extremwerten im Element */
           for (j = 0; j < nn; j++) {
             val = GetNodeVal(nodes[j], ndx1);
             if(val<val_min_ele) {
                val_min_ele = val;
                val_min_node = nodes[j];
             } 
             if(val>val_max_ele) {
                val_max_ele = val;
                val_max_node = nodes[j];
             } 
           }

           /* Bei zu grossen Differenzen eingreifen */ 
           if ((val_max_ele-val_min_ele)/val_min_max > val_max_diff*1.0001) {
             /* Weitere Iterationen noetig */
             go_on = 1; 

             /* Gelumpte Masse fuer die Extremknoten holen */
             m1 = NodeCalcLumpedMass(val_max_node);
             m2 = NodeCalcLumpedMass(val_min_node);
             
             /* Verteilung ausrechnen */
             val_max_neu = (val_max_ele*m1+val_min_ele*m2+val_min_max*val_max_diff*m2)/(m1+m2);
             val_min_neu = val_max_neu-val_min_max*val_max_diff;

             /* Neue Werte setzen */
             SetNodeVal(val_max_node, ndx1, val_max_neu);
             SetNodeVal(val_min_node, ndx1, val_min_neu);

             /* Protokoll */
             sum_dist += fabs(val_max_ele - val_max_neu) * m1;
             damping++;
          }
        }
     }
  } while(go_on==1);

  if(damping) {
      DisplayMsg("   Warning: Damping performed ");
      DisplayLong(damping);
      DisplayMsg(" times. Distributed: ");
      DisplayDouble(sum_dist,0,0);
      DisplayMsgLn("");
  }
  return damping;
}

//SB:
/**************************************************************************/
/* ROCKFLOW - Funktion: FctReadHeterogeneousFields
                                      */
/* Aufgabe:
   Liest zu jedem Knoten einen Wert der Permeabilität ein.
   Identifikation über Koordinaten
                                      */
/* Ergebnis:
    0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
    09/2003     SB  First Version
	01/2004     SB  Verallgemeinert auf beliebig viele Parameter
    06/2005     MB  msh / layer 
    08/2005     MB $NUM_TYPE NEW
                                      */
/**************************************************************************/
int FctReadHeterogeneousFields(char *name_file, CMediumProperties *m_mat_mp)
{
  int ok = 0, method;
  double  *convertfact, *values, *defaultvalues, **invals;
  long i, j, no_values, nof, ihet;
  int *help;
  char zeile[MAX_ZEILE], outname[80];
  string line;
  int interpolation_option = 1;
  ifstream ein;
  ofstream out, out1;
  std::stringstream in;
  Element *elem;
  long NumberOfElements;
  CFEMesh* m_msh = NULL;
  int layer = 1;
  int material_properties_index = -1;
  int EleStart = -1;
  int EleEnd = -1;
  int NumberOfLayers = -1;
  long NumberOfElementsPerLayer = -1;

  Mesh_Group::CElem* m_ele = NULL;

  //------------------------------------------------------------------------
  DisplayMsgLn("Input file Heterogeneous Fields ");
  //------------------------------------------------------------------------
  // File handling
  ein.open(name_file);
  if(!ein){
    //DisplayMsgLn(" ERROR opening file with heterogeneous fields!");
	//DisplayMsgLn(" File does not exist.");
    cout << " FctReadHeterogeneousFields" << endl;
    cout << " Cannot find " << name_file << endl;
    exit(0);
  }
  //------------------------------------------------------------------------
  // Read MSH data
  string line_string;
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  in >> line_string;
  in.clear();
  if(line_string.find("$MSH_TYPE")!=string::npos){
    GetLineFromFile(zeile,&ein);
    in.str((string)zeile);
    in >> line_string;
    in.clear();
    m_msh = FEMGet(line_string);
    if(!m_msh){
      cout << "FctReadHeterogeneousFields: no MSH data" << endl;
    }
  }
  //------------------------------------------------------------------------
  // Read Interpolation option
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  in >> line_string;
  in.clear();
  if(line_string.find("$INTERPOLATION")!=string::npos){
    GetLineFromFile(zeile,&ein);
    in.str((string)zeile);
    in >> line_string;
    in.clear();
    if(line_string.find("NEAREST_VALUE")!=string::npos){
      interpolation_option = 1;
    }
    if(line_string.find("GEOMETRIC_MEAN")!=string::npos){
      interpolation_option = 2;
    }  
  }
  //------------------------------------------------------------------------
  /* read number of fields */
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  in >> nof;
  in.clear();
  hf = Createhetfields(nof,name_file);
  //------------------------------------------------------------------------
  /* Speicher */
  convertfact = (double *)Malloc(nof*sizeof(double));
  values = (double *) Malloc(nof * sizeof(double));
  defaultvalues = (double *) Malloc(nof * sizeof(double));
  for(i=0;i<nof;i++){
    convertfact[i]=0.0;
    defaultvalues[i] = 0.0;
  }
  //------------------------------------------------------------------------
  /* read names of heterogeneous fields */
  GetLineFromFile(zeile,&ein);
  line = (string ) zeile;
  in.str(line);
  for(i=0;i<nof;i++){
    in >> outname;
    set_hetfields_name(hf,i,outname);

    //set material_properties_index
    if(line.find("permeability")==0){  
      material_properties_index = 0;
    }
    if(line.find("porosity")==0){  
      material_properties_index = 1;
    }
  }
  in.clear();
  //------------------------------------------------------------------------
  /* read conversion factors */
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  for(i=0;i<nof;i++)
    in >> convertfact[i];
  in.clear();
  //------------------------------------------------------------------------
  // read default
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  for(i=0;i<nof;i++)
    in >> defaultvalues[i];
  in.clear();
  // for(i=0;i<nof;i++)
  //  defaultvalues[i] *= convertfact[i]; MB->SB finde ich eher verwirrend ?
  //------------------------------------------------------------------------
  /* read number of data sets, for which values of the heterogeneous fields are given */
  GetLineFromFile(zeile,&ein);
  in.str((string ) zeile);
  in >> no_values >> method;
  in.clear();

  //------------------------------------------------------------------------
  if(m_msh){
    NumberOfElements = (long)m_msh->ele_vector.size();
  }
  else{
    NumberOfElements = ElListSize();
  }
  
  //------------------------------------------------------------------------
  if(m_msh){
    
    NumberOfLayers = m_msh->no_msh_layer;
    NumberOfElementsPerLayer = NumberOfElements / NumberOfLayers;
  
    //layers
    if(m_mat_mp->geo_type_name.compare("LAYER") == 0){
      char* temp = strdup(m_mat_mp->geo_name.c_str());
      layer = atoi(temp);
      EleStart = (layer - 1) * NumberOfElementsPerLayer;
      EleEnd = layer * NumberOfElementsPerLayer;
    }
    //complete mesh
    if(m_mat_mp->geo_type_name.compare("DOMAIN") == 0){
      layer = 1;
      EleStart = 0;
      EleEnd = NumberOfElementsPerLayer;
    }
    //Warning
    if(no_values < NumberOfElementsPerLayer) {
      DisplayMsgLn("Warning! Fewer element values in File for heterogeneous permeability field than elements in element list");
    }
  } //if(m_msh)..
  //------------------------------------------------------------------------
  /* field (int) for helping sort */
  help = (int *) Malloc(NumberOfElements * sizeof(int));
  for(i=0;i<NumberOfElements;i++)
    help[i] = 0;
 
  /* initialize element values in element list; this is for the case, if not for all
  elements values are given in the input file */
 
 double test1;
 double test2;
 double test;
 if(m_msh){
    for(i=EleStart;i<EleEnd;i++){
      m_ele = m_msh->ele_vector[i];
      if (m_ele->mat_vector.Size() == 0) {
        m_ele->mat_vector.resize(2);
      }
      m_ele->mat_vector(material_properties_index)= defaultvalues[0];
     // test1 = m_ele->mat_vector(0);
    } 
  }

  else{
    for(i=0;i<NumberOfElements;i++){
      for(j=0;j<nof;j++){
	    ELESetHetFieldValues(i, nof, defaultvalues);
      }
    } 
  }

  m_ele = m_msh->ele_vector[0];
  test1 = m_ele->mat_vector(0);
  test2 = m_ele->mat_vector(1);
  
  

  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //METHOD = 0:  read in unsorted values for coordinates and distribute to corresponding elements */
  if(method == 0){ 

    // allocate storage to read in file with het values and initialize
    invals = (double **) Malloc((no_values) * sizeof(double *));
    for(i=0;i<no_values;i++)
      invals[i] = (double *) Malloc((3+nof+1) * sizeof(double));
    // initialize
    for(i=0;i<no_values;i++)
      for(j=0;j<(3+nof+1);j++) //+1 wegen GetAverageHetVal
	    invals[i][j] = 0.0;

    //------------------------------------------------------------------------
    // read values
    for(i=0;i<no_values;i++){
      GetLineFromFile(zeile,&ein);
	  in.str((string) zeile);
      //		in >> x >> y >> z ;
	  in >> invals[i][0] >> invals[i][1] >> invals[i][2];
	  for(j=0;j<nof;j++)
	    in >> invals[i][j+3];
	  in.clear();
    
      // convert values by convertfact
	  for(j=0;j<nof;j++){
	    invals[i][j+3] = invals[i][j+3] * convertfact[j];
	  }
    } // end for read values

    //------------------------------------------------------------------------
    // element loop
    for(i=EleStart;i<EleEnd;i++){
      //.....................................................................
      //Get the values that are nearest to element mid point
 	  if(interpolation_option == 1){ 
        ihet = GetNearestHetVal(i, m_msh, no_values, invals);
        if(ihet<0)
	      DisplayMsgLn(" Error getting nearest het_value location");
	    else{
          for(j=0;j<nof;j++){
	        values[j] = invals[ihet][j+3];
          }
        }
        //DisplayMsg(" Het Val for element: "); DisplayLong(i); DisplayMsg(" with coordinates "); 
	    //DisplayDouble(x,0,0); DisplayMsg(", "); DisplayDouble(y,0,0); DisplayMsg(", "); DisplayDouble(z,0,0); DisplayMsg("       found at: ");
	    //DisplayDouble(invals[ihet][0],0,0); DisplayMsg(", "); DisplayDouble(invals[ihet][1],0,0); DisplayMsg(", "); DisplayDouble(invals[ihet][2],0,0); DisplayMsgLn(". ");
      }
      //.....................................................................
      //Get all values in Element and calculate the geometric mean
      if(interpolation_option == 2){
      
        values[0] = GetAverageHetVal(i, m_msh, no_values, invals);

        DisplayMsgLn(" AverageHetVal ");
        DisplayMsg(" Element ");
        DisplayDouble(i,0,0); 
        DisplayMsg("  Value: ");
        DisplayDouble(values[0],0,0); 
      }

      // save values
      if(m_msh){
        m_ele = m_msh->ele_vector[i];
        m_ele->mat_vector(material_properties_index) = values[0];
      }
      else{
	    ELESetHetFieldValues(i, nof, values);
      }
    
    } //end for element loop
	
    ein.close();
    // free storage for input values
    invals = (double **) Free(invals);

    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    //  OUT   write out fields sorted by element number
    //------------------------------------------------------------------------
    // Header
    sprintf(outname,"%s%i",name_file,1);
    out.open(outname); 
    out << "$MSH_TYPE" << endl;
    out << "  GROUNDWATER_FLOW" << endl;  //ToDo as variable
    //out << "$LAYER" << endl;  
    //out << "  " << layer << endl;
    out << "$INTERPOLATION" << endl;
    out << "  GEOMETRIC_MEAN" << endl;  //ToDo as variable
    /* Field name */
    out << nof << endl;
    for(i=0;i<nof;i++)
      out << get_hetfields_name(hf,i) << ' ';
    out << endl;
    /* conversion factor is one in this case */
    for(i=0;i<nof;i++)
      out << 1.0 << ' ';
    out << endl;
    // default values  
    for(i=0;i<nof;i++)
	  out << defaultvalues[i] << ' ';
    out << endl;
    //out << NumberOfElements << ' ' << 1 << endl;
    out << NumberOfElementsPerLayer << ' ' << 1 << endl;

    out.setf(ios::scientific);
    out.precision(5);

    //------------------------------------------------------------------------
    // Element data
    if(m_msh){
      for(i=EleStart;i<EleEnd;i++){
	    m_ele = m_msh->ele_vector[i];
	    for(j=0;j<nof;j++){
          out << m_ele->mat_vector(material_properties_index) << ' ';	
        }
	    out << endl;
	  }
    }
    else{
	  for(i=0;i<NumberOfElements;i++){
	    elem = ElGetElement(i);
        for(j=0;j<nof;j++){
          out << elem->hetfields[j] << ' ';			
        }
        out << endl;
	  }
    } 

    out.close();

  } /* end if (method == 0) */


  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  //METHOD = 1:  read in one sorted column, index is element number no conversion, no sorting
  if(method == 1){
    for(i=EleStart;i<EleEnd;i++){
	  GetLineFromFile(zeile,&ein);
      in.str((string)zeile);
	  for(j=0;j<nof;j++)
		in >> values[j];
	  in.clear();
      if(m_msh){
        m_ele = m_msh->ele_vector[i];
        m_ele->mat_vector(material_properties_index) = values[0];
        test = m_ele->mat_vector(material_properties_index);
      }
      else{
	    ELESetHetFieldValues(i, nof, values);
      }
	}
  ein.close();
  } /* end if (method == 1) */

  /* data structures deallocate */
  convertfact = (double *) Free(convertfact); 
  values = (double *) Free(values);
  defaultvalues = (double *) Free(defaultvalues);
  help = (int *) Free(help);

  return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetNearestElement
                                      */
/* Aufgabe:
   Bestimmt zu gegebenen Koordinaten das nächstliegende Element,
   indem der Abstand zum Mittelpunkt des Elements berechnet wird.
                                         */
/* Ergebnis:
   NUmmer dieses nächsten Elements
                                      */
/* Programmaenderungen:
    10/2003     SB  First Version
                                      */
/**************************************************************************/
long GetNearestElement(double x,double y,double z, int * help){
	
	long i, nextele;
	double ex, ey, ez, dist, dist1, dist2;
	
	dist = 10000000.0; //Startwert
	dist2 = 0.01;	// Abstand zwischen eingelesenen Knoten und Geometrieknoten-RF; 
					// Achtung, doppelbelegung möglich bei kleinen Gitterabständen
	nextele = -1;
	if(help[0] == 0) nextele = -1;

	for(i=0;i<ElListSize();i++){
//		if (ElGetElement(i)!=NULL){  /* Element existiert */
//		if (help[i]<1){  /* Element noch nicht vergeben  - raus für lokale Gitteradaption */
			ex=ELEGetEleMidPoint(i,0);
			ey=ELEGetEleMidPoint(i,1);
			ez=ELEGetEleMidPoint(i,2);
			dist1 = (ex-x)*(ex-x)+(ey-y)*(ey-y)+(ez-z)*(ez-z);
			if(dist1<dist){
					dist = dist1;
					nextele = i;
			}
//			if(dist<dist2) return nextele;
//		}
	}
	return nextele;
}


/**************************************************************************
MSHLib-Method: GetAverageHetVal
Task: 
Programing:
06/2005 MB Implementation
**************************************************************************/
double GetAverageHetVal(long EleIndex, CFEMesh *m_msh, long no_values, double ** invals){
	
  long i, j, ihet;
  double average;
  double xp[3],yp[3];
  double value;
  double NumberOfValues;
  double InvNumberOfValues;
  CGLPoint *m_point = NULL;
  Mesh_Group::CElem* m_ele = NULL;
  
  j = 0; //only for 1 value

  //-----------------------------------------------------------------------
  //Get element data
  m_ele = m_msh->ele_vector[EleIndex];
  for(j=0;j<3; j++) {
    xp[j] = m_ele->GetNode(j)->X();
    yp[j] = m_ele->GetNode(j)->Y();
    //zp[j] = 0.0;
  }

  //-----------------------------------------------------------------------
  //Find data points in the element
  NumberOfValues = 0;
  InvNumberOfValues = 0; 
  m_point = new CGLPoint;
 
  average = -1;
  value = 0;

  for(i=0;i<no_values;i++){
    if(invals[i][4]!= -999999.0){ //Data point not within an element yet
      m_point->x = invals[i][0];
      m_point->y = invals[i][1];
      m_point->z = 0.0;
 
      //....................................................................
      //Calculate the product of values in element
      if(m_point->IsInTriangleXYProjection(xp,yp)) {//CC 10/05
        value = value + invals[i][3];
        NumberOfValues ++;
        invals[i][4] = -999999.0; //used as marker
      }
    }
  }//end for
  //........................................................................
  if(NumberOfValues == 0){ //if no data points in element --> get neares value
    ihet = GetNearestHetVal(EleIndex, m_msh, no_values, invals);
    if(ihet<0)
	  DisplayMsgLn(" Error getting nearest het_value location");
	else{
      average = invals[ihet][j+3];     
    }
  }
  //........................................................................
  else{ //if data points in element --> Calculate arithmetic mean
    average = value / NumberOfValues;
  }
  delete m_point;
  return average;
}


/**************************************************************************
MSHLib-Method: GetAverageHetVal
Task: 
Programing:
0?/2004 SB Implementation
09/2005 MB EleClass
**************************************************************************/
long GetNearestHetVal(long EleIndex, CFEMesh *m_msh, long no_values, double ** invals){
	
  long i, nextele;
  double ex, ey, ez, dist, dist1, dist2;
  double x, y, z;
  double* center = NULL;
  Mesh_Group::CElem* m_ele = NULL;
  
  //----------------------------------------------------------------------
  // MB ToDo
  //EleIndex = -1;
  //m_msh = NULL;
  //----------------------------------------------------------------------

  x=0.0; y=0.0; z=0.0;
  dist = 10000000.0; //Startwert
  dist2 = 0.01;	    // Abstand zwischen eingelesenen Knoten und Geometrieknoten-RF; 
					// Achtung, doppelbelegung möglich bei kleinen Gitterabständen
  nextele = -1;

  //Get element data
  m_ele = m_msh->ele_vector[EleIndex];
  center = m_ele->GetGravityCenter();
  x = center[0];
  y = center[1];
  z = center[2];
  
  //Calculate distances  
  for(i=0;i<no_values;i++){
    ex=invals[i][0];
    ey=invals[i][1];
	ez=invals[i][2];
	dist1 = (ex-x)*(ex-x)+(ey-y)*(ey-y)+(ez-z)*(ez-z);
	if(dist1<dist){
	  dist = dist1;
	  nextele = i;
	}
  }
  
  return nextele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetLineFromFile
                                      */
/* Aufgabe:
   Liest aus dem Eingabefile für die heterogenen Felder die nächste ZEile
   Fängt die Zeile mit ; an, wird sie ausgelassen
   SB:todo
                                         */
/* Ergebnis:
   Nächste Zeile aus dem Eingabefile
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
 int GetLineFromFile(char *zeile, ifstream *ein){
 
 int ok = 1;
 string line;
 int fertig=0, i, j;

 while(fertig < 1){
	 if(ein->getline(zeile,MAX_ZEILE)){				//Zeile lesen
	line = zeile;							//character in string umwandeln
	i = (int) line.find_first_not_of(" ",0);		//Anfängliche Leerzeichen überlesen, i=Position des ersten Nichtleerzeichens im string
	j = (int) line.find(";",i) ;					//Nach Kommentarzeichen ; suchen. j = Position des Kommentarzeichens, j=-1 wenn es keines gibt.
	if(j!=i)fertig = 1;				//Wenn das erste nicht-leerzeichen ein Kommentarzeichen ist, zeile überlesen. Sonst ist das eine Datenzeile
	 }
	 else{ //end of file found
		 ok=0;
		 fertig=1;
	 }
}//while
return ok;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Createhetfields
                                      */
/* Aufgabe:
   Erstellt die Datenstruktur zu den heterogenen Feldern
 und initialisiert diese                                   */
/* Ergebnis:
    -
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
hetfields * Createhetfields(int n, char* name_file){

 int i;
 hetfields * hf;

 hf = (hetfields *) Malloc(sizeof(hetfields));
 hf->nof = n;
 hf->filename = name_file;
 hf->convertfact = (double *)Malloc(n*sizeof(double));
 hf->names = (char **) Malloc(n * sizeof(char *));
 for(i=0;i<n;i++)
  hf->names[i] = (char *) Malloc(256 * sizeof(char));
 
 return hf;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: set_hetfields_name
                                      */
/* Aufgabe:
   set Funktion zur Struktur hetfields
   gesetzt wird ein Name aus dem Eingabefile
                                         */
/* Ergebnis:
    -
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
void set_hetfields_name(hetfields *hf,int i, char * name){
 strcpy( hf->names[i], name);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: get_hetfields_name
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
char* get_hetfields_name(hetfields *hf,int i){
 return hf->names[i];
}

/**************************************************************************/
/* ROCKFLOW - Funktion: get_hetfields_number
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
int get_hetfields_number(hetfields *hf){
 return hf->nof;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetHetValue
                                      */
/* Aufgabe:
   Liest zu einen übergebenen Namen den enstrechenden Wert aus 
   der elementstruktur het_fields aus.
   
                                      */
/* Ergebnis:
    value = Ausgelesener Wert
                                      */
/* Programmaenderungen:
    01/2004     SB  First Version
                                   */
/**************************************************************************/
double GetHetValue(int ele_no, char * inname ){

int i, n;
char *name;
double value;
int material_properties_index = 0;
CFEMesh* m_msh = NULL;
n = get_hetfields_number(hf);
m_msh = FEMGet("GROUNDWATER_FLOW");
for(i=0;i<n;i++){
 name = get_hetfields_name(hf,i);
 if(strstr(inname, name) && (strcmp(inname,name) == 0)){
  /* found */
//  value = ELEGetHetFieldValue(ele_no,i);
	 value =  m_msh->ele_vector[ele_no]->mat_vector(material_properties_index);
  return value;
 }
}
 DisplayMsgLn(" Error - No het. Field Value found");
 return -1.0;
}


//SB ende

/**************************************************************************
   ROCKFLOW - Function: TOLSortNodes1
   
   Task:  
   Sort nodes descending according to the criterium. 
         
   Parameter: (I: Input; R: Return; X: Both)
           I: long* nodes, double* criterium, int anz
           
   Return:
           *long nodes (aber sortiert!)
   
   Programming:
   09/2002   MB   First Version
 **************************************************************************/


/**************************************************************************
   ROCKFLOW - Function: SortDataSet
   
   Task:  
   Sort data set1 and set2 descending according to the criterium. 
         
   Parameter: (I: Input; R: Return; X: Both)
           I: long* nodes, double* criterium, int anz
           
   Return:
           *long nodes (aber sortiert!)
   
   Programming:
   05/2003   MB   First Version  based on GeoSortNodes
 **************************************************************************/
void TOLSortDataSet(double* set1, double* set2, double* criterium, int anz)
{
  int flag=1; 
  int i;
  int nummer=0;
  double tempset1;
  double tempset2;
  double temp;

  do  {
    flag = 0;
    nummer++;
    for (i=0; i < (anz - nummer); i++)  {
      if ( criterium[i] < criterium[i+1])  {
        flag = 1;
        tempset1 = set1[i];
        tempset2 = set2[i];
        temp = criterium[i];
        set1[i] = set1[i+1];
        set2[i] = set2[i+1];
        criterium[i] = criterium[i+1];
        set1[i+1] = tempset1;
        set2[i+1] = tempset2;
        criterium[i+1] = temp;
      } /* end if */
    } /* end for */
  } while (flag == 1);
}

