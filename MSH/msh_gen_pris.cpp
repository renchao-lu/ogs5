/**************************************************************************
PRISM GENERATOR

1. Schritt 3/2003  TK => Eingabe und Ausgabe der RFI-Dateien
10/2004 OK msh_nodes_vector - name conflicts
**************************************************************************/
//------------------------------------------------------------------------
// MFC
#include "stdafx.h"
//------------------------------------------------------------------------
// C/C++
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//using namespace std;
//------------------------------------------------------------------------
// MSH
#include "msh_elem.h"
using namespace Mesh_Group;
//------------------------------------------------------------------------
#include "msh_gen_pris.h"

//static ZeigerFeld *nodelist = NULL;

// Vector/List
vector<CMSHNodesPrisGen*>msh_pris_nodes_vector; //OK
CMSHNodesPrisGen *m_msh_nodes = NULL;
// constructor
CMSHNodesPrisGen::CMSHNodesPrisGen(void)
{
}
// destructor
CMSHNodesPrisGen::~CMSHNodesPrisGen(void)
{
}
// Vector/List
vector<CMSHElementsPrisGen*> msh_pris_elements_vector; //OK
CMSHElementsPrisGen *m_msh_elements = NULL;
// constructor
CMSHElementsPrisGen::CMSHElementsPrisGen(void)
{
}
// destructor
CMSHElementsPrisGen::~CMSHElementsPrisGen(void)
{
}



/******************************************************************************/

void MSH_GetRFINodes (CFEMesh*);
void MSH_GetRFIElements (CFEMesh*);
void MSH_CreateFirstPrismlayer (double thickness_prism_layers, CFEMesh*);
void MSH_CreateFirstHexLayer (double thickness_layers, CFEMesh*);
void MSH_CopyFirstPrismlayer (long nb_prism_layers);
void MSH_CopyFirstLayer (long nb_prism_layers);
void MSH_Destroy_PrismDataConstructs ();

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**/
/*=*=*=*=*=*=*=*=*=*=*=                   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=* F U N C T I O N S =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*=*=*=*=*=*=*=*=*=*=*=                   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**/
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**************************************************************************/
/* PRISGEN - Funktion: Create_Rectangles2Triangles
                                                                          */
/* Aufgabe:                                                               */
/* Programmaenderungen:
   6/2005     TK        Erste Version
**************************************************************************/
void Create_Quads2Tri(CFEMesh*m_msh)
{	
  MSH_GetRFINodes(m_msh);
  MSH_GetRFIElements(m_msh);
  MSH_Tri_from_Quads();
}


/**************************************************************************/
/* PRISGEN - Funktion: Create_Triangles2Prisms
                                                                          */
/* Aufgabe: Erstellt aus einem 2D (x,y) Dreiecksgitter ein 3D
            Prismengitter, anhand der Anzahl der Prismenschichten und deren 
			Mächtigkeit.
                                                                          */
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void Create_Triangles2Prisms(long nb_prism_layers,double thickness_prism_layers,CFEMesh*m_msh)
{	
  MSH_GetRFINodes(m_msh);
  MSH_GetRFIElements(m_msh);
  MSH_CreateFirstPrismlayer (thickness_prism_layers,m_msh);
  MSH_CopyFirstPrismlayer (nb_prism_layers);
}

/**************************************************************************/
/* PRISGEN - Funktion: Create_Quads2Hex
                                                                          */
/* Aufgabe: Erstellt aus einem 2D (x,y) Rechtecksgitter ein 3D
            Prismengitter, anhand der Anzahl der Prismenschichten und deren 
			Mächtigkeit.
                                                                          */
/* Programmaenderungen:
   6/2005     TK        Erste Version
**************************************************************************/
void Create_Quads2Hex(long nb_layers,double thickness_layers,CFEMesh*m_msh)
{	
  MSH_GetRFINodes(m_msh);
  MSH_GetRFIElements(m_msh);
  MSH_CreateFirstHexLayer (thickness_layers,m_msh);
  MSH_CopyFirstLayer (nb_layers);
}


/**************************************************************************/
/* PRISGEN - Funktion: MSH_GetRFINodes 
                                                                          */
