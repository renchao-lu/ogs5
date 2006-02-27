/**************************************************************************/
/* ROCKFLOW - Modul:  msh_nodes.c
                                                                          */
/* Task:
   Relationships between MSH and NOD
                                                                          */
                                                                                                                                                    
/* Programming:
   09/2002   MB   First Version
   11/2002   OK   
   03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt   
   05/2003 OK GEOConfigELEGeometry() -> ELEConfigJacobianMatrix
   11/2003 OK rf_geo -> msh_nodes
                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
#include "makros.h"
// C++ STL
#include <list>
#include <iostream>
using namespace std;
/* GEOLib */
#include "geo_sfc.h"
#include "geo_vol.h"
#include "geo_dom.h"
/* MSHLib */
#include "msh_nodes_rfi.h"
#include  "msh_lib.h"//CC
/* FEMLib */
#include "nodes.h"
#include "elements.h"           /* für ElGetElementGroupNumber */
#include "edges.h"              /* für GetEdge */
#include "cgs_asm.h" 
 /* Tools */
#include "mathlib.h"            /* für MAngleVectors */
#include "tools.h"
 /* Models */
#include "rf_pcs.h" //OK_MOD"
#include "rf_mmp_new.h"


// Vector/List
vector<CMSHNodes*> msh_nodes_vector;
// constructor
CMSHNodes::CMSHNodes(void)
{
  selected = true; //OK
  x=y=z=0.0; //OK
}
CMSHNodes::~CMSHNodes(void)
{
  //ele_number_vector.clear();
}

/**************************************************************************/
/* ROCKFLOW - Function: MSHGetNextNode
                                                                          */
/* Task:
   Find the next node to the starting node in user defined direction
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
   I: startnode, direction
                                                                          */
/* Return:
   nextnode
                                                                          */
/* Programming:
   09/2002     MB         First Version
   08/2005     MB
                                                                          */
/**************************************************************************/
long MSHGetNextNode (long startnode, CFEMesh* m_msh)
{
  long nextnode;

  long NumberOfNodes;
  long NumberOfNodesPerLayer;
  int NumberOfLayers;

  NumberOfNodes = (long)m_msh->nod_vector.size();
  NumberOfLayers = m_msh->no_msh_layer;
  
  NumberOfNodesPerLayer = NumberOfNodes / (NumberOfLayers + 1);  

  nextnode = startnode + NumberOfNodesPerLayer;

  return nextnode; 
}


/**************************************************************************
   ROCKFLOW - Function: MSHGetNeighborNodes
   
   Task: Find all neighbor nodes of a given nodes along its edges  
         
   Parameter: (I: Input; R: Return; X: Both)
           I: startnode
           
   Return:
           *neighbor_nodes
   
   Programming:
   09/2002   MB   First Version
 **************************************************************************/
long* MSHGetNeighborNodes(long startnode)
{
  long *kanten=NULL;
  int anz_alle_kanten;
  long nd1[2];  
  int i;
  int j =0;
  long* neighbor_nodes =NULL;

  kanten = EDGGetNodeVerticalEdges(startnode, &anz_alle_kanten);


  /* Alle benachbarten Knoten holen */
  for (i = 0; i< anz_alle_kanten; i++)  {
    /* Knotennummern von Kante holen */
    nd1[0] = GetEdge(kanten[i]) -> knoten[0];
    nd1[1] = GetEdge(kanten[i]) -> knoten[1];
    
    j++;
    neighbor_nodes=(long*)Realloc(neighbor_nodes, j*sizeof(long));
    /* Aber nicht den Startknoten nehmen */
    if (nd1[0] == startnode) {
      neighbor_nodes[i] = nd1[1];
    } 
    else {
      neighbor_nodes[i] = nd1[0];
    }
  } /* end for i */
  kanten = (long*) Free(kanten);
  return neighbor_nodes;
}



/**************************************************************************
   ROCKFLOW - Function: MSHGetNodesInColumn
   
   Task:  
   Gets nodes of a column searching downward from startnode. 
         
   Parameter: (I: Input; R: Return; X: Both)
           I: long node, int anz_zeilen
           
   Return:
           *long strang 
   
   Programming:
   09/2002   MB   First Version
   08/2005   MB   m_msh
 **************************************************************************/
long* MSHGetNodesInColumn(long nextnode, int anz_zeilen, CFEMesh* m_msh)
{
  int i;
  long startnode;
  long *strang=NULL;

  for (i = 0; i< anz_zeilen +1; i++)  {
    strang = (long*) Realloc(strang,(i+1)*sizeof(long));         
    strang[i] = nextnode;
    startnode = nextnode;
    //nextnode = MSHGetNextNode (startnode, direction);
    nextnode = MSHGetNextNode (startnode, m_msh);
  } 
  return strang;  
}


