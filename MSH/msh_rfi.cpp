/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C++
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_elements_rfi.h"
#include "msh_gen_pris.h"

#include "msh_lib.h"
#include "msh_mesh.h"

// PCSLib
#include "gs_project.h"
#include "geo_strings.h"
#include "matrix_class.h"
using  Math_Group::vec;

/**************************************************************************
 MSHLIB - Funktion: Get_Rockflow_RFI
                                                                          
 Aufgabe: liest die alte Rockflow-RFI-Datenstrukturen und überträgt sie
			in den Mesh-Vektor der Mesh-Lib.
                                                                          
 Programmaenderungen:
   04/2004     TK        Erste Version
08/2005 TK
**************************************************************************/
void Get_Rockflow_RFI ()
{
  static long *element_nodes;
  int i=0, j=0;
  long nodelistlength=0;
  long elementlistlength=0;
  CMSHNodes *m_rfi_nodes = NULL; //OK
  CMSHElements* m_msh_elements = NULL; //OK
  int size = (int)msh_nodes_vector.size();
  
  //==========================================================
  //              Node-Vektor
  //==========================================================
    if ((int)msh_nodes_vector.size() > 0)
    {
	    for (j=0;j<(int)msh_nodes_vector.size();j++)
        {
            delete msh_nodes_vector[j];
        }
        msh_nodes_vector.clear();
    }
  //==========================================================
  if(NODListExists()){	  
	nodelistlength  =  NodeListSize ();
	for (i=0;i<nodelistlength;i++)
	{
		m_rfi_nodes = new CMSHNodes;
		m_rfi_nodes->nodenumber = i;
		m_rfi_nodes->x =  GetNodeX (i);
		m_rfi_nodes->y =  GetNodeY (i);
		m_rfi_nodes->z =  GetNodeZ (i);
		msh_nodes_vector.push_back(m_rfi_nodes);  
	}
  }
 


  //==========================================================
  //              Element-Vektor
  //==========================================================

  size = (int)msh_nodes_vector.size();

    if ((int)msh_elements_vector.size() > 0)
    {
	    for (j=0;j<(int)msh_elements_vector.size();j++)
        {
            delete msh_elements_vector[j];
        }
        msh_elements_vector.clear();
    }
  //==========================================================
  if(ELEListExists()) {
		elementlistlength =  ElListSize ();
		for (i=0;i<elementlistlength;i++)
		{
			/*LINES = 1*/ 
			if (ElGetElementType(i) == 1)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 =  GetNodeX (element_nodes[0]);
			m_msh_elements->y1 =  GetNodeY (element_nodes[0]);
			m_msh_elements->z1 =  GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 =  GetNodeX (element_nodes[1]);
			m_msh_elements->y2 =  GetNodeY (element_nodes[1]);
			m_msh_elements->z2 =  GetNodeZ (element_nodes[1]);
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*RECTANGLES = 2*/ 
			if (ElGetElementType(i) == 2)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 =  GetNodeX (element_nodes[0]);
			m_msh_elements->y1 =  GetNodeY (element_nodes[0]);
			m_msh_elements->z1 =  GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 =  GetNodeX (element_nodes[1]);
			m_msh_elements->y2 =  GetNodeY (element_nodes[1]);
			m_msh_elements->z2 =  GetNodeZ (element_nodes[1]);
			m_msh_elements->x3 =  GetNodeX (element_nodes[2]);
			m_msh_elements->y3 =  GetNodeY (element_nodes[2]);
			m_msh_elements->z3 =  GetNodeZ (element_nodes[2]);
 			m_msh_elements->x4 =  GetNodeX (element_nodes[3]);
			m_msh_elements->y4 =  GetNodeY (element_nodes[3]);
			m_msh_elements->z4 =  GetNodeZ (element_nodes[3]);
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*HEXAHEDRA = 3*/ 
			if (ElGetElementType(i) == 3)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->node5 = element_nodes[4];
			m_msh_elements->node6 = element_nodes[5];
			m_msh_elements->node7 = element_nodes[6];
			m_msh_elements->node8 = element_nodes[7];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 = GetNodeX (element_nodes[0]);
			m_msh_elements->y1 = GetNodeY (element_nodes[0]);
			m_msh_elements->z1 = GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 = GetNodeX (element_nodes[1]);
			m_msh_elements->y2 = GetNodeY (element_nodes[1]);
			m_msh_elements->z2 = GetNodeZ (element_nodes[1]);
			m_msh_elements->x3 = GetNodeX (element_nodes[2]);
			m_msh_elements->y3 = GetNodeY (element_nodes[2]);
			m_msh_elements->z3 = GetNodeZ (element_nodes[2]);
 			m_msh_elements->x4 = GetNodeX (element_nodes[3]);
			m_msh_elements->y4 = GetNodeY (element_nodes[3]);
			m_msh_elements->z4 = GetNodeZ (element_nodes[3]);
			m_msh_elements->x5 = GetNodeX (element_nodes[4]);
			m_msh_elements->y5 = GetNodeY (element_nodes[4]);
			m_msh_elements->z5 = GetNodeZ (element_nodes[4]);
			m_msh_elements->x6 = GetNodeX (element_nodes[5]);
			m_msh_elements->y6 = GetNodeY (element_nodes[5]);
			m_msh_elements->z6 = GetNodeZ (element_nodes[5]);
			m_msh_elements->x7 = GetNodeX (element_nodes[6]);
			m_msh_elements->y7 = GetNodeY (element_nodes[6]);
			m_msh_elements->z7 = GetNodeZ (element_nodes[6]);
			m_msh_elements->x8 = GetNodeX (element_nodes[7]);
			m_msh_elements->y8 = GetNodeY (element_nodes[7]);
			m_msh_elements->z8 = GetNodeZ (element_nodes[7]);
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*TRIANGLES = 4*/ 
			if (ElGetElementType(i) == 4)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 =  GetNodeX (element_nodes[0]);
			m_msh_elements->y1 =  GetNodeY (element_nodes[0]);
			m_msh_elements->z1 =  GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 =  GetNodeX (element_nodes[1]);
			m_msh_elements->y2 =  GetNodeY (element_nodes[1]);
			m_msh_elements->z2 =  GetNodeZ (element_nodes[1]);
			m_msh_elements->x3 =  GetNodeX (element_nodes[2]);
			m_msh_elements->y3 =  GetNodeY (element_nodes[2]);
			m_msh_elements->z3 =  GetNodeZ (element_nodes[2]);
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*TETRAHEDRAS = 5*/ 
			if (ElGetElementType(i) == 5)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 =   GetNodeX (element_nodes[0]);
			m_msh_elements->y1 =   GetNodeY (element_nodes[0]);
			m_msh_elements->z1 =   GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 =   GetNodeX (element_nodes[1]);
			m_msh_elements->y2 =   GetNodeY (element_nodes[1]);
			m_msh_elements->z2 =   GetNodeZ (element_nodes[1]);
			m_msh_elements->x3 =   GetNodeX (element_nodes[2]);
			m_msh_elements->y3 =   GetNodeY (element_nodes[2]);
			m_msh_elements->z3 =   GetNodeZ (element_nodes[2]);
 			m_msh_elements->x4 =   GetNodeX (element_nodes[3]);
			m_msh_elements->y4 =   GetNodeY (element_nodes[3]);
			m_msh_elements->z4 =   GetNodeZ (element_nodes[3]);
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*PRISMS = 6*/ 
			if (ElGetElementType(i) == 6)
			{
			element_nodes = ElGetElementNodes(i);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
			m_msh_elements->element_type = ElGetElementType(i);
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->node5 = element_nodes[4];
			m_msh_elements->node6 = element_nodes[5];
			m_msh_elements->materialnumber = ElGetElementGroupNumber(i);
			m_msh_elements->x1 = GetNodeX (element_nodes[0]);
			m_msh_elements->y1 = GetNodeY (element_nodes[0]);
			m_msh_elements->z1 = GetNodeZ (element_nodes[0]);
			m_msh_elements->x2 = GetNodeX (element_nodes[1]);
			m_msh_elements->y2 = GetNodeY (element_nodes[1]);
			m_msh_elements->z2 = GetNodeZ (element_nodes[1]);
			m_msh_elements->x3 = GetNodeX (element_nodes[2]);
			m_msh_elements->y3 = GetNodeY (element_nodes[2]);
			m_msh_elements->z3 = GetNodeZ (element_nodes[2]);
 			m_msh_elements->x4 = GetNodeX (element_nodes[3]);
			m_msh_elements->y4 = GetNodeY (element_nodes[3]);
			m_msh_elements->z4 = GetNodeZ (element_nodes[3]);
			m_msh_elements->x5 = GetNodeX (element_nodes[4]);
			m_msh_elements->y5 = GetNodeY (element_nodes[4]);
			m_msh_elements->z5 = GetNodeZ (element_nodes[4]);
			m_msh_elements->x6 = GetNodeX (element_nodes[5]);
			m_msh_elements->y6 = GetNodeY (element_nodes[5]);
			m_msh_elements->z6 = GetNodeZ (element_nodes[5]);
			msh_elements_vector.push_back(m_msh_elements); 

			} 
		}
	}

}