/* Aufgabe: Liest Knoten aus dem Rockflow-Node-Array und kopiert diese
            in einen C++ Standard Vektor
                                                                          */
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void MSH_GetRFINodes (CFEMesh*m_msh)
{	
  int i=0;

  for (i=0;i<(int)msh_pris_nodes_vector.size();i++)
  {
        delete msh_pris_nodes_vector[i];
  }
  msh_pris_nodes_vector.clear();

  for (i=0;i<(long)m_msh->nod_vector.size();i++){
     m_msh_nodes = new CMSHNodesPrisGen;
     m_msh_nodes->rfi_node_id = i;
     m_msh_nodes->x = m_msh->nod_vector[i]->X();
     m_msh_nodes->y = m_msh->nod_vector[i]->Y();
	 m_msh_nodes->z = m_msh->nod_vector[i]->Z();
     m_msh_nodes->tri_mesh_number= 0;
     msh_pris_nodes_vector.push_back(m_msh_nodes);
  }

}
/**************************************************************************/
/* PRISGEN - Funktion: MSH_GetRFIElements 
                                                                          */
/* Aufgabe: Liest Knoten aus der Rockflow-Elementliste und kopiert diese
            in einen C++ Standard Vektor
                                                                          */
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void MSH_GetRFIElements (CFEMesh*m_msh)
{
  // Not used: static long *element_nodes;
  int i=0;
  int first_ele_type = 0;
  CElem* m_ele = NULL; //OKWW
  for (i=0;i<(int)msh_pris_elements_vector.size();i++)
  {
    delete msh_pris_elements_vector[i];
  }
  msh_pris_elements_vector.clear();

  if ((int)m_msh->ele_vector.size() > 0)
  first_ele_type =  m_msh->ele_vector[0]->GetElementType();

    for (i=0;i<(long)m_msh->ele_vector.size();i++)
    {  
        if(m_msh->ele_vector[i]->GetElementType() == first_ele_type){
        m_ele = m_msh->ele_vector[i];
        //OKWW element_nodes = m_ele->nodes;
        m_msh_elements = new CMSHElementsPrisGen;
        m_msh_elements->rfi_element_id = i;
        m_msh_elements->elementtype = m_ele->GetElementType();
        m_msh_elements->node1 = m_ele->nodes_index[0]; //OKWW
        m_msh_elements->node2 = m_ele->nodes_index[1];
        m_msh_elements->node3 = m_ele->nodes_index[2];
        m_msh_elements->node4 = m_ele->nodes_index[3];
        m_msh_elements->node5 = m_ele->nodes_index[4];
        m_msh_elements->node6 = m_ele->nodes_index[5];
        m_msh_elements->node7 = m_ele->nodes_index[6];
        m_msh_elements->node8 = m_ele->nodes_index[7];       
        m_msh_elements->layernumber = 0;
	    m_msh_elements->materialnumber= 0;  
	    msh_pris_elements_vector.push_back(m_msh_elements);
        }
    }
}
/*************************************************************************
 PRISGEN - Funktion: MSH_CreateFirstPrismlayer
                                                                          
 Aufgabe: Berechnet neue Knoten und setzt sie in KnotenVektor
           Erstellt neue Prismen-Elemente und überschreibt Element-Vektor                                                                          
 Programmaenderungen:
   10/2003     TK        Erste Version
   03/2004 OK treat only triangles
**************************************************************************/
void MSH_CreateFirstPrismlayer (double thickness_prism_layers, CFEMesh*m_msh)
{
long i=0; 
long nodelistlength;

if(m_msh){
  nodelistlength = (long)m_msh->nod_vector.size();
}

else{
  nodelistlength =  NodeListSize ();
}
  /*Add Nodes:*/
  for (i=0;i<nodelistlength;i++){
    m_msh_nodes = new CMSHNodesPrisGen;
    m_msh_nodes->rfi_node_id = i + nodelistlength;
    m_msh_nodes->x = m_msh->nod_vector[i]->X();
    m_msh_nodes->y = m_msh->nod_vector[i]->Y();
	   m_msh_nodes->z = m_msh->nod_vector[i]->Z() - thickness_prism_layers;
    m_msh_nodes->tri_mesh_number= 1;
    msh_pris_nodes_vector.push_back(m_msh_nodes);
  }

  /*Generate New Elements:*/ 
  //OK long elementlistlength =  ElListSize ();
  long msh_triangle_elements_vector_length = (long)msh_pris_elements_vector.size();
  //for (i=0;i<elementlistlength;i++)
for (i=0;i<msh_triangle_elements_vector_length;i++)  {
	msh_pris_elements_vector[i]->rfi_element_id = i;
	msh_pris_elements_vector[i]->elementtype = 6;
    msh_pris_elements_vector[i]->layernumber = 0;
    msh_pris_elements_vector[i]->materialnumber = 0;
    msh_pris_elements_vector[i]->node4 = (msh_pris_elements_vector[i]->node1)+nodelistlength;
    msh_pris_elements_vector[i]->node5 = (msh_pris_elements_vector[i]->node2)+nodelistlength;
    msh_pris_elements_vector[i]->node6 = (msh_pris_elements_vector[i]->node3)+nodelistlength;
}
  
}


