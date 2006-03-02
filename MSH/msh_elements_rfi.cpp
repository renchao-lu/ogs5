/**************************************************************************
PRISM GENERATOR

1. Schritt 3/2003  TK => Eingabe und Ausgabe der RFI-Dateien

**************************************************************************/

#include "stdafx.h" /* MFC */

#ifdef MFC
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#endif

#include "msh_elements_rfi.h"
#include "math.h"
// C/C++
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <fstream>
using namespace std;
// GEOLib
#include "geo_sfc.h"
#include "geo_vol.h"
// MSHLib
#include "msh_nodes_rfi.h"
#include "msh_gen_pris.h"
//FEMLib
#include "rf_mmp_new.h"
#include "files.h"
#include "edges.h"
#include "plains.h"
extern void RFConfigRenumber(void);
#include "gs_project.h"
#include "mathlib.h"
#include "tools.h"

// Vector/List
vector<CMSHElements*>msh_elements_vector; //OK
//CMSHElements *m_msh_elements = NULL;
// constructor
CMSHElements::CMSHElements(void)
{
}

CMSHElements::~CMSHElements(void)
{
}

/**************************************************************************
MshLib-Method: MSHGetElementVector
Task: liefert den Element-Vector
Programing:
07/2003 OK Implementation
**************************************************************************/
vector<CMSHElements*> MSHGetElementVector(void)
{
  return msh_elements_vector;
}

/**************************************************************************
MshLib-Method: MSHGetNodeVector
Task: liefert den Node-Vector
Programing:
07/2003 OK Implementation
**************************************************************************/
vector<CMSHNodes*> MSHGetNodeVector(void)
{
 return msh_nodes_vector;
}

/**************************************************************************
MshLib-Method: 
Task: 
Programing:
03/2004 OK Implementation
**************************************************************************/
void MSHCreateNodes()
{
  long i;
  Knoten *msh_node = NULL;
  long msh_nodes_vector_length = (long)msh_pris_nodes_vector.size();
  long node_list_length = NodeListLength;
  for(i=node_list_length;i<msh_nodes_vector_length;i++){
    msh_node = (Knoten *)CreateNodeGeometry();
    msh_node->index = i;
    msh_node->x = msh_pris_nodes_vector[i]->x;
    msh_node->y = msh_pris_nodes_vector[i]->y;
    msh_node->z = msh_pris_nodes_vector[i]->z;
    PlaceNode(msh_node,NodeListLength);
  }
}

/**************************************************************************
MshLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
01/2005 CC  add the variable: type to set the element type for deleting double elements
**************************************************************************/
void MSHDeleteDoubleElements(int type)
{
  long i;
  Element *element = NULL;
  //----------------------------------------------------------------------
  // 2 remove double elements
  for(i=0;i<ElementListLength;i++){
      if(ElGetElementType(i)==type){
    element = ElGetElement(i);
    if(element->doubled)
      ELEDeleteElementFromList(i,i);
      }
  }
}

