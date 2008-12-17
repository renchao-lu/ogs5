/**************************************************************************/
/* ROCKFLOW - Modul: femlib.h
 */
/* Aufgabe:
   Funktionen fuer die FEM
 */
/**************************************************************************/

#ifndef femlib_INC
#define femlib_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */

extern void CalcElementsGeometry(void);
  /* Berechnung der Jakobiane und Elementvolumina */
extern void InitElementGeometry(long index);
  /* Setzt Gausspunkte, berechnet inverse Jakobi, berechnet Volumen */

extern void CalcElementJacobiMatrix(long number, double *vec, double *dj);
  /* Berechnet und setzt die inverse Jakobi-Matrix im Zentrum des
     Elements number */
extern void CalcXDElementJacobiMatrix(long index, double r, double s, double t, double *vec, double *dj);
  /* Berechnet und setzt die inverse Jakobi-Matrix des Elements number */

extern void Calc1DElementJacobiMatrix(long number, double *vec, double *dj);
extern void Calc2DElementJacobiMatrix(long number, double r, double s, double *erg, double *dj);
extern void CalcTransformationsMatrix(long index, double *koma, double *trans);
extern void Calc2DElementCoordinatesTriangle(long index, double *x, double *y,double *dircos);
extern void Calc3DElementJacobiMatrix(long number, double r, double s, double t, double *erg, double *dj);
  /* Berechnet die Determinante und die Inverse der Jacobi-Matrix /
     Drehvektor des Elements number fuer den angegebenen Gausspunkt */
extern void Calc2DElementJacobiMatrix_9N(long number, double r, double s, double *erg, double *dj);
extern void CalcTransformationsMatrix_9N(long index, double *koma, double *trans);
extern void Calc2DElementJacobiMatrixXY_9N(long index, double r, double s, double *invjac_t, double *dj);

extern void Calc3DElementJacobiMatrix_20N(long number, double r, double s, double t, double *erg, double *dj);

//
extern void CalcPrismElementJacobiMatrix(long number, double r, double s, double t, double *erg, double *dj);
extern double MXPGaussPktTri(int anzgp, long grd, long pkt);
/* Punkte fuer die X Punkt Gauss-Integration */

extern int CalcGradXTri(long index, double* GradXTri);
extern int CalcGradYTri(long index, double* GradYTri);

extern int Get_Nt_x_Nt(double* Ct);
extern int Get_Nt_x_gradNt(double* Nt_x_gradNt);
extern int Get_gradNt_x_Nt(double* gradNt_x_Nt);
extern int Get_gradNt_x_gradNt(double* GradNGradN);
extern int Get_NTrinangle_x_NTrinangle(long index, double* Tri_x_Tri);
extern int Get_IntNTri(long index, double* IntNTri);
extern double  GetInvJac3Dz(long index);
extern double* GetOmegaTri(long index, double x, double y);

extern double MXPGaussFktTri(int anzgptri, long pkt);
extern int MGradOmegaPrism(double r, double s, double t,double *vf);
/*  Berechnet Gradient der 3D Ansatzfunktionen */
extern int MOmegaPrism(double *vf,double r, double s, double t);
/*  Berechnet 3D Ansatzfunktionen */



/* Faktoren fuer die X Punkt Gauss-Integration */
//


extern void CalcElementVolume(long number, double *volume);
  /* Berechnet das Volumen des Elements number */

extern void Calc1DElementVolume(long number, double *volume);
extern void Calc2DElementVolume(long number, double *volume);
extern void Calc3DElementVolume(long number, double *volume);
extern double CalcBaseAreaPrism(long index);
  /* Berechnet das Volumen des Elements number (1D, 2D, 3D) */

  /* Interpoliert Knotenwerte */
extern double InterpolValue(long number, int ndx, double r, double s, double t);
#ifdef obsolete //WW. 06.11.2008
extern double InterpolValueVector(long number, double *werte, double r, double s, double t);
#endif

extern void Calc2DElement_ab2rs_Coord(long index, double *erg, double a, double b);
extern void Calc2DElement_rs2ab_Coord(long index, double *erg, double r, double s);
extern void Calc2DElement_xyz2ab_Coord(long index, double *erg, double x, double y, double z);
extern void Calc2DElement_ab2xyz_Coord(long index, double *erg, double a, double b);
  /* Koordinatentransformationen 2D-Element */

extern void Calc2DElement_ab2xyz_Vector(long index, double *vec_ab, double *vec_xyz);
extern void Calc2DElement_xyz2ab_Vector(long index, double *vec_xyz, double *vec_ab);
extern void Calc2DEdgeUnitNormalVec(long index, long side, double *n);

/* Weitere externe Objekte */
#ifndef NEW_EQS //WW. 06.11.2008
extern void IncorporateMatrix(long element, double *left_matrix, double *right_vector, double *rechts);
#endif
extern void FEMCorrectFluxesOverIrregularNodes(int ndx);
extern void FEMCorrectFluxesOverIrregularNodesOnVector(double *flux);
extern void FEMCalcElementCharacterisiticLength(long index); //CMCD 05.2004

#endif