/**************************************************************************
 PRISGEN - Funktion: MSH_Tri_from_Quads
                                                                          
 Aufgabe: Berechnet neue Knoten und setzt sie in KnotenVektor
            Erstellt neue Hex-Elemente und überschreibt Element-Vektor 
 Programmaenderungen:
   10/2003     TK        Erste Version
   03/2004 OK treat only triangles
**************************************************************************/
void MSH_Tri_from_Quads ()
{
	long i=0;

    /*Add Elements:*/ 
    int msh_triangle_elements_vector_length = (int)msh_pris_elements_vector.size();
		for (i=0;i<msh_triangle_elements_vector_length;i++)
		{
          msh_pris_elements_vector[i]->rfi_element_id = i;
		  msh_pris_elements_vector[i]->elementtype = 4;
		  msh_pris_elements_vector[i]->materialnumber= 0;  	
		}
  
		for (i=0;i<msh_triangle_elements_vector_length;i++)
		{
          m_msh_elements = new CMSHElementsPrisGen;
          m_msh_elements->rfi_element_id = i + msh_triangle_elements_vector_length;
		  m_msh_elements->elementtype = 4;
		  m_msh_elements->materialnumber= 0;  	
		  m_msh_elements->node1 = msh_pris_elements_vector[i]->node1;
		  m_msh_elements->node2 = msh_pris_elements_vector[i]->node3;
		  m_msh_elements->node3 = msh_pris_elements_vector[i]->node4; 
		  msh_pris_elements_vector.push_back(m_msh_elements);
		}
  
}


/**************************************************************************
 PRISGEN - Funktion: MSH_CreateFirstHexlayer
                                                                          
 Aufgabe: Berechnet neue Knoten und setzt sie in KnotenVektor
            Erstellt neue Hex-Elemente und überschreibt Element-Vektor 
 Programmaenderungen:
   10/2003     TK        Erste Version
   03/2004 OK treat only triangles
**************************************************************************/
void MSH_CreateFirstHexLayer (double thickness_prism_layers, CFEMesh*m_msh)
{
long i=0; 
  
  /*Add Nodes:*/
  long nodelistlength = (long)m_msh->nod_vector.size();
  for (i=0;i<(long)m_msh->nod_vector.size();i++){
    m_msh_nodes = new CMSHNodesPrisGen;
    m_msh_nodes->rfi_node_id = i + nodelistlength;
    m_msh_nodes->x = m_msh->nod_vector[i]->X();
    m_msh_nodes->y = m_msh->nod_vector[i]->Y();
	m_msh_nodes->z = m_msh->nod_vector[i]->Z() - thickness_prism_layers;
    m_msh_nodes->tri_mesh_number= 1;
    msh_pris_nodes_vector.push_back(m_msh_nodes);
  }

  /*Add Elements:*/
    for (i=0;i<(long)msh_pris_elements_vector.size();i++)  {
	    msh_pris_elements_vector[i]->rfi_element_id = i;
	    msh_pris_elements_vector[i]->elementtype = 3;
        msh_pris_elements_vector[i]->layernumber = 0;
        msh_pris_elements_vector[i]->materialnumber = 0;
        msh_pris_elements_vector[i]->node5 = (msh_pris_elements_vector[i]->node1)+nodelistlength;
        msh_pris_elements_vector[i]->node6 = (msh_pris_elements_vector[i]->node2)+nodelistlength;
        msh_pris_elements_vector[i]->node7 = (msh_pris_elements_vector[i]->node3)+nodelistlength;
        msh_pris_elements_vector[i]->node8 = (msh_pris_elements_vector[i]->node4)+nodelistlength;
    }
  
}