/***************************************************************************
   ROCKFLOW - Funktion: MSHCalcTriangleArea3

   Aufgabe: Berechnet die Flaeche eines vorgegebenen Dreiecks befindet

   Formalparameter:
           E long n1, n2, n3: Knoten des Dreiecks

   Ergebnis: Dreiecksflaeche


   Aenderungen/Korrekturen:
   05/2003          MB, nach GEOCalcTriangleArea2
 **************************************************************************/
double MSHCalcTriangleArea3 (double* node1, double* node2, double* node3)
{
  double vec1[3], vec2[3], vec3[3];
  double area;

  vec1[0] = node2[0] - node1[0];
  vec1[1] = node2[1] - node1[1];
  vec1[2] = node2[2] - node1[2];

  vec2[0] = node1[0] - node3[0];
  vec2[1] = node1[1] - node3[1];
  vec2[2] = node1[2] - node3[2];

  M3KreuzProdukt(vec1,vec2,vec3);

  area = 0.5 * MBtrgVec(vec3,3);

  return area;
}


/***************************************************************************
   ROCKFLOW - Funktion: MSHDeleteElementFromList

   Task: 

   Parameter:

   Return:

   Programing:
   09/2002   OK   Implementation

 **************************************************************************/
void MSHDeleteElementFromList (long element_start,long element_end)
{
  long i,j,k,l;
  int k0;
  long *element_nodes;
  long array_length=0l;
  long this_node;
  long *array=NULL;
  int delete_this_node = 0;  
  long old_node_array_length;
  long *node_table;
  long *new_element_nodes;
  Element *element=NULL;
  long *node_array_delete=NULL;
  long number_of_nodes_delete;
  int nn;

  /* 1 Array of nodes belonging to element to be deleted */
  /* 1 Delete elements from element array */
  k0 = ElNumberOfNodes[ElGetElementType(element_start)-1];
  for (i=element_start;i<element_end+1;i++) {
    element_nodes = ElGetElementNodes(i);
    k = ElNumberOfNodes[ElGetElementType(i)-1];
    array_length += k;
    array = (long*) Realloc(array,array_length*sizeof(long));
    for (j=0;j<k;j++)
      array[array_length+j-k0]=element_nodes[j];
    ElDeleteElement(i);
  }


  /* 2 Reduce and renumber element array */
  ElCompressElementList();
  for (i=0;i<ElListSize();i++) {
    element = ElGetElement(i);
    ElSetElementStartNumber(i,i);
  }

  /* 3 Search for unconnected nodes */
  old_node_array_length = NodeListLength;
  l=0;
  for (i=0;i<array_length;i++) {
    this_node = array[i];
    for (j=0;j<ElListSize();j++) {
      element_nodes = ElGetElementNodes(j);
      for (k=0;k<ElNumberOfNodes[ElGetElementType(j)-1];k++) {
        delete_this_node = 1;
        if (this_node==element_nodes[k]) {
          delete_this_node = 0;
          break;
        }
      if(!delete_this_node) break;
      }
    if(!delete_this_node) break;
    }
    if(GetNode(this_node)&&delete_this_node) {
      node_array_delete = (long*) Realloc(node_array_delete,++l*sizeof(long));
      node_array_delete[l] = this_node;
      NODDeleteNodeGeometric(this_node);
    }
  }
  number_of_nodes_delete = l;

/*
  new_node_array_length = old_node_array_length - number_of_nodes_delete;
  for (i=0;i<number_of_nodes_delete;i++) {
    if (node_array_delete[i]>new_node_array_length) {
      node_array_available[j] = node_array_delete[i];
    }
  }
*/


  /* 4 Renumber nodes */
  j=0;
  node_table = (long *) Malloc(NodeListSize()*sizeof(long));
  for (i=0;i<NodeListSize();i++) {
    if (GetNode(i)!=NULL) {
      node_table[i] = j;
      j++;
    }
  }

  /* 5 New element nodes */
  for (i=0;i<ElListSize();i++) {
    nn = ElNumberOfNodes[ElGetElementType(i)-1];
    element_nodes = ElGetElementNodes(i);
    new_element_nodes = (long*) Malloc(nn*sizeof(long));
    for (j=0;j<nn;j++) {
      new_element_nodes[j] = node_table[element_nodes[j]];
    }
    ElSetElementNodes(i,new_element_nodes);
    new_element_nodes = ElGetElementNodes(i);
  }

  CompressNodeList();

  array = (long*) Free(array);
  node_table = (long *) Free(node_table);

}