/**************************************************************************
MshLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
ToDo CC common without switch case
**************************************************************************/
long MSHMarkDoubleElementsType(int ele_type)
{
  long i,j;
  long *node1 = NULL;
  long *node2 = NULL;
  Element *element = NULL;
  long count = 0;
  //----------------------------------------------------------------------
  // 0 reset flag
  for(i=0;i<ElementListLength;i++){
    if(ElGetElementType(i)==ele_type) {
      element = ElGetElement(i);
      element->doubled = 0;
    }
  }
  //----------------------------------------------------------------------
  // 0 reset flag
  for(i=0;i<ElementListLength;i++){
    if(ElGetElementType(i)==ele_type) {
      switch (ele_type) {
        //..................................................................
        case 1: // Lines
          node1 = ElGetElementNodes(i);
          for(j=i+1;j<ElementListLength;j++){
            if(ElGetElementType(j)==ele_type) {
              element = ElGetElement(j);
              if(element->doubled)
                continue;
              node2 = ElGetElementNodes(j);
              if( (node1[0]==node2[0])&&(node1[1]==node2[1]) )
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[1])&&(node1[1]==node2[0]) )
              {
                element->doubled = 1;
                count++;
              }
            }
          }
          break;
        //..................................................................
        case 4: // Triangles - lengthy
          node1 = ElGetElementNodes(i);
          for(j=i+1;j<ElementListLength;j++){
            if(ElGetElementType(j)==ele_type) {
              element = ElGetElement(j);
              if(element->doubled)
                continue;
              node2 = ElGetElementNodes(j);
              if( (node1[0]==node2[0])&&(node1[1]==node2[1])&&(node1[2]==node2[2]) ) 
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[0])&&(node1[1]==node2[2])&&(node1[2]==node2[1]) ) 
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[1])&&(node1[1]==node2[2])&&(node1[2]==node2[0]) ) 
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[1])&&(node1[1]==node2[0])&&(node1[2]==node2[2]) ) 
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[2])&&(node1[1]==node2[0])&&(node1[2]==node2[1]) ) 
              {
                element->doubled = 1;
                count++;
              }
              if( (node1[0]==node2[2])&&(node1[1]==node2[1])&&(node1[2]==node2[0]) ) 
              {
                element->doubled = 1;
                count++;
              }
            }
          }
          break;
        //..................................................................
        case 2: // Quads, orientation
          node1 = ElGetElementNodes(i);
          for(j=i+1;j<ElementListLength;j++){
            if(ElGetElementType(j)==ele_type) {
              element = ElGetElement(j);
              if(element->doubled)
                continue;
              node2 = ElGetElementNodes(j);
              if( (node1[0]==node2[0])&&(node1[1]==node2[1])&&(node1[2]==node2[2])&&(node1[3]==node2[4]) ) {
                element->doubled = 1;
                count++;
                continue;
              }
              if( (node1[0]==node2[1])&&(node1[1]==node2[2])&&(node1[2]==node2[3])&&(node1[3]==node2[0]) ) {
                element->doubled = 1;
                count++;
                continue;
              }
              if( (node1[0]==node2[2])&&(node1[1]==node2[3])&&(node1[2]==node2[0])&&(node1[3]==node2[1]) ) {
                element->doubled = 1;
                count++;
                continue;
              }
              if( (node1[0]==node2[3])&&(node1[1]==node2[0])&&(node1[2]==node2[1])&&(node1[3]==node2[2]) ) {
                element->doubled = 1;
                count++;
                continue;
              }
            }
          }
          break;
      }
    }
  }
  //----------------------------------------------------------------------
  return count;
}

