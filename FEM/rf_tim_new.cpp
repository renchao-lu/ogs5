/**************************************************************************
FEMLib - Object: TIM
Task: 
Programing:
08/2004 OK Implementation
last modified:
**************************************************************************/
#include "stdafx.h" /* MFC */
// C++ STL
#include <math.h>
#include <iostream>
using namespace std;
#include "rfstring.h"
// FEM-Makros
#include "makros.h"
// GeoSys-GeoLib
#include "geo_strings.h"
// GeoSys-FEMLib
#include "rf_tim_new.h"
#include "rf_pcs.h"
#include "rf_mmp_new.h"
#include "fem_ele_std.h"
#include "mathlib.h"
#include "elements.h" //set functions for stability criteria
// ToDo
double aktuelle_zeit;
long aktueller_zeitschritt = 0;
double dt = 0.0;
//==========================================================================
vector<CTimeDiscretization*>time_vector;
/**************************************************************************
FEMLib-Method: 
Task: OBJ constructor
Programing:
08/2004 OK Implementation
**************************************************************************/
CTimeDiscretization::CTimeDiscretization(void)
                    :Write_tim_discrete(false),tim_discrete(NULL) //YD
{
  step_current = 0;
  time_start = 0.0;
  time_end = 1.0;
  time_type_name = "CONSTANT"; //OK
  time_unit = "SECOND"; 
  max_time_step = 1.e10;   //YD 
  min_time_step = 0;   //YD 
  repeat = false; //OK/YD
  step_current = 0;		//WW
  this_stepsize = 0.; //WW
  dt_sum = .0;			//WW
}
/**************************************************************************
FEMLib-Method: 
Task: OBJ destructor
Programing:
08/2004 OK Implementation
**************************************************************************/
CTimeDiscretization::~CTimeDiscretization(void) 
{
  if(tim_discrete)  //YD
  {
    tim_discrete->close();
    if(tim_discrete) delete tim_discrete;
    time_step_vector.clear();
    time_adapt_tim_vector.clear();
    time_adapt_coe_vector.clear();
  }
}

