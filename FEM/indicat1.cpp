/**************************************************************************/
/* ROCKFLOW - Modul: indicat1.c
                                                                          */
/* Aufgabe:
   -Heuristische- Fehlerindikatoren zum Verfeinern und Vergroebern.
   Der Fehlerindikator wird ueber die Text-Eingabedatei gewaehlt. Vorgabe
   ist Indicator1. Der Indikator bestimmt fuer alle aktiven Elemente, ob
   ein Element verfeinert oder vergroebert werden soll. Die Stimmigkeit
   des Netzes (sekundaere Verfeinerungskriterien) wird hier nicht
   ueberprueft. Der Indikator traegt in das "verfeinern" - Feld der
   Elementdatenstruktur folgende Werte ein:
     0 : Element weder verfeinern noch vergroebern
         (Vorgabe, muss nicht explizit gesetzt werden)
     1 : Element soll verfeinert werden
     2 : Element soll vergroebert werden
                                                                          */
/* Programmaenderungen:
   12/1994     MSR         Modul angelegt
   12/1995     MSR         Indikator1
   01/1995     cb          korrigiert
   07/1995     Rene Kaiser Indikator2
   01/1996     cb          I1,I2 erweitert, Indikator3
   03/1996     cb          indicat1 und indicat2
   01.07.1997  R.Kaiser    ROCKFLOW-Anpassung (aTM -> ROCKFLOW)
   04.08.1998  R.Kaiser    Adaption fuer mehrere Loesungsgroessen
   03/2003     RK          Quellcode bereinigt, Globalvariablen entfernt   
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "elements.h"
#include "nodes.h"
#include "edges.h"
#include "mathlib.h"
#include "adaptiv.h"
#include "rf_mmp_new.h"

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator0
                                                                          */
/* Aufgabe:
   hier passiert nichts
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ: Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)

                                                                          */
/* Ergebnis:
   double
                                                                          */
/* Programmaenderungen:
   03/1996     cb         Erste Version
                                                                          */
