/**************************************************************************/
/* ROCKFLOW - Modul: indicat2.c
                                                                          */
/* Aufgabe:
   -Analytische- Fehlerindikatoren zum Verfeinern und Vergroebern
   fuer das ROCKFLOW-aTM
                                                                          */
/* Programmaenderungen:
   03/1996     cb          indicat1 und indicat2
   01.07.1997  R.Kaiser    Rockflow_Anpassung (aTM -> ROCKFLOW)
   04.08.1998  R.kaiser    Adaption fuer mehrere Loesungsgroessen
                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
/* Header / Andere intern benutzte Module */
#include "indicatr.h"
#include "elements.h"
#include "nodes.h"
#include "edges.h"
#include "mathlib.h"
#include "adaptiv.h"
#include "femlib.h"
#include "intrface.h"
#include "memory.h"
#include "cvel.h"
#include "tools.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
extern double gravity_constant;

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_tran1
                                                                          */
/* Aufgabe:
   Residuenindikator fuer die Poisson-Gleichung nach Bieterman & Babuska
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0:   Index des Knotenwertes (hier: conc0)
   E int idx1:   Index des Knotenwertes (hier: conc1)
                 (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   04/1996     cb              Erste Version
   01.07.1997  R.Kaiser        Rockflow-Anpassung (aTM -> ROCKFLOW)
   03/2003     RK              Quellcode bereinigt, Globalvariablen entfernt   

                                                                          */
/**************************************************************************/
double Indicator_tran1(long index, int typ, int idx0, int idx1)
{
    static double error;        /* Fehler */
    static double fehler_zeit;  /* Fehler des Zeitanteils */
    static double fehler_spr;   /* Fehler aufgrund der Spruenge */
    static long i, j, k, m, ii;
    static double *velovec;     /* Zeiger auf die Gauss-Geschwindigkeiten */
    static double *matrix;
    static long *elems;
    static long *knoten;        /* Zeiger auf die Elementknoten */
    static long *kno;
    static double *invjac;
    static long nn;             /* Anzahl Knoten pro Element */
    static double conc1;        /* Knotenkonzentrationen (neu) */
    static double conc0;        /* Knotenkonzentrationen (alt) */
    static double wert1[4], wert2[4], erg[12], sum[3];
    static double r, s, t;
    static double v, d, l[2];
    static double detjac, zwi[12], trans[6];
    static long anzgp;
    static int anz, z;
    static double g[4], h, det, inv[4], w[4], gc[3];
    static long *elekanten;
    static Kante *kante, *elek;
    static long *ele;
    static long *nachbarele;
    static int anzahl, anznachbarn;
    static Knoten *dk1, *dk2;
    static double ds, theta;
    static long kk[2];
    static double C = 3.5e+3;   /* sqrt(1./12.) */
    static double area,porosity;
  static double mass_dispersion_longitudinal,mass_dispersion_transverse,molecular_diffusion;
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  area = m_mmp->geo_area;
  porosity = m_mmp->Porosity(index,NULL,theta);
  mass_dispersion_longitudinal = m_mmp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mmp->mass_dispersion_transverse;
    if (typ == 2) {
        DisplayMsgLn("Indikator 4 fuer 3D-Elemente nicht implementiert");
        return 0.0;
    }
    nn = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */
    knoten = ElGetElementNodes(index);  /* Knoten des Elements index */
/*    theta = ADAPGetThetaTransport(); */
    theta = GetNumericalTimeCollocation("TRANSPORT");
    for (j = 0; j < nn; j++) {  /* Ergebnisse des */
        conc0 = GetNodeVal(knoten[j], idx0);        /* letzten Zeitschritts */
        conc1 = GetNodeVal(knoten[j], idx1);        /* jetzigen Zeitschritts */
        wert1[j] = (conc1 - conc0) / dt;        /* Zeitableitung */
        wert2[j] = theta * conc1 + (1.0 - theta) * conc0;       /* Ortsableitung */
    }

    /* Integral ueber das Residuum des Zeitanteils */
    /* (n*retard)^2 int ( omega omega ) ds * (dc/dt)^2 */
    /* vereinfacht: n * retard * Massenintegral * (dc/dt)^2 */
    matrix = ADAPGetElementMassMatrix(index);
    MMultMatVec(matrix, nn, nn, wert1, nn, erg, nn);
    fehler_zeit = porosity * ADAPGetElementRetardFac(index) * \
        MSkalarprodukt(wert1, erg, nn);

    /* Integral ueber die Spruenge senkrecht zur Elementkante */
    /* n^2 * d^2 / l * sprung^2 * kante */
    MNulleVec(g, nn);
    switch (typ) {
    case 0:
        /* Hilfswert fuer Vorfaktor */
        z = 3;
        /* Elementlaenge */
/*        l[0] = ADAPGetElementVolume(index) / area; */
        l[0] = ElGetElementVolume(index) / area;
        /* Spruenge in den Knoten */
        invjac = GetElementJacobiMatrix(index, &detjac);
        h = 0.5 * (wert2[1] - wert2[0]);
        for (j = 0; j < 3; j++)
            gc[j] = h * invjac[j];      /* bezogen auf x,y,z (global) */
        for (j = 0; j < nn; j++) {
            MNulleVec(sum, 3);
            elems = GetNode1DElems(knoten[j], &anz);
            for (i = 0; i < anz; i++)
                if (elems[i] != index) {
                    /* Knoten des Elements i am Knoten j */
                    kno = ElGetElementNodes(elems[i]);
                    for (k = 0; k < nn; k++) {
                        conc0 = GetNodeVal(kno[k], idx0);
                        conc1 = GetNodeVal(kno[k], idx1);
                        w[k] = theta * conc1 + (1.0 - theta) * conc0;
                    }
                    Calc1DElementJacobiMatrix(elems[i], inv, &det);
                    h = 0.5 * (w[1] - w[0]);
                    for (k = 0; k < 3; k++)
                        sum[k] += (gc[k] - h * inv[k]); /* Summe der Spruenge */
                }
            /* Betrag der gemittelten Spruenge in den Knoten */
            if (anz > 1)
                g[j] = MBtrgVec(sum, 3) / (double) (anz - 1);
        }
        /* printf("sprung=%e %e\n",g[0],g[1]); */
        /* Quadrat der elementgemittelten halben Spruenge */
        fehler_spr = 0.125 * MSkalarprodukt(g, g, nn);
        break;
    case 1:
        /* Hilfswert fuer Vorfaktor */
        z = 2;
        /* Elementlaenge */
        /* dr(lokal) = J * dx(global) */
        invjac = GetElementJacobiMatrix(index, &detjac);
        for (j = 0; j < 4; j++)
            zwi[j] = invjac[j];
        M2InvertiereUndTransponiere(zwi);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jakobi-Matrix */
        l[0] = 2.0 * (zwi[0] + zwi[1]);
        l[1] = 2.0 * (zwi[2] + zwi[3]);
        /* Kantenfehler */
        /* Spruenge in den Knoten senkrecht ueber die Kante */
        elekanten = ElGetElementEdges(index);
        for (j = 0; j < 4; j++) {
            kante = GetEdge(elekanten[j]);
            anznachbarn = 0;


            if (kante->anz_nachbarn_2D == 1) {
                if (kante->vorgaenger == -1)    /* Kante hat keinen Vorgaenger */
                    /* Randkante, kein Nachbarelement, kein Sprung */
                    continue;
                else if (kante->vorgaenger >= 0)        /* Kante hat Vorgaenger */
                    if (GetEdge(kante->vorgaenger)->anz_nachbarn_2D == 1)
                        /* Randkante, kein Nachbarelement, kein Sprung */
                        continue;
            }
            if (kante->kinder[0] == -1) {       /* Kante hat keine Kinder */
                if (kante->vorgaenger == -1) {  /* Kante hat keinen Vorgaenger */
                    /* Nachbarelemente haben gleichen level */
                    elek = kante;
                    nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                    ele = (long *) Malloc(anzahl * sizeof(long));
                    anznachbarn = anzahl;
                    for (ii = 0; ii < anzahl; ii++)
                        ele[ii] = nachbarele[ii];
                } else if (kante->vorgaenger >= 0) {     /* Kante hat Vorgaenger */
                    if (kante->anz_nachbarn_2D == GetEdge(kante->vorgaenger)->anz_nachbarn_2D) {
                        /* Nachbarelemente haben gleichen level */
                        elek = kante;
                        nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                        ele = (long *) Malloc(anzahl * sizeof(long));
                        anznachbarn = anzahl;
                        for (ii = 0; ii < anzahl; ii++)
                            ele[ii] = nachbarele[ii];
                    } else if (kante->anz_nachbarn_2D != GetEdge(kante->vorgaenger)->anz_nachbarn_2D) {
                        /* Nachbarelemente mit niedrigerem Level */
                        elek = GetEdge(kante->vorgaenger);
                        nachbarele = GetEdge2DElems(kante->vorgaenger, &anzahl);
                        ele = (long *) Malloc(anzahl * sizeof(long));
                        anznachbarn = anzahl;
                        for (ii = 0; ii < anzahl; ii++)
                            ele[ii] = nachbarele[ii];
                    }
                 }
            } else if (kante->kinder[0] >= 0) {
                /* Kante hat Kinder (-> Nachbarelemente mit hoeherem Level) */
                elek = kante;
                nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                ele = (long *) Malloc(anzahl * sizeof(long));
                anznachbarn = anzahl;
                for (ii = 0; ii < anzahl; ii++)
                    ele[ii] = nachbarele[ii];
            }
            MNulleVec(zwi, 12);
            for (k = 0; k < anznachbarn; k++) { /* ele[0] bis ele[anznachbarn] */
                /* Konzentrationen an ele[k] */
                kno = ElGetElementNodes(ele[k]);
                for (i = 0; i < nn; i++) {
                    conc0 = GetNodeVal(kno[i], idx0);
                    conc1 = GetNodeVal(kno[i], idx1);
                    w[i] = theta * conc1 + (1.0 - theta) * conc0;
                }
                /* printf("w=%e %e %e %e\n",w[0],w[1],w[2],w[3]); */
                /* lokale Koordinaten der Kantenknoten */
                kk[0] = kk[1] = -1;
                for (i = 0; i < 4; i++) {       /* Knoten an ele[k] */
                    if (kno[i] == elek->knoten[0])
                        kk[0] = i;
                    else if (kno[i] == elek->knoten[1])
                        kk[1] = i;
                }
                /* printf("kk=%d %d\n",kk[0],kk[1]); */

                for (i = 0; i < 2; i++) {       /* Gradient im Knoten kk[i] des Elements ele[k] */
                    MGetCoor(typ, kk[i], &r, &s, &t);
                    Calc2DElementJacobiMatrix(ele[k], r, s, invjac, &detjac);
                    ADAPCalcTransMatrix(ele[k], erg, trans);
                    MGradOmega2D(erg, r, s);
                    MMultMatVec(erg, 2, nn, w, nn, gc, 2);      /* r,s */
                    MMultVecMat(gc, 2, invjac, 2, 2, erg, 2);   /* a,b */
                    MMultMatVec(trans, 3, 2, erg, 2, gc, 3);    /* x,y,z */

                    for (m = 0; m < 3; m++)
                        zwi[6 * k + 3 * i + m] = gc[m]; /* Speichern */
                    /* printf("gc=%e %e %e\n",gc[0],gc[1],gc[2]); */
                }
            }
            /* Kantenlaenge */
            dk1 = GetNode(elek->knoten[0]);
            dk2 = GetNode(elek->knoten[1]);
            ds = sqrt((dk1->x - dk2->x) * (dk1->x - dk2->x)
                      + (dk1->y - dk2->y) * (dk1->y - dk2->y)
                      + (dk1->z - dk2->z) * (dk1->z - dk2->z));
            /* printf("ds=%e\n",ds); */
            /* halbe Spruenge quadrieren, integrieren */
            for (k = 0; k < 2; k++)
                for (i = 0; i < 3; i++)
                    zwi[3 * k + i] -= zwi[6 + 3 * k + i];
            /* MZeigVec(zwi,6,"d grad c"); */
            g[j] = ds * 0.125 * (zwi[0] * zwi[0] + zwi[1] * zwi[1] + zwi[2] * zwi[2]
                  + zwi[3] * zwi[3] + zwi[4] * zwi[4] + zwi[5] * zwi[5]);
        }
        /* printf("int(0.5 sprung)^2=%e %e %e %e\n",g[0],g[1],g[2],g[3]); */
        fehler_spr = g[0] + g[1] + g[2] + g[3];
        break;
    }
/*    anzgp = (long) pow((double) ElGetGPNum(index), (double) (typ + 1));*/
    anzgp = (long) pow((double)GetNumericsGaussPoints(ElGetElementType(index)), (double) (typ + 1));

    velovec = ElGetVelocity(index);
    for (j = 0; j < z; j++) {
        erg[j] = 0.0;
        for (k = 0; k < anzgp; k++)
            erg[j] += velovec[z * k + j] / porosity;
        erg[j] /= (double) anzgp;
    }
    v = MBtrgVec(erg, z);
    d = molecular_diffusion + \
          v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) + \
                   (mass_dispersion_transverse * mass_dispersion_transverse));
    /* l * d * n^2 */
    fehler_spr *= (d * porosity * porosity * MBtrgVec(l, (typ + 1)));

    /* h^2 / d */
    fehler_zeit *= (MSkalarprodukt(l, l, (typ + 1)) / d);

    /* printf("zeit=%e spr=%e fkt=%e\n",fehler_zeit,fehler_spr,fkt); */
    error = C * sqrt(fehler_zeit + fehler_spr);

    ele = (long *)Free(ele);

    return error;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_tran2
                                                                          */
