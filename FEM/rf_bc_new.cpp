/**************************************************************************
FEMLib - Class: BC BoundaryConditions
Task: 
Programing:
02/2004 OK Implementation
last modified
**************************************************************************/
#include "stdafx.h" // MFC
#include "makros.h"
// C++ STL
#include <math.h>
#include <iostream>
using namespace std;
// GEOLib
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//include "geo_strings.h"
#include "geo_dom.h"
#include "rfstring.h"
// MSHLib
//#include "mshlib.h"
// FEMLib
extern void remove_white_space(string*);
#include "gs_project.h"
#include "nodes.h"
#include "tools.h"
#include "rf_node.h"
#include "rf_bc_new.h"
#include "rf_pcs.h"
#include "rf_fct.h"
// MathLib
#include "mathlib.h"

CBoundaryConditionNode::CBoundaryConditionNode()
{
  conditional = false;
//  funct_name = "";
}
//==========================================================================
list<CBoundaryCondition*>bc_list;
vector<string>bc_db_head;
list<CBoundaryConditionsGroup*>bc_group_list;
vector<CBoundaryCondition*>bc_db_vector;

/**************************************************************************
FEMLib-Method: 
Task: BC constructor
Programing:
01/2004 OK Implementation
**************************************************************************/
CBoundaryCondition::CBoundaryCondition(void)
{
  delimiter_type = " ";
  dis_type = -1;
  CurveIndex = -1;
  // FCT
  conditional = false;
  display_mode = false; //OK
  time_dep_interpol = false;
}

/**************************************************************************
FEMLib-Method: 
Task: BC deconstructor
Programing:
01/2004 OK Implementation
**************************************************************************/
CBoundaryCondition::~CBoundaryCondition(void) {
    // PCS
    pcs_type_name.clear();
    // GEO
    geo_type = -1;
    geo_type_name.clear();
    geo_name.clear();
    // DIS
    dis_type = -1;
    dis_type_name.clear();
    dis_file_name.clear();
    node_number_vector.clear();
    node_value_vector.clear();
    node_renumber_vector.clear();
    geo_node_number = -1;
    geo_node_value = 0.0;
    delimiter_type.clear();
    //DB
    db_file_name.clear();
    // MSH
    msh_node_number = -1;
    PointsHaveDistribedBC.clear();
    DistribedBC.clear();
}

