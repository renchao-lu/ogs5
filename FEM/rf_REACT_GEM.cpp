//-------------------------------------
// rf_REACT_GEM.cpp
// Haibing Shao 23.05.07
// haibing.shao@ufz.de
// GEM Reaction Package
// based on the PSI node-GEM source code
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------

#include "rf_REACT_GEM.h"
#include "node.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"

REACT_GEM::REACT_GEM(void)
{
   m_Node = new TNode();
   REACT_GEM::dch_input_file_path = "calcite-dch.dat";
   REACT_GEM::ipm_input_file_path = "calcite-ipm.dat";
   REACT_GEM::dbr_input_file_path = "calcite-dbr-0-0000.dat";
   REACT_GEM::init_input_file_path= "calcite-init.dat";

   nIC = 0;
   nDC = 0;
   nPH = 0;
   nPS = 0;
   nNodes = 0;
   initialized_flag = 0;
}

REACT_GEM::~REACT_GEM(void)
{
  delete m_Node;
  for (long i=0 ; i < nNodes ; i++)
  {
	  /*delete m_xDC[i];
	  delete m_gam[i];
	  delete m_xPH[i];
	  delete m_vPS[i];
	  delete m_mPS[i];
	  delete m_bPS[i];
	  delete m_aPH[i];
	  delete m_xPA[i];
	  delete m_dul[i];
	  delete m_dll[i];
	  delete m_bIC[i];
	  delete m_rMB[i];
	  delete m_uIC[i];  */
  }

  free ( m_xDC);
  free ( m_gam);
  free ( m_xPH);
  free ( m_vPS);
  free ( m_mPS);
  free ( m_bPS);
  free ( m_aPH);
  free ( m_xPA);
  free ( m_dul);
  free ( m_dll);
  free ( m_bIC);
  free ( m_rMB);
  free ( m_uIC); 

  m_NodeHandle.clear();
  m_NodeStatusCH.clear();
  m_IterDone.clear();
  m_T.clear();
  m_P.clear();
  m_Vs.clear();
  m_Ms.clear();
  m_Gs.clear();
  m_Hs.clear();
  m_IC.clear();
  m_pH.clear();
  m_pe.clear();
  m_Eh.clear();

/*  m_xDC.clear();
  m_gam.clear();
  m_xPH.clear();
  m_aPH.clear();
  m_vPS.clear();
  m_mPS.clear();
  m_bPS.clear();
  m_xPA.clear();
  m_dul.clear();
  m_dll.clear();
  m_bIC.clear();
  m_rMB.clear();
  m_uIC.clear(); */

}

// Initialization of the GEM TNode Class
short REACT_GEM::Init_Nodes(string Project_path)
{

   // Creating TNode structure accessible trough node pointer
   // Here we read the files needed as input for initializing GEMIPM2K
   // The easiest way to prepare them is to use GEMS-PSI code (GEM2MT module)
   if ( Load_Init_File(Project_path))
   {
	   // The init file is successfully loaded
	   // Getting direct access to DataCH structure in GEMIPM2K memory
	   dCH = m_Node->pCSD();
	   if( !dCH  )
		   return 3;

	   // Getting direct access to work node DATABR structure which
	   // exchanges data between GEMIPM and FMT parts
	   dBR = m_Node->pCNode();
	   if( !dBR  )
		   return 4;
	 
	   // Extracting data bridge array sizes
	   nIC = dCH->nICb;//Num of Independent components
	   nDC = dCH->nDCb;//Num of Chemical species in the reactive part
	   nPH = dCH->nPHb;//Num of Phases
	   nPS = dCH->nPSb;//Num of multicomponent phases; ASSERT(nPS < nPH)

	   // Get number of Nodes
	   nNodes = REACT_GEM::GetNodeNumber_MT();

	   // Allocating work memory for FMT part (here only chemical variables)
	   for (long in = 0; in < nNodes ; in++)
	   {
		   short tempCH,tempHD,tempID;
		   m_NodeHandle.push_back(tempHD);
		   m_NodeStatusCH.push_back(tempCH);
		   m_IterDone.push_back(tempID);

		   double tempT, tempP, tempVs, tempMs, tempGs, tempHs, tempIC, tempPH, tempPE, tempEh; 
		   m_T.push_back(tempT);
		   m_P.push_back(tempP);
		   m_Vs.push_back(tempVs);
		   m_Ms.push_back(tempMs);
		   m_Gs.push_back(tempGs);
		   m_Hs.push_back(tempHs);
		   m_IC.push_back(tempIC);
		   m_pH.push_back(tempPH);
		   m_pe.push_back(tempPE);
		   m_Eh.push_back(tempEh);
	   }

	   m_bIC = (double*)malloc( nNodes*nIC*sizeof(double) );
	   m_rMB = (double*)malloc( nNodes*nIC*sizeof(double) );
	   m_uIC = (double*)malloc( nNodes*nIC*sizeof(double) );
	   m_xDC = (double*)malloc( nNodes*nDC*sizeof(double) );
	   m_gam = (double*)malloc( nNodes*nDC*sizeof(double) );
	   m_dul = (double*)malloc( nNodes*nDC*sizeof(double) );
	   m_dll = (double*)malloc( nNodes*nDC*sizeof(double) );
	   m_aPH = (double*)malloc( nNodes*nPH*sizeof(double) );
	   m_xPH = (double*)malloc( nNodes*nPH*sizeof(double) );
	   m_vPS = (double*)malloc( nNodes*nPS*sizeof(double) );
	   m_mPS = (double*)malloc( nNodes*nPS*sizeof(double) );
	   m_bPS = (double*)malloc( nNodes*nIC*nPS*sizeof(double) );
	   m_xPA = (double*)malloc( nNodes*nPS*sizeof(double) );

	   return 0;//successed
   }
   else
   {
      cout << "Error loading initial files to GEMS" <<endl;
	  return 1;   
   };
}

