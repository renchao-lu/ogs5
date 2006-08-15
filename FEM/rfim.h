/**************************************************************************/
/* ROCKFLOW - Modul: rfim.h
                                                                          */
/* Aufgabe: Inverse Methoden (Algorithmen)
   - Datenstrukturen und Definitionen fuer den Datentyp INVERSE_METHOD
   - Verwaltung von INVERSE_METHOD mit dem Datentyp LIST_INVERSE_METHOD
   - Bereitstellung von Methoden fuer den Datentyp INVERSE_METHOD
   - Schnittstellenfunktion zu Rockflow-Modelle

                                                                          */
/**************************************************************************/


#ifndef rfim_INC

#define rfim_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Prototypen */
typedef int (*TimeLoopFunction) (void);


/* Andere oeffentlich benutzte Module */
#include "rfadt.h"


typedef struct {
    char *name;
    char *group_name;
    long type;

    double lambda;
    long   second_maxiter;
    double start_lambda;
    double lambda_abs_eps;
    double lambda_fac1;
    double lambda_fac2;


    TimeLoopFunction time_loop_function;  /* Zeit-Funktion */


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
    long curve;

    long count_of_values;
    double *values;
} INVERSE_METHOD;



/* Erzeugt eine Instanz vom Typ INVERSE_METHOD */
extern INVERSE_METHOD *create_inverse_method(char *name);
/* Zerstoert die Instanz vom Typ INVERSE_METHOD */
extern void destroy_inverse_method(void *member);
/* Initialisierung der Instanz vom Typ INVERSE_METHOD */
extern INVERSE_METHOD *init_inverse_method(INVERSE_METHOD *im, \
                                                                                        long count_of_values);
/* Loescht die Instanz vom Typ INVERSE_METHOD */
extern INVERSE_METHOD *delete_inverse_method(INVERSE_METHOD *im);

/* Setzt Name des IM's */
extern void set_im_name(INVERSE_METHOD *im, char *name);
/* Liefert Name des IM's */
extern char *get_im_name(INVERSE_METHOD *im);
/* Setzt Name des IM's */
extern void set_im_group_name(INVERSE_METHOD *im, char *group_name);
/* Liefert Name des IM's */
extern char *get_im_group_name(INVERSE_METHOD *im);
/* Setzt Typ des IM's */
void set_im_type(INVERSE_METHOD *im, long type);
/* Liefert Typ des IM's */
long get_im_type(INVERSE_METHOD *im);

/* Setzt Zeitfunktion des IM's */
void set_im_time_loop_function(INVERSE_METHOD *im, TimeLoopFunction time_loop_function);
/* Liefert Zeitfunktion des IM's */
TimeLoopFunction get_im_time_loop_function(INVERSE_METHOD *im);


/* Setzt (Marquardt) maximale Iterationszahl des IM's */
void set_im_second_maxiter(INVERSE_METHOD *im, long second_maxiter);
/* Liefert maximale Iterationszahl des IM's */
long get_im_second_maxiter(INVERSE_METHOD *im);
/* Setzt Lambda-Startwert fuer IM's */
void set_im_start_lambda(INVERSE_METHOD *im, double start_lambda);
/* Liefert Lambda-Startwert fuer IM's */
double get_im_start_lambda(INVERSE_METHOD *im);
/* Setzt (Lambda) absolute Genauigkeit des IM's */
void set_im_lambda_abs_eps(INVERSE_METHOD *im, double lambda_abs_eps);
/* Liefert (Lambda) absolute Genauigkeit des IM's */
double get_im_lambda_abs_eps(INVERSE_METHOD *im);
/* Setzt (Lambda) Faktor 1 des IM's */
void set_im_lambda_fac1(INVERSE_METHOD *im, double lambda_fac1);
/* Liefert (Lambda) Faktor 1 des IM's */
double get_im_lambda_fac1(INVERSE_METHOD *im);
/* Setzt (Lambda) Faktor 2 des IM's */
void set_im_lambda_fac1(INVERSE_METHOD *im, double lambda_fac1);
/* Liefert (Lambda) Faktor 2 des IM's */
double get_im_lambda_fac1(INVERSE_METHOD *im);

/* Setzt Lambda fuer IM's */
void set_im_lambda(INVERSE_METHOD *im, double lambda);
/* Liefert Lambda fuer IM's */
double get_im_lambda(INVERSE_METHOD *im);

/* Setzt maximale Iterationszahl des IM's */
void set_im_maxiter(INVERSE_METHOD *im, long maxiter);
/* Liefert maximale Iterationszahl des IM's */
long get_im_maxiter(INVERSE_METHOD *im);
/* Setzt Kriterium (Konvergenztyp) des IM's */
void set_im_criterium(INVERSE_METHOD *im, long type);
/* Liefert Kriterium (Konvergenztyp) des IM's */
long get_im_criterium(INVERSE_METHOD *im);
/* Setzt Norm des IM's */
void set_im_norm(INVERSE_METHOD *im, int norm);
/* Liefert Norm des IM's */
int get_im_norm(INVERSE_METHOD *im);

/* Setzt absolute Genauigkeit des IM's */
void set_im_abs_eps(INVERSE_METHOD *im, double abs_eps);
/* Liefert absolute Genauigkeit des IM's */
double get_im_abs_eps(INVERSE_METHOD *im);
/* Setzt relative Genauigkeit des IM's */
void set_im_rel_eps(INVERSE_METHOD *im, double rel_eps);
/* Liefert relative Genauigkeit des IM's */
double get_im_rel_eps(INVERSE_METHOD *im);


