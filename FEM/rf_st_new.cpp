/**************************************************************************
FEMLib - Object: Source Terms ST
Task: 
Programing:
01/2004 OK Implementation
last modified
**************************************************************************/

#include "stdafx.h" // MFC
#include "makros.h"
#include "nodes.h"
// C++ STL
#include <fstream>
#include <iostream>
using namespace std;

#include "rfstring.h"
//WW #include "elements.h"
// FEM-Makros
#include "geo_lib.h"
#include "geo_strings.h"
#include "mathlib.h"

// GeoSys-GeoLib
// GeoSys-MshLib
#include "fem_ele.h"
#include "msh_elem.h"
#include "msh_lib.h"

#include "tools.h" //GetLineFromFile
/* Tools */
#ifndef NEW_EQS //WW. 06.11.2008
#include "matrix.h"
#endif

// GeoSys-FEMLib
//OK_IC #include "rfsousin.h"
#include "rf_st_new.h"

// Math
#include "matrix_class.h" 

#include "pcs_dm.h"

// For analytical source terms
#include "rf_mfp_new.h"

using FiniteElement::CElement;
using Mesh_Group::CElem;
using Mesh_Group::CEdge;
using Mesh_Group::CNode;
using Math_Group::vec;
#ifndef GRAVITY_CONSTANT
#define GRAVITY_CONSTANT 9.81
#endif

//==========================================================================
vector<CSourceTerm*> st_vector;
list<CSourceTermGroup*>st_group_list;
vector<string>analytical_processes;
vector<string>analytical_processes_polylines;
vector<NODE_HISTORY*> node_history_vector;//CMCD
/**************************************************************************
FEMLib-Method: 
Task: ST constructor
Programing:
01/2004 OK Implementation
**************************************************************************/
CSourceTerm::CSourceTerm(void)
{
  delimiter_type = " ";
  CurveIndex = -1;
  conditional = false;
  river = false;
  critical_depth = false;
  COUPLING_SWITCH = false;
  geo_node_value=0.0; 
  plyST = NULL; //OK
  nodes = NULL; //OK
  analytical = false;//CMCD
  display_mode = false; //OK
}

/**************************************************************************
FEMLib-Method: 
Task: BC deconstructor
Programing:
04/2004 OK Implementation
**************************************************************************/
CSourceTerm::~CSourceTerm(void) {
    DeleteHistoryNodeMemory();
    // PCS
    pcs_pv_name.clear();
    pcs_number = -1;
    // GEO
    geo_type = -1;
    geo_type_name.clear();
    geo_name.clear();
    geo_node_number = -1;
    geo_node_value = 0.0;
    msh_node_number = -1;
    // DIS
    dis_type = -1;
    dis_type_name.clear();
//    dis_file_name.clear();
    node_number_vector.clear();
    node_value_vector.clear();
    node_renumber_vector.clear();
    delimiter_type.clear();
    PointsHaveDistribedBC.clear();
    DistribedBC.clear();
    element_st_vector.clear();
    //WW----------22.02.2007-------------------
    int i, size; 
    size = (int)normal2surface.size(); 
    for(i=0; i<size; i++) delete normal2surface[i];
    size = (int)pnt_parameter_vector.size(); 
    for(i=0; i<size; i++) delete pnt_parameter_vector[i];
    //WW---------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: ST read function
Programing:
01/2004 OK Implementation
11/2004 MB neues read Konzept
02/2005 MB River condition 
03/2005 WW Node force released by excavation 
11/2005 CMCD Analytical source for matrix
04/2006 OK CPL
04/2006 OK MSH_TYPE
**************************************************************************/
ios::pos_type CSourceTerm::Read(ifstream *st_file)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  bool new_keyword = false;
  string hash("#");
  std::stringstream in;
  channel = 0, node_averaging = 0, no_surface_water_pressure = 0; // JOD 4.10.01
  ios::pos_type position;
  string sub_string,sub_string1;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while (!new_keyword) {
    position = st_file->tellg();
    if(!GetLineFromFile(line,st_file)) break;
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    remove_white_space(&line_string); //OK
    /* Keywords nacheinander durchsuchen */
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(st_file));
      in >> pcs_type_name;
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$PRIMARY_VARIABLE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(st_file));
      in >> pcs_pv_name;
	  in.clear();
	  continue;
     }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { 
      position  = ReadGeoType(st_file);
      continue;
    }
    //....................................................................
    if(line_string.find("$DIS_TYPE")!=string::npos)  //05.09.2008 WW
    { 
    //10.04.2008. WW  if(line_string.compare("$DIS_TYPE")==0) { 
      if(line_string.find("CONDITION")!=string::npos)  //05.09.2008 WW
      {
         conditional = true;
         position = ReadDistributionType(st_file);
         in.str(GetLineFromFile1(st_file)); //PCS type
         in >> line_string >> pcs_type_name_cond;
         in.clear();
         in.str(GetLineFromFile1(st_file)); // 
         in >> pcs_pv_name_cond;
         in.clear();
	     in.str(GetLineFromFile1(st_file));
         in >> coup_leakance >> rill_height;
         in.clear(); 
      }
      else   //05.09.2008 WW
      { 
        position = ReadDistributionType(st_file);
        continue;
      }
    }
	//..............................................
    //..............................................
    if(line_string.find("$COUPLING_SWITCH")!=string::npos) { 
      COUPLING_SWITCH = true;  // switch case 
	  in.str(GetLineFromFile1(st_file));
	  in >> rainfall >> rainfall_duration;
      in.clear();
	} 
	//..............................................
    if(line_string.find("$NODE_AVERAGING")!=string::npos) { 
      in.clear();
      node_averaging = 1;
      continue;
    } 
	//..............................................
    if(line_string.find("$NEGLECT_SURFACE_WATER_PRESSURE")!=string::npos) {  // JOD 4.10.01
      in.clear();
      no_surface_water_pressure = 1;
      continue;
    } 
    //..............................................
    if(line_string.find("$CHANNEL")!=string::npos) { 
      in.clear();
	  in.str(GetLineFromFile1(st_file));
      in >>channel_width;
      channel = 1;
      continue;
    } 
    //..............................................
    if(line_string.find("$TIM_TYPE")!=string::npos) { 
      in.str(GetLineFromFile1(st_file));
      in >> tim_type_name;
      if(tim_type_name.find("CURVE")!=string::npos) {
        dis_type = 0;
        in>>CurveIndex;
      }
      in.clear(); 
      continue;
	}
	//..............................................
    if(line_string.find("$FCT_TYPE")!=string::npos) { 
	  in.str(GetLineFromFile1(st_file));
      in >> fct_name; //sub_line
      if(fct_name.find("METHOD")!=string::npos) //WW
        in >> fct_method;   
      in.clear(); 
    }
    //....................................................................
    if(line_string.find("$MSH_TYPE")!=string::npos) { 
	  in.str(GetLineFromFile1(st_file));
      in >> sub_string; //sub_line
      msh_type_name = "NODE";
      if(sub_string.find("NODE")!=string::npos) {
	    in >> msh_node_number;
    	in.clear();
      }
	  continue;
    }
    //....................................................................
  } // end !new_keyword
  return position;
}
/**************************************************************************
FEMLib-Method: 
Task: for CSourceTerm::Read
Programing:
11/2007 JOD Implementation
02/2009 WW  Add a functionality to diectly assign source terms to element nodes.
**************************************************************************/
ios::pos_type CSourceTerm::ReadDistributionType(ifstream *st_file)
{

  std::stringstream in;
  ios::pos_type position;
	
  int abuff, nLBC=0;
  double bbuff;
  double cbuff;
  double dbuff;
  double ebuff;
  double fbuff;
 
  in.str(GetLineFromFile1(st_file));
  in >> dis_type_name;

   if(dis_type_name.find("CONSTANT")!=string::npos) {
        if(dis_type_name.find("NEUMANN")!=string::npos) 
          dis_type_name = "CONSTANT_NEUMANN";
        else   
          dis_type_name = "CONSTANT";
        in >> geo_node_value;
        in.clear();
      } 
    //....................................................................
     if(dis_type_name.find("ANALYTICAL")!=string::npos) {
        in >> analytical_material_group;//Which material group is it being applied to
        in >> analytical_diffusion;//D value
        in >> analytical_porosity;//n value of matrix
        in >> analytical_tortousity;//t value of matrix
        in >> analytical_linear_sorption_Kd;//Linear sorption coefficient
        in >> analytical_matrix_density;//Density of solid
        in >> number_of_terms;//no timesteps to consider in solution
        in >> resolution;//every nth term will be considered
        in >> factor;//to convert temperature to energy
        analytical = true;
        analytical_processes.push_back(pcs_pv_name);
        if(geo_type_name.compare("POLYLINE")==0) analytical_processes_polylines.push_back(geo_name);
        in.clear();
      }
	  //....................................................................
      if(dis_type_name.find("LINEAR")!=string::npos) {
        in >> nLBC;
        in.clear();
        for(int i=0; i<nLBC; i++)
        {
          in.str(GetLineFromFile1(st_file));
          in>>abuff>>bbuff;
          in.clear(); 
          PointsHaveDistribedBC.push_back(abuff);
          DistribedBC.push_back(bbuff);
        }
 //....................................................................
        if(dis_type_name.find("NEUMANN")!=string::npos) {
          dis_type_name = "LINEAR_NEUMANN";
        }
        else  {
          dis_type_name = "LINEAR";
        }
//      Read LINENODES AND VALUES......
        in.clear();
      }
	   //....................................................................
      if(dis_type_name.find("RIVER")!=string::npos) {
        river = true;
        in >> nLBC;
        in.clear();
        for(int i=0; i<nLBC; i++)  {
          in.str(GetLineFromFile1(st_file));
          // POINT   HRiver  KRiverBed WRiverBed TRiverBed BRiverBed
          in >>abuff >>bbuff >>cbuff   >>dbuff   >>ebuff   >>fbuff;
          in.clear(); 
          PointsHaveDistribedBC.push_back(abuff); //POINT
          DistribedBC.push_back(bbuff); //HRiver
          DistBC_KRiverBed.push_back(cbuff); //KRiverBed
          DistBC_WRiverBed.push_back(dbuff); //WRiverBed;
          DistBC_TRiverBed.push_back(ebuff); //TRiverBed;
          DistBC_BRiverBed.push_back(fbuff); //BRiverBed;
          //printf("\n %d %d %d %d %d %d", abuff, bbuff, cbuff, dbuff, ebuff, fbuff);
          geo_node_value = bbuff; //OK
        }
        dis_type_name = "RIVER";
        in.clear();
      }
	   //....................................................................
      if(dis_type_name.find("CRITICALDEPTH")!=string::npos) {
        dis_type_name = "CRITICALDEPTH";
        critical_depth = true;
        in >> geo_node_value;
        in.clear();
		in.str(GetLineFromFile1(st_file));
        in >> rill_height;
        in.clear();
      }
	   //....................................................................
	  if(dis_type_name.find("NORMALDEPTH")!=string::npos) {
        dis_type_name = "NORMALDEPTH";
        in >> geo_node_value;
        in.clear();
		in.str(GetLineFromFile1(st_file));
        in  >> normaldepth_slope >> rill_height;
        in.clear();
      }
	   //....................................................................
	  if(dis_type_name.find("PHILIP")!=string::npos) {
        dis_type_name = "PHILIP";
		in >> geo_node_value;
		in.clear();
		in.str(GetLineFromFile1(st_file));
        in >> sorptivity >> constant >> rainfall;
        in.clear();
      }
 //....................................................................
	  if(dis_type_name.find("GREEN_AMPT")!=string::npos) {
        dis_type_name = "GREEN_AMPT";
		in >> geo_node_value;
		in.clear();
		in.str(GetLineFromFile1(st_file));
        in >> sorptivity >> constant >> rainfall >> moistureDeficit;
        in.clear();
	  }
      // Soure terms are assign to element nodes directly. 23.02.2009. WW
	  if(dis_type_name.find("DIRECT")!=string::npos)
      {
        dis_type_name = "DIRECT";     
		in >> fname;
        fname = FilePath+fname;
		in.clear();
	  }

 return position;

}
/**************************************************************************
FEMLib-Method: 
Task: for CSourceTerm::Read
Programing:
11/2007 JOD Implementation
**************************************************************************/
ios::pos_type CSourceTerm::ReadGeoType(ifstream *st_file)
{

	ios::pos_type position;
string sub_string;
	std::stringstream in;


  in.str(GetLineFromFile1(st_file));
 in >>  geo_type_name;


 
      if(geo_type_name.find("POINT")!=string::npos) {
        geo_type = 0;
        in >> geo_name;
        geo_type_name = "POINT";
        in.clear();
      }
      if(geo_type_name.compare("LINE")==0) {
        geo_type = 1;
        in >> geo_id;
        geo_type_name = "LINE";
        in.clear();
      }
      if(geo_type_name.find("POLYLINE")!=string::npos) {
        geo_type = 1;
        in >> geo_name;
        geo_type_name = "POLYLINE";
        in.clear();
      }
      if(geo_type_name.find("SURFACE")!=string::npos) {
        geo_type = 2;
        in >> geo_name;
        geo_type_name = "SURFACE";
        in.clear();
      }
      if(geo_type_name.find("VOLUME")!=string::npos) {
        geo_type = 3;
        in >> geo_name;
        geo_type_name = "VOLUME";
        in.clear();
      }
      if(geo_type_name.find("DOMAIN")!=string::npos) {//CMCD for analytical function
        geo_type = 3;
        geo_type_name = "DOMAIN";
        in.clear();
      }
	  if(geo_type_name.find("COLUMN")!=string::npos) {
        geo_type = 0;
        in >> geo_name;
        geo_type_name = "COLUMN";
        in.clear();
      }
      if(pcs_pv_name.find("EXCAVATION")!=string::npos) //WW
      {
          in.str(GetLineFromFile1(st_file));
          in>>sub_string>>geo_type;      
          in.clear();
      } 

return position;

}