short REACT_GEM::Init_RUN()
{
long StatusCheck = 0;

for (long in = 0; in < nNodes ; in++)
{
	// Order GEM to run
	dBR->NodeStatusCH = NEED_GEM_AIA;
	
	m_NodeStatusCH.at(in) = (short)m_Node->GEM_run();

	if ( !( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_PIA ) )
	{
		StatusCheck = 1;
		return 5;
	}
	
	GetReactInfoFromGEM(in);

	cout << "Initial Running GEM on Node #" << in <<  " successful. "  << endl;
	//  Uncomment this to test variable pressures and temperatures
    //  m_T[in] += (in-1)*5;
    //  m_P[in] += (in-1)*20;
    //  Here the file output for the initial conditions can be implemented

    // Initialization of GEMIPM and chemical data kept in the FMT part
    // Can be done in a loop over boundary nodes
    //   cout << "Begin Initialiation part" << endl;

    // Read DATABR structure from text file (read boundary condition)
    // TNode::na->GEM_read_dbr( dbr_input_file_name );//HB ? What is this?
	// This is for BC and IC data, not set here.


}	
	return 0;
}

string REACT_GEM::Get_Init_File_Path(void)
{
	return init_input_file_path;
}

string REACT_GEM::Get_IPM_File_Path(void)
{
	return ipm_input_file_path;
}

string REACT_GEM::Get_DBR_File_Path(void)
{
	return dbr_input_file_path;
}

string REACT_GEM::Get_DCH_File_Path(void)
{
	return dch_input_file_path;
}