/* Aufgabe:
   Fehlerindikator in Anlehnung an einen Residuenindikator
   fuer parabolische Differentialgleichungen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */

/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   04/1996     cb              Erste Version
   28.07.1997  R.Kaiser        Rockflow-Anpassung (aTM -> ROCKFLOW)

   letzte Aenderung      R.Kaiser      28.07.1997
                                                                          */
/**************************************************************************/
double Indicator_tran2(long index, int typ, int idx0, int idx1)
{
    static double error;        /* Fehler */
    static double fehler_disp;  /* Fehler des Dispersionsanteils */
    static double fehler_res;   /* Residuum-Fehler */
    static double fehler_spr;   /* Fehler aufgrund der Spruenge */
    static long i, j, k, m;
    static double *velovec;     /* Zeiger auf die Gauss-Geschwindigkeiten */
    static double *matrix;
    static long *elems;
    static long *knoten;        /* Zeiger auf die Elementknoten */
    static long *kno;
    static long nn;             /* Anzahl Knoten pro Element */
    static double conc1;        /* Knotenkonzentrationen (neu) */
    static double conc0;        /* Knotenkonzentrationen (alt) */
    static double wert1[8], wert2[8], erg[24], sum[3];
    static double r, s, t, theta;
    static double v, d, l;
    static double invjac[9], detjac, zwi[12], trans[6];
    static long anzgp;
    static int anz, z;
    static double fkt, g[8], h, inv[9], det, w[8], sp[3], gc[3];

    static double area,porosity;
    static double molecular_diffusion,mass_dispersion_longitudinal,mass_dispersion_transverse;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  area = m_mmp->geo_area;
  porosity = m_mmp->Porosity(index,NULL,theta);
  mass_dispersion_longitudinal = m_mmp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mmp->mass_dispersion_transverse;
    nn = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */
    knoten = ElGetElementNodes(index);  /* Knoten des Elements index */
    theta = GetNumericalTimeCollocation("TRANSPORT");
/*    theta = ADAPGetThetaTransport(); */

    for (j = 0; j < nn; j++) {  /* Ergebnisse des */
        conc0 = GetNodeVal(knoten[j], idx0);        /* letzten Zeitschritts */
        conc1 = GetNodeVal(knoten[j], idx1);        /* jetzigen Zeitschritts */
        wert1[j] = (conc1 - conc0) / dt;        /* Zeitableitung */
        wert2[j] = theta * conc1 + (1.0 - theta) * conc0;       /* Ortsableitung */
    }

    /* Massenintegral */
    matrix = ADAPGetElementMassMatrix(index);
    MMultMatVec(matrix, nn, nn, wert1, nn, erg, nn);

    /* Dispersionsintegral */
    matrix = ADAPGetElementDispMatrix(index);
    /* einschl. Rand */
    /* n * disp * sprung^2 * d */
    switch (typ) {
    case 0:
        /* Hilfswerte fuer Vorfaktor */
        z = 3;
/*        l = ADAPGetElementVolume(index) / area; OK rf3230 */
        l = ElGetElementVolume(index) / area;    /* Elementlaenge */
        /* Dispersionsintegral */
        fehler_disp = 0.0;
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        Calc1DElementJacobiMatrix(index, invjac, &detjac);
        h = 0.5 * (wert2[1] - wert2[0]);
        for (j = 0; j < 3; j++)
            gc[j] = h * invjac[j];      /* bezogen auf x,y,z (global) */
        for (j = 0; j < nn; j++) {
            MNulleVec(sum, 3);
            elems = GetNode1DElems(knoten[j], &anz);
            for (i = 0; i < anz; i++)
                if (elems[i] != index) {
                    /* Knoten des Elements i am Knoten j */
                    kno = ElGetElementNodes(elems[i]);
                    for (k = 0; k < nn; k++) {
                        conc0 = GetNodeVal(kno[k], idx0);
                        conc1 = GetNodeVal(kno[k], idx1);
                        w[k] = theta * conc1 + (1.0 - theta) * conc0;
                    }
                    Calc1DElementJacobiMatrix(elems[i], inv, &det);
                    h = 0.5 * (w[1] - w[0]);
                    for (k = 0; k < 3; k++)
                        sum[k] += (gc[k] - h * inv[k]); /* Summe der Spruenge */
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e\n",g[0],g[1]); */
        break;
    case 1:
        /* Hilfswerte fuer Vorfaktor */
        z = 2;
/*        l = sqrt(ADAPGetElementVolume(index) / area); OK rf3230 */
        l = sqrt(ElGetElementVolume(index) / area);      /* Elementlaenge */
        /* Dispersionsintegral */
        /* Elementfehler */
        MMultMatVec(matrix, nn, nn, wert2, nn, zwi, nn);
        fehler_disp = MSkalarprodukt(zwi, zwi, nn);
        for (j = 0; j < nn; j++)
            erg[j] += zwi[j];
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        for (j = 0; j < nn; j++) {
            /* Gradient im Knoten j des Elements index */
            MGetCoor(typ, j, &r, &s, &t);
            Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
            ADAPCalcTransMatrix(index, zwi, trans);
            MGradOmega2D(erg, r, s);
            MMultMatVec(erg, 2, nn, wert2, nn, gc, 2);  /* r,s */
            MMultVecMat(gc, 2, invjac, 2, 2, erg, 2);   /* a,b */
            MMultMatVec(trans, 3, 2, erg, 2, gc, 3);    /* x,y,z */
            elems = GetNode2DElems(knoten[j], &anz);
            MNulleVec(sum, 3);  /* Spruenge in den Knoten */
            for (k = 0; k < anz; k++)
                if (elems[k] != index) {
                    kno = ElGetElementNodes(elems[k]);
                    for (i = 0; i < nn; i++) {
                        conc0 = GetNodeVal(kno[i], idx0);
                        conc1 = GetNodeVal(kno[i], idx1);
                        w[i] = theta * conc1 + (1.0 - theta) * conc0;
                    }
                    MNulleVec(sp, 3);
                    for (i = 0; i < nn; i++)
                        if (kno[i] == knoten[j]) {
                            /* Gradient im Knoten i des Elements k */
                            MGetCoor(typ, i, &r, &s, &t);
                            Calc2DElementJacobiMatrix(elems[k], r, s, inv, &det);
                            ADAPCalcTransMatrix(elems[k], zwi, trans);
                            MGradOmega2D(erg, r, s);
                            MMultMatVec(erg, 2, nn, w, nn, sp, 2);      /* r,s */
                            MMultVecMat(sp, 2, inv, 2, 2, erg, 2);      /* a,b */
                            MMultMatVec(trans, 3, 2, erg, 2, sp, 3);    /* x,y,z */
                            /* Sprung zwischen index und elems[k] in i */
                            for (m = 0; m < 3; m++)
                                sp[m] = gc[m] - sp[m];
                            break;
                        }
                    for (i = 0; i < 3; i++)
                        sum[i] += sp[i];
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e %e %e\n",g[0],g[1],g[2],g[3]); */
        break;
    case 2:
        /* Hilfswerte fuer Vorfaktor */
        z = 3;
/*        l = pow(ADAPGetElementVolume(index), Mdrittel); OK rf3230 */
        l = pow(ElGetElementVolume(index), Mdrittel); /* Elementlaenge */
        /* Dispersionsintegral */
        /* Elementfehler */
        MMultMatVec(matrix, nn, nn, wert2, nn, zwi, nn);
        fehler_disp = MSkalarprodukt(zwi, zwi, nn);
        for (j = 0; j < nn; j++)
            erg[j] += zwi[j];
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        for (j = 0; j < nn; j++) {
            /* Gradient im Knoten j des Elements index */
            MGetCoor(typ, j, &r, &s, &t);
            Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
            MGradOmega3D(erg, r, s, t);
            MMultMatVec(erg, 3, nn, wert2, nn, trans, 3);       /* r,s,t */
            MMultMatVec(invjac, 3, 3, trans, 3, gc, 3); /* x,y,z */
            elems = GetNode3DElems(knoten[j], &anz);
            MNulleVec(sum, 3);  /* Spruenge in den Knoten */
            for (k = 0; k < anz; k++)
                if (elems[k] != index) {
                    kno = ElGetElementNodes(elems[k]);
                    for (i = 0; i < nn; i++) {
                        conc0 = GetNodeVal(kno[i], idx0);
                        conc1 = GetNodeVal(kno[i], idx1);
                        w[i] = theta * conc1 + (1.0 - theta) * conc0;
                    }
                    MNulleVec(sp, 3);
                    for (i = 0; i < nn; i++)
                        if (kno[i] == knoten[j]) {
                            /* Gradient im Knoten i des Elements k */
                            MGetCoor(typ, i, &r, &s, &t);
                            Calc3DElementJacobiMatrix(elems[k], r, s, t, inv, &det);
                            MGradOmega3D(erg, r, s, t);
                            MMultMatVec(erg, 3, nn, w, nn, trans, 3);   /* r,s,t */
                            MMultMatVec(inv, 3, 3, trans, 3, sp, 3);    /* x,y,z */
                            /* Sprung zwischen index und elems[k] in i */
                            for (m = 0; m < 3; m++)
                                sp[m] = gc[m] - sp[m];
                            break;
                        }
                    for (i = 0; i < 3; i++)
                        sum[i] += sp[i];
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e %e %e %e %e %e %e \n", \
        g[0],g[1],g[2],g[3],g[4],g[5],g[6],g[7]); */
        break;
    }

    /* Vorfaktor */
/*    anzgp = (long) pow((double) ElGetGPNum(index), (double) (typ + 1));*/
    anzgp = (long) pow((double)GetNumericsGaussPoints(ElGetElementType(index)), (double) (typ + 1));

    velovec = ElGetVelocity(index);
    for (j = 0; j < z; j++) {
        erg[j] = 0.0;
        for (k = 0; k < anzgp; k++)
            erg[j] += velovec[z * k + j] / porosity;
        erg[j] /= (double) anzgp;
    }
    v = MBtrgVec(erg, z);
    d = molecular_diffusion + \
        v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) + \
        (mass_dispersion_transverse * mass_dispersion_transverse));
    /* Randintegral */
    MMultMatVec(matrix, nn, nn, g, nn, erg, nn);
    fehler_spr = MSkalarprodukt(erg, g, nn) * d * porosity;
    /* Elementfehler */
    fkt = l * l / d;
    /* printf("\n disp=%e res=%e spr=%e fkt=%e",\
            fehler_disp,fehler_res,fehler_spr,fkt); */
    error = sqrt((fehler_res + fehler_spr) * fkt);
    return error;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_tran3
                                                                          */
/* Aufgabe:
   Fehlerindikator fuer hyperbolische Dgln. nach Johnson [1990]
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   Elementfehler
                                                                          */
/* Programmaenderungen:
   09/1996      cb              Erste Version
   28.07.1997   R.Kaiser        Rockflow-Anpassung (aTM -> ROCKFLOW)      */

/**************************************************************************/
double Indicator_tran3(long index, int typ, int idx0, int idx1)
{
    static double grad_rs[2], grad_ab[2], gradient[3],gradient_neighbour[3];
    static double node_values[8];  /* Feld fuer Ergebniswerte (max. 8) */
    static double jump_average_max, jump_average;    
    static long *element_nodes_neighbour;  /* Elementknoten Nachbarelement */
    static double jump[3];
    static double error;        /* Fehler */
    static double fehler1, fehler2; 
    static double fehler_res;   /* Masse und Advektion */
    static double residuum;
    static double *velovec;
    static double *invjac, *inv;
    static int i, j, k, m;
    static long *elems;
    static long *knoten;
    static long *kno;
    static long nn;
    static double conc1, conc0, wert1[4], wert2[4], theta;
    static double zwi[12], zwo[12], trans[6];
    static double vf[24]; /* Vektorfelder (max. 3x8) */    
    static double detjac, volumen, mue, epsilon;
    static int anz, z, randele;
    static double v, vvec[2], d, dd, l, vt, spm;
    static long anzgp, anzele;
    static double h, det, w[8], gc[3];
    static long *ele;
    static double C1 = 1.0;     /* 0.1; Konstante C1 */
    static double C2 = 1.0;     /* 1.3e+7; Konstante C2 */

    static double area,porosity;
    static double molecular_diffusion,mass_dispersion_longitudinal,mass_dispersion_transverse;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  area = m_mmp->geo_area;
  porosity = m_mmp->Porosity(index,NULL,theta);
  mass_dispersion_longitudinal = m_mmp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mmp->mass_dispersion_transverse;
    if (typ == 2) {
        DisplayMsgLn("Indikator 6 fuer 3D-Elemente nicht implementiert");
        return 0.0;
    }
    nn = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */
    knoten = ElGetElementNodes(index);  /* Knoten des Elements index */
    theta = GetNumericalTimeCollocation("TRANSPORT");


    for (j = 0; j < nn; j++) {  /* Ergebnisse des */
        conc0 = GetNodeVal(knoten[j], idx0);        /* letzten Zeitschritts */
        conc1 = GetNodeVal(knoten[j], idx1);        /* jetzigen Zeitschritts */
        wert1[j] = (conc1 - conc0) / dt;        /* Zeitableitung */
        wert2[j] = theta * conc1 + (1.0 - theta) * conc0;       /* Ortsableitung */
    }

  
   
    z = typ + 1;
    if (typ == 0)
        z = 3;

    anzgp = (long) pow((double)GetNumericsGaussPoints(ElGetElementType(index)), (double) (typ + 1));
    velovec = ElGetVelocity(index);
    for (j = 0; j < z; j++) {
        vvec[j] = 0.0;
        for (k = 0; k < anzgp; k++)
            vvec[j] += velovec[z * k + j] / porosity;
        vvec[j] /= (double) anzgp;
    }
    v = MBtrgVec(vvec, z);

    /* ohne kuenstliche Diffusion */
    d = molecular_diffusion + v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) \
                              +(mass_dispersion_transverse * mass_dispersion_transverse));

  
    volumen = ElGetElementVolume(index);
    invjac = GetElementJacobiMatrix(index, &detjac);

    randele = 0;
    switch (typ) {
    case 0:
      /* Elementlaenge */
      l = volumen / area;
      /* Anzahl aktive Elemente */
      anzele = anz_active_1D;
      
      /* Randelement ? */
      if ((GetNodeState(knoten[0]) == -3) || (GetNodeState(knoten[1]) == -3))
        randele = 1;

      /* Residuum im Mittelpunkt des Elements */
      /* Masse */
      /* n * R * Omega(0) * wert1 */
      fehler1 = porosity * ADAPGetElementRetardFac(index) \
        * 0.5 * (wert1[0] + wert1[1]);
        
      /* Advektion */
      /* n * vt * grad Omega(+1,-1) * wert2 */
      vt = MSkalarprodukt(velovec, invjac, 3) / porosity;
      fehler2 = porosity * vt * 0.5 * (wert2[1] - wert2[0]);

    
      fehler_res = fehler1 + fehler2;
       

      /* Betrag */
      residuum = error = fabs(fehler_res);

      /* Spruenge in den Knoten */
      h = 0.5 * (d + ElGetArtDiff(index)) * (wert2[1] - wert2[0]);
      for (j = 0; j < 3; j++)
        gc[j] = h * invjac[j];      /* bezogen auf x,y,z (global) */
      spm = 0.0;
      for (j = 0; j < nn; j++) {
        elems = GetNode1DElems(knoten[j], &anz);
        for (i = 0; i < anz; i++)
          if (elems[i] != index) {
            porosity = m_mmp->Porosity(index,NULL,theta);
            v = MBtrgVec(ElGetVelocity(elems[i]), 3) / porosity;
            dd = molecular_diffusion + ElGetArtDiff(elems[i]) \
                 + v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) \
                 + (mass_dispersion_transverse * mass_dispersion_transverse));
            /* Knoten des Elements i am Knoten j */
            kno = ElGetElementNodes(elems[i]);
            for (k = 0; k < nn; k++) {
              conc0 = GetNodeVal(kno[k], idx0);
              conc1 = GetNodeVal(kno[k], idx1);
              w[k] = theta * conc1 + (1.0 - theta) * conc0;
            }
            inv = GetElementJacobiMatrix(elems[i], &det);
            h = 0.5 * dd * (w[1] - w[0]);
            for (k = 0; k < 3; k++)
              zwo[k] = gc[k] - h * inv[k];    /* Sprung am Knoten j */
            spm = max(spm, MBtrgVec(zwo, 3));   /* Maximalwert */
          }
      }
      residuum += (spm / l);
      break;
    case 1:
      l = sqrt(volumen / area);
    
      /* Anzahl aktive Elemente */
      anzele = anz_active_2D;
      /* Randelement ? */
      if ((GetNodeState(knoten[0]) == -3) ||
          (GetNodeState(knoten[1]) == -3) ||
          (GetNodeState(knoten[2]) == -3) ||
          (GetNodeState(knoten[3]) == -3))
        randele = 1;

      /* Residuum im Elementmittelpunkt */
      /* Masse */
      /* n * R * Omega(0,0) * wert1 */
      MOmega2D(zwo, 0.0, 0.0);
      fehler_res = porosity * ADAPGetElementRetardFac(index) * \
                   MSkalarprodukt(zwo, wert1, 4);

      /* Advektion */
      /* n * v * (J^-1)T * grad Omega * wert2 */
      /* vt = v * (J^-1)T = J^-1 * v */
      MMultMatVec(invjac, 2, 2, vvec, 2, gc, 2);
      MGradOmega2D(zwi, 0.0, 0.0);
      MMultVecMat(gc, 2, zwi, 2, 4, zwo, 4);
      fehler_res += (porosity * MSkalarprodukt(zwo, wert2, 4));

      residuum = error = fabs(fehler_res);

      /* Kantenfehler */
     
      
      /* eigener Gradient -> gc */
      for (i=0;i<nn;i++)
        node_values[i] = theta * GetNodeVal(knoten[i],idx1) 
                         + (1.0 - theta) * GetNodeVal(knoten[i],idx0) ;
  
    
      /* T * (J^-1 * grad(omega) * c) */
      ADAPCalcTransMatrix(index,vf,trans);
      MGradOmega2D(vf,0.0,0.0);
      MMultMatVec(vf,2,4,node_values,4,grad_rs,2); /* r,s */
      MMultVecMat(grad_rs,2,invjac,2,2,grad_ab,2); /* a,b */
      MMultMatVec(trans,3,2,grad_ab,2,gradient,3); /* x,y,z */
      
      
      dd = molecular_diffusion + ElGetArtDiff(index) \
           + v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) \
           + (mass_dispersion_transverse * mass_dispersion_transverse));    
      
      for (k=0;k<3;k++)
        gradient[k] = gradient[k] * dd;
      
      
      jump_average_max = 0.0;
 
 
      for (j=0;j<nn;j++) {
        jump_average = 0.0;
    
      elems = GetNodeXDElems[typ](knoten[j],&anz);
      for (k=0;k<anz;k++)
        if (elems[k] != index) {
		  mass_dispersion_longitudinal = mmp_vector[ElGetElementGroupNumber(elems[k])]->mass_dispersion_longitudinal;
		  mass_dispersion_transverse = mmp_vector[ElGetElementGroupNumber(elems[k])]->mass_dispersion_transverse;
		  porosity = m_mmp->Porosity(elems[k],NULL,theta);
          v = MBtrgVec(ElGetVelocity(elems[k]), 3) / porosity;
          dd = molecular_diffusion + ElGetArtDiff(elems[k]) \
               + v * sqrt((mass_dispersion_longitudinal * mass_dispersion_longitudinal) \
               + (mass_dispersion_transverse * mass_dispersion_transverse));
        
        
          /* Gradienten der Nachbarn am Knoten j -> gn */
          element_nodes_neighbour = ElGetElementNodes(elems[k]);
          for (m=0;m<nn;m++)
            node_values[m] = theta * GetNodeVal(knoten[m],idx1) 
                         + (1.0 - theta) * GetNodeVal(knoten[m],idx0) ;        
          /* Gradient im Mittelpunkt des Elements */
          invjac = ADAPGetElementJacobi(elems[k],&detjac);

          /* T * (J^-1 * grad(omega) * c) */
          ADAPCalcTransMatrix(elems[k],vf,trans);
          MGradOmega2D(vf,0.0,0.0);
          MMultMatVec(vf,2,4,node_values,4,grad_rs,2); /* r,s */
          MMultVecMat(grad_rs,2,invjac,2,2,grad_ab,2); /* a,b */
          MMultMatVec(trans,3,2,grad_ab,2,gradient_neighbour,3); /* x,y,z */
          
          for (i=0;i<3;i++)
            gradient_neighbour[i] = gradient_neighbour[i] * dd;
      
          
          /* mittlerer Gradientensprung aller angreifenden Elemente */
          for (m=0;m<3;m++)
            jump[m] = gradient[m] - gradient_neighbour[m];
       
          jump_average += MBtrgVec(jump,3); 
        }
    
        if (anz > 1)
          jump_average = jump_average/(double)(anz-1);
        else 
          jump_average = 0.0; /* MBtrgVec(gradient,3); */

        jump_average_max = max(jump_average_max,jump_average);
      }
  
      residuum += jump_average_max / l;
 
      break;
    }

    /* #cb# Hinweis zu JO-1990: h_j = l/2 und theta = 0.5 und mue = 1.9 */

    
    /* kuenstliche Diffusion */
    mue = pow(l, 1.9);
    epsilon = max(d, (C1 * mue * error));       /* Residuum ohne Spruenge */
    /*epsilon = max(d,(C1*mue*residuum));*/  /*Residuum mit Spruengen */

    if (randele == 1)
        error = C2 * sqrt(mue * error); 
    /*  error = C2 * sqrt(mue*residuum); */
    else
        error = C2 * min(1.0, (l * l / epsilon)) * residuum * sqrt(l);
       
    epsilon -= d;
    
    ElSetArtDiff(index, epsilon);
    ele = (long *)Free(ele);
    return error;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_flow1
                                                                          */
