
/* Modules */
#include <stdio.h>


/* Declarationen */
#define DELAUN3D_DLL "fdelaun3d.dll"
#define DLL_FUNC_NAME "TETGEN"
#define NODE_MAX 10000
#define ELEM_MAX 20000

typedef void (*TFUNC)(void*,void*,void*,void*,void*,void*,void*,void*,void*,void*);


/* functions /prototypes */
extern int ReadInputFile(char* filepath, int *node, double *x, double *y, double *z);
extern int WriteOutputFile(char* filepath, int *node, double *x, double *y, double *z, int *nelm, int *mtj, int *jac, int *maxelm);
extern void ExecuteDelaunay3D(char* infilepath, char* outfilepath);
extern int ExecuteDLL_Delaunay3D( int *node, double *x, double *y, double *z, int *nelm, int *mtj, int *jac, int *err,int *maxnode, int *maxelm );





