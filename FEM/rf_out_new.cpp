/**************************************************************************
FEMLib - Object: OUT
Task: 
Programing:
06/2004 OK Implementation
last modified:
**************************************************************************/
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
#include "files0.h"
// GeoSys-GeoLib
#include "files0.h"
#include "geo_ply.h"
#include "geo_sfc.h"
// GeoSys-FEMLib
#include "rf_out_new.h"
#include "rf_pcs.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "mathlib.h"
#include "fem_ele_std.h"
#include "rf_msp_new.h"
// GeoSys-MSHLib
#include "msh_lib.h"

extern int max_dim;  //OK411 todo

#ifdef CHEMAPP
  #include "./EQL/eqlink.h"
#endif
#include "vtk.h"
// MPI Parallel
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
#include "par_ddc.h"
#endif
#ifdef SUPERCOMPUTER
// kg44 this is usefull for io-buffering as endl flushes the buffer 
#define endl '\n'
#define MY_IO_BUFSIZE 4096
#endif
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
  new_file_opened = false; //WW
  m_pcs = NULL;
  vtk = NULL; //NW
}
/**************************************************************************
FEMLib-Method: 
Task: OUT deconstructor
Programing:
04/2004 OK Implementation
**************************************************************************/
COutput::~COutput(void) 
{
	//added by Haibing 08112006--------
	long i;
	for (i=0;i<(long)out_line_vector.size();i++)delete[] out_line_vector[i];
	out_line_vector.clear();
	//---------------------------------
  mmp_value_vector.clear(); //OK
  //mfp_value_vector.clear(); //OK
  if (this->vtk != NULL) delete vtk; //NW
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
08/2008 OK MAT
**************************************************************************/
ios::pos_type COutput::Read(ifstream *out_file)
{
//  char buffer[MAX_ZEILE];
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
  std::stringstream in;
  string name;
  ios::pos_type position_subkeyword;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while(!new_keyword){
    position = out_file->tellg();
    if(new_subkeyword)
      out_file->seekg(position_subkeyword,ios::beg);
    new_subkeyword = false;
    // SB new input		out_file->getline(buffer,MAX_ZEILE);
	// SB new input 	line_string = buffer;
	line_string.clear();
	line_string = GetLineFromFile1(out_file);
	if(line_string.size() < 1) break;
    
    if(Keyword(line_string)) 
      return position;
    //--------------------------------------------------------------------
    if(line_string.find("$NOD_VALUES")!=string::npos) { // subkeyword found
      while ((!new_keyword)&&(!new_subkeyword)) {
        position_subkeyword = out_file->tellg();
        //SB input with comments  *out_file >> line_string>>ws;
		line_string = GetLineFromFile1(out_file);
        if(line_string.find(hash)!=string::npos) {
          return position;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
		if(line_string.size()==0) 
          break; //SB: empty line
		in.str(line_string);
		in >> name;
        nod_value_vector.push_back(name);
		in.clear();
      }
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$PCON_VALUES")!=string::npos) { // subkeyword found //MX
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
          break; 
        pcon_value_vector.push_back(line_string);
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
      /*
      // Commented by WW
      // Remove files
      tec_file_name = file_base_name + "_domain_ele" + TEC_FILE_EXTENSION;
      remove(tec_file_name.c_str());
      */
      continue;
    }
    //-------------------------------------------------------------------- // Added 03.2010 JTARON
    if(line_string.find("$RWPT_VALUES")!=string::npos) { // subkeyword found
      while ((!new_keyword)&&(!new_subkeyword)) {
        position_subkeyword = out_file->tellg();
		line_string = GetLineFromFile1(out_file);
        if(line_string.find(hash)!=string::npos) {
          return position;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
		if(line_string.size()==0)
          break; //SB: empty line
		in.str(line_string);
		in >> name;
        rwpt_value_vector.push_back(name);
		in.clear();
      }
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
		line.clear(); //JT
        /* // Comment by WW
		// Remove files
        tec_file_name = file_base_name + "_domain" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_line" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_quad" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());

#ifdef USE_MPI
	sprintf(t_fname, "%d",myrank);
	tec_file_name = file_base_name + "_domain_tri" + t_fname + TEC_FILE_EXTENSION;
#else
	tec_file_name = file_base_name + "_domain_tri" + TEC_FILE_EXTENSION;
#endif
//orig        tec_file_name = file_base_name + "_domain_tri" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
#ifdef USE_MPI
	sprintf(t_fname, "%d",myrank);
	tec_file_name = file_base_name + "_domain_pris" + t_fname + TEC_FILE_EXTENSION;
#else
	tec_file_name = file_base_name + "_domain_pris" + TEC_FILE_EXTENSION;
#endif
//orig        tec_file_name = file_base_name + "_domain_pris" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
#ifdef USE_MPI
	sprintf(t_fname, "%d",myrank);
	tec_file_name = file_base_name + "_domain_tet" + t_fname + TEC_FILE_EXTENSION;
#else
	tec_file_name = file_base_name + "_domain_tet" + TEC_FILE_EXTENSION;
#endif

//orig        tec_file_name = file_base_name + "_domain_tet" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + "_domain_hex" + TEC_FILE_EXTENSION;
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + ".rfo"; //OK4105
        remove(tec_file_name.c_str());
        tec_file_name = file_base_name + ".vtk"; //kg44
        remove(tec_file_name.c_str());
        */
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
		if(line_string.find("STEPPING")!=string::npos) { // JTARON 2010, reconfigured (and added RWPT)... didn't work
		  double stepping_length, stepping_end, stepping_current;
		  *out_file >> stepping_length >> stepping_end;
		  stepping_current = stepping_length;
		  while(stepping_current <= stepping_end) {
            time_vector.push_back(stepping_current);
			rwpt_time_vector.push_back(stepping_current);
            stepping_current += stepping_length;
		  }
        }
        else{
          time_vector.push_back(strtod(line_string.data(),NULL));
		  rwpt_time_vector.push_back(strtod(line_string.data(),NULL));
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
      /* // Comment by WW
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
      */
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
    if(line_string.find("$MMP_VALUES")!=string::npos) { //OK
      ok = true;
      while(ok)
      {
        position_line = out_file->tellg();
       *out_file >> line_string;
        if(SubKeyword(line_string))
        {
          out_file->seekg(position_line,ios::beg);
          ok = false;
          continue;
        }
        if(Keyword(line_string))
          return position;
        mmp_value_vector.push_back(line_string);
        out_file->ignore(MAX_ZEILE,'\n');
      }
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find("$MFP_VALUES")!=string::npos) { //OK
      ok = true;
      while(ok)
      {
        position_line = out_file->tellg();
       *out_file >> line_string;
        if(SubKeyword(line_string))
        {
          out_file->seekg(position_line,ios::beg);
          ok = false;
          continue;
        }
        if(Keyword(line_string))
          return position;
        mfp_value_vector.push_back(line_string);
        out_file->ignore(MAX_ZEILE,'\n');
      }

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
06/2006 WW Remove the old files by new way
**************************************************************************/
bool OUTRead(string file_base_name)
{
  //----------------------------------------------------------------------
//OK  OUTDelete();  
  //----------------------------------------------------------------------
  COutput *m_out = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  char number_char[3]; //OK4709
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
    // Give version in file name 
    if(line_string.find("#VERSION")!=string::npos) //15.01.2008. WW
    {
       file_base_name.append("(V");
       file_base_name.append(ROCKFLOW_VERSION);
       file_base_name.append(")");
    }
    //----------------------------------------------------------------------
    if(line_string.find("#OUTPUT")!=string::npos) { // keyword found
      m_out = new COutput();
      m_out->file_base_name = file_base_name;
      position = m_out->Read(&out_file);
      out_vector.push_back(m_out);
      sprintf(number_char,"%i",(int)out_vector.size()-1); //OK4709
      number_string = number_char; //OK4709
      m_out->ID = number_string; //OK4709
      out_file.seekg(position,ios::beg);
      /*
      // Commented by WW
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
	  */
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
12/2008 NW DAT_TYPE
05/2009 OK bug fix STEPS
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
  if(tim_type_name=="STEPS")
  {
    *out_file << "  " << tim_type_name << " " << nSteps << endl;
  }
  else
  {
    for(i=0;i<(int)time_vector.size();i++){
      *out_file << "  ";
      *out_file << time_vector[i] << endl;
    }
  }
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
  // DAT_TYPE
  *out_file << " $DAT_TYPE" << endl;
  *out_file << "  ";
  *out_file << dat_type_name << endl;
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
#ifdef SUPERCOMPUTER
// kg44 buffer the output
  char   mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
  out_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
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
03/2006 WW Flag to remove exsiting files
08/2006 OK FLX calculations
08/2007 WW Output initial values of variables
**************************************************************************/
void OUTData(double time_current, const int time_step_number)
{
  int i,j;
  int no_times;
  COutput *m_out = NULL;
  CRFProcess* m_pcs = NULL;
  CFEMesh* m_msh = NULL;
  bool OutputBySteps = false;
  double tim_value;
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
    if(!m_pcs)
      m_pcs = m_out->GetPCS(m_out->pcs_type_name); //OK
    if(!m_pcs)
    {
      cout << "Warning in OUTData - no PCS data" << endl;
      //OK4704 continue;
    }
    //--------------------------------------------------------------------
    m_out->time = time_current;
    no_times = (int)m_out->time_vector.size();
    //--------------------------------------------------------------------
    if(no_times==0&&(m_out->nSteps>0)&&(time_step_number%m_out->nSteps==0))
      OutputBySteps = true;
    if(time_step_number<2) //WW
      OutputBySteps = true;
    //======================================================================
    // TECPLOT
    if(m_out->dat_type_name.compare("TECPLOT")==0 || m_out->dat_type_name.compare("MATLAB")==0)
    {
	  m_out->matlab_delim = " ";
	  if(m_out->dat_type_name.compare("MATLAB")==0) // JTARON, just for commenting header for matlab
		  m_out->matlab_delim = "%";

      switch(m_out->geo_type_name[2]){
        //------------------------------------------------------------------
        case 'M': // domain data
          cout << "Data output: Domain" << endl;
          if(OutputBySteps)
          {
		    if(m_out->pcon_value_vector.size() > 0)  
				m_out->PCONWriteDOMDataTEC();  //MX
			else {
              m_out->NODWriteDOMDataTEC();
              m_out->ELEWriteDOMDataTEC();
			}
            OutputBySteps = false;
            if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
          }
          else 
		  {
            for(j=0;j<no_times;j++){
              if((time_current>m_out->time_vector[j])
                 || fabs(time_current-m_out->time_vector[j])<MKleinsteZahl){ //WW MKleinsteZahl 
			  if(m_out->pcon_value_vector.size() > 0)  
				m_out->PCONWriteDOMDataTEC();  //MX
			  else {
                m_out->NODWriteDOMDataTEC();
                m_out->ELEWriteDOMDataTEC();
			  }
                m_out->time_vector.erase(m_out->time_vector.begin()+j); 
                if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
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
            tim_value = m_out->NODWritePLYDataTEC(time_step_number);
            if(tim_value>0.0) m_out->TIMValue_TEC(tim_value); //OK
            if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
            OutputBySteps = false;
          } 
		  else
		  {
            for(j=0;j<no_times;j++){
              if((time_current>m_out->time_vector[j])
                 || fabs(time_current-m_out->time_vector[j])<MKleinsteZahl){ //WW MKleinsteZahl
                tim_value = m_out->NODWritePLYDataTEC(j+1); //OK
                if(tim_value>0.0) m_out->TIMValue_TEC(tim_value);
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
                if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
                break;
              }
		    }
          }
          //..............................................................
        break;
        //------------------------------------------------------------------
        case 'I': // breakthrough curves in points
          cout << "Data output: Breakthrough curves - " << m_out->geo_name << endl;
          m_out->NODWritePNTDataTEC(time_current,time_step_number);
          if(!m_out->new_file_opened) m_out->new_file_opened=true; //WW
        break;
        //------------------------------------------------------------------
        case 'R': // profiles at surfaces
          cout << "Data output: Surface profile" << endl;
          //..............................................................
          if(m_out->dis_type_name.compare("AVERAGE")==0){
            if(OutputBySteps){
              m_out->NODWriteSFCAverageDataTEC(time_current,time_step_number);
              OutputBySteps = false;
              if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
            } 
		    else{
            }
          }
          //..............................................................
          else{
            if(OutputBySteps){
              m_out->NODWriteSFCDataTEC(time_step_number);
              OutputBySteps = false;
              if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
            } 
		    else{
              for(j=0;j<no_times;j++){
                if((time_current>m_out->time_vector[j]) 
                   || fabs(time_current-m_out->time_vector[j])<MKleinsteZahl){ //WW MKleinsteZahl                m_out->NODWriteSFCDataTEC(j);
                    m_out->NODWriteSFCDataTEC(j);
                    m_out->time_vector.erase(m_out->time_vector.begin()+j);
                    if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
                    break;
                }
              }
            }
          }
          //..............................................................
          // ELE data
          if((int)m_out->ele_value_vector.size()>0)
          {
            m_out->ELEWriteSFC_TEC();
          }
          //..............................................................
        break;
        //------------------------------------------------------------------
        case 'Y': // Layer
          cout << "Data output: Layer" << endl;
          if(OutputBySteps)
          {
            m_out->NODWriteLAYDataTEC(time_step_number);
            OutputBySteps = false;
          }
          else 
		  {
            for(j=0;j<no_times;j++){
              if((time_current>m_out->time_vector[j])
                 || fabs(time_current-m_out->time_vector[j])<MKleinsteZahl){ 
                m_out->NODWriteLAYDataTEC(j);
                m_out->time_vector.erase(m_out->time_vector.begin()+j); 
                break;	  
            }
		  } 
          }

        break;
        //------------------------------------------------------------------
      }
    }
    //--------------------------------------------------------------------
    // vtk 
    else if(m_out->dat_type_name.compare("VTK")==0){
      switch(m_out->geo_type_name[2]){
        case 'M': // domain data
          if(OutputBySteps)
		  {
		    OutputBySteps = false;
            m_out->WriteDataVTK(time_step_number); //OK
            if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
		  }
		  else
		  {
            for(j=0;j<no_times;j++){
              if(time_current>=m_out->time_vector[j]){
                m_out->WriteDataVTK(time_step_number); //OK
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
                if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
		        break;	 
              }
		    } 
          }
          break;
      }
    }
    //--------------------------------------------------------------------
    // PVD (ParaView)
    else if(m_out->dat_type_name.find("PVD")!=string::npos){
      if (m_out->vtk == NULL)
        m_out->vtk = new CVTK();
      CVTK* vtk = m_out->vtk ;

      bool vtk_appended = false;
      if(m_out->dat_type_name.find("PVD_A")!=string::npos){
        vtk_appended = true;
      }

      switch(m_out->geo_type_name[2]){
        case 'M': // domain data
//          static CVTK vtk;
          if (time_step_number == 0) {
         	vtk->InitializePVD(m_out->file_base_name, m_out->pcs_type_name, vtk_appended);
          }
          // Set VTU file name and path
          string vtk_file_name = m_out->file_base_name;
          if(m_out->pcs_type_name.size()>0) // PCS
            vtk_file_name += "_" + m_out->pcs_type_name;
          string pvd_vtk_file_name = vtk->pvd_vtk_file_name_base;
          stringstream stm;
          stm << time_step_number;
          vtk_file_name += stm.str() + ".vtu";
          pvd_vtk_file_name += stm.str() + ".vtu";
          // Output
          if(OutputBySteps)
		  {
		    OutputBySteps = false;
            vtk->WriteXMLUnstructuredGrid(vtk_file_name, m_out, time_step_number);
            VTK_Info dat;
            vtk->vec_dataset.push_back(dat);
            vtk->vec_dataset.back().timestep = m_out->time;
            vtk->vec_dataset.back().vtk_file = pvd_vtk_file_name;
            vtk->UpdatePVD(vtk->pvd_file_name, vtk->vec_dataset);
		  }
		  else
		  {
            for(j=0;j<no_times;j++){
              if(time_current>=m_out->time_vector[j]){
                vtk->WriteXMLUnstructuredGrid(vtk_file_name, m_out, time_step_number);
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
                VTK_Info dat;
                vtk->vec_dataset.push_back(dat);
                vtk->vec_dataset.back().timestep = m_out->time;
                vtk->vec_dataset.back().vtk_file = pvd_vtk_file_name;
                vtk->UpdatePVD(vtk->pvd_file_name, vtk->vec_dataset);
                break;
              }
		    }
          }
          break;

      }
    }
    // ROCKFLOW    
    else if(m_out->dat_type_name.compare("ROCKFLOW")==0){
      switch(m_out->geo_type_name[2]){
        case 'M': // domain data
          if(OutputBySteps)
		  {
		    OutputBySteps = false;
            m_out->WriteRFO(); //OK
            if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
		  }
		  else
		  {
            for(j=0;j<no_times;j++){
              if((time_current>m_out->time_vector[j])
                 || fabs(time_current-m_out->time_vector[j])<MKleinsteZahl){ //WW MKleinsteZahl 
                m_out->WriteRFO(); //OK
                m_out->time_vector.erase(m_out->time_vector.begin()+j);
                if(!m_out->new_file_opened)  m_out->new_file_opened=true; //WW
		        break;	 
              }
		    } 
          }
          break;
      }
    } 
    //--------------------------------------------------------------------
    // ELE values
    m_out->CalcELEFluxes();
  } // OUT loop
  //======================================================================
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
03/2005 OK MultiMSH
08/2005 WW Changes for MultiMSH
12/2005 OK VAR,MSH,PCS concept
07/2007 NW Multi Mesh Type
**************************************************************************/
void COutput::NODWriteDOMDataTEC()
{
  int te=0;
  string eleType;
  string tec_file_name;
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
  char tf_name[10];
  std::cout << "Process " << myrank << " in WriteDOMDataTEC" << "\n";
#endif
  //----------------------------------------------------------------------
  // Tests  
  if(((int)nod_value_vector.size()==0)&&((int)mfp_value_vector.size()==0)) //OK4704
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
  //======================================================================
  vector<int> mesh_type_list; //NW
  if(m_msh->msh_no_line>0)
    mesh_type_list.push_back(1);
	if (m_msh->msh_no_quad>0)
    mesh_type_list.push_back(2);
	if (m_msh->msh_no_hexs>0)
    mesh_type_list.push_back(3);
	if (m_msh->msh_no_tris>0)
    mesh_type_list.push_back(4);
	if (m_msh->msh_no_tets>0)
    mesh_type_list.push_back(5);
	if (m_msh->msh_no_pris>0)
    mesh_type_list.push_back(6);

  // Output files for each mesh type
  for (int i=0; i<(int)mesh_type_list.size(); i++) //NW
  {
    te = mesh_type_list[i];
  //----------------------------------------------------------------------
  // File name handling
  tec_file_name = file_base_name + "_" + "domain";
  if(msh_type_name.size()>0) // MultiMSH
    tec_file_name += "_" + msh_type_name;
  if(pcs_type_name.size()>0) // PCS
    tec_file_name += "_" + pcs_type_name;
  //======================================================================
    switch (te) //NW
    {
    case 1:
      tec_file_name += "_line";
      eleType = "QUADRILATERAL"; 
      break;
    case 2:
      tec_file_name += "_quad";
      eleType = "QUADRILATERAL"; 
      break;
    case 3:
      tec_file_name += "_hex";
      eleType = "BRICK"; 
      break;
    case 4:
      tec_file_name += "_tri";
      eleType = "QUADRILATERAL";
      break;
    case 5:
      tec_file_name += "_tet";
  	  eleType = "TETRAHEDRON"; 
      break;
    case 6:
      tec_file_name += "_pris";
      eleType = "BRICK"; 
      break;
    }
/*
  if(m_msh->msh_no_line>0)
	{
      tec_file_name += "_line";
      eleType = "QUADRILATERAL"; 
	  te=1;
	}
	else if (m_msh->msh_no_quad>0)
	{
      tec_file_name += "_quad";
      eleType = "QUADRILATERAL"; 
	  te=2;
    }
	else if (m_msh->msh_no_hexs>0)
	{
      tec_file_name += "_hex";
      eleType = "BRICK"; 
	  te=3;
    }
	else if (m_msh->msh_no_tris>0)
	{
      tec_file_name += "_tri";
//???Who was this eleType = "TRIANGLE";
      eleType = "QUADRILATERAL";
	  te=4;
    }
	else if (m_msh->msh_no_tets>0)
	{
      tec_file_name += "_tet";
	  eleType = "TETRAHEDRON"; 
      te=5;
    }
	else if (m_msh->msh_no_pris>0)
	{
      tec_file_name += "_pris";
      eleType = "BRICK"; 
	  te=6;
	}
*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
    sprintf(tf_name, "%d", myrank);
    tec_file_name += "_" + string(tf_name);
    std::cout << "Tecplot filename: " << tec_file_name << endl;
#endif
    tec_file_name += TEC_FILE_EXTENSION;
    if(!new_file_opened) remove(tec_file_name.c_str()); //WW
    fstream tec_file (tec_file_name.data(),ios::app|ios::out);
    tec_file.setf(ios::scientific,ios::floatfield);
    tec_file.precision(12);
    if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuf1 [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuf1,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
// 
    WriteTECHeader(tec_file,te,eleType);
    WriteTECNodeData(tec_file);
    WriteTECElementData(tec_file,te);
    tec_file.close(); // kg44 close file 
    //--------------------------------------------------------------------
    // tri elements
    if(msh_no_tris>0){
    //string tec_file_name = pcs_type_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
// buffer the output
      char sxbuf1[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
      fstream tec_file1 (tec_file_name.data(),ios::app|ios::out);
      tec_file1.setf(ios::scientific,ios::floatfield);
      tec_file1.precision(12);
      if (!tec_file1.good()) return;
#ifdef SUPERCOMPUTER
      tec_file1.rdbuf()->pubsetbuf(sxbuf1,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      //OK  tec_file1.clear();
      //OK  tec_file1.seekg(0L,ios::beg);
      WriteTECHeader(tec_file1,4,"TRIANGLE");
      WriteTECNodeData(tec_file1);
      WriteTECElementData(tec_file1,4);
      tec_file1.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // quad elements
    if(msh_no_quad>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
      char sxbuf2[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf2,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      WriteTECHeader(tec_file,2,"QUADRILATERAL");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,2);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // tet elements
    if(msh_no_tets>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_tet" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
      char sxbuf3[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif

      string tec_file_name = file_base_name + "_" + "domain" + "_tet";

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
      sprintf(tf_name, "%d", myrank);
      tec_file_name += "_" + string(tf_name);
#endif

      tec_file_name += TEC_FILE_EXTENSION;

      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf3,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif

      WriteTECHeader(tec_file,5,"TETRAHEDRON");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,5);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // pris elements
    if(msh_no_pris>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
        char sxbuf4[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf4,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif

      WriteTECHeader(tec_file,6,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,6);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // hex elements
    if(msh_no_hexs>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
        char sxbuf5[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif

      string tec_file_name = file_base_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);


      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf5,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      WriteTECHeader(tec_file,3,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,3);
      tec_file.close(); // kg44 close file 
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
void COutput::WriteTECNodeData(fstream &tec_file)
{
  const int nName = (int)nod_value_vector.size();
  long j;
  double x[3];
  double val_n = 0.; //WW
  int i, k;
  int nidx, nidx_dm[3];
  vector<int> NodeIndex(nName);
  string nod_value_name; //OK
  int m;
  int timelevel;
  //----------------------------------------------------------------------
  m_msh = GetMSH();
  //----------------------------------------------------------------------
  //OK4704 CRFProcess* m_pcs = NULL;
  CRFProcess* m_pcs_out = NULL;
  //======================================================================
  // MSH
   //--------------------------------------------------------------------
   for(k=0;k<nName;k++){
     m_pcs = PCSGet(nod_value_vector[k],true);
     if(m_pcs != NULL){
     NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
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
                 if(!m_pcs_out)
                   continue;
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
     else
     {
       for(k=0;k<nName;k++)
       {
         m_pcs = GetPCS(nod_value_vector[k]);
         if(m_pcs != NULL)
         {   //WW
           if(NodeIndex[k]>-1)
           {
              val_n = m_pcs->GetNodeValue( m_msh->nod_vector[j]->GetIndex(),NodeIndex[k]); //WW
              tec_file << val_n << " ";
              if(m_pcs->type==1212&&nod_value_vector[k].find("SATURATION")!=string::npos) //WW
                 tec_file << 1.-val_n << " ";  //WW
           } 
         }
       }
       for(k=0;k<(int)mfp_value_vector.size();k++) //OK4704
       {
	     //tec_file << MFPGetNodeValue(m_msh->nod_vector[j]->GetIndex(),mfp_value_vector[k]) << " "; //NB
         tec_file << MFPGetNodeValue(m_msh->nod_vector[j]->GetIndex(),
			               mfp_value_vector[k],
						   atoi(&mfp_value_vector[k][mfp_value_vector[k].size()-1])-1) << " "; //NB: MFP output for all phases 	 
       }
     }
     tec_file << endl;
     //..................................................................
   }
   //======================================================================
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
07/2007 NW Multi Mesh Type 
**************************************************************************/
void COutput::WriteTECElementData(fstream &tec_file,int e_type)
{
  long i;
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  //m_msh = FEMGet(pcs_type_name);
  m_msh = GetMSH();
    for(i=0l;i<(long)m_msh->ele_vector.size();i++)
	  {
      if(!m_msh->ele_vector[i]->GetMark()) continue;       
      if(m_msh->ele_vector[i]->GetElementType() == e_type) {  //NW
        m_msh->ele_vector[i]->WriteIndex_TEC(tec_file);
      }
	  }
  //----------------------------------------------------------------------
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
void COutput::WriteTECHeader(fstream &tec_file,int e_type, string e_type_name)
{
  int k;
  const int nName = (int)nod_value_vector.size(); 
  //--------------------------------------------------------------------
  // PCON
  const int nPconName = (int)pcon_value_vector.size();   //MX
  //--------------------------------------------------------------------
  // MSH
  m_msh = GetMSH();
  //--------------------------------------------------------------------
  //OK411
  long no_elements = 0;
  for(long i=0;i<(long)m_msh->ele_vector.size();i++)  
  {
    if (m_msh->ele_vector[i]->GetMark())   
    if(m_msh->ele_vector[i]->GetElementType()==e_type) no_elements++;
  }
  //--------------------------------------------------------------------
  // Write Header I: variables
  CRFProcess *m_pcs = NULL;     //WW
  tec_file << "VARIABLES  = \"X\",\"Y\",\"Z\"";
  for(k=0;k<nName;k++){
    tec_file << ", \"" << nod_value_vector[k] << "\"";
    //-------------------------------------WW
    m_pcs = GetPCS(nod_value_vector[k]); 
    if(m_pcs != NULL)
    { 
       if(m_pcs->type==1212&&nod_value_vector[k].find("SATURATION")!=string::npos) 
         tec_file << ", SATURATION2";
    } 
    //-------------------------------------WW
  }
  for(k=0;k<(int)mfp_value_vector.size();k++) 
     tec_file << ", \"" << mfp_value_vector[k] << "\"";  //NB
  if (nPconName) {
	for(k=0;k<nPconName;k++){
	  tec_file << ", " << pcon_value_vector[k] << "";   //MX
	}
  }
  tec_file << endl;
  //--------------------------------------------------------------------
  // Write Header II: zone
  tec_file << "ZONE T=\"";
  tec_file << time << "s\", ";
  //OK411
  tec_file << "N=" << m_msh->GetNodesNumber(false) << ", ";
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
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif

  //--------------------------------------------------------------------
  WriteELEValuesTECHeader(tec_file);
  WriteELEValuesTECData(tec_file);
  //--------------------------------------------------------------------
   tec_file.close(); // kg44 close file 
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
last modification:
**************************************************************************/
void COutput::WriteELEValuesTECHeader(fstream &tec_file)
{
  int i;
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = \"X\",\"Y\",\"Z\",\"VX\",\"VY\",\"VZ\"";
  for(i=0;i<(int)ele_value_vector.size();i++){
	  if(ele_value_vector[i].find("VELOCITY")==string::npos) //WW
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
void COutput::WriteELEValuesTECData(fstream &tec_file)
{
  CRFProcess* m_pcs = NULL;
  if(ele_value_vector.size()>0)
    m_pcs = GetPCS_ELE(ele_value_vector[0]);
  else
    return;
  //--------------------------------------------------------------------
  int j;
  int no_ele_values = (int)ele_value_vector.size();
  bool out_element_vel = false;
  for(j=0; j<no_ele_values; j++) //WW
  {
    if(ele_value_vector[j].find("VELOCITY")!=string::npos)
	{
      out_element_vel = true;
      break;
	}
  }
  vector<int>ele_value_index_vector(no_ele_values);
  GetELEValuesIndexVector(ele_value_index_vector);
  //--------------------------------------------------------------------
  double* xyz;
  CElem* m_ele = NULL;
  FiniteElement::ElementValue* gp_ele = NULL;
  for(long i=0l;i<(long)m_msh->ele_vector.size();i++){
    //....................................................................
    m_ele = m_msh->ele_vector[i];
    xyz = m_ele->GetGravityCenter();
    tec_file << xyz[0] << " " << xyz[1] << " " << xyz[2] << " ";
    if(out_element_vel) //WW
	{
		if(PCSGet("FLUID_MOMENTUM"))	// PCH 16.11 2009
		{
			CRFProcess* pch_pcs = PCSGet("FLUID_MOMENTUM");

			tec_file << pch_pcs->GetElementValue(i, pch_pcs->GetElementValueIndex("VELOCITY1_X")+1) << " ";
			tec_file << pch_pcs->GetElementValue(i, pch_pcs->GetElementValueIndex("VELOCITY1_Y")+1) << " ";
			tec_file << pch_pcs->GetElementValue(i, pch_pcs->GetElementValueIndex("VELOCITY1_Z")+1) << " ";
		}
		else
		{
       //....................................................................
       gp_ele = ele_gp_value[i];
       tec_file << gp_ele->Velocity(0,0) << " ";
       tec_file << gp_ele->Velocity(1,0) << " ";
       tec_file << gp_ele->Velocity(2,0) << " ";
		}
	}
	else
	{
       //....................................................................
       for(j=0;j<(int)ele_value_index_vector.size();j++){
           tec_file << m_pcs->GetElementValue(i,ele_value_index_vector[j]) << " ";
       }
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
12/2005 WW Output stress invariants
08/2006 OK FLUX
10/2008 OK MFP values
**************************************************************************/
double COutput::NODWritePLYDataTEC(int number)
{
  int i,k;
//WW  int nidx;
  long j, gnode;
  bool bdummy = false;
  int ns = 4;
  int stress_i[6], strain_i[6];
  double ss[6];
  double val_n =0.; //WW
  //----------------------------------------------------------------------
  // Tests  
  // OUT
  //OK4704 if((int)nod_value_vector.size()==0)
  if(((int)nod_value_vector.size()==0)&&((int)mfp_value_vector.size()==0)) //OK4704
    return 0.0;
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name + "_ply_" + geo_name + "_t" + ID; //OK4709
  if(pcs_type_name.size()>0)
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>0)
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out); //WW

  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if(!tec_file.good()) return 0.0;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //----------------------------------------------------------------------
  // Tests  
  //......................................................................
  // GEO
  CGLPolyline* m_ply = GEOGetPLYByName(geo_name);//CC
  if(!m_ply)
  {
    cout << "Warning in COutput::NODWritePLYDataTEC - no GEO data" << endl;
    tec_file << "Warning in COutput::NODWritePLYDataTEC - no GEO data: " << geo_name << endl;
    tec_file.close();
    return 0.0;
  }
  //......................................................................
  // MSH
  CFEMesh* m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::NODWritePLYDataTEC - no MSH data" << endl;
    //OKtec_file << "Warning in COutput::NODWritePLYDataTEC - no MSH data: " << geo_name << endl;
    //OKtec_file.close();
    //OKToDo return;
  }
  else
    m_msh->SwitchOnQuadraticNodes(false); //WW
  //......................................................................
  // PCS
  m_pcs = PCSGet(pcs_type_name);
  CRFProcess* dm_pcs = NULL; //WW
  for(i=0;i<(int)pcs_vector.size();i++)
  {
	if(pcs_vector[i]->pcs_type_name.find("DEFORMATION")!=string::npos)
    {
       dm_pcs = pcs_vector[i];
       break;
    }
  }
  //......................................................................
  // VEL
  int v_eidx[3];
  CRFProcess* m_pcs_flow = NULL;
  //m_pcs_flow = PCSGet("GROUNDWATER_FLOW"); //OKToDo
  m_pcs_flow = PCSGetFlow(); //OK
  if(!m_pcs_flow)
  {
    //WW cout << "Warning in COutput::NODWritePLYDataTEC() - no PCS flow data" << endl;
    //tec_file << "Warning in COutput::NODWritePLYDataTEC() - no PCS flow data " << endl;
    //tec_file.close();
    //return 0.0;
  }
  else
  {
    v_eidx[0] = m_pcs_flow->GetElementValueIndex("VELOCITY1_X");
    v_eidx[1] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Y");
    v_eidx[2] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Z");
  }
  for(i=0;i<3;i++)
  {
    if(v_eidx[i]<0)
    {
      //WW cout << "Warning in COutput::NODWritePLYDataTEC() - no PCS flow data" << endl;
      //tec_file << "Warning in COutput::NODWritePLYDataTEC() - no PCS flow data " << endl;
      //tec_file.close();
    }
  }
  //--------------------------------------------------------------------
  // NIDX for output variables
  int no_variables = (int)nod_value_vector.size();
  vector<int>NodeIndex(no_variables);
  GetNodeIndexVector(NodeIndex);
  //--------------------------------------------------------------------
  // Write header
  if(number==0||number==1) //WW if(number==1)
  {
    string project_title_string = "Profiles along polylines"; //project_title;
    tec_file << matlab_delim << "TITLE = \"" << project_title_string << "\"" << endl;
    tec_file << matlab_delim << "VARIABLES = \"DIST\" ";
    for(k=0;k<no_variables;k++)
    {
    tec_file << "\""<< nod_value_vector[k] << "\" ";
      //-------------------------------------WW
      m_pcs = GetPCS(nod_value_vector[k]);  
      if(m_pcs&&m_pcs->type==1212&&nod_value_vector[k].find("SATURATION")!=string::npos) 
         tec_file << "SATURATION2 ";
      //-------------------------------------WW
      if(nod_value_vector[k].compare("FLUX")==0)
        tec_file << "FLUX_INNER" << " ";
    }
    //....................................................................
    for(k=0;k<(int)mfp_value_vector.size();k++) //OK4709
    {
      tec_file << "\""<< mfp_value_vector[k] << "\" ";
    }
    //....................................................................
    // WW: M specific data
    if(dm_pcs)  //WW
    {
      tec_file<< " p_(1st_Invariant) "<<" q_(2nd_Invariant)  "<<" Effective_Strain";
    }
    tec_file << endl;
  }
  //....................................................................
  // WW: M specific data
  if(dm_pcs)  //WW
  {
    stress_i[0] = dm_pcs->GetNodeValueIndex("STRESS_XX");      
    stress_i[1] = dm_pcs->GetNodeValueIndex("STRESS_YY");      
    stress_i[2] = dm_pcs->GetNodeValueIndex("STRESS_ZZ");      
    stress_i[3] = dm_pcs->GetNodeValueIndex("STRESS_XY"); 
    strain_i[0] = dm_pcs->GetNodeValueIndex("STRAIN_XX"); 
    strain_i[1] = dm_pcs->GetNodeValueIndex("STRAIN_YY");
    strain_i[2] = dm_pcs->GetNodeValueIndex("STRAIN_ZZ");
    strain_i[3] = dm_pcs->GetNodeValueIndex("STRAIN_XY");
    if(max_dim==2) // 3D
 	{
      ns = 6;
      stress_i[4] = dm_pcs->GetNodeValueIndex("STRESS_XZ");
      stress_i[5] = dm_pcs->GetNodeValueIndex("STRESS_YZ");        
      strain_i[4] = dm_pcs->GetNodeValueIndex("STRAIN_XZ");
      strain_i[5] = dm_pcs->GetNodeValueIndex("STRAIN_YZ");
    }
  }
  //......................................................................
  tec_file << matlab_delim << "ZONE T=\"TIME=" << time << "\"" << endl; // , I=" << NodeListLength << ", J=1, K=1, F=POINT" << endl;
  //----------------------------------------------------------------------
  // Write data
  //======================================================================
  double flux_sum = 0.0; //OK
  double flux_nod;
  //....................................................................
  m_msh->SwitchOnQuadraticNodes(false); //WW
  // NOD at PLY
  vector<long>nodes_vector;
  m_msh->GetNODOnPLY(m_ply,nodes_vector);
  //....................................................................
  // ELE at PLY
  if((int)ele_value_vector.size()>0)
  {
    vector<long>ele_vector_at_geo;
    m_msh->GetELEOnPLY(m_ply,ele_vector_at_geo);
  }
  //--------------------------------------------------------------------
  //bool b_specified_pcs = (m_pcs != NULL); //NW m_pcs = PCSGet(pcs_type_name);
  for(j=0;j<(long)nodes_vector.size();j++)
  {
    tec_file << m_ply->sbuffer[j] << " ";
    gnode = nodes_vector[m_ply->OrderedPoint[j]]; //WW
    //------------------------------------------------------------------
    for(k=0;k<no_variables;k++)
    {
      //if(!(nod_value_vector[k].compare("FLUX")==0))  // removed JOD, does not work for multiple flow processes
      //if (!b_specified_pcs) //NW
		if(msh_type_name != "COMPARTMENT") // JOD 4.10.01
          m_pcs = PCSGet(nod_value_vector[k],bdummy);
		

      if(!m_pcs)
      {
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        tec_file << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        return 0.0;
      }
      //-----------------------------------------WW
      val_n = m_pcs->GetNodeValue(gnode, NodeIndex[k]);
      tec_file << val_n << " ";
      if(m_pcs->type==1212&&(nod_value_vector[k].find("SATURATION")!=string::npos))
        tec_file << 1.-val_n << " ";
      //-----------------------------------------WW         
      //................................................................
      if(nod_value_vector[k].compare("FLUX")==0)
      {
        if(aktueller_zeitschritt==0) //OK
          flux_nod = 0.0; 
        else
          flux_nod = NODFlux(gnode);
        tec_file << flux_nod << " ";
        //flux_sum += abs(m_pcs->eqs->b[gnode]);
        flux_sum += abs(flux_nod);
        //OK cout << gnode << " " << flux_nod << " " << flux_sum << endl;
      }
      //................................................................
    }
    if(dm_pcs) //WW
    {
      for(i=0;i<ns;i++)
        ss[i] = dm_pcs->GetNodeValue(gnode,stress_i[i]);
      tec_file<<-DeviatoricStress(ss)/3.0<<" ";
      tec_file<<sqrt(3.0*TensorMutiplication2(ss,ss, m_msh->GetCoordinateFlag()/10)/2.0)<<"  ";
      for(i=0;i<ns;i++)
        ss[i] = dm_pcs->GetNodeValue(gnode,strain_i[i]);
      DeviatoricStress(ss);
      tec_file<<sqrt(3.0*TensorMutiplication2(ss,ss, m_msh->GetCoordinateFlag()/10)/2.0);        
    }
    //....................................................................
    // MFP //OK4704
    for(k=0;k<(int)mfp_value_vector.size();k++) //OK4704
    {
 //     tec_file << MFPGetNodeValue(gnode,mfp_value_vector[k],0) << " "; //NB
      tec_file << MFPGetNodeValue(gnode,mfp_value_vector[k],
						   atoi(&mfp_value_vector[k][mfp_value_vector[k].size()-1])-1) << " "; //NB: MFP output for all phases 	 
    }
    //....................................................................
    tec_file << endl;
  }
    tec_file.close(); // kg44 close file 

  //OK cout << "Flux averall: " << flux_sum << endl;
  //======================================================================
  //======================================================================
  return flux_sum;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
08/2005 WW MultiMesh
12/2005 OK VAR,MSH,PCS concept
12/2005 WW Output stress invariants
**************************************************************************/
void COutput::NODWritePNTDataTEC(double time_current,int time_step_number)
{
  int i, k; //WW
  double flux_nod, flux_sum = 0.0;
  //----------------------------------------------------------------------
  CRFProcess* dm_pcs = NULL;
  for(i=0;i<(int)pcs_vector.size();i++){
	if(pcs_vector[i]->pcs_type_name.find("DEFORMATION")!=string::npos){
       dm_pcs = pcs_vector[i];
       break;
    }
  }
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name + "_time_" + geo_name;
  if(pcs_type_name.size()>0)
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>0)
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);

  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if(!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
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
  int no_variables = (int)nod_value_vector.size();
  vector<int>NodeIndex(no_variables);
  GetNodeIndexVector(NodeIndex);
  //--------------------------------------------------------------------
  // Write header
  if(time_step_number==0){ //WW  Old: if(time_step_number==1)
    string project_title_string = "Time curves in points"; //project_title;
    tec_file << matlab_delim << "TITLE = \"" << project_title_string << "\"" << endl;
    tec_file << matlab_delim << "VARIABLES = \"TIME \" ";
    for(k=0;k<no_variables;k++)
    {//WW 
       tec_file << " \"" << nod_value_vector[k] << "\" ";
       //-------------------------------------WW
       m_pcs = GetPCS(nod_value_vector[k]);  
       if(m_pcs&&m_pcs->type==1212&&nod_value_vector[k].find("SATURATION")!=string::npos) 
         tec_file << "SATURATION2 ";
      //-------------------------------------WW
    }
	for(k=0;k<(int)mfp_value_vector.size();k++) //OK411
        tec_file << " \"" << mfp_value_vector[k] << "\" "; //NB MFP data names for multiple phases
    //
    #ifdef RFW_FRACTURE
    for(i=0; i<(int)mmp_vector.size(); ++i)
    {
      if( mmp_vector[i]->frac_num >0)
      {
        for(int j=0; j<mmp_vector[i]->frac_num; ++j)
        {
          tec_file << mmp_vector[i]->frac_names[j]<<"_k "<< mmp_vector[i]->frac_names[j] <<"_aper " 
                    <<mmp_vector[i]->frac_names[j] <<"_closed ";
        }
      }
    }
    #endif
    
    if(dm_pcs) //WW
       tec_file<< " p_(1st_Invariant) "<<" q_(2nd_Invariant)  "<<" Effective_Strain";
    tec_file << endl;
    tec_file << matlab_delim << "ZONE T=\"POINT=" << geo_name << "\"" << endl; //, I=" << anz_zeitschritte << ", J=1, K=1, F=POINT" << endl;



  }
  // For deformation
  int ns = 4;
  int stress_i[6], strain_i[6];
  double ss[6];
  if(dm_pcs) //WW
  {
     stress_i[0] = dm_pcs->GetNodeValueIndex("STRESS_XX");      
     stress_i[1] = dm_pcs->GetNodeValueIndex("STRESS_YY");      
     stress_i[2] = dm_pcs->GetNodeValueIndex("STRESS_ZZ");      
     stress_i[3] = dm_pcs->GetNodeValueIndex("STRESS_XY"); 
     strain_i[0] = dm_pcs->GetNodeValueIndex("STRAIN_XX"); 
     strain_i[1] = dm_pcs->GetNodeValueIndex("STRAIN_YY");
     strain_i[2] = dm_pcs->GetNodeValueIndex("STRAIN_ZZ");
     strain_i[3] = dm_pcs->GetNodeValueIndex("STRAIN_XY");
	 if(m_msh->GetCoordinateFlag()/10==3) // 3D
	 {
       ns = 6;
       stress_i[4] = dm_pcs->GetNodeValueIndex("STRESS_XZ");
       stress_i[5] = dm_pcs->GetNodeValueIndex("STRESS_YZ");        
       strain_i[4] = dm_pcs->GetNodeValueIndex("STRAIN_XZ");
       strain_i[5] = dm_pcs->GetNodeValueIndex("STRAIN_YZ");
	 }
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
  else
  {
    for(i=0;i<(int)nod_value_vector.size();i++)
    {
      //..................................................................
      // PCS
      if(!(nod_value_vector[i].compare("FLUX")==0)   ||  pcs_type_name == "OVERLAND_FLOW")  //JOD separate infiltration flux output in overland flow 
      {
        m_pcs = GetPCS(nod_value_vector[i]);
      }
      else
      {
        m_pcs = GetPCS(pcs_type_name);
      }
      if(!m_pcs)
      {
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        tec_file << "Warning in COutput::NODWritePLYDataTEC - no PCS data" << endl;
        return;
      }
      //..................................................................
      // PCS
      if(!(nod_value_vector[i].compare("FLUX")==0)  || pcs_type_name == "OVERLAND_FLOW") // JOD separate infiltration flux output in overland flow 
      {
        //-----------------------------------------WW
        double val_n =  m_pcs->GetNodeValue(msh_node_number,NodeIndex[i]);
        tec_file << val_n << " ";
        m_pcs = GetPCS(nod_value_vector[i]);
        if(m_pcs->type==1212&&(nod_value_vector[i].find("SATURATION")!=string::npos))
        tec_file << 1.-val_n << " ";
        //-----------------------------------------WW         
      }
      else
      {
        flux_nod = NODFlux(msh_node_number);
        tec_file << flux_nod << " ";
        //flux_sum += abs(m_pcs->eqs->b[gnode]);
        flux_sum += abs(flux_nod);
        //OK cout << gnode << " " << flux_nod << " " << flux_sum << endl;
      }
    }
    //....................................................................    
    #ifdef RFW_FRACTURE
    for(i=0; i<(int)mmp_vector.size(); ++i)
    {
      if( mmp_vector[i]->frac_num >0)
      {
        for(int j=0; j<mmp_vector[i]->frac_num; ++j)
        {
          tec_file << mmp_vector[i]->frac_perm[j]<<" "<< mmp_vector[i]->avg_aperture[j] <<" " 
            <<mmp_vector[i]->closed_fraction[j] <<" ";
        }
      }
    }
    #endif
    //....................................................................
    if(dm_pcs) //WW
    {
         for(i=0;i<ns;i++)
           ss[i] = dm_pcs->GetNodeValue(msh_node_number,stress_i[i]);
         tec_file<<-DeviatoricStress(ss)/3.0<<" ";
         tec_file<<sqrt(3.0*TensorMutiplication2(ss,ss, m_msh->GetCoordinateFlag()/10)/2.0)<<"  ";
         for(i=0;i<ns;i++)
           ss[i] = dm_pcs->GetNodeValue(msh_node_number,strain_i[i]);
         DeviatoricStress(ss);
         tec_file<<sqrt(3.0*TensorMutiplication2(ss,ss, m_msh->GetCoordinateFlag()/10)/2.0);        
    }
	for (k=0;k<(int)mfp_value_vector.size();k++) //OK411
		tec_file << MFPGetNodeValue(msh_node_number,mfp_value_vector[k],
		                  atoi(&mfp_value_vector[k][mfp_value_vector[k].size()-1])-1) << " "; //NB
  }
  tec_file << endl;
  //----------------------------------------------------------------------
   tec_file.close(); // kg44 close file 
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
  for(i=0;i<out_vector_size;i++) 
  {
    if(out_vector[i]->msh_node_number>=0)
    {
      //OK411 y[0]=GetNodeX(out_vector[i]->msh_node_number);
      //OK411 y[1]=GetNodeY(out_vector[i]->msh_node_number);
      //OK411 y[2]=GetNodeZ(out_vector[i]->msh_node_number);
      dist=EuklVek3dDist(x,y);
      if (dist < distmin) 
      {
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
void COutput::WriteRFOHeader(fstream &rfo_file)
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
void COutput::WriteRFONodes(fstream &rfo_file)
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
void COutput::WriteRFOElements(fstream &rfo_file)
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
void COutput::WriteRFOValues(fstream &rfo_file)
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
  if(!new_file_opened) remove(rfo_file_name.c_str()); //WW
  fstream rfo_file (rfo_file_name.data(),ios::app|ios::out);

  rfo_file.setf(ios::scientific,ios::floatfield);
  rfo_file.precision(12);
  if (!rfo_file.good()) return;
  rfo_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    rfo_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  WriteRFOHeader(rfo_file);
  WriteRFONodes(rfo_file);
  WriteRFOElements(rfo_file);
  WriteRFOValues(rfo_file);
//  RFOWriteELEValues();
  rfo_file.close(); // kg44 close file 

}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
05/2004 OK Implementation
**************************************************************************/
void COutput::NODWriteSFCDataTEC(int number)
{
  if((int)nod_value_vector.size()==0)
    return;
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
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  // Write header
  int k;
  string project_title_string = "Profile at surface"; //project_title;
  tec_file << matlab_delim << "TITLE = \"" << project_title_string << "\"" << endl;
  tec_file << matlab_delim << "VARIABLES = \"X\",\"Y\",\"Z\",";
  for(k=0;k<(int)nod_value_vector.size();k++){
     tec_file << nod_value_vector[k] << ",";
  }
  tec_file << endl;
  tec_file << matlab_delim << "ZONE T=\"TIME=" << time << "\"" << endl; // , I=" << NodeListLength << ", J=1, K=1, F=POINT" << endl;
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
      tec_file.close(); // kg44 close file 
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
<<<<<<< HEAD
last modification:
=======
last modification: 03/2010 JTARON
>>>>>>> FETCH_HEAD
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
Task: Return m_out for variable name (based on OUTGet)
Programing:
03/2010 JTARON Implementation
last modification: 
**************************************************************************/
COutput* OUTGetRWPT(string out_name)
{
  COutput *m_out = NULL;
  string vecname;
  for(int i=0;i<(int)out_vector.size();i++)
  {
    m_out = out_vector[i];
	for(int j=0;j<(int)m_out->rwpt_value_vector.size();j++)
	{
		vecname = m_out->rwpt_value_vector[j];
		if(vecname.compare(out_name)==0)
			return m_out;
	}
  }
  return NULL;
}
/**************************************************************************
FEMLib-Method:
12/2005 OK Implementation
04/2006 CMCD no mesh option & flux weighting
last modification:
**************************************************************************/
void COutput::NODWriteSFCAverageDataTEC(double time_current,int time_step_number)
{
  bool no_pcs = false;
  double dtemp;
  vector<long>sfc_nodes_vector;
  double node_flux = 0.0;
  int idx = -1;
  double t_flux = 0.0;
  double node_conc = 0.0;
  CRFProcess* m_pcs_gw = NULL;
  m_pcs_gw = PCSGet("GROUNDWATER_FLOW");
  if (!m_pcs_gw)PCSGet("LIQUID_FLOW"); 
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
    no_pcs = true;
    //cout << "Warning in COutput::NODWriteSFCAverageDataTEC - no PCS data" << endl;
    //return;
  }
  //--------------------------------------------------------------------
  // File handling
  string tec_file_name = file_base_name + "_TBC_" + geo_type_name + "_" + geo_name + TEC_FILE_EXTENSION;
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
  tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  // Write header
  int i,j;
  if(time_step_number==0){ // WW Old:  if(time_step_number==1)
    string project_title_string = "Time curve at surface"; //project_title;
    tec_file << matlab_delim << "TITLE = \"" << project_title_string << "\"" << endl;
    tec_file << matlab_delim << "VARIABLES = Time ";
    for(i=0;i<(int)nod_value_vector.size();i++){
      tec_file << nod_value_vector[i] << " ";
    }
    tec_file << endl;
    tec_file << matlab_delim << "ZONE T=\"SFC=" << geo_name << "\"" << endl; //, I=" << anz_zeitschritte << ", J=1, K=1, F=POINT" << endl;
  }
  //--------------------------------------------------------------------
  // node_value_index_vector
  vector<int>node_value_index_vector((int)nod_value_vector.size());
  int itemp;
  if (m_pcs){
    for(i=0;i<(int)nod_value_vector.size();i++){
      node_value_index_vector[i] = m_pcs->GetNodeValueIndex(nod_value_vector[i])+1;
      itemp = node_value_index_vector[i];
	    for(i=0;i<m_pcs->GetPrimaryVNumber();i++){
        if(nod_value_vector[i].compare(m_pcs->pcs_primary_function_name[i])==0){
          node_value_index_vector[i]++;
          break;
	      }
	    }
    }
  }
  //--------------------------------------------------------------------
  // Write data
  if(no_pcs){
    tec_file << time_current << " ";
    for(i=0;i<(int)nod_value_vector.size();i++){
      m_pcs = PCSGet("MASS_TRANSPORT",nod_value_vector[i]);//Specified currently for MASS_TRANSPORT only.
      node_value_index_vector[i] = m_pcs->GetNodeValueIndex(nod_value_vector[i])+1;
      m_pcs->m_msh->GetNODOnSFC(m_sfc,sfc_nodes_vector);
      dtemp = 0.0;
      t_flux = 0.0;
	    for(j=0;j<(int)sfc_nodes_vector.size();j++){
        idx = m_pcs_gw->GetNodeValueIndex("FLUX");
        node_flux = abs(m_pcs_gw->GetNodeValue(sfc_nodes_vector[j],idx));
        node_conc = m_pcs->GetNodeValue(sfc_nodes_vector[j],node_value_index_vector[i]);
        dtemp += (node_flux * node_conc);
        t_flux += node_flux;
      }
      dtemp /= t_flux;
      tec_file << dtemp << " ";
    }
    tec_file << endl;
  }
  else{
    m_msh->GetNODOnSFC(m_sfc,sfc_nodes_vector);
    idx = m_pcs_gw->GetNodeValueIndex("FLUX");
    tec_file << time_current << " ";
    dtemp = 0.0;
    t_flux = 0.0;
    for(i=0;i<(int)nod_value_vector.size();i++){
      dtemp = 0.0;
	    for(j=0;j<(int)sfc_nodes_vector.size();j++){
        node_flux = abs(m_pcs_gw->GetNodeValue(sfc_nodes_vector[j],idx));
        node_conc = m_pcs->GetNodeValue(sfc_nodes_vector[j],node_value_index_vector[i]);
        dtemp += (node_flux * node_conc);
        t_flux += node_flux;
      }
      dtemp /= t_flux;
      tec_file << dtemp << " ";
    }
    tec_file << endl;
  }
      tec_file.close(); // kg44 close file 
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
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data: " << nod_value_vector[k] << endl;
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
        if(nod_value_vector[k].compare("COUPLING")==0)  // JOD
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
        cout << "Warning in COutput::NODWritePLYDataTEC - no PCS data: " << nod_value_vector[k] << endl;
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
CRFProcess* COutput::GetPCS(const string &var_name)
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
CRFProcess* COutput::GetPCS_ELE(const string &var_name)
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

/**************************************************************************
FEMLib-Method: 
Programing:
01/2006 OK Implementation
**************************************************************************/
void COutput::SetNODFluxAtPLY()
{
  //----------------------------------------------------------------------
  // Tests
  CGLPolyline* m_ply = GEOGetPLYByName(geo_name);
//  m_polyline->GetPointOrderByDistance(); 
  if(!m_ply)
  {
    cout << "Warning in COutput::SetNODFluxAtPLY() - no PLY data" << endl;
    return;
  }
  CFEMesh* m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::SetNODFluxAtPLY() - no MSH data" << endl;
    return;
  }
  CRFProcess* m_pcs = GetPCS("FLUX");
  if(!m_pcs)
  {
    cout << "Warning in COutput::SetNODFluxAtPLY() - no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  vector<long>nodes_vector;
  m_msh->GetNODOnPLY(m_ply,nodes_vector);
  vector<double> node_value_vector;
  node_value_vector.resize(nodes_vector.size());
  int nidx = m_pcs->GetNodeValueIndex("FLUX");
  for(int i=0;i<(int)nodes_vector.size();i++)
  {
    node_value_vector[i] = m_pcs->GetNodeValue(nodes_vector[i],nidx); 
  }
  //----------------------------------------------------------------------
//m_st->FaceIntegration(m_pcs,nodes_vector,node_value_vector);
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
04/2006 KG44 Implementation
09/2006 KG44 Output for MPI - correct OUTPUT not yet implemented
12/2008 NW Remove ios::app, Add PCS name to VTK file name
**************************************************************************/
void COutput::WriteDataVTK(int number)
{
  char number_char[10];
  sprintf(number_char,"%i",number);
  string number_string = number_char;

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
  char tf_name[10];
  std::cout << "Process " << myrank << " in WriteDataVTK" << "\n";
#endif

  m_msh = FEMGet(pcs_type_name);
  if(!m_msh){
    cout << "Warning in COutput::WriteVTKNodes - no MSH data" << endl;
    return;
  }
  //--------------------------------------------------------------------
  // File handling
  string vtk_file_name;
  vtk_file_name = file_base_name;
  if(pcs_type_name.size()>0) // PCS
    vtk_file_name += "_" + pcs_type_name;
  vtk_file_name += number_string ;

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
	// kg44 removed "_0" as this make it impossible for visit/paraview to identify the cycle number
   // sprintf(tf_name, "%d", myrank);
   // vtk_file_name += "_" + string(tf_name);
    // std::cout << "VTK filename: " << vtk_file_name << endl;
#endif
  vtk_file_name += ".vtk";
  if(!new_file_opened) remove(vtk_file_name.c_str()); //KG44
  fstream vtk_file (vtk_file_name.data(),ios::out); //NW remove ios::app
  vtk_file.setf(ios::scientific,ios::floatfield);
  vtk_file.precision(12);
  if (!vtk_file.good()) return;
  vtk_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
     char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
     vtk_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  WriteVTKHeader(vtk_file);
  WriteVTKNodeData(vtk_file);
  WriteVTKElementData(vtk_file);
  WriteVTKValues(vtk_file);
  //======================================================================
  // vtk
  vtk_file.close(); // kg44 close file 
}

/**************************************************************************
FEMLib-Method: 
Programing:
04/2006 KG44 Implementation
**************************************************************************/
void COutput::WriteVTKHeader(fstream &vtk_file)
{

  // Write Header
  vtk_file << "# vtk DataFile Version 3.6.2" << endl;
  vtk_file << "Unstructured Grid: OpenGeoSys->Paraview. Current time (s) = " << aktuelle_zeit << endl;
// data type here always ASCII
  vtk_file << "ASCII"  << endl;
  vtk_file << endl;

// geometry/topoology
  vtk_file << "DATASET UNSTRUCTURED_GRID"  << endl;
}
/**************************************************************************
FEMLib-Method: 
Programing:
04/2006 KG44 Implementation
**************************************************************************/
void COutput::WriteVTKNodeData(fstream &vtk_file)
{
// header for node data 
   CFEMesh* m_msh = GetMSH(); //WW
   vtk_file << "POINTS "<< m_msh->GetNodesNumber(false) << " float" << endl;


  CNode* m_nod = NULL;
  for(long i=0;i<m_msh->GetNodesNumber(false) ;i++){
    m_nod = m_msh->nod_vector[i];
    vtk_file << " " << m_nod->X() << " " << m_nod->Y() << " " << m_nod->Z() << " " << endl;
  }
  vtk_file << endl; 

}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
04/2006 KG44 Implementation
**************************************************************************/
void COutput::WriteVTKElementData(fstream &vtk_file)
{

  int j;
  long no_all_elements =0;
  CElem* m_ele = NULL;
  CFEMesh* m_msh = GetMSH(); //WW

// count overall length of element vector 
  for(long i=0;i<(long)m_msh->ele_vector.size();i++){
    m_ele = m_msh->ele_vector[i];
    no_all_elements=no_all_elements+(m_ele->GetNodesNumber(false))+1;
    }

// write element header
  vtk_file << "CELLS " << (long)m_msh->ele_vector.size() << " " << no_all_elements << endl; 

// write elements
  for(long i=0;i<(long)m_msh->ele_vector.size();i++){
    m_ele = m_msh->ele_vector[i];

        switch(m_ele->GetElementType()){
	    case 1:  // vtk_line=3
            vtk_file \
              << "2  " ;
            break;
	    case 2:  // quadrilateral=9
            vtk_file \
              << "4  ";
            break;
	    case 3: // hexahedron=12
            vtk_file \
              << "8  ";
            break;
	    case 4:  // triangle=5
            vtk_file \
              << "3  ";
            break;
	    case 5:  // tetrahedron=10
            vtk_file \
              << "4  ";
            break;
	    case 6:   // wedge=13
            vtk_file \
              << "6  ";
            break;
        }

    for(j=0;j<m_ele->GetNodesNumber(false);j++){
      vtk_file << m_ele->nodes_index[j] << " ";
    }
    vtk_file << endl;
  }

  vtk_file << endl; 

// write cell types

// write cell_types header
  vtk_file << "CELL_TYPES " << (int) m_msh->ele_vector.size() << endl; 

  for(long i=0;i<(long)m_msh->ele_vector.size();i++){
    m_ele = m_msh->ele_vector[i];

        switch(m_ele->GetElementType()){
	    case 1:  // vtk_line=3
            vtk_file \
              << "3  "<< endl ;
            break;
	    case 2:  // quadrilateral=9
            vtk_file \
              << "9  "<< endl;
            break;
	    case 3: // hexahedron=12
            vtk_file \
              << "12  "<< endl;
            break;
	    case 4:  // triangle=5
            vtk_file \
              << "5  "<< endl;
            break;
	    case 5:  // tetrahedron=10
            vtk_file \
              << "10  "<< endl;
            break;
	    case 6:   // wedge=13
            vtk_file \
              << "13  "<< endl;
            break;
        }
  }
  vtk_file << endl; 

 
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
04/2006 kg44 Implementation
10/2006 WW Output secondary variables
08/2008 OK MAT values
06/2009 WW/OK WriteELEVelocity for different coordinate systems
**************************************************************************/
void COutput::WriteVTKValues(fstream &vtk_file)
{
  CRFProcess* m_pcs = NULL;
  int num_nod_values = (int)nod_value_vector.size();
  int num_ele_values = (int)ele_value_vector.size();
  vector<int> nod_value_index_vector(num_nod_values);
  vector<int> ele_value_index_vector(num_ele_values);
  long j;
  int i, k;
  double val_n = 0.;
  double *tensor = NULL; // JTARON 2010, added for permeability output
  //======================================================================
  // NODAL DATA
  // ---------------------------------------------------------------------
  vtk_file << "POINT_DATA " <<m_msh->GetNodesNumber(false) << endl;
  //WW
  for(k=0;k<num_nod_values;k++)
  {
    m_pcs = PCSGet(nod_value_vector[k],true);
    if(!m_pcs) continue;
    nod_value_index_vector[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
    //   if(nod_value_vector[k].find("SATURATION")!=string::npos)
    //   NodeIndex[k]++;
    for(i=0; i<m_pcs->GetPrimaryVNumber(); i++)
    {
      if(nod_value_vector[k].compare(m_pcs->pcs_primary_function_name[i])==0)
      {
         nod_value_index_vector[k]++;
         break;
       }
    }
    vtk_file << "SCALARS " << nod_value_vector[k] << " float 1" << endl;
    vtk_file << "LOOKUP_TABLE default" <<endl;
    //....................................................................
    for(j=0l;j<m_msh->GetNodesNumber(false);j++)
    {
       if(nod_value_index_vector[k]>-1)
          vtk_file <<" "<< m_pcs->GetNodeValue( m_msh->nod_vector[j]->GetIndex(),nod_value_index_vector[k]) << endl;
    }
  }
  //======================================================================
  // Saturation 2 for 1212 pp - scheme. 01.04.2009. WW
// ---------------------------------------------------------------------
  if(num_nod_values > 0) //SB added
	m_pcs = PCSGet(nod_value_vector[0],true);
  if(m_pcs&&m_pcs->type==1212)
  {
    i = m_pcs->GetNodeValueIndex("SATURATION1");
    vtk_file << "SCALARS SATURATION2 float 1" << endl;
    //
    vtk_file << "LOOKUP_TABLE default" <<endl;
    //....................................................................
    for(j=0l;j<m_msh->GetNodesNumber(false);j++)
    {
       val_n = m_pcs->GetNodeValue( m_msh->nod_vector[j]->GetIndex(),i); //WW
       vtk_file <<" "<< 1.-val_n  << endl;
    }
  }
  // ELEMENT DATA
  // ---------------------------------------------------------------------
  bool wroteAnyEleData = false; //NW
  if(num_ele_values>0)
  {
    m_pcs = GetPCS_ELE(ele_value_vector[0]);
	GetELEValuesIndexVector(ele_value_index_vector);
    vtk_file << "CELL_DATA " << (long)m_msh->ele_vector.size() << endl;
    wroteAnyEleData = true;
    //....................................................................
//
    for(k=0;k<num_ele_values;k++)
    {
//    JTARON 2010, "VELOCITY" should only write as vector, scalars handled elswhere
      if(ele_value_vector[k].compare("VELOCITY")==0)
      {
		  vtk_file << "VECTORS velocity float " << endl;
		  WriteELEVelocity(vtk_file); //WW/OK
	  }
//	  PRINT CHANGING (OR CONSTANT) PERMEABILITY TENSOR?   // JTARON 2010
      else if(ele_value_vector[k].compare("PERMEABILITY")==0)
      {
		  vtk_file << "VECTORS permeability float " << endl;
		  CMediumProperties* MediaProp = NULL;
		  CElem* m_ele = NULL;
		  for(j=0l;j<(long)m_msh->ele_vector.size();j++)
		  {
			  m_ele = m_msh->ele_vector[j];
			  MediaProp = mmp_vector[m_ele->GetPatchIndex()];
			  tensor = MediaProp->PermeabilityTensor(j);
			  for(i=0;i<3;i++)
				  vtk_file << tensor[i*3+i] << " ";
			  vtk_file << endl;
		  }
	  }
	  else if(ele_value_index_vector[k]>-1)
	  {
//	  NOW REMAINING SCALAR DATA  // JTARON 2010, reconfig
		  vtk_file << "SCALARS " << ele_value_vector[k] << " float 1" << endl;
		  vtk_file << "LOOKUP_TABLE default" <<endl;
		  for(long i=0l;i<(long)m_msh->ele_vector.size();i++)
			  vtk_file << m_pcs->GetElementValue(i,ele_value_index_vector[k]) << endl;
	  }
    }
    //--------------------------------------------------------------------
    ele_value_index_vector.clear();
  }
  //======================================================================
  // MAT data
  double mat_value = 0.0; //OK411
  CMediumProperties* m_mmp = NULL;
  CElem* m_ele = NULL;
  int mmp_id = -1;
  if(mmp_value_vector.size()>0)
  {
    // Identify MAT value
    if(mmp_value_vector[0].compare("POROSITY")==0)
      mmp_id = 0;
      // Let's say porosity
    // write header for cell data
    if (!wroteAnyEleData)
      vtk_file << "CELL_DATA " << (long)m_msh->ele_vector.size() << endl;
    wroteAnyEleData = true;
    for(long i=0;i<(long)m_msh->ele_vector.size();i++)
    {
      m_ele = m_msh->ele_vector[i];
      m_mmp = mmp_vector[m_ele->GetPatchIndex()];
      switch(mmp_id)
      {
        case 0: mat_value = m_mmp->Porosity(i,0.0); break;
        default: cout << "COutput::WriteVTKValues: no MMP values specified" << endl;
      }
      vtk_file << mat_value << endl;
    }
  }
  // PCH: Material groups from .msh just for temparary purpose
  if(mmp_vector.size() > 1)
  {
    // write header for cell data
    if (!wroteAnyEleData) //NW: check whether the header has been already written 
      vtk_file << "CELL_DATA " << (long)m_msh->ele_vector.size() << endl;
    wroteAnyEleData = true;
	// header now scalar data
	vtk_file << "SCALARS " << "MatGroup" << " int 1" << endl;
	vtk_file << "LOOKUP_TABLE default" <<endl;
	for(long i=0;i<(long)m_msh->ele_vector.size();i++)
	{
      m_ele = m_msh->ele_vector[i];
	  vtk_file << m_ele->GetPatchIndex() << endl;
	}
  }
}

/**************************************************************************
FEMLib-Method: 
06/2006 OK Implementation
**************************************************************************/
void COutput::ELEWriteSFC_TEC()
{
  //----------------------------------------------------------------------
  if(ele_value_vector.size()==0)
    return;
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name + "_surface" + "_ele";
  if(pcs_type_name.size()>1) // PCS
    tec_file_name += "_" + msh_type_name;
  if(msh_type_name.size()>1) // MSH
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  if(!new_file_opened) remove(tec_file_name.c_str()); //WW
  //......................................................................
  fstream tec_file (tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
  tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  vector<long>tmp_ele_sfc_vector;
  tmp_ele_sfc_vector.clear();
  //--------------------------------------------------------------------
  ELEWriteSFC_TECHeader(tec_file);
  ELEWriteSFC_TECData(tec_file);
  //--------------------------------------------------------------------
      tec_file.close(); // kg44 close file 
}

/**************************************************************************
FEMLib-Method: 
06/2006 OK Implementation
**************************************************************************/
void COutput::ELEWriteSFC_TECHeader(fstream &tec_file)
{
  int i;
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = \"X\",\"Y\",\"Z\"";
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
06/2006 OK Implementation
**************************************************************************/
void COutput::ELEWriteSFC_TECData(fstream &tec_file)
{
tec_file << "COutput::ELEWriteSFC_TECData - implementation not finished" << endl;
  long i;
  int j;
  CElem* m_ele = NULL;
  CElem* m_ele_neighbor = NULL;
  double v[3];
  CRFProcess* m_pcs = NULL;
  double v_n;
  //--------------------------------------------------------------------
  m_pcs = pcs_vector[0]; //GetPCS_ELE(ele_value_vector[0]);
  int nidx[3];
  nidx[0] = m_pcs->GetElementValueIndex("VELOCITY1_X");
  nidx[1] = m_pcs->GetElementValueIndex("VELOCITY1_Y");
  nidx[2] = m_pcs->GetElementValueIndex("VELOCITY1_Z");
  //--------------------------------------------------------------------
  for(i=0l;i<(long)m_msh->ele_vector.size();i++)
  {
    m_ele = m_msh->ele_vector[i];
    for(j=0;j<m_ele->GetFacesNumber();j++)
    {
      m_ele_neighbor = m_ele->GetNeighbor(j);
      if((m_ele->GetDimension() - m_ele_neighbor->GetDimension())==1)
      {
        v[0] = m_pcs->GetElementValue(m_ele->GetIndex(),nidx[0]);
        v[1] = m_pcs->GetElementValue(m_ele->GetIndex(),nidx[1]);
        v[2] = m_pcs->GetElementValue(m_ele->GetIndex(),nidx[2]);
        m_ele_neighbor->SetNormalVector();
        v_n = v[0]*m_ele_neighbor->normal_vector[0] \
            + v[1]*m_ele_neighbor->normal_vector[1] \
            + v[2]*m_ele_neighbor->normal_vector[2];
      }
    }
  }
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
08/2006 OK Implementation
**************************************************************************/
void COutput::CalcELEFluxes()
{
  CGLPoint* m_pnt = NULL;
  CGLPolyline* m_ply = NULL;
  Surface* m_sfc = NULL;
  CGLVolume* m_vol = NULL;
  double f_n_sum = 0.0;
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = PCSGet(pcs_type_name);
  if(!m_pcs) // WW moved it here.
  {
    //WW cout << "Warning in COutput::CalcELEFluxes(): no PCS data" << endl;
    return;
  }
  if( pcs_type_name.find("DEFORMATION")!=string::npos
   ||pcs_type_name.find("FLOW")==string::npos
   ||m_pcs->m_msh->geo_name.find("REGIONAL")!=string::npos) //WW
    return;
  //----------------------------------------------------------------------
  switch(geo_type_name[3])
  {
    case 'N': //poiNt
      m_pnt = GEOGetPointByName(geo_name);
      //m_pcs->CalcELEFluxes(m_pnt);
      break;
    case 'Y': //polYline
      m_ply = GEOGetPLYByName(geo_name);
      if(!m_ply)
        cout << "Warning in COutput::CalcELEFluxes - no GEO data" << endl;
      f_n_sum = m_pcs->CalcELEFluxes(m_ply);
      ELEWritePLY_TEC();
//BUGFIX_4402_OK_1
      TIMValue_TEC(f_n_sum);
      break;
    case 'F': //surFace
      m_sfc = GEOGetSFCByName(geo_name);
      //m_pcs->CalcELEFluxes(m_sfc);
      break;
    case 'U': //volUme
      m_vol = GEOGetVOL(geo_name);
      //m_pcs->CalcELEFluxes(m_vol);
      break;
    case 'A': //domAin
      //m_pcs->CalcELEFluxes(m_dom);
      break;
    default:
      cout << "Warning in COutput::CalcELEFluxes(): no GEO type data" << endl;
  }
  if(new_file_opened)  new_file_opened=true; //WW
  // WW   m_pcs->CalcELEFluxes(m_ply) changed 'mark' of elements
  for(long i=0;i<(long)fem_msh_vector.size();i++)
  {
    for(long j=0;j<(long)fem_msh_vector[i]->ele_vector.size();j++)
     fem_msh_vector[i]->ele_vector[j]->MarkingAll(true);
  }


}

/**************************************************************************
FEMLib-Method: 
08/2006 OK Implementation
**************************************************************************/
void COutput::ELEWritePLY_TEC()
{
  //----------------------------------------------------------------------
  if(ele_value_vector.size()==0)
    return;
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  string tec_file_name = file_base_name; // + "_ply" + "_ele";
  tec_file_name += "_" + geo_type_name;
  tec_file_name += "_" + geo_name;
  tec_file_name += "_ELE";
  if(pcs_type_name.size()>1) // PCS
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>1) // MSH
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  if(!new_file_opened) 
    remove(tec_file_name.c_str()); //WW
  //......................................................................
  fstream tec_file(tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
  tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  vector<long>tmp_ele_ply_vector;
  tmp_ele_ply_vector.clear();
  //--------------------------------------------------------------------
  ELEWritePLY_TECHeader(tec_file);
  ELEWritePLY_TECData(tec_file);
  //--------------------------------------------------------------------

      tec_file.close(); // kg44 close file 
}

/**************************************************************************
FEMLib-Method: 
06/2006 OK Implementation
**************************************************************************/
void COutput::ELEWritePLY_TECHeader(fstream &tec_file)
{
  int i;
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = \"X\",\"Y\",\"Z\"";
  for(i=0;i<(int)ele_value_vector.size();i++){
    tec_file << "," << ele_value_vector[i];
  }
  tec_file << endl;
  //--------------------------------------------------------------------
  // Write Header II: zone
  tec_file << "ZONE T=\"";
  tec_file << time << "s\", ";
  tec_file << endl;
}

/**************************************************************************
FEMLib-Method: 
06/2006 OK Implementation
**************************************************************************/
void COutput::ELEWritePLY_TECData(fstream &tec_file)
{
  long i;
  int j;
  CElem* m_ele = NULL;
  CEdge* m_edg = NULL;
  vec<CEdge*>ele_edges_vector(15);
  vec<CNode*>edge_nodes(3);    
  double edge_mid_vector[3];
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = PCSGet(pcs_type_name);
  int f_eidx[3];
  f_eidx[0] = m_pcs->GetElementValueIndex("FLUX_X");
  f_eidx[1] = m_pcs->GetElementValueIndex("FLUX_Y");
  f_eidx[2] = m_pcs->GetElementValueIndex("FLUX_Z");
  for(i=0;i<3;i++)
  {
    if(f_eidx[i]<0)
    {
      cout << "Fatal error in CRFProcess::CalcELEFluxes(CGLPolyline*m_ply) - abort"; abort();
    }
  }
  int v_eidx[3];
  CRFProcess* m_pcs_flow = NULL;
  if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
  {
    m_pcs_flow = m_pcs;
  }
  else
  {
    m_pcs_flow = PCSGet("GROUNDWATER_FLOW");
  }
  v_eidx[0] = m_pcs_flow->GetElementValueIndex("VELOCITY1_X");
  v_eidx[1] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Y");
  v_eidx[2] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Z");
  for(i=0;i<3;i++)
  {
    if(v_eidx[i]<0)
    {
      cout << "Fatal error in CRFProcess::CalcELEFluxes(CGLPolyline*m_ply) - abort"; abort();
    }
  }
  //----------------------------------------------------------------------
  CGLPolyline* m_ply = GEOGetPLYByName(geo_name);
  //----------------------------------------------------------------------
  // Get elements at GEO
  vector<long>ele_vector_at_geo;
  m_msh->GetELEOnPLY(m_ply,ele_vector_at_geo);
  //--------------------------------------------------------------------
  for(i=0;i<(long)ele_vector_at_geo.size();i++)
  {
    m_ele = m_msh->ele_vector[ele_vector_at_geo[i]];
    // x,y,z
    m_ele->GetEdges(ele_edges_vector);
    for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
    {
      m_edg = ele_edges_vector[j];
      if(m_edg->GetMark())
      {
        m_edg->GetNodes(edge_nodes);
        edge_mid_vector[0] = 0.5*(edge_nodes[1]->X() + edge_nodes[0]->X());
        edge_mid_vector[1] = 0.5*(edge_nodes[1]->Y() + edge_nodes[0]->Y());
        edge_mid_vector[2] = 0.5*(edge_nodes[1]->Z() + edge_nodes[0]->Z());
      }
    }
    tec_file << edge_mid_vector[0] << " " << edge_mid_vector[1] << " " << edge_mid_vector[2];
    // ele vector values
    tec_file << " " << m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[0]);
    tec_file << " " << m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[1]);
    tec_file << " " << m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[2]);
    tec_file << " " << m_pcs->GetElementValue(m_ele->GetIndex(),f_eidx[0]);
    tec_file << " " << m_pcs->GetElementValue(m_ele->GetIndex(),f_eidx[1]);
    tec_file << " " << m_pcs->GetElementValue(m_ele->GetIndex(),f_eidx[2]);
    tec_file << endl;
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
08/2006 OK Implementation
**************************************************************************/
void COutput::TIMValue_TEC(double tim_value)
{
  //----------------------------------------------------------------------
  // File handling
  //......................................................................
  fstream tec_file;
  string tec_file_name = file_base_name; // + "_ply" + "_ele";
  tec_file_name += "_" + geo_type_name;
  tec_file_name += "_" + geo_name;
  tec_file_name += "_TIM";
  if(pcs_type_name.size()>1) // PCS
    tec_file_name += "_" + pcs_type_name;
  if(msh_type_name.size()>1) // MSH
    tec_file_name += "_" + msh_type_name;
  tec_file_name += TEC_FILE_EXTENSION;
  if(!new_file_opened) 
    remove(tec_file_name.c_str()); //WW
  //......................................................................
  tec_file.open(tec_file_name.data(),ios::app|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
  tec_file.seekg(0L,ios::beg);
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif 
  //--------------------------------------------------------------------
  // Write Header I: variables
  if(aktueller_zeitschritt==1)
  {
  tec_file << "VARIABLES = \"Time\",\"Value\"";
  tec_file << endl;
  //--------------------------------------------------------------------
  // Write Header II: zone
  tec_file << "ZONE T=";
  tec_file << geo_name;
  tec_file << endl;
  }
  //--------------------------------------------------------------------
  tec_file << aktuelle_zeit << " " << tim_value << endl;
  //--------------------------------------------------------------------
      tec_file.close(); // kg44 close file 

}

/**************************************************************************
FEMLib-Method: 
08/2006 OK Implementation
**************************************************************************/
double COutput::NODFlux(long nod_number)
{
  nod_number = nod_number; //OK411
/*
  cout << gnode << " " \
       << m_pcs->GetNodeValue(gnode,NodeIndex[k]) << end
  flux_sum += m_pcs->GetNodeValue(gnode,NodeIndex[k]);
*/
  // All elements at node //OK
  CNode* m_nod = m_msh->nod_vector[nod_number];
#ifdef NEW_EQS //WW. 07.11.2008
  return 0.; //To do: m_pcs->eqs_new->b[nod_number];   
#else
  // Element nodal RHS contributions
  m_pcs->eqs->b[nod_number] = 0.0;
  m_pcs->AssembleParabolicEquationRHSVector(m_nod);
  return m_pcs->eqs->b[nod_number];   
#endif
}

/**************************************************************************
FEMLib-Method: 
04/2006 OK Implementation
08/2006 YD
**************************************************************************/
void COutput::NODWriteLAYDataTEC(int time_step_number)
{
  int k;
  string tec_file_name;
  //----------------------------------------------------------------------
  // Tests  
  const int nName = (int)nod_value_vector.size();
  if(nName==0)
    return;
  vector<int>NodeIndex(nName);
  //......................................................................
  // PCS
  CRFProcess* m_pcs = PCSGet(pcs_type_name);
  if(!m_pcs)
    return;
  for(k=0;k<nName;k++){
    NodeIndex[k] = m_pcs->GetNodeValueIndex(nod_value_vector[k]);
  }
  //......................................................................
  // MSH
  m_msh = GetMSH();
  if(!m_msh)
  {
    cout << "Warning in COutput::NODWriteLAYDataTEC() - no MSH data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  // File name handling
  char char_time_step_number[10];
  sprintf(char_time_step_number,"%i",time_step_number);
  tec_file_name = file_base_name + "_" + "layer_";
  tec_file_name += char_time_step_number;
  tec_file_name += TEC_FILE_EXTENSION;
  fstream tec_file (tec_file_name.data(),ios::trunc|ios::out);

  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
// 
// kg44 buffer the output
    char mybuffer [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
  tec_file.rdbuf()->pubsetbuf(mybuffer,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
// 
#endif
  //--------------------------------------------------------------------
  // Write Header I: variables
  tec_file << "VARIABLES = X,Y,Z,N";
  for(k=0;k<nName;k++){
    tec_file << "," << nod_value_vector[k] << " ";
  }
  tec_file << endl;
  //======================================================================
  long j;
  long no_per_layer = m_msh->GetNodesNumber(false)/(m_msh->no_msh_layer+1);
  long jl;
  for(int l=0;l<m_msh->no_msh_layer+1;l++)
  {
    //--------------------------------------------------------------------
    tec_file << "ZONE T=LAYER" << l << endl;
    //--------------------------------------------------------------------
    for(j=0l;j<no_per_layer;j++)
    {
      jl = j + j*m_msh->no_msh_layer + l;
      //..................................................................
      // XYZ
      tec_file << m_msh->nod_vector[jl]->X() << " ";
      tec_file << m_msh->nod_vector[jl]->Y() << " ";
      tec_file << m_msh->nod_vector[jl]->Z() << " ";
      tec_file << jl << " ";
      //..................................................................
      for(k=0;k<nName;k++)
      {
        tec_file << m_pcs->GetNodeValue(m_msh->nod_vector[jl]->GetIndex(),NodeIndex[k]) << " ";
      }
      tec_file << endl;
    }
  }
      tec_file.close(); // kg44 close file 

}

/**************************************************************************
FEMLib-Method: 
Task: Write PCON data for ChemApp output
Programing:
08/2008 MX Implementation
**************************************************************************/
void COutput::PCONWriteDOMDataTEC()
{
  int te=0;
  string eleType;
  string tec_file_name;
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
  char tf_name[10];
  std::cout << "Process " << myrank << " in WriteDOMDataTEC" << "\n";
#endif


  //----------------------------------------------------------------------
  // Tests  
  if((int)pcon_value_vector.size()==0)
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
  //======================================================================
  vector<int> mesh_type_list; //NW
  if(m_msh->msh_no_line>0)
    mesh_type_list.push_back(1);
	if (m_msh->msh_no_quad>0)
    mesh_type_list.push_back(2);
	if (m_msh->msh_no_hexs>0)
    mesh_type_list.push_back(3);
	if (m_msh->msh_no_tris>0)
    mesh_type_list.push_back(4);
	if (m_msh->msh_no_tets>0)
    mesh_type_list.push_back(5);
	if (m_msh->msh_no_pris>0)
    mesh_type_list.push_back(6);

  // Output files for each mesh type
  for (int i=0; i<(int)mesh_type_list.size(); i++) //NW
  {
    te = mesh_type_list[i];
  //----------------------------------------------------------------------
  // File name handling
  tec_file_name = file_base_name + "_" + "domain_PCON";
  if(msh_type_name.size()>0) // MultiMSH
    tec_file_name += "_" + msh_type_name;
  if(pcs_type_name.size()>0) // PCS
    tec_file_name += "_" + pcs_type_name;
  //======================================================================
    switch (te) //NW
    {
    case 1:
      tec_file_name += "_line";
      eleType = "QUADRILATERAL"; 
      break;
    case 2:
      tec_file_name += "_quad";
      eleType = "QUADRILATERAL"; 
      break;
    case 3:
      tec_file_name += "_hex";
      eleType = "BRICK"; 
      break;
    case 4:
      tec_file_name += "_tri";
      eleType = "QUADRILATERAL";
      break;
    case 5:
      tec_file_name += "_tet";
  	  eleType = "TETRAHEDRON"; 
      break;
    case 6:
      tec_file_name += "_pris";
      eleType = "BRICK"; 
      break;
    }
/*
  if(m_msh->msh_no_line>0)
	{
      tec_file_name += "_line";
      eleType = "QUADRILATERAL"; 
	  te=1;
	}
	else if (m_msh->msh_no_quad>0)
	{
      tec_file_name += "_quad";
      eleType = "QUADRILATERAL"; 
	  te=2;
    }
	else if (m_msh->msh_no_hexs>0)
	{
      tec_file_name += "_hex";
      eleType = "BRICK"; 
	  te=3;
    }
	else if (m_msh->msh_no_tris>0)
	{
      tec_file_name += "_tri";
//???Who was this eleType = "TRIANGLE";
      eleType = "QUADRILATERAL";
	  te=4;
    }
	else if (m_msh->msh_no_tets>0)
	{
      tec_file_name += "_tet";
	  eleType = "TETRAHEDRON"; 
      te=5;
    }
	else if (m_msh->msh_no_pris>0)
	{
      tec_file_name += "_pris";
      eleType = "BRICK"; 
	  te=6;
	}
*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
    sprintf(tf_name, "%d", myrank);
    tec_file_name += "_" + string(tf_name);
    std::cout << "Tecplot filename: " << tec_file_name << endl;
#endif
    tec_file_name += TEC_FILE_EXTENSION;
    if(!new_file_opened) remove(tec_file_name.c_str()); //WW
    fstream tec_file (tec_file_name.data(),ios::app|ios::out);
    tec_file.setf(ios::scientific,ios::floatfield);
    tec_file.precision(12);
    if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
// kg44 buffer the output
    char mybuf1 [MY_IO_BUFSIZE*MY_IO_BUFSIZE];
    tec_file.rdbuf()->pubsetbuf(mybuf1,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
// 
    WriteTECHeader(tec_file,te,eleType);
    WriteTECNodePCONData(tec_file);
    WriteTECElementData(tec_file,te);
    tec_file.close(); // kg44 close file 
    //--------------------------------------------------------------------
    // tri elements
    if(msh_no_tris>0){
    //string tec_file_name = pcs_type_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
// buffer the output
      char sxbuf1[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_tri" + TEC_FILE_EXTENSION;
      fstream tec_file1 (tec_file_name.data(),ios::app|ios::out);
      tec_file1.setf(ios::scientific,ios::floatfield);
      tec_file1.precision(12);
      if (!tec_file1.good()) return;
#ifdef SUPERCOMPUTER
      tec_file1.rdbuf()->pubsetbuf(sxbuf1,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      //OK  tec_file1.clear();
      //OK  tec_file1.seekg(0L,ios::beg);
      WriteTECHeader(tec_file1,4,"TRIANGLE");
      WriteTECNodeData(tec_file1);
      WriteTECElementData(tec_file1,4);
      tec_file1.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // quad elements
    if(msh_no_quad>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
      char sxbuf2[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_quad" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf2,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      WriteTECHeader(tec_file,2,"QUADRILATERAL");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,2);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // tet elements
    if(msh_no_tets>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_tet" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
      char sxbuf3[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif

      string tec_file_name = file_base_name + "_" + "domain" + "_tet";

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
      sprintf(tf_name, "%d", myrank);
      tec_file_name += "_" + string(tf_name);
#endif

      tec_file_name += TEC_FILE_EXTENSION;

      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf3,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif

      WriteTECHeader(tec_file,5,"TETRAHEDRON");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,5);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // pris elements
    if(msh_no_pris>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
        char sxbuf4[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif
      string tec_file_name = file_base_name + "_" + "domain" + "_pris" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);

      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf4,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif

      WriteTECHeader(tec_file,6,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,6);
      tec_file.close(); // kg44 close file 
    }
    //--------------------------------------------------------------------
    // hex elements
    if(msh_no_hexs>0){
      //string tec_file_name = pcs_type_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
#ifdef SUPERCOMPUTER
        char sxbuf5[MY_IO_BUFSIZE*MY_IO_BUFSIZE];
#endif

      string tec_file_name = file_base_name + "_" + "domain" + "_hex" + TEC_FILE_EXTENSION;
      fstream tec_file (tec_file_name.data(),ios::app|ios::out);


      tec_file.setf(ios::scientific,ios::floatfield);
      tec_file.precision(12);
      if (!tec_file.good()) return;
#ifdef SUPERCOMPUTER
      tec_file.rdbuf()->pubsetbuf(sxbuf5,MY_IO_BUFSIZE*MY_IO_BUFSIZE);
#endif
      WriteTECHeader(tec_file,3,"BRICK");
      WriteTECNodeData(tec_file);
      WriteTECElementData(tec_file,3);
      tec_file.close(); // kg44 close file 
    }
  }

}


/**************************************************************************
FEMLib-Method: 
Task: Node value output of PCON in aquous
Programing:
08/2008 MX Implementation
**************************************************************************/
void COutput::WriteTECNodePCONData(fstream &tec_file)
{
  const int nName = (int)pcon_value_vector.size();
  long j;
  double x[3];
  int i, k;
  int  nidx_dm[3];
  vector<int> PconIndex(nName);
  //----------------------------------------------------------------------
  m_msh = GetMSH();
  //======================================================================
   #ifdef CHEMAPP
	CEqlink *eq=NULL;
	
	eq = eq->GetREACTION();
	if (!eq) 
		return;
	const int nPCON_aq = eq->NPCON[1];  //GetNPCON(1);
	eq->GetPconNameAq();

    for(i=0;i<nName;i++){
	   for(k=0;k<nPCON_aq;k++){
//		 pcon_value_name = PconName_Aq[i];
		 if(pcon_value_vector[i].compare(PconName_Aq[k])==0)
         { 
		    PconIndex[i] = k;
			break;
		 }
	   }
    }
   #endif
  // MSH
   //--------------------------------------------------------------------
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
     for(k=0;k<nName;k++){
      #ifdef CHEMAPP
	   tec_file << eq->GetPconAq_mol_amount(j,PconIndex[k]) << " ";
      #endif
     }
     tec_file << endl;
     //..................................................................
   }
   //======================================================================
  //----------------------------------------------------------------------
}

/***************************************************************************************
	Function checking the consistency of the output data as specified in the input file
	This means up to now, that data for missing processes is not written
	05/09	SB
*****************************************************************************************/
void OUTCheck(void){

  int i,j, l, m;
  COutput *m_out = NULL;
  CRFProcess* m_pcs = NULL;
  CRFProcess* m_pcs_out;
  m_pcs_out = NULL;
  CFEMesh* m_msh = NULL;
  bool found = false;
  vector <string> del_index;
  cout << "Checking output data " << endl;

  // Go through all out objects (#OUTPUT-section in input file)
  for(i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
	del_index.clear();
    // test MSH
    m_msh = m_out->GetMSH();
    if(!m_msh){
      // cout << "Warning in OUTData - no MSH data" << endl;
      //OK continue;
    }

    // for all 
	if(m_out->nod_value_vector.size()>0){
		for(j=0;j<(int)m_out->nod_value_vector.size();j++){
		  found = false; // initialize variable found
		  for(l=0;l<(int)pcs_vector.size();l++){
		    m_pcs = pcs_vector[l];
				for(m=0;m<(int)m_pcs->nod_val_name_vector.size();m++){
					if(m_pcs->nod_val_name_vector[m].compare(m_out->nod_value_vector[j])==0){
						found=true;
						del_index.push_back(m_out->nod_value_vector[j]);
						break;
					}
				} // end for(m...)
		  } // end for(l...)
		  if(!found){
			cout << "Warning - no PCS data for output variable " << m_out->nod_value_vector[j] << " in ";
			cout << m_out->geo_type_name << " " << m_out->geo_name << endl;
		  }
		} // end for(j...)

	// Reduce vector m_out->nod_value_vector by elements which have no PCS
		if(del_index.size() < m_out->nod_value_vector.size()){
			cout << " Reducing output to variables with existing PCS-data " << endl;
		m_out->nod_value_vector.clear();
		for(j=0;j<(int)del_index.size();j++){ //OK411
//			cout << del_index[j] << endl;
			m_out->nod_value_vector.push_back(del_index[j]);
		}
		}
		if(!m_pcs)
			m_pcs = m_out->GetPCS(m_out->pcs_type_name); //OK 
		if(!m_pcs){
			cout << "Warning in OUTData - no PCS data" << endl;
			//OK4704 continue;
		}
	} // end if(m_out->nod_value_vector.size()>0)
  } // end for(i...)
}

/**************************************************************************
FEMLib-Method: 
06/2009 WW/OK Implementation
**************************************************************************/
inline void COutput::WriteELEVelocity(iostream &vtk_file)
{
    int k; 
    int vel_ind[3];
    FiniteElement::ElementValue* gp_ele = NULL;

    vel_ind[0] = 0; 
    vel_ind[1] = 1; 
    vel_ind[2] = 2; 
    // 1D 
    if(m_msh->GetCoordinateFlag()/10==1)
    {
      // 0 y 0
      if(m_msh->GetCoordinateFlag()%10==1)
      {
        vel_ind[0] = 1; 
        vel_ind[1] = 0;
      }
      // 0 0 z
      else if(m_msh->GetCoordinateFlag()%10==2)
      {
        vel_ind[0] = 2; 
        vel_ind[2] = 0; 
      }
    }
    // 2D 
    if(m_msh->GetCoordinateFlag()/10==2)
    {
      // 0 y z
      if(m_msh->GetCoordinateFlag()%10==1)
      {
        vel_ind[0] = 1; 
        vel_ind[1] = 2; 
      }
      // x 0 z
      else if(m_msh->GetCoordinateFlag()%10==2)
      {
        vel_ind[0] = 0; 
        vel_ind[1] = 2; 
        vel_ind[2] = 1; 
      }
    }
   
    for(long i=0;i<(long)m_msh->ele_vector.size();i++)
    {
      gp_ele = ele_gp_value[i];
      for(k=0; k<3; k++)      
         vtk_file << gp_ele->Velocity(vel_ind[k],0) << " ";
      vtk_file << endl;
    }
}
