/**************************************************************************/
/* ROCKFLOW - Modul: rfiter.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps INVERSE_ITERATION
   (Iterationseigenschaften).
                                                                          */
/**************************************************************************/


#ifndef rfii_INC

#define rfii_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


/* ================================================================ */
/* INVERSE_ITERATION */
/* ================================================================ */

extern char *keyword_inverse_iteration;


typedef struct {
    char *name;
    char *group_name;
    char *process_name;
    long type;

    double lse;

    int norm;
    long maxiter;
    double abs_eps;
    double rel_eps;

    double var_eps;
    double begin_eps;
    double end_eps;
    double fac_eps;
    double exp_eps;
    long kind;

    long criterium;
    double rel_cg_eps;
    double theta;
    double time;

    long level;
    long count_of_values;
    double *values;
    long distribution_type;

} INVERSE_ITERATION;



/* Erzeugt eine Instanz vom Typ INVERSE_ITERATION */
extern INVERSE_ITERATION *create_inverse_iteration(char *name);
/* Zerstoert die Instanz vom Typ INVERSE_ITERATION */
extern void destroy_inverse_iteration(void *member);
/* Initialisierung der Instanz vom Typ INVERSE_ITERATION */
extern INVERSE_ITERATION *init_inverse_iteration(INVERSE_ITERATION *ii, \
                                                       long count_of_values);
/* Loescht die Instanz vom Typ INVERSE_ITERATION */
extern INVERSE_ITERATION *delete_inverse_iteration(INVERSE_ITERATION *ii);

/* Setzt Name des II's */
extern void set_ii_name(INVERSE_ITERATION *ii, char *name);
/* Liefert Name des II's */
extern char *get_ii_name(INVERSE_ITERATION *ii);
/* Setzt Gruppen-Name des II's */
extern void set_ii_group_name(INVERSE_ITERATION *ii, char *group_name);
/* Liefert Gruppen-Name des II's */
extern char *get_ii_group_name(INVERSE_ITERATION *ii);
/* Setzt Prozess-Name des II's */
extern void set_ii_process_name(INVERSE_ITERATION *ii, char *process_name);
/* Liefert Prozess-Name des II's */
extern char *get_ii_process_name(INVERSE_ITERATION *ii);
/* Setzt Typ des II's */
void set_ii_type(INVERSE_ITERATION *ii, long type);
/* Liefert Typ des II's */
long get_ii_type(INVERSE_ITERATION *ii);

/* Setzt least square error des II's */
void set_ii_lse(INVERSE_ITERATION *ii, double lse);
/* Liefert least square error des II's */
double get_ii_lse(INVERSE_ITERATION *ii);

/* Setzt maximale Iterationszahl des II's */
void set_ii_maxiter(INVERSE_ITERATION *ii, long maxiter);
/* Liefert maximale Iterationszahl des II's */
long get_ii_maxiter(INVERSE_ITERATION *ii);
/* Setzt Kriterium (Konvergenztyp) des II's */
void set_ii_criterium(INVERSE_ITERATION *ii, long type);
/* Liefert Kriterium (Konvergenztyp) des II's */
long get_ii_criterium(INVERSE_ITERATION *ii);
/* Setzt Norm des II's */
void set_ii_norm(INVERSE_ITERATION *ii, int norm);
/* Liefert Norm des II's */
int get_ii_norm(INVERSE_ITERATION *ii);

/* Setzt absolute Genauigkeit des II's */
void set_ii_abs_eps(INVERSE_ITERATION *ii, double abs_eps);
/* Liefert absolute Genauigkeit des II's */
double get_ii_abs_eps(INVERSE_ITERATION *ii);
/* Setzt relative Genauigkeit des II's */
void set_ii_rel_eps(INVERSE_ITERATION *ii, double rel_eps);
/* Liefert relative Genauigkeit des II's */
double get_ii_rel_eps(INVERSE_ITERATION *ii);
/* Setzt relative Genauigkeit des II's */
void set_ii_rel_cg_eps(INVERSE_ITERATION *ii, double rel_eps);
/* Liefert relative Genauigkeit des II's */
double get_ii_rel_cg_eps(INVERSE_ITERATION *ii);
/* Setzt Gewichtsfaktor des II's */
void set_ii_theta(INVERSE_ITERATION *lsp, double theta);
/* Liefert Gewichtsfaktor des II's */
double get_ii_theta(INVERSE_ITERATION *lsp);
/* Setzt die Zeit der Gueltigkeit des II's */
void set_ii_time(INVERSE_ITERATION *lsp, double time);
/* Liefert die Zeit der Gueltigkeit des II's */
double get_ii_time(INVERSE_ITERATION *lsp);
/* Setzt Art der Zeitgueltigkeit des II's */
void set_ii_kind(INVERSE_ITERATION *ii, long kind);
/* Liefert Art der Zeitgueltigkeit des II's */
long get_ii_kind(INVERSE_ITERATION *ii);

