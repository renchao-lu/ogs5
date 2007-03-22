// delaunay3D.cpp : Okayama 2006 3D Tetgen.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include "delaunay3D.h"

/**************************************************************************/
/* MSH - Function: Start_Delaunay3D
                                                                          */
/* Task: Independent start on a console application, which
   is the former main-function from a stand-alone application.
   If necessary, this should be modified for a call within the GeoSys-Console.

   So far, ExecuteDelaunay3D will be called directly.

/* History:
   12/2006     NW        1st Coding
   12/2007     TK        Implementation
**************************************************************************/
int Start_Delaunay3D (int argc, char** argv)
{
	char infilepath[128];
	char outfilepath[128];
	if (argc == 1) {
		printf("INPUT FILE=?\n");
		scanf("%s",infilepath);
	} else {
		strcpy(infilepath, argv[1]);
	}
	if (argc < 3) {
		printf("OUTPUT FILE=?\n");
		scanf("%s",outfilepath);
	} else {
		strcpy(outfilepath, argv[2]);
	}
    ExecuteDelaunay3D(infilepath,outfilepath);
    return 0;
}
/**************************************************************************/
/* MSH - Function: ReadInputFile
                                                                          */
/* Task: Read Function for special input files

TK: a read function of GLI-files should be added
    or better: a direct access to Geosys data structures

/* History:
   12/2006     NW        1st Coding
   12/2007     TK        Implementation
**************************************************************************/
int ReadInputFile(char* filepath, int *node, double *x, double *y, double *z)
{
	int i=0;
	FILE *fp;
	fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("ERROR: INPUT FILE OPEN ERROR - %s\n",filepath);
        return 0;
	}
	if (fscanf(fp, "%d", node) != EOF) {
		for (i=0; i<*node;i++) {
			x[i] = 0.0;
			y[i] = 0.0;
			z[i] = 0.0;
		}
		for (i=0; i<*node;i++) {
			if (fscanf(fp,"%lf %lf %lf",&x[i],&y[i],&z[i]) == EOF) {
				break;
			}
		}	
	}
	fclose(fp);
	printf("\n[INPUT DATA]\n");
	printf("NUMBER OF NODE = %d\n",*node);
	printf("\n");
	return -1;
}
/**************************************************************************/
/* MSH - Function: WriteOutputFile
                                                                          */
/* Task: Write Function for special output files

TK: a write function of a MSH files is not needed, here
     a direct access and fill-up of the Geosys data structures
	 is required. Intermediate Solution: Writing a GeoSys MSH file

	 
/* History:
   12/2006     NW        1st Coding
   12/2007     TK        Implementation
   12/2007     TK        Modification for Geosys MSH file
**************************************************************************/
int WriteOutputFile(char* filepath, int *node, double *x, double *y, double *z, int *nelm, int *mtj, int *jac, int *maxelm)
{
	FILE *msh_file;
	int i = 0;
	jac=jac;
	msh_file = fopen(filepath, "w");
	if (msh_file == NULL) {
		printf("ERROR: OUTPUT FILE OPEN ERROR - %s\n",filepath);
        return 0;
	}
    //write MSH Head
	   	fprintf( msh_file, "%s\n", "#FEM_MSH");
    //write PCS Type
        fprintf( msh_file, "%s\n", " $PCS_TYPE");
	   	fprintf( msh_file, "%s\n", "  NO_PCS");
    //Write Nodes
	   	fprintf( msh_file, "%s\n", " $NODES");
        fprintf(msh_file,"% ld\n",*node);
    //write Geometry    
	for (i=0; i<*node; i++) {
        fprintf( msh_file, "%d ", i);
		fprintf(msh_file, "%15.7lf%15.7lf%15.7lf\n",x[i],y[i],z[i]);
	}
    //Write Elements
	   	fprintf( msh_file, "%s\n", " $ELEMENTS");
		fprintf(msh_file,"% ld\n",*nelm);
    //write Topology
    for (i=0; i<*nelm; i++) {
        fprintf( msh_file, "%d ", i);
        fprintf( msh_file, " 0 -1 tet ");     
    	fprintf(msh_file, "%d %d %d %d\n",*(mtj+i)-1,*(mtj+*maxelm+i)-1,*(mtj+(*maxelm)*2+i)-1,*(mtj+(*maxelm)*3+i)-1);	
		/*
		//Prepared for Neighbourhood write out but no read function for this yet
		// There is a neighbourhood data structrure filled when reading the MSH file
		fprintf(msh_file, "%7d%7d%7d%7d%7d%7d%7d%7d\n",
			*(mtj+i)-1,*(mtj+*maxelm+i)-1,*(mtj+(*maxelm)*2+i)-1,*(mtj+(*maxelm)*3+i)-1,
			*(jac+i)-1,*(jac+*maxelm+i)-1,*(jac+(*maxelm)*2+i)-1,*(jac+(*maxelm)*3+i)-1);
	    */
	}
    //Write File Terminator
	fprintf( msh_file, "%s\n", "#STOP");
	
	fclose(msh_file);
	return -1;
}

