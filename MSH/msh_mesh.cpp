/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#include "stdafx.h" // MFC
#ifdef MFC
#include "afxpriv.h"    // For WM_SETMESSAGESTRING
#endif
#include "mathlib.h"
// C++
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
#include "gs_project.h"
// GEOLib
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_sfc.h"
// MSHLib
#include "elements.h"
#include "nodes.h"
#include "msh_mesh.h"
#ifdef RANDOM_WALK
#include "rf_random_walk.h"
#endif
#include "msh_lib.h"
// PCSLib
extern string GetLineFromFile1(ifstream*);

long msh_no_line = 0;
long msh_no_quad = 0;
long msh_no_hexs = 0;
long msh_no_tris = 0;
long msh_no_tets = 0;
long msh_no_pris = 0;

//========================================================================
namespace Mesh_Group
{
/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
**************************************************************************/
CFEMesh::CFEMesh(void)
{
  selected = false;
  no_msh_layer = 0;
  useQuadratic = false;
  coordinate_system = 1;
  axisymmetry=false; //WW
  cross_section=false; //MB
  no_msh_layer = 0; //OK
  min_edge_length = 1e-3; //OK
  max_mmp_groups = 0; //OKCC

#ifdef RANDOM_WALK
  PT=NULL; // WW+TK
#endif
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
07/2005 WW Changes due to the geometry objects
01/2006 YD Changes for face normal
**************************************************************************/
CFEMesh::~CFEMesh(void)
{
  long i;
  // Nodes
  for(i=0; i<(long)nod_vector.size(); i++)
    delete nod_vector[i];
  nod_vector.clear();
  // Edges
  for(i=0; i<(long)edge_vector.size(); i++)
     delete edge_vector[i];
  edge_vector.clear();
  // Surface faces
  for(i=0; i<(long)face_vector.size(); i++)
     delete face_vector[i];
  face_vector.clear();
  // Element
  for(i=0; i<(long)ele_vector.size(); i++)
     delete ele_vector[i];
  ele_vector.clear();
  // normal  YD
  for(i=0; i<(long)face_normal.size(); i++)
     delete face_normal[i];
  face_normal.clear();

#ifdef RANDOM_WALK
  delete PT; // PCH
#endif
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
07/2005 WW Changes due to the geometry objects
08/2005 WW/MB Keyword CrossSection
09/2005 WW 2D-3D flag
12/2005 OK MAT_TYPE
**************************************************************************/
ios::pos_type CFEMesh::Read(ifstream *fem_file)
{
  string sub_line;
  string line_string;
  bool new_keyword = false;
  string hash("#");
  ios::pos_type position;
  string sub_string,sub_string1;
  long i, ibuff;
  long no_elements;
  long no_nodes;
  double x,y,z;
  CNode* newNode = NULL;
  CElem* newElem = NULL;
  //========================================================================
  // Keyword loop
  while (!new_keyword) {
    position = fem_file->tellg();
    //if(!GetLineFromFile(line,fem_file)) 
    //  break;
    //line_string = line;
    getline(*fem_file, line_string);
    if(fem_file->fail()) 
      break;
    if(line_string.find(hash)!=string::npos)
	{
      new_keyword = true;
      break;
    }
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
      *fem_file >> pcs_name>>ws; //WW
      continue;
    }
    //....................................................................
    if(line_string.find("$GEO_NAME")!=string::npos) { // subkeyword found
      *fem_file >> geo_name >> ws; //WW
      continue;
    }
    //....................................................................
    if(line_string.find("$AXISYMMETRY")!=string::npos) { // subkeyword found
      axisymmetry=true;
      continue;
    }
    //....................................................................
    if(line_string.find("$CROSS_SECTION")!=string::npos) { // subkeyword found
      cross_section=true;
      continue;
    }
    //....................................................................
    if(line_string.find("$NODES")!=string::npos) { // subkeyword found
      *fem_file  >> no_nodes>>ws;
      for(i=0;i<no_nodes;i++){
         *fem_file>>ibuff>>x>>y>>z>>ws;
         newNode = new CNode(ibuff,x,y,z);
         nod_vector.push_back(newNode);            
      }
      continue;
    }
    //....................................................................
    if(line_string.find("$ELEMENTS")!=string::npos) { // subkeyword found
      *fem_file >> no_elements>>ws;
      for(i=0;i<no_elements;i++){
         newElem = new CElem(i);
         newElem->Read(*fem_file);
        ele_type = newElem->geo_type ;//CC02/2006
         if(newElem->GetPatchIndex()>max_mmp_groups)
           max_mmp_groups = newElem->GetPatchIndex();
		 ele_vector.push_back(newElem);
      }
      continue;
    }
    //....................................................................
    if(line_string.find("$LAYER")!=string::npos) { // subkeyword found
       *fem_file >> no_msh_layer>>ws;
       continue;
    }
    //....................................................................
  }
  //========================================================================
  return position;
}

/**************************************************************************
FEMLib-Method: ConnectedElements2Node
Task: 
Programing:
04/2006 WW Cut from Construct grid
**************************************************************************/
void CFEMesh::ConnectedElements2Node()
{
   long i, j, e, ni;
   CElem* thisElem0=NULL;
//   CElem* thisElem=NULL;
   bool done = false;
   // Set neighbors of node
   for(i=0; i<(long)nod_vector.size(); i++)
     nod_vector[i]->connected_elements.clear();
   for(e=0; e<(long)ele_vector.size(); e++)
   {
      thisElem0 = ele_vector[e];   
      if(!thisElem0->GetMark()) continue;
      for(i=0; i<thisElem0->nnodes; i++)
       {
          done = false;
          ni = thisElem0->GetNodeIndex(i);
          for(j=0; j<(int)nod_vector[ni]->connected_elements.size(); j++)
          {
            if(e==nod_vector[ni]->connected_elements[j])
            {
              done = true;
              break;
            } 
          }
          if(!done)  
            nod_vector[ni]->connected_elements.push_back(e);
      }
   }
}

/**************************************************************************
FEMLib-Method: Construct grid
Task: Establish topology of a grid
Programing:
05/2005 WW Implementation
02/2006 YD Add 1D line neighbor element set
**************************************************************************/
void CFEMesh::ConstructGrid( const bool quadratic)
{
   int counter;	 
   int i, j, k, ii, jj, m0, m, n0, n;
			int nnodes0, nedges0, nedges;
   long e, ei, ee,  e_size,  e_size_l;
   bool done;
   double x0,y0,z0;
   double x_sum,y_sum,z_sum;

   int edgeIndex_loc0[2];
   int edgeIndex_loc[2];
   int faceIndex_loc0[10];
   int faceIndex_loc[10];
   vec<CNode*> e_nodes0(20);
   vec<long> node_index_glb(20);
   vec<long> node_index_glb0(20);
   vec<int> Edge_Orientation(15);
   vec<CEdge*> Edges(15);
   vec<CEdge*> Edges0(15);
   vec<CElem*> Neighbors(15);
   vec<CElem*> Neighbors0(15);

   vec<CNode*> e_edgeNodes0(3);
   vec<CNode*> e_edgeNodes(3);
   CElem* thisElem0=NULL;
   CElem* thisElem=NULL;

   //Elem->nodes not initialized

   e_size = (long)ele_vector.size();  
   NodesNumber_Linear= (long)nod_vector.size();

   Edge_Orientation = 1;
  //----------------------------------------------------------------------
  // Set neighbors of node
  ConnectedElements2Node();
  //----------------------------------------------------------------------

//TEST WW
//#ifdef PARALLEL
  ConnectedNodes();
//#endif
  //----------------------------------------------------------------------
   // Compute neighbors and edges
   for(e=0; e<e_size; e++)
   {
       thisElem0 = ele_vector[e];   
       nnodes0 = thisElem0->nnodes; // Number of nodes for linear element
       thisElem0->GetNodeIndeces(node_index_glb0);
       thisElem0->GetNeighbors(Neighbors0);
       for(i=0; i<nnodes0; i++) // Nodes
         e_nodes0[i] = nod_vector[node_index_glb0[i]];  
       m0 = thisElem0->GetFacesNumber();
	   // neighbors
       if(thisElem0->GetDimension()==2){     //YD
       for(i=0; i<m0; i++) // Faces
       {
          if(Neighbors0[i])
               continue;
          n0 = thisElem0->GetElementFaceNodes(i, faceIndex_loc0);
          done = false;  
          for(k=0;k<n0;k++)
          {    
             e_size_l = (long)e_nodes0[faceIndex_loc0[k]]->connected_elements.size();         
             for(ei=0; ei<e_size_l; ei++)
             {
                ee = e_nodes0[faceIndex_loc0[k]]->connected_elements[ei];   
                if(ee==e) continue;
                thisElem = ele_vector[ee];   
                thisElem->GetNodeIndeces(node_index_glb);
                thisElem->GetNeighbors(Neighbors);
                m = thisElem->GetFacesNumber();

                for(ii=0; ii<m; ii++) // Faces
                {
                   n = thisElem->GetElementFaceNodes(ii, faceIndex_loc);
                   if(n0!=n) continue;
                   counter = 0;
                   for(j=0; j<n0; j++) 
                   {             
                      for(jj=0; jj<n; jj++) 
                      {
                         if(node_index_glb0[faceIndex_loc0[j]]
                             ==node_index_glb[faceIndex_loc[jj]])    
                         {
                             counter++;
                             break;
                         }
                      }
                   }
                   if(counter==n)
                   {
                       Neighbors0[i] = thisElem;
                       Neighbors[ii] = thisElem0;  
                       thisElem->SetNeighbor(ii, thisElem0);
                       done = true;
                       break;                 
                   }
                }
                if(done) break;
             }
             if(done) break;
          }
       }
       thisElem0->SetNeighbors(Neighbors0);						
      }
//------------neighbor of 1D line
      if(thisElem0->GetDimension()==1){    //YD
       ii = 0;
       for(i=0; i<m0; i++)
       {
          n0 = thisElem0->GetElementFaceNodes(i, faceIndex_loc0);
          for(k=0;k<n0;k++)
          {
           e_size_l = (long)e_nodes0[faceIndex_loc0[k]]->connected_elements.size();  
             for(ei=0; ei<e_size_l; ei++)
             {
                ee = e_nodes0[faceIndex_loc0[k]]->connected_elements[ei];  
                thisElem = ele_vector[ee]; 
                if(e_size_l == 2 && thisElem->GetIndex() != thisElem0->GetIndex()){
                       Neighbors0[i] = thisElem;
                       Neighbors[ii] = thisElem;
                      // thisElem->SetNeighbor(ii, thisElem0);   //?? Todo YD
                       ii++;
                }
             }
          }  
       }
       thisElem0->SetNeighbors(Neighbors0);	
      }

       // --------------------------------
       // Edges
       nedges0 = thisElem0->GetEdgesNumber();
       thisElem0->GetEdges(Edges0);
       for(i=0; i<nedges0; i++)
       { 
          thisElem0->GetLocalIndicesOfEdgeNodes(i, edgeIndex_loc0);    
          // Check neighbors 
          done = false; 
          for(k=0;k<2;k++)
          {    
             e_size_l = (long)e_nodes0[edgeIndex_loc0[k]]->connected_elements.size();         
             for(ei=0; ei<e_size_l; ei++)
             {
                ee = e_nodes0[edgeIndex_loc0[k]]->connected_elements[ei];   
                if(ee==e) continue;
                thisElem = ele_vector[ee];                   
                thisElem->GetNodeIndeces(node_index_glb);
                nedges = thisElem->GetEdgesNumber();
                thisElem->GetEdges(Edges);
                // Edges of neighbors
                for(ii=0; ii<nedges; ii++)
                { 
                    thisElem->GetLocalIndicesOfEdgeNodes(ii, edgeIndex_loc);
                    if((  node_index_glb0[edgeIndex_loc0[0]]==node_index_glb[edgeIndex_loc[0]]
                        &&node_index_glb0[edgeIndex_loc0[1]]==node_index_glb[edgeIndex_loc[1]])
				                 ||(  node_index_glb0[edgeIndex_loc0[0]]==node_index_glb[edgeIndex_loc[1]]
                        &&node_index_glb0[edgeIndex_loc0[1]]==node_index_glb[edgeIndex_loc[0]]) )
                     {
                         if(Edges[ii])
                         {
                            Edges0[i] = Edges[ii]; 
                            Edges[ii]->GetNodes(e_edgeNodes); 
                            if(  node_index_glb0[edgeIndex_loc0[0]]==e_edgeNodes[1]->GetIndex()
                             && node_index_glb0[edgeIndex_loc0[1]]==e_edgeNodes[0]->GetIndex())
			                             Edge_Orientation[i] = -1; 
                            if(quadratic)  // Get middle node
                            {
                               node_index_glb0[nnodes0] = e_edgeNodes[2]->GetIndex();
                               e_nodes0[nnodes0] = e_edgeNodes[2];
                               nnodes0++;
                            }
                            done = true;
                            break;
                        }
		            }                   
                } //  for(ii=0; ii<nedges; ii++)
                if(done) break;
             } // for(ei=0; ei<e_size_l; ei++)
             if(done) break;
          }//for(k=0;k<2;k++)
          if(!done) // new edges and new node
          {
              Edges0[i] = new CEdge((long)edge_vector.size()); 
              Edges0[i]->SetOrder(quadratic); 
              e_edgeNodes0[0] = e_nodes0[edgeIndex_loc0[0]];
              e_edgeNodes0[1] = e_nodes0[edgeIndex_loc0[1]];
              if(quadratic)  // new node: middle point of edges
              {
                  e_edgeNodes0[2] = new CNode((long)nod_vector.size());
                  e_edgeNodes0[2]->SetX(0.5*(e_edgeNodes0[0]->X()+e_edgeNodes0[1]->X()));                
                  e_edgeNodes0[2]->SetY(0.5*(e_edgeNodes0[0]->Y()+e_edgeNodes0[1]->Y()));                
                  e_edgeNodes0[2]->SetZ(0.5*(e_edgeNodes0[0]->Z()+e_edgeNodes0[1]->Z()));    
                  nod_vector.push_back(e_edgeNodes0[2]);
                  node_index_glb0[nnodes0] = e_edgeNodes0[2]->GetIndex();
                  e_nodes0[nnodes0] = e_edgeNodes0[2];
                  nnodes0++;
              }
             Edges0[i]->SetNodes(e_edgeNodes0); 
             edge_vector.push_back(Edges0[i]);		               
          } // new edges
   	  } //  for(i=0; i<nedges0; i++)
      //
      if(quadratic&&thisElem0->GetElementType()==2) // Quadrilateral
      {
         x0=y0=z0=0.0;
         CNode* newNode = new CNode((long)nod_vector.size());
         e_nodes0[nnodes0] = newNode;
         nnodes0 = thisElem0->nnodes;
         for(i=0; i<nnodes0; i++) // Nodes
         {
            x0 += e_nodes0[i]->X();	
            y0 += e_nodes0[i]->Y();	
            z0 += e_nodes0[i]->Z();	
         }         
         x0 /= (double)nnodes0;
         y0 /= (double)nnodes0;
         z0 /= (double)nnodes0;
         newNode->SetX(x0);
         newNode->SetY(y0);
         newNode->SetZ(z0);
         nod_vector.push_back(newNode);         
      }     
      // Set edges and nodes
      thisElem0->SetOrder(quadratic);
      thisElem0->SetEdgesOrientation(Edge_Orientation); 
      thisElem0->SetEdges(Edges0); 
      // Resize is true
      thisElem0->SetNodes(e_nodes0, true);						
   }// Over elements
   // Set faces on surfaces and others
   msh_no_line=0;  // Should be members of mesh
   msh_no_quad=0;
   msh_no_hexs=0;
   msh_no_tris=0;
   msh_no_tets=0;
   msh_no_pris=0;
   for(e=0; e<e_size; e++)
   {
       thisElem0 = ele_vector[e];   
	   switch(thisElem0->GetElementType())
	   {
	      case 1: msh_no_line++; break;
	      case 2: msh_no_quad++; break;
	      case 3: msh_no_hexs++; break;
	      case 4: msh_no_tris++; break;
	      case 5: msh_no_tets++; break;
	      case 6: msh_no_pris++; break;
	   }
       // Compute volume meanwhile
	   thisElem0->ComputeVolume();
       
	   if(thisElem0->GetElementType()==1) continue; // line element  
       thisElem0->GetNodeIndeces(node_index_glb0);
       thisElem0->GetNeighbors(Neighbors0);
       m0 = thisElem0->GetFacesNumber();

       // Check face on surface
       for(i=0; i<m0; i++) // Faces
       {		  
          if(Neighbors0[i])
             continue;
          CElem* newFace = new CElem((long)face_vector.size(), thisElem0, i);
//          thisElem0->boundary_type='B';
		  thisElem0->no_faces_on_surface++;
          face_vector.push_back(newFace);
          Neighbors0[i] = newFace;        
       }
       thisElem0->SetNeighbors(Neighbors0);		

   }
   NodesNumber_Quadratic= (long)nod_vector.size();
   if((msh_no_hexs+msh_no_tets+msh_no_pris)>0) max_ele_dim=3;
   else if((msh_no_quad+msh_no_tris)>0) max_ele_dim=2;
   else max_ele_dim=1;
   // For high order element
   if(quadratic) 
   {
      for(e=0; e<e_size; e++)
      {
         thisElem0 = ele_vector[e];   
         thisElem0->GetNodeIndeces(node_index_glb0);
         for(i=thisElem0->nnodes; i<thisElem0->nnodesHQ; i++)
         {
            done = false;
            for(j=0; j<(int)nod_vector[node_index_glb0[i]]->connected_elements.size(); j++)
            {
              if(e==nod_vector[node_index_glb0[i]]
                         ->connected_elements[j])
              {
                  done = true;
                  break;
			  }
            }
            if(!done)  
            nod_vector[node_index_glb0[i]]->connected_elements.push_back(e);
        }
     }
   }
   //----------------------------------------------------------------------
   // Node information 
   // 1. Default node index <---> eqs index relationship
   // 2. Coordiate system flag
   x_sum=0.0;
   y_sum=0.0;
   z_sum=0.0;
   Eqs2Global_NodeIndex.clear();
   for(e=0; e<(long)nod_vector.size(); e++)
   {
	   nod_vector[e]->SetEquationIndex(e);
	   Eqs2Global_NodeIndex.push_back(nod_vector[e]->GetIndex());
	   x_sum += fabs(nod_vector[e]->X());
	   y_sum += fabs(nod_vector[e]->Y());
	   z_sum += fabs(nod_vector[e]->Z());
   }
   if(x_sum>0.0&&y_sum<MKleinsteZahl&&z_sum<MKleinsteZahl)
      coordinate_system = 10;
   else if(y_sum>0.0&&x_sum<MKleinsteZahl&&z_sum<MKleinsteZahl)
      coordinate_system = 11;
   else if(z_sum>0.0&&x_sum<MKleinsteZahl&&y_sum<MKleinsteZahl)
      coordinate_system = 12;
   else if(x_sum>0.0&&y_sum>0.0&&z_sum<MKleinsteZahl)
      coordinate_system = 21;
   else if(x_sum>0.0&&z_sum>0.0&&y_sum<MKleinsteZahl)
      coordinate_system = 22;
   else if(x_sum>0.0&&y_sum>0.0&&z_sum>0.0)
      coordinate_system = 32;
   max_dim = coordinate_system/10-1;
   /*
   //----------------------------------------------------------------------
   //Determin msh dimension
   double max_nodes = 0.0;
   double anz_nodes = 0.0;
   for(e=0; e<e_size; e++){
     thisElem0 = ele_vector[e];  
     anz_nodes = thisElem0->GetNodesNumber(false);
     max_nodes = max(max_nodes, anz_nodes);
   }
   if(max_nodes <= 2.0) msh_max_dim = 1;      // 1-D elements
   else if(max_nodes <= 4.0) msh_max_dim = 2; // 2-D elements
   else msh_max_dim = 3;                      // 3-D elements
   */

   //----------------------------------------------------------------------
   // Gravity center
   for(e=0; e<e_size; e++)
   {
     thisElem0 = ele_vector[e];  
     nnodes0 = thisElem0->nnodes;   
     for(i=0; i<nnodes0; i++) // Nodes
     {
       thisElem0->gravity_center[0] += thisElem0->nodes[i]->X();	
       thisElem0->gravity_center[1] += thisElem0->nodes[i]->Y();	
       thisElem0->gravity_center[2] += thisElem0->nodes[i]->Z();	
     }         
     thisElem0->gravity_center[0] /= (double)nnodes0;
     thisElem0->gravity_center[1] /= (double)nnodes0;
     thisElem0->gravity_center[2] /= (double)nnodes0;
   }
   //----------------------------------------------------------------------
   e_nodes0.resize(0);
   node_index_glb.resize(0);
   node_index_glb0.resize(0);
   Edge_Orientation.resize(0);
   Edges.resize(0);
   Edges0.resize(0);
   Neighbors.resize(0);
   Neighbors0.resize(0);
   e_edgeNodes0.resize(0);
   e_edgeNodes.resize(0);
}

/**************************************************************************
FEMLib-Method: 
Task:  Renumbering nodes corresponding to the activiate of elements
Programing:
09/2005 WW Implementation
**************************************************************************/
void CFEMesh::FillTransformMatrix()  
{
   CElem* elem = NULL;
   if(coordinate_system!=32) 
      return;
   for (long i = 0; i < (long)ele_vector.size(); i++)
   {
      elem = ele_vector[i];
      if (elem->GetMark()) // Marked for use
	  {
         if(  elem->GetElementType()==1
            ||elem->GetElementType()==2 			 
            ||elem->GetElementType()==4 )
           elem->FillTransformMatrix();
	  } 
   }
}

/**************************************************************************
FEMLib-Method: 
Task:  Renumbering nodes corresponding to the activiate of elements
Programing:
05/2005 WW Implementation
**************************************************************************/
void CFEMesh::RenumberNodesForGlobalAssembly()
{
  int i;
  long l, el, el_0;  
 
  CElem* elem = NULL;
  Eqs2Global_NodeIndex.clear();
  for (l = 0; l < (long)nod_vector.size(); l++)
	  nod_vector[l]->SetEquationIndex(-1);
  
  el_0=0; 
  // Lower order
  for (l = 0; l < (long)ele_vector.size(); l++)
  {
     elem = ele_vector[l];
     if (elem->GetMark()) // Marked for use
     {          
        for(i=0; i<elem->GetVertexNumber(); i++)
        {
           if(elem->nodes[i]->GetEquationIndex()<0)
           {
	           elem->nodes[i]->SetEquationIndex(el_0);
			   Eqs2Global_NodeIndex.push_back(elem->nodes[i]->GetIndex());
               el_0++;
           }
           else continue;
        }                 
     } 
  }   
  el = el_0;
  if(!getOrder())
  {
     NodesNumber_Linear=el_0;
     NodesNumber_Quadratic=el;
     return;
  }
  // High order
  for (l = 0; l < (long)ele_vector.size(); l++)
  {
     elem = ele_vector[l];
     if (elem->GetMark()) // Marked for use
     {  
        
        for(i=elem->GetVertexNumber(); i<elem->GetNodesNumber(true); i++)
        {
			if(elem->nodes[i]->GetEquationIndex()<0)
			{
	           elem->nodes[i]->SetEquationIndex(el);
			   Eqs2Global_NodeIndex.push_back(elem->nodes[i]->GetIndex());
               el++;                  
			}
			else continue;
		}                 
     } 
  }   
  NodesNumber_Linear=el_0;
  NodesNumber_Quadratic=el;
  //
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
07/2005 WW Write by member methods of geometry objects.
12/2005 OK MAT_TYPE
**************************************************************************/
void CFEMesh::Write(fstream*fem_msh_file)
{
  long i;
  //--------------------------------------------------------------------
  //KEYWORD
  *fem_msh_file  << "#FEM_MSH" << endl;
  //--------------------------------------------------------------------
  // PCS
  *fem_msh_file << " $PCS_TYPE" << endl;
  *fem_msh_file << "  ";
  *fem_msh_file << pcs_name << endl;
  //--------------------------------------------------------------------
  // MAT
  if(geo_name.size()>0)
  {
    *fem_msh_file << " $GEO_NAME" << endl;
    *fem_msh_file << "  ";
    *fem_msh_file << geo_name << endl;
  }
  //--------------------------------------------------------------------
  // NODES
  *fem_msh_file << " $NODES" << endl;
  *fem_msh_file << "  ";
  *fem_msh_file << (long)nod_vector.size() << endl;
  for(i=0;i<(long)nod_vector.size();i++)
	 nod_vector[i]->Write(*fem_msh_file); //WW
  //--------------------------------------------------------------------
  // ELEMENTS
  *fem_msh_file << " $ELEMENTS" << endl;
  *fem_msh_file << "  ";
  *fem_msh_file << (long)ele_vector.size() << endl;
  for(i=0;i<(long)ele_vector.size();i++)
  {
      ele_vector[i]->SetIndex(i); //20.01.06 WW/TK
	  ele_vector[i]->WriteIndex(*fem_msh_file); //WW
  }
  //--------------------------------------------------------------------
  *fem_msh_file << " $LAYER" << endl;
  *fem_msh_file << "  ";
  *fem_msh_file << no_msh_layer << endl;
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method:
Task: Ermittelt den nahliegenden existierenden Knoten
Programing:
03/2005 OK Implementation (based on GetNodeNumberClose by AH)
07/2005 WW Node object is replaced
last modification:
**************************************************************************/
long CFEMesh::GetNODOnPNT(CGLPoint*m_pnt)
{
 double dist,distmin;
 double pnt[3];
 double nod[3];
 long number;
 pnt[0]=m_pnt->x; pnt[1]=m_pnt->y; pnt[2]=m_pnt->z;
 number=-1;
 distmin=1.e+300;
 for(long i=0;i<NodesInUsage();i++) {
   nod[0] = nod_vector[i]->X();
   nod[1] = nod_vector[i]->Y();
   nod[2] = nod_vector[i]->Z();
   dist=EuklVek3dDist(pnt,nod);
   if(dist<distmin) {
     distmin = dist;
     number = i;
   }
 }
 return number;
}



/**************************************************************************
FEMLib-Method: GetNodesOnArc
Task:  To get fe nodes on a Arc defined by 
       start point, center point, end point 
Programing:
01/2005 WW Implementation
**************************************************************************/
//WW. (x1-x0).(x2-x0)
inline double dotProduction(const double *x1,  const double *x2, const double *x0)
{
    return  (x1[0]-x0[0])*(x2[0]-x0[0])
           +(x1[1]-x0[1])*(x2[1]-x0[1])
           +(x1[2]-x0[2])*(x2[2]-x0[2]);
}

//WW
inline double Distance(const double *x1, const double *x2)
{
     return sqrt((x1[0]-x2[0])*(x1[0]-x2[0])
                +(x1[1]-x2[1])*(x1[1]-x2[1])
                +(x1[2]-x2[2])*(x1[2]-x2[2]));

}
/**************************************************************************
FEMLib-Method: GetNodesOnArc
Task:  To get fe nodes on a Arc defined by 
       start point, center point, end point 
Programing:
01/2005 WW Implementation
05/2005 WW Transplant to this object from GEOLIB
**************************************************************************/
void CFEMesh::GetNodesOnArc(CGLPolyline*m_ply, vector<long>&msh_nod_vector)  
{
   long i;

   //Obtain fem node for groupvector
   CGLPoint *CGPa=NULL, *CGPb=NULL, *CGPc=NULL;
   const long nNodes = NodesInUsage();   
   const int SizeCGLPoint = (int)m_ply->point_vector.size();
   double r1, r2,a0,a1,a2;
   const double Tol = 1.0e-5;
   double xa[3],xb[3],xc[3], xn[3];
   msh_nod_vector.clear();
   if(SizeCGLPoint>3)
   {
       cout<<"More than 3 points that an arc needed"<<endl;
       abort();
   }
   else if (SizeCGLPoint==0)
   {
       cout<<"No point are given for this arc"<<endl;
       abort();      
   }
   CGPa = m_ply->point_vector[0];
   CGPb = m_ply->point_vector[2];
   CGPc = m_ply->point_vector[1];
   xa[0] = CGPa->x;
   xa[1] = CGPa->y;
   xa[2] = CGPa->z;
   xb[0] = CGPb->x;
   xb[1] = CGPb->y;
   xb[2] = CGPb->z;
   xc[0] = CGPc->x;
   xc[1] = CGPc->y;
   xc[2] = CGPc->z;

   r1 = Distance(xa,xc);
   r2 = Distance(xb,xc);


   if(fabs(r1-r2)>m_ply->epsilon)
   {
       cout<<"Start point and end point do not have identical "
       <<"distance to the center of the arc"<<endl;
       abort();
   }
   a0 = acos(dotProduction(xa,xb, xc)/(r1*r1));
    
   // Check nodes by comparing distance 
   for(i=0; i<nNodes; i++)
   {        
      xn[0] = nod_vector[i]->X();
      xn[1] = nod_vector[i]->Y();
      xn[2] = nod_vector[i]->Z();
      r2 =  Distance(xn,xc);
      if(fabs(r2-r1)<m_ply->epsilon)
      {
         if(a0<Tol) // Closed arc
             msh_nod_vector.push_back(i);
         else
         {
            a1 = acos(dotProduction(xa,xn, xc)/(r1*r2));
            a2 = acos(dotProduction(xb,xn, xc)/(r1*r2));
            if(fabs(a1+a2-a0)<Tol)  
               msh_nod_vector.push_back(nod_vector[i]->GetIndex());
         }
      }
   }
   m_ply->GetPointOrderByDistance();
}
/**************************************************************************
FEMLib-Method:
Task: Ermittelt den nahliegenden existierenden Knoten
Programing:
03/2005 OK Implementation (based on ExecuteSourceSinkMethod11 by CT)
07/2005 WW Node object is replaced
10/2005 OK test
**************************************************************************/
void CFEMesh::GetNODOnPLY(CGLPolyline*m_ply,vector<long>&msh_nod_vector)
{
  if((int)m_ply->point_vector.size()==0)
    return;
  long j,k,l;
  double pt1[3],line1[3],line2[3];
  double mult_eps = 1.0;
  double dist1p,dist2p,*length,laenge;
  long anz_relevant = 0;
  typedef struct {
    long knoten;
    long abschnitt;
    double laenge;
  } INFO;
  INFO *relevant=NULL;
  int weiter;
  double w1,w2;
  long knoten_help;
  double laenge_help;
  double gesamte_laenge = 0.;
  m_ply->sbuffer.clear();
  m_ply->ibuffer.clear();
  msh_nod_vector.clear();
  //
  length = (double*) Malloc(sizeof(double)*(int)m_ply->point_vector.size());
  /* */
  for (k=0;k<(long)m_ply->point_vector.size()-1;k++) {
    line1[0] = m_ply->point_vector[k]->x;
    line1[1] = m_ply->point_vector[k]->y;
    line1[2] = m_ply->point_vector[k]->z;
    line2[0] = m_ply->point_vector[k+1]->x;
    line2[1] = m_ply->point_vector[k+1]->y;
    line2[2] = m_ply->point_vector[k+1]->z;
    length[k] = MCalcDistancePointToPoint(line2, line1);
    gesamte_laenge += length[k];
  }
  /* Wiederholen bis zumindest ein Knoten gefunden wurde */
  while(anz_relevant==0) {
    /* Schleife ueber alle Knoten */
    for (j=0;j<NodesInUsage();j++) {
      /* Schleife ueber alle Punkte des Polygonzuges */
      for (k=0;k<(long)m_ply->point_vector.size()-1;k++) {
        /* ??? */
        pt1[0] = nod_vector[j]->X();
        pt1[1] = nod_vector[j]->Y();
        pt1[2] = nod_vector[j]->Z();
        line1[0] = m_ply->point_vector[k]->x;
        line1[1] = m_ply->point_vector[k]->y;
        line1[2] = m_ply->point_vector[k]->z;
        line2[0] = m_ply->point_vector[k+1]->x;
        line2[1] = m_ply->point_vector[k+1]->y;
        line2[2] = m_ply->point_vector[k+1]->z;
        /* Ist der Knoten nah am Polygonabschnitt? */
        if ( MCalcDistancePointToLine(pt1,line1,line2) <= mult_eps*m_ply->epsilon ) {
          /* Im folgenden wird mit der Projektion weitergearbeitet */
          MCalcProjectionOfPointOnLine(pt1,line1,line2,pt1);
          /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
          dist1p = MCalcDistancePointToPoint(line1, pt1);
          /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
          dist2p = MCalcDistancePointToPoint(line2, pt1);
          /* Ist der Knoten innerhalb des Intervalls? */
          /* bis rf3807: if ((length[k] - dist1p - dist2p + MKleinsteZahl)/(length[k] + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl){ */
          if ((dist1p+dist2p-length[k]) <=  mult_eps*m_ply->epsilon ) {
            // For boundara conditions. WW
            m_ply->sbuffer.push_back(dist1p);
            m_ply->ibuffer.push_back(k); //Section index
            anz_relevant++;
            /* Feld anpassen */
            //nodes_all = (long *) Realloc(nodes_all,sizeof(long)*anz_relevant);
            relevant = (INFO *) Realloc(relevant, sizeof(INFO) * anz_relevant);
            /* Ablegen von Knotennummer und Position */
            //nodes_all[anz_relevant-1] = j;
			msh_nod_vector.push_back(nod_vector[j]->GetIndex());
            /* Position ermitteln */
            laenge = 0.;
            for (l=0; l < k; l++)
              laenge += length[l];
            /* Ablegen von Knotennummer und Position */
            relevant[anz_relevant-1].knoten = j;
            relevant[anz_relevant-1].laenge = laenge + dist1p;
            /* Suche am Polygon abbrechen, naechster Knoten */
            k =(long)m_ply->point_vector.size();
          }
        } /* endif */
      } /* Ende Schleife ueber Polygonabschnitte */
    } /* Ende Schleife ueber Knoten */
    if(anz_relevant==0) mult_eps *=2.;
  } /* Ende Schleife Wiederholungen */
  if (mult_eps > 1.)
    cout << "!!! Epsilon increased in sources!" << endl;
  /* Schleife ueber alle Knoten; sortieren nach Reihenfolge auf dem Abschnitt (zyklisches Vertauschen, sehr lahm)*/
  do {
    weiter = 0;
    for (k=0;k<anz_relevant-1;k++) {
      w1=relevant[k].laenge;
      w2=relevant[k+1].laenge;
      if (w1>w2) { /* Die Eintraege vertauschen */
        knoten_help = relevant[k].knoten;
        laenge_help = relevant[k].laenge;
        relevant[k].knoten = relevant[k+1].knoten;
        relevant[k].laenge = relevant[k+1].laenge;
        relevant[k+1].knoten = knoten_help;
        relevant[k+1].laenge = laenge_help;
        weiter=1;
      }
    }
  } while (weiter);
  relevant = (INFO*) Free(relevant);

  //WW
  m_ply->GetPointOrderByDistance();
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
04/2005 OK
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  msh_nod_vector.clear();
  //----------------------------------------------------------------------
  switch(m_sfc->type) {
    //....................................................................
    case 0: // Surface polygon
      #ifdef MFC
          //TODO allow only TIN or selected and user-controlled Nodes
          //because it is more reliable
          if(gsp_vector.size() > 0){
          string path = gsp_vector[0]->path; 
          path = path + "temp_gmsh.msh";
          const char* file_name_const_char = 0;
          file_name_const_char = path.data();         
          Clear_Selected_Nodes_Elements();
          Mesh_Single_Surface(m_sfc->name, file_name_const_char);
          GMSH2TIN(file_name_const_char);
          Select_Nodes_Elements_by_TINFile(file_name_const_char);
          CopySelectedNodes(msh_nod_vector);
          Clear_Selected_Nodes_Elements(); 
          }
          //TK
      #else
        GetNODOnSFC_PLY(m_sfc,msh_nod_vector);
      #endif
      break;
    //....................................................................
    case 1: // TIN
      if(!m_sfc->TIN){
#ifdef MFC
        CString m_str;
        m_str = "Error in CFEMesh::GetNODOnSFC - no TIN data for ";
        m_str += m_sfc->name.data();
        AfxMessageBox(m_str);
#endif
        return;
      }
      GetNODOnSFC_TIN(m_sfc,msh_nod_vector);
      break;
    //....................................................................
    case 2: // 2 vertical polylines
      GetNODOnSFC_Vertical(m_sfc,msh_nod_vector);
      break;
    //....................................................................
    case 3: // projection on xy plane (all mesh points above and below the surface) //MB
      GetNODOnSFC_PLY_XY(m_sfc,msh_nod_vector);
      break;
    //....................................................................
	case 100:
      GetNodesOnCylindricalSurface(m_sfc,msh_nod_vector);
	  break;
  }
}

/**************************************************************************
MSHLib-Method: 
Task: Get nodes on plane surface by comparing the area of polygon computed
      by triangles, which are formed by node and the gravity center 
      with edges of polygon, respectively  
Programing:
09/2004 WW Implementation
04/2005 OK MSH
07/2005 WW Node object is replaced
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_PLY(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i,j,k;
  int nPointsPly = 0;
  double gC[3],p1[3],p2[3];
  double Area1, Area2;
  double Tol = m_sfc->epsilon;
  CGLPolyline* m_ply = NULL;
  vector<CGLPolyline*>::iterator p_ply;//CC
  // Init
  msh_nod_vector.clear();
  //----------------------------------------------------------------------
  // nodes close to first polyline 
  p_ply = m_sfc->polyline_of_surface_vector.begin();//CC
  while(p_ply!=m_sfc->polyline_of_surface_vector.end()) {//CC
    m_ply = *p_ply;
    nPointsPly = (int)m_ply->point_vector.size();
    //....................................................................
    // Grativity center of this polygon
    for(i=0; i<3; i++) gC[i] = 0.0;
    for(i=0; i<nPointsPly; i++)
    { 
      gC[0] += m_ply->point_vector[i]->x;
      gC[1] += m_ply->point_vector[i]->y;
      gC[2] += m_ply->point_vector[i]->z;
    } 
    for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;
    //....................................................................
    // Area of this polygon by the grativity center
    Area1 = 0.0;
    for(i=0; i<nPointsPly; i++)
    { 
      p1[0] = m_ply->point_vector[i]->x;
      p1[1] = m_ply->point_vector[i]->y;
      p1[2] = m_ply->point_vector[i]->z;
      if(i<nPointsPly-1)
      {
        p2[0] = m_ply->point_vector[i+1]->x;
        p2[1] = m_ply->point_vector[i+1]->y;
        p2[2] = m_ply->point_vector[i+1]->z;
      }
      else
      {
        p2[0] = m_ply->point_vector[0]->x;
        p2[1] = m_ply->point_vector[0]->y;
        p2[2] = m_ply->point_vector[0]->z;
      }
      Area1 += fabs(ComputeDetTri(p1, gC, p2));
    }
    //....................................................................
    // Check nodes by comparing area 
    for(j=0;j<NodesInUsage();j++)
    {        
      Area2 = 0.0;
      gC[0] = nod_vector[j]->X();
      gC[1] = nod_vector[j]->Y();
      gC[2] = nod_vector[j]->Z();
      for(i=0; i<nPointsPly; i++)
      { 
        p1[0] = m_ply->point_vector[i]->x;
        p1[1] = m_ply->point_vector[i]->y;
        p1[2] = m_ply->point_vector[i]->z;
        k = i+1;
        if(i==nPointsPly-1)
          k = 0;
        p2[0] = m_ply->point_vector[k]->x;
        p2[1] = m_ply->point_vector[k]->y;
        p2[2] = m_ply->point_vector[k]->z;
        Area2 += fabs(ComputeDetTri (p1, gC, p2));
      }
      if(fabs(Area1-Area2)<Tol) 
        msh_nod_vector.push_back(nod_vector[j]->GetIndex());
    } 
    p_ply++;
  }
}

/**************************************************************************
MSHLib-Method: 
Task: Get nodes on plane surface by comparing the area of polygon computed
      by triangles, which are formed by node and the gravity center 
      with edges of polygon, respectively  
Programing:
08/2005 MB based on GetNODOnSFC_PLY
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_PLY_XY(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i,j,k;
  int nPointsPly = 0;
  double gC[3],p1[3],p2[3];
  double Area1, Area2;
  double Tol = m_sfc->epsilon;
  CGLPolyline* m_ply = NULL;
  vector<CGLPolyline*>::iterator p_ply;//CC
  // Init
  msh_nod_vector.clear();
  //----------------------------------------------------------------------
  // nodes close to first polyline 
  p_ply = m_sfc->polyline_of_surface_vector.begin();//CC
  while(p_ply!=m_sfc->polyline_of_surface_vector.end()) {//CC
    m_ply = *p_ply;
    nPointsPly = (int)m_ply->point_vector.size();
    //....................................................................
    // Grativity center of this polygon
    for(i=0; i<3; i++) gC[i] = 0.0;
    for(i=0; i<nPointsPly; i++)
    { 
      gC[0] += m_ply->point_vector[i]->x;
      gC[1] += m_ply->point_vector[i]->y;
      gC[2] += 0.0;
    } 
    for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;
    //....................................................................
    // Area of this polygon by the grativity center
    Area1 = 0.0;
    for(i=0; i<nPointsPly; i++)
    { 
      p1[0] = m_ply->point_vector[i]->x;
      p1[1] = m_ply->point_vector[i]->y;
      p1[2] = 0.0;
      if(i<nPointsPly-1)
      {
        p2[0] = m_ply->point_vector[i+1]->x;
        p2[1] = m_ply->point_vector[i+1]->y;
        p2[2] = 0.0;
      }
      else
      {
        p2[0] = m_ply->point_vector[0]->x;
        p2[1] = m_ply->point_vector[0]->y;
        p2[2] = 0.0;
      }
      Area1 += fabs(ComputeDetTri(p1, gC, p2));
    }
    //....................................................................
    // Check nodes by comparing area 
    for(j=0;j<NodesInUsage();j++)
    {        
      Area2 = 0.0;
      gC[0] = nod_vector[j]->X();
      gC[1] = nod_vector[j]->Y();
      gC[2] = 0.0;
      for(i=0; i<nPointsPly; i++)
      { 
        p1[0] = m_ply->point_vector[i]->x;
        p1[1] = m_ply->point_vector[i]->y;
        p1[2] = 0.0;
        k = i+1;
        if(i==nPointsPly-1)
          k = 0;
        p2[0] = m_ply->point_vector[k]->x;
        p2[1] = m_ply->point_vector[k]->y;
        p2[2] = 0.0;
        Area2 += fabs(ComputeDetTri (p1, gC, p2));
      }
      if(fabs(Area1-Area2)<Tol) 
        msh_nod_vector.push_back(nod_vector[j]->GetIndex());
    } 
    p_ply++;
  }
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
04/2005 OK Implementation based on vector<long>Surface::GetMSHNodesClose()
last modification:
**************************************************************************/
/*
//vector<long>Surface::GetMSHNodesClose()
void CFEMesh::GetNODOnSFC_PLY(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i;
  double x,y,z;
  double *xp=NULL,*yp=NULL,*zp=NULL;
  //----------------------------------------------------------------------
  m_sfc->CreatePolygonPointVector();
  //----------------------------------------------------------------------
  // Polygon point vector
  if(m_sfc->polygon_point_vector.empty())
    return;
  long polygon_point_vector_size = (long)m_sfc->polygon_point_vector.size();
  xp = new double[polygon_point_vector_size];
  yp = new double[polygon_point_vector_size];
  zp = new double[polygon_point_vector_size];
  for(i=0;i<polygon_point_vector_size;i++) {
    xp[i] = m_sfc->polygon_point_vector[i]->x;
    yp[i] = m_sfc->polygon_point_vector[i]->y;
    zp[i] = m_sfc->polygon_point_vector[i]->z;
  }
  // close the polygon ???
  //----------------------------------------------------------------------
  for(i=0;i<(long)nod_vector.size();i++) {
    x = nod_vector[i]->x;
    y = nod_vector[i]->y;
    z = nod_vector[i]->z;
    if(IsPointInsidePolygonPlain(x,y,z,xp,yp,zp,\
                                 (long)m_sfc->polygon_point_vector.size())) {
      msh_nod_vector.push_back(i);
    }
  }
  //----------------------------------------------------------------------
  // Destructions
  delete [] xp;
  delete [] yp;
  delete [] zp;
}
*/

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
04/2005 OK
07/2005 WW Node object is replaced
04/2006 TK new method
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_TIN(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i=0,j=0,k=0,m=0;
  double angle_sum, dist;
  double tolerance = 0.001;
  double min_mesh_dist=0.0;
  double tri_point1[3],tri_point2[3],tri_point3[3],checkpoint[3];
  double sfc_min[3],sfc_max[3];

  CGLPoint m_node;
  CTriangle *m_triangle = NULL;
  //Loop over all generated triangles of surface
  //----------------------------------------------------------------------
/*  
  double tri_x[3],tri_y[3],tri_z[3];
    for(m=0;m<(long)m_sfc->TIN->Triangles.size();m++){
    m_triangle = m_sfc->TIN->Triangles[m];
    tri_x[0] = m_triangle->x[0];
    tri_y[0] = m_triangle->y[0];
    tri_z[0] = m_triangle->z[0];
    tri_x[1] = m_triangle->x[1];
    tri_y[1] = m_triangle->y[1];
    tri_z[1] = m_triangle->z[1];
    tri_x[2] = m_triangle->x[2];
    tri_y[2] = m_triangle->y[2];
    tri_z[2] = m_triangle->z[2];
    
    for(j=0;j<NodesInUsage();j++){
      m_node.x = nod_vector[j]->X();
      m_node.y = nod_vector[j]->Y();
      m_node.z = nod_vector[j]->Z();
      if(m_node.IsInsideTriangle(xp,yp,zp)){//CC 10/05
         msh_nod_vector.push_back(nod_vector[j]->GetIndex());
      }
    }
  }*/

  //TK 03.2006
  // ATTENTION: NO MULTI MESH YET!!!!!!
  // Several possibilities to do this: 
  // Mesh assignment by user input would be flexible or by process name is more static

  //----------------------------------------------------------------------
  // Create Bounding BOX = MIN/MAX of X/Y/Z
  //----------------------------------------------------------------------

  //Loop over all generated triangles of surface
  for(m=0;m<(long)m_sfc->TIN->Triangles.size();m++){
    m_triangle = m_sfc->TIN->Triangles[m];
    tri_point1[0] = m_triangle->x[0];
    tri_point1[1] = m_triangle->y[0];
    tri_point1[2] = m_triangle->z[0];
    tri_point2[0] = m_triangle->x[1];
    tri_point2[1] = m_triangle->y[1];
    tri_point2[2] = m_triangle->z[1];
    tri_point3[0] = m_triangle->x[2];
    tri_point3[1] = m_triangle->y[2];
    tri_point3[2] = m_triangle->z[2];
    if (m==0)
    {
     sfc_min[0]= tri_point1[0];
     sfc_min[1]= tri_point1[1];
     sfc_min[2]= tri_point1[2];
     sfc_max[0]= tri_point1[0];
     sfc_max[1]= tri_point1[1];
     sfc_max[2]= tri_point1[2];
     if (tri_point1[0] < sfc_min[0]) sfc_min[0] = tri_point1[0];
     if (tri_point2[0] < sfc_min[0]) sfc_min[0] = tri_point2[0];
     if (tri_point3[0] < sfc_min[0]) sfc_min[0] = tri_point3[0];
     if (tri_point1[0] > sfc_max[0]) sfc_max[0] = tri_point1[0];
     if (tri_point2[0] > sfc_max[0]) sfc_max[0] = tri_point2[0];
     if (tri_point3[0] > sfc_max[0]) sfc_max[0] = tri_point3[0];
     if (tri_point1[1] < sfc_min[1]) sfc_min[1] = tri_point1[1];
     if (tri_point2[1] < sfc_min[1]) sfc_min[1] = tri_point2[1];
     if (tri_point3[1] < sfc_min[1]) sfc_min[1] = tri_point3[1];
     if (tri_point1[1] > sfc_max[1]) sfc_max[1] = tri_point1[1];
     if (tri_point2[1] > sfc_max[1]) sfc_max[1] = tri_point2[1];
     if (tri_point3[1] > sfc_max[1]) sfc_max[1] = tri_point3[1];
     if (tri_point1[2] < sfc_min[2]) sfc_min[2] = tri_point1[2];
     if (tri_point2[2] < sfc_min[2]) sfc_min[2] = tri_point2[2];
     if (tri_point3[2] < sfc_min[2]) sfc_min[2] = tri_point3[2];
     if (tri_point1[2] > sfc_max[2]) sfc_max[2] = tri_point1[2];
     if (tri_point2[2] > sfc_max[2]) sfc_max[2] = tri_point2[2];
     if (tri_point3[2] > sfc_max[2]) sfc_max[2] = tri_point3[2];
    }
    else
    {
     if (tri_point1[0] < sfc_min[0]) sfc_min[0] = tri_point1[0];
     if (tri_point2[0] < sfc_min[0]) sfc_min[0] = tri_point2[0];
     if (tri_point3[0] < sfc_min[0]) sfc_min[0] = tri_point3[0];
     if (tri_point1[0] > sfc_max[0]) sfc_max[0] = tri_point1[0];
     if (tri_point2[0] > sfc_max[0]) sfc_max[0] = tri_point2[0];
     if (tri_point3[0] > sfc_max[0]) sfc_max[0] = tri_point3[0];
     if (tri_point1[1] < sfc_min[1]) sfc_min[1] = tri_point1[1];
     if (tri_point2[1] < sfc_min[1]) sfc_min[1] = tri_point2[1];
     if (tri_point3[1] < sfc_min[1]) sfc_min[1] = tri_point3[1];
     if (tri_point1[1] > sfc_max[1]) sfc_max[1] = tri_point1[1];
     if (tri_point2[1] > sfc_max[1]) sfc_max[1] = tri_point2[1];
     if (tri_point3[1] > sfc_max[1]) sfc_max[1] = tri_point3[1];
     if (tri_point1[2] < sfc_min[2]) sfc_min[2] = tri_point1[2];
     if (tri_point2[2] < sfc_min[2]) sfc_min[2] = tri_point2[2];
     if (tri_point3[2] < sfc_min[2]) sfc_min[2] = tri_point3[2];
     if (tri_point1[2] > sfc_max[2]) sfc_max[2] = tri_point1[2];
     if (tri_point2[2] > sfc_max[2]) sfc_max[2] = tri_point2[2];
     if (tri_point3[2] > sfc_max[2]) sfc_max[2] = tri_point3[2];
    }
  }
  //----------------------------------------------------------------------
  // Create Local Search Vector 
  // Only nodes inside searching box  
  //----------------------------------------------------------------------
  
  CFEMesh* m_msh_aux = NULL; 
  m_msh_aux = new CFEMesh();
  CNode* node = NULL;

   //Loop over all edges
        for(i=0;i<(long)edge_vector.size();i++)
        {
            if (j==0 && i==0){
              min_mesh_dist = edge_vector[i]->Length();
            }
            else{
              if (min_mesh_dist  > edge_vector[i]->Length())
                  min_mesh_dist =  edge_vector[i]->Length();
            }
        }
        tolerance = min_mesh_dist;
    //Loop over all mesh nodes
        for(i=0;i<(long)nod_vector.size();i++)
        {
            checkpoint[0] = nod_vector[i]->X();
            checkpoint[1] = nod_vector[i]->Y(); 
            checkpoint[2] = nod_vector[i]->Z();
            node = new CNode(i,checkpoint[0],checkpoint[1],checkpoint[2]);
            if((checkpoint[0]>=sfc_min[0] && checkpoint[0]<=sfc_max[0] )&&
               (checkpoint[1]>=sfc_min[1] && checkpoint[1]<=sfc_max[1] )&&
               (checkpoint[2]>=sfc_min[2] && checkpoint[2]<=sfc_max[2] ) )
            {              
                m_msh_aux->nod_vector.push_back(node);
            }
        }
  
  //----------------------------------------------------------------------
  // Search preselected Nodes within TIN Triangles
  //----------------------------------------------------------------------
  for(m=0;m<(long)m_sfc->TIN->Triangles.size();m++){
    m_triangle = m_sfc->TIN->Triangles[m];
    tri_point1[0] = m_triangle->x[0];
    tri_point1[1] = m_triangle->y[0];
    tri_point1[2] = m_triangle->z[0];
    tri_point2[0] = m_triangle->x[1];
    tri_point2[1] = m_triangle->y[1];
    tri_point2[2] = m_triangle->z[1];
    tri_point3[0] = m_triangle->x[2];
    tri_point3[1] = m_triangle->y[2];
    tri_point3[2] = m_triangle->z[2];
    //Loop over all preselected mesh nodes
        for(i=0;i<(long)m_msh_aux->nod_vector.size();i++)
        {
            checkpoint[0] = m_msh_aux->nod_vector[i]->X();
            checkpoint[1] = m_msh_aux->nod_vector[i]->Y(); 
            checkpoint[2] = m_msh_aux->nod_vector[i]->Z();
            dist = MCalcDistancePointToPlane(checkpoint,tri_point1,tri_point2,tri_point3);
            if (k==0) m_msh_aux->nod_vector[i]->epsilon = dist;
            else
            {
                if (m_msh_aux->nod_vector[i]->epsilon > dist)
                    m_msh_aux->nod_vector[i]->epsilon = dist;
            }
                if (dist<=tolerance && dist>=-tolerance)
                {
                  angle_sum = AngleSumPointInsideTriangle(checkpoint,tri_point1,tri_point2,tri_point3, min_mesh_dist);
                  if(angle_sum>359)
                  m_msh_aux->nod_vector[i]->selected = 1;
                }
        }
  }

  //----------------------------------------------------------------------
  // Identify the preselected nodes of the search vector and copy to msh_nod_vector
  // TODO: Works only for one mesh!!!
  //----------------------------------------------------------------------
  int index;
    //Loop over selected nodes
        for(i=0;i<(long)m_msh_aux->nod_vector.size();i++)
        {
            index = m_msh_aux->nod_vector[i]->GetIndex();
            if(index < (int)nod_vector.size())
            {
            if ((m_msh_aux->nod_vector[i]->GetIndex() == nod_vector[index]->GetIndex()) 
                &&
                m_msh_aux->nod_vector[i]->selected==1
                &&
                (m_msh_aux->nod_vector[i]->X() == nod_vector[index]->X()) 
                && 
                (m_msh_aux->nod_vector[i]->Y() == nod_vector[index]->Y()) 
                &&
                (m_msh_aux->nod_vector[i]->Z() == nod_vector[index]->Z())) 
            {
                 msh_nod_vector.push_back(nod_vector[index]->GetIndex());
            }
            }
        }
  //----------------------------------------------------------------------
  // Delete Search Vector at the end of fem_msh_vector
  // TODO: Proper delete by MSHDelete!!!
  //----------------------------------------------------------------------
        for(i=0;i<(long)m_msh_aux->nod_vector.size();i++)
        {
          delete m_msh_aux->nod_vector[i];
        }
}

/**************************************************************************
GeoLib-Method: 
Task: Get nodes on cylindrical surface by comparing the area of 
      triangles form by nodes and two axis points
Programing:
10/2004 WW Implementation
05/2005 WW Transplant to this object from GEOLIB
04/2006 WW Case of quadratic elements
last modification:
**************************************************************************/
void CFEMesh::GetNodesOnCylindricalSurface(Surface*m_sfc, vector<long>& NodesS)
{
  int k, l, nf;
  long i, j, m, fnode;
  const int nNodes = NodesInUsage(); 
  int faceIndex_loc[10];
  double gC[3],p1[3],p2[3];
  double dist, R, dc1, dc2;
  CElem* elem = NULL;
  CNode* cnode = NULL; 
  NodesS.clear();
  m_sfc->epsilon = 1.0e-6;
  p1[0] = m_sfc->polygon_point_vector[0]->x;
  p1[1] = m_sfc->polygon_point_vector[0]->y;
  p1[2] = m_sfc->polygon_point_vector[0]->z;

  p2[0] = m_sfc->polygon_point_vector[1]->x;
  p2[1] = m_sfc->polygon_point_vector[1]->y;
  p2[2] = m_sfc->polygon_point_vector[1]->z;

  dist = sqrt( (p1[0]-p2[0])*(p1[0]-p2[0])
                    +(p1[1]-p2[1])*(p1[1]-p2[1])
                    +(p1[2]-p2[2])*(p1[2]-p2[2]));

  // Check nodes by comparing area 
  for(j=0; j<nNodes; j++)
  {        
     cnode = nod_vector[j];
     gC[0] = cnode->X();
     gC[1] = cnode->Y();
     gC[2] = cnode->Z();

     dc1 =  (p2[0]-p1[0])*(gC[0]-p1[0])
           +(p2[1]-p1[1])*(gC[1]-p1[1])
           +(p2[2]-p1[2])*(gC[2]-p1[2]); 
     dc2 =  (p2[0]-p1[0])*(gC[0]-p2[0])
           +(p2[1]-p1[1])*(gC[1]-p2[1])
           +(p2[2]-p1[2])*(gC[2]-p2[2]); 
     if(dc1<0.0) continue;
     if(dc2>0.0) continue;

     R = 2.0*fabs(ComputeDetTri (p1, gC, p2))/dist;

     if(fabs(R-m_sfc->Radius)<m_sfc->epsilon) 
         NodesS.push_back(cnode->GetIndex());
  } 
  bool done = false;
  int counter = 0;
  int hs;
  long NodesS_size = (long)NodesS.size();
  //
  if(useQuadratic)
  {
     // Face elements are only in quadrilaterals or triangles
     for(i=0; i<NodesS_size; i++)
     {        
         cnode = nod_vector[NodesS[i]];
         for(j=0; j<(long)cnode->connected_elements.size(); j++)
		 {
            elem = ele_vector[cnode->connected_elements[j]];
            for(k=0; k<elem->GetFacesNumber(); k++) 
			{
                nf = elem->GetElementFaceNodes(k, faceIndex_loc);
                counter = 0;
                hs = (int)(nf/2); 
                for(l=0; l<hs; l++) // loop over face vertices
                {
                   fnode = elem->GetNodeIndex(faceIndex_loc[l]);
                   for(m=0; m<NodesS_size; m++)
                   {
                      if(fnode==NodesS[m])
						  counter++;
				   }
                }
                //
				if(counter==hs) // face is found on surface
                {
                   for(l=hs; l<nf; l++) // loop over face vertices
                   {
                      fnode = elem->GetNodeIndex(faceIndex_loc[l]);
                      done = false;
                      for(m=0; m<(long)NodesS.size(); m++)
                      {
                         if(fnode==NodesS[m])
						 {
                             done=true;
                             break;
						 }
				      }
                      if(!done)
                          NodesS.push_back(fnode); 
                   }
                }                                        
			}
		 }
	 }                  
  }
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
04/2005 OK
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_Vertical(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i,j;
  long *nodes_array = NULL;
  double xp[3],yp[3],zp[3];
  CGLPoint m_node;
  CGLPolyline* m_polyline = NULL;
  CGLPolyline* m_polyline1 = NULL;
  CGLPolyline* m_polyline2 = NULL;
  vector<CGLPolyline*>::iterator p_ply;
  long no_nodes = 0;
  // .................................................................
  // nodes close to first polyline 
  p_ply = m_sfc->polyline_of_surface_vector.begin();
  while(p_ply!=m_sfc->polyline_of_surface_vector.end()) {
    m_polyline = *p_ply;
    //OK41 nodes_array = m_polyline->MSHGetNodesCloseXY(&no_nodes);
    nodes_array = MSHGetNodesClose(&no_nodes,m_polyline);//CC 10/05
    break;
  }
  // .....................................................................
  // using triangles
  p_ply = m_sfc->polyline_of_surface_vector.begin();
  while(p_ply!=m_sfc->polyline_of_surface_vector.end()) {
    m_polyline1 = *p_ply; 
    ++p_ply;
    m_polyline2 = *p_ply;
    break;
  }
  long no_points = (long)m_polyline1->point_vector.size();
  for(j=0;j<no_nodes;j++) {
    //OK m_node.x = GetNodeX(nodes_array[j]);
    //OK m_node.y = GetNodeY(nodes_array[j]);
    //OK m_node.z = GetNodeZ(nodes_array[j]);
    for(i=0;i<no_points-1;i++) {
      // first triangle of quad
      xp[0] = m_polyline1->point_vector[i]->x; 
      yp[0] = m_polyline1->point_vector[i]->y; 
      zp[0] = m_polyline1->point_vector[i]->z; 
      xp[1] = m_polyline1->point_vector[i+1]->x; 
      yp[1] = m_polyline1->point_vector[i+1]->y; 
      zp[1] = m_polyline1->point_vector[i+1]->z; 
      xp[2] = m_polyline2->point_vector[i]->x; 
      yp[2] = m_polyline2->point_vector[i]->y; 
      zp[2] = m_polyline2->point_vector[i]->z; 
      if(m_node.IsInsideTriangle(xp,yp,zp)) {//CC 10/05
        msh_nod_vector.push_back(nodes_array[j]);
      }
      // second triangle of quad
      xp[0] = m_polyline2->point_vector[i]->x; 
      yp[0] = m_polyline2->point_vector[i]->y; 
      zp[0] = m_polyline2->point_vector[i]->z; 
      xp[1] = m_polyline2->point_vector[i+1]->x; 
      yp[1] = m_polyline2->point_vector[i+1]->y; 
      zp[1] = m_polyline2->point_vector[i+1]->z; 
      xp[2] = m_polyline1->point_vector[i+1]->x; 
      yp[2] = m_polyline1->point_vector[i+1]->y; 
      zp[2] = m_polyline1->point_vector[i+1]->z; 
      if(m_node.IsInsideTriangle(xp,yp,zp)) {//CC 10/05
        msh_nod_vector.push_back(nodes_array[j]);
      }
    } // no_points
  } // no_nodes
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
05/2005 OK
last modification:
**************************************************************************/
void CFEMesh::SetNOD2ELETopology()
{
  int k;
  long j;
  double xr[4],yr[4],zr[4];
  CGLPoint m_pnt;
  CGLPoint m_pnt1,m_pnt2,m_pnt3,m_pnt4;
//  FiniteElement::CElement* m_ele = NULL;
//  CFEMesh* m_msh = NULL;
  //----------------------------------------------------------------------
/*
  m_msh = FEMGet("OVERLAND_FLOW");
  for(long i=0;i<(long)nod_vector.size();i++){
    m_pnt.x = nod_vector[i]->x;
    m_pnt.y = nod_vector[i]->y;
    m_pnt.z = nod_vector[i]->z;
    for(j=0;j<(long)m_msh->ele_vector.size();j++){
      m_ele = m_msh->ele_vector[j];
      if(m_pnt.PointInRectangle(xr,yr,zr))
        nod_vector[i]->ele_vector.push_back(m_ele);
    }
  }
*/
  Element* element = NULL;
  for(long i=0;i<(long)nod_vector.size();i++){
    m_pnt.x = nod_vector[i]->X();
    m_pnt.y = nod_vector[i]->Y();
    m_pnt.z = nod_vector[i]->Z();
    for(j=0;j<ElListSize();j++){
      element = ElGetElement(j);
      for(k=0;k<4;k++){
        xr[k] = GetNodeX(element->elementknoten[k]);
        yr[k] = GetNodeY(element->elementknoten[k]);
        zr[k] = GetNodeZ(element->elementknoten[k]);
      }
      if(m_pnt.IsInsideRectangle(xr,yr,zr)){//CC 10/05
//WW ?????????        nod_vector[i]->ele_vector.push_back(element);
        //cout << i << " " << element->element_start_number << endl;
      }
    }
  }
  //----------------------------------------------------------------------
/*
  m_msh = FEMGet("GROUNDWATER_FLOW");
  for(long i=0;i<(long)nod_vector.size();i++){
    m_pnt.x = nod_vector[i]->x;
    m_pnt.y = nod_vector[i]->y;
    m_pnt.z = nod_vector[i]->z;
    for(j=0;j<(long)m_msh->ele_vector.size();j++){
      m_ele = m_msh->ele_vector[j];
      for(k=0;k<4;k++){
        xr[k] = m_msh->nod_vector[m_ele->nodes_index[k]]->x;
        yr[k] = m_msh->nod_vector[m_ele->nodes_index[k]]->y;
        zr[k] = m_msh->nod_vector[m_ele->nodes_index[k]]->z;
      }
      if(m_pnt.PointInRectangle(xr,yr,zr)){
        nod_vector[i]->ele_vector_new.push_back(m_ele);
      }
    }
  }
*/
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
05/2005 OK Implementation: hex elements to line nodes
last modification:
**************************************************************************/
void CFEMesh::SetELE2NODTopology()
{
/* //WW TODO
  int k;
  long j;
  double xr[4],yr[4],zr[4];
  CGLPoint m_pnt;
  CGLPoint m_pnt1,m_pnt2,m_pnt3,m_pnt4;
  FiniteElement::CElement* m_ele = NULL;
  CFEMesh* m_msh_cond = NULL;
  CMSHNodes* m_mod = NULL;
  //----------------------------------------------------------------------
  m_msh_cond = FEMGet("RICHARDS_FLOW");
  for(long i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    for(k=0;k<4;k++){
      xr[k] = nod_vector[m_ele->nodes_index[k]]->x;
      yr[k] = nod_vector[m_ele->nodes_index[k]]->y;
      zr[k] = nod_vector[m_ele->nodes_index[k]]->z;
    }
    for(j=0;j<(long)m_msh_cond->nod_vector.size();j++){
      m_mod = m_msh_cond->nod_vector[j];
      m_mod->nodenumber = j;
      m_pnt.x = m_msh_cond->nod_vector[j]->x;
      m_pnt.y = m_msh_cond->nod_vector[j]->y;
      m_pnt.z = m_msh_cond->nod_vector[j]->z;
      if(m_pnt.PointInRectangle(xr,yr,zr)){
        ele_vector[i]->nod_vector.push_back(m_mod);
//cout << i << " " << j << endl;
      }
    }
  }
  */
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
ios::pos_type CFEMesh::GMSReadTIN(ifstream *tin_file)
{
  string line_string;
  std::stringstream in;
  ios::pos_type position;
  int i, ibuf;
  double d_buf;
  i=0; ibuf=0; d_buf=0.0;
  string line;
  string sub_line;
  long no_vertexes = 0;
  CNode* m_nod = NULL;
  long no_triangles = 0;
  CElem* m_ele = NULL;
#ifdef MFC
  char a = '\"';
  CString m_strSubLine;
#endif
  double xyz[3];
  //========================================================================
  while(sub_line.compare("ENDT")){
    in.str(GetLineFromFile1(tin_file));
    in >> sub_line;
    //................................................................
    if(sub_line.find("TNAM")!=string::npos){ // TNAM "PriTIN_1gr"
      in >> sub_line;
#ifdef MFC
      m_strSubLine = sub_line.c_str();
      m_strSubLine.Replace(a,'_');
      pcs_name = (string)m_strSubLine;
      CString m_strInfo = "Read GMS TIN data: ";
      CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
      m_strInfo = "Read GMS-TIN data: ";
      m_strInfo += pcs_name.c_str();
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
    }
    //................................................................
    if(sub_line.find("VERT")!=string::npos){ // VERT 3173
      in >> no_vertexes;
      in.clear();
      for(i=0;i<no_vertexes;i++){
        m_nod = new CNode(i);
        in.str(GetLineFromFile1(tin_file)); // 4501500.2044314	5685936.4582194	0.0	1
        in >> xyz[0] >> xyz[1] >> xyz[2];
        m_nod->SetCoordinates(xyz);
        nod_vector.push_back(m_nod);
        in.clear();
      }
    }
    //................................................................
    if(sub_line.find("TRI")!=string::npos){ // TRI 6117
      // Evaluate ele_type
      in >> no_triangles;
      in.clear();
/* OKWW
      for(i=0;i<no_triangles;i++){
        m_ele = new FiniteElement::CElement();
        m_ele->type_name = "tri";
        m_ele->ElementType = ele_type;
        m_ele->nnodes = 3;
        m_ele->nodes = new long[3];
        in.str(GetLineFromFile1(tin_file)); // 3169	3168	3173
        in >> ele_nod_number;
        m_ele->nodes_index[0] = ele_nod_number-1;
        in >> ele_nod_number;
        m_ele->nodes_index[1] = ele_nod_number-1;
        in >> ele_nod_number;
        m_ele->nodes_index[2] = ele_nod_number-1;
        in.clear();
        m_ele->nodes_xyz = new double[9];
        for(k=0;k<m_ele->nnodes;k++){
          m_ele->nodes_xyz[k]                 = nod_vector[m_ele->nodes_index[k]]->x;
          m_ele->nodes_xyz[k+m_ele->nnodes]   = nod_vector[m_ele->nodes_index[k]]->y;
          m_ele->nodes_xyz[k+2*m_ele->nnodes] = nod_vector[m_ele->nodes_index[k]]->z;
        }
        ele_vector.push_back(m_ele);
      }
*/
      for(i=0;i<no_triangles;i++){
        m_ele = new CElem(i);
        m_ele->geo_type = 4;
        m_ele->Read(*tin_file, 3);
        ele_vector.push_back(m_ele);
      }
    }
    //................................................................
    in.clear();
  }
  return position;
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
02/2005 OK Implementation (MMP groups)
02/2005 OK Activate from vector
08/2005 WW Changes due to geometry objects applied
see also: BuildActiveElementsArray
**************************************************************************/
void CFEMesh::SetActiveElements(vector<long>&elements_active)
{
  long i;
  //-----------------------------------------------------------------------
  for(i=0;i<(long)this->ele_vector.size();i++){
	  ele_vector[i]->MarkingAll(false);
  }
  //-----------------------------------------------------------------------
  for(i=0;i<(long)elements_active.size();i++){
    ele_vector[elements_active[i]]->MarkingAll(true);
  }
  //-----------------------------------------------------------------------
  // Inactivate element with -1 MMP group
}

/**************************************************************************
MSHLib-Method: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
ios::pos_type CFEMesh::ReadBIN(ifstream *fem_file)
{
  int j;
  long i,ii;
  long no_elements;
  long no_nodes;
  ios::pos_type position;
  double x,y,z;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  //======================================================================
  char binary_char[10];
  fem_file->read((char*)(&binary_char),sizeof(binary_char));
  char pcs_type_test;
  position = fem_file->tellg();
  fem_file->read((char*)(&pcs_type_test),sizeof(char));
  fem_file->seekg(position,ios::beg);
  switch(pcs_type_test){
    case 'G':  // GROUNDWATER_FLOW
      char pcs_char_G[17];
      fem_file->read((char*)(&pcs_char_G),sizeof(char[17]));
      pcs_name = pcs_char_G;
      break;
    case 'L': // LIQUID_FLOW
      char pcs_char_L[12];
      fem_file->read((char*)(&pcs_char_L),sizeof(char[12]));
      pcs_name = pcs_char_L;
      break;
    case 'R': // RIVER_FLOW
      char pcs_char_R[11];
      fem_file->read((char*)(&pcs_char_R),sizeof(char[11]));
      pcs_name = pcs_char_R;
      break;
    case 'O': // OVERLAND_FLOW
      char pcs_char_O[14];
      fem_file->read((char*)(&pcs_char_O),sizeof(char[14]));
      pcs_name = pcs_char_O;
      break;
    case 'A': // AIR_FLOW
      char pcs_char_A[9];
      fem_file->read((char*)(&pcs_char_A),sizeof(char[9]));
      pcs_name = pcs_char_A;
      break;
    case 'T': // TWO_PHASE_FLOW
      char pcs_char_T[15];
      fem_file->read((char*)(&pcs_char_T),sizeof(char[15]));
      pcs_name = pcs_char_T;
      break;
    case 'C': // COMPONENTAL_FLOW
      char pcs_char_C[17];
      fem_file->read((char*)(&pcs_char_C),sizeof(char[17]));
      pcs_name = pcs_char_C;
      break;
    case 'H': // HEAT_TRANSPORT
      char pcs_char_H[15];
      fem_file->read((char*)(&pcs_char_H),sizeof(char[15]));
      pcs_name = pcs_char_H;
      break;
    case 'D': // DEFORMATION
      char pcs_char_D[12];
      fem_file->read((char*)(&pcs_char_D),sizeof(char[12]));
      pcs_name = pcs_char_D;
      break;
    case 'M': // MASS_TRANSPORT
      char pcs_char_M[15];
      fem_file->read((char*)(&pcs_char_M),sizeof(char[15]));
      pcs_name = pcs_char_M;
      break;
    case 'F': // FLUID_MOMENTUM
      char pcs_char_F[15];
      fem_file->read((char*)(&pcs_char_F),sizeof(char[15]));
      pcs_name = pcs_char_F;
      break;
  }
  //--------------------------------------------------------------------
  char nodes_char[7];
  fem_file->read((char*)(&nodes_char),sizeof(nodes_char));
  fem_file->read((char*)(&no_nodes),sizeof(long));
  for(i=0;i<no_nodes;i++){
    fem_file->read((char*)(&ii),sizeof(long));
    fem_file->read((char*)(&x),sizeof(double));
    fem_file->read((char*)(&y),sizeof(double));
    fem_file->read((char*)(&z),sizeof(double));
    m_nod = new CNode(ii,x,y,z);
    nod_vector.push_back(m_nod);
  }
  //--------------------------------------------------------------------
  char elements_char[10];
  fem_file->read((char*)(&elements_char),sizeof(elements_char));
  fem_file->read((char*)(&no_elements),sizeof(long));
  char ele_type_test;
  string ele_type_string;
  for(i=0;i<no_elements;i++){
    m_ele = new CElem(i);
    fem_file->read((char*)(&ii),sizeof(long));
    fem_file->read((char*)(&m_ele->patch_index),sizeof(int));
    fem_file->read((char*)(&ii),sizeof(long));
    position = fem_file->tellg();
    fem_file->read((char*)(&ele_type_test),sizeof(ele_type_test));
    if((ele_type_test=='t')||(ele_type_test=='h')){
      char ele_type_char[4];
      fem_file->seekg(position,ios::beg);
      fem_file->read((char*)(&ele_type_char),sizeof(char[4]));
      ele_type_string = ele_type_char;
    }
    else{
      char ele_type_char[5];
      fem_file->seekg(position,ios::beg);
      fem_file->read((char*)(&ele_type_char),sizeof(ele_type_char));
      ele_type_string = ele_type_char;
    }
    //..................................................................
    if(ele_type_string.compare("line")==0){
      m_ele->geo_type = 1;
      m_ele->nnodes = 2;
      msh_no_line++;
    }
    //..................................................................
    if(ele_type_string.compare("quad")==0){
      m_ele->geo_type = 2;
      m_ele->nnodes = 4;
      msh_no_quad++;
    }
    //..................................................................
    if(ele_type_string.compare("hex")==0){
      m_ele->geo_type = 3;
      m_ele->nnodes = 8;
      msh_no_hexs++;
    }
    //..................................................................
    if(ele_type_string.compare("tri")==0){
      m_ele->geo_type = 4;
      m_ele->nnodes = 3;
      msh_no_tris++;
    }
    //..................................................................
    if(ele_type_string.compare("tet")==0){
      m_ele->geo_type = 5;
      m_ele->nnodes = 4;
      msh_no_tets++;
    }
    //..................................................................
    if(ele_type_string.compare("pris")==0){
      m_ele->geo_type = 6;
      m_ele->nnodes = 6;
      msh_no_pris++;
    }
    m_ele->nodes_index.resize(m_ele->nnodes);
    for(j=0;j<m_ele->nnodes;j++)
      fem_file->read((char*)(&m_ele->nodes_index[j]),sizeof(long));
    ele_vector.push_back(m_ele);
  }
  //--------------------------------------------------------------------
  char layer_char[7];
  fem_file->read((char*)(&layer_char),sizeof(layer_char));
  fem_file->read((char*)(&no_msh_layer),sizeof(int));
  //--------------------------------------------------------------------
  position = fem_file->tellg();
  return position;
}

/**************************************************************************
MSHLib-Method: 
Task:
Programing:
10/2005 OK Implementation
**************************************************************************/
void CFEMesh::WriteBIN(fstream*fem_msh_file,fstream*msh_file_test)
{
  int j;
  long i;
  string binary_string;
  long binary_long;
  double binary_double;
//  fstream msh_file_test("test_binary.msh",ios::trunc|ios::out);
  //--------------------------------------------------------------------
  //KEYWORD
  char binary_char_9[9] = "#FEM_MSH";
  fem_msh_file->write((char*)(&binary_char_9),sizeof(char[9]));
  *msh_file_test << binary_char_9 << endl;
  //--------------------------------------------------------------------
  // TYPE
  char binary_char_10[10] = "$PCS_TYPE";
  fem_msh_file->write((char*)(&binary_char_10),sizeof(char[10]));
  *msh_file_test << binary_char_10 << endl;
  switch(pcs_name[0]){
    case 'G':  // GROUNDWATER_FLOW
      char dyn_char_G[17];
      strcpy(dyn_char_G,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_G),sizeof(char[17]));
      *msh_file_test << dyn_char_G << endl;
      break;
    case 'L': // LIQUID_FLOW
      char dyn_char_L[12];
      strcpy(dyn_char_L,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_L),sizeof(char[12]));
      *msh_file_test << dyn_char_L << endl;
      break;
    case 'R': // RIVER_FLOW
      char dyn_char_R[11];
      strcpy(dyn_char_R,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_R),sizeof(char[11]));
      *msh_file_test << dyn_char_R << endl;
      break;
    case 'O': // OVERLAND_FLOW
      char dyn_char_O[14];
      strcpy(dyn_char_O,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_O),sizeof(char[14]));
      *msh_file_test << dyn_char_O << endl;
      break;
    case 'A': // AIR_FLOW
      char dyn_char_A[9];
      strcpy(dyn_char_A,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_A),sizeof(char[9]));
      *msh_file_test << dyn_char_A << endl;
      break;
    case 'T': // TWO_PHASE_FLOW
      char dyn_char_T[15];
      strcpy(dyn_char_T,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_T),sizeof(char[15]));
      *msh_file_test << dyn_char_T << endl;
      break;
    case 'C': // COMPONENTAL_FLOW
      char dyn_char_C[17];
      strcpy(dyn_char_C,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_C),sizeof(char[17]));
      *msh_file_test << dyn_char_C << endl;
      break;
    case 'H': // HEAT_TRANSPORT
      char dyn_char_H[15];
      strcpy(dyn_char_H,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_H),sizeof(char[15]));
      *msh_file_test << dyn_char_H << endl;
      break;
    case 'D': // DEFORMATION
      char dyn_char_D[12];
      strcpy(dyn_char_D,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_D),sizeof(char[12]));
      *msh_file_test << dyn_char_D << endl;
      break;
    case 'M': // MASS_TRANSPORT
      char dyn_char_M[15];
      strcpy(dyn_char_M,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_M),sizeof(char[15]));
      *msh_file_test << dyn_char_M << endl;
      break;
    case 'F': // FLUID_MOMENTUM
      char dyn_char_F[15];
      strcpy(dyn_char_F,pcs_name.data());
      fem_msh_file->write((char*)(&dyn_char_F),sizeof(char[15]));
      *msh_file_test << dyn_char_F << endl;
      break;
  }
  //delete [] dyn_char;
  //--------------------------------------------------------------------
  // NODES
  char binary_char_7[7] = "$NODES";
  fem_msh_file->write((char*)(&binary_char_7),sizeof(char[7]));
  *msh_file_test << binary_char_7 << endl;
  binary_long = (long)nod_vector.size();
  fem_msh_file->write((char*)(&binary_long),sizeof(binary_long));
  *msh_file_test << binary_long << endl;
  CNode* m_nod = NULL;
  for(i=0;i<(long)nod_vector.size();i++){
    m_nod = nod_vector[i];
    fem_msh_file->write((char*)(&i),sizeof(long));
    *msh_file_test << i << " ";
    binary_double = m_nod->X();
    fem_msh_file->write((char*)(&binary_double),sizeof(double));
    *msh_file_test << binary_double << " ";
    binary_double = m_nod->Y();
    fem_msh_file->write((char*)(&binary_double),sizeof(double));
    *msh_file_test << binary_double << " ";
    binary_double = m_nod->Z();
    fem_msh_file->write((char*)(&binary_double),sizeof(double));
    *msh_file_test << binary_double << '\n';
  }
  //--------------------------------------------------------------------
  // ELEMENTS
  char binary_char[10] = "$ELEMENTS";
  fem_msh_file->write((char*)(&binary_char),sizeof(binary_char));
  *msh_file_test << binary_char << '\n';
  binary_long = (long)ele_vector.size();
  fem_msh_file->write((char*)(&binary_long),sizeof(binary_long));
  *msh_file_test << binary_long << '\n';
  CElem* m_ele = NULL;
  for(i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    fem_msh_file->write((char*)(&i),sizeof(long));
    *msh_file_test << i << " ";
    fem_msh_file->write((char*)(&m_ele->patch_index),sizeof(int));
    *msh_file_test << m_ele->patch_index << " ";
    binary_long = -1;
    fem_msh_file->write((char*)(&binary_long),sizeof(binary_long));
    *msh_file_test << binary_long << " ";
    switch(m_ele->geo_type){
      case 1:
        fem_msh_file->write((char*)("line"),sizeof(char[5]));
        *msh_file_test << "line" << " ";
        break;
      case 2:
        fem_msh_file->write((char*)("quad"),sizeof(char[5]));
        *msh_file_test << "quad" << " ";
        break;
      case 3:
        fem_msh_file->write((char*)("hex"),sizeof(char[4]));
        *msh_file_test << "hex" << " ";
        break;
      case 4:
        fem_msh_file->write((char*)("tri"),sizeof(char[4]));
        *msh_file_test << "tri" << " ";
        break;
      case 5:
        fem_msh_file->write((char*)("tet"),sizeof(char[4]));
        *msh_file_test << "tet" << " ";
        break;
      case 6:
        fem_msh_file->write((char*)("pris"),sizeof(char[5]));
        *msh_file_test << "pris" << " ";
        break;
    }
    for(j=0;j<m_ele->nnodes;j++){
      fem_msh_file->write((char*)(&m_ele->nodes_index[j]),sizeof(long));
      *msh_file_test << m_ele->nodes_index[j] << " ";
    }
    *msh_file_test << '\n';
  }
  //--------------------------------------------------------------------
  char layer_char[7] = "$LAYER";
  fem_msh_file->write((char*)(&layer_char),sizeof(layer_char));
  *msh_file_test << layer_char  << '\n';
  fem_msh_file->write((char*)(&no_msh_layer),sizeof(int));
  *msh_file_test << no_msh_layer  << '\n';
  //--------------------------------------------------------------------
  *msh_file_test << "#STOP"  << '\n';
}