/**************************************************************************
 PRISGEN - Funktion: MSH_CopyFirstPrismlayer
                                                                          
 Aufgabe: Setzt eine neue, bestimmte Anzahl von PrismenSchichten
            Berechnet neue Knoten und setzt neue Elemente

 Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void MSH_CopyFirstPrismlayer (long nb_prism_layers)
{
	long i=0;
	long k=0;
    long nb_trianglenodes = (long)msh_pris_nodes_vector.size()/2;
    long nb_ele_1stlayer = (long)msh_pris_elements_vector.size();
	double thickness = fabs((msh_pris_nodes_vector[0]->z)-(msh_pris_nodes_vector[nb_trianglenodes]->z));

    /*Add Nodes:*/ 
	for (k=0;k<nb_prism_layers-1;k++)
	{
		for (i=0;i<nb_trianglenodes;i++)
		{
		 m_msh_nodes = new CMSHNodesPrisGen;
		 m_msh_nodes->rfi_node_id = i+((k+2)*nb_trianglenodes);
		 m_msh_nodes->x = msh_pris_nodes_vector[i]->x;
		 m_msh_nodes->y = msh_pris_nodes_vector[i]->y;
         m_msh_nodes->z = msh_pris_nodes_vector[i]->z - ((k+2)*thickness);
    	 m_msh_nodes->tri_mesh_number= k+2;
		 msh_pris_nodes_vector.push_back(m_msh_nodes);
		}
	}    
    /*Add Elements:*/ 
	for (k=0;k<nb_prism_layers-1;k++)
	{
		for (i=0;i<nb_ele_1stlayer;i++)
		{
          m_msh_elements = new CMSHElementsPrisGen;
          m_msh_elements->rfi_element_id = i+((k+1)*nb_ele_1stlayer);
		  m_msh_elements->elementtype = 6;
          m_msh_elements->layernumber = k+1;
		  m_msh_elements->materialnumber= k+1;  	
		  m_msh_elements->node1 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node4;
		  m_msh_elements->node2 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node5;
		  m_msh_elements->node3 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node6; 
		  m_msh_elements->node4 = nb_trianglenodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node4;
		  m_msh_elements->node5 = nb_trianglenodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node5;
		  m_msh_elements->node6 = nb_trianglenodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node6;  
		  msh_pris_elements_vector.push_back(m_msh_elements);
		}
	}
}

/**************************************************************************
 PRISGEN - Funktion: MSH_CopyFirstLayer
                                                                          
 Aufgabe: Setzt eine neue, bestimmte Anzahl von HEX-Schichten
            Berechnet neue Knoten und setzt neue Elemente
 Programmaenderungen:
   06/2005     TK        Erste Version
**************************************************************************/
void MSH_CopyFirstLayer (long nb_prism_layers)
{
	long i=0;
	long k=0;
    long nb_nodes = (long)msh_pris_nodes_vector.size()/2;
    long nb_ele_1stlayer = (long)msh_pris_elements_vector.size();
	double thickness = fabs((msh_pris_nodes_vector[0]->z)-(msh_pris_nodes_vector[nb_nodes]->z));

    /*Add Nodes:*/ 
	for (k=0;k<nb_prism_layers-1;k++)
	{
		for (i=0;i<nb_nodes;i++)
		{
		 m_msh_nodes = new CMSHNodesPrisGen;
		 m_msh_nodes->rfi_node_id = i+((k+2)*nb_nodes);
		 m_msh_nodes->x = msh_pris_nodes_vector[i]->x;
		 m_msh_nodes->y = msh_pris_nodes_vector[i]->y;
         m_msh_nodes->z = msh_pris_nodes_vector[i]->z - ((k+2)*thickness);
    	 m_msh_nodes->tri_mesh_number= k+2;
		 msh_pris_nodes_vector.push_back(m_msh_nodes);
		}
	}    
    /*Add Elements:*/ 
	for (k=0;k<nb_prism_layers-1;k++)
	{
		for (i=0;i<nb_ele_1stlayer;i++)
		{
          m_msh_elements = new CMSHElementsPrisGen;
          m_msh_elements->rfi_element_id = i+((k+1)*nb_ele_1stlayer);
		  m_msh_elements->elementtype = 6;
          m_msh_elements->layernumber = k+1;
		  m_msh_elements->materialnumber= k+1;  	
		  m_msh_elements->node1 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node5;
		  m_msh_elements->node2 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node6;
		  m_msh_elements->node3 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node7; 
		  m_msh_elements->node4 = msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node8;
		  m_msh_elements->node5 = nb_nodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node5;
		  m_msh_elements->node6 = nb_nodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node6;  
		  m_msh_elements->node7 = nb_nodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node7;  
		  m_msh_elements->node8 = nb_nodes + msh_pris_elements_vector[i+((k)*nb_ele_1stlayer)]->node8;  
		  msh_pris_elements_vector.push_back(m_msh_elements);
		}
	}
}


