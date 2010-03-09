/**************************************************************************
MSHLib - Object:
Task:
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#include "rf_mmp_new.h"

#include "mathlib.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  //WW

using namespace std;

#include "gs_project.h"

// GEOLib
#include "geo_pnt.h"
#include "geo_ply.h"

// FEM
#include "files0.h"

// MSHLib
#include "msh_mesh.h"
#ifdef BENCHMARKING
#include "benchtimer.h"
#endif
#ifdef RANDOM_WALK
#include "rf_random_walk.h"
#endif
#include "msh_lib.h"
// For surface integration. WW. 29.02.2009
#include "fem_ele.h"
using FiniteElement::CElement;

// PCSLib
extern string GetLineFromFile1(ifstream*);

long msh_no_line = 0;
long msh_no_quad = 0;
long msh_no_hexs = 0;
long msh_no_tris = 0;
long msh_no_tets = 0;
long msh_no_pris = 0;

#define noMSH_CHECK

int max_dim = 0; //OK411

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
  max_ele_dim = 0; //NW
  m_bCheckMSH = false; //OK
  pcs_name = "NotSpecified"; //WW
#ifdef RANDOM_WALK
  PT=NULL; // WW+TK
  fm_pcs=NULL;  //WW
#endif
  // 1.11.2007 WW
#ifdef NEW_EQS
  sparse_graph = NULL;
  sparse_graph_H = NULL;
#endif
  map_counter = 0;  //21.01.2009 WW
  mapping_check = false;  //23.01.2009 WW
  has_multi_dim_ele = false; //NW
#ifdef MFC
  n_area_val = NULL;
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
  // 1.11.2007 WW
#ifdef NEW_EQS
  if(sparse_graph) delete sparse_graph;
  if(sparse_graph_H) delete sparse_graph_H;
  sparse_graph = NULL;
  sparse_graph_H = NULL;
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
  this->max_ele_dim = 0; //NW
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
    if(line_string.find("$GEO_TYPE")!=string::npos) { //OK9_4310
      *fem_file >> geo_type_name >> geo_name >> ws;
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
      string s;
      for(i=0;i<no_nodes;i++)
      {
        *fem_file>>ibuff>>x>>y>>z;
        newNode = new CNode(ibuff,x,y,z);
        nod_vector.push_back(newNode);
        position = fem_file->tellg();
        *fem_file>>s;
        if(s.find("$AREA")!=string::npos)
        {
          *fem_file>>newNode->patch_area;
        }
        else
          fem_file->seekg(position,ios::beg);
        *fem_file>>ws;
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
         if (newElem->GetDimension() > this->max_ele_dim) //NW
           this->max_ele_dim = newElem->GetDimension();
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

#ifdef USE_TOKENBUF
int
CFEMesh::Read(TokenBuf *tokenbuf)
{
  string sub_line;
  string line_string;
  char line_buf[LINE_MAX];
  char str1[LINE_MAX], str2[LINE_MAX];
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
#ifdef BENCHMARKING
  BenchTimer read_timer;

  read_timer.start();
#endif

  //========================================================================
  // Keyword loop
  while (!new_keyword && !tokenbuf->done()) {
    tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
    if(tokenbuf->done()) break;
    line_string = std::string(line_buf);
    if(line_string.find(hash)!=string::npos)
    {
      new_keyword = true;
      break;
    }
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      pcs_name = std::string(line_buf);
      continue;
    }
    //....................................................................
    if(line_string.find("$GEO_NAME")!=string::npos) { // subkeyword found
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      geo_name = std::string(line_buf);
      continue;
    }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { //OK9_4310
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      sscanf(line_buf, "%s %s", str1, str2);
      geo_type_name = std::string(str1);
      geo_name      = std::string(str2);
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
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      sscanf(line_buf, "%ld", &no_nodes);
      for(i=0;i<no_nodes;i++){
        tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
        sscanf(line_buf, "%ld %lf %lf %lf", &ibuff, &x, &y, &z);
        newNode = new CNode(ibuff,x,y,z);
        nod_vector.push_back(newNode);
      }
      continue;
    }
    //....................................................................
    if(line_string.find("$ELEMENTS")!=string::npos) { // subkeyword found
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      sscanf(line_buf, "%ld", &no_elements);
      for(i=0;i<no_elements;i++){
        newElem = new CElem(i);
        newElem->Read(tokenbuf);
        ele_type = newElem->geo_type ;//CC02/2006
        if(newElem->GetPatchIndex()>max_mmp_groups)
          max_mmp_groups = newElem->GetPatchIndex();
        ele_vector.push_back(newElem);
      }
      continue;
    }
    //....................................................................
    if(line_string.find("$LAYER")!=string::npos) { // subkeyword found
      tokenbuf->get_non_empty_line(line_buf, LINE_MAX);
      sscanf(line_buf, "%ld", &no_msh_layer);
      continue;
    }
    //....................................................................
  }
  //========================================================================

#ifdef BENCHMARKING
  read_timer.stop();
  std::cout << "Reading mesh took " << read_timer.time_s() << " s." << std::endl;
#endif
  return 0;
}
#endif


/**************************************************************************
FEMLib-Method: ConnectedElements2Node
Task:
Programing:
04/2007 WW Cut from Construct grid
**************************************************************************/
void CFEMesh::ConnectedElements2Node(bool quadratic)
{
   long i, j, e, ni;
   CElem* thisElem0=NULL;
//   CElem* thisElem=NULL;
   bool done = false;
   // Set neighbors of node
   for(e=0; e<(long)nod_vector.size(); e++)
     nod_vector[e]->connected_elements.clear();
   for(e=0; e<(long)ele_vector.size(); e++)
   {
      thisElem0 = ele_vector[e];
      if(!thisElem0->GetMark()) continue;
      for(i=0; i<thisElem0->GetNodesNumber(quadratic); i++)
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
01/2010 NW Changed to determine the coordinate system by domain dimensions
**************************************************************************/
void CFEMesh::ConstructGrid()
{
   int counter;
   int i, j, k, ii, jj, m0, m, n0, n;
   int nnodes0, nedges0, nedges;
   long e, ei, ee,  e_size,  e_size_l;
   bool done;
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
//------------neighbor of 1D line
	   if(thisElem0->geo_type==1) //YD
       {
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
              Edges0[i]->SetOrder(false);
              e_edgeNodes0[0] = e_nodes0[edgeIndex_loc0[0]];
              e_edgeNodes0[1] = e_nodes0[edgeIndex_loc0[1]];
              e_edgeNodes0[2] = NULL;
             Edges0[i]->SetNodes(e_edgeNodes0);
             edge_vector.push_back(Edges0[i]);
          } // new edges
   	  } //  for(i=0; i<nedges0; i++)
      //
      // Set edges and nodes
      thisElem0->SetOrder(false);
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

   // check if this mesh includes multi-dimensional elements
   if (max_ele_dim==2 && msh_no_line>0) { //NW
     this->has_multi_dim_ele = true;
   } else if (max_ele_dim==3 && (msh_no_quad+msh_no_tris+msh_no_line)>0) {
     this->has_multi_dim_ele = true;
   }

   //----------------------------------------------------------------------
   // Node information
   // 1. Default node index <---> eqs index relationship
   // 2. Coordiate system flag
   x_sum=0.0;
   y_sum=0.0;
   z_sum=0.0;
   Eqs2Global_NodeIndex.clear();
   double xyz_max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX}; //NW
   double xyz_min[3] = {DBL_MAX,DBL_MAX,DBL_MAX}; //NW

   for(e=0; e<(long)nod_vector.size(); e++)
   {
	   nod_vector[e]->SetEquationIndex(e);
	   Eqs2Global_NodeIndex.push_back(nod_vector[e]->GetIndex());
	   x_sum += fabs(nod_vector[e]->X());
	   y_sum += fabs(nod_vector[e]->Y());
	   z_sum += fabs(nod_vector[e]->Z());
	   if (nod_vector[e]->X() > xyz_max[0]) xyz_max[0] =  nod_vector[e]->X();
	   if (nod_vector[e]->Y() > xyz_max[1]) xyz_max[1] =  nod_vector[e]->Y();
	   if (nod_vector[e]->Z() > xyz_max[2]) xyz_max[2] =  nod_vector[e]->Z();
	   if (nod_vector[e]->X() < xyz_min[0]) xyz_min[0] =  nod_vector[e]->X();
	   if (nod_vector[e]->Y() < xyz_min[1]) xyz_min[1] =  nod_vector[e]->Y();
	   if (nod_vector[e]->Z() < xyz_min[2]) xyz_min[2] =  nod_vector[e]->Z();
   }
   double xyz_dim[3]; //NW
   xyz_dim[0] = xyz_max[0] - xyz_min[0];
   xyz_dim[1] = xyz_max[1] - xyz_min[1];
   xyz_dim[2] = xyz_max[2] - xyz_min[2];

  //check dimension of the domain to select appropriate coordinate system
  if(xyz_dim[0]>0.0&&xyz_dim[1]<MKleinsteZahl&&xyz_dim[2]<MKleinsteZahl)
	 coordinate_system = 10;
  else if(xyz_dim[1]>0.0&&xyz_dim[0]<MKleinsteZahl&&xyz_dim[2]<MKleinsteZahl)
	 coordinate_system = 11;
  else if(xyz_dim[2]>0.0&&xyz_dim[0]<MKleinsteZahl&&xyz_dim[1]<MKleinsteZahl)
	 coordinate_system = 12;
  else if(xyz_dim[0]>0.0&&xyz_dim[1]>0.0&&xyz_dim[2]<MKleinsteZahl)
	 coordinate_system = 21;
  else if(xyz_dim[0]>0.0&&xyz_dim[2]>0.0&&xyz_dim[1]<MKleinsteZahl)
	 coordinate_system = 22;
  else if(xyz_dim[0]>0.0&&xyz_dim[1]>0.0&&xyz_dim[2]>0.0)
	 coordinate_system = 32;

   // 1D in 2D
   if(msh_no_line>0)
   {
     if(xyz_dim[0]>0.0&&xyz_dim[1]>0.0&&xyz_dim[2]<MKleinsteZahl)
        coordinate_system = 32;
     if(xyz_dim[0]>0.0&&xyz_dim[2]>0.0&&xyz_dim[1]<MKleinsteZahl)
        coordinate_system = 32;
   }

   max_dim = coordinate_system/10-1;
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

//TEST WW
   // For sparse matrix
   ConnectedNodes(false);
   //
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
FEMLib-Method: GenerateHighOrderNodes()
Task:
Programing:
07/2007 WW Implementation
01/2010 NW Case: a mesh with line elements
**************************************************************************/
void CFEMesh::GenerateHighOrderNodes()
{
   int i, j, k, ii;
   int nnodes0, nedges0, nedges;
   long e, ei, ee,  e_size,  e_size_l;
   int edgeIndex_loc0[2];
   bool done;
   double x0=0.0,y0=0.0,z0=0.0; //OK411

   // Set neighbors of node. All elements, even in deactivated subdomains, are taken into account here.
   for(e=0; e<(long)nod_vector.size(); e++)
     nod_vector[e]->connected_elements.clear();
   done = false;
   for(e=0; e<(long)ele_vector.size(); e++)
   {
      CElem *thisElem0 = ele_vector[e];
      for(i=0; i<thisElem0->GetNodesNumber(false); i++)
      {
          done = false;
          long ni = thisElem0->GetNodeIndex(i);
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
   //
   CNode *aNode=NULL;
   vec<CNode*> e_nodes0(20);
   vec<CNode*> e_nodes(20);
   CElem *thisElem0=NULL;
   CElem *thisElem=NULL;
   CEdge *thisEdge0=NULL;
   CEdge *thisEdge=NULL;
   //----------------------------------------------------------------------
   // Loop over elements (except for line elements)
   e_size = (long)ele_vector.size();
   for(e=0; e<e_size; e++)
   {
      thisElem0 = ele_vector[e];
      if (thisElem0->GetElementType()==1) continue; //NW

      nnodes0 = thisElem0->nnodes; // Number of nodes for linear element
//      thisElem0->GetNodeIndeces(node_index_glb0);
      for(i=0; i<nnodes0; i++) // Nodes
        e_nodes0[i] = thisElem0->GetNode(i);
      // --------------------------------
      // Edges
      nedges0 = thisElem0->GetEdgesNumber();
      // Check if there is any neighbor that has new middle points
      for(i=0; i<nedges0; i++)
      {
          thisEdge0 = thisElem0->GetEdge(i);
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
                nedges = thisElem->GetEdgesNumber();
                // Edges of neighbors
                for(ii=0; ii<nedges; ii++)
                {
                   thisEdge = thisElem->GetEdge(ii);
                   if(*thisEdge0==*thisEdge)
                   {
                      aNode = thisEdge->GetNode(2);
                      if(aNode) // The middle point exist
                      {
                         e_nodes0[nnodes0] = aNode;
                         nnodes0++;
                         done = true;
                         break;
                      }
                   }
                } //  for(ii=0; ii<nedges; ii++)
                if(done) break;
             } // for(ei=0; ei<e_size_l; ei++)
             if(done) break;
          }//for(k=0;k<2;k++)
         if(!done)
         {
            aNode = new CNode((long)nod_vector.size());
            aNode->SetX(0.5*(thisEdge0->GetNode(0)->X()+thisEdge0->GetNode(1)->X()));
            aNode->SetY(0.5*(thisEdge0->GetNode(0)->Y()+thisEdge0->GetNode(1)->Y()));
            aNode->SetZ(0.5*(thisEdge0->GetNode(0)->Z()+thisEdge0->GetNode(1)->Z()));
            e_nodes0[nnodes0] = aNode;
            thisEdge0->SetNode(2, aNode);
            nnodes0++;
            nod_vector.push_back(aNode);
         }
 	    } //  for(i=0; i<nedges0; i++)

      // No neighors or no neighbor has new middle point
      //
      if(thisElem0->GetElementType()==2) // Quadrilateral
      {
         x0=y0=z0=0.0;
         aNode = new CNode((long)nod_vector.size());
         e_nodes0[nnodes0] = aNode;
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
         aNode->SetX(x0);
         aNode->SetY(y0);
         aNode->SetZ(z0);
         nod_vector.push_back(aNode);
      }
      // Set edges and nodes
      thisElem0->SetOrder(true);
      // Resize is true
      thisElem0->SetNodes(e_nodes0, true);
   }// Over elements

   // Setup 1d line elements at the end
   if (msh_no_line > 0) {
     for (e=0; e<e_size; e++) {
        thisElem0 = ele_vector[e];
        if (thisElem0->GetElementType()!=1) continue;

        nnodes0 = thisElem0->nnodes;
        for(i=0; i<nnodes0; i++)
          e_nodes0[i] = thisElem0->GetNode(i);

        done = false;

        for (i=0; i<thisElem0->GetFacesNumber(); i++) {
          thisElem = thisElem0->GetNeighbor(i);
          // look for adjacent solid elements
          if (thisElem->GetElementType()==1) continue;

          for(j=0; j<thisElem->nnodes; j++)
            e_nodes[j] = thisElem->GetNode(j);
          nedges = thisElem->GetEdgesNumber();
          // search a edge connecting to this line element
          for(j=0; j<nedges; j++)
          {
              thisEdge = thisElem->GetEdge(j);
              thisElem->GetLocalIndicesOfEdgeNodes(j, edgeIndex_loc0);
              // Check neighbors
              for(k=0;k<2;k++)
              {
                 //OK411 CNode *tmp_nod = e_nodes[edgeIndex_loc0[k]];
                 e_size_l = (long)e_nodes[edgeIndex_loc0[k]]->connected_elements.size();
                 for(ei=0; ei<e_size_l; ei++)
                 {
                    ee = e_nodes[edgeIndex_loc0[k]]->connected_elements[ei];
                    if(ele_vector[ee]!=thisElem0) continue;
                    //the edge is found now
                    aNode = thisEdge->GetNode(2);
                    if(aNode) // The middle point exist
                    {
                       e_nodes0[nnodes0] = aNode;
                       nnodes0++;
                       done = true;
                       break;
                    }
                    if(done) break;
                 } // for(ei=0; ei<e_size_l; ei++)
                 if(done) break;
              }//for(k=0;k<2;k++)
              if(done) break;
          } //  for(i=0; i<nedges0; i++)
          if(done) break;
        }
         if(!done)
         {
            aNode = new CNode((long)nod_vector.size());
            for(i=0; i<nnodes0; i++) // Nodes
            {
              x0 += e_nodes0[i]->X();
              y0 += e_nodes0[i]->Y();
              z0 += e_nodes0[i]->Z();
            }
            x0 /= (double)nnodes0;
            y0 /= (double)nnodes0;
            z0 /= (double)nnodes0;
            aNode->SetX(x0);
            aNode->SetY(y0);
            aNode->SetZ(z0);
            e_nodes0[nnodes0] = aNode;
            nnodes0++;
            nod_vector.push_back(aNode);
         }
        thisElem0->SetOrder(true);
        thisElem0->SetNodes(e_nodes0, true);
      }
   }
   //
   NodesNumber_Quadratic= (long)nod_vector.size();
   for(e=NodesNumber_Linear; e<NodesNumber_Quadratic; e++)
   {
	   nod_vector[e]->SetEquationIndex(e);
	   Eqs2Global_NodeIndex.push_back(nod_vector[e]->GetIndex());
   }
   for(e=0; e<e_size; e++)
   {
      thisElem0 = ele_vector[e];
      for(i=thisElem0->nnodes; i<thisElem0->nnodesHQ; i++)
      {
         done = false;
         aNode = thisElem0->GetNode(i);
         for(k=0; k<(int)aNode->connected_elements.size(); k++)
         {
            if(e==aNode->connected_elements[k])
            {
               done = true;
               break;
            }
         }
         if(!done)
            aNode->connected_elements.push_back(e);
     }
  }

   // For sparse matrix
   ConnectedNodes(true);
   ConnectedElements2Node(true);
   //
   e_nodes0.resize(0);

   // Test	WW
   /*
   fstream n_out;
   n_out.open("node.txt", ios::out );
   for(e=0; e<NodesNumber_Quadratic; e++)
					nod_vector[e]->Write(n_out);
   n_out.close();
		 */
}

/**************************************************************************
FEMLib-Method:
Task:  Renumbering nodes corresponding to the activiate of elements
Programing:
09/2005 WW Implementation
05/2007 WW 1D in 2D
**************************************************************************/
void CFEMesh::FillTransformMatrix()
{
   CElem* elem = NULL;
   CRFProcess* m_pcs = PCSGet("FLUID_MOMENTUM");	// PCH
   //
   if((msh_no_hexs+msh_no_tets+msh_no_pris)==(long)ele_vector.size())
       return;
   else if(coordinate_system!=32 && !this->has_multi_dim_ele)
   {
	  if(m_pcs) ;	// Need to do FillTransformMatrix	// PCH
	  else
		return;
   }
   bool tilted = false;
   if(coordinate_system==32||coordinate_system==21||coordinate_system==22)
     tilted = true;
   if(!tilted)
      return;
   for (long i = 0; i < (long)ele_vector.size(); i++)
   {
      elem = ele_vector[i];
      if (elem->GetMark()) // Marked for use
	  {
         if(coordinate_system==21||coordinate_system==22)
         {
            if(elem->GetElementType()==1)
               elem->FillTransformMatrix();
         }
         else
         {
           if(  elem->GetElementType()==1
              ||elem->GetElementType()==2
              ||elem->GetElementType()==4 )
             elem->FillTransformMatrix();
         }
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
    *fem_msh_file << " $GEO_TYPE" << endl;
    *fem_msh_file << "  ";
    *fem_msh_file << geo_type_name << " " << geo_name << endl; //OK10_4310
  }
  //--------------------------------------------------------------------
  // NODES
  *fem_msh_file << " $NODES" << endl;
  *fem_msh_file << "  ";
  *fem_msh_file << GetNodesNumber(false) << endl; //WW
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
	double dist, distmin;
	double pnt[3];
	double nod[3];
	long number;
	pnt[0] = m_pnt->x;
	pnt[1] = m_pnt->y;
	pnt[2] = m_pnt->z;
	number = -1;
	distmin = 1.e+300;
	for (long i = 0; i < NodesInUsage(); i++) {
		nod[0] = nod_vector[i]->X();
		nod[1] = nod_vector[i]->Y();
		nod[2] = nod_vector[i]->Z();
		dist = EuklVek3dDist(pnt, nod);
		if (dist < distmin) {
			distmin = dist;
			number = i;
		}
	}
	return number;
}

/**************************************************************************
FEMLib-Method:
Task: Ermittelt den nahliegenden existierenden Knoten
Programing:
03/2010 TF implementation based on long CFEMesh::GetNODOnPNT(CGLPoint*m_pnt)
		by OK, WW
**************************************************************************/
long CFEMesh::GetNODOnPNT(const GEOLIB::Point* const pnt)
{
	double sqr_dist (0.0), distmin (std::numeric_limits<double>::max());
	long number (-1);

	for (long i=0; i<NodesInUsage(); i++) {
		sqr_dist += (nod_vector[i]->X() - (*pnt)[0]) * (nod_vector[i]->X() - (*pnt)[0]);
		sqr_dist += (nod_vector[i]->Y() - (*pnt)[1]) * (nod_vector[i]->Y() - (*pnt)[1]);
		sqr_dist += (nod_vector[i]->Z() - (*pnt)[2]) * (nod_vector[i]->Z() - (*pnt)[2]);
		if (sqr_dist < distmin) {
			distmin = sqr_dist;
			number = i;
		}
	}
	return number;
}

/**************************************************************************
FEMLib-Method:
Task: Ermittelt das nahliegende Element
Programing:
03/2010 TF implementation based on long CFEMesh::GetNODOnPNT(CGLPoint*m_pnt)
		by MB
**************************************************************************/
long CFEMesh::GetNearestELEOnPNT(const GEOLIB::Point* const pnt)
{
	long nextele (-1);
	double ex, ey, ez, dist (std::numeric_limits<double>::max()), dist1;
	double x((*pnt)[0]), y((*pnt)[1]), z((*pnt)[2]);
	double* center(NULL);

	for (size_t i=0; i<ele_vector.size(); i++) {
		center = ele_vector[i]->GetGravityCenter();
		ex = center[0];
		ey = center[1];
		ez = center[2];
		dist1 = (ex-x) * (ex-x) + (ey-y) * (ey-y) + (ez-z) * (ez-z);
		if (dist1 < dist) {
			dist = dist1;
			nextele = i;
		}
	}
	return nextele;
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
  //----------------------------------------------------------------------
#ifdef MSH_CHECK
  cout << "MSH nodes at polyline:" << endl;
  for(j=0;j<(int)msh_nod_vector.size();j++)
  {
    cout << msh_nod_vector [j] << endl;
  }
#endif
  //----------------------------------------------------------------------
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
       /*   if(gsp_vector.size() > 0){
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
          }*/
          //TK
//CC----------------------------------------begin
	GetNODOnSFC_PLY(m_sfc,msh_nod_vector);
	break;
//CC----------------------------------------end
      #else
        GetNODOnSFC_PLY(m_sfc,msh_nod_vector);
      #endif
      break;
    //....................................................................
    case 1: // TIN
      if(!m_sfc->TIN){
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
    //....................................................................
    case 4: // layer polyline, all z
      GetNODOnSFC_PLY_Z(m_sfc,msh_nod_vector);
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
03/2009 WW Case only search the specified nodes given in msh_nod_vector
03/2009 WW Efficiency improvement
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_PLY_XY(Surface*m_sfc,vector<long>&msh_nod_vector,
                                 bool givenNodesOnSurface)
{
  long i,j,k;
  int nPointsPly = 0;
  double gC[3],p1[3],p2[3];
  double Area1, Area2;
  double Tol = m_sfc->epsilon;
  CGLPolyline* m_ply = NULL;
  vector<CGLPolyline*>::iterator p_ply;//CC
  // Init
  //----19.03.2009. WW
  CNode *a_node = NULL;
  vector<long> temp_v;
  //
  if(givenNodesOnSurface)
  {
     temp_v.resize((long)msh_nod_vector.size());
     temp_v = msh_nod_vector;
  }
  p1[2] = p2[2] = 0.;
  //----19.03.2009. WW
  //
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
    }
    for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;
    //....................................................................
    // Area of this polygon by the grativity center
    Area1 = 0.0;
    for(i=0; i<nPointsPly; i++)
    {
      p1[0] = m_ply->point_vector[i]->x;
      p1[1] = m_ply->point_vector[i]->y;
      k = i+1;
      if(i==nPointsPly-1)
        k = 0;
      p2[0] = m_ply->point_vector[k]->x;
      p2[1] = m_ply->point_vector[k]->y;
      Area1 += fabs(ComputeDetTri(p1, gC, p2));
    }
    //....................................................................
    // Check nodes by comparing area
    //------- 19.03.2009. WW ----------------
    if(givenNodesOnSurface)
    {
      for(j=0;j<(long)temp_v.size();j++)
      {
        Area2 = 0.0;
        a_node = nod_vector[temp_v[j]];
        gC[0] = a_node->X();
        gC[1] = a_node->Y();
        for(i=0; i<nPointsPly; i++)
        {
          p1[0] = m_ply->point_vector[i]->x;
          p1[1] = m_ply->point_vector[i]->y;
          k = i+1;
          if(i==nPointsPly-1)
            k = 0;
          p2[0] = m_ply->point_vector[k]->x;
          p2[1] = m_ply->point_vector[k]->y;
          Area2 += fabs(ComputeDetTri (p1, gC, p2));
        }
        if(fabs(Area1-Area2)<Tol)
          msh_nod_vector.push_back(a_node->GetIndex());
      }
    }
    //-----------------------------------------------
    else
    {
      for(j=0;j<NodesInUsage();j++)
      {
        Area2 = 0.0;
        a_node = nod_vector[j]; //19.03.2009. WW

        gC[0] = a_node->X();
        gC[1] = a_node->Y();
        for(i=0; i<nPointsPly; i++)
        {
          p1[0] = m_ply->point_vector[i]->x;
          p1[1] = m_ply->point_vector[i]->y;
          k = i+1;
          if(i==nPointsPly-1)
            k = 0;
          p2[0] = m_ply->point_vector[k]->x;
          p2[1] = m_ply->point_vector[k]->y;
          Area2 += fabs(ComputeDetTri (p1, gC, p2));
        }
        if(fabs(Area1-Area2)<Tol)
          msh_nod_vector.push_back(a_node->GetIndex());
      }
    }
    p_ply++;
  }
  //
  if(givenNodesOnSurface) //19.03.2009. WW
     temp_v.clear();
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
01/2010 NW use epsilon specified in GEO as tolerance
last modification:
**************************************************************************/
void CFEMesh::GetNODOnSFC_TIN(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  long i=0,k=0,m=0;
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

  tolerance = m_sfc->epsilon; //NW
   // NW commented out below. Minimum edge length doesn't work for some cases
   ////Loop over all edges
   //     for(i=0;i<(long)edge_vector.size();i++)
   //     {
   //         if (j==0 && i==0){
   //           min_mesh_dist = edge_vector[i]->Length();
   //         }
   //         else{
   //           if (min_mesh_dist  > edge_vector[i]->Length())
   //               min_mesh_dist =  edge_vector[i]->Length();
   //         }
   //     }
   //     tolerance = min_mesh_dist;
    //Loop over all mesh nodes
        for(i=0;i<NodesInUsage();i++) //NW cannot use nod_vector.size() because of higher order elements
        {
            checkpoint[0] = nod_vector[i]->X();
            checkpoint[1] = nod_vector[i]->Y();
            checkpoint[2] = nod_vector[i]->Z();
            node = new CNode(i,checkpoint[0],checkpoint[1],checkpoint[2]);
            if((checkpoint[0]>=sfc_min[0]-tolerance && checkpoint[0]<=sfc_max[0]+tolerance )&&
               (checkpoint[1]>=sfc_min[1]-tolerance && checkpoint[1]<=sfc_max[1]+tolerance )&&
               (checkpoint[2]>=sfc_min[2]-tolerance && checkpoint[2]<=sfc_max[2]+tolerance ) )
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
  //m_sfc->epsilon = 1.0e-6;
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
  for(i=0;i<GetNodesNumber(false);i++) //WW
  {
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
void CFEMesh::ConnectedNodes(bool quadratic)
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
      for(l=0;l<m_ele->GetNodesNumber(quadratic);l++){
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
   if((long)face_normal.size()>0) return; //WW
//------------------------
   for (i = 0; i < (long)face_vector.size(); i++)
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
//    x1 = x0 + m_tri_ele->normal_vector[0]*dl;
//    y1 = y0 + m_tri_ele->normal_vector[1]*dl;
//    z1 = z0 + m_tri_ele->normal_vector[2]*dl;
	x1 = x0 + (*m_tri_ele->tranform_tensor)(2,0)*dl; //WW
    y1 = y0 + (*m_tri_ele->tranform_tensor)(2,1)*dl; //WW
    z1 = z0 + (*m_tri_ele->tranform_tensor)(2,2)*dl; //WW
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
    MSHWrite(m_gsp->path + "test");
  else
    MSHWrite("test");
}

/**************************************************************************
MSHLib-Method:
Programing:
03/2006 OK Implementation
**************************************************************************/
void CFEMesh::SetELENormalVectors()
{
  FillTransformMatrix();  //WW
  /*
  //----------------------------------------------------------------------
  long i;
  double v1[3],v2[3], v3[3];
  double patch_area;
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
  */
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
  int j, k;
  double v3[3], nr1[3], nr2[3];
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
    for(k=0; k<3; k++)
       nr1[k] = (*m_ele->tranform_tensor)(2,k);
    //....................................................................
    // Compare element normal vectors
    for(j=1;j<(int)m_nod->connected_elements.size();j++)
    {
      e = m_nod->connected_elements[j];
      m_ele1 = ele_vector[e];
      for(k=0; k<3; k++)
        nr2[k] = (*m_ele1->tranform_tensor)(2,k);
      CrossProduction(nr1,nr2,v3);
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


/**************************************************************************
MSHLib-Method:
Programing:
04/2006 OK Implementation
**************************************************************************/
void CFEMesh::CreateLineELEFromTriELE()
{
  int j,k;
  long i;
  double x,y,z;
  double x0,y0,z0;
  double x1,y1,z1;
  double dl;
  double* gravity_center;
  CNode* m_nod_line = NULL;
  CElem* m_tri_ele = NULL;
  CElem* m_ele = NULL;
  //----------------------------------------------------------------------
  // 1 - Element normal vector (for 2D elements only)
  SetELENormalVectors();
  //----------------------------------------------------------------------
  // 2 - Create MSH
  MSHDelete("LINE_from_TRI");
  CFEMesh* m_msh_line = NULL;
  m_msh_line = new CFEMesh();
  m_msh_line->pcs_name = "LINE_from_TRI";
  m_msh_line->ele_type = 1;
  m_msh_line->no_msh_layer = 20; // User-defined
  double element_length = -0.05; // User-defined
  dl = element_length * m_msh_line->no_msh_layer;
  //----------------------------------------------------------------------
  // 3.1 - Line nodes
  for(i=0;i<(long)ele_vector.size();i++)
  {
    m_tri_ele = ele_vector[i];
    //....................................................................
    // Element normal vector
    gravity_center = m_tri_ele->GetGravityCenter();
    x0 = gravity_center[0];
    y0 = gravity_center[1];
    z0 = gravity_center[2];
//    x1 = x0 + m_tri_ele->normal_vector[0]*dl;
//    y1 = y0 + m_tri_ele->normal_vector[1]*dl;
//    z1 = z0 + m_tri_ele->normal_vector[2]*dl;

    x1 = x0 + (*m_tri_ele->tranform_tensor)(2,0)*dl; //WW
    y1 = y0 + (*m_tri_ele->tranform_tensor)(2,1)*dl; //WW
    z1 = z0 + (*m_tri_ele->tranform_tensor)(2,2)*dl; //WW

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
  // 3.2 - Line elements
  long i_count = 0;
  for(i=0;i<(long)ele_vector.size();i++)
  {
    m_tri_ele = ele_vector[i];
    //....................................................................
    // Line elements
    for(j=0;j<m_msh_line->no_msh_layer;j++)
    {
      m_ele = new Mesh_Group::CElem;
      m_ele->SetIndex((long)m_msh_line->ele_vector.size());
      m_ele->SetElementType(1);
      m_ele->nnodes = 2;
      m_ele->SetPatchIndex((int)mmp_vector.size()); //OK4310
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
    MSHWrite(m_gsp->path + "test");
  else
    MSHWrite("test");
}

/**************************************************************************
MSHLib-Method:
08/2006 OK Implementation
**************************************************************************/
void CFEMesh::GetELEOnPLY(CGLPolyline*m_ply,vector<long>&ele_vector_ply)
{
#ifdef MSH_CHECK
  cout << "CFEMesh::GetELEOnPLY" << endl;
#endif
  long i;
  int j;
  int k;
  CElem* m_ele = NULL;
  CEdge* m_edg = NULL;
//WW  CNode* m_nod = NULL;
  vec<CEdge*> ele_edges_vector(15);
  vector<long>nodes_vector_ply;
  vec<long>ele_nodes(8);
  //int edge_node_numbers[2];
  vec<CNode*>edge_nodes(3);
  long edge_node_0,edge_node_1;
  long nn;
  //----------------------------------------------------------------------
  GetNODOnPLY(m_ply,nodes_vector_ply);
  //----------------------------------------------------------------------
#ifdef MSH_CHECK
  cout << "Elements at polyline: " << endl;
#endif
  switch(m_ply->type)
  {
    //....................................................................
    case 0: // PNT-TOP
    //....................................................................
    case 2: // PNT-TOP CC!!!
      //..................................................................
      // All elements having 2 points in common with m_ply
/*
      for(i=0;i<(long)ele_vector.size();i++)
      {
        m_ele = ele_vector[i];
        m_ele->selected = 0;
        m_ele->GetNodeIndeces(ele_nodes);
        for(j=0;j<(int)m_ele->GetNodesNumber(false);j++)
        {
          for(k=0;k<(long)nodes_vector_ply.size();k++)
          {
            if(ele_nodes[j]==nodes_vector_ply[k])
            {
              m_ele->selected++;
            }
          }
        }
        if(m_ele->selected==2)
          m_ele->SetMark(true);
      }
*/
      //..................................................................
      // All elements having an edge in common with m_ply
      for(i=0;i<(long)ele_vector.size();i++)
      {
        m_ele = ele_vector[i];
        m_ele->SetMark(false);
        m_ele->selected = 0;
        m_ele->GetEdges(ele_edges_vector);
        for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
        {
          m_edg = ele_edges_vector[j];
          m_edg->SetMark(false);
        }
      }
      for(i=0;i<(long)ele_vector.size();i++)
      {
        m_ele = ele_vector[i];
        m_ele->SetMark(false);
        m_ele->GetEdges(ele_edges_vector);
        for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
        {
          m_edg = ele_edges_vector[j];
          //m_ele->GetLocalIndicesOfEdgeNodes(j,edge_node_numbers);
          m_edg->GetNodes(edge_nodes);
          m_ele->selected = 0;
          for(k=0;k<(long)nodes_vector_ply.size();k++)
          {
            nn = nodes_vector_ply[k];
            //if(edge_node_numbers[0]==nodes_vector_ply[k])
            edge_node_0 = edge_nodes[0]->GetIndex();
            if(edge_nodes[0]->GetIndex()==nodes_vector_ply[k])
              m_ele->selected++;
            //if(edge_node_numbers[1]==nodes_vector_ply[k])
            edge_node_1 = edge_nodes[1]->GetIndex();
            if(edge_nodes[1]->GetIndex()==nodes_vector_ply[k])
              m_ele->selected++;
          }
          if(m_ele->selected==2)
          {
            m_ele->SetMark(true);
            m_edg->SetMark(true);
          }
        }
      }
      break;
    //....................................................................
    case 1: // PLY-RAS
      break;
    default:
      cout << "Warning in CFEMesh::GetELEOnPLY: case not implemented" << endl;
  }
  //----------------------------------------------------------------------
  ele_vector_ply.clear();
  vec<long>node_indeces(8);
  for(i=0;i<(long)ele_vector.size();i++)
  {
    m_ele = ele_vector[i];
    m_ele->GetEdges(ele_edges_vector);
    if(m_ele->GetMark())
    {
      ele_vector_ply.push_back(m_ele->GetIndex());
#ifdef MSH_CHECK
      cout << "Element: " << m_ele->GetIndex() << ", Nodes:";
#endif
      m_ele->GetNodeIndeces(node_indeces);
#ifdef MSH_CHECK
      for(j=0;j<(int)m_ele->GetNodesNumber(false);j++)
      {
        cout << " " << node_indeces[j];
      }
#endif
    }
    for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
    {
      m_edg = ele_edges_vector[j];
      if(m_edg->GetMark())
      {
        m_edg->GetNodes(edge_nodes);
#ifdef MSH_CHECK
        cout << ", Edge nodes: " << edge_nodes[0]->GetIndex() << "," << edge_nodes[1]->GetIndex() << endl;
#endif
      }
    }
  }
  //----------------------------------------------------------------------
  nodes_vector_ply.clear();
}

/**************************************************************************
MSHLib-Method:
Programing:
05/2006 OK Implementation
08/2006 YD
**************************************************************************/
void CFEMesh::CreateLineELEFromSFC()
{
  int j,k,i_k;
  double x0,y0,z0;
  double z,dz;
  Surface* m_sfc = NULL;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  CColumn* m_col = NULL;
//  CGLLine* m_lin = NULL;
  CSoilProfile* m_prf = NULL;  //YD
  //======================================================================
  i_k=0;
  dz = -0.05;
  long i_count = 0;
  for(long i=0;i<(long)surface_vector.size();i++)
  {
    m_sfc = surface_vector[i];
    m_sfc->CalcCenterPoint();
    m_col = COLGet(m_sfc->name);
    if(!m_col)
      return;
    m_prf = profile_vector[m_sfc->profile_code-1];
    //--------------------------------------------------------------------
    // NOD
    x0 = m_sfc->center_point[0];
    y0 = m_sfc->center_point[1];
    z0 = m_sfc->center_point[2];
    for(j=0;j<no_msh_layer+1;j++)
    {
      z =  z0 + dz*(j);
      m_nod = new CNode((long)nod_vector.size(),x0,y0,z);
      nod_vector.push_back(m_nod);
    }
    //--------------------------------------------------------------------
    // ELE
    for(j=0;j<no_msh_layer;j++)
    {
      //..................................................................
      m_ele = new Mesh_Group::CElem;
      m_ele->SetIndex((long)ele_vector.size());
      m_ele->SetElementType(1);
      m_ele->nnodes = 2;
      m_ele->nodes_index.resize(m_ele->nnodes);
      m_ele->SetPatchIndex(-1);
      m_ele->gravity_center[0] = 0.0;
      m_ele->gravity_center[1] = 0.0;
      m_ele->gravity_center[2] = 0.0;
      //..................................................................
      // Line element nodes
      for(k=0;k<m_ele->nnodes;k++)
      {
       // if(k == 0) i_k=1;           //YD: Right habd rule
       // if(k == 1) i_k=0;
        m_ele->nodes_index[k] = i_count*no_msh_layer + j + i_k + i_count;
        m_ele->nodes[k] = nod_vector[m_ele->nodes_index[k]];
        m_ele->gravity_center[0] += nod_vector[m_ele->nodes_index[k]]->X()/m_ele->nnodes;
        m_ele->gravity_center[1] += nod_vector[m_ele->nodes_index[k]]->Y()/m_ele->nnodes;
        m_ele->gravity_center[2] += nod_vector[m_ele->nodes_index[k]]->Z()/m_ele->nnodes;
      }
      //..................................................................
      // MAT
/*
      for(k=0;k<(int)m_col->line_vector.size();k++)
      {
        m_lin = m_col->line_vector[k];
        if((abs(m_ele->gravity_center[2])>m_lin->m_point1->z)&&(abs(m_ele->gravity_center[2])<m_lin->m_point2->z))
        {
          m_ele->SetPatchIndex(m_lin->mat_group);
        }
      }
*/

      for(k=0;k<(int)m_prf->soil_layer_thickness.size()-1;k++)
      {
       if((abs(m_ele->gravity_center[2])> m_prf->soil_layer_thickness[k])&&(abs(m_ele->gravity_center[2])<m_prf->soil_layer_thickness[k+1]))
       m_ele->SetPatchIndex(m_prf->soil_type[k]);
      }
      //..................................................................
      ele_vector.push_back(m_ele);
      //..................................................................
    }
    i_count++;
    //--------------------------------------------------------------------
  }
}

#ifdef NEW_EQS   // 1.11.2007 WW
/**************************************************************************
MSHLib-Method:
Programing:
11/2007 WW Implementation
**************************************************************************/
void CFEMesh::CreateSparseTable()
{
  // Symmetry case is skipped.
  // 1. Sparse_graph_H for high order interpolation. Up to now, deformation
  if(NodesNumber_Linear!=NodesNumber_Quadratic)
    sparse_graph_H = new SparseTable(this, true);
  // 2. M coupled with other processes with linear element
  if(sparse_graph_H)
  {
     if((int)pcs_vector.size()>1)
      sparse_graph = new SparseTable(this, false);
  }
  // 3. For process with linear elements
  else
    sparse_graph = new SparseTable(this, false);


  //sparse_graph->Write();
  //  sparse_graph_H->Write();
  //
  //ofstream Dum("sparse.txt", ios::out);
  //sparse_graph_H->Write(Dum);
}
#endif

#ifdef MFC
/**************************************************************************
ROCKFLOW - Modul:
Aufgabe:
Moves Z-Values of nodes according to a specified surface in
filename.dat file. Works for regular grids only!!!
const char *dateiname     :   file name
const int NLayers         :   Number of layers in the
const int row             :   row number of the row to be mapped
Programmaenderungen:
10/2003 WW/MB Erste Version
04/2004 WW faster method
02/2005 CC Modification change the grid height read order
07/2005 OK MSH
09/2005 OK/CC Empty lines in DAT files
01/2009 WW Set as a member of CFEMesh
***************************************************************************/
void CFEMesh::LayerMapping(const char *dateiname, const int NLayers,\
            const int row, const int DataType, int integ,  int infil_integ)
{
  FILE *f;
  fpos_t pos;
  char *s;  /* gelesene Zeile */
  int i, j;
  int ncols = 0;
  int nrows = 0;
  int nx, ny;
  int NPoints = 0;
//  int count = 0;
  double x,y, z;
  double MinX = 1.0e+10;
  double MinY = 1.0e+10;
  //double MinZ = 1.0e+10;
  double MaxX = -1.0e+10;
  double MaxY = -1.0e+10;
  double dx =  1.0e+10;
  double dy =  1.0e+10;
  double xi =  0.0;
  double eta =  0.0;
  double locX[4];
  double locY[4];
  double locH0[4];
  double geo_tolerance = 1.0e-3;
  double* GridX =NULL;
  double* GridY =NULL;
  double* H0=NULL;
  double **H=NULL;

  double ome[4];
  long NNodesPerRow = 0;
  long NNodes = 0;
  char charbuff[41];
  long counter = 0; //OK
  string s_str; //OK/CC

  //

  //----------------------------------------------------------------------
  NNodes = (long)nod_vector.size();
  //----------------------------------------------------------------------
  // Open grid file
  if((f = fopen(dateiname,"rb"))==NULL)  {
    DisplayErrorMsg("Fehler: Cannot open .dat file. It may not exist !");
    abort();
  }
  s = (char *) Malloc(MAX_ZEILE);
  //----------------------------------------------------------------------
  switch(DataType)
  {
    //====================================================================
    case 1:
       /* Count number of points of the grid */
       if( fgetpos(f, &pos ) != 0 )  {
         perror( "fgetpos error" );
         abort();
       }
       else  {
         NPoints=-1;
         while(!feof(f))    {
           fgets(s,MAX_ZEILE,f);
           s_str = s; //OK/CC
           is_line_empty(&s_str);
           if(s_str.size()>2)
           NPoints++;
         }
       }
       rewind(f);
       fsetpos(f, &pos );
       /* Allocate memory for grid and the specified surface*/
       GridX = (double *) Malloc(NPoints * sizeof(double));
       GridY = (double *) Malloc(NPoints * sizeof(double));
       H0 = (double *) Malloc(NPoints * sizeof(double));
       /* Read surface data and determine the range*/
       for(i=0; i<NPoints; i++)   {
         if(fgets(s,MAX_ZEILE,f)==NULL)	 {
           //DisplayErrorMsg("Error: End of .grd file reached ");
           abort();
         }
         s_str = s; //OK/CC
         is_line_empty(&s_str);
         if(s_str.size()<=2){
           --i;
           continue; //OK/CC
         }
         if (sscanf(s," %lf %lf %lf  ", &(GridX[i]),&(GridY[i]),&(H0[i]))!=3) 	   {
           //DisplayErrorMsg("Error: More than one integal in .grd file!");
           abort();
         }
       }
	   for(i=0; i<NPoints; i++)   {
          // Determine the range of the grid
          if(GridX[i]>=MaxX) MaxX = GridX[i];
          if(GridY[i]>=MaxY) MaxY = GridY[i];
          if(GridX[i]<=MinX) MinX = GridX[i];
          if(GridY[i]<=MinY) MinY = GridY[i];
          if(i>0&&(GridX[i]-GridX[i-1]!=0))
          if(dx>=fabs(GridX[i]-GridX[i-1])) dx = fabs(GridX[i]-GridX[i-1]);
          if(i>0&&(GridY[i]-GridY[i-1]!=0))
	      if(dy>=fabs(GridY[i]-GridY[i-1])) dy = fabs(GridY[i]-GridY[i-1]);
       }
       //.................................................................
       if(dx>dy)
         geo_tolerance = dy*1e-3;
       else
         geo_tolerance = dx*1e-3;
       //.................................................................
	   // Copy the data of H0 to H[][] in order to enhance the computation
       ncols = (int)((MaxX-MinX)/dx)+1;
       nrows = (int)((MaxY-MinY)/dy)+1;
       H = new double*[nrows];
	   double dist;
	   for(i=0; i<nrows; i++)
         H[i] = new double[ncols];
       for(i=0; i<nrows; i++)
	   {
         y = MinY+i*dy;
         for(j=0; j<ncols; j++)
		 {
           x = MinX+j*dx;
           for(int k=0; k<NPoints; k++)
		   {
             dist = sqrt((x-GridX[k])*(x-GridX[k])+(y-GridY[k])*(y-GridY[k]));
			 if(dist<geo_tolerance)
			 {
               counter++; //OK
			   H[i][j] = H0[k];
			   break;
			 }
		   }
		 }
	   }
       if(counter<NPoints){
         cout << "Warning: not all grid points found, increase tolerance" << endl;
       }
	   break;
    //====================================================================
	 case 2:
       ncols = 0;
       nrows = 0;
	   double x0, y0, z0;
       fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d  ", charbuff, &ncols);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d ", charbuff, &nrows);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &x0);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &y0);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &dx);
	   dy = dx;
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &z0);
       MinX = x0;
       MaxX = x0+dx*ncols;
       MinY = y0;
       MaxY = y0+dy*nrows;
       // Allocate memory for grid and the specified surface
       H = new double*[nrows];
	   for(i=0; i<nrows; i++)  H[i] = new double[ncols];
	   // Compute the grid points:
	 /*  for(i=0; i<nrows; i++)
	   {
    	  for(j=0; j<ncols; j++)
		  {
             fscanf(f,"%lf", &H[i][j]);
		  }
	   } */
       //CC 02/2005
         // Compute the grid points:
	   for(i=nrows-1; i>=0; i--)
	   {
    	  for(j=0; j<ncols; j++)
		  {
             fscanf(f,"%lf", &H[i][j]);
		  }
	   }
	   //
       break;
  }
  //----------------------------------------------------------------------
  NNodesPerRow = NNodes / (NLayers+1);
  CNode *anode = NULL; //19.01.2009. WW
  /* 1. Compute the height of points to be attached*/
  for(i=0; i<NNodes; i++)
  {
    if(i >= (row-1) * NNodesPerRow  &&    i <= (row * NNodesPerRow) -1 )
    {
        anode = nod_vector[i];
        x = anode->X();
        y = anode->Y();
      //..................................................................
	  if(x<MinX||x>MaxX||y<MinY||y>MaxY)   {
        // Release memory
        if(GridX) GridX  = (double*) Free(GridX);
        if(GridY) GridY  = (double*) Free(GridY);
        if(H0) H0 = (double*) Free(H0);
        if(s) s = (char *)Free(s);
        return;
	  }
      //..................................................................
	  nx = (int)((x-MinX)/dx);
      ny = (int)((y-MinY)/dy);
      if(nx*dx+MinX>=x)  nx -= 1;
      if(ny*dy+MinY>=y)  ny -= 1;
	  if(nx>ncols) nx = ncols-2;
	  if(ny>nrows) ny = nrows-2;
      if(nx<0) nx = 0;
      if(ny<0) ny = 0;

	  locX[0] = MinX+nx*dx;
	  locY[0] = MinY+ny*dy;
	  locH0[0] = H[ny][nx];

	  locX[1] = MinX+(nx+1)*dx;
	  locY[1] = MinY+ny*dy;
	  locH0[1] = H[ny+1][nx];

	  locX[2] = MinX+(nx+1)*dx;
	  locY[2] = MinY+(ny+1)*dy;
	  locH0[2] = H[ny+1][nx+1];

	  locX[3] = MinX+nx*dx;
	  locY[3] = MinY+(ny+1)*dy;
	  locH0[3] = H[ny][nx+1];
      //------------YW/WW
      int nblack = 0;
      for(j=0; j<4; j++)
      {
         if(fabs(locH0[j]+9999)<1e-16)
           nblack++;
      }
      if(nblack==0)  //19.01.2009. WW
      {
         // Interpolate
         xi =  2.0*(x-0.5*(locX[0]+locX[1]))/dx;
         eta =  2.0*(y-0.5*(locY[1]+locY[2]))/dy;
         MPhi2D(ome, xi, eta);

         z=0.0;
         for(j=0; j<4; j++) z += ome[j]*locH0[j];
         anode->SetZ(z);   //19.01.2009. WW
         anode->SetMark(true);  //19.01.2009. WW
      }
      else    //19.01.2009. WW
      {
         if(row==1)
         {
            return;
         }
         if(row==NLayers+1)
         {
            return;
         }
         anode->SetZ(-9999);   //19.01.2009. WW
         anode->SetMark(false);  //19.01.2009. WW
         mapping_check = true;
      }
    }
  }
  //
/*
  CFileDialog dlg(FALSE, NULL, "new", OFN_ENABLESIZING ,
                 " Geometry Files (*.rfi)|*.rfi| All Files (*.*)|*.*||" );
  dlg.DoModal();
  CString m_filepath = dlg.GetPathName();
  //
  const char* cpsz = static_cast<LPCTSTR>(m_filepath);
  xxxxx = cpsz;
  DATWriteRFIFile(xxxxx);
*/
  // ------------------------------
  //----------------------------------------------------------------------
  // Release memory
  if(GridX) GridX  = (double*) Free(GridX);
  if(GridY) GridY  = (double*) Free(GridY);
  if(H0) H0 = (double*) Free(H0);
  if(s) s = (char *)Free(s);
  fclose(f);

  // -----------------22.01.2009 WW
  map_counter++;
  if(!mapping_check)
  {
     if((map_counter == NLayers+1)&&infil_integ)
       WriteCurve2RFD(map_counter, dateiname);
     return;
  }
  if(map_counter == NLayers+1)
  {
     LayerMapping_Check(dateiname, NLayers, integ);
     map_counter = 0;
  }

}

/**************************************************************************
GeoSys:
Programming:
03/2009 FS/WW
Modification:
***************************************************************************/
inline void CFEMesh::WriteCurve2RFD(const int NLayers, const char *dateiname)
{
  string path = dateiname;
  CNode *node_b;
  static double timec[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  //
  //string ofname = path+".rfd";
  string ofname = path+".fct";
  ofstream ofile_t(ofname.c_str(), ios::trunc|ios::out);
  ofile_t.setf(ios::scientific, ios::floatfield);
  setw(14);
  ofile_t.precision(14);

  //ofile_t<<"#PROJECT\n Produced by GUI"<<endl;
  ofile_t<<"GeoSys-FCT: Functions ------------------------------------------------"<<endl;
  long i, NNodesPerRow;
  int k;
  NNodesPerRow = (long)nod_vector.size() /NLayers;
  // double val = 0.;
  for(i=0; i<NNodesPerRow; i++)
  {
     //ofile_t<<"#CURVES  //"<<i<<endl;
     ofile_t<<"#FUNCTION\n $TYPE\n  fromCMBsPrecipEvapoCalculations\n $GEO_TYPE"<<endl;
     ofile_t<<"POINT"<<i<<endl;
     ofile_t<<"$VARIABLES\n  TIME POINT\n $DATA"<<endl;
     // Area val = n_area_val[i];
     for(k=0; k<NLayers; k++)
     {
        node_b = nod_vector[k*NNodesPerRow+i];
        ofile_t<< timec[k]<<"  "<<node_b->Z()/1000.0<<endl; //m/day
     }
  }
  ofile_t<<"#STOP"<<endl;
  ofile_t.close();
  delete [] n_area_val;
  n_area_val = NULL;
}
/**************************************************************************
GeoSys:
  Check and remove/adjsut singular elements: nodes overided
Argument:
const int NLayers         :   Number of layers in the

Programming:
01/2009 WW
01/2009 WW Surface integration
Modification:

***************************************************************************/
void CFEMesh::LayerMapping_Check(const char *dateiname, const int NLayers, int integ)
{
  CElem *elem;
  CNode *node_t;
  CNode *node_b;
  CNode *node_n;
  int k, j, l, m;
  long i, NNodesPerRow;
  int flat;
  int ov_idx[3];

  double ref_dep = -999999999.0;
  double tol = 1.e-16;
  //
  vector<long> nodes_2node_on_1st_srf;  //WW. 26.01.2009

  l = j = 0;
  NNodesPerRow = (long)nod_vector.size() / (NLayers+1);

  //18.02.2009 WW
  if(integ==1)
  {
    for(i=0; i<NNodesPerRow; i++)
    {
       for(k=0; k<NLayers+1; k++)
       {
          node_b = nod_vector[k*NNodesPerRow+i];
          if(k==0)
            node_b->SetBoundaryType('0');
          else if(k==NLayers)
            node_b->SetBoundaryType('1');
          else
            node_b->SetBoundaryType('I');
       }
    }
  }



  for(i=0; i<NNodesPerRow; i++)
  {
     flat = 0;
     for(k=0; k<NLayers-1; k++)
     {
        node_b = nod_vector[k*NNodesPerRow+i];
        node_t = nod_vector[(k+1)*NNodesPerRow+i];
        if(!node_t->GetMark())
        {

           if(k==0) //WW. 27.01.2009
           {
             nodes_2node_on_1st_srf.clear();
             for(j=0; j<(int)node_t->connected_nodes.size(); j++)
               nodes_2node_on_1st_srf.push_back(node_t->connected_nodes[j]);
           }

           node_t->SetZ(node_b->Z());
           node_t->connected_nodes.clear();
           for(l=k; l>=0; l--)  //WW/YW. 23.01.2009
           {
              node_n = nod_vector[l*NNodesPerRow+i];
              if(node_n->GetMark())
              {
                 node_t->connected_nodes.push_back(node_n->GetIndex());
                 break;
              }
           }
           flat++;
        }
     }
     //

//     if((flat==NLayers-1)&&(NLayers>3))
     //---- 27.01.2009. WW
     if(flat==NLayers-1)
     {

       node_b = nod_vector[NNodesPerRow+i];
       node_b->SetMark(true);
       node_b->connected_nodes.clear();
       for(j=0; j<(int)nodes_2node_on_1st_srf.size(); j++)
          node_b->connected_nodes.push_back(nodes_2node_on_1st_srf[j]);

       // Upper
       node_t = nod_vector[NLayers*NNodesPerRow+i];
       node_t->SetMark(false);
       node_b->SetZ(node_t->Z());
       node_b->SetBoundaryType('1');
       //
       for(k=1; k<NLayers; k++)
       {
          node_t = nod_vector[(k+1)*NNodesPerRow+i];
          node_t->SetZ(ref_dep);
          node_t->connected_nodes.clear();
          node_t->connected_nodes.push_back(node_b->GetIndex());
       }
     }

  }


  //----------------------------------------------------------------------
  //
  //
  vector<CElem*> new_elems;
  for(i=0; i<(long)ele_vector.size(); i++)
  {
     elem  = ele_vector[i];
     elem->SetMark(true);
     // All upper layres are overidered together.

     flat = 0;
     for(k=0; k<3;k++)
     {
        node_b = elem->GetNode(k);
        node_t = elem->GetNode(k+3);
        if(  fabs(node_b->Z()+ref_dep)<tol
           ||fabs(node_t->Z()+ref_dep)<tol)
        {
           flat = 1;
           elem->SetMark(false);
           break;
        }
     }
     if(flat==1)
       continue;

     // Check flat element
     // All top layer nodes are overided together with bottom nodes
     flat = 0;
     for(k=0; k<3;k++)
     {
        node_b = elem->GetNode(k);
        node_t = elem->GetNode(k+3);

        if(fabs(node_t->Z()-node_b->Z())<1.e-5)
        {
           ov_idx[flat] = k;
           flat++;       //WW/YW. 23.09.2009
        }
     }
     //
     switch(flat)
     {
        case 0:
          elem->SetMark(true);
          break;
        case 1:
          k = ov_idx[0];
          j = (k+2)%3;
          l = (k+1)%3;
          node_b = elem->GetNode(k);
          node_t = elem->GetNode(k+3);
          if(node_t->GetBoundaryType()=='1') //24.02.2009. WW
            node_b->SetBoundaryType('1');
         //-------- New ------------------------
          CElem *new_elem;
          new_elem = new CElem((long)ele_vector.size()+(long)new_elems.size());
          new_elems.push_back(new_elem);
          // Only for linear element
          new_elem->nodes.resize(4);
          new_elem->nodes_index.resize(4);
          new_elem->nodes[0] = node_b;
          new_elem->nodes_index[0] = node_b->GetIndex();
          new_elem->nodes[1] = elem->nodes[j+3];
          new_elem->nodes_index[1] = elem->nodes_index[j+3];
          new_elem->nodes[2] = elem->nodes[l+3];
          new_elem->nodes_index[2] = elem->nodes_index[l+3];
          new_elem->nodes[3] = elem->nodes[l];
          new_elem->nodes_index[3] = elem->nodes_index[l];

          new_elem->nnodes = 4;
          new_elem->nnodesHQ = 10;
          new_elem->ele_dim = 3;
          new_elem->geo_type = 5;
          new_elem->nfaces = 4;
          new_elem->nedges = 6;
          new_elem->mark = true;

          new_elem->patch_index =  elem->patch_index;
	      new_elem->quadratic = elem->quadratic;

          new_elem->boundary_type='I';
          // Initialize topological properties
          new_elem->neighbors.resize(new_elem->nfaces);
          for(m=0; m<new_elem->nfaces; m++)
            new_elem->neighbors[m] = NULL;
          new_elem->edges.resize(new_elem->nedges);
          new_elem->edges_orientation.resize(new_elem->nedges);
          for(m=0; m<new_elem->nedges; m++)
          {
            new_elem->edges[m] = NULL;
            new_elem->edges_orientation[m] = 1;
          }
          new_elem->area = -1.0;

          //-------- Old ------------------------
          elem->geo_type = 5;
          elem->mark = true;

          elem->nnodes = 4;
          elem->nnodesHQ = 10;
          elem->ele_dim = 3;
          elem->geo_type = 5;
          elem->nfaces = 4;
          elem->nedges = 6;
          elem->mark = true;
          // Initialize topological properties
          elem->neighbors.resize(elem->nfaces);
          for(m=0; m<elem->nfaces; m++)
            elem->neighbors[m] = NULL;
          elem->edges.resize(elem->nedges);
          elem->edges_orientation.resize(elem->nedges);
          for(m=0; m<elem->nedges; m++)
          {
            elem->edges[m] = NULL;
            elem->edges_orientation[m] = 1;
          }

          elem->nodes.resize(4);
          elem->nodes[0] = nod_vector[elem->nodes_index[j]];
          elem->nodes[1] = nod_vector[elem->nodes_index[k]];
          elem->nodes[2] = nod_vector[elem->nodes_index[l]];
          elem->nodes[3] = nod_vector[elem->nodes_index[j+3]];

          elem->nodes_index.resize(4);
          for(m=0; m<4; m++)
            elem->nodes_index[m] = elem->nodes[m]->GetIndex();

          break;
        case 2:
          k = ov_idx[0];
          j = (k+2)%3;
          l = (k+1)%3;
          if( ov_idx[1]==j)
            k = l;
          else if( ov_idx[1]==l)
            k = j;

           // 09.02. 2009. WW
          j = (k+2)%3;
          l = (k+1)%3;
          // 24.02.2009. WW
          node_b = elem->GetNode(j+3);
          node_t = elem->GetNode(l+3);

          //-------- Old ------------------------
          elem->geo_type = 5;
          elem->mark = true;

          elem->nnodes = 4;
          elem->nnodesHQ = 10;
          elem->ele_dim = 3;
          elem->geo_type = 5;
          elem->nfaces = 4;
          elem->nedges = 6;
          elem->mark = true;
          // Initialize topological properties
          elem->neighbors.resize(elem->nfaces);
          for(m=0; m<elem->nfaces; m++)
            elem->neighbors[m] = NULL;
          elem->edges.resize(elem->nedges);
          elem->edges_orientation.resize(elem->nedges);
          for(m=0; m<elem->nedges; m++)
          {
            elem->edges[m] = NULL;
            elem->edges_orientation[m] = 1;
          }

          elem->nodes.resize(4);
          elem->nodes[0] = nod_vector[elem->nodes_index[j]];
          elem->nodes[1] = nod_vector[elem->nodes_index[k]];
          elem->nodes[2] = nod_vector[elem->nodes_index[l]];
          elem->nodes[3] = nod_vector[elem->nodes_index[k+3]];

          //for j, l nodes if they becomes on top surface. 24.02.2009. WW
          if(node_b->GetBoundaryType()=='1')
            elem->nodes[0]->SetBoundaryType('1');
          if(node_t->GetBoundaryType()=='1')
            elem->nodes[2]->SetBoundaryType('1');


          elem->nodes_index.resize(4);
          for(m=0; m<4; m++)
            elem->nodes_index[m] = elem->nodes[m]->GetIndex();
          break;
        case 3:
          elem->SetMark(false);
          break;
     }
  }

  long counter;
  //
  for(i=0; i<(long)new_elems.size(); i++)
    ele_vector.push_back(new_elems[i]);
  std::vector<CElem*>::iterator beg_e = ele_vector.begin( ), last_e;
  counter = 0;
  while ( beg_e != ele_vector.end( ) )
  {
    last_e = beg_e++;
    elem = *last_e;
    if ( !(elem->GetMark()))
    {
       delete elem;
       beg_e= ele_vector.erase(last_e);
    }
    else
    {
       elem->SetIndex(counter);
       counter++;
       for(m=0; m<elem->nnodes; m++)
       {
          node_b =  elem->nodes[m];
          if(!node_b->GetMark())
          {
             node_t = nod_vector[node_b->connected_nodes[0]];
             elem->nodes[m] = node_t;
          }
       }
    }
  }
  counter = 0;
  std::vector<CNode*>::iterator beg = nod_vector.begin( ), last;
  while ( beg != nod_vector.end( ) )
  {
    last = beg++;
    node_b = *last;
    if ( !(node_b->GetMark()) )
    {
       delete  node_b;
       node_b = NULL;
       beg= nod_vector.erase(last);
    }
    else
    {
       node_b->SetIndex(counter);
       node_b->connected_elements.clear();
       node_b->connected_nodes.clear();
       counter++;
    }
  }
  //
  for(i=0; i<(long)ele_vector.size(); i++)
  {
    elem = ele_vector[i];
    for(m=0; m<elem->nnodes; m++)
      elem->nodes_index[m] = elem->nodes[m]->GetIndex();

    for(m=0; m<elem->nfaces; m++)
      elem->neighbors[m] = NULL;
    elem->edges.resize(elem->nedges);
    elem->edges_orientation.resize(elem->nedges);
    for(m=0; m<elem->nedges; m++)
    {
      elem->edges[m] = NULL;
      elem->edges_orientation[m] = 1;
    }
  }


  beg_e = ele_vector.begin( ), last_e;
  counter = 0;
  bool flatf = false;
  while ( beg_e != ele_vector.end( ) )
  {
    last_e = beg_e++;
    elem = *last_e;

    //10.02.2009. WW !!!!!!!!!!!!!!!!!!!!!!
    for(m=0; m<elem->nnodes; m++)
    {
      flatf = false;
      for(int mm=0; mm<elem->nnodes; mm++)
      {
         if(m==mm) continue;
         if(elem->GetNodeIndex(m)==elem->GetNodeIndex(mm))
         {
            flatf = true;
            break;
         }
      }
    }
    if(flatf)
    {
       delete elem;
       beg_e= ele_vector.erase(last_e);
       continue;
    }
    else
    {
       elem->SetIndex(counter);
       counter++;
       for(m=0; m<elem->nnodes; m++)
       {
          node_b =  elem->nodes[m];
          if(!node_b->GetMark())
          {
             node_t = nod_vector[node_b->connected_nodes[0]];
             elem->nodes[m] = node_t;
          }
       }
    }
  }


  ConnectedElements2Node();

  counter = 0;
  beg = nod_vector.begin( );
  while ( beg != nod_vector.end( ) )
  {
    last = beg++;
    node_b = *last;
    if (node_b->connected_elements.size()==0 )
    {
       delete  node_b;
       node_b = NULL;
       beg= nod_vector.erase(last);
    }
    else
    {
       node_b->SetIndex(counter);
       counter++;
    }
  }
  //
  for(i=0; i<(long)ele_vector.size(); i++)
  {
    elem = ele_vector[i];
    for(m=0; m<elem->nnodes; m++)
      elem->nodes_index[m] = elem->nodes[m]->GetIndex();
  }

  // 19.02.2009 WW
  while(edge_vector.size()>0)
  {
     delete edge_vector[edge_vector.size()-1];
     edge_vector.pop_back();
  }
  while(face_vector.size()>0)
  {
     delete face_vector[face_vector.size()-1];
     face_vector.pop_back();
  }
  ConstructGrid();

  // Surface integration
  //18-20.02.2009 WW
  if(integ==1)
  {
    string path = dateiname;
    basic_string <char>::size_type indexCh1a;
    indexCh1a = path.find_last_of("\\");
    if( indexCh1a < path.size())
    {
      string stra = path.substr (0, indexCh1a);
      path.clear();
      path = stra;
    }
    //
    CElem* elem = NULL;
    CElement* fem = new CElement(GetCoordinateFlag());


    string deli(" ");
    string ofname = path+"\\bottom_integrated_node_st.txt";
    ofstream ofile_b(ofname.c_str(), ios::trunc|ios::out);
    ofile_b.setf(ios::scientific, ios::floatfield);
    setw(14);
    ofile_b.precision(14);
    //
    ofname = path+"\\top_integrated_node_st.txt";
    ofstream ofile_t(ofname.c_str(), ios::trunc|ios::out);
    ofile_t.setf(ios::scientific, ios::floatfield);
    //
    ofname = path+"\\surface_mesh.msh";
    streamoff f_loc, fe_loc;
    ofstream ofile_m(ofname.c_str(), ios::trunc|ios::out);
    ofile_m.setf(ios::scientific, ios::floatfield);
    ofile_m<<"#FEM_MSH\n$PCS_TYPE\nNO_PCS\n$NODES"<<endl;
    f_loc = ofile_m.tellp();
    ofile_m<<"                    "<<endl;
    //
    ofile_t.precision(14);
    ofile_b.precision(14);
    ofile_m.precision(14);
    //
    long ctop=0;
    long cbottom=0;
    long nSize = 0;
    double nodesFVal[8];
    double *n_val;

    nSize = (long)nod_vector.size();
    n_val = new double[nSize];
    for(i=0; i<nSize; i++)
      n_val[i] = 0;

    // Top surface faces:
    vector<long> node_map(nSize);
    long s_node = 0;
    for(i=0; i<(long)nSize; i++)
    {
       node_b = nod_vector[i];
       node_map[i] = 0;
       if(node_b->GetBoundaryType()=='1')
       {
          node_map[i] = s_node;
          ofile_m<<s_node<<deli<<node_b->X()<<deli<<node_b->Y()<<deli<<node_b->Z()<<endl;
          s_node++;
       }
    }
    ofile_m<<"$ELEMENTS"<<endl; // Total nodes on top surface
    fe_loc = ofile_m.tellp();
    ofile_m<<"                       "<<endl;
    string e_type;
    long s_ele = 0;

    for(i=0; i<(long)face_vector.size(); i++)
    {
       elem = face_vector[i];
       ctop = cbottom = 0;
       for(k=0; k<elem->nnodes; k++)
       {
         node_b = elem->nodes[k];
         if(node_b->GetBoundaryType()=='0')
           cbottom++;
         else if(node_b->GetBoundaryType()=='1')
           ctop++;
       }
       if(cbottom == elem->nnodes)
       {
         for(k=0; k<elem->nnodes; k++)
           nodesFVal[k] = 1.0;
		 elem->ComputeVolume();
         elem->FillTransformMatrix();
         fem->setOrder(getOrder()+1);
		 fem->ConfigElement(elem);
		 fem->FaceIntegration(nodesFVal);
         for(k=0; k<elem->nnodes; k++)
         {
           node_b = elem->nodes[k];
           n_val[elem->nodes[k]->GetIndex()] += nodesFVal[k]*fabs(elem->getTransformTensor(8));
         }
         delete elem->tranform_tensor;
         elem->tranform_tensor = NULL;
       }
       else if (ctop == elem->nnodes)
       {
         for(k=0; k<elem->nnodes; k++)
           nodesFVal[k] = 1.0;
		 elem->ComputeVolume();
         elem->FillTransformMatrix();
         fem->setOrder(getOrder()+1);
		 fem->ConfigElement(elem);
		 fem->FaceIntegration(nodesFVal);
         // Dump to file
         e_type = "tri ";
         if(elem->nnodes==4)  e_type = "quad ";
         ofile_m<<s_ele<<" 0 "<<e_type;
         s_ele++;

         for(k=0; k<elem->nnodes; k++)
         {
            node_b = elem->nodes[k];
            // Dump to file
            ofile_m<<node_map[node_b->GetIndex()]<<deli;
            //
            n_val[node_b->GetIndex()] += nodesFVal[k]*fabs(elem->getTransformTensor(8));
         }
         // Dump to file
         ofile_m<<endl;
         delete elem->tranform_tensor;
         elem->tranform_tensor = NULL;
       }
    }

    ofile_m<<"#STOP"<<endl;
    // Rewind
    ofile_m.clear();
    ofile_m.seekp(f_loc);
    ofile_m<<s_node;
    ofile_m.seekp(fe_loc);
    ofile_m<<s_ele;
    ofile_m.close();

    for(i=0; i<nSize; i++)
    {
       node_b = nod_vector[i];
       if(node_b->GetBoundaryType()=='0')
          ofile_b<<node_b->GetIndex()<<deli<<n_val[i]<<endl;
       else if(node_b->GetBoundaryType()=='1')
          ofile_t<<node_b->GetIndex()<<deli<<n_val[i]<<endl;
       else
          continue;
    }
    ofile_b<<"#STOP"<<endl;
    ofile_t<<"#STOP"<<endl;
    ofile_b.close();
    ofile_t.close();
    delete [] n_val;
    delete fem;
    fem = NULL;
    n_val = NULL;
    //
  }
}
#endif
/**************************************************************************
MSHLib-Method:
Task: All nodes vertical to a polyline
02/2009 OK
**************************************************************************/
void CFEMesh::GetNODOnSFC_PLY_Z(Surface*m_sfc,vector<long>&msh_nod_vector)
{
  vector<CGLPolyline*>::iterator p_ply;
  CGLPolyline* m_ply = NULL;
  // .................................................................
  // nodes close to first polyline
  p_ply = m_sfc->polyline_of_surface_vector.begin();
  while(p_ply!=m_sfc->polyline_of_surface_vector.end())
  {
    m_ply = *p_ply;
    GetNODOnPLY_XY(m_ply,msh_nod_vector);
    break;
  }
}
//========================================================================
} // namespace Mesh_Group

//========================================================================