/**************************************************************************
FEMLib-Method:
Task: Ermittelt den nahliegenden existierenden Knoten
Programing:
03/2005 OK Implementation (based on ExecuteSourceSinkMethod11 by CT)
last modification:
**************************************************************************/
void CFEMesh::GetNODOnPLY_XY(CGLPolyline*m_ply,vector<long>&msh_nod_vector)
{
  long j,k,l;
  double pt1[3],line1[3],line2[3],pt0[3];
  double mult_eps = 1.0;
  double dist1p,dist2p,*length,laenge;
  long anz_relevant = 0;
  typedef struct {
    long knoten;
    long abschnitt;
    double laenge;
  } INFO;
  INFO *relevant=NULL;
  int weiter;
  double w1,w2;
  long knoten_help;
  double laenge_help;
  double gesamte_laenge = 0.;
  m_ply->sbuffer.clear();
  m_ply->ibuffer.clear();
  msh_nod_vector.clear();
  //
  length = (double*) Malloc(sizeof(double)*(long)m_ply->point_vector.size());
  pt0[0] = m_ply->point_vector[0]->x;
  pt0[1] = m_ply->point_vector[0]->y;
  pt0[2] = 0.0;
  /* */
  for (k=0;k<(long)m_ply->point_vector.size()-1;k++) {
    line1[0] = m_ply->point_vector[k]->x;
    line1[1] = m_ply->point_vector[k]->y;
    line1[2] = 0.0;
    line2[0] = m_ply->point_vector[k+1]->x;
    line2[1] = m_ply->point_vector[k+1]->y;
    line2[2] = 0.0;
    length[k] = MCalcDistancePointToPoint(line2, line1);
    gesamte_laenge += length[k];
  }
  /* Wiederholen bis zumindest ein Knoten gefunden wurde */
  while(anz_relevant==0) {
    /* Schleife ueber alle Knoten */
    for (j=0;j<(long)nod_vector.size();j++) {
      /* Schleife ueber alle Punkte des Polygonzuges */
      for (k=0;k<(long)m_ply->point_vector.size()-1;k++) {
        /* ??? */
        pt1[0] = nod_vector[j]->X();
        pt1[1] = nod_vector[j]->Y();
        pt1[2] = 0.0;
        line1[0] = m_ply->point_vector[k]->x;
        line1[1] = m_ply->point_vector[k]->y;
        line1[2] = 0.0;
        line2[0] = m_ply->point_vector[k+1]->x;
        line2[1] = m_ply->point_vector[k+1]->y;
        line2[2] = 0.0;
        /* Ist der Knoten nah am Polygonabschnitt? */
        if ( MCalcDistancePointToLine(pt1,line1,line2) <= mult_eps*m_ply->epsilon ) {
          /* Im folgenden wird mit der Projektion weitergearbeitet */
          MCalcProjectionOfPointOnLine(pt1,line1,line2,pt1);
          /* Abstand des Punktes zum ersten Punkt des Polygonabschnitts */
          dist1p = MCalcDistancePointToPoint(line1, pt1);
          /* Abstand des Punktes zum zweiten Punkt des Polygonabschnitts */
          dist2p = MCalcDistancePointToPoint(line2, pt1);
          /* Ist der Knoten innerhalb des Intervalls? */
          /* bis rf3807: if ((length[k] - dist1p - dist2p + MKleinsteZahl)/(length[k] + dist1p + dist2p + MKleinsteZahl) > -MKleinsteZahl){ */
          if ((dist1p+dist2p-length[k]) <=  mult_eps*m_ply->epsilon ) {
            // For boundara conditions. WW
            m_ply->sbuffer.push_back(dist1p);
            m_ply->ibuffer.push_back(k);
            anz_relevant++;
            /* Feld anpassen */
            //nodes_all = (long *) Realloc(nodes_all,sizeof(long)*anz_relevant);
            relevant = (INFO *) Realloc(relevant, sizeof(INFO) * anz_relevant);
            /* Ablegen von Knotennummer und Position */
            //nodes_all[anz_relevant-1] = j;
            msh_nod_vector.push_back(j);
            /* Position ermitteln */
            laenge = 0.;
            for (l=0; l < k; l++)
              laenge += length[l];
            /* Ablegen von Knotennummer und Position */
            relevant[anz_relevant-1].knoten = j;
            relevant[anz_relevant-1].laenge = laenge + dist1p;
            /* Suche am Polygon abbrechen, naechster Knoten */
            k =(long)m_ply->point_vector.size();
          }
        } /* endif */
      } /* Ende Schleife ueber Polygonabschnitte */
    } /* Ende Schleife ueber Knoten */
    if(anz_relevant==0) mult_eps *=2.;
  } /* Ende Schleife Wiederholungen */
  if (mult_eps > 1.)
    cout << "!!! Epsilon increased in sources!" << endl;
  /* Schleife ueber alle Knoten; sortieren nach Reihenfolge auf dem Abschnitt (zyklisches Vertauschen, sehr lahm)*/
  do {
    weiter = 0;
    for (k=0;k<anz_relevant-1;k++) {
      w1=relevant[k].laenge;
      w2=relevant[k+1].laenge;
      if (w1>w2) { /* Die Eintraege vertauschen */
        knoten_help = relevant[k].knoten;
        laenge_help = relevant[k].laenge;
        relevant[k].knoten = relevant[k+1].knoten;
        relevant[k].laenge = relevant[k+1].laenge;
        relevant[k+1].knoten = knoten_help;
        relevant[k+1].laenge = laenge_help;
        weiter=1;
      }
    }
  } while (weiter);
  relevant = (INFO*) Free(relevant);
}