/**************************************************************************
FEMLib-Method: 
Task: BC read function
Programing:
01/2004 OK Implementation
09/2004 OK POINTS method
11/2004 MX stream string
**************************************************************************/
ios::pos_type CBoundaryCondition::Read(ifstream *bc_file)
{
  string sub_line;
  string line_string;
  string delimiter(" ");
  bool new_keyword = false;
  bool new_subkeyword = false;
  string hash("#");
  ios::pos_type position;
  string sub_string, strbuff;
  int  ibuff;  //pos,
  double dbuff; //WW
  ios::pos_type position_line;
  std::stringstream in;
  int nLBC;
 
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while (!new_keyword) {
    new_subkeyword = false;
    position = bc_file->tellg();
	line_string = GetLineFromFile1(bc_file);
	if(line_string.size() < 1) break;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
	}
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(bc_file));
      in >> pcs_type_name;
	  in.clear();
     }
    //....................................................................
    if(line_string.find("$PRIMARY_VARIABLE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(bc_file));
      in >> pcs_pv_name;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { //subkeyword found
	  in.str(GetLineFromFile1(bc_file));
      in >> sub_string; //sub_line
      if(sub_string.compare("POINT")==0) { //OK
        in >> geo_name; //sub_line
    	in.clear();
        geo_type_name = "POINT";
        geo_type = 0;
       // CGLPoint* m_point = NULL;//CC
       // m_point= GEOGetPointByName(geo_name);//CC
        //if(m_point)
         // geo_node_number =  m_point->id;
       // sub_string = geo_name.substr(5,100);//CC remove
        //geo_node_number = strtol(sub_string.c_str(),NULL,0);//CC remove
      }
 /*    if(sub_string.find("POINTS")!=string::npos) { //OK //MX ToDo
        bc_file->seekg(position_line,ios::beg);
       *bc_file >> geo_type_name;
        pos = (int)geo_type_name.find(":");
        if(pos>0)
          geo_type_name.erase(pos);
       *bc_file >> geo_name;
        remove_white_space(&geo_name);
        sub_string = geo_name.substr(5,100);
        geo_node_number = strtol(sub_string.c_str(),NULL,0);
        bc_file->ignore(MAX_ZEILE,'\n');
        continue;
      }
 */
	  if(sub_string.find("POLYLINE")!=string::npos) {
	    in >> geo_name; //sub_line
    	in.clear();
        geo_type_name = "POLYLINE";
        CGLPolyline* m_ply = NULL;
        m_ply = GEOGetPLYByName(geo_name); //CC 10/05
        if(!m_ply)
          cout << "Warning in BCRead: no PLY data" << endl;
#ifdef MFC
        if(!m_ply)
          AfxMessageBox("Warning in BCRead: no PLY data");
#endif
        geo_type = 1;
      }

      if(sub_string.find("SURFACE")!=string::npos) {
	    in >> geo_name; //sub_line
    	in.clear();
        geo_type_name = "SURFACE";
        geo_type = 2;
      }
      if(sub_string.find("VOLUME")!=string::npos) {
	    in >> geo_name; //sub_line
    	in.clear();

        geo_type_name = "VOLUME";
        geo_type = 3;
      }
    }
    //....................................................................
    if(line_string.find("$DIS_TYPE")!=string::npos) { //PCH
	  in.str(GetLineFromFile1(bc_file));
      in >> line_string; //sub_line
	  periodic  = false; // JOD
      // Soure terms are assign to element nodes directly. 23.02.2009. WW
	  if(line_string.find("DIRECT")!=string::npos)
      {
        dis_type_name = "DIRECT";     
		in >> fname;
        fname = FilePath+fname;
		in.clear();
	  }
      // Patch-wise constant. 19.03.2009. WW
	  if(line_string.find("PATCH_C")!=string::npos)
      {
        dis_type_name = "PATCH_C";     
		in >> fname;
        fname = FilePath+fname;
        in >> geo_node_value;
		in.clear();
	  }

      if(line_string.find("CONSTANT")!=string::npos) {
        dis_type_name = "CONSTANT";
        dis_type = 0;

	    in >> geo_node_value; //sub_line
    	in.clear();
      }
      if(line_string.find("POINTS")!=string::npos) {
        dis_type_name = "POINTS";
        dis_type = 1;

	    in >> db_file_name; //sub_line
    	in.clear();
//        pos1=pos2+1;
//        sub_string = get_sub_string(buffer,delimiter,pos1,&pos2);
//        db_file_name = sub_string;
        BCReadDataBase(db_file_name);
      }
      if(line_string.find("LINEAR")!=string::npos) {
        dis_type_name = "LINEAR";
        dis_type = 2;
        // Distribued. WW
	    in >> nLBC; //sub_line
    	in.clear();

//        sub_string = strtok(buffer,seps);
//        sub_string = strtok( NULL, seps );
//        int nLBC = atoi(sub_string.c_str());
        for(int i=0; i<nLBC; i++)
        {
		  in.str(GetLineFromFile1(bc_file));
          in >> ibuff >> dbuff >> strbuff;
          in.clear(); 

//           *bc_file>>ibuff>>dbuff;
           PointsHaveDistribedBC.push_back(ibuff);
           DistribedBC.push_back(dbuff);
           if(strbuff.size() > 0)
           {
              PointsFCTNames.push_back(strbuff);
              time_dep_interpol = true;
           }
        }
//        bc_file->ignore(MAX_ZEILE,'\n');
      }
      if(line_string.find("DATA_BASE")!=string::npos) {
        dis_type_name = "DATA_BASE";
        dis_type = 3;
		in >> db_file_name; //sub_line
    	in.clear();

//        *bc_file >> db_file_name;
        BCReadDataBase(db_file_name);
      }
      if(line_string.find("SUBSTITUTE")!=string::npos) {
        dis_type_name = "VARIABLE";
	    in >> geo_node_substitute; //sub_line
    	in.clear();
      }
	  if(line_string.find("PERIODIC")!=string::npos) { // JOD
       dis_type_name = "PERIODIC";
	    periodic  = true;
        dis_type = 0;
	    in >> geo_node_value;
		in.clear();
	    in.str(GetLineFromFile1(bc_file));
		in >> periode_time_length >> periode_phase_shift ; //sub_line
    	in.clear();
      } // subkeyword found
	}
    // Time dependent function
    //..Time dependent curve ............................................
    if(line_string.find("$TIM_TYPE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(bc_file));
      in >> line_string;

//      bc_file->getline(buffer,MAX_ZEILE);
//      line_string = buffer;
//      pos1 = 0;
//      sub_string = get_sub_string(buffer,delimiter_type,pos1,&pos2);
      if(line_string.find("CURVE")!=string::npos) {
        tim_type_name = "CURVE";
        dis_type = 0;
        in>>CurveIndex;
        in.clear(); 

//        pos1=pos2+1;
//        sub_string = get_sub_string(buffer,delimiter,pos1,&pos2);
//		CurveIndex = atoi(sub_string.c_str());
      }
	  continue;
	}
    //....................................................................
    if(line_string.find("$FCT_TYPE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(bc_file));
      in >> fct_name; //sub_line
      in.clear(); 

//		bc_file->getline(line,MAX_ZEILE);
//      line_string = line;
//      remove_white_space(&line_string);
//      fct_name = line_string.substr(0);
    }
    //....................................................................
//OK4105
    if(line_string.find("$MSH_TYPE")!=string::npos) { //subkeyword found
	  in.str(GetLineFromFile1(bc_file));
      in >> sub_string; //sub_line
      msh_type_name = "NODE";
      if(sub_string.find("NODE")!=string::npos) {
	    in >> msh_node_number;
    	in.clear();
      }
    }
    //....................................................................
//OK4108
    if(line_string.find("$DIS_TYPE_CONDITION")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(bc_file)); // CONSTANT -21500.0
      in >> line_string;
      if(line_string.find("CONSTANT")!=string::npos) {
        dis_type_name = "CONSTANT";
        dis_type = 0;
	    in >> geo_node_value;
    	in.clear();
      }
      in.str(GetLineFromFile1(bc_file)); // 0.0 IF HEAD > 0.04
      in >> node_value_cond >> line_string >> pcs_pv_name_cond >> line_string >> condition;
      in.clear();
      in.str(GetLineFromFile1(bc_file)); // PCS OVERLAND_FLOW
      in >> line_string >> pcs_type_name_cond;
      in.clear();
      conditional = true;
    }
    //....................................................................
  }
  return position;
}


/**************************************************************************
FEMLib-Method: CBoundaryCondition::Write
02/2004 OK Implementation
07/2007 OK LINEAR
10/2008 OK NOD
**************************************************************************/
void CBoundaryCondition::Write(fstream* rfd_file)
{
  //KEYWORD
  *rfd_file  << "#BOUNDARY_CONDITION" << endl;
  //--------------------------------------------------------------------
  //NAME+NUMBER
  *rfd_file << " $PCS_TYPE" << endl;
  *rfd_file << "  " << pcs_type_name << endl;
  *rfd_file << " $PRIMARY_VARIABLE" << endl;
  *rfd_file << "  " << pcs_pv_name << endl;
  //--------------------------------------------------------------------
  //GEO_TYPE
  *rfd_file << " $GEO_TYPE" << endl;
  *rfd_file << "  ";
  *rfd_file << geo_type_name << delimiter_type << geo_name << endl;
  //--------------------------------------------------------------------
//OK4105
  //MSH_TYPE
  if(msh_node_number>0){
   *rfd_file << " $MSH_TYPE" << endl;
   *rfd_file << "  ";
   *rfd_file << "NODE" << delimiter_type << msh_node_number << endl;
  }
  //--------------------------------------------------------------------
  //DIS_TYPE
  *rfd_file << " $DIS_TYPE" << endl;
  *rfd_file << "  ";
  *rfd_file << dis_type_name;
  switch(dis_type_name[0]) 
  {
    case 'C': // Constant
      *rfd_file << delimiter_type << geo_node_value;
      *rfd_file << endl;
      break;
    case 'L': // Linear
	  *rfd_file << " " << (int)PointsHaveDistribedBC.size() << endl;
      for(long i=0;i<(long)PointsHaveDistribedBC.size();i++)
      {
	    *rfd_file << "  " << PointsHaveDistribedBC[i] << " ";
	    *rfd_file << "  " << DistribedBC[i] << endl;
      }
      break;
    case 'N': //OK4801 NOD
      *rfd_file << delimiter_type << geo_node_value;
      *rfd_file << endl;
      break;
  }
  //--------------------------------------------------------------------
  //FCT
  if(fct_name.length()>0){ //OK4108
    *rfd_file << " $FCT_TYPE" << endl;
    *rfd_file << "  ";
    *rfd_file << fct_name << endl;
  }
}

/**************************************************************************
FEMLib-Method: CBoundaryCondition::Write
Task: write function
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
void CBoundaryCondition::WriteTecplot(fstream* tec_file)
{
  long i;
  CGLPolyline* m_polyline1 = NULL;
  CGLPolyline* m_polyline2 = NULL;
 // list<CGLPolyline*>::const_iterator p;
vector<CGLPolyline*>::iterator p;
  Surface *m_surface = NULL;
  long no_points = 0;
  vector<CTriangle*>triangle_vector;

  *tec_file << "VARIABLES = X,Y,Z,V1" << endl;

  if(geo_type_name.compare("SURFACE")==0){
    m_surface = GEOGetSFCByName(geo_name);//CC
    if(m_surface)
    switch(m_surface->type){
      case 2:
        p = m_surface->polyline_of_surface_vector.begin();
        while(p!=m_surface->polyline_of_surface_vector.end()) {
          m_polyline1 = *p; 
          ++p;
          m_polyline2 = *p;
          break;
        }
        no_points = (long)m_polyline1->point_vector.size();
/*
        for(i=0;i<no_points-1;i++) {
          m_triangle = new CTriangle;
          m_triangle->x[0] = m_polyline1->point_vector[i]->x; 
          m_triangle->y[0] = m_polyline1->point_vector[i]->y; 
          m_triangle->z[0] = m_polyline1->point_vector[i]->z; 
          m_triangle->x[1] = m_polyline1->point_vector[i+1]->x; 
          m_triangle->y[1] = m_polyline1->point_vector[i+1]->y; 
          m_triangle->z[1] = m_polyline1->point_vector[i+1]->z; 
          m_triangle->x[2] = m_polyline2->point_vector[i+1]->x; 
          m_triangle->y[2] = m_polyline2->point_vector[i+1]->y; 
          m_triangle->z[2] = m_polyline2->point_vector[i+1]->z; 
          triangle_vector.push_back(m_triangle);
          m_triangle = new CTriangle;
          m_triangle->x[0] = m_polyline2->point_vector[i]->x; 
          m_triangle->y[0] = m_polyline2->point_vector[i]->y; 
          m_triangle->z[0] = m_polyline2->point_vector[i]->z; 
          m_triangle->x[1] = m_polyline2->point_vector[i+1]->x; 
          m_triangle->y[1] = m_polyline2->point_vector[i+1]->y; 
          m_triangle->z[1] = m_polyline2->point_vector[i+1]->z; 
          m_triangle->x[2] = m_polyline1->point_vector[i+1]->x; 
          m_triangle->y[2] = m_polyline1->point_vector[i+1]->y; 
          m_triangle->z[2] = m_polyline1->point_vector[i+1]->z; 
          triangle_vector.push_back(m_triangle);
        }
*/
      break;
    }
  }

  long no_nodes = 2*no_points;
  //long no_elements = triangle_vector.size();
  long no_elements = 2*(no_points-1);
  // Write 
  *tec_file << "ZONE T = " << geo_name << ", " \
             << "N = " << no_nodes << ", " \
             << "E = " << no_elements << ", " \
             << "F = FEPOINT" << ", " << "ET = TRIANGLE" << endl;
 if(m_polyline1)
  for(i=0;i<no_points;i++) {
    *tec_file \
      << m_polyline1->point_vector[i]->x << " " << m_polyline1->point_vector[i]->y << " " << m_polyline1->point_vector[i]->z << " " \
      << geo_node_value << endl;
  }
 if(m_polyline2)
  for(i=0;i<no_points;i++) {
    *tec_file \
      << m_polyline2->point_vector[i]->x << " " << m_polyline2->point_vector[i]->y << " " << m_polyline2->point_vector[i]->z << " " \
      << geo_node_value << endl;
  }
  for(i=0;i<no_points-1;i++) {
    *tec_file \
      << i+1 << " " << i+1+1 << " " << no_points+i+1 << endl;
  }
  for(i=0;i<no_points-1;i++) {
    *tec_file \
      << no_points+i+1 << " " << no_points+i+1+1 << " " << i+1+1 << endl;
  }
}


