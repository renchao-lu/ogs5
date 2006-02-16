/**************************************************************************/
/* ROCKFLOW - Modul: pst_flow.c
                                                                          */
/* Aufgabe:
   Funktionen fuer das Postprozessing
                                                                          */
/* Programmaenderungen:
   05/1997     R.Kaiser     Erste Version (1D, 2D) (cflux_asm.c)
   27.05.1997  OK           Fallunterscheidung fuer SM und GM
   28.05.1997  R.Kaiser     Ueberarbeitet (1D, 2D) und korrigiert
   09.06.1997  R.Kaiser     Umbenannt (cflux_asm.c -> pst_flow.c)
   16.09.1997  O.Kolditz    case = 1 (nichtlineare Fliessgesetze)
   29.08.1998  AH           Korrektur bei 2D
   30.08.1998  AH           Korrektur bei 3D
   10/99       AH           GM (3D)
   10/99       AH           RSM
   03/2003     RK           Quellcode bereinigt, Globalvariablen entfernt   
     

   letzte Aenderung     RK   12.03.2003
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTSM

/* Header / Andere intern benutzte Module */
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "femlib.h"
#include "cvel.h"
#include "rfmodel.h"

#include "int_asm.h"
#include "int_agm.h"
#include "int_rsm.h"

/* material properties */
#include "material.h"
/* ok */
#include "geo_strings.h"



typedef void (*VoidFuncLongFileX) (long, FILE *);

static double zaehler;

/* Interne (statische) Deklarationen */
void CalcMassFlux1D(long index, FILE * f);
void CalcMassFlux2D(long index, FILE * f);
void CalcMassFlux3D(long index, FILE * f);

/* Funktionszeiger auf die 1D-, 2D-, 3D- Berechnungsfunktionen
   fuer die Massenstroeme */
static VoidFuncLongFileX CalcMassFlux[3] = \
{CalcMassFlux1D, CalcMassFlux2D, CalcMassFlux3D};


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcMassFlux_ASM
                                                                          */
/* Aufgabe:
   Berechnung des Mittelpunktes des Elements und der zugehoerigen
   Geschwindigkeit, der Massenstroeme
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1997     R.Kaiser   Erste Version
                                                                          */
