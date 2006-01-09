/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C
#include "math.h"
// C++
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_lib.h"
#include "msh_elem.h"
#include "mathlib.h"
// PCSLib
#include "rf_pcs.h"
#include "elements.h"//CC
#include "nodes.h"

/**************************************************************************
MshLib-Method: 
Task: Create QUAD elements from line elements
Programing:
02/2004 OK Implementation
**************************************************************************/
void MSHCreateQuadsFromLine(CGLLine *m_line)
{
  int i,k;
  int col=0;
  long j;
  long **matrix = NULL;
  double x,y,x1,y1;
  long msh_node_number;
  double dist;
  double eps = 1e-3;
  int no_msh_nodes = m_line->no_msh_nodes;
  //---------------------------------------------------------------------
  // Node columns
  ///matrix = new long*[number_of_msh_nodes];
  matrix = (long**) Malloc(no_msh_nodes*sizeof(long));
  for(i=0;i<no_msh_nodes;i++) {
    msh_node_number = m_line->msh_nodes[i];
    x1 = GetNodeX(msh_node_number);
    y1 = GetNodeY(msh_node_number);
    col=0;
    matrix[i] = NULL;
    for(j=0;j<NodeListLength;j++) {
      x = GetNodeX(j); // ToDo NodeNumber[j]
      y = GetNodeY(j);
      dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
      if(dist<eps) {
        col++;
        matrix[i] = (long*) Realloc(matrix[i],col*sizeof(long));
        matrix[i][col-1] = j; // ToDoNodeNumber[j];
      }
    }
  }
  //-----------------------------------------------------------------------
  // Create Quads
  long *nodes = NULL;
  for(i=0;i<no_msh_nodes-1;i++) {
    for(k=0;k<col-1;k++) {
      nodes = (long*) Malloc(sizeof(long)*ElNumberOfNodes[1]);
      nodes[0] = matrix[i][k];
      nodes[1] = matrix[i+1][k];
      nodes[2] = matrix[i+1][k+1];
      nodes[3] = matrix[i][k+1];
      ELECreateTopology(2,-1,0,ElementListLength);
      ElSetElementNodes(ElementListLength-1,nodes);
      ElSetElementGroupNumber(ElementListLength-1,m_line->mat_group);
      anz_2D++;
    }
  }
  //-----------------------------------------------------------------------
  // Memory
  for(i=0;i<no_msh_nodes-1;i++) {
    matrix[i] = (long*) Free(matrix[i]);
  }
  matrix = (long**) Free(matrix);

/*
    //---------------------------------------------------------------------
    // Strings
  vector<long>string1;
  vector<long>string2;
  msh_node_number = m_line->m_point1->gli_point_id;
  x1 = GetNodeX(msh_node_number);
  y1 = GetNodeY(msh_node_number);
  string1.push_back(msh_node_number);
  msh_node_number = m_line->m_point2->gli_point_id;
  x2 = GetNodeX(msh_node_number);
  y2 = GetNodeY(msh_node_number);
  string2.push_back(msh_node_number);
   // String1+2
  for(i=0;i<NodeListLength;i++) {
    x = GetNodeX(NodeNumber[i]);
    y = GetNodeY(NodeNumber[i]);
    dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    if(dist<eps) {
      string1.push_back(NodeNumber[i]);
    }
    dist = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
    if(dist<eps) {
      string2.push_back(msh_node_number);
    }
  }
  //-----------------------------------------------------------------------
  // Sort by distance
   // String1
   // String2
  //-----------------------------------------------------------------------
  // Create Quads
  long *nodes = NULL;
  int string_length = string1.size();
  for(i=0;i<string_length-1;i++) {
    nodes = (long*) Malloc(sizeof(long)*ElNumberOfNodes[1]);
    nodes[0] = string1[i];
    nodes[1] = string1[i+1];
    nodes[2] = string2[i+1];
    nodes[3] = string2[i];
    ELECreateTopology(2,-1,0,ElementListLength);
    ElSetElementNodes(ElementListLength-1,nodes);
    anz_2D++;
  }
*/
}


/**************************************************************************
MshLib-Method: 
Task: Create QUAD elements from polylines
Programing:
02/2004 OK Implementation
**************************************************************************/
void MSHCreateQuadsFromPLY(CGLPolyline *m_polyline,int msh_type)
{
  CGLLine *m_line = NULL;
  vector<CGLLine*>::iterator pl;//CC

  switch(msh_type){
    case 1: 
      // create polyline lines, if necessary
      if(m_polyline->line_vector.size()==0)
        m_polyline->ComputeLines(m_polyline);
      pl = m_polyline->line_vector.begin();//CC
      while(pl!=m_polyline->line_vector.end()) {//CC
        m_line = *pl;
        m_line->mat_group = m_polyline->mat_group;
        if(m_line->no_msh_nodes==0)
          SetRFIPointsClose(m_line);//CC 10/05
        MSHCreateQuadsFromLine(m_line);
        pl++;
      }
      break;
  }
}