/**************************************************************************
FEMLib-Method: 
Task: Searches mobile nodes and sets node->free_surface = 4
Programing:
09/2004 OK / MB Implementation
05/2005 OK Bugfix
07/2005 MB MMP keyword
08/2005 MB m_pcs
**************************************************************************/
void MSHDefineMobile(CRFProcess*m_pcs)
{
  long* mobile_nodes = NULL;
  long no_mobile_nodes = -1;
  long i;
  CMediumProperties *m_mat_mp = NULL;

  //----------------------------------------------------------------------
  // Define mobile MSH nodes
  //----------------------------------------------------------------------
  // MMP Groups
  if(mmp_vector.size()==0) return;
  ////Schleife über alle Gruppen
  for(i=0;i<(long) mmp_vector.size();i++){
    m_mat_mp = mmp_vector[i];
    
    if (m_mat_mp->unconfined_flow_group ==1 && m_pcs->m_msh->GetMaxElementDim() == 3){
      //....................................................................
      //DOMAIN
      if(m_mat_mp->geo_type_name.find("DOMAIN")!=string::npos){
        //CGLDomain *m_domain = NULL;
        //m_domain = m_domain->Get(m_mat_mp->geo_name);
        //mobile_nodes = m_domain->GetPointsIn(&no_mobile_nodes);
        //ToDo einlesen von domains ????
        if (m_pcs->m_msh){
          for(i=0; i < (long)m_pcs->m_msh->nod_vector.size(); i++) {
            mobile_nodes = (long *) Realloc(mobile_nodes,sizeof(long)*(i+1));
            mobile_nodes[i] = i;
          }
          no_mobile_nodes = (long)m_pcs->m_msh->nod_vector.size();
        }
        else{
          for(i=0; i<NodeListLength; i++){
            mobile_nodes = (long *) Realloc(mobile_nodes,sizeof(long)*(i+1));
            mobile_nodes[i] = i;
          }
        no_mobile_nodes = NodeListLength;
        }    
      }
      //....................................................................
      //LAYER
      if(m_mat_mp->geo_type_name.find("LAYER")!=string::npos){
        //TODO next version, change to msh file !!!
      }
      //....................................................................
      //SURFACE 
      if(m_mat_mp->geo_type_name.find("SURFACE")!=string::npos){
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(m_mat_mp->geo_name);//CC
        mobile_nodes = GetPointsIn(m_surface,&no_mobile_nodes);//CC
      }
      //....................................................................
      //VOLUME
      if(m_mat_mp->geo_type_name.find("VOLUME")!=string::npos){
        CGLVolume *m_volume = NULL;
        m_volume = GEOGetVOL(m_mat_mp->geo_name);//CC 10/05
        //ToDo TK
        mobile_nodes =GetPointsInVolume(m_volume,&no_mobile_nodes);//CC 10/05
      }
  
    } //end if unconfined flow group

  } //end for mmp vector

  //----------------------------------------------------------------------
  // Set mobile MSH nodes flag
  for(i=0;i<no_mobile_nodes;i++){
    m_pcs->m_msh->nod_vector[i]->free_surface = 4;
  }
  //----------------------------------------------------------------------
  if (no_mobile_nodes > 0)  {
    m_pcs->mobile_nodes_flag = 1;
    MSHSelectFreeSurfaceNodes(m_pcs->m_msh);
  }
  
}


/**************************************************************************/
/* ROCKFLOW - Function: MSHSelectFreeSurfaceNodes
                                                                          */
/* Task:
   Selection of free surface nodes, i.e. Setting free surface node flag = 1
   for the uppermost row and free surface node flag = 2 for the lowermost
   row. (Lowermost row is not moving)
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   - void -
                                                                          */
/* Programming:
   03/2003     MB   First Version
   09/2004     MB   PCS   
   08/2005     MB msh    
                                                                          */
/**************************************************************************/
void MSHSelectFreeSurfaceNodes (CFEMesh* m_msh)
{
  long i;
  int j =0;
  long startnode;
  long nextnode;
  long *strang=NULL;
  long NumberOfNodes;
  long NumberOfNodesPerLayer;
  int NumberOfLayers;

  // Number of nodes per node layer
  NumberOfNodes = (long)m_msh->nod_vector.size();
  NumberOfLayers = m_msh->no_msh_layer;
  NumberOfNodesPerLayer = NumberOfNodes / (NumberOfLayers + 1);  
  int no_unconfined_layer = 0;
  // create array with nodes in vertical column
  for (i = 0; i < NumberOfNodesPerLayer; i++) {
   
    if(m_msh->nod_vector[i]->free_surface == 4){
      nextnode = i; 
      no_unconfined_layer = 0;  
      for (j=0; j < m_msh->no_msh_layer; j++) {
        strang = (long*) Realloc(strang,(j+1)*sizeof(long));     
        strang[j] = nextnode;
        startnode = nextnode;
        nextnode = MSHGetNextNode (startnode, m_msh);
        if(m_msh->nod_vector[nextnode]->free_surface == 4)
        {
          strang[j+1] = nextnode;
          no_unconfined_layer++;
        }
        else  {
          continue;
        }
      } 
    } //endif free_surface==4
    
    // mark start of vertical column with 1 - end of column with 2
    // this is than used in MSHMoveNODUcFlow 
    m_msh->nod_vector[strang[0]]->free_surface = 1;
    m_msh->nod_vector[strang[no_unconfined_layer]]->free_surface = 2;

   
  } /*endfor*/ 
}