/**************************************************************************
 PRISGEN - Funktion: MSH_OverWriteMSH
                                                                          
 Aufgabe: Schreibt ein neues MSH-File 

 Programmaenderungen:
   10/2003     TK        Erste Version
   10/2005     TK        Changed to MSH
**************************************************************************/
void MSH_OverWriteMSH (string m_filepath, long numberofprismlayers)
{
    long i=0; 
    long nb_prismnodes;
    long nb_elements;
    const char *filepath = 0; 
    filepath = m_filepath.data();

    nb_prismnodes = (long)msh_pris_nodes_vector.size();
    nb_elements = (long)msh_pris_elements_vector.size();

    /*Datei anlegen*/ 
    FILE *msh_file=NULL;
    msh_file = fopen(filepath, "w+t");

    //write MSH Head
	   	fprintf( msh_file, "%s\n", "#FEM_MSH");
    //write PCS Type
        fprintf( msh_file, "%s\n", " $PCS_TYPE");
	   	fprintf( msh_file, "%s\n", "  NO_PCS"); //TODO
    //Write Nodes
	   	fprintf( msh_file, "%s\n", " $NODES");
        fprintf(msh_file,"% ld\n",nb_prismnodes);
    //write Geometry    
    for (i=0;i<nb_prismnodes;i++)
	{
      fprintf(msh_file,"%ld ",i);
      fprintf(msh_file,"%20.14f ",msh_pris_nodes_vector[i]->x);
      fprintf(msh_file,"%20.14f ",msh_pris_nodes_vector[i]->y);
      fprintf(msh_file,"%20.14f \n",msh_pris_nodes_vector[i]->z);     
	}
    //Write Elements
      fprintf(msh_file, "%s\n", " $ELEMENTS");
	  fprintf(msh_file,"% ld\n",nb_elements);
    /*Schreibe Topologie*/ 
    long element_type = msh_pris_elements_vector[0]->elementtype;

    switch (element_type) {
      case 1:  /* line */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"line ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node2);
	            }
              break;
      case 2: /* quadrilaterals */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"quad ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node2);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node3);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node4);
	            }
              break;
      case 3: /* hexahedra */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"hex ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node2);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node3);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node4);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node5);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node6);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node7);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node8);     
	            }
              break;
      case 4: /* triangle */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"tri ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node2);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node3);
	            }
              break;
      case 5: /* tetrahedra */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"tet ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node2);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node3);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node4);
	            }
              break;
      case 6: /* prism */
                for (i=0;i<nb_elements;i++)
	            {
                fprintf(msh_file,"%ld ",i);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->materialnumber);
	            fprintf(msh_file,"-1 ");
	            fprintf(msh_file,"pris ");
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node1);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node2);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node3);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node4);
                fprintf(msh_file,"%ld ",msh_pris_elements_vector[i]->node5);
                fprintf(msh_file,"%ld \n",msh_pris_elements_vector[i]->node6);
	            }
              break;

    }

    //Write STOP
   	fprintf( msh_file, "%s\n", " $LAYER");
   	fprintf( msh_file, "%  d\n", numberofprismlayers);
    fprintf( msh_file, "%s\n", "#STOP");

	/*Datei Schließen*/ 
	fclose(msh_file);
    MSH_Destroy_PrismDataConstructs ();
    // -> void CPRISGEN::OnCancel() 
}



/**************************************************************************
 PRISGEN - Funktion: MSH_Destroy_PrismDataConstructs
                                                                          
 Aufgabe: Zerstört/löscht die Inhalte der Vektoren

 Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void MSH_Destroy_PrismDataConstructs ()
{
  msh_pris_nodes_vector.clear();
  msh_pris_elements_vector.clear();
}


/**************************************************************************
MshLib-Method: 
Task: 
Programing:
03/2004 OK Implementation
**************************************************************************/
void MSHCreatePrismsFromTriangles(void)
{
  //-----------------------------------------------------------------------
  long *nodes = NULL;
  long i;
  int mat_group = 0;
  long no_prisms = (long)msh_pris_elements_vector.size();
  for(i=0;i<no_prisms;i++) {
      nodes = (long*) Malloc(sizeof(long)*ElNumberOfNodes[5]);
      nodes[0] = msh_pris_elements_vector[i]->node1;
      nodes[1] = msh_pris_elements_vector[i]->node2;
      nodes[2] = msh_pris_elements_vector[i]->node3;
      nodes[3] = msh_pris_elements_vector[i]->node4;
      nodes[4] = msh_pris_elements_vector[i]->node5;
      nodes[5] = msh_pris_elements_vector[i]->node6;
//OK      ELECreateTopology(elementtyp,vorgaenger,level,ElementListLength);
      ElSetElementNodes(ElementListLength-1,nodes);
      ElSetElementGroupNumber(ElementListLength-1,mat_group);
      anz_3D++;
  }
  //-----------------------------------------------------------------------
  msh_no_pris = no_prisms;
}
