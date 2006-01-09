/**************************************************************************
 ROCKFLOW - Modul: cel_ge.c

 Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen fuer das Modul GE.
   (Geo-Elektrik)

 Programmaenderungen:
   1/2001   C.Thorenz   Erste Version
  03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt
   

**************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTCEL_GE

/* Header / Andere intern benutzte Module */
#include "int_ge.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
#include "femlib.h"
#include "models.h" //OK_MOD"
/* material properties */
#include "rf_mmp_new.h"
#include "rf_msp_new.h"
#include "numerics.h"

/* Interne (statische) Deklarationen */
void CalcEle1D_GE(long index);
  /* berechnet Elementmatrizen fuer 1D-Elemente */
void CalcEle2D_GE(long index);
  /* berechnet Elementmatrizen fuer 2D-Elemente */
void CalcEle3D_GE(long index);
  /* berechnet Elementmatrizen fuer 3D-Elemente */
void CalcEle2DTriangle_GE(long index);
  /* berechnet Elementmatrizen fuer lineare Dreiecks-Elemente */

/* Vorkonfigurieren des Kernels */
static int ge_method = 0;

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_GE

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/
int InitializeKernel_GE(void)
{
  NUMERICALPARAMETER *ge_numerics = NumericsGetPointer("GEOELECTRIC0");

  if (ge_numerics)
    {
      /* Numerische Methode */
      if (ge_numerics -> method_read)
        {
          ge_method = ge_numerics -> method;
        }
    }                                  /* endif */
  return 0;
}