ios::pos_type GetNextSubKeyword(ifstream* file,string* line, bool* keyword)
{
  char buffer[MAX_ZEILE];
  ios::pos_type position;
  position = file->tellg();
  *keyword = false;
  string line_complete;
  int i,j;
  // Look for next subkeyword
  while(!(line_complete.find("$")!=string::npos)&&(!file->eof())){
    file->getline(buffer,MAX_ZEILE);
    line_complete = buffer;
    if(line_complete.find("#")!=string::npos){
      *keyword = true;
      return position;
    }
	i = (int) line_complete.find_first_not_of(" ",0); //Anfängliche Leerzeichen überlesen, i=Position des ersten Nichtleerzeichens im string
	j = (int) line_complete.find(";",i);			 //Nach Kommentarzeichen ; suchen. j = Position des Kommentarzeichens, j=-1 wenn es keines gibt.
    if(j<0)
      j = (int)line_complete.length();
	//if(j!=i) break;						 //Wenn das erste nicht-leerzeichen ein Kommentarzeichen ist, zeile überlesen. Sonst ist das eine Datenzeile
	if(i!=-1)
      *line = line_complete.substr(i,j-i);          //Ab erstem nicht-Leerzeichen bis Kommentarzeichen rauskopieren in neuen substring, falls Zeile nicht leer ist
  }
  return position;
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ read function
Programing:
08/2004 OK Implementation
11/2004 OK string streaming by SB for lines
10/2005 YD Time Controls
**************************************************************************/
ios::pos_type CTimeDiscretization::Read(ifstream *tim_file)
{
  string sub_line;
  string line_string;
  string delimiter(" ");
  bool new_keyword = false;
  string hash("#");
  ios::pos_type position;
  string sub_string;
  bool new_subkeyword = false;
  string dollar("$");
  int no_time_steps = 0;
  double time_step_length;
  ios::pos_type position_subkeyword;
  std::stringstream line;
  string line_complete;
  int iter_times;   //YD
  double multiply_coef;    //YD
  int i;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet("RICHARDS_FLOW");
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while(!new_keyword) {
    if(new_subkeyword)
      tim_file->seekg(position,ios::beg);
    new_subkeyword = false;
    position = GetNextSubKeyword(tim_file,&line_string,&new_keyword);
    if(new_keyword) 
      return position;
/*
    position = tim_file->tellg();
    if(new_subkeyword)
      tim_file->seekg(position_subkeyword,ios::beg);
    new_subkeyword = false;
    tim_file->getline(buffer,MAX_ZEILE);
    line_string = buffer;
	if(line_string.size()<1) // empty line
      continue; 
    if(Keyword(line_string)) 
      return position;
*/
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
	  line.str(GetLineFromFile1(tim_file));
      line >> pcs_type_name;
      line.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$TIME_START")!=string::npos) { // subkeyword found
	  line.str(GetLineFromFile1(tim_file));
      line >> time_start;
      line.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$TIME_END")!=string::npos) { // subkeyword found
	  line.str(GetLineFromFile1(tim_file));
      line >> time_end;
      line.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$TIME_UNIT")!=string::npos) { // subkeyword found
      *tim_file>>time_unit>>ws; //WW unit of time
      continue;
    }
    //....................................................................
    if(line_string.find("$TIME_STEPS")!=string::npos) { // subkeyword found
      while((!new_keyword)||(!new_subkeyword)||(!tim_file->eof())){
        position = tim_file->tellg();
        line_string = GetLineFromFile1(tim_file);
        if(line_string.find("#")!=string::npos){
          return position;
        }
        if(line_string.find("$")!=string::npos){
          new_subkeyword = true;
          break;
        }
        line.str(line_string);
        line >> no_time_steps;
        line >> time_step_length;
        for(i=0;i<no_time_steps;i++)
          time_step_vector.push_back(time_step_length);
        line.clear();
      }
    }
    //....................................................................
    if(line_string.find("$TIME_CONTROL")!=string::npos) { // subkeyword found
      while((!new_keyword)||(!new_subkeyword)||(!tim_file->eof())){
        position = tim_file->tellg();
        line_string = GetLineFromFile1(tim_file);
        if(line_string.find("#")!=string::npos){
          return position;
        }
        if(line_string.find("$")!=string::npos){
          new_subkeyword = true;
          break;
        }
        line.str(line_string);
        line >> time_control_name;
        line.clear();
  	    if(time_control_name=="COURANT_MANIPULATE"){
          line_string = GetLineFromFile1(tim_file);
          line.str(line_string);
          line >> time_control_manipulate;
          line.clear();
        }
  	    if(time_control_name=="NEUMANN"){
          line.clear();
        }
  	    if(time_control_name=="ERROR_CONTROL_ADAPTIVE"){
          m_pcs->adaption = true;
          line.clear();
        }
  	    if(time_control_name=="SELF_ADAPTIVE"){
          m_pcs->adaption = true;
          while((!new_keyword)||(!new_subkeyword)||(!tim_file->eof())){
          position = tim_file->tellg();
          line_string = GetLineFromFile1(tim_file);
          if(line_string.find("#")!=string::npos){
            return position;
          }
          if(line_string.find("$")!=string::npos){
            new_subkeyword = true;
            break;
          }
          if(line_string.find("MAX_TIME_STEP")!=string::npos){
          *tim_file >> line_string;
          max_time_step = strtod(line_string.data(),NULL);
          line.clear();
            break;
          }
          if(line_string.find("MIN_TIME_STEP")!=string::npos){
          *tim_file >> line_string;
          min_time_step = strtod(line_string.data(),NULL);
          line.clear();
            break;
          }
          line.str(line_string);
          line >> iter_times;
          line >> multiply_coef;
          time_adapt_tim_vector.push_back(iter_times);
          time_adapt_coe_vector.push_back(multiply_coef);
          line.clear();
		  }
        }
      }
    }
    //....................................................................
  }
  return position;
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ read function
Programing:
08/2004 OK Implementation
01/2005 OK Boolean type
01/2005 OK Destruct before read
**************************************************************************/
bool TIMRead(string file_base_name)
{
  //----------------------------------------------------------------------
//OK  TIMDelete();  
  //----------------------------------------------------------------------
  CTimeDiscretization *m_tim = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //========================================================================
  // File handling
  string tim_file_name = file_base_name + TIM_FILE_EXTENSION;
  ifstream tim_file (tim_file_name.data(),ios::in);
  if (!tim_file.good()) 
    return false;
  tim_file.seekg(0L,ios::beg);
  //========================================================================
  // Keyword loop
  cout << "TIMRead" << endl;
  while (!tim_file.eof()) {
    tim_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return true;
    //----------------------------------------------------------------------
    if(line_string.find("#TIME_STEPPING")!=string::npos) { // keyword found
      m_tim = new CTimeDiscretization();
      position = m_tim->Read(&tim_file);
      m_tim->time_current = m_tim->time_start;
    //----------------------------------------------------------------------
  if(m_tim->Write_tim_discrete)     //YD Write out Time Steps & Iterations
  {
     string m_file_name = file_base_name + "_TimeDiscrete.txt";
     m_tim->tim_discrete = new fstream(m_file_name.c_str(),ios::trunc|ios::out);
     fstream *tim_dis =  m_tim->tim_discrete;
     *tim_dis << " No. Time  Tim-Disc  Iter"<<endl;
     if (!m_tim->tim_discrete->good())
       cout << "Warning : Time-Discrete files are not found" << endl;
  }
    //----------------------------------------------------------------------
      time_vector.push_back(m_tim);
      tim_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  return true;
}
/**************************************************************************
FEMLib-Method: 
Task: master write function
Programing:
01/2005 OK Implementation
last modification:
**************************************************************************/
void TIMWrite(string base_file_name)
{
  CTimeDiscretization *m_tim = NULL;
  string sub_line;
  string line_string;
  //========================================================================
  // File handling
  string tim_file_name = base_file_name + TIM_FILE_EXTENSION;
  fstream tim_file (tim_file_name.data(),ios::trunc|ios::out);
  tim_file.setf(ios::scientific,ios::floatfield);
  tim_file.precision(12);
  if (!tim_file.good()) return;
  tim_file.seekg(0L,ios::beg);
  //========================================================================
  tim_file << "GeoSys-TIM: ------------------------------------------------\n";
  //========================================================================
  // OUT vector
  int tim_vector_size =(int)time_vector.size();
  int i;
  for(i=0;i<tim_vector_size;i++){
    m_tim = time_vector[i];
    m_tim->Write(&tim_file);
  }
  tim_file << "#STOP";
  tim_file.close();
}
/**************************************************************************
FEMLib-Method: 
Task: write function
Programing:
01/2004 OK Implementation
last modification:
**************************************************************************/
void CTimeDiscretization::Write(fstream*tim_file)
{
  int i;
  //--------------------------------------------------------------------
  // KEYWORD
  *tim_file  << "#TIME_STEPPING" << endl;
  //--------------------------------------------------------------------
  // PCS_TYPE
  *tim_file  << " $PCS_TYPE" << endl;
  *tim_file  << "  " << pcs_type_name << endl;
  //--------------------------------------------------------------------
  *tim_file  << " $TIME_START" << endl;
  *tim_file  << "  " << time_start << endl;
  //--------------------------------------------------------------------
  *tim_file  << " $TIME_END" << endl;
  *tim_file  << "  " << time_end << endl;
  //--------------------------------------------------------------------
  *tim_file  << " $TIME_STEPS" << endl;
  for(i=0;i<(int)time_step_vector.size();i++){
    *tim_file  << "  " << 1 << " " << time_step_vector[i] << endl;
  }
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
**************************************************************************/
double CTimeDiscretization::CalcTimeStep(void) 
{
    // time_step_length = 0.0;
    int no_time_steps = (int)time_step_vector.size();
    if(no_time_steps>0)
      time_step_length = time_step_vector[0];
    // Standard time stepping
    if(step_current<no_time_steps){
      time_step_length = time_step_vector[step_current];
    }
    // Time step controls
    if( time_control_name == "NEUMANN"||time_control_name == "SELF_ADAPTIVE")
    {
      if(aktuelle_zeit < MKleinsteZahl) 
        time_step_length = FirstTimeStepEstimate();
      else if( time_control_name == "NEUMANN" )
        time_step_length = NeumannTimeControl();
      else if(time_control_name == "SELF_ADAPTIVE") 
      time_step_length = SelfAdaptiveTimeControl();
    }
    if(time_control_name == "ERROR_CONTROL_ADAPTIVE")
    {
      if(aktuelle_zeit < MKleinsteZahl) 
        time_step_length = AdaptiveFirstTimeStepEstimate();
      else 
        time_step_length = ErrorControlAdaptiveTimeControl();
    }
    //--------
  return time_step_length;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
12/2004 OK Implementation
last modified:
**************************************************************************/
CTimeDiscretization* TIMGet(string pcs_type_name)
{
  CTimeDiscretization *m_tim = NULL;
  int i;
  int no_times = (int)time_vector.size();
  for(i=0;i<no_times;i++){
    m_tim = time_vector[i];
    if(m_tim->pcs_type_name.compare(pcs_type_name)==0)
      return time_vector[i];
  }
  return NULL;
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void TIMDelete()
{
  long i;
  int no_tim =(int)time_vector.size();
  for(i=0;i<no_tim;i++){
    delete time_vector[i];
  }
  time_vector.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void TIMDelete(string pcs_type_name)
{
  long i;
  CTimeDiscretization *m_tim = NULL;
  int no_tim =(int)time_vector.size();
  for(i=0;i<no_tim;i++){
    m_tim = TIMGet(pcs_type_name);
    if(!m_tim) //OK
      continue;
    if(m_tim->pcs_type_name.compare(pcs_type_name)==0){
      delete time_vector[i];
      time_vector.erase(time_vector.begin()+i);
    }
  }
}
/**************************************************************************
FEMLib-Method: 
Task: Neumann estimation
Programing:  
10/2005 YD Implementation
**************************************************************************/
double CTimeDiscretization::FirstTimeStepEstimate(void) 
{
  CMediumProperties* m_mmp = NULL;
  CRFProcess* m_pcs = NULL;
  CElem* elem =NULL;
  int i, j, EleType;
  int idxS;
  long group;
  double GP[3];  
  static double Node_Sat[8];
  double buffer;
  int no_time_steps;
//WW  int no_processes =(int)pcs_vector.size();
  int mmp_vector_size = (int)mmp_vector.size(); 
  CFluidProperties *m_mfp = NULL;
  m_mfp = MFPGet("LIQUID");  //WW
  double density_fluid = m_mfp->Density(); //WW
  // 
  for(int n_p = 0; n_p< (int)pcs_vector.size(); n_p++){
  m_pcs = pcs_vector[n_p];
  CFiniteElementStd* fem = m_pcs->GetAssembler();

  switch(m_pcs->pcs_type_name[0]){
    case 'R': // Richards
      idxS  = m_pcs->GetNodeValueIndex("SATURATION1");
      no_time_steps = 1000000000; //OK (int)(1.0e10);
	  time_step_length = 1.e10;
	  for(int m=0;m< mmp_vector_size;m++) m_mmp = mmp_vector[m];
	  for (i=0;i< (long)m_pcs->m_msh->ele_vector.size();i++){  
        elem = m_pcs->m_msh->ele_vector[i];
        if (elem->GetMark())     // Element selected
           {
            // Activated Element 
            group = elem->GetPatchIndex();
            m_mmp = mmp_vector[group];
            m_mmp->m_pcs=m_pcs;
            EleType = elem->GetElementType();
            if(EleType==4) // Traingle
              {
                 GP[0] = GP[1] = 0.1/0.3; 
                 GP[2] = 0.0;
              }
              else if(EleType==5) 
		         GP[0] = GP[1] = GP[2] = 0.25;
              else
		         GP[0] = GP[1] = GP[2] = 0.0; 
            }
		   for(j=0; j<elem->GetVertexNumber(); j++)
              Node_Sat[j] =  m_pcs->GetNodeValue(elem->GetNodeIndex(j),idxS);
			  buffer = m_mmp->SaturationPressureDependency(fem->interpolate(Node_Sat),  density_fluid, m_pcs->m_num->ls_theta);
			  buffer *= 0.5*elem->GetVolume()*elem->GetVolume();
			  buffer *=m_mmp->porosity_model_values[0]*mfp_vector[0]->Viscosity()/m_mmp->permeability_tensor[0];
              buffer /=m_pcs->time_unit_factor;
			  time_step_length = MMin(time_step_length, buffer);  
			  }
			  if (time_step_length < MKleinsteZahl){
				  cout<<"Waning : Time Control Step Wrong, dt = 0.0 "<<endl;
                  time_step_length = 1.e-6;
			  }
			 cout<<"Neumann Time Step: "<<time_step_length<<endl;
             time_step_length_neumann = 1.e10; 
             time_step_length = MMin(time_step_length, max_time_step);
			if(Write_tim_discrete)
			*tim_discrete<<aktueller_zeitschritt<<"  "<<aktuelle_zeit<<"   "<<time_step_length<< "  "<<m_pcs->iter<<endl;

      break;
//-----------------------------------------------------
 }
}
  return time_step_length;
}
/**************************************************************************
FEMLib-Method: 
Task: Nuemann Control
Programing:
10/2005 YD Implementation
**************************************************************************/
double CTimeDiscretization::NeumannTimeControl(void) 
{
  CRFProcess* m_pcs = NULL;

  for(int n_p = 0; n_p< (int)pcs_vector.size(); n_p++){
  m_pcs = pcs_vector[n_p];
  switch(m_pcs->pcs_type_name[0]){
  case 'R': // Richards
  time_step_length = time_step_length_neumann;
  break;
  default:
      cout << "Fatal error: No valid PCS type" << endl;
      break;
 }
}
  cout<<"Neumann Time Step: "<<time_step_length<<endl;
	time_step_length_neumann = 1.e10;   
  if(Write_tim_discrete)
      *tim_discrete<<aktueller_zeitschritt<<"  "<<aktuelle_zeit<<"   "<<time_step_length<< "  "<<m_pcs->iter<<endl;
  return time_step_length;
}
/**************************************************************************
FEMLib-Method: 
Task: Self adaptive method
Programing:
10/2005 YD Implementation
**************************************************************************/
double CTimeDiscretization::SelfAdaptiveTimeControl(void) 
{
  CRFProcess* m_pcs = NULL; //YDToDo: m_pcs should be member

  if(repeat)
  {
    cout << "   TIM step is repeated" << endl;  
    m_pcs = PCSGet(pcs_type_name);
    m_pcs->PrimaryVariableReload();
  }

  for(int n_p = 0; n_p< (int)pcs_vector.size(); n_p++){
  m_pcs = pcs_vector[n_p];
  switch(m_pcs->pcs_type_name[0]){
  default:
      cout << "Fatal error: No valid PCS type" << endl;
      break;
  case 'R': // Richards
  if(m_pcs->iter <= time_adapt_tim_vector[0]) 
    time_step_length = time_step_length*time_adapt_coe_vector[0];
  else if(m_pcs->iter  >= time_adapt_tim_vector[time_adapt_tim_vector.size()-1])
    time_step_length *= time_adapt_coe_vector[time_adapt_tim_vector.size()-1];
  break;
 }
}
  time_step_length = MMin(time_step_length,max_time_step);
  cout<<"Self_Adaptive Time Step: "<<time_step_length<<endl;
  if(Write_tim_discrete)
     *tim_discrete<<aktueller_zeitschritt<<"  "<<aktuelle_zeit<<"   "<<time_step_length<< "  "<<m_pcs->iter<<endl;
//}
  return time_step_length;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:CMCD 03/2006
**************************************************************************/
/*
void CTimeDiscretization::CheckCourant(void)
{
  long index; 
  long group;
  double velocity[3]={0.,0.,0.};
  double porosity, vg, advective_velocity, length, courant;
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGetFluxProcess();
  if (!m_pcs) return;
  int pcs_no = m_pcs->pcs_number;
  CMediumProperties* m_mmp = NULL;
  CElem* elem =NULL;
  ElementValue* gp_ele;
  long critical_element_no = -1;
  double recommended_time_step = 0.0;
  double stable_time_step = 0.0;
  int edx;


  for (index=0;index< (long)m_pcs->m_msh->ele_vector.size();index++){  
    elem = m_pcs->m_msh->ele_vector[index];
    length = elem->GetRepLength();
    group = elem->GetPatchIndex();
    m_mmp = mmp_vector[group];
    m_mmp->m_pcs = m_pcs;
    porosity = m_mmp->Porosity(m_mmp->Fem_Ele_Std);
    gp_ele = ele_gp_value[index];//to get gp velocities
    gp_ele->getIPvalue_vec(pcs_no, velocity);
    vg = MBtrgVec(velocity,3);
    advective_velocity = vg/porosity;
    courant = dt * advective_velocity/length;
    elem->SetCourant(courant);
    edx = m_pcs->GetElementValueIndex("COURANT");
    m_pcs->SetElementValue(index,edx,courant);
    stable_time_step = (1./courant)*dt;
    if (index == 0) recommended_time_step = stable_time_step;
    if (stable_time_step < recommended_time_step){
      recommended_time_step = stable_time_step;
      critical_element_no = index;
    }
  }
  cout<<"Courant time step control, critical element = "<<critical_element_no<<" Recomended time step "<<recommended_time_step<<endl;
}
*/
/**************************************************************************
FEMLib-Method: 
Task: Neumann estimation
Programing:  
04/2006 YD Implementation
**************************************************************************/
double CTimeDiscretization::AdaptiveFirstTimeStepEstimate(void) 
{
  CNumerics *m_num = NULL;
  CRFProcess* m_pcs = NULL;
  CElem* elem =NULL;
  int i, j, idxp;
  static double Node_p[8];
  double p_ini,buff=0.0;
  int no_time_steps;
  m_num = num_vector[0];
  safty_coe = 5.0;
  p_ini = 1.0e-10;

  for(int n_p = 0; n_p< (int)pcs_vector.size(); n_p++){
  m_pcs = pcs_vector[n_p];
  CFiniteElementStd* fem = m_pcs->GetAssembler();

  switch(m_pcs->pcs_type_name[0]){
    case 'R': // Richards
      idxp  = m_pcs->GetNodeValueIndex("PRESSURE1")+1;
      no_time_steps = 1000000000; //OK (int)(1e10);
	  time_step_length = 1.e10;
	  for (i=0;i< (long)m_pcs->m_msh->ele_vector.size();i++){  
        elem = m_pcs->m_msh->ele_vector[i];
		for(j=0; j<elem->GetVertexNumber(); j++)
           Node_p[j] =  m_pcs->GetNodeValue(elem->GetNodeIndex(j),idxp);
        p_ini = MMax(fabs(fem->interpolate(Node_p)),p_ini);
	  }
	  buff = safty_coe*sqrt(m_num->nls_error_tolerance/p_ini);
      buff /=m_pcs->time_unit_factor;
      time_step_length = MMin(time_step_length,buff);  
	  if (time_step_length < MKleinsteZahl){
		cout<<"Waning : Time Control Step Wrong, dt = 0.0 "<<endl;
        time_step_length = 1.0e-8;
	  }
	  cout<<"Error Control Time Step: "<<time_step_length<<endl;
	  if(Write_tim_discrete)
		*tim_discrete<<aktueller_zeitschritt<<"  "<<aktuelle_zeit<<"   "<<time_step_length<< "  "<<m_pcs->iter<<endl;
      break;
 }
}
  return time_step_length;
}

/**************************************************************************
FEMLib-Method: 
Task: Error control adaptive method                                                              
Programing:
04/2006 YD Implementation
**************************************************************************/
double CTimeDiscretization::ErrorControlAdaptiveTimeControl(void) 
{
  CRFProcess* m_pcs = NULL;
  double rmax = 5.0;
  double rmin = 0.5;
  //double error = 0.0;
  double safty_coe = 0.8;
  //----------------------------------------------------------------------
  for(int n_p = 0; n_p< (int)pcs_vector.size(); n_p++)
  {
    m_pcs = pcs_vector[n_p];
    switch(m_pcs->pcs_type_name[0])
    {
      //..................................................................
      default:
        cout << "Fatal error: No valid PCS type" << endl;
        break;
      //..................................................................
      case 'R': // Richards, accepted and refused time step
        //nonlinear_iteration_error = m_pcs->nonlinear_iteration_error;
        if(repeat)
        {
          time_step_length *=MMax(safty_coe*sqrt(m_pcs->m_num->nls_error_tolerance/nonlinear_iteration_error),rmin); 
        }
        else
        {
          time_step_length *=MMin(safty_coe*sqrt(m_pcs->m_num->nls_error_tolerance/nonlinear_iteration_error),rmax); 
        }
        cout<<"Error_Self_Adaptive Time Step: "<<time_step_length<<endl;
        if(Write_tim_discrete)
          *tim_discrete<<aktueller_zeitschritt<<"  "<<aktuelle_zeit<<"   "<<time_step_length<< "  "<<m_pcs->iter<<endl;
      //..................................................................
    }
  }
  return time_step_length;
}
/**************************************************************************
FEMLib-Method: 
Task:  Check the time of the process in the case: different process has
       different time step size   
Return boolean value: skip or execute the process                                                             
Programing:
06/2007 WW Implementation
09/2007 WW The varable of the time step accumulation as a  member
**************************************************************************/
double CTimeDiscretization::CheckTime(double const c_time, const double dt0) 
{
  double pcs_step;
  double time_forward; 
  bool ontime = false;
  this_stepsize = 0.;
  if((int)time_vector.size()==1)
    return dt0;
//WW please check +1
//OK   double pcs_step = time_step_vector[step_current+1];
  if(step_current>=(int)time_step_vector.size()) //OK
    pcs_step = time_step_vector[(int)time_step_vector.size()-1]; //OK
  else
    pcs_step = time_step_vector[step_current]; //OK
  time_forward = c_time - time_current-pcs_step; 
  if(time_forward>0.0||fabs(time_forward)<DBL_MIN)
  {
    time_current += pcs_step;
    step_current++; 
   	this_stepsize = dt_sum+dt0; 
    ontime = true;
    dt_sum = 0.0;
  }
  if((fabs(pcs_step-time_end)<DBL_MIN)&&fabs(c_time-time_end)<DBL_MIN)
  {
    this_stepsize = dt_sum+dt0;
    ontime = true;
    dt_sum = 0.0;
  }
  if(!ontime)
    dt_sum += dt0; 
  if(pcs_step>time_end)   // make output for other processes
  {
     dt_sum = 0.0; 
     this_stepsize = 0.0;
  }
  return this_stepsize;   
}
/**************************************************************************
FEMLib-Method: 
Programing:
09/2007 WW Implementation
**************************************************************************/
bool IsSynCron()
{
   int i, count = 0;
   for(i=0; i<(int)time_vector.size(); i++)
   {
      if(time_vector[i]->dt_sum<DBL_MIN) 
      count++; 
   }
   if(count==(int)time_vector.size())
     return true;
   else
     return false;   
}