/* Setzt Anfangsgenauigkeit des II's */
void set_ii_var_eps(INVERSE_ITERATION *ii, double var_eps);
/* Liefert Anfangsgenauigkeit des II's */
double get_ii_var_eps(INVERSE_ITERATION *ii);
/* Setzt Anfangsgenauigkeit des II's */
void set_ii_begin_eps(INVERSE_ITERATION *ii, double begin_eps);
/* Liefert Anfangsgenauigkeit des II's */
double get_ii_begin_eps(INVERSE_ITERATION *ii);
/* Setzt Endgenauigkeit des II's */
void set_ii_end_eps(INVERSE_ITERATION *ii, double end_eps);
/* Liefert Endgenauigkeit des II's */
double get_ii_end_eps(INVERSE_ITERATION *ii);
/* Setzt Faktor der variablen Genauigkeit des II's */
void set_ii_fac_eps(INVERSE_ITERATION *ii, double fac_eps);
/* Liefert Faktor der variablen Genauigkeit des II's */
double get_ii_fac_eps(INVERSE_ITERATION *ii);
/* Setzt Exponenten der variablen Genauigkeit des II's */
void set_ii_exp_eps(INVERSE_ITERATION *ii, double exp_eps);
/* Liefert Exponenten der variablen Genauigkeit des II's */
double get_ii_exp_eps(INVERSE_ITERATION *ii);


