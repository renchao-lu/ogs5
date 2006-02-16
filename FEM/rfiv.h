/**************************************************************************/
/* ROCKFLOW - Modul: rfiv.h
                                                                          */
/* Aufgabe: Verwaltung und Bearbeitung der inversen Variablen
   - Datenstrukturen und Definitionen fuer den Datentyp INVERSE_VARIABLE
   - Verwaltung von INVERSE_VARIABLE mit dem Datentyp LIST_INVERSE_VARIABLE
   - Bereitstellung von Methoden fuer den Datentyp INVERSE_VARIABLE
   - Schnittstellenfunktion zu Rockflow

                                                                          */
/**************************************************************************/


#ifndef rfiv_INC

#define rfiv_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


typedef struct {
    char *name;
    char *group_name;
    long type;
    void *ptr;

    double min_value;
    double max_value;
    double start_value;
    double coeff_value;
    double eps_value;
    long max_iter;

    long kind;
    long curve;
    long count_of_values;
    double *values;

} INVERSE_VARIABLE;



/* Erzeugt eine Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *create_inverse_variable(char *name);
/* Zerstoert die Instanz vom Typ INVERSE_VARIABLE */
extern void destroy_inverse_variable(void *member);
/* Initialisierung der Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *init_inverse_variable(INVERSE_VARIABLE *iv, \
                                                                                        long count_of_values);
/* Loescht die Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *delete_inverse_variable(INVERSE_VARIABLE *iv);
/* Initialisierung der Koordinaten fuer eine Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *init_inverse_variable_coor(INVERSE_VARIABLE *iv, \
                                                                                        long count_of_points);
/* Loescht die Koordinaten fuer Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *delete_inverse_variable_coor(INVERSE_VARIABLE *iv);
/* Initialisierung die Punkte einer Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *init_inverse_variable_nodes(INVERSE_VARIABLE *iv, \
                                                                                        long count_of_points);
/* Loescht die Punkte einer Instanz vom Typ INVERSE_VARIABLE */
extern INVERSE_VARIABLE *delete_inverse_variable_nodes(INVERSE_VARIABLE *iv);

/* Setzt Name des IV's */
extern void set_iv_name(INVERSE_VARIABLE *iv, char *name);
/* Liefert Name des IV's */
extern char *get_iv_name(INVERSE_VARIABLE *iv);
/* Setzt Name des IV's */
extern void set_iv_group_name(INVERSE_VARIABLE *iv, char *group_name);
/* Liefert Name des IV's */
extern char *get_iv_group_name(INVERSE_VARIABLE *iv);
/* Setzt Typ des IV's */
void set_iv_type(INVERSE_VARIABLE *iv, long type);
/* Liefert Typ des IV's */
long get_iv_type(INVERSE_VARIABLE *iv);
/* Setzt Element-Zeiger des IV's */
extern int set_iv_ptr(INVERSE_VARIABLE *iv, void *ptr);
/* Liefert Element-Zeiger des IV's */
extern void *get_iv_ptr(INVERSE_VARIABLE *iv);
/* Setzt Anfangswert des IV's */
extern void set_iv_start_value(INVERSE_VARIABLE *iv, double start_value);
/* Liefert Anfangswert des IV's */
extern double get_iv_start_value(INVERSE_VARIABLE *iv);
/* Setzt Minimumswert des IV's */
extern void set_iv_min_value(INVERSE_VARIABLE *iv, double min_value);
/* Liefert Minimumswert Knoten des IV's */
extern double get_iv_min_value(INVERSE_VARIABLE *iv);
/* Setzt Maximumswert des IV's */
extern void set_iv_max_value(INVERSE_VARIABLE *iv, double max_value);
/* Liefert Maximumswert des IV's */
extern double get_iv_max_value(INVERSE_VARIABLE *iv);
/* Setzt Gewichtswert des IV's */
extern void set_iv_coeff_value(INVERSE_VARIABLE *iv, double coeff_value);
/* Liefert Gewichtswert des IV's */
extern double get_iv_coeff_value(INVERSE_VARIABLE *iv);
/* Setzt Abbruchschranke des IV's */
extern void set_iv_eps_value(INVERSE_VARIABLE *iv, double eps_value);
/* Liefert Abbruchschranke des IV's */
extern double get_iv_eps_value(INVERSE_VARIABLE *iv);
/* Setzt maximale Iteration der IV. */
extern void set_iv_max_iter(INVERSE_VARIABLE *iv, long max_iter);
/* Liefert maximale Iteration der IV. */
extern long get_iv_max_iter(INVERSE_VARIABLE *iv);