/**************************************************************************
MSHLib-Method:
Task:
   const int NLayers        :      Number of layers (start mesh)
   const int Layer          :      Layer number of the layer to be refined 
   const int SUBLayers      :      Number of sublayers
Programing:
10/2003 WW/MB Erste Version (PrismRefine)
08/2004 MB NElementsPerLayer = msh_no_pris / NLayers; 
09/2005 OK MSH
***************************************************************************/
void CFEMesh::PrismRefine(const int Layer, const int subdivision)
{
  const int nn=6;
  int i, j,nes;
  int iii;
  long *element_nodes = NULL;
//WW  static long ele_nodes[6];
  static double nx[6], ny[6], nz[6];
//WW  static int newNode0[3], newNode1[3];  
  double dx[3], dy[3], dz[3];
  double newz;
//WW  static double  newx0[3],newy0[3],newz0[3];
//WW  static double  newx1[3],newy1[3],newz1[3];
  int NumNodesNew, NumElementNew;  
  long *knoten = NULL; 
  int NNodesPerRow = 0;
  int NElementsPerLayer = 0;
  int row;
  int NRowsToShift;
  int NRows;
  int count;
  int CountNLayers;
  CNode* m_nod = NULL;
  double xyz[3];
  //----------------------------------------------------------------------
  const int NSubLayers = subdivision + 1; //OK
  const long NumElement0 = (long)ele_vector.size();
  NNodesPerRow = (long)nod_vector.size() / (no_msh_layer+1); //NodeListSize() / (NLayers+1); 
  NElementsPerLayer = (long)ele_vector.size() / no_msh_layer; //msh_no_pris / NLayers;
  row = Layer;
  NRows = no_msh_layer + 1;
  NumNodesNew = (long)nod_vector.size() - 1; //NodeListSize()-1;
  NumElementNew = (long)ele_vector.size() - 1; //ElListSize()-1;
  //----------------------------------------------------------------------
  long nod_vector_size_add = NNodesPerRow * subdivision;
  for(i=0;i<nod_vector_size_add;i++){
    m_nod = new CNode(i);
    nod_vector.push_back(m_nod);
  }
  //nod_vector.resize(nod_vector_size_new);
  //----------------------------------------------------------------------
  // Initialisierung der Knoten flags
  for(i=0;i<(long)nod_vector.size();i++){
    nod_vector[i]->SetMark(true);
  }
  //======================================================================
  CElem* m_ele = NULL;
  CElem* m_ele_new = NULL;
  for(int ne=0;ne<NumElement0;ne++){
    m_ele = ele_vector[ne];
    if(m_ele->GetElementType()==6){
      //element_nodes = m_ele->nodes;
      CountNLayers = no_msh_layer;
      for(i=0; i<nn; i++)   {
        nx[i] = nod_vector[m_ele->nodes_index[i]]->X();
        ny[i] = nod_vector[m_ele->nodes_index[i]]->Y();
        nz[i] = nod_vector[m_ele->nodes_index[i]]->Z();
      } 
      nes=0;
      for(i=0;i<3;i++){
        if(element_nodes[i] >= (row-1) * NNodesPerRow  &&
           element_nodes[i] <= (row * NNodesPerRow) -1){
          nes++;    
        }  
      }
      if(nes==3){
        for(i=0;i<3;i++){
          dx[i] = (nx[i+3]-nx[i])/(float)NSubLayers;
          dy[i] = (ny[i+3]-ny[i])/(float)NSubLayers;
          dz[i] = (nz[i+3]-nz[i])/(float)NSubLayers;
        }
        //----------------------------------------------------------------
        // Create new nodes
        for(iii=0;iii< NSubLayers-1;iii++) {  // Loop over SubLayers
          //..............................................................
          // neue Knoten ganz unten
          for(i=0;i<3;i++){
            //if(NODGetFreeSurfaceFlag(element_nodes[i])==0){  
            if(nod_vector[element_nodes[i]]->GetMark()){  
              //m_nod = new CMSHNodes(); //kno = (Knoten *)CreateNodeGeometry(); 
              m_nod = nod_vector[(m_ele->nodes_index[i] + ((CountNLayers+2) - row) * NNodesPerRow)];
              xyz[0] = nod_vector[m_ele->nodes_index[i] + ((CountNLayers+1) - row) * NNodesPerRow]->X();
              xyz[1] = nod_vector[m_ele->nodes_index[i] + ((CountNLayers+1) - row) * NNodesPerRow]->Y();
              xyz[2] = nod_vector[m_ele->nodes_index[i] + ((CountNLayers+1) - row) * NNodesPerRow]->Z();
              //PlaceNode(kno,(element_nodes[i] + ((CountNLayers+2) - row) * NNodesPerRow));
              m_nod->SetCoordinates(xyz);
              nod_vector[(m_ele->nodes_index[i] + ((CountNLayers+2) - row) * NNodesPerRow)] = m_nod;
            }
          }  
          //..............................................................
          // neues Element ganz unten
          m_ele_new = new CElem(); 
          //m_ele_new = m_ele;
          m_ele_new->nnodes = m_ele->nnodes;
          m_ele_new->patch_index = m_ele->GetPatchIndex();
          m_ele_new->SetElementType(m_ele->GetElementType());
          knoten = new long[6];
          for(j=0;j<3;j++){
            knoten[j] =   element_nodes[j] + ((CountNLayers+1) - row) * NNodesPerRow;
            knoten[j+3] = element_nodes[j] + ((CountNLayers+2) - row) * NNodesPerRow;
          } 
          for(j=0;j<6;j++){
            m_ele_new->nodes_index[j] = knoten[j];
          }
          ele_vector.push_back(m_ele_new);
          //..............................................................
          /* "rowx hochziehen"   */
          /* loop ?er die betroffenen rows   */
          NRowsToShift = NRows - Layer;
          count = 0;
          for(i=NRowsToShift;i>0;i--){
            if (i!=1){
              count++;
              for(j=0;j<3;j++){
                //if(NODGetFreeSurfaceFlag(element_nodes[j])==0){  
                if(nod_vector[element_nodes[j]]->GetMark()){  
                  m_nod = nod_vector[element_nodes[j+3] + NNodesPerRow*(iii+i-1)];
                  m_nod->SetZ(nod_vector[element_nodes[j] + NNodesPerRow*(iii+i-1)]->Z());
                  //SetNodeZ(element_nodes[j+3] + NNodesPerRow*(iii+i-1), GetNodeZ(element_nodes[j] + NNodesPerRow*(iii+i-1)));
                } 
              }
            }
            else{
              for(j=0;j<3;j++){
                //if(NODGetFreeSurfaceFlag(element_nodes[j])==0)   {  
                if(nod_vector[element_nodes[j]]->GetMark()){  
                  newz = nod_vector[element_nodes[j]]->Z() + (dz[j]*(iii+1));
                  //newz = GetNodeZ(element_nodes[j]) + (dz[j]*(iii+1));
                  nod_vector[element_nodes[j] + (i) * NNodesPerRow *(iii+1)]->SetZ(newz);
                  //SetNodeZ(element_nodes[j] + (i) * NNodesPerRow *(iii+1), newz);
                }                               
              } 
            }
          } /* end for Rows to shift */
          //..............................................................
          if(iii==NSubLayers-2){
            for(j=0;j<3;j++){
              //NODSetFreeSurfaceFlag(element_nodes[j], 33);
              nod_vector[element_nodes[j]]->SetMark(false);
            }
          }  
          CountNLayers++; 
        } /* End Loop over SubLayers  */
      }  /* End if nes==3 */
    }  /* Elementtyp ==6 */
  }  /* Element loop */ 
  //======================================================================
  no_msh_layer += subdivision;
  //----------------------------------------------------------------------
}