/* Aufgabe:
   Residuenindikator fuer die Poisson-Gleichung nach Bieterman & Babuska
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   08.08.1997     R.Kaiser           Uebertragen aus dem aTM

  
                                                                          */
/**************************************************************************/
double Indicator_flow1(long index, int typ, int idx0, int idx1)
{

    static double error;        /* Fehler */
    static double fehler_zeit;  /* Fehler des Zeitanteils */
    static double fehler_spr;   /* Fehler aufgrund der Spruenge */
    static long i, j, k, m, ii;
    static double *matrix;
    static long *elems;
    static long *knoten;        /* Zeiger auf die Elementknoten */
    static long *kno;
    static double *invjac;
    static long nn;             /* Anzahl Knoten pro Element */
    static double head1;        /* Standrohrspiegelhoehe (neu) */
    static double head0;        /* Standrohrspiegelhoehe (alt) */
    static double wert1[4], wert2[4], erg[12], sum[3];
    static double r, s, t;
    static double l[2];
    static double detjac, trans[6];
    static double zwi2[12];
    static double *zwi;
    static int anz;
    static double g[4], h, det, inv[4], w[4], gc[3];
    static long *elekanten;
    static Kante *kante, *elek;
    static long *ele;
    static long *nachbarele;
    static int anzahl, anznachbarn;
    static Knoten *dk1, *dk2;
    static double ds, theta;
    static long kk[2];
    static double C = 3.5e+3;   /* sqrt(1./12.) */
    static double mu, rho, gr;
    static double area,storativity,k_xx;
    double* permeability;

    CFluidProperties *m_mfp = NULL;
    m_mfp = mfp_vector[0];


  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  area = m_mmp->geo_area;
  storativity = m_mmp->StorageFunction(index,NULL,theta);
  permeability = m_mmp->PermeabilityTensor(index);
  k_xx = permeability[0];
  mu = m_mfp->Viscosity();
  rho = m_mfp->Density();
  gr = gravity_constant;

    if (typ == 2) {
        DisplayMsgLn("Indikator 4 fuer 3D-Elemente nicht implementiert");
        return 0.0;
    }
    nn = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */
    knoten = ElGetElementNodes(index);  /* Knoten des Elements index */

    theta = GetNumericalTimeCollocation("PRESSURE");
    for (j = 0; j < nn; j++) {  /* Ergebnisse des */
        head0 = (GetNodeVal(knoten[j], idx0)) / (gr * rho) + \
            (GetNode(knoten[j])->z);    /* letzten Zeitschritts */
        head1 = (GetNodeVal(knoten[j], idx1)) / (gr * rho) + \
            (GetNode(knoten[j])->z);    /* jetzigen Zeitschritts */
        wert1[j] = (head1 - head0) / dt;        /* Zeitableitung */
        wert2[j] = theta * head1 + (1.0 - theta) * head0;       /* Ortsableitung */
    }

    /* Integral ueber das Residuum des Zeitanteils */
    matrix = ADAPGetElementCapacitanceMatrix(index);
    MMultMatVec(matrix, nn, nn, wert1, nn, erg, nn);
    fehler_zeit = storativity * rho * gr * \
        MSkalarprodukt(wert1, erg, nn);

    /* Integral ueber die Spruenge senkrecht zur Elementkante */
    MNulleVec(g, nn);
    switch (typ) {
    case 0:
        /* Elementlaenge */
/*        l[0] = ADAPGetElementVolume(index) / area; OK rf3230 */
        l[0] = ElGetElementVolume(index) / area;
        /* Spruenge in den Knoten */
        invjac = GetElementJacobiMatrix(index, &detjac);
        h = 0.5 * (wert2[1] - wert2[0]);
        for (j = 0; j < 3; j++)
            gc[j] = h * invjac[j];      /* bezogen auf x,y,z (global) */
        for (j = 0; j < nn; j++) {
            MNulleVec(sum, 3);
            elems = GetNode1DElems(knoten[j], &anz);
            for (i = 0; i < anz; i++)
                if (elems[i] != index) {
                    /* Knoten des Elements i am Knoten j */
                    kno = ElGetElementNodes(elems[i]);
                    for (k = 0; k < nn; k++) {
                        head0 = (GetNodeVal(kno[k], idx0)) / (gr * rho) + \
                            (GetNode(kno[k])->z);       /* letzten Zeitschritts */
                        head1 = (GetNodeVal(kno[k], idx1)) / (gr * rho) + \
                            (GetNode(kno[k])->z);       /* jetzigen Zeitschritts */
                        w[k] = theta * head1 + (1.0 - theta) * head0;
                    }
                    Calc1DElementJacobiMatrix(elems[i], inv, &det);
                    h = 0.5 * (w[1] - w[0]);
                    for (k = 0; k < 3; k++)
                        sum[k] += (gc[k] - h * inv[k]); /* Summe der Spruenge */
                }
            /* Betrag der gemittelten Spruenge in den Knoten */
            if (anz > 1)
                g[j] = MBtrgVec(sum, 3) / (double) (anz - 1);
        }
        /* printf("sprung=%e %e\n",g[0],g[1]); */
        /* Quadrat der elementgemittelten halben Spruenge */
        fehler_spr = 0.125 * MSkalarprodukt(g, g, nn);
        break;
    case 1:
        /* Elementlaenge */
        /* dr(lokal) = J * dx(global) */
        invjac = GetElementJacobiMatrix(index, &detjac);
        for (j = 0; j < 4; j++)
            zwi2[j] = invjac[j];
        M2InvertiereUndTransponiere(zwi2);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jakobi-Matrix */
        l[0] = 2.0 * (zwi2[0] + zwi2[1]);
        l[1] = 2.0 * (zwi2[2] + zwi2[3]);
        /* Kantenfehler */
        /* Spruenge in den Knoten senkrecht ueber die Kante */
        elekanten = ElGetElementEdges(index);
        for (j = 0; j < 4; j++) {
            kante = GetEdge(elekanten[j]);
            anznachbarn = 0;

            if (kante->anz_nachbarn_2D == 1) {
                if (kante->vorgaenger == -1)    /* Kante hat keinen Vorgaenger */
                    /* Randkante, kein Nachbarelement, kein Sprung */
                    continue;
                else if (kante->vorgaenger >= 0)        /* Kante hat Vorgaenger */
                    if (GetEdge(kante->vorgaenger)->anz_nachbarn_2D == 1)
                        /* Randkante, kein Nachbarelement, kein Sprung */
                        continue;
            }
            if (kante->kinder[0] == -1) {       /* Kante hat keine Kinder */
                if (kante->vorgaenger == -1) {  /* Kante hat keinen Vorgaenger */
                    /* Nachbarelemente haben gleichen level */
                    elek = kante;
                    nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                    ele = (long *) Malloc(anzahl * sizeof(long));
                    anznachbarn = anzahl;
                    for (ii = 0; ii < anzahl; ii++)
                        ele[ii] = nachbarele[ii];
                } else if (kante->vorgaenger >= 0) {     /* Kante hat Vorgaenger */
                    if (kante->anz_nachbarn_2D == GetEdge(kante->vorgaenger)->anz_nachbarn_2D) {
                        /* Nachbarelemente haben gleichen level */
                        elek = kante;
                        nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                        ele = (long *) Malloc(anzahl * sizeof(long));
                        anznachbarn = anzahl;
                        for (ii = 0; ii < anzahl; ii++)
                            ele[ii] = nachbarele[ii];
                    } else if (kante->anz_nachbarn_2D != GetEdge(kante->vorgaenger)->anz_nachbarn_2D) {
                        /* Nachbarelemente mit niedrigerem Level */
                        elek = GetEdge(kante->vorgaenger);
                        nachbarele = GetEdge2DElems(kante->vorgaenger, &anzahl);
                        ele = (long *) Malloc(anzahl * sizeof(long));
                        anznachbarn = anzahl;
                        for (ii = 0; ii < anzahl; ii++)
                            ele[ii] = nachbarele[ii];
                    }
                  }
            } else if (kante->kinder[0] >= 0) {
                /* Kante hat Kinder (-> Nachbarelemente mit hoeherem Level) */
                elek = kante;
                nachbarele = GetEdge2DElems(elekanten[j], &anzahl);
                ele = (long *) Malloc(anzahl * sizeof(long));
                anznachbarn = anzahl;
                for (ii = 0; ii < anzahl; ii++)
                    ele[ii] = nachbarele[ii];
            }

            /* MNulleVec(zwi, 12); */
            zwi = (double *) Malloc((2*3*anznachbarn) * sizeof(double));

            for (k = 0; k < anznachbarn; k++) { /* ele[0] bis ele[anznachbarn] */
                /* Standrohrspiegelhoehen an ele[k] */
                kno = ElGetElementNodes(ele[k]);
                for (i = 0; i < nn; i++) {
                    head0 = (GetNodeVal(kno[i], idx0)) / (gr * rho) + \
                        (GetNode(kno[i])->z);   /* letzten Zeitschritts */
                    head1 = (GetNodeVal(kno[i], idx1)) / (gr * rho) + \
                        (GetNode(kno[i])->z);   /* jetzigen Zeitschritts */
                    w[i] = theta * head1 + (1.0 - theta) * head0;
                }
                /* printf("w=%e %e %e %e\n",w[0],w[1],w[2],w[3]); */
                /* lokale Koordinaten der Kantenknoten */
                kk[0] = kk[1] = -1;
                for (i = 0; i < 4; i++) {       /* Knoten an ele[k] */
                    if (kno[i] == elek->knoten[0])
                        kk[0] = i;
                    else if (kno[i] == elek->knoten[1])
                        kk[1] = i;
                }
                /* printf("kk=%d %d\n",kk[0],kk[1]); */

                for (i = 0; i < 2; i++) {       /* Gradient im Knoten kk[i] des Elements ele[k] */
                    MGetCoor(typ, kk[i], &r, &s, &t);
                    Calc2DElementJacobiMatrix(ele[k], r, s, invjac, &detjac);
                    ADAPCalcTransMatrix(ele[k], erg, trans);
                    MGradOmega2D(erg, r, s);
                    MMultMatVec(erg, 2, nn, w, nn, gc, 2);      /* r,s */
                    MMultVecMat(gc, 2, invjac, 2, 2, erg, 2);   /* a,b */
                    MMultMatVec(trans, 3, 2, erg, 2, gc, 3);    /* x,y,z */

                    for (m = 0; m < 3; m++)
                        zwi[6 * k + 3 * i + m] = gc[m]; /* Speichern */
                    /* printf("gc=%e %e %e\n",gc[0],gc[1],gc[2]); */
                }
            }
            /* Kantenlaenge */
            dk1 = GetNode(elek->knoten[0]);
            dk2 = GetNode(elek->knoten[1]);
            ds = sqrt((dk1->x - dk2->x) * (dk1->x - dk2->x)
                      + (dk1->y - dk2->y) * (dk1->y - dk2->y)
                      + (dk1->z - dk2->z) * (dk1->z - dk2->z));
            /* printf("ds=%e\n",ds); */
            /* halbe Spruenge quadrieren, integrieren */

            if (anznachbarn > 2) {
            for (k = 2; k < anznachbarn; k++)
              for (i = 0; i < 2; i++)
                for (m = 0; m < 3; m++) {
                  zwi[6 + 3 * i + m] += zwi[6 * k + 3 * i + m];
                }
              for ( i = 0; i < 2; i++)
                for (m = 0; m < 3; m++)
                  zwi[6 + 3 * i + m] = zwi[6 + 3 * i + m] / (anznachbarn - 1);
            }

            for (i = 0; i < 2; i++)
                for (m = 0; m < 3; m++)
                    zwi[3 * i + m] -= zwi[6 + 3 * i + m];

            /* MZeigVec(zwi,6,"d grad c"); */
            g[j] = ds * 0.125 * (zwi[0] * zwi[0] + zwi[1] * zwi[1] + zwi[2] * zwi[2]
                  + zwi[3] * zwi[3] + zwi[4] * zwi[4] + zwi[5] * zwi[5]);
        }
        /* printf("int(0.5 sprung)^2=%e %e %e %e\n",g[0],g[1],g[2],g[3]); */
        fehler_spr = g[0] + g[1] + g[2] + g[3];
        break;
    }

    fehler_spr *= (((k_xx * rho * gr) / mu) * MBtrgVec(l, (typ + 1)));

    /* h^2 / k */
    fehler_zeit *= (MSkalarprodukt(l, l, (typ + 1)) / ((k_xx * rho * gr) / mu));

    /* printf("zeit=%e spr=%e fkt=%e\n",fehler_zeit,fehler_spr,fkt); */
    error = C * sqrt(fehler_zeit + fehler_spr);

    ele = (long *)Free(ele);
    zwi = (double *)Free(zwi);

    return error;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_flow2
                                                                          */
/* Aufgabe:
   Fehlerindikator in Anlehnung an einen Residuenindikator
   fuer parabolische Differentialgleichungen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0: Index des Knotenwertes (z.B. conc0)
   E int idx1: Index des Knotenwertes (z.B. conc1)
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   11.08.1997     R.Kaiser           Uebertragen aus dem aTM

 
                                                                          */
/**************************************************************************/
double Indicator_flow2(long index, int typ, int idx0, int idx1)
{
    static double error;        /* Fehler */
    static double fehler_cond;
    static double fehler_res;   /* Residuum-Fehler */
    static double fehler_spr;   /* Fehler aufgrund der Spruenge */
    static long i, j, k, m;
    static double *matrix;
    static long *elems;
    static long *knoten;        /* Zeiger auf die Elementknoten */
    static long *kno;
    static long nn;             /* Anzahl Knoten pro Element */
    static double head1;        /* Standrohrspiegelhoehe (neu) */
    static double head0;        /* Standrohrspiegelhoehe (alt) */
    static double wert1[8], wert2[8], erg[24], sum[3];
    static double r, s, t, theta;
    static double l;
    static double invjac[9], detjac, zwi[12], trans[6];
    static int anz;
    static double fkt, g[8], h, inv[9], det, w[8], sp[3], gc[3];
    static double mu, rho, gr;
    static double area,k_xx;
    double* permeability;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

    area = m_mmp->geo_area;
//    k_xx = GetSoilPermeability(index,0);
    permeability = m_mmp->PermeabilityTensor(index);
    k_xx = permeability[0];



    mu = mfp_vector[0]->Viscosity();
    rho = mfp_vector[0]->Density();
    gr = gravity_constant;


    nn = ElNumberOfNodes[typ];  /* Anzahl Elementknoten */
    knoten = ElGetElementNodes(index);  /* Knoten des Elements index */
    theta = GetNumericalTimeCollocation("PRESSURE");

    for (j = 0; j < nn; j++) {  /* Ergebnisse des */
        head0 = (GetNodeVal(knoten[j], idx0)) / (gr * rho) + \
            (GetNode(knoten[j])->z);    /* letzten Zeitschritts */
        head1 = (GetNodeVal(knoten[j], idx1)) / (gr * rho) + \
            (GetNode(knoten[j])->z);    /* jetzigen Zeitschritts */
        wert1[j] = (head1 - head0) / dt;        /* Zeitableitung */
        wert2[j] = theta * head1 + (1.0 - theta) * head0;       /* Ortsableitung */
    }

    /* Massenintegral */
    matrix = ADAPGetElementCapacitanceMatrix(index);
    MMultMatVec(matrix, nn, nn, wert1, nn, erg, nn);


    matrix = ADAPGetElementConductanceMatrix(index);
    /* einschl. Rand */

    switch (typ) {
    case 0:
/*        l = ADAPGetElementVolume(index) / area; OK rf3230 */
        l = ElGetElementVolume(index) / area;    /* Elementlaenge */
        fehler_cond = 0.0;
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        Calc1DElementJacobiMatrix(index, invjac, &detjac);
        h = 0.5 * (wert2[1] - wert2[0]);
        for (j = 0; j < 3; j++)
            gc[j] = h * invjac[j];      /* bezogen auf x,y,z (global) */
        for (j = 0; j < nn; j++) {
            MNulleVec(sum, 3);
            elems = GetNode1DElems(knoten[j], &anz);
            for (i = 0; i < anz; i++)
                if (elems[i] != index) {
                    /* Knoten des Elements i am Knoten j */
                    kno = ElGetElementNodes(elems[i]);
                    for (k = 0; k < nn; k++) {
                        head0 = (GetNodeVal(kno[k], idx0)) / (gr * rho) + \
                            (GetNode(kno[k])->z);       /* letzten Zeitschritts */
                        head1 = (GetNodeVal(kno[k], idx1)) / (gr * rho) + \
                            (GetNode(kno[k])->z);       /* jetzigen Zeitschritts */
                        w[k] = theta * head1 + (1.0 - theta) * head0;
                    }
                    Calc1DElementJacobiMatrix(elems[i], inv, &det);
                    h = 0.5 * (w[1] - w[0]);
                    for (k = 0; k < 3; k++)
                        sum[k] += (gc[k] - h * inv[k]); /* Summe der Spruenge */
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e\n",g[0],g[1]); */
        break;
    case 1:
/*        l = sqrt(ADAPGetElementVolume(index) / area); OK rf3230 */
        l = sqrt(ElGetElementVolume(index) / area);      /* Elementlaenge */

        /* Elementfehler */
        MMultMatVec(matrix, nn, nn, wert2, nn, zwi, nn);
        fehler_cond = MSkalarprodukt(zwi, zwi, nn);
        for (j = 0; j < nn; j++)
            erg[j] += zwi[j];
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        for (j = 0; j < nn; j++) {
            /* Gradient im Knoten j des Elements index */
            MGetCoor(typ, j, &r, &s, &t);
            Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
            ADAPCalcTransMatrix(index, zwi, trans);
            MGradOmega2D(erg, r, s);
            MMultMatVec(erg, 2, nn, wert2, nn, gc, 2);  /* r,s */
            MMultVecMat(gc, 2, invjac, 2, 2, erg, 2);   /* a,b */
            MMultMatVec(trans, 3, 2, erg, 2, gc, 3);    /* x,y,z */
            elems = GetNode2DElems(knoten[j], &anz);
            MNulleVec(sum, 3);  /* Spruenge in den Knoten */
            for (k = 0; k < anz; k++)
                if (elems[k] != index) {
                    kno = ElGetElementNodes(elems[k]);
                    for (i = 0; i < nn; i++) {
                        head0 = (GetNodeVal(kno[i], idx0)) / (gr * rho) + \
                            (GetNode(kno[i])->z);       /* letzten Zeitschritts */
                        head1 = (GetNodeVal(kno[i], idx1)) / (gr * rho) + \
                            (GetNode(kno[i])->z);       /* jetzigen Zeitschritts */
                        w[i] = theta * head1 + (1.0 - theta) * head0;
                    }
                    MNulleVec(sp, 3);
                    for (i = 0; i < nn; i++)
                        if (kno[i] == knoten[j]) {
                            /* Gradient im Knoten i des Elements k */
                            MGetCoor(typ, i, &r, &s, &t);
                            Calc2DElementJacobiMatrix(elems[k], r, s, inv, &det);
                            ADAPCalcTransMatrix(elems[k], zwi, trans);
                            MGradOmega2D(erg, r, s);
                            MMultMatVec(erg, 2, nn, w, nn, sp, 2);      /* r,s */
                            MMultVecMat(sp, 2, inv, 2, 2, erg, 2);      /* a,b */
                            MMultMatVec(trans, 3, 2, erg, 2, sp, 3);    /* x,y,z */
                            /* Sprung zwischen index und elems[k] in i */
                            for (m = 0; m < 3; m++)
                                sp[m] = gc[m] - sp[m];
                            break;
                        }
                    for (i = 0; i < 3; i++)
                        sum[i] += sp[i];
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e %e %e\n",g[0],g[1],g[2],g[3]); */
        break;
    case 2:
/*        l = pow(ADAPGetElementVolume(index), Mdrittel); OK rf3230 */
        l = pow(ElGetElementVolume(index), Mdrittel); /* Elementlaenge */

        /* Elementfehler */
        MMultMatVec(matrix, nn, nn, wert2, nn, zwi, nn);
        fehler_cond = MSkalarprodukt(zwi, zwi, nn);
        for (j = 0; j < nn; j++)
            erg[j] += zwi[j];
        fehler_res = MSkalarprodukt(erg, erg, nn);
        /* Kantenfehler */
        /* Spruenge in den Knoten */
        for (j = 0; j < nn; j++) {
            /* Gradient im Knoten j des Elements index */
            MGetCoor(typ, j, &r, &s, &t);
            Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
            MGradOmega3D(erg, r, s, t);
            MMultMatVec(erg, 3, nn, wert2, nn, trans, 3);       /* r,s,t */
            MMultMatVec(invjac, 3, 3, trans, 3, gc, 3); /* x,y,z */
            elems = GetNode3DElems(knoten[j], &anz);
            MNulleVec(sum, 3);  /* Spruenge in den Knoten */
            for (k = 0; k < anz; k++)
                if (elems[k] != index) {
                    kno = ElGetElementNodes(elems[k]);
                    for (i = 0; i < nn; i++) {
                        head0 = (GetNodeVal(kno[i], idx0)) / (gr * rho) + \
                            (GetNode(kno[i])->z);       /* letzten Zeitschritts */
                        head1 = (GetNodeVal(kno[i], idx1)) / (gr * rho) + \
                            (GetNode(kno[i])->z);       /* jetzigen Zeitschritts */
                        w[i] = theta * head1 + (1.0 - theta) * head0;
                    }
                    MNulleVec(sp, 3);
                    for (i = 0; i < nn; i++)
                        if (kno[i] == knoten[j]) {
                            /* Gradient im Knoten i des Elements k */
                            MGetCoor(typ, i, &r, &s, &t);
                            Calc3DElementJacobiMatrix(elems[k], r, s, t, inv, &det);
                            MGradOmega3D(erg, r, s, t);
                            MMultMatVec(erg, 3, nn, w, nn, trans, 3);   /* r,s,t */
                            MMultMatVec(inv, 3, 3, trans, 3, sp, 3);    /* x,y,z */
                            /* Sprung zwischen index und elems[k] in i */
                            for (m = 0; m < 3; m++)
                                sp[m] = gc[m] - sp[m];
                            break;
                        }
                    for (i = 0; i < 3; i++)
                        sum[i] += sp[i];
                }
            /* Betrag der gemittelten Spruenge */
            g[j] = MBtrgVec(sum, 3) / (double) (anz);
        }
        /* printf("g=%e %e %e %e %e %e %e %e \n", \
        g[0],g[1],g[2],g[3],g[4],g[5],g[6],g[7]); */
        break;
    }

    /* Randintegral */
    MMultMatVec(matrix, nn, nn, g, nn, erg, nn);
    fehler_spr = MSkalarprodukt(erg, g, nn) * ((k_xx * rho * gr) / mu);
    /* Elementfehler */
    fkt = l * l / ((k_xx * rho * gr) / mu);

    /* printf("\n cond=%e res=%e spr=%e fkt=%e",\
            fehler_cond,fehler_res,fehler_spr,fkt); */
    error = sqrt((fehler_res + fehler_spr) * fkt);
    return error;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_flow3
                                                                          */
/* Aufgabe:
   Heuristischer Sprungindikator (nur Druckfeld)
   2.5 Kluftnetzwerke (nur 2d-Elemente !!)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Element-Dimension - 1
   E int idx0: Index des Knotenwertes
   E int idx1: Index des Knotenwertes
               (Hier als Zwischenspeicher genutzt !!!)
                                                                          */
/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   03.01.2001     R.Kaiser      Erste Version
                                                                          */
/**************************************************************************/
double Indicator_flow3(long index, int typ, int idx0, int idx1)
{
    static int dummy;
    static int i, j, p, q, k;   /* Laufvariablen */
    static double vf[12];       /* Vektorfeld */
    static double v[2];
    static double trans[6];
    static double kf[4]; 
    static double vel[3];
    static double rs[2], ab[2];
    static long *nodes;
    static long *edges;
    static long *elem, *elemkind0, *elemkind1, *elemvorg;
    static int anz, anzkind0, anzkind1, anzvorg;
    static double edge_middle[3];
    static double n[3], vel_n, vel2_n, velkind0, velkind1;
    static double vorfaktor;
    static double jump[4], sprung, sprung0, sprung1;
    static long nd1[2];
    static double my, rho, g; 
    static double error;
    static double area;
    //static double gp[3];
//	int theta = 0; //SB:m
    double* permeability;
	CFluidProperties *m_mfp = NULL;

    CMediumProperties *m_mmp = NULL;
    long group = ElGetElementGroupNumber(index);
    m_mmp = mmp_vector[group];

    permeability = m_mmp->PermeabilityTensor(index);

    area = m_mmp->geo_area;

    dummy = idx1;
    dummy = idx0;
    dummy = typ;

	my = m_mfp->Viscosity();
    rho = m_mfp->Density();
    g = gravity_constant;

    error = 0.0;

    nodes = ElGetElementNodes(index);
    edges = ElGetElementEdges(index);   /* Kanten des betrachteten
                                           Elements holen */

    for (j = 0; j < 4; j++)
//        kf[j] = - GetSoilPermeability(index,j) * rho * g / my;
        kf[j] = - permeability[j] * rho * g / my;

    for (i = 0; i < 4; i++) {   /* Schleife ueber die Elementkanten */
        jump[i] = 0.0;
        k = 0;


        /* Berechnung des Einheitsnormalenvektors der Kante */
        Calc2DEdgeUnitNormalVec(index, edges[i], n);

        /* Knoten der Kante bereitstellen */
        for (j = 0; j < 2; j++) {
            nd1[j] = GetEdge(edges[i])->knoten[j];
        }

        /* Berechnung der x,y,z - Koordinaten der Kantenmitte */
        edge_middle[0] = (GetNode(nd1[0])->x + GetNode(nd1[1])->x) / 2;
        edge_middle[1] = (GetNode(nd1[0])->y + GetNode(nd1[1])->y) / 2;
        edge_middle[2] = (GetNode(nd1[0])->z + GetNode(nd1[1])->z) / 2;

        /* Transformation der Koordinaten in das r,s - System */
        Calc2DElement_xyz2ab_Coord(index, ab, edge_middle[0], \
                                   edge_middle[1], edge_middle[2]);
        Calc2DElement_ab2rs_Coord(index, rs, ab[0], ab[1]);

        /* Berechnung der Geschwindigkeit in der Kantenmitte */
        /* Geschwindigkeit im a,b - System berechnen */
        CalcVelo2Drs(0, index, 1., rs[0], rs[1], 0, v);
        /*gp[0] = rs[0];
        gp[1] = rs[1];
        gp[2] = 0.0;
        VELCalcGaussQuadLocal(0, index, gp, m_pcs, v);*/

        /* Geschwindigkeit im x,y,z - System */
        ADAPCalcTransMatrix(index, vf, trans);
        MMultMatVec(trans, 3, 2, v, 2, vel, 3);

        /* Geschwindigkeit in Einheitsnormalenrichtung berechnen */
        vel_n = MSkalarprodukt(vel, n, 3);

        /* Ueberpruefen, ob Randkante (Kante und Vorgaengerkante haben
           maximal ein zugehoeriges 2D-Element) */



        if (((GetEdge(edges[i])->anz_nachbarn_2D != 2) && \
             (GetEdge(edges[i])->vorgaenger == -1)) || \
            ((GetEdge(edges[i])->anz_nachbarn_2D != 2) && \
             (GetEdge(edges[i])->vorgaenger >= 0) && \
        (GetEdge(GetEdge(edges[i])->vorgaenger)->anz_nachbarn_2D != 2))) {
            jump[i] = 0.0;
        } 
        else {
            elem = GetEdge2DElems(edges[i], &anz);
            for (j = 0; j < anz; j++) {
                if (index != elem[j]) {
                    if (ElGetElementActiveState(elem[j]) == 1) {
                        /* Transformation der Koordinaten in das r,s - System */
                        Calc2DElement_xyz2ab_Coord(elem[j], ab, edge_middle[0], \
                                         edge_middle[1], edge_middle[2]);
                        Calc2DElement_ab2rs_Coord(elem[j], rs, ab[0], ab[1]);

                        /* Geschwindigkeit im a,b - System */
                        CalcVelo2Drs(0, elem[j], 1., rs[0], rs[1], 0, v);

                        /* Geschwindigkeit im x,y,z - System */
                        ADAPCalcTransMatrix(elem[j], vf, trans);
                        MMultMatVec(trans, 3, 2, v, 2, vel, 3);

                        /* Geschwindigkeit in Einheitsnormalenrichtung berechnen */
                        vel2_n = MSkalarprodukt(vel, n, 3);

                        /* Geschwindigkeitssprung berechnen */
                        sprung = max(vel_n, vel2_n) - min(vel_n, vel2_n);

                        jump[i] = max(jump[i], sprung);
                    } else {
                        k++;
                        elemkind0 = GetEdge2DElems((GetEdge(edges[i]))->kinder[0], &anzkind0);
                        elemkind1 = GetEdge2DElems((GetEdge(edges[i]))->kinder[1], &anzkind1);

                        for (p = 0; p < anzkind0; p++) {
                            /* Transformation der Koordinaten in das r,s - System */
                            Calc2DElement_xyz2ab_Coord(elemkind0[p], ab, edge_middle[0], \
                                         edge_middle[1], edge_middle[2]);
                            Calc2DElement_ab2rs_Coord(elemkind0[p], rs, ab[0], ab[1]);

                            /* Geschwindigkeit im a,b - System */
                            CalcVelo2Drs(0, elemkind0[p], 1., rs[0], rs[1], 0, v);

                            /* Geschwindigkeit im x,y,z - System */
                            ADAPCalcTransMatrix(elemkind0[p], vf, trans);
                            MMultMatVec(trans, 3, 2, v, 2, vel, 3);

                            /* Geschwindigkeit in Einheitsnormalenrichtung berechnen */
                            velkind0 = MSkalarprodukt(vel, n, 3);

                            /* Geschwindigkeitssprung berechnen */
                            sprung0 = max(vel_n, velkind0) - min(vel_n, velkind0);

                            for (q = 0; q < anzkind1; q++) {
                                if (ElGetElementPred(elemkind0[p]) == \
                                    ElGetElementPred(elemkind1[q])) {

                                    /* Transformation der Koordinaten in das r,s - System */
                                    Calc2DElement_xyz2ab_Coord(elemkind1[q], ab, edge_middle[0], \
                                         edge_middle[1], edge_middle[2]);
                                    Calc2DElement_ab2rs_Coord(elemkind1[q], rs, ab[0], ab[1]);

                                    /* Geschwindigkeit im a,b - System */
                                    CalcVelo2Drs(0, elemkind1[q], 1., rs[0], rs[1], 0, v);

                                    /* Geschwindigkeit im x,y,z - System */
                                    ADAPCalcTransMatrix(elemkind1[q], vf, trans);
                                    MMultMatVec(trans, 3, 2, v, 2, vel, 3);

                                    /* Geschwindigkeit in Einheitsnormalenrichtung berechnen */
                                    velkind1 = MSkalarprodukt(vel, n, 3);
                                    sprung1 = max(vel_n, velkind1) - min(vel_n, velkind1);
                                }
                            }
                            /* Mittlerer Sprung */
                            sprung = (sprung0 + sprung1) / 2;
                            jump[i] = max(jump[i], sprung);

                        }
                    }
                }
            }
            if (GetEdge(edges[i])->vorgaenger >= 0l) {
                elemvorg = GetEdge2DElems(GetEdge(edges[i])->vorgaenger, &anzvorg);
                if (k == 0) {
                    for (j = 0; j < anzvorg; j++) {
                        if (ElGetElementActiveState(elemvorg[j]) == 1) {
                            /* Transformation der Koordinaten in das r,s - System */
                            Calc2DElement_xyz2ab_Coord(elemvorg[j], ab, edge_middle[0], \
                                         edge_middle[1], edge_middle[2]);
                            Calc2DElement_ab2rs_Coord(elemvorg[j], rs, ab[0], ab[1]);

                            /* Geschwindigkeit im a,b - System */
                            CalcVelo2Drs(0, elemvorg[j], 1., rs[0], rs[1], 0, v);

                            /* Geschwindigkeit im x,y,z - System */
                            ADAPCalcTransMatrix(elemvorg[j], vf, trans);
                            MMultMatVec(trans, 3, 2, v, 2, vel, 3);

                            /* Geschwindigkeit in Einheitsnorrmalenrichtung berechnen */
                            vel2_n = MSkalarprodukt(vel, n, 3);
                            sprung = max(vel_n, vel2_n) - min(vel_n, vel2_n);

                            jump[i] = max(jump[i], sprung);
                        }
                    }
                }
            }
        }
        
        error = max(error, jump[i]);
    }

   /* vorfaktor = sqrt(ElGetElementVolume(index) / area); */
     
      vorfaktor = sqrt(ElGetElementVolume(index) / area) / fabs(kf[0]); 


    error *= vorfaktor;

    return error;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Indicator_flow4
                                                                          */
/* Aufgabe:
   Kruemmungen der Stromlinien
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Element-Index
   E int typ:    Nicht benutzt
   E int idx0: Nicht benutzt
   E int idx1: Nicht benutzt
                                                                          */
/* Ergebnis:
   Element-Fehler
                                                                          */
/* Programmaenderungen:
   03.03.1999 C. Thorenz                Erste Version
                                                                          */
/**************************************************************************/
double Indicator_flow4(long index, int typ, int idx0, int idx1)
{
    static int dummy;
    static int i, j, k;   /* Laufvariablen */
    static double v[3], v_nachbar[3];
    static long *nodes, *nachbarn;
    static int anzahl_nachbarn;
    static double error;

    dummy = idx1;
    dummy = idx0;
    dummy = typ;

    error = 0.0;

    CalcVeloXDrst(0, index, 1., 0.,0.,0., v);

    nodes = ElGetElementNodes(index);

    for (i = 0; i < ElGetElementNodesNumber(index); i++) {   /* Schleife ueber die Elementknoten */
        for (j = 0; j < 3; j++) {   /* Schleife ueber die Elementdimensionen der Nachbarn */
            nachbarn = GetNodeXDElems[j](nodes[i], &anzahl_nachbarn);
            for (k = 0; k < anzahl_nachbarn; k++) {   /* Schleife ueber die Nachbarn */
                if (ElGetElementActiveState(nachbarn[k])) {
                    CalcVeloXDrst(0, nachbarn[k], 1., 0.,0.,0., v_nachbar);
                    MVekSum(v_nachbar,-1.,v,3);
                    error = max(error, MBtrgVec(v_nachbar,3));
                }
            }
        }
    }

    return error;


}