/* Setzt die variable Genauigkeit des IM's */
void set_im_var_eps(INVERSE_METHOD *im, double var_eps);
/* Liefert variable Genauigkeit des IM's */
double get_im_var_eps(INVERSE_METHOD *im);
/* Setzt Anfangsgenauigkeit des IM's */
void set_im_begin_eps(INVERSE_METHOD *im, double begin_eps);
/* Liefert Anfangsgenauigkeit des IM's */
double get_im_begin_eps(INVERSE_METHOD *im);
/* Setzt Endgenauigkeit des IM's */
void set_im_end_eps(INVERSE_METHOD *im, double end_eps);
/* Liefert Endgenauigkeit des IM's */
double get_im_end_eps(INVERSE_METHOD *im);
/* Setzt Faktor der variablen Genauigkeit des IM's */
void set_im_fac_eps(INVERSE_METHOD *im, double fac_eps);
/* Liefert Faktor der variablen Genauigkeit des IM's */
double get_im_fac_eps(INVERSE_METHOD *im);
/* Setzt Exponenten der variablen Genauigkeit des IM's */
void set_im_exp_eps(INVERSE_METHOD *im, double exp_eps);
/* Liefert Exponenten der variablen Genauigkeit des IM's */
double get_im_exp_eps(INVERSE_METHOD *im);


/* Setzt Art der IM. */
void set_im_kind(INVERSE_METHOD *im, long kind);
/* Liefert Art der IM. */
long get_im_kind(INVERSE_METHOD *im);
/* Setzt Modus des IM's */
void set_im_curve(INVERSE_METHOD *im, long curve);
/* Liefert Modus des IM's */
long get_im_curve(INVERSE_METHOD *im);


/* Setzt Anzahl der Werte des IM's */
extern void set_im_count_of_values(INVERSE_METHOD *im, long count_of_values);
/* Liefert Anzahl der Werte des IM's */
extern long get_im_count_of_values(INVERSE_METHOD *im);

/* Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IM's */
void set_im_value(INVERSE_METHOD *im, long index, double value);
/* Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IM's */
double get_im_value(INVERSE_METHOD *im, long index);

/* Setzt Verteilungsradius IM's */
void set_im_radius(INVERSE_METHOD *im, double radius);
/* Liefert Verteilungsradius des IM's */
double get_im_radius(INVERSE_METHOD *im);



typedef struct {
    char *name;
    LList *im_list;
    long count_of_inverse_method;
    char **names_of_inverse_method;
    long count_of_inverse_method_name;
    double *im_vector;
} LIST_INVERSE_METHOD;


/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_INVERSE_METHOD *create_inverse_method_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_inverse_method_list(void);
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int inverse_method_list_empty(void);

/* Bekanntmachung der inversen Methode mit dem Name name. */
extern long declare_inverse_method_name (char *name);
/*   Alle inversen Methoden von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt */
extern long undeclare_inverse_method_names (void);

/* Fuegt eine Instanz vom Typ INVERSE_METHOD in die Liste */
extern long insert_inverse_method_list (INVERSE_METHOD *im);

/* Prueft auf Existenz eines Namens */
long inverse_method_name_exist (char *name);


/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Liste */
  /* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateInverseMethodList();
  /* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyInverseMethodList();
  /* Gibt Auskunft ob die Liste leer oder nicht ist */
int InverseMethodListEmpty(void);

/* Konfiguration */
extern void ConfigInverseMethod(void);

/* Items */
  /* Liefert Zeiger auf Liste "name" */
extern INVERSE_METHOD *GetInverseMethodGroup(char *name,INVERSE_METHOD *im);
  /* Zerstoert saemtliche INVERSE_METHOD-Objekte der Gruppe NAME */
extern int DestroyInverseMethodListGroup(char *name);


/* ============= Methoden ausfuehren ============= */

/* Vorwärtsrechnen mit den Start-Variablen */
void ExecuteInverseMethodMethod0 (INVERSE_METHOD *im);

/*
                Line search methods
                method=0: none
                method=1: Bracket method (Two point search method)  
                method=2: Bracket method (Three point search method)  
          ==>   method=3: Golden section search method  
                method=4: Fibonacci section search method  
          ==>   method=5: Quadratic interpolation method  

                Gradient search methods
                method=6: steepest descent method  
                method=7: Conjugate gradient method  
                        Quasi-Newton methods
                method=8: DFP method (Davidon, Fletcher and Powell) 
                method=9: BFGS method (Broyton, Fletcher, Glodfarb and Shanno) 
                
                Newton methods
           ==>  method=10: Gauss-Newton method  
           ==>  method=11: Gauss-Newton-Levenberg-Marquardt method  
*/

void ExecuteInverseMethodMethod1 (INVERSE_METHOD *im);
void ExecuteInverseMethodMethod2 (INVERSE_METHOD *im);
void ExecuteInverseMethodMethod3 (INVERSE_METHOD *im);
void ExecuteInverseMethodMethod4 (INVERSE_METHOD *im);
void ExecuteInverseMethodMethod5 (INVERSE_METHOD *im);
int InverseMethod5 (double a, double fa, double b, double fb, double c, double fc, 
                     double *d);
void ExecuteInverseMethodMethod10 (INVERSE_METHOD *im);
void ExecuteInverseMethodMethod11 (INVERSE_METHOD *im);



/* IM-Modell */

/* Inverses Algorithmus mit dem Namen name ausfuehren */
int ExecuteInverseMethodNew (char *name, TimeLoopFunction time_loop_function);


/* Forward Simulation */
void ExecuteForwardSimulation (INVERSE_METHOD *im);

/* Lesefunktionen */
extern int FctInverseMethod ( char *data, int found, FILE *f );

/* Schluesselwort */
extern char *keyword_inverse_method;


#endif