int REACT_GEM::Set_IPM_FILE_PATH(string m_path)
{
	REACT_GEM::ipm_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_DBR_FILE_PATH(string m_path)
{
	REACT_GEM::dbr_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_DCH_FILE_PATH(string m_path)
{
	REACT_GEM::dch_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_Init_File_Path(string m_path)
{
	REACT_GEM::init_input_file_path = m_path;
	return 0;
}
bool REACT_GEM::Load_Init_File(string m_Project_path)
{
	string init_path;

	//Checking absolute path
	if (init_input_file_path.find( m_Project_path ) != string::npos  )
	{
		init_path = init_input_file_path;
	}
	else
	{
		init_path = m_Project_path.append( init_input_file_path );
	}

	if( m_Node->GEM_init( init_path.c_str() ) )
	{
	    return 0;  // error occured during reading the files
	}
	else 
	{
		//read init file successed
		return 1;
	}
}

short REACT_GEM::GetReactInfoFromMassTransport(int timelevel)
{
	heatflag = GetHeatFlag_MT();
	flowflag = GetFlowType_MT();
	REACT_GEM::nNodes = GetNodeNumber_MT();

	for (long node_i=0; node_i < nNodes ; node_i++)
	{
		//get temperature from MT
		m_T[node_i] = REACT_GEM::GetTempValue_MT(node_i, timelevel);

		//get pressure from MT
		m_P[node_i] = REACT_GEM::GetPressureValue_MT(node_i, timelevel);

		//get Independent and dependent Component value from MT
		REACT_GEM::GetAqComponentValue_MT(node_i, timelevel, m_bIC+node_i*nIC);

		// Setting Solid Phase Component // HS: Solid does not move with MT, so do not need to couple. 06.2007
		// REACT_GEM::GetSoComponentValue_MT(node_i, timelevel, m_bIC[node_i]);

		//get PH value from MT
		m_pH[node_i] = REACT_GEM::GetComponentValue_MT(node_i,"pH", timelevel);

		//get pe value from MT
		m_pe[node_i] = REACT_GEM::GetComponentValue_MT(node_i,"pe", timelevel);
	}

	return 0;
}
short REACT_GEM::SetReactInfoBackMassTransport(int timelevel)
{
	for (long in=0; in < nNodes ; in++)
	{
		// Setting Temperature
		REACT_GEM::SetTempValue_MT(in,timelevel,m_T.at(in));

		// Setting Pressure
		REACT_GEM::SetPressureValue_MT(in,timelevel,m_P.at(in));

		// Setting Independent Component
		REACT_GEM::SetAqComponentValue_MT(in,timelevel,m_bPS+in*nIC*nPS);

		// Setting Solid Phase Component
		// REACT_GEM::SetSoComponentValue_MT(in,timelevel,m_xPH+in*nPH);

		// Setting PH
		REACT_GEM::SetPHValue_MT(in,timelevel,m_pH.at(in));
		
		// Setting pe
		REACT_GEM::SetPeValue_MT(in,timelevel,m_pe.at(in));
	}
	return 0;
}

void REACT_GEM::GetReactInfoFromGEM(long in)
{
    // Extracting chemical datat into FMT part
	m_Node->GEM_restore_MT( m_NodeHandle.at(in), m_NodeStatusCH.at(in), m_T.at(in),
		m_P.at(in), m_Vs.at(in), m_Ms.at(in),
       m_bIC+in*nIC, m_dul+in*nDC, m_dll+in*nDC, m_aPH+in*nPH );
	// Extracting GEMIPM output data to FMT part
	m_Node->GEM_to_MT( m_NodeHandle.at(in), m_NodeStatusCH.at(in), m_IterDone.at(in),
		m_Vs.at(in), m_Ms.at(in), m_Gs.at(in), m_Hs.at(in), m_IC.at(in), m_pH.at(in), m_pe.at(in),
       m_Eh.at(in), m_rMB+in*nIC, m_uIC+in*nIC, m_xDC+in*nDC, m_gam+in*nDC,
       m_xPH+in*nPH, m_vPS+in*nPS, m_mPS+in*nPS,
       m_bPS+in*nIC*nPS, m_xPA+in*nPS );
	return;
}
void REACT_GEM::SetReactInfoBackGEM(long in)
{
		// Setting input data for GEMIPM
		m_Node->GEM_from_MT( m_NodeHandle.at(in), m_NodeStatusCH.at(in),
				 m_T.at(in), m_P.at(in), m_Vs.at(in), m_Ms.at(in),
				 m_bIC+in*nIC, m_dul+in*nDC, m_dll+in*nDC, m_aPH+in*nPH );
	return;
}

short REACT_GEM::Run_MainLoop()
{
	REACT_GEM::nNodes = GetNodeNumber_MT();

	for (long in = 0; in < nNodes ; in++)
	{
		//Get data
		REACT_GEM::SetReactInfoBackGEM(in);

		// Order GEM to run
		dBR->NodeStatusCH = NEED_GEM_AIA;
		
		m_NodeStatusCH.at(in) = (short)m_Node->GEM_run();

		if ( !( m_NodeStatusCH.at(in) == OK_GEM_AIA || m_NodeStatusCH.at(in) == OK_GEM_PIA ) )
		{
			cout << "Error: Main Loop failed when running GEM on Node #" << in << "." << endl;
			//return 5;
		}
	
		cout << "Main Loop Running GEM on Node #" << in <<  " successful. "  << endl;
		// Give Databack
		REACT_GEM::GetReactInfoFromGEM(in);
	}	
	return 0;
}


int REACT_GEM::GetHeatFlag_MT(void)
{
	//heat transport
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("HEAT_TRANSPORT") == 0)
		{
			return 1;
		}
	}
	return 0;
}

int REACT_GEM::GetFlowType_MT(void)
{
	//flow type
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("GROUNDWATER_FLOW")==0)
		{
			return 1;
		}
		else if (m_pcs->pcs_type_name.compare("LIQUID_FLOW")==0)
		{
			return 2;
		}
		else if (m_pcs->pcs_type_name.compare("RICHARDS_FLOW")==0)
		{
			return 3;
		}
		else if (m_pcs->pcs_type_name.compare("FLOW")==0)
		{
			return 4;
		}
	}
	return 0;
}