/**************************************************************************/
/* MSH - Function: ExecuteDelaunay3D
                                                                          */
/* Task: Starts and Executes the 3D Delaunay mesh generator   
   char* infilepath:  pointer of input path
   char* outfilepath:  pointer of input path
/* History:
   12/2006     NW        1st Coding
   12/2007     TK        Implementation
**************************************************************************/
void ExecuteDelaunay3D(char* infilepath, char* outfilepath)
{
	int node = 0;
	double *x = (double*)malloc(sizeof(double)*(NODE_MAX+4));
	double *y = (double*)malloc(sizeof(double)*(NODE_MAX+4));
	double *z = (double*)malloc(sizeof(double)*(NODE_MAX+4));
	int nelm = 0;
	int *mtj = (int*)malloc(sizeof(int)*(ELEM_MAX*4));
	int *jac = (int*)malloc(sizeof(int)*(ELEM_MAX*4));
	int err = 0;
	int maxelm = ELEM_MAX;
	int maxnode = NODE_MAX;
	printf("\n[AVAILABLE MEMORY]\n");
	printf("X: %x\nY: %x\nZ: %x\n", x, y, z);
	printf("MTJ: %x\nJAC: %x\n\n", mtj, jac);
	if (x==NULL || y==NULL || z==NULL || mtj==NULL || jac==NULL) {
		printf("ERROR: Fail to malloc()");
	}
	if(ReadInputFile(infilepath, &node, x, y, z)) {
		if (ExecuteDLL_Delaunay3D(&node,x,y,z,&nelm,mtj,jac,&err,&maxnode,&maxelm)) {
			if (err == 0) {
				WriteOutputFile(outfilepath, &node,x,y,z,&nelm,mtj,jac,&maxelm);
			}
		}
	} else {
		printf("ERROR: input()\n");
	}
	free(x);
	free(y);
	free(z);
	free(mtj);
	free(jac);
}
/**************************************************************************/
/* MSH - Function: ExecuteDLL_Delaunay3D
                                                                          */
/* Task: Calls and Executes the 3D Okayama DLL (Fortran based)
	
	int *node:		??
	double *x *y *z:??
	int *nelm:		??
	int *mtj:		??
	int *jac:		??
	int *err:		??
	int *maxnode:	??
	int *maxelm:	??
/* History:
   12/2006     NW        1st Coding
   12/2007     TK        Implementation
**************************************************************************/
int ExecuteDLL_Delaunay3D(int *node, double *x, double *y, double *z, int *nelm, int *mtj, int *jac, int *err, int *maxnode, int *maxelm)
{
    HINSTANCE   hInstDLL;
    TFUNC       DllFunction;
	hInstDLL=LoadLibrary(DELAUN3D_DLL);
	printf("\n[DLL LOAD]\n",hInstDLL);
	printf("DLL NAME: %s\n",DELAUN3D_DLL);
	printf("FUNCTION NAME: %s\n",DLL_FUNC_NAME);
	printf("Lib: %x\n",hInstDLL);
    if (hInstDLL==NULL) {
		printf("ERROR: LoadLibrary()\n");
        return 0;
    }
    DllFunction=(TFUNC)GetProcAddress(hInstDLL, DLL_FUNC_NAME);
	printf("Func: %x\n",DllFunction);
    if (DllFunction != NULL) {
		printf("\n-EXECUTE ""%s""()\n",DLL_FUNC_NAME);
		DllFunction(err,node,x,y,z,nelm,mtj,jac,maxnode,maxelm);	
	    printf("\n[RESULT]\n");
	    printf("NELM = %d\n",*nelm);
	    printf("ERR = %d\n\n",*err);
	} else {
		printf("ERROR: GetProcAddress()\n");
	}

    if (!FreeLibrary(hInstDLL)) {
		printf("ERROR: FreeLibrary()\n");
        return 0;
     }    
	return -1;
}