/**************************************************************************
  ROCKFLOW - Modul:PrismRefine()
                                                                          
  Aufgabe:
    To refine prism element layers  
  
   const int NLayers        :      Number of layers (start mesh)
   const int Layer          :      Layer number of the layer to be refined 
   const int SUBLayers      :      Number of sublayers
                                                                          
   Programmaenderungen:
   10/2003     WW/MB     Erste Version
   08/2004     MB        NElementsPerLayer = msh_no_pris / NLayers; 
                                                                          
***************************************************************************/
void PrismRefine(const int NLayers, const int Layer, const int NSubLayers)
{
  const int nn=6;
  int i, j,nes;
  int iii;
  long group; 
  long *element_nodes = NULL;
  //WW  static long ele_nodes[6];
  static double nx[6], ny[6], nz[6];
  //WW  static int newNode0[3], newNode1[3];  
  double dx[3], dy[3], dz[3];
  double newz;
  //WW  static double  newx0[3],newy0[3],newz0[3];
  //WW  static double  newx1[3],newy1[3],newz1[3];
  int element_type=0; 
  int NumNodesNew, NumElementNew;  
//  const double Nhood = 1.0e-8;
  const int NumElement0 = ElListSize();
//  const int NumNodes0 = NodeListSize();
  long *knoten = NULL; 
//  int pos = 0;
  long vorgaenger;  
  Knoten *kno = NULL;  
  int NNodesPerRow = 0;
  int NElementsPerLayer = 0;
  int row;
  int NRowsToShift;
  int NRows;
  int count;
  int CountNLayers;

  NNodesPerRow = NodeListSize() / (NLayers+1); 
  //NElementsPerLayer = ElListSize() / NLayers;
  NElementsPerLayer = msh_no_pris / NLayers;

  row = Layer;
  NRows = NLayers +1;

  NumNodesNew = NodeListSize()-1;
  NumElementNew = ElListSize()-1;

  /* Initialisierung der Knoten flags */
  for (i = 0; i < NodeListLength; i++) {
  NODSetFreeSurfaceFlag(i, 0);
  }

  for (int ne = 0; ne < NumElement0; ne++)   {
    if (ElGetElement(ne) != NULL)    {    /* Element existiert */
      element_type=ElGetElementType(ne); 
      if(element_type==6) {

      element_nodes = ElGetElementNodes(ne);
      CountNLayers = NLayers;

         for(i=0; i<nn; i++)   {
           nx[i] = GetNodeX(element_nodes[i]);
           ny[i] = GetNodeY(element_nodes[i]);
           nz[i] = GetNodeZ(element_nodes[i]);
         } 

         nes=0;
         for(i=0; i<3; i++)   {
           if(element_nodes[i] >= (row-1) * NNodesPerRow  &&
              element_nodes[i] <= (row * NNodesPerRow) -1)   {
              nes++;    
           }  
         }

         if(nes==3)   {
           for(i=0; i<3; i++)   {
             dx[i] = (nx[i+3]-nx[i])/(float)NSubLayers;
             dy[i] = (ny[i+3]-ny[i])/(float)NSubLayers;
             dz[i] = (nz[i+3]-nz[i])/(float)NSubLayers;
           }

           /* Create new nodes */
           for(iii=0; iii < NSubLayers-1 ; iii++) {  /* Loop over SubLayers */
             /* neue Knoten ganz unten */
             for(i=0; i<3; i++)   {
               if (NODGetFreeSurfaceFlag(element_nodes[i])==0)   {  
                 kno = (Knoten *)CreateNodeGeometry(); 
                 kno->x = GetNodeX (element_nodes[i] + ((CountNLayers+1) - row) * NNodesPerRow);
                 kno->y = GetNodeY(element_nodes[i] + ((CountNLayers+1) - row) * NNodesPerRow);
                 kno->z = GetNodeZ(element_nodes[i] + ((CountNLayers+1) - row) * NNodesPerRow);
                   if((element_nodes[i] + ((CountNLayers+2) - row) * NNodesPerRow) == 390) {
                     i = i;
                   }
                 PlaceNode(kno,(element_nodes[i] + ((CountNLayers+2) - row) * NNodesPerRow));
               }
             }  
             /* neues Element ganz unten */
             NumElementNew++;
             vorgaenger=-1; 
             CreateElementTopology(element_type,vorgaenger,0,NumElementNew); 
             group = ElGetElementGroupNumber(NumElementNew - NElementsPerLayer);
             ElSetElementGroupNumber(NumElementNew,group);
               
             knoten = (long *) Malloc(sizeof(long)*nn);
             for(j=0; j<3; j++)  {
               knoten[j] =   element_nodes[j] + ((CountNLayers+1) - row) * NNodesPerRow;
               knoten[j+3] = element_nodes[j] + ((CountNLayers+2) - row) * NNodesPerRow;
             } 
             ElSetElementNodes(NumElementNew,knoten);
             ElSetElementActiveState(NumElementNew,1);

             /* "rowx hochziehen"   */
             /* loop über die betroffenen rows   */
             NRowsToShift = NRows - Layer;
             count = 0;

             for(i = NRowsToShift; i > 0; i--)  {
               if (i != 1)   {
                 count++;
                 for(j=0; j<3; j++)  {
                   if (NODGetFreeSurfaceFlag(element_nodes[j])==0)   {  
                     SetNodeZ(element_nodes[j+3] + NNodesPerRow*(iii+i-1), GetNodeZ(element_nodes[j] + NNodesPerRow*(iii+i-1)));
                   } 
                 }
               }
               else   {
                 for(j=0; j<3; j++)  {
                   if (NODGetFreeSurfaceFlag(element_nodes[j])==0)   {  
                     newz = GetNodeZ(element_nodes[j]) + (dz[j]*(iii+1));
                     SetNodeZ(element_nodes[j] + (i) * NNodesPerRow *(iii+1), newz);
                   }                               
                 } 
               }
             } /* end for Rows to shift */
            
             if (iii== NSubLayers-2)   {
               for(j=0; j<3; j++)   {
                 NODSetFreeSurfaceFlag(element_nodes[j], 33);
               }
             }  
           
             CountNLayers++; 
          } /* End Loop over SubLayers  */
       }  /* End if nes==3 */
    }  /* Elementtyp ==6 */
    }  /* Element existiert */
  }  /* Element loop */ 

  /* Get Path name */
#ifdef MFC //OK
  CFileDialog dlg(FALSE, NULL, "new", OFN_ENABLESIZING ,
                 " Geometry Files (*.rfi)|*.rfi| All Files (*.*)|*.*||" );
  dlg.DoModal();
  CString m_filepath = dlg.GetPathName();
  anz_active_elements = ElListSize();
  anz_3D = anz_active_elements;
  /* Write rfi file */ 
  const char* cpsz = static_cast<LPCTSTR>(m_filepath);
  const char* xxxxx;
  xxxxx = cpsz;     
  DATWriteRFIFile(xxxxx);
#endif
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
07/2005 MB Implementation
11/2005 WW/MB element class
**************************************************************************/
void CFEMesh::CreatePriELEFromTri(int no_layer,double layer_thickness)
{
  no_layer = no_layer;
  layer_thickness = layer_thickness;
  int j;
  int hempel, hampel;
  long i;
  int k;
  long size;
  long no_tri_elements = (long)ele_vector.size();
  long no_tri_nodes = (long)nod_vector.size();
  Mesh_Group::CElem* m_tri_ele = NULL;
  Mesh_Group::CElem* m_ele = NULL;
  //----------------------------------------------------------------------
  // Create MSH
  MSHDelete("PRIS_from_TRI");
  CFEMesh* m_msh_pri = NULL;
  m_msh_pri = new CFEMesh();
  m_msh_pri->pcs_name = "PRIS_from_TRI";
  m_msh_pri->ele_type = 6;
  //m_msh_pri->no_msh_layer = no_layer;
  m_msh_pri->no_msh_layer  = no_layer;
  //----------------------------------------------------------------------
  // Create Prism elements
  size = (no_layer + 1) * no_tri_nodes; 
  m_msh_pri->nod_vector.resize(size);
  for(j=0;j<size;j++)
    m_msh_pri->nod_vector[j] = NULL;
  for(j=0;j<no_layer;j++){
    for(i=0;i<no_tri_elements;i++){
      //Elements
      m_tri_ele = ele_vector[i];
      m_ele = new  Mesh_Group::CElem;
      m_ele->SetPatchIndex(j); //OK
      m_ele->SetElementType(6);
      m_ele->nnodes = 6;
      m_ele->nodes_index.resize(m_ele->nnodes);
      //Set indices
      m_ele->nodes_index[0] = m_tri_ele->GetNodeIndex(0)+ j*no_tri_nodes;
      m_ele->nodes_index[1] = m_tri_ele->GetNodeIndex(1) + j*no_tri_nodes;
      m_ele->nodes_index[2] = m_tri_ele->GetNodeIndex(2) + j*no_tri_nodes;
      m_ele->nodes_index[3] = m_ele->GetNodeIndex(0)+ no_tri_nodes;
      m_ele->nodes_index[4] = m_ele->GetNodeIndex(1)+ no_tri_nodes;
      m_ele->nodes_index[5] = m_ele->GetNodeIndex(2)+ no_tri_nodes;
      //Nodes
      hempel = 0;
      hampel = 0; 
      for(k=0;k<m_ele->nnodes;k++){
        if(m_msh_pri->nod_vector[m_ele->GetNodeIndex(k)]==NULL){
          m_ele->nodes[k] = new CNode(m_ele->GetNodeIndex(k));
          m_msh_pri->nod_vector[m_ele->GetNodeIndex(k)] = m_ele->nodes[k];
          if(k>2){ 
            hempel = 3;
            hampel = 1;
          }
          m_ele->nodes[k]->SetX(nod_vector[m_tri_ele->GetNodeIndex(k-hempel)]->X());
          m_ele->nodes[k]->SetY(nod_vector[m_tri_ele->GetNodeIndex(k-hempel)]->Y());
          m_ele->nodes[k]->SetZ(nod_vector[m_tri_ele->GetNodeIndex(k-hempel)]->Z() - (j+hampel)*layer_thickness);      
        }
        else {
          m_ele->nodes[k] = m_msh_pri->nod_vector[m_ele->GetNodeIndex(k)];
        }
      } //end for m_ele->nnodes
      m_msh_pri->ele_vector.push_back(m_ele);
    }  //end for no_tri_elements 
  }  //end for no_layers
  //----------------------------------------------------------------------
  if(m_msh_pri->ele_vector.size()>0){
    m_msh_pri->ConstructGrid();
    fem_msh_vector.push_back(m_msh_pri);
  }
  else 
    delete m_msh_pri;
}

/**************************************************************************
MshLib-Method: CreateLines
Task: Create linear 1-D elements from polylines
Programing:
05/2005 OK Implementation
**************************************************************************/
void CFEMesh::CreateLineELEFromPLY(CGLPolyline *m_ply)
{
  m_ply = m_ply;
//WWW
#ifdef TODO 
  long i;
  double dx,dy,dz;
  CGLPoint* m_pnt = NULL;
  //----------------------------------------------------------------------
  // Create LINE elements
  FiniteElement::CElement* m_quad_ele = NULL;
  for(i=0;i<nr;i++){
    m_ele = new FiniteElement::CElement;
    m_ele->GetPatchIndex() = (int)mmp_vector.size();
    m_ele->type_name = "line";
    m_ele->ElementType = 1;
    m_ele->nnodes = 2;
    m_ele->nodes = new long[2];
    m_ele->nodes_index[0] = i;
    m_ele->nodes_index[1] = i+1;
    ele_vector.push_back(m_ele);
  }
  //----------------------------------------------------------------------
  // Create LINE nodes
  CMSHNodes* m_nod = NULL;
  m_pnt = m_pnt->Get(m_ply->point_vector[0]->gli_point_id);
  m_ply->point_vector[0]->x = m_pnt->x;
  m_ply->point_vector[0]->y = m_pnt->y;
  m_ply->point_vector[0]->z = m_pnt->z;
  m_pnt = m_pnt->Get(m_ply->point_vector[1]->gli_point_id);
  m_ply->point_vector[1]->x = m_pnt->x;
  m_ply->point_vector[1]->y = m_pnt->y;
  m_ply->point_vector[1]->z = m_pnt->z;
  dx = (m_ply->point_vector[1]->x - m_ply->point_vector[0]->x) / (double)nr;
  dy = (m_ply->point_vector[1]->y - m_ply->point_vector[0]->y) / (double)nr;
  dz = (m_ply->point_vector[1]->z - m_ply->point_vector[0]->z) / (double)nr;
  for(i=0;i<nr+1;i++){
    m_nod = new CMSHNodes();
    //m_nod->nodenumber = i + j*no_quad_elements;
    m_nod->x = m_ply->point_vector[0]->x + i*dx;
    m_nod->y = m_ply->point_vector[0]->y + i*dy;
    m_nod->z = m_ply->point_vector[0]->z + i*dz;
    nod_vector.push_back(m_nod);
  }
  //----------------------------------------------------------------------
#endif
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
04/2005 OK Implementation
11/2005 MB ELE
**************************************************************************/
void CFEMesh::CreateLineELEFromQuad(int m_numberofprismlayers,double m_thicknessofprismlayer,int m_iMATGroup)
{
  m_iMATGroup = m_iMATGroup;
  int j;
  long i, k;
  double x, y, z;
  long size;
  long no_quad_elements = (long)ele_vector.size();
  //long no_quad_nodes = (long)nod_vector.size();
  double* center = NULL;
  Mesh_Group::CElem* m_quad_ele = NULL;
  Mesh_Group::CElem* m_ele = NULL;

  //----------------------------------------------------------------------
  // Create MSH
  MSHDelete("LINE_from_QUAD");
  CFEMesh* m_msh_line = NULL;
  m_msh_line = new CFEMesh();
  m_msh_line->pcs_name = "LINE_from_QUAD";
  m_msh_line->ele_type = 1;
  m_msh_line->no_msh_layer = m_numberofprismlayers;
  //----------------------------------------------------------------------
  // Create LINE elements
  size = (m_numberofprismlayers+1) * no_quad_elements;
  m_msh_line->nod_vector.resize(size);

  for(j=0;j<size;j++){
    m_msh_line->nod_vector[j]=NULL;
  }

  //for(j=0;j<m_numberofprismlayers;j++){
  //  for(i=0;i<no_quad_elements;i++){
  for(i=0;i<no_quad_elements;i++){    //Elements
    for(j=0;j<m_numberofprismlayers;j++){
      //Elements
      m_quad_ele = ele_vector[i];
      m_ele = new Mesh_Group::CElem;
      //m_ele->GetPatchIndex() = m_msh_line->mat_group;
      m_ele->SetIndex((i*m_numberofprismlayers) + j);
      m_ele->SetElementType(1);
      m_ele->nnodes = 2;
      m_ele->nodes_index.resize(m_ele->nnodes);
      //Set indices
      m_ele->nodes_index[0] = j + i * (m_numberofprismlayers+1);
      m_ele->nodes_index[1] = m_ele->nodes_index[0]+ 1;
      m_msh_line->ele_vector.push_back(m_ele);
      //
      center = m_quad_ele->GetGravityCenter();
      x = center[0];
      y = center[1];
      z = center[2];
      //Nodes
      for(k=0;k<m_ele->nnodes;k++){
        if(m_msh_line->nod_vector[m_ele->GetNodeIndex(k)]==NULL){ //if new node
          m_ele->nodes[k] = new CNode(m_ele->GetNodeIndex(k));
          m_msh_line->nod_vector[m_ele->GetNodeIndex(k)] = m_ele->nodes[k];
          //Set Coordinates
          m_ele->nodes[k]->SetX(x);
          m_ele->nodes[k]->SetY(y);
          z = z + ((k+j) * m_thicknessofprismlayer);
          m_ele->nodes[k]->SetZ(z);
        } // end if new node
        else{
          m_ele->nodes[k] = m_msh_line->nod_vector[m_ele->GetNodeIndex(k)];
        }
      } // end for nnodes
    } // end for no_layers
  } // end for quad elements
  if(m_msh_line->ele_vector.size()>0)
    fem_msh_vector.push_back(m_msh_line);
  else 
    delete m_msh_line;
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
05/2005 OK Implementation based on 2D_2 by KM
**************************************************************************/
void CFEMesh::CreateQuadELEFromSFC(Surface*m_sfc)
{
  m_sfc = m_sfc;
///WWW
#ifdef TODO 

  long ir,is;
  double x[4],y[4],z[4];
  CGLPolyline* m_ply = NULL;
  //----------------------------------------------------------------------
  // 
  vector<CGLPolyline*>::iterator p_sfc_ply = m_sfc->polyline_of_surface_vector.begin();//CC
  while(p_sfc_ply!=m_sfc->polyline_of_surface_vector.end()){//CC
    m_ply = *p_sfc_ply; 
    if(m_ply){
      x[0] = m_ply->point_vector[0]->x;
      x[1] = m_ply->point_vector[1]->x;
      x[2] = m_ply->point_vector[2]->x;
      x[3] = m_ply->point_vector[3]->x;
      y[0] = m_ply->point_vector[0]->y;
      y[1] = m_ply->point_vector[1]->y;
      y[2] = m_ply->point_vector[2]->y;
      y[3] = m_ply->point_vector[3]->y;
      z[0] = m_ply->point_vector[0]->z;
      z[1] = m_ply->point_vector[1]->z;
      z[2] = m_ply->point_vector[2]->z;
      z[3] = m_ply->point_vector[3]->z;
    }
    ++p_sfc_ply;
  }
  //----------------------------------------------------------------------
  // Nodes
  int k;
  double N4[4];
  double u[2];
  CMSHNodes* m_nod = NULL;
  for(is=0;is<ns+1;is++){ 
    u[1] = 1. - 2.*(double)is/(double)ns;
    for(ir=0;ir<(nr+1);ir++){
      m_nod = new CMSHNodes();
      u[0] = 1. - 2.*(double)ir/(double)nr;
      ShapeFunctionQuad(N4,u);
      for(k=0;k<4;k++){
  	    m_nod->x += N4[k]*x[k];
	    m_nod->y += N4[k]*y[k];
	    m_nod->z += N4[k]*z[k];
      }
      nod_vector.push_back(m_nod);
    }
  }
  //----------------------------------------------------------------------
  // Elements
  FiniteElement::CElement* m_ele = NULL;
  for(is=0;is<ns;is++){ 
    for(ir=0;ir<nr;ir++){
      m_ele = new FiniteElement::CElement();
      m_ele->type_name = "quad";
      m_ele->nnodes = 4;
      m_ele->GetPatchIndex() = (int)mmp_vector.size(); //m_sfc->mat_group;
      m_ele->nodes = new long[4];
      m_ele->nodes_index[0] = is*(nr+1) + ir;
      m_ele->nodes_index[1] = is*(nr+1) + ir+1;
      m_ele->nodes_index[2] = (is+1)*(nr+1) + ir+1;
      m_ele->nodes_index[3] = (is+1)*(nr+1) + ir;
      m_ele->nodes_xyz = new double[12];
      for(k=0;k<m_ele->nnodes;k++){
        m_ele->nodes_xyz[k]                 = nod_vector[m_ele->nodes_index[k]]->x;
        m_ele->nodes_xyz[k+m_ele->nnodes]   = nod_vector[m_ele->nodes_index[k]]->y;
        m_ele->nodes_xyz[k+2*m_ele->nnodes] = nod_vector[m_ele->nodes_index[k]]->z;
      }
      ele_vector.push_back(m_ele);
    }
  }
  //----------------------------------------------------------------------

#endif
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CFEMesh::AppendLineELE()
{
//WWW
#ifdef TODO 
  int j;
  long i;
  //----------------------------------------------------------------------
  // Create LINE elements
  long no_line_elements = (long)ele_vector.size();
  CFEMesh* m_msh_quad = NULL;
  m_msh_quad = FEMGet("QUADfromSFC");
  long no_line_elements_base = (long)m_msh_quad->ele_vector.size();
  for(j=0;j<no_layer;j++){
    for(i=0;i<no_line_elements_base;i++){
      m_ele = new FiniteElement::CElement;
      m_ele->GetPatchIndex() = mat_group;
      m_ele->type_name = "line";
      m_ele->ElementType = 1;
      m_ele->nnodes = 2;
      m_ele->nodes = new long[2];
      m_ele->nodes_index[0] = no_line_elements + i + j*no_line_elements_base;
      m_ele->nodes_index[1] = m_ele->nodes_index[0]+ no_line_elements_base;
      ele_vector.push_back(m_ele);
    }
  }
  //----------------------------------------------------------------------
  // Create LINE nodes
  CMSHNodes* m_nod = NULL;
  double dz = z_min/(double)no_layer;
  long no_line_nodes = (long)nod_vector.size();
  long no_line_nodes_base = (long)m_msh_quad->ele_vector.size();
  long i_start = no_line_nodes - no_line_nodes_base;
  for(j=1;j<no_layer+1;j++){
    for(i=i_start;i<no_line_nodes;i++){
      m_nod = new CMSHNodes();
      m_nod->x = nod_vector[i]->x;
      m_nod->y = nod_vector[i]->y;
      m_nod->z = nod_vector[i]->z + j*dz;
      nod_vector.push_back(m_nod);
    }
  }
  //----------------------------------------------------------------------

#endif
}


/**************************************************************************
MshLib-Method: CreateLines
Task: Create linear 1-D elements from triangulated meshes
Programing:
01/2004 OK Implementation
04/2005 OK MSH method for case 1
**************************************************************************/
void CFEMesh::CreateLineELEFromPLY(CGLPolyline *m_polyline,int type,CFEMesh*m_msh_ply)
{
  m_polyline = m_polyline;
  type = type;
  m_msh_ply = m_msh_ply;
//WWW
#ifdef TODO 
  long no_points;
  long *nodes = NULL;
  CGLLine *m_line=NULL;
  list<CGLLine*>::const_iterator pl;
  long *element_nodes = NULL;
  int hits;
  long i,j,k;
  double pt1[3],pt2[3];
  long *nodes_unsorted = NULL;
  long no_nodes;
  double *node_distances = NULL;
  list<CGLLine*>msh_line_list;
  list<CGLLine*>::iterator pl1;
  list<CGLLine*>::iterator pl2;
  list<CGLLine*>::iterator pl3;
  long m_point11,m_point12,m_point21,m_point22;
  CGLLine *m_line1,*m_line2;
  bool hitP0,hitP1,hitP2;
  double v1[3],v2[3];
  double angle;
  double eps_angle = 1.;
  CMSHNodes* m_nod = NULL;

  switch(type) {
    //--------------------------------------------------------------------
    case 0: // simply sort by distance
      // 1 - sort by distance
      nodes_unsorted = m_polyline->MSHGetNodesClose(&no_nodes);
      pt1[0] = m_polyline->point_vector[0]->x;
      pt1[1] = m_polyline->point_vector[0]->y;
      pt1[2] = m_polyline->point_vector[0]->z;
      node_distances = new double[no_nodes];
      for(i=0;i<no_nodes;i++) {
        pt2[0] = GetNodeX(nodes_unsorted[i]);
        pt2[1] = GetNodeY(nodes_unsorted[i]);
        pt2[2] = GetNodeZ(nodes_unsorted[i]);
        node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
      }
      nodes = TOLSortNodes1(nodes_unsorted,node_distances,no_nodes);
      delete [] node_distances;
      // 2 - create polyline MSH nodes
      m_polyline->msh_nodes_vector.clear();
      for(i=0;i<no_nodes;i++){
         m_polyline->msh_nodes_vector.push_back(nodes[i]);
      }
      // 3 - create line elements
      m_polyline->MSHCreateLines();
      break;
    //--------------------------------------------------------------------
    case 1: // based on lines
      // 0 - create polyline lines, if necessary
      if(m_polyline->line_list.size()==0)
        m_polyline->ComputeLines(m_polyline);
      // 1 - get MSH nodes close to polyline->lines
      pl = m_polyline->line_list.begin();
      while(pl!=m_polyline->line_list.end()) {
        m_line = *pl;
        m_line->epsilon = m_polyline->epsilon;
        if(ElListSize()>0)
          m_line->SetRFIPointsClose();
        if((long)ele_vector.size()>0)
          SetLINPointsClose(m_line);
        pl++;
      } 
      // 2 - create line elements for polyline->lines
      pl = m_polyline->line_list.begin();
      while(pl!=m_polyline->line_list.end()) {
        m_line = *pl;
        m_line->mat_group = m_polyline->mat_group;
        if(ElListSize()>0)
          m_line->CreateMSHLines();
        if((long)ele_vector.size()>0)
          m_msh_ply->CreateLineELEFromLIN(m_line);
        pl++;
      }
      // 3 - create new MSH nodes for polyline->lines
      pl = m_polyline->line_list.begin();
      while(pl!=m_polyline->line_list.end()) {
        m_line = *pl;
        for(i=0;i<m_line->no_msh_nodes-1;i++){
          m_nod = new CMSHNodes();
          m_nod->origin_rfi_node_number = m_line->msh_nodes[i];
          m_nod->x = nod_vector[m_line->msh_nodes[i]]->x;
          m_nod->y = nod_vector[m_line->msh_nodes[i]]->y;
          m_nod->z = nod_vector[m_line->msh_nodes[i]]->z;
          m_msh_ply->nod_vector.push_back(m_nod);
        }
        pl++;
      }
      pl--;
      m_line = *pl; // last node
      if(m_line){
        m_nod = new CMSHNodes();
        m_nod->origin_rfi_node_number = m_line->msh_nodes[m_line->no_msh_nodes-1];
        m_nod->x = nod_vector[m_line->msh_nodes[m_line->no_msh_nodes-1]]->x;
        m_nod->y = nod_vector[m_line->msh_nodes[m_line->no_msh_nodes-1]]->y;
        m_nod->z = nod_vector[m_line->msh_nodes[m_line->no_msh_nodes-1]]->z;
        m_msh_ply->nod_vector.push_back(m_nod);
      }
      // 4 - renumber elements
      for(i=0;i<(long)m_msh_ply->ele_vector.size();i++){
        m_ele = m_msh_ply->ele_vector[i];
        m_ele->nodes_index[0] = i;
        m_ele->nodes_index[1] = i+1;
      }
      break;
    //--------------------------------------------------------------------
    case 2: // based on triangles
      msh_line_list.clear();
      // 1 - get MSH nodes close to polyline
      nodes = m_polyline->MSHGetNodesClose(&no_points);
      // 2 - determine triangle edges along polyline
      for(i=0;i<ElementListLength;i++){
        if(ElGetElementType(i)==4){
          element_nodes = ElGetElementNodes(i);
          hits=0;
          m_line = new CGLLine();
          m_line->m_point1 = new CGLPoint();
          m_line->m_point2 = new CGLPoint();
          hitP0 = hitP1 = hitP2 = false;
          for(j=0;j<no_points;j++){
            if(nodes[j]==element_nodes[0]){
/*
              if(hits==0)
                m_line->m_point1->gli_point_id = element_nodes[0];
              if(hits==1)
                m_line->m_point2->gli_point_id = element_nodes[0];
              hits++; 
*/
              hitP0 = true;
              hits++;
            }
            if(nodes[j]==element_nodes[1]){
/*
              if(hits==0)
                m_line->m_point1->gli_point_id = element_nodes[1];
              if(hits==1)
                m_line->m_point2->gli_point_id = element_nodes[1];
              hits++;
*/
              hitP1 = true;
              hits++;
            }
            if(nodes[j]==element_nodes[2]){
/*
              if(hits==0)
                m_line->m_point1->gli_point_id = element_nodes[2];
              if(hits==1)
                m_line->m_point2->gli_point_id = element_nodes[2];
              hits++;
*/
              hitP2 = true;
              hits++;
            }
/*
            if(hits==2){
              msh_line_list.push_back(m_line);
              m_line->gli_line_id = i;
              break;
            }
*/
            if(hitP0&&hitP1&&!hitP2){
              v1[0] = GetNodeX(element_nodes[0])-GetNodeX(element_nodes[1]);
              v1[1] = GetNodeY(element_nodes[0])-GetNodeY(element_nodes[1]);
              v1[2] = GetNodeZ(element_nodes[0])-GetNodeZ(element_nodes[1]);
              long point_vector_length = (long)m_polyline->point_vector.size();
              for(k=0;k<point_vector_length-1;k++){
                v2[0] = m_polyline->point_vector[k+1]->x - m_polyline->point_vector[k]->x;
                v2[1] = m_polyline->point_vector[k+1]->y - m_polyline->point_vector[k]->y;
                v2[2] = m_polyline->point_vector[k+1]->z - m_polyline->point_vector[k]->z;
                angle  = MAngleVectors(v1,v2);
                if(angle<eps_angle){
                  m_line->m_point1->gli_point_id = element_nodes[0];
                  m_line->m_point2->gli_point_id = element_nodes[1];
                  msh_line_list.push_back(m_line);
                  m_line->gli_line_id = i;
                }
              }
              hitP0 = hitP1 = false;
            }
            if(hitP1&&hitP2&&!hitP0){
              v1[0] = GetNodeX(element_nodes[1])-GetNodeX(element_nodes[2]);
              v1[1] = GetNodeY(element_nodes[1])-GetNodeY(element_nodes[2]);
              v1[2] = GetNodeZ(element_nodes[1])-GetNodeZ(element_nodes[2]);
              long point_vector_length = (long)m_polyline->point_vector.size();
              for(k=0;k<point_vector_length-1;k++){
                v2[0] = m_polyline->point_vector[k+1]->x - m_polyline->point_vector[k]->x;
                v2[1] = m_polyline->point_vector[k+1]->y - m_polyline->point_vector[k]->y;
                v2[2] = m_polyline->point_vector[k+1]->z - m_polyline->point_vector[k]->z;
                angle  = MAngleVectors(v1,v2);
                if(angle<eps_angle){
                  m_line->m_point1->gli_point_id = element_nodes[1];
                  m_line->m_point2->gli_point_id = element_nodes[2];
                  msh_line_list.push_back(m_line);
                  m_line->gli_line_id = i;
                }
              }
              hitP1 = hitP2 = false;
            }
            if(hitP2&&hitP0&&!hitP1){
              v1[0] = GetNodeX(element_nodes[2])-GetNodeX(element_nodes[0]);
              v1[1] = GetNodeY(element_nodes[2])-GetNodeY(element_nodes[0]);
              v1[2] = GetNodeZ(element_nodes[2])-GetNodeZ(element_nodes[0]);
              long point_vector_length = (long)m_polyline->point_vector.size();
              for(k=0;k<point_vector_length-1;k++){
                v2[0] = m_polyline->point_vector[k+1]->x - m_polyline->point_vector[k]->x;
                v2[1] = m_polyline->point_vector[k+1]->y - m_polyline->point_vector[k]->y;
                v2[2] = m_polyline->point_vector[k+1]->z - m_polyline->point_vector[k]->z;
                angle  = MAngleVectors(v1,v2);
                if(angle<eps_angle){
                  m_line->m_point1->gli_point_id = element_nodes[2];
                  m_line->m_point2->gli_point_id = element_nodes[0];
                  msh_line_list.push_back(m_line);
                  m_line->gli_line_id = i;
                }
              }
              hitP0 = hitP2 = false;
            }
          }
          if(hits<2) {
            delete m_line->m_point1;
            delete m_line->m_point2;
            delete m_line;
          }
        } // Triangles
      }
      // 3 - remove double lines
/*
      pl1=msh_line_list.begin();
      while(pl1!=msh_line_list.end()) {
        node1 = *pl1;
        ++pl1;
*/
/*
        while(pl1!=msh_line_list.end()) {
          node2 = *pl1;
          if( ((node1[0]==node2[0])&&(node1[1]==node2[1])) || \
              ((node1[0]==node2[1])&&(node1[1]==node2[0])) ) {
            msh_line_list.remove(node2);
          }
          ++pl1;
        }
      }
*/
      m_polyline->line_list.clear();
      pl1=msh_line_list.begin();
      while(pl1!=msh_line_list.end()) {
        m_line = *pl1;
        m_line->m_point1->x = GetNodeX(m_line->m_point1->gli_point_id);
        m_line->m_point1->y = GetNodeY(m_line->m_point1->gli_point_id);
        m_line->m_point2->x = GetNodeX(m_line->m_point2->gli_point_id);
        m_line->m_point2->y = GetNodeY(m_line->m_point2->gli_point_id);
        m_polyline->line_list.push_back(m_line);
        ++pl1;
      }
      // Remove double elements
      pl1=msh_line_list.begin();
      while(pl1!=msh_line_list.end()) {
        m_line1 = *pl1;
        m_point11 = m_line1->m_point1->gli_point_id;
        m_point12 = m_line1->m_point2->gli_point_id;
        pl2=msh_line_list.begin();
        while(pl2!=msh_line_list.end()) {
          m_line2 = *pl2;
          if (m_line1->gli_line_id==m_line2->gli_line_id) {
            pl2++;
            continue;
          }
          m_point21 = m_line2->m_point1->gli_point_id;
          m_point22 = m_line2->m_point2->gli_point_id;
          if( ( (m_point11==m_point21)&&(m_point12==m_point22) ) \
            ||( (m_point11==m_point22)&&(m_point12==m_point21) ) ) {
           pl3 = msh_line_list.erase(pl2);
           pl2 = pl3;
          }
          else
            pl2++;
        }
        pl1++;
      }
      // Create elements
      pl1=msh_line_list.begin();
      while(pl1!=msh_line_list.end()) {
        m_line = *pl1;
        m_line->mat_group = m_polyline->mat_group;
        m_line->no_msh_nodes = 2;
        m_line->msh_nodes = new long[2];
        m_line->msh_nodes[0] = m_line->m_point1->gli_point_id;
        m_line->msh_nodes[1] = m_line->m_point2->gli_point_id;
        //m_line->CreateMSHLines();
      ELECreateTopology(1,-1,0,ElementListLength);
      ElSetElementNodes(ElementListLength-1,m_line->msh_nodes);
      ElSetElementGroupNumber(ElementListLength-1,m_line->mat_group);
      anz_1D++;
        pl1++;
      }
      break;
    //--------------------------------------------------------------------
  }
#endif
}


/**************************************************************************
GeoLib-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CFEMesh::CreateLineELEFromLIN(CGLLine*m_lin)
{
  m_lin = m_lin;
//WWW
#ifdef TODO 
  long j;
  for(j=0;j<m_lin->no_msh_nodes-1;j++){
    m_ele = new FiniteElement::CElement;
    m_ele->GetPatchIndex() = m_lin->mat_group;
    m_ele->type_name = "line";
    m_ele->ElementType = 1;
    m_ele->nnodes = 2;
    m_ele->nodes = new long[2];
    m_ele->nodes_index[0] = m_lin->msh_nodes[j];
    m_ele->nodes_index[1] = m_lin->msh_nodes[j+1];
    anz_1D++;
    msh_no_line++;
    ele_vector.push_back(m_ele);
  }
/////
#endif
}
	
/**************************************************************************
MSHLib-Method:
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void CFEMesh::CreateHexELEFromQuad(int no_layer,double layer_thickness)
{
  int j;
  int hempel, hampel;
  long i;
  int k;
  long size;
  long no_quad_elements = (long)ele_vector.size();
  long no_quad_nodes = (long)nod_vector.size();
  Mesh_Group::CElem* m_quad_ele = NULL;
  Mesh_Group::CElem* m_ele = NULL;

  //----------------------------------------------------------------------
  // Create MSH
  MSHDelete("HEX_from_QUAD");
  CFEMesh* m_msh_hex = NULL;
  m_msh_hex = new CFEMesh();
  m_msh_hex->pcs_name = "HEX_from_QUAD";
  m_msh_hex->ele_type = 3;
  m_msh_hex->no_msh_layer = no_layer;
  //----------------------------------------------------------------------
  // Create HEX elements
  size = (no_layer + 1) * no_quad_nodes; 
  m_msh_hex->nod_vector.resize(size);

  for(j=0;j<size;j++)
    m_msh_hex->nod_vector[j] = NULL;
   
  for(j=0;j<no_layer;j++){
    for(i=0;i<no_quad_elements;i++){
      //Elements
      m_quad_ele = ele_vector[i];
      m_ele = new Mesh_Group::CElem;
      //m_ele->SetPatchIndex(j);
      m_ele->SetIndex((j*no_quad_elements) + i);
      m_ele->SetElementType(3);
      m_ele->nnodes = 8;
      m_ele->nodes_index.resize(m_ele->nnodes);
      //Set indices
      m_ele->nodes_index[0] = m_quad_ele->GetNodeIndex(0) + j*no_quad_nodes;
      m_ele->nodes_index[1] = m_quad_ele->GetNodeIndex(1) + j*no_quad_nodes;
      m_ele->nodes_index[2] = m_quad_ele->GetNodeIndex(2) + j*no_quad_nodes;
      m_ele->nodes_index[3] = m_quad_ele->GetNodeIndex(3) + j*no_quad_nodes;
      m_ele->nodes_index[4] = m_ele->GetNodeIndex(0)+ no_quad_nodes;
      m_ele->nodes_index[5] = m_ele->GetNodeIndex(1)+ no_quad_nodes;
      m_ele->nodes_index[6] = m_ele->GetNodeIndex(2)+ no_quad_nodes;
      m_ele->nodes_index[7] = m_ele->GetNodeIndex(3)+ no_quad_nodes;
      //Nodes
      hempel = 0;
      hampel = 0; 
      for(k=0;k<m_ele->nnodes;k++){
        if(m_msh_hex->nod_vector[m_ele->GetNodeIndex(k)]==NULL){
          m_ele->nodes[k] = new CNode(m_ele->GetNodeIndex(k));
          m_msh_hex->nod_vector[m_ele->GetNodeIndex(k)] = m_ele->nodes[k];
          if(k>3){ 
            hempel = 4;
            hampel = 1;
          }
          m_ele->nodes[k]->SetX(nod_vector[m_quad_ele->GetNodeIndex(k-hempel)]->X());
          m_ele->nodes[k]->SetY(nod_vector[m_quad_ele->GetNodeIndex(k-hempel)]->Y());
          m_ele->nodes[k]->SetZ(nod_vector[m_quad_ele->GetNodeIndex(k-hempel)]->Z() - (j+hampel)*layer_thickness);      
        }
        else {
          m_ele->nodes[k] = m_msh_hex->nod_vector[m_ele->GetNodeIndex(k)];
        }
      } //end for m_ele->nnodes
      //msh_no_hexs++;
      m_msh_hex->ele_vector.push_back(m_ele);
    } //end for no_quad_elements
  } //end for no_layers
  //----------------------------------------------------------------------
  if(m_msh_hex->ele_vector.size()>0)
    fem_msh_vector.push_back(m_msh_hex);
  else 
    delete m_msh_hex;

}

/**************************************************************************
MSHLib-Method: 
Task:
Programing:
04/2005 OK Implementation based on MSH2RFI by WW
08/2005 WW Re-implememtation
10/2005 TK proper ordering and closing of gaps
**************************************************************************/
void GMSH2MSH(char* filename,CFEMesh* m_msh)
{
  long id;
  long i=0;
  int NumNodes=0;
  int NumElements=0;
  double x,y,z;
  string strbuffer;

  bool quad=false;
  CRFProcess* m_pcs = NULL;
  for(i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
	if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos){
      quad=true;
      break;
	}
  }

  CNode* node = NULL;
  CElem* elem = NULL;
  ifstream msh_file(filename,ios::in);
  //----------------------------------------------------------------------
  while (strbuffer != "$ENDELM") 
  {
    getline(msh_file, strbuffer);// Node keyword
    msh_file>>NumNodes>>ws;
    //....................................................................
	// Node data
    for(i=0;i<NumNodes;i++){
      msh_file>>id>>x>>y>>z>>ws;
      node = new CNode(id,x,y,z);
	  m_msh->nod_vector.push_back(node);
    }

	getline(msh_file, strbuffer); // End Node keyword
    //....................................................................
	// Element data
    getline(msh_file, strbuffer); // Element keyword
    msh_file>>NumElements>>ws;
    for(i=0;i<NumElements; i++)
	{
      elem = new CElem(i);
	  elem->Read(msh_file, 2);
      m_msh->ele_vector.push_back(elem);
	}
    getline(msh_file, strbuffer);// END keyword

  // ordering nodes and closing gaps TK
  vector<int> gmsh_id;
  long new_node_id;
  int counter=0;
  int diff=0;
  int j=0;
    for(i=0;i<(int)m_msh->nod_vector.size();i++){
        diff = m_msh->nod_vector[i]->GetIndex()-counter;
        if (diff == 0){
            gmsh_id.push_back(i);
            counter++;
        }
        else {
            for(j=0;j<diff;j++){
            gmsh_id.push_back(i);
            counter++;
            }
            i--;
        }
    }

    for(i=0;i<(int)m_msh->ele_vector.size();i++){
        for(j=0;j<(int)m_msh->ele_vector[i]->GetVertexNumber();j++){
            new_node_id = gmsh_id[m_msh->ele_vector[i]->GetNodeIndex(j)+1];
            //m_msh->ele_vector[i]->nodes[j]->SetIndex(new_node_id);/*global*/ 
            m_msh->ele_vector[i]->nodes_index[j]=new_node_id;/*local*/ 
        }
    }
    for(i=0;i<(int)m_msh->nod_vector.size();i++){
        m_msh->nod_vector[i]->SetIndex(i);
    }
    // END OF: ordering nodes and closing gaps TK


  }/*End while*/ 

  //----------------------------------------------------------------------
  m_msh->ConstructGrid(quad);
  
  msh_file.close();
}

/**************************************************************************
MSHLib-Method: 
Task:   Takes Surface, meshs it and saves it with defined file name

        surface_name         = name of surface for meshing
        file_name_const_char = path + name without extension

        Attention: You have to remove/delete files extern.

Programing:
12/2005 TK implementation
**************************************************************************/
void Mesh_Single_Surface(string surface_name, const char *file_name_const_char)
{
  
  //Searching Methods
  int i=0, j=0, k=0;
  string Name;
  vector<CGLPoint*> surface_points_searchvector;
  CGLPoint *m_point = NULL;

  for (i=0; i<(int)surface_points_searchvector.size();i++)
  {
    delete surface_points_searchvector[i];
  }
    surface_points_searchvector.clear();

  //Get SFC
  for (i=0;i<(int)surface_vector.size();i++)
  {
      if (surface_vector[i]->name.find(surface_name)==0)
      {
        for (j=0;j<(int)surface_vector[i]->polyline_of_surface_vector.size();j++)
        {
	      if (j==0)
          {
          for (k=0;k<(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size();k++)
		  {
            m_point = new CGLPoint;
            m_point->nb_of_ply = i;
            m_point->id = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->id;
            m_point->x  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->x;
            m_point->y  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->y;
            m_point->z  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->z;
            surface_points_searchvector.push_back(m_point);
            if (k==(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1 &&
                surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->id == surface_vector[i]->polyline_of_surface_vector[j]->point_vector[0]->id)
            {
                surface_points_searchvector.erase(surface_points_searchvector.begin()+k);
            }
          }
          }

          else
          {
          if (m_point->id != surface_vector[i]->polyline_of_surface_vector[j]->point_vector[0]->id && 
          m_point->id == surface_vector[i]->polyline_of_surface_vector[j]->point_vector[(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1]->id)
		  {
            for (k=1;k<(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size();k++)
            {
            m_point = new CGLPoint;
            m_point->nb_of_ply = i;
            m_point->id = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1-k]->id;
            m_point->x  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1-k]->x;
            m_point->y  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1-k]->y;
            m_point->z  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size()-1-k]->z;
            surface_points_searchvector.push_back(m_point);                   
            }
          }
          else
          {
            for (k=1;k<(int)surface_vector[i]->polyline_of_surface_vector[j]->point_vector.size();k++)
            {
            m_point = new CGLPoint;
            m_point->nb_of_ply = i;
            m_point->id = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->id;
            m_point->x  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->x;
            m_point->y  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->y;
            m_point->z  = surface_vector[i]->polyline_of_surface_vector[j]->point_vector[k]->z;
            surface_points_searchvector.push_back(m_point);
            }
          }
          }
		}

        if (surface_points_searchvector[0]->id == surface_points_searchvector[surface_points_searchvector.size()-1]->id)
            surface_points_searchvector.erase(surface_points_searchvector.begin());


  //Write GMSH_GEO_FILE of marked Surface and mesh it
  string m_strFileNameGEO = file_name_const_char;
  m_strFileNameGEO = m_strFileNameGEO + ".geo";
  file_name_const_char = m_strFileNameGEO.data();
  FILE *geo_file=NULL;
  geo_file = fopen(file_name_const_char, "w+t");
  long id=0;
  double density = 1e100;
  double topologic_distance;

          for (k=0;k<(int)surface_points_searchvector.size();k++)
          {
            if (k==0)
            {
             density =   EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[k+1]->x,
                                            surface_points_searchvector[k+1]->y,
                                            surface_points_searchvector[k+1]->z);
             topologic_distance = EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[surface_points_searchvector.size()-1]->x,
                                            surface_points_searchvector[surface_points_searchvector.size()-1]->y,
                                            surface_points_searchvector[surface_points_searchvector.size()-1]->z);
             if (topologic_distance < density) density = topologic_distance;
            }

            if (k>0 && k<(int)surface_points_searchvector.size()-1)
            {
             density =   EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[k-1]->x,
                                            surface_points_searchvector[k-1]->y,
                                            surface_points_searchvector[k-1]->z);
             topologic_distance = EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[k+1]->x,
                                            surface_points_searchvector[k+1]->y,
                                            surface_points_searchvector[k+1]->z);
             if (topologic_distance < density) density = topologic_distance;
            }

            if (k == (int)surface_points_searchvector.size()-1)
            {
             density =   EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[k-1]->x,
                                            surface_points_searchvector[k-1]->y,
                                            surface_points_searchvector[k-1]->z);
             topologic_distance = EuklVek3dDistCoor ( surface_points_searchvector[k]->x,
                                            surface_points_searchvector[k]->y,
                                            surface_points_searchvector[k]->z,
                                            surface_points_searchvector[0]->x,
                                            surface_points_searchvector[0]->y,
                                            surface_points_searchvector[0]->z);
             if (topologic_distance < density) density = topologic_distance;
            }


            id++;
            fprintf(geo_file,"%s","Point(");
		    fprintf(geo_file,"%li",id);
		    fprintf(geo_file,"%s",") = {");
		    fprintf(geo_file,"%g",surface_points_searchvector[k]->x);
		    fprintf(geo_file,"%s",", ");
		    fprintf(geo_file,"%g",surface_points_searchvector[k]->y);
		    fprintf(geo_file,"%s",", ");
		    fprintf(geo_file,"%g",surface_points_searchvector[k]->z);
		    fprintf(geo_file,"%s",", ");
		    fprintf(geo_file,"%g",density);
		    fprintf(geo_file,"%s\n","};");
                    
          }
          id=0;
          for (k=0;k<(int)surface_points_searchvector.size()-1;k++)
          {
            id++;
			fprintf(geo_file,"%s","Line(");
			fprintf(geo_file,"%li",id);
			fprintf(geo_file,"%s",") = {");
			fprintf(geo_file,"%d",k+1);
			fprintf(geo_file,"%s",", ");
			fprintf(geo_file,"%d",k+2);
			fprintf(geo_file,"%s\n","};");
                
          }
            id++;
            fprintf(geo_file,"%s","Line(");
		    fprintf(geo_file,"%li",id);
		    fprintf(geo_file,"%s",") = {");
		    fprintf(geo_file,"%ld",id);
		    fprintf(geo_file,"%s",", 1");
		    fprintf(geo_file,"%s\n","};");
        
        	fprintf(geo_file,"%s","Line Loop(");
			fprintf(geo_file,"%li",id+1);
			fprintf(geo_file,"%s",") = {");	
			for (k=0;k<(int)surface_points_searchvector.size();k++)
			{	
				fprintf(geo_file,"%i",k+1);
				if (k<(int)surface_points_searchvector.size()-1) 
                    fprintf(geo_file,"%s",", ");
			}
			fprintf(geo_file,"%s\n","};");

       		fprintf(geo_file,"%s","Plane Surface(");
    	    fprintf(geo_file,"%li",id+2);
		    fprintf(geo_file,"%s",") = {");
            fprintf(geo_file,"%ld",id+1);
            fprintf(geo_file,"%s\n","};");

       		fprintf(geo_file,"%s","Physical Surface(");
    	    fprintf(geo_file,"%li",id+3);
		    fprintf(geo_file,"%s",") = {");
            fprintf(geo_file,"%ld",id+2);
            fprintf(geo_file,"%s\n","};");

  fclose(geo_file);
  string m_strExecuteGEO = "..\\LIB\\gmsh " + m_strFileNameGEO +" -2";
  const char *m_strExecute=0;
  m_strExecute = m_strExecuteGEO.data();
  system(m_strExecute); 
  //remove(file_name_const_char);
  //END Meshing
       break;
      }
  }
}