/* Setzt Art der IV. */
void set_iv_kind(INVERSE_VARIABLE *iv, int kind);
/* Liefert Art der IV. */
int get_iv_kind(INVERSE_VARIABLE *iv);
/* Setzt Kurve des IV's */
void set_iv_curve(INVERSE_VARIABLE *iv, int curve);
/* Liefert Kurve des IV's */
int get_iv_curve(INVERSE_VARIABLE *iv);


/* Setzt Anzahl der Werte des IV's */
extern void set_iv_count_of_values(INVERSE_VARIABLE *iv, long count_of_values);
/* Liefert Anzahl der Werte des IV's */
extern long get_iv_count_of_values(INVERSE_VARIABLE *iv);
/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IV's */
void set_iv_value(INVERSE_VARIABLE *iv, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IV's */
double get_iv_value(INVERSE_VARIABLE *iv, long index);


typedef struct {
    char *name;
    List *iv_list;
    long count_of_inverse_variable;
    char **names_of_inverse_variable;
    long count_of_inverse_variable_name;
    double *iv_vector;
} LIST_INVERSE_VARIABLE;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_INVERSE_VARIABLE *create_inverse_variable_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_inverse_variable_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int inverse_variable_list_empty(void);

/* Bekanntmachung der Randbedingung mit dem Name name. */
extern long declare_inverse_variable_name (char *name);
/*   Alle inverse Variable von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_inverse_variable_names (void);

/* Fuegt eine Instanz vom Typ INVERSE_VARIABLE in die Liste */
extern long insert_inverse_variable_list (INVERSE_VARIABLE *iv);

/* Prueft auf Existenz eines Namens */
long inverse_variable_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateInverseVariableList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyInverseVariableList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int InverseVariableListEmpty(void);
  /* Ganze Randbedingungstabelle initialisieren */
void InitAllInverseVariable (void);

/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern INVERSE_VARIABLE *GetInverseVariableGroup(char *name,INVERSE_VARIABLE *iv);
  /* Randbedingungstabellenspalte initialisieren */
void InitInverseVariable (char *name);
  /* Spaltenindex der IV-Objekte mit dem Namen (name) */
long IndexInverseVariable(char *name);
  /* Zerstoert saemtliche INVERSE_VARIABLE-Objekte der Gruppe NAME */
extern int DestroyInverseVariableListGroup(char *name);


/* IV-Modell */

/* IV-Objekt mit dem Namen (name) setzen */
void SetInverseVariable(char *name, char *group_name,  int type, void *item);


/* IV-Abfragen */

/* Liefert die Anzahl der gesetzten Namen fuer die IV. */
long GetInverseVariableNamesNumber (void);
/* Liefert einen gesetzten Namen fuer die IV. */
char *GetInverseVariableName (long i);



/* Inverse Variable bearbeiten */
void InitAllInverseVariable(void);
void SetAllStartInverseVariable(void);
void SetAllInverseVariable(void);
void SetAllMinInverseVariable(void);
void SetAllMaxInverseVariable(void);

int SetCurrentInverseVariable(char *name, double current_value);
int GetCurrentInverseVariable(char *name, double *current_value);

int SetStartInverseVariable(char *name, double start_value);
int GetStartInverseVariable(char *name, double *start_value);
int SetMinInverseVariable(char *name, double min_value);
int GetMinInverseVariable(char *name, double *min_value);
int SetMaxInverseVariable(char *name, double max_value);
int GetMaxInverseVariable(char *name, double *max_value);
int GetCoeffInverseVariable(char *name, double *coeff);
int GetEpsInverseVariable(char *name, double *eps);
int GetMaxiterInverseVariable(char *name, long *max_iter);


void PrintAllStartInverseVariable(char *db_name);
void PrintStartInverseVariable(char *name, char *db_name);
void PrintStartInverseText(char *db_name);
void PrintStartInverseIteration(long i, char *db_name);
void PrintStartInverseLSE(double lse, char *db_name);
void PrintCycleInverseIteration(long i, char *db_name);
void PrintNameInverseVariable(char *name, char *db_name);
void PrintStartInverseLF(char *db_name);


/* Lesefunktionen */
extern int FctInverseVariableNew ( char *data, int found, FILE *f );

/* Schluesselwort */
extern char *keyword_inverse_variable;


#endif