/**************************************************************************/
void CalcMassFlux_ASM(FILE * f)
{
    static int i;
    zaehler = 0.0;
    for (i = 0; i < ElListSize(); i++)
        if (ElGetElement(i) != NULL)    /* Element existiert */
            if (ElGetElementActiveState(i)) {   /* aktives Element */
                if (f == NULL) {
                    printf("pstprz.rf kann nicht geoeffnet werden \n");
                } else {
                    CalcMassFlux[ElGetElementType(i) - 1] (i, f);
                }
            }
    DisplayMsg("Massenbilanz: ");
    DisplayDouble(zaehler, 0, 0);
    DisplayMsgLn("");
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalcMassFlux1D
                                                                          */
/* Aufgabe:
   Berechnung der Darcygeschwindigkeiten im Elementmittelpunkt
   und der Massenstroeme ueber den Gebietsrand (1D-Elemente)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1997     R.Kaiser     Erste Version
   27.05.1997  OK           Fallunterscheidung fuer SM und GM
   28.05.1997  R.Kaiser     Ueberarbeitet und korrigiert
   09.06.1997  R.Kaiser     Berechnung der Elementvolumen entfernt
   11/1997     O.Kolditz    Nichtlineare Fliessgesetze
   10/99       AH           RSM

   letzte Aenderung   OK   30.11.97
                                                                          */
/**************************************************************************/
void CalcMassFlux1D(long index, FILE * f)
{
    /* Elementgeometriedaten */
    static long *nodes;
    static Knoten *k0;
    static Knoten *k1;
    static double XMiddle, YMiddle, ZMiddle;
    /* Knotendaten */
    static double velo[3];
    static double flux[3];
    /* Materialdaten */
    static double area;
    /* static double T_0; */
    static double rho;

/*  static FILE *testfile;
  testfile = fopen("test.dat","w");
*/

    /* Elementgeometriedaten bereitstellen */
    nodes = ElGetElementNodes(index);
    k0 = GetNode(nodes[0]);
    k1 = GetNode(nodes[1]);

    XMiddle = 0.5 * (GetNodeX(nodes[0]) + GetNodeX(nodes[1]));
    YMiddle = 0.5 * (GetNodeY(nodes[0]) + GetNodeY(nodes[1]));
    ZMiddle = 0.5 * (GetNodeZ(nodes[0]) + GetNodeZ(nodes[1]));

    /* Materialparameter bereitstellen */
    area = GetElementExchangeArea(index);
    rho = GetFluidDensity(0, index, 0., 0., 0., 0.);

    CalcVeloXDrst(0, index, 0., 0., 0., 0., velo);

    /* Massenstrom im Elementmittelpunkt berechnen */
    flux[0] = velo[0] * rho * area;
    flux[1] = velo[1] * rho * area;
    flux[2] = velo[2] * rho * area;

    /***************************************************************************/
    /* Speichern der berechneten Elementdaten zum spaeteren Protokollieren     */
    switch (GetRFControlModel()) {
    case 0:                     /* SM */
        ASMSetElementXMiddle(index, XMiddle);
        ASMSetElementYMiddle(index, YMiddle);
        ASMSetElementZMiddle(index, ZMiddle);
        ASMSetElementXVelo(index, velo[0]);
        ASMSetElementYVelo(index, velo[1]);
        ASMSetElementZVelo(index, velo[2]);
        ASMSetElementXFlux(index, flux[0]);
        ASMSetElementYFlux(index, flux[1]);
        ASMSetElementZFlux(index, flux[2]);
        break;
    case 1:                     /* SM nichtlineare Flie´gesetze */
        ASMSetElementXMiddle(index, XMiddle);
        ASMSetElementYMiddle(index, YMiddle);
        ASMSetElementZMiddle(index, ZMiddle);
        ASMSetElementXVelo(index, velo[0]);
        ASMSetElementYVelo(index, velo[1]);
        ASMSetElementZVelo(index, velo[2]);
        ASMSetElementXFlux(index, flux[0]);
        ASMSetElementYFlux(index, flux[1]);
        ASMSetElementZFlux(index, flux[2]);
        break;
    case 2:                     /* GM */
        AGMSetElementXMiddle(index, XMiddle);
        AGMSetElementYMiddle(index, YMiddle);
        AGMSetElementZMiddle(index, ZMiddle);
        AGMSetElementXVelo(index, velo[0]);
        AGMSetElementYVelo(index, velo[1]);
        AGMSetElementZVelo(index, velo[2]);
        AGMSetElementXFlux(index, flux[0]);
        AGMSetElementYFlux(index, flux[1]);
        AGMSetElementZFlux(index, flux[2]);
        break;
    case 7:                     /* RM */
        RSMSetElementXMiddle(index, XMiddle);
        RSMSetElementYMiddle(index, YMiddle);
        RSMSetElementZMiddle(index, ZMiddle);
        RSMSetElementXVelo(index, velo[0]);
        RSMSetElementYVelo(index, velo[1]);
        RSMSetElementZVelo(index, velo[2]);
        RSMSetElementXFlux(index, flux[0]);
        RSMSetElementYFlux(index, flux[1]);
        RSMSetElementZFlux(index, flux[2]);
        break;

    default:                    /* darf nicht auftreten */
        FilePrintString(f," !!! Fehler CalcMassFlux1: Modell nicht eingetragen "); LineFeed(f);
        DisplayErrorMsg("CalcMassFlux1D: Modell nicht eingetragen !");
        abort();
    }
    return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcMassFlux2D
                                                                          */
/* Aufgabe:
   Berechnung der Darcygeschwindigkeiten im Elementmittelpunkt
   und der Massenstroeme ueber den Gebietsrand (2D-Elemente)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
                                                                          */

/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1997     R.Kaiser     Erste Version
   27.05.1997  OK           Fallunterscheidung fuer SM und GM
   28.05.1997  R.Kaiser     Ueberarbeitet und korrigiert
   09.06.1997  R.Kaiser     Massenfluesse ueber die Gebietsraender
   31.10.1997  R.Kaiser     Je Kante beliebige Anzahl an 2D-Nachbarelementen
                            (fuer Kluftnetzwerke)
   11/1997     O.Kolditz    Nichtlineare Fliessgesetze
   09.03.1998  R.Kaiser     Ueberarbeitet
   29.08.1998  AH           Korrektur
   10/99       AH           RSM

   letzte Aenderung   A.H.  29.08.1998
                                                                          */
/**************************************************************************/
void CalcMassFlux2D(long index, FILE * f)
{
    /* Laufvariablen und Hilfsfelder */
    static int i, j;
    static double vf[12];       /* Vektorfeld */
    /* Elementgeometriedaten */
    static long *nodes;
    static long *edges;
    static double edge_middle[3], rs[2], ab[2], edge_length, edge[3];
    static double element_middle[3];
    static long nd[2];
    static double n[3];
    static double trans[6];     /* Drehmatrix */
    /* Knotendaten */
    static double v[2];
    static double velo[3], vel[3], vel_n;
    static double flux[3];
    static double massflux;
    /* Materialdaten */
    static double area;
    /* static double T_0; */
    static double rho;
    static double element_thickness;


    /* Elementgeometriedaten bereitstellen */
    nodes = ElGetElementNodes(index);
    edges = ElGetElementEdges(index);

    /* Elementmaterialdaten bereitstellen */
    area = GetElementExchangeArea(index);
    rho = GetFluidDensity(0, index, 0., 0., 0., 0.);

    /* Berechnung der Geschwindigkeitsvektoren im a,b - System */
    CalcVeloXDrst(0, index, 0., 0., 0., 0., v);

    /* Transformation der Geschwindigkeitsvektoren
       in das globale physikalische Koordinatensystem (x,y,z) */
    /* Koordinatentransformatiomsmatrix [2D_T_3D] und
       Knotenkoordinatenmatrix [A_3D]^T berechnen */
    CalcTransformationsMatrix(index, vf, trans);
    /* [2D_T_3D] {v'} = {v} */
    MMultMatVec(trans, 3, 2, v, 2, velo, 3);

    /* Berechnung des Elementmittelpunktes */
    element_middle[0] = (vf[3] + vf[0] + vf[9] + vf[6]) / 4;
    element_middle[1] = (vf[4] + vf[1] + vf[10] + vf[7]) / 4;
    element_middle[2] = (vf[5] + vf[2] + vf[11] + vf[8]) / 4;

    /* Massentrom im Elementmittelpunkt berechnen - ??? Flaeche */
    flux[0] = velo[0] * rho;    /* * element_thickness; */
    flux[1] = velo[1] * rho;
    flux[2] = velo[2] * rho;

    /* Berechnung der Massenfluesse ueber die Gebietsraender */
    if (max_dim == 1) {         /* maximal 2D-Elemente */
        for (j = 0; j < 4; j++) {       /* Schleife ueber die Elementkanten */
            /* Ueberpruefen, ob Randkante (Kante und Vorgaengerkante haben
               maximal ein zugehoeriges 2D-Element) */
            if (((GetEdge(edges[j])->anz_nachbarn_2D == 1) && \
                 (GetEdge(edges[j])->vorgaenger == -1)) || \
                ((GetEdge(edges[j])->anz_nachbarn_2D == 1) && \
                 (GetEdge(edges[j])->vorgaenger >= 0) && \
                 (GetEdge(GetEdge(edges[j])->vorgaenger)->anz_nachbarn_2D == 1))) {

                Calc2DEdgeUnitNormalVec(index, edges[j], n);

                for (i = 0; i < 2; i++) {
                    nd[i] = GetEdge(edges[j])->knoten[i];
                }

                /* Berechnung der x,y,z - Koordinaten der Kantenmitte */
                edge_middle[0] = (GetNode(nd[0])->x + GetNode(nd[1])->x) / 2;
                edge_middle[1] = (GetNode(nd[0])->y + GetNode(nd[1])->y) / 2;
                edge_middle[2] = (GetNode(nd[0])->z + GetNode(nd[1])->z) / 2;

                /* Berechnung der Kantenlaenge */
                edge[0] = (GetNode(nd[0])->x - GetNode(nd[1])->x);
                edge[1] = (GetNode(nd[0])->y - GetNode(nd[1])->y);
                edge[2] = (GetNode(nd[0])->z - GetNode(nd[1])->z);
                edge_length = MBtrgVec(edge, 3);

                /* Transformation der Koordinaten in das r,s - System */
                Calc2DElement_xyz2ab_Coord(index, ab, edge_middle[0], edge_middle[1], edge_middle[2]);
                Calc2DElement_ab2rs_Coord(index, rs, ab[0], ab[1]);

                /* Geschwindigkeit im a,b - System berechnen */
                CalcVeloXDrst(0, index, 0., rs[0], rs[1], 0., v);

                /* Berechnung der Massenfluesse */
                /* Geschwindigkeit im x,y,z - System */
                CalcTransformationsMatrix(index, vf, trans);    /* x,y,z */
                MMultMatVec(trans, 3, 2, v, 2, vel, 3);

                /* Geschwindigkeit in Normalenrichtung berechnen */
                vel_n = MSkalarprodukt(vel, n, 3);

                /* Massenfluss ueber Randkante berechnen */
                massflux = rho * element_thickness * edge_length * vel_n;
                zaehler += massflux;

                fprintf(f, "%ld ", index);
                fprintf(f, "%f %f %f ", GetNode(nd[0])->x, GetNode(nd[0])->y, GetNode(nd[0])->z);
                fprintf(f, "%f %f %f ", GetNode(nd[1])->x, GetNode(nd[1])->y, GetNode(nd[1])->z);
                fprintf(f, "%g\n", massflux);
            }
        }                       /* Ende Schleife ueber Elementkanten */
    }
    /* Speichern der berechneten Elementdaten zum spaeteren Protokollieren */
    switch (GetRFControlModel()) {
    case 0:                     /* SM */
        ASMSetElementXMiddle(index, element_middle[0]);
        ASMSetElementYMiddle(index, element_middle[1]);
        ASMSetElementZMiddle(index, element_middle[2]);
        ASMSetElementXVelo(index, velo[0]);
        ASMSetElementYVelo(index, velo[1]);
        ASMSetElementZVelo(index, velo[2]);
        ASMSetElementXFlux(index, flux[0]);
        ASMSetElementYFlux(index, flux[1]);
        ASMSetElementZFlux(index, flux[2]);
        break;
    case 1:                     /* SM */
        ASMSetElementXMiddle(index, element_middle[0]);
        ASMSetElementYMiddle(index, element_middle[1]);
        ASMSetElementZMiddle(index, element_middle[2]);
        ASMSetElementXVelo(index, velo[0]);
        ASMSetElementYVelo(index, velo[1]);
        ASMSetElementZVelo(index, velo[2]);
        ASMSetElementXFlux(index, flux[0]);
        ASMSetElementYFlux(index, flux[1]);
        ASMSetElementZFlux(index, flux[2]);
        break;
    case 2:                     /* GM */
        AGMSetElementXMiddle(index, element_middle[0]);
        AGMSetElementYMiddle(index, element_middle[1]);
        AGMSetElementZMiddle(index, element_middle[2]);
        AGMSetElementXVelo(index, velo[0]);
        AGMSetElementYVelo(index, velo[1]);
        AGMSetElementZVelo(index, velo[2]);
        AGMSetElementXFlux(index, flux[0]);
        AGMSetElementYFlux(index, flux[1]);
        AGMSetElementZFlux(index, flux[2]);
        break;
    case 7:                     /* RM */
        RSMSetElementXMiddle(index, element_middle[0]);
        RSMSetElementYMiddle(index, element_middle[1]);
        RSMSetElementZMiddle(index, element_middle[2]);
        RSMSetElementXVelo(index, velo[0]);
        RSMSetElementYVelo(index, velo[1]);
        RSMSetElementZVelo(index, velo[2]);
        RSMSetElementXFlux(index, flux[0]);
        RSMSetElementYFlux(index, flux[1]);
        RSMSetElementZFlux(index, flux[2]);
        break;

    default:                    /* darf nicht auftreten */
        DisplayErrorMsg("pst_flow.c: Modell nicht eingetragen !");
        abort();
    }
    return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcMassFlux3D
                                                                          */
/* Aufgabe:
   Berechnung der Darcygeschwindigkeiten im Elementsmittelpunkt
   (3D-Elemente)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements, dessen Gauss-Geschwindigkeiten
                  berechnet werden sollen
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   27.07.1997     R.Kaiser   Erste Version
   10/99          AH         GM (3D)
   10/99          AH         RSM

   letzte Aenderung      R.Kaiser     27.07.1997
                                                                          */
/**************************************************************************/
void CalcMassFlux3D(long index, FILE * f)
{
    static int i;
    static double X1, X2, XMiddle, Y1, Y2, YMiddle, Z1, Z2, ZMiddle;
    static double rho;
    static long *nodes;
    static double koma[24];
    static double vf[12],velo[3],flux[3];
    static Knoten *nd;

    nodes = ElGetElementNodes(index);

    rho = GetFluidDensity(0, index, 0., 0., 0., 0.);

    for (i = 0; i < 8; i++) {
        nd = GetNode(nodes[i]);
        koma[i * 3] = nd->x;
        koma[i * 3 + 1] = nd->y;
        koma[i * 3 + 2] = nd->z;
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
    XMiddle = X1 + 0.5 * (X2 - X1);
    YMiddle = Y1 + 0.5 * (Y2 - Y1);
    ZMiddle = Z1 + 0.5 * (Z2 - Z1);

    CalcVeloXDrst(0, index, 0., 0., 0., 0., velo);

    /* Massenstrom im Elementmittelpunkt berechnen */
    flux[0] = velo[0] * rho;
    flux[1] = velo[1] * rho;
    flux[2] = velo[2] * rho;

    switch (GetRFControlModel()) {
    case 0:                     /* SM */
        ASMSetElementXMiddle(index, XMiddle);
        ASMSetElementYMiddle(index, YMiddle);
        ASMSetElementZMiddle(index, ZMiddle);
        ASMSetElementXVelo(index, velo[0]);
        ASMSetElementYVelo(index, velo[1]);
        ASMSetElementZVelo(index, velo[2]);
        ASMSetElementXFlux(index, flux[0]);
        ASMSetElementYFlux(index, flux[1]);
        ASMSetElementZFlux(index, flux[2]);
        break;
    case 2:                     /* GM */
        AGMSetElementXMiddle(index, XMiddle);
        AGMSetElementYMiddle(index, YMiddle);
        AGMSetElementZMiddle(index, ZMiddle);
        AGMSetElementXVelo(index, velo[0]);
        AGMSetElementYVelo(index, velo[1]);
        AGMSetElementZVelo(index, velo[2]);
        AGMSetElementXFlux(index, flux[0]);
        AGMSetElementYFlux(index, flux[1]);
        AGMSetElementZFlux(index, flux[2]);
        break;
    case 7:                     /* RM */
        RSMSetElementXMiddle(index, XMiddle);
        RSMSetElementYMiddle(index, YMiddle);
        RSMSetElementZMiddle(index, ZMiddle);
        RSMSetElementXVelo(index, velo[0]);
        RSMSetElementYVelo(index, velo[1]);
        RSMSetElementZVelo(index, velo[2]);
        RSMSetElementXFlux(index, flux[0]);
        RSMSetElementYFlux(index, flux[1]);
        RSMSetElementZFlux(index, flux[2]);
        break;
    default:                    /* darf nicht auftreten */
        FilePrintString(f," !!! Fehler CalcMassFlux1: Modell nicht eingetragen "); LineFeed(f);
        DisplayErrorMsg("CalcMassFlux3D: Modell nicht eingetragen !");
        abort();
    }

    FilePrintString(f,"; CalcMassFlux3D: Ausgabe fuer 3-D Elemente ");
    LineFeed(f);

}