/**************************************************************************
MshLib-Method: ExecuteBowyerWatsonAlgorithm
Task: 
Programing:
05/2004 TK Implementation
**************************************************************************/
void ExecuteBowyerWatsonAlgorithm(double x, double y, double z, string projectname)
{
	int i=0, j=0, k=0;
	double pi = 3.14159265359;
	long number_of_all_elements, number_of_nodes_to_remesh,number_of_all_nodes ;
	long id;
	int treffer=0;
	vector<CMSHElements*> element_vector;
    CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	vector<CMSHNodes*> remesh_nodes_vector;
	CMSHNodes *m_remesh_nodes = NULL;
	vector<CMSHNodes*> nodes_vector;
	CMSHNodes *m_nodes = NULL;
	nodes_vector = MSHGetNodeVector();
	number_of_all_nodes	   = (long)nodes_vector.size();
	number_of_all_elements = (long)element_vector.size();

	//Stop, if Node already exists:
	for (i=0;i<number_of_all_nodes;i++)
	{
		if(x == nodes_vector[i]->x &&
		   y == nodes_vector[i]->y &&
		   z == nodes_vector[i]->z)
	    return;
	}
	

	//Search all Nodes to remesh and write them into the new node vector
	for (i=0;i<number_of_all_elements;i++)
	{
		if(element_vector[i]->element_marker == TRUE)
		{	
			id = element_vector[i]->element_id;
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
			
			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  if (element_vector[i]->node1==remesh_nodes_vector[j]->nodenumber) treffer++;
			}
			if (treffer==0) 
			{
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node1;
				m_remesh_nodes->x =  element_vector[i]->x1;
				m_remesh_nodes->y =  element_vector[i]->y1;
				m_remesh_nodes->z =  element_vector[i]->z1;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
		}
		if(element_vector[i]->element_marker == TRUE)
		{				
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
			
			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  if (element_vector[i]->node2==remesh_nodes_vector[j]->nodenumber) treffer++;
			}
			if (treffer==0) 
			{
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node2;
				m_remesh_nodes->x =  element_vector[i]->x2;
				m_remesh_nodes->y =  element_vector[i]->y2;
				m_remesh_nodes->z =  element_vector[i]->z2;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
		}
		if(element_vector[i]->element_marker == TRUE)
		{				
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
			
			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  if (element_vector[i]->node3==remesh_nodes_vector[j]->nodenumber) treffer++;
			}
			if (treffer==0) 
			{
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node3;
				m_remesh_nodes->x =  element_vector[i]->x3;
				m_remesh_nodes->y =  element_vector[i]->y3;
				m_remesh_nodes->z =  element_vector[i]->z3;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
		}

	}
 			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
			//number_of_nodes_to_remesh is the number of new triangles

    // Construct new Elements

    double a[3],b[3],c[3];
    double a_quantum,b_quantum,c_quantum;
    double cos_alpha,alpha_rad,alpha_deg;
    double cos_beta,cos_gamma, beta_rad,beta_deg,gamma_rad,gamma_deg;
    double min_deg;
    long nnb_a, nnb_b;
    long tri_nb_00=0,tri_nb_01=0,tri_nb_02=0;
    long parent_tri_nb_00=0, parent_tri_nb_01=0, parent_tri_nb_02=0;
    for(i=0; i<3; i++)
    {      
      a[i] = b[i] = c[i] = 0.0;
    }

	for (i=0;i<number_of_nodes_to_remesh;i++)
	{
		nnb_a= remesh_nodes_vector[i]->nodenumber;
		if(i==0)
		{
		nnb_a= remesh_nodes_vector[i]->nodenumber;
		a[0] = (remesh_nodes_vector[i]->x) - x;
		a[1] = (remesh_nodes_vector[i]->y) - y;
		a[2] = (remesh_nodes_vector[i]->z) - z;
		}
		else
		{
          for (k=0;k<number_of_nodes_to_remesh;k++)
		  { 
			  nnb_a= remesh_nodes_vector[k]->nodenumber;
			  if (nnb_a == parent_tri_nb_02)
			  {
				a[0] = (remesh_nodes_vector[k]->x) - x;
				a[1] = (remesh_nodes_vector[k]->y) - y;
				a[2] = (remesh_nodes_vector[k]->z) - z;
				a_quantum = sqrt( (a[0]*a[0]) + (a[1]*a[1]) + (a[2]*a[2]));
				break;
			  }
		  }
		}
		a_quantum = sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
		min_deg = 360;
		for (j=0;j<number_of_nodes_to_remesh;j++)
		{		
			nnb_b= remesh_nodes_vector[j]->nodenumber;
			b[0] = (remesh_nodes_vector[j]->x) - x;
			b[1] = (remesh_nodes_vector[j]->y) - y;
			b[2] = (remesh_nodes_vector[j]->z) - z;
			b_quantum = sqrt( (b[0]*b[0]) + (b[1]*b[1]) + (b[2]*b[2]));

			c[0] = (remesh_nodes_vector[j]->x) - remesh_nodes_vector[k]->x;
			c[1] = (remesh_nodes_vector[j]->y) - remesh_nodes_vector[k]->y;
			c[2] = (remesh_nodes_vector[j]->z) - remesh_nodes_vector[k]->z;
			c_quantum = sqrt( (c[0]*c[0]) + (c[1]*c[1])+  (c[2]*c[2]));

			if (c_quantum>0 && b_quantum>0){
			cos_alpha = ((a_quantum*a_quantum)+(b_quantum*b_quantum)-(c_quantum*c_quantum))/
				         (2*(a_quantum*b_quantum));
			cos_beta = ((c_quantum*c_quantum)+(b_quantum*b_quantum)-(a_quantum*a_quantum))/
				         (2*(b_quantum*c_quantum));
			cos_gamma = ((a_quantum*a_quantum)+(c_quantum*c_quantum)-(b_quantum*b_quantum))/
				         (2*(a_quantum*c_quantum));
			alpha_rad = acos(cos_alpha);
			alpha_deg = ( 180/pi )* acos(cos_alpha);
			beta_rad = acos(cos_beta);
			beta_deg = ( 180/pi )* acos(cos_beta);
			gamma_rad = acos(cos_gamma);
			gamma_deg = ( 180/pi )* acos(cos_gamma);

				if (alpha_deg < min_deg) 
				{
					if (nnb_b != parent_tri_nb_01 && nnb_a != nnb_b)
					{
					min_deg = alpha_deg;
					tri_nb_00 = number_of_all_nodes;
					tri_nb_01 = nnb_a;
					tri_nb_02 = nnb_b;
					}
				}
			}	
		}
 			parent_tri_nb_00 = tri_nb_00;
			parent_tri_nb_01 = tri_nb_01;
			parent_tri_nb_02 = tri_nb_02;

			//Elemente in den Elementvektor eintragen
			number_of_all_elements = (long)element_vector.size();
			m_elements = new CMSHElements;
			m_elements->element_id = number_of_all_elements;
			m_elements->elementtype= 4;
			m_elements->materialnumber= -1;
			m_elements->node1 = tri_nb_00;
			m_elements->node2 = tri_nb_01;
			m_elements->node3 = tri_nb_02;
			element_vector.push_back(m_elements);		
				
	}

	long  check=0;
	//l�chen der markierten Element-Eintr�e
	for (i=0;i<number_of_all_elements;i++)
	{
		if(element_vector[i]->element_marker == TRUE)
		{	
			id = element_vector[i]->element_id;
			element_vector.erase(element_vector.begin()+i);
			i--;
		}
		else
		{
			check++;

		}
	}

    //Knoten in den Knotenvektor eintragen
	if (number_of_nodes_to_remesh > 0)
	{
	m_nodes = new CMSHNodes;
	m_nodes->nodenumber = number_of_all_nodes;
	m_nodes->x =  x;
	m_nodes->y =  y;
	m_nodes->z =  z;
	nodes_vector.push_back(m_nodes);
	}
	//RFI neu schreiben
	string filename = projectname + ".rfi";
	const char *file = 0;
	static char kopf[81];
	long output_nr=0,output_geom=0;
	char rf_version_string[16];
    long nb_prismnodes;
    long nb_elements;
	file = filename.data(); 
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
		nb_prismnodes = (long)nodes_vector.size();
		nb_elements = (long)element_vector.size();
		fprintf(dat_out_rfi,"0 ");
		fprintf(dat_out_rfi,"%ld ",nb_prismnodes);
		fprintf(dat_out_rfi,"%ld \n",nb_elements);

	/*Schreibe Geometrie*/ 
		for (i=0;i<nb_prismnodes;i++)
		{
		fprintf(dat_out_rfi,"%ld ",nodes_vector[i]->nodenumber);
		fprintf(dat_out_rfi,"%20.14f ",nodes_vector[i]->x);
		fprintf(dat_out_rfi,"%20.14f ",nodes_vector[i]->y);
		fprintf(dat_out_rfi,"%20.14f \n",nodes_vector[i]->z);     
		}

	/*Schreibe Topologie*/ 
		for (i=0;i<nb_elements;i++)
		{
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->element_id);
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->materialnumber);
			fprintf(dat_out_rfi,"-1 ");

			fprintf(dat_out_rfi,"tri ");
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->node1);
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->node2);
			fprintf(dat_out_rfi,"%ld \n",element_vector[i]->node3);
		}

	/*Datei Schlie�n*/ 
	fclose(dat_out_rfi);

   	/*Vektoren leeren*/ 
    msh_nodes_vector.clear();
    msh_elements_vector.clear();
	remesh_nodes_vector.clear();
	
	/*Serialisierung der Daten*/ 
	Serialize_RFI (filename);

    nodes_vector.clear();
    element_vector.clear();

}

/**************************************************************************
GeoSys-Method: MSHOpen
Task: 
Programing:
11/2003 OK Implementation
**************************************************************************/
void MSHDestroy()
{
  long i;
  //---------------------------------------------------------------------
  // Destroy NOD data
  if(NODListExists()) {
    for (i=0;i<NodeListSize();i++) {
      if (GetNode(i) != NULL) {                            
        DestroyModelNodeData(i);
      }
    }
    DestroyNodeList();
  }
  //-----------------------------------------------------------------------
  // Destroy ELE data
  if(ELEListExists()) {
    ELEDestroyPtrArray();
  }
}

