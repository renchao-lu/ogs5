/**************************************************************************/
/* ROCKFLOW - Modul: rf_mg.h
                                                                          */
/* Aufgabe:
   Datenstrukturen und Definitionen des Datentyps MESH_GENERATION
   (Netzgenerierung / Netzverknuepfung).
                                                                          */
/**************************************************************************/


#ifndef rf_mg_INC

#define rf_mg_INC
  /* Schutz gegen mehrfaches Einfuegen */

#include "rfadt.h"

/* Andere oeffentlich benutzte Module */
typedef struct {
    char *name;
    long  type;
    char *file_name;
} MESH_GENERATION;

/*------------------------------------------------------------------------*/
/* List member construction */

/* Erzeugt eine Instanz vom Typ MESH_GENERATION */
extern MESH_GENERATION *create_mesh_generation(char *name);
/* Zerstoert die Instanz vom Typ MESH_GENERATION */
extern void destroy_mesh_generation(void *member);
/* Initialisierung der Instanz vom Typ MESH_GENERATION */
extern MESH_GENERATION *init_mesh_generation(MESH_GENERATION *sosi,long count_of_values);
/* Loescht die Instanz vom Typ SOURCE_SINK */
extern MESH_GENERATION *delete_mesh_generation(MESH_GENERATION *sosi);


typedef struct {
        char *name;
        List *mg_list;
        long count_of_mesh_generations;
        char **names_of_mesh_generations;
        long count_of_mesh_generations_name;
} LIST_MESH_GENERATION;

/* Erzeugt leere bennante Liste von Anfangszustaende */
extern LIST_MESH_GENERATION *create_mesh_generation_list (char *name);
/* Entfernt komplette Liste aus dem Speicher */
extern void destroy_mesh_generation_list(void);


/* Bekanntmachung der Randbedingung mit dem Name name. */
extern long declare_mesh_generation_name (char *name);

/* Setzt Typ des MG's */
void set_mg_type(MESH_GENERATION *mg, long type);
/* Liefert Typ des MG's */
long get_mg_type(MESH_GENERATION *mg);
/* Setzt Daeinamen des MG's */
extern void set_mg_file_name(MESH_GENERATION *mg, char *file_name);
/* Liefert Dateinamen des MG's */
extern char *get_mg_file_name(MESH_GENERATION *mg);

/* ================================================================ */
/* Interface */
/* ================================================================ */

/* Erzeugt leere unbenannte Liste von Anfangszustaende */
extern void CreateMeshGenerationList();
/* Zerstoert unbenannte Liste von Anfangszustaende */
extern void DestroyMeshGenerationList();
/* Gibt Auskunft ob die Liste leer oder nicht ist */
int MeshGenerationListEmpty(void);


/* Lesefunktion */
int FctMeshGeneration ( char *data, int found, FILE *f );


/********** omega.h *****************************************************
***************************************************************************/

/***  omega.h               *****************************************************/
/**********************************************************************************/
                 

/***             Konstanten                         ***/
/***                                                ***/
/** Anzahl Element pro Knoten Standard ist 8 im 3D   **/
#define DEF_no_of_el_per_node_3d 64

/* Mesh - Generator - Version */
#define MG_VERSION  "OMEGA 1.0.02"
#define MG_DATE     "11.04.2003"
#define MG_NAME     "Open MEsh Generation and Adaptation Tool"        


/*** Netzdatenstrukturen ***/

typedef struct
{
   double x;
   double y;
   double z;
} NODE;

typedef struct
{
   long material_group;  /* Materialgruppe */
   long numerics_group;  /* neue (>3.6.03) Kennzeichnung von Elementtypen (z.B. Undurchlässig für Strömung) */
   long node[4];
} ELEMENT2D;

typedef struct
{
   long material_group;  /* Materialgruppe */
   long numerics_group;  /* neue (>3.6.03) Kennzeichnung von Elementtypen (z.B. Undurchlässig für Strömung) */
   long node[8];
} ELEMENT3D;