long REACT_GEM::GetNodeNumber_MT(void)
{
	long number;
	//------------read number of nodes--------------
	for (int i=0; i < (int)pcs_vector.size(); i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
		{
			number = (long) m_pcs->m_msh->GetNodesNumber(false);
			return number;
		}
	}
	//------------end of reading number of nodes---- 
	return 0;
}

double REACT_GEM::GetTempValue_MT(long node_Index, int timelevel)
{
	int indx0,indx1;
	double temp;
	
	if (heatflag == 1)
	{
		m_pcs = PCSGet("HEAT_TRANSPORT");

		indx0 = m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
		// timelevel =1;   //HS: 06.2007 no idea why MX set it to 1;
		indx1 = indx0 +1;  //m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
		temp = m_pcs->GetNodeValue(node_Index, indx1); //HS 05.06.2007 Confusion here. why indx1???//13.06.2007, HS: 0-previous; 1-current?
        //sysT[i] = m_pcs->GetNodeValue(i, indx1); 
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo °C->K
        //if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo °C->K
	}
    else 
	{
		temp = 25.0;
	 //HS: 04.06.2007 This might cause toubles. Better to set to a variable which could be modified from outside. 
	}
	return temp;
}
short REACT_GEM::SetTempValue_MT(long node_Index, int timelevel, double temp)
{
	int indx0,indx1;
		
	if (heatflag == 1)
	{
		m_pcs = PCSGet("HEAT_TRANSPORT");

		indx0 = m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
		// timelevel =1;   //HS: 06.2007 no idea why MX set it to 1;
		indx1 = indx0 +1;  //m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
		m_pcs->SetNodeValue(node_Index, indx0, temp); // HS: why indx1? take care here.
		//sysT[i] = m_pcs->GetNodeValue(i, indx1); 
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo °C->K
		//if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo °C->K
		return 1;
	}
	else
	return 0;
}

