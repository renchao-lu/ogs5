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
#include "elements.h"
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
#include "matrix.h"

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



//==========================================================================
vector<CSourceTerm*> st_vector;
list<CSourceTermGroup*>st_group_list;
vector<string>analytical_processes;
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
  geo_node_value=0.0; 
  plyST = NULL; //OK
  nodes = NULL; //OK
  analytical = false;//CMCD
}

/**************************************************************************
FEMLib-Method: 
Task: BC deconstructor
Programing:
04/2004 OK Implementation
**************************************************************************/
CSourceTerm::~CSourceTerm(void) {
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
**************************************************************************/
ios::pos_type CSourceTerm::Read(ifstream *st_file)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  bool new_keyword = false;
  string hash("#");
  std::stringstream in;
  int abuff, nLBC=0;
  double bbuff;
  double cbuff;
  double dbuff;
  double ebuff;
  double fbuff;
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
     }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { //subkeyword found
      in.str(GetLineFromFile1(st_file));
      in >> geo_type_name;
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
      if(pcs_pv_name.find("EXCAVATION")!=string::npos) //WW
      {
          in.str(GetLineFromFile1(st_file));
          in>>sub_string>>geo_type;      
          in.clear();
      } 
      continue;
    }
    //....................................................................
    //if(line_string.find("$DIS_TYPE")!=string::npos) { //PCH
    if(line_string.compare("$DIS_TYPE")==0) { //MB not = $DIS_TYPE_CONDITION
      in.str(GetLineFromFile1(st_file));
      in >> dis_type_name;
      if(dis_type_name.find("CONSTANT")!=string::npos) {
        if(dis_type_name.find("NEUMANN")!=string::npos) {
          dis_type_name = "CONSTANT_NEUMANN";
        }
        else  {  
          dis_type_name = "CONSTANT";
        }
        in >> geo_node_value;
        in.clear();
      }
     if(dis_type_name.find("ANALYTICAL")!=string::npos) {
        in >> analytical_material_group;//Which material group is it being applied to
        in >> analytical_diffusion;//D value
        in >> number_of_terms;//no timesteps to consider in solution
        if (number_of_terms > 25) number_of_terms = 25;//Set a maximum number of terms to evaluate
        analytical = true;
        analytical_processes.push_back(pcs_pv_name);
        in.clear();
      }
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

        if(dis_type_name.find("NEUMANN")!=string::npos) {
          dis_type_name = "LINEAR_NEUMANN";
        }
        else  {
          dis_type_name = "LINEAR";
        }
//      Read LINENODES AND VALUES......
        in.clear();
      }
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
      if(dis_type_name.find("CRITICALDEPTH")!=string::npos) {
        dis_type_name = "CRITICALDEPTH";
        critical_depth = true;
        in >> geo_node_value;
        in.clear();
      }
	  if(dis_type_name.find("NORMALDEPTH")!=string::npos) {
        dis_type_name = "NORMALDEPTH";
        in >> geo_node_value;
        in.clear();
      }
      continue;
    } // subkeyword found
    //..Time dependent curve ............................................
    if(line_string.find("$TIM_TYPE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(st_file));
      in >> tim_type_name;
      if(tim_type_name.find("CURVE")!=string::npos) {
        dis_type = 0;
        in>>CurveIndex;
      }
      in.clear(); 
      continue;
	}
    //....................................................................
//OK4108
    if(line_string.find("$DIS_TYPE_CONDITION")!=string::npos) { // subkeyword found
	  conditional = true;
      in.str(GetLineFromFile1(st_file)); // CONSTANT 0.0
      in >> line_string;
      if(line_string.find("CONSTANT_NEUMANN")!=string::npos) {
        dis_type_name = "CONSTANT_NEUMANN";
        dis_type = 0;
	    in >> geo_node_value;
    	in.clear();
      }

      ////////////////////////////////////
      if(line_string.find("RIVER")!=string::npos) {
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
      //Coupling
      in.str(GetLineFromFile1(st_file)); //PCS type
      in >> line_string >> pcs_type_name_cond;
      in.clear();
      in.str(GetLineFromFile1(st_file)); // 
      in >> pcs_pv_name_cond;
      in.clear();
    }
    //....................................................................
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
  STDelete();  
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
    if(line_string.find("#STOP")!=string::npos)
      return true;
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
          st_value = m_point->property; // i.e. node property
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
		/*
		*st_file << delimiter_type << geo_node_value[0]
		         << delimiter_type << geo_node_value[1];*/
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
last modification:
**************************************************************************/
void CSourceTerm::SetDISType(void)
{
  if(dis_type_name.compare("CONSTANT")==0) dis_type = 0;
  if(dis_type_name.compare("LINEAR")  ==0) dis_type = 1;
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
last modification:
02/2005 MB River condition, CriticalDepth
08/2008 WW Re-implementing  edge, face and domain integration versatile for 
           all element types
**************************************************************************/
void CSourceTermGroup::Set(CRFProcess* m_pcs, const int ShiftInNodeVector, string this_pv_name)
{
  long number_of_nodes;
  long *nodes = NULL; //WW  To be deleted
  vector<long>nodes_vector;
  vector<long>nodes_vector_conditional;
  long i, j;
  int k,l;
  vector<double> node_value_vector;
  vector<double> node_value_vectorA;
  vector<double> node_value_vectorB;
  vector<double> node_value_vectorC;
  vector<double> node_value_vectorD;
  vector<double> node_value_vectorE;
  vector<double> node_value_vectorArea;
  CSourceTerm *m_st = NULL;
  CNodeValue *m_node_value = NULL;
  CGLPolyline *m_polyline = NULL;
  
  CFEMesh* m_msh = m_pcs->m_msh;
  CElem* elem = NULL;
  CNode* cnode = NULL; //WW

  //----------------------------------------------------------------------
  // Tests //OK
  if(!m_msh){
    cout << "Warning in CSourceTermGroup::Set - no MSH data" << endl;
    //return;
  }
  //----------------------------------------------------------------------
  if(m_msh)//WW
  {
     /// In case of P_U coupling monolithic scheme
     if(m_pcs->type==41) //WW Mono
     {
       if(pcs_pv_name.find("DISPLACEMENT")!=string::npos) //Deform 
          m_pcs->m_msh->SwitchOnQuadraticNodes(true);
       else 
          m_pcs->m_msh->SwitchOnQuadraticNodes(false);
     }
     else if(m_pcs->type==4)
        m_pcs->m_msh->SwitchOnQuadraticNodes(true);
	 else
        m_pcs->m_msh->SwitchOnQuadraticNodes(false);
  }

  long no_st =(long)st_vector.size();
  for(j=0;j<no_st;j++){
    m_st = st_vector[j];
    //====================================================================
    //OK if(m_st->pcs_name.compare(pcs_name)==0) { //OK/SB 
    if((m_st->pcs_type_name.compare(pcs_type_name)==0)&&(m_st->pcs_pv_name.compare(pcs_pv_name)==0)){
      //------------------------------------------------------------------
      if(m_st->geo_type_name.compare("POINT")==0) {
        m_node_value = new CNodeValue();
        CGLPoint* m_point = NULL;
        m_point = GEOGetPointByName(m_st->geo_name);
        if(m_point)
        m_node_value->geo_node_number = m_point->id;
        if(m_point)
          if(m_msh) //WW
			 m_node_value->msh_node_number =  m_msh->GetNODOnPNT(m_point) +ShiftInNodeVector;
		  else
		  {  
             m_node_value->msh_node_number =
		       GetNodeNumberClose(m_point->x,m_point->y,m_point->z)
		         	 +ShiftInNodeVector;
		  }
        else
          m_node_value->msh_node_number = -1;
		m_node_value->CurveIndex = m_st->CurveIndex;
        m_node_value->geo_node_number =  m_node_value->msh_node_number-ShiftInNodeVector; //WW
        m_node_value->node_value = m_st->geo_node_value;
        m_node_value->tim_type_name = m_st->tim_type_name;
        if(m_st->dis_type_name.compare("CRITICALDEPTH")==0) {
           m_node_value->node_distype = 6;
           m_node_value->node_area = 1.0;
        }

		if(m_st->dis_type_name.compare("NORMALDEPTH")==0) {
           m_node_value->node_distype = 7;
           m_node_value->node_area = 1.0;
        }


      //------------------------------------------------------------------
		if(m_st->dis_type_name.compare("SYSTEM_DEPENDENT")==0){      //YD
           m_node_value->node_distype = 7;
           for (int ii = 0; ii < (long)m_msh->ele_vector.size(); ii++)
             {
                elem = m_msh->ele_vector[ii];
                if(!elem->GetMark()) continue;
                int nn = elem->GetNodesNumber(m_msh->getOrder());
				for(k=0; k < nn; k++){
					cnode = elem->GetNode(k); //WW
                    if(cnode->GetIndex()==m_st->geo_node_number)
                    m_st->element_st_vector.push_back(ii);
				}
			 }
	    }         
        //WW        group_vector.push_back(m_node_value);
        //WW        st_group_vector.push_back(m_st); //OK
        m_pcs->st_node_value.push_back(m_node_value);  //WW
        m_pcs->st_node.push_back(m_st); //WW
      }
      //------------------------------------------------------------------
//OK
      if(m_st->geo_type_name.compare("LINE")==0) {
        CGLLine* m_lin = NULL;
        m_lin = m_lin->GEOGetLine(m_st->geo_id);
        if(m_lin){
          double* coordinates;
          m_polyline = new CGLPolyline;
          m_polyline->point_vector.push_back(m_lin->m_point1);
          m_polyline->point_vector.push_back(m_lin->m_point2);
          nodes = MSHGetNodesClose(&number_of_nodes, m_polyline);//CC
		  node_value_vector.resize(number_of_nodes);
          for(i=0;i<number_of_nodes;i++){
            node_value_vector[i] =  m_st->geo_node_value / number_of_nodes;
            coordinates = new double[3];
			coordinates[0] = GetNodeX(nodes[i]);
            coordinates[1] = GetNodeY(nodes[i]);
			coordinates[2] = GetNodeZ(nodes[i]);
			m_lin->nodes_coor_vector.push_back(coordinates);       
		  }
          //InterpolationAlongPolyline(m_polyline,node_value_vector);
          for(i=0;i<number_of_nodes;i++){
            m_node_value = new CNodeValue();
            m_node_value->msh_node_number = -1; 
            m_node_value->msh_node_number = nodes[i]+ShiftInNodeVector;
            m_node_value->geo_node_number = nodes[i];
            m_node_value->node_value = node_value_vector[i];
            m_node_value->CurveIndex = m_st->CurveIndex;
            //WW        group_vector.push_back(m_node_value);
            //WW        st_group_vector.push_back(m_st); //OK
            m_pcs->st_node_value.push_back(m_node_value);  //WW
            m_pcs->st_node.push_back(m_st); //WW
          }
          node_value_vector.clear();
          m_polyline->point_vector.clear();
          delete m_polyline;
        }
        else{
          cout << "Warning - CSourceTermGroup::Set: LIN not found" << endl;
        }
      }
      //------------------------------------------------------------------
      if(m_st->geo_type_name.compare("POLYLINE")==0) {
        m_polyline = GEOGetPLYByName(m_st->geo_name);//CC
		m_st->plyST = m_polyline;
        if(m_polyline){
          if(m_msh){ //MSH OK
            if(m_polyline->type==100) //WW
				m_pcs->m_msh->GetNodesOnArc(m_polyline,nodes_vector);
			else
               m_msh->GetNODOnPLY(m_polyline,nodes_vector);
            number_of_nodes = (long)nodes_vector.size();
          }
          else{ //RFI //WW  To be deleted
            if(m_polyline->type==100)
              nodes = GetNodesOnArc(number_of_nodes, m_polyline); //WW CC change
		    else
		    {
              m_polyline->type = 3;  //??
              nodes = MSHGetNodesClose(&number_of_nodes, m_polyline);//CC
		    }
            m_polyline->GetPointOrderByDistance(); 
          }
		  node_value_vector.resize(number_of_nodes);

          //..............................................................
          if(m_st->conditional){
            nodes_vector_conditional.resize(number_of_nodes);
            m_st->SetNOD2MSHNOD(nodes_vector, nodes_vector_conditional);
          }
          //..............................................................

          // Here start DIS types
          int dit_ply = -1;
          if(m_st->dis_type_name.compare("CONSTANT")==0) dit_ply = 1;
          if(m_st->dis_type_name.compare("LINEAR")==0) dit_ply = 2;
          if(m_st->dis_type_name.compare("CONSTANT_NEUMANN")==0) dit_ply = 3;
          if(m_st->dis_type_name.compare("LINEAR_NEUMANN")==0) dit_ply = 4;
          if(m_st->dis_type_name.compare("RIVER")==0) dit_ply = 5;
          if(m_st->dis_type_name.compare("CRITICALDEPTH")==0) dit_ply = 6;
          if(m_st->dis_type_name.compare("SYSTEM_DEPENDENT")==0) dit_ply = 7;   //YD  
          if(m_st->dis_type_name.compare("NORMALDEPTH")==0) dit_ply = 8;  //JOD MB


          if(dit_ply==2||dit_ply==4)  {
            // Piecewise linear distributed. WW
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistribedBC[k];
                  break;
                }
              } 
            }
            
			InterpolationAlongPolyline(m_polyline, node_value_vector);

          }
          else if(dit_ply==7)  {  //System Dependented YD
          long no_face= (long)m_pcs->m_msh->face_vector.size();
          for(int ii=0; ii < no_face;ii++){
          int node_on_line = 0;
          int no_vertex = m_pcs->m_msh->face_vector[ii]->GetVertexNumber();
              for(int jj=0; jj < no_vertex; jj++){
              for(int kk=0; kk < number_of_nodes; kk++)
              if(nodes_vector[kk] == m_pcs->m_msh->face_vector[ii]->GetNodeIndex(jj))  node_on_line++;   
              }
              if(node_on_line == 2)
              m_st->element_st_vector.push_back(m_pcs->m_msh->face_vector[ii]->GetOwner()->GetIndex());
          }
          }
		  else //WW
		  {
             for(i=0;i<number_of_nodes;i++)
               node_value_vector[i] =  m_st->geo_node_value;
		  }
          
          //------------------------------------------------------------------------------------
          if(dit_ply==5)  {  //River
            
            node_value_vectorA.resize(number_of_nodes);
            node_value_vectorB.resize(number_of_nodes);
            node_value_vectorC.resize(number_of_nodes);
            node_value_vectorD.resize(number_of_nodes);
            node_value_vectorE.resize(number_of_nodes);
            node_value_vectorArea.resize(number_of_nodes);

            // Piecewise linear distributed. WW
            // HRiver 
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistribedBC[k];
                  break;
                }
              } 
            }
            InterpolationAlongPolyline(m_polyline, node_value_vectorA);
            // KRiverBed
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistBC_KRiverBed[k];
//printf("\n Node: %d %f ",m_polyline->point_vector[l]->gli_point_id,m_st->DistBC_KRiverBed[k]); 
                  break;
                }
              } 
            }
            InterpolationAlongPolyline(m_polyline, node_value_vectorB);
            // WRiverBed
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistBC_WRiverBed[k];
                  break;
                }
              } 
            }
            InterpolationAlongPolyline(m_polyline, node_value_vectorC);
            // TRiverBed
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistBC_TRiverBed[k];
                  break;
                }
              } 
            }
            InterpolationAlongPolyline(m_polyline, node_value_vectorD);
            // BRiverBed
            for(k=0;k<(int)m_st->DistribedBC.size(); k++)  {              
              for(l=0;l<(int)m_polyline->point_vector.size(); l++) {
                if(m_st->PointsHaveDistribedBC[k]== m_polyline->point_vector[l]->id)  {
                  if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                  m_polyline->point_vector[l]->property = m_st->DistBC_BRiverBed[k];
                  break;
                }
              } 
            }
            InterpolationAlongPolyline(m_polyline, node_value_vectorE);
            // NodeReachLength
            for(i=0;i<number_of_nodes;i++)  {
              node_value_vectorArea[i] =  1.0;
    		}

			m_st->EdgeIntegration(m_pcs, nodes_vector, node_value_vectorArea);   
            for(i=0;i<number_of_nodes;i++)  {
              //double test = node_value_vectorArea[i];
              node_value_vectorArea[i] = node_value_vectorArea[i] * 1.0 ;
              
            }
 
          }  /* end if River */
 

          //------------------------------------------------------------------------------------
          if(dit_ply==6 || dit_ply == 8)  {  //CriticalDepth or NormalDepth

            node_value_vectorArea.resize(number_of_nodes);

            // NodeReachLength
            for(i=0;i<number_of_nodes;i++)  {
              node_value_vectorArea[i] =  1.0;  //Element width !
    		}
			m_st->EdgeIntegration(m_pcs, nodes_vector, node_value_vectorArea);   
            
          }  /* CriticalDepth */

          //------------------------------------------------------------------------------------
         
          if(dit_ply==3||dit_ply==4)  

			if(m_msh->GetMaxElementDim()==1) // 1D  //WW MB
	          m_st->DomainIntegration(m_pcs, nodes_vector, node_value_vector);
            else
              m_st->EdgeIntegration(m_pcs, nodes_vector, node_value_vector);   
		  
          double test=0.0;
          for(i=0;i<number_of_nodes;i++){
            m_node_value = new CNodeValue();
            m_node_value->msh_node_number = -1; 
            m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
            m_node_value->geo_node_number = nodes_vector[i];
            m_node_value->node_distype = 0;
            m_node_value->node_value = node_value_vector[i];
            m_node_value->tim_type_name = m_st->tim_type_name; //OK
            if(m_st->conditional)
              m_node_value->msh_node_number_conditional = nodes_vector_conditional[i];
            test = test + m_node_value->node_value;
            // River
            if(dit_ply ==5)  {
              m_node_value->node_distype = 5;
              m_node_value->node_area = node_value_vectorArea[i];
              m_node_value->node_parameterA = node_value_vectorA[i];
              m_node_value->node_parameterB = node_value_vectorB[i];
              m_node_value->node_parameterC = node_value_vectorC[i];
              m_node_value->node_parameterD = node_value_vectorD[i];
              m_node_value->node_parameterE = node_value_vectorE[i];  
            }
            if(dit_ply ==6)  {
              m_node_value->node_distype = 6;
              m_node_value->node_area = node_value_vectorArea[i];
            } 
            if(dit_ply ==7)  {
              m_node_value->node_distype = 7;
            }
            if(dit_ply ==8)  {
              m_node_value->node_distype = 8;
            }
            m_node_value->CurveIndex = m_st->CurveIndex;
            //WW        group_vector.push_back(m_node_value);
            //WW        st_group_vector.push_back(m_st); //OK
            m_pcs->st_node_value.push_back(m_node_value);  //WW
            m_pcs->st_node.push_back(m_st); //WW
        }
        node_value_vector.clear();
	
        //delete(values);
        Free(nodes); //WW  To be deleted
       }
    

      }
      //------------------------------------------------------------------
      if(m_st->geo_type_name.compare("DOMAIN")==0) {
        GEOGetNodesInMaterialDomain(m_msh,m_st->analytical_material_group,nodes_vector,false);
        number_of_nodes = (long)nodes_vector.size();
        for(i=0;i<number_of_nodes;i++){
          m_node_value = new CNodeValue();
          m_node_value->msh_node_number = -1; 
          m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
          m_node_value->geo_node_number = nodes_vector[i];
          m_node_value->node_distype = 0;
          m_node_value->node_value = 0;
          //WW        group_vector.push_back(m_node_value);
          //WW        st_group_vector.push_back(m_st); //OK
           m_pcs->st_node_value.push_back(m_node_value);  //WW
           m_pcs->st_node.push_back(m_st); //WW
        }
      }
      
      //------------------------------------------------------------------
      // "SURFACE" has to be changed to "FACE". WW
      if(m_st->geo_type_name.compare("SURFACE")==0) {
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(m_st->geo_name);//CC
        //m_suface->
        if(m_surface) {
          int dit_ply = -1;
          if(m_st->dis_type_name.compare("CONSTANT")==0) dit_ply = 1;
          if(m_st->dis_type_name.compare("LINEAR")==0) dit_ply = 2;
          if(m_st->dis_type_name.compare("CONSTANT_NEUMANN")==0) dit_ply = 3;
          if(m_st->dis_type_name.compare("LINEAR_NEUMANN")==0) dit_ply = 4;
          if(m_msh){ //MSH
             m_msh->GetNODOnSFC(m_surface,nodes_vector);
          }
          else{
            //WW nodes_vector = m_surface->GetMSHNodesClose();  
            if(pcs_type_name.compare("OVERLAND_FLOW")==0)
              GetMSHNodesOnSurfaceXY(m_surface,nodes_vector); //CC
            else
              GetMSHNodesOnSurface(m_surface,nodes_vector);//CC
          }
          long nodes_vector_length = (long)nodes_vector.size();
          node_value_vector.resize(nodes_vector_length);         

          for(i=0;i<nodes_vector_length;i++)  {
            node_value_vector[i] =  m_st->geo_node_value;
		  }
 
          //..............................................................
          if(m_st->conditional){
            nodes_vector_conditional.resize(nodes_vector_length);
            m_st->SetNOD2MSHNOD(nodes_vector, nodes_vector_conditional);
          }
          //..............................................................


          // Piecewise linear distributed. WW
          if(dit_ply==2||dit_ply==4)
          {
             // Linear interpolation  polygon-wise. WW
             //  list<CGLPolyline*>::const_iterator p = m_surface->polyline_of_surface_list.begin();
             vector<CGLPolyline*>::iterator p = m_surface->polyline_of_surface_vector.begin();
             p = m_surface->polyline_of_surface_vector.begin();
             while(p!=m_surface->polyline_of_surface_vector.end())
             {
                m_polyline = *p;
                for(k=0;k<(int)m_st->DistribedBC.size(); k++)
                {              
                   for(l=0;l<(int)m_polyline->point_vector.size(); l++)
                   {
                      if(m_st->PointsHaveDistribedBC[k]==m_polyline->point_vector[l]->id)
                      {
                         if(fabs(m_st->DistribedBC[k])<MKleinsteZahl) m_st->DistribedBC[k] = 1.0e-20;
                         m_polyline->point_vector[l]->property = m_st->DistribedBC[k];
                         break;
                      }
                   }
                }
               // InterpolationAlongPolyline(m_polyline, node_value_vector);
                p++;
            }
         }
          
		  if(dit_ply==3||dit_ply==4)  
          
          //if(max_dim==2) m_st->FaceIntegration(m_pcs, nodes_vector, node_value_vector);
          //if(max_dim==1) m_st->DomainIntegration(m_pcs, nodes_vector, node_value_vector);

          //Msh dependent MB
          if(m_msh->GetMaxElementDim()==2)     // For all meshes with 1-D or 2-D elements
            m_st->DomainIntegration(m_pcs, nodes_vector, node_value_vector);
          //
          else if(m_msh->GetMaxElementDim()==3)  // For all meshes with 3-D elements
            m_st->FaceIntegration(m_pcs, nodes_vector, node_value_vector);
         

          for(i=0;i<nodes_vector_length;i++){
            m_node_value = new CNodeValue();
            m_node_value->msh_node_number = -1;
            m_node_value->msh_node_number = nodes_vector[i]+ShiftInNodeVector;
            m_node_value->geo_node_number = nodes_vector[i];
            m_node_value->node_value = node_value_vector[i];
            m_node_value->CurveIndex = m_st->CurveIndex;
            if(m_st->conditional)
              m_node_value->msh_node_number_conditional = nodes_vector_conditional[i];
           
            //WW        group_vector.push_back(m_node_value);
            //WW        st_group_vector.push_back(m_st); //OK
            m_pcs->st_node_value.push_back(m_node_value);  //WW
            m_pcs->st_node.push_back(m_st); //WW
          }
        }
      }
     //====================================================================
    }
  }