/* Setzt Modus des II's */
void set_ii_curve(INVERSE_ITERATION *ii, long curve);
/* Liefert Modus des II's */
long get_ii_curve(INVERSE_ITERATION *ii);
/* Setzt Modus des II's */
void set_ii_index(INVERSE_ITERATION *ii, int index);
/* Liefert Modus des II's */
int get_ii_index(INVERSE_ITERATION *ii);
/* Setzt Modus des II's */
void set_ii_mode(INVERSE_ITERATION *ii, long mode);
/* Liefert Modus des II's */
long get_ii_mode(INVERSE_ITERATION *ii);
/* Setzt Anzahl der Werte des II's */
extern void set_ii_count_of_values(INVERSE_ITERATION *ii, long count_of_values);
/* Liefert Anzahl der Werte des II's */
extern long get_ii_count_of_values(INVERSE_ITERATION *ii);
/* Setzt Anfangsknoten des IC's */
extern void set_ii_begin_node(INVERSE_ITERATION *ic, long begin_node);
/* Liefert Anfangsknoten des IC's */
extern long get_ii_begin_node(INVERSE_ITERATION *ic);
/* Setzt Endknoten des IC's */
extern void set_ii_end_node(INVERSE_ITERATION *ic, long end_node);
/* Liefert Endknoten des IC's */
long get_ii_end_node(INVERSE_ITERATION *ic);
/* Setzt Schrittweite der Zwischenknoten des IC's */
void set_ii_step_nodes(INVERSE_ITERATION *ic, long step_nodes);
/* Liefert Schrittweite der Zwischenknoten des IC's */
long get_ii_step_nodes(INVERSE_ITERATION *ic);
/* Setzt Anfangsknoten des II's */
extern void set_ii_begin_element(INVERSE_ITERATION *ii, long begin_element);
/* Liefert Anfangsknoten des II's */
extern long get_ii_begin_element(INVERSE_ITERATION *ii);
/* Setzt Endknoten des II's */
extern void set_ii_end_element(INVERSE_ITERATION *ii, long end_element);
/* Liefert Endknoten des II's */
long get_ii_end_element(INVERSE_ITERATION *ii);
/* Setzt Schrittweite der Zwischenknoten des II's */
void set_ii_step_elements(INVERSE_ITERATION *ii, long step_elements);
/* Liefert Schrittweite der Zwischenknoten des II's */
long get_ii_step_elements(INVERSE_ITERATION *ii);
/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des II's */
void set_ii_value(INVERSE_ITERATION *ii, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des II's */
double get_ii_value(INVERSE_ITERATION *ii, long index);
/* Setzt Verteilungstyp des II's */
void set_ii_distribution_type(INVERSE_ITERATION *ii, long distribution_type);
/* Liefert Verteilungstyp des II's */
long get_ii_distribution_type(INVERSE_ITERATION *ii);
/* Setzt X-Koordinate mit dem Index (index) des II's */
void set_ii_x(INVERSE_ITERATION *ii, long index, double xcoor);
/* Liefert X-Koordinate mit dem Index (index) des II's */
double get_ii_x(INVERSE_ITERATION *ii, long index);
/* Setzt Y-Koordinate mit dem Index (index) des II's */
void set_ii_y(INVERSE_ITERATION *ii, long index, double ycoor);
/* Liefert Y-Koordinate mit dem Index (index) des II's */
double get_ii_y(INVERSE_ITERATION *ii, long index);
/* Setzt Z-Koordinate mit dem Index (index) des II's */
void set_ii_z(INVERSE_ITERATION *ii, long index, double zcoor);
/* Liefert Z-Koordinate mit dem Index (index) des II's */
double get_ii_z(INVERSE_ITERATION *ii, long index);
/* Setzt Verteilungsradius II's */
void set_ii_radius(INVERSE_ITERATION *ii, double radius);
/* Liefert Verteilungsradius des II's */
double get_ii_radius(INVERSE_ITERATION *ii);
/* Setzt Level des II's */
void set_ii_level(INVERSE_ITERATION *ii, long level);
/* Liefert Level des II's */
long get_ii_level(INVERSE_ITERATION *ii);
/* Setzt Traecernummer II's */
void set_ii_component_number(INVERSE_ITERATION *ii, int component_number);
/* Liefert Traecernummer des II's */
int get_ii_component_number(INVERSE_ITERATION *ii);
/* Setzt Knotennummer mit dem Index (index) des II's */
void set_ii_nodes(INVERSE_ITERATION *ii, long index, long node);
/* Liefert Knotennummer mit dem Index (index) des II's */
long get_ii_nodes(INVERSE_ITERATION *ii, long index);



typedef struct {
    char *name;
    LList *ii_list;
    long count_of_inverse_iteration;
    char **names_of_inverse_iteration;
    long count_of_inverse_iteration_name;
    double *ii_vector;
} LIST_INVERSE_ITERATION;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_INVERSE_ITERATION *create_inverse_iteration_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_inverse_iteration_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int inverse_iteration_list_empty(void);

/* Bekanntmachung der Iteration mit dem Name name. */
extern long declare_inverse_iteration_name (char *name);
/*   Alle Iterationen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_inverse_iteration_names (void);

/* Fuegt eine Instanz vom Typ INVERSE_ITERATION in die Liste */
extern long insert_inverse_iteration_list (INVERSE_ITERATION *ii);

/* Prueft auf Existenz eines Namens */
long inverse_iteration_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateInverseIterationList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyInverseIterationList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int InverseIterationListEmpty(void);
  /* Spaltenindex der IP-Objekte mit dem Namen (name) */
long IndexInverseIteration(char *name);


/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern INVERSE_ITERATION *GetInverseIterationGroup(char *name,INVERSE_ITERATION *ii);
  /* Zerstoert saemtliche INVERSE_ITERATION-Objekte der Gruppe NAME */
extern int DestroyInverseIterationListGroup(char *name);


/* Methoden ausfuehren */

/* Setzt einen Knoten-Iteration (Eingabe in Knotennummer) */
int ExecuteInverseIterationMethod0 (INVERSE_ITERATION *ii, double *current, double *last, long dim, double *cur_eps);
/*void ExecuteInverseIterationMethod0 (INVERSE_ITERATION *ii);*/
/* Setzt einen Knoten-Iteration (Eingabe in x, y und z) */
int ExecuteInverseIterationMethod1 (INVERSE_ITERATION *ii, 
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps);
int ExecuteInverseIterationMethod2 (INVERSE_ITERATION *ii, 
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps);


/* IP-Modell */

/* Alle IP-Objekte mit dem Namen (name) konfigurieren */
void SetInverseIteration(char *name);
  /* Ganze Iterationstabelle initialisieren */
void InitAllInverseIteration (void);
  /* Iterationstabellenspalte initialisieren */
void InitInverseIteration (char *name);
/* Iterationen mit dem Namen name ausfuehren */
int ExecuteInverseIteration (char *name, double *current, double *last,
                                double *ref, long dim);

/* Liefert Anzahl der ausgefuehrten Iterationen */
long GetIterationNumberInverseIteration (char *name);
/* Liefert die Konvergenzrate der ausgefuehrten Iteration */
double GetConvergenceRateInverseIteration (char *name);


/* News */
int ExecuteAllInverseIteration (char *db_name, double *lse);
int PrintInverseIteration (char *name, char *text);
char *GetPocessNameInverseIteration(char *name);
int ExecuteInverseIterationNew (char *iv_name, char *name, char *db_name, double *lse);
int ConfigInverseIteration (char *name);


/* Lesefunktionen */
extern int FctInverseIteration ( char *data, int found, FILE *f );

#endif

