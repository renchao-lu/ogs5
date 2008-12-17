/**************************************************************************/
/* ROCKFLOW - Modul: rfiter.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps ITERATION_PROPERTIES
   (Iterationseigenschaften).
                                                                          */
/**************************************************************************/


#ifndef rfiter_INC

#define rfiter_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


/* ================================================================ */
/* ITERATION_PROPERTIES */
/* ================================================================ */

extern char *keyword_iteration_properties_fluid_phase;
extern char *keyword_iteration_properties_water_content;
extern char *keyword_iteration_properties_saturation;
extern char *keyword_iteration_properties_tracer_component;
extern char *keyword_iteration_properties_solute_component;
extern char *keyword_iteration_properties_sorbed_component;
extern char *keyword_iteration_properties_temperature_phase;
extern char *keyword_iteration_properties_immobile_soluted_phase;


  /* Vordefinierten IP-Objektname ueber Makros holen */
#define DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(i) \
        GetDefaultNameIterationPropertiesFluidPhase(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(i) \
        GetDefaultNameIterationPropertiesTracerComponent(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_SOLUTE_COMPONENT(i) \
        GetDefaultNameIterationPropertiesSoluteComponent(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_SORBED_COMPONENT(i) \
        GetDefaultNameIterationPropertiesSorbedComponent(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_SATURATION(i) \
        GetDefaultNameIterationPropertiesSaturation(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_WATER_CONTENT(i) \
        GetDefaultNameIterationPropertiesWaterContent(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_TEMPRATURE_PHASE(i) \
        GetDefaultNameIterationPropertiesTemperaturePhase(i)
#define DEFAULT_NAME_ITERATION_PROPERTIES_IMMOBILE_SOLUTE_COMPONENT(i) \
        GetDefaultNameIterationPropertiesImmobileSoluteComponent(i)



extern char *name_iteration_properties_fluid_phase;
extern char *name_iteration_properties_water_content;
extern char *name_iteration_properties_saturation;
extern char *name_iteration_properties_tracer_component;
extern char *name_iteration_properties_solute_component;
extern char *name_iteration_properties_sorbed_component;
extern char *name_iteration_properties_temperature_phase;
extern char *name_iteration_properties_immobile_soluted_component;


extern void ConfigIterationProperties(void);
  /* Konfiguration von Iterations-Parametern */
extern void InitDefaultIterationPropertiesNames(char *sys_name);
  /* Alle Vordefinierte lineare Loesereigenschaften-Objekt-Namen konfigurieren */
void FreeDefaultIterationPropertiesNames(void);
  /* Destruktor */

extern char *GetDefaultNameIterationPropertiesFluidPhase(int number);
extern char *GetDefaultNameIterationPropertiesWaterContent(int number);
extern char *GetDefaultNameIterationPropertiesSaturation(int number);
extern char *GetDefaultNameIterationPropertiesTracerComponent(int number);
extern char *GetDefaultNameIterationPropertiesSoluteComponent(int number);
extern char *GetDefaultNameIterationPropertiesSorbedComponent(int number);
extern char *GetDefaultNameIterationPropertiesTemperaturePhase(int number);
extern char *GetDefaultNameIterationPropertiesImmobileSoluteComponent(int number);
  /* Vordefinierten IP-Objektname holen */


typedef struct {
    char *name;
    long type;

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

} ITERATION_PROPERTIES;



/* Erzeugt eine Instanz vom Typ ITERATION_PROPERTIES */
extern ITERATION_PROPERTIES *create_iteration_properties(char *name);
/* Zerstoert die Instanz vom Typ ITERATION_PROPERTIES */
extern void destroy_iteration_properties(void *member);
/* Initialisierung der Instanz vom Typ ITERATION_PROPERTIES */
extern ITERATION_PROPERTIES *init_iteration_properties(ITERATION_PROPERTIES *ip, \
                                                       long count_of_values);
/* Loescht die Instanz vom Typ ITERATION_PROPERTIES */
extern ITERATION_PROPERTIES *delete_iteration_properties(ITERATION_PROPERTIES *ip);

/* Setzt Name des IP's */
extern void set_ip_name(ITERATION_PROPERTIES *ip, char *name);
/* Liefert Name des IP's */
extern char *get_ip_name(ITERATION_PROPERTIES *ip);
/* Setzt Typ des IP's */
void set_ip_type(ITERATION_PROPERTIES *ip, long type);
/* Liefert Typ des IP's */
long get_ip_type(ITERATION_PROPERTIES *ip);
/* Setzt maximale Iterationszahl des IP's */
void set_ip_maxiter(ITERATION_PROPERTIES *ip, long maxiter);
/* Liefert maximale Iterationszahl des IP's */
long get_ip_maxiter(ITERATION_PROPERTIES *ip);
/* Setzt Kriterium (Konvergenztyp) des IP's */
void set_ip_criterium(ITERATION_PROPERTIES *ip, long type);
/* Liefert Kriterium (Konvergenztyp) des IP's */
long get_ip_criterium(ITERATION_PROPERTIES *ip);
/* Setzt Norm des IP's */
void set_ip_norm(ITERATION_PROPERTIES *ip, int norm);
/* Liefert Norm des IP's */
int get_ip_norm(ITERATION_PROPERTIES *ip);

/* Setzt absolute Genauigkeit des IP's */
void set_ip_abs_eps(ITERATION_PROPERTIES *ip, double abs_eps);
/* Liefert absolute Genauigkeit des IP's */
double get_ip_abs_eps(ITERATION_PROPERTIES *ip);
/* Setzt relative Genauigkeit des IP's */
void set_ip_rel_eps(ITERATION_PROPERTIES *ip, double rel_eps);
/* Liefert relative Genauigkeit des IP's */
double get_ip_rel_eps(ITERATION_PROPERTIES *ip);
/* Setzt relative Genauigkeit des IP's */
void set_ip_rel_cg_eps(ITERATION_PROPERTIES *ip, double rel_eps);
/* Liefert relative Genauigkeit des IP's */
double get_ip_rel_cg_eps(ITERATION_PROPERTIES *ip);
/* Setzt Gewichtsfaktor des IP's */
void set_ip_theta(ITERATION_PROPERTIES *lsp, double theta);
/* Liefert Gewichtsfaktor des IP's */
double get_ip_theta(ITERATION_PROPERTIES *lsp);
/* Setzt die Zeit der Gueltigkeit des IP's */
void set_ip_time(ITERATION_PROPERTIES *lsp, double time);
/* Liefert die Zeit der Gueltigkeit des IP's */
double get_ip_time(ITERATION_PROPERTIES *lsp);
/* Setzt Art der Zeitgueltigkeit des IP's */
void set_ip_kind(ITERATION_PROPERTIES *ip, long kind);
/* Liefert Art der Zeitgueltigkeit des IP's */
long get_ip_kind(ITERATION_PROPERTIES *ip);

/* Setzt Anfangsgenauigkeit des IP's */
void set_ip_var_eps(ITERATION_PROPERTIES *ip, double var_eps);
/* Liefert Anfangsgenauigkeit des IP's */
double get_ip_var_eps(ITERATION_PROPERTIES *ip);
/* Setzt Anfangsgenauigkeit des IP's */
void set_ip_begin_eps(ITERATION_PROPERTIES *ip, double begin_eps);
/* Liefert Anfangsgenauigkeit des IP's */
double get_ip_begin_eps(ITERATION_PROPERTIES *ip);
/* Setzt Endgenauigkeit des IP's */
void set_ip_end_eps(ITERATION_PROPERTIES *ip, double end_eps);
/* Liefert Endgenauigkeit des IP's */
double get_ip_end_eps(ITERATION_PROPERTIES *ip);
/* Setzt Faktor der variablen Genauigkeit des IP's */
void set_ip_fac_eps(ITERATION_PROPERTIES *ip, double fac_eps);
/* Liefert Faktor der variablen Genauigkeit des IP's */
double get_ip_fac_eps(ITERATION_PROPERTIES *ip);
/* Setzt Exponenten der variablen Genauigkeit des IP's */
void set_ip_exp_eps(ITERATION_PROPERTIES *ip, double exp_eps);
/* Liefert Exponenten der variablen Genauigkeit des IP's */
double get_ip_exp_eps(ITERATION_PROPERTIES *ip);


/* Setzt Modus des IP's */
void set_ip_curve(ITERATION_PROPERTIES *ip, long curve);
/* Liefert Modus des IP's */
long get_ip_curve(ITERATION_PROPERTIES *ip);
/* Setzt Modus des IP's */
void set_ip_index(ITERATION_PROPERTIES *ip, int index);
/* Liefert Modus des IP's */
int get_ip_index(ITERATION_PROPERTIES *ip);
/* Setzt Modus des IP's */
void set_ip_mode(ITERATION_PROPERTIES *ip, long mode);
/* Liefert Modus des IP's */
long get_ip_mode(ITERATION_PROPERTIES *ip);
/* Setzt Anzahl der Werte des IP's */
extern void set_ip_count_of_values(ITERATION_PROPERTIES *ip, long count_of_values);
/* Liefert Anzahl der Werte des IP's */
extern long get_ip_count_of_values(ITERATION_PROPERTIES *ip);
/* Setzt Anfangsknoten des IC's */
extern void set_ip_begin_node(ITERATION_PROPERTIES *ic, long begin_node);
/* Liefert Anfangsknoten des IC's */
extern long get_ip_begin_node(ITERATION_PROPERTIES *ic);
/* Setzt Endknoten des IC's */
extern void set_ip_end_node(ITERATION_PROPERTIES *ic, long end_node);
/* Liefert Endknoten des IC's */
long get_ip_end_node(ITERATION_PROPERTIES *ic);
/* Setzt Schrittweite der Zwischenknoten des IC's */
void set_ip_step_nodes(ITERATION_PROPERTIES *ic, long step_nodes);
/* Liefert Schrittweite der Zwischenknoten des IC's */
long get_ip_step_nodes(ITERATION_PROPERTIES *ic);
/* Setzt Anfangsknoten des IP's */
extern void set_ip_begin_element(ITERATION_PROPERTIES *ip, long begin_element);
/* Liefert Anfangsknoten des IP's */
extern long get_ip_begin_element(ITERATION_PROPERTIES *ip);
/* Setzt Endknoten des IP's */
extern void set_ip_end_element(ITERATION_PROPERTIES *ip, long end_element);
/* Liefert Endknoten des IP's */
long get_ip_end_element(ITERATION_PROPERTIES *ip);
/* Setzt Schrittweite der Zwischenknoten des IP's */
void set_ip_step_elements(ITERATION_PROPERTIES *ip, long step_elements);
/* Liefert Schrittweite der Zwischenknoten des IP's */
long get_ip_step_elements(ITERATION_PROPERTIES *ip);
/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IP's */
void set_ip_value(ITERATION_PROPERTIES *ip, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IP's */
double get_ip_value(ITERATION_PROPERTIES *ip, long index);
/* Setzt Verteilungstyp des IP's */
void set_ip_distribution_type(ITERATION_PROPERTIES *ip, long distribution_type);
/* Liefert Verteilungstyp des IP's */
long get_ip_distribution_type(ITERATION_PROPERTIES *ip);
/* Setzt X-Koordinate mit dem Index (index) des IP's */
void set_ip_x(ITERATION_PROPERTIES *ip, long index, double xcoor);
/* Liefert X-Koordinate mit dem Index (index) des IP's */
double get_ip_x(ITERATION_PROPERTIES *ip, long index);
/* Setzt Y-Koordinate mit dem Index (index) des IP's */
void set_ip_y(ITERATION_PROPERTIES *ip, long index, double ycoor);
/* Liefert Y-Koordinate mit dem Index (index) des IP's */
double get_ip_y(ITERATION_PROPERTIES *ip, long index);
/* Setzt Z-Koordinate mit dem Index (index) des IP's */
void set_ip_z(ITERATION_PROPERTIES *ip, long index, double zcoor);
/* Liefert Z-Koordinate mit dem Index (index) des IP's */
double get_ip_z(ITERATION_PROPERTIES *ip, long index);
/* Setzt Verteilungsradius IP's */
void set_ip_radius(ITERATION_PROPERTIES *ip, double radius);
/* Liefert Verteilungsradius des IP's */
double get_ip_radius(ITERATION_PROPERTIES *ip);
/* Setzt Level des IP's */
void set_ip_level(ITERATION_PROPERTIES *ip, long level);
/* Liefert Level des IP's */
long get_ip_level(ITERATION_PROPERTIES *ip);
/* Setzt Traecernummer IP's */
void set_ip_component_number(ITERATION_PROPERTIES *ip, int component_number);
/* Liefert Traecernummer des IP's */
int get_ip_component_number(ITERATION_PROPERTIES *ip);
/* Setzt Knotennummer mit dem Index (index) des IP's */
void set_ip_nodes(ITERATION_PROPERTIES *ip, long index, long node);
/* Liefert Knotennummer mit dem Index (index) des IP's */
long get_ip_nodes(ITERATION_PROPERTIES *ip, long index);



typedef struct {
    char *name;
    LList *ip_list;
    long count_of_iteration_properties;
    char **names_of_iteration_properties;
    long count_of_iteration_properties_name;
    double *ip_vector;
} LIST_ITERATION_PROPERTIES;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_ITERATION_PROPERTIES *create_iteration_properties_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_iteration_properties_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int iteration_properties_list_empty(void);

/* Bekanntmachung der Iteration mit dem Name name. */
extern long declare_iteration_properties_name (char *name);
/*   Alle Iterationen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_iteration_properties_names (void);

/* Fuegt eine Instanz vom Typ ITERATION_PROPERTIES in die Liste */
extern long insert_iteration_properties_list (ITERATION_PROPERTIES *ip);

/* Prueft auf Existenz eines Namens */
long iteration_properties_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateIterationPropertiesList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyIterationPropertiesList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int IterationPropertiesListEmpty(void);
  /* Spaltenindex der IP-Objekte mit dem Namen (name) */
long IndexIterationProperties(char *name);


/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern ITERATION_PROPERTIES *GetIterationPropertiesGroup(char *name,ITERATION_PROPERTIES *ip);
  /* Zerstoert saemtliche ITERATION_PROPERTIES-Objekte der Gruppe NAME */
extern int DestroyIterationPropertiesListGroup(char *name);


/* Methoden ausfuehren */

/* Setzt einen Knoten-Iteration (Eingabe in Knotennummer) */
int ExecuteIterationPropertiesMethod0 (ITERATION_PROPERTIES *ip, double *current, double *last, long dim, double *cur_eps);
/*void ExecuteIterationPropertiesMethod0 (ITERATION_PROPERTIES *ip);*/
/* Setzt einen Knoten-Iteration (Eingabe in x, y und z) */
int ExecuteIterationPropertiesMethod1 (ITERATION_PROPERTIES *ip,
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps);
int ExecuteIterationPropertiesMethod2 (ITERATION_PROPERTIES *ip,
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps);


/* IP-Modell */

/* Alle IP-Objekte mit dem Namen (name) konfigurieren */
void SetIterationProperties(char *name);
  /* Ganze Iterationstabelle initialisieren */
void InitAllIterationProperties (void);
  /* Iterationstabellenspalte initialisieren */
void InitIterationProperties (char *name);
/* Iterationen mit dem Namen name ausfuehren */
int ExecuteIterationProperties (char *name, double *current, double *last,
                                double *ref, long dim);

/* Liefert Anzahl der ausgefuehrten Iterationen */
long GetIterationNumberIterationProperties (char *name);
/* Liefert die Konvergenzrate der ausgefuehrten Iteration */
double GetConvergenceRateIterationProperties (char *name);

/* Ermitteln des Fehlers bei Iterationen */
extern double CalcIterationError(double *, double *, double *, long, int);
extern double CalcIterationErrorVectorAgainstNodes(double *new_iteration, int old_iteration_ndx, int reference_ndx, int method);
extern double CalcIterationErrorVectorAgainstNodesV2(int,int,int,int);

/* Lesefunktionen */
extern int FctIterationProperties ( char *data, int found, FILE *f );
extern int FctIterationPropertiesPressure( char *data, int found, FILE * f);
extern int FctIterationPropertiesSaturation(char *data, int found, FILE * f);
extern int FctIterationPropertiesConcentration(char *data, int found, FILE * f);
extern int FctIterationPropertiesSorbedConcentration(char *data, int found, FILE * f);
extern int FctIterationPropertiesPressureNew( char *data, int found, FILE * f);
extern int FctIterationPropertiesSaturationNew(char *data, int found, FILE * f);
extern int FctIterationPropertiesConcentrationNew(char *data, int found, FILE * f);
extern int FctIterationPropertiesSorbedConcentrationNew(char *data, int found, FILE * f);
extern int FctIterationPropertiesImmobileSolutedConcentrationNew ( char *data,int found, FILE *f );


#endif

