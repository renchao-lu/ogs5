/**************************************************************************/
/* ROCKFLOW - Modul: tools.h
                                                                          */
/* Aufgabe:
   verschiedene Funktionen, die von verschiedenen Modulen gebraucht
   werden und keine Adaptivitaet voraussetzen (sie aber teilweise
   unterstuetzen)
                                                                          */
/**************************************************************************/

#ifndef tools_INC

#define tools_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "prototyp.h"
#include "rf_pcs.h"
#include "rf_mmp_new.h" //MB


#include <sstream>
using namespace std;

/* Deklarationen */
/* Deklarationen fuer Schluesselwort #CURVES */
typedef struct {  /* fuer Kurven (Stuetzstellen) */
    double punkt;
    double wert;
} StuetzStellen;

typedef struct {  /* fuer Kurven (Kurven) */
    long anz_stuetzstellen;
      /* Anzahl der Stuetzstellen */
    StuetzStellen *stuetzstellen;
      /* Feld mit den eingelesenen Stuetzstellen */
} Kurven;

/* Liefert Wert einer Kurve fuer die angegebene Stuetzstelle */
extern double GetCurveValue ( int, int, double, int *);
/* Liefert Punkt einer Kurve fuer den angegebenen Wert */
extern double GetMatrixValue (double, double, int *); //NB
extern double GetCurveValueInverse ( int, int, double, int *);
/* Liefert die Ableitung einer Kurve fuer den angegebenen Punkt */
extern double GetCurveDerivative(int kurve, int methode, double punkt, int *gueltig);

extern Kurven *kurven;  /* Feld mit Kurven */
extern int anz_kurven;  /* Anzahl der Kurven */

extern int CreateFunctionsData(void);
extern void DestroyFunctionsData(void);

/******************************************************/
/* C1.11 Miscellaneous                                 */
/******************************************************/

/* Deklarationen fuer Schluesselwort #FRACTURE_APERTURE_DISTRIBUTION */
extern double *fracture_aperture_array;
  /* Feld mit Knoten, die ausgegeben werden sollen */
extern long fracture_aperture_anz;  /* Feldgroesse */




/* Baut Element-zu-Knoten-Verzeichnis auf (auch adaptiv) */
extern void ConstructElemsToNodesList ( void );

/* belegt Loesungsvektor vor --> NULLE_ERGEBNIS */
extern void PresetErgebnis ( double *ergebnis, int nidx );

/* berechnet Anfangszeitschritt --> automat. Zeitschrittsteuerung (aTM) */
extern double StartTimeStep ( int dtidx );

/* Sucht massgebendes Frontelement --> automat. Zeitschrittsteuerung (aTM) */
extern long FindFrontElement ( double *mindt, int nidx, int dtidx,
                               DoubleXFuncLDX ElGetElJacobi );

/* Ermittelt den zulaessigen Courant-Zeitschritt pro Element */
extern double CalcCourantTimeStep ( long index, long ndx , double dc);

/* Ermittelt den zulaessigen Courant-Zeitschritt im System */
extern double CalcSystemCourantTimeStep ( long ndx , double dc);

/* Sucht den zulaessigen Courant/Neumann-Zeitschritt im System */
extern double GetSystemCourantNeumannTimeStep ( long ndx, int dtidx, double acknowledge );

/* Testet, ob sich Knotenwerte im Element geaendert haben */
extern int TestElementDirtyness ( long index, long ndx1, long ndx2, double acknowledge);


/* Ermittelt das Vorzeichen */
extern int Signum(double);

/* Vergleicht zwei Zeichenketten, wobei einen sicheres Handling von
   Konstanten gewaehrleistet wird. Es wird zwischen Klein und
   Grossbuchstaben nicht unterschieden */
extern int StrCmp ( char *string1, char *string2 );

/* Bildet den arithmetischen Mittel einer Elementgroesse durch Interpolation
   der zugehoerigen Knotenwerte */
double InterpolateElementNodesValues ( long index, long idx );

extern int FctCurves ( char *data, int found, FILE *f );
extern int FctFractureApertureDistribution ( char *data, int found, FILE *f );
extern int FctReadHeterogeneousPermeabilityField(char *name_file); //SB
long GetNearestElement(double x,double y,double z, int * help); //SB
long GetNearestHetVal(long EleIndex, CFEMesh*, long no_values, double ** invals); //SB
double GetAverageHetVal(long EleIndex, CFEMesh*, long no_values, double ** invals); //MB
extern double GetHetValue(int,char*); //SB

typedef struct{
	int nof; //number of field values
	char *filename; // Name of input file for heterogeneous values
	char ** names; //Names of field variables
	double *convertfact; //conversion factors
}hetfields; //SB

extern hetfields * hf;  //SB
extern hetfields * Createhetfields(int n, char* name_file); //SB
extern void set_hetfields_name(hetfields *hf ,int i, char * name); //SB
extern char* get_hetfields_name(hetfields *hf,int i); //SB
extern int get_hetfields_number(hetfields *hf); //SB
//extern int FctReadHeterogeneousFields(char *name_file); //MB
extern int FctReadHeterogeneousFields(char *name_file, CMediumProperties*); //MB

/* Sortierfunktionen fuer Matrizen und Vektoren */
extern void P0260 (long imat[], long ma, long na, long iz1, long iz2);
extern void P0265 (long ivec[], double rvec[], long n, long *sort);

/* Daempft Oszillationen im Loesungsfeld */
extern long DampOscillations(int ndx1, int oscil_damp_method, double *oscil_damp_parameter, double (*NodeCalcLumpedMass) (long));

extern void TOLSortDataSet(double*, double*, double*, int);

extern int GetLineFromFile(char*, ifstream*);

typedef struct
{
  long row;
  long col;
  double **m;
} DMATRIX;

/* Erzeugt eine Instanz vom Typ DMATRIX */
extern DMATRIX *CreateDoubleMatrix(long row, long col);
/* Zerstoert die Instanz vom Typ DMATRIX */
extern void DestroyDoubleMatrix(DMATRIX *dm);


typedef struct
{
  long row;
  long col;
  long **m;
} LMATRIX;

/* Erzeugt eine Instanz vom Typ LMATRIX */
extern LMATRIX *CreateLongMatrix(long row, long col);
/* Zerstoert die Instanz vom Typ LMATRIX */
extern void DestroyLongMatrix(LMATRIX *lm);


#endif