/**************************************************************************
 ROCKFLOW - Funktion: GetXXX_GE

 Aufgabe:
   Gibt div. Kernelparameter zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version

**************************************************************************/
/* Numerische Methode */
int GetMethod_GE(void)
{
  return ge_method;
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcElementMatrix_GE

 Aufgabe:
   Berechnet elektr. Leitfaehigkeitsmatrix (conductance matrix)
   des angegebenen Elements ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer

 Ergebnis:
   - void -

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version

**************************************************************************/
void CalcElementMatrix_GE(long index)
{
  switch (ElGetElementType(index))
    {
    case 1:
      CalcEle1D_GE(index);
      break;
    case 2:
      CalcEle2D_GE(index);
      break;
    case 3:
      CalcEle3D_GE(index);
      break;
    case 4:
      CalcEle2DTriangle_GE(index);
      break;
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle1D_GE

 Aufgabe:
   Berechnet elektr. Leitfaehigkeitsmatrix (conductance matrix)
   des angegebenen Elements ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer

 Ergebnis:
   - void -

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version

**************************************************************************/
void CalcEle1D_GE(long index)
{
  static double *invjac, detjac;
  static double t, k, vorfk;
  static double *conductance_matrix;


  /* 1. Elementdaten bereitstellen */
  /* Determinante und Inverse der Jacobi-Matrix bereitstellen */
  /* detjac = AL/2, invjac = 2/(AL) */
  invjac = GetElementJacobiMatrix(index, &detjac);

//SB:m  k = GetFluidElectricConductivity(0, index, 0., 0., 0., 0.);
//SB:m  k += GetSoilElectricConductivity(index, 0., 0., 0., 0.);
DisplayMsgLn(" Electric material parameters not yet implemented ");
  /* t ist das Produkt aus der inversen Jacobi-
     und der Koordinaten-Transformations-Matrix  */
  t = k * MSkalarprodukt(invjac, invjac, 3);

  /* 2. Elementmatrizen berechnen: */

  /* Leitfaehigkeitsmatrix (conductance matrix) ohne Zeitanteile */
  conductance_matrix = GEGetElementElectricConductivityMatrix(index);
  if (!conductance_matrix)
    conductance_matrix = (double *) Malloc(4 * sizeof(double));

  vorfk = (detjac * 0.5) * t;
  conductance_matrix[0] = +1.0 * vorfk;
  conductance_matrix[1] = -1.0 * vorfk;
  conductance_matrix[2] = -1.0 * vorfk;
  conductance_matrix[3] = +1.0 * vorfk;
  GESetElementElectricConductivityMatrix(index, conductance_matrix);

#ifdef TESTCEL_GE
  MZeigMat(conductance_matrix, 2, 2, "   CONDUCTANCE-MATRIX");
#endif


}

/**************************************************************************
 ROCKFLOW - Funktion: CalcEle2D_GE

 Aufgabe:
   Berechnet elektr. Leitfaehigkeitsmatrix (conductance matrix)
   des angegebenen Elements ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer

 Ergebnis:
   - void -

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version

**************************************************************************/
void CalcEle2D_GE(long index)
{
  static double invjac[4], invjac_t[4], detjac;
  static double g_phi[8], g_phi_t[8], cond;
  static double g_ome_t[8];
  static double *conductance_matrix;
  static long *element_nodes;
  static int anzgp;
  static long i, j, k;
  static double r, s, fkt;
  static double tkmyt[4], kmy[4];
  static double mat2x2[4], mat4x4[16], mat4x2[8];
  static double area, k_xx, k_yy;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  /* Numerische Parameter */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  element_nodes = ElGetElementNodes(index);

  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  conductance_matrix = GEGetElementElectricConductivityMatrix(index);
  if (!conductance_matrix)
    conductance_matrix = (double *) Malloc(16 * sizeof(double));

  /* Initialisieren */
  MNulleMat(conductance_matrix, 4, 4);

  /* Materialparameter */
  area = m_mmp->geo_area;

//SB:m  cond = GetFluidElectricConductivity(0, index, 0., 0., 0., 0.);
//SB:m  cond += GetSoilElectricConductivity(index, 0., 0., 0., 0.);
DisplayMsgLn(" Electric material parameters not yet implemented ");
  k_xx = cond;
  k_yy = cond;

/*------------------------------------------------------------------------*/
  /* Schleife ueber GaussPunkte */
  for (i = 0; i < anzgp; i++)
    {
      for (j = 0; j < anzgp; j++)
        {
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);

          /* J^-1 */
          Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
          /* Faktor */
          fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;

          /* conductance tensor */
          MNulleMat(kmy, 2, 2);
          kmy[0] = (k_xx);
          kmy[3] = (k_yy);

          /* tkmyt = J^-1 * (K/my) * (J^-1)T */
          MMultMatMat(invjac, 2, 2, kmy, 2, 2, mat2x2, 2, 2);
          MTranspoMat(invjac, 2, 2, invjac_t);
          MMultMatMat(mat2x2, 2, 2, invjac_t, 2, 2, tkmyt, 2, 2);


/*----------------------------------------------------------------*/
          /* conductance matrix und gravity matrix */
          /* grad(phi)^T */
          MGradPhi2D(g_phi_t, r, s);   /* 2 Zeilen 4 Spalten */
          MTranspoMat(g_phi_t, 2, 4, g_phi);    /* 4 Zeilen 2 Spalten */
          /* grad(phi) * tkmyt */
          MMultMatMat(g_phi, 4, 2, tkmyt, 2, 2, mat4x2, 4, 2);
          /* grad(omega) */
          MGradOmega2D(g_ome_t, r, s); /* 2 Zeilen 4 Spalten */
          /* grad(phi)T * tkmyt * grad(omega) */
          MMultMatMat(mat4x2, 4, 2, g_ome_t, 2, 4, mat4x4, 4, 4);

          for (k = 0; k < 16; k++)
            {
              conductance_matrix[k] += (mat4x4[k] * fkt);
            }

        }
    }                                  /* Ende der Schleife ueber GaussPunkte */
/*------------------------------------------------------------------------*/

  /* Volumenelemente */
  for (i = 0; i < 16; i++)
    {
      conductance_matrix[i] *= (area);
    }


  /* Elementmatrizenspeichern */
  GESetElementElectricConductivityMatrix(index, conductance_matrix);

#ifdef TESTCEL_GE
  MZeigMat(conductance_matrix, 4, 4, "CalcEle2D_GE: CONDUCTANCE-MATRIX");
#endif

  element_nodes = NULL;
}



/**************************************************************************
 ROCKFLOW - Funktion: CalcEle3D_GE

 Aufgabe:

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version

**************************************************************************/
void CalcEle3D_GE(long index)
{
  static double invjac[9], detjac;
  static double *conductance_matrix;
  static long *element_nodes;
  static double tkmyt[9], kmy[9], cond;
  static int anzgp;
  static long i, j, k, l;
  static double r, s, t, fkt;
  static double zwi[64];
  static double zwa[24], zwo[9];
  static double phi[8], ome[24];
  static double k_xx, k_yy, k_zz;


  conductance_matrix = GEGetElementElectricConductivityMatrix(index);
  if (!conductance_matrix)
    conductance_matrix = (double *) Malloc(64 * sizeof(double));

  /* Initialisieren */
  MNulleMat(conductance_matrix, 8, 8);

  /* Material */
//SB:m  cond = GetFluidElectricConductivity(0, index, 0., 0., 0., 0.);
//SB:m  cond += GetSoilElectricConductivity(index, 0., 0., 0., 0.);
  DisplayMsgLn(" Electric material parameters not yet implemented ");

  k_xx = cond;
  k_yy = cond;
  k_zz = cond;

  /* Anzahl der Gausspunkte holen */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  /* Schleife ueber GaussPunkte */
  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)
        {
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          t = MXPGaussPkt(anzgp, k);

          /* conductance tensor */
          MNulleMat(kmy, 3, 3);
          kmy[0] = (k_xx);
          kmy[4] = (k_yy);
          kmy[8] = (k_zz);

          Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
          /* Faktor */
          fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * \
              MXPGaussFkt(anzgp, k) * detjac;

          /* tkmyt = (J^-1)T * (K/my) * J^-1 */
          MMultMatMat(kmy, 3, 3, invjac, 3, 3, zwa, 3, 3);
          MTranspoMat(invjac, 3, 3, zwi);
          MMultMatMat(zwi, 3, 3, zwa, 3, 3, tkmyt, 3, 3);

          /* capacitance matrix */
          /* phi * omega */
          MPhi3D(phi, r, s, t);
          MOmega3D(zwo, r, s, t);
          MMultVecVec(phi, 8, zwo, 8, zwi, 8, 8);

          /* conductance matrix und gravity matrix */
          /* grad(phi)T */
          MGradPhi3D(zwa, r, s, t);    /* 3 Zeilen 8 Spalten */
          MTranspoMat(zwa, 3, 8, zwi); /* jetzt: 8 Zeilen 3 Spalten */
          /* grad(phi)T * tkmyt */
          MMultMatMat(zwi, 8, 3, tkmyt, 3, 3, zwa, 8, 3);
          /* grad(omega) */
          MGradOmega3D(ome, r, s, t);  /* 3 Zeilen 8 Spalten */
          /* grad(phi)T * tkmyt * grad(omega) */
          MMultMatMat(zwa, 8, 3, ome, 3, 8, zwi, 8, 8);

          for (l = 0; l < 64; l++)
            {
              conductance_matrix[l] += (zwi[l] * fkt);
            }
        }                              /* Ende der Schleife ueber GaussPunkte */


  element_nodes = NULL;

  GESetElementElectricConductivityMatrix(index, conductance_matrix);


#ifdef TESTCEL_GE
  MZeigMat(conductance_matrix, 8, 8, "   CONDUCTANCE-MATRIX");
#endif
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle2DTriangle_GE

 Aufgabe:
   Elementmatrix fuer lineares Dreieckselement

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer

 Ergebnis:
   - void -

 Programmaenderungen:
   2.2.2001   C.Thorenz    Erste Version (basierend auf Olaf's ASM-Dreieck)

**************************************************************************/
void CalcEle2DTriangle_GE(long index)
{
  /* Materials */
  static double thickness, k_xx, k_yy;
  /* Matrices */
  static double *conductance_matrix;
  static long *element_nodes;
  static double x[3], y[3];
  static long i;
  static int nn, nn2;
  static double fac_K;
  static double area;
  double* permeability;
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  /* Numerische Parameter */
  element_nodes = ElGetElementNodes(index);
  permeability = m_mmp->PermeabilityTensor(index);
  nn = 3;
  nn2 = 9;
  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  conductance_matrix = GEGetElementElectricConductivityMatrix(index);
  if (!conductance_matrix)
    conductance_matrix = (double *) Malloc(nn2 * sizeof(double));
  /* Initialisieren */
  MNulleMat(conductance_matrix, nn, nn);
/*------------------------------------------------------------------------*/
  /* Materialparameter */
  thickness = m_mmp->geo_area;
  k_xx = permeability[0];
  k_yy = permeability[1];
/*------------------------------------------------------------------------*/
  for (i = 0; i < nn; i++)
    {
      x[i] = GetNodeX(element_nodes[i]);
      y[i] = GetNodeY(element_nodes[i]);
    }
  area = ElGetElementVolume(index);


/*----------------------------------------------------------------*/
  /* conductance matrix */
  conductance_matrix[0] = k_xx * (y[1] - y[2]) * (y[1] - y[2]) + k_yy * (x[2] - x[1]) * (x[2] - x[1]);
  conductance_matrix[1] = k_xx * (y[1] - y[2]) * (y[2] - y[0]) + k_yy * (x[2] - x[1]) * (x[0] - x[2]);
  conductance_matrix[2] = k_xx * (y[1] - y[2]) * (y[0] - y[1]) + k_yy * (x[2] - x[1]) * (x[1] - x[0]);
  conductance_matrix[3] = k_xx * (y[2] - y[0]) * (y[1] - y[2]) + k_yy * (x[0] - x[2]) * (x[2] - x[1]);
  conductance_matrix[4] = k_xx * (y[2] - y[0]) * (y[2] - y[0]) + k_yy * (x[0] - x[2]) * (x[0] - x[2]);
  conductance_matrix[5] = k_xx * (y[2] - y[0]) * (y[0] - y[1]) + k_yy * (x[0] - x[2]) * (x[1] - x[0]);
  conductance_matrix[6] = k_xx * (y[0] - y[1]) * (y[1] - y[2]) + k_yy * (x[1] - x[0]) * (x[2] - x[1]);
  conductance_matrix[7] = k_xx * (y[0] - y[1]) * (y[2] - y[0]) + k_yy * (x[1] - x[0]) * (x[0] - x[2]);
  conductance_matrix[8] = k_xx * (y[0] - y[1]) * (y[0] - y[1]) + k_yy * (x[1] - x[0]) * (x[1] - x[0]);
  fac_K = .25 * area;

/*----------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
  /* Volumenelemente */
  for (i = 0; i < nn2; i++)
    {
      conductance_matrix[i] *= thickness * fac_K;
    }

/*------------------------------------------------------------------------*/
  /* Elementmatrizen speichern */
  GESetElementElectricConductivityMatrix(index, conductance_matrix);

#ifdef TESTCEL_GE
  MZeigMat(conductance_matrix, nn, nn, "CalcEle2DTriangle_GE: CONDUCTANCE-MATRIX");
#endif

  element_nodes = NULL;
}

