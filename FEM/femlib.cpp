/**************************************************************************
   ROCKFLOW - Modul: femlib.c

   Aufgabe:
   Funktionen fuer die FEM

   Programmaenderungen:
   02/1997     R.Kaiser     Uebertragen aus dem aTM
   04/1997     R.Kaiser     Funktionsnamen geaendert
   05/1997     R.Kaiser     jac_asm.c -> femlib.c
   16.10.1997  R.Kaiser     Funktionen zur Berechnung der Elementvolumen
   09.03.1998  R.Kaiser     Funktionen zur Berechnung eines
                            Einheitsnormalenvektors zur Elementkante (2D)
   02/2000     RK           Funktionen fuer zweidim. 9-Knoten-Elemente
   02/2000     C.Thorenz    Wrapper fuer InterpolValuexD
   11/2000     OK           Volumenberechnung fuer Tetraeder und Prismen
   03/2003     RK           Quellcode bereinigt, Globalvariablen entfernt

 **************************************************************************/
#include "stdafx.h"                    /* MFC */
// C++ STL
#include <iostream>
/* Preprozessor-Definitionen */
#include "makros.h"
/* Intern benutzte Module */
#include "edges.h"
#include "femlib.h"
#include "mathlib.h"
#include "intrface.h"
#include "adaptiv.h"
#include "rf_pcs.h" //OK_MOD"
#include "matrix.h"
/* Intern benutzte Objekte */
#include "nodes.h"
#include "elements.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
/* Definitionen */
void Calc2DTriangleElementVolume(long index, double *volume);
void Calc3DTetraederElementVolume(long index, double *volume);
void Calc3DPrismElementVolume(long index, double *volume);


/**************************************************************************/
/* ROCKFLOW - Funktion: InitElementGeometry
 */
/* Aufgabe:
   Initialisiert die zu einem Element gehoerige Geometrie:
   - Gausspunkte
   - Jakobimatrix
   - Volumen
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02/1999   CT   Erste Version
   07/2000   OK   Erweiterung fuer Dreiecks-Elemente
   11/2000   OK   Erweiterung fuer Tetraeder und Prismen
 */