/**************************************************************************/
/* ROCKFLOW - Function: MSHMoveNODUcFlow
                                                                          */
/* Task: 
   Moves free surface nodes according to the pressure distribution
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   - void -
                                                                          */
/* Programming:
   09/2002     MB       First Version
   05/2003     MB       verallgemeinert für Prismen und Vierecke 
   09/2004     MB       Methode vereinfacht
   09/2004     MB       PCS    
  08/2005      MB       m_msh                                                                   */
/**************************************************************************/
void MSHMoveNODUcFlow (CRFProcess*m_pcs)
{
  long nextnode = -1;
  long startnode;
  int index, index2;
  long node;
  int anz_zeilen = 0;
  int i;
  double spanne_ges;
  double spanne_rel;
  long* strang=NULL;
  double head =0.0;
  int xxflag;
  int nidy;
  long NumberOfNodes;
  long NumberOfNodesPerLayer;
  int NumberOfLayers;
  double MinThickness = 1e-1; //OKMB
  double z_bottom; //OKMB

  // Number of nodes per node layer
  NumberOfNodes = (long)m_pcs->m_msh->nod_vector.size();
  NumberOfLayers = m_pcs->m_msh->no_msh_layer;
  NumberOfNodesPerLayer = NumberOfNodes / (NumberOfLayers + 1);  
  
  for (node = 0; node < NumberOfNodesPerLayer; node++) {
    
    index = m_pcs->m_msh->nod_vector[node]->free_surface;
    if (index == 1) {
      /* Zählen der Zeilen (-> anz_zeilen) */
      anz_zeilen = 0;
      xxflag = 0;
      nextnode = node;   
      do {
        startnode = nextnode;
        nextnode = MSHGetNextNode (startnode, m_pcs->m_msh);

        /* Test2: Gehört der nächste Knoten zu unterer Reihe ==> Abbruch */
        index2 = m_pcs->m_msh->nod_vector[nextnode]->free_surface;
        
        if (index2 == 2)  {
          xxflag = 1;
        }
        anz_zeilen++; /* Anzahl der beweglichen Zeilen (ohne die feste untere Zeile) */
      } while (xxflag != 1);
      /** Ende Zählen der Zeilen zwischen den oberen free surface node etc... und den Unteren **/

    /* Die Knoten unterhalb eines Free Surface Knotens bilden einen Strang */
    /* Die Knoten eines Stranges werden zwischengespeichert */
    strang = MSHGetNodesInColumn(node, anz_zeilen, m_pcs->m_msh);
  
    /* Die Knoten eines Stranges werden entsprechend der neuen Druckverteilung  verformt */
    /* Standrohrspiegelhöhe bestimmen */
    nidy = m_pcs->GetNodeValueIndex("HEAD")+1;
    if (GetRFProcessDensityFlow()) {  /* mit Dichteunterschiede */
      //OK_MOD     head = MODCalcHeadInColumn_MB(strang, anz_zeilen);
    }
    else {  /* ohne Dichteunterschiede */
      head = m_pcs->GetNodeValue(strang[0],nidy);
    } 

    /* nicht über surface elevation */
    CRFProcess* m_pcs_OLF = NULL;
    m_pcs_OLF = PCSGet("OVERLAND_FLOW");
    double SurfaceZ; 
  
    if(m_pcs_OLF!=NULL){
      SurfaceZ = m_pcs_OLF->m_msh->nod_vector[strang[0]]->Z();
      if (head > SurfaceZ){   
        head = SurfaceZ;
      }
    }
    
    /* Set minimum thickness */
    z_bottom = m_pcs->m_msh->nod_vector[strang[anz_zeilen]]->Z();
    if(head - z_bottom < MinThickness)
      head = z_bottom + MinThickness;
    
    /* Berechnung der Differenz */
    spanne_ges = head - z_bottom;
    spanne_rel = spanne_ges / anz_zeilen;
    m_pcs->m_msh->nod_vector[strang[0]]->SetZ(head); 

    if(spanne_ges != 0) {
      /* Setzen der neuen Z-Werte entlang eines Stranges */
      for (i = 1; i< anz_zeilen; i++)  {  /* Schleife über Anzahl der Zeilen */
        m_pcs->m_msh->nod_vector[strang[i]]->SetZ(head -i * spanne_rel); 
      } 
    }

  strang = (long*) Free(strang);     
  }  /*endif index ==1 */
  } /* end for Schleife über alle Knoten */
}


/**************************************************************************/
/* ROCKFLOW - Function: MSHGetGravityCenter
                                                                          */
/* Task:  Calculates the coordinates of the gravity center of trianle
   elements and the base of prism elements
     
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
   I: element
                                                                          */
/* Return:
   coord
                                                                          */
/* Programming:
   09/2002     MB         First Version
                                                                          */