/**************************************************************************
GEOLib-Method:
Task: Prescibe the boundary values to all points of a polyline by the means 
of spline
Programing:
02/2004 WW Implementation
last modification:
**************************************************************************/
void InterpolateValues(vector<CNodeValue*>node_value_vector)
{
  long node_value_vector_length = (long)node_value_vector.size();
  long i;
  double *distance_vector = NULL;
  double x0,y0,z0;
  double x,y,z;
  double distance;
  distance_vector = new double[node_value_vector_length];
  // --------------------------------------------------------------------
  distance_vector[0] = 0.0;
  for(i=1;i<node_value_vector_length;i++) {
    x0 = GetNodeX(node_value_vector[i-1]->msh_node_number);
    y0 = GetNodeY(node_value_vector[i-1]->msh_node_number);
    z0 = GetNodeZ(node_value_vector[i-1]->msh_node_number);
    x = GetNodeX(node_value_vector[i]->msh_node_number);
    y = GetNodeY(node_value_vector[i]->msh_node_number);
    z = GetNodeZ(node_value_vector[i]->msh_node_number);
    distance_vector[i] = distance_vector[i-1]
	                      + sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0)+(z-z0)*(z-z0));
  }
  //distance_vector[]
  //node_value_vector[]->node_value
  // Spline interpolation
  double ss;
  vector<double> ss0;
  vector<double> bVal;
  for(i=0;i<node_value_vector_length;i++) {
    if(fabs(node_value_vector[i]->node_value)> MKleinsteZahl) {
      ss = distance_vector[i];
      ss0.push_back(ss);
      bVal.push_back(node_value_vector[i]->node_value);
    }
  }
  int ss0_length = (int)ss0.size();
  if(ss0_length==0)
    return;

  CubicSpline *csp = NULL;
  csp = new CubicSpline(ss0, bVal);

  for(i=0;i<node_value_vector_length;i++) {
    if(fabs(node_value_vector[i]->node_value)< MKleinsteZahl) {
      distance = distance_vector[i];
      node_value_vector[i]->node_value = csp->interpolation(distance);
    }   
  }

  // Memory release
  ss0.clear();
  bVal.clear();
  delete distance_vector;
}

/**************************************************************************
FEMLib-Method: 
Task: BC DB function
Programing:
02/2004 OK Implementation
**************************************************************************/
void CBoundaryCondition::ExecuteDataBasePolyline(CGLPolyline *m_polyline)
{
  CBoundaryCondition *m_bc = NULL;
  CGLPoint *m_point = NULL;
  long number_of_nodes;
  long *nodes = NULL;
  vector<CNodeValue*>node_value_vector_tmp;
  long i,j;
  CNodeValue *node_value = NULL;
  long bc_db_vector_size = (long)bc_db_vector.size();
  double pt1[3],pt2[3];
  double distance;
  long *nodes_unsorted = NULL;
  //...............................................................
  // Auxillary node_value vector
  nodes_unsorted = MSHGetNodesClose(&number_of_nodes,m_polyline);
  // Sort by distance
  pt1[0] = m_polyline->point_vector[0]->x;
  pt1[1] = m_polyline->point_vector[0]->y;
  pt1[2] = m_polyline->point_vector[0]->z;
  double *node_distances = new double[number_of_nodes];
  for(i=0;i<number_of_nodes;i++) {
    pt2[0] = GetNodeX(nodes_unsorted[i]);
    pt2[1] = GetNodeY(nodes_unsorted[i]);
    pt2[2] = GetNodeZ(nodes_unsorted[i]);
    node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
  }
  nodes = TOLSortNodes1(nodes_unsorted,node_distances,number_of_nodes);
  //
  for(i=0;i<number_of_nodes;i++) {
    node_value = new CNodeValue;
    node_value->geo_node_number = -1;
    node_value->msh_node_number = nodes[i];
    node_value->node_value = 0.0;
    pt1[0] = GetNodeX(nodes[i]);
    pt1[1] = GetNodeY(nodes[i]);
    pt1[2] = GetNodeZ(nodes[i]);
    for(j=0;j<bc_db_vector_size;j++) {
      m_bc = bc_db_vector[j];
      m_point = GEOGetPointByName(m_bc->geo_name);//CC
      m_bc->geo_node_number = m_point->id; //CC
      pt2[0] = m_point->x;
      pt2[1] = m_point->y;
      pt2[2] = m_point->z;
      distance = MCalcDistancePointToPoint(pt1,pt2);
      if(distance<1e-3) {
        node_value->geo_node_number = m_bc->geo_node_number;
        node_value->node_value = m_bc->geo_node_value;
      }
    }
    node_value_vector_tmp.push_back(node_value);
  }
  //...............................................................
  // Interpolate at polyline
  InterpolateValues(node_value_vector_tmp);
  //...............................................................
  // Fill BC node_value vector
  double tmp;
  long l_tmp;
  for(i=0;i<number_of_nodes;i++) {
    l_tmp = node_value_vector_tmp[i]->msh_node_number;
    node_number_vector.push_back(node_value_vector_tmp[i]->msh_node_number);
    tmp = node_value_vector_tmp[i]->node_value;
    node_value_vector.push_back(node_value_vector_tmp[i]->node_value);
  }

  // Release memory
  node_value_vector_tmp.clear();
  delete [] node_distances;
  delete [] nodes_unsorted;
}

/**************************************************************************
FEMLib-Method: 
Task: BC DB function
Programing:
02/2004 OK Implementation
**************************************************************************/
void BCExecuteDataBase(void)
{
  CBoundaryCondition *m_bc = NULL;
  CGLPolyline *m_polyline = NULL;
  //-----------------------------------------------------------------------
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if(!(m_bc->db_file_name.empty())) {
      //...................................................................
      // Polylines
      if(m_bc->geo_type_name.compare("POLYLINE")==0) {
        m_polyline = GEOGetPLYByName(m_bc->geo_name);
        if(m_polyline) {
          m_bc->ExecuteDataBasePolyline(m_polyline);
        }
      } 
      //...................................................................
      if(m_bc->geo_type_name.compare("SURFACE")==0) {
      // Surfaces
      }
    } // if DB
    ++p_bc;
  } // while BC
}

