/**************************************************************************/
/* ROCKFLOW - Modul: rfpriref.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps APRIORI_REFINE
   (Vorverfeinerer).
                                                                          */
/**************************************************************************/


#ifndef rfpriref_INC

#define rfpriref_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"

#include "nodes.h" /* OK 3261 */


typedef struct {
    char *name;
    long type;
    long mode;
    long level;
    long start_element;
    long end_element;
    long step_elements;
    long count_of_values;
    double *values;
    double *x;
    double *y;
    double *z;
    long count_of_points;
    long refine_type;
    double radius;
    long distribution_type;
    long base_type;
    double base_value;
} APRIORI_REFINE;



/* Erzeugt eine Instanz vom Typ APRIORI_REFINE */
extern APRIORI_REFINE *create_apriori_refine(char *name);
/* Zerstoert die Instanz vom Typ APRIORI_REFINE */
extern void destroy_apriori_refine(void *member);
/*extern APRIORI_REFINE *destroy_apriori_refine(APRIORI_REFINE *ar);*/
/* Initialisierung der Instanz vom Typ APRIORI_REFINE */
extern APRIORI_REFINE *init_apriori_refine(APRIORI_REFINE *ar, \
                                                                                        long count_of_values);
/* Loescht die Instanz vom Typ APRIORI_REFINE */
extern APRIORI_REFINE *delete_apriori_refine(APRIORI_REFINE *ar);
/* Initialisierung der Koordinaten fuer eine Instanz vom Typ APRIORI_REFINE */
extern APRIORI_REFINE *init_apriori_refine_coor(APRIORI_REFINE *ar, \
                                                                                        long count_of_points);
/* Loescht die Koordinaten fuer Instanz vom Typ APRIORI_REFINE */
extern APRIORI_REFINE *delete_apriori_refine_coor(APRIORI_REFINE *ar);
/* Lesen aus einem string */
extern APRIORI_REFINE *sread_apriori_refine(APRIORI_REFINE *ar, char *string);
/* Lesen aus einer Datei */
extern APRIORI_REFINE *fread_apriori_refine(APRIORI_REFINE *ar, FILE *fp);