/**************************************************************************/
double* MSHGetGravityCenter (long element)
{
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  double *coord = NULL;     

  knoten = ElGetElementNodes(element);
  
  coord = (double*)Realloc(coord, 2*sizeof(double));

  coord[0] = (GetNodeX(knoten[0]) + GetNodeX(knoten[1]) + GetNodeX(knoten[2]))/3 ;
  coord[1] = (GetNodeY(knoten[0]) + GetNodeY(knoten[1]) + GetNodeY(knoten[2]))/3 ;

  return coord; 
}


/**************************************************************************/
/* ROCKFLOW - Function: GEOGetNodeArea
                                                                          */
/* Task:
   Calculates the surface area of a node
   works for prism elements
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
   I: startnode, direction
                                                                          */
/* Return:
   area
                                                                          */
/* Programming:
   04/2003     MB         First Version
                                                                          */
/**************************************************************************/
double MSHGetNodeArea (long node)
{
  int anz_elem;
  int i;
  long* elements = NULL;
  double* coord = NULL;
  double* x = NULL;
  double* y = NULL;
  double* winkel = NULL; 
  double SVector[3]; 
  double direction[3];
  double area;
  double SPunkt1[3];  
  double SPunkt2[3];  
  double MPunkt[3];

  /* Alle 3-D Elemente holen, die an diesem Knoten haengen */
  elements = GetNode3DElems(node, &anz_elem);

  for (i = 0; i < anz_elem; i++)  {  /* Schleife über Anzahl der benachbarten Elemente */
    coord = MSHGetGravityCenter(elements[i]);
    
    x = (double*) Realloc(x,(i+1)*sizeof(double));
    y = (double*) Realloc(y,(i+1)*sizeof(double));
    winkel = (double*) Realloc(winkel,(i+1)*sizeof(double));

    
    x[i] = coord[0];
    y[i] = coord[1];
    
    
    /* Die Schwerpunkte müssen zur Flächenberechnung sortiert werden. 
    das geht über die Bestimmung der Winkel */

    direction[0] = 0;
    direction[1] = 1;
    direction[2] = 0;

    SVector[0] = x[i] - GetNodeX(node);
    SVector[1] = y[i] - GetNodeY(node);
    SVector[2] = 0;

    winkel[i] = MAngleVectors(SVector, direction);
    if (SVector[0] < 0.0) {
      winkel[i] = 180 + (180 - winkel[i]);
    }
  } /* end for Schleife über Anzahl der benachbarten Elemente */

  
  /* Knoten jetzt nach winkel sortieren */
  TOLSortDataSet(x, y, winkel, anz_elem);

  /* Flächen ausrechnen */
  MPunkt[0] = GetNodeX(node);
  MPunkt[1] = GetNodeY(node);
  MPunkt[2] = 0;

  area = 0;

  for (i = 0; i < anz_elem; i++)  {  /* Schleife über Anzahl der benachbarten Elemente */
    
    SPunkt1[0] = x[i];
    SPunkt1[1] = y[i];
    SPunkt1[2] = 0;

    if (i == anz_elem -1) {
      SPunkt2[0] = x[0];
      SPunkt2[1] = y[0];
      SPunkt2[2] = 0;
    } 
    else {
      SPunkt2[0] = x[i+1];
      SPunkt2[1] = y[i+1];
      SPunkt2[2] = 0;
    }
    area = area + MSHCalcTriangleArea3 (SPunkt1, SPunkt2, MPunkt);

  } /* end for Schleife über Anzahl der benachbarten Elemente */
  
  return area;
  
}

/**************************************************************************/
/* ROCKFLOW - Function: ConstructVerticalEdgeListHex
                                                                          */
/* Task:
   Calculates Edge List for prism elements.
   Only vertical edges are registered, i.e. number of edges per element = 3
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   - void -
                                                                          */
/* Programming:
   03/2003     MB       First Version (based on ConstructEdgeList)
                                                                          */