double REACT_GEM::GetPressureValue_MT(long node_Index, int timelevel)
{
//Get pressure value 
  double pressure;
  int indx0;
  if(flowflag > 0)
  {
	  switch(flowflag)
	  {
        case 1:
		  m_pcs = PCSGet("GROUNDWATER_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("HEAD")+timelevel;
		  pressure = m_pcs->GetNodeValue(node_Index, indx0); 
		  // change the pressure unit from meters of water to bar. 
		  pressure = pressure / 1.0e5;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
          break;
        case 2:
		  m_pcs = PCSGet("LIQUID_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  pressure = m_pcs->GetNodeValue(node_Index, indx0); 
		  // change the pressure unit from meters of water to bar. 
		  pressure = pressure / 1.0e5;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
		  break;
        case 3:
		  m_pcs = PCSGet("RICHARDS_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
  		  pressure = m_pcs->GetNodeValue(node_Index, indx0);
		  // change the pressure unit from meters of water to bar. 
		  pressure = pressure / 1.0e5;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
          break;
        case 4:
		  DisplayErrorMsg("Error: Not implemented for the flow in GEM case!!!");
		  pressure = 1.0;   //TODO  MX // HB: TODO
          break;
      }
  }
  else 
  {
	  DisplayErrorMsg("Warning: Not valid flow process!!");
  }
  return pressure;
}
short REACT_GEM::SetPressureValue_MT(long node_Index, int timelevel, double pressure)
{
//Set pressure value 
  int indx0;
  if(flowflag > 0)
  {
	  switch(flowflag)
	  {
        case 1:
		  m_pcs = PCSGet("GROUNDWATER_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("HEAD")+timelevel;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
		  m_pcs->SetNodeValue(node_Index, indx0, pressure); 
          break;
        case 2:
		  m_pcs = PCSGet("LIQUID_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
		  m_pcs->SetNodeValue(node_Index, indx0, pressure); 
		  break;
        case 3:
		  m_pcs = PCSGet("RICHARDS_FLOW");
		  indx0 = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
		  m_pcs->SetNodeValue(node_Index, indx0, pressure); 
          break;
        case 4:
		  if (pressure<1.0) pressure = 1.0;//HS 04.06.2007 Seems all the pressures are set to 1 bar.
		  m_pcs->SetNodeValue(node_Index, indx0, pressure); 
          break;
      }
	  return 1;
  }
  else 
  {
	  DisplayErrorMsg("Warning: Not valid flow process!!");
	  return 0;
  }

}
double REACT_GEM::GetComponentValue_MT(long node_Index, string m_component, int timelevel)
{
	double m_comp_value;
	m_comp_value = -1.0;
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{
			if (strcmp(m_pcs->pcs_primary_function_name[0],m_component.c_str()) == 0)
			{m_comp_value = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]+timelevel));}

		}
	}
	if ( m_comp_value != -1.0 ){return m_comp_value;}
	else{DisplayErrorMsg("Error: Corresponding Component NOT FOUND!!!");}
}

short REACT_GEM::GetAqComponentValue_MT(long node_Index, int timelevel, double* m_Component)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;
			int x_Component = -1;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;

			if (str.compare("pH") != 0 && str.compare("pe") != 0 )
			{
				x_Component = m_Node->IC_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
				if ( x_Component != -1)
				{
					*(m_Component+x_Component) = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]+timelevel));
				}
				else
				{
					x_Component = m_Node->DC_name_to_xDB(str.c_str());
					if ( x_Component != -1 )
					{
						*(m_Component+x_Component) = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]+timelevel));
					}
					else
					{
						DisplayErrorMsg("Error: Corresponding Component NOT FOUND in GEM part!!");
						return 0;
					}
				}
			}
		}
	}
	return 1;
}
short REACT_GEM::GetSoComponentValue_MT(long node_Index, int timelevel, double* m_Component)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;
			double val;
			int x_Component = 0;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			x_Component = m_Node->IC_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( x_Component != -1)
			{
				*m_Component = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]+timelevel));
				return 1;
			}
			else
			{
				DisplayErrorMsg("Error: Corresponding Component NOT FOUND in GEM part!!");
				return 0;
			}
		}
		else
		{
			DisplayErrorMsg("Error: MASS TRANSPORT NOT FOUND!!");
			return 0;
		}
	}
}
short REACT_GEM::SetAqComponentValue_MT(long node_Index, int timelevel, double* m_Component)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;
			int x_Component = -1;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;

			if (str.compare("pH") != 0 && str.compare("pe") != 0 )
			{
				x_Component = m_Node->IC_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
				if ( x_Component != -1)
				{
					m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]),*(m_Component+x_Component));	
				}
				else
				{
					x_Component = m_Node->DC_name_to_xDB(str.c_str());
					if ( x_Component != -1 )
					{
						m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]),*(m_Component+x_Component));
					}
					else
					{
						DisplayErrorMsg("Error: Corresponding Component NOT FOUND in MT part!!");
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

short REACT_GEM::SetSoComponentValue_MT(long node_Index, int timelevel, double* m_Component)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;
			int x_Component = -1;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;

			if (str.compare("pH") != 0 && str.compare("pe") != 0 )
			{
				x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
				if ( x_Component != -1)
				{
					m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]),*(m_Component+x_Component));	
				}
				else
				{
					DisplayErrorMsg("Error: Corresponding Component NOT FOUND in MTSo part!!");
					return 0;
				}
			}
		}
	}
	return 1;
}

short REACT_GEM::SetPHValue_MT(long node_Index, int timelevel, double m_PH)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;
						
			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare("pH") == 0)
			{
				m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]),m_PH);
				return 1;
			}
		}
	}
	// Not found PH: 
	DisplayErrorMsg("Error: PH NOT FOUND in MT part!!");
	return 0;
}

short REACT_GEM::SetPeValue_MT(long node_Index, int timelevel, double m_PE)
{							   
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare("pe") == 0)
			{
				m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0]),m_PE);
				return 1;
			}
		}
	}
	// Not found PE: 
	DisplayErrorMsg("Error: PE NOT FOUND in MT part!!");
	return 0;
}