/**************************************************************************
MSHLib-Method:
Programing:
09/2005 TK/OK Implementation ab 4.2
***************************************************************************/
void CFEMesh::EdgeLengthMinMax()
{
  int j;
  long i;
  double edge_length;
  CNode* m_nod1 = NULL;
  CNode* m_nod2 = NULL;
  CElem* m_ele = NULL;
  for(i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    for(j=0;j<m_ele->nnodes-1;j++){
      m_nod1 = nod_vector[m_ele->nodes_index[j]];
      m_nod2 = nod_vector[m_ele->nodes_index[j+1]];
      edge_length = sqrt(((m_nod1->X()-m_nod2->X())*(m_nod1->X()-m_nod2->X()))+ \
                         ((m_nod1->Y()-m_nod2->Y())*(m_nod1->Y()-m_nod2->Y()))+ \
                         ((m_nod1->Z()-m_nod2->Z())*(m_nod1->Z()-m_nod2->Z())));
      if(i==0&&j==0)
      {
        min_edge_length = edge_length;
        max_edge_length = edge_length;
      }
      else
      {
        if(min_edge_length > edge_length)
          min_edge_length = edge_length;
        if (max_edge_length < edge_length) 
          max_edge_length = edge_length;
      }
    }
    m_nod1 = nod_vector[m_ele->nodes_index[m_ele->nnodes-1]];
    m_nod2 = nod_vector[m_ele->nodes_index[0]];
    edge_length = sqrt(((m_nod1->X()-m_nod2->X())*(m_nod1->X()-m_nod2->X()))+ \
                       ((m_nod1->Y()-m_nod2->Y())*(m_nod1->Y()-m_nod2->Y()))+ \
                       ((m_nod1->Z()-m_nod2->Z())*(m_nod1->Z()-m_nod2->Z())));
    if(min_edge_length > edge_length)
      min_edge_length = edge_length;
    if (max_edge_length < edge_length) 
        max_edge_length = edge_length;
  }
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
09/2005 OK Implementation
09/2005 OK Epsilon
10/2005 OK Delete existing layer polylines
02/2006 CC polyline id 
**************************************************************************/
void CFEMesh::CreateLayerPolylines(CGLPolyline*m_ply)
{
  long i,j;
  CGLPoint* m_point = NULL;
  CGLPolyline* m_polyline = NULL;
  CNode* m_nod = NULL;
  char layer_number[3];
  //---------------------------------------------------------------------
  // Delete existing layer polylines
  string ply_lay_name = m_ply->name + "_L";
  for(int k=0;k<(int)polyline_vector.size();k++){
    m_polyline = polyline_vector[k];
    if(m_polyline->name.find(ply_lay_name)!=string::npos){ 
      GEORemovePLY(m_polyline);
      //GEORemovePolyline(polyline_vector.begin()+(k-l));
      k--;
    }
  }
  //---------------------------------------------------------------------
  //
  vector<long>ply_nod_vector;
  vector<long>ply_nod_vector_dummy;
  GetNODOnPLY_XY(m_ply,ply_nod_vector);
  //nodes = MSHGetNodesCloseXY(&no_nodes); //OK41
  long nodes_per_layer = (long)nod_vector.size()/(no_msh_layer+1);
  int ply_nod_vector_layer = (int)ply_nod_vector.size()/(no_msh_layer+1);
  //---------------------------------------------------------------------
  // Create layer polylines
 //polyline id CC8888---------------------------------------------------
    long size = 0;
    CGLPolyline *ms_polyline = NULL;
    long number_of_polylines = (long)polyline_vector.size();
    if(number_of_polylines==0)
        size = 0;  
    else
      {
        vector<CGLPolyline*>::iterator ps = polyline_vector.begin();
        while (ps!=polyline_vector.end()){
          ms_polyline = *ps;
          ++ps;
      }
        size = ms_polyline->id + 1;
      }

  if(ply_nod_vector_layer<1){
#ifdef MFC
    AfxMessageBox("Not enough NOD data, increase EPS");
#endif
    return;
  }
  //---------------------------------------------------------------------
  // Create layer polylines
  //......................................................................
  m_polyline = new CGLPolyline;
  sprintf(layer_number,"%ld",0L);
  //CString names =  m_ply->name + "_L" + layer_number;
 // m_polyline->name = names;
  m_polyline->ply_name = m_ply->name.data();//CC/TK8888
  m_polyline->ply_name.append("_L");//CC/TK8888
  m_polyline->ply_name.append(layer_number);//CC/TK8888
  m_polyline->name  = m_polyline->ply_name.data();//CC/TK8888
  m_polyline->data_type = 1;
  m_polyline->id = size;//CC8888
  m_polyline->epsilon = m_ply->epsilon; //OK
  m_polyline->ply_data = m_polyline->ply_name + ".ply";//CC
  m_polyline->ply_type = "NOD_POINTS";//CC
#ifdef MFC
  if(m_ply->epsilon<min_edge_length)
    AfxMessageBox("Warning: PLY-EPS < MSH_MIN_EDGE_LENGHT");
    //m_polyline->epsilon = min_edge_length / 2.; //OK
#endif
  for(i=0;i<ply_nod_vector_layer;i++) {
    m_point = new CGLPoint;
    m_nod = nod_vector[ply_nod_vector[i]];
    m_point->x = m_nod->X();
    m_point->y = m_nod->Y();
    m_point->z = m_nod->Z();
    m_polyline->point_vector.push_back(m_point);
  }
  m_polyline->SetPointOrderByDistance(m_ply->point_vector[0]);
  polyline_vector.push_back(m_polyline); 
  m_polyline->WritePointVector(m_polyline->ply_name);
  m_polyline->WriteTecplot(" "); //OK41
  //......................................................................
  for(j=1;j<(no_msh_layer+1);j++) {
    m_polyline = new CGLPolyline;
    sprintf(layer_number,"%ld",j);
    m_polyline->ply_name = m_ply->name.data();//CC/TK8888
    m_polyline->ply_name.append("_L");//CC/TK8888
    m_polyline->ply_name.append(layer_number);//CC/TK8888
    m_polyline->name  = m_polyline->ply_name.data();//CC/TK8888
    m_polyline->data_type = 1;
    m_polyline->ply_type = "NOD_POINTS";
    m_polyline->epsilon = min_edge_length / 2.; //OK
    for(i=0;i<ply_nod_vector_layer;i++) {
      m_point = new CGLPoint;
      m_nod = nod_vector[ply_nod_vector[i]+j*nodes_per_layer];
      m_point->x = m_nod->X();
      m_point->y = m_nod->Y();
      m_point->z = m_nod->Z();
      m_polyline->point_vector.push_back(m_point);
    }
//OK    m_polyline->SortPointVectorByDistance();
    m_polyline->SetPointOrderByDistance(m_ply->point_vector[0]);
    polyline_vector.push_back(m_polyline); 
    m_polyline->WritePointVector(m_polyline->ply_name);
    m_polyline->WriteTecplot(" "); //OK41
  }
}


/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
10/2005 OK Implementation based on GEOWriteVolumes2Tecplot
last modification: 
**************************************************************************/
void CFEMesh::ELEVolumes()
{
  long i;
  long ib,ie;
  int vol_number = 0;
  double* center;
  // GEO
  CGLPoint m_point;
  Surface* m_sfc = NULL;
  vector<Surface*>::const_iterator p_sfc; // vector in GEOLib2
  CGLVolume *m_vol = NULL;
  vector<CGLVolume*>::const_iterator p_vol; // vector in GEOLib2
  // ELE
  CElem* m_ele = NULL;
  long ep_layer = (long)ele_vector.size() / no_msh_layer;
  //----------------------------------------------------------------------
  p_vol = volume_vector.begin();
  while(p_vol!=volume_vector.end()) {
    m_vol = *p_vol;
    p_sfc = m_vol->surface_vector.begin();
    m_sfc = *p_sfc;
    if(!m_sfc){
      cout << "Warning in CFEMesh::ELEVolumes(): no SFC data" << '\n';
      return;
    }
    //....................................................................
    ib = (m_vol->layer-1)*ep_layer;
    ie = ib + ep_layer;
    //....................................................................
    for(i=ib;i<ie;i++){
      if(ele_vector[i]->GetElementType()==6){
        m_ele = ele_vector[i];
        center = m_ele->GetGravityCenter();
        m_point.x = center[0];
        m_point.y = center[1];
        m_point.z = center[2];
        if(m_sfc->PointInSurface(&m_point)){
          m_ele->volume = vol_number;
        }
      }
    }
    //....................................................................
    vol_number++;
    ++p_vol;
  //----------------------------------------------------------------------
  }
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CFEMesh::SetMATGroupFromVOLLayer(CGLVolume*m_vol)
{
  CElem* m_ele = NULL;
  //......................................................................
  CGLPoint m_pnt;
  Surface* m_sfc = NULL;
  vector<Surface*>::const_iterator p_sfc; 
  p_sfc = m_vol->surface_vector.begin();
  m_sfc = *p_sfc;
  //......................................................................
  long ep_layer = (long)ele_vector.size() / no_msh_layer;
  long jb = (m_vol->layer-1)*ep_layer;
  long je = jb + ep_layer;
  double* xy;
  for(long j=jb;j<je;j++){
    // Point in surface
    m_ele = ele_vector[j];
    xy = m_ele->GetGravityCenter();
    m_pnt.x = xy[0];
    m_pnt.y = xy[1];
    if(m_sfc->PointInSurface(&m_pnt))
      m_ele->SetPatchIndex(m_vol->mat_group);
  }
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
09/2005 OK Implementation
**************************************************************************/
void CFEMesh::SetMATGroupsFromVOLLayer()
{
  CGLVolume* m_vol;
  vector<CGLVolume*>::const_iterator p_vol = volume_vector.begin();
  while(p_vol!=volume_vector.end()) {
    m_vol = *p_vol;
    //....................................................................
    // LAYER
    SetMATGroupFromVOLLayer(m_vol);
    //....................................................................
    ++p_vol;
  }
}
/**************************************************************************
FEMLib-Method:
Task: Ermittelt das nahliegende Element
Programing:
11/2005 MB, based on GetNearestElement
**************************************************************************/
long CFEMesh::GetNearestELEOnPNT(CGLPoint*m_pnt)
{
  long i, nextele;
  double ex, ey, ez, dist, dist1, dist2;
  double x, y, z;
  double* center = NULL;
  Mesh_Group::CElem* m_ele = NULL;
	
  dist = 10000000.0; //Startwert
  dist2 = 0.01;	// Abstand zwischen eingelesenen Knoten und Geometrieknoten-RF; 
					// Achtung, doppelbelegung m?lich bei kleinen Gitterabst?den
  nextele = -1;

  x = m_pnt->x;
  y = m_pnt->y;
  z = m_pnt->z;
  
  for(i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    center = m_ele->GetGravityCenter();
    ex = center[0];
	ey = center[1];
	ez = center[2];
	dist1 = (ex-x)*(ex-x)+(ey-y)*(ey-y)+(ez-z)*(ez-z);
	if(dist1<dist){
	  dist = dist1;
	  nextele = i;
	}
  }
  return nextele;

}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2005 MB Implementation based on NodeExists..
**************************************************************************/
bool CFEMesh::NodeExists(long node)
{
  long i;
  long no_nodes = (long)nod_vector.size();
 
  for(i=0;i<no_nodes;i++){
    if(node==Eqs2Global_NodeIndex[i])
      return true;
  }
  return false;
}

/**************************************************************************
MSHLib-Method:
Task:
Programing:
10/2005 OK Implementation
02/2006 WW Ordering and remove bugs
**************************************************************************/
void CFEMesh::ConnectedNodes()
{
  int i, j,l, k, n;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  bool exist = false;
#define noTestConnectedNodes
  //----------------------------------------------------------------------
  for(i=0;i<(long)nod_vector.size();i++){
    m_nod = nod_vector[i];
    for(j=0;j<(int)m_nod->connected_elements.size();j++){
      m_ele = ele_vector[m_nod->connected_elements[j]];
      for(l=0;l<m_ele->GetNodesNumber(false);l++){
          exist = false;
          for(k=0;k<(int)m_nod->connected_nodes.size();k++) //WW
          {
             if(m_nod->connected_nodes[k]==m_ele->nodes_index[l])
             {
                 exist = true;
                 break;
             }
          }
          if(!exist) //WW
             m_nod->connected_nodes.push_back(m_ele->nodes_index[l]);      
      }
    }
  }
  //----------------------------------------------------------------------
 /*
  // Memory problem Commented by WW.
  vector<long>aux_vector;
  bool flag;
  //......................................................................
  for(i=0;i<(long)nod_vector.size();i++){
    m_nod = nod_vector[i];
    k = m_nod->connected_nodes[0];
    aux_vector.push_back(k);
    //....................................................................
    for(j=1;j<(int)m_nod->connected_nodes.size();j++){
      k = m_nod->connected_nodes[j];
      int aux_vector_size = (int)aux_vector.size();
      flag = true;
      for(l=0;l<aux_vector_size;l++){
        if(k==aux_vector[l])
          flag = false;
      }
      if(flag) {
        aux_vector.push_back(k);
      }
    }
    //....................................................................
    m_nod->connected_nodes.clear();
    m_nod->connected_nodes.resize((int)aux_vector.size());
    m_nod->connected_nodes = aux_vector;
    aux_vector.clear();
  }  
 */
  // Sorting. WW
  for(i=0;i<(long)nod_vector.size();i++){//WW
    m_nod = nod_vector[i];
    j = (int)m_nod->connected_nodes.size();
    for(k=0; k<j; k++)
    {     
       for(l=k; l<j; l++)
       {
          if(m_nod->connected_nodes[l]<m_nod->connected_nodes[k])
          {
             n = m_nod->connected_nodes[k];
             m_nod->connected_nodes[k] = m_nod->connected_nodes[l];
             m_nod->connected_nodes[l] = n;
          }
       }     
    }
    
//    for(k=0; k<j; k++)
//      cout<<m_nod->connected_nodes[k]<<"  ";
//    cout<<endl;


    m_nod->m5_index.resize(j);
  }
  //----------------------------------------------------------------------
#ifdef TestConnectedNodes
  for(i=0;i<(long)nod_vector.size();i++){
    m_nod = nod_vector[i];
    cout << (int)m_nod->connected_nodes.size() << ": ";
    for(m=0;m<(int)m_nod->connected_nodes.size();m++){
      cout << m_nod->connected_nodes[m] << " ";
    }
    cout << endl;
  }
#endif
}



/**************************************************************************
MSHLib-Method: 
Task: Copies the selected nodes to a msh_node_vector
Programing:
12/2005 TK Implementation
**************************************************************************/
void CFEMesh::CopySelectedNodes(vector<long>&msh_nod_vector)
{
  int i=0, j=0;
  // Init
  msh_nod_vector.clear();

    //Loop over all meshes
    for(j=0;j<(long)fem_msh_vector.size();j++)
    {
    //Loop over all mesh nodes
    for(i=0;i<(long)fem_msh_vector[j]->nod_vector.size();i++)
    {
      if (fem_msh_vector[j]->nod_vector[i]->selected == 1)
      {
        msh_nod_vector.push_back(fem_msh_vector[j]->nod_vector[i]->GetIndex());
      }
    }
    }  
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
03/2004 OK Implementation
11/2005 OK MSH
03/2006 CC
**************************************************************************/
void CFEMesh::CreateSurfaceTINfromTri(Surface*m_sfc)
{
  int j;
  CGLPoint m_point;
  CTriangle *m_triangle;
  CElem* m_ele = NULL;
  double* xyz;
  vec<long>node_indeces(3);
  m_sfc->TIN = new CTIN;//CC
  //----------------------------------------------------------------------
  for(long i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    if(m_ele->GetElementType()==4){ // use only triangles
      xyz = m_ele->GetGravityCenter();
      m_point.x = xyz[0];
      m_point.y = xyz[1];
      m_point.z = xyz[2];
      if(IsPointInSurface(m_sfc,&m_point)){
        m_triangle = new CTriangle;
        m_triangle->number = (long)m_sfc->TIN->Triangles.size();
        m_ele->GetNodeIndeces(node_indeces);
        for(j=0;j<3;j++){
          m_triangle->x[j] = nod_vector[node_indeces[j]]->X();
          m_triangle->y[j] = nod_vector[node_indeces[j]]->Y();
          m_triangle->z[j] = nod_vector[node_indeces[j]]->Z();
        }
        m_sfc->TIN->Triangles.push_back(m_triangle);
        m_sfc->TIN->name = m_sfc->name;//CC
      } // element found
    } // triangle
  } // ele_vector
}

/**************************************************************************
GeoLib-Method: 
Task: Geometric / topological method
Programing:
01/2005 OK Implementation
01/2005 CC add variable :long i for the function
08/2005 CC move from Geolib to Mshlib
**************************************************************************/
void CFEMesh::CreateLayerSurfaceTINsfromPris(Surface*m_sfc)
{
  int j;
  CGLPoint m_point;
  CTriangle *m_tri;
  CTriangle *m_tri_new;
  CElem* m_ele = NULL;
  double* xyz;
  vec<long>node_indeces(6);
  //---------------------------------------------------------------------
  if(!m_sfc){
#ifdef MFC
    AfxMessageBox("No SFC data");
#endif
    return;
  }
  //---------------------------------------------------------------------
  // Create layer surfaces
  char layer_number[3];
  Surface *m_sfc_layer = NULL;
  CTIN *m_TIN = NULL;
  string sfc_layer_name = m_sfc->name + "_layer_";
  for(int l=0;l<no_msh_layer+1;l++) {
    m_sfc_layer = new Surface;
    m_sfc_layer->type_name = "TIN";
    sprintf(layer_number,"%i",l);
    m_sfc_layer->name = sfc_layer_name + layer_number;
    m_sfc_layer->data_name = m_sfc_layer->name + ".tin";
    m_TIN = new CTIN;
    m_TIN->name = m_sfc_layer->name;
    m_sfc_layer->TIN = m_TIN;
    surface_vector.push_back(m_sfc_layer);
  }
  //----------------------------------------------------------------------
  Surface *m_sfc_layer_0 = NULL;
  sfc_layer_name = m_sfc->name + "_layer_0";
  m_sfc_layer_0 = GEOGetSFCByName(sfc_layer_name);
  if(!m_sfc_layer_0){
#ifdef MFC
    AfxMessageBox("No SFC data");
#endif
    return;
  }
  long no_nodes_per_layer = (long)nod_vector.size() / (no_msh_layer+1);
  long no_elements_per_layer = (long)ele_vector.size() / (no_msh_layer);
  for(long i=0;i<no_elements_per_layer;i++){
    m_ele = ele_vector[i];
    if(m_ele->GetElementType()==6){ // prism
      xyz = m_ele->GetGravityCenter();
      m_point.x = xyz[0];
      m_point.y = xyz[1];
      m_point.z = xyz[2];
      if(IsPointInSurface(m_sfc,&m_point)){
        m_tri = new CTriangle;
        m_tri->number = (long)m_sfc_layer_0->TIN->Triangles.size();
        m_ele->GetNodeIndeces(node_indeces);
        for(j=0;j<3;j++){
          m_tri->x[j] = nod_vector[node_indeces[j]]->X();
          m_tri->y[j] = nod_vector[node_indeces[j]]->Y();
          m_tri->z[j] = nod_vector[node_indeces[j]]->Z();
        }
        m_sfc_layer_0->TIN->Triangles.push_back(m_tri);
        m_sfc_layer_0->TIN->name = m_sfc_layer_0->name;//CC
        for(int l=0;l<no_msh_layer;l++){
          sprintf(layer_number,"%i",l+1);
          sfc_layer_name = m_sfc->name + "_layer_" + layer_number;
          m_sfc_layer = GEOGetSFCByName(sfc_layer_name);
          if(!m_sfc_layer){
#ifdef MFC
            AfxMessageBox("No SFC data");
#endif
            return;
          }
          m_tri_new = new CTriangle;
          for(j=0;j<3;j++) {
            m_tri_new->number = m_tri->number;
            m_tri_new->x[j] = m_tri->x[j];
            m_tri_new->y[j] = m_tri->y[j];
            m_tri_new->z[j] = nod_vector[node_indeces[j+3]+l*no_nodes_per_layer]->Z();
          }
          m_sfc_layer->TIN->Triangles.push_back(m_tri_new);
          m_sfc_layer->TIN->name = m_sfc_layer->name;//CC
        }
      } // element found
    } // triangle
  } // ele_vector
  //---------------------------------------------------------------------
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2005 OK
last modification:
**************************************************************************/
void CFEMesh::GetELEOnSFC(Surface*m_sfc,vector<long>&msh_ele_vector)
{
  msh_ele_vector.clear();
  //----------------------------------------------------------------------
   switch(m_sfc->type) {
    //....................................................................
    case 0: // Surface polygon
      cout << "Warning in CFEMesh::GetELEOnSFC: case not implemented" << endl;
      break;
    //....................................................................
    case 1: // TIN
      GetELEOnSFC_TIN(m_sfc,msh_ele_vector);
      break;
    //....................................................................
    case 2: // 2 vertical polylines
      cout << "Warning in CFEMesh::GetELEOnSFC: case not implemented" << endl;
      break;
    //....................................................................
    case 3: // projection on xy plane (all mesh points above and below the surface) //MB
      cout << "Warning in CFEMesh::GetELEOnSFC: case not implemented" << endl;
      break;
    //....................................................................
	case 100:
      cout << "Warning in CFEMesh::GetELEOnSFC: case not implemented" << endl;
	  break;
  }
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2005 OK
last modification:
**************************************************************************/
void CFEMesh::GetELEOnSFC_TIN(Surface*m_sfc,vector<long>&msh_ele_vector)
{
  long i,j;
  double xp[3],yp[3],zp[3];
  CGLPoint m_node;
  CTriangle *m_triangle = NULL;
  CElem* m_ele = NULL;
  double* xyz;
  //----------------------------------------------------------------------
  for(i=0;i<(long)m_sfc->TIN->Triangles.size();i++){
    m_triangle = m_sfc->TIN->Triangles[i];
    xp[0] = m_triangle->x[0];
    yp[0] = m_triangle->y[0];
    zp[0] = m_triangle->z[0];
    xp[1] = m_triangle->x[1];
    yp[1] = m_triangle->y[1];
    zp[1] = m_triangle->z[1];
    xp[2] = m_triangle->x[2];
    yp[2] = m_triangle->y[2];
    zp[2] = m_triangle->z[2];
    for(j=0;j<(long)ele_vector.size();j++){
      m_ele = ele_vector[j];
      xyz = m_ele->GetGravityCenter();
      m_node.x = xyz[0];
      m_node.y = xyz[1];
      m_node.z = xyz[2];
      if(m_node.IsInsideTriangle(xp,yp,zp)){
         msh_ele_vector.push_back(m_ele->GetIndex());
      }
    }
  }
  //----------------------------------------------------------------------
}
/**************************************************************************
FEMLib-Method: 
Task:  Renumbering nodes corresponding to the activiate of elements
Programing:
01/2006 YD Implementation
**************************************************************************/
void CFEMesh::FaceNormal()  
{
   int i,j;
   int idx0_face,idx1_face,idx_owner,index0=0,index1=0;

   CElem* elem = NULL;
   CElem* elem_face = NULL;

  // if(coordinate_system!=32) 
  //   return;
//------------------------
   for (i = 0; i < (int)face_vector.size(); i++)
   {
      double* normal = new double[3]; 
      elem_face = face_vector[i];
	  elem = face_vector[i]->GetOwner();

      if(elem->GetElementType()==1)
      return;

//WW      int no_face_vertex = face_vector[i]->GetVertexNumber();
	  int no_owner_vertex = face_vector[i]->GetOwner()->GetVertexNumber();
      idx0_face = face_vector[i]->GetNodeIndex(0);
      idx1_face = face_vector[i]->GetNodeIndex(1);
	  for(j = 0; j < no_owner_vertex; j++){
      idx_owner = face_vector[i]->GetOwner()->GetNodeIndex(j);
	  if(idx0_face == idx_owner) index0=j;
      }
	  for(j = 0; j < no_owner_vertex; j++){
      idx_owner = face_vector[i]->GetOwner()->GetNodeIndex(j);
	  if(idx1_face == idx_owner) index1=j;
      }
	  if(elem->GetMark()){
	     if((index1-index0) >= 1)
           elem->FaceNormal(index0,index1,normal);
	     else
           elem->FaceNormal(index1,index0,normal);

	  }
      face_normal.push_back(normal);
      elem_face->ComputeVolume();
   }
}

/**************************************************************************
MSHLib-Method:
Programing:
02/2006 OK Implementation
**************************************************************************/
void CFEMesh::CreateLineELEFromTri()
{
  int j,k;
  long i; //,e;
//WW  double v1[3],v2[3],v3[3];
//  double patch_area;
  double x,y,z;
  double x0,y0,z0;
  double x1,y1,z1;
  double dl;
  CNode* m_nod = NULL;
//WW  CNode* m_nod1 = NULL;
//WW  CNode* m_nod2 = NULL;
  CNode* m_nod_line = NULL;
  CElem* m_tri_ele = NULL;
  CElem* m_ele = NULL;
//  CElem* m_ele1 = NULL;
  //----------------------------------------------------------------------
  // 1 - Element normal vector (for 2D elements only)
  SetELENormalVectors();
  //----------------------------------------------------------------------
  // 2 - Node patch area
  SetNODPatchAreas();
  //----------------------------------------------------------------------
  // 3 - Intersection nodes
  //OKSetNetworkIntersectionNodes();
  //----------------------------------------------------------------------
  // 4 - Create MSH
  MSHDelete("LINE_from_TRI");
  CFEMesh* m_msh_line = NULL;
  m_msh_line = new CFEMesh();
  m_msh_line->pcs_name = "LINE_from_TRI";
  m_msh_line->ele_type = 1;
  m_msh_line->no_msh_layer = 10; // User-defined
  double element_length = -0.1; // User-defined
  dl = element_length * m_msh_line->no_msh_layer;
  //----------------------------------------------------------------------
  // 4.1 - Line nodes
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i];
    //OKif(m_nod->selected)
      //OKcontinue;
    if((int)m_nod->connected_elements.size()==0)
      continue;
    m_tri_ele = ele_vector[m_nod->connected_elements[0]];
    //....................................................................
    // Node normal vector
    x0 = m_nod->X();
    y0 = m_nod->Y();
    z0 = m_nod->Z();
    x1 = x0 + m_tri_ele->normal_vector[0]*dl;
    y1 = y0 + m_tri_ele->normal_vector[1]*dl;
    z1 = z0 + m_tri_ele->normal_vector[2]*dl;
    //....................................................................
    for(j=0;j<m_msh_line->no_msh_layer+1;j++)
    {
      x =  x0 + (x1-x0)*(j)/m_msh_line->no_msh_layer;
      y =  y0 + (y1-y0)*(j)/m_msh_line->no_msh_layer;
      z =  z0 + (z1-z0)*(j)/m_msh_line->no_msh_layer;
      m_nod_line = new CNode((long)m_msh_line->nod_vector.size(),x,y,z);
      m_msh_line->nod_vector.push_back(m_nod_line);
    }
  }
  //----------------------------------------------------------------------
  // 4.2 - Line elements
  long i_count = 0;
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i];
    //....................................................................
    // Intersection node
    if(m_nod->selected)
      continue;
    if((int)m_nod->connected_elements.size()==0)
      continue;
    m_tri_ele = ele_vector[m_nod->connected_elements[0]];
    //....................................................................
    // Line elements
    for(j=0;j<m_msh_line->no_msh_layer;j++)
    {
      m_ele = new Mesh_Group::CElem;
      m_ele->SetIndex((long)m_msh_line->ele_vector.size());
      m_ele->SetElementType(1);
      m_ele->nnodes = 2;
      m_ele->nodes_index.resize(m_ele->nnodes);
      //....................................................................
      // Line element nodes
      for(k=0;k<m_ele->nnodes;k++)
      {
        m_ele->nodes_index[k] = i_count*m_msh_line->no_msh_layer + j + k + i_count;
        m_ele->nodes[k] = m_msh_line->nod_vector[m_ele->nodes_index[k]];
      }
      //....................................................................
      m_msh_line->ele_vector.push_back(m_ele);
    }
    i_count++;
  }
  //----------------------------------------------------------------------
  if(m_msh_line->ele_vector.size()>0)
    fem_msh_vector.push_back(m_msh_line);
  else 
    delete m_msh_line;
  //----------------------------------------------------------------------
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("gli");
  if(m_gsp)
    FEMWrite(m_gsp->path + "test");
  else
    FEMWrite("test");
}

/**************************************************************************
MSHLib-Method:
Programing:
03/2006 OK Implementation
**************************************************************************/
void CFEMesh::SetELENormalVectors()
{
  //----------------------------------------------------------------------
  long i;
  double v1[3],v2[3];
//WW  double patch_area;
  CNode* m_nod = NULL;
  CNode* m_nod1 = NULL;
  CNode* m_nod2 = NULL;
  CElem* m_tri_ele = NULL;
  //----------------------------------------------------------------------
  for(i=0;i<(long)ele_vector.size();i++)
  {
    m_tri_ele = ele_vector[i];
    if(m_tri_ele->GetElementType()!=4) // just for triangles
      continue;
    m_nod = nod_vector[m_tri_ele->GetNodeIndex(0)];
    m_nod1 = nod_vector[m_tri_ele->GetNodeIndex(1)];
    m_nod2 = nod_vector[m_tri_ele->GetNodeIndex(2)];
    v1[0] = m_nod1->X() - m_nod->X();
    v1[1] = m_nod1->Y() - m_nod->Y();
    v1[2] = m_nod1->Z() - m_nod->Z();
    v2[0] = m_nod2->X() - m_nod->X();
    v2[1] = m_nod2->Y() - m_nod->Y();
    v2[2] = m_nod2->Z() - m_nod->Z();
    CrossProduction(v1,v2,m_tri_ele->normal_vector);
    NormalizeVector(m_tri_ele->normal_vector,3);
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
MSHLib-Method:
Programing:
03/2006 OK Implementation
**************************************************************************/
void CFEMesh::SetNODPatchAreas()
{
  long i,e;
  int j,k;
  int n1=0,n2=0;
  double v1[3],v2[3],v3[3];
  double patch_area;
  double x0,y0,z0;
  double* gravity_center;
  CNode* m_nod = NULL;
  CNode* m_nod1 = NULL;
  CNode* m_nod2 = NULL;
  CElem* m_ele = NULL;
  //----------------------------------------------------------------------
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i]; // this node
    patch_area = 0.0;
    //....................................................................
    // triangle neighbor nodes
    for(j=0;j<(int)m_nod->connected_elements.size();j++)
    {
      e = m_nod->connected_elements[j];
      m_ele = ele_vector[e];
      for(k=0;k<3;k++)
      {
        if(m_ele->GetNodeIndex(k)==i)
        {
          switch(k)
          {
            case 0:
              n1 = 2;
              n2 = 1;
              break;
            case 1:
              n1 = 0;
              n2 = 2;
              break;
            case 2:
              n1 = 1;
              n2 = 0;
              break;
          }
        }
      }
      //..................................................................
      gravity_center = m_ele->GetGravityCenter();
      v2[0] = gravity_center[0] - m_nod->X();
      v2[1] = gravity_center[1] - m_nod->Y();
      v2[2] = gravity_center[2] - m_nod->Z();
      //..................................................................
      m_nod1 = nod_vector[m_ele->GetNodeIndex(n1)];
      x0 = 0.5*(m_nod1->X()-m_nod->X());
      y0 = 0.5*(m_nod1->Y()-m_nod->Y());
      z0 = 0.5*(m_nod1->Z()-m_nod->Z());
      v1[0] = x0 - m_nod->X();
      v1[1] = y0 - m_nod->Y();
      v1[2] = z0 - m_nod->Z();
      CrossProduction(v1,v2,v3);
      patch_area += 0.5*MBtrgVec(v3,3);
      //..................................................................
      m_nod2 = nod_vector[m_ele->GetNodeIndex(n2)];
      x0 = 0.5*(m_nod2->X()-m_nod->X());
      y0 = 0.5*(m_nod2->Y()-m_nod->Y());
      z0 = 0.5*(m_nod2->Z()-m_nod->Z());
      v1[0] = x0 - m_nod->X();
      v1[1] = y0 - m_nod->Y();
      v1[2] = z0 - m_nod->Z();
      CrossProduction(v1,v2,v3);
      patch_area += 0.5*MBtrgVec(v3,3);
      //..................................................................
    }
    m_nod->patch_area = patch_area;
  }
}

/**************************************************************************
MSHLib-Method:
Programing:
03/2006 OK Implementation
**************************************************************************/
void CFEMesh::SetNetworkIntersectionNodes()
{
  long i,e;
  int j;
  double v3[3];
//WW  double* gravity_center;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  CElem* m_ele1 = NULL;
  //----------------------------------------------------------------------
  // Is node intersection node
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i];
    m_nod->selected = false;
  }
  double eps = 1e-3;
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i]; // this node
    if((int)m_nod->connected_elements.size()==0)
      continue;
    m_ele = ele_vector[m_nod->connected_elements[0]];
    //....................................................................
    // Compare element normal vectors
    for(j=1;j<(int)m_nod->connected_elements.size();j++)
    {
      e = m_nod->connected_elements[j];
      m_ele1 = ele_vector[e];
      CrossProduction(m_ele->normal_vector,m_ele1->normal_vector,v3);
      if(MBtrgVec(v3,3)>eps)
        m_nod->selected = true;
    }
  }
  // Count non-intersection nodes
  long no_non_intersection_nodes = 0;
  for(i=0;i<(long)nod_vector.size();i++)
  {
    m_nod = nod_vector[i];
    if(m_nod->selected)
      continue;
    no_non_intersection_nodes++;
  }
}

} // namespace Mesh_Group
//========================================================================
