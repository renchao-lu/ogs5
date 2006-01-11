/**************************************************************************
FEMLib - Object: OUT
Task: 
Programing:
06/2004 OK Implementation
last modified:
**************************************************************************/
#include "stdafx.h" /* MFC */
#include "makros.h"
// C++ STL
#include <math.h>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;
// FEM-Makros
#include "makros.h"
#include "rfstring.h"
// GeoSys-GeoLib
#include "geo_strings.h"
#include "geo_ply.h"
#include "geo_sfc.h"
// GeoSys-FEMLib
#include "rf_out_new.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "elements.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "mathlib.h"
#include "fem_ele_std.h"
// GeoSys-MSHLib
#include "msh_lib.h"

using Mesh_Group::CFEMesh;
//==========================================================================
vector<COutput*>out_vector;
/**************************************************************************
FEMLib-Method: 
Task: OUT constructor
Programing:
01/2004 OK Implementation
**************************************************************************/
COutput::COutput(void): out_amplifier(0.0), m_msh(NULL), nSteps(-1)
{
  delimiter_type = ":";
  msh_node_number = -1;
  INITIAL_CONDITIONS = false;
  tim_type_name = "TIMES";
  dat_type_name = "TECPLOT";
  geo_type_name = "DOMAIN";
  selected = false;
}
/**************************************************************************
FEMLib-Method: 
Task: OUT deconstructor
Programing:
04/2004 OK Implementation
**************************************************************************/
COutput::~COutput(void) 
{
}
/**************************************************************************
FEMLib-Method: 
Task: OUT read function
Programing:
06/2004 OK Implementation
07/2004 WW Remove old files
11/2004 OK string streaming by SB for lines
03/2005 OK PCS_TYPE
12/2005 OK DIS_TYPE
12/2005 OK MSH_TYPE
**************************************************************************/
ios::pos_type COutput::Read(ifstream *out_file)
{
  char buffer[MAX_ZEILE];
  string sub_line;
  string line_string;
  string delimiter(" ");
  bool new_keyword = false;
  string hash("#");
  ios::pos_type position;
  string sub_string;
  bool new_subkeyword = false;
  string dollar("$");
  string tec_file_name;
  ios::pos_type position_line;
  bool ok = true;
  std::stringstream line;
  ios::pos_type position_subkeyword;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while(!new_keyword){
    position = out_file->tellg();
    if(new_subkeyword)
      out_file->seekg(position_subkeyword,ios::beg);
    new_subkeyword = false;
    out_file->getline(buffer,MAX_ZEILE);
    line_string = buffer;
	if(line_string.size()<1) // empty line
      continue; 
    if(Keyword(line_string)) 
      return position;
    //--------------------------------------------------------------------
    if(line_string.find("$NOD_VALUES")!=string::npos) { // subkeyword found
      while ((!new_keyword)&&(!new_subkeyword)) {
        position_subkeyword = out_file->tellg();
        *out_file >> line_string>>ws;
        if(line_string.find(hash)!=string::npos) {
          return position;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
		    if(line_string.size()==0) 
          break; //SB: empty line
        nod_value_vector.push_back(line_string);
      }
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$ELE_VALUES")!=string::npos) { // subkeyword found
      ok = true;
      while(ok){
        position_line = out_file->tellg();
       *out_file >> line_string;
        if(SubKeyword(line_string)){
          out_file->seekg(position_line,ios::beg);
          ok = false;
          continue;
        }
        if(Keyword(line_string))
          return position;
        ele_value_vector.push_back(line_string);
        out_file->ignore(MAX_ZEILE,'\n');
      }
      // Remove files
      tec_file_name = file_base_name + "_domain_ele" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$GEO_TYPE")!=string::npos) { //subkeyword found
      //OK out_file->getline(buffer,MAX_ZEILE);
      //OK line_string = buffer;
	  line.str(GetLineFromFile1(out_file));
      line >> geo_type_name;
      if(geo_type_name.find("POINT")!=string::npos) {
        geo_type = 0;
        line >> geo_name;
        line.clear();
      }
      if(geo_type_name.find("POLYLINE")!=string::npos) {
        geo_type = 1;
        line >> geo_name;
        line.clear();
      }
      if(geo_type_name.find("SURFACE")!=string::npos) {
        geo_type = 2;
        line >> geo_name;
        line.clear();
      }
      if(geo_type_name.find("VOLUME")!=string::npos) {
        geo_type = 3;
        line >> geo_name;
        line.clear();
      }
      if(geo_type_name.find("DOMAIN")!=string::npos) {
        geo_type = 4;
		// Remove files
        tec_file_name = file_base_name + "_domain" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_line" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_quad" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_tri" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_pris" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_tet" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_hex" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + ".rfo"; //OK4105
        remove(tec_file_name.c_str());
      }
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$TIM_TYPE")!=string::npos) { // subkeyword found
      while ((!new_keyword)&&(!new_subkeyword)) {
        position_subkeyword = out_file->tellg();
       *out_file >> line_string;
        if(line_string.size()==0) //SB
          break;
        if(line_string.find(hash)!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
        if(line_string.find("STEPS")!=string::npos) {
         *out_file >> nSteps;
          tim_type_name = "STEPS"; //OK
        }
        else{
          time_vector.push_back(strtod(line_string.data(),NULL));
		}
        out_file->ignore(MAX_ZEILE,'\n');
      }
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$DAT_TYPE")!=string::npos) { // subkeyword found
     *out_file >> dat_type_name;
      out_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$AMPLIFIER")!=string::npos) { // subkeyword found
     *out_file>>out_amplifier;
      out_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
     *out_file >> pcs_type_name;
      out_file->ignore(MAX_ZEILE,'\n');
      // Remove files
      tec_file_name = pcs_type_name + "_" + "domain" + "_line" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      tec_file_name = pcs_type_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      tec_file_name = pcs_type_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      tec_file_name = pcs_type_name + "_" + "domain" + "_tet" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      tec_file_name = pcs_type_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      tec_file_name = pcs_type_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$DIS_TYPE")!=string::npos) { // subkeyword found
     *out_file>>dis_type_name;
      out_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$MSH_TYPE")!=string::npos) { // subkeyword found
     *out_file>>msh_type_name;
      out_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //--------------------------------------------------------------------
  }
  return position;
}

/**************************************************************************
FEMLib-Method: 
Task: OUT read function
Programing:
06/2004 OK Implementation
08/2004 WW Remove the old files
01/2005 OK Boolean type
01/2005 OK Destruct before read
**************************************************************************/
bool OUTRead(string file_base_name)
{
  //----------------------------------------------------------------------
  OUTDelete();  
  //----------------------------------------------------------------------
  COutput *m_out = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  char number_char[3];
  string number_string;
  string tec_file_name;
  //========================================================================
  // File handling
  string out_file_name = file_base_name + OUT_FILE_EXTENSION;
  ifstream out_file (out_file_name.data(),ios::in);
  if(!out_file.good()) 
    return false;
  out_file.seekg(0L,ios::beg);
  //========================================================================
  // Keyword loop
  cout << "OUTRead" << endl;
  while (!out_file.eof()) {
    out_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return true;
    //----------------------------------------------------------------------
    if(line_string.find("#OUTPUT")!=string::npos) { // keyword found
      m_out = new COutput();
      m_out->file_base_name = file_base_name;
      position = m_out->Read(&out_file);
      out_vector.push_back(m_out);
      out_file.seekg(position,ios::beg);
      // Remove the old files
	  if(m_out->geo_type==1){
	    for(int i=0; i<(int)m_out->time_vector.size(); i++){
          sprintf(number_char,"%i",i);
          number_string = number_char;
		  tec_file_name = file_base_name + "_ply_" + m_out->geo_name \
		                + "_t" + number_string + TEC_FILE_EXTENSION;
          remove(tec_file_name.c_str());
        }
	  }
      tec_file_name=file_base_name + "_time_" + m_out->geo_name + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      //  End remove files
    } // keyword found
  } // eof
  return true;
}

/**************************************************************************
FEMLib-Method: 
Task: write function
Programing:
06/2004 OK Implementation
01/2005 OK Extensions
12/2005 OK DIS_TYPE
12/2005 OK MSH_TYPE
**************************************************************************/
void COutput::Write(fstream* out_file)
{
  int i;
  //--------------------------------------------------------------------
  // KEYWORD
  *out_file  << "#OUTPUT" << endl;
  //--------------------------------------------------------------------
  // PCS_TYPE
  *out_file << " $PCS_TYPE" << endl;
  *out_file << "  ";
  *out_file << pcs_type_name << endl;
  //--------------------------------------------------------------------
  // NOD_VALUES
  *out_file << " $NOD_VALUES" << endl;
  int nod_value_vector_size = (int)nod_value_vector.size();
  for(i=0;i<nod_value_vector_size;i++){
    *out_file << "  ";
    *out_file << nod_value_vector[i] << endl;
  }
  //--------------------------------------------------------------------
  // ELE_VALUES
  *out_file << " $ELE_VALUES" << endl;
  int ele_value_vector_size = (int)ele_value_vector.size();
  for(i=0;i<ele_value_vector_size;i++){
    *out_file << "  ";
    *out_file << ele_value_vector[i] << endl;
  }
  //--------------------------------------------------------------------
  // GEO_TYPE
  *out_file << " $GEO_TYPE" << endl;
  *out_file << "  ";
  *out_file << geo_type_name << " " << geo_name << endl;
  //--------------------------------------------------------------------
  // TIM_TYPE
  *out_file << " $TIM_TYPE" << endl;
  for(i=0;i<(int)time_vector.size();i++){
    *out_file << "  ";
    *out_file << time_vector[i] << endl;
  }
  if((int)time_vector.size()==0)
    *out_file << "  STEPS 1" << endl;
  //--------------------------------------------------------------------
  // DIS_TYPE
  if(dis_type_name.size()>0){
    *out_file << " $DIS_TYPE" << endl;
    *out_file << "  ";
    *out_file << dis_type_name << endl;
  }
  //--------------------------------------------------------------------
  // MSH_TYPE
  if(msh_type_name.size()>0){
    *out_file << " $MSH_TYPE" << endl;
    *out_file << "  ";
    *out_file << msh_type_name << endl;
  }
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: OUTWrite
Task: master write function
Programing:
06/2004 OK Implementation
last modification:
**************************************************************************/
void OUTWrite(string base_file_name)
{
  COutput *m_out = NULL;
  string sub_line;
  string line_string;
  //========================================================================
  // File handling
  string out_file_name = base_file_name + OUT_FILE_EXTENSION;
  fstream out_file (out_file_name.data(),ios::trunc|ios::out);
  out_file.setf(ios::scientific,ios::floatfield);
  out_file.precision(12);
  if (!out_file.good()) return;
  out_file.seekg(0L,ios::beg);
  //========================================================================
  out_file << "GeoSys-OUT: Output ------------------------------------------------\n";
  //========================================================================
  // OUT vector
  int out_vector_size =(int)out_vector.size();
  int i;
  for(i=0;i<out_vector_size;i++){
    m_out = out_vector[i];
    m_out->Write(&out_file);
  }
  out_file << "#STOP";
  out_file.close();
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
08/2004 WW Output by steps given in .out file
03/2005 OK RFO format
05/2005 OK MSH
05/2005 OK Profiles at surfaces
12/2005 OK VAR,MSH,PCS concept
**************************************************************************/
void OUTData(double time_current, const int time_step_number)
{
  int i,j;
  int no_times;
  COutput *m_out = NULL;
  CRFProcess* m_pcs = NULL;
  CFEMesh* m_msh = NULL;
  bool OutputBySteps = false;
  //======================================================================
  for(i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
    //--------------------------------------------------------------------
    //....................................................................
    // MSH
    m_msh = m_out->GetMSH();
    if(!m_msh){
      cout << "Warning in OUTData - no MSH data" << endl;
      //OK continue;
    }
    //....................................................................
    // PCS
    if(m_out->nod_value_vector.size()>0)
      m_pcs = m_out->GetPCS(m_out->nod_value_vector[0]);
    if(m_out->ele_value_vector.size()>0)
      m_pcs = m_out->GetPCS_ELE(m_out->ele_value_vector[0]);
    if(!m_pcs){
      cout << "Warning in OUTData - no PCS data" << endl;
      continue;
    }
    if(!m_pcs->selected)
      continue;
    //--------------------------------------------------------------------
    m_out->time = time_current;
    no_times = (int)m_out->time_vector.size();
    //----------------------------------------------------------------------
    if (m_out->nSteps == -1) 
      m_out->nSteps = no_times;//CMCD
    if(no_times==0&&(m_out->nSteps>0)&&(time_step_number%m_out->nSteps==0))
    OutputBySteps = true; 
    //======================================================================
    // TECPLOT    
    if(m_out->dat_type_name.compare("TECPLOT")==0)
    {
      switch(m_out->geo_type_name[2]){
        //------------------------------------------------------------------
        case 'M': // domain data
          cout << "Data output: Domain" << endl;
          if(OutputBySteps)
          {
            m_out->NODWriteDOMDataTEC();
            m_out->ELEWriteDOMDataTEC();
            OutputBySteps = false;
          }
          else 
		  {
            for(j=0;j<no_times;j++){
              if(time_current>=m_out->time_vector[j]){
                m_out->NODWriteDOMDataTEC();
                m_out->ELEWriteDOMDataTEC();
                m_out->time_vector.erase(m_out->time_vector.begin()+j); 
                break;	  
            }
		  } 
        }
        break;
        //------------------------------------------------------------------
        case 'L': // profiles along polylines 
          cout << "Data output: Polyline profile - " << m_out->geo_name << endl;
          if(OutputBySteps)
	      {
            m_out->NODWritePLYDataTEC(time_step_number);
            OutputBySteps = false;
          } 
		  else
		  {
            for(j=0;j<no_times;j++){
              if(time_current>=m_out->time_vector[j]){
                m_out->NODWritePLYDataTEC(j);
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
                break;
              }
		    }
          }
        break;
        //------------------------------------------------------------------
        case 'I': // breakthrough curves in points
          cout << "Data output: Breakthrough curves - " << m_out->geo_name << endl;
          m_out->NODWritePNTDataTEC(time_current,time_step_number);
        break;
        //------------------------------------------------------------------
        case 'R': // profiles at surfaces
          cout << "Data output: Surface profile" << endl;
          //..............................................................
          if(m_out->dis_type_name.compare("AVERAGE")==0){
            if(OutputBySteps){
              m_out->NODWriteSFCAverageDataTEC(time_current,time_step_number);
              OutputBySteps = false;
            } 
		    else{
            }
          }
          //..............................................................
          else{
            if(OutputBySteps){
              m_out->NODWriteSFCDataTEC(time_step_number);
              OutputBySteps = false;
            } 
		    else{
              for(j=0;j<no_times;j++){
                if(time_current>=m_out->time_vector[j]){
                  m_out->NODWriteSFCDataTEC(j);
                  m_out->time_vector.erase(m_out->time_vector.begin()+j);
                  break;
                }
              }
            }
          }
          //..............................................................
        break;
        //------------------------------------------------------------------
      }
    }
    //--------------------------------------------------------------------
    // ROCKFLOW    
    else if(m_out->dat_type_name.compare("ROCKFLOW")==0){
      switch(m_out->geo_type_name[2]){
        case 'M': // domain data
          if(OutputBySteps)
		  {
		    OutputBySteps = false;
            m_out->WriteRFO(); //OK
		  }
		  else
		  {
            for(j=0;j<no_times;j++){
              if(time_current>=m_out->time_vector[j]){
                m_out->WriteRFO(); //OK
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
		        break;	 
              }
		    } 
          }
          break;
      }
    } 
    //======================================================================
  } // OUT loop
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
03/2005 OK MultiMSH
08/2005 WW Changes for MultiMSH
12/2005 OK VAR,MSH,PCS concept
**************************************************************************/
void COutput::NODWriteDOMDataTEC()
{
  int te=0;
  string eleType;
  string tec_file_name;
  //----------------------------------------------------------------------
  // Tests  
  if((int)nod_value_vector.size()==0)
    return;
  //......................................................................
  // MSH
  //m_msh = FEMGet(pcs_type_name);
  m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::NODWriteDOMDataTEC() - no MSH data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  // File name handling
  if((int)fem_msh_vector.size()>1) // MultiMSH
    tec_file_name = file_base_name + "_" + msh_type_name + "_" + "domain";
  else
    tec_file_name = file_base_name + "_" + "domain";
  //======================================================================
  if(m_msh){//WW
    if(msh_no_line>0)
	{
      tec_file_name += "_line";
      eleType = "QUADRILATERAL"; 
	  te=1;
	}
	else if (msh_no_quad>0)
	{
      tec_file_name += "_quad";
      eleType = "QUADRILATERAL"; 
	  te=2;
    }
	else if (msh_no_hexs>0)
	{
      tec_file_name += "_hex";
      eleType = "BRICK"; 
	  te=3;
    }
	else if (msh_no_tris>0)
	{
      tec_file_name += "_tri";
//???Who was this eleType = "TRIANGLE";
      eleType = "QUADRILATERAL";
	  te=4;
    }
	else if (msh_no_tets>0)
	{
      tec_file_name += "_tet";
	  eleType = "TETRAHEDRON"; 
      te=5;
    }
	else if (msh_no_pris>0)
	{
      tec_file_name += "_pris";
      eleType = "BRICK"; 
	  te=6;
    }
    tec_file_name += TEC_FILE_EXTENSION;
    fstream tec_file (tec_file_name.data(),ios::app|ios::out);
    tec_file.setf(ios::scientific,ios::floatfield);
    tec_file.precision(12);
    if (!tec_file.good()) return;
    WriteTECHeader(tec_file,te,eleType);
    WriteTECNodeData(tec_file);
    WriteTECElementData(tec_file,te);
  }
  //======================================================================
  // RFI
  else{
    //--------------------------------------------------------------------
    // line elements
    if(msh_no_line>0){
    //string tec_file_name = file_base_name + "_" + out_type_name + "_line" + TEC_FILE_EXTENSION;
    //string tec_file_name = pcs_type_name + "_" + "domain" + "_line" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_line" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
      WriteTECHeader(tec_file,1,"QUADRILATERAL");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,1);
    }
    //--------------------------------------------------------------------
    // tri elements
    if(msh_no_tris>0){
    //string tec_file_name = pcs_type_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
      fstream tec_file1 (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file1.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file1.setf(ios::scientific,ios::floatfield);
      tec_file1.precision(12);
      if (!tec_file1.good()) return;
      //OK  tec_file1.clear();
      //OK  tec_file1.seekg(0L,ios::beg);
      WriteTECHeader(tec_file1,4,"TRIANGLE");
      WriteTECNodeData(tec_file1);
      WriteTECElementData(tec_file1,4);
      // tec_file1.close();
    }
    //--------------------------------------------------------------------
    // quad elements
    if(msh_no_quad>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
      WriteTECHeader(tec_file,2,"QUADRILATERAL");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,2);
    }
    //--------------------------------------------------------------------
    // tet elements
    if(msh_no_tets>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_tet" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_tet" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
      WriteTECHeader(tec_file,5,"TETRAHEDRON");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,5);
    }
    //--------------------------------------------------------------------
    // pris elements
    if(msh_no_pris>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
      WriteTECHeader(tec_file,6,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,6);
    }
    //--------------------------------------------------------------------
    // hex elements
    if(msh_no_hexs>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
#ifdef SX
      char sxbuffer[4096*4096];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);
#ifdef SX
      tec_file.rdbuf()->pubsetbuf(sxbuffer,4096*4096);
#endif
      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
      WriteTECHeader(tec_file,3,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,3);
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Programing:
08/2004 OK Implementation
08/2004 WW Output node variables by their names given in .out file
03/2005 OK MultiMSH
08/2005 WW Correction of node index
12/2005 OK Mass transport specifics
OK ??? too many specifics
**************************************************************************/
void COutput::WriteTECNodeData(fstream& tec_file)
{
  const int nName = (int)nod_value_vector.size();
  Knoten *node = NULL;
  long j;
  double x[3];
  int i, k;
  int nidx, nidx_dm[3];
  vector<int> NodeIndex(nName);
  string nod_value_name; //OK
  int m;
  int timelevel;
  //----------------------------------------------------------------------
  m_msh = GetMSH();
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  CRFProcess* m_pcs_out = NULL;
  //======================================================================
  // MSH
  if(m_msh){
    //--------------------------------------------------------------------
    for(k=0;k<nName;k++){
      m_pcs = PCSGet(nod_value_vector[k],true);
      if(m_pcs != NULL){
      NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
      //??????????????????????????????????????????????????????????????????
	  if(nod_value_vector[k].find("SATURATION")!=string::npos)
	  {
        NodeIndex[k]++;
        continue;           
	  }
      for(i=0; i<m_pcs->GetPrimaryVNumber(); i++) 
      {
        if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
        { 
          NodeIndex[k]++;
          break;
        }
      }
      }
    }
    //--------------------------------------------------------------------
    //????????????????????????????????????????????????????????????????????
    if(M_Process||MH_Process)  //WW
    {
      m_pcs = PCSGet("DISPLACEMENT_X1",true);
      nidx_dm[0] = m_pcs->GetNodeValueIndex("DISPLACEMENT_X1")+1;  
      nidx_dm[1] = m_pcs->GetNodeValueIndex("DISPLACEMENT_Y1")+1;  
      if(max_dim>1)
        nidx_dm[2] = m_pcs->GetNodeValueIndex("DISPLACEMENT_Z1")+1;
      else  
        nidx_dm[2] = -1;
    }
    //....................................................................
    for(j=0l;j<m_msh->GetNodesNumber(false);j++)
    {
      //..................................................................
      // XYZ
      x[0] = m_msh->nod_vector[j]->X();
      x[1] = m_msh->nod_vector[j]->Y();
      x[2] = m_msh->nod_vector[j]->Z();
      // Amplifying DISPLACEMENTs
      if(M_Process||MH_Process)  //WW
      {
        for(k=0;k<max_dim+1;k++)
          x[k] += out_amplifier*m_pcs->GetNodeValue(m_msh->nod_vector[j]->GetIndex(), nidx_dm[k]);
      }
      for(i=0;i<3;i++)
        tec_file << x[i] << " ";
      //..................................................................
      // NOD values
      //..................................................................
      // Mass transport
      if(pcs_type_name.compare("MASS_TRANSPORT")==0){
        for(i=0;i<(int)nod_value_vector.size();i++){
          nod_value_name = nod_value_vector[i];
          for(int l=0;l<(int)pcs_vector.size();l++){
            m_pcs = pcs_vector[l];
            if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0){
              timelevel = 0;
              for(m=0;m<(int)m_pcs->nod_val_name_vector.size();m++){
                if(m_pcs->nod_val_name_vector[m].compare(nod_value_name)==0){
                  m_pcs_out = PCSGet(pcs_type_name,nod_value_name);
                  if(timelevel==1){
                    nidx = m_pcs_out->GetNodeValueIndex(nod_value_name)+timelevel;
                    tec_file << m_pcs_out->GetNodeValue(m_msh->nod_vector[j]->GetIndex(),nidx) << " ";
                  }
                  timelevel++;
                }
              }
            }
          }
        }
      }
      //..................................................................
      else{
        for(k=0;k<nName;k++){
          m_pcs = GetPCS(nod_value_vector[k]);
          if(m_pcs != NULL)
           if(NodeIndex[k]>-1)
            tec_file << m_pcs->GetNodeValue( m_msh->nod_vector[j]->GetIndex(),NodeIndex[k]) << " ";
        }
      }
      tec_file << endl;
      //..................................................................
    }
  }
  //======================================================================
  // RFI
  else{
    for(j=0l;j<NodeListSize();j++)
	{
      if((node=GetNode(j))!=NULL)
	  {
        x[0] = node->x;
        x[1] = node->y;
        x[2] = node->z;
        tec_file << node->x << " "<< node->y << " "<< node->z << " ";
        for(k=0;k<nName;k++){
          //SB - namepatch        nidx = PCSGetNODValueIndex(pcs_name_vector[k],1);
  		  nidx = PCSGetNODValueIndex(GetPFNamebyCPName(nod_value_vector[k]),1);
		  tec_file << GetNodeVal(j,nidx) << " ";
        }
      }
      tec_file << endl;
    }
  } 
  //----------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
03/2005 OK MultiMSH
08/2005 WW Wite for MultiMSH
12/2005 OK GetMSH
**************************************************************************/
void COutput::WriteTECElementData(fstream& tec_file,int e_type)
{
  long i;
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  //m_msh = FEMGet(pcs_type_name);
  m_msh = GetMSH();
  if(m_msh){
    for(i=0l;i<(long)m_msh->ele_vector.size();i++)
	{
       if(!m_msh->ele_vector[i]->GetMark()) continue;       
       m_msh->ele_vector[i]->WriteIndex_TEC(tec_file);
	 }
  }
  //----------------------------------------------------------------------
  else{
    long *nodes = NULL;
    for(i=0;i<ElementListLength;i++) {
      if (!ElGetElementActiveState(i)) continue; //WW
      nodes = ElGetElementNodes(i);
      if(ElGetElementType(i)==e_type){
        switch(ElGetElementType(i)){
          case 1:
            tec_file \
              << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[1]+1 << " " << nodes[0]+1 << endl;
            break;
          case 2:
            tec_file \
              << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << " " << nodes[3]+1 << endl;
            break;
          case 3:
            tec_file \
              << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << " " << nodes[3]+1 << " " \
              << nodes[4]+1 << " " << nodes[5]+1 << " " << nodes[6]+1 << " " << nodes[7]+1 << endl;
            break;
          case 4:
            tec_file \
              << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << endl;
            break;
          case 5:
            tec_file \
              << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << " " << nodes[3]+1 << endl;
            break;
          case 6:
            tec_file \
              << nodes[0]+1 << " " << nodes[0]+1 << " " << nodes[1]+1 << " " << nodes[2]+1 << " " \
              << nodes[3]+1 << " " << nodes[3]+1 << " " << nodes[4]+1 << " " << nodes[5]+1 << endl;
            break;
        }
      }
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Programing:
08/2004 OK Implementation
08/2004 WW Header by the names gives in .out file
03/2005 OK MultiMSH
04/2005 WW Output active elements only
08/2005 WW Output by MSH
12/2005 OK GetMSH
**************************************************************************/
void COutput::WriteTECHeader(fstream& tec_file,int e_type, string e_type_name)
{
  int k;
  const int nName = (int)nod_value_vector.size(); 
  //--------------------------------------------------------------------
  // MSH
  m_msh = GetMSH();
  //--------------------------------------------------------------------
  long no_elements = 0;
  if(m_msh)
  {
    for (long i = 0; i < (long)m_msh->ele_vector.size(); i++)  
    {
      if (m_msh->ele_vector[i]->GetMark())   
        if(m_msh->ele_vector[i]->GetElementType()==e_type)  no_elements++;
    }
  }
  else
  {
    for (long i = 0; i < ElListSize(); i++)  
    {
      if (ElGetElement(i) != NULL)     // Element existing 
        if (ElGetElementActiveState(i)) // Active
        {
          if(ElGetElementType(i)==e_type) no_elements++;
	    }
    }
  }
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = X,Y,Z";
  for(k=0;k<nName;k++){
    tec_file << "," << nod_value_vector[k] << " ";
  }
  tec_file << endl;
  //--------------------------------------------------------------------
  // Write Header II: zone
  tec_file << "ZONE T=\"";
  tec_file << time << "s\", ";
  if(m_msh){
     tec_file << "N=" << m_msh->GetNodesNumber(false) << ", ";
  }
  else{
    if(NodeListSize()>0)
      tec_file << "N=" << NodeListSize() << ", ";
  }
  tec_file << "E=" << no_elements << ", ";
  tec_file << "F=" << "FEPOINT" << ", ";
  tec_file << "ET=" << e_type_name;
  tec_file << endl;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
01/2006 OK VAR,PCS,MSH concept
**************************************************************************/
void COutput::ELEWriteDOMDataTEC()
{
  //----------------------------------------------------------------------
  if(ele_value_vector.size()==0)
    return;
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name + "_domain" + "_ele";
  if(pcs_type_name.size()>1) // PCS
    tec_file_name += "_" + msh_type_name;
  if(msh_type_name.size()>1) // MSH
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  WriteELEValuesTECHeader(tec_file);
  WriteELEValuesTECData(tec_file);
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
last modification:
**************************************************************************/
void COutput::WriteELEValuesTECHeader(fstream& tec_file)
{
  int i;
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = X,Y,Z,VX,VY,VZ";
  for(i=0;i<(int)ele_value_vector.size();i++){
     tec_file << "," << ele_value_vector[i];
  }
  tec_file << endl;
  //--------------------------------------------------------------------
  // Write Header II: zone
  tec_file << "ZONE T=\"";
  tec_file << time << "s\", ";
  tec_file << "I=" << (long)m_msh->ele_vector.size() << ", ";
  tec_file << "F=" << "POINT" << ", ";
  tec_file << "C=" << "BLACK";
  tec_file << endl;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
11/2005 OK MSH
01/2006 OK 
**************************************************************************/
void COutput::WriteELEValuesTECData(fstream& tec_file)
{
  CRFProcess* m_pcs = NULL;
  if(ele_value_vector.size()>0)
    m_pcs = GetPCS_ELE(ele_value_vector[0]);
  else
    return;
  //--------------------------------------------------------------------
  int no_ele_values = (int)ele_value_vector.size();
  vector<int>ele_value_index_vector(no_ele_values);
  GetELEValuesIndexVector(ele_value_index_vector);
  //--------------------------------------------------------------------
  int j;
  double* xyz;
  CElem* m_ele = NULL;
  FiniteElement::ElementValue* gp_ele = NULL;
  for(long i=0l;i<(long)m_msh->ele_vector.size();i++){
    //....................................................................
    m_ele = m_msh->ele_vector[i];
    xyz = m_ele->GetGravityCenter();
    tec_file << xyz[0] << " " << xyz[1] << " " << xyz[2] << " ";
    //....................................................................
    gp_ele = ele_gp_value[i];
    tec_file << gp_ele->Velocity(0,0) << " ";
    tec_file << gp_ele->Velocity(1,0) << " ";
    tec_file << gp_ele->Velocity(2,0) << " ";
    //....................................................................
    for(j=0;j<(int)ele_value_index_vector.size();j++){
      tec_file << m_pcs->GetElementValue(i,ele_value_index_vector[j]) << " ";
    }
/*
  int j;
  int eidx;
  char ele_value_char[20];
  int no_ele_values = (int)ele_value_vector.size();
    for(j=0;j<no_ele_values;j++){
      sprintf(ele_value_char,"%s",ele_value_vector[j].data());
      eidx = PCSGetELEValueIndex(ele_value_char);
      tec_file << ElGetElementVal(i,eidx) << " ";
    }
*/
    tec_file << endl;
  }
  //--------------------------------------------------------------------
  ele_value_index_vector.clear();
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
08/2004 WW Output by the order of distance to one end of the polyline
        OK this should be done in the PLY method
08/2005 WW Changes due to the Geometry element class applied
           Remove existing files
12/2005 OK Mass transport specifics
12/2005 OK VAR,MSH,PCS concept
**************************************************************************/
void COutput::NODWritePLYDataTEC(int number)
{
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  char number_char[10];
  sprintf(number_char,"%i",number);
  string number_string = number_char;
  string tec_file_name = file_base_name + "_ply_" + geo_name + "_t" + number_string;
  if(pcs_type_name.size()>0)
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>0)
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if(!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  // Tests
  //......................................................................
  // GEO
  CGLPolyline* m_ply = GEOGetPLYByName(geo_name);//CC
  if(!m_ply){
    cout << "Warning in COutput::NODWritePLYDataTEC - no GEO data" << endl;
    tec_file << "Warning in COutput::NODWritePLYDataTEC - no GEO data: " << geo_name << endl;
    tec_file.close();
    return;
  }
  //......................................................................
/*
  CFEMesh* m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::NODWritePLYDataTEC - no MSH data: " << endl;
    tec_file << "Warning in COutput::NODWritePLYDataTEC - no MSH data: " << endl;
    tec_file.close();
    return;
  }
*/
  //----------------------------------------------------------------------
  // NIDX for output variables
  int no_variables = (int)nod_value_vector.size();
  vector<int>NodeIndex(no_variables);
  GetNodeIndexVector(NodeIndex);
  //--------------------------------------------------------------------
  // Write header
  int k;
  string project_title_string = "Profiles along polylines"; //project_title;
  tec_file << "TITLE = \"" << project_title_string << "\"" << endl;
  tec_file << "VARIABLES = DIST ";
  for(k=0;k<no_variables;k++){
    tec_file << nod_value_vector[k] << " ";
  }
  tec_file << endl;
  tec_file << "ZONE T=\"TIME=" << time << "\"" << endl; // , I=" << NodeListLength << ", J=1, K=1, F=POINT" << endl;
  //--------------------------------------------------------------------
  // Write data
  int nidx;
  long j;
  //======================================================================
  // MSH
  CFEMesh* m_msh = GetMSH();
  if(m_msh){
    m_msh->SwitchOnQuadraticNodes(false); //WW
    vector<long>nodes_vector;
    m_msh->GetNODOnPLY(m_ply,nodes_vector);
    //--------------------------------------------------------------------
    CRFProcess* m_pcs = NULL;
    bool bdummy = false;
    for(j=0;j<(long)nodes_vector.size();j++){
      tec_file << m_ply->sbuffer[j] << " ";
      for(k=0;k<no_variables;k++){
        m_pcs = PCSGet(nod_value_vector[k],bdummy);
        if(!m_pcs)
          cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        else
	      tec_file << m_pcs->GetNodeValue(nodes_vector[m_ply->OrderedPoint[j]], NodeIndex[k]) << " ";
      }
      tec_file << endl;
    }
  }
  //======================================================================
  // RFI
  else{
    long *nodes = NULL;
    long no_nodes = 0;
    nodes = MSHGetNodesClose(&no_nodes, m_ply);//CC
    m_ply->GetPointOrderByDistance();
    for(j=0;j<no_nodes;j++){
      tec_file << m_ply->sbuffer[j] << " ";
      for(k=0;k<no_variables;k++){
//SB        nidx = PCSGetNODValueIndex(pcs_name_vector[k],1);
//SB: namepatch
	    nidx = PCSGetNODValueIndex(GetPFNamebyCPName(nod_value_vector[k]),1);
		tec_file << GetNodeVal(nodes[m_ply->OrderedPoint[j]],nidx) << " ";
      }
      tec_file << endl;
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
08/2005 WW MultiMesh
12/2005 OK VAR,MSH,PCS concept
**************************************************************************/
void COutput::NODWritePNTDataTEC(double time_current,int time_step_number)
{
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name + "_time_" + geo_name;
  if(pcs_type_name.size()>0)
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>0)
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if(!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  // Tests
  //......................................................................
  CGLPoint *m_pnt = GEOGetPointByName(geo_name);
  if(!m_pnt)
  {
    cout << "Warning in COutput::NODWritePNTDataTEC - no PNT data: " << geo_name << endl;
    tec_file << "Warning in COutput::NODWritePNTDataTEC - no PNT data: " << geo_name << endl;
    tec_file.close();
    return;
  }
  //......................................................................
  CFEMesh* m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::NODWritePNTDataTEC - no MSH data: " << endl;
    tec_file << "Warning in COutput::NODWritePNTDataTEC - no MSH data: " << endl;
    tec_file.close();
    return;
  }
  //----------------------------------------------------------------------
  // NIDX for output variables
  int k,i;
  CRFProcess* m_pcs = NULL;
  int no_variables = (int)nod_value_vector.size();
  vector<int>NodeIndex(no_variables);
  GetNodeIndexVector(NodeIndex);
  //--------------------------------------------------------------------
  // Write header
  if(time_step_number==1){
    string project_title_string = "Time curves in points"; //project_title;
    tec_file << "TITLE = \"" << project_title_string << "\"" << endl;
    tec_file << "VARIABLES = Time ";
    for(k=0;k<no_variables;k++){
      tec_file << nod_value_vector[k] << " ";
    }
    tec_file << endl;
    tec_file << "ZONE T=\"POINT=" << geo_name << "\"" << endl; //, I=" << anz_zeitschritte << ", J=1, K=1, F=POINT" << endl;
  }
  //--------------------------------------------------------------------
  // Write data
  //......................................................................
  tec_file << time_current << " ";
  //......................................................................
  // NOD values
  int l,m;
  int nidx;
  int timelevel;
  CRFProcess* m_pcs_out = NULL;
  long msh_node_number;
  msh_node_number = m_msh->GetNODOnPNT(m_pnt);      
  //..................................................................
  // Mass transport
  if(pcs_type_name.compare("MASS_TRANSPORT")==0){
    for(i=0;i<(int)nod_value_vector.size();i++){
      nod_value_name = nod_value_vector[i];
      for(l=0;l<(int)pcs_vector.size();l++){
        m_pcs = pcs_vector[l];
        if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0){
          timelevel = 0;
          for(m=0;m<(int)m_pcs->nod_val_name_vector.size();m++){
            if(m_pcs->nod_val_name_vector[m].compare(nod_value_name)==0){
              m_pcs_out = PCSGet(pcs_type_name,nod_value_name);
              if(timelevel==1){
                nidx = m_pcs_out->GetNodeValueIndex(nod_value_name)+timelevel;
                tec_file << m_pcs_out->GetNodeValue(msh_node_number,nidx) << " ";
              }
              timelevel++;
            }
          }
        }
      }
    }
  }
  //..................................................................
  else{
    for(i=0;i<(int)nod_value_vector.size();i++){
      m_pcs = GetPCS(nod_value_vector[i]);
      tec_file << m_pcs->GetNodeValue(msh_node_number,NodeIndex[i]) << " ";
    }
  }
  tec_file << endl;
  //----------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2004 OK Implementation
last modification:
**************************************************************************/
int COutput::GetPointClose(CGLPoint m_point)
{
  int i;
  double x[3];
  double y[3];
  long number=-1;
  double distmin=1.e+300;
  int out_vector_size =(int)out_vector.size();
  double dist;
  x[0] = m_point.x;
  x[1] = m_point.y;
  x[2] = m_point.z;
  for(i=0;i<out_vector_size;i++) {
    if(out_vector[i]->msh_node_number>=0){
      y[0]=GetNodeX(out_vector[i]->msh_node_number);
      y[1]=GetNodeY(out_vector[i]->msh_node_number);
      y[2]=GetNodeZ(out_vector[i]->msh_node_number);
      dist=EuklVek3dDist(x,y);
      if (dist < distmin) {
        distmin=dist;
        number=i;
      }
    }
  }
 return number;
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
07/2004 OK Implementation for OUT points
last modification:
**************************************************************************/
void OUTWriteNODValues(string base_file_name,FEMNodesElements *m_ne)
{
  int i;
  int no_out_points =(int)out_vector.size();
  COutput *m_out = NULL;
  char char_i[2];
  string out_extension;
  string out_file_name;

  for(i=0;i<no_out_points;i++){
    m_out = out_vector[i];
    m_out->m_nodes_elements = m_ne;
    //--------------------------------------------------------------------
    // File handling
    sprintf(char_i,"%i",i);
    out_extension = "N";
    out_extension += char_i;
    out_file_name = base_file_name + "." + out_extension;
    fstream out_file (out_file_name.data(),ios::trunc|ios::out);
    out_file.setf(ios::scientific,ios::floatfield);
    out_file.precision(12);
    if (!out_file.good()) return;
    out_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    // Header
    m_out->WriteTimeCurveHeader(out_file);
    //--------------------------------------------------------------------
    // Data
    m_out->WriteTimeCurveData(out_file);
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
07/2004 OK Implementation for RFO data
07/2004 CC modification
last modification:
**************************************************************************/
void COutput::WriteTimeCurveHeader(fstream& out_file)
{
  out_file << "GeoSys-OUT: Output Time Curve ------------------------------------------------\n";
  out_file << msh_node_number << " " << GetNodeX(msh_node_number) << " " \
                                      << GetNodeY(msh_node_number) << " " \
                                      << GetNodeZ(msh_node_number) << endl;
  //------------------------------------------------------------------------
  out_file << "4" << endl;
  out_file << "TIME, s" << endl;
  int i;
  for(i=0;i<m_nodes_elements->number_of_node_variables;i++){
    out_file << m_nodes_elements->nodevariablenames[i] << endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
07/2004 OK Implementation for RFO data
last modification:
**************************************************************************/
void COutput::WriteTimeCurveData(fstream& out_file)
{
  int i,j;
  for(j=0;j<m_nodes_elements->number_of_times;j++){
    out_file << m_nodes_elements->values_at_times[j] << "  ";
    for(i=0;i<m_nodes_elements->number_of_node_variables;i++){
      out_file << m_nodes_elements->nodevalues[msh_node_number][i][j] << " ";
    }
    out_file << endl;
  }
}


/*
  int no_processes =(int)pcs_vector.size();
  CRFProcess *m_pcs = NULL;
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_nval_data->speichern){
      out_file << m_pcs->pcs_nval_data->name << ", " << m_pcs->pcs_nval_data->einheit << endl;
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
void OUTDelete()
{
  long i;
  int no_out =(int)out_vector.size();
  for(i=0;i<no_out;i++){
    delete out_vector[i];
  }
  out_vector.clear();
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
12/2005 OK MSH
**************************************************************************/
void COutput::WriteRFOHeader(fstream&rfo_file)
{
//#0#0#0#1#0.00000000000000e+000#0#3915###########################################
  rfo_file << "#0#0#0#1#";
  rfo_file << time;
  rfo_file << "#0#";
  rfo_file << ROCKFLOW_VERSION;
  rfo_file << "###########################################";
  rfo_file << endl;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
12/2005 OK MSH
**************************************************************************/
void COutput::WriteRFONodes(fstream&rfo_file)
{
//0 101 100 
  rfo_file << 0 << " " << (long)m_msh->nod_vector.size() << " " << (long)m_msh->ele_vector.size() << endl;
//0 0.00000000000000 0.00000000000000 0.00000000000000 
  CNode* m_nod = NULL;
  for(long i=0;i<(long)m_msh->nod_vector.size();i++){
    m_nod = m_msh->nod_vector[i];
    rfo_file << i << " " << m_nod->X() << " " << m_nod->Y() << " " << m_nod->Z() << " " << endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
12/2005 OK MSH
**************************************************************************/
void COutput::WriteRFOElements(fstream&rfo_file)
{
  int j;
  CElem* m_ele = NULL;
//0 0 -1 line 0 1 
  for(long i=0;i<(long)m_msh->ele_vector.size();i++){
    m_ele = m_msh->ele_vector[i];
    rfo_file << i << " " << m_ele->GetPatchIndex() \
                         << " -1 " \
                         << m_ele->GetName() << " ";
    for(j=0;j<m_ele->GetNodesNumber(false);j++){
      rfo_file << m_ele->nodes_index[j] << " ";
    }
    rfo_file << endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
12/2005 OK MSH
**************************************************************************/
void COutput::WriteRFOValues(fstream&rfo_file)
{
  int p,nidx;
  CRFProcess* m_pcs = NULL;
//1 2 4 
  rfo_file << "1 1 4" << endl;
//2 1 1 
  int no_processes = (int)pcs_vector.size();
  rfo_file << no_processes;
  for(p=0;p<no_processes;p++){
    rfo_file << " 1";
  }
  rfo_file << endl;
//PRESSURE1, Pa
  // Names and units
  for(p=0;p<no_processes;p++){
    m_pcs = pcs_vector[p];
    rfo_file << m_pcs->pcs_primary_function_name[0];
    rfo_file << ", ";
    rfo_file << m_pcs->pcs_primary_function_unit[0];
    rfo_file << endl;
  }
//0 0.00000000000000 0.00000000000000 
  // Node values
  for(long i=0;i<(long)m_msh->nod_vector.size();i++){
    rfo_file << i;
    for(p=0;p<no_processes;p++){
      m_pcs = pcs_vector[p];
      nidx = m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0])+1;
      rfo_file << " " << m_pcs->GetNodeValue(i,nidx);
    }
    rfo_file << " " << endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
12/2005 OK COutput
last modification:
**************************************************************************/
void COutput::WriteRFO()
{
  m_msh = FEMGet(pcs_type_name);
  if(!m_msh){
    cout << "Warning in COutput::WriteRFONodes - no MSH data" << endl;
    return;
  }
  //--------------------------------------------------------------------
  // File handling
  string rfo_file_name;
  rfo_file_name = file_base_name + "." + "rfo";
  fstream rfo_file (rfo_file_name.data(),ios::app|ios::out);
  rfo_file.setf(ios::scientific,ios::floatfield);
  rfo_file.precision(12);
  if (!rfo_file.good()) return;
  rfo_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  WriteRFOHeader(rfo_file);
  WriteRFONodes(rfo_file);
  WriteRFOElements(rfo_file);
  WriteRFOValues(rfo_file);
//  RFOWriteELEValues();
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
05/2004 OK Implementation
**************************************************************************/
void COutput::NODWriteSFCDataTEC(int number)
{
  //--------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet(pcs_type_name);
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(pcs_type_name);
  //--------------------------------------------------------------------
  // File handling
  char number_char[3];
  sprintf(number_char,"%i",number);
  string number_string = number_char;
  string tec_file_name = pcs_type_name + "_sfc_" + geo_name + "_t" + number_string + TEC_FILE_EXTENSION;
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  // Write header
  int k;
  string project_title_string = "Profile at surface"; //project_title;
  tec_file << "TITLE = \"" << project_title_string << "\"" << endl;
  tec_file << "VARIABLES = X,Y,Z,";
  for(k=0;k<(int)nod_value_vector.size();k++){
     tec_file << nod_value_vector[k] << ",";
  }
  tec_file << endl;
  tec_file << "ZONE T=\"TIME=" << time << "\"" << endl; // , I=" << NodeListLength << ", J=1, K=1, F=POINT" << endl;
  //--------------------------------------------------------------------
  // Write data
  int nidx;
  long i;
  vector<long>nodes_vector;
  Surface*m_sfc = NULL;
  m_sfc = GEOGetSFCByName(geo_name);//CC
  if(m_sfc){
    m_msh->GetNODOnSFC(m_sfc,nodes_vector);
	for(i=0;i<(long)m_msh->nod_vector.size();i++){
      tec_file << m_msh->nod_vector[i]->X() << " ";
      tec_file << m_msh->nod_vector[i]->Y() << " ";
      tec_file << m_msh->nod_vector[i]->Z() << " ";
      for(k=0;k<(int)nod_value_vector.size();k++){
        nidx = m_pcs->GetNodeValueIndex(nod_value_vector[k])+1;
        tec_file << m_pcs->GetNodeValue(nodes_vector[i],nidx) << " ";
      }
      tec_file << endl;
    }
  }
  else{
    tec_file << "Error in NODWritePLYDataTEC: polyline " << geo_name << " not found" << endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
last modification:
**************************************************************************/
COutput* OUTGet(string out_name)
{
  COutput *m_out = NULL;
  for(int i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
    if(m_out->pcs_type_name.compare(out_name)==0)
      return m_out;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 
12/2005 OK Implementation
last modification:
**************************************************************************/
void COutput::NODWriteSFCAverageDataTEC(double time_current,int time_step_number)
{
  //--------------------------------------------------------------------
  // Tests
  Surface* m_sfc = NULL;
  m_sfc = GEOGetSFCByName(geo_name);
  if(!m_sfc){
    cout << "Warning in COutput::NODWriteSFCAverageDataTEC - no GEO data" << endl;
    return;
  }
  CFEMesh* m_msh=NULL; 
  m_msh = FEMGet(pcs_type_name);
  if(!m_msh){
    cout << "Warning in COutput::NODWriteSFCAverageDataTEC - no MSH data" << endl;
    return;
  }
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet(pcs_type_name);
  if(!m_pcs){
    cout << "Warning in COutput::NODWriteSFCAverageDataTEC - no PCS data" << endl;
    return;
  }
  //--------------------------------------------------------------------
  // File handling
  string tec_file_name = file_base_name + "_TBC_" + geo_type_name + "_" + geo_name + TEC_FILE_EXTENSION;
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  // Write header
  int i,j;
  if(time_step_number==1){
    string project_title_string = "Time curve at surface"; //project_title;
    tec_file << "TITLE = \"" << project_title_string << "\"" << endl;
    tec_file << "VARIABLES = Time ";
    for(i=0;i<(int)nod_value_vector.size();i++){
      tec_file << nod_value_vector[i] << " ";
    }
    tec_file << endl;
    tec_file << "ZONE T=\"SFC=" << geo_name << "\"" << endl; //, I=" << anz_zeitschritte << ", J=1, K=1, F=POINT" << endl;
  }
  //--------------------------------------------------------------------
  // node_value_index_vector
  vector<int>node_value_index_vector((int)nod_value_vector.size());
  int itemp;
  for(i=0;i<(int)nod_value_vector.size();i++){
    node_value_index_vector[i] = m_pcs->GetNodeValueIndex(nod_value_vector[i]);
    itemp = node_value_index_vector[i];
	for(i=0;i<m_pcs->GetPrimaryVNumber();i++){
      if(nod_value_vector[i].compare(m_pcs->pcs_primary_function_name[i])==0){
        node_value_index_vector[i]++;
        break;
	  }
	}
  }
  //if(node_value_index_vector[i]>-1) //CMCD&WW >-1
  //--------------------------------------------------------------------
  // Write data
  double dtemp;
  vector<long>sfc_nodes_vector;
  m_msh->GetNODOnSFC(m_sfc,sfc_nodes_vector);
  tec_file << time_current << " ";
  for(i=0;i<(int)nod_value_vector.size();i++){
    dtemp = 0.0;
	for(j=0;j<(int)sfc_nodes_vector.size();j++){
      dtemp += m_pcs->GetNodeValue(sfc_nodes_vector[j],node_value_index_vector[i]);
    }
    dtemp /= (double)sfc_nodes_vector.size();
    tec_file << dtemp << " ";
  }
  tec_file << endl;
}

/**************************************************************************
FEMLib-Method: 
Programing:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* COutput::GetMSH()
{
  m_msh = NULL;
  if(pcs_type_name.size()>0)
    m_msh = FEMGet(pcs_type_name);
  else if(msh_type_name.size()>0)
    m_msh = MSHGet(msh_type_name);
  else if(fem_msh_vector.size()==1)
    m_msh = fem_msh_vector[0];
  return m_msh;
}

/**************************************************************************
FEMLib-Method: 
Programing:
12/2005 OK Implementation
**************************************************************************/
void COutput::GetNodeIndexVector(vector<int>&NodeIndex)
{
  int i,k;
  CRFProcess* m_pcs = NULL;
  const int nName = (int)nod_value_vector.size();
  //----------------------------------------------------------------------
  if(pcs_type_name.size()>0)
  {
    m_pcs = PCSGet(pcs_type_name);
    //..................................................................
    for(k=0;k<nName;k++)
    {
      if(pcs_type_name.compare("MASS_TRANSPORT")==0)
      {
        m_pcs = PCSGet("MASS_TRANSPORT",nod_value_vector[k]);
      }
      if(!m_pcs)
      {
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        return;
      }
      NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
      for(i=0;i<m_pcs->GetPrimaryVNumber();i++) 
      {
        if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
        {
          NodeIndex[k]++;
          break;
        }
	  }
    }
  }
  //----------------------------------------------------------------------
  else if(msh_type_name.size()>0)
  {
    m_pcs = PCSGet(msh_type_name);
    if(!m_pcs)
    {
      cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
      return;
    }
    for(k=0;k<nName;k++)
    {
      NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
      for(i=0;i<m_pcs->GetPrimaryVNumber();i++) 
      {
      if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
        {
          NodeIndex[k]++;
          break;
        }
      }
    }
  }
  //----------------------------------------------------------------------
  else if(fem_msh_vector.size()==1)
  {
    bool bdummy = true;
    for(k=0;k<nName;k++)
    {
      m_pcs = PCSGet(nod_value_vector[k],bdummy);
      if(!m_pcs)
      {
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        return;
      }
      NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
      for(i=0;i<m_pcs->GetPrimaryVNumber();i++) 
      {
        if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
        {
          NodeIndex[k]++;
          break;
        }
	  }
    }
  }
  //----------------------------------------------------------------------
}

/*
  for(k=0;k<no_variables;k++){
    m_pcs = PCSGet(nod_value_vector[k],true);
    if(m_pcs){
      NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
      if(NodeIndex[k]<0)
      {
        cout << "Warning in COutput::NODWritePNTDataTEC - no NOD data: " << nod_value_vector[k] << endl;
        tec_file << "Warning in COutput::NODWritePNTDataTEC - no NOD data: " << nod_value_vector[k] << endl;
        tec_file.close();
        return;
      }
      for(i=0;i<m_pcs->GetPrimaryVNumber();i++) 
      {
        if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
        { 
          NodeIndex[k]++;
          break;
        }
      }
    }
  }
*/

/**************************************************************************
PCSLib-Method: 
Programing:
12/2005 OK Implementation
**************************************************************************/
CRFProcess* COutput::GetPCS(string var_name)
{
  CRFProcess* m_pcs = NULL;
  if(pcs_type_name.size()>0)
    m_pcs = PCSGet(pcs_type_name);
  else if(msh_type_name.size()>0)
    m_pcs = PCSGet(msh_type_name);
  if(!m_pcs)
    m_pcs = PCSGet(var_name,true);
  return m_pcs;
}

/**************************************************************************
PCSLib-Method: 
Programing:
12/2005 OK Implementation
**************************************************************************/
CRFProcess* COutput::GetPCS_ELE(string var_name)
{
  int i,j;
  string pcs_var_name;
  CRFProcess* m_pcs = NULL;
  //----------------------------------------------------------------------
  if(pcs_type_name.size()>0)
    m_pcs = PCSGet(pcs_type_name);
  //----------------------------------------------------------------------
  else if(msh_type_name.size()>0)
    m_pcs = PCSGet(msh_type_name);
  //----------------------------------------------------------------------
  if(!m_pcs)
  {
    for(i=0;i<(int)pcs_vector.size();i++)
    {
      m_pcs = pcs_vector[i];
      for(j=0;j<m_pcs->pcs_number_of_evals;j++)
      {
        pcs_var_name = m_pcs->pcs_eval_name[j];
        if(pcs_var_name.compare(var_name)==0)
        {
          return m_pcs;
        }
      }
    }
  }
  //----------------------------------------------------------------------
  return m_pcs;
}

/**************************************************************************
FEMLib-Method: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void COutput::GetELEValuesIndexVector(vector<int>&ele_value_index_vector)
{
  if(ele_value_vector[0].size()==0)
    return;
  CRFProcess* m_pcs = NULL;
  m_pcs = GetPCS_ELE(ele_value_vector[0]);
  for(int i=0;i<(int)ele_value_vector.size();i++)
  {
    ele_value_index_vector[i] = m_pcs->GetElementValueIndex(ele_value_vector[i]);
  }
}