typedef struct
{
   long no_of_elements; 
   long elem[4];
} NODEELEMENTS2D;

typedef struct
{
   long no_of_elements; 
   long elem[DEF_no_of_el_per_node_3d];
} NODEELEMENTS3D;


typedef struct
{
   long no_of_nodes;
   long no_of_elements;
   NODE *node;
   ELEMENT2D *elem;
   NODEELEMENTS2D *node_elem;
} MESH2D;

typedef struct
{
   long no_of_nodes;
   long no_of_elements;
   NODE *node;
   ELEMENT3D *elem;
   NODEELEMENTS3D *node_elem; /* Knoten-Element-Beziehungen: Anzahl, Elementnummern */
} MESH3D;

/***** end. omega.h *****************************************************
***************************************************************************/


/********** mg_lib.h ****************************************************
***************************************************************************/

/***  mg_lib.h              *****************************************************/
/***  Mesh Generator Library  *****************************************************/
/**********************************************************************************/
                                                                                  

  extern MESH2D *OmegaPlateHoleQuarter (long material_group_circle, long material_group_rect,
               long plane, long part, 
               double x0, double y0, double x1, double y1, 
               double radius, long no_rings, long no_sectors); 
  extern MESH2D *OmegaHoleRingPlateQuarter (long material_group_circle, long material_group_ring, long material_group_rect,
               long plane, long part,
               double x0, double y0, double x1, double y1, 
               double radius_circle, double radius_ring, 
               long no_rings_ring, long no_rings_rect, long no_sectors, 
               int sort_bound_nodes); 
  extern MESH2D *OmegaRectangle (char *file_name, long material_group);
  extern MESH3D *OmegaExpand2DTo3D (char *file_name, long material_group, MESH2D *mesh_2d);
  extern MESH3D *OmegaRotate2DTo3D (char *file_name, char *path_name, long material_group, MESH2D *mesh_2d);
  extern void OmegaDelEqualNodes3D(MESH3D *mesh);   

  extern void OmegaOut_rfi2D (char *file_name, char *path_name, MESH2D *mesh_2d);
  extern void OmegaOut_rfi3D (char *file_name, char *path_name, MESH3D *mesh_3d);
  extern MESH2D *OmegaBuildNodeElRel2D(MESH2D *mesh_2d);
  extern MESH3D *OmegaBuildNodeElRel3D(MESH3D *mesh_3d);
  extern MESH2D *OmegaMeshCombine2D (MESH2D *mesh_1,MESH2D *mesh_2);
  extern MESH3D *OmegaMeshCombine3D (MESH3D *mesh_1,MESH3D *mesh_2);

  extern void OmegaMinNodeDistance2D(MESH2D *mesh_2d);
  extern void OmegaMinNodeDistance3D(MESH3D *mesh_3d);
  extern void OmegaMinElSize2D(MESH2D *mesh_2d);
  extern void OmegaMinElSize3D(MESH3D *mesh_3d);

  extern void OmegaDisplaceNode(NODE *node0, NODE *vec);
  extern void OmegaRotateNode(NODE *node0, NODE *unit_axis, double alpha);
  extern void OmegaDisplaceMesh2D(MESH2D *mesh_2d, NODE *vec);
  extern void OmegaDisplaceMesh3D(MESH3D *mesh_3d, NODE *vec);
  extern void OmegaRotateMesh2D(MESH2D *mesh, NODE *vec, double alpha);
  extern void OmegaRotateMesh3D(MESH3D *mesh, NODE *vec, double alpha);
  extern MESH2D *OmegaDuplicateMesh2D(MESH2D *mesh);
  extern MESH3D *OmegaDuplicateMesh3D(MESH3D *mesh);

  double OmegaNodeDistance(NODE *node0, NODE *node1);

/***** end. mg_lib.h ****************************************************
***************************************************************************/


#endif