/**************************************************************************/
double Indicator0 ( long index, int typ, int idx0, int idx1 )
{
  long dummy2; /* dummy */
  int dummy;

  dummy2 = index;
  dummy = typ;
  dummy = idx0;
  dummy = idx1;

  return -1.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator1
                                                                          */
/* Aufgabe:
   Verfeinerungsindikator:
   (gemittelte) Differenzen der Ergebniswerte eines Elements
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ: Element-Dimension - 1
   E int idx0: Index des Knotenwertes
   E int idx1: Index des Knotenwertes
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   double indicator_value: Indicator_Wert
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   01/1995     cb         2d eingebaut
   01/1996     cb         3d eingebaut
   03/1996     cb         bezug auf Maximalwert
   01.07.1997  R.Kaiser   Rockflow-Anpassung (aTM -> ROCKFLOW)
   09.07.1999  R.Kaiser   Ueberarbeitet
                                                                          */
/**************************************************************************/
double Indicator1 ( long index, int typ, int idx0, int idx1 )
{
  static int dummy;
  static int j;  /* Laufvariable */
  static double node_values[8];  /* Feld fuer Ergebniswerte an max. 8 Elementknoten */
  static long *element_nodes;  /* Zeiger auf long-Feld, fuer Elementknoten */
  static int number_nodes;  /* Anzahl der Elementknoten: 2, 4 oder 8 */
  static double indicator_value; /* Indicator-Wert */
  static double average_value; /* Mittelwert */
  static double C = 1.0; /* 2.5;  Konstante */

  dummy = idx0;

  element_nodes = ElGetElementNodes(index);
  number_nodes = ElNumberOfNodes[typ];

  average_value = 0.0;
  for (j=0;j<number_nodes;j++) {
    node_values[j] = GetNodeVal(element_nodes[j],idx1);
    average_value += node_values[j];
  }
  average_value /= (double)number_nodes;

  indicator_value = 0.0;
  for (j=0;j<number_nodes;j++)
    indicator_value += fabs(node_values[j] - average_value);
  indicator_value = C * indicator_value;

  return (indicator_value);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator2
                                                                          */
/* Aufgabe:
   Verfeinerungsindikator:
   Gradient der Ergebniswerte im Mittelpunkt eines Elements
   skaliert an einer massgebenden Elementlaenge [John,1994]
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ: Element-Dimension - 1
   E int idx0: Index des Knotenwertes
   E int idx1: Index des Knotenwertes
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   double indicator_value: Indikator-Wert
                                                                          */
/* Programmaenderungen:
   08/1995     R.Kaiser        Erste Version
   01/1996     cb              umgestellt
   01.07.1997  R.Kaiser        Rockflow-Anpassung (aTM -> ROCKFLOW)
   08.07.1999  R.Kaiser        Ueberarbeitet

   TODO: Massgebende Elementlaenge bei 2d-Elementen
                                                                          */
/**************************************************************************/
double Indicator2 ( long index, int typ, int idx0, int idx1 )
{
  static int dummy;
  static int j;  /* Laufvariable */
  static double node_values[8];  /* Feld fuer Ergebniswerte an max. 8 Elementknoten */
  static long *element_nodes;  /* Zeiger auf Elementknoten */
  static int number_nodes;  /* Anzahl der Elementknoten: 2, 4 oder 8 */
  static double grad_rs[2], grad_rst[3]; /* Gradienten der Ergebniswerte im rs-System/rst-System */
  static double grad_ab[2], grad_xyz[3]; /* Gradienten der Ergebniswerte im ab-System/xyz-System */
  static double gradient; /* Gradient der Ergebniswerte im Element (Betrag) */
  static double vf[24]; /* Vektorfelder (max. 3*8) */
  static double indicator_value; /* Indicator-Wert */
  static double *invjac;
  static double detjac;
  static double delta_s; /* charakteristische Laenge */
  static double C=1.; /* Konstante */
  /*static double C2=0.01;*/ /* System auf 1 beziehen */
  static double area;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  dummy = idx0;

  element_nodes = ElGetElementNodes(index);  /* Elementknoten */
  number_nodes = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */

  area = m_mmp->geo_area;

  for (j=0;j<number_nodes;j++)
    node_values[j] = GetNodeVal(element_nodes[j],idx1);
  invjac = ADAPGetElementJacobi(index,&detjac);

  switch (typ) {
    case 0:
      gradient = 0.5 * fabs(node_values[1] - node_values[0]) * MBtrgVec(invjac,3);

      /* Elementlaenge */
      /* delta_s = C2 * ElGetElementVolume(index) / area; */
      delta_s = ElGetElementVolume(index) / area;    
      break;

    case 1:
      MGradOmega2D(vf,0.0,0.0);
      MMultMatVec(vf,2,4,node_values,4,grad_rs,2); /* r,s */
      MMultVecMat(grad_rs,2,invjac,2,2,grad_ab,2); /* a,b */
      gradient = MBtrgVec(grad_ab,2);

      /* sqrt(maximale Elementlaenge) */
      for (j=0;j<4;j++)
        vf[j] = invjac[j];
      M2InvertiereUndTransponiere(vf);  /* MK_bitte_pruefen!!! */
      vf[0] = 2.0 * (vf[0] + vf[1]);
      vf[1] = 2.0 * (vf[2] + vf[3]);
      /* delta_s = sqrt(C2 * max(fabs(vf[0]),fabs(vf[1]))); */
      delta_s = sqrt(ElGetElementVolume(index)/area);      
      break;

    case 2:
      MGradOmega3D(vf,0.0,0.0,0.0);
      MMultMatVec(vf,3,8,node_values,8,grad_rst,3); /* r,s,t */
      MMultMatVec(invjac,3,3,grad_rst,3,grad_xyz,3); /* x,y,z */
      gradient = MBtrgVec(grad_xyz,3);

      /* ohne Vorfaktor */
      /* delta_s = 1.0; */
      delta_s = pow(ElGetElementVolume(index),0.3333);     
      break;
  }
  indicator_value = C * delta_s * gradient;
  return (indicator_value);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator3
                                                                          */
/* Aufgabe:
   Fehlerindikator auf Basis von `Kruemmungen' der Loesungskurve, d.h.
   max. Sprung der am Knoten gemittelten Gradienten der Ergebniswerte
   fehler = max. | [dc_i/dn] | skaliert an einer massgebenden Element-
   laenge (nach [John,1994])
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ: Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   double dgc: Element-Fehler
                                                                          */
/* Programmaenderungen:
   01/1996     C.Barlag        Erste Version
   02/1996     cb              Korrektur
   03/1996     cb              mal Elementlaenge
   10/1996     cb              ... wieder raus, Bloedsinn
   11/1996     cb              doch kein Bloedsinn
   01.07.1997  R.Kaiser        Rockflow-Anpassung (aTM -> ROCKFLOW)
   09.07.1999  R.Kaiser        Ueberarbeitet
   07/2000     R.Kaiser        Indikator 3d, charakteristische Elementlänge
                                                                          */
/**************************************************************************/
double Indicator3 ( long index, int typ, int idx0, int idx1 )
{
  static int dummy;
  static int j,k,l; /* Laufvariablen */
  static int anz;
  static int number_nodes;  /* Anzahl der Elementknoten */
  static long *elems; /* Elemente am Knoten */
  static long *element_nodes;  /* Elementknoten von index */
  static long *element_nodes_neighbour;  /* Elementknoten Nachbarelement */
  static double vf[24]; /* Vektorfelder (max. 3x8) */
  static double trans[6];
  static double node_values[8];  /* Feld fuer Ergebniswerte (max. 8) */
  static double grad_r;
  static double grad_rs[2], grad_ab[2];
  static double grad_rst[3], gradient[3], gradient_neighbour[3];
  static double jump[3];
  static double jump_average_max, jump_average;
  static double delta_s; /* charakteristische Elementlaenge */
  static double detjac,*invjac; /* Inverse Jakobi-Matrix (max. 3x3) */
  static double C=1.0; /* Konstante */
  /*static double C2=0.01;*/ /* System auf 1 beziehen */
  static double area;
  static double indicator_value;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  area = m_mmp->geo_area;

  dummy = idx0;

  element_nodes = ElGetElementNodes(index); /* Knoten */

  number_nodes = ElNumberOfNodes[typ];

  /* eigener Gradient -> gc */
  for (l=0;l<number_nodes;l++)
    node_values[l] = GetNodeVal(element_nodes[l],idx1);
  invjac = ADAPGetElementJacobi(index,&detjac);

  switch (typ) {
    case 0:
      grad_r = 0.5 * (node_values[1] - node_values[0]); /* r */
      for (l=0;l<3;l++)
        gradient[l] = grad_r * invjac[l]; /* x,y,z */
      /* sqrt(Elementlaenge) */
      /* delta_s = sqrt(C2 * ElGetElementVolume(index)/area); */
      delta_s = sqrt(ElGetElementVolume(index)/area);  
      break;

    case 1:
      /* T * (J^-1 * grad(omega) * c) */
      ADAPCalcTransMatrix(index,vf,trans);
      MGradOmega2D(vf,0.0,0.0);
      MMultMatVec(vf,2,4,node_values,4,grad_rs,2); /* r,s */
      MMultVecMat(grad_rs,2,invjac,2,2,grad_ab,2); /* a,b */
      MMultMatVec(trans,3,2,grad_ab,2,gradient,3); /* x,y,z */
      /* delta_s=1.0; ohne Vorfaktor */
      delta_s = sqrt(ElGetElementVolume(index)/area);
      break;

    case 2:
      /* J^-1 * grad(omega) * c */
      MGradOmega3D(vf,0.0,0.0,0.0);
      MMultMatVec(vf,3,8,node_values,8,grad_rst,3); /* r,s,t */
      MMultMatVec(invjac,3,3,grad_rst,3,gradient,3); /* x,y,z */

      /* 1./sqrt(maximale Elementlaenge) */
      for (j=0;j<9;j++)
        vf[j]=invjac[j];
      M3Invertiere(vf);
      vf[0] = 2.0 * (vf[0] + vf[1] + vf[2]);
      vf[1] = 2.0 * (vf[3] + vf[4] + vf[5]);
      vf[2] = 2.0 * (vf[6] + vf[7] + vf[8]);
      /*delta_s = 1.0/sqrt(C2 * max(max(fabs(vf[0]),fabs(vf[1])),fabs(vf[2])));*/
      delta_s = pow(ElGetElementVolume(index),0.3333);
      break;
  }


  jump_average_max = 0.0;
  for (j=0;j<number_nodes;j++) {
    jump_average = 0.0;
    elems = GetNodeXDElems[typ](element_nodes[j],&anz);
    for (k=0;k<anz;k++)
      if (elems[k] != index) {
        /* Gradienten der Nachbarn am Knoten j -> gn */
        element_nodes_neighbour = ElGetElementNodes(elems[k]);
        for (l=0;l<number_nodes;l++)
          node_values[l] = GetNodeVal(element_nodes_neighbour[l],idx1);
        /* Gradient im Mittelpunkt des Elements */
        invjac = ADAPGetElementJacobi(elems[k],&detjac);

        switch (typ) {
          case 0:
            grad_r = 0.5 * (node_values[1] - node_values[0]); /* r */
            for (l=0;l<3;l++)
              gradient_neighbour[l] = grad_r * invjac[l]; /* x,y,z */
            break;

          case 1:
            /* T * (J^-1 * grad(omega) * c) */
            ADAPCalcTransMatrix(elems[k],vf,trans);
            MGradOmega2D(vf,0.0,0.0);
            MMultMatVec(vf,2,4,node_values,4,grad_rs,2); /* r,s */
            MMultVecMat(grad_rs,2,invjac,2,2,grad_ab,2); /* a,b */
            MMultMatVec(trans,3,2,grad_ab,2,gradient_neighbour,3); /* x,y,z */
            break;

          case 2:
            /* J^-1 * grad(omega) * c */
            MGradOmega3D(vf,0.0,0.0,0.0);
            MMultMatVec(vf,3,8,node_values,8,grad_rst,3); /* r,s,t */
            MMultMatVec(invjac,3,3,grad_rst,3,gradient_neighbour,3); /* x,y,z */
            break;
        }
        /* mittlerer Gradientensprung aller angreifenden Elemente */
        for (l=0;l<3;l++)
          jump[l] = gradient[l] - gradient_neighbour[l];
        jump_average += MBtrgVec(jump,3);
      }
    if (anz > 1)
      jump_average = jump_average/(double)(anz-1);
    else
      jump_average = 0.0; /* MBtrgVec(gradient,3); */

    jump_average_max = max(jump_average_max,jump_average);
  }

  indicator_value = C * jump_average_max * delta_s;

  return (indicator_value);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator4
                                                                          */
/* Aufgabe:
   Fehlerindikator auf der Basis der charakteristischen Laenge der Elemente
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ: Element-Dimension - 1
   E int idx0: Nicht benutzt
   E int idx1: Nicht benutzt
                                                                          */
/* Ergebnis:
   double dgc: Element-Fehler
                                                                          */
/* Programmaenderungen:
   09/1999     C.Thorenz
                                                                          */
/**************************************************************************/
double Indicator4 ( long index, int typ, int idx0, int idx1)
{
  double ds=0.;
  double area;
  int dummy;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  dummy = idx0;
  dummy = idx1;

  area = m_mmp->geo_area;

  switch (typ) {
      case 0:
          ds = ElGetElementVolume(index)/area;
          break;
      case 1:
          ds = sqrt(ElGetElementVolume(index)/area);
          break;
      case 2:
          ds = pow(ElGetElementVolume(index),0.3333);
          break;
  }
  /*  printf("fehler=%e\n",ds);  */
  return ds;
}


