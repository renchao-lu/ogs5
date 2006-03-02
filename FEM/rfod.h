/**************************************************************************/
/* ROCKFLOW - Modul: rfod.h
                                                                          */
/* Aufgabe: Randbedingungen
   - Datenstrukturen und Definitionen fuer den Datentyp OBSERVATION_DATA
   - Verwaltung von OBSERVATION_DATA mit dem Datentyp LIST_OBSERVATION_DATA
   - Bereitstellung von Methoden fuer den Datentyp OBSERVATION_DATA
   - Schnittstellenfunktion zu Rockflow-Modelle

                                                                          */
/**************************************************************************/


#ifndef rfod_INC

#define rfod_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


typedef struct {
    char *name;
    char *group_name;
    char *process_name;
    char *iter_name;

    double lse;

    long type;
    double time_radius;

    long curve;
    int index;

    long node;
    long count_of_values;
    double *values;
    long count_of_times;
    double *times;
    long count_of_coeffs;
    double *coeffs;
    long count_of_points;

    double *x;
    double *y;
    double *z;
    double radius;
    long *nodes;
} OBSERVATION_DATA;



/* Erzeugt eine Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *create_observation_data(char *name);
/* Zerstoert die Instanz vom Typ OBSERVATION_DATA */
extern void destroy_observation_data(void *member);
/* Initialisierung der Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data(OBSERVATION_DATA *od, \
                                               long count_of_values);
/* Loescht die Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *delete_observation_data(OBSERVATION_DATA *od);

/* Initialisierung der Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data_values(OBSERVATION_DATA *od, \
                                               long count_of_values);
/* Initialisierung der Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data_times(OBSERVATION_DATA *od, \
                                               long count_of_times);
/* Initialisierung der Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data_coeffs(OBSERVATION_DATA *od, \
                                               long count_of_coeffs);


/* Initialisierung der Koordinaten fuer eine Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data_coor(OBSERVATION_DATA *od, \
                                                    long count_of_points);
/* Loescht die Koordinaten fuer Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *delete_observation_data_coor(OBSERVATION_DATA *od);
/* Initialisierung die Punkte einer Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *init_observation_data_nodes(OBSERVATION_DATA *od, \
                                                     long count_of_points);
/* Loescht die Punkte einer Instanz vom Typ OBSERVATION_DATA */
extern OBSERVATION_DATA *delete_observation_data_nodes(OBSERVATION_DATA *od);

/* Setzt Name des OD's */
extern void set_od_name(OBSERVATION_DATA *od, char *name);
/* Liefert Name des OD's */
extern char *get_od_name(OBSERVATION_DATA *od);
/* Setzt Gruppen-Name des OD's */
extern void set_od_group_name(OBSERVATION_DATA *od, char *group_name);
/* Liefert Gruppen-Name des OD's */
extern char *get_od_group_name(OBSERVATION_DATA *od);
/* Setzt Prozess-Name des OD's */
extern void set_od_process_name(OBSERVATION_DATA *od, char *process_name);
/* Liefert Prozess-Name des OD's */
extern char *get_od_process_name(OBSERVATION_DATA *od);
/* Setzt Iterationsname des OD's */
extern void set_od_iter_name(OBSERVATION_DATA *od, char *iter_name);
/* Liefert Iterationsname des OD's */
extern char *get_od_iter_name(OBSERVATION_DATA *od);
/* Setzt Typ des OD's */
void set_od_type(OBSERVATION_DATA *od, long type);
/* Liefert Typ des OD's */
long get_od_type(OBSERVATION_DATA *od);

/* Setzt least square error des OD's */
void set_od_lse(OBSERVATION_DATA *od, double lse);
/* Liefert least square error des OD's */
double get_od_lse(OBSERVATION_DATA *od);

/* Setzt Zeitradius des OD's */
void set_od_time_radius(OBSERVATION_DATA *od, double time_radius);
/* Liefert Zeitradius des OD's */
double get_od_time_radius(OBSERVATION_DATA *od);
/* Setzt Modus des OD's */
void set_od_curve(OBSERVATION_DATA *od, long curve);
/* Liefert Modus des OD's */
long get_od_curve(OBSERVATION_DATA *od);
/* Setzt Modus des OD's */
void set_od_index(OBSERVATION_DATA *od, int index);
/* Liefert Modus des OD's */
int get_od_index(OBSERVATION_DATA *od);

/* Setzt Anzahl der Werte des OD's */
extern void set_od_count_of_values(OBSERVATION_DATA *od, long count_of_values);
/* Liefert Anzahl der Werte des OD's */
extern long get_od_count_of_values(OBSERVATION_DATA *od);
/* Setzt Anzahl der Werte des OD's */
extern void set_od_count_of_times(OBSERVATION_DATA *od, long count_of_times);
/* Liefert Anzahl der Werte des OD's */
extern long get_od_count_of_times(OBSERVATION_DATA *od);
/* Setzt Anzahl der Werte des OD's */
extern void set_od_count_of_coeffs(OBSERVATION_DATA *od, long count_of_coeffs);
/* Liefert Anzahl der Werte des OD's */
extern long get_od_count_of_coeffs(OBSERVATION_DATA *od);

/* Setzt Knoten des OD's */
extern void set_od_node(OBSERVATION_DATA *ic, long node);
/* Liefert Knoten des OD's */
extern long get_od_node(OBSERVATION_DATA *ic);