/**************************************************************************/
void MSHConstructVerticalEdgeListHex ( void )
{
  long i, j, k, l;  /* Laufvariablen */
  int typ;  /* Elementtyp - 1 */
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *kanten = NULL;  /* Zeiger auf Elementkanten */
  Kante *kante = NULL;  /* Zeiger auf Kante */
  /* Schleife ueber alle Ausgangselemente */
  for (i=0;i<start_new_elems;i++) {
      if(i==132){
         i = i;
      }
      typ = ElGetElementType(i) - 1;
      knoten = ElGetElementNodes(i);
      kanten = (long *) Malloc(4*sizeof(long));
      for (j=0l;j<4;j++) {  /* Schleife ueber alle Kanten */
          k = 0l;
          l = EdgeListLength;
          while (k<l) {  /* Kante suchen */
              kante = GetEdge(k);
              if (((kante->knoten[0]==knoten[j]) && (kante->knoten[1]==knoten[j+4])) ||
                  ((kante->knoten[1]==knoten[j]) && (kante->knoten[0]==knoten[j+4]))  )
                  l = -20l;  /* Kante gefunden, Abbruchkriterium */
              else
                  k++;
          }
          if (l==EdgeListLength) {  /* Kante existiert noch nicht */
              k = AddEdge(kante=NewEdge());  /* Kante erzeugen */
              kante->knoten[0] = knoten[j];
              kante->knoten[1] = knoten[j+4];
              kante->knoten[2] = -1;
              if (typ)   /* 2D-Element ( oder 3D ... ) */
                  /*Traegt 2D-Element ele bei Kante number ein */
                  Assign2DElementEdges(k,i);
              else /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          else {  /* Kante existiert bereits mit Index k */
              if (typ)    /* 2D-Element ( oder 3D ... ) */
                  Assign2DElementEdges(k,i);
              else  /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          /* Kante bei Element eintragen */
          kanten[j] = k;
      }
      /* Kantenverweise bei Element i eintragen */
      ElSetElementEdges(i,kanten);
  }
  start_new_edges = EdgeListLength;
    /* Alle Knoten und Elemente sollten jetzt im Kantenverzeichnis
       vermerkt sein */
}
/* ROCKFLOW - Function: ConstructVerticalEdgeListPrism
                                                                          */
/* Task:
   Calculates Edge List for prism elements.
   Only vertical edges are registered, i.e. number of edges per element = 3
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   - void -
                                                                          */
/* Programming:
   03/2003     MB       First Version (based on ConstructEdgeList)
                                                                          */
/**************************************************************************/
void MSHConstructVerticalEdgeListPrism ( void )
{
  long i, j, k, l;  /* Laufvariablen */
  int typ;  /* Elementtyp - 1 */
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *kanten = NULL;  /* Zeiger auf Elementkanten */
  Kante *kante = NULL;  /* Zeiger auf Kante */
  /* Schleife ueber alle Ausgangselemente */
  for (i=0;i<start_new_elems;i++) {
      typ = ElGetElementType(i) - 1;
      knoten = ElGetElementNodes(i);
      kanten = (long *) Malloc(3*sizeof(long));
      for (j=0l;j<3;j++) {  /* Schleife ueber alle Kanten */
          k = 0l;
          l = EdgeListLength;
          while (k<l) {  /* Kante suchen */
              kante = GetEdge(k);
              if (((kante->knoten[0]==knoten[j]) && (kante->knoten[1]==knoten[j+3%4])) ||
                  ((kante->knoten[1]==knoten[j]) && (kante->knoten[0]==knoten[j+3%4]))  )
                  l = -20l;  /* Kante gefunden, Abbruchkriterium */
              else
                  k++;
          }
          if (l==EdgeListLength) {  /* Kante existiert noch nicht */
              k = AddEdge(kante=NewEdge());  /* Kante erzeugen */
              kante->knoten[0] = knoten[j];
              kante->knoten[1] = knoten[j+3%4];
              kante->knoten[2] = -1;
              if (typ)   /* 2D-Element ( oder 3D ... ) */
                  /*Traegt 2D-Element ele bei Kante number ein */
                  Assign2DElementEdges(k,i);
              else /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          else {  /* Kante existiert bereits mit Index k */
              if (typ)    /* 2D-Element ( oder 3D ... ) */
                  Assign2DElementEdges(k,i);
              else  /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          /* Kante bei Element eintragen */
          kanten[j] = k;
      }
      /* Kantenverweise bei Element i eintragen */
      ElSetElementEdges(i,kanten);
  }
  start_new_edges = EdgeListLength;
    /* Alle Knoten und Elemente sollten jetzt im Kantenverzeichnis
       vermerkt sein */
}


/**************************************************************************/
/* ROCKFLOW - Function: ConstructVerticalEdgeListRectangle
                                                                          */
/* Task:
   Calculates Edge List for quadrilateral elements.
   Only vertical edges are registered, i.e. number of edges per element = 3
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   - void -
                                                                          */
/* Programming:
   03/2003     MB       First Version (based on ConstructEdgeList)
                                                                          */
/**************************************************************************/
void MSHConstructVerticalEdgeListRectangle ( void )
{
  long i, j, k, l;  /* Laufvariablen */
  int typ;  /* Elementtyp - 1 */
  long *knoten = NULL;  /* Zeiger auf Elementknoten */
  long *kanten = NULL;  /* Zeiger auf Elementkanten */
  Kante *kante = NULL;  /* Zeiger auf Kante */
  /* Schleife ueber alle Ausgangselemente */
  for (i=0;i<start_new_elems;i++) {
      typ = ElGetElementType(i) - 1;
      knoten = ElGetElementNodes(i);
      kanten = (long *) Malloc(2*sizeof(long));
      for (j=0l;j<2;j++) {  /* Schleife ueber alle Kanten */
          k = 0l;
          l = EdgeListLength;
          while (k<l) {  /* Kante suchen */
              kante = GetEdge(k);
              if (((kante->knoten[0]==knoten[j]) && (kante->knoten[1]==knoten[j+3%4])) ||
                  ((kante->knoten[1]==knoten[j]) && (kante->knoten[0]==knoten[j+1%4]))  )
                  l = -20l;  /* Kante gefunden, Abbruchkriterium */
              else
                  k++;
          }
          if (l==EdgeListLength) {  /* Kante existiert noch nicht */
              k = AddEdge(kante=NewEdge());  /* Kante erzeugen */
              if (j == 0) {
                  kante->knoten[0] = knoten[j];
                  kante->knoten[1] = knoten[j+3%4];
                  kante->knoten[2] = -1;
              }
              if (j == 1) {
                  kante->knoten[0] = knoten[j];
                  kante->knoten[1] = knoten[j+1%4];
                  kante->knoten[2] = -1;
              }
              if (typ)   /* 2D-Element ( oder 3D ... ) */
                  /*Traegt 2D-Element ele bei Kante number ein */
                  Assign2DElementEdges(k,i);
              else /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          else {  /* Kante existiert bereits mit Index k */
              if (typ)    /* 2D-Element ( oder 3D ... ) */
                  Assign2DElementEdges(k,i);
              else  /* 1D-Element */
                  kante->nachbar_1D = i;
          }
          /* Kante bei Element eintragen */
          kanten[j] = k;
      }
      /* Kantenverweise bei Element i eintragen */
      ElSetElementEdges(i,kanten);
  }
  start_new_edges = EdgeListLength;
    /* Alle Knoten und Elemente sollten jetzt im Kantenverzeichnis
       vermerkt sein */
}


/**************************************************************************
   ROCKFLOW - Funktion: GEOIsNodeAMovingNode                              */

/* Task:
   Checks if node is a moving node
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)   - void -
                                                                          */
/* Return:
   0 if true
                                                                          */
/* Programming:
   03/2003     MB       First Version
                                                                          */
/**************************************************************************/
long GEOIsNodeAMovingNode (long node)
{
  int i=0;
  int anz_elem;
  long *elemente = NULL;
  int flag;

  /* Alle 2-D / 3-D Elemente holen, die an diesem Knoten haengen */
  switch (ElGetElementType(1)) {
  case 2:
    elemente = GetNode2DElems(node, &anz_elem);
    break;
  case 6:
    elemente = GetNode3DElems(node, &anz_elem);
    break;
  default: 
    anz_elem = 0;
  }

  /* Test if a all elements belonging to this node are unconfined elements
  if not --> not a moving node --> return 0 */
  do  {
  flag = 0;
  if (ElGetElement(i)!=NULL) { 
//OK    if (MATGetMediumUnconfinedFlowGroup(elemente[i]) == 1)  flag = 1; 
//OK    else flag = 2; 
  }  
  i++;
  }  while (flag != 2 && i < anz_elem);

  //Test Temporär
  //if (flag == 1) return 1;
  //else return 0;

  if (flag == 1) return 1;
  else return 1;
  

}


/**************************************************************************
MSHLib-Method:
Task: 
Programing:
04/2003 MB Implementation for prism elements
09/2004 OK 
last modification:
ToDo: MSHGetNodeArea
**************************************************************************/
double NODCalcPatchArea(long node,Surface* m_sfc)
{
  m_sfc = m_sfc;
  int i;
  long* elements = NULL;
  int no_3d_elements;
  double d12[3],d13[3],dn[3];
  double dist12,dist13;
  int dim;
  vector<long*>face_vector;
//  long face_nodes[4];
//  double eps;
  double area = 1.;
  long* nodes = NULL;
  // NOD elements
  elements = GetNode3DElems(node,&no_3d_elements);
  for(i=0;i<no_3d_elements;i++){
    nodes = ElGetElementNodes(elements[i]);    
    switch(ElGetElementType(elements[i])){
      case 6: // prism
        dim = 3;
        // Element faces in surface plane
        for(i=1;i<3;i++){
        }
          d12[0] = GetNodeX(nodes[i+1])-GetNodeX(nodes[i]);
          d12[1] = GetNodeY(nodes[i+1])-GetNodeY(nodes[i]);
          d12[2] = GetNodeZ(nodes[i+1])-GetNodeZ(nodes[i]);
          dist12 = MBtrgVec(d12,dim);
          if(dist12<0) continue;
          d13[0] = GetNodeX(nodes[i+3])-GetNodeX(nodes[i]);
          d13[1] = GetNodeY(nodes[i+3])-GetNodeY(nodes[i]);
          d13[2] = GetNodeZ(nodes[i+3])-GetNodeZ(nodes[i]);
          dist13 = MBtrgVec(d13,dim);
          if(dist13<0) continue;
          M3KreuzProdukt(d12,d13,dn);
//          dist = MSkalarprodukt(dn,m_sfc->normal_vector,dim);
//          if(dist<eps)
//            face_vector.push_back(face_nodes);
        break;
      default:
        cout << "Error in NODCalcPatchArea: element type not implemented" << endl;
    }
  }
  return area;
}

/**************************************************************************
MSHLib-Method: 
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void MSHCreateNOD2ELERelations(CFEMesh*m_msh)
{
  m_msh = m_msh;
/*WW
  int k;
  long i,j;
  CMSHNodes*m_nod = NULL;
  FiniteElement::CElement*m_ele = NULL;
  for(i=0;i<(long)m_msh->nod_vector.size();i++){
    m_nod = m_msh->nod_vector[i];
    for(j=0;j<(long)m_msh->ele_vector.size();j++){
      m_ele = m_msh->ele_vector[j];
      for(k=0;k<m_ele->nnodes;k++){
        if(m_ele->nodes_index[k]==i){
          m_nod->ele_number_vector.push_back(j);
          break;
        }
      }
    }
  }
*/
}

/**************************************************************************
FEMLib-Method: 
Task: Searches mobile nodes and sets node->free_surface = 4
Programing:
09/2004 OK / MB Implementation
05/2005 OK Bugfix
07/2005 MB MMP keyword
08/2005 MB m_pcs
01/2006 OK LAYER
**************************************************************************/
void CFEMesh::DefineMobileNodes(CRFProcess*m_pcs)
{
  long* mobile_nodes = NULL;
  long no_mobile_nodes = -1;
  long i,j;
  //----------------------------------------------------------------------
  // Define mobile MSH nodes
  //----------------------------------------------------------------------
  //......................................................................
  //DOMAIN
  if(m_pcs->geo_type.find("DOMAIN")!=string::npos)
  {
    for(i=0;i<(long)nod_vector.size();i++) 
    {
      mobile_nodes = (long *) Realloc(mobile_nodes,sizeof(long)*(i+1));
      mobile_nodes[i] = i;
    }
    no_mobile_nodes = (long)m_pcs->m_msh->nod_vector.size();
  }
  //......................................................................
  //LAYER
  if(m_pcs->geo_type.find("LAYER")!=string::npos)
  {
    string m_string;
    long no_nodes_per_layer = (long)nod_vector.size() / (no_msh_layer+1);
    int pos = 0;
    int layer_start=0,layer_end=0;
    if(m_pcs->geo_type_name.find("-")!=string::npos)
    {
      pos = m_pcs->geo_type_name.find("-")!=string::npos;
      m_string = m_pcs->geo_type_name.substr(0,pos);
      layer_start = strtol(m_string.c_str(),NULL,0);
      m_string = m_pcs->geo_type_name.substr(pos+1,string::npos);
      layer_end = strtol(m_string.c_str(),NULL,0);
    }
    else
    {
      layer_start = strtol(m_pcs->geo_type_name.c_str(),NULL,0);
      layer_end = layer_start;
    }
    int no_layers = layer_end-layer_start+1;
    no_mobile_nodes = (no_layers+1)*no_nodes_per_layer;
    mobile_nodes = new long[no_mobile_nodes];
    for(i=0;i<no_layers+1;i++)
    {
      for(j=0;j<no_nodes_per_layer;j++) 
      {
        mobile_nodes[i*no_nodes_per_layer+j] = j + (layer_start-1+i)*no_nodes_per_layer;
      }
    }
  }
  //......................................................................
  //SURFACE 
  if(m_pcs->geo_type.find("SURFACE")!=string::npos)
  {
    Surface *m_sfc = NULL;
    m_sfc = GEOGetSFCByName(m_pcs->geo_type_name);//CC
    if(m_sfc)
      mobile_nodes = GetPointsIn(m_sfc,&no_mobile_nodes);//CC
    else
      cout << "Warning in CFEMesh::DefineMobileNodes - no GEO data" << endl;
  }
  //......................................................................
  //VOLUME
  if(m_pcs->geo_type.find("VOLUME")!=string::npos)
  {
    CGLVolume *m_vol = NULL;
    m_vol = GEOGetVOL(m_pcs->geo_type_name);//CC 10/05
    if(m_vol)
      mobile_nodes = GetPointsInVolume(m_vol,&no_mobile_nodes);//CC 10/05
    else
      cout << "Warning in CFEMesh::DefineMobileNodes - no GEO data" << endl;
  }
  //----------------------------------------------------------------------
  // Set mobile MSH nodes flag
  //----------------------------------------------------------------------
  for(i=0;i<(long)nod_vector.size();i++) 
  {
    nod_vector[i]->free_surface = -1;
  }
  for(i=0;i<no_mobile_nodes;i++){
    nod_vector[i]->free_surface = 4;
    //nod_vector[mobile_nodes[i]]->free_surface = 4;
  }
  //----------------------------------------------------------------------
  if (no_mobile_nodes > 0)  {
    m_pcs->mobile_nodes_flag = 1;
    MSHSelectFreeSurfaceNodes(this);
  }
  //----------------------------------------------------------------------
  delete [] mobile_nodes;
  mobile_nodes = NULL;
}