//......................................................
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
void CSourceTerm::EdgeIntegration(CRFProcess* m_pcs, vector<long>&nodes_on_ply, 
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

  CFEMesh* msh = m_pcs->m_msh;
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
                            Weight *= 2.0*pai*radius;
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
                            Weight *= 2.0*pai*radius;
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
void CSourceTerm::FaceIntegration(CRFProcess* m_pcs, vector<long>&nodes_on_sfc, 
                                vector<double>&node_value_vector)
{
  if(!m_pcs->m_msh)
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

               vn[2] += m_polyline->point_vector[i]->property;
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

               vn[0] =  m_polyline->point_vector[i]->property;
               vn[1] =  m_polyline->point_vector[k]->property;

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
  CFEMesh* msh = m_pcs->m_msh;
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
void CSourceTerm::DomainIntegration(CRFProcess* m_pcs, vector<long>&nodes_in_dom, 
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
  CFEMesh* msh = m_pcs->m_msh;
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
//double CSourceTermGroup::GetConditionalNODValue(int i,CSourceTerm* m_st)
double GetConditionalNODValue(CSourceTerm* m_st, CNodeValue* cnodev) //WW
{
  int nidx;
  double value_cond = 0.0;
  double NodeReachLength;
  CRFProcess* m_pcs_cond = NULL;
  CRFProcess* m_pcs_this = NULL;
  long node_cond;

  m_pcs_this = PCSGet(m_st->pcs_type_name);
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
Task:
Programing:
02/2006 MB Implementation
02/2006 WW Change argument
**************************************************************************/
//double CSourceTermGroup::GetRiverNODValue(int i,CSourceTerm* m_st, long msh_node) //WW
double GetRiverNODValue(CNodeValue* cnodev,CSourceTerm* m_st, long msh_node) //WW
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
  double value;
  /* // Commented by WW
  paraA = group_vector[i]->node_parameterA; //HRiver
  paraB = group_vector[i]->node_parameterB; //KRiverBed
  paraC = group_vector[i]->node_parameterC; //WRiverBed
  paraD = group_vector[i]->node_parameterD; //TRiverBed
  paraE = group_vector[i]->node_parameterE; //BRiverBed
  NodeReachLength = group_vector[i]->node_area;
  */
  //WW
  paraA = cnodev->node_parameterA; //HRiver
  paraB = cnodev->node_parameterB; //KRiverBed
  paraC = cnodev->node_parameterC; //WRiverBed
  paraD = cnodev->node_parameterD; //TRiverBed
  paraE = cnodev->node_parameterE; //BRiverBed
  NodeReachLength = cnodev->node_area;
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
  h = m_pcs_this->GetNodeValue(msh_node,nidx1);
        
  if(h > paraE)  {  //HAquiver > BRiverBed
    //q = (RiverConductance * HRiver)   -  (RiverConductance * HAquifer)  
    value = RiverConductance * paraA; 
    MXInc(msh_node,msh_node,RiverConductance);
  } 
  if(h < paraE)  {  //HAquiver < BRiverBed
    //q = (RiverConductance * HRiver)   - (RiverConductance * BRiverBed)  
    value = RiverConductance * (paraA - paraE);
  }
  if(h == paraE) value = 0.;
  //_________________________________________________________________
  //Safe Flux values
  int nidxFLUX = m_pcs_this->GetNodeValueIndex("FLUX")+1;
  double flux = value / NodeReachLength  ;  //fluxes in m^2/s !!
  m_pcs_this->SetNodeValue(msh_node, nidxFLUX, flux);

  return value;
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB Implementation
02/2006 WW Change argument
**************************************************************************/
//double CSourceTermGroup::GetCriticalDepthNODValue(CNodeValue* cnodev,CSourceTerm* m_st, long msh_node)
double GetCriticalDepthNODValue(CNodeValue* cnodev,CSourceTerm* m_st, long msh_node)
{
  double value;
  double H;
  int AnzNodes = 0;
  double Haverage = 0;
  long msh_ele;
  long temp2;
  int j;


  CRFProcess* m_pcs_this = NULL;
  
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  

  //MB for CriticalDepth
//WW  for(j=0;j<(long)group_vector.size();j++) {
  for(j=0;j<(long)m_pcs_this->st_node_value.size();j++) {
    if (m_pcs_this->st_node_value[j]->node_distype==6){
      temp2 = m_pcs_this->st_node_value[j]->msh_node_number;
      Haverage += m_pcs_this->GetNodeValue(temp2,1);
      AnzNodes += 1;
    }
  }   
  Haverage = Haverage / AnzNodes;

//WW  double test1 = m_pcs_this->GetNodeValue(msh_node,1);
//WW  double test2 = m_pcs_this->m_msh->nod_vector[msh_node]->Z();

  H = m_pcs_this->GetNodeValue(msh_node,1) - m_pcs_this->m_msh->nod_vector[msh_node]->Z(); 
         
  if (H < 0.0)  {H = 0.0;}      
  double epsilon = 1.e-5; // like in pcs->assembleParabolicEquationNewton
  double H3 = pow(H,3.);
  double H3_epsilon = pow(H+epsilon,3.);
  double value_jacobi;
  double width = cnodev->node_area;
  if(m_pcs_this->m_msh->GetMaxElementDim() ==1 ) {
    msh_ele = m_pcs_this->m_msh->nod_vector[msh_node]->connected_elements[0]; 
    int group = m_pcs_this->m_msh->ele_vector[msh_ele]->GetPatchIndex();
    width = mmp_vector[group]->channel_width; 
  }
  value = -  sqrt(9.8066500000 * H3) * width;   
  value_jacobi =  sqrt(9.8066500000 * H3_epsilon) * width + value;  
  MXInc(msh_node,msh_node, value_jacobi / epsilon);      // write source term into jacobi  
  //cout << value << endl;
  //printf("\n Node %d: Depth Hmobile valueAdd %e %e %e ", msh_node, GetNodeVal(msh_node,1), H, value);
  // rausschreiben der Flux Werte 
  double Haverage3 = pow(Haverage,3);
  double temp = - 1 * sqrt(9.80665000000000 * Haverage3);
  temp = temp * width;
  m_pcs_this->SetNodeValue(msh_node,m_pcs_this->GetNodeValueIndex("FLUX")+0,temp*AnzNodes);

  return value;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2006 MB JOD Implementation
**************************************************************************/
//double CSourceTermGroup::GetNormalDepthNODValue(CSourceTerm* m_st, long msh_node)
double GetNormalDepthNODValue(CSourceTerm* m_st, long msh_node)
{
  double value;
  double H = 0.0;
//WW  int AnzNodes = 0;
//WW  double Haverage = 0;
  long msh_ele;

  CFEMesh* m_msh = NULL;
  CElem *m_ele = NULL;
  CRFProcess* m_pcs_this = NULL;
  m_pcs_this = PCSGet(m_st->pcs_type_name);
  m_msh = m_pcs_this->m_msh;
  
  double power, fric, width, temp;
  int group;
  if(m_msh->GetMaxElementDim() > 1)
	cout << "!!!!! NORMAL DEPTH only for one dimensional river flow implemented !!!!!" << endl;
    
  H = m_pcs_this->GetNodeValue(msh_node,1) - m_msh->nod_vector[msh_node]->Z(); 
  if (H < 0.0)  {H = 0.0;}      
  double epsilon = 1.e-5; // pcs->assembleParabolicEquationNewton !!!!!!!!!
  double H_epsilon = H + epsilon;
  double value_for_jacobi;
  msh_ele = m_msh->nod_vector[msh_node]->connected_elements[0]; 
  m_ele = m_msh->ele_vector[msh_ele];
  double elementlength =sqrt( 
	pow(m_ele->GetNode(1)->X()- m_ele->GetNode(0)->X(),2.)
	+ pow(m_ele->GetNode(1)->Y()- m_ele->GetNode(0)->Y(),2.)  
	+ pow(m_ele->GetNode(1)->Z()- m_ele->GetNode(0)->Z(),2.) ); 
  double S_0 =  (m_ele->GetNode(1)->Z() - m_ele->GetNode(0)->Z() ) / elementlength;
  
  if(S_0 < 0) S_0 = -S_0;
  group = m_msh->ele_vector[msh_ele]->GetPatchIndex();
  width = mmp_vector[group]->channel_width;
  fric  = mmp_vector[group]->friction_coefficient;
  if(mmp_vector[group]->friction_model == 2) {
    temp = width * sqrt( S_0 ) * fric;
    power = 1./2.; // Chezy 
  }
  else {
	temp = width * sqrt( S_0 ) / fric;
    power = 2./3.;  // Manning
  }
  value = - pow(H * width / (2 * H + width), power) * H * temp;    
  value_for_jacobi =  pow(H_epsilon * width / (2 * H_epsilon + width), power ) * H_epsilon * temp + value  ;  
  MXInc(msh_node,msh_node, value_for_jacobi / epsilon);      // write source term into jacobi  
  cout << value << endl;
  m_pcs_this->SetNodeValue(msh_node, m_pcs_this->GetNodeValueIndex("FLUX")+0, value);

  return value;
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
07/2005 OK Implementation based on CSourceTermGroup::Set
**************************************************************************/
void CSourceTermGroup::SetPLY(CSourceTerm*m_st)
{
  int i,k,l;
  long ply_nod_vector_size;
  double parameter;
  vector<long>ply_nod_vector;
  //OK vector<long>ply_nod_vector_ordered;
  vector<double>ply_nod_val_vector;
  CNodeValue *m_nod_val = NULL;
  //----------------------------------------------------------------------
  CGLPolyline* m_ply = NULL;
  m_ply = GEOGetPLYByName(m_st->geo_name);//CC
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet(m_st->pcs_type_name);
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(m_st->pcs_type_name);
  //======================================================================
  if(m_ply&&m_msh){
    //--------------------------------------------------------------------
    m_ply->GetPointOrderByDistance(); //WW ordering
    m_msh->GetNODOnPLY(m_ply,ply_nod_vector);
    //nodes = m_polyline->GetNodesOnArc(number_of_nodes); //WW
    ply_nod_vector_size = (long)ply_nod_vector.size();
    ply_nod_val_vector.resize(ply_nod_vector_size);
    //--------------------------------------------------------------------
    for(i=0;i<ply_nod_vector_size;i++){
      m_nod_val = new CNodeValue();
      m_nod_val->msh_node_number = -1; // constructor
      m_nod_val->msh_node_number = m_ply->OrderedPoint[i]; //+ShiftInNodeVector; //WW
      //m_nod_val->geo_node_number = nodes[i];
      m_nod_val->node_distype = 0;
      m_nod_val->node_value = ply_nod_val_vector[i];
      m_nod_val->CurveIndex = m_st->CurveIndex;
    }
    //--------------------------------------------------------------------
    // DIS type definition to read funcion
    int dit_ply = -1;
    if(m_st->dis_type_name.compare("CONSTANT")==0) dit_ply = 1;
    if(m_st->dis_type_name.compare("LINEAR")==0) dit_ply = 2;
    if(m_st->dis_type_name.compare("CONSTANT_NEUMANN")==0) dit_ply = 3;
    if(m_st->dis_type_name.compare("LINEAR_NEUMANN")==0) dit_ply = 4;
    if(m_st->dis_type_name.compare("RIVER")==0) dit_ply = 5;
    if(m_st->dis_type_name.compare("CRITICALDEPTH")==0) dit_ply = 6;
    if(m_st->dis_type_name.compare("SYSTEM_DEPENDENT")==0) dit_ply = 7;   //YD    
    //--------------------------------------------------------------------
    // CONSTANT
    if(dit_ply==1){
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = m_st->geo_node_value;
      }
    }
    //--------------------------------------------------------------------
    // LINEAR distributed single parameter data
    if(dit_ply==2||dit_ply==4){
      for(k=0;k<(int)m_st->DistribedBC.size();k++)  {              
        for(l=0;l<(int)m_ply->point_vector.size(); l++) {
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id){
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            m_ply->point_vector[l]->property = m_st->DistribedBC[k];
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector); //WWOK
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
    }
    //--------------------------------------------------------------------
    // LINEAR distributed multiple parameter data
    if(dit_ply==5){
      //..................................................................
      // HRiver 
      for(k=0;k<(int)m_st->DistribedBC.size();k++){              
        for(l=0;l<(int)m_ply->point_vector.size();l++){
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id)  {
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            parameter = m_st->pnt_parameter_vector[k][0];
            m_ply->point_vector[l]->property = parameter;
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector);
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
      //..................................................................
      // KRiverBed
      for(k=0;k<(int)m_st->DistribedBC.size();k++){              
        for(l=0;l<(int)m_ply->point_vector.size();l++){
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id)  {
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            parameter = m_st->pnt_parameter_vector[k][1];
            m_ply->point_vector[l]->property = parameter;
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector);
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
      //..................................................................
      // WRiverBed
      for(k=0;k<(int)m_st->DistribedBC.size();k++){              
        for(l=0;l<(int)m_ply->point_vector.size();l++){
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id)  {
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            parameter = m_st->pnt_parameter_vector[k][2];
            m_ply->point_vector[l]->property = parameter;
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector);
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
      //..................................................................
      // TRiverBed
      for(k=0;k<(int)m_st->DistribedBC.size();k++){              
        for(l=0;l<(int)m_ply->point_vector.size();l++){
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id)  {
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            parameter = m_st->pnt_parameter_vector[k][3];
            m_ply->point_vector[l]->property = parameter;
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector);
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
      //..................................................................
      // BRiverBed
      for(k=0;k<(int)m_st->DistribedBC.size();k++){              
        for(l=0;l<(int)m_ply->point_vector.size();l++){
          if(m_st->PointsHaveDistribedBC[k]==m_ply->point_vector[l]->id)  {
            if(fabs(m_st->DistribedBC[k]) < MKleinsteZahl) 
              m_st->DistribedBC[k] = 1.0e-20;
            parameter = m_st->pnt_parameter_vector[k][4];
            m_ply->point_vector[l]->property = parameter;
            break;
          }
        } 
      }
      InterpolationAlongPolyline(m_ply,ply_nod_val_vector);
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_value = ply_nod_val_vector[i];
      }
      //..................................................................
      // NodeReachLength
      for(i=0;i<ply_nod_vector_size;i++)  {
        ply_nod_val_vector[i] =  1.0;
      }
      if(max_dim==1) 
        if(m_pcs)
          m_st->EdgeIntegration(m_pcs,ply_nod_vector,ply_nod_val_vector);
      for(i=0;i<ply_nod_vector_size;i++)  {
        ply_nod_val_vector[i] *= 12.5 * 0.2 / 2 ;
      }
      //..................................................................
      for(i=0;i<ply_nod_vector_size;i++){
        m_nod_val->node_area = ply_nod_val_vector[i];
      }
    }
    //--------------------------------------------------------------------
    // LINEAR distributed multiple parameter data
    if(dit_ply==6){ // CriticalDepth
      //..................................................................
      // NodeReachLength
      for(i=0;i<ply_nod_vector_size;i++)  {
        ply_nod_val_vector[i] =  1.0;
      }
      //m_st->FaceIntegration(nodes,node_value_vectorArea);
    }
    //....................................................................
    if(dit_ply==3||dit_ply==4){
      //m_st->FaceIntegration(nodes, node_value_vector);   
    }
    //--------------------------------------------------------------------
    //WW        group_vector.push_back(m_nod_val);
    //WW        st_group_vector.push_back(m_st); //OK
    m_pcs->st_node_value.push_back(m_nod_val);  //WW
    m_pcs->st_node.push_back(m_st); //WW
    //--------------------------------------------------------------------
    // Memory
    ply_nod_vector.clear();
    ply_nod_val_vector.clear();
    //--------------------------------------------------------------------
  }
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
FEMLib-Method:
Task:
Programing:
11/2005 CMCD Implementation
04/2006 Moved from CSourceTermGroup and changed the arguments
last modification:
**************************************************************************/
//double CSourceTermGroup::GetAnalyticalSolution(CSourceTerm *m_st, long node_number, string primvar)
double GetAnalyticalSolution(long node_number, CSourceTerm *m_st)
{
  int idx, n;
  int size, process_no;
  long i, group;
  double value, source, gradient, ref_value;
  double timevalue;
  double fac = 1.0;
  double temp_time,temp_value;
  double pi = 3.1415926;
  double D = m_st->analytical_diffusion;
  double t0, tn, tnn, val1,val2, area;
  double tvol, vol;
  bool out = false;
  string primvar; //WW
  CNode* Node = NULL;
  CElem* Ele = NULL; 
  vector<double>time_history;
  vector<double>value_history;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(m_st->pcs_type_name); //OK
  CFEMesh* m_msh = m_pcs->m_msh;  //WW
  Node = m_msh->nod_vector[node_number]; //WW
  primvar = m_st->pcs_pv_name; //WW
  t0 = tn = tnn = source = gradient = val1 = val2 = area = 0.0;
  idx = m_pcs->GetNodeValueIndex(primvar);
  value = m_pcs->GetNodeValue(node_number,idx);
  timevalue = aktuelle_zeit;
  size = (int)analytical_processes.size();
  process_no = 0;
  for (i = 0; i < size; i++){
     if (analytical_processes[i] == primvar) process_no = i;
  }
  process_no*=2;//;
  //Save data in a vector attached to the nodes
  SetNodePastValue(node_number,process_no,0,m_st->number_of_terms,timevalue);
  SetNodePastValue(node_number,process_no+1,0,m_st->number_of_terms,value);
  //Recall historical data
  ref_value = GetNodePastValueReference (node_number, process_no+1);
  for (i = 0; i <m_st->number_of_terms; i++){ 
    temp_time = GetNodePastValue(node_number,process_no,i);
    temp_value = (GetNodePastValue(node_number,process_no+1,i)-ref_value);
    time_history.push_back(temp_time);
    value_history.push_back(temp_value);
  }
  //Calculate individual terms and sum  
  if ((node_number == 32)&&(aktuelle_zeit ==2.4e+006 )) out = true;
  for ( i = m_st->number_of_terms-1; i >0 ; i--){ 
    t0 = time_history[0];
    if (i == m_st->number_of_terms-1) tn = (t0-time_history[i])+(time_history[i-1]-time_history[i]);
    else tn = t0-time_history[i+1];
    tnn= t0-time_history[i];
    val1 = 1/(sqrt(pi*D*tn));
    val2 = 1/(sqrt(pi*D*tnn));
    gradient += ((val2-val1)*value_history[i]);
  }
  tn = t0-time_history[1];
  tnn = 0;
  val1 = 1/(sqrt(pi*D*tn));
  gradient -=(val1*value_history[0]);
  //Include area represented by node
  //Area only good for lines, triangles and quads.
  Node->connected_elements.size();
  tvol = 0.0;
  for (i = 0; i <  (int) Node->connected_elements.size(); i++){
     Ele = m_msh->ele_vector[ Node->connected_elements[i]];
     vol = Ele->GetVolume();
     n = Ele->GetVertexNumber();
     tvol += (vol/n);
  }
  CMediumProperties *Matprop;
  CFluidProperties *FluidProp;
  if (primvar == "TEMPERATURE1") {
    fac = 0.0;
    for (i = 0; i <  (int) Node->connected_elements.size(); i++){
      group = m_msh->ele_vector[ Node->connected_elements[i]]->GetPatchIndex();
      FluidProp = mfp_vector[group];
      Matprop = mmp_vector[group];
      fac += FluidProp->Density() * FluidProp->SpecificHeatCapacity()*Matprop->porosity;
    } 
  fac /= (double) Node->connected_elements.size();
  }
  else {
    fac = 0.0;
    for (i = 0; i <  (int) Node->connected_elements.size(); i++){
      group = m_msh->ele_vector[ Node->connected_elements[i]]->GetPatchIndex();
      Matprop = mmp_vector[group];
      fac += Matprop->porosity;
    } 
  fac /= (double) Node->connected_elements.size();
  } 
  area = tvol*2.;//* 2 because the diffusion is in two direction perpendicular to the fracture
  source = gradient * m_st->analytical_diffusion * area * fac;

  return source;
}