/* Setzt Name des AR's */
extern void set_ar_name(APRIORI_REFINE *ar, char *name);
/* Liefert Name des AR's */
extern char *get_ar_name(APRIORI_REFINE *ar);
/* Setzt Typ des AR's */
void set_ar_type(APRIORI_REFINE *ar, long type);
/* Liefert Typ des AR's */
long get_ar_type(APRIORI_REFINE *ar);
/* Setzt Modus des AR's */
void set_ar_mode(APRIORI_REFINE *ar, long mode);
/* Liefert Modus des AR's */
long get_ar_mode(APRIORI_REFINE *ar);
/* Setzt Anzahl der Werte des AR's */
extern void set_ar_count_of_values(APRIORI_REFINE *ar, long count_of_values);
/* Liefert Anzahl der Werte des AR's */
extern long get_ar_count_of_values(APRIORI_REFINE *ar);
/* Setzt Anfangsknoten des AR's */
extern void set_ar_start_element(APRIORI_REFINE *ar, long start_element);
/* Liefert Anfangsknoten des AR's */
extern long get_ar_start_element(APRIORI_REFINE *ar);
/* Setzt Endknoten des AR's */
extern void set_ar_end_element(APRIORI_REFINE *ar, long end_element);
/* Liefert Endknoten des AR's */
long get_ar_end_element(APRIORI_REFINE *ar);
/* Setzt Schrittweite der Zwischenknoten des AR's */
void set_ar_step_elements(APRIORI_REFINE *ar, long step_elements);
/* Liefert Schrittweite der Zwischenknoten des AR's */
long get_ar_step_elements(APRIORI_REFINE *ar);
/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des AR's */
void set_ar_value(APRIORI_REFINE *ar, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des AR's */
double get_ar_value(APRIORI_REFINE *ar, long index);
/* Setzt Verteilungstyp des AR's */
void set_ar_refine_type(APRIORI_REFINE *ar, long refine_type);
/* Liefert Verteilungstyp des AR's */
long get_ar_refine_type(APRIORI_REFINE *ar);
/* Setzt X-Koordinate mit dem Index (index) des AR's */
void set_ar_x(APRIORI_REFINE *ar, long index, double xcoor);
/* Liefert X-Koordinate mit dem Index (index) des AR's */
double get_ar_x(APRIORI_REFINE *ar, long index);
/* Setzt Y-Koordinate mit dem Index (index) des AR's */
void set_ar_y(APRIORI_REFINE *ar, long index, double ycoor);
/* Liefert Y-Koordinate mit dem Index (index) des AR's */
double get_ar_y(APRIORI_REFINE *ar, long index);
/* Setzt Z-Koordinate mit dem Index (index) des AR's */
void set_ar_z(APRIORI_REFINE *ar, long index, double zcoor);
/* Liefert Z-Koordinate mit dem Index (index) des AR's */
double get_ar_z(APRIORI_REFINE *ar, long index);
/* Setzt Verteilungsradius AR's */
void set_ar_radius(APRIORI_REFINE *ar, double radius);
/* Liefert Verteilungsradius des AR's */
double get_ar_radius(APRIORI_REFINE *ar);
/* Setzt Verteilungstyp des IC's */
void set_ar_distribution_type(APRIORI_REFINE *ar, long distribution_type);
/* Liefert Verteilungstyp des IC's */
long get_ar_distribution_type(APRIORI_REFINE *ar);

/* Setzt Level des AR's */
void set_ar_level(APRIORI_REFINE *ar, long level);
/* Liefert Level des AR's */
long get_ar_level(APRIORI_REFINE *ar);

/* Setzt Basistyp des AR's */
void set_ar_base_type(APRIORI_REFINE *ar, long base_type);
/* Liefert Basistyp des AR's */
long get_ar_base_type(APRIORI_REFINE *ar);
/* Setzt Basiswert des AR's */
void set_ar_base_value(APRIORI_REFINE *ar, double base_value);
/* Liefert Basiswert des AR's */
void set_ar_base_value(APRIORI_REFINE *ar, double base_value);



typedef struct {
        char *name;
        LList *ar_list;
        long count_of_refine_elements;
        char **names_of_refine_elements;
        long count_of_refine_elements_name;
} LIST_APRIORI_REFINE;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_APRIORI_REFINE *create_apriori_refine_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_apriori_refine_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int refine_elements_list_empty(void);

/* Bekanntmachung der Randbedingung mit dem Name name. */
extern long declare_apriori_refine_name (char *name);
/*   Alle Vorverfeinerer von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_refine_elements_names (void);

/* Fuegt eine Instanz vom Typ APRIORI_REFINE in die Liste */
extern long insert_apriori_refine_list (APRIORI_REFINE *ar);

/* Prueft auf Existenz eines Namens */
long apriori_refine_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateRefineElementList();
/* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyRefineElementList();
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int RefineElementListEmpty(void);

/* Alle Objekte mit dem Namen (name) Ausfuehren */
void AprioriRefineElements(char *name);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Gleichmaessig Vorverfeinern */
void APrioriRefineElementsLevelM0(APRIORI_REFINE *ar);
long APrioriRefine1DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine2DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine3DElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine1DElementLevelMode0 (APRIORI_REFINE *ar, long i, int search);
long APrioriRefine2DDElementLevelAllModeRekursiv (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Ungleichmaessig Vorverfeinern */
void APrioriRefineElementsLevelM1(APRIORI_REFINE *ar);
long APrioriRefine1DElementLevelAllModeRekursivLeftRight (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine1DElementLevelAllModeRekursivLeft (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine1DElementLevelAllModeRekursivRight (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine2DElementLevelAllModeRekursivLeft (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um eine Quelle/Senke mit dem Index i */
void APrioriRefineElementsLevelM2(APRIORI_REFINE *ar);
long APrioriRefine1DElementLevelAllModeRekursivNode (APRIORI_REFINE *ar, long i, long level, int search);
long APrioriRefine2DElementLevelAllModeRekursivNode (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um eine Quelle/Senke.
   Wie Methode 2, aber Eingabe in x, y und z */
void APrioriRefineElementsLevelM3(APRIORI_REFINE *ar);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um eine Quelle/Senke.
   Wie Methode 3. Der Knoten wird geloescht (noch nicht benutzen!! */
void APrioriRefineElementsLevelM4(APRIORI_REFINE *ar);
void APrioriDeleteAllElementNode (APRIORI_REFINE *ar);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um allen Knoten mit der Methode 2 */
void APrioriRefineElementsLevelM5(APRIORI_REFINE *ar);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern entlang einer Kante (noch nicht benutzen)!! */
void APrioriRefineElementsLevelM6(APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivEdge (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                                                         long numnode2, long level, int search);

/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern ??? (noch nicht benutzen)!! */
void APrioriRefineElementsLevelM7(APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivNodeM2 (APRIORI_REFINE *ar, long i, long numnode, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Matrixdiffusion. Konstante Aufteilung */
void APrioriRefineElementsLevelM8 (APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivT3 (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Matrixdiffusion. Veraenderliche Aufteilung */
void APrioriRefineElementsLevelM9 (APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivT4 (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um allen Knoten, die sich auf einer Linie befinden */
void APrioriRefineElementsLevelM10 (APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivT2 (APRIORI_REFINE *ar, long i, long level, int search);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Vorverfeinern um allen Knoten, die sich in einer Umgebung von
   einem Punkt befinden */
void APrioriRefineElementsLevelM11 (APRIORI_REFINE *ar);


/* Verfeinert Alle Elemente a priori bis zu einem Level.
   Matrixdiffusion. Veraenderliche Aufteilung und mit Level */
void APrioriRefineElementsLevelM12 (APRIORI_REFINE *ar);
long APrioriRefine2DElementLevelAllModeRekursivT5 (APRIORI_REFINE *ar, long i, long level, int search);
/* Wie Methode 12. Verfeinert aber nur ein Element a priori bis zu einem Level.
   Matrixdiffusion. Veraenderliche Aufteilung und mit Level */
void APrioriRefineElementsLevelM13 (APRIORI_REFINE *ar);

/* Verfeinert Alle 2D-Elemente a priori bis zu einem Level */
void APrioriRefineElementsLevelM14 (APRIORI_REFINE *ar);
/* Wie Methode 14. Verfeinert aber nur ein 2D-Element a priori bis zu einem Level */
void APrioriRefineElementsLevelM15 (APRIORI_REFINE *ar);


/* Verfeinert a priori bis zu einem Level alle Elemente, die sich an einem Gerade angrenzen.
   Matrixdiffusion. Veraenderliche Aufteilung und mit Level */
void APrioriRefineElementsLevelM16 (APRIORI_REFINE *ar);

/* Anisotropes Vorverfeinern */
void APrioriRefineElementsLevelM17 (APRIORI_REFINE *ar);
long APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotrop (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long level, int search);
long APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotrop (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long level, int search);
/* Anisotropes Vorverfeinern (Angabe Intervall) */
void APrioriRefineElementsLevelM18 (APRIORI_REFINE *ar);
long APrioriRefine1DElementLevelAllModeRekursivEdgeAnisotropI (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long ni, int search);
long APrioriRefine2DElementLevelAllModeRekursivEdgeAnisotropI (APRIORI_REFINE *ar, long i, long numnode1, \
                                                                    long numnode2, long level, int search);


/* Tools */
int TestIfElementTriangle (long index);
Knoten *GetXNodeNumberClose (double xcoor, double ycoor, double zcoor, \
                                                         long *number, double *distance);
int GetMiddelXPoint (Knoten *node[], double *m, long np);
int GetMiddelXPoint1D (Knoten *node1, Knoten *node2, double *m);
int GetMiddelXPoint2D (Knoten *node1, Knoten *node2, Knoten *node3, Knoten *node4, \
                                          double *m);


int TangUnitVectorXPoint(Knoten *node1, Knoten *node2, double *t);
int GetXPoint(Knoten *node1, double *t1, double dist1, \
                          double *t2, double dist2, double *p);


/* Lesefunktion */
int FctAprioriRefineElement ( char *data, int found, FILE *f );

extern int REFListExists(void);

#endif