/**************************************************************************
FEMLib-Method: 
Task: BC read function
Programing:
01/2004 OK Implementation
01/2005 OK Boolean type
01/2005 OK Destruct before read
**************************************************************************/
bool BCRead(string file_base_name)
{
  //----------------------------------------------------------------------
//OK  BCDelete();  
  //----------------------------------------------------------------------
  CBoundaryCondition *m_bc = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  string bc_file_name;
  ios::pos_type position;
  //========================================================================
  // File handling
  bc_file_name = file_base_name + BC_FILE_EXTENSION;
  ifstream bc_file (bc_file_name.data(),ios::in);
  if (!bc_file.good()){
    cout << "! Error in BCRead: No boundary conditions !" << endl;
    return false;
  }
  // Rewind the file
  bc_file.clear();
  bc_file.seekg(0L,ios::beg); 
  //========================================================================
  // Keyword loop
  cout << "BCRead" << endl;
  while (!bc_file.eof()) {
    bc_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return true;
    //----------------------------------------------------------------------
    if(line_string.find("#BOUNDARY_CONDITION")!=string::npos) { // keyword found
      m_bc = new CBoundaryCondition();
      position = m_bc->Read(&bc_file);
      bc_list.push_back(m_bc);
      bc_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  return true;
}

/**************************************************************************
FEMLib-Method: BCWrite
Task: master write function
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
void BCWrite(string base_file_name)
{
  CBoundaryCondition *m_bc = NULL;
  string sub_line;
  string line_string;
  //========================================================================
  // File handling
  string bc_file_name = base_file_name + BC_FILE_EXTENSION;
  fstream bc_file (bc_file_name.data(),ios::trunc|ios::out);
  bc_file.setf(ios::scientific,ios::floatfield);
  bc_file.precision(12);
  //OK string tec_file_name = base_file_name + ".tec";
  //OK fstream tec_file (tec_file_name.data(),ios::trunc|ios::out);
  //OK tec_file.setf(ios::scientific,ios::floatfield);
  //OK tec_file.precision(12);
  if (!bc_file.good()) return;
  bc_file.seekg(0L,ios::beg); // rewind?
  //========================================================================
  bc_file << "GeoSys-BC: Boundary Conditions ------------------------------------------------\n";
  //========================================================================
  // BC list
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    m_bc->Write(&bc_file);
    //OK m_bc->WriteTecplot(&tec_file);
    ++p_bc;
  }
  bc_file << "#STOP";
  bc_file.close();
  //OK tec_file.close();
}

/**************************************************************************
FEMLib-Method: 
01/2004 OK Implementation
07/2007 OK V2, global function
**************************************************************************/
CBoundaryCondition* BCGet(string pcs_name,string geo_type_name,string geo_name)
{
  CBoundaryCondition *m_bc = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) 
  {
    m_bc = *p_bc;
    if((m_bc->pcs_type_name.compare(pcs_name)==0)&&
       (m_bc->geo_type_name.compare(geo_type_name)==0)&&
       (m_bc->geo_name.compare(geo_name)==0))
       return m_bc;
    ++p_bc;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: CBoundaryCondition::SetDISType
Task: 
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
void CBoundaryCondition::SetDISType(void)
{
  if(dis_type_name.compare("CONSTANT")==0) dis_type = 0;
  if(dis_type_name.compare("LINEAR")  ==0) dis_type = 1;
}

/**************************************************************************
FEMLib-Method: CBoundaryCondition::SetGEOType
Task: 
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
void CBoundaryCondition::SetGEOType(void)
{
  if(geo_type_name.compare("POINT")==0) {
  }
  if(geo_type_name.compare("POLYLINE")) {
    CGLPolyline *m_polyline = NULL;
    m_polyline = GEOGetPLYByName(geo_name);//CC
    if(m_polyline)
      m_polyline->type = BC;
  }
  if(geo_type_name.compare("SURFACE")==0) {
  }
}

/**************************************************************************
GeoSys source term function: 
02/2009 WW Implementation
**************************************************************************/
inline void CBoundaryCondition::DirectAssign(const long ShiftInNodeVector)
{
  string line_string;
  string st_file_name;
  std::stringstream in;
  long n_index;
  double n_val;
  CRFProcess* m_pcs = NULL;
  CBoundaryConditionNode  *m_node_value = NULL;

  m_pcs = PCSGet(pcs_type_name);

  //========================================================================
  // File handling
  ifstream d_file (fname.c_str(),ios::in);
  //if (!st_file.good()) return;

  if (!d_file.good()){
    cout << "! Error in direct node source terms: Could not find file:!\n" 
         <<fname<<endl;
    abort();
  }
  // Rewind the file
  d_file.clear();
  d_file.seekg(0L,ios::beg);
  //========================================================================
  while (!d_file.eof()) 
  {
    line_string = GetLineFromFile1(&d_file);
    if(line_string.find("#STOP")!=string::npos)
      break;
    
    in.str(line_string); 
    in>>n_index>>n_val;   
    in.clear(); 
    // 
    m_node_value = new CBoundaryConditionNode;
	m_node_value->conditional = false;
    m_node_value->msh_node_number = n_index + ShiftInNodeVector;
    m_node_value->geo_node_number = n_index;
    m_node_value->node_value = n_val;
    m_node_value->CurveIndex = CurveIndex;
    m_pcs->bc_node.push_back(this);  
    m_pcs->bc_node_value.push_back(m_node_value); 
  } // eof
}
/**************************************************************************
GeoSys BC function: 
03/2009 WW Implementation
**************************************************************************/
inline void CBoundaryCondition::PatchAssign(const long ShiftInNodeVector)
{
  string line_string;
  string st_file_name;
  std::stringstream in;
  long n_index;
  vector<long> sfc_nodes;
  CRFProcess* m_pcs = NULL;
  CBoundaryConditionNode  *m_node_value = NULL;

  m_pcs = PCSGet(pcs_type_name);

  Surface *m_surface = NULL;
  m_surface = GEOGetSFCByName(geo_name);
  //========================================================================
  // File handling
  ifstream d_file (fname.c_str(),ios::in);
  //if (!st_file.good()) return;

  if (!d_file.good()){
    cout << "! Error in direct node source terms: Could not find file:!\n" 
         <<fname<<endl;
    abort();
  }
  // Rewind the file
  d_file.clear();
  d_file.seekg(0L,ios::beg);
  //========================================================================
  while (!d_file.eof()) 
  {
    line_string = GetLineFromFile1(&d_file);
    if(line_string.find("#STOP")!=string::npos)
      break;
    
    in.str(line_string); 
    in>>n_index;   
    in.clear();
    sfc_nodes.push_back(n_index);
  }
  if(m_surface)
     m_pcs->m_msh->GetNODOnSFC_PLY_XY(m_surface,sfc_nodes, true);
  for(long i=0; i<(long)sfc_nodes.size(); i++)
  {  
    // 
    m_node_value = new CBoundaryConditionNode;
	m_node_value->conditional = false;
    n_index = sfc_nodes[i];
    m_node_value->msh_node_number = n_index + ShiftInNodeVector;
    m_node_value->geo_node_number = n_index;
    m_node_value->node_value = geo_node_value;
    m_node_value->CurveIndex = CurveIndex;
    m_pcs->bc_node.push_back(this);  
    m_pcs->bc_node_value.push_back(m_node_value); 
  } // eof
}


/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2004 OK Implementation
**************************************************************************/
void BCReadDataBase(string csv_file_name)
{
  char buffer[MAX_ZEILE];
  string sub_string,cut_string;
  string line;
  int pos1,pos2;
  string delimiter_type(";");
  CBoundaryCondition *m_bc = NULL;
  //========================================================================
  // File handling
  ifstream csv_file (csv_file_name.data(),ios::in);
  if (!csv_file.good()){
    cout << "! Warning in BCReadDataBase: DB file " << csv_file_name << " not found!" << endl;
    return;
  }
  csv_file.seekg(0L,ios::beg);
  //========================================================================
  // 
  pos1=0;
  pos2=0;
  csv_file.getline(buffer,MAX_ZEILE);
  line = buffer;
  cut_string = line;
  //------------------------------------------------------------------------
  // read header
  while (pos2>=0) {
    //sub_string = get_sub_string(line,delimiter_type,pos1,&pos2);
    pos2 = (int)cut_string.find_first_of(delimiter_type);
    string db_head = cut_string.substr(0,pos2);
    bc_db_head.push_back(db_head);
    string tmp = cut_string.substr(pos2+delimiter_type.size());
    cut_string = tmp;
    //pos1=pos2;
  }
  //------------------------------------------------------------------------
  // read data
  int bc_db_head_size = (int)bc_db_head.size();
  int i;
  while (!csv_file.eof()) {
    pos1=0;
    csv_file.getline(buffer,MAX_ZEILE);
line = buffer;
if((line.find("#STOP")!=string::npos)||(line.empty())) return;
cut_string = line;
    m_bc = new CBoundaryCondition;
    for(i=0;i<bc_db_head_size;i++) {
      //string db_entry = get_sub_string(buffer,delimiter_type,pos1,&pos2);
pos2 = (int)cut_string.find_first_of(delimiter_type);
string db_entry = cut_string.substr(0,pos2);
string tmp = cut_string.substr(pos2+delimiter_type.size());
cut_string = tmp;
      if(bc_db_head[i].compare("GEO_TYPE")==0) {
        m_bc->geo_type_name = db_entry;
      }
      if(bc_db_head[i].compare("GEO_ID")==0) {
        m_bc->geo_node_number = strtol(db_entry.data(),NULL,0);
      }
      if(bc_db_head[i].compare("BC_HEAD")==0) {
        m_bc->pcs_type_name = "PRESSURE1"; //OK4108
        m_bc->geo_node_value = strtod(db_entry.data(),NULL);
      }
      pos1=pos2;
    }
    bc_db_vector.push_back(m_bc);
  } // eof
}


CBoundaryConditionsGroup::CBoundaryConditionsGroup(void)
{
	msh_node_number_subst = -1; //
    time_dep_bc = -1;
}

CBoundaryConditionsGroup::~CBoundaryConditionsGroup(void)
{
/*
  int group_vector_length = group_vector.size();
  int i;
  for(i=0;i<group_vector_length;i++)
    group_vector.pop_back();
*/
//  group_vector.clear();

}

/**************************************************************************
FEMLib-Method: CBoundaryCondition::Set
Task: set boundary conditions
Programing:
02/2004 OK Implementation
09/2004 WW Interpolation of piecewise linear BC
02/2005 OK MSH types
03/2005 OK MultiMSH, PNT
08/2005 WW Changes due to the new goematry finite element.
12/2005 OK FCT
04/2006 WW New storage
09/2006 WW Move linear interpolation to new MSH strcuture
12/2007 WW Linear distributed BC in a surface
10/2008 WW/CB SetTransientBCtoNodes  
last modification:
**************************************************************************/
void CBoundaryConditionsGroup::Set(CRFProcess* m_pcs, const int ShiftInNodeVector, 
                              	   string this_pv_name)
{
  long number_of_nodes = 0;
  long *nodes = NULL;
  vector<long>nodes_vector;
  vector<double>node_value;
  CGLPolyline *m_polyline = NULL;
  long i,j;
  CBoundaryCondition *m_bc = NULL;
  CBoundaryConditionNode *m_node_value = NULL;
  group_name = pcs_type_name;
  bool quadratic = false;
  bool cont = false; //WW
  //----------------------------------------------------------------------
  if(this_pv_name.size()!=0)
    pcs_pv_name = this_pv_name;
  CFEMesh* m_msh = m_pcs->m_msh;
  // Tests //OK
  if(!m_msh){
    cout << "Warning in CBoundaryConditionsGroup::Set - no MSH data" << endl;
    //return;
  }
  if(m_msh)//WW
  {
     /// In case of P_U coupling monolithic scheme
     if(m_pcs->type==41) //WW Mono
     {
       if(pcs_pv_name.find("DISPLACEMENT")!=string::npos) //Deform 
          quadratic = true; 
       else if(pcs_pv_name.find("VELOCITY_DM")!=string::npos) //Deform 
          quadratic = true;  
       else quadratic = false;  
     }
     else if(m_pcs->type==4) quadratic = true; 
	 else quadratic = false; 
     m_pcs->m_msh->SwitchOnQuadraticNodes(quadratic);
  }
  

  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if(m_bc->time_dep_interpol)  //WW/CB
    { 
      ++p_bc;
      continue; 
    }
    //====================================================================
    //OK if(m_bc->pcs_type_name.compare(pcs_type_name)==0){ //OK/SB 4108
    if((m_bc->pcs_type_name.compare(pcs_type_name)==0)&&(m_bc->pcs_pv_name.compare(pcs_pv_name)==0)){
        //................................................................
        //-- 23.02.3009. WW
        if(m_bc->dis_type_name.find("DIRECT")!=string::npos)
        {
           m_bc->DirectAssign(ShiftInNodeVector);
           ++p_bc;
           continue;
        }        
        //-- 19.03.3009. WW
        if(m_bc->dis_type_name.find("PATCH_C")!=string::npos)
        {
           m_bc->PatchAssign(ShiftInNodeVector);
           ++p_bc;
           continue;
        }        
        //
        cont = false;
        if(m_bc->dis_type_name.compare("VARIABLE")==0){ //OK
          cont = true;
          CGLPoint* m_geo_point = NULL;
          m_geo_point = GEOGetPointByName(m_bc->geo_node_substitute);//C10/05
          if(m_geo_point){
//            m_node_value->msh_node_number = ShiftInNodeVector 
//                                          + GetNodeNumberClose(m_geo_point->x,m_geo_point->y,m_geo_point->z);
			  msh_node_number_subst = ShiftInNodeVector + m_msh->GetNODOnPNT(m_geo_point);
          }
        }

       

      //------------------------------------------------------------------
      if(m_bc->geo_type_name.compare("POINT")==0) {
        m_node_value = new CBoundaryConditionNode;
        //m_node_value->geo_node_number = m_bc->geo_node_number;//CC remove
        // Get MSH node number
        CGLPoint* m_geo_point = NULL;
        if(m_bc->dis_type_name.compare("CONSTANT")==0 || m_bc->dis_type_name.compare("PERIODIC")==0){ //JOD
        m_geo_point = GEOGetPointByName(m_bc->geo_name);//CC
        if(m_geo_point)
        m_bc->geo_node_number = m_geo_point->id;//CC
        m_node_value->geo_node_number = m_bc->geo_node_number;//CC
          if(m_geo_point){
             if(m_msh)
                m_node_value->geo_node_number = m_msh->GetNODOnPNT(m_geo_point);
			 else
			 { 
                 m_node_value->msh_node_number = ShiftInNodeVector+
	             GetNodeNumberClose(m_geo_point->x,m_geo_point->y,m_geo_point->z);
			 }
		  }
		}
		
        //................................................................
        if(m_bc->dis_type_name.compare("VARIABLE")==0){ //OK
          m_node_value->conditional = true;
          m_geo_point = GEOGetPointByName(m_bc->geo_name);   //YD
          if(m_geo_point){
             m_node_value->geo_node_number =  m_msh->GetNODOnPNT(m_geo_point); 
             m_node_value->msh_node_number = m_node_value->geo_node_number; // WW ShiftInNodeVector + 
                                      //    + GetNodeNumberClose(m_geo_point->x,m_geo_point->y,m_geo_point->z);
//             m_node_value->msh_node_number_subst = ShiftInNodeVector + m_msh->GetNODOnPNT(m_geo_point);
          }
        }
        m_node_value->conditional = cont;
		m_node_value->CurveIndex = m_bc->CurveIndex;
        m_node_value->node_value = m_bc->geo_node_value;
        m_node_value->msh_node_number = m_node_value->geo_node_number +ShiftInNodeVector; //WW 
		m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
        m_node_value->msh_node_number_subst = msh_node_number_subst;
        m_pcs->bc_node.push_back(m_bc);  //WW
        m_pcs->bc_node_value.push_back(m_node_value);  //WW
      }
      //------------------------------------------------------------------
      // MHS node close (<=eps) to point 
      if(m_bc->geo_type_name.compare("POINTS")==0) {
        // Get MSH nodes numbers
        CGLPoint* m_geo_point = NULL;
        vector<long> bc_group_msh_nodes_vector;
        m_geo_point = GEOGetPointByName(m_bc->geo_name);//CC
        if(m_geo_point){
          m_bc->geo_node_number = m_geo_point->id;//CC
          MSHGetNodesClose(bc_group_msh_nodes_vector, m_geo_point);//CC
          long no_nodes = (long)bc_group_msh_nodes_vector.size();
          for(i=0;i<no_nodes;i++){
            m_node_value = new CBoundaryConditionNode;
	        m_node_value->conditional = cont;
            m_node_value->geo_node_number = m_bc->geo_node_number;
            m_node_value->CurveIndex = m_bc->CurveIndex;
            m_node_value->msh_node_number = bc_group_msh_nodes_vector[i];
            m_node_value->node_value = m_bc->geo_node_value;
            m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
         	m_pcs->bc_node.push_back(m_bc);  //WW
            m_pcs->bc_node_value.push_back(m_node_value);  //WW
            //WW group_vector.push_back(m_node_value);
            //WW bc_group_vector.push_back(m_bc); //OK
          }
        }
        bc_group_msh_nodes_vector.clear(); // ? enough
      }
      //------------------------------------------------------------------
      if(m_bc->geo_type_name.compare("POLYLINE")==0) {
        m_polyline = GEOGetPLYByName(m_bc->geo_name);//CC
        if(m_polyline) {
        //  if(m_polyline->type==100) 
        //    m_bc->dis_type_name = "CONSTANT"; // arc. WW 
        //  else  
        //    m_polyline->type = 3;
          //..............................................................
          if(m_bc->dis_type_name.compare("CONSTANT")==0 || m_bc->dis_type_name.compare("PERIODIC")==0){ // JOD
            if(m_msh){ //OK
              if(m_polyline->type==100) //WW
                   m_msh->GetNodesOnArc(m_polyline,nodes_vector);
              else
                   m_msh->GetNODOnPLY(m_polyline,nodes_vector);
              for(i=0;i<(long)nodes_vector.size();i++){
                m_node_value = new CBoundaryConditionNode();
                m_node_value->msh_node_number = -1;
                m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
                m_node_value->geo_node_number = nodes_vector[i];
                m_node_value->node_value = m_bc->geo_node_value;  //dis_prop[0];
                m_node_value->CurveIndex = m_bc->CurveIndex;
                m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
                m_pcs->bc_node.push_back(m_bc);  //WW
                m_pcs->bc_node_value.push_back(m_node_value);  //WW
                //WW group_vector.push_back(m_node_value);
                //WW bc_group_vector.push_back(m_bc); //OK
              }
            }
            else{
              if(m_polyline->type==100)
                nodes = GetNodesOnArc(number_of_nodes,m_polyline); //WW CC
              else
                nodes = MSHGetNodesClose(&number_of_nodes,m_polyline);
              for(i=0;i<number_of_nodes;i++){
                m_node_value = new CBoundaryConditionNode();
                m_node_value->msh_node_number = -1;
                m_node_value->msh_node_number = nodes[i]+ShiftInNodeVector;
                m_node_value->geo_node_number = nodes[i];
                m_node_value->node_value = m_bc->geo_node_value;  //dis_prop[0];
                m_node_value->CurveIndex = m_bc->CurveIndex;
                m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
                m_pcs->bc_node.push_back(m_bc);  //WW
                m_pcs->bc_node_value.push_back(m_node_value);  //WW
              }
            }
          }

          if(m_bc->dis_type_name.compare("VARIABLE")==0){
             if(m_polyline->type==100) //WW
                 m_msh->GetNodesOnArc(m_polyline,nodes_vector);
	         else
                 m_msh->GetNODOnPLY(m_polyline,nodes_vector);
             for(i=0;i<(long)nodes_vector.size();i++){
                m_node_value = new CBoundaryConditionNode();
				m_node_value->conditional = cont;
                m_node_value->msh_node_number = -1;
                m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
                m_node_value->geo_node_number = nodes_vector[i];
                m_node_value->node_value = m_bc->geo_node_value;  //dis_prop[0];
                m_node_value->CurveIndex = m_bc->CurveIndex;
                m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
                m_node_value->msh_node_number_subst = msh_node_number_subst; //WW
                m_pcs->bc_node.push_back(m_bc);  //WW
                m_pcs->bc_node_value.push_back(m_node_value);  //WW
                //WW group_vector.push_back(m_node_value);
                //WW bc_group_vector.push_back(m_bc); //OK
             }
          }

          //................................................................
          if(m_bc->dis_type_name.compare("POINTS")==0){
            long node_number_vector_length = (long)m_bc->node_number_vector.size();
            for(i=0;i<node_number_vector_length;i++) {
              m_node_value = new CBoundaryConditionNode;
              m_node_value->msh_node_number = ShiftInNodeVector\
			  	                            + m_bc->node_number_vector[i];
              //m_node_value->geo_node_number = m_bc->geo_node_number;
              m_node_value->node_value = m_bc->node_value_vector[i];
              m_node_value->CurveIndex = m_bc->CurveIndex;
              m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
              m_pcs->bc_node.push_back(m_bc);  //WW
              m_pcs->bc_node_value.push_back(m_node_value);  //WW
              //WW group_vector.push_back(m_node_value);
              //WW bc_group_vector.push_back(m_bc); //OK
            }
          }
          //................................................................
          if(m_bc->dis_type_name.compare("LINEAR")==0){ //WW
            if(m_polyline->type==100) //WW
                m_msh->GetNodesOnArc(m_polyline,nodes_vector);
            else
                m_msh->GetNODOnPLY(m_polyline,nodes_vector);

            //            nodes = MSHGetNodesClose(&number_of_nodes, m_polyline);//CC
			node_value.resize(nodes_vector.size());
            // Piecewise linear distributed. WW
            for(i=0;i<(int)m_bc->DistribedBC.size(); i++)
            {              
              for(j=0;j<(int)m_polyline->point_vector.size(); j++)
              {
                if(m_bc->PointsHaveDistribedBC[i]==m_polyline->point_vector[j]->id)
                {
                  if(fabs(m_bc->DistribedBC[i])< MKleinsteZahl) m_bc->DistribedBC[i] = 1.0e-20;
                  m_polyline->point_vector[j]->propert = m_bc->DistribedBC[i];
                  break;
                }
              }  
            }
 		    InterpolationAlongPolyline(m_polyline, node_value);
            for(i=0;i<(int)nodes_vector.size();i++){
              m_node_value = new CBoundaryConditionNode();
              m_node_value->msh_node_number = -1;
              m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
              m_node_value->geo_node_number = nodes_vector[i];
              m_node_value->node_value = node_value[i];  
              m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
              m_node_value->CurveIndex = m_bc->CurveIndex;
              m_pcs->bc_node.push_back(m_bc);  //WW
              m_pcs->bc_node_value.push_back(m_node_value);  //WW
              //WW group_vector.push_back(m_node_value);
              //WW bc_group_vector.push_back(m_bc); //OK
            }
		    node_value.clear();
          }
          //................................................................
          //delete(values);
          Free(nodes);
        } // if(m_ply) 
      }
      //------------------------------------------------------------------
      if(m_bc->geo_type_name.compare("SURFACE")==0) {
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(m_bc->geo_name);//CC10/05
        if(m_surface){
          //..............................................................
          if(m_msh) //MSH
            m_msh->GetNODOnSFC(m_surface,nodes_vector);
          long nodes_vector_length = (long)nodes_vector.size();
          //..............................................................
          if(m_bc->dis_type_name.compare("LINEAR")==0){
            // Linear interpolation  polygon-wise. WW
           // list<CGLPolyline*>::const_iterator p = m_surface->polyline_of_surface_list.begin();
            vector<CGLPolyline*>::iterator p = m_surface->polyline_of_surface_vector.begin();
            node_value.resize(nodes_vector_length);
            p = m_surface->polyline_of_surface_vector.begin();
            while(p!=m_surface->polyline_of_surface_vector.end())
            {
              m_polyline = *p;
              for(i=0;i<(int)m_bc->DistribedBC.size(); i++)
              {              
                for(j=0;j<(int)m_polyline->point_vector.size(); j++)
                {
                  if(m_bc->PointsHaveDistribedBC[i]==m_polyline->point_vector[j]->id)
                  {
                    if(fabs(m_bc->DistribedBC[i])< MKleinsteZahl) 
                      m_bc->DistribedBC[i] = 1.0e-20;
                    m_polyline->point_vector[j]->propert = m_bc->DistribedBC[i];
                    break;
                  }
                }
              }
              //InterpolationAlongPolyline(m_polyline, node_value);
              p++;
            }
            node_value.resize(nodes_vector_length); //WW 
            m_bc->SurfaceIntepolation(m_pcs, nodes_vector, node_value); //WW
          }
          //..............................................................
          for(i=0;i<nodes_vector_length;i++){
            m_node_value = new CBoundaryConditionNode();
            m_node_value->msh_node_number = -1;
            m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector; //nodes[i];
            m_node_value->geo_node_number = nodes_vector[i]; //nodes[i];
            m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
            if(m_bc->dis_type_name.compare("LINEAR")==0)  //WW
              m_node_value->node_value = node_value[i];  
            else
              m_node_value->node_value = m_bc->geo_node_value;  
            m_node_value->CurveIndex = m_bc->CurveIndex;
            m_bc->node_number_vector = nodes_vector; //OK
            m_pcs->bc_node.push_back(m_bc);  //WW
            m_pcs->bc_node_value.push_back(m_node_value);  //WW
              //WW group_vector.push_back(m_node_value);
              //WW bc_group_vector.push_back(m_bc); //OK
          }
  		  node_value.clear();
        }
      }      
      //------------------------------------------------------------------
      // Material domain
      if(m_bc->geo_type_name.find("MATERIAL_DOMAIN")==0){
        GEOGetNodesInMaterialDomain(m_msh, m_bc->geo_type, nodes_vector, quadratic);
        for(i=0;i<(long)nodes_vector.size();i++){
          m_node_value = new CBoundaryConditionNode();
          m_node_value->msh_node_number = -1;
          m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector; //nodes[i];
          m_node_value->geo_node_number = nodes_vector[i]; //nodes[i];
          m_node_value->node_value = m_bc->geo_node_value;  
          m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
          m_node_value->CurveIndex = m_bc->CurveIndex;
          m_pcs->bc_node.push_back(m_bc);  //WW
          m_pcs->bc_node_value.push_back(m_node_value);  //WW
          //WW group_vector.push_back(m_node_value);
          //WW bc_group_vector.push_back(m_bc); //OK
        }
      }
      //------------------------------------------------------------------
      // MSH types //OK4105
      if(m_bc->msh_type_name.compare("NODE")==0) {
        m_node_value = new CBoundaryConditionNode;
        m_node_value->msh_node_number = m_bc->msh_node_number;
        m_node_value->geo_node_number =  m_bc->msh_node_number;
        m_node_value->node_value = m_bc->geo_node_value;
        m_node_value->CurveIndex = m_bc->CurveIndex;
        m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
        m_pcs->bc_node.push_back(m_bc);  //WW
        m_pcs->bc_node_value.push_back(m_node_value);  //WW
        //WW group_vector.push_back(m_node_value);
        //WW bc_group_vector.push_back(m_bc); //OK
      }
      //------------------------------------------------------------------
      // FCT types //OK
      if(m_bc->fct_name.size()>0){
        //WW 
        for(i=0;i<(long)m_pcs->bc_node_value.size();i++)
		{
			m_pcs->bc_node_value[i]->fct_name =  m_bc->fct_name;
			m_pcs->bc_node_value[i]->msh_node_number_subst = msh_node_number_subst;
		}
       //WW fct_name = m_bc->fct_name;
      }
      //------------------------------------------------------------------
    } // PCS
    ++p_bc;
  } // list
  //======================================================================
  // SetTransientBCtoNodes  10/2008 WW/CB Implementation
  p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
    if(!m_bc->time_dep_interpol)  //WW/CB
    { 
      ++p_bc;
      continue; 
    }
    //====================================================================
    //OK if(m_bc->pcs_type_name.compare(pcs_type_name)==0){ //OK/SB 4108
    if((m_bc->pcs_type_name.compare(pcs_type_name)==0)&&(m_bc->pcs_pv_name.compare(pcs_pv_name)==0))
    {
        //................................................................
      if(m_bc->geo_type_name.compare("POLYLINE")==0) 
      {
        m_polyline = GEOGetPLYByName(m_bc->geo_name);//CC
        if(m_polyline) 
        {
          //................................................................
          if(m_bc->dis_type_name.compare("LINEAR")==0){ //WW
            if(m_polyline->type==100) //WW
                m_msh->GetNodesOnArc(m_polyline,nodes_vector);
            else
                m_msh->GetNODOnPLY(m_polyline,nodes_vector);

			node_value.resize(nodes_vector.size());
 		    //InterpolationAlongPolyline(m_polyline, node_value);
            m_pcs->bc_transient_index.push_back((long)m_pcs->bc_node.size());
            for(i=0;i<(int)nodes_vector.size();i++)
            {
              m_node_value = new CBoundaryConditionNode();
              m_node_value->msh_node_number = -1;
              m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
              m_node_value->geo_node_number = nodes_vector[i];
              m_node_value->node_value = 0.0;  
              m_node_value->pcs_pv_name = pcs_pv_name; //YD/WW
              m_node_value->CurveIndex = m_bc->CurveIndex;
              m_pcs->bc_node.push_back(m_bc);  //WW
              m_pcs->bc_node_value.push_back(m_node_value);  //WW
            }
		    node_value.clear();
          }
          //................................................................
          //delete(values);
          Free(nodes);
        } // if(m_ply) 
      }
      //------------------------------------------------------------------
    } // PCS
    ++p_bc;
  } // list
  /* // Make the following as comment by WW
  // Test
  long no_bc = (long)m_pcs->bc_node_value.size();
  if(no_bc<1)
    cout << "Warning: no boundary conditions specified for " << pcs_type_name << endl;
  */
}

/**************************************************************************
FEMLib-Method: CBoundaryCondition::Get
Task: set boundary conditions
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
CBoundaryConditionsGroup* CBoundaryConditionsGroup::Get(string pcs_name)
{
  CBoundaryConditionsGroup *m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
  while(p_bc_group!=bc_group_list.end()) {
    m_bc_group = *p_bc_group;
    if(m_bc_group->group_name.compare(pcs_name)==0) 
      return m_bc_group;
    ++p_bc_group;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
04/2005 OK Implementation
last modification:
**************************************************************************/
CBoundaryConditionsGroup* BCGetGroup(string pcs_type_name,string pcs_pv_name)
{
  CBoundaryConditionsGroup *m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
  while(p_bc_group!=bc_group_list.end()) {
    m_bc_group = *p_bc_group;
    if((m_bc_group->pcs_type_name.compare(pcs_type_name)==0)&&\
       (m_bc_group->pcs_pv_name.compare(pcs_pv_name)==0))
        return m_bc_group;
    ++p_bc_group;
  }
  return NULL;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: IsNodeBoundaryCondition
 */
/* Aufgabe:
   Abfrage ob ein Knoten der Gruppe name ein R.B. besitzt oder nicht.
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E BOUNDARY_CONDITIONS *bc: Zeiger auf die Datenstruktur bc.
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   10/1998     AH    Erste Version
   7/1999     CT    Coredump bei fehlenden BC beseitigt
   6/2000     CT    Bugfix fuer Rueckgabe bei col<0
 */
/**************************************************************************/
long IsNodeBoundaryCondition(char *name, long node)
{
name = name;
node = node;
/* OK_BC
  long col = IndexBoundaryConditions(name);

  if (GetNode(node) == NULL)
    {
      DisplayErrorMsg("Fehler in IsNodeBoundaryCondition --> Abbruch !!!");
      abort();
    }

  if ((bc_matrix_flags) && (col >= 0))
    return (bc_matrix_flags -> m[node][col] >= 0) ? 1 : 0;
  else
*/
    return 0;
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
08/2004 OK Implementation
06/2005 OK PCS
05/2006 WW  
last modification:
**************************************************************************/
//WW int NodeSetBoundaryConditions(char *pv_name,int ndx,string pcs_type_name)
int NodeSetBoundaryConditions(char *pv_name,int ndx,CRFProcess* m_pcs)
{
  CBoundaryCondition *m_bc = NULL;
  CBoundaryConditionNode *m_node = NULL;
  long i;
  //WW
  long no_bc = (long)m_pcs->bc_node_value.size();
  for(i=0;i<no_bc;i++){
      m_node = m_pcs->bc_node_value[i];
      m_bc = m_pcs->bc_node[i];
	  if(m_bc->pcs_pv_name.compare(pv_name)==0)      
        SetNodeVal(m_node->msh_node_number,ndx,m_node->node_value); //???
  }
  if(no_bc==0)
    cout << "? Warning - NodeSetBoundaryConditions: m_bc_group not found" << endl;
  return 1;

  
/* //WW

  //OK m_bc_group = m_bc_group->Get(name);
  m_bc_group = BCGetGroup(pcs_type_name,(string)pv_name); //OK
  if(m_bc_group){
    long no_bc = (long)m_bc_group->group_vector.size();
    for(i=0;i<no_bc;i++){
      m_node = m_bc_group->group_vector[i];
      //SetNodeVal(NodeNumber[m_node->msh_node_number],ndx,m_node->node_value);
      SetNodeVal(m_node->msh_node_number,ndx,m_node->node_value); //???
    }
  }
  else
    cout << "? Warning - NodeSetBoundaryConditions: m_bc_group not found" << endl;
  return 1;
*/  
}

/**************************************************************************
FEMLib-Method:
Task: write function based on CBoundaryCondition::WriteTecplot
Programing:
01/2005 OK Implementation
last modification:
**************************************************************************/
void CBoundaryConditionsGroup::WriteTecplot()
{
  // File handling
  string bc_path;
  CGSProject* m_gsp = GSPGetMember("bc");
  if(m_gsp)
    bc_path = m_gsp->path; 
  string tec_file_name;
  tec_file_name = bc_path + "BC_" + group_name + ".tec";
  fstream tec_file (tec_file_name.data(),ios::trunc|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  // Write data
  /*
  // To be moved to pcs
  long i;
  long no_nodes = (long)group_vector.size();
  tec_file << "TITLE = Scatter Plot Types" << endl;
  tec_file << "VARIABLES = X,Y,Z,V" << endl;
  for(i=0;i<no_nodes;i++){
    tec_file << GetNodeX(group_vector[i]->msh_node_number) << ", ";
    tec_file << GetNodeY(group_vector[i]->msh_node_number) << ", ";
    tec_file << GetNodeZ(group_vector[i]->msh_node_number) << ", ";
    tec_file << group_vector[i]->node_value << endl;
  }  
  */
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void BCDelete()
{
  CBoundaryCondition* m_bc = NULL;
  list<CBoundaryCondition*>::const_iterator p=bc_list.begin();
  while (p!=bc_list.end())
  {
    //bc_list.remove(*p);
    m_bc = *p;
    delete m_bc;
    ++p;
  }
  bc_list.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void BCGroupDelete()
{
  CBoundaryConditionsGroup* m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p=bc_group_list.begin();
  while (p!=bc_group_list.end()){
    m_bc_group = *p;
    delete m_bc_group;
    //bc_group_list.remove(*p);
    ++p;
  }
  bc_group_list.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
last modification:
**************************************************************************/
/*
double CBoundaryConditionsGroup::GetConditionalNODValue(int i,CBoundaryCondition* m_bc)
{
  int j;
  int nidx;
  int no_nodes = 4;
  double m_val = 0.0;
  CRFProcess* m_pcs_cond = NULL;
  CRFProcess* m_pcs_this = NULL;
  CNode* m_nod = NULL;
  Mesh_Group::CElem* m_ele = NULL;

  //----------------------------------------------------------------------
  m_pcs_this = PCSGet(m_bc->pcs_type_name);
  m_pcs_cond = PCSGet(m_bc->pcs_type_name_cond);
  m_nod = m_pcs_this->m_msh->nod_vector[group_vector[i]->msh_node_number];
      
  //if(!m_nod->selected)
  //  return 0.0;
  //MB ??? CheckMarkedElement();
      
  m_nod->connected_elements[0];
  m_ele = m_pcs_cond->m_msh->ele_vector[0];

  nidx = m_pcs_cond->GetNodeValueIndex(m_bc->pcs_pv_name_cond)+1;
  for(j=0;j<no_nodes;j++){
    m_val += m_pcs_cond->GetNodeValue(m_ele->GetNodesNumber(m_pcs_cond->m_msh->getOrder()),nidx);
  }

  m_val /= no_nodes;

  //..................................................................
  if(m_val > m_bc->condition){
    //cout << m_val << " " << m_bc->node_value_cond << endl;
    return m_bc->node_value_cond;
  }
  else{
    //cout << m_val << " " << m_bc->geo_node_value << endl;
    return m_bc->geo_node_value;
  }
}
*/
/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void BCGroupDelete(string pcs_type_name,string pcs_pv_name)
{
  CBoundaryConditionsGroup* m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p=bc_group_list.begin();
  while (p!=bc_group_list.end()){
    m_bc_group = *p;
    if((m_bc_group->pcs_type_name.compare(pcs_type_name)==0)&&
       (m_bc_group->pcs_pv_name.compare(pcs_pv_name)==0)){
      delete m_bc_group;
      bc_group_list.remove(m_bc_group);
      return;
    }
    ++p;
  }
}

/**************************************************************************
FEMLib-Method: 
07/2007 OK Implementation
**************************************************************************/
CBoundaryCondition* BCGet(string pcs_type_name)
{
  CBoundaryCondition *m_bc = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) 
  {
    m_bc = *p_bc;
    if(m_bc->pcs_type_name.compare(pcs_type_name)==0)
      return m_bc;
    ++p_bc;
  }
  return NULL;
}


/**************************************************************************
ROCKFLOW - Funktion: 
Programming:
 11/2007 WW Implementation
**************************************************************************/
void CBoundaryCondition::SurfaceIntepolation(CRFProcess* m_pcs, vector<long>&nodes_on_sfc, 
                                vector<double>&node_value_vector)
{

  long i, j, k, l;

  //----------------------------------------------------------------------
  // Interpolation of polygon values to nodes_on_sfc
  int nPointsPly = 0;
  double Area1, Area2;
  double Tol = 1.0e-9;
  bool Passed;
  const int Size = (int)nodes_on_sfc.size();
  double gC[3],p1[3],p2[3], pn[3], vn[3],unit[3], NTri[3];
  //
  CGLPolyline* m_polyline = NULL;
  Surface *m_surface = NULL;
  m_surface = GEOGetSFCByName(geo_name);//CC

  // list<CGLPolyline*>::const_iterator p = m_surface->polyline_of_surface_list.begin();
  vector<CGLPolyline*>::iterator p = m_surface->polyline_of_surface_vector.begin();

  for(j=0; j<Size; j++)
  {
     pn[0] = m_pcs->m_msh->nod_vector[nodes_on_sfc[j]]->X(); 
     pn[1] = m_pcs->m_msh->nod_vector[nodes_on_sfc[j]]->Y(); 
     pn[2] = m_pcs->m_msh->nod_vector[nodes_on_sfc[j]]->Z(); 
     node_value_vector[j] = 0.0;  
     Passed = false;
     // nodes close to first polyline 
     p = m_surface->polyline_of_surface_vector.begin();
     while(p!=m_surface->polyline_of_surface_vector.end()) {
        m_polyline = *p;
        // Grativity center of this polygon
        for(i=0; i<3; i++) gC[i] = 0.0;
        vn[2] = 0.0;
        nPointsPly = (int)m_polyline->point_vector.size();
        for(i=0; i<nPointsPly; i++)
        { 
            gC[0] += m_polyline->point_vector[i]->x;
            gC[1] += m_polyline->point_vector[i]->y;
            gC[2] += m_polyline->point_vector[i]->z;
            vn[2] += m_polyline->point_vector[i]->propert;
        } 
        for(i=0; i<3; i++) gC[i] /= (double)nPointsPly;
        // BC value at center is an average of all point values of polygon
        vn[2] /= (double)nPointsPly; 
        // Area of this polygon by the grativity center
        for(i=0; i<nPointsPly; i++)
        { 
            p1[0] = m_polyline->point_vector[i]->x;
            p1[1] = m_polyline->point_vector[i]->y;
            p1[2] = m_polyline->point_vector[i]->z;
            k = i+1;
            if(i==nPointsPly-1)
               k = 0;
            p2[0] = m_polyline->point_vector[k]->x;
            p2[1] = m_polyline->point_vector[k]->y;
            p2[2] = m_polyline->point_vector[k]->z;
            vn[0] =  m_polyline->point_vector[i]->propert;
            vn[1] =  m_polyline->point_vector[k]->propert;

            Area1 = fabs(ComputeDetTri(p1, gC, p2));

            Area2 = 0.0;
            // Check if pn is in the triangle by points (p1, gC, p2)
            Area2 = fabs(ComputeDetTri(p2, gC, pn));
            unit[0] = fabs(ComputeDetTri(gC, p1, pn));
            unit[1] = fabs(ComputeDetTri(p1, p2, pn));
            Area2 += unit[0]+unit[1];
            if(fabs(Area1-Area2)<Tol) 
            {
                // Intopolation whin triangle (p1,p2,gC)
                // Shape function
                for(l=0; l<2; l++)
                   unit[l] /= Area1;
                ShapeFunctionTri(NTri, unit);
                for(l=0; l<3; l++)
                  node_value_vector[j] += vn[l]*NTri[l];
                Passed = true;
                break;
            }
        
        }  
        //
        p++;
        if(Passed) break;
     }// while
  }//j
}   