/**************************************************************************
 MSHLIB - Funktion: Get_Rockflow_RFI_from_File
                                                                          
 Aufgabe: liest das alte Rockflow-RFI und füllt den Mesh-Vektor
			der Mesh-Lib.
                                                                          
 Programmaenderungen:
   04/2004     TK        Erste Version
**************************************************************************/
void Get_Rockflow_RFI_from_File (string filename)
{
 char text[1024];
const char *file = 0;
int rfi_nodeorigin_id = 0;
int rfi_elementorigin_id  = 0;
char tetrakey [5];
double x, y, z;
long ldummy  = 0l;
long material_group  = 0l;
long numberofnodes = 0l;
long knotendummy1,knotendummy2,knotendummy3,knotendummy4,knotendummy5,knotendummy6,knotendummy7,knotendummy8;
     knotendummy1=knotendummy2=knotendummy3=knotendummy4=knotendummy5=knotendummy6=knotendummy7=knotendummy8=0l;
int serialnode=0;
int serialelement_id=0;
int i=0;
  CMSHNodes* m_rfi_nodes = NULL; //OK
  CMSHElements* m_msh_elements = NULL; //OK

file = filename.data();
FILE *dat_in_rfi;
dat_in_rfi = fopen(file, "r+"); // OPEN RFI FILE

//==========================================================
//              RFI lesen und Vektoren füllen
//==========================================================
while (!feof(dat_in_rfi)) 
 {
   fgets(text,1024,dat_in_rfi);

   if (4 == sscanf(text,"%i %le %le %le", &rfi_nodeorigin_id,&x,&y,&z))
   {
     m_rfi_nodes = new CMSHNodes;
	 m_rfi_nodes->origin_rfi_node_number = rfi_nodeorigin_id;
	 m_rfi_nodes->serialized_rfi_node_number = serialnode;
	 m_rfi_nodes->x = x;
	 m_rfi_nodes->y = y;
	 m_rfi_nodes->z = z;
	 m_rfi_nodes->rfi_node_id = serialnode;
	 msh_nodes_vector.push_back(m_rfi_nodes);
	 serialnode++;
   }

   //Variante für altes RFI - ohne ldummy:
   //if (6 <= sscanf(text,"%ld %ld %s %ld %ld %ld %ld", &rfi_elementorigin_id,&material_group, tetrakey, &knotendummy1,&knotendummy2,&knotendummy3,&knotendummy4,&knotendummy5,&knotendummy6,&knotendummy7,&knotendummy8))

   //Variante für neues RFI - mit ldummy:
   if (6 <= sscanf(text,"%i %ld %ld %s %ld %ld %ld %ld %ld %ld %ld %ld", 
              &rfi_elementorigin_id,&material_group, &ldummy, tetrakey, 
              &knotendummy1,&knotendummy2,&knotendummy3,&knotendummy4,
              &knotendummy5,&knotendummy6,&knotendummy7,&knotendummy8))
   {
	 numberofnodes = (int)msh_nodes_vector.size();
	 m_msh_elements = new CMSHElements;
	 m_msh_elements->element_id = serialelement_id;
	 m_msh_elements->materialnumber = material_group;
	 if (!strcmp(tetrakey , "line"))  m_msh_elements->elementtype = 1;
     if (!strcmp(tetrakey , "quad"))  m_msh_elements->elementtype = 2;
	 if (!strcmp(tetrakey , "hex"))  m_msh_elements->elementtype = 3;
	 if (!strcmp(tetrakey , "tri"))  m_msh_elements->elementtype = 4;
	 if (!strcmp(tetrakey , "tet"))  m_msh_elements->elementtype = 5;
	 if (!strcmp(tetrakey , "prism"))  m_msh_elements->elementtype = 6;   
	 for (i=0;i<numberofnodes;i++)
	 {
		 if (knotendummy1 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node1 = msh_nodes_vector[i]->serialized_rfi_node_number;
		 if (knotendummy2 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node2 = msh_nodes_vector[i]->serialized_rfi_node_number;
         if (knotendummy3 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node3 = msh_nodes_vector[i]->serialized_rfi_node_number;
         if (knotendummy4 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node4 = msh_nodes_vector[i]->serialized_rfi_node_number;
		 if (knotendummy5 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node5 = msh_nodes_vector[i]->serialized_rfi_node_number;
		 if (knotendummy6 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node6 = msh_nodes_vector[i]->serialized_rfi_node_number;
         if (knotendummy7 == msh_nodes_vector[i]->origin_rfi_node_number)
		     m_msh_elements->node7 = msh_nodes_vector[i]->serialized_rfi_node_number;
		 if (knotendummy8 == msh_nodes_vector[i]->origin_rfi_node_number)
			 m_msh_elements->node8 =  msh_nodes_vector[i]->serialized_rfi_node_number;
	 }
 	 msh_elements_vector.push_back(m_msh_elements);
	 serialelement_id++;  
   } 
 }
 /*Datei zerstören und neu anlegen*/ 
 fclose(dat_in_rfi);
 remove(file);
}


/**************************************************************************
 MSHLIB - Funktion: Serialize_RFI_Numbering
                                                                          
 Aufgabe: Liest RFI file und nummeriert neu; 
   startet mit 0 und Nummerierung hat keine Lücken
                                                                          
 Programmaenderungen:
   12/2003     TK        Erste Version
   04/2004     TK        Get_Rockflow_RFI (filename) gekapselt
**************************************************************************/
void Serialize_RFI (string filename)
{
  Get_Rockflow_RFI_from_File (filename);
//==========================================================
//            MSH-RFI Vektoren lesen und File schreiben
//==========================================================

 const char *file = 0;
 file = filename.data();
 long ldummy  = 0l;
 long knotendummy1,knotendummy2,knotendummy3,knotendummy4,knotendummy5,knotendummy6,knotendummy7,knotendummy8;
      knotendummy1=knotendummy2=knotendummy3=knotendummy4=knotendummy5=knotendummy6=knotendummy7=knotendummy8=0l;
 int i=0;
 char rf_version_string[16];
 long j=0; 
 static char kopf[81];
 long output_nr=0,output_geom=0;
 long nb_prismnodes;
 long nb_elements;

 FILE *dat_out_rfi;
 dat_out_rfi = fopen(file, "w+t");   

    /*Lesen der Versionsnummer und loeschen der Punkte aus dem Versions-String */ 
    strcpy(rf_version_string,ROCKFLOW_VERSION);
    j = 0;
    for (i=0;((i<(int)(int)strlen(rf_version_string))&&(rf_version_string[i]));i++) 
    {
        if (rf_version_string[i]!='.')
            rf_version_string[i-j] = rf_version_string[i];
            else
            j++;
    }   
    rf_version_string[i-j] = 0;

	/*Schreibe Kopf*/ 
	for (i=0;i<80;i++)
          kopf[i] = ' ';
      kopf[80] = '\0';
      sprintf(kopf," 0 0 %ld %ld %#g 0 %s",output_nr,output_geom,0.0,rf_version_string);
      for (i=0;i<80;i++)
          if ((kopf[i]==' ') || (kopf[i]=='\0'))
              kopf[i] = '#';
      if (fprintf(dat_out_rfi,"%s\n",kopf)!=81) {
          DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Kopf) !!");
      }

	/*Schreibe 2.Zeile*/ 
    nb_prismnodes = (long)msh_nodes_vector.size();
    nb_elements = (long)msh_elements_vector.size()-1;
    fprintf(dat_out_rfi,"0 ");
    fprintf(dat_out_rfi,"%ld ",nb_prismnodes);
    fprintf(dat_out_rfi,"%ld \n",nb_elements);
   
	/*Schreibe Geometrie*/ 
    for (i=0;i<nb_prismnodes;i++)
	{
	  fprintf(dat_out_rfi,"%ld ",msh_nodes_vector[i]->rfi_node_id);
      fprintf(dat_out_rfi,"%20.14g ",msh_nodes_vector[i]->x);
      fprintf(dat_out_rfi,"%20.14g ",msh_nodes_vector[i]->y);
      fprintf(dat_out_rfi,"%20.14g \n",msh_nodes_vector[i]->z);     
	}

	/*Schreibe Topologie*/ 
    for (i=0;i<nb_elements;i++)
	{
	  fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->element_id);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->materialnumber);
	  fprintf(dat_out_rfi,"-1 ");
	  
	  ldummy = msh_elements_vector[i]->elementtype;


	  if (msh_elements_vector[i]->elementtype == 1)
	  {
	  fprintf(dat_out_rfi,"line ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node2);
	  }

      if (msh_elements_vector[i]->elementtype == 2)
	  {
	  fprintf(dat_out_rfi,"quad ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node2);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node3);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node4);
	  }

	  if (msh_elements_vector[i]->elementtype == 3)
	  {
	  fprintf(dat_out_rfi,"hex ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node2);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node3);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node4);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node5);
	  fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node6);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node7);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node8);
	  }

	  if (msh_elements_vector[i]->elementtype == 4)
	  {
	  fprintf(dat_out_rfi,"tri ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node2);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node3);
	  }

	  if (msh_elements_vector[i]->elementtype == 5)
	  {
	  fprintf(dat_out_rfi,"tet ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node2);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node3);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node4);
	  }

	  if (msh_elements_vector[i]->elementtype == 6)
	  {
	  fprintf(dat_out_rfi,"pris ");
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node1);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node2);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node3);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node4);
      fprintf(dat_out_rfi,"%ld ",msh_elements_vector[i]->node5);
      fprintf(dat_out_rfi,"%ld \n",msh_elements_vector[i]->node6);
	  }

	}


  	/*Datei Schließen*/ 
	fclose(dat_out_rfi);

	/*Vektoren leeren*/ 
    msh_nodes_vector.clear();
    msh_elements_vector.clear();
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
void RFIWriteTecplot()
{
  //----------------------------------------------------------------------
  // File handling
  string file_path = "MSH";
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("rfi");
  if(m_gsp)
    file_path = m_gsp->path + "MSH";
  //----------------------------------------------------------------------
  long i;
  CMSHElements *m_element;
  vector<CMSHElements*>line_elements;
  vector<CMSHElements*>tri_elements;
  vector<CMSHElements*>quad_elements;
  vector<CMSHElements*>hex_elements;
  vector<CMSHElements*>tet_elements;
  vector<CMSHElements*>pris_elements;
  long *nodes = NULL;
  string delimiter(", ");

  //----------------------------------------------------------------------
  for(i=0;i<ElListSize();i++){
    nodes = ElGetElementNodes(i);
    m_element = new CMSHElements();
    switch(ElGetElementType(i)){
      case 1:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          line_elements.push_back(m_element);
        break;
      case 2:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          m_element->node3 = nodes[2];
          m_element->node4 = nodes[3];
          quad_elements.push_back(m_element);
        break;
      case 3:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          m_element->node3 = nodes[2];
          m_element->node4 = nodes[3];
          m_element->node5 = nodes[4];
          m_element->node6 = nodes[5];
          m_element->node7 = nodes[6];
          m_element->node8 = nodes[7];
          hex_elements.push_back(m_element);
        break;
      case 4:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          m_element->node3 = nodes[2];
          tri_elements.push_back(m_element);
        break;
      case 5:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          m_element->node3 = nodes[2];
          m_element->node4 = nodes[3];
          tet_elements.push_back(m_element);
        break;
      case 6:
          m_element->node1 = nodes[0];
          m_element->node2 = nodes[1];
          m_element->node3 = nodes[2];
          m_element->node4 = nodes[3];
          m_element->node5 = nodes[4];
          m_element->node6 = nodes[5];
          pris_elements.push_back(m_element);
        break;
    }
  }
  //----------------------------------------------------------------------
  // 2 - Write
    long no_nodes = NodeListLength;
    //....................................................................
    // LINE
    long no_lines = (long)line_elements.size();
    if(no_lines>0){
      string line_file_name = file_path + "_LINE" + TEC_FILE_EXTENSION;
      fstream line_file (line_file_name.data(),ios::trunc|ios::out);
      line_file.setf(ios::scientific,ios::floatfield);
  	  line_file.precision(12);
      if (!line_file.good()) return;
      line_file.seekg(0L,ios::beg);
      line_file << "VARIABLES = X,Y,Z" << endl;
      line_file << "ZONE T = " << "LINES" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_lines << delimiter \
               << "F = FEPOINT" << delimiter << "ET = QUADRILATERAL" << endl;
      for(i=0;i<no_nodes;i++) {
        line_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_lines;i++) {
        line_file \
          << line_elements[i]->node1+1 << " " << line_elements[i]->node2+1 << " " \
          << line_elements[i]->node2+1 << " " << line_elements[i]->node1+1 << endl;
      }
    }
    //....................................................................
    // QUAD
    long no_quads = (long)quad_elements.size();
    if(no_quads>0){
      string quad_file_name = file_path + "_QUAD" + TEC_FILE_EXTENSION;
      fstream quad_file (quad_file_name.data(),ios::trunc|ios::out);
      quad_file.setf(ios::scientific,ios::floatfield);
  	  quad_file.precision(12);
      if (!quad_file.good()) return;
      quad_file.seekg(0L,ios::beg);
      quad_file << "VARIABLES = X,Y,Z" << endl;
      quad_file << "ZONE T = " << "QUADS" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_quads << delimiter \
               << "F = FEPOINT" << delimiter << "ET = QUADRILATERAL" << endl;
      for(i=0;i<no_nodes;i++) {
        quad_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_quads;i++) {
        quad_file \
          << quad_elements[i]->node1+1 << " " << quad_elements[i]->node2+1 << " " \
          << quad_elements[i]->node3+1 << " " << quad_elements[i]->node4+1 << endl;
      }
    }
    //....................................................................
    // HEX
    long no_hexs = (long)hex_elements.size();
    if(no_hexs>0){
      string hex_file_name = file_path + "_HEX" + TEC_FILE_EXTENSION;
      fstream hex_file (hex_file_name.data(),ios::trunc|ios::out);
      hex_file.setf(ios::scientific,ios::floatfield);
  	  hex_file.precision(12);
      if (!hex_file.good()) return;
      hex_file.seekg(0L,ios::beg);
      hex_file << "VARIABLES = X,Y,Z" << endl;
      hex_file << "ZONE T = " << "HEXS" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_hexs << delimiter \
               << "F = FEPOINT" << delimiter << "ET = BRICK" << endl;
      for(i=0;i<no_nodes;i++) {
        hex_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_hexs;i++) {
        hex_file \
          << hex_elements[i]->node1+1 << " " << hex_elements[i]->node2+1 << " " \
          << hex_elements[i]->node3+1 << " " << hex_elements[i]->node4+1 << " " \
          << hex_elements[i]->node5+1 << " " << hex_elements[i]->node6+1 << " " \
          << hex_elements[i]->node7+1 << " " << hex_elements[i]->node8+1 << endl;
      }
    }
    //....................................................................
    // TRI
    long no_tris = (long)tri_elements.size();
    if(no_tris>0){
      string tri_file_name = file_path + "_TRI" + TEC_FILE_EXTENSION;
      fstream tri_file (tri_file_name.data(),ios::trunc|ios::out);
      tri_file.setf(ios::scientific,ios::floatfield);
  	  tri_file.precision(12);
      if (!tri_file.good()) return;
      tri_file.seekg(0L,ios::beg);
      tri_file << "VARIABLES = X,Y,Z" << endl;
      tri_file << "ZONE T = " << "TRI" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_tris << delimiter \
               << "F = FEPOINT" << delimiter << "ET = TRIANGLE" << endl;
      for(i=0;i<no_nodes;i++) {
        tri_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_tris;i++) {
        tri_file \
          << tri_elements[i]->node1+1 << " " << tri_elements[i]->node2+1 << " " \
          << tri_elements[i]->node3+1 << endl;
      }
    }
    //....................................................................
    // TET
    long no_tets = (long)tet_elements.size();
    if(no_tets>0){
      string tet_file_name = file_path + "_TET" + TEC_FILE_EXTENSION;
      fstream tet_file (tet_file_name.data(),ios::trunc|ios::out);
      tet_file.setf(ios::scientific,ios::floatfield);
  	  tet_file.precision(12);
      if (!tet_file.good()) return;
      tet_file.seekg(0L,ios::beg);
      tet_file << "VARIABLES = X,Y,Z" << endl;
      tet_file << "ZONE T = " << "TETS" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_tets << delimiter \
               << "F = FEPOINT" << delimiter << "ET = TETRAHEDRON" << endl;
      for(i=0;i<no_nodes;i++) {
        tet_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_tets;i++) {
        tet_file \
          << tet_elements[i]->node1+1 << " " << tet_elements[i]->node2+1 << " " \
          << tet_elements[i]->node3+1 << " " << tet_elements[i]->node4+1 << endl;
      }
    }
    //....................................................................
    // PRIS
    long no_pris = (long)pris_elements.size();
    if(no_pris>0){
      string pris_file_name = file_path + "_PRIS" + TEC_FILE_EXTENSION;
      fstream pris_file (pris_file_name.data(),ios::trunc|ios::out);
      pris_file.setf(ios::scientific,ios::floatfield);
  	  pris_file.precision(12);
      if (!pris_file.good()) return;
      pris_file.seekg(0L,ios::beg);
      pris_file << "VARIABLES = X,Y,Z" << endl;
      pris_file << "ZONE T = " << "PRIS" << delimiter \
               << "N = " << no_nodes << delimiter \
               << "E = " << no_pris << delimiter \
               << "F = FEPOINT" << delimiter << "ET = BRICK" << endl;
      for(i=0;i<no_nodes;i++) {
        pris_file \
          << GetNodeX(i) << " " << GetNodeY(i) << " " << GetNodeZ(i) << endl;
      }
      for(i=0;i<no_pris;i++) {
        pris_file \
          << pris_elements[i]->node1+1 << " " << pris_elements[i]->node1+1 << " " \
          << pris_elements[i]->node2+1 << " " << pris_elements[i]->node3+1 << " " \
          << pris_elements[i]->node4+1 << " " << pris_elements[i]->node4+1 << " " \
          << pris_elements[i]->node5+1 << " " << pris_elements[i]->node6+1 << endl;
      }
    }

  //----------------------------------------------------------------------
  // 3 - Destruction
  line_elements.clear();
  tri_elements.clear();
  quad_elements.clear();
  hex_elements.clear();
  tet_elements.clear();
  pris_elements.clear();
}

/**************************************************************************
FEM2RF - Funktion: MSH2RFI
Task: 
- convert data for gmsh and to RFI file
Programming:
11/2002 WW/OK Implementation
05/2004 TK
**************************************************************************/
void MSH2RFI (char *filename, FILE *rfi_file)
{
  int Loca=0;
  int i=0, j=0;
  long l_read=0;
  int pos=0,pos_inc=0;
  int NumNodes=0;
  int NumElements=0;
  int Pos=8; 
  int MatId=0;
  int End = 1;
  long id, adummy, bdummy, cdummy, nben, en1, en2, en3, en4;
  char text[MAX_ZEILE];
  
  double f_read=0;

  while(filename[Loca]!='\0') Loca++;

  FILE * msh_file;   
  filename[Loca-3] = 'm';
  filename[Loca-2] = 's';
  filename[Loca-1] = 'h';
  msh_file = fopen(filename, "rt");

  FilePrintString(rfi_file, " #0#0#0#1#0.0#0#3909OK#######");    
  LineFeed (rfi_file);  
  FilePrintInt(rfi_file, MatId);
  FilePrintString(rfi_file, " ");    
  FilePrintInt(rfi_file, NumNodes);
  FilePrintString(rfi_file, " ");    
  FilePrintInt(rfi_file, NumElements);
  FilePrintString(rfi_file, "            \n");    

  
  while (End) 
  {

    /*rewind(rfm_file);*/
    /*------------------------------------------------------------------*/
    fgets(text,MAX_ZEILE,msh_file); /* Node keyword */

    fgets(text,MAX_ZEILE,msh_file); /* Number of nodes */
    StringReadLong (&l_read,text,&pos);
	NumNodes=l_read;
	/* Read coordinates */
    for(i=0; i<NumNodes; i++)
	{
      pos=0;
      fgets(text,MAX_ZEILE,msh_file); /* Node index */
      StringReadLong (&l_read,text,&pos_inc);
	  FilePrintInt(rfi_file, l_read-1); //OK
	  FilePrintString(rfi_file, " ");    
	  for(j=0; j<3; j++)
	  {
        StringReadDouble (&f_read,&text[pos+=pos_inc],&pos_inc);
	    FilePrintDouble(rfi_file, f_read);
	    FilePrintString(rfi_file, " ");
	  }
      LineFeed (rfi_file);  

	}

	fgets(text,MAX_ZEILE,msh_file); /* End Node keyword */
    
	/* Elements */
	pos_inc=0;
    fgets(text,MAX_ZEILE,msh_file); /* Node keyword */
    fgets(text,MAX_ZEILE,msh_file); /* Number of elements */
    StringReadLong (&l_read,text,&pos);
	NumElements=l_read;

    for(i=0; i<NumElements; i++)
	{
      pos=0;
      fgets(text,MAX_ZEILE,msh_file); /* Element index */
	  sscanf(text,"%ld %ld %ld %ld %ld %ld %ld %ld %ld", &id, &adummy, &bdummy, &cdummy, &nben, &en1, &en2, &en3, &en4);
	  if (nben==3){
	  StringReadLong (&l_read,text,&pos_inc);
	  FilePrintInt(rfi_file, l_read-1); //OK
	  FilePrintString(rfi_file, " ");    
	  FilePrintInt(rfi_file, MatId);
	  FilePrintString(rfi_file, " -1");    
	  FilePrintString(rfi_file, " tri ");    
      for(j=0; j<Pos-1; j++)
	  {
        StringReadLong (&l_read,&text[pos+=pos_inc],&pos_inc);
		if(j>3)
		{
	      FilePrintInt(rfi_file, l_read-1);
	      FilePrintString(rfi_file, " ");
		}
	  }
      LineFeed (rfi_file); 
	  }
	}
    fgets(text,MAX_ZEILE,msh_file);
    End=feof(rfi_file);

  }

  rewind(rfi_file);
  FilePrintString(rfi_file, " #0#0#0#1#0.0#0#3909OK########");    
  LineFeed (rfi_file);  
  FilePrintInt(rfi_file, MatId);
  FilePrintString(rfi_file, " ");    
  FilePrintInt(rfi_file, NumNodes);
  FilePrintString(rfi_file, " ");    
  FilePrintInt(rfi_file, NumElements);

  fclose(msh_file);
}


/**************************************************************************
ROCKFLOW - Funktion: DATWriteRFIFile
Task: Write RFI file
Programing:
01/2003   OK   Implementation
**************************************************************************/
void DATWriteRFIFile(const char *file_name)
{
  long i;
  long *element_nodes = NULL;
  FILE *rfi_file = NULL;
  char rfi_file_name[MAX_ZEILE];

if((NodeListLength>0)&&(ElListSize()>0)) {

  sprintf(rfi_file_name,"%s.%s",file_name,"rfi");
  rfi_file = fopen(rfi_file_name,"w+t");

  fprintf(rfi_file," %s\n","#0#0#0#1#0.0#0#3909OK#######");
  fprintf(rfi_file," %i %ld %ld\n",0,NodeListLength,ElListSize());
  /* Nodes */
  for(i=0;i<NodeListLength;i++) {
    fprintf(rfi_file," %ld %17.12e %17.12e %17.12e\n",i,GetNodeX(i),GetNodeY(i),GetNodeZ(i));
  }
  /* Elements */
  for(i=0;i<ElListSize();i++) {
    element_nodes = ElGetElementNodes(i);
    switch (ElGetElementType(i)) {
      case 1:  /* line */
              fprintf(rfi_file," %ld %ld %d %s %ld %ld\n",\
                      i, ElGetElementGroupNumber(i), -1,  "line",\
                      element_nodes[0], element_nodes[1]);
              break;
      case 2: /* quadrilaterals */
              fprintf(rfi_file," %ld %ld %d %s %ld %ld %ld %ld\n",\
                                              i,\
                                              ElGetElementGroupNumber(i),\
                                              -1,\
                                              "quad",\
                                              element_nodes[0],
                                              element_nodes[1],
                                              element_nodes[2],
                                              element_nodes[3]);
              break;
      case 3: /* hexahedra */
              fprintf(rfi_file," %ld %ld %d %s %ld %ld %ld %ld %ld %ld %ld %ld\n",\
                                              i,\
                                              ElGetElementGroupNumber(i),\
                                              -1,\
                                              "hex",\
                                              element_nodes[0],
                                              element_nodes[1],
                                              element_nodes[2],
                                              element_nodes[3],
                                              element_nodes[4],
                                              element_nodes[5],
                                              element_nodes[6],
                                              element_nodes[7]);
              break;
      case 4: /* triangle */
              fprintf(rfi_file," %ld %ld %d %s %ld %ld %ld\n",\
                                              i,\
                                              ElGetElementGroupNumber(i),\
                                              -1,\
                                              "tri",\
                                              element_nodes[0],
                                              element_nodes[1],
                                              element_nodes[2]);
              break;
      case 6: /* prism */
              fprintf(rfi_file," %ld %ld %d %s %ld %ld %ld %ld %ld %ld\n",\
                                              i,\
                                              ElGetElementGroupNumber(i),\
                                              -1,\
                                              "pris",\
                                              element_nodes[0],
                                              element_nodes[1],
                                              element_nodes[2],
                                              element_nodes[3],
                                              element_nodes[4],
                                              element_nodes[5]);
              break;
    }

  }
  fclose(rfi_file);

 }

}

/*************************************************************************
  ROCKFLOW - Function: ReadRFIFile
  Task: Input of RFI data
  Programming: 12/2002   OK   Implementation
  last modified:
**************************************************************************/
int ReadRFIFile(string g_strFileNameBase) 
{
  // File names
  string g_strFileNameRFI = g_strFileNameBase.append(RF_INPUT_EXTENSION);
  string g_strFileNameBAK = g_strFileNameBase.append(".bak");
  string ExecuteString = "copy "; 
  ExecuteString.append(g_strFileNameRFI);
  ExecuteString.append(" ");
  ExecuteString.append(g_strFileNameBAK);
  const char *str_file_rfi = 0; 
  str_file_rfi = g_strFileNameRFI.data();
  // Anlegen einer Sicherheitskopie
  const char *ExecuteChar = 0; 
  ExecuteChar = ExecuteString.data();
//OK  system(ExecuteChar); 
  // 3 Objekt-Listen 
  DestroyNodeList();
  ElDestroyElementList();
  CreateNodeList();
  ElCreateElementList();
  /****************************************************************************/
  /* Read geometry RFI file */
  /*--------------------------------------------------------------------*/
  /* Jakobian und Element-Volumen berechnen */
  //CalcElementsGeometry();

  return 1;
}

/**************************************************************************
 MSHLIB - Funktion: Get_Rockflow_RFI
                                                                          
 Aufgabe: liest MSH-ELE_NOD Vector und überträgt sie
			in den Mesh-Vektor der Mesh-Lib zur Qualitätsbestimmung.
                                                                          
 Programmaenderungen:
   04/2004     TK        Erste Version
08/2005 TK
**************************************************************************/
void Get_MSH_for_Quality (int meshnumber)
{
  int i=0, j=0;
  CMSHNodes *m_msh_nodes = NULL; //OK
  CMSHElements* m_msh_elements = NULL; //OK
  int size = (int)msh_nodes_vector.size();
  vec<long> element_nodes(20);
  
  //==========================================================
  //              Node-Vektor
  //==========================================================
    if ((int)msh_nodes_vector.size() > 0)
    {
	    for (j=0;j<(int)msh_nodes_vector.size();j++)
        {
            delete msh_nodes_vector[j];
        }
        msh_nodes_vector.clear();
    }
  //==========================================================
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
      if (meshnumber == j)
      {
        for (i=0;i<(int)fem_msh_vector[j]->nod_vector.size();i++)
	    {
		    m_msh_nodes = new CMSHNodes;
		    m_msh_nodes->nodenumber = i;
		    m_msh_nodes->x =  fem_msh_vector[j]->nod_vector[i]->X();
		    m_msh_nodes->y =  fem_msh_vector[j]->nod_vector[i]->Y();
		    m_msh_nodes->z =  fem_msh_vector[j]->nod_vector[i]->Z();
		    msh_nodes_vector.push_back(m_msh_nodes);  
	    }
      }

    }    

  //==========================================================
  //              Element-Vektor
  //==========================================================

  size = (int)msh_nodes_vector.size();

    if ((int)msh_elements_vector.size() > 0)
    {
	    for (j=0;j<(int)msh_elements_vector.size();j++)
        {
            delete msh_elements_vector[j];
        }
        msh_elements_vector.clear();
    }
  //==========================================================
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
      if (meshnumber == j)
      {
        for (i=0;i<(int)fem_msh_vector[j]->ele_vector.size();i++)
	    {
			/*LINES = 1*/ 
            if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 1)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();

            fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
            fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
            fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();


            m_msh_elements->x1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*RECTANGLES = 2*/ 
			if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 2)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
            m_msh_elements->x1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			m_msh_elements->x3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->X();
			m_msh_elements->y3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->Y();
			m_msh_elements->z3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->Z();
 			m_msh_elements->x4 =  fem_msh_vector[j]->nod_vector[element_nodes[3]]->X();
			m_msh_elements->y4 =  fem_msh_vector[j]->nod_vector[element_nodes[3]]->Y();
			m_msh_elements->z4 =  fem_msh_vector[j]->nod_vector[element_nodes[3]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*HEXAHEDRA = 3*/ 
			if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 3)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->node5 = element_nodes[4];
			m_msh_elements->node6 = element_nodes[5];
			m_msh_elements->node7 = element_nodes[6];
			m_msh_elements->node8 = element_nodes[7];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
            m_msh_elements->x1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			m_msh_elements->x3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->X();
			m_msh_elements->y3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->Y();
			m_msh_elements->z3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->Z();
 			m_msh_elements->x4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->X();
			m_msh_elements->y4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->Y();
			m_msh_elements->z4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->Z();
			m_msh_elements->x5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->X();
			m_msh_elements->y5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->Y();
			m_msh_elements->z5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->Z();
			m_msh_elements->x6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->X();
			m_msh_elements->y6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->Y();
			m_msh_elements->z6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->Z();
			m_msh_elements->x7 = fem_msh_vector[j]->nod_vector[element_nodes[6]]->X();
			m_msh_elements->y7 = fem_msh_vector[j]->nod_vector[element_nodes[6]]->Y();
			m_msh_elements->z7 = fem_msh_vector[j]->nod_vector[element_nodes[6]]->Z();
			m_msh_elements->x8 = fem_msh_vector[j]->nod_vector[element_nodes[7]]->X();
			m_msh_elements->y8 = fem_msh_vector[j]->nod_vector[element_nodes[7]]->Y();
			m_msh_elements->z8 = fem_msh_vector[j]->nod_vector[element_nodes[7]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*TRIANGLES = 4*/ 
			if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 4)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
			m_msh_elements->x1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 =  fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 =  fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			m_msh_elements->x3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->X();
			m_msh_elements->y3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->Y();
			m_msh_elements->z3 =  fem_msh_vector[j]->nod_vector[element_nodes[2]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*TETRAHEDRAS = 5*/ 
			if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 5)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
			m_msh_elements->x1 =   fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 =   fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 =   fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 =   fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 =   fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 =   fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			m_msh_elements->x3 =   fem_msh_vector[j]->nod_vector[element_nodes[2]]->X();
			m_msh_elements->y3 =   fem_msh_vector[j]->nod_vector[element_nodes[2]]->Y();
			m_msh_elements->z3 =   fem_msh_vector[j]->nod_vector[element_nodes[2]]->Z();
 			m_msh_elements->x4 =   fem_msh_vector[j]->nod_vector[element_nodes[3]]->X();
			m_msh_elements->y4 =   fem_msh_vector[j]->nod_vector[element_nodes[3]]->Y();
			m_msh_elements->z4 =   fem_msh_vector[j]->nod_vector[element_nodes[3]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
			}
			/*PRISMS = 6*/ 
			if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 6)
			{
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(element_nodes);
			m_msh_elements = new CMSHElements;     
			m_msh_elements->element_id = i;
            m_msh_elements->element_type = fem_msh_vector[j]->ele_vector[i]->GetElementType();
			m_msh_elements->node1 = element_nodes[0];
			m_msh_elements->node2 = element_nodes[1];
			m_msh_elements->node3 = element_nodes[2];
 			m_msh_elements->node4 = element_nodes[3];
			m_msh_elements->node5 = element_nodes[4];
			m_msh_elements->node6 = element_nodes[5];
            m_msh_elements->materialnumber = fem_msh_vector[j]->ele_vector[i]->GetPatchIndex();
			m_msh_elements->x1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->X();
			m_msh_elements->y1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->Y();
			m_msh_elements->z1 = fem_msh_vector[j]->nod_vector[element_nodes[0]]->Z();
			m_msh_elements->x2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->X();
			m_msh_elements->y2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->Y();
			m_msh_elements->z2 = fem_msh_vector[j]->nod_vector[element_nodes[1]]->Z();
			m_msh_elements->x3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->X();
			m_msh_elements->y3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->Y();
			m_msh_elements->z3 = fem_msh_vector[j]->nod_vector[element_nodes[2]]->Z();
 			m_msh_elements->x4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->X();
			m_msh_elements->y4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->Y();
			m_msh_elements->z4 = fem_msh_vector[j]->nod_vector[element_nodes[3]]->Z();
			m_msh_elements->x5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->X();
			m_msh_elements->y5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->Y();
			m_msh_elements->z5 = fem_msh_vector[j]->nod_vector[element_nodes[4]]->Z();
			m_msh_elements->x6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->X();
			m_msh_elements->y6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->Y();
			m_msh_elements->z6 = fem_msh_vector[j]->nod_vector[element_nodes[5]]->Z();
			msh_elements_vector.push_back(m_msh_elements); 
        }
         } 
     }
    }

}