/**************************************************************************
FEMLib-Method: 
Task: ST read function
Programing:
01/2004 OK Implementation
**************************************************************************/
bool STRead(string file_base_name)
{
  //----------------------------------------------------------------------
//OK  STDelete();  
  //----------------------------------------------------------------------
  CSourceTerm *m_st = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  string st_file_name;
  ios::pos_type position;
  //========================================================================
  // File handling
  st_file_name = file_base_name + ST_FILE_EXTENSION;
  ifstream st_file (st_file_name.data(),ios::in);
  //if (!st_file.good()) return;

  if (!st_file.good()){
    cout << "! Warning in STRead: No source terms !" << endl;
    return false;
  }
  // Rewind the file
  st_file.clear();
  st_file.seekg(0L,ios::beg);
  //========================================================================
  // Keyword loop
  cout << "STRead" << endl;
  while (!st_file.eof()) {
    st_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos){//Code included to make dynamic memory for analytical solution
      int no_source_terms = (int) st_vector.size();
      int no_an_sol = 0;
      long number_of_terms = 0;
      int i;
      for (i=0; i < no_source_terms; i++){//Need to find the memory size limits for the anal. solution.
        if (st_vector[i]->analytical){
          no_an_sol++;
          number_of_terms = max(st_vector[i]->number_of_terms,number_of_terms);
        }
      }
      if (no_an_sol > 0) {
        for (i=0; i < no_source_terms; i++){
          st_vector[i]->no_an_sol = no_an_sol;
          st_vector[i]->max_no_terms = number_of_terms;
        }
      }
      return true;
      }
    //----------------------------------------------------------------------
    if(line_string.find("#SOURCE_TERM")!=string::npos) { // keyword found
      m_st = new CSourceTerm();
      position = m_st->Read(&st_file);
      st_vector.push_back(m_st);
      st_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  return true;
}


/**************************************************************************
FEMLib-Method: 
Task: ST to right-hand-side vector
Programing:
01/2004 OK Implementation
**************************************************************************/
void ST2RHS(string pcs_function,double* rhs_vector)
{
  int j;
  long i;
  long node_number_vector_length;
  CSourceTerm *m_st = NULL;
  long no_st =(long)st_vector.size();
  for(j=0;j<no_st;j++){
    m_st = st_vector[j];
    if(pcs_function.compare(m_st->pcs_pv_name)==0) {
      node_number_vector_length = (long)m_st->node_number_vector.size();
      for(i=0;i<node_number_vector_length;i++) {
        rhs_vector[m_st->node_number_vector[i]] = m_st->node_value_vector[i];
      }
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: ST to mesh nodes
Programing:
01/2004 OK Implementation
**************************************************************************/
void ST2NOD(void)
{
  long i;
  CSourceTerm *m_st = NULL;
  CGLPoint *m_point = NULL;
  CGLPolyline *m_polyline = NULL;
  long points_along_polyline;
  Surface *m_surface = NULL;
  long* nodes = NULL;
  long points_in_surface;
  CGLVolume *m_volume = NULL;
  long st_point_number;
  long geo_point_number;
  double st_value;

  //========================================================================
  // Nodes
  long j;
  long no_st =(long)st_vector.size();
  for(j=0;j<no_st;j++){
    m_st = st_vector[j];
    switch(m_st->geo_type) {
      case GS_POINT:
       m_point =  GEOGetPointByName(m_st->geo_prop_name);//CC
        m_st->node_number_vector.push_back(m_point->id);
        break;
      case GS_POLYLINE:
        m_polyline = GEOGetPLYByName(m_st->geo_prop_name);//CC
        points_along_polyline = (long)m_polyline->point_vector.size();
        for(i=0;i<points_along_polyline;i++) {
          m_point = m_polyline->point_vector[i];
          m_st->node_number_vector.push_back(m_point->id);
        }
        break;
      case GS_SURFACE:
m_surface = GEOGetSFCByName(m_st->geo_prop_name);//CC
        nodes = GetPointsIn(m_surface,&points_in_surface);
//MB patch areas
        for(i=0;i<points_in_surface;i++) {
          m_st->node_number_vector.push_back(nodes[i]);
        }
        break;
      case GS_VOLUME:
m_volume = GEOGetVOL(m_st->geo_prop_name);
        break;
    } // switch
  } // while
  //========================================================================
  // Values
  for(j=0;j<no_st;j++){
    m_st = st_vector[j];
    switch(m_st->dis_type) {
      case CONSTANT:
        st_point_number = (long)m_st->node_number_vector.size();
        for(i=0;i<st_point_number;i++) {
          m_st->node_value_vector.push_back(m_st->dis_prop[0]);
        }
        break;
      case LINEAR: // for polylines
        st_point_number = (long)m_st->node_number_vector.size();
        m_polyline = GEOGetPLYByName(m_st->geo_prop_name);//CC
        geo_point_number = (long)m_polyline->point_vector.size();
        //if(!(st_point_number==geo_point_number)) Warning !
        for(i=0;i<geo_point_number;i++) {
          m_point = m_polyline->point_vector[i];
          st_value = m_point->propert; // i.e. node property
          m_st->node_value_vector.push_back(st_value);
        }
        break;
    } // switch
  } // while
}

/**************************************************************************
FEMLib-Method: STWrite
Task: master write function
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
void STWrite(string base_file_name)
{
  CSourceTerm *m_st = NULL;
  string sub_line;
  string line_string;
  //========================================================================
  // File handling
  string st_file_name = base_file_name + ST_FILE_EXTENSION;
  fstream st_file (st_file_name.data(),ios::trunc|ios::out);
  st_file.setf(ios::scientific,ios::floatfield);
  st_file.precision(12);
  if (!st_file.good()) return;
  st_file.seekg(0L,ios::beg);
  //========================================================================
  st_file << "GeoSys-ST: Source Terms ------------------------------------------------\n";
  //========================================================================
  // ST
  long j;
  long no_st =(long)st_vector.size();
  for(j=0;j<no_st;j++){
    m_st = st_vector[j];
    m_st->Write(&st_file);
  }
  st_file << "#STOP";
  st_file.close();
}

/**************************************************************************
FEMLib-Method: 
Task: write function
Programing:
02/2004 OK Implementation
04/2005 OK PRIMARY_VARIABLE
06/2005 OK RIVER
last modification:
**************************************************************************/
void CSourceTerm::Write(fstream* st_file)
{
  //KEYWORD
  *st_file  << "#SOURCE_TERM" << endl;
  //--------------------------------------------------------------------
  //NAME+NUMBER
  *st_file << " $PCS_TYPE" << endl;
  *st_file << "  ";
  *st_file << pcs_type_name << endl;
  *st_file << " $PRIMARY_VARIABLE" << endl;
  *st_file << "  ";
  *st_file << pcs_pv_name << endl;
  //--------------------------------------------------------------------
  //GEO_TYPE
  *st_file << " $GEO_TYPE" << endl;
  *st_file << "  ";
  if(geo_type_name.compare("LINE")==0){
    *st_file << geo_type_name << delimiter_type << geo_id << endl;
  }
  else
  *st_file << geo_type_name << delimiter_type << geo_name << endl;
  //--------------------------------------------------------------------
  // TIM_TYPE
if(tim_type_name.size()>0){ //OK
  *st_file << " $TIM_TYPE" << endl;
  *st_file << "  ";
  *st_file << tim_type_name << endl;
}
  //--------------------------------------------------------------------
  //DIS_TYPE
  *st_file << " $DIS_TYPE" << endl;
  *st_file << "  ";
  *st_file << dis_type_name;
  switch(dis_type_name[0]) {
    case 'C': // Constant
      *st_file << delimiter_type << geo_node_value;
      *st_file << endl;
      break;
    case 'L': // Linear
	  *st_file << " " << (int)PointsHaveDistribedBC.size() << endl;
      for(long i=0;i<(long)PointsHaveDistribedBC.size();i++)
      {
	    *st_file << "  " << PointsHaveDistribedBC[i] << " ";
	    *st_file << "  " << DistribedBC[i] << endl;
      }
      break;
    case 'R': // RIVER
	  *st_file << " " << (int)PointsHaveDistribedBC.size() << endl;
      for(long i=0;i<(long)PointsHaveDistribedBC.size();i++){
	    *st_file << "  " << PointsHaveDistribedBC[i] << " ";
	    *st_file << "  " << DistribedBC[i] << " ";
	    *st_file << "  " << DistBC_KRiverBed[i] << " ";
	    *st_file << "  " << DistBC_WRiverBed[i] << " ";
	    *st_file << "  " << DistBC_TRiverBed[i] << " ";
	    *st_file << "  " << DistBC_BRiverBed[i] << endl;
      }
      break;
  }
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
04/2004 OK Implementation
11/2007 JOD Reaktivation
last modification:
**************************************************************************/
void CSourceTerm::SetDISType()
{
	
  if(dis_type_name.compare("CONSTANT")==0)         dis_type = 1;
  if(dis_type_name.compare("LINEAR")==0)           dis_type = 2;
  if(dis_type_name.compare("CONSTANT_NEUMANN")==0) dis_type = 3;
  if(dis_type_name.compare("LINEAR_NEUMANN")==0)   dis_type = 4;
  if(dis_type_name.compare("RIVER")==0)            dis_type = 5;
  if(dis_type_name.compare("CRITICALDEPTH")==0)    dis_type = 6;
  if(dis_type_name.compare("SYSTEM_DEPENDENT")==0) dis_type = 7;   //YD 
  if(dis_type_name.compare("NORMALDEPTH")==0)      dis_type = 8;  //JOD MB
  if(dis_type_name.compare("ANALYTICAL")==0)       dis_type = 9;//CMCD 02 2006
  if(dis_type_name.compare("PHILIP")==0)           dis_type = 10; // JOD 
  if(dis_type_name.compare("GREEN_AMPT")==0)       dis_type = 11; // JOD 

  // if(dis_type_name.compare("CONSTANT")==0) dis_type = 0;
  // if(dis_type_name.compare("LINEAR")  ==0) dis_type = 1;
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
void CSourceTerm::SetGEOType(void)
{
  if(geo_type_name.compare("POINT")) {
  }
  if(geo_type_name.compare("POLYLINE")) {
  //  CGLPolyline *m_polyline = NULL;
    plyST = GEOGetPLYByName(geo_name);//CC
    if(plyST)
      plyST->type = BC;
  }
  if(geo_type_name.compare("SURFACE")) {
  }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
CSourceTermGroup* CSourceTermGroup::Get(string this_pcs_name)
{
  CSourceTermGroup *m_group = NULL;
  list<CSourceTermGroup*>::const_iterator p_group = st_group_list.begin();
  while(p_group!=st_group_list.end()) {
    m_group = *p_group;
    if(m_group->pcs_name.find(this_pcs_name)!=string::npos) 
      return m_group;
    ++p_group;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method:
Task: set ST group member
Programing:
02/2004 OK Implementation
09/2004 WW Face integration of Neumann boundary condition for all element type
09/2004 WW Interpolation for piece-wise linear distributed source term or BC
03/2005 OK LINE sources
02/2005 MB River condition, CriticalDepth
08/2006 WW Re-implementing edge,face and domain integration versatile for all element types
04/2006 OK MSH types
02/2009 WW Direct assign node source terms
**************************************************************************/
void CSourceTermGroup::Set(CRFProcess* m_pcs, const int ShiftInNodeVector, string this_pv_name) {
 
  CSourceTerm *m_st = NULL;
  if(this_pv_name.size()!=0) //WW
    pcs_pv_name = this_pv_name;
  m_msh = FEMGet(m_pcs->pcs_type_name);
 
  if(m_msh) {  //WW
     /// In case of P_U coupling monolithic scheme
	 if(m_pcs->type==41) {//WW Mono
     
       if(pcs_pv_name.find("DISPLACEMENT")!=string::npos) //Deform 
          m_pcs->m_msh->SwitchOnQuadraticNodes(true);
       else 
          m_pcs->m_msh->SwitchOnQuadraticNodes(false);
     }
     else if(m_pcs->type==4)
        m_pcs->m_msh->SwitchOnQuadraticNodes(true);
	 else
        m_pcs->m_msh->SwitchOnQuadraticNodes(false);
	 //====================================================================
     long no_st =(long)st_vector.size();
     for(long i = 0; i < no_st;i++) {
       m_st = st_vector[i];
       if(m_st->conditional)
         m_msh_cond = FEMGet(m_st->pcs_type_name_cond);
       //
       //-- 23.02.3009. WW
       if(m_st->dis_type_name.find("DIRECT")!=string::npos)
       {
          m_st->DirectAssign(ShiftInNodeVector);
          continue;
       }
       //-----------
       if((m_st->pcs_type_name.compare(pcs_type_name)==0)&&(m_st->pcs_pv_name.compare(pcs_pv_name)==0)) {
		 m_st->m_pcs = m_pcs; // HS: 01.09.2009
         if(m_st->geo_type_name.compare("POINT")==0)
           SetPNT(m_pcs, m_st, ShiftInNodeVector);
         if(m_st->geo_type_name.compare("LINE")==0) 
	       SetLIN(m_pcs, m_st, ShiftInNodeVector);
         if(m_st->geo_type_name.compare("POLYLINE")==0) 
       	   SetPLY(m_st, ShiftInNodeVector);
         if(m_st->geo_type_name.compare("DOMAIN")==0)
		   SetDMN(m_st, ShiftInNodeVector);
         if(m_st->geo_type_name.compare("SURFACE")==0)  // "SURFACE" has to be changed to "FACE". WW
           SetSFC(m_st, ShiftInNodeVector);
		 if(m_st->geo_type_name.compare("COLUMN")==0)
		   SetCOL(m_st, ShiftInNodeVector);
         // MSH types //OK4310
         if(m_st->msh_type_name.compare("NODE")==0) 
			 m_st->SetNOD();
         // FCT types //YD
         if(m_st->fct_name.size()>0)
           fct_name = m_st->fct_name;
       } // end pcs name & pv
     } // end st loop
  } // end msh
  else
   cout << "Warning in CSourceTermGroup::Set - no MSH data" << endl;





}


/**************************************************************************
FEMLib-Method:
Task: 
Programing:
08/2004 OK Implementation
last modification:
**************************************************************************/
void STCreateFromPNT()
{
  long no_pnt_properties = (long)pnt_properties_vector.size();
  long i;
  CSourceTerm *m_st = NULL;
  CGLPoint *m_pnt = NULL;

  for(i=0;i<no_pnt_properties;i++){
    m_pnt = pnt_properties_vector[i];
    m_st = new CSourceTerm();
    // PCS ---------------------------------------------------------------
    m_st->pcs_pv_name = "PRESSURE1"; // ToDo
    // GEO ---------------------------------------------------------------
    m_st->geo_type_name = "POINT";
    m_st->geo_type = 0;
    m_st->geo_node_number = m_pnt->id;
    char char_string[20];
    sprintf(char_string,"POINT%ld",m_st->geo_node_number);
    m_st->geo_name = char_string; //OK4105
/*OK
    for(int j=0;j<no_points;j++){
      m_geo_pnt = gli_points_vector[j];
      if(m_geo_pnt->PointDisXY(m_pnt)<m_geo_pnt->mesh_density) // ToDo eps
        m_st->geo_node_number = m_geo_pnt->gli_point_id;
    }
*/
    // DIS ---------------------------------------------------------------
    m_st->dis_type_name = "CONSTANT";
    m_st->dis_type = 0;
    m_st->geo_node_value = m_pnt->value;
    // TIM ---------------------------------------------------------------
    m_st->tim_type_name = m_pnt->name;
    // VEC ---------------------------------------------------------------
    st_vector.push_back(m_st);
  }
  pnt_properties_vector.clear();
}



/**************************************************************************
ROCKFLOW - Funktion: FaceIntegration
Task: Translate distributed Neumann boundary condition /source term on edges
      found on a polyline to nodes value for all kinds of element 
Programming:
 07/2005 WW Re-Implementation
 12/2005 WW Axismmetry
**************************************************************************/
void CSourceTerm::EdgeIntegration(CFEMesh* msh, vector<long>&nodes_on_ply, 
                                vector<double>&node_value_vector)
{
  long i, j, k, l;
  long this_number_of_nodes;
  int elemsCnode;
  int nedges, ii;
  vec<CNode*> e_nodes(3);    
  vec<CEdge*> e_edges(12);    

  double Jac = 0.0;    
  double Weight = 0.0;
  double eta = 0.0;   
  double v1,v2, radius = 0.0;   
  double Shfct[3];
  bool Const = false;

 
  if(dis_type_name.find("CONSTANT")!=string::npos)
     Const = true;

  //CFEMesh* msh = m_pcs->m_msh;
  //CFEMesh* msh;  // JOD
  //msh = FEMGet(pcs_type_name);
  CElem* elem = NULL;
  CEdge* edge = NULL;
  CNode* node = NULL;

  int nSize = (long)msh->nod_vector.size();
  this_number_of_nodes = (long)nodes_on_ply.size();
  vector<long> G2L(nSize);
  vector<double> NVal(this_number_of_nodes);

  // Unmakr edges.
  for (i = 0; i < (long)msh->edge_vector.size(); i++)
     msh->edge_vector[i]->SetMark(false);
  for (i = 0; i < nSize; i++)
  {
      msh->nod_vector[i]->SetMark(false);
      G2L[i] = -1;
  }

  // Search edges on polyline
  for (i = 0; i <this_number_of_nodes; i++)
  {
     NVal[i] = 0.0;
     k = nodes_on_ply[i];
     G2L[k] = i;
     node = msh->nod_vector[k];
     elemsCnode=(int)node->connected_elements.size();      
     for(j=0; j<elemsCnode; j++)
	 {
		 l =  msh->nod_vector[k]->connected_elements[j];  
         elem = msh->ele_vector[l];
		 nedges = elem->GetEdgesNumber();
		 elem->GetEdges(e_edges);
         for(ii=0; ii<nedges; ii++)
		 {
             edge = e_edges[ii];
			 if(edge->GetMark()) continue;
			 edge->GetNodes(e_nodes);     
             // Edge A
             if(*node==*e_nodes[0])
                 e_nodes[0]->SetMark(true);
             // Edge B
             if(*node==*e_nodes[1])
                 e_nodes[1]->SetMark(true);
			 if(msh->getOrder()) // Quadratic
			 {
               if(*node==*e_nodes[2])
                  e_nodes[2]->SetMark(true);
			 }  
			 if(e_nodes[0]->GetMark()&&e_nodes[1]->GetMark())
			 {
                 if(msh->getOrder())
				 {
                    if(e_nodes[2]->GetMark()) edge->SetMark(true); 
				 }
				 else
                   edge->SetMark(true); 
			 }

		 }// e_edges               
     }
  }

  for (i = 0; i < (long)msh->edge_vector.size(); i++)
  {
      edge = msh->edge_vector[i];
	  if(!edge->GetMark()) continue;
      edge->GetNodes(e_nodes);
	  if(msh->getOrder()) // Quad
	  {
         if(e_nodes[0]->GetMark()
             &&e_nodes[1]->GetMark()&&e_nodes[2]->GetMark())
	     {
            Jac = 0.5*edge->Length();
			v1 = node_value_vector[G2L[e_nodes[0]->GetIndex()]]; 
            v2 = node_value_vector[G2L[e_nodes[1]->GetIndex()]];
			if(Const&&(!msh->isAxisymmetry()))
            {
                NVal[G2L[e_nodes[0]->GetIndex()]] += Jac*v1/3.0;
                NVal[G2L[e_nodes[1]->GetIndex()]] += Jac*v1/3.0;  
                NVal[G2L[e_nodes[2]->GetIndex()]] += 4.0*Jac*v1/3.0;  
                
            }
            else
            {
                for(k=0; k<3; k++) // Three nodes 
                {
                    // Numerical integration   
                    for(l=0; l<3; l++) // Gauss points 
                    {
                         Weight = Jac*MXPGaussFkt(3, l);                                
                         eta = MXPGaussPkt(3, l);
                         ShapeFunctionLineHQ(Shfct, &eta);
                         //Axisymmetical problem
						 if(msh->isAxisymmetry()) 
		                 {
                            radius = 0.0;
                            for(ii=0; ii<3; ii++) 
                               radius += Shfct[ii]*e_nodes[ii]->X();                             
                            Weight *= radius; //2.0*pai*radius;
                         }
                         NVal[G2L[e_nodes[k]->GetIndex()]] += 
							  0.5*(v1+v2+eta*(v2-v1))*Shfct[k]*Weight;         
                     }                                           
                 }
            }
         }          
      }
      else  // Linear
      {
         if(e_nodes[0]->GetMark()&&e_nodes[1]->GetMark())
	     {
            Jac = 0.5*edge->Length();
			v1 = node_value_vector[G2L[e_nodes[0]->GetIndex()]]; 
            v2 = node_value_vector[G2L[e_nodes[1]->GetIndex()]];
            if(!msh->isAxisymmetry())
			{
              if(Const)
              {
                  NVal[G2L[e_nodes[0]->GetIndex()]] += Jac*v1;
                  NVal[G2L[e_nodes[1]->GetIndex()]] += Jac*v1;  
              }
              else
              {
                  NVal[G2L[e_nodes[0]->GetIndex()]]
                      += Jac*(2.0*v1+v2)/3.0;
                  NVal[G2L[e_nodes[1]->GetIndex()]]
                      += Jac*(v1+2.0*v2)/3.0;                        
              }
			}
			else // Axisymmetry
			{

               for(k=0; k<2; k++) // Three nodes 
                {
                    // Numerical integration   
                    for(l=0; l<3; l++) // Gauss points 
                    {
                         Weight = Jac*MXPGaussFkt(3, l);                                
                         eta = MXPGaussPkt(3, l);
                         ShapeFunctionLine(Shfct, &eta);
                         //Axisymmetical problem
						 if(msh->isAxisymmetry()) 
		                 {
                            radius = 0.0;
                            for(ii=0; ii<2; ii++) 
                               radius += Shfct[ii]*e_nodes[ii]->X();                             
                            Weight *= radius; //2.0*pai*radius;
                         }
                         NVal[G2L[e_nodes[k]->GetIndex()]] += 
							  0.5*(v1+v2+eta*(v2-v1))*Shfct[k]*Weight;         
                     }                                           
                 }
			}// End of is (!axi)
		 }           
      }      
  }
  for (i = 0; i <this_number_of_nodes; i++)
    node_value_vector[i] = NVal[i];
  for (i = 0; i < (long)msh->edge_vector.size(); i++)
     msh->edge_vector[i]->SetMark(true);
  for (i = 0; i < nSize; i++)
      msh->nod_vector[i]->SetMark(true);
  NVal.clear();
  G2L.clear();
  e_nodes.resize(0);    
  e_edges.resize(0);    

}
/**************************************************************************
ROCKFLOW - Funktion: FaceIntegration
Task: Translate distributed Neumann boundary condition /source term on faces
      found on a surface into nodes value for all kinds of element 
Programming:
 08/2005 WW Re-Implementation
11/2005 WW/OK Layer optimization
**************************************************************************/
void CSourceTerm::FaceIntegration(CFEMesh* msh, vector<long>&nodes_on_sfc, 
                                vector<double>&node_value_vector)
{
  if(!msh)
  {
    cout << "Warning in CSourceTerm::FaceIntegration: no MSH data, function doesn't function";
    return;
  }

  long i, j, k, l;
  long this_number_of_nodes;
  int nfaces, nfn;
  int nodesFace[8];
  double nodesFVal[8];

  bool Const = false;
  if(dis_type_name.find("CONSTANT")!=string::npos)
     Const = true;
  //----------------------------------------------------------------------
  // Interpolation of polygon values to nodes_on_sfc
  if(!Const)  // Get node BC by interpolation with surface
  {
     int nPointsPly = 0;
     double Area1, Area2;
     double Tol = 1.0e-9;
     bool Passed;
     const int Size = (int)nodes_on_sfc.size();
     double gC[3],p1[3],p2[3], pn[3], vn[3],unit[3], NTri[3];

     CGLPolyline* m_polyline = NULL;
     Surface *m_surface = NULL;
     m_surface = GEOGetSFCByName(geo_name);//CC

    // list<CGLPolyline*>::const_iterator p = m_surface->polyline_of_surface_list.begin();
    vector<CGLPolyline*>::iterator p = m_surface->polyline_of_surface_vector.begin();

     for(j=0; j<Size; j++)
     {
        pn[0] = msh->nod_vector[nodes_on_sfc[j]]->X(); 
        pn[1] = msh->nod_vector[nodes_on_sfc[j]]->Y(); 
        pn[2] = msh->nod_vector[nodes_on_sfc[j]]->Z(); 
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


  int Axisymm = 1; // ani-axisymmetry
  //CFEMesh* msh = m_pcs->m_msh;
  if(msh->isAxisymmetry()) Axisymm = -1; // Axisymmetry is true
  CElem* elem = NULL;
  CElem* face = new CElem(1);
  CElement* fem = new CElement(Axisymm*msh->GetCoordinateFlag());
  CNode* e_node = NULL;
  CElem* e_nei = NULL;
  //vec<CNode*> e_nodes(20);
 // vec<CElem*> e_neis(6);

  face->SetFace();
  this_number_of_nodes = (long)nodes_on_sfc.size();
  int nSize = (long)msh->nod_vector.size();
  vector<long> G2L(nSize);
  vector<double> NVal(this_number_of_nodes);

  for (i = 0; i < nSize; i++)
  {
      msh->nod_vector[i]->SetMark(false);
      G2L[i] = -1;
  }

  for (i = 0; i <this_number_of_nodes; i++)
  {
     NVal[i] = 0.0;
     k = nodes_on_sfc[i];
     G2L[k] = i;
  }
 

  int count;
  double fac=1.0;
  for (i = 0; i < (long)msh->ele_vector.size(); i++)
  {
      elem = msh->ele_vector[i];
      if(!elem->GetMark()) continue;
	  nfaces = elem->GetFacesNumber();
	  elem->SetOrder(msh->getOrder());
      for(j=0; j<nfaces; j++)
      { 
         e_nei =  elem->GetNeighbor(j);
		 nfn = elem->GetElementFaceNodes(j, nodesFace);
         count=0;
         for(k=0; k<nfn; k++)
         { 
            e_node = elem->GetNode(nodesFace[k]);
            for (l = 0; l <this_number_of_nodes; l++)
            {
               if(*e_node==*msh->nod_vector[nodes_on_sfc[l]])
               {
                   count++;
                   break;
               }
			}
		 }
         if(count!=nfn) continue; 
         for(k=0; k<nfn; k++)
		 { 
            e_node = elem->GetNode(nodesFace[k]);
            nodesFVal[k] = node_value_vector[G2L[e_node->GetIndex()]];
		 }
		 fac = 1.0;
		 if(elem->GetDimension()==e_nei->GetDimension()) // Not a surface face 
	        fac = 0.5;
		 face->SetFace(elem, j);
		 face->SetOrder(msh->getOrder());
		 face->ComputeVolume();
         fem->setOrder(msh->getOrder()+1);
		 fem->ConfigElement(face, true);
		 fem->FaceIntegration(nodesFVal);
         for(k=0; k<nfn; k++)
		 {
            e_node = elem->GetNode(nodesFace[k]);
            NVal[G2L[e_node->GetIndex()]] += fac*nodesFVal[k];      
		 }
      }      
  }   

  for (i = 0; i <this_number_of_nodes; i++)
    node_value_vector[i] = NVal[i];
  for (i = 0; i < nSize; i++)
     msh->nod_vector[i]->SetMark(true);

  NVal.clear();
  G2L.clear();
  delete fem;
  delete face;
}

/**************************************************************************
ROCKFLOW - Funktion: DomainIntegration
Task:  Translate distributed source term within elements into nodes value 
       for all kinds of element 
Programming:
 08/2005 WW Re-Implementation
**************************************************************************/
void CSourceTerm::DomainIntegration(CFEMesh* msh, vector<long>&nodes_in_dom, 
                                    vector<double>&node_value_vector)
{
  long i, j, k;
  int nn;
  long this_number_of_nodes;
  double nodesFVal[8];

  bool Const = false;
 
  if(dis_type_name.find("CONSTANT")!=string::npos)
     Const = true;

  int Axisymm = 1; // ani-axisymmetry
  //CFEMesh* msh = m_pcs->m_msh;
  //CFEMesh* msh;  // JOD
  //msh = FEMGet(pcs_type_name);
  if(msh->isAxisymmetry()) Axisymm = -1; // Axisymmetry is true
  CElem* elem = NULL;
  CElement* fem = new CElement(Axisymm*msh->GetCoordinateFlag());
  vec<CNode*> e_nodes(20);

  this_number_of_nodes = (long)nodes_in_dom.size();
  int nSize = (long)msh->nod_vector.size();
  vector<long> G2L(nSize);
  vector<double> NVal(this_number_of_nodes);

  for (i = 0; i < nSize; i++)
  {
      msh->nod_vector[i]->SetMark(false);
      G2L[i] = -1;
  }

  for (i = 0; i <this_number_of_nodes; i++)
  {
     NVal[i] = 0.0;
     k = nodes_in_dom[i];
     G2L[k] = i;
  }
 

  int count=0;
  for (i = 0; i < (long)msh->ele_vector.size(); i++)
  {
      elem = msh->ele_vector[i];
      if(!elem->GetMark()) continue;
	  elem->GetNodes(e_nodes);
      nn= elem->GetNodesNumber(msh->getOrder());
      count=0;
	  for(j=0; j<nn; j++)
      { 
         for (k = 0; k <this_number_of_nodes; k++)
         {
            if(*e_nodes[j]==*msh->nod_vector[nodes_in_dom[k]])
            {
                count++;
                break;
            }
         }
	  }
      if(count!=nn) continue; 
      for(j=0; j<nn; j++)
          nodesFVal[j] = node_value_vector[G2L[e_nodes[j]->GetIndex()]];
      fem->ConfigElement(elem, true);
	  fem->setOrder(msh->getOrder()+1);
      fem->FaceIntegration(nodesFVal);
      for(j=0; j<nn; j++)
          NVal[G2L[e_nodes[j]->GetIndex()]] += nodesFVal[j];       
	  }

  for (i = 0; i <this_number_of_nodes; i++)
    node_value_vector[i] = NVal[i];
  for (i = 0; i < nSize; i++)
    msh->nod_vector[i]->SetMark(true);

  NVal.clear();
  G2L.clear();
  e_nodes.resize(0);
  delete fem;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void STDelete()
{
  long i;
  int no_st =(int)st_vector.size();
  for(i=0;i<no_st;i++){
    delete st_vector[i];
  }
  st_vector.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void STGroupsDelete()
{
  CSourceTermGroup* m_st_group = NULL;
  list<CSourceTermGroup*>::const_iterator p=st_group_list.begin();
  while (p!=st_group_list.end()){
    m_st_group = *p;
    delete m_st_group;
    //st_group_list.remove(*p);
    ++p;
  }
  st_group_list.clear();
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
03/2005 OK Implementation
last modification:
**************************************************************************/
void STCreateFromLIN(vector<CGLLine*>lin_properties_vector)
{
  long i;
  CSourceTerm *m_st = NULL;
  CGLLine *m_lin = NULL;
  long lin_properties_vector_size = (long)lin_properties_vector.size();
  for(i=0;i<lin_properties_vector_size;i++){
    m_st = new CSourceTerm();
    m_lin = lin_properties_vector[i];
    m_st->pcs_pv_name = "PRESSURE1"; // ToDo
    m_st->geo_type_name = "LINE";
    m_st->geo_name = m_lin->name;
    m_st->geo_id = m_lin->gli_line_id;
    m_st->dis_type_name = "CONSTANT_NEUMANN";
    m_st->geo_node_value = m_lin->value;
    m_st->tim_type_name = m_lin->name;
    st_vector.push_back(m_st);
  }
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
03/2005 OK Implementation
last modification:
**************************************************************************/
CSourceTerm* STGet(string geo_name)
{
  int i;
  int st_vector_size =(int)st_vector.size();
  CSourceTerm* m_st = NULL;
  for(i=0;i<st_vector_size;i++){
    m_st = st_vector[i];
    if(m_st->geo_name.compare(geo_name)==0)
      return m_st;
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
CSourceTermGroup* STGetGroup(string pcs_type_name,string pcs_pv_name)
{
  CSourceTermGroup *m_st_group = NULL;
  list<CSourceTermGroup*>::const_iterator p_st_group = st_group_list.begin();
  while(p_st_group!=st_group_list.end()) {
    m_st_group = *p_st_group;
    if((m_st_group->pcs_type_name.compare(pcs_type_name)==0)&&\
       (m_st_group->pcs_pv_name.compare(pcs_pv_name)==0))
      return m_st_group;
    ++p_st_group;
  }
  return NULL;
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB Implementation
**************************************************************************/
double GetConditionalNODValue(CSourceTerm* m_st, CNodeValue* cnodev) //WW
{
  int nidx;
  double value_cond = 0.0;
  double NodeReachLength;
  CRFProcess* m_pcs_cond = NULL;
  long node_cond;

  m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);

//WW  node_cond = group_vector[i]->msh_node_number_conditional;
  node_cond = cnodev->msh_node_number_conditional; //WW
  nidx = m_pcs_cond->GetNodeValueIndex(m_st->pcs_pv_name_cond)+1;
  value_cond = m_pcs_cond->GetNodeValue(node_cond,nidx);
 
  if(m_st->pcs_pv_name_cond.find("FLUX")!=string::npos){
//WW    NodeReachLength = group_vector[i]->node_value;
    NodeReachLength = cnodev->node_value; //WW
    value_cond = value_cond * NodeReachLength;
  }

  return value_cond;
}
/**************************************************************************
FEMLib-Method:
Task: Calculate Philips (1957) two-term infiltration flux term
      calculated separately for each node
Programing:
05/2007 JOD Implementation

**************************************************************************/
void GetPhilipNODValue(double &value,CSourceTerm* m_st)
{
  double area;
  double infiltration,sorptivity, constant, availableWater;

area = value;

  sorptivity = m_st->sorptivity;
  constant = m_st->constant;
  availableWater = m_st->rainfall;

  infiltration = constant + sorptivity / sqrt(aktuelle_zeit);

  infiltration = min(availableWater, infiltration);    
  infiltration *= -area;
  
  value = infiltration;

}
/**************************************************************************
FEMLib-Method:
Task: Calculate Green-Ampt infiltration flux term
      for homogeneous soil, includes water depth
	  writes cumulative infiltration in COUPLINGFLUX
	  solution is sensitive to time step 
	  infiltration is estimated with first order derivative
	  of cumulative infiltration
Programing:
05/2007 JOD Implementation

**************************************************************************/
void GetGreenAmptNODValue(double &value, CSourceTerm* m_st, long msh_node)
{
 
   double F, Fiter, Fold, infiltration;
   double conductivity, suction, Theta, wdepth;
   double a, b;
   CFEMesh* m_msh = NULL;
   CRFProcess* m_pcs_this = NULL;
   m_pcs_this = PCSGet(m_st->pcs_type_name);
   m_msh = m_pcs_this->m_msh;
   
   double area = value ;
  
   wdepth = max(0.,m_pcs_this->GetNodeValue(msh_node, m_pcs_this->GetNodeValueIndex("HEAD") + 1)- m_msh->nod_vector[msh_node]->Z() );
   conductivity = m_st->constant;
   suction = m_st->sorptivity + wdepth; // water depth included
   Theta = m_st->moistureDeficit * suction;
 
   Fold = m_pcs_this->GetNodeValue(msh_node, m_pcs_this->GetNodeValueIndex("COUPLING"));
   F = Fold;
  
   do {  // Newton iteration loop
	  Fiter = F;
	  if(Fiter == 0) // avoids a = 0 
		  Fiter = 1.e-5;
   a = 1- Theta / (Fiter + Theta);
   
   b = Fold - Fiter + Theta * log((Fiter + Theta)/(Fold+ Theta)) + conductivity * dt; // dt = timeStep
   F = Fiter + b / a;
   }while( fabs(F - Fiter) >1.e-10 );

   infiltration = (F - Fold) / dt;

   if( infiltration  >  m_st->rainfall)// + wdepth / timestep )   // compare with available water
     infiltration = m_st->rainfall;//  +  wdepth / timestep ;
  
   F = infiltration * dt + Fold;
   
  
   m_pcs_this->SetNodeValue(msh_node, m_pcs_this->GetNodeValueIndex("COUPLING") +1, F); // output is cumulative infiltration
 
   infiltration *= -area;
   value = infiltration;

}


/**************************************************************************
FEMLib-Method:
Task: Calculate coupling flux
      Mutual coupling of overland, Richards and groundwater flow
Programing:
01/2007 JOD Implementation
**************************************************************************/
#ifndef NEW_EQS //WW. 06.11.2008
void GetCouplingNODValue(double &value, CSourceTerm* m_st, CNodeValue* cnodev)
{

  if(m_st->COUPLING_SWITCH == true) { // alternatively mixed boundary cond/ source term coupling
      GetCouplingNODValueMixed(value, m_st, cnodev);  
      return;
	}

  if( m_st->pcs_type_name == "GROUNDWATER_FLOW" ||  m_st->pcs_type_name == "RICHARDS_FLOW" )
    GetCouplingNODValuePicard(value, m_st, cnodev);
  else if( m_st->pcs_type_name == "OVERLAND_FLOW" )
    GetCouplingNODValueNewton(value, m_st, cnodev);
  else 
    cout << "Error in GetCouplingNODValue";

}
#endif
/**************************************************************************
FEMLib-Method:
Task: Calculate coupling flux for GetCouplingNODValue
      for the soil or groundwater flow case
	  prerequisite: fluid data in mfp_vector[0] (2 times)
Programing:
01/2007 JOD Implementation
10/2008 JOD overland node shifting for soil columns, averaging automatically 4.7.10
**************************************************************************/
#ifndef NEW_EQS //WW. 06.11.2008
void GetCouplingNODValuePicard(double &value, CSourceTerm* m_st, CNodeValue* cnodev)
{

  double relPerm, condArea, gamma;
  CRFProcess* m_pcs_cond = NULL;
  CRFProcess* m_pcs_this = NULL;
  double h_this, h_cond, z_this, z_cond, h_cond_shifted, help;
  
  gamma =  mfp_vector[0]->Density() * GRAVITY_CONSTANT; // only one phase 
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);
  GetCouplingFieldVariables(&h_this, &h_cond, &h_cond_shifted, &help, &z_this, &z_cond, m_st, cnodev); // z_cond shifted for soil columns
  ////////   coupling factor
  if( h_this > h_cond_shifted ) 
       relPerm = GetRelativeCouplingPermeability(m_pcs_this, h_this, m_st, cnodev->msh_node_number); // groundwater or richards
  else 
       relPerm = GetRelativeCouplingPermeability(m_pcs_cond, h_cond, m_st, cnodev->msh_node_number_conditional); // overland

  condArea =  value * relPerm; // leakance in relPerm
  if(m_st->channel)  // wetted perimeter, pcs_cond has to be overland flow 
     condArea *= m_st->channel_width + (h_cond - z_cond);
  ///////   source term & matrix
  value = CalcCouplingValue( condArea, h_this, h_cond_shifted, z_cond, m_st);  // hydrostatic overland pressure

  if(m_st->pcs_type_name == "RICHARDS_FLOW")
    condArea /= gamma ;

  if(m_st->pcs_type_name == "GROUNDWATER_FLOW" && h_this < z_cond && m_st->pcs_type_name_cond == "OVERLAND_FLOW")
    condArea = 0 ;
    
  MXInc(cnodev->msh_node_number,cnodev->msh_node_number, condArea);  // groundwater / soil pressure
  /////  output
  m_pcs_this->SetNodeValue(cnodev->msh_node_number, m_pcs_this->GetNodeValueIndex( "COUPLING") +1, h_this);// update coupling variable for error estimation
  
  if(m_st->no_surface_water_pressure)  // 4.10.01
    value = 0;

}
#endif
/**************************************************************************
FEMLib-Method:
Task: Calculate coupling flux for GetCouplingNODValue
      for the overland flow case
	  prerequisite: fluid data in mfp_vector[0]
Programing:
01/2007 JOD Implementation
10/2008 JOD node shifting for soil columns 4.7.10
**************************************************************************/
#ifndef NEW_EQS //WW. 06.11.2008
void GetCouplingNODValueNewton(double &value, CSourceTerm* m_st, CNodeValue* cnodev)
{
  double relPerm, area, condArea;
  CRFProcess* m_pcs_cond = NULL;
  CRFProcess* m_pcs_this = NULL;
  double h_this, h_cond, z_this, z_cond, h_this_shifted, h_this_averaged;
  double epsilon = 1.e-7, value_jacobi, h_this_epsilon, relPerm_epsilon, condArea_epsilon;// epsilon as in pcs->assembleParabolicEquationNewton
 
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);
  GetCouplingFieldVariables(&h_this, &h_cond, &h_this_shifted, &h_this_averaged, &z_this, &z_cond, m_st, cnodev);// z_this shifted for soil columns
  ///// factor
  if( h_this_shifted > h_cond )  	
    relPerm = GetRelativeCouplingPermeability(m_pcs_this, h_this, m_st, cnodev->msh_node_number);  // overland
  else   
    relPerm = GetRelativeCouplingPermeability(m_pcs_cond, h_cond, m_st, cnodev->msh_node_number_conditional);// richards or groundwater

  area  = value; // for condArea_epsilon
  condArea = value * relPerm; // leakance in relPerm
  if(m_st->channel)  // wetted perimeter 
    condArea *= m_st->channel_width + (h_this - z_this);
  //////  source term
  value = CalcCouplingValue(condArea, h_this_averaged, h_cond, z_cond, m_st);
  /////   epsilon shift for jacobian 
  if( m_st->node_averaging ) {
	for( long i = 0; i < (long)cnodev->msh_node_numbers_averaging.size(); i++)
    	if( cnodev->msh_node_numbers_averaging[i] ==  cnodev->msh_node_number) 
           h_this_epsilon = h_this_averaged + epsilon * cnodev->msh_node_weights_averaging[i];
  }
  else
     h_this_epsilon = h_this + epsilon;
  /////  factor 
  if(h_this_shifted + epsilon > h_cond) 
    relPerm_epsilon = GetRelativeCouplingPermeability(m_pcs_this, h_this + epsilon, m_st, cnodev->msh_node_number);
  else   
    relPerm_epsilon = GetRelativeCouplingPermeability(m_pcs_cond, h_cond, m_st, cnodev->msh_node_number_conditional);// richards or groundwater

  condArea_epsilon =  area * relPerm_epsilon;
  if(m_st->channel)  // wetted perimeter 
    condArea_epsilon *= m_st->channel_width + (h_this_epsilon - z_this);

  if(m_st->no_surface_water_pressure) // 4.10.01
    h_this_epsilon = z_this;
  /////  jacobian	
  value_jacobi = - condArea_epsilon *( h_cond - h_this_epsilon) + value; 
  MXInc(cnodev->msh_node_number,cnodev->msh_node_number, value_jacobi / epsilon);
  /////  output
  m_pcs_this->SetNodeValue(cnodev->msh_node_number, m_pcs_this->GetNodeValueIndex("COUPLING") + 1, -value / area); // coupling flux (m/s)

}
#endif
/**************************************************************************
FEMLib-Method:
Task: Calculate relative coupling permeability for GetCouplingNODValue(***).
Programing: prerequisites: phase 0 in mfp
06/2007 JOD Implementation
10/2008 JOD include leakance 4.7.10
**************************************************************************/

double GetRelativeCouplingPermeability(CRFProcess* m_pcs, double head, CSourceTerm* m_st, long msh_node)
{
  double relPerm , z, sat;
  

  z = m_pcs->m_msh->nod_vector[msh_node]->Z();

	if( m_pcs->pcs_type_name  == "OVERLAND_FLOW") { 
	       
   			sat =  (head - z) / max(1.e-6, m_st->rill_height);
	       
			 if( sat > 1 )
				 relPerm = 1;
			 else if( sat < 0 )
				 relPerm = 0;
			 else
			     relPerm = pow(sat, 2*(1- sat));
		}
	 else if( m_pcs->pcs_type_name == "GROUNDWATER_FLOW")
            relPerm = 1;
	 else if( m_pcs->pcs_type_name == "RICHARDS_FLOW") {
         
            /*CElem *m_ele = NULL;
			long msh_ele;
			int group;
            double gamma =  mfp_vector[0]->Density() * GRAVITY_CONSTANT; // only one phase  
          
		    msh_ele = m_pcs->m_msh->nod_vector[msh_node]->connected_elements[0]; 
            m_ele = m_pcs->m_msh->ele_vector[msh_ele];
            group = m_pcs->m_msh->ele_vector[msh_ele]->GetPatchIndex();	
		  
			sat = mmp_vector[group]->SaturationCapillaryPressureFunction( -(head - z) * gamma, 0);  
            relPerm = mmp_vector[group]->PermeabilitySaturationFunction(sat,0);*/
			
			relPerm = 1; // JOD 4.10.01
	 }
	 else {
            cout << "!!!!! Coupling flux upwinding not implemented for this case !!!!!" << endl;
            relPerm = 1;
	 }


   return relPerm * m_st->coup_leakance;
}
/**************************************************************************
FEMLib-Method:
Task: Coupling of overland and soil flow by using water depth as soil boundary
      condition and flux term as overland source term according to 
	  Morita and Yen, J. Hydr. Eng. 184, 2002
Programing: prerequisites: constant precipitation with assigned duration, 
            phase = 0 in mfp, soil data in mmp_vetor[1] !!!!!
06/2007 JOD Implementation
**************************************************************************/
#ifndef NEW_EQS //WW. 06.11.2008
void GetCouplingNODValueMixed(double& value, CSourceTerm* m_st, CNodeValue* cnodev) 
{

	 double cond1, cond0, pressure1, pressure0, bc_value, depth, gamma, sat, area;
     double leakance, deltaZ;
	 int phase = 0;  // RESTRICTION for mfp !!!!!!!
   
	 CElem *m_ele = NULL;
	 long msh_ele;
	 int group, nidx;
     CRFProcess* m_pcs_cond = NULL;
     CRFProcess* m_pcs_this = NULL;
     m_pcs_this = PCSGet(m_st->pcs_type_name);
     m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);
	
     area = value;
     leakance = m_st->coup_leakance;
     deltaZ = m_st->rill_height;
     gamma =  mfp_vector[0]->Density() * GRAVITY_CONSTANT;  // phase  = 0 !!!!
     long  msh_node_2nd;
	 double x_this = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->X();
     double y_this = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->Y();
     double z_this = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->Z();
 
     msh_node_2nd = -1; //WW

     cond0 =  leakance *  deltaZ; 
     cond1 = cond0;

     if(m_st->pcs_type_name.compare("OVERLAND_FLOW")==0  ) {

///// get number of second mesh node, provisional implementation
	 double epsilon = 1.e-5;
	 double x_cond = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->X();
     double y_cond = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->Y();
     double z_cond = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->Z();
	 
	 for(int i=0;i<(long)m_pcs_cond->m_msh->nod_vector.size();i++) {  
        if(m_pcs_cond->m_msh->nod_vector[i]->X() - x_cond < epsilon)
          if(m_pcs_cond->m_msh->nod_vector[i]->Y() - y_cond < epsilon)
            if(m_pcs_cond->m_msh->nod_vector[i]->Z() - (z_cond - deltaZ) < epsilon)
              msh_node_2nd = i; 
	 }
//////////////////////////

	     nidx = m_pcs_cond->GetNodeValueIndex("PRESSURE1")+1;
	      
		 pressure0 = m_pcs_cond->GetNodeValue(cnodev->msh_node_number_conditional,nidx);
		 pressure1 = m_pcs_cond->GetNodeValue(msh_node_2nd,nidx);
        		
         
		 double gamma =  mfp_vector[phase]->Density() * GRAVITY_CONSTANT; // only one phase  
          
		  msh_ele = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->connected_elements[0]; 
          m_ele = m_pcs_cond->m_msh->ele_vector[msh_ele];
          group = m_pcs_cond->m_msh->ele_vector[msh_ele]->GetPatchIndex();	
	     
		  //sat = mmp_vector[group]->SaturationCapillaryPressureFunction( -pressure0, 0);  
    	  //cond0 *=  mmp_vector[group]->PermeabilitySaturationFunction(sat,0); 
		 
		  sat = mmp_vector[group]->SaturationCapillaryPressureFunction( -pressure1, phase);  
	      cond1 *=  mmp_vector[group]->PermeabilitySaturationFunction(sat, phase); 
			       // use of relative permeability for second node (absolute perm. for top node !!!!)
          
          value =   (pressure1 - pressure0 - deltaZ * gamma)* (cond0 + cond1) / (2* deltaZ * gamma);

          m_pcs_this->SetNodeValue(cnodev->msh_node_number, m_pcs_this->GetNodeValueIndex("COUPLING") +1, -value );
	 
          value *=  area;
     } // end overland
     else {  // Richards 

///// get number of second mesh node, provisional implementation
	double epsilon = 1.e-5;
	 for(int i=0;i<(long)m_pcs_this->m_msh->nod_vector.size();i++) {
        if(m_pcs_this->m_msh->nod_vector[i]->X() - x_this < epsilon)
          if(m_pcs_this->m_msh->nod_vector[i]->Y() - y_this < epsilon)
            if(m_pcs_this->m_msh->nod_vector[i]->Z() - (z_this - deltaZ) < epsilon)
              msh_node_2nd = i; 
	 }
//////////////////////////

        double inf_cap, supplyRate, rainfall;			   
        long bc_eqs_index = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->GetEquationIndex();
        double z_cond = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->Z();
        depth = max(0.,m_pcs_cond->GetNodeValue(cnodev->msh_node_number_conditional, m_pcs_cond->GetNodeValueIndex("HEAD")+1 )- z_cond );   
		     

	    nidx = m_pcs_this->GetNodeValueIndex("PRESSURE1")+1;
	    pressure0 = m_pcs_this->GetNodeValue(cnodev->msh_node_number,nidx); 
        pressure1 = m_pcs_this->GetNodeValue(msh_node_2nd,nidx); 

       
          
        msh_ele = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->connected_elements[0]; 
        m_ele = m_pcs_this->m_msh->ele_vector[msh_ele];
        group = m_pcs_this->m_msh->ele_vector[msh_ele]->GetPatchIndex();	    
  	   
		//sat = mmp_vector[group]->SaturationCapillaryPressureFunction( -pressure0, phase);  
	    //cond0 *=  mmp_vector[group]->PermeabilitySaturationFunction(sat,phase); 

		sat = mmp_vector[group]->SaturationCapillaryPressureFunction( -pressure1, phase);  
        cond1 *=  mmp_vector[group]->PermeabilitySaturationFunction(sat, phase); 
                 // use of relative permeability for second node (absolute perm. for top node !!!!)
	 
		// calculate infiltration capacity
		if( aktuelle_zeit < m_st->rainfall_duration)
	   	  rainfall = m_st->rainfall;
	 	else 
    	  rainfall = 0;

		inf_cap =  ( depth + deltaZ - pressure1 / gamma) * (cond0 + cond1) / (2 * deltaZ ); 
        supplyRate = m_st->rainfall;//+ (depth ) / dt; // dt = timeStep
       
  	    m_pcs_this->SetNodeValue(cnodev->msh_node_number, m_pcs_this->GetNodeValueIndex("COUPLING") + 1, inf_cap);// update coupling variable for error estimation
      
	
	    if(inf_cap > supplyRate)
			bc_value =  pressure1 - deltaZ * gamma + gamma * supplyRate  * deltaZ * 2 / (cond0 + cond1); 
		else  
            bc_value =  pressure1 - deltaZ * gamma + gamma * inf_cap  * deltaZ * 2 / (cond0 + cond1); 
	    // bc_value = supplyRate * gamma * dt;  


       MXRandbed(bc_eqs_index,bc_value,m_pcs_this->eqs->b);
       value = 0;

	 } // end Richards
	

 }

/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB Implementation
02/2006 WW Change argument
**************************************************************************/
//double CSourceTermGroup::GetRiverNODValue(int i,CSourceTerm* m_st, long msh_node) //WW
void GetRiverNODValue(double &value, CNodeValue* cnodev,CSourceTerm* m_st) //WW
{
  double h;
  double paraA; //HRiver
  double paraB; //KRiverBed
  double paraC; //WRiverBed
  double paraD; //TRiverBed
  double paraE; //BRiverBed
  double NodeReachLength;
  double RiverConductance;
  int nidx1;
  
 
  paraA = cnodev->node_parameterA; //HRiver
  paraB = cnodev->node_parameterB; //KRiverBed
  paraC = cnodev->node_parameterC; //WRiverBed
  paraD = cnodev->node_parameterD; //TRiverBed
  paraE = cnodev->node_parameterE; //BRiverBed
  NodeReachLength = value;
  CRFProcess* m_pcs_this = NULL;
  
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  //_________________________________________________________________
  //paraA jetzt aus dem Prozess Overland Flow
  if(m_st->conditional) {

    CRFProcess* m_pcs_cond = NULL;
    m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);

    int nidx = m_pcs_cond->GetNodeValueIndex(m_st->pcs_pv_name_cond)+1;
//WW    long node_cond = group_vector[i]->msh_node_number_conditional;
    long node_cond = cnodev->msh_node_number_conditional; //WW
    paraA = m_pcs_cond->GetNodeValue(node_cond,nidx);
  }
  
  RiverConductance = paraB * paraC * NodeReachLength / (paraD - paraE);
   
  nidx1 = m_pcs_this->GetNodeValueIndex("HEAD")+1;
  h = m_pcs_this->GetNodeValue(cnodev->msh_node_number,nidx1);
        
  if(h > paraD)  {  //HAquiver > BRiverBed
    //q = (RiverConductance * HRiver)   -  (RiverConductance * HAquifer)  
    value = RiverConductance * paraA; 
    MXInc(cnodev->msh_node_number,cnodev->msh_node_number,RiverConductance);
  } 
  if(h < paraD)  {  //HAquiver < BRiverBed
    //q = (RiverConductance * HRiver)   - (RiverConductance * BRiverBed)  
    value = RiverConductance * (paraA - paraD);
  }
  if(h == paraE) value = 0.;
  //_________________________________________________________________
  //Safe Flux values
  int nidxFLUX = m_pcs_this->GetNodeValueIndex("FLUX")+1;
  double flux = value / NodeReachLength  ;  //fluxes in m^2/s !!
  m_pcs_this->SetNodeValue(cnodev->msh_node_number, nidxFLUX, flux);

}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB Implementation
02/2006 WW Change argument
**************************************************************************/
//double CSourceTermGroup::GetCriticalDepthNODValue(CNodeValue* cnodev,CSourceTerm* m_st, long msh_node)
void GetCriticalDepthNODValue(double &value,CSourceTerm* m_st, long msh_node)
{
  double value_jacobi;
  double width, flowdepth, flowdepth3, flowdepth3_epsilon;
  long msh_ele;
  double epsilon = 1.e-7; // like in pcs->assembleParabolicEquationNewton
  

  CRFProcess* m_pcs_this = NULL;
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  long nidx1 = m_pcs_this->GetNodeValueIndex("HEAD")+1;
  flowdepth = m_pcs_this->GetNodeValue(msh_node,nidx1) - m_pcs_this->m_msh->nod_vector[msh_node]->Z()-  m_st->rill_height; 

  if (flowdepth < 0.0) { 
	  value = 0;
	  m_pcs_this->SetNodeValue(msh_node,m_pcs_this->GetNodeValueIndex("FLUX")+0,-value); 		 
  }
  else {
    flowdepth3 = pow(flowdepth,3.);
    flowdepth3_epsilon = pow(flowdepth+epsilon,3.);
    value_jacobi;
    width = value;
    if(m_pcs_this->m_msh->GetMaxElementDim() == 1 ) {
      msh_ele = m_pcs_this->m_msh->nod_vector[msh_node]->connected_elements[0]; 
      int group = m_pcs_this->m_msh->ele_vector[msh_ele]->GetPatchIndex();
      width = mmp_vector[group]->overland_width; 
    }
   
    value = -  sqrt(GRAVITY_CONSTANT * flowdepth3) * width;   
 
    value_jacobi =  sqrt(GRAVITY_CONSTANT * flowdepth3_epsilon) * width + value;  
    MXInc(msh_node,msh_node, value_jacobi / epsilon);      // write source term into jacobi  
    
    m_pcs_this->SetNodeValue(msh_node,m_pcs_this->GetNodeValueIndex("FLUX")+0,-value); 
   
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB JOD Implementation
06/2007 JOD 2D case with slope in st-file
**************************************************************************/
void GetNormalDepthNODValue(double &value, CSourceTerm* m_st, long msh_node)
{

  double flowdepth, flowdepth_epsilon;
//WW  int AnzNodes = 0;
//WW  double Haverage = 0;
  long msh_ele;

  CFEMesh* m_msh = NULL;
  CElem *m_ele = NULL;
  CRFProcess* m_pcs_this = NULL;
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  m_msh = m_pcs_this->m_msh;
  
  double  width, temp;
  double fric_coef, slope_exp, depth_exp;
  int group;
  double elementlength;
  double value_for_jacobi, S_0;
  double epsilon = 1.e-7; // pcs->assembleParabolicEquationNewton !!!!!!!!!
  
  
  msh_ele = m_msh->nod_vector[msh_node]->connected_elements[0]; 
  m_ele = m_msh->ele_vector[msh_ele];
  group = m_msh->ele_vector[msh_ele]->GetPatchIndex();
  width = mmp_vector[group]->overland_width;
  fric_coef  = mmp_vector[group]->friction_coefficient;
  slope_exp = mmp_vector[group]->friction_exp_slope; 
  depth_exp = mmp_vector[group]->friction_exp_depth;
  if(m_st->normaldepth_slope == -1) {
    if(m_msh->GetMaxElementDim() > 1)
	  cout << "!!!!! give slope for NORMAL DEPTH in st-file !!!!!" << endl;
 
    elementlength =sqrt( 
	  pow(m_ele->GetNode(1)->X()- m_ele->GetNode(0)->X(),2.)
	  + pow(m_ele->GetNode(1)->Y()- m_ele->GetNode(0)->Y(),2.)  
	  + pow(m_ele->GetNode(1)->Z()- m_ele->GetNode(0)->Z(),2.) ); 
    S_0 =  (m_ele->GetNode(1)->Z() - m_ele->GetNode(0)->Z() ) / elementlength;
    if(S_0 < 0) S_0 = -S_0;
  }
  else 
    S_0 = m_st->normaldepth_slope;

  flowdepth = m_pcs_this->GetNodeValue(msh_node,1) - m_msh->nod_vector[msh_node]->Z() -  m_st->rill_height; 
flowdepth_epsilon = flowdepth + epsilon;
  if (flowdepth < 0.0)  {
	  flowdepth = 0.0;
      flowdepth_epsilon = 0.0;
  }      
  
  temp = width * fric_coef * pow( S_0, slope_exp );
  if(mmp_vector[group]->channel == 1) {
    value = - pow(flowdepth * width / (2 * flowdepth + width), depth_exp ) * flowdepth * temp;    
    value_for_jacobi =  pow(flowdepth_epsilon * width / (2 * flowdepth_epsilon + width), depth_exp ) * flowdepth_epsilon * temp + value;
  }
  else {
    value = - pow(flowdepth, depth_exp + 1 ) * temp;    
    value_for_jacobi =  pow(flowdepth_epsilon , depth_exp + 1 )  * temp + value;
  }

  MXInc(msh_node,msh_node, value_for_jacobi / epsilon);      // write source term into jacobi  
  m_pcs_this->SetNodeValue(msh_node, m_pcs_this->GetNodeValueIndex("FLUX")+0, -value);


}
#endif
/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation
**************************************************************************/
void GetNODValue(double& value, CNodeValue* cnodev,CSourceTerm* m_st) {


#ifndef NEW_EQS //WW. 06.11.2008
 if(m_st->conditional) 
    GetCouplingNODValue(value, m_st, cnodev); 
 else if(m_st->analytical) {
    //WW      m_st_group->m_msh = m_msh;
    value = GetAnalyticalSolution(cnodev->msh_node_number,m_st); //WW
    //WW         value = m_st_group->GetAnalyticalSolution(m_st,msh_node,(string)function_name[j]);
  }

  if(cnodev->node_distype == 5)       // River Condition
	GetRiverNODValue(value, cnodev, m_st); //MB
  if(cnodev->node_distype == 6)         // CriticalDepth Condition
    GetCriticalDepthNODValue(value, m_st, cnodev->msh_node_number); //MB
  if(cnodev->node_distype == 8)      // NormalDepth Condition JOD
    GetNormalDepthNODValue(value, m_st, cnodev->msh_node_number); //MB        
#endif  
  if(cnodev->node_distype == 10)      // Philip infiltration JOD
    GetPhilipNODValue(value, m_st);  
  if(cnodev->node_distype == 11) // Green_Ampt infiltration JOD
    GetGreenAmptNODValue(value, m_st, cnodev->msh_node_number);   

}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void STGroupDelete(string pcs_type_name,string pcs_pv_name)
{
  CSourceTermGroup* m_st_group = NULL;
  list<CSourceTermGroup*>::const_iterator p=st_group_list.begin();
  while (p!=st_group_list.end()){
    m_st_group = *p;
    if((m_st_group->pcs_type_name.compare(pcs_type_name)==0)&&
       (m_st_group->pcs_pv_name.compare(pcs_pv_name)==0)){
      delete m_st_group;
      st_group_list.remove(m_st_group);
      return;
    }
    ++p;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation 
**************************************************************************/
void CSourceTermGroup::SetPNT(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector)
{


 CGLPoint* m_pnt = NULL;


 CNodeValue *m_nod_val = NULL;
 m_nod_val = new CNodeValue();
 


 m_pnt = GEOGetPointByName(m_st->geo_name);

        if(m_pnt)
           m_nod_val->geo_node_number = m_pnt->id;


        if(m_pnt)
          if(m_msh) //WW
			 m_nod_val->msh_node_number =  m_msh->GetNODOnPNT(m_pnt) +ShiftInNodeVector;
		  else
		  {  
             m_nod_val->msh_node_number =
		       GetNodeNumberClose(m_pnt->x,m_pnt->y,m_pnt->z)
		         	 +ShiftInNodeVector;
		  }
        else
          m_nod_val->msh_node_number = -1;
		m_nod_val->CurveIndex = m_st->CurveIndex;
        m_nod_val->geo_node_number =  m_nod_val->msh_node_number-ShiftInNodeVector; //WW
        m_nod_val->node_value = m_st->geo_node_value;
        m_nod_val->tim_type_name = m_st->tim_type_name;

        if(m_st->dis_type_name.compare("CRITICALDEPTH")==0) {
           m_nod_val->node_distype = 6;
           m_nod_val->node_area = 1.0;
        }

		if(m_st->dis_type_name.compare("NORMALDEPTH")==0) {
           m_nod_val->node_distype = 8;
           m_nod_val->node_area = 1.0;
        }

	if(m_st->dis_type_name.compare("PHILIP")==0) { // JOD
           m_nod_val->node_distype = 10;
           m_nod_val->node_area = 1.0;
        }

     if(m_st->dis_type_name.compare("GREEN_AMPT")==0) { // JOD
           m_nod_val->node_distype = 11;
           m_nod_val->node_area = 1.0;
        }
      //------------------------------------------------------------------
		if(m_st->dis_type_name.compare("SYSTEM_DEPENDENT")==0){      //YD
           m_nod_val->node_distype = 7;
           m_pcs->compute_domain_face_normal = true; //WW
           CElem* elem = NULL;
		   CNode* cnode = NULL; //WW
           for (long i = 0; i < (long)m_msh->ele_vector.size(); i++)
             {
                elem = m_msh->ele_vector[i];
                if(!elem->GetMark()) continue;
                int nn = elem->GetNodesNumber(m_msh->getOrder());
				for(long j=0; j < nn; j++){
					cnode = elem->GetNode(j); //WW
                    if(cnode->GetIndex()==m_st->geo_node_number)
                    m_st->element_st_vector.push_back(i);
				}
			 }
	    }         
        //WW        group_vector.push_back(m_node_value);
        //WW        st_group_vector.push_back(m_st); //OK
        m_pcs->st_node_value.push_back(m_nod_val);  //WW
        m_pcs->st_node.push_back(m_st); //WW


}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation 
**************************************************************************/
void CSourceTermGroup::SetLIN(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector)
{
  long number_of_nodes;
  vector<long>lin_nod_vector;
  vector<double>lin_nod_val_vector;
  CGLLine* m_lin = NULL;
  CGLPolyline* m_ply = NULL;
  
 
  long *nodes = NULL; 

 
  m_lin = m_lin->GEOGetLine(m_st->geo_id);

        if(m_lin){
          double* coordinates;
          m_ply = new CGLPolyline;
          m_ply->point_vector.push_back(m_lin->m_point1);
          m_ply->point_vector.push_back(m_lin->m_point2);
          nodes = MSHGetNodesClose(&number_of_nodes, m_ply);//CC
		  lin_nod_val_vector.resize(number_of_nodes);
          for(long i = 0; i < number_of_nodes; i++){
            lin_nod_val_vector[i] =  m_st->geo_node_value / number_of_nodes;
            coordinates = new double[3];
			coordinates[0] = GetNodeX(nodes[i]);
            coordinates[1] = GetNodeY(nodes[i]);
			coordinates[2] = GetNodeZ(nodes[i]);
			m_lin->nodes_coor_vector.push_back(coordinates);       
		  }
          //InterpolationAlongPolyline(m_polyline,node_value_vector);
          for(long i=0; i < number_of_nodes; i++){
			CNodeValue* m_nod_val = NULL;
            m_nod_val = new CNodeValue();
            m_nod_val->msh_node_number = -1; 
            m_nod_val->msh_node_number = nodes[i]+ShiftInNodeVector;
            m_nod_val->geo_node_number = nodes[i];
            m_nod_val->node_value = lin_nod_val_vector[i];
            m_nod_val->CurveIndex = m_st->CurveIndex;
            //WW        group_vector.push_back(m_node_value);
            //WW        st_group_vector.push_back(m_st); //OK
            m_pcs->st_node_value.push_back(m_nod_val);  //WW
            m_pcs->st_node.push_back(m_st); //WW
          }
          lin_nod_val_vector.clear();
          m_ply->point_vector.clear();
          delete m_ply;
        }
        else
          cout << "Warning - CSourceTermGroup::Set: LIN not found" << endl;
       



}

/**************************************************************************
FEMLib-Method:
Task:
Programing
07/2005 OK Implementation based on CSourceTermGroup::Set
**************************************************************************/
void CSourceTermGroup::SetPLY(CSourceTerm* m_st, const int ShiftInNodeVector) {
 
  
  vector<long> ply_nod_vector;
  vector<long> ply_nod_vector_cond;
  vector<double> ply_nod_val_vector;
  CGLPolyline* m_ply = NULL;
 
  m_ply = GEOGetPLYByName(m_st->geo_name); 

  if(m_ply){
    SetPolylineNodeVector(m_ply, ply_nod_vector);
	if(m_st->conditional)
      SetPolylineNodeVectorConditional(m_st, m_ply, ply_nod_vector, ply_nod_vector_cond);
    m_st->SetDISType();
    SetPolylineNodeValueVector(m_st, m_ply, ply_nod_vector, ply_nod_vector_cond, ply_nod_val_vector);
    m_st->SetNodeValues(ply_nod_vector, ply_nod_vector_cond, ply_nod_val_vector, ShiftInNodeVector);
  } // end polyline 

}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation 
**************************************************************************/
void CSourceTermGroup::SetDMN(CSourceTerm *m_st, const int ShiftInNodeVector)
{
 long number_of_nodes;
 vector<long>dmn_nod_vector;
 vector<double>dmn_nod_val_vector;
 vector<long>dmn_nod_vector_cond;
 
 GEOGetNodesInMaterialDomain(m_msh,m_st->analytical_material_group, dmn_nod_vector,false);
 number_of_nodes = (long)dmn_nod_vector.size();
 dmn_nod_val_vector.resize(number_of_nodes);

 for(long i = 0; i < number_of_nodes; i++)
   dmn_nod_val_vector[i] = 0;

 m_st->SetNodeValues(dmn_nod_vector, dmn_nod_vector_cond, dmn_nod_val_vector, ShiftInNodeVector);

}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2008 JOD Implementation 
**************************************************************************/
void CSourceTermGroup::SetCOL(CSourceTerm *m_st, const int ShiftInNodeVector)
{
 long number_of_nodes;
 vector<long>col_nod_vector;
 vector<double>col_nod_val_vector;
 vector<long>col_nod_vector_cond;
 
 
 long i = 0;
 if(  m_st->geo_name == "BOTTOM")
   i = m_msh->no_msh_layer - 1;

 while(i < (long)m_msh->nod_vector.size()) {
   col_nod_vector.push_back(i);
   i += m_msh->no_msh_layer;
 }
 number_of_nodes = (long)col_nod_vector.size();
 col_nod_val_vector.resize(number_of_nodes);

 for(long i = 0; i < number_of_nodes; i++)
   col_nod_val_vector[i] = 1;

 m_st->SetSurfaceNodeVectorConditional(col_nod_vector, col_nod_vector_cond);
    
    
 m_st->SetNodeValues(col_nod_vector, col_nod_vector_cond, col_nod_val_vector, ShiftInNodeVector);

}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation 
**************************************************************************/
void CSourceTermGroup::SetSFC(CSourceTerm* m_st, const int ShiftInNodeVector) {

  vector<long>sfc_nod_vector;
  vector<long>sfc_nod_vector_cond;
  vector<double>sfc_nod_val_vector;
  Surface* m_sfc = NULL;
  
  m_sfc = GEOGetSFCByName(m_st->geo_name);//CC
      
  if(m_sfc) {

    SetSurfaceNodeVector(m_sfc, sfc_nod_vector); 
    if(m_st->conditional)
      m_st->SetSurfaceNodeVectorConditional(sfc_nod_vector, sfc_nod_vector_cond);
	m_st->SetDISType();
    SetSurfaceNodeValueVector(m_st, m_sfc, sfc_nod_vector, sfc_nod_val_vector);
   	m_st->SetNodeValues(sfc_nod_vector, sfc_nod_vector_cond, sfc_nod_val_vector, ShiftInNodeVector);
 
  } // end surface
    
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
11/2007 JOD Implementation 
**************************************************************************/
void CSourceTerm::SetNOD() {

  
  vector<long>nod_vector;
  vector<long>nod_vector_cond;
  vector<double>nod_val_vector;
  int ShiftInNodeVector;

 nod_vector.push_back(msh_node_number);
  nod_vector_cond.push_back(msh_node_number);
  nod_val_vector.push_back(geo_node_value);


  /*nod_vector[0] = msh_node_number;
  nod_vector_cond[0] = msh_node_number;
  nod_val_vector[0] =geo_node_value;*/
  ShiftInNodeVector = 0;
  

  SetNodeValues(nod_vector, nod_vector_cond, nod_val_vector, ShiftInNodeVector);
  
  

}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::SetPolylineNodeVector(CGLPolyline* m_ply, vector<long>&ply_nod_vector) {

  long  number_of_nodes;

  if(m_msh){ //MSH OK
      if(m_ply->type==100) //WW
		m_msh->GetNodesOnArc(m_ply,ply_nod_vector);
	  else if(m_ply->type==3) // JOD
	    m_msh->GetNODOnPLY_XY(m_ply,ply_nod_vector);
	  else
        m_msh->GetNODOnPLY(m_ply,ply_nod_vector);
      //number_of_nodes = (long)ply_nod_vector.size();
     } // end mesh
    else { //RFI //WW  To be deleted
	  long *nodes = NULL;
      if(m_ply->type==100)
        nodes = GetNodesOnArc(number_of_nodes, m_ply); //WW CC change
	  else {
        m_ply->type = 3;  //??
        nodes = MSHGetNodesClose(&number_of_nodes, m_ply);//CC
	  }
      m_ply->GetPointOrderByDistance(); 
  } // end !mesh
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::SetSurfaceNodeVector(Surface* m_sfc, vector<long>&sfc_nod_vector) {

	if(m_msh) //MSH
       m_msh->GetNODOnSFC(m_sfc, sfc_nod_vector);
    else{
       //WW nodes_vector = m_surface->GetMSHNodesClose();  
       if(pcs_type_name.compare("OVERLAND_FLOW")==0)
          GetMSHNodesOnSurfaceXY(m_sfc, sfc_nod_vector); //CC
        else
          GetMSHNodesOnSurface(m_sfc, sfc_nod_vector);//CC
    }  

}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::SetPolylineNodeVectorConditional(CSourceTerm* m_st, CGLPolyline* m_ply, 
														vector<long>&ply_nod_vector, vector<long>&ply_nod_vector_cond) {

  long assembled_mesh_node, number_of_nodes;
  
  
  if(m_st->node_averaging) {
	if(m_msh_cond) {
      if(pcs_type_name == "RICHARDS_FLOW") {
	    m_msh_cond->GetNODOnPLY(m_ply,ply_nod_vector_cond);
        number_of_nodes = (long)ply_nod_vector_cond.size();
		assembled_mesh_node = ply_nod_vector[0];
        ply_nod_vector.resize(number_of_nodes);
        for(long i = 0; i < number_of_nodes; i++) 
          ply_nod_vector[i] = assembled_mesh_node;
	  } // end richards
      else if(pcs_type_name == "OVERLAND_FLOW" || pcs_type_name == "GROUNDWATER_FLOW") { // JOD 4.10.01
		number_of_nodes = (long)ply_nod_vector.size();
        m_msh_cond->GetNODOnPLY(m_ply,ply_nod_vector_cond);
        assembled_mesh_node = ply_nod_vector_cond[0];
	    ply_nod_vector_cond.resize(number_of_nodes);
        for(long i = 0; i < number_of_nodes; i++) 
          ply_nod_vector_cond[i] = assembled_mesh_node;
      } // end overland, groundwater 
	  else
        cout << "Warning in CSourceTermGroup::SetPolylineNodeVectorConditional - no area assembly for this process" << endl;
	} // end mesh_cond
	else 
      cout << "Warning in CSourceTermGroup::SetPLY - no MSH_COND data" << endl;
  } // end area_assembly
  else {
	number_of_nodes = (long)ply_nod_vector.size();
    ply_nod_vector_cond.resize(number_of_nodes);
	  m_st->SetNOD2MSHNOD(ply_nod_vector, ply_nod_vector_cond);
  } // end !area_assembly

}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
/*void CSourceTerm::SetPolylineNodeValueVectors(CGLPolyline* m_ply, vector<long>& ply_nod_vector) {

 long number_of_nodes = (long)ply_nod_vector.size();
 
 if(dis_type == 5)
	SetPolylineRiverNodeValueVectors(m_ply, number_of_nodes);

  if(dis_type == 5 || dis_type == 6 || dis_type == 8 || dis_type == 9)  {  // area
	  // RiverCriticalDepth, NormalDepth or analytical
    node_value_vectorArea.resize(number_of_nodes);
    for(long i = 0; i < number_of_nodes; i++)  
      node_value_vectorArea[i] =  1.0;  //Element width !
    EdgeIntegration(m_pcs, ply_nod_vector, node_value_vectorArea);   
  } 
  
}*/
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTerm::SetPolylineRiverNodeValueVectors(CGLPolyline* m_ply, int number_of_nodes) {

 node_value_vectorA.resize(number_of_nodes);  // HRiver 
 node_value_vectorB.resize(number_of_nodes); // KRiverBed
 node_value_vectorC.resize(number_of_nodes); // WRiverBed
 node_value_vectorD.resize(number_of_nodes);  // TRiverBed
 node_value_vectorE.resize(number_of_nodes); // BRiverBed


  InterpolatePolylineRiverNodeValueVector(m_ply, DistribedBC, node_value_vectorA);// HRiver 
  InterpolatePolylineRiverNodeValueVector(m_ply, DistBC_KRiverBed, node_value_vectorB); // KRiverBed
  InterpolatePolylineRiverNodeValueVector(m_ply, DistBC_WRiverBed, node_value_vectorC);// WRiver 
  InterpolatePolylineRiverNodeValueVector(m_ply, DistBC_TRiverBed, node_value_vectorD); // TRiverBed
  InterpolatePolylineRiverNodeValueVector(m_ply, DistBC_BRiverBed, node_value_vectorE);// BRiver 

}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTerm::InterpolatePolylineRiverNodeValueVector(CGLPolyline* m_ply, vector<double>& Distribed, vector<double>& ply_nod_vector) {

 for(long k = 0; k < (long)DistribedBC.size(); k++)  {         
       for(long l = 0 ; l < (long)m_ply->point_vector.size(); l++) {
         if(PointsHaveDistribedBC[k]== m_ply->point_vector[l]->id)  {
		   if(fabs(DistribedBC[k]) < MKleinsteZahl) DistribedBC[k] = 1.0e-20;
              m_ply->point_vector[l]->propert = Distribed[k];
            break;
          }
       } 
    }
    InterpolationAlongPolyline(m_ply, ply_nod_vector);
}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::SetPolylineNodeValueVector(CSourceTerm* m_st, CGLPolyline* m_ply, vector<long>&ply_nod_vector, 
											 vector<long>&ply_nod_vector_cond, vector<double>& ply_nod_val_vector) {

 long number_of_nodes = (long)ply_nod_vector.size();
 ply_nod_val_vector.resize(number_of_nodes);
 
 if(m_st->dis_type == 2 || m_st->dis_type == 4) // linear
	  m_st->InterpolatePolylineRiverNodeValueVector(m_ply, m_st->DistribedBC,ply_nod_val_vector);
 //........................................................................................
 else if(m_st->dis_type == 7)  {  //System Dependented YD
    CRFProcess* m_pcs = NULL;
    m_pcs = PCSGet(pcs_type_name);
	m_pcs->compute_domain_face_normal = true; //WW
    long no_face= (long)m_msh->face_vector.size();
	for(long i = 0; i < no_face; i++){
       int node_on_line = 0;
       int no_vertex = m_msh->face_vector[i]->GetVertexNumber();
       for(long jj=0; jj < no_vertex; jj++){
	     for(long kk=0; kk < number_of_nodes; kk++) {
              if(ply_nod_vector[kk] == m_msh->face_vector[i]->GetNodeIndex(jj))  
				  node_on_line++;   
		 } // end nodes
	   } // end vertices
       if(node_on_line == 2)
         m_st->element_st_vector.push_back(m_msh->face_vector[i]->GetOwner()->GetIndex());
    } // end faces
  } // end system dependent
  else { //WW
    for(long i=0;i<number_of_nodes;i++)
      ply_nod_val_vector[i] =  m_st->geo_node_value;
 }
 //........................................................................................
 if(m_st->dis_type == 3 || m_st->dis_type == 4 || m_st->dis_type == 10 || m_st->dis_type == 11) { // neumann, Philip, Green-Ampt
   if(m_msh->GetMaxElementDim()==1) // 1D  //WW MB
	 m_st->DomainIntegration(m_msh, ply_nod_vector, ply_nod_val_vector);
   else
     m_st->EdgeIntegration(m_msh, ply_nod_vector, ply_nod_val_vector);   
 }
 //........................................................................................
 if(m_st->dis_type == 5) // river
   m_st->SetPolylineRiverNodeValueVectors(m_ply, number_of_nodes);
 //........................................................................................
 if(m_st->dis_type == 5 || m_st->dis_type == 6 || m_st->dis_type == 8 || m_st->dis_type == 9)  {  
	  // River, CriticalDepth, NormalDepth or analytical
    m_st->node_value_vectorArea.resize(number_of_nodes);
    for(long i = 0; i < number_of_nodes; i++)  
      m_st->node_value_vectorArea[i] =  1.0;  //Element width !
    m_st->EdgeIntegration(m_msh, ply_nod_vector, m_st->node_value_vectorArea);   
 } 
 //.......................................................................................
 if(m_st->conditional && m_st->node_averaging) 
  AreaAssembly(m_st, ply_nod_vector_cond, ply_nod_val_vector);
 

}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::AreaAssembly(CSourceTerm* m_st,  vector<long>&ply_nod_vector_cond, 
									vector<double>& ply_nod_val_vector) {

 if(pcs_type_name == "RICHARDS_FLOW") { 
      if(m_msh_cond->GetMaxElementDim()==1) // 1D  //WW MB
	    m_st->DomainIntegration(m_msh_cond, ply_nod_vector_cond, ply_nod_val_vector);
      else
        m_st->EdgeIntegration(m_msh_cond, ply_nod_vector_cond, ply_nod_val_vector);  
      double sum_node_value = 0;
	  for(long i = 0; i < (long)ply_nod_val_vector.size(); i++)
        sum_node_value += ply_nod_val_vector[i];
      for(long i = 0; i < (long)ply_nod_val_vector.size(); i++)
        ply_nod_val_vector[i] /= sum_node_value;
  }

								
}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTermGroup::SetSurfaceNodeValueVector(CSourceTerm* m_st, Surface* m_sfc, 
												 vector<long>&sfc_nod_vector, vector<double>&sfc_nod_val_vector) {

// CRFProcess* m_pcs = NULL;
// m_pcs = PCSGet(pcs_type_name);
 long number_of_nodes = (long)sfc_nod_vector.size();
 sfc_nod_val_vector.resize(number_of_nodes);

 for(long i = 0; i < number_of_nodes; i++)  
       sfc_nod_val_vector[i] =  m_st->geo_node_value;
 
 if( m_st->dis_type == 2 ||  m_st->dis_type == 4)   {     // Piecewise linear distributed, polygon-wise WW
   CGLPolyline* m_ply = NULL; 
   vector<CGLPolyline*>::iterator p = m_sfc->polyline_of_surface_vector.begin();
   p = m_sfc->polyline_of_surface_vector.begin();
   while(p!=m_sfc->polyline_of_surface_vector.end()) {
      m_ply = *p;
      for(long k = 0; k < (long) m_st->DistribedBC.size(); k++) {              
        for(long l = 0; l < (long)m_ply->point_vector.size(); l++) {
          if( m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id) {
            if(fabs( m_st->DistribedBC[k])<MKleinsteZahl)  m_st->DistribedBC[k] = 1.0e-20;
              m_ply->point_vector[l]->propert =  m_st->DistribedBC[k];
            break;
          } // end l
        } // end k
      } // end polyline
    // InterpolationAlongPolyline(m_polyline, node_value_vector);
    p++;
   } // end while
 } // end linear
          
 if( m_st->dis_type == 3 ||  m_st->dis_type == 4 ||  m_st->dis_type == 10 ||  m_st->dis_type == 11) {  
	 // neumann, Green-Ampt, Philip
   if(m_msh->GetMaxElementDim()== 2)     // For all meshes with 1-D or 2-D elements
      m_st->DomainIntegration(m_msh, sfc_nod_vector, sfc_nod_val_vector);
   else if(m_msh->GetMaxElementDim()== 3)  // For all meshes with 3-D elements
      m_st->FaceIntegration(m_msh, sfc_nod_vector, sfc_nod_val_vector);
 } // end neumann

}
/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTerm::SetSurfaceNodeVectorConditional(vector<long>&sfc_nod_vector, vector<long>&sfc_nod_vector_cond) {
 
  long number_of_nodes;
  number_of_nodes = (long)sfc_nod_vector.size();
    
  sfc_nod_vector_cond.resize(number_of_nodes);
  SetNOD2MSHNOD(sfc_nod_vector, sfc_nod_vector_cond);

}


/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2007 JOD
last modification:
**************************************************************************/
void CSourceTerm::SetNodeValues(vector<long>&nodes, vector<long>&nodes_cond, vector<double>&node_values, const int ShiftInNodeVector) {
  
 //CRFProcess* m_pcs = NULL; // HS: removed 01.09.2009
 CNodeValue *m_nod_val = NULL; 
 long number_of_nodes = (long)nodes.size();
 // m_pcs = PCSGet(pcs_type_name); // HS: removed 01.09.2009

 for(long i = 0; i < number_of_nodes; i++){
     m_nod_val = new CNodeValue();
     m_nod_val->msh_node_number = nodes[i]+ShiftInNodeVector;
     m_nod_val->geo_node_number = nodes[i];
	 m_nod_val->node_distype = dis_type;
     m_nod_val->node_value = node_values[i];
     m_nod_val->CurveIndex = CurveIndex;
	 if(conditional) { // JOD 4.7.10
       m_nod_val->msh_node_number_conditional = nodes_cond[i];
	   if( (pcs_type_name == "OVERLAND_FLOW" || pcs_type_name == "GROUNDWATER_FLOW" ) && node_averaging) { // JOD 4.10.01
		   double weights = 0;
		   for(long j = 0; j < number_of_nodes; j++) {
			 m_nod_val->msh_node_numbers_averaging.push_back(nodes[j]);
	            m_nod_val->msh_node_weights_averaging.push_back(node_values[j]);
		       weights += node_values[j];
		   }
           for(long j = 0; j < number_of_nodes; j++) 
              m_nod_val->msh_node_weights_averaging[j] /= weights;
	   }
	 }
     //WW        group_vector.push_back(m_node_value);
     //WW        st_group_vector.push_back(m_st); //OK
     if(dis_type == 5)  {// River  
       m_nod_val->node_value = node_value_vectorArea[i];
       m_nod_val->node_parameterA = node_value_vectorA[i];
       m_nod_val->node_parameterB = node_value_vectorB[i];
       m_nod_val->node_parameterC = node_value_vectorC[i];
       m_nod_val->node_parameterD = node_value_vectorD[i];
       m_nod_val->node_parameterE = node_value_vectorE[i];  
     } 
     if(dis_type == 6 || dis_type == 8 || dis_type == 9)  // critical depth, normal depth, analytical
     {
        m_nod_val->node_value = node_value_vectorArea[i];   
        m_nod_val->node_area = node_value_vectorArea[i]; //CMCD bugfix on 4.9.06
     }   
     m_pcs->st_node_value.push_back(m_nod_val);  //WW
     m_pcs->st_node.push_back(this); //WW
 } // end nodes

}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
11/2005 MB
last modification:
**************************************************************************/
void CSourceTerm::SetNOD2MSHNOD(vector<long>&nodes, vector<long>&conditional_nodes)
{
  CGLPoint* m_pnt = NULL;
  long number;
  CFEMesh* m_msh_cond = NULL;
  CFEMesh* m_msh_this = NULL;

  m_msh_cond = FEMGet(pcs_type_name_cond);
  m_msh_this = FEMGet(pcs_type_name);
  m_pnt = new CGLPoint;
	
  for(long i=0;i<(long)nodes.size();i++){
    m_pnt->x = m_msh_this->nod_vector[nodes[i]]->X();
    m_pnt->y = m_msh_this->nod_vector[nodes[i]]->Y();
    m_pnt->z = m_msh_this->nod_vector[nodes[i]]->Z();

    number = m_msh_cond->GetNODOnPNT(m_pnt);
    conditional_nodes[i]=number;
    
  }

  delete m_pnt;

}

/**************************************************************************
GeoSys source term function: 
02/2009 WW Implementation
**************************************************************************/
inline void CSourceTerm::DirectAssign(const long ShiftInNodeVector)
{
  string line_string;
  string st_file_name;
  std::stringstream in;
  long n_index;
  double n_val;
  CRFProcess* m_pcs = NULL;
  CNodeValue *m_nod_val = NULL; 
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
    m_nod_val = new CNodeValue();
    m_nod_val->msh_node_number = n_index + ShiftInNodeVector;
    m_nod_val->geo_node_number = n_index;
    m_nod_val->node_distype = dis_type;
    m_nod_val->node_value = n_val;
    m_nod_val->CurveIndex = CurveIndex;   
    m_pcs->st_node_value.push_back(m_nod_val);  
    m_pcs->st_node.push_back(this); 
    // 
  } // eof
}

/**************************************************************************
FEMLib-Method:
Task: Anaylitical diffusion in matrix. Replaces matrix. See paper to be issued.
Programing:
11/2005 CMCD Implementation
04/2006 Moved from CSourceTermGroup and changed the arguments
last modification:
04/2006 CMCD Updated
**************************************************************************/
double GetAnalyticalSolution(long location, CSourceTerm *m_st)
{
  int idx, n;
  int size, process_no;
  long i;
  long step, no_terms_included;
  double value, source, gradient, ref_value = 0.0;
  double timevalue;
  double fac = 1.0;
  double temp_time,temp_value;
  double pi = 3.1415926;
  double D = m_st->analytical_diffusion;
  double ne = m_st->analytical_porosity;
  double tort = m_st->analytical_tortousity;
  double Kd = m_st->analytical_linear_sorption_Kd;
  double rho = m_st->analytical_matrix_density;
  double Dtrans = (D*ne)/((ne+Kd*rho)*tort);
  double Dsteady = D*ne/tort;
  double t0, tn, tnn, val1,val2, node_area;
  double tvol, vol, flux_area, tflux_area;
  double mass_solute_present,mass_to_remove;
//WW  bool out = false;
//WW  int dimension = m_st->analytical_material_group;
  string process;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(m_st->pcs_type_name, m_st->pcs_pv_name);
  CFEMesh* m_msh = m_pcs->m_msh;  //WW
  CElem* Ele = NULL; 
  long node_number = location; //WW m_pcs->st_node_value[location]->msh_node_number;
  CNode* Node = m_msh->nod_vector[node_number]; 
  double area = m_pcs->st_node_value[location]->node_area;
  vector<double>time_history;
  vector<double>value_history;
  //Initialise
  time_history.clear();
  value_history.clear();
  t0 = tn = tnn = source = gradient = val1 = val2 = node_area = flux_area = 0.0;
  idx = m_pcs->GetNodeValueIndex(m_st->pcs_pv_name);
  value = m_pcs->GetNodeValue(node_number,idx);
  if (value < MKleinsteZahl) value = 0.0;
  timevalue = aktuelle_zeit;
  step = aktueller_zeitschritt;
  if (step < 10) step = 10;
  size = (int)analytical_processes.size();
  process_no = 0;
  //Domain or Polyline
  for (i = 0; i < size; i++){
     if (analytical_processes[i] == m_st->pcs_pv_name){
        if(m_st->geo_type_name.compare("POLYLINE")==0){
          if(m_st->geo_name == analytical_processes_polylines[i])
              process_no = i;
        }
        if(m_st->geo_type_name.compare("DOMAIN")==0)
          process_no = i;
       }
  }
  //Identify process
  if (process_no == 1){
      process_no = process_no;
  }
  process_no*=2;//first column time, second column value, hence two columns per process;

  //If time step require new calculation of source term then start
  if ((aktueller_zeitschritt-1) % m_st->resolution == 0){

    //Save data in a vector attached to the nodes
    m_st->SetNodePastValue(node_number,process_no,0,timevalue);
    m_st->SetNodePastValue(node_number,process_no+1,0,value);

    //Recall historical data
    ref_value = m_st->GetNodePastValueReference (node_number, (process_no/2));
    if (step > m_st->number_of_terms) no_terms_included = m_st->number_of_terms;
    else no_terms_included = step;
    for (i = 0; i <no_terms_included; i++){ 
      temp_time = m_st->GetNodePastValue(node_number,process_no,i);
      temp_value = (m_st->GetNodePastValue(node_number,process_no+1,i)-ref_value);
      time_history.push_back(temp_time);
      value_history.push_back(temp_value);
    }
    
    //Calculate individual terms and sum for gradient 
    for ( i = no_terms_included-1; i >0 ; i--){ 
      t0 = time_history[0];
      if (i == no_terms_included-1) tn = (t0-time_history[i])+(time_history[i-1]-time_history[i]);
      else tn = t0-time_history[i+1];
      tnn= t0-time_history[i];
      val1 = 1/(sqrt(pi*Dtrans*tn));
      val2 = 1/(sqrt(pi*Dtrans*tnn));
      gradient += ((val2-val1)*value_history[i]);
    }
    tn = t0-time_history[1];
    tnn = 0;
    val1 = 1/(sqrt(pi*Dtrans*tn));
    gradient -=(val1*value_history[0]);

    //Area calculations
    mass_solute_present = 1.e99; //Initially set value very high
 
   //Area for lines, triangles and quads in domain.
   //  if (area < 0) {//set in set source terms function, domain area = -1 to start with
	 if (area < DBL_MIN) { // HS 04.2008
      tvol = 0.0;
      tflux_area = 0.0;
      for (i = 0; i <  (int) Node->connected_elements.size(); i++){
        Ele = m_msh->ele_vector[ Node->connected_elements[i]];
        vol = Ele->GetVolume();//Assuming 1m thickness
        flux_area = Ele->GetFluxArea();//Real thickness for a fracture
        n = Ele->GetVertexNumber();
        tvol += (vol/n);
        tflux_area += (flux_area/n);
      }
      node_area = tvol*2.;//* 2 because the diffusion is in two direction perpendicular to the fracture
      mass_solute_present = tflux_area * tvol * value;
    }
    //Area for polylines
    else node_area = area;

    //factor for conversion to energy for temperature if necessary
    fac = m_st->factor;
    source = gradient * Dsteady * fac * node_area;
    mass_to_remove = abs(source)*dt;
    if (mass_to_remove > mass_solute_present){
          source*=(mass_solute_present/mass_to_remove);
          }
    m_st->SetNodeLastValue (node_number, (process_no/2),source);

  } // If new source term calculation not required
  else source = m_st->GetNodeLastValue (node_number, (process_no/2));
  return source;
}
/**************************************************************************
FEMLib-Method: 
Task: master write function
Programing:
11/2005 CMCD Implementation, functions to access and write the time history data
of nodes 
last modification:
**************************************************************************/
void CSourceTerm::SetNodePastValue ( long n, int idx, int pos, double value)
{
  long k = 0;
  int j = 0;
  bool endstepreached = false;
  pos=pos; //WW
  //----------------------------------------------------------------------
  //Check whether this is the first call
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(pcs_type_name, pcs_pv_name);
  if(!m_pcs){ //OK
    cout << "Warning in SetNodePastValue - no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  int size1 = (int) m_pcs->nod_val_vector.size();
  int size = (int)node_history_vector.size();
  
  //Create memory for the values
  if (size == 0){
//WW     int number_of_terms = max_no_terms;
     for (k = 0; k< size1; k++){
       NODE_HISTORY *nh = new NODE_HISTORY;
       CreateHistoryNodeMemory(nh);
       node_history_vector.push_back(nh);
     }
     for (k=0; k< size1; k++){
       for (j=0;j<no_an_sol;j++) 
          node_history_vector[k]->value_reference.push_back(-1.0);
     }
  }

  //Store the first set of values as reference values
  int flipflop = idx%2;
  if (aktueller_zeitschritt==1){
    //if (size2 == idx)
       if (flipflop == 1)
          node_history_vector[n]->value_reference[(idx-1)/2]=value;
  }
//  size2 = (int) node_history_vector[n]->value_reference.size();
  long steps = 10;
  if (aktueller_zeitschritt > steps) steps = aktueller_zeitschritt;
  if (max_no_terms >= steps) steps = aktueller_zeitschritt;  
  else {
    steps = max_no_terms;
    endstepreached = true;
  }
  //Enter the value and push the other values back
  if (!endstepreached){
    for (k = steps-1; k>0; k--)
      node_history_vector[n]->value_store[idx][k] = node_history_vector[n]->value_store[idx][k-1];
    node_history_vector[n]->value_store[idx][0] = value;
  }
  double cutvalue = 0.0;
  double nextvalue = 0.0;
  long no_steps_past_cutof = 0;
  if (endstepreached){
    no_steps_past_cutof = aktueller_zeitschritt-max_no_terms;
    cutvalue = node_history_vector[n]->value_store[idx][steps-1];
    nextvalue = node_history_vector[n]->value_store[idx][steps-2];
    node_history_vector[n]->value_store[idx][steps-1]= (cutvalue * (double)(no_steps_past_cutof)+nextvalue)/((double)no_steps_past_cutof+1);
    for (k = steps-2; k>0; k--)
      node_history_vector[n]->value_store[idx][k] = node_history_vector[n]->value_store[idx][k-1];
    node_history_vector[n]->value_store[idx][0] = value;
  }
}
void CSourceTerm::SetNodeLastValue (long n, int idx, double value)
{
     int i;
     int size = (int) node_history_vector[n]->last_source_value.size();
     if (size == 0) {
       for(i=0;i<no_an_sol;i++)
         node_history_vector[n]->last_source_value.push_back(0);
     }  
     node_history_vector[n]->last_source_value[idx] = value;
}
double CSourceTerm::GetNodeLastValue (long n, int idx)
{
     double value = 0.0;
     value = node_history_vector[n]->last_source_value[idx];
     return value;
}
double CSourceTerm::GetNodePastValue ( long n, int idx, int pos)
{
  double value;
  value = node_history_vector[n]->value_store[idx][pos];
  return value;
}
double CSourceTerm::GetNodePastValueReference ( long n, int idx )
{
  double value;
  value = node_history_vector[n]->value_reference[idx];
  return value;
}

void CSourceTerm::CreateHistoryNodeMemory(NODE_HISTORY* nh )
{
  int s_col=no_an_sol*2;
  long s_row=number_of_terms;
  int k,i;
  long j;

  nh->value_store = new double* [s_col];
  for(i=0; i<s_col; i++)
   nh->value_store[i] = new double [s_row];

  for (k = 0; k<s_col; k++){
    for (j = 0; j < s_row; j++)
      nh->value_store[k][j] = 0.0;
  }
}

void CSourceTerm::DeleteHistoryNodeMemory()
{
  long size = (long) node_history_vector.size(); //m_st->
  int s_row=no_an_sol*2;
//WW  long s_col=max_no_terms;
  long j;
  int i;
  if (size > 0){
    for (j=0; j < size; j++){
      for(i=0; i<s_row; i++)
        delete node_history_vector[j]->value_store[i];
    delete node_history_vector[j]->value_store;
    }
    node_history_vector.clear();
  }
}

/**************************************************************************
FEMLib-Method: 
07/2007 OK Implementation
**************************************************************************/
CSourceTerm* STGet(string pcs_name,string geo_type_name,string geo_name)
{
  CSourceTerm *m_st = NULL;
  for(int i=0;i<(int)st_vector.size();i++)
  {
    m_st = st_vector[i];
    if((m_st->pcs_type_name.compare(pcs_name)==0)&&
       (m_st->geo_type_name.compare(geo_type_name)==0)&&
       (m_st->geo_name.compare(geo_name)==0))
      return m_st;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 4.7.10 shift and average field variables
10/2008 JOD Implementation
**************************************************************************/
void GetCouplingFieldVariables(double* h_this, double* h_cond, double* h_shifted, double* h_averaged, double* z_this, double* z_cond, CSourceTerm* m_st, CNodeValue* cnodev) {

  int nidx, nidx_cond;
  double gamma;
  CRFProcess* m_pcs_this = NULL;
  CRFProcess* m_pcs_cond = NULL;
  
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  m_pcs_cond = PCSGet(m_st->pcs_type_name_cond);
  gamma =  mfp_vector[0]->Density() * GRAVITY_CONSTANT; // only one phase  
  *z_this = m_pcs_this->m_msh->nod_vector[cnodev->msh_node_number]->Z();
  *z_cond = m_pcs_cond->m_msh->nod_vector[cnodev->msh_node_number_conditional]->Z();
  nidx = m_pcs_this->GetNodeValueIndex( m_st->pcs_pv_name)+1;
  nidx_cond = m_pcs_cond->GetNodeValueIndex(m_st->pcs_pv_name_cond)+1;
  *h_this = m_pcs_this->GetNodeValue(cnodev->msh_node_number,nidx);
  *h_cond = m_pcs_cond->GetNodeValue(cnodev->msh_node_number_conditional,nidx_cond);
  
 if(m_st->pcs_type_name == "OVERLAND_FLOW")  {
    if(m_st->node_averaging) { // shift overland node on soil column top, averaging over nodes
      *h_shifted =  *h_this - *z_this + *z_cond; 
	  *h_averaged = 0;
	  for(long i = 0; i <  (long)cnodev->msh_node_numbers_averaging.size(); i++) 
       *h_averaged += cnodev->msh_node_weights_averaging[i] * ( m_pcs_this->GetNodeValue(cnodev->msh_node_numbers_averaging[i],nidx)  - m_pcs_this->m_msh->nod_vector[cnodev->msh_node_numbers_averaging[i]]->Z() );
	  
	  *h_averaged += *z_cond;
	  *z_this = *z_cond; 
    } // end averaging
    else { // no averaging
      *h_shifted = *h_this;
      *h_averaged = *h_this;
    }   // end no averaging

	if(m_st->pcs_pv_name_cond == "PRESSURE1") {
      *h_cond /= gamma;
      *h_cond += *z_cond;
    }
	if(m_st->pcs_type_name_cond == "GROUNDWATER_FLOW") 
	  h_cond = max(h_cond, z_this);		//groundwater level might not reach overland flow bottom, than only hydrostatic overland pressure
 } // end overland flow
 else {   // richards & groundwater flow 
	 if(m_st->pcs_pv_name_cond == "PRESSURE1") { // JOD 4.10.01
      *h_cond /= gamma;
      *h_cond += *z_cond;
	 }
    if(m_st->node_averaging) { // shift overland/groundwater node on soil column top, averaging over nodes
      *h_shifted= *h_cond - *z_cond;
      *h_shifted += *z_this;
      *z_cond = *z_this;
    }  // end averaging
    else 
      *h_shifted = *h_cond;

    if( m_st->pcs_pv_name == "PRESSURE1") {
      *h_this /= gamma;
      *h_this += *z_this;
    }
 } // end richards & groundwater flow 

}

/**************************************************************************
FEMLib-Method: 4.7.10
10/2008 JOD Implementation
**************************************************************************/
double CalcCouplingValue(double factor, double h_this, double h_cond, double z_cond, CSourceTerm* m_st) {
 
  if(m_st->pcs_type_name == "OVERLAND_FLOW")  {
    if(m_st->no_surface_water_pressure)  // 4.10.01
	   return factor *( h_cond - z_cond);  
	else
	   return factor *( h_cond - h_this);  
  }
  else {  // richards & groundwater flow 
    if(m_st->pcs_type_name == "GROUNDWATER_FLOW") {  
      if( h_this < z_cond && m_st->pcs_type_name_cond == "OVERLAND_FLOW")
         return factor * (h_cond - z_cond); 
       else
         return factor * h_cond; 
     }
     else // richards flow
  	   return factor * (h_cond - z_cond); 
    
   }

}