/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's */
void set_od_value(OBSERVATION_DATA *od, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's */
double get_od_value(OBSERVATION_DATA *od, long index);
/* Liefert Zeiger auf Feldwerte des OD's */
double *get_od_values(OBSERVATION_DATA *od, long *count_of_values);
/* Liefert Zeiger auf Feldwerte des OD's */
double *get_od_times(OBSERVATION_DATA *od, long *count_of_times);
/* Liefert Zeiger auf Feldwerte des OD's */
double *get_od_coeffs(OBSERVATION_DATA *od, long *count_of_values);

/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des OD's */
void set_od_time(OBSERVATION_DATA *od, long index, double time);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des OD's */
double get_od_time(OBSERVATION_DATA *od, long index);
/* Setzt X-Koordinate mit dem Index (index) des OD's */
void set_od_x(OBSERVATION_DATA *od, long index, double xcoor);
/* Liefert X-Koordinate mit dem Index (index) des OD's */
double get_od_x(OBSERVATION_DATA *od, long index);
/* Setzt Y-Koordinate mit dem Index (index) des OD's */
void set_od_y(OBSERVATION_DATA *od, long index, double ycoor);
/* Liefert Y-Koordinate mit dem Index (index) des OD's */
double get_od_y(OBSERVATION_DATA *od, long index);
/* Setzt Z-Koordinate mit dem Index (index) des OD's */
void set_od_z(OBSERVATION_DATA *od, long index, double zcoor);
/* Liefert Z-Koordinate mit dem Index (index) des OD's */
double get_od_z(OBSERVATION_DATA *od, long index);
/* Setzt Verteilungsradius OD's */
void set_od_radius(OBSERVATION_DATA *od, double radius);
/* Liefert Verteilungsradius des OD's */
double get_od_radius(OBSERVATION_DATA *od);

/* Setzt Knotennummer mit dem Index (index) des OD's */
void set_od_nodes(OBSERVATION_DATA *od, long index, long node);
/* Liefert Knotennummer mit dem Index (index) des OD's */
long get_od_nodes(OBSERVATION_DATA *od, long index);


typedef struct {
    char *name;
    List *od_list;
    long count_of_observation_data;
    char **names_of_observation_data;
    long count_of_observation_data_name;
    double *od_vector;
} LIST_OBSERVATION_DATA;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_OBSERVATION_DATA *create_observation_data_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_observation_data_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int observation_data_list_empty(void);

/* Bekanntmachung der Randbedingung mit dem Name name. */
extern long declare_observation_data_name (char *name);
/*   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_observation_data_names (void);

/* Fuegt eine Instanz vom Typ OBSERVATION_DATA in die Liste */
extern long insert_observation_data_list (OBSERVATION_DATA *od);

/* Prueft auf Existenz eines Namens */
long observation_data_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateObservationDataList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyObservationDataList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int ObservationDataListEmpty(void);


/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern OBSERVATION_DATA *GetObservationDataGroup(char *name,OBSERVATION_DATA *od);
  /* Randbedingungstabellenspalte initialisieren */
long IndexObservationData(char *name);
  /* Zerstoert saemtliche OBSERVATION_DATA-Objekte der Gruppe NAME */
extern int DestroyObservationDataListGroup(char *name);


/* Methoden ausfuehren */

/* OD-Modell */

/* Alle OD-Objekte mit dem Namen (name) konfigurieren */
void SetObservationData(char *name);



/* OD-RF */

/* Liefert die Anzahl der gesetzten Namen fuer die OD. */
long GetObservationDataNamesNumber (void);
/* Liefert einen gesetzten Namen fuer die OD. */
char *GetObservationDataName (long i);

/* news to inverse modelling */
void SetAllObservationData (void);
void InitAllObservationData (void);
void ConfigAllObservationData (void);
void SaveAllModelObservationData (void);
void SaveToLastAllModelObservationData (void);
void CopyToLastAllModelObservationData (void);

double *GetTimesObservationData (char *name, long *number_of_times);
double *GetValuesObservationData (char *name, long *number_of_values);
double *GetCoeffsObservationData (char *name, long *number_of_values);
double *GetModelValuesObservationData (char *name, long *number_of_values);
double *GetLastModelValuesObservationData (char *name, long *number_of_values);

char *GetIterationNameObservationData (char *name);
int SaveLeastSquareErrorObservationData (char *name, double lse);
int GetLeastSquareErrorObservationData (char *name, double *lse);
int GetAllLeastSquareErrorObservationData(double *lse);

int WriteBothValuesObservationData (char *name, FILE *f);

int CalculateSensitivityParameterInverseIteration(char *name, double dx, double *dfdx);
int CalculateRightSideSensitivityParameterInverseIteration(char *name, double *dfdx);


/* Lesefunktionen */
extern int FctObservationData ( char *data, int found, FILE *f );

/* Steuerwort */
extern char *keyword_observation_data;



/* =============== Tools ============== */

typedef struct
{
  long row;
  long *col;
  double **m;
} VARDMATRIX;

/* Erzeugt eine Instanz vom Typ DMATRIX */
extern VARDMATRIX *CreateVarDoubleMatrix(long row);
/* Zerstoert die Instanz vom Typ DMATRIX */
extern void DestroyVarDoubleMatrix(VARDMATRIX *dm);
/* Initialisiert die Instanz vom Typ DMATRIX */
extern VARDMATRIX *InitVarDoubleMatrix(VARDMATRIX *vdm, long row, long col);

/* Uebertraegt die Matrix-Inhalte von src nach dest */
VARDMATRIX *CopyVarDoubleMatrix(VARDMATRIX *dest, VARDMATRIX *src);

#endif


