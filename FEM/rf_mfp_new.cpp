/**************************************************************************
FEMLib - Object: MFP Fluid Properties
Task: 
Programing:
08/2004 OK Implementation
last modified:
**************************************************************************/
#include "stdafx.h" /* MFC */
#include "makros.h"
// C++ STL
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;
// FEM-Makros
#include "mathlib.h"
// GeoSys-GeoLib
#include "geo_strings.h"
#include "rfstring.h"
// GeoSys-FEMLib
#include "fem_ele_std.h"
//
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
extern double InterpolValue(long number,int ndx,double r,double s,double t);
#include "rf_pcs.h"
extern double GetCurveValue(int,int,double,int*);
#include "nodes.h"
#include "tools.h" //GetLineFromFile

// Structure element. To be removed
#include"elements.h"
/* Umrechnungen SI - Amerikanisches System */
#include "steam67.h"
#define PSI2PA 6895.
#define PA2PSI 1.4503263234227701232777374909355e-4
#define GAS_CONSTANT        8314.51
#define COMP_MOL_MASS_AIR   28.96
#define COMP_MOL_MASS_WATER 18.016
#define GAS_CONSTANT_V  461.5
double gravity_constant = 9.81; //TEST for FEBEX OK 9.81;

//==========================================================================
vector<CFluidProperties*>mfp_vector;

/**************************************************************************
FEMLib-Method: 
Task: OBJ constructor
Programing:
08/2004 OK Implementation
**************************************************************************/
CFluidProperties::CFluidProperties(void)
{
  name = "WATER";
  phase = 0;
  // Density
  density_model = 1;
  rho_0 = 1000.;
  drho_dp = 0.;
  drho_dT = 0.;
  drho_dC = 0.;
    // Viscosity
  viscosity_model = 1;
  my_0 = 1e-3;
  dmy_dp = 0.;
  dmy_dT = 0.;
  dmy_dC = 0.;
  // Thermal properties
  heat_capacity_model = 1;
  specific_heat_capacity = 4680.; //CMCD we should give this as SHC not HC GeoSys 4 9/2004
  heat_conductivity_model = 1;
  heat_conductivity = 0.6;
  // Electrical properties
  // Chemical properties
  diffusion_model = 1;
  diffusion = 2.13e-6;
  // State variables
  p_0 = 101325.;
  T_0 = 293.;
  C_0 = 0.;
  Z = 1.;
  cal_gravity = true; 
  // Data
  mode = 0; // Gauss point values
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ deconstructor
Programing:
08/2004 OK Implementation
**************************************************************************/
CFluidProperties::~CFluidProperties(void) 
{
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ read function
Programing:
08/2004 OK Implementation
11/2004 SB string streaming
**************************************************************************/
ios::pos_type CFluidProperties::Read(ifstream *mfp_file)
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
  string delimiter_type(":");
  std::stringstream in;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while (!new_keyword) {
    new_subkeyword = false;
    position = mfp_file->tellg();
//SB    mfp_file->getline(buffer,MAX_ZEILE);
//SB    line_string = buffer;
	line_string = GetLineFromFile1(mfp_file);
	if(line_string.size() < 1) break;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    //....................................................................
    if(line_string.find("$FLUID_TYPE")!=string::npos) { // subkeyword found
//      *mfp_file >> fluid_name;
//       mfp_file->ignore(MAX_ZEILE,'\n');
	  in.str(GetLineFromFile1(mfp_file));
      in >> name; //sub_line
	  in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$DAT_TYPE")!=string::npos) { // subkeyword found
//      *mfp_file >> dat_name;
//       mfp_file->ignore(MAX_ZEILE,'\n');
	  in.str(GetLineFromFile1(mfp_file));
      in >> name; //sub_line
	  in.clear();
      continue;
    }
    if(line_string.find("$NON_GRAVITY")!=string::npos) { //YD/WW subkeyword found
       cal_gravity = false;
       continue;
    }
	//....................................................................
    if(line_string.find("$DENSITY")!=string::npos) { // subkeyword found
      new_subkeyword = false;
//      *mfp_file >> density_model;
	  in.str(GetLineFromFile1(mfp_file));
      in >> density_model;
      if(density_model==0){ // rho = f(x)
//        *mfp_file >> rho_fct_name;
		  in >> rho_fct_name;
      }
      if(density_model==1){ // rho = const
        //*mfp_file >> rho_0;
		in >> rho_0;
      }
      if(density_model==2){ // rho(p) = rho_0*(1+beta_p*(p-p_0))
//        *mfp_file >> rho_0;
//        *mfp_file >> p_0;
//        *mfp_file >> drho_dp;
		in >> rho_0;
        in >> p_0;
        in >> drho_dp;
        density_pcs_name_vector.push_back("PRESSURE1");
      }
      if(density_model==3){ // rho(C) = rho_0*(1+beta_C*(C-C_0))
//        *mfp_file >> rho_0;
//        *mfp_file >> C_0;
//        *mfp_file >> drho_dC;
		in >> rho_0;
        in >> C_0;
        in >> drho_dC;
        density_pcs_name_vector.push_back("CONCENTRATION1");
      }
      if(density_model==4){ // rho(T) = rho_0*(1+beta_T*(T-T_0))
//        *mfp_file >> rho_0;
//        *mfp_file >> T_0;
//        *mfp_file >> drho_dT;
        in >> rho_0;
        in >> T_0;
        in >> drho_dT;
        density_pcs_name_vector.push_back("TEMPERATURE1");
      }
      if(density_model==5){ // rho(C,T) = rho_0*(1+beta_C*(C-C_0)+beta_T*(T-T_0))
//        *mfp_file >> rho_0;
//        *mfp_file >> C_0;
//        *mfp_file >> drho_dC;
//        *mfp_file >> T_0;
//        *mfp_file >> drho_dT;
        in >> rho_0;
        in >> C_0;
        in >> drho_dC;
        in >> T_0;
        in >> drho_dT;
        density_pcs_name_vector.push_back("CONCENTRATION1");
        density_pcs_name_vector.push_back("TEMPERATURE1");
      }
      if(density_model==6){ // rho(p,T) = rho_0*(1+beta_p*(p-p_0)+beta_T*(T-T_0))
//        *mfp_file >> rho_0;
//        *mfp_file >> p_0;
//        *mfp_file >> drho_dp;
//        *mfp_file >> T_0;
//        *mfp_file >> drho_dT;
        in >> rho_0;
        in >> p_0;
        in >> drho_dp;
        in >> T_0;
        in >> drho_dT;
        density_pcs_name_vector.push_back("PRESSURE1");
        density_pcs_name_vector.push_back("TEMPERATURE1");
      }
      if(density_model==7){ // rho(p,p_v,T)
        // no input data required
      }
	  if(density_model==8){ // rho(p,T,C)
//        *mfp_file >> C_0;
        in >> C_0;
		density_pcs_name_vector.push_back("PRESSURE1");
        density_pcs_name_vector.push_back("TEMPERATURE1");
      }
//      mfp_file->ignore(MAX_ZEILE,'\n');
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$VISCOSITY")!=string::npos) { // subkeyword found
//      *mfp_file >> viscosity_model;
	  in.str(GetLineFromFile1(mfp_file));
      in >> viscosity_model;
      if(viscosity_model==0){ // my = fct(x)
//        *mfp_file >> my_fct_name;
        in >> my_fct_name;
      }
      if(viscosity_model==1){ // my = const
//        *mfp_file >> my_0;
        in >> my_0;
      }
      if(viscosity_model==2){ // my(p) = my_0*(1+gamma_p*(p-p_0))
//        *mfp_file >> my_0;
//        *mfp_file >> p_0;
//        *mfp_file >> dmy_dp;
        in >> my_0;
        in >> p_0;
        in >> dmy_dp;
        viscosity_pcs_name_vector.push_back("PRESSURE1");
      }
      if(viscosity_model==3){ // my(T), Yaws et al. (1976)
      }
      if(viscosity_model==4){ // my(T), ???
      }
      if(viscosity_model==5){ // my(p,T), Reichenberg (1971)
      }
      if(viscosity_model==6){ // my(C,T), 
      }
      if(viscosity_model==7){ // my(p,T,C)
        *mfp_file >> C_0;
        viscosity_pcs_name_vector.push_back("PRESSURE1");
        viscosity_pcs_name_vector.push_back("TEMPERATURE1");
      }
//      mfp_file->ignore(MAX_ZEILE,'\n');
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$SPECIFIC_HEAT_CAPACITY")!=string::npos) { // subkeyword found
//      *mfp_file >> heat_capacity_model;
	  in.str(GetLineFromFile1(mfp_file));
      in >> heat_capacity_model;
      if(heat_capacity_model==0){ // c = fct(x)
//        *mfp_file >> heat_capacity_fct_name;
        in >> heat_capacity_fct_name;
      }
      if(heat_capacity_model==1){ // c = const
//        *mfp_file >> specific_heat_capacity;//CMCD Change to specific heat capacity, as fluid mass content is f(T,P,n, density)
        in >> specific_heat_capacity;
        specific_heat_capacity_pcs_name_vector.push_back("PRESSURE1");
        specific_heat_capacity_pcs_name_vector.push_back("TEMPERATURE1");
      }
	  if(heat_capacity_model==2){ // my(p,T,C)
//        *mfp_file >> C_0;
        in >> C_0;
        specific_heat_capacity_pcs_name_vector.push_back("PRESSURE1");
        specific_heat_capacity_pcs_name_vector.push_back("TEMPERATURE1");
      }
	  if(heat_capacity_model==3){ // YD: improved phase change
        in >> T_Latent1; // Tmin for phase change
        in >> T_Latent2; // Tmax for phase change
		in >> heat_phase_change_curve;  
        specific_heat_capacity_pcs_name_vector.push_back("PRESSURE1");
        specific_heat_capacity_pcs_name_vector.push_back("TEMPERATURE1");
        specific_heat_capacity_pcs_name_vector.push_back("SATURATION1");
        enthalpy_pcs_name_vector.push_back("TEMPERATURE1");
      }
	  if(heat_capacity_model==4){ // YD: improved phase change
        in >> T_Latent1;    // Tmin for phase change
        in >> T_Latent2;    // Tmax for phase change
        in >> specific_heat_capacity;   // ^c
        in >> latent_heat;    // L
        specific_heat_capacity_pcs_name_vector.push_back("PRESSURE1");
        specific_heat_capacity_pcs_name_vector.push_back("TEMPERATURE1");
        specific_heat_capacity_pcs_name_vector.push_back("SATURATION1");
        enthalpy_pcs_name_vector.push_back("TEMPERATURE1");
      }
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$HEAT_CONDUCTIVITY")!=string::npos) { // subkeyword found
//      *mfp_file >> heat_conductivity_model;
	  in.str(GetLineFromFile1(mfp_file));
      in >> heat_conductivity_model;

      if(heat_conductivity_model==0){ // my = fct(x)
//        *mfp_file >> heat_conductivity_fct_name;
        in >> heat_conductivity_fct_name;
      }
      if(heat_conductivity_model==1){ // my = const
//        *mfp_file >> heat_conductivity;
        in >> heat_conductivity;
      }
	  if(heat_conductivity_model==2){ // my = f(p,T,C)
//        *mfp_file >> C_0;
        in >> C_0;
        heat_conductivity_pcs_name_vector.push_back("PRESSURE1");
        heat_conductivity_pcs_name_vector.push_back("TEMPERATURE1");
      }
//      mfp_file->ignore(MAX_ZEILE,'\n');
      in.clear();
      continue;
    }
	if(line_string.find("$PHASE_DIFFUSION")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(mfp_file));
      in >> diffusion_model;

      if(diffusion_model==0){ // D = fct(x) 
        in >> dif_fct_name;
      }
      if(diffusion_model==1){ // D = const //MX
        in >> diffusion;
      }
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$GRAVITY")!=string::npos) { // CMCD outer space version
	  in.str(GetLineFromFile1(mfp_file));
      in >> gravity_constant; 
	  in.clear();
      continue;
    }
    //....................................................................
  }
  return position;
}