/**************************************************************************
MSHLib-Method: 
Task:    file_name_const_char = Path + Name  without extension of GMSH *msh-File

Programing:
12/2005 TK implementation
**************************************************************************/
void Select_Nodes_Elements_by_GMSHFile(const char *file_name_const_char)
{
   int i=0, j=0, k=0;
//READ GMSH-File and fill local Element Vector
  vector<Mesh_Group::CFEMesh*>check_msh_vector;
  Mesh_Group::CFEMesh* m_check_elements;
  char text[1024];
  long nbnod, nbelm;
  long node_id;
  long pnt;
  double x,y,z;

  string m_strFileNameGMSH = file_name_const_char;
  m_strFileNameGMSH = m_strFileNameGMSH + ".msh";
  file_name_const_char = m_strFileNameGMSH.data();
  ifstream gmsh_file (file_name_const_char,ios::in);

  m_check_elements = new Mesh_Group::CFEMesh;
  while (!gmsh_file.eof()) 
  {
    gmsh_file.getline(text, 1024);
    if (!strncmp(text,"$NOD",4)){
        gmsh_file>>nbnod>>ws;
        m_check_elements->nod_vector.resize(nbnod);
        for(i=0;i<nbnod;i++)
        {
            gmsh_file>>node_id>>x>>y>>z>>ws;  
            m_check_elements->nod_vector[i] = new CNode(i,x,y,z);
        }

    }
    if (!strncmp(text,"$ELM",4)){
        gmsh_file>>nbelm>>ws; 
        m_check_elements->ele_vector.resize(nbelm);
        for(i=0;i<nbelm;i++)
        {
            m_check_elements->ele_vector[i] = new CElem(i);
	        m_check_elements->ele_vector[i]->Read(gmsh_file, 2);
        }

    }
   if (!strncmp(text,"$ENDELM",7)) break;
  }

//Loop over all generated triangles of surface
  CGLPoint point;
  double angle_sum, dist;
  double tolerance = 0.001;
  double tri_point1[3],tri_point2[3],tri_point3[3],checkpoint[3];
  double tri_x[3],tri_y[3],tri_z[3];
  double min_mesh_dist=0.0;

  for(k=0;k<(int)m_check_elements->ele_vector.size();k++)
  {    
        pnt = m_check_elements->ele_vector[k]->GetNodeIndex(0);
         tri_point1[0] = m_check_elements->nod_vector[pnt]->X();
         tri_point1[1] = m_check_elements->nod_vector[pnt]->Y();
         tri_point1[2] = m_check_elements->nod_vector[pnt]->Z();
        pnt = m_check_elements->ele_vector[k]->GetNodeIndex(1);
         tri_point2[0] = m_check_elements->nod_vector[pnt]->X();
         tri_point2[1] = m_check_elements->nod_vector[pnt]->Y();
         tri_point2[2] = m_check_elements->nod_vector[pnt]->Z();
        pnt = m_check_elements->ele_vector[k]->GetNodeIndex(2);
         tri_point3[0] = m_check_elements->nod_vector[pnt]->X();
         tri_point3[1] = m_check_elements->nod_vector[pnt]->Y();
         tri_point3[2] = m_check_elements->nod_vector[pnt]->Z();
         
         tri_x[0]=tri_point1[0];
         tri_x[1]=tri_point2[0];
         tri_x[2]=tri_point3[0];
         tri_y[0]=tri_point1[1];
         tri_y[1]=tri_point2[1];
         tri_y[2]=tri_point3[1];
         tri_z[0]=tri_point1[2];
         tri_z[1]=tri_point2[2];
         tri_z[2]=tri_point3[2];
    //Loop over all meshes
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
    //Loop over all edges
        for(i=0;i<(long)fem_msh_vector[j]->edge_vector.size();i++)
        {
            if (j==0 && i==0){
              min_mesh_dist = fem_msh_vector[j]->edge_vector[i]->Length();
            }
            else{
              if (min_mesh_dist  > fem_msh_vector[j]->edge_vector[i]->Length())
                  min_mesh_dist = fem_msh_vector[j]->edge_vector[i]->Length();
            }
        }
        tolerance = min_mesh_dist;
    //Loop over all mesh nodes
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
            point.x = fem_msh_vector[j]->nod_vector[i]->X();
            point.y = fem_msh_vector[j]->nod_vector[i]->Y();
            point.z = fem_msh_vector[j]->nod_vector[i]->Z();
            checkpoint[0] = fem_msh_vector[j]->nod_vector[i]->X();
            checkpoint[1] = fem_msh_vector[j]->nod_vector[i]->Y(); 
            checkpoint[2] = fem_msh_vector[j]->nod_vector[i]->Z();
            dist = MCalcDistancePointToPlane(checkpoint,tri_point1,tri_point2,tri_point3);
            if (k==0) fem_msh_vector[j]->nod_vector[i]->epsilon = dist;
            else
            {
                if (fem_msh_vector[j]->nod_vector[i]->epsilon > dist)
                    fem_msh_vector[j]->nod_vector[i]->epsilon = dist;
            }
                if (dist<=tolerance && dist>=-tolerance)
                {
                  angle_sum = AngleSumPointInsideTriangle(checkpoint,tri_point1,tri_point2,tri_point3, min_mesh_dist);
                  //if (point.PointInTriangle(tri_x,tri_y,tri_z) || angle_sum>359)
                  //if (point.PointInTriangle(tri_x,tri_y,tri_z))
                  if(angle_sum>359)
                  fem_msh_vector[j]->nod_vector[i]->selected = 1;
                }
        }     
        // Loop over all mesh elements
        vec<long> node_index(20);
        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
            fem_msh_vector[j]->ele_vector[i]->GetNodeIndeces(node_index);

            if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 3) /*Quad*/ 
            {
                if (fem_msh_vector[j]->nod_vector[node_index[0]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[1]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[2]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[3]]->selected == 1 )
                    fem_msh_vector[j]->ele_vector[i]->selected = 1;   
            }

            if (fem_msh_vector[j]->ele_vector[i]->GetElementType() == 4) /*TRI*/ 
            {
                if (fem_msh_vector[j]->nod_vector[node_index[0]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[1]]->selected == 1 &&
                    fem_msh_vector[j]->nod_vector[node_index[2]]->selected == 1 )
                    fem_msh_vector[j]->ele_vector[i]->selected = 1;  
            }
        }     


    }
  }

}