/**************************************************************************/
void InitElementGeometry(long index)
{
  double *invjac, dj, volume;


  /* Determinante und Inverse der Jacobi-Matrix berechnen */
  switch (ElGetElementType(index))
    {
    case 1:
      invjac = (double *) Malloc(3 * sizeof(double));
      break;
    case 2:
      invjac = (double *) Malloc(4 * sizeof(double));
      break;
    case 3:
      invjac = (double *) Malloc(9 * sizeof(double));
      break;
    case 4:                            /* Dreiecke */
      invjac = (double *) Malloc(4 * sizeof(double));
      break;
    case 5:                            /* Tetraeder */
      invjac = (double *) Malloc(9 * sizeof(double));
      break;
    case 6:                            /* Prismen */
      invjac = (double *) Malloc(9 * sizeof(double));
      break;
    default:
      abort();
    }

  CalcElementJacobiMatrix(index, invjac, &dj);
  SetElementJacobiMatrix(index, invjac, dj);
  CalcElementVolume(index, &volume);
  //OK volume = fabs(volume); //CMCD
  
  /*
  if (volume < MKleinsteZahl)
    {
      DisplayMsgLn("");
      DisplayMsg("Warning! Element ");
      DisplayLong(index);
      DisplayMsg(" has a volume of ");
      DisplayDouble(volume, 0, 0);
      DisplayMsgLn("");
      element = ElGetElement(index);
      if(element->elementtyp==3){ //OK
        nodes[0] = element->elementknoten[0];
        nodes[1] = element->elementknoten[1];
        nodes[2] = element->elementknoten[2];
        nodes[3] = element->elementknoten[3];
        nodes[4] = element->elementknoten[4];
        nodes[5] = element->elementknoten[5];
        nodes[6] = element->elementknoten[6];
        nodes[7] = element->elementknoten[7];
        element->elementknoten[0] = nodes[4];
        element->elementknoten[1] = nodes[5];
        element->elementknoten[2] = nodes[6];
        element->elementknoten[3] = nodes[7];
        element->elementknoten[4] = nodes[0];
        element->elementknoten[5] = nodes[1];
        element->elementknoten[6] = nodes[2];
        element->elementknoten[7] = nodes[3];
        CalcElementJacobiMatrix(index, invjac, &dj);
        SetElementJacobiMatrix(index, invjac, dj);
        CalcElementVolume(index,&volume);
        if(volume<MKleinsteZahl){
          cout << "Negative volume: " << index << "," << volume << endl;
        }
        else
          cout << "Volume corrected: " << volume << endl;
      }
    }
   */ 
  ElSetElementVolume(index, volume);
  FEMCalcElementCharacterisiticLength(index); /* CMCD 05/2004 */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcElementJacobiMatrix
 */
/* Aufgabe:
   Berechnung der Inversen der Jacobi-Matrix
   (1D, 2D oder 3D) im Zentrum des Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
   X double *vector: Inverse der Jacobi-Matrix
   X double *dj: Determinante der Jacobi-Matrix
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02/1997   RK   Uebertragen aus dem aTM (-> CECalcJacobi_ASM)
   04/1997   RK   Umbenannt zu CalcElementJacobiMatrix
   06/1997   RK   Verallgemeinert (unabhaengig von den RF-Modellen)
   07/2000   OK   Vorbereitung fuer Dreieck-Elemente
   9/2000   CT   Zugriff auf CalcXDElementJacobiMatrix
 */
/**************************************************************************/
void CalcElementJacobiMatrix(long index, double *vec, double *dj)
{
  CalcXDElementJacobiMatrix(index, 0., 0., 0., vec, dj);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcXDElementJacobiMatrix
 */
/* Aufgabe:
   Berechnung der Inversen der Jacobi-Matrix
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements
   E double r: Lokale Koordinate
   E double s: Lokale Koordinate
   E double t: Lokale Koordinate
   X double *vec: Inverse der Jacobi-Matrix
   X double *dj: Determinante der Jacobi-Matrix
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   09/2000   CT     Erste Version
 */
/**************************************************************************/
void CalcXDElementJacobiMatrix(long index, double r, double s, double t, double *vec, double *dj)
{
  switch (ElGetElementType(index))
    {
    case 1:
      Calc1DElementJacobiMatrix(index, vec, dj);
      break;
    case 2:
      Calc2DElementJacobiMatrix(index, r, s, vec, dj);
      break;
    case 3:
      Calc3DElementJacobiMatrix(index, r, s, t, vec, dj);
      break;
    case 4:
      vec[0] = 1.;
      *dj = 1.;
      break;
    case 5:
      vec[0] = 1.;
      *dj = 1.;
      break;
    case 6:
      vec[0] = 1.;
      *dj = 1.;
      break;
    default:
      DisplayMsgLn("CalcXDElementJacobiMatrix: Invalid element type");
      break;
    }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Calc1DElementJacobiMatrix
 */
/* Aufgabe:
   Berechnet die "Determinante" und die Inverse der Jacobi-Matrix /
   den Drehvektor des angegebenen 1D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s
   X double *vec
   X double *dj                                                       */

/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02/1997     R.Kaiser     Uebertragen aus dem aTM (-> CalcJac1D_ASM)
   04/1997     R.Kaiser     Umbenannt zu Calc1DElementJacobiMatrix
 */
/**************************************************************************/
void Calc1DElementJacobiMatrix(long index, double *vec, double *dj)
{
  static double delta_x, delta_y, delta_z, vorfk;
  static long *nodes;
  static Knoten *xx;
  static Knoten *yy;
  static double area;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  area = m_mmp->geo_area;

  /* "Inverse" */
  nodes = ElGetElementNodes(index);
  xx = GetNode(nodes[0]);
  yy = GetNode(nodes[1]);
  delta_x = yy -> x - xx -> x;
  delta_y = yy -> y - xx -> y;
  delta_z = yy -> z - xx -> z;
/*  DisplayMsg("In der Jacobi : ");
   DisplayDouble(delta_x,0,0);
   DisplayDouble(delta_y,0,0);
   DisplayDouble(delta_z,0,0); */
  vorfk = 2.0 / (delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
  vec[0] = delta_x * vorfk;
  vec[1] = delta_y * vorfk;
  vec[2] = delta_z * vorfk;
  /* "Determinante" */
  *dj = area * 0.5 * sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElementJacobiMatrix
 */
/* Aufgabe:
   Berechnet die Inverse und die Determinante der Jacobi-Matrix /
   Drehvektor des angegebenen 2D - Elements

   -1               T -1 T
   y  = ((grad(w) A T)  )


   -1
   y         - inverse Jacobi-Matrix

   grad(w)   - Gradientenmatrix der Ansatzfunktion

   A         - Koordinatenmatrix
   T
   | x1 y1 z1 |   [x,y,z] - Vektor d. Knotens
   |          |
   | x2 y2 z2 |
   A = |          |
   | x3 y3 z3 |
   |          |
   | x4 y4 z4 |

   T
   T         - Transponierte der Transformationsmatrix
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s
   X double *erg
   X double *dj
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02/1997     R.Kaiser     Uebertragen aus dem aTM (-> CalcJac2D_ASM)
   04/1997     R.Kaiser     Umbenannt zu Calc2DElementJacobiMatrix
 */
/**************************************************************************/
void Calc2DElementJacobiMatrix(long index, double r, double s, double *erg, double *dj)
{
  static double koma[12];
  static double grd[8];
  static double zw[6];
  static double t[6];

/*  DisplayMsgLn("In der Jacobi"); */
  CalcTransformationsMatrix(index, koma, t);

/*  MZeigMat(koma,4,3,"KORDINATEN");
   MZeigMat(t,3,2,"TRANS"); */

  MGradOmega2D(grd, r, s);
/*  MZeigMat(grd,2,4,"GRADIENT"); */

  MMultMatMat(grd, 2, 4, koma, 4, 3, zw, 2, 3);
  MMultMatMat(zw, 2, 3, t, 3, 2, erg, 2, 2);
  *dj = M2Determinante(erg);
  M2InvertiereUndTransponiere(erg); /* Invertieren + Transponieren ist korrekt */
/*    MZeigMat(erg,2,2,"ERGEBNIS"); */

}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcTransformationsMatrix
 */
/* Aufgabe:
   Berechnet die Koordinatenmatrix und die Transformationsmatrix
   a,b -> x,y,z des angegebenen 2D - Elements,
   gehoert direkt zu CalcJac2D


   T
   | x1 y1 z1 |   [x,y,z] - Vektor d. Knotens
   |          |
   | x2 y2 z2 |
   A = |          |
   | x3 y3 z3 |
   |          |
   | x4 y4 z4 |

   T         - Transformationsmatrix
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *koma: Koordinatenmatrix (4x3)
   X double *trans: Transformationsmatrix (3x2)
 */
/* Ergebnis:
   - Matrizen -
 */
/* Programmaenderungen:
   02/1997     R.Kaiser     Uebertragen aus dem aTM (-> CalcTransMatrix_ASM)
   04/1997     R.Kaiser     Umbenannt zu CalcTransformationsMatrix
 */
/**************************************************************************/
void CalcTransformationsMatrix(long index, double *koma, double *trans)
{
  static double vec2[3];
  static double vec1[3];
  static double winkel;         /* kann zur Zeit nicht ueber Eingabe eingelesen werden */
  /* Zur Zeit sind nur isotrope Probleme beruecksichtigt */
  static Knoten *nd1;
  static Knoten *nd2;
  static Knoten *nd3;
  static Knoten *nd4;
  static long *nodes;

  nodes = ElGetElementNodes(index);
  winkel = 0.0;
  /* Uebersichtlicher ?
     for (i=0;i<4;i++) {
     nd = GetNode(nodes[i]);
     koma[i*3  ] = nd->x;
     koma[i*3+1] = nd->y;
     koma[i*3+2] = nd->z;
     } */
  nd1 = GetNode(nodes[0]);
  nd2 = GetNode(nodes[1]);
  nd3 = GetNode(nodes[2]);
  nd4 = GetNode(nodes[3]);

  /* Koordinatenmatrix A (koma) */
  koma[0] = nd1 -> x;
  koma[1] = nd1 -> y;
  koma[2] = nd1 -> z;

  koma[3] = nd2 -> x;
  koma[4] = nd2 -> y;
  koma[5] = nd2 -> z;

  koma[6] = nd3 -> x;
  koma[7] = nd3 -> y;
  koma[8] = nd3 -> z;

  koma[9] = nd4 -> x;
  koma[10] = nd4 -> y;
  koma[11] = nd4 -> z;

  /* zwei Vektoren aus der Element-Ebene
     fuer die Transformationsmatrix */
  vec1[0] = koma[3] - koma[0];
  vec1[1] = koma[4] - koma[1];
  vec1[2] = koma[5] - koma[2];
  vec2[0] = koma[9] - koma[0];
  vec2[1] = koma[10] - koma[1];
  vec2[2] = koma[11] - koma[2];

  MKTFMat3Dr2D(vec1, vec2, winkel, trans);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc3DElementJacobiMatrix
 */
/* Aufgabe:
   Berechnet die Inverse Jacobi-Matrix des angegebenen 3D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s,t
   X double *erg
   X double *dj
 */
/* Ergebnis:
   - double -
 */
/* Programmaenderungen:
   02/1997     R.Kaiser     Uebertragen aus dem aTM (-> CalcJac3D_ASM)
   04/1997     R.Kaiser     Umbenannt zu Calc3DElementJacobiMatrix
 */
/**************************************************************************/
void Calc3DElementJacobiMatrix(long index, double r, double s, double t, double *erg, double *dj)
{
  static double koma[24];
  static double grd[24];
  static Knoten *nd;
  static long *nodes;
  static long i;
  /* Koordinatenmatrix A */
  nodes = ElGetElementNodes(index);
  for (i = 0; i < 8; i++)
    {
      nd = GetNode(nodes[i]);
      koma[i * 3] = nd -> x;
      koma[i * 3 + 1] = nd -> y;
      koma[i * 3 + 2] = nd -> z;
    }
  MGradOmega3D(grd, r, s, t);
  MMultMatMat(grd, 3, 8, koma, 8, 3, erg, 3, 3);
  *dj = M3Determinante(erg);
  M3Invertiere(erg);
 /* MK_Calc3DElementJacobiMatrix_error_detected_14.01.2002
    Hier wird nicht die inverse, sondern die transponierte der inversen
    Jakobimatrix berechnet (J-T)!!!
    Diese zwangslaeufige Fehlerquelle wird demnaechst entschaerft, MK.   
    -> alter Name, neue Beschreibung
    -> gut waere eine Anpassung im den cel's innvac_t statt invjac       */                                                                                    
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalcElementVolume
 */
/* Aufgabe:
   Berechnung des Elementvolumens (1D, 2D oder 3D)
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements

 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   10/1997   RK   Erste Version
   07/2000   OK   Erweiterung fuer Dreieck-Elemente
   04/2002   OK   Tetraeder
   05/2003   MB   Dreiecksprisma
 */
/**************************************************************************/
void CalcElementVolume(long index, double *volume)
{
  switch (ElGetElementType(index))
    {
    case 1:
      Calc1DElementVolume(index, volume);
      break;
    case 2:
      Calc2DElementVolume(index, volume);
      break;
    case 3:
      Calc3DElementVolume(index, volume);
      break;
    case 4:
      Calc2DTriangleElementVolume(index, volume);
      break;
    case 5:
      Calc3DTetraederElementVolume(index, volume);
      break;
    case 6:
      Calc3DPrismElementVolume(index, volume);
      break;
    }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc1DElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen des angegebenen 1D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume                                                       */

/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   16.10.1997   R.Kaiser    Erste Version
 */
/**************************************************************************/
void Calc1DElementVolume(long index, double *volume)
{
  static double delta_x, delta_y, delta_z, L2;
  static Knoten *k0;
  static Knoten *k1;
  static long *nodes;

  static double area;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  area = m_mmp->geo_area;

  nodes = ElGetElementNodes(index);

  k0 = GetNode(nodes[0]);
  k1 = GetNode(nodes[1]);
  delta_x = k1 -> x - k0 -> x;
  delta_y = k1 -> y - k0 -> y;
  delta_z = k1 -> z - k0 -> z;

  /* L^2 berechnen */
  L2 = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;

  *volume = sqrt(L2) * area;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen des angegebenen 2D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   16.10.1997    R.Kaiser    Erste Version
 */
/**************************************************************************/
void Calc2DElementVolume(long index, double *volume)
{
  static int i, j, anzgp;
  static double detjac, r, s, volumen;
  static double invjac[4];
  static double area;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  area = m_mmp->geo_area;

  volumen = 0.0;

/*  anzgp = ElGetGPNum(index); */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      {
        r = MXPGaussPkt(anzgp, i);
        s = MXPGaussPkt(anzgp, j);

        Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);

        volumen += (MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac);
      }

  *volume = volumen * area;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DTriangleElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen des angegebenen 2D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   05/2000   OK   Erste Version
   08/2000   OK   Beliebige Raumlage der Dreieckselemente
 */
/**************************************************************************/
void Calc2DTriangleElementVolumeOld(long index, double *volume)
{
  static double area;
  static double mat3x3[9];
  static long *element_nodes = NULL;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  element_nodes = ElGetElementNodes(index);
  mat3x3[0] = 1.0;
  mat3x3[1] = 1.0;
  mat3x3[2] = 1.0;
  mat3x3[3] = (GetNodeX(element_nodes[1]) - GetNodeX(element_nodes[0]));
  mat3x3[4] = (GetNodeY(element_nodes[1]) - GetNodeY(element_nodes[0]));
  mat3x3[5] = (GetNodeZ(element_nodes[1]) - GetNodeZ(element_nodes[0]));
  mat3x3[6] = (GetNodeX(element_nodes[2]) - GetNodeX(element_nodes[0]));
  mat3x3[7] = (GetNodeY(element_nodes[2]) - GetNodeY(element_nodes[0]));
  mat3x3[8] = (GetNodeZ(element_nodes[2]) - GetNodeZ(element_nodes[0]));

  area = m_mmp->geo_area;
  *volume = fabs(0.5 * M3Determinante(mat3x3) * area);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DTriangleElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen des angegebenen 2D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   05/2000   OK   Erste Version
   08/2000   OK   Beliebige Raumlage der Dreieckselemente
   03/2004   CMCD Correction of triangle volume and area function
			 Application of determinant value incorrect, the eq. 7.97
			 in Olafs book refers to three different axes.
 */
/**************************************************************************/
void Calc2DTriangleElementVolume(long index, double *volume)
{
  
  double sectionarea,surfacearea;
  double vec1[3],vec2[3],vec3[3];
  long *element_nodes = NULL;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  element_nodes = ElGetElementNodes(index);
  vec1[0] = (GetNodeX(element_nodes[1]) - GetNodeX(element_nodes[0]));
  vec1[1] = (GetNodeY(element_nodes[1]) - GetNodeY(element_nodes[0]));
  vec1[2] = (GetNodeZ(element_nodes[1]) - GetNodeZ(element_nodes[0]));
  vec2[0] = (GetNodeX(element_nodes[2]) - GetNodeX(element_nodes[0]));
  vec2[1] = (GetNodeY(element_nodes[2]) - GetNodeY(element_nodes[0]));
  vec2[2] = (GetNodeZ(element_nodes[2]) - GetNodeZ(element_nodes[0]));

  sectionarea = m_mmp->geo_area;
  //Forms the axis vec3 perpendicular to the vec1 and vec2
  M3KreuzProdukt(vec1, vec2, vec3);
  //Find vec3 length representing the area between vec1 and vec2, divide by 2 for triangle
  surfacearea = pow(MSkalarprodukt(vec3, vec3, 3),0.5)*0.5;
  *volume = sectionarea * surfacearea;
}

/* nur fuer 2-D xy Probleme */
void Calc2DTriangleElementVolumeV0(long index, double *volume)
{
  static double area;
  static double mat3x3[9];
  static long *element_nodes = NULL;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  element_nodes = ElGetElementNodes(index);
  mat3x3[0] = GetNodeX(element_nodes[0]);
  mat3x3[1] = GetNodeY(element_nodes[0]);
  mat3x3[2] = 1.0;
  mat3x3[3] = GetNodeX(element_nodes[1]);
  mat3x3[4] = GetNodeY(element_nodes[1]);
  mat3x3[5] = 1.0;
  mat3x3[6] = GetNodeX(element_nodes[2]);
  mat3x3[7] = GetNodeY(element_nodes[2]);
  mat3x3[8] = 1.0;

  area = m_mmp->geo_area;
  *volume = 0.5 * M3Determinante(mat3x3) * area;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc3DElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen des angegebenen 3D - Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   16.10.1997     R.Kaiser     Erste Version
 */
/**************************************************************************/
void Calc3DElementVolume(long index, double *volume)
{
  static int i, j, k, anzgp;
  static double detjac, r, s, t, volumen;
  static double invjac[9];

  volumen = 0.0;

/*  anzgp = ElGetGPNum(index); */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)
        {
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          t = MXPGaussPkt(anzgp, k);

          Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);

          /* Berechnung der Elementflaeche */
          volumen += (MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * \
                      MXPGaussFkt(anzgp, k) * detjac);
        }

  *volume = volumen; //??? fabs
}



/**************************************************************************/
/* ROCKFLOW - Funktion: Calc3DTetraederElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen eines Tetraeder-Elements
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   09/2000   OK   Erste Version
   07/2003 OK fabs
 */
/**************************************************************************/
void Calc3DTetraederElementVolume(long index, double *volume)
{
  static double mat4x4[16];
  static long *element_nodes = NULL;

  element_nodes = ElGetElementNodes(index);
  mat4x4[0] = 1.0;
  mat4x4[1] = GetNodeX(element_nodes[0]);
  mat4x4[2] = GetNodeY(element_nodes[0]);
  mat4x4[3] = GetNodeZ(element_nodes[0]);
  mat4x4[4] = 1.0;
  mat4x4[5] = GetNodeX(element_nodes[1]);
  mat4x4[6] = GetNodeY(element_nodes[1]);
  mat4x4[7] = GetNodeZ(element_nodes[1]);
  mat4x4[8] = 1.0;
  mat4x4[9] = GetNodeX(element_nodes[2]);
  mat4x4[10] = GetNodeY(element_nodes[2]);
  mat4x4[11] = GetNodeZ(element_nodes[2]);
  mat4x4[12] = 1.0;
  mat4x4[13] = GetNodeX(element_nodes[3]);
  mat4x4[14] = GetNodeY(element_nodes[3]);
  mat4x4[15] = GetNodeZ(element_nodes[3]);

  *volume = (M4Determinante(mat4x4) / 6.);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc3DPrismElementVolume
 */
/* Aufgabe:
   Berechnet das Volumen eines Prismen-Elements mit Dreiecksbasis
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *volume
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   09/2000   OK   Erste Version
 */
/**************************************************************************/
void Calc3DPrismElementVolume(long index, double *volume)
{
  static int i, j;
  static int anzgptri, anzgplin;  
  static double detjac, r, s, t, volumen;
  static double invjac[9];
  anzgptri = 3;
  anzgplin = 2; 
  volumen = 0.0;
  /* Schleife ueber GaussPunkte */
  for (i = 0; i < anzgplin; i++)  {
    for (j = 0; j < anzgptri; j++)  {
        
      r = MXPGaussPktTri(anzgptri,j,0);
      s = MXPGaussPktTri(anzgptri,j,1);
      t = MXPGaussPkt(anzgplin,i);

      CalcPrismElementJacobiMatrix(index, r, s, t, invjac, &detjac);
      
      volumen += MXPGaussFktTri(anzgptri,j) * MXPGaussFkt(anzgplin,i) * detjac;
      volumen = fabs(volumen);

    }  /* Schleife über Anzahl der GaussPunkte im Dreieck (xy-Richtung) */
  }    /* Schleife über Anzahl der GaussPunkte in z Richtung */

  *volume = volumen;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcBaseAreaPrism
 */
/* Aufgabe:
   Berechnet die Fläche des Grundrisses eines Prismas
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *area
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   01/2003   MB   Erste Version
 */
/**************************************************************************/
double CalcBaseAreaPrism(long index)
{
  static double area;
  static double mat3x3[9];
  static long *element_nodes = NULL;

  element_nodes = ElGetElementNodes(index);
  mat3x3[0] = 1.0;
  mat3x3[1] = 1.0;
  mat3x3[2] = 1.0;
  mat3x3[3] = (GetNodeX(element_nodes[1]) - GetNodeX(element_nodes[0]));
  mat3x3[4] = (GetNodeY(element_nodes[1]) - GetNodeY(element_nodes[0]));
  mat3x3[5] = 0;
  mat3x3[6] = (GetNodeX(element_nodes[2]) - GetNodeX(element_nodes[0]));
  mat3x3[7] = (GetNodeY(element_nodes[2]) - GetNodeY(element_nodes[0]));
  mat3x3[8] = 0;  
  area = fabs(0.5 * M3Determinante(mat3x3));
  return area; 
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InterpolValue
 */
/* Aufgabe:
   Berechnet die Interpolation einer Knotengroesse auf einen Punkt
   im lokalen System
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Elements
   E  int    ndx: Index des Elements
   E double    r: Lokale Koordinate
   R double    c: Interpolierter Wert
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   26.01.98  C.Thorenz     Erste Version
   16.01.01  C.Thorenz     Zweite Version
   05.05.05 WW Re-implementing
   16.06.06 YD 1D correcting
 */
/**************************************************************************/
double InterpolValue(long number, int ndx, double r, double s, double t)
{

  static long *nodes;
  static int i, nn;
  static double Shape[20],u[3];
  double Val;

  nodes = ElGetElementNodes(number);
  nn = ElNumberOfNodes[ElGetElementType(number) - 1];

  //WW-----------------------------------------
  u[0] = r;
  u[1] = s;
  u[2] = t;
  VoidFuncDXCDX ShapeFunction=NULL;
  switch(ElGetElementType(number))
  {
	 case 1: // Line
       ShapeFunction = ShapeFunctionLine;
	   break;    
	 case 2: // Quadrilateral 
       ShapeFunction = ShapeFunctionQuad;
	   break;    
     case 3: // Hexahedra 
       ShapeFunction = ShapeFunctionHex;
	   break;
     case 4: // Triangle 
        // The following condition has to be removed
        // if functions calling this function are checked
       if(fabs(r)<MKleinsteZahl&&fabs(s)<MKleinsteZahl)
         u[0] = u[1] = 0.5;  
       ShapeFunction = ShapeFunctionTri;
	   break;
     case 5: // Tedrahedra 
        // The following condition has to be removed
        // if functions calling this function are checked
       if(fabs(r)<MKleinsteZahl&&fabs(s)<MKleinsteZahl
          &&fabs(t)<MKleinsteZahl)
          u[0] = u[1] = u[2] = 0.5;  
       ShapeFunction = ShapeFunctionTet;
	   break;
     case 6: // Pris 
      Shape[0] = 0.166666667;
      Shape[1] = 0.166666667;
      Shape[2] = 0.166666667;
      Shape[3] = 0.166666667;
      Shape[4] = 0.166666667;
      Shape[5] = 0.166666667;
      break;
  }
  if(ElGetElementType(number)!=6)
      ShapeFunction(Shape, u); 
   
  Val = 0.0;
  for (i = 0; i < nn; i++)
    Val += GetNodeVal(nodes[i], ndx)*Shape[i];
  return Val;
  //WW-----------------------------------------
 // return InterpolValueVector(number, k, r, s, t);

}



/**************************************************************************
ROCKFLOW - Funktion: InterpolValueVector
Aufgabe:
   Berechnet die Interpolation der in einem Vektor uebergegebenen
   Knotenwerte auf einen Punkt im lokalen System
Programmaenderungen:
16.01.01  C.Thorenz     Erste Version
04/2002 OK Dreiecke
06/2005 WW
06/2005 OK ele_type
**************************************************************************/
double InterpolValueVector(long ele_type, double *vec, double r, double s, double t)
{
  static double vf[8];
  int nn;
  switch(ele_type) //(ElGetElementType(number))
    {
    case (1):
      MOmega1D(&vf[0], r);
      nn = 2;
      break;

    case (2):
      MOmega2D(&vf[0], r, s);
      nn = 4;
      break;

    case (3):
      MOmega3D(&vf[0], r, s, t);
      nn = 8;
      break;

    case (4): /* lineare Dreiecke */
      MOmega2DTriangle(&vf[0],r,s,ele_type);
      nn = 3;
      break;

    case (5): /* linear tetrahedron */
      /*MOmega3DTetrahedron(&vf[0],r,s,t,number);*/
      vf[0]=0.25;
      vf[1]=0.25;
      vf[2]=0.25;
      vf[3]=0.25;
      nn = 4;
      break;

    case (6): /* Dreiecksprismen */
      vf[0] = 0.166666667;
      vf[1] = 0.166666667;
      vf[2] = 0.166666667;
      vf[3] = 0.166666667;
      vf[4] = 0.166666667;
      vf[5] = 0.166666667;
      nn = 6;
      break;

    default:
      DisplayMsgLn("Fehler in InterpolValue: Falscher Elementtyp");
      return -1.e99;
    }                                  /* switch */

//WW  return MSkalarprodukt(&vf[0], &vec[0], nn);
  return MSkalarprodukt(vf, vec, nn);

}



/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_xyz2ab_Coord
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element und die vorgegebenen
   x,y,z-Koordinaten die zugehoerigen a,b-Koordinaten
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double x,y,z: Koordinaten im globalen System
   X double *vec: Vektor der Knotenkoordinaten im a,b - System
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02.02.1998     R.Kaiser     Erste Version
 */
/**************************************************************************/
void Calc2DElement_xyz2ab_Coord(long index, double *vec, double x, double y, double z)
{
  static int i;                 /* Laufvariable */
  static double vf[12];         /* x,y,z - Koordinatenmatrix (Elementknoten) */
  static double epsilon;
  static double trans[6];       /* Transformationsmatrix */
  static double coord[3];       /* Vektor fuer die x,y,z - Koordinaten */

  epsilon = 1.e-8;

  /* Transformationsmatrix */
  CalcTransformationsMatrix(index, vf, trans);

  /* (x,y,z - Koordinaten) - (Koordinatenursprung des a,b - Systems) */
  coord[0] = x - vf[0];
  coord[1] = y - vf[1];
  coord[2] = z - vf[2];

  /* Transformation in das a,b - System */
  MMultVecMat(coord, 3, trans, 3, 2, vec, 2);

  for (i = 0; i < 2; i++)
    if (fabs(vec[i]) < epsilon)
      vec[i] = 0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_ab2xyz_Coord
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element und die vorgegebenen
   a,b-Koordinaten die zugehoerigen x,y,z-Koordinaten
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double a,b: Knotenkoordinaten im a,b - System
   X double *vec: Vektor der Knotenkoordinaten im x,y,z - System
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02.02.1998     R.Kaiser     Erste Version
 */
/**************************************************************************/
void Calc2DElement_ab2xyz_Coord(long index, double *vec, double a, double b)
{
  static int i;                 /* Laufvariable */
  static double epsilon;
  static double vf[12];         /* x,y,z - Koordinatenmatrix (Elementknoten) */
  static double trans[6];       /* Transformationsmatrix */
  static double vec1[3], vec2[3];       /* Orthonormierte Basisvektoren
                                           des a,b-Systems */

  epsilon = 1.e-8;

  CalcTransformationsMatrix(index, vf, trans);

  vec1[0] = trans[0];
  vec1[1] = trans[2];
  vec1[2] = trans[4];
  vec2[0] = trans[1];
  vec2[1] = trans[3];
  vec2[2] = trans[5];

  for (i = 0; i < 3; i++)
    {
      vec[i] = vf[i] + a * vec1[i] + b * vec2[i];
      if (fabs(vec[i]) < epsilon)
        vec[i] = 0.0;
    }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_rs2ab_Coord
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element und die vorgegebenen
   r,s-Koordinaten die zugehoerigen a,b-Koordinaten
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s: Koordinaten im lokalen r,s - System
   X double *vec: Vektor der Knotenkoordinaten im a,b - System
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02.02.1998     R.Kaiser     Erste Version
 */
/**************************************************************************/
void Calc2DElement_rs2ab_Coord(long index, double *vec, double r, double s)
{
  static int i;                 /* Laufvariable */
  static double epsilon;
  static double vf[12], vfneu[12];      /* x,y,z - Koordinatenmatrix */
  static double trans[6];       /* Transformationsmatrix */
  static double coord[8];       /* Koordinaten der Elementpunkte im a,b - System */

  epsilon = 1.e-8;

  CalcTransformationsMatrix(index, vf, trans);

  vfneu[0] = vf[0] - vf[0];
  vfneu[1] = vf[1] - vf[1];
  vfneu[2] = vf[2] - vf[2];
  vfneu[3] = vf[3] - vf[0];
  vfneu[4] = vf[4] - vf[1];
  vfneu[5] = vf[5] - vf[2];
  vfneu[6] = vf[6] - vf[0];
  vfneu[7] = vf[7] - vf[1];
  vfneu[8] = vf[8] - vf[2];
  vfneu[9] = vf[9] - vf[0];
  vfneu[10] = vf[10] - vf[1];
  vfneu[11] = vf[11] - vf[2];

  MMultMatMat(vfneu, 4, 3, trans, 3, 2, coord, 4, 2);

  vec[0] = 0.25 * (1 + r) * (1 + s) * coord[0] + \
      0.25 * (1 - r) * (1 + s) * coord[2] + \
      0.25 * (1 - r) * (1 - s) * coord[4] + \
      0.25 * (1 + r) * (1 - s) * coord[6];
  vec[1] = 0.25 * (1 + r) * (1 + s) * coord[1] + \
      0.25 * (1 - r) * (1 + s) * coord[3] + \
      0.25 * (1 - r) * (1 - s) * coord[5] + \
      0.25 * (1 + r) * (1 - s) * coord[7];



  for (i = 0; i < 2; i++)
    if (fabs(vec[i]) < epsilon)
      vec[i] = 0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_ab2rs_Coord
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element und die vorgegebenen
   a,b-Koordinaten die zugehoerigen r,s-Koordinaten
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double a,b: Koordinaten im a,b - System
   X double *vec: Vektor der Knotenkoordinaten im r,s - System
 */
/* Ergebnis:
   -void -
 */
/* Programmaenderungen:
   02.02.1998     R.Kaiser     Erste Version
   05.11.1999     R.Kaiser     Korrektur
   TODO: Vollstaendige Ueberarbeitung !!!!!
 */
/**************************************************************************/
void Calc2DElement_ab2rs_Coord(long index, double *vec, double a, double b)
{
  static int i, j;
  static double epsilon;
  static double ab1[2], ab2[2], ab3[2], ab4[2], ab[2];
  static double wert1, wert2, wert3, wert4, wert5, wert6, wert7, wert8, wert[8];
  static double k, l, m, s1, s2, r1, r2, e, f;

  s1 = -1000.0;
  s2 = -1000.0;
  r1 = -1000.0;
  r2 = -1000.0;
  i = 0;
  epsilon = 1.e-8;

  Calc2DElement_rs2ab_Coord(index, ab1, 1.0, 1.0);
  Calc2DElement_rs2ab_Coord(index, ab2, -1.0, 1.0);
  Calc2DElement_rs2ab_Coord(index, ab3, -1.0, -1.0);
  Calc2DElement_rs2ab_Coord(index, ab4, 1.0, -1.0);

  wert[0] = ab1[1] + ab2[1] + ab3[1] + ab4[1];
  wert[1] = ab1[1] + ab2[1] - ab3[1] - ab4[1];
  wert[2] = ab1[1] - ab2[1] - ab3[1] + ab4[1];
  wert[3] = ab1[1] - ab2[1] + ab3[1] - ab4[1];
  wert[4] = ab1[0] + ab2[0] + ab3[0] + ab4[0];
  wert[5] = ab1[0] + ab2[0] - ab3[0] - ab4[0];
  wert[6] = ab1[0] - ab2[0] - ab3[0] + ab4[0];
  wert[7] = ab1[0] - ab2[0] + ab3[0] - ab4[0];

  wert1 = wert[0];
  wert2 = wert[1];
  wert3 = wert[2];
  wert4 = wert[3];
  wert5 = wert[4];
  wert6 = wert[5];
  wert7 = wert[6];
  wert8 = wert[7];

  k = wert2 * wert8 - wert6 * wert4;
  l = -4.0 * b * wert8 + wert1 * wert8 + wert2 * wert7 + \
      4.0 * a * wert4 - wert5 * wert4 - wert6 * wert3;
  m = 4.0 * b * wert7 - wert1 * wert7 - 4.0 * a * wert3 + wert3 * wert5;

  if (fabs(k)>  MKleinsteZahl )
    {
      s1 = -1.0 * (l / k) / 2.0 + sqrt((pow((l / k), 2) / 4) + (m / k));
      s2 = -1.0 * (l / k) / 2.0 - sqrt((pow((l / k), 2) / 4) + (m / k));

      if ((s1 > -1.001) && (s1 < 1.001))
        {
          e = 4.0 * a - wert5 - s1 * wert6;
          f = wert7 + s1 * wert8;
          if (fabs(f)>  MKleinsteZahl )
            {
              r1 = e / f;
              if ((r1 > -1.001) && (r1 < 1.001))
                {
                  vec[0] = r1;
                  vec[1] = s1;
                  for (j = 0; j < 2; j++)
                    if (fabs(vec[j]) < epsilon)
                      vec[j] = 0.0;
                  i++;
                }
            }
        }
      if ((s2 > -1.001) && (s2 < 1.001))
        {
          e = 4.0 * a - wert5 - s2 * wert6;
          f = wert7 + s2 * wert8;
          if (fabs(f)> MKleinsteZahl)
            {
              r2 = e / f;
              if ((r2 > -1.001) && (r2 < 1.001))
                {
                  vec[0] = r2;
                  vec[1] = s2;
                  for (j = 0; j < 2; j++)
                    if (fabs(vec[j]) < epsilon)
                      vec[j] = 0.0;
                  i++;
                }
            }
        }


      /* Ueberpruefung der Ergebnisse (TEST) */
   //WW      if ((r1 != -1000.0) && (s1 != -1000.0))
      if ((fabs(r1+1000.0)> MKleinsteZahl) && (fabs(s1+1000.0)> MKleinsteZahl ))
        {
          Calc2DElement_rs2ab_Coord(index, ab, r1, s1);
	  //WW          if ((ab[0] != a) || (ab[1] != b))
          if ((fabs(ab[0]- a)> MKleinsteZahl) || (fabs(ab[1] - b)> MKleinsteZahl))
            i = 0;
        }
      //     if ((r2 != -1000.0) && (s2 != -1000.0))
      if ((fabs(r2+1000.0)> MKleinsteZahl) && (fabs(s2+1000.0)> MKleinsteZahl ))
        {
          Calc2DElement_rs2ab_Coord(index, ab, r2, s2);
	  //WW          if ((ab[0] != a) || (ab[1] != b))
          if ((fabs(ab[0]- a)> MKleinsteZahl) || (fabs(ab[1] - b)> MKleinsteZahl))
            i = 0;
        }

      if (i == 0)
        {
          if (fabs(l)>  MKleinsteZahl)
            {
              s1 = m / l;
              e = 4.0 * a - wert5 - s1 * wert6;
              f = wert7 + s1 * wert8;
              if (fabs(f)>  MKleinsteZahl)
                {
                  r1 = e / f;
                  if (((r1 > -1.001) && (r1 < 1.001)) && ((s1 > -1.001) && (s1 < 1.001)))
                    {
                      vec[0] = r1;
                      vec[1] = s1;
                      for (j = 0; j < 2; j++)
                        if (fabs(vec[j]) < epsilon)
                          vec[j] = 0.0;
                    }
                }
              else
                {
                  DisplayMsgLn("ElementNr.: ");
                  DisplayLong(index);
                  DisplayMsgLn("");
                  DisplayMsgLn("Koordinatentransformation3 ab -> rs nicht moeglich !");
                }
            }
          else
            {
              DisplayMsgLn("ElementNr.: ");
              DisplayLong(index);
              DisplayMsgLn("");
              DisplayMsgLn("Koordinatentransformation4 ab->rs nicht moeglich !");
            }


        }
      if (i == 2)
        {
          DisplayMsgLn("ElementNr.: ");
          DisplayLong(index);
          DisplayMsgLn("");
          DisplayMsgLn("Koordinatentransformation2 ab->rs nicht eindeutig !");
        }
    }
  else
    {
      if (fabs(l)>  MKleinsteZahl)
        {
          s1 = m / l;
          e = 4.0 * a - wert5 - s1 * wert6;
          f = wert7 + s1 * wert8;
          if (fabs(f)>  MKleinsteZahl)
            {
              r1 = e / f;
              if (((r1 > -1.001) && (r1 < 1.001)) && ((s1 > -1.001) && (s1 < 1.001)))
                {
                  vec[0] = r1;
                  vec[1] = s1;
                  for (j = 0; j < 2; j++)
                    if (fabs(vec[j]) < epsilon)
                      vec[j] = 0.0;
                }
            }
          else
            {
              DisplayMsgLn("ElementNr.: ");
              DisplayLong(index);
              DisplayMsgLn("");
              DisplayMsgLn("Koordinatentransformation3 ab -> rs nicht moeglich !");
            }
        }
      else
        {
          DisplayMsgLn("ElementNr.: ");
          DisplayLong(index);
          DisplayMsgLn("");
          DisplayMsgLn("Koordinatentransformation4 ab->rs nicht moeglich !");
        }

    }

}

/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DEdgeUnitNormalVec
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element und die angegebene Kante
   den nach aussen gereichteten Einheitsnormalenvektor
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E long side: Kante
   X double *n: Normaleneinheitsvektor
 */
/* Ergebnis:
   -void -
 */
/* Programmaenderungen:
   20.02.1998     R.Kaiser     Erste Version
 */
/**************************************************************************/
void Calc2DEdgeUnitNormalVec(long index, long side, double *n)
{
  static int i, k;
  static long *edges;
  static long nd1[2], nd2[2], kn1, kn2;
  static double side1[3], side2[3];
  static double vec[3];

  edges = ElGetElementEdges(index);    /* Kanten des betrachteten
                                          Elements holen */

  k = 0;
  for (i = 0; i < 4; i++)
    if (edges[i] == side)
      {
        k = k + i;
        break;
      }

  for (i = 0; i < 2; i++)
    {
      /* Knotennummern von zwei Kanten holen */
      nd1[i] = GetEdge(edges[k]) -> knoten[i];
      nd2[i] = GetEdge(edges[(k + 1) % 4]) -> knoten[i];
    }

  /* Knotennummern fuer weitere Rechnungen umsortieren */
  if ((nd1[0] != nd2[0]) && (nd1[1] != nd2[1]))
    {
      kn2 = nd2[0];
      nd2[0] = nd2[1];
      nd2[1] = kn2;
    }
  if (nd1[1] == nd2[1])
    {
      kn1 = nd1[0];
      kn2 = nd2[0];
      nd1[0] = nd1[1];
      nd2[0] = nd2[1];
      nd1[1] = kn1;
      nd2[1] = kn2;
    }

  /* Zwei Elementseiten (Vektoren) berechnen */
  side1[0] = GetNode(nd1[1]) -> x - GetNode(nd1[0]) -> x;
  side1[1] = GetNode(nd1[1]) -> y - GetNode(nd1[0]) -> y;
  side1[2] = GetNode(nd1[1]) -> z - GetNode(nd1[0]) -> z;
  side2[0] = GetNode(nd2[1]) -> x - GetNode(nd2[0]) -> x;
  side2[1] = GetNode(nd2[1]) -> y - GetNode(nd2[0]) -> y;
  side2[2] = GetNode(nd2[1]) -> z - GetNode(nd2[0]) -> z;

  /* Normalenvektor der Elementflaeche berechnen */
  M3KreuzProdukt(side1, side2, vec);

  /* Nach aussen gerichteten Normaleneinheitsvektor berechnen */
  M3KreuzProdukt(side1, vec, n);
  MNormiere(n, 3);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_xyz2ab_Vector
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element die Transformation
   eines Vektors von xyz in ab-Koordinaten

 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *vec_xyz: Vektor im xyz - System
   X double *vec_ab : Vektor im a,b - System
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   7/2000 C.Thorenz  Erste Version
 */
/**************************************************************************/
void Calc2DElement_xyz2ab_Vector(long index, double *vec_xyz, double *vec_ab)
{
  double vf[12];                /* x,y,z - Koordinatenmatrix (Elementknoten) */
  double trans[6];              /* Transformationsmatrix */

  /* Transformationsmatrix */
  CalcTransformationsMatrix(index, vf, trans);

  /* Transformation in das a,b - System */
  MMultVecMat(vec_xyz, 3, trans, 3, 2, vec_ab, 2);

}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElement_ab2xyz_Vector
 */
/* Aufgabe:
   Berechnet fuer das angegebene 2D-Element die Transformation
   eines Vektors von xyz in ab-Koordinaten

 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *vec_ab : Vektor im a,b - System
   X double *vec_xyz: Vektor im xyz - System
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   7/2000 C.Thorenz  Erste Version
 */
/**************************************************************************/
void Calc2DElement_ab2xyz_Vector(long index, double *vec_ab, double *vec_xyz)
{
  int i;                        /* Laufvariable */
  double vf[12];                /* x,y,z - Koordinatenmatrix (Elementknoten) */
  double trans[6];              /* Transformationsmatrix */
  double vec1[3], vec2[3];      /* Orthonormierte Basisvektoren des a,b-Systems */

  CalcTransformationsMatrix(index, vf, trans);

  vec1[0] = trans[0];
  vec1[1] = trans[2];
  vec1[2] = trans[4];
  vec2[0] = trans[1];
  vec2[1] = trans[3];
  vec2[2] = trans[5];

  for (i = 0; i < 3; i++)
    vec_xyz[i] = vec_ab[0] * vec1[i] + vec_ab[1] * vec2[i];

}


/**************************************************************************
FEMLib-Method: CalcElementsGeometry
Task: 
   Berechnung geometrischer Elementdaten:
   - Jakobian
   - Element-Volumen
Programing:
12/1999 OK Implementation
**************************************************************************/
void CalcElementsGeometry(void)
{
  long i;
  Element* element = NULL;
  for(i=0;i<ElListSize();i++){
    element = ElGetElement(i);
    if((element)&&(element->aktiv)&&(element->group_number>-1)){
      // Initialisieren der Element-Geometrie
      //Jacobian und Volumenberechnung 
      InitElementGeometry(i);
    }
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElementJacobiMatrix_9N
 */
/* Aufgabe:
   Berechnet die Inverse und die Determinante der Jacobi-Matrix /
   Drehvektor des angegebenen 2D - Elements fuer zweidim. 9-Knoten-Elemente

   -1               T -1 T
   y  = ((grad(w) A T)  )


   -1
   y         - inverse Jacobi-Matrix

   grad(w)   - Gradientenmatrix der Ansatzfunktion

   A         - Koordinatenmatrix

   T
   T         - Transponierte der Transformationsmatrix
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s
   X double *erg
   X double *dj
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   02/2000   RK   Erste Version
   07/2000   OK   Variante 2 (nach Theorie)
 */
/**************************************************************************/
void Calc2DElementJacobiMatrix_9N(long index, double r, double s, double *erg, double *dj)
{
  static double koma[27];
  static double grd[18];
  static double zw[6];
  static double t[6];
  /* static double a_2D[18]; */

  CalcTransformationsMatrix_9N(index, koma, t);
  /* MZeigMat(koma,4,3,"KOORDINATEN"); */
  /* MZeigMat(t,3,2,"TRANS"); */
  /* zunaechst nur fuer echte 2-D Probleme */
  t[0] = 1.0;                          /* cos(x,x') */
  t[1] = 0.0;                          /* cos(x,y') */
  t[2] = 0.0;                          /* cos(y,x') */
  t[3] = 1.0;                          /* cos(y,y') */
  t[4] = 0.0;                          /* cos(z,x') */
  t[5] = 1.0;                          /* cos(z,y') */


  MGradOmega2D_9N(grd, r, s);
  /* MZeigMat(grd,2,4,"GRADIENT"); */

  /* Variante 1 */
  MMultMatMat(grd, 2, 9, koma, 9, 3, zw, 2, 3);
  /* MZeigMat(zw,2,3,"[grad_rs N]^T : [A_3D]"); */
  MMultMatMat(zw, 2, 3, t, 3, 2, erg, 2, 2);
  /* MZeigMat(erg,2,2,"Variante 1 [J_2D]"); */

  /* Variante 2 - nach Theorie
     MMultMatMat(koma,9,3,t,3,2,a_2D,9,2);
     MZeigMat(a_2D,9,2,"[A_2D'] = [A_3D] : [2D_T_3D]^T");
     MMultMatMat(grd,2,9,a_2D,9,2,erg,2,2);
     MZeigMat(erg,2,2,"Variante 2 [J_2D]"); */

  *dj = M2Determinante(erg);
  M2InvertiereUndTransponiere(erg);  /* Invertieren + Transponieren ist korrekt */
  /* MZeigMat(erg,2,2,"[J_2D^(-1)]^T"); */
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcTransformationsMatrix_9N
 */
/* Aufgabe:
   Berechnet die Koordinatenmatrix und die Transformationsmatrix
   a,b -> x,y,z des angegebenen 2D - Elements (9-Knoten-Element)          */

/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *koma: Koordinatenmatrix (4x3)
   X double *trans: Transformationsmatrix (3x2)
 */
/* Ergebnis:
   - Matrizen -
 */
/* Programmaenderungen:
   02/2000     R.Kaiser  Erste Version

 */
/**************************************************************************/
void CalcTransformationsMatrix_9N(long index, double *koma, double *trans)
{
  static double vec2[3];
  static double vec1[3];
  static double winkel;         /* kann zur Zeit nicht ueber Eingabe eingelesen werden */
  /* Zur Zeit sind nur isotrope Probleme beruecksichtigt */
  static Knoten *nd1;
  static Knoten *nd2;
  static Knoten *nd3;
  static Knoten *nd4;
  static Knoten *nd5;
  static Knoten *nd6;
  static Knoten *nd7;
  static Knoten *nd8;
  static Knoten *nd9;
  static long *nodes;

  nodes = ElGetElementNodes(index);
  winkel = 0.0;

  nd1 = GetNode(nodes[0]);
  nd2 = GetNode(nodes[1]);
  nd3 = GetNode(nodes[2]);
  nd4 = GetNode(nodes[3]);
  nd5 = GetNode(nodes[4]);
  nd6 = GetNode(nodes[5]);
  nd7 = GetNode(nodes[6]);
  nd8 = GetNode(nodes[7]);
  nd9 = GetNode(nodes[8]);

  /* Koordinatenmatrix A (koma) */
  koma[0] = nd1 -> x;
  koma[1] = nd1 -> y;
  koma[2] = nd1 -> z;

  koma[3] = nd2 -> x;
  koma[4] = nd2 -> y;
  koma[5] = nd2 -> z;

  koma[6] = nd3 -> x;
  koma[7] = nd3 -> y;
  koma[8] = nd3 -> z;

  koma[9] = nd4 -> x;
  koma[10] = nd4 -> y;
  koma[11] = nd4 -> z;

  koma[12] = nd5 -> x;
  koma[13] = nd5 -> y;
  koma[14] = nd5 -> z;

  koma[15] = nd6 -> x;
  koma[16] = nd6 -> y;
  koma[17] = nd6 -> z;

  koma[18] = nd7 -> x;
  koma[19] = nd7 -> y;
  koma[20] = nd7 -> z;

  koma[21] = nd8 -> x;
  koma[22] = nd8 -> y;
  koma[23] = nd8 -> z;

  koma[24] = nd9 -> x;
  koma[25] = nd9 -> y;
  koma[26] = nd9 -> z;

  /* zwei Vektoren aus der Element-Ebene
     fuer die Transformationsmatrix */
  vec1[0] = koma[3] - koma[0];
  vec1[1] = koma[4] - koma[1];
  vec1[2] = koma[5] - koma[2];
  vec2[0] = koma[9] - koma[0];
  vec2[1] = koma[10] - koma[1];
  vec2[2] = koma[11] - koma[2];

  MKTFMat3Dr2D(vec1, vec2, winkel, trans);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElementJacobiMatrixXY_9N
 */
/* Aufgabe:
   Berechnet die Inverse und die Determinante der Jacobi-Matrix
   im globalen x-y Koordinatensystem
         T 
   [J_2D] = [grad(N)] : [A_2D]
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s
   X double *erg
   X double *dj
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   07/2000   OK   Variante 2 (nach Theorie)
 */
/**************************************************************************/
void Calc2DElementJacobiMatrixXY_9N(long index, double r, double s, double *j_2d, double *dj)
{
  static double a_2d[18];
  static double g_ome_rs[18];
  static long *nodes;

  MGradOmega2D_9N(g_ome_rs, r, s);
  MZeigMat(g_ome_rs, 2, 9, "Calc2DElementJacobiMatrixXY_9N: g_ome_rs");

  nodes = ElGetElementNodes(index);

  /* Koordinatenmatrix A (koma) */
  a_2d[0] = GetNodeX(nodes[0]);
  a_2d[1] = GetNodeY(nodes[0]);
  a_2d[2] = GetNodeX(nodes[1]);
  a_2d[3] = GetNodeY(nodes[1]);
  a_2d[4] = GetNodeX(nodes[2]);
  a_2d[5] = GetNodeY(nodes[2]);
  a_2d[6] = GetNodeX(nodes[3]);
  a_2d[7] = GetNodeY(nodes[3]);
  a_2d[8] = GetNodeX(nodes[4]);
  a_2d[9] = GetNodeY(nodes[4]);
  a_2d[10] = GetNodeX(nodes[5]);
  a_2d[11] = GetNodeY(nodes[5]);
  a_2d[12] = GetNodeX(nodes[6]);
  a_2d[13] = GetNodeY(nodes[6]);
  a_2d[14] = GetNodeX(nodes[7]);
  a_2d[15] = GetNodeY(nodes[7]);
  a_2d[16] = GetNodeX(nodes[8]);
  a_2d[17] = GetNodeY(nodes[8]);

  /* Variante 1 */
  MMultMatMat(g_ome_rs, 2, 9, a_2d, 9, 2, j_2d, 2, 2);
  MZeigMat(j_2d, 2, 2, "Calc2DElementJacobiMatrixXY_9N: [J_2D] = [grad_rs N]^T : [A_3D]");

  /* Variante 2 - nach Theorie
     MMultMatMat(koma,9,3,t,3,2,a_2D,9,2);
     MZeigMat(a_2D,9,2,"[A_2D'] = [A_3D] : [2D_T_3D]^T");
     MMultMatMat(grd,2,9,a_2D,9,2,erg,2,2);
     MZeigMat(erg,2,2,"Variante 2 [J_2D]"); */

  *dj = M2Determinante(j_2d);
  M2InvertiereUndTransponiere(j_2d); /* Invertieren + Transponieren ist korrekt */
  MZeigMat(j_2d, 2, 2, "Calc2DElementJacobiMatrixXY_9N: [J_2D^(-1)]^T");
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc3DElementJacobiMatrix_20N
                                                                          */
/* Aufgabe:
   Berechnet die Inverse Jacobi-Matrix des angegebenen 3D - Elements 
   (dreidim, 20 Knoten)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s,t
   X double *erg
   X double *dj
                                                                          */
/* Ergebnis:
   - double -
                                                                          */
/* Programmaenderungen:
   10/2001     R.Kaiser     Erste Version
                                                                          */
/**************************************************************************/
void Calc3DElementJacobiMatrix_20N(long index, double r, double s, double t, double *erg, double *dj)
{
  static double koma[60];
  static double grd[60];
  static Knoten *nd;
  static long *nodes;
  static long i;
  /* Koordinatenmatrix A */
  nodes = ElGetElementNodes(index);
  for (i = 0; i < 20; i++)
    {
      nd = GetNode(nodes[i]);
      koma[i * 3] = nd -> x;
      koma[i * 3 + 1] = nd -> y;
      koma[i * 3 + 2] = nd -> z;
    }
  MGradOmega3D_20N(grd, r, s, t);
  MMultMatMat(grd, 3, 20, koma, 20, 3, erg, 3, 3);
  *dj = M3Determinante(erg);
  M3Invertiere(erg);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: IncorporateMatrix
 */
/* Aufgabe:
   Speichert Elementmatrix und Rechte-Seite-Vektor in
   Gleichungssystem ein          */

/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long element: Index des Elements
   X double *left_matrix: Matrix des Elements
   X double *right_vector: Rechte Seite-Vektor des Elements
   X double *rechts: Rechte Seite-Vektor des Gleichungssystems
 */
/* Ergebnis:

 */
/* Programmaenderungen:
   10/2000   CT   Erste Version
   11/2000   OK   ??? warum in femlib.c
 */
/**************************************************************************/
void IncorporateMatrix(long element, double *left_matrix, double *right_vector, double *rechts)
{
  register int i, j, nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  static long element_node_index[8];

  /* Knotennummern in element_node_index geben an, wo sich die Knoten des Elements
     als Eintraege im Gesamtgleichungssystem finden lassen */
  for (i = 0; i < nn; i++)
    element_node_index[i] = GetNodeIndex(ElGetElementNodes(element)[i]);

  /* irregulaere Knoten eliminieren */
  if (GetRFControlGridAdapt())
    {
      if (AdaptGetMethodIrrNodes() == 1)
        {
          if (ElGetElementType(element) == 2)
            {
              DelIrrNodes2D(element, element_node_index, left_matrix, right_vector);
            }                          /* endif */
          if (ElGetElementType(element) == 3)
            {
              DelIrrNodes3D(element, element_node_index, left_matrix, right_vector);
            }                          /* endif */
        }
    }

  /* Einspeichern in linke Seite */
  for (i = 0; i < nn; i++)
    for (j = 0; j < nn; j++)
      MXInc(element_node_index[i], element_node_index[j], left_matrix[i * nn + j]);

  /* Einspeichern in rechte Seite */
  for (i = 0; i < nn; i++)
    rechts[element_node_index[i]] += right_vector[i];
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Calc2DElementCoordinatesTriangle
 */
/* Aufgabe: Element-Koordinaten im lokalen Element-System (x',y')==(a,b)
   Koordinatenmatrix

          | x1 y1 z1 |
   A_3D = | x2 y2 z2 |   [3x3]
          | x3 y3 z3 |

   Transformationsmatrix
   a,b -> x,y,z fuer ein lineares Dreieckselement

             | cos(x',x) cos(y',x) |
   2D_T_3D = | cos(x',y) cos(y',y) |   [3x2]
             | cos(x',z) cos(y',z) |

 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   X double *vec1: x'-Koordinaten fuer die drei Knoten
   X double *vec2: y'-Koordinaten fuer die drei Knoten
 */
/* Ergebnis:
   - Matrizen -
 */
/* Programmaenderungen:
   08/2000   OK   aus CalcTransformationsMatrix (RK) abgeleitet
   04/2004   CMCD Included directional cosin vector
 */
/**************************************************************************/
void Calc2DElementCoordinatesTriangle(long index, double *vec1, double *vec2, double *dircos)
{
  static Knoten *nd1;
  static Knoten *nd2;
  static Knoten *nd3;
  static long *nodes;
  double koma[9]; //CMCD
  double trans[6];//CMCD
  double xy[6];//CMCD

  nodes = ElGetElementNodes(index);    /* ? Test ob Dreieckselement */
  nd1 = GetNode(nodes[0]);
  nd2 = GetNode(nodes[1]);
  nd3 = GetNode(nodes[2]);

  /* Koordinatenmatrix [A_3D] (koma) */
  koma[0] = nd1 -> x;
  koma[1] = nd1 -> y;
  koma[2] = nd1 -> z;
  koma[3] = nd2 -> x;
  koma[4] = nd2 -> y;
  koma[5] = nd2 -> z;
  koma[6] = nd3 -> x;
  koma[7] = nd3 -> y;
  koma[8] = nd3 -> z;

  /* zwei Vektoren aus der Element-Ebene fuer die Transformationsmatrix */
  /*As these vectors are based on point 1, point 1 now becomes 0,0,0 for the transformed element*/
  /*The transformation is then described as (+x1,+y1,+z1)*/
  vec1[0] = koma[3] - koma[0];
  vec1[1] = koma[4] - koma[1];
  vec1[2] = koma[5] - koma[2];
  vec2[0] = koma[6] - koma[0];
  vec2[1] = koma[7] - koma[1];
  vec2[2] = koma[8] - koma[2];

  /* Richtungs-Kosinusse [2D_T_3D] */
  MKTFMat3Dr2D(vec1, vec2, 0.0, trans);
  if(dircos){
    dircos[0]=trans[0];
    dircos[1]=trans[1];
    dircos[2]=trans[2];
    dircos[3]=trans[3];
    dircos[4]=trans[4];
    dircos[5]=trans[5];
  }
  /* Koordinatenmatrix [A'_2D] */
  MMultMatMat(koma, 3, 3, trans, 3, 2, xy, 3, 2);
  vec1[0] = xy[0];
  vec1[1] = xy[2];
  vec1[2] = xy[4];
  vec2[0] = xy[1];
  vec2[1] = xy[3];
  vec2[2] = xy[5];
}





/*************************************************************************
 ROCKFLOW - Funktion: FEMCorrectFluxesOverIrregularNodes

 Aufgabe:
   Korrigiert die virtuellen Fluesse ueber irregulaere Knoten indem sie
   auf die umliegenden regulaeren Knoten verteilt werden.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int ndx: Knotenindex der zu behandelnden Groesse

 Programmaenderungen:
   02.02.2001  C. Thorenz    Erste Version

*************************************************************************/
void FEMCorrectFluxesOverIrregularNodes(int ndx)
{
  long j, k, node, nachbarn[4];
  int anz_nachbarn;
  double wert;

  for (j = 0l; j < NodeListLength; j++)
    {
      node = NodeNumber[j];
      anz_nachbarn = IrrNodeGetRegularNeighbors(node, nachbarn);
      if (anz_nachbarn)
        {
          wert = GetNodeVal(node, ndx) / (double) anz_nachbarn;
          for (k = 0l; k < anz_nachbarn; k++)
            {
              SetNodeVal(nachbarn[k], ndx, GetNodeVal(nachbarn[k], ndx) + wert);
            }
          SetNodeVal(node, ndx, 0.);
        }
    }
}

/*************************************************************************
 ROCKFLOW - Funktion: FEMCorrectFluxesOverIrregularNodesOnVector

 Aufgabe:
   Korrigiert die virtuellen Fluesse ueber irregulaere Knoten indem sie
   auf die umliegenden regulaeren Knoten verteilt werden. Die Werte werden
   in einem Vektor mit der Nummerierung des Glecihungssystems uebergeben.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *flux: Vektor der ermittelten Groesse

 Programmaenderungen:
   2.1.2001   C. Thorenz    Erste Version

*************************************************************************/
void FEMCorrectFluxesOverIrregularNodesOnVector(double *flux)
{
  long i, k, node, nachbarn[4];
  int anz_nachbarn;
  double wert;

  for (i = 0l; i < NodeListLength; i++)
    {
      node = NodeNumber[i];
      anz_nachbarn = IrrNodeGetRegularNeighbors(node, nachbarn);
      if (anz_nachbarn)
        {
          wert = flux[i] / (double) anz_nachbarn;
          for (k = 0l; k < anz_nachbarn; k++)
            {
              flux[GetNodeIndex(nachbarn[k])] += wert;
              flux[i] = 0.;
            }
        }
    }
}


/***************************************************************************
   ROCKFLOW - Funktion: MXPGaussPktTri
   Aufgabe:
   X Punkt Gauss-Integration Dreieck
           bestimmtes Integral
           1/           X
            | P(x)dx = Sigma  Fi*P(xi)
            |
          -1/          i=1
   Formalparameter:
           E: int anzgp:    Anzahl der Gauspunkte im Dreieck
           E: long xx:      Nr des Gauspunktes
           E: long coord:   coordr
           return : double coord des Gauspunktes
   Ergebnis:
           
   Aenderungen/Korrekturen:
   07/2003     mb        Erste Version

 **************************************************************************/
double MXPGaussPktTri(int anzgp, long xx, long coordr)
{
 double a[3] = {0.66666666666666,0.16666666666666,0.16666666666666};
 double b[3] = {0.16666666666666,0.66666666666666,0.16666666666666};
 double c[3] = {0.16666666666666,0.16666666666666,0.66666666666666};
 if (anzgp == 1) {
   return 0.3333333333333333;
 }
 else if (anzgp == 3) {
   switch (xx) {
     case 0:
       return a[coordr];
     case 1:
       return b[coordr];
     case 2:
       return c[coordr];
   }
 }
 else if (anzgp ==4) {

  switch (xx) {
  case 0:
    switch (coordr) {
      case 0:
        return 0.333333333333333;
      case 1:
        return 0.333333333333333;
        }
      break;

  case 1:
    switch (coordr) {
      case 0:
        return 0.600000000000000;
      case 1:
        return 0.200000000000000;
        }
      break;

  case 2:
    switch (coordr) {
      case 0:
        return 0.200000000000000;
      case 1:
        return 0.600000000000000;
        }
      break;

  case 3:
    switch (coordr) {
      case 0:
        return 0.200000000000000;
      case 1:
        return 0.200000000000000;
        }
      break;

    }                 
 } /*else if*/          /* switch grd */
    return 0.0;
}


/***************************************************************************
   ROCKFLOW - Funktion: MXPGaussFktTri
   Aufgabe:
   X Punkt Gauss-Integration Dreieck
           bestimmtes Integral
           1/           X
            | P(x)dx = Sigma  Fi*P(xi)
            |
          -1/          i=1
   Formalparameter:
           E: int anzgp:     Anzahl der Gauspunkte im Dreieck
           E: long pkt:      Nr des Gauspunktes
           return : Wichtung des Gauspunktes
   Ergebnis:  double: Wichtung des Gauspunktes
           
   Aenderungen/Korrekturen:
   07/2003     mb        Erste Version

 **************************************************************************/
double MXPGaussFktTri(int anzgp, long pkt)
{
 if(anzgp==1) 
   return 0.5;
 else if (anzgp==3) {
   return 0.166666666666666;
 }
 else if (anzgp==4) {

    switch (pkt) {
    case 0:
      return -0.281250000000000;
      break;
    case 1:
      return  0.260416666666667;
      break;
    case 2:
      return  0.260416666666667;
      break;
    case 3:
      return  0.260416666666667;
      break;
    }
  }
    return 0.0;
}


/**************************************************************************
  ROCKFLOW - Function: Get_Nt_x_Nt
   
  Task:  
  Gets the linear component of the capacitance matrix for triangular prisms:
  integral Nt x Nt

  Parameter: (I: Input; R: Return; X: Both)
           I: void
           
  Return:
           *Ct
   
  Programming:
  07/2003   MB   First Version
 
 **************************************************************************/
int Get_Nt_x_Nt(double* Ct)
{
  double Ct1;
  double Ct2;

  Ct1 = 2.0 / 3.0;
  Ct2 = 1.0 / 3.0;

  Ct[0] =  Ct[21] = Ct1;
  Ct[1] =  Ct[22] = Ct1;
  Ct[2] =  Ct[23] = Ct1;
  Ct[6] =  Ct[27] = Ct1;
  Ct[7] =  Ct[28] = Ct1;
  Ct[8] =  Ct[29] = Ct1;
  Ct[12] = Ct[33] = Ct1;
  Ct[13] = Ct[34] = Ct1;
  Ct[14] = Ct[35] = Ct1;

  Ct[3] =  Ct[18] = Ct2;
  Ct[4] =  Ct[19] = Ct2;
  Ct[5] =  Ct[20] = Ct2;
  Ct[9] =  Ct[24] = Ct2;
  Ct[10] = Ct[25] = Ct2;
  Ct[11] = Ct[26] = Ct2;
  Ct[15] = Ct[30] = Ct2;
  Ct[16] = Ct[31] = Ct2;
  Ct[17] = Ct[32] = Ct2;

  return 1;
}     


/**************************************************************************
 ROCKFLOW - Function: Get_Nt_x_gradNt
   
   Task:  
   Gets the linear component for triangular prisms:
   integral Nt x gradNt 

   Parameter: (I: Input; R: Return; X: Both)
           I: void
           
   Return:
           *Ct
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
int Get_Nt_x_gradNt(double* Nt_x_gradNt)
{
  double Ct1;
  double Ct2;

  Ct1 = 1.0 / 2.0;
  Ct2 = -1.0 / 2.0;

  Nt_x_gradNt[0] =  Nt_x_gradNt[18] = Ct1;
  Nt_x_gradNt[1] =  Nt_x_gradNt[19] =  Ct1;
  Nt_x_gradNt[2] =  Nt_x_gradNt[20] = Ct1;
  Nt_x_gradNt[6] =  Nt_x_gradNt[24] = Ct1;
  Nt_x_gradNt[7] =  Nt_x_gradNt[25] =  Ct1;
  Nt_x_gradNt[8] =  Nt_x_gradNt[26] = Ct1;
  Nt_x_gradNt[12] = Nt_x_gradNt[30] = Ct1;
  Nt_x_gradNt[13] = Nt_x_gradNt[31] = Ct1;
  Nt_x_gradNt[14] = Nt_x_gradNt[32] = Ct1;

  Nt_x_gradNt[3] = Nt_x_gradNt[21] = Ct2;
  Nt_x_gradNt[4] = Nt_x_gradNt[22] = Ct2;
  Nt_x_gradNt[5] = Nt_x_gradNt[23] = Ct2;
  Nt_x_gradNt[9] = Nt_x_gradNt[27] = Ct2;
  Nt_x_gradNt[10] = Nt_x_gradNt[28] = Ct2;
  Nt_x_gradNt[11] = Nt_x_gradNt[29] = Ct2;
  Nt_x_gradNt[15] = Nt_x_gradNt[33] = Ct2;
  Nt_x_gradNt[16] = Nt_x_gradNt[34] = Ct2;
  Nt_x_gradNt[17] = Nt_x_gradNt[35] = Ct2;

  return 1;
}     


/**************************************************************************
 ROCKFLOW - Function: Get_gradNt_x_Nt
   
   Task:  
   Gets the linear component for triangular prisms:
   integral Nt x gradNt 

   Parameter: (I: Input; R: Return; X: Both)
           I: void
           
   Return:
           *Ct
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
int Get_gradNt_x_Nt(double* gradNt_x_Nt)
{
  double Ct1;
  double Ct2;

  Ct1 = 1.0 / 2.0;
  Ct2 = -1.0 / 2.0;

  gradNt_x_Nt[0] =  gradNt_x_Nt[3] = Ct1;
  gradNt_x_Nt[1] =  gradNt_x_Nt[4] =  Ct1;
  gradNt_x_Nt[2] =  gradNt_x_Nt[5] = Ct1;
  gradNt_x_Nt[6] =  gradNt_x_Nt[9] = Ct1;
  gradNt_x_Nt[7] =  gradNt_x_Nt[10] =  Ct1;
  gradNt_x_Nt[8] =  gradNt_x_Nt[11] = Ct1;
  gradNt_x_Nt[12] = gradNt_x_Nt[15] = Ct1;
  gradNt_x_Nt[13] = gradNt_x_Nt[16] = Ct1;
  gradNt_x_Nt[14] = gradNt_x_Nt[17] = Ct1;

  gradNt_x_Nt[18] =  gradNt_x_Nt[21] = Ct2;
  gradNt_x_Nt[19] =  gradNt_x_Nt[22] =  Ct2;
  gradNt_x_Nt[20] =  gradNt_x_Nt[23] = Ct2;
  gradNt_x_Nt[24] =  gradNt_x_Nt[27] = Ct2;
  gradNt_x_Nt[25] =  gradNt_x_Nt[28] =  Ct2;
  gradNt_x_Nt[26] =  gradNt_x_Nt[29] = Ct2;
  gradNt_x_Nt[30] =  gradNt_x_Nt[33] = Ct2;
  gradNt_x_Nt[31] =  gradNt_x_Nt[34] = Ct2;
  gradNt_x_Nt[32] =  gradNt_x_Nt[35] = Ct2;

  return 1;
}     


/**************************************************************************
 ROCKFLOW - Function: Get_gradNt_x_gradNt
   
   Task:  
   Gets the linear component for triangular prisms:
   integral gradNt x gradNt 

   Parameter: (I: Input; R: Return; X: Both)
           I: void
           
   Return:
           *GradNGradN
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
int Get_gradNt_x_gradNt(double* GradNGradN)
{
  double Ct1;
  double Ct2;

  Ct1 =  1.0 / 2.0;
  Ct2 = - 1.0 / 2.0;
  
  GradNGradN[0] =  GradNGradN[21] = Ct1;
  GradNGradN[1] =  GradNGradN[22] = Ct1;
  GradNGradN[2] =  GradNGradN[23] = Ct1;
  GradNGradN[6] =  GradNGradN[27] = Ct1;
  GradNGradN[7] =  GradNGradN[28] = Ct1;
  GradNGradN[8] =  GradNGradN[29] = Ct1;
  GradNGradN[12] = GradNGradN[33] = Ct1;
  GradNGradN[13] = GradNGradN[34] = Ct1;
  GradNGradN[14] = GradNGradN[35] = Ct1;

  GradNGradN[3] =  GradNGradN[18] = Ct2;
  GradNGradN[4] =  GradNGradN[19] = Ct2;
  GradNGradN[5] =  GradNGradN[20] = Ct2;
  GradNGradN[9] =  GradNGradN[24] = Ct2;
  GradNGradN[10] = GradNGradN[25] = Ct2;
  GradNGradN[11] = GradNGradN[26] = Ct2;
  GradNGradN[15] = GradNGradN[30] = Ct2;
  GradNGradN[16] = GradNGradN[31] = Ct2;
  GradNGradN[17] = GradNGradN[32] = Ct2;

  return 1;
}     


/**************************************************************************
 ROCKFLOW - Function: Get_IntN
   
   Task:  
   Gets integral of NTriangle for triangular prisms:

   Parameter: (I: Input; R: Return; X: Both)
           I: long node
           
   Return:
           *IntN
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
/*double* Get_IntN(long index)
{

  double* IntN = NULL;
  IntN  = (double *) Malloc(3 * sizeof(double));

  IntN[0] = 1.0 / 3.0;   
  IntN[1] = 1.0 / 3.0;
  IntN[2] = 1.0 / 3.0;

  return IntN;
} */                          


/**************************************************************************
 ROCKFLOW - Function: Get_NTrinangle_x_NTrinangle
   
   Task:  
   Gets the triangle component of the capacitance matrix for triangular prisms:
   Integral NTriangle x NTriangle

   Parameter: (I: Input; R: Return; X: Both)
           I: long node
           
   Return:
           *Tri_x_Tri
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
int Get_NTrinangle_x_NTrinangle(long index, double* Tri_x_Tri)
{

  long *element_nodes;
  double area;

  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 

  Tri_x_Tri[0] =   (1.0 / 6.0)  * area;
  Tri_x_Tri[1] =   (1.0 / 12.0) * area;
  Tri_x_Tri[2] =   (1.0 / 12.0) * area;

  Tri_x_Tri[3] =   (1.0 / 12.0) * area;
  Tri_x_Tri[4] =   (1.0 / 6.0)  * area;
  Tri_x_Tri[5] =   (1.0 / 12.0) * area;

  Tri_x_Tri[6] =   (1.0 / 12.0) * area;
  Tri_x_Tri[7] =   (1.0 / 12.0) * area;
  Tri_x_Tri[8] =   (1.0 / 6.0)  * area;

  return 1;
}                          


/**************************************************************************
 ROCKFLOW - Function: Get_IntNTri
   
   Task:  
   Gets the linear component for triangular prisms:
   integral Nt x Jac3D 

   Parameter: (I: Input; R: Return; X: Both)
           I: long node 
           
   Return:
           *IntNTri
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
int Get_IntNTri(long index, double* IntNTri)
{
  double area;
  double fact;

  area = CalcBaseAreaPrism(index); 
  fact = (1.0 / 3.0) * area;

  IntNTri[0] = IntNTri[1] = IntNTri[2] = fact;

  return 1;
}     


/**************************************************************************
 ROCKFLOW - Function: GetInvJac3Dz
   
   Task:  
   Gets InvJac3Dz

   Parameter: (I: Input; R: Return; X: Both)
           I: long node
           
   Return:
           InvJac3Dz
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
double GetInvJac3Dz(long index)
{
  long *element_nodes=NULL;
  double thick;
  double InvJac3Dz;
 
  element_nodes = ElGetElementNodes(index);

  /* Mächtigkeit des Elementes im Mittelpunkt approximiert mit:*/
  /* Thickness of Element = 1/3 * (|z14| + |z25| + |z36| */
  thick =  fabs (GetNodeZ(element_nodes[3]) - GetNodeZ(element_nodes[0]))\
           + fabs (GetNodeZ(element_nodes[4]) - GetNodeZ(element_nodes[1]))\
           + fabs (GetNodeZ(element_nodes[5]) - GetNodeZ(element_nodes[2]));
  thick = thick / 3.0;  
  InvJac3Dz = 2.0 / thick;

  return InvJac3Dz;
}                          


/**************************************************************************
 ROCKFLOW - Function: GetOmegaTri
   
   Task: Gets the value of the triangle interpolation functions at given point
   
   Parameter: (I: Input; R: Return; X: Both)
           I: long node, double xx, double yy
           
   Return:
           *OmegaTri
   
   Programming:
   07/2003   MB   First Version
 
 **************************************************************************/
double* GetOmegaTri(long index, double xx, double yy)
{
  /* Laufvariablen */
  int i;
  int nn = 3;
  long *element_nodes=NULL;
  double *OmegaTri=NULL;
  /* Elementdaten */
  double x[3], y[3];
  double area;
  double fact;

  /* Speicher */ 
  OmegaTri  = (double *) Malloc(3 * sizeof(double));

  /* --- Element-Daten -----------------------------------------------------*/
  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 
  fact = 1.0 / ( 2.0 * area); 

  for (i = 0; i < nn; i++)  {
    x[i] = GetNodeX(element_nodes[i]);
    y[i] = GetNodeY(element_nodes[i]);
  }

  OmegaTri[0] = ((x[1]*y[2]-x[2]*y[1]) + (y[1]-y[2])*xx + (x[2]-x[1])*yy);
  OmegaTri[1] = ((x[2]*y[0]-x[0]*y[2]) + (y[2]-y[0])*xx + (x[0]-x[2])*yy);
  OmegaTri[2] = ((x[0]*y[1]-x[1]*y[0]) + (y[0]-y[1])*xx + (x[1]-x[0])*yy);

  OmegaTri[0] = OmegaTri[0] * fact;
  OmegaTri[1] = OmegaTri[1] * fact;
  OmegaTri[2] = OmegaTri[2] * fact;

  return OmegaTri;
}   


/**************************************************************************
   ROCKFLOW - Function: CalcGradXTri
   
   Task:  
   Gets the X derivative of the triangle shape function.
         
   Parameter: (I: Input; R: Return; X: Both)
           I: long node
           
   Return:
           *GradXTri 
   
   Programming:
   07/2003   MB   First Version
 **************************************************************************/

int CalcGradXTri(long index, double* GradXTri)
{
  long *element_nodes=NULL;
  double y[3];
  double area;
  double fact;

  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 

  fact = 1.0 / ( 2.0 * area); 

  y[0] = GetNodeY(element_nodes[0]);
  y[1] = GetNodeY(element_nodes[1]);
  y[2] = GetNodeY(element_nodes[2]);

  GradXTri[0] = (y[1] - y[2]) * fact ;
  GradXTri[1] = (y[2] - y[0]) * fact;
  GradXTri[2] = (y[0] - y[1]) * fact;

  return 1;  
}


/**************************************************************************
   ROCKFLOW - Function: CalcGradYTri
   
   Task:  
      Gets the Y derivative of the triangle shape function.
         
   Parameter: (I: Input; R: Return; X: Both)
           I: long node
           
   Return:
           *GradYTri
   
   Programming:
   07/2003   MB   First Version
 **************************************************************************/

int CalcGradYTri(long index, double* GradYTri)
{
  long *element_nodes;
  double x[3];
  double area;
  double fact;

  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 

  fact = 1.0 / (2.0 * area); 

  x[0] = GetNodeX(element_nodes[0]);
  x[1] = GetNodeX(element_nodes[1]);
  x[2] = GetNodeX(element_nodes[2]);

  GradYTri[0] = (x[2] - x[1]) * fact ;
  GradYTri[1] = (x[0] - x[2]) * fact ;
  GradYTri[2] = (x[1] - x[0]) * fact ;

  return 1;  
}
                       

/***************************************************************************
   ROCKFLOW - Funktion: MOmegaPrism
   Aufgabe:
           Berechnet Ansatzfunktion (r,s,t) eines Dreieckprismas.

   Formalparameter:
           Z: *vf - 1x6 Feld
           E: r,s,t (s.o.)
   Ergebnis:
           double *vf
   Aenderungen/Korrekturen:
   07/2003   MB   First Version
 **************************************************************************/

int MOmegaPrism(double *vf, double r, double s, double t)
{
  long i;
  int ok = 0;
#ifdef ERROR_CONTROL
  if (vf == NULL) {
    ok = 0;
    return ok;
  }
#endif
  vf[0] = (1.0 - r - s) * (1.0 + t);
  vf[1] = r * (1.0 + t);
  vf[2] = s * (1.0 + t);
  vf[3] = (1.0 - r - s) * (1.0 - t);
  vf[4] = r * (1.0 - t);
  vf[5] = s * (1.0 - t);
  for (i = 0; i < 6; i++)  {
    vf[i] *= 0.5;
  }
  return ok = 1;
}


/***************************************************************************
   ROCKFLOW - Funktion: MGradOmegaPrism
   Aufgabe:
           Berechnet Gradient der Ansatzfunktionen (r,s,t).

                    |  0   1   2   3   4   5   |
                    |                          |
      grad (omega)= |  6   7   8   9  10  11   |
                    |                          |
                    | 12  13  14  15  16  17   |

     Zahlen in der Matrix stehen fuer Positionen im Feld vf.
     Gleichungen s.u..

   Formalparameter:
           Z: *vf - 3x6 Feld
           E: r,s (s.o.)
   Ergebnis:
           3x8 Matrix
   Aenderungen/Korrekturen:
   06/2003     mb        Erste Version

 **************************************************************************/

int MGradOmegaPrism(double r, double s, double t, double *vf)
{
  long i;
  int ok = 0;
#ifdef ERROR_CONTROL
  if (vf == NULL) {
    ok = 0;
    return ok;
  }
#endif
  /*grad r N */
  vf[0] =  -(1.0 + t);
  vf[1] =   (1.0 + t);
  vf[2] =    0.0;
  vf[3] =  -(1.0 - t);
  vf[4] =   (1.0 - t);
  vf[5] =   0.0;
  /*grad s N */
  vf[6] =  -(1.0 + t);
  vf[7] =    0.0;
  vf[8] =   (1.0 + t);
  vf[9] =  -(1.0 - t);
  vf[10] =   0.0;
  vf[11] =  (1.0 - t);
  /*grad t N */
  vf[12] =  (1.0 - r - s);
  vf[13] =   r;
  vf[14] =   s;
  vf[15] = -(1.0 - r - s);;
  vf[16] = - r;
  vf[17] = - s;

  for (i = 0; i < 18; i++)  {
    vf[i] *= 0.5;
  }
  return ok = 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcPrismElementJacobiMatrix
                                                                          */
/* Aufgabe:
   Berechnet die Inverse Jacobi-Matrix eines Dreieckprismas
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements
   E double r,s,t
   X double *erg
   X double *dj                                                           */
/* Ergebnis:
   - double -
                                                                          */
/* Programmaenderungen:
   07/2003     MB
                                                                          */
/**************************************************************************/
void CalcPrismElementJacobiMatrix(long index, double r, double s, double t, double *erg, double *dj)
{
  static double koma[18];
  static double grd[18];
  static Knoten *nd;
  static long *nodes;
  static long i;

  /* Koordinatenmatrix A */
  nodes = ElGetElementNodes(index);
  for (i = 0; i < 6; i++)
    {
      nd = GetNode(nodes[i]);
      koma[i * 3] = nd -> x;
      koma[i * 3 + 1] = nd -> y;
      koma[i * 3 + 2] = nd -> z;
    }
  MGradOmegaPrism(r, s, t, grd);
  MMultMatMat(grd, 3, 6, koma, 6, 3, erg, 3, 3);

  *dj = M3Determinante(erg);
  M3Invertiere(erg);
}
/*ROCKFLOW - Funktion: FEMCalculatecharacterisitcelementlength 

Purpose
Function calculated the characterisitc element lenth needed later for the
evaluation of the stability criteria for diffusive transport

Parameters
Index of element

Program information
05.2005     CMCD First programming

*/

void FEMCalcElementCharacterisiticLength(long index)
{
  double side[9];
  double length;
  int i,a;
  double volume;
  double znodes[8],ynodes[8],xnodes[8];
  double dx[9],dy[9],dz[9];
  Element *elem = NULL;
  elem = ElGetElement(index);
  
  long *element_nodes;
  element_nodes = ElGetElementNodes(index);

    switch (ElGetElementType(index))
    {
    case 1:                            /* Line */
      volume = ElGetElementVolume(index);
      elem->Element_Char_Length = volume;
      break;
    case 2:                            /* Quadralateral */
      volume = ElGetElementVolume(index);
      elem->Element_Char_Length = pow(volume,0.5);      
      break;
    case 3:                            /* Cube */
      volume = ElGetElementVolume(index);
      elem->Element_Char_Length = pow(volume,(1./3.));         
      break;
    case 4:                            /* Triangle */
      for (i=0;i<3;i++)
		{
		    znodes[i]=GetNodeZ(element_nodes[i]);
			ynodes[i]=GetNodeY(element_nodes[i]);
			xnodes[i]=GetNodeX(element_nodes[i]);
		}
        dz[0] = znodes[1]-znodes[0];
        dz[1] = znodes[2]-znodes[1];
        dz[2] = znodes[0]-znodes[2];

        dy[0] = ynodes[1]-ynodes[0];
        dy[1] = ynodes[2]-ynodes[1];
        dy[2] = ynodes[0]-ynodes[2];

        dx[0] = xnodes[1]-xnodes[0];
        dx[1] = xnodes[2]-xnodes[1];
        dx[2] = xnodes[0]-xnodes[2];

        for (i = 0;i<3;i++)
            {
            side[i]=MVectorlength (dz[i],dy[i],dx[i]);
            }

        length = MMin(side[0],side[1]);
        length = MMin(side[2],length);

        elem->Element_Char_Length = length;
      break;
    case 5:                            /* Tetraeder */
       for (i=0;i<4;i++)
		{
		    znodes[i]=GetNodeZ(element_nodes[i]);
			ynodes[i]=GetNodeY(element_nodes[i]);
			xnodes[i]=GetNodeX(element_nodes[i]);
		}
        
        a=1;
        for (i=0;i<4;i++)
        {
        dz[i] = znodes[a]-znodes[i];
        dy[i] = ynodes[a]-ynodes[i];
        dx[i] = xnodes[a]-xnodes[i];
        a++;
        if (a == 4)
            {
            a = 0;
            }
        }
 

        for (i = 0;i<4;i++)
            {
            side[i]=MVectorlength (dz[i],dy[i],dx[i]);
            }

        length = MMin(side[0],side[1]);
        length = MMin(side[2],length);
        length = MMin(side[3],length);

        elem->Element_Char_Length = length;
      break;

    case 6:                            /* Prism */
       for (i=0;i<5;i++)
		{
		    znodes[i]=GetNodeZ(element_nodes[i]);
			ynodes[i]=GetNodeY(element_nodes[i]);
			xnodes[i]=GetNodeX(element_nodes[i]);
		}
        dz[0] = znodes[1]-znodes[0];
        dz[1] = znodes[2]-znodes[1];
        dz[2] = znodes[0]-znodes[2];

        dz[3] = znodes[4]-znodes[3];
        dz[4] = znodes[5]-znodes[4];
        dz[5] = znodes[3]-znodes[5];

        dz[6] = znodes[3]-znodes[0];
        dz[7] = znodes[4]-znodes[1];
        dz[8] = znodes[5]-znodes[2];
        

        dy[0] = ynodes[1]-ynodes[0];
        dy[1] = ynodes[2]-ynodes[1];
        dy[2] = ynodes[0]-ynodes[2];

        dy[3] = ynodes[4]-ynodes[3];
        dy[4] = ynodes[5]-ynodes[4];
        dy[5] = ynodes[3]-ynodes[5];

        dy[6] = ynodes[3]-ynodes[0];
        dy[7] = ynodes[4]-ynodes[1];
        dy[8] = ynodes[5]-ynodes[2];


        dx[0] = xnodes[1]-xnodes[0];
        dx[1] = xnodes[2]-xnodes[1];
        dx[2] = xnodes[0]-xnodes[2];

        dx[3] = xnodes[4]-xnodes[3];
        dx[4] = xnodes[5]-xnodes[4];
        dx[5] = xnodes[3]-xnodes[5];

        dx[6] = xnodes[3]-xnodes[0];
        dx[7] = xnodes[4]-xnodes[1];
        dx[8] = xnodes[5]-xnodes[2];

        for (i = 0;i<8;i++)
            {
            side[i]=MVectorlength (dz[i],dy[i],dx[i]);
            }

        length = MMin(side[0],side[1]);
        for (i = 2;i<8;i++)length = MMin(side[i],length);
        elem->Element_Char_Length = length;
      break;

    default:
      abort();
    }
}