/**************************************************************************
FEMLib-Method: 
Task: Master read function
Programing:
08/2004 OK Implementation
01/2005 OK Boolean type
01/2005 OK Destruct before read
**************************************************************************/
bool MFPRead(string file_base_name)
{
  //----------------------------------------------------------------------
  MFPDelete();  
  //----------------------------------------------------------------------
  CFluidProperties *m_mfp = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //========================================================================
  // File handling
  string mfp_file_name = file_base_name + MFP_FILE_EXTENSION;
  ifstream mfp_file (mfp_file_name.data(),ios::in);
  if (!mfp_file.good()) 
    return false;
  mfp_file.seekg(0L,ios::beg);
  //========================================================================
  // Keyword loop
  cout << "MFPRead" << endl;
  while (!mfp_file.eof()) {
    mfp_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return true;
    //----------------------------------------------------------------------
    if(line_string.find("#FLUID_PROPERTIES")!=string::npos) { // keyword found
      m_mfp = new CFluidProperties();
      m_mfp->file_base_name = file_base_name;
      position = m_mfp->Read(&mfp_file);
      m_mfp->phase = (int)mfp_vector.size(); //OK4108
      mfp_vector.push_back(m_mfp);
      mfp_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  //========================================================================
  // Configuration
  int i;
  int no_fluids =(int)mfp_vector.size();
  if(no_fluids==1){
    m_mfp = mfp_vector[0];
    m_mfp->phase=0;
  }
  else if(no_fluids==2){
    for(i=0;i<no_fluids;i++){
      m_mfp = mfp_vector[i];
      if(m_mfp->name.find("GAS")!=string::npos)
        m_mfp->phase=0;
      else
        m_mfp->phase=1;
    }
  }
  //----------------------------------------------------------------------
  // Test
  if((int)mfp_vector.size()==0){
    cout << "Error in MFPRead: no MFP data" << endl;
#ifdef MFC
    AfxMessageBox("Error in MFPRead: no MFP data");
#endif    
    abort();
  }
  //----------------------------------------------------------------------
  return true;
}

/**************************************************************************
FEMLib-Method: 
Task: write function
Programing:
11/2004 SB Implementation
last modification:
**************************************************************************/
void CFluidProperties::Write(ofstream* mfp_file)
{
  //KEYWORD
  *mfp_file  << "#FLUID_PROPERTIES" << endl;
  *mfp_file  << " $FLUID_TYPE" << endl;
  *mfp_file  << "  " << name << endl;
  *mfp_file  << " $DAT_TYPE" << endl;
  *mfp_file  << "  " << name << endl;
  *mfp_file  << " $DENSITY" << endl;
  if(density_model == 0) *mfp_file << "  " << density_model << " " << rho_fct_name << endl;
  if(density_model == 1) *mfp_file << "  " << density_model << " " << rho_0 << endl;
  //todo
  *mfp_file  << " $VISCOSITY" << endl;
  if(viscosity_model == 0) *mfp_file << "  " << viscosity_model << " " << my_fct_name << endl;
  if(viscosity_model == 1) *mfp_file << "  " << viscosity_model << " " << my_0 << endl;
  //todo
  *mfp_file  << " $SPECIFIC_HEAT_CAPACITY" << endl;
  if(heat_capacity_model == 0) *mfp_file << "  " << heat_capacity_model << " " << heat_capacity_fct_name << endl;
  if(heat_capacity_model == 1) *mfp_file << "  " << heat_capacity_model << " " << specific_heat_capacity << endl;
  *mfp_file  << " $SPECIFIC_HEAT_CONDUCTIVITY" << endl;
  if(heat_conductivity_model == 0) *mfp_file << "  " << heat_conductivity_model << " " << heat_conductivity_fct_name << endl;
  if(heat_conductivity_model == 1) *mfp_file << "  " << heat_conductivity_model << " " << heat_conductivity << endl;
  //--------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method:
Task: Master write function
Programing:
08/2004 OK Implementation
last modification:
**************************************************************************/
void MFPWrite(string base_file_name)
{
  CFluidProperties *m_mfp = NULL;
  string sub_line;
  string line_string;
  ofstream mfp_file;
  //========================================================================
  // File handling
  string mfp_file_name = base_file_name + MFP_FILE_EXTENSION;
  mfp_file.open(mfp_file_name.data(),ios::trunc|ios::out);
  mfp_file.setf(ios::scientific,ios::floatfield);
  mfp_file.precision(12);
  if (!mfp_file.good()) return;
//  mfp_file.seekg(0L,ios::beg);
  //========================================================================
  mfp_file << "GeoSys-MFP: Material Fluid Properties -------------" << endl;
  //========================================================================
  // OUT vector
  int mfp_vector_size =(int)mfp_vector.size();
  int i;
  for(i=0;i<mfp_vector_size;i++){
    m_mfp = mfp_vector[i];
    m_mfp->Write(&mfp_file);
  }
  mfp_file << "#STOP";
  mfp_file.close();
//  delete mfp_file;
}

////////////////////////////////////////////////////////////////////////////
// Properties functions
////////////////////////////////////////////////////////////////////////////
/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
09/2005 WW implementation 
11/2005 YD modification
11/2005 CMCD Inclusion current and previous time step quantities
last modification:
**************************************************************************/
void CFluidProperties::CalPrimaryVariable(vector<string>& pcs_name_vector)
{
  CRFProcess* m_pcs = NULL;
 
  int nidx0,nidx1;
  if(!Fem_Ele_Std) //OK
    return;
  for(int i=0;i<(int)pcs_name_vector.size();i++){
    //MX  m_pcs = PCSGet("HEAT_TRANSPORT");
     m_pcs = PCSGet(pcs_name_vector[i],true);
	 if (!m_pcs) return;  //MX
     nidx0 = m_pcs->GetNodeValueIndex(pcs_name_vector[i]);
	 nidx1 = nidx0+1;
    
    if(mode==0){ // Gauss point values
      primary_variable_t0[i]= Fem_Ele_Std->interpolate(nidx0,m_pcs); 
      primary_variable_t1[i]= Fem_Ele_Std->interpolate(nidx1,m_pcs);
      primary_variable[i] = (1.-Fem_Ele_Std->pcs->m_num->ls_theta)*Fem_Ele_Std->interpolate(nidx0,m_pcs) 
                                + Fem_Ele_Std->pcs->m_num->ls_theta*Fem_Ele_Std->interpolate(nidx1,m_pcs);
 
    }
    else if(mode==2){ // Element average value
      primary_variable[i] = (1.-Fem_Ele_Std->pcs->m_num->ls_theta)*Fem_Ele_Std->elemnt_average(nidx0,m_pcs)
                                + Fem_Ele_Std->pcs->m_num->ls_theta*Fem_Ele_Std->elemnt_average(nidx1,m_pcs);
      primary_variable_t0[i]= Fem_Ele_Std->elemnt_average(nidx0,m_pcs); 
      primary_variable_t1[i]= Fem_Ele_Std->elemnt_average(nidx1,m_pcs);
	}
  }
}
////////////////////////////////////////////////////////////////////////////
// Fluid density
/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation 
           based on MATCalcFluidDensity by OK/JdJ,AH,MB
11/2005 YD Modification
last modification:
**************************************************************************/
double CFluidProperties::Density()
{
  static double density;
  static double air_gas_density,vapour_density,vapour_pressure;
  int fct_number = 0;
  int gueltig;
  //----------------------------------------------------------------------
  CalPrimaryVariable(density_pcs_name_vector);
  //----------------------------------------------------------------------
  switch(density_model){
    case 0: // rho = f(x)
      density = GetCurveValue(fct_number,0,primary_variable[0],&gueltig);
      break;
    case 1: // rho = const
      density = rho_0;
      break;
    case 2: // rho(p) = rho_0*(1+beta_p*(p-p_0))
      density = rho_0*(1.+drho_dp*(max(primary_variable[0],0.0)-p_0));
      break;
    case 3: // rho(C) = rho_0*(1+beta_C*(C-C_0))
      density = rho_0*(1.+drho_dC*(max(primary_variable[0],0.0)-C_0));
      break;
    case 4: // rho(T) = rho_0*(1+beta_T*(T-T_0))
      density = rho_0*(1.+drho_dT*(max(primary_variable[0],0.0)-T_0));
      break;
    case 5: // rho(C,T) = rho_0*(1+beta_C*(C-C_0)+beta_T*(T-T_0))
      density = rho_0*(1.+drho_dC*(max(primary_variable[0],0.0)-C_0)+drho_dT*(max(primary_variable[1],0.0)-T_0));
      break;
    case 6: // rho(p,T) = rho_0*(1+beta_p*(p-p_0)+beta_T*(T-T_0))
      density = rho_0*(1.+drho_dp*(max(primary_variable[0],0.0)-p_0)+drho_dT*(max(primary_variable[1],0.0)-T_0));
      break;
    case 7: // rho_w^l(p,T) for gas phase
      vapour_pressure = MFPCalcVapourPressure(primary_variable[0]);
      air_gas_density = (COMP_MOL_MASS_AIR * (primary_variable[1]-vapour_pressure)) / (GAS_CONSTANT*(primary_variable[0]+0.0));
      vapour_density = (COMP_MOL_MASS_WATER*vapour_pressure) / (GAS_CONSTANT*(primary_variable[0]+0.0));
      density = vapour_density + air_gas_density;
      break;
    case 8: // M14 von JdJ
	  density = MATCalcFluidDensityMethod8(primary_variable[0],primary_variable[1],primary_variable[2]);
      break;
    default:
      cout << "Error in CFluidProperties::Density: no valid model" << endl;
      break;
  }
  return density;
}

/*************************************************************************
 ROCKFLOW - Funktion: MATCalcFluidDensityMethod8

 Task:
   Density of a geothermal fluid as a function of temperature, pressure
   and concentration. 

   Fluid-Density function according to IAPWS-IF97
 
 Programmaenderungen:
   09/2003   CMCD  ARL  First implementation
   09/2004   CMCD  Inclusion in GeoSys vs. 4

*************************************************************************/
double CFluidProperties::MATCalcFluidDensityMethod8(double Press, double TempK, double Conc)
{ 
  Conc = Conc;
	/*int c_idx;*/
	double rho_0;
    double GammaPi, Pressurevar, Tau, pressure_average, temperature_average;
	double Tstar, Pstar,GazConst;
    double L[35],J[35],n[35];
    int i;
    double salinity;
	
	pressure_average = Press;
	temperature_average = TempK;
	salinity = C_0;
	Tstar = 1386;
	Pstar = 16.53e6; // MPa
	GazConst = 0.461526e3; //

	 n[0] = 0.0;
	 n[1] = 0.14632971213167;
	 n[2] = -0.84548187169114;
	 n[3] = -0.37563603672040e1;
	 n[4] = 0.33855169168385e1;
	 n[5] = -0.95791963387872;
	 n[6] = 0.15772038513228;
	 n[7] = -0.16616417199501e-1;
	 n[8] = 0.81214629983568e-3;
	 n[9] = 0.28319080123804e-3;
	 n[10] = -0.60706301565874e-3;
	 n[11] = -0.18990068218419e-1;
	 n[12] = -0.32529748770505e-1;
	 n[13] = -0.21841717175414e-1;
	 n[14] = -0.52838357969930e-4;
	 n[15] = -0.47184321073267e-3;
	 n[16] = -0.30001780793026e-3;
	 n[17] = 0.47661393906987e-4;
	 n[18] = -0.44141845330846e-5;
	 n[19] = -0.72694996297594e-15;
	 n[20] = -0.31679644845054e-4;
	 n[21] = -0.28270797985312e-5;
	 n[22] = -0.85205128120103e-9;
	 n[23] = -0.22425281908000e-5;
	 n[24] = -0.65171222895601e-6;
	 n[25] = -0.14341729937924e-12;
	 n[26] = -0.40516996860117e-6;
	 n[27] = -0.12734301741641e-8;
	 n[28] = -0.17424871230634e-9;
	 n[29] = -0.68762131295531e-18;
	 n[30] = 0.14478307828521e-19;
	 n[31] = 0.26335781662795e-22;
	 n[32] = -0.11947622640071e-22;
	 n[33] = 0.18228094581404e-23;
	 n[34] = -0.93537087292458e-25;



	 L[0] =0.;
	 L[1] =0.;
	 L[2] =0.;
	 L[3] =0.;
	 L[4] =0.;
	 L[5] =0.;
	 L[6] =0.;
	 L[7] =0.;
	 L[8] =0.;
	 L[9] =1.;
	 L[10] =1.;
	 L[11] =1.;
	 L[12] =1.;
	 L[13] =1.;
	 L[14] =1.;
	 L[15] =2.;
	 L[16] =2.;
	 L[17] =2.;
	 L[18] =2.;
	 L[19] =2.;
	 L[20] =3.;
	 L[21] =3.;
	 L[22] =3.;
	 L[23] =4.;
	 L[24] =4.;
	 L[25] =4.;
	 L[26] =5.;
	 L[27] =8.;
	 L[28] =8.;
	 L[29] =21.;
	 L[30] =23.;
	 L[31] =29.;
	 L[32] =30.;
	 L[33] =31.;
	 L[34] =32.;

	 J[0] =-2.;
	 J[1] =-2.;
	 J[2] =-1.;
	 J[3] =0.;
	 J[4] =1.;
	 J[5] =2.;
	 J[6] =3.;
	 J[7] =4.;
	 J[8] =5.;
	 J[9] =-9.;
	 J[10] =-7.;
	 J[11] =-1.;
	 J[12] =0.;
	 J[13] =1.;
	 J[14] =3.;
	 J[15] =-3.;
	 J[16] =0.;
	 J[17] =1.;
	 J[18] =3.;
	 J[19] =17.;
	 J[20] =-4.;
	 J[21] =0.;
	 J[22] =6.;
	 J[23] =-5.;
	 J[24] =-2.;
	 J[25] =10.;
	 J[26] =-8.;
	 J[27] =-11.;
	 J[28] =-6.;
	 J[29] =-29.;
	 J[30] =-31.;
	 J[31] =-38.;
	 J[32] = -39.;
	 J[33] =-40.;
	 J[34] =-41.;

		
	Pressurevar = pressure_average / Pstar;
	Tau = Tstar / temperature_average;


			/*BEGIN:Calculation of GammaPi*/
			GammaPi = 0.;

			for (i=1; i<35; i++)
			{
				
			GammaPi = GammaPi - (n[i]) * (L[i]) * (pow((7.1-Pressurevar),(L[i] -1))) * (pow((Tau-1.222),J[i]));
				
			}
			/*END: Calculation of GammaPi*/
			
			/*BEGIN: Calculation of density*/
			rho_0 = pressure_average / (GazConst * temperature_average * Pressurevar * GammaPi);
			/*END: Calculation of density*/


/*  return rho_0 + drho_dC * (concentration_average - c0);   */
			/*printf("%f", rho_0 + salinity);*/
			return rho_0 + salinity;
}

////////////////////////////////////////////////////////////////////////////
// Fluid viscosity
/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK Implementation
11/2005 YD Modification
last modification:
**************************************************************************/
double CFluidProperties::Viscosity()
{
  static double viscosity;
  int fct_number = 0;
  int gueltig;
  //----------------------------------------------------------------------
  bool New = false; // To be
  if(fem_msh_vector.size()>0) New = true;
  //----------------------------------------------------------------------
  CalPrimaryVariable(viscosity_pcs_name_vector);
  //----------------------------------------------------------------------
  switch(viscosity_model){
    case 0: // rho = f(x)
      viscosity = GetCurveValue(fct_number,0,primary_variable[0],&gueltig);
      break;
    case 1: // my = const
      viscosity = my_0;
      break;
    case 2: // my(p) = my_0*(1+gamma_p*(p-p_0))
      viscosity = my_0*(1.+dmy_dp*(max(primary_variable[0],0.0)-p_0));
      break;
    case 3: // my^l(T), Yaws et al. (1976)
      viscosity = LiquidViscosity_Yaws_1976(primary_variable[1]); //ToDo pcs_name
      break;
    case 4: // my^g(T), Marsily (1986)
      viscosity = LiquidViscosity_Marsily_1986(primary_variable[0]);
      break;
    case 5: // my^g(p,T), Reichenberg (1971)
      viscosity = GasViscosity_Reichenberg_1971(primary_variable[0],primary_variable[1]);
      break;
    case 6: // my(C,T), 
      viscosity = LiquidViscosity_NN(primary_variable[0],primary_variable[1]);
      break;
    case 7: // my(p,C,T), 
      viscosity = LiquidViscosity_CMCD(primary_variable[0],primary_variable[1],primary_variable[2]);
      break;
    default:
      cout << "Error in CFluidProperties::Viscosity: no valid model" << endl;
      break;
  }
  return viscosity;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Dynamische Gasviskositaet nach Reichenberg (1971)
   als Funktion von Temperatur und Druck
   in Reid et al. (1988), S. 420
Programing:
08/2004 OK MFP implementation based on CalcFluidViscosityMethod7 by OK
last modification:
**************************************************************************/
double CFluidProperties::GasViscosity_Reichenberg_1971(double p,double T)
{
  double my,my0;
  double A,B,C,D;
  double Q,Pr,Tr;
  double alpha1, alpha2, beta1, beta2, gamma1, gamma2, delta1, delta2;
  double a,c,d;
  double pc,Tc;
  
  alpha1 = 1.9824e-3;
  alpha2 = 5.2683;
  beta1  = 1.6552;
  beta2  = 1.2760;
  gamma1 = 0.1319;
  gamma2 = 3.7035;
  delta1 = 2.9496;
  delta2 = 2.9190;
  a = -0.5767;
  c = -79.8678;
  d = -16.6169;
  Q = 1.0;
  Tc = 126.2; 
  Tr = T/Tc;
  pc = 33.9*10000.; /* bar->Pascal*/
  Pr = p/pc;

  A = alpha1/Tr * exp(alpha2*pow(Tr,a));
  B = A*(beta1*Tr-beta2);
  C = gamma1/Tr * exp(gamma2*pow(Tr,c));
  D = delta1/Tr * exp(delta2*pow(Tr,d));

  my0 = 26.69*sqrt(28.96)*sqrt(T)/(3.7*3.7) * 1.e-6 * 0.1; /* Poise->Pa*s */
  my = my0 * ( 1.0 + Q * (A * pow(Pr,1.5))/(B*Pr+(1/(1+C*pow(Pr,D)))) );
  return my;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Dynamische Flüssigkeits-Viskositaet nach Yaws et al. (1976)
   als Funktion von Temperatur
   in Reid et al. (1988), S. 441/455
   Eqn.(3): ln(my) = A + B/T + CT + DT^2
Programing:
08/2004 OK MFP implementation 
           based on CalcFluidViscosityMethod8 by OK (06/2001)
last modification:
**************************************************************************/
double CFluidProperties::LiquidViscosity_Yaws_1976(double T)
{
  double ln_my,my;
  double A,B,C,D;

  A = -2.471e+01;
  B =  4.209e+03;
  C =  4.527e-02;
  D = -3.376e-5;

  ln_my = A + B/T + C*T + D*T*T;
  my = exp(ln_my); /* in cP */
  my = my * 1.e-3; /* in Pa s */
  return my;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Flüssigkeits-Viskositaet in Abhaengigkeit von der Temperatur
   (nach Marsily 1986)
Programing:
08/2004 OK MFP implementation 
           based on CalcFluidViscosityMethod9 by OK (05/2001)
last modification:
**************************************************************************/
double CFluidProperties::LiquidViscosity_Marsily_1986(double T)
{
  double my;
  my = 2.285e-5 + 1.01e-3*log(T);
  return my;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Liefert die Viskositaet in Abhaengigkeit von der Konzentration
   und der Temperatur.
Programing:
02/2000 AH Erste Version
08/2004 OK MFP implementation 
last modification:
**************************************************************************/
double CFluidProperties::LiquidViscosity_NN(double c,double T)
{
  double f1, f2, mu0 = 0.001, mu;
  double omega0, omega, sigma0, sigma;

  if (rho_0 <  MKleinsteZahl|| T_0 < MKleinsteZahl)
      return 0.;

  omega = c / rho_0;
  omega0 = C_0 / rho_0;
  sigma = (T - T_0) / T_0;
  sigma0 = 0.;

  f1 = (1. + 1.85 * omega0 - 4.1 * omega0 * omega0 + 44.5 * omega0 * omega0 * omega0) /
       (1. + 1.85 * omega - 4.1 * omega * omega + 44.5 * omega * omega * omega);
  f2 = (1 + 0.7063 * sigma - 0.04832 * sigma * sigma * sigma) /
       (1 + 0.7063 * sigma0 - 0.04832 * sigma0 * sigma * sigma0);
  mu = mu0 / (f1 + f2);
  return mu;
}

////////////////////////////////////////////////////////////////////////////
// Fluid thermal properties
/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation based on MATCalcFluidHeatCapacity (OK)
10/2005 WW/YD Case 3: simplified phase change
10/2005 YD Case 4: improved phase change
11/2005 CMCD edited cases and expanded case 3 & 4
last modification:
**************************************************************************/
double CFluidProperties::SpecificHeatCapacity()
{
  int gueltig = -1;
  int group = -1;
  double density_gas,density_liquid,density_vapor;
  double specific_heat_capacity_gas,specific_heat_capacity_liquid, heat_capacity_liquid;
  double pressure_liquid,saturation_liquid, temperature;
  double porosity,latent_heat_liquid, heat_capacity_gas, humi;
  double drdT, heat_capacity_phase_change;
  double H1,H0,T0,T_1;//T1 defined at 662 in steam67???
  CMediumProperties* m_mmp;
  //......................................................................
  CalPrimaryVariable(specific_heat_capacity_pcs_name_vector);
  pressure_liquid = primary_variable[0];  
  temperature =  primary_variable[1]; 
  saturation_liquid = primary_variable[2]; 
  //......................................................................
  //
  switch(heat_capacity_model){
    case 0: // rho = f(x)
      specific_heat_capacity = GetCurveValue(0,0,temperature,&gueltig);
      break;
    case 1: // c = const, value already read in to specific_heat_capacity
      break;
	  case 2: // c = f(p,T,Conc)
	    specific_heat_capacity = MATCalcFluidHeatCapacityMethod2(primary_variable[0],primary_variable[1],primary_variable[2]);
	    break;
	  case 3: // phase change c = f(x)
		  T_1 = primary_variable_t1[1];
      if(T_1 <= T_Latent1 || T_1 >= T_Latent2)
        specific_heat_capacity = GetCurveValue(heat_phase_change_curve,0,temperature_buffer,&gueltig);
  		else{
        heat_capacity_model = 5;
		    H1 = CalcEnthalpy();
		    T0 = primary_variable_t0[1];
        if(fabs(T_1-T0)<1.0e-8) T_1 +=1.0e-8;
			    H0 = CalcEnthalpy(); 
		      specific_heat_capacity = (H1-H0)/(T_1-T_0);
		    }
        heat_capacity_model = 3;
	    break;
	  case 4: // improved phase change (Richards model)
		  T_1 = primary_variable_t1[1];
      if(T_1 <= T_Latent1 || T_1 >= T_Latent2){
        group = Fem_Ele_Std->GetMeshElement()->GetPatchIndex();
        m_mmp = mmp_vector[group];
        porosity = m_mmp->Porosity(Fem_Ele_Std); // 0,NULL,0.0,
	    // Gas phase
  	    density_gas = Fem_Ele_Std->GasProp->vaporDensity(temperature_buffer); 
	      specific_heat_capacity_gas = Fem_Ele_Std->GasProp->specific_heat_capacity;    //0.0; // C_g  to do
	    // Liquid phase
        density_liquid = Fem_Ele_Std->FluidProp->Density();
	      specific_heat_capacity_liquid = Fem_Ele_Std->FluidProp->specific_heat_capacity;    //0.0; // C_g  to do
        latent_heat_liquid = Fem_Ele_Std->FluidProp->latent_heat;
        // see formula in Manual
		    heat_capacity_gas = porosity * (1.-saturation_liquid) * density_gas * specific_heat_capacity_gas;
        heat_capacity_liquid = porosity * saturation_liquid * density_liquid * specific_heat_capacity_liquid;
        humi = exp(pressure_liquid/(GAS_CONSTANT_V*temperature_buffer*density_liquid));   
	      density_vapor = humi*density_liquid; 
        drdT = (Fem_Ele_Std->FluidProp->vaporDensity_derivative(temperature_buffer)*humi \
                    - density_vapor*pressure_liquid/(GAS_CONSTANT_V*density_liquid*pow(temperature_buffer,2.0)))/density_liquid;
	      H1 = porosity*(1.0-saturation_liquid)*  \
                     (latent_heat_liquid+specific_heat_capacity_liquid*(temperature_buffer-Fem_Ele_Std->FluidProp->T_Latent1));
        heat_capacity_phase_change = H1*drdT;
        specific_heat_capacity = heat_capacity_gas + heat_capacity_liquid + heat_capacity_phase_change;
      }
  		else {
        heat_capacity_model = 5;
		  	H1 = CalcEnthalpy();
		    T0 = primary_variable_t0[1];
        if(fabs(T_1-T0)<1.0e-8) T_1 +=1.0e-8;
			    H0 = CalcEnthalpy(); 
		    specific_heat_capacity = (H1-H0)/(T_1-T0);
		  } 
      heat_capacity_model = 4;
	    break;
      case 5:
        specific_heat_capacity = GetCurveValue(heat_phase_change_curve,0,temperature_buffer,&gueltig);
      break;
  }
  return specific_heat_capacity;
}

/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation based on MATCalcFluidHeatCapacity (OK)
10/2005 YD/OK: general concept for heat capacity
**************************************************************************/
double MFPCalcFluidsHeatCapacity(long index,double*gp,double theta, CFiniteElementStd* assem)
{
  double saturation_phase;
  double heat_capacity_fluids=0.0;
  int nidx0,nidx1;
  //--------------------------------------------------------------------
  // MMP medium properties
  bool New = false; // To be removed. WW
  if(fem_msh_vector.size()>0) New = true;
  //----------------------------------------------------------------------
  CFluidProperties *m_mfp = NULL;
  int no_fluids =(int)mfp_vector.size();
  //YD----------- ?? OK efficiency
  CRFProcess* m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
	if(m_pcs->pcs_type_name.find("RICHARDS_FLOW"))no_fluids =1;
  }
  //YD-----------
  switch(no_fluids){
    //....................................................................
    case 1:
      m_mfp = mfp_vector[0]; //YD
      if(New) //WW
	  {
          heat_capacity_fluids = m_mfp->Density() \
                              * m_mfp->SpecificHeatCapacity();
	  }
	  else
          heat_capacity_fluids = m_mfp->Density() \
                               * m_mfp->SpecificHeatCapacity();

      break;
    //....................................................................
    case 2:
      if(New) //WW
	  {
         nidx0 = m_pcs->GetNodeValueIndex("SATURATION1");
         nidx1 = nidx0+1;
		 saturation_phase = (1.-theta)*assem->interpolate(nidx0,m_pcs)
                                + theta*assem->interpolate(nidx1,m_pcs);
	  }
	  else
	  {
         nidx0 = PCSGetNODValueIndex("SATURATION1",0);
         nidx1 = PCSGetNODValueIndex("SATURATION1",1);	  
         saturation_phase = (1.-theta)*InterpolValue(index,nidx0,gp[0],gp[1],gp[2]) \
                       + theta*InterpolValue(index,nidx1,gp[0],gp[1],gp[2]);
	  }
      m_mfp = mfp_vector[0];
      heat_capacity_fluids = saturation_phase \
                           * m_mfp->Density() \
                           * m_mfp->SpecificHeatCapacity();
      m_mfp = mfp_vector[1];
      heat_capacity_fluids += (1.0-saturation_phase) \
                           * m_mfp->Density() \
                           * m_mfp->SpecificHeatCapacity();
      break;
    //....................................................................
    case 3: // Entropy based
      break;
    //....................................................................
    default:
      cout << "Error in MFPCalcFluidsHeatCapacity: no fluid phase data" << endl;
  }
  return heat_capacity_fluids;
}
/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation based on MATCalcFluidHeatCapacity (OK)
10/2005 YD/OK: general concept for heat capacity
overloaded function, see above, taken out, CMCD
**************************************************************************/
/*double MFPCalcFluidsHeatCapacity(double temperature, CFiniteElementStd* assem)
{
  double saturation_phase;
  double heat_capacity_fluids=0.0;
  int nidx0,nidx1;
  //--------------------------------------------------------------------
  // MMP medium properties
  bool New = false; // To be removed. WW
  if(fem_msh_vector.size()>0) New = true;
  //----------------------------------------------------------------------
  CFluidProperties *m_mfp = NULL;
  int no_fluids =(int)mfp_vector.size();
  switch(no_fluids){
    //....................................................................
    case 1:
      //m_mfp = mfp_vector[0];
      if(New) //WW
	  {
          heat_capacity_fluids = assem->FluidProp->Density() \
                              * assem->FluidProp->SpecificHeatCapacity();
	  }
	  else
          heat_capacity_fluids = assem->FluidProp->Density() \
                               * assem->FluidProp->SpecificHeatCapacity();

      break;
    //....................................................................
    case 2:
      if(New) //WW
	  {
         nidx0 = GetNodeValueIndex("SATURATION1");
         nidx1 = nidx0+1;
		 saturation_phase = (1.-assem->pcs->m_num->ls_theta)*assem->interpolate(nidx0)
                                + assem->pcs->m_num->ls_theta*assem->interpolate(nidx1);
	  }
	  else
	  {
         nidx0 = PCSGetNODValueIndex("SATURATION1",0);
         nidx1 = PCSGetNODValueIndex("SATURATION1",1);	  
         saturation_phase = (1.-assem->pcs->m_num->ls_theta)*assem->interpolate(nidx0) \
                       + assem->pcs->m_num->ls_theta*assem->interpolate(nidx1);
	  }
      m_mfp = mfp_vector[0];
      heat_capacity_fluids = saturation_phase \
                           * assem->FluidProp->Density() \
                           * assem->FluidProp->SpecificHeatCapacity();
      m_mfp = mfp_vector[1];
      heat_capacity_fluids += (1.0-saturation_phase) \
                           * assem->FluidProp->Density() \
                           * assem->FluidProp->SpecificHeatCapacity();
      break;
    //....................................................................
    case 3: // Entropy based
      break;
    //....................................................................
    default:
      cout << "Error in MFPCalcFluidsHeatCapacity: no fluid phase data" << endl;
  }
  return heat_capacity_fluids;
}*/

/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation based on MATCalcFluidHeatCapacity (OK)
11/2005 YD Modification
last modification:
**************************************************************************/
double CFluidProperties::HeatConductivity()
{
  int fct_number = 0;
  int gueltig;

  CalPrimaryVariable(heat_conductivity_pcs_name_vector);

  switch(heat_conductivity_model){
    case 0: // rho = f(x)      
      heat_conductivity = GetCurveValue(fct_number,0,primary_variable[0],&gueltig);
      break;
    case 1: // c = const
      heat_conductivity = heat_conductivity;
      break;
	case 2:
	  heat_conductivity = MATCalcHeatConductivityMethod2(primary_variable[0],primary_variable[1], primary_variable[2]);
	  break;
  }
  return heat_conductivity;
}

/**************************************************************************
FEMLib-Method:
Task: Master calc function
Programing:
08/2004 OK MFP implementation based on MATCalcFluidHeatCapacity (OK)
last modification:
**************************************************************************/
double MFPCalcFluidsHeatConductivity(long index,double*gp,double theta, CFiniteElementStd* assem)
{
  double saturation_phase;
  double heat_conductivity_fluids=0.0;
  int nidx0,nidx1;
  bool New = false; // To be removed. WW
  if(fem_msh_vector.size()>0) New = true;

  //--------------------------------------------------------------------
  //----------------------------------------------------------------------
  CFluidProperties *m_mfp = NULL;
  int no_fluids =(int)mfp_vector.size();
  //YD-----------
  CRFProcess* m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
	if(m_pcs->pcs_type_name.find("RICHARDS_FLOW"))no_fluids =1;
  }
  //YD-----------
  switch(no_fluids){
    case 1:
      m_mfp = mfp_vector[0];
      heat_conductivity_fluids = m_mfp->HeatConductivity();
      break;
    case 2:
      m_mfp = mfp_vector[0];
      if(New) //WW
	  {
         nidx0 = m_pcs->GetNodeValueIndex("SATURATION1");
         nidx1 = nidx0+1;
		 saturation_phase = (1.-theta)*assem->interpolate(nidx0,m_pcs)
                                + theta*assem->interpolate(nidx1,m_pcs);

	  }
	  else
	  {
         nidx0 = PCSGetNODValueIndex("SATURATION1",0);
         nidx1 = PCSGetNODValueIndex("SATURATION1",1);	  
         saturation_phase = (1.-theta)*InterpolValue(index,nidx0,gp[0],gp[1],gp[2]) \
                       + theta*InterpolValue(index,nidx1,gp[0],gp[1],gp[2]);
	  }
      heat_conductivity_fluids = saturation_phase 
                               * m_mfp->HeatConductivity();
      m_mfp = mfp_vector[1];
      heat_conductivity_fluids += (1.0-saturation_phase )
                               * m_mfp->HeatConductivity();
      break;
    default:
      cout << "Error in MFPCalcFluidsHeatConductivity: no fluid phase data" << endl;
  }
  return heat_conductivity_fluids;
}

////////////////////////////////////////////////////////////////////////////
// Fluid phase change properties

/**************************************************************************
FEMLib-Method:
Task: Vapour pressure from table
Programing:
03/2002 OK/JdJ Implementation
last modification:
**************************************************************************/
double MFPCalcVapourPressure(double temperature)
{
  double temperature_F;
  double pressure;
  double quality;
  double weight;
  double enthalpy;
  double entropy;
  double saturation_temperature;
  double saturation_pressure;
  double degrees_superheat;
  double degrees_subcooling;
  double viscosity;
  double critical_velocity;
  int action=0;
  double pressure_vapour;

/*
  double vapour_pressure;
  double vapour_enthalpy = 2258.0; kJ/kg
  double potenz;
  potenz = ((1./temperature_ref)-(1./(*temperature))) * \
                  ((vapour_enthalpy*comp_mol_mass_water)/gas_constant);
  vapour_pressure = pressure_ref * exp(potenz);
*/
    pressure = 1.e-3; /*Vorgabe eines vernEftigen Wertes*/
    pressure *= PA2PSI; /* Umrechnung Pa in psia */
    temperature -= 273.15; /* Kelvin -> Celsius */
    temperature_F = temperature*1.8+32.; /* Umrechnung Celsius in Fahrenheit */
    steam67 (&temperature_F,
           &pressure,
           &quality,
           &weight,
           &enthalpy,
           &entropy,
           &saturation_temperature,
           &saturation_pressure,
           &degrees_superheat,
           &degrees_subcooling,
           &viscosity,
           &critical_velocity,
           action);
    pressure_vapour = saturation_pressure * PSI2PA; /* Umrechnung psia in Pa */
    /*
density_vapour = 0.062427962/weight; Dichte in kg/m^3
enthalpy_vapour = enthalpy * 1055. / 0.454; Umrechnung von btu/lbm in J/kg
    */
  return pressure_vapour;
}

/**************************************************************************
FEMLib-Method:
Task: Get phase and species related enthalpies
Programing:
03/2002 OK/JdJ Implementation
08/2004 OK MFP implementation
last modification:
**************************************************************************/
double CFluidProperties::Enthalpy(int comp,double temperature)
{
  double temperature_F;
  double pressure;
  double quality;
  double weight;
  double enthalpy=0.0;
  double entropy;
  double saturation_temperature;
  double saturation_pressure;
  double degrees_superheat;
  double degrees_subcooling;
  double viscosity;
  double critical_velocity;
  int action=0;
  
    if((phase==0)&&(comp==0)) {
      enthalpy = 733.0*temperature + (GAS_CONSTANT*(temperature+0.0))/COMP_MOL_MASS_AIR;
    }
    else if((phase==0)&&(comp==1)) { /* h_w^g: water species in gaseous phase */
      pressure = 1.e-3; /*Vorgabe eines vernünftigen Wertes */
      pressure *= PA2PSI; /* Umrechnung Pa in psia */
      temperature -= 273.15; /* Kelvin -> Celsius */
      temperature_F = temperature*1.8+32.; /* Umrechnung Celsius in Fahrenheit*/
      steam67 (&temperature_F,
             &pressure,
             &quality,
             &weight,
             &enthalpy,
             &entropy,
             &saturation_temperature,
             &saturation_pressure,
             &degrees_superheat,
             &degrees_subcooling,
             &viscosity,
             &critical_velocity,
             action);
      enthalpy = enthalpy * 1055. / 0.454; /* Umrechnung von btu/lbm in J/kg */
    }
    else if((phase==1)&&(comp==0)) { /* h_a^l: air species in liquid phase */
    }
    else if((phase==1)&&(comp==1)) { /* h_w^l: water species in liquid phase */
    }
  return enthalpy;
}

/**************************************************************************
FEMLib-Method:
Task: Get phase and species related enthalpies
Programing:
03/2002 OK/JdJ Implementation
08/2004 OK MFP implementation
last modification:
**************************************************************************/
double CFluidProperties::EnthalpyPhase(long number,int comp,double*gp,double theta)
{
  double temperature;
  double enthalpy=0.0;
  double mass_fraction_air,enthalpy_air,mass_fraction_water,enthalpy_water;
  int nidx0,nidx1;

  nidx0 = PCSGetNODValueIndex("TEMPERATURE1",0);
  nidx1 = PCSGetNODValueIndex("TEMPERATURE1",1);
  temperature = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
              + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);

  if(phase==0) {
   if(comp<0) {
    comp=0;
    mass_fraction_air = MassFraction(number,comp,gp,theta);
    comp=1;
    mass_fraction_water = MassFraction(number,comp,gp,theta);
    comp=0;
    enthalpy_air = Enthalpy(comp,temperature);
    comp=1;
    enthalpy_water = Enthalpy(comp,temperature);

        enthalpy = mass_fraction_air * enthalpy_air + \
               mass_fraction_water * enthalpy_water;
   }
   else if (comp>=0)
     enthalpy = Enthalpy(comp,temperature);
  }
  else if(phase==1) {
    enthalpy = SpecificHeatCapacity() * temperature;    //
  }

  return enthalpy;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Calculate Henry constant
Programing:
02/2002 JdJ First implementation
04/2003 JdJ temperature conversion from celsius to kelvin
last modification:
**************************************************************************/
double MFPCalcHenryConstant(double temperature)
{
  double HenryConstant;
  HenryConstant =  (0.8942 +1.47*exp(-0.04394*(temperature-273.14)))*0.0000000001;
  return HenryConstant;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Calculate mass fractions
   of gas phase    X^g_a, X^g_w according to Claudius-Clapeyron law
   of liquid phase X^l_a, X^l_w according to Henry law
Programing:
01/2002 OK/JdJ First implementation
03/2002 JdJ Gas phase in argument for pressure in mass fraction.
04/2003 JdJ Dichteberechnung (setzt voraus, das Phase 0 gas ist).
04/2003 JdJ Druckberechnung (setzt voraus, das Phase 0 gas ist).
04/2004 JdJ Bugfix Gauss Points 
08/2004 OK MFP implementation
last modification:
**************************************************************************/
double CFluidProperties::MassFraction(long number,int comp,double*gp,double theta,CFiniteElementStd* assem)
{
  double mass_fraction=0.0;
  double mass_fraction_air_in_gas,mass_fraction_air_in_liquid;
  double gas_density=0.0;
  double vapour_pressure;
  double temperature;
  double henry_constant;
  int nidx0,nidx1;
  double gas_pressure;
  /*--------------------------------------------------------------------------*/
  /* Get and calc independent variables */
  nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
  nidx1 = PCSGetNODValueIndex("PRESSURE1",1);
  if(mode==0){ // Gauss point values
    gas_pressure = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
                        + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);
  }
  else{ // Node values
    gas_pressure = (1.-theta)*GetNodeVal(number,nidx0) \
                        + theta*GetNodeVal(number,nidx1);
  }
  nidx0 = PCSGetNODValueIndex("TEMPERATURE1",0);
  nidx1 = PCSGetNODValueIndex("TEMPERATURE1",1);
  if(mode==0){ // Gauss point values
    temperature = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
                + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);
  }
  else{ // Node values
    temperature = (1.-theta)*GetNodeVal(number,nidx0) \
                + theta*GetNodeVal(number,nidx1);
  }
  gas_density = Density();
  vapour_pressure = MFPCalcVapourPressure(temperature);
  /*--------------------------------------------------------------------------*/
  /* Calc mass fractions */
  switch (phase) {
    case 0: /* gas phase */
            mass_fraction_air_in_gas = \
                           ((gas_pressure-vapour_pressure)*COMP_MOL_MASS_AIR) \
                         / (GAS_CONSTANT*(temperature+0.0)*gas_density);
            mass_fraction_air_in_gas = MRange(0.0,mass_fraction_air_in_gas,1.0);
            if(comp==0) { /* air specie */ 
              mass_fraction = mass_fraction_air_in_gas;
            }
            if(comp==1) { /* water specie */
              mass_fraction = 1.0 - mass_fraction_air_in_gas;
            }
            break;
    case 1: /* liquid phase */
            henry_constant = MFPCalcHenryConstant(temperature);
            mass_fraction_air_in_liquid = \
                           COMP_MOL_MASS_AIR / (COMP_MOL_MASS_AIR \
                         - COMP_MOL_MASS_WATER * (1.0-1.0/(henry_constant*(gas_pressure-vapour_pressure))));
            mass_fraction_air_in_liquid = MRange(0.0,mass_fraction_air_in_liquid,1.0);
            if(comp==0) { /* air specie */ 
              mass_fraction = mass_fraction_air_in_liquid;
            }
            if(comp==1) { /* water specie X_w^l = 1-X_a^l */
              mass_fraction = 1.0 - mass_fraction_air_in_liquid;
            }
            break;
  }
  /*--------------------------------------------------------------------------*/
  return mass_fraction;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Calculate Henry constant
Programing:
02/2002 OK/JdJ Implementation
08/2004 OK MFP implementation
last modification:
**************************************************************************/
double CFluidProperties::InternalEnergy(long number,double*gp,double theta)
{
  double energy=0.0;
  int nidx0,nidx1;
  double temperature,pressure;

  nidx0 = PCSGetNODValueIndex("TEMPERATURE1",0);
  nidx1 = PCSGetNODValueIndex("TEMPERATURE1",1);
  temperature = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
              + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);
  energy = SpecificHeatCapacity() * temperature;  //YD
  //energy = HeatCapacity(temperature,m_ele_fem_std) * temperature;

  //if(fluid_name.find("GAS")!=string::npos){
  if(phase==0){
    nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
    nidx1 = PCSGetNODValueIndex("PRESSURE1",1);
    pressure = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
              + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);
    energy += pressure / Density();  //YD
  }
  return energy;
}

/**************************************************************************
FEMLib-Method:
Task: 
   Calculate Henry constant
   temperature in Kelvin
Programing:
01/2003 OK/JdJ Implementation
08/2004 OK MFP implementation
last modification:
**************************************************************************/
double CFluidProperties::DensityTemperatureDependence(long number,int comp,double*gp,double theta)
{ 
  double vapour_pressure;
  double dvapour_pressure_dT;
  double drho_dT;
  double temperature;
  int nidx0,nidx1;
  //----------------------------------------------------------------------
  // State functions
  nidx0 = PCSGetNODValueIndex("TEMPERATURE1",0);
  nidx1 = PCSGetNODValueIndex("TEMPERATURE1",1);
  if(mode==0){ // Gauss point values
    temperature = (1.-theta)*InterpolValue(number,nidx0,gp[0],gp[1],gp[2]) \
                + theta*InterpolValue(number,nidx1,gp[0],gp[1],gp[2]);
  }
  else{ // Node values
    temperature = (1.-theta)*GetNodeVal(number,nidx0) \
                + theta*GetNodeVal(number,nidx1);
  }
  //----------------------------------------------------------------------
  // Vapour
  vapour_pressure = MFPCalcVapourPressure(temperature);
  dvapour_pressure_dT = COMP_MOL_MASS_WATER * Enthalpy(comp,temperature) \
                      / (GAS_CONSTANT*temperature*temperature) \
                      * vapour_pressure;

  drho_dT = -1.0 * COMP_MOL_MASS_WATER / (GAS_CONSTANT*temperature) \
          * (dvapour_pressure_dT - vapour_pressure/temperature);
  //----------------------------------------------------------------------
  // Test
  if((phase>0)||(comp==0)) {
    DisplayMsgLn("MATCalcFluidDensityTemperatureDependence: Incorrect use of function");
    abort();
  }
  return drho_dT;
}


/**************************************************************************
FEMLib-Method:
Task: 

Programing:

last modification:
**************************************************************************/
double CFluidProperties::LiquidViscosity_CMCD(double Press,double TempK,double C)
{
C = C;
	/*CMcD variables for 20 ALR*/
	double A1,A2,A3,A4,A5,A6,A7,A8; /*constants*/
	double TempC,TempF, Pbar,Salinity; /* Temperature [K], Temperature [F], Pressure [bar]*/
	double my_Zero,PsatBar, PsatKPa; /*my pure water, my saline water, Saturation pressure [bar], Saturation pressure [KPa]*/
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,sum8, exponent; /*intermediate values*/
	/*CMcD end variables for 20 ALR*/
	 /* //Prepared for introduction of solute transport in PCS version
	    //Average Concentration 
	    comp=0; // nur für Einkomponenten-Systeme 
		timelevel=1;
		concentration_average = 0.0;
		for (i = 0; i < count_nodes; i++)
		  concentration_average += GetNodeVal(element_nodes[i], c_idx);
		concentration_average /= count_nodes;
		fp->salinity=fp->rho_0+concentration_average*fp->drho_dC;
                */
               //Link to function from ALR
		Salinity=C_0/1000.;
		/***constant values*******/   
		A1 = -7.419242;
		A2 = -0.29721;
		A3 = -0.1155286;
		A4 = -0.008685635;
		A5 = 0.001094098;
		A6 = 0.00439993;
		A7 = 0.002520658;
		A8 = 0.0005218684;

		/*Unit conversions*/
		TempC = TempK-273.15;
		TempF = TempC*1.8 + 32.0;
		Pbar = Press/100000.0;
		/*end of units conversion*/

		/*Calculation of the saturation pressure*/
		sum1=pow((0.65-0.01*TempK),0)*A1;
		sum2=pow((0.65-0.01*TempK),1)*A2;
		sum3=pow((0.65-0.01*TempK),2)*A3;
		sum4=pow((0.65-0.01*TempK),3)*A4;
		sum5=pow((0.65-0.01*TempK),4)*A5;
		sum6=pow((0.65-0.01*TempK),5)*A6;
		sum7=pow((0.65-0.01*TempK),6)*A7;
		sum8=pow((0.65-0.01*TempK),7)*A8;
	
		exponent = sum1+sum2+sum3+sum4+sum5+sum6+sum7+sum8; /*intermediate value*/
		exponent= exponent*(374.136-TempC)/TempK; /*intermediate value*/

		PsatKPa = exp(exponent)*22088; /*saturation pressure in kPa*/
		PsatBar = PsatKPa/(1000*100000); /*Saturation pressure in bar*/

		/*Viscosity of pure water in Pa-S*/
		my_Zero = 243.18e-7 * (pow(10.,(247.8/(TempK-140)))) * (1+(Pbar-PsatBar)*1.0467e-6 * (TempK-305));
	
		/*Viscosity of saline water in Pa-S*/
		viscosity = my_Zero * (1-0.00187* (pow(Salinity,0.5)) + 0.000218* (pow(Salinity,2.5))+(pow(TempF,0.5)-0.0135*TempF)*(0.00276*Salinity-0.000344* (pow(Salinity,1.5))));
   return viscosity;
 }


/**************************************************************************/
/* ROCKFLOW - Function: MATCalcFluidHeatConductivityMethod2
                                                                          */
/* Task:
   Calculate heat conductivity of all fluids
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
   I double temperature
                                                                          */
/* Return:
   Value of heat conductivity of fluids as a function of (p,C)
                                                                          */
/* Programming:
   09/2003   CMCD ARL   Implementation
   08/2004   CMCD inclusion in GeoSys v. 4.
                                                                             */
/**************************************************************************/
double CFluidProperties::MATCalcHeatConductivityMethod2(double Press, double TempK, double Conc)
{
Conc = Conc;
		int i, j;
		double TauTC, PiiTC, Nabla, Delta, Nabla0, Nabla1, Nabla2;
		double heat_conductivity, Rho, temperature_average, pressure_average, viscosity;
		double Rhostar, TstarTC, Lambdastar, Pstar1;
		double nZero[4];
		double n[5][6];
		double A1,A2,A3,A4,A5,A6,A7,A8; /*constants*/
		double TempC, TempF, Pbar, Salinity; /* Temperature [K], Temperature [F], Pressure [bar]*/
		double my_Zero,PsatBar, PsatKPa; /*my pure water, my saline water, Saturation pressure [bar], Saturation pressure [KPa]*/
		double sum1,sum2,sum3,sum4,sum5,sum6,sum7,sum8, exponent; /*intermediate values*/

/*************************************************************************************************/
/*************************************************************************************************/
/*************************Partial derivatives calculation*****************************************/
		double GammaPi, GammaPiTau, GammaPiPi, Pii, Tau,  GazConst;
		double LGamma[35];
		double JGamma[35];
		double nGamma[35];
		double Tstar, Pstar;
		double TstarTilda, PstarTilda, RhostarTilda;
		double First_derivative, Second_derivative;

		
		pressure_average = Press;
		temperature_average = TempK;
		Salinity = C_0;
		Tstar = 1386;
		Pstar = 16.53e6; // MPa
		GazConst = 0.461526e3; //!!!!Given by equation (2.1)
		TstarTilda = 647.226;
		PstarTilda = 22.115e6;
		RhostarTilda = 317.763;
		Lambdastar = 0.4945;
		Rhostar = 317.763;
		TstarTC = 647.226;
		Pstar1 = 22.115e-6;


		/*BEGIN: reduced dimensions*/
		TauTC = TstarTC / temperature_average;
		//Delta = Rho / Rhostar;
		PiiTC = pressure_average / Pstar1;
		/*END: reduced dimensions*/

		
		

	 nGamma[1] = 0.14632971213167;
	 nGamma[2] = -0.84548187169114;
	 nGamma[3] = -0.37563603672040e1;
	 nGamma[4] = 0.33855169168385e1;
	 nGamma[5] = -0.95791963387872;
	 nGamma[6] = 0.15772038513228;
	 nGamma[7] = -0.16616417199501e-1;
	 nGamma[8] = 0.81214629983568e-3;
	 nGamma[9] = 0.28319080123804e-3;
	 nGamma[10] = -0.60706301565874e-3;
	 nGamma[11] = -0.18990068218419e-1;
	 nGamma[12] = -0.32529748770505e-1;
	 nGamma[13] = -0.21841717175414e-1;
	 nGamma[14] = -0.52838357969930e-4;
	 nGamma[15] = -0.47184321073267e-3;
	 nGamma[16] = -0.30001780793026e-3;
	 nGamma[17] = 0.47661393906987e-4;
	 nGamma[18] = -0.44141845330846e-5;
	 nGamma[19] = -0.72694996297594e-15;
	 nGamma[20] = -0.31679644845054e-4;
	 nGamma[21] = -0.28270797985312e-5;
	 nGamma[22] = -0.85205128120103e-9;
	 nGamma[23] = -0.22425281908000e-5;
	 nGamma[24] = -0.65171222895601e-6;
	 nGamma[25] = -0.14341729937924e-12;
	 nGamma[26] = -0.40516996860117e-6;
	 nGamma[27] = -0.12734301741641e-8;
	 nGamma[28] = -0.17424871230634e-9;
	 nGamma[29] = -0.68762131295531e-18;
	 nGamma[30] = 0.14478307828521e-19;
	 nGamma[31] = 0.26335781662795e-22;
	 nGamma[32] = -0.11947622640071e-22;
	 nGamma[33] = 0.18228094581404e-23;
	 nGamma[34] = -0.93537087292458e-25;



	 LGamma[1] =0.;
	 LGamma[2] =0.;
	 LGamma[3] =0.;
	 LGamma[4] =0.;
	 LGamma[5] =0.;
	 LGamma[6] =0.;
	 LGamma[7] =0.;
	 LGamma[8] =0.;
	 LGamma[9] =1.;
	 LGamma[10] =1.;
	 LGamma[11] =1.;
	 LGamma[12] =1.;
	 LGamma[13] =1.;
	 LGamma[14] =1.;
	 LGamma[15] =2.;
	 LGamma[16] =2.;
	 LGamma[17] =2.;
	 LGamma[18] =2.;
	 LGamma[19] =2.;
	 LGamma[20] =3.;
	 LGamma[21] =3.;
	 LGamma[22] =3.;
	 LGamma[23] =4.;
	 LGamma[24] =4.;
	 LGamma[25] =4.;
	 LGamma[26] =5.;
	 LGamma[27] =8.;
	 LGamma[28] =8.;
	 LGamma[29] =21.;
	 LGamma[30] =23.;
	 LGamma[31] =29.;
	 LGamma[32] =30.;
	 LGamma[33] =31.;
	 LGamma[34] =32.;

	 JGamma[1] =-2.;
	 JGamma[2] =-1.;
	 JGamma[3] =0.;
	 JGamma[4] =1.;
	 JGamma[5] =2.;
	 JGamma[6] =3.;
	 JGamma[7] =4.;
	 JGamma[8] =5.;
	 JGamma[9] =-9.;
	 JGamma[10] =-7.;
	 JGamma[11] =-1.;
	 JGamma[12] =0.;
	 JGamma[13] =1.;
	 JGamma[14] =3.;
	 JGamma[15] =-3.;
	 JGamma[16] =0.;
	 JGamma[17] =1.;
	 JGamma[18] =3.;
	 JGamma[19] =17.;
	 JGamma[20] =-4.;
	 JGamma[21] =0.;
	 JGamma[22] =6.;
	 JGamma[23] =-5.;
	 JGamma[24] =-2.;
	 JGamma[25] =10.;
	 JGamma[26] =-8.;
	 JGamma[27] =-11.;
	 JGamma[28] =-6.;
	 JGamma[29] =-29.;
	 JGamma[30] =-31.;
	 JGamma[31] =-38.;
	 JGamma[32] = -39.;
	 JGamma[33] =-40.;
	 JGamma[34] =-41.;

	
	 

	Pii = pressure_average / Pstar;
	Tau = Tstar / temperature_average;


			/*BEGIN:Calculation of GammaPi*/
			GammaPi = 0;


			for (i=1; i<35; i++)
			{
				GammaPi = GammaPi - (nGamma[i]) * (LGamma[i]) * (pow((7.1-Pii),(LGamma[i] -1))) * (pow((Tau-1.222),JGamma[i]));
			}
			/*END: Calculation of GammaPi*/

			/*BEGIN:Calculation of GammaPiTau*/
			GammaPiTau = 0;
			for (i=1; i<35; i++)
			{
				GammaPiTau = GammaPiTau - (nGamma[i]) * (LGamma[i]) * (pow((7.1-Pii),(LGamma[i] -1))) * (JGamma[i]) * (pow((Tau-1.222),(JGamma[i]-1)));
			}
			/*END: Calculation of GammaPiTau*/

			/*BEGIN:Calculation of GammaPiPi*/
			GammaPiPi = 0;
			for (i=1; i<=34; i++)
			{
				GammaPiPi = GammaPiPi + (nGamma[i]) * (LGamma[i]) * (LGamma[i] -1) * (pow((7.1-Pii),(LGamma[i] -2))) * (pow((Tau-1.222),(JGamma[i])));
			}
			/*END: Calculation of GammaPiPi*/


			/*BEGIN:Calculation of derivative*/
			First_derivative = ((TstarTilda) * (Pstar) * ((GammaPiTau)*(Tstar) - (GammaPi) * (temperature_average))) / (PstarTilda * pow(temperature_average,2) * GammaPiPi),
			Second_derivative = ((-1) * (PstarTilda) * (GammaPiPi) ) / ( (RhostarTilda) * (temperature_average) * (GazConst) * (pow(GammaPi,2)));
			/*End:Calculation of derivative*/


			/*BEGIN: Calculation of density*/
			Rho = pressure_average / (GazConst * (temperature_average) * (Pii) * (GammaPi));
			/*END: Calculation of density*/


/*************************Partial derivatives calculation*****************************************/
/*************************************************************************************************/
/*************************************************************************************************/


		

		
		/*BEGIN: Constant values*/

		Lambdastar = 0.4945;
		Rhostar = 317.763;
		TstarTC = 647.226;
		Pstar1 = 22.115e6;


		nZero[0] = 0.1e1;
		nZero[1] = 0.6978267e1;
		nZero[2] = 0.2599096e1;
		nZero[3] = -0.998254;
		
		n[0][0] = 0.13293046e1;
		n[0][1] = -0.40452437;
		n[0][2] = 0.24409490;
		n[0][3] = 0.18660751e-1;
		n[0][4] = -0.12961068;
		n[0][5] = 0.44809953e-1;

		n[1][0] = 0.17018363e1;
		n[1][1] = -0.22156845e1;
		n[1][2] = 0.16511057e1;
		n[1][3] = -0.76736002;
		n[1][4] = 0.37283344;
		n[1][5] = -0.11203160;

		n[2][0] = 0.52246158e1;
		n[2][1] = -0.10124111e2;
		n[2][2] = 0.49874687e1;
		n[2][3] = -0.27297694;
		n[2][4] = -0.43083393;
		n[2][5] = 0.13333849;

		n[3][0] = 0.87127675e1;
		n[3][1] = -0.95000611e1;
		n[3][2] = 0.43786606e1;
		n[3][3] = -0.91783782;
		n[3][4] = 0;
		n[3][5] = 0;

		n[4][0] = -0.18525999e1;
		n[4][1] = 0.93404690;
		n[4][2] = 0;
		n[4][3] = 0;
		n[4][4] = 0;
		n[4][5] = 0;
		/*END: Constant values*/
	
		/*BEGIN: reduced dimensions*/
		TauTC = TstarTC / temperature_average;
		Delta = Rho / Rhostar;
		PiiTC = pressure_average / Pstar1;
		/*END: reduced dimensions*/

		/*BEGIN: Nabla0*/
		Nabla0 = 0;
		for (i=0; i<=3;i++)
		{
			Nabla0 = Nabla0 + (nZero[i]) * (pow(TauTC,i));	
		}
		
		Nabla0 = Nabla0 * (pow(TauTC, 0.5));
		Nabla0 = 1 / Nabla0;		
		/*END: Nabla0*/


		/*BEGIN: Nabla1*/
		Nabla1 = 0;
		for (i=0; i<=4;i++)
		{
			for (j=0; j<=5;j++)
			{
				Nabla1 = Nabla1 + (n[i][j]) * (pow((TauTC-1),i)) * (pow((Delta-1),j));
			}
					
		}
		Nabla1 = Delta * (Nabla1);
		Nabla1 = exp(Nabla1);
		/*END: Nabla1*/
	
		/*Calculate Viscosity*/
		/*Link to function from ALR*/
		

		TempK=temperature_average;
		Press=pressure_average;
		Salinity=C_0/1000.;
        
		
		/***constant values*******/   
		A1 = -7.419242;
		A2 = -0.29721;
		A3 = -0.1155286;
		A4 = -0.008685635;
		A5 = 0.001094098;
		A6 = 0.00439993;
		A7 = 0.002520658;
		A8 = 0.0005218684;

		/*Unit conversions*/
		TempC = TempK-273.15;
		TempF = TempC*1.8 + 32.0;
		if (TempF < 0.0){
			TempF = 0.0;
			}
		Pbar = Press/100000.0;
		/*end of units conversion*/

		/*Calculation of the saturation pressure*/
		sum1=pow((0.65-0.01*TempK),0)*A1;
		sum2=pow((0.65-0.01*TempK),1)*A2;
		sum3=pow((0.65-0.01*TempK),2)*A3;
		sum4=pow((0.65-0.01*TempK),3)*A4;
		sum5=pow((0.65-0.01*TempK),4)*A5;
		sum6=pow((0.65-0.01*TempK),5)*A6;
		sum7=pow((0.65-0.01*TempK),6)*A7;
		sum8=pow((0.65-0.01*TempK),7)*A8;
	
		exponent = sum1+sum2+sum3+sum4+sum5+sum6+sum7+sum8; /*intermediate value*/
		exponent= exponent*(374.136-TempC)/TempK; /*intermediate value*/

		PsatKPa = exp(exponent)*22088; /*saturation pressure in kPa*/
		PsatBar = PsatKPa/(1000*100000); /*Saturation pressure in bar*/

		/*Viscosity of pure water in Pa-S*/
		my_Zero = 243.18e-7 * (pow(10.,(247.8/(TempK-140)))) * (1+(Pbar-PsatBar)*1.0467e-6 * (TempK-305));
	
		/*Viscosity of saline water in Pa-S*/
		viscosity = my_Zero * (1-0.00187* (pow(Salinity,0.5)) + 0.000218* (pow(Salinity,2.5))+(pow(TempF,0.5)-0.0135*TempF)*(0.00276*Salinity-0.000344* (pow(Salinity,1.5))));
		
		/* End of viscosity function*/


		/*BEGIN: Nabla2*/
		Nabla2 = 0.0013848 / ((viscosity)/55.071e-6) * (pow(((TauTC)*(Delta)),(-2))) * (pow(First_derivative,2)) * (pow((Delta * (Second_derivative)),0.4678)) * (pow(Delta,0.5)) * exp(-18.66 * (pow((1/TauTC-1),2)) - (pow(Delta-1,4)));
		/*END: Nabla2*/
	
		/*BEGIN: Nabla => heat_conductivity*/
		Nabla = Nabla0 * (Nabla1) + Nabla2;
		heat_conductivity = Nabla * (Lambdastar);
		/*END: Nabla => Lambda*/
		
	
		return heat_conductivity;	
	}

/**************************************************************************
ROCKFLOW - Funktion: 
Task: 
Programming:
 08/2005 WW Implementation 
**************************************************************************/
double CFluidProperties::vaporDensity(const double T_abs) 
{
    return 1.0e-3*exp(19.81-4975.9/T_abs);
}
double CFluidProperties::vaporDensity_derivative(const double T_abs) 
{
    return 4.9759*exp(19.81-4975.9/T_abs)/(T_abs*T_abs);
}


	/**************************************************************************/
/* ROCKFLOW - Function: MATCalcFluidHeatCapacity
                                                                          */
/* Task:
   Calculate heat capacity of all fluids
                                                                          */
/* Parameter: (I: Input; R: Return; X: Both)
   I double temperature
                                                                          */
/* Return:
   Value of heat capacity of all fluids
                                                                          */
/* Programming:
   09/2003   CMCD ARL   Implementation
   09/2004   CMCD included in Geosys ver. 4
   
                                                                          */
/**************************************************************************/
double CFluidProperties::MATCalcFluidHeatCapacityMethod2(double Press, double TempK, double Conc)
{ 
Conc = Conc;	
    double Pressurevar, Tau, pressure_average, temperature_average, Tstar, Pstar,GazConst;
    double GammaPi, GammaPiTau, GammaPiPi, GammaTauTau;
	double L[35],J[35],n[35];
    int i;
    double salinity;
	double Cp, Cv; 

	pressure_average = Press;
	temperature_average = TempK;
	
    salinity=C_0;

	Tstar = 1386;

	Pstar = 16.53e6; // MPa
	GazConst = 0.461526e3; //

	 n[0] = 0.0;
	 n[1] = 0.14632971213167;
	 n[2] = -0.84548187169114;
	 n[3] = -0.37563603672040e1;
	 n[4] = 0.33855169168385e1;
	 n[5] = -0.95791963387872;
	 n[6] = 0.15772038513228;
	 n[7] = -0.16616417199501e-1;
	 n[8] = 0.81214629983568e-3;
	 n[9] = 0.28319080123804e-3;
	 n[10] = -0.60706301565874e-3;
	 n[11] = -0.18990068218419e-1;
	 n[12] = -0.32529748770505e-1;
	 n[13] = -0.21841717175414e-1;
	 n[14] = -0.52838357969930e-4;
	 n[15] = -0.47184321073267e-3;
	 n[16] = -0.30001780793026e-3;
	 n[17] = 0.47661393906987e-4;
	 n[18] = -0.44141845330846e-5;
	 n[19] = -0.72694996297594e-15;
	 n[20] = -0.31679644845054e-4;
	 n[21] = -0.28270797985312e-5;
	 n[22] = -0.85205128120103e-9;
	 n[23] = -0.22425281908000e-5;
	 n[24] = -0.65171222895601e-6;
	 n[25] = -0.14341729937924e-12;
	 n[26] = -0.40516996860117e-6;
	 n[27] = -0.12734301741641e-8;
	 n[28] = -0.17424871230634e-9;
	 n[29] = -0.68762131295531e-18;
	 n[30] = 0.14478307828521e-19;
	 n[31] = 0.26335781662795e-22;
	 n[32] = -0.11947622640071e-22;
	 n[33] = 0.18228094581404e-23;
	 n[34] = -0.93537087292458e-25;



	 L[0] =0.;
	 L[1] =0.;
	 L[2] =0.;
	 L[3] =0.;
	 L[4] =0.;
	 L[5] =0.;
	 L[6] =0.;
	 L[7] =0.;
	 L[8] =0.;
	 L[9] =1.;
	 L[10] =1.;
	 L[11] =1.;
	 L[12] =1.;
	 L[13] =1.;
	 L[14] =1.;
	 L[15] =2.;
	 L[16] =2.;
	 L[17] =2.;
	 L[18] =2.;
	 L[19] =2.;
	 L[20] =3.;
	 L[21] =3.;
	 L[22] =3.;
	 L[23] =4.;
	 L[24] =4.;
	 L[25] =4.;
	 L[26] =5.;
	 L[27] =8.;
	 L[28] =8.;
	 L[29] =21.;
	 L[30] =23.;
	 L[31] =29.;
	 L[32] =30.;
	 L[33] =31.;
	 L[34] =32.;

	 J[0] =-2.;
	 J[1] =-2.;
	 J[2] =-1.;
	 J[3] =0.;
	 J[4] =1.;
	 J[5] =2.;
	 J[6] =3.;
	 J[7] =4.;
	 J[8] =5.;
	 J[9] =-9.;
	 J[10] =-7.;
	 J[11] =-1.;
	 J[12] =0.;
	 J[13] =1.;
	 J[14] =3.;
	 J[15] =-3.;
	 J[16] =0.;
	 J[17] =1.;
	 J[18] =3.;
	 J[19] =17.;
	 J[20] =-4.;
	 J[21] =0.;
	 J[22] =6.;
	 J[23] =-5.;
	 J[24] =-2.;
	 J[25] =10.;
	 J[26] =-8.;
	 J[27] =-11.;
	 J[28] =-6.;
	 J[29] =-29.;
	 J[30] =-31.;
	 J[31] =-38.;
	 J[32] = -39.;
	 J[33] =-40.;
	 J[34] =-41.;

		
	Pressurevar = pressure_average / Pstar;
	Tau = Tstar / temperature_average;



			/*BEGIN:Calculation of GammaPi*/
			GammaPi = 0;


			for (i=1; i<35; i++)
			{
				GammaPi = GammaPi - (n[i]) * (L[i]) * (pow((7.1-Pressurevar),(L[i] -1.))) * (pow((Tau-1.222),J[i]));
			}
			/*END: Calculation of GammaPi*/



			/*BEGIN:Calculation of GammaPiTau*/
			GammaPiTau = 0;
			for (i=1; i<35; i++)
			{
				GammaPiTau = GammaPiTau - (n[i]) * (L[i]) * (pow((7.1-Pressurevar),(L[i] -1.))) * (J[i]) * (pow((Tau-1.222),(J[i]-1.)));
			}
			/*END: Calculation of GammaPiTau*/


			/*BEGIN:Calculation of GammaTauTau*/
			GammaTauTau = 0;
			for (i=1; i<35; i++)
			{
				GammaTauTau = GammaTauTau + (n[i]) * (pow((7.1-Pressurevar),(L[i]))) * (J[i]) * (J[i] -1.) * (pow((Tau - 1.222),(J[i]-2)));
			}
			/*END: Calculation of GammaTauTau*/


			/*BEGIN:Calculation of GammaPiPi*/
			GammaPiPi = 0;
			for (i=1; i<35; i++)
			{
				GammaPiPi = GammaPiPi + (n[i]) * (L[i]) * (L[i] -1) * (pow((7.1-Pressurevar),(L[i] -2.))) * (pow((Tau-1.222),(J[i])));
			}
			/*END: Calculation of GammaPiPi*/


/*************************Partial derivatives calculation*****************************************/
/*************************************************************************************************/
/*************************************************************************************************/

			
			/*BEGIN: Fluid isobaric heat capacity*/
			Cp = - (pow(Tau,2))* (GammaTauTau) * (GazConst); 
			/*END: Fluid isobaric heat capacity*/


			/*BEGIN: Fluid isochoric heat capacity*/
			Cv = (- (pow(Tau,2))* (GammaTauTau) + pow(GammaPi - Tau * (GammaPiTau),2) / GammaPiPi) * GazConst; /* Cv is not used currently 9.2003*/
			/*BEGIN: Fluid isochoric heat capacity*/

			
		return Cp;	
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void MFPDelete()
{
  long i;
  int no_mfp =(int)mfp_vector.size();
  for(i=0;i<no_mfp;i++){
    delete mfp_vector[i];
  }
  mfp_vector.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
10/2005 OK/YD Implementation
**************************************************************************/
CFluidProperties* MFPGet(string name)
{
  CFluidProperties* m_mfp = NULL;
  for(int i=0;i<(int)mfp_vector.size();i++){
    m_mfp = mfp_vector[i];
    if(m_mfp->name.compare(name)==0)
      return m_mfp;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
10/2005 YD Calculate Enthalpy
**************************************************************************/
double CFluidProperties::CalcEnthalpy()
{
  //----------------------------------------------------------------------
  CalPrimaryVariable(enthalpy_pcs_name_vector);
  //----------------------------------------------------------------------
  double temperature = primary_variable[0];
  double val = 0.0;
  double T0_integrate = 0.0;
  double T1_integrate = 0.0;         
  double heat_capacity_all;
    
  switch(GetHeatCapacityModel()){
    case 5:
    MFPGet("LIQUID");
//------------PART 1--------------------------------
  T0_integrate = 273.15;
  T1_integrate = T_Latent1+273.15; 
  int npoint = 100;  //Gauss point
  double DT = (T1_integrate-T0_integrate)/npoint;
  for(int i=0;i<npoint;i++){
    temperature_buffer = T0_integrate+i*DT;
    heat_capacity_all = Fem_Ele_Std->FluidProp->SpecificHeatCapacity();
    temperature_buffer = T0_integrate+(i+1)*DT;
    heat_capacity_all += Fem_Ele_Std->FluidProp->SpecificHeatCapacity();
	val += 0.5*DT*heat_capacity_all;
  }
 
//------------PART 2--------------------------------
    npoint = 500;
    T0_integrate = T_Latent1+273.15;
    T1_integrate = temperature+273.15;
	DT = (T1_integrate-T0_integrate)/npoint;
	for(int i=0;i<npoint;i++){
    temperature_buffer = T0_integrate+i*DT;
    heat_capacity_all = Fem_Ele_Std->FluidProp->SpecificHeatCapacity();
    temperature_buffer = T0_integrate+(i+1)*DT;
    heat_capacity_all += Fem_Ele_Std->FluidProp->SpecificHeatCapacity();   
	val += 0.5*DT*heat_capacity_all;
	}
   break;
 //  case 5:
}
   return val;
}