/**************************************************************************
MSHLib-Method: 
Task:    Clears the selection and set flag selected = 0;
Programing:
12/2005 TK implementation
**************************************************************************/
void Clear_Selected_Nodes_Elements()
{
int i=0, j=0;
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
        for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
        {
          fem_msh_vector[j]->nod_vector[i]->epsilon = 0.0;
          fem_msh_vector[j]->nod_vector[i]->selected= 0;
        }     

        for(i=0;i<(long)fem_msh_vector[j]->ele_vector.size();i++)
        {
          fem_msh_vector[j]->ele_vector[i]->selected= 0;
        }     
    }
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
11/2005 MB
**************************************************************************/
void CFEMesh::SetMSHPart(vector<long>&elements_active, long StrangNumber)
{
  int j;
  int k;
  long i;
  long size;
  CElem* m_ele = NULL;
  CFEMesh* m_msh_strang = NULL;
  CNode* m_nod = NULL;
  bool found = false;
  
  size = (long)elements_active.size();

  m_msh_strang = FEMGet("MSH_Strang");
 
  // Create MSH
  if (!m_msh_strang){
    m_msh_strang = new CFEMesh();
    m_msh_strang->pcs_name = "MSH_Strang";
    m_msh_strang->ele_type = 1;
    m_msh_strang->no_msh_layer = no_msh_layer;
    //Resize
    m_msh_strang->ele_vector.resize(size);
    m_msh_strang->Eqs2Global_NodeIndex.resize(size+1);
    
    fem_msh_vector.push_back(m_msh_strang);
  }

  m_msh_strang->nod_vector.resize(0);

  //Elements
  for(i=0;i<(long)elements_active.size();i++){
    //Elements
    m_ele = ele_vector[elements_active[i]];
    m_msh_strang->ele_vector[i] = m_ele;
    //Nodes
    for(k=0;k<m_ele->nnodes;k++){
      m_nod = m_ele->GetNode(k);
      found = false;
      for(j=0; j<(int)m_msh_strang->nod_vector.size(); j++) {
        if(*m_msh_strang->nod_vector[j]==*m_nod)
          found = true;
      }
      if(!found)
        m_msh_strang->nod_vector.push_back(m_nod);
    }
   // cout<< "Element"  << i << "  " << m_ele->GetNodeIndex(0)<< "  "  << m_ele->GetNodeIndex(1) <<  endl;
  } 

  for(i=0; i<(long)m_msh_strang->nod_vector.size(); i++)  {
 	m_msh_strang->nod_vector[i]->SetEquationIndex(i);
    m_msh_strang->Eqs2Global_NodeIndex[i] = m_msh_strang->nod_vector[i]->GetIndex(); //+ test;
	//cout<< " " << i << "  " << m_msh_strang->Eqs2Global_NodeIndex[i] <<  endl;
  } 
  
}

