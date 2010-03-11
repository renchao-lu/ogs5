//-------------------------------------
// rf_REACT_GEM.cpp
// Haibing Shao 25.03.08
// haibing.shao@ufz.de
// GEM Reaction Package
// based on the PSI node-GEM source code
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------
#include "Configure.h"

#include "rf_REACT_GEM.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"
#include "msh_node.h"
#include "msh_elem.h"
extern string GetLineFromFile1(ifstream*);

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

//
#ifdef _WIN32
#include "direct.h" // on win32 and win64 platform
#else
#include "unistd.h" // on unix/linux platform
#include "stdlib.h"
#endif

#ifdef USE_MPI_GEMS
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#include "mpi.h"//Parallel Computing Support
#include "par_ddc.h"
// HS 07.01.2008: Comment the following 2 lines on LiClus.
// int size;
// int myrank;
#endif

#ifdef GEM_REACT
REACT_GEM::REACT_GEM ( void )
{
	m_Node = new TNode();
	REACT_GEM::dch_input_file_path = "calcite-dch.dat";
	REACT_GEM::ipm_input_file_path = "calcite-ipm.dat";
	REACT_GEM::dbr_input_file_path = "calcite-dbr-0-0000.dat";
	REACT_GEM::dbr_bc_input_file_path = "calcite-dbr-0-0001.dat";
	REACT_GEM::init_input_file_path= "calcite-init.dat";
	REACT_GEM::init_input_file_sig = "-init.dat";

	nIC = 0;
	nDC = 0;
	nPH = 0;
	nPS = 0;
	nNodes = 0;
	nElems = 0;
	idx_water = -1;
	initialized_flag = 0;
	heatflag = 0;
	flowflag = 0;
	flag_node_element_based = 0;//0-node based; 1-elem based;
	flag_porosity_change = 1   ;//0-not coupled;1=coupled;
	min_possible_porosity=1.e-4; // minimum porostiy in case of changing porosity: avoid zero porosity
	max_possible_porosity=0.9999;// max porosity
	flag_coupling_hydrology = 0;//0-not coupled;1=coupled;
	flag_permeability_porosity=0;//0-no coupling; 1-Kozeny-Carman; 2-Kozeny-Carman normalized;
	m_gem_temperature=298.15;  //default gem temperature is 25°C, in Kelvin!
	m_gem_pressure=1.0e+5; // default pressure 1 bar
	flag_iterative_scheme = 0;  //0-not iteration;1=iteration;
	// flag for different iterative scheme
	// 0 - sequential non-iterative scheme
	// 1 - standard iterative scheme
	// 2 - symetric iterative scheme
	// 3 - strang splitting scheme
	gem_mass_scale=1.0e-0; // GEMS default mass scaling parameter
	flag_gem_smart=0; // should be zero if working with kinetics
	m_IterDoneIndexSort = 100; // every 100 timesteps we sort NodeIndex for MPI
	m_ShuffleGems = 0; // do not randomize GEMS execution


	mp_nodeTypes = new long;
	* ( mp_nodeTypes ) = 0;

	m_FluidProp = NULL;
}


REACT_GEM::~REACT_GEM ( void )
{
	if ( initialized_flag > 0 )
	{

	        
		delete [] m_xDC; delete [] m_gam;
		delete  [] m_xPH; delete []m_aPH;
		delete []m_vPS;delete []m_mPS; delete []m_bPS; delete []m_xPA; delete []m_dul; delete []m_dll; delete []m_uIC; delete []m_bIC; delete []m_bIC_dummy; delete []m_rMB; delete []m_xDC_pts; delete []m_xDC_MT_delta; delete []m_xDC_Chem_delta; delete []m_NodeHandle;

		delete [] m_NodeStatusCH; delete [] m_IterDone; delete [] m_IterDoneIndex; delete [] m_IterDoneCumulative; delete [] m_T; delete [] m_P; delete [] m_Vs; delete [] m_Ms; delete [] m_Gs; delete [] m_Hs; delete [] m_IC; delete [] m_pH; delete [] m_pe; delete [] m_Eh; delete [] m_porosity; delete [] m_excess_water; delete [] m_excess_gas; delete [] m_Node_Volume;
		// delete MPI buffer--------
		delete [] m_NodeHandle_buff; delete [] m_NodeStatusCH_buff; delete [] m_IterDone_buff;

		delete [] m_Vs_buff; delete [] m_Ms_buff;
		delete []  m_Gs_buff; delete [] m_Hs_buff;
		delete [] m_IC_buff; delete [] m_pH_buff;
		delete [] m_pe_buff; delete [] m_Eh_buff;

		delete [] m_rMB_buff; delete [] m_uIC_buff; delete [] m_xDC_buff; delete [] m_gam_buff;
		delete [] m_xPH_buff; delete [] m_vPS_buff; delete [] m_mPS_buff; delete [] m_bPS_buff;
		delete [] m_xPA_buff,delete [] m_excess_water_buff; delete [] m_excess_gas_buff;
		delete [] m_porosity_buff; delete []m_boundary;
		delete []m_dul_buff; delete []m_dll_buff;
		delete [] m_xDC_pts_buff ;
		delete [] m_xDC_MT_delta_buff ;
		delete [] m_xDC_Chem_delta_buff;
		delete [] m_aPH_buff;delete [] m_bIC_buff; delete [] m_bIC_dummy_buff; delete [] m_porosity_Elem_buff; delete [] m_porosity_Elem;
		// -------------------------
		delete mp_nodeTypes;

		m_FluidProp = NULL;
		m_pcs = NULL;
		m_flow_pcs = NULL;

	}
	delete  m_Node;
}


// Initialization of the GEM TNode Class
short REACT_GEM::Init_Nodes ( string Project_path )
{

	// Creating TNode structure accessible trough node pointer
	// Here we read the files needed as input for initializing GEMIPM2K
	// The easiest way to prepare them is to use GEMS-PSI code (GEM2MT module)
	if ( Load_Init_File ( Project_path ) )
	{
		// The init file is successfully loaded
		// Getting direct access to DataCH structure in GEMIPM2K memory
		dCH = m_Node->pCSD();
		if ( !dCH )
			return 3;

		// Getting direct access to work node DATABR structure which
		// exchanges data between GEMIPM and FMT parts
		dBR = m_Node->pCNode();
		if ( !dBR )
			return 4;

		// Extracting data bridge array sizes
		nIC = dCH->nICb;//Num of Independent components
		nDC = dCH->nDCb;//Num of Chemical species in the reactive part
		nPH = dCH->nPHb;//Num of Phases
		nPS = dCH->nPSb;//Num of multicomponent phases; ASSERT(nPS < nPH)

		// get the index of water
		idx_water = FindWater_xDC();
		// imediately check
		if ( idx_water == -1 ) return 1;

		heatflag = GetHeatFlag_MT();// Get heatflag
		flowflag = GetFlowType_MT();// Get flow flag

		// get m_flow_pcs already, then check the flag:
		if ( flag_coupling_hydrology == 1 )
		{
			// need to couple to flow process;
			// mark the flag
			m_flow_pcs->flag_couple_GEMS = 1;
		}
		else
		{
			m_flow_pcs->flag_couple_GEMS = 0; //default is set to not-coupled!
		}

		// Get number of Nodes
		nNodes = GetNodeNumber_MT();
		// Get number of Elems
		nElems = GetElemNumber_MT();

		// Allocating work memory for FMT part (here only chemical variables)
		m_NodeHandle = new long [nNodes];
		m_NodeStatusCH = new long [nNodes];
		m_IterDone = new long [nNodes];
		m_IterDoneCumulative = new long [nNodes];
		m_IterDoneIndex = new long[nNodes];

		// MPI Buffer Variable---------------
		m_NodeHandle_buff = new long [nNodes];
		m_NodeStatusCH_buff = new long [nNodes];
		m_IterDone_buff = new long[nNodes];



		m_T  = new double [nNodes];
		m_P  = new double [nNodes];
		m_Vs = new double [nNodes];
		m_Ms = new double [nNodes];
		m_Gs = new double [nNodes];
		m_Hs = new double [nNodes];
		m_IC = new double [nNodes];
		m_pH = new double [nNodes];
		m_pe = new double [nNodes];
		m_Eh = new double [nNodes];
		m_porosity     = new double [nNodes];
		m_excess_water = new double [nNodes];
		m_excess_gas = new double [nNodes];
		m_Node_Volume  = new double [nNodes];


		m_porosity_Elem = new double [nElems];

		m_boundary = new int [nNodes]; // this marks boundary nodes with fixed concentrations!?

		// MPI Buffer Variable---------------
		m_Vs_buff = new double[nNodes];
		m_Ms_buff = new double[nNodes];
		m_Gs_buff = new double[nNodes];
		m_Hs_buff = new double[nNodes];
		m_IC_buff = new double[nNodes];
		m_pH_buff = new double[nNodes];
		m_pe_buff = new double[nNodes];
		m_Eh_buff = new double[nNodes];
		m_porosity_buff = new double[nNodes];
		m_excess_water_buff = new double [nNodes];
		m_excess_gas_buff = new double [nNodes];

		m_porosity_Elem_buff = new double [nElems];

		m_bIC = new double [nNodes*nIC];
		m_bIC_dummy = new double [nNodes*nIC];
		m_rMB = new double [nNodes*nIC];
		m_uIC = new double [nNodes*nIC];
		m_xDC = new double [nNodes*nDC];
		m_gam = new double [nNodes*nDC];
		m_dul = new double [nNodes*nDC];
		m_dll = new double [nNodes*nDC];
		m_aPH = new double [nNodes*nPH];
		m_xPH = new double [nNodes*nPH];
		m_vPS = new double [nNodes*nPS];
		m_mPS = new double [nNodes*nPS];
		m_bPS = new double [nNodes*nIC*nPS];
		m_xPA = new double [nNodes*nPS];
		m_xDC_pts = new double [nNodes*nDC];
		m_xDC_MT_delta = new double [nNodes*nDC];
		m_xDC_Chem_delta =  new double [nNodes*nDC];


		m_bIC_buff = new double [nNodes*nIC];
		m_bIC_dummy_buff = new double [nNodes*nIC];
		m_rMB_buff = new double [nNodes*nIC];
		m_uIC_buff = new double [nNodes*nIC];
		m_xDC_buff = new double [nNodes*nDC];
		m_gam_buff = new double [nNodes*nDC];
		m_xPH_buff = new double [nNodes*nPH];
		m_aPH_buff = new double [nNodes*nPH];
		m_vPS_buff = new double [nNodes*nPS];
		m_mPS_buff = new double [nNodes*nPS];
		m_bPS_buff = new double [nNodes*nIC*nPS];
		m_xPA_buff = new double [nNodes*nPS];
		m_dul_buff = new double [nNodes*nDC];
		m_dll_buff = new double [nNodes*nDC];
		m_xDC_pts_buff = new double [nNodes*nDC];
		m_xDC_MT_delta_buff = new double [nNodes*nDC];
		m_xDC_Chem_delta_buff =  new double [nNodes*nDC];

		// ----------------------------------


		for ( long in = 0; in < nNodes ; in++ )
		{
			m_boundary[in]=0; // cout << "boundary init ok"<<endl;
			m_NodeHandle[in] = 0;
			m_NodeStatusCH[in] = 0;
			m_IterDone[in] = 0;
			m_IterDoneCumulative[in]=0;
			m_IterDoneIndex[in]=in; //initial values order of Nodes
			m_NodeHandle_buff[in] = 0;
			m_NodeStatusCH_buff[in] = 0;
			m_IterDone_buff[in] = 0;

			m_T[in] = 298.15; //equivalent to 25°C
			m_P[in] = 1.0e+5;
			m_Vs[in] = 0.0;
			m_Ms[in] = 0.0;
			m_Gs[in] = 0.0;
			m_Hs[in] = 0.0;
			m_IC[in] = 0.0;
			m_pH[in] = 0.0;
			m_pe[in] = 0.0;
			m_Eh[in] = 0.0;
			m_porosity[in]=0.1;

			m_Vs_buff[in] = 0.0;
			m_Ms_buff[in] = 0.0;
			m_Gs_buff[in] = 0.0;
			m_Hs_buff[in] = 0.0;
			m_IC_buff[in] = 0.0;
			m_pH_buff[in] = 0.0;
			m_pe_buff[in] = 0.0;
			m_Eh_buff[in] = 0.0;
			m_porosity_buff[in]= 0.0;

			m_excess_water[in] = 0.0;
			m_excess_gas[in] = 0.0;

			m_Node_Volume[in]  = REACT_GEM::GetNodeAdjacentVolume ( in );
			m_excess_water_buff[in] = 0.0;
			m_excess_gas_buff[in] = 0.0;


			int ii;
			for ( ii = 0; ii < nIC ; ii++ )
			{
				m_bIC[in*nIC + ii ] = 0.0;
				m_bIC_dummy [ in*nIC + ii ] = 0.0;
				m_rMB[in*nIC + ii ] = 0.0;
				m_uIC[in*nIC + ii ] = 0.0;

				m_bIC_buff[in*nIC + ii ] = 0.0;
				m_bIC_dummy_buff [ in*nIC + ii ] = 0.0;
				m_rMB_buff[in*nIC + ii ] = 0.0;
				m_uIC_buff[in*nIC + ii ] = 0.0;
			}

			for ( ii = 0; ii < nDC ; ii++ )
			{
				m_xDC[in*nDC+ii ] = 0.0;
				m_gam[in*nDC+ii ] = 0.0;
				m_dul[in*nDC+ii ] = 1.0e+10; // this should be a large number, because after scaling to 1kg in Gems it should be 1.e+6
				m_dll[in*nDC+ii ] = 0.0; // zero is ok
				m_xDC_pts[in*nDC+ii ] = 0.0;
				m_xDC_MT_delta[in*nDC+ii ] = 0.0;
				m_xDC_Chem_delta[in*nDC+ii ] = 0.0;

				m_xDC_buff[in*nDC+ii ] = 0.0;
				m_gam_buff[in*nDC+ii ] = 0.0;
				m_dul_buff[in*nDC+ii ] = 0.0;
				m_dll_buff[in*nDC+ii ] = 0.0;
				m_xDC_pts_buff[in*nDC+ii ] = 0.0;
				m_xDC_MT_delta_buff[in*nDC+ii ] = 0.0;
				m_xDC_Chem_delta_buff[in*nDC+ii ] = 0.0;
			}

			for ( ii = 0; ii < nPH ; ii++ )
			{
				m_aPH[in*nPH+ii ] = 0.0;
				m_xPH[in*nPH+ii ] = 0.0;

				m_xPH_buff[in*nPH+ii ] = 0.0;
				m_aPH_buff[in*nPH+ii ] = 0.0;
			}

			for ( ii = 0; ii < nPS ; ii++ )
			{
				m_vPS[in*nPS+ii ] = 0.0;
				m_mPS[in*nPS+ii ] = 0.0;
				m_xPA[in*nPS+ii ] = 0.0;

				m_vPS_buff[in*nPS+ii ] = 0.0;
				m_mPS_buff[in*nPS+ii ] = 0.0;
				m_xPA_buff[in*nPS+ii ] = 0.0;
			}


			for ( ii = 0; ii < nIC ; ii++ )
			{
				for ( int jj = 0; jj < nPS ; jj++ )
				{
					m_bPS[in*nIC*nPS+jj ] = 0.0;

					m_bPS_buff[in*nIC*nPS+jj ] = 0.0;
				}
			}
		}

		for ( long in = 0; in < nElems ; in++ )
		{
			m_porosity_Elem[in] = 0.0;
			m_porosity_Elem_buff[in] = 0.0;
		}

		for ( long i=0 ; i < nNodes ; i++ ) // run Gems for all nodes exactly once! ---will be done in the Init-Run again with IC and BC values
		{
			dBR->NodeStatusCH = NEED_GEM_AIA;
			m_NodeStatusCH[i] = m_Node->GEM_run ( 1.0, false );

			if ( ! ( m_NodeStatusCH[i] == OK_GEM_AIA || m_NodeStatusCH[i] == OK_GEM_SIA ) )
			{
				cout << "Initial GEMs run second pass failed at node " << i << endl;
				REACT_GEM::GetReactInfoFromGEM ( i ); // this we need also for restart runs
			}
			else   // init arrays with value from databridge file ..all nodes should get the same
			{
				REACT_GEM::GetReactInfoFromGEM ( i ); // this we need also for restart runs

			}
		}
		// m_Node->na->GEM_write_dbr ( "dbr_for_crash_node1.txt" );
		// m_Node->na->GEM_print_ipm ( "ipm_for_crash_node1.txt" );
		return 0;//successed
	}
	else
	{
#ifdef USE_MPI_GEMS
		if ( myrank == 0 /*should be set to root*/ )
#endif

			cout << "Error loading initial files to GEMS" <<endl;
		exit ( 1 );
		return 1;
	}
}

short REACT_GEM::Init_RUN()
{
	long StatusCheck = 0;
	long i=0,in = 0,k=0,p=0,j=0;
	long idummy;
	double scalfac=1.0;
//        CompProperties *m_cp = NULL;
	CRFProcess* this_pcs;
	// Marking species with fixed concentrations (in boundary nodes)
	// first we check the boundary nodes with fixed concentrations
	// this is adopted from rf_REACT_BRNS...we only look for the first species, as with GEMS we should define boundary conditions for ALL species
	double BCValue = 0.0;


	for ( long i=0 ; i < nNodes ; i++ )
	{
		this_pcs = NULL;
//			m_cp = cp_vec[0];

		// Get the pointer to the proper PCS.
		this_pcs = PCSGet ( "MASS_TRANSPORT" );
		if ( this_pcs )
		{
			// BC printing
			if ( IsThisPointBCIfYesStoreValue ( i, this_pcs, BCValue ) )
			{
				// If this node is on the fixed boudnary for this component
				cout << "Node " << i <<", Comp " << this_pcs->pcs_primary_function_name[0] << " ,Value " << BCValue << " is boundary node" <<endl;
				m_boundary[i] = 1;
			}
			else
			{
				// If this node is NOT on the fixed boudnary for this component
				m_boundary[i] = 0;
			}
		}
		else  // not getting the pointer to the proper PCS.
		{
			cout << this_pcs->pcs_primary_function_name[0]<<"!!! In InitGEMS, can not find corresponding PCS for checking boundary conditions! "<<endl;
			abort();
		}

	}


// we have to make sure saturation is properly defined..otherwise we can not calculate concentrations properly
	if ( flowflag == 3 ) //this is only for Richards flow
	{
		cout << "GEM-INIT: " ;
		if ( m_flow_pcs->saturation_switch == true )
		{
			cout << "CalcSaturationRichards " << endl;
			m_flow_pcs->CalcSaturationRichards ( 1, true ); //is true here correct?
		} // JOD
		else
		{
			m_flow_pcs->CalcSecondaryVariablesUnsaturatedFlow ( true ); //WW
			cout << " CalcSecondaryVariablesUnsaturatedFlow" << endl;
		}
	}


//if (!m_flow_pcs->GetRestartFlag() >=2) {
// now we do the calculation!
// for GEM_REACT we also need internal information on porosity!....


	if ( ( m_flow_pcs->GetRestartFlag() >=2 ) ) {if ( !ReadReloadGem() ) abort();}

	GetReactInfoFromMassTransport ( 1 ); //get the initial values from MD - last time step!



	for ( in = 0; in < nNodes ; in++ )
	{
		// only when porosity is coupled; kg44...no..should be done always! as porosity might be spatially varying
		// if ( flag_porosity_change > 0 )
		//{
		// if ( m_porosity[in]<min_possible_porosity ) m_porosity[in]= min_possible_porosity;
		// if ( m_porosity[in]>max_possible_porosity ) m_porosity[in]= max_possible_porosity;
		// here get mass of
		// Convert to concentration NOT NECESSARY FOR FIRST RUN AS ICs should be given in MOLS...but necessary for reload case!!
		// cout << m_porosity[in] << endl;
		if ( ( m_flow_pcs->GetRestartFlag() >=2 ) ) REACT_GEM::ConcentrationToMass ( in ,1 );
		//}

		//Get data
		REACT_GEM::SetReactInfoBackGEM ( in ); // this is necessary, otherwise the correct data is not available
		// m_Node->na->GEM_write_dbr ( "dbr_for_crash_node0.txt" );
		// m_Node->na->GEM_print_ipm ( "ipm_for_crash_node0.txt" );
		// Order GEM to run
		dBR->NodeStatusCH = NEED_GEM_AIA;


		m_NodeStatusCH[in] = m_Node->GEM_run ( gem_mass_scale, false );


		if ( ! ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA ) )
		{
			cout << "Initial GEMs run second pass failed at node " << in ;
			idummy=0;scalfac=gem_mass_scale*100.0;
			// we try to rerun with different scaling factors....start from 1. down to 1.e-8
			while ( idummy < 1 )
			{
				REACT_GEM::SetReactInfoBackGEM ( in );
				dBR->NodeStatusCH = NEED_GEM_AIA;
				m_NodeStatusCH[in] = m_Node->GEM_run ( scalfac, false );
				if ( ! ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA ) )
				{
					scalfac /= 2.0;
				}
				else
				{
					cout << " sucess with mass_scaling: "<< scalfac << endl;

					// check if result is ok..we do it via volume
					//if ( fabs ( 1.0-m_Vs[in] ) >0.1 )
					//{
					//	cout << "Gems2 weird result at node " << in << " volume " << m_Vs[in] << " re-run GEMS" << endl;
					//	m_Node->na->GEM_write_dbr ( "dbr_for_crash_node0.txt" );
					//	m_Node->na->GEM_print_ipm ( "ipm_for_crash_node0.txt" );
					//	exit ( 1 );
					//}
					// calculate the chemical porosity
					//if ( flag_porosity_change > 0 ) REACT_GEM::CalcPorosity ( in );
					// Convert to concentration
					//REACT_GEM::MassToConcentration ( in );
					// here we do the calculation of excess water and correct water with repsect to porosity
					//REACT_GEM::CalcExcessWater ( in );
					idummy=1; // end the while loop because we have a result

				}
				if ( scalfac < 1.e-5 )
				{
					cout << " Error: Init Loop failed when running GEM on Node #" << in << "." << endl << "Returned Error Code: " << m_NodeStatusCH[in] << endl;
					m_Node->na->GEM_write_dbr ( "dbr_for_crash_node.txt" );
					exit ( 1 );
				}
			}
		}// end loop if initial gems run fails

		REACT_GEM::GetReactInfoFromGEM ( in ); // this we need also for restart runs



// this we have to do in any case at least once!!!!!!!! otherwise porosity might be different to the one assigned at the boundaries	// even if the porosity should not change, an initial consistent liquid - solid ratio is needed from GEMS!!!!
//		if ( flag_porosity_change > 0 )
//		{
		// calculate the chemical porosity
		REACT_GEM::CalcPorosity ( in,1.0 ); // depending on the accuracy of GEMS..this might change the porosity a bit!
//		}
		REACT_GEM::CalcReactionRate ( in, m_T[in], m_P[in] );
		// Convert to concentration: should be done always...
		REACT_GEM::MassToConcentration ( in,1 /*new timestep*/ );

		if ( flag_coupling_hydrology >0 )  REACT_GEM::CalcExcessWater ( in );

		// we should also push back the initial system including boundary nodes to avoid inconsistencies

		REACT_GEM::SetDCValue_MT ( in , 0, & ( m_xDC[in*nDC] ) ); // old timestep
		REACT_GEM::SetDCValue_MT ( in , 1, & ( m_xDC[in*nDC] ) ); // new timestep


		//GetReactInfoFromGEM(in);// HS 11.07.2007

#ifdef USE_MPI_GEMS
		if ( myrank == 0 /*should be set to root*/ ) {}
#endif
		//		cout << "Pass 2: Initial Running GEM on Node #" << in <<  " successful. "  << endl;

	}



	if ( flag_porosity_change > 0 ) // we do this also for restart!
	{
		// when switch is on;
		ConvPorosityNodeValue2Elem ( 0 ); // old timestep: update element porosity and push back values
		ConvPorosityNodeValue2Elem ( 1 ); // new timestep: update element porosity and push back values
	}

	cout << "Initial Running GEM  to get the correct porosities successful. "  << endl;

	return 0;
}


string REACT_GEM::Get_Init_File_Path ( void )
{
	return init_input_file_path;
}

string REACT_GEM::Get_IPM_File_Path ( void )
{
	return ipm_input_file_path;
}

string REACT_GEM::Get_DBR_File_Path ( void )
{
	return dbr_input_file_path;
}

string REACT_GEM::Get_DCH_File_Path ( void )
{
	return dch_input_file_path;
}

int REACT_GEM::Set_IPM_FILE_PATH ( string m_path )
{
	REACT_GEM::ipm_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_DBR_FILE_PATH ( string m_path )
{
	REACT_GEM::dbr_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_DCH_FILE_PATH ( string m_path )
{
	REACT_GEM::dch_input_file_path = m_path;
	return 0;
}

int REACT_GEM::Set_Init_File_Path ( string m_path )
{
	REACT_GEM::init_input_file_path = m_path;
	return 0;
}
bool REACT_GEM::Load_Init_File ( string m_Project_path )
{
	string init_path;
	char *buffer;

	init_path = m_Project_path.append ( REACT_GEM::init_input_file_path );

#ifdef _WIN32
	if ( init_path.rfind ( "\\" ) == string::npos ) // keep this on windows
#else
	if ( init_path.rfind ( "/" ) == string::npos ) // keep this on linux
#endif
	{
#ifdef _WIN32
		if ( ( buffer = _getcwd ( NULL, 0 ) ) == NULL )
#else
		if ( ( buffer = getcwd ( NULL, 0 ) ) == NULL )
#endif
			perror ( "_getcwd error" );
		else
		{
#ifdef _WIN32
			init_path.insert ( 0, "\\" ); // keep this on window
#else
			init_path.insert ( 0, "/" ); // keep this on linux
#endif
			init_path.insert ( 0, buffer );
		}
	}

    free(buffer);

	if ( m_Node->GEM_init ( init_path.c_str() , mp_nodeTypes , false ) )
	{
		return 0; // error occured during reading the files
	}
	else
	{
		return 1; // read init file successed
	}
}

short REACT_GEM::GetReactInfoFromMassTransport ( int timelevel )
{
	heatflag = GetHeatFlag_MT();
	flowflag = GetFlowType_MT();
	REACT_GEM::nNodes = GetNodeNumber_MT();

	for ( long node_i=0; node_i < nNodes ; node_i++ )
	{
		//get temperature from MT
		m_T[node_i] = REACT_GEM::GetTempValue_MT ( node_i, timelevel );

		//get pressure from MT
		m_P[node_i] = REACT_GEM::GetPressureValue_MT ( node_i, timelevel );
		//get Independent and dependent Component value from MT
		REACT_GEM::GetDCValue_MT ( node_i, timelevel, m_xDC+node_i*nDC, m_xDC_pts+node_i*nDC, m_xDC_MT_delta+node_i*nDC );
		// Convert to mole values
		// if(conv_concentration == 1)	REACT_GEM::ConcentrationToMass( node_i );

		// Setting Solid Phase Component // HS: Solid does not move.
		// REACT_GEM::GetSoComponentValue_MT(node_i, timelevel, m_xPH+node_i*nPH );

	}


	if ( m_flow_pcs && flag_coupling_hydrology ) m_flow_pcs->Clean_Water_ST_vec();

	return 0;
}

short REACT_GEM::SetReactInfoBackMassTransport ( int timelevel )
{

	for ( long in=0; in < nNodes ; in++ )
	{

		// Setting Temperature // disabled by HS. temperature is NOT the output from chemistry.
		// REACT_GEM::SetTempValue_MT(in,timelevel,m_T[in]);

		// Setting Pressure // disabled by HS. pressure is NOT the output from chemistry.
		// REACT_GEM::SetPressureValue_MT(in,timelevel,m_P[in]);

		// if (m_pcs->m_msh->nod_vector[in]->onBoundary() == false) {
		// Setting Independent Component
		if ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA )
		{
			REACT_GEM::SetDCValue_MT ( in , timelevel , & ( m_xDC[in*nDC] ) );
			// Setting PH
			REACT_GEM::SetPHValue_MT ( in,timelevel, m_pH[in] );

			// Setting pe
			REACT_GEM::SetPeValue_MT ( in,timelevel, m_pe[in] );

			// Setting pe
			REACT_GEM::SetEhValue_MT ( in,timelevel, m_Eh[in] );

			// Setting node_porosity
			REACT_GEM::SetNodePorosityValue_MT ( in,timelevel, m_porosity[in] );

		}

		// Set the extra water as source/sink term;
		// REACT_GEM::CalcExcessWater( in );
		if ( flag_coupling_hydrology>0 ) REACT_GEM::SetSourceSink_MT ( in, dt /*in sec*/ );
		// Setting Solid Phase Component
		// REACT_GEM::SetSoComponentValue_MT( in ,timelevel ,&( m_xPH[in*nPH]));
	}
	if ( flag_porosity_change>0 ) ConvPorosityNodeValue2Elem ( timelevel ); // new timestep :update element porosity and push back values
	return 0;
}

void REACT_GEM::GetReactInfoFromGEM ( long in )
{
	m_Node->GEM_to_MT ( m_NodeHandle[in], m_NodeStatusCH[in], m_IterDone[in],
	                    m_Vs[in], m_Ms[in], m_Gs[in], m_Hs[in], m_IC[in], m_pH[in], m_pe[in], m_Eh[in],
	                    m_rMB+in*nIC, m_uIC+in*nIC, m_xDC+in*nDC, m_gam+in*nDC, m_xPH+in*nPH, m_vPS+in*nPS, m_mPS+in*nPS,
	                    m_bPS+in*nIC*nPS, m_xPA+in*nPS, m_aPH+in*nPH );

	return;
}


void REACT_GEM::SetReactInfoBackGEM ( long in )
{
	long i;
	// Setting input data for GEMIPM

	// Using the overloaded version of GEM_from_MT() to load the data	// HS 10.07.2007
	for ( i=0;i<nIC;i++ )
	{
		m_bIC_dummy[in*nIC+i]=0.0; //old B vector: this should be always zero
	}
//	cout << *m_xDC+in*nDC << "   " << *m_bIC_dummy+in*nIC << endl;

	m_Node->GEM_from_MT ( m_NodeHandle[in], m_NodeStatusCH[in],
	                      m_T[in], m_P[in], m_Vs[in], m_Ms[in],
	                      m_bIC_dummy+in*nIC, m_dul+in*nDC, m_dll+in*nDC, m_aPH+in*nPH  ,m_xDC+in*nDC );
//	cout << m_xDC+in*nDC << endl;
	// set charge to zero
	m_Node->pCNode()->bIC[nIC-1]=0.0;


	return;
}


short REACT_GEM::Run_MainLoop ( string Project_path, long aktueller_zeitschritt )
{
	nNodes = GetNodeNumber_MT();
	nElems = GetElemNumber_MT();
	long /*i,j,ii,*/in,ii,idummy,node_fail=0;
	double scalfac,oldvolume;



#ifdef USE_MPI_GEMS
	// MPI initialization.
	// So here is going to distribute the task.
	MPI_Bcast ( &nNodes, 1, MPI_LONG, 0, MPI_COMM_WORLD );
	// here "myrank" is the index of the CPU Processes, and "mysize" is the number of CPU Processes
	for ( ii = myrank; ii < nNodes ; ii+= mysize )
	{
		in=m_IterDoneIndex[ii]; // m_IterDoneIndex can be sortet to account for nodes that spend a lot of time for gems
//		cout << " rank, calc node, ii " << myrank << " " << in <<" " <<ii<< endl;
#else
	if ( m_ShuffleGems ) ShuffleIterations ( m_IterDoneIndex, nNodes ); // randomize order of Gems executions !
	for ( ii = 0; ii < nNodes; ii++ )
	{
		in=m_IterDoneIndex[ii]; // m_IterDoneIndex can be sortet to account for nodes that spend a lot of time for gems
		// cout <<"iteration "<< ii << " node " << in <<endl;
#endif
		if ( m_boundary[in] ) // do this only without calculation if  on a boundary!!!
		{
#ifdef USE_MPI_GEMS
			REACT_GEM::CopyToMPIBuffer ( in ); // copy old values to buffer..otherwise we loose them
#endif
		}
		else // calculate if not on a boundary
		{
			//if(in == 100) { m_Node->na->GEM_write_dbr("dbr_for_crash_node_1.txt");  m_Node->na->GEM_print_ipm("ipm_for_crash_node_1.txt");}
			// Convert from concentration
			REACT_GEM::ConcentrationToMass ( in,1 ); // I believe this is save for MPI

			//Get data
			REACT_GEM::SetReactInfoBackGEM ( in ); // this should be also save for MPI
			// take values from old B volume for comparison
			oldvolume=m_Vs[in];
			// Order GEM to run
			//if(in == 100) { m_Node->na->GEM_write_dbr("dbr_for_crash_node_2.txt"); m_Node->na->GEM_print_ipm("ipm_for_crash_node_2.txt");}
			if ( flag_gem_smart )
			{
				dBR->NodeStatusCH = NEED_GEM_SIA;
				m_NodeStatusCH[in] = m_Node->GEM_run ( gem_mass_scale, true );
			}
			else
			{
				dBR->NodeStatusCH = NEED_GEM_AIA; // first try without simplex using old solution
				m_NodeStatusCH[in] = m_Node->GEM_run ( gem_mass_scale, false );
			}


			if ( ! ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA ) )   // ups...failed..try again without SIMPLEX
			{
				dBR->NodeStatusCH = NEED_GEM_AIA;
				m_NodeStatusCH[in] = m_Node->GEM_run ( gem_mass_scale, false );
			}


			if ( ! ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA ) )
			{
				// cout << "GEMS failed first pass at node " << in ;
				idummy=0;scalfac=gem_mass_scale*10.0;
				// we try to rerun with different scaling factors....start from 10. down to 0.1
				while ( idummy < 1 )
				{
					REACT_GEM::SetReactInfoBackGEM ( in );
					dBR->NodeStatusCH = NEED_GEM_AIA;
					m_NodeStatusCH[in] = m_Node->GEM_run ( scalfac, false );
					if ( ! ( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_SIA ) )
					{
						scalfac /= 1.5;
						// cout << " scalfac " << scalfac ;
					}
					else
					{
						REACT_GEM::GetReactInfoFromGEM ( in ); // we have buffer variable now if GEM_MPI is defined
						// check if result is ok..we do it via volume
						if ( fabs ( 1.0- ( m_Vs[in]/oldvolume ) ) >0.5 )
						{
							cout << "GEM weird result at node " << in << " volume " << m_Vs[in] << " old volume " <<oldvolume << endl;
							m_Node->na->GEM_write_dbr ( "dbr_for_crash_node_3.txt" );
							m_Node->na->GEM_print_ipm ( "ipm_for_crash_node_3.txt" );
							exit ( 1 );
						}
						idummy=1; // end the while loop because we have a result
						// cout << " GEM sucess with scaling of : " << scalfac << endl;
					}
					if ( scalfac < 1.0e-1 )
					{
						cout << "Error: Main Loop failed when running GEM on Node #" << in << "." << endl << "Returned Error Code: " << m_NodeStatusCH[in] ;
						cout << " scalfac " << scalfac << endl;
						m_Node->na->GEM_write_dbr ( "dbr_for_crash_node_fail.txt" );
						m_Node->na->GEM_print_ipm ( "ipm_for_crash_node_fail.txt" );
						// exit ( 1 );
						node_fail=1;
						idummy=1;
					}
				}
			}// end loop if initial gems run fails
			else
			{
				// this is if gem run is ok
				REACT_GEM::GetReactInfoFromGEM ( in ); //from here on we have buffer values if GEMS_MPI is defined
				// check if result is ok..we do it via volume
				if ( ( m_Vs[in]/oldvolume ) >2.0 )
				{
					cout << "GEM weird result at node " << in << " volume " << m_Vs[in] << " old volume " <<oldvolume << endl;
					m_Node->na->GEM_write_dbr ( "dbr_for_crash_node_4.txt" );
					m_Node->na->GEM_print_ipm ( "ipm_for_crash_node_4.txt" );
					// exit ( 1 );
				}
			}
			if ( node_fail <1 ) // this needs to be done with buffer variables for MPI
			{
				// CALC kintetic constrains
				REACT_GEM::CalcReactionRate ( in, m_T[in], m_P[in] ); // check for kinetics is done in the subroutine for each species separately

				// test kinetics!
				//Get data
				//		REACT_GEM::SetReactInfoBackGEM ( in ); // this should be also save for MPI
				//		m_NodeStatusCH[in] = m_Node->GEM_run ( scalfac, false );
				// CALC kintetic constrains
				//		cout << " second pass kinetics m_NodeStatusCH[in] " << m_NodeStatusCH[in]<< endl;
				//		REACT_GEM::CalcReactionRate ( in, m_T[in], m_P[in] ); // check for kinetics is done in the subroutine for each species separately
				// test kinetics finished
				// calculate the chemical porosity
				if ( flag_porosity_change > 0 )
				{
					REACT_GEM::CalcPorosity ( in );
				}
				// Convert to concentration  ..this should be done before eexcess water is calculated (otherwise mass balance for dissolved species changes?)
				REACT_GEM::MassToConcentration ( in, 1 );


				// here we do the calculation of excess water and correct water with respect to porosity
				if ( flag_porosity_change > 0 ) REACT_GEM::CalcExcessWater ( in ); //should be done always if porosity changes...as this corrects water volume ---- coupling to hydrology is accounted for separately in SetReactInfoBackMassTransport
			}
			else
			{
				// REACT_GEM::MassToConcentration ( in );
				node_fail=0;

			}
#ifdef USE_MPI_GEMS
				REACT_GEM::CopyToMPIBuffer ( in ); //copy data to MPI buffer in any case
#endif
		} //end if check for boundary node
	} // end for loop for all nodes
#ifdef USE_MPI_GEMS
// For MPI scheme, gather the data here.
	REACT_GEM::GetGEMResult_MPI();
	REACT_GEM::CleanMPIBuffer();
// we sort it every timestep...this can be improved by a parallel sorting and/or more effective sorting algorithm
	if ( aktueller_zeitschritt%m_IterDoneIndexSort == 0 )
	{
		if ( myrank == 0 ) 	cout << "GEMS: Run_Main resorting. timestep " << aktueller_zeitschritt << endl;
		SortIterations ( m_IterDoneCumulative,m_IterDoneIndex,nNodes );
		for ( ii=0;ii<nNodes;ii++ ) m_IterDoneCumulative[ii]=0;
	}
#endif
// this is done in main loop?        REACT_GEM::SetReactInfoBackMassTransport(1);
	cout << " GEM  run successful. "  << endl;
	return 0;
}


int REACT_GEM::GetHeatFlag_MT ( void )
{
	//heat transport
	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "HEAT_TRANSPORT" ) == 0 )
		{
			return 1;
		}
	}
	return 0;
}

int REACT_GEM::GetFlowType_MT ( void )
{
	//flow type
	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "GROUNDWATER_FLOW" ) ==0 )
		{
			m_flow_pcs = m_pcs;
			return 1;
		}
		else if ( m_pcs->pcs_type_name.compare ( "LIQUID_FLOW" ) ==0 )
		{
			m_flow_pcs = m_pcs;
			return 2;
		}
		else if ( m_pcs->pcs_type_name.compare ( "RICHARDS_FLOW" ) ==0 )
		{
			m_flow_pcs = m_pcs;
			return 3;
		}
		else if ( m_pcs->pcs_type_name.compare ( "MULTI_PHASE_FLOW" ) ==0 )
		{
			m_flow_pcs = m_pcs;
			return 4;
		}
	}
	return 0;
}

void REACT_GEM::GetFluidProperty_MT ( void )
{
	m_FluidProp = MFPGet ( "LIQUID" );
}

long REACT_GEM::GetNodeNumber_MT ( void )
{
	long number;
	//------------read number of nodes--------------
	for ( int i=0; i < ( int ) pcs_vector.size(); i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) ==0 )
		{
			number = ( long ) m_pcs->m_msh->GetNodesNumber ( false );
			return number;
		}
	}
	//------------end of reading number of nodes----
	return 0;
}

long REACT_GEM::GetElemNumber_MT ( void )
{
	long number;
	//------------read number of elems--------------
	for ( int i=0; i < ( int ) pcs_vector.size(); i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) ==0 )
		{
			number = ( long ) m_pcs->m_msh->ele_vector.size();
			return number;
		}
	}
	//------------end of reading number of nodes----
	return 0;
}

double REACT_GEM::GetTempValue_MT ( long node_Index, int timelevel )
{
	int indx;
	double temp;

	if ( heatflag == 1 )
	{
		m_pcs = PCSGet ( "HEAT_TRANSPORT" );

		indx = m_pcs->GetNodeValueIndex ( "TEMPERATURE1" ) +timelevel;
		temp = m_pcs->GetNodeValue ( node_Index, indx );

		//sysT[i] = m_pcs->GetNodeValue(i, indx1);
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo �C->K
		//if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo �C->K
	}
	else
	{
		temp = m_gem_temperature;
	}
	return temp;
}
short REACT_GEM::SetTempValue_MT ( long node_Index, int timelevel, double temp )
{
	int indx;

	if ( heatflag == 1 )
	{
		m_pcs = PCSGet ( "HEAT_TRANSPORT" );

		indx = m_pcs->GetNodeValueIndex ( "TEMPERATURE1" ) +timelevel;
		m_pcs->SetNodeValue ( node_Index, indx, temp );

		//sysT[i] = m_pcs->GetNodeValue(i, indx1);
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo �C->K
		//if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo �C->K
		return 1;
	}
	else
		return 0;
}

double REACT_GEM::GetPressureValue_MT ( long node_Index, int timelevel )
{
	//Get pressure value
	double pressure;
	int indx;
	pressure = 0.0;

	if ( flowflag > 0 )
	{
		GetFluidProperty_MT();
		switch ( flowflag )
		{
			case 1: // for "GROUNDWATER_FLOW";
				if ( aktueller_zeitschritt > 0 ) // not for first time step
				{
					indx = m_flow_pcs->GetNodeValueIndex ( "HEAD" );
					pressure = m_flow_pcs->GetNodeValue ( node_Index, indx + timelevel ); // The unit of HEAD is in meters
// cout << pressure << " " << " timelevel "<<timelevel;
					// change the pressure unit from hydraulic head to Pa.
					pressure = Pressure_M_2_Pa ( pressure , m_FluidProp->Density() );
//y cout << pressure << " density" << m_FluidProp->Density()<<endl;
					// add atmospheric pressure
					if ( pressure <= 0.0    /*valcumm suction in groundwater is not so realistic*/
					        || pressure > 1.0e+15  /*some very high pressure*/
					   ) {	cout << " high pressure " << pressure << endl; pressure = 1.0e+05; } // then set it to 1.0 bar = 1.0e5 Pa;
					break;
				}
				else
				{
					// just set to 1.0 bar.
					pressure = 1.0e+05 ;
					break;
				}
			case 2: // for "LIQUID_FLOW", not tested!!!
				if ( aktueller_zeitschritt > 0 ) // not for first time step
				{
					cout << "Liquid flow not supported - quitting program" << endl;
					exit ( 1 );

					indx = m_flow_pcs->GetNodeValueIndex ( "PRESSURE1" ) +timelevel;
					pressure = m_flow_pcs->GetNodeValue ( node_Index, indx ); // The unit of HEAD is in meters

					// change the pressure unit from meters of water to bar.
					// pressure = Pressure_M_2_Bar ( pressure , m_FluidProp->Density() );
					// add atmospheric pressure
					if ( pressure < 0.0    /*valcumm suction in groundwater is not so realistic*/
					        || pressure > 1.0e+15  /*some very high pressure*/
					   ) pressure = 1.0e+5;   // then set it to 1.0 bar;
					break;
				}
				else
				{
					// just set to 1.0 bar.
					pressure = 1.0e+05 ;
					break;
				}
			case 3: // for "RICHARDS_FLOW", not tested!!!
				if ( aktueller_zeitschritt > 0 ) // not for first time step
				{
					pressure = m_gem_pressure;
					//	indx = m_flow_pcs->GetNodeValueIndex ( "PRESSURE1" ) +timelevel;
					//	pressure = m_flow_pcs->GetNodeValue ( node_Index, indx ); // The unit of HEAD is in meters

					// change the pressure unit from meters of water to bar.
					//	pressure = Pressure_M_2_Bar ( pressure , m_FluidProp->Density() );
					// add atmospheric pressure
					//	pressure +=1.0;
					if ( pressure < 0.0    /*valcumm suction in groundwater is not so realistic*/
					        || pressure > 1.0e+15  /*some very high pressure*/
					   ) pressure = 1.0e+5;   // then set it to 1.0 bar;
					break;
				}
				else
				{
					// just set to 1.0 bar.
					pressure = m_gem_pressure;
					if ( pressure < 0.0    /*valcumm suction in groundwater is not so realistic*/
					        || pressure > 1.0e+15  /*some very high pressure*/
					   ) pressure = 1.0e+5;   // then set it to 1.0 bar;
					break;
				}
			case 4: // MULTIPHASE ....not tested
				if ( aktueller_zeitschritt > 0 ) // not for first time step
				{
					indx = m_flow_pcs->GetNodeValueIndex ( "PRESSURE1" );
					pressure = m_flow_pcs->GetNodeValue ( node_Index, indx +timelevel ); // The unit of HEAD is in meters

					// change the pressure unit from meters of water to bar.
					// pressure = Pressure_M_2_Bar ( pressure , m_FluidProp->Density() );
					// add atmospheric pressure

					if ( pressure < 0.0    /*valcumm suction in groundwater is not so realistic*/
					        || pressure > 1.0e+15  /*some very high pressure*/
					   ) pressure = 1.0e+5;   // then set it to 1.0 bar;
					break;
				}
				else
				{
					// just set to 1.0 bar.
					pressure = 1.0e+05 ;
					break;
				}
			default:
#ifdef USE_MPI_GEMS
				if ( myrank == 0 /*should be set to root*/ )
#endif
					cout<<"Error: Not implemented for the flow in GEM case!!!"<<endl;
				pressure = 1.0e+05;
				break;
		} // end of switch case;
	} // end of if (flow_flag);
	else
	{
		// if no valid flow pcs existing;
		cout<< "Warning: No valid flow process!!" <<endl;
	}
	return pressure;
}

short REACT_GEM::SetPressureValue_MT ( long node_Index, int timelevel, double pressure )
{
	//Set pressure value
	int indx;
	indx = 0;
	if ( flowflag > 0 )
	{
		switch ( flowflag )
		{
			case 1:
				m_pcs = PCSGet ( "GROUNDWATER_FLOW" );
				pressure = Pressure_Pa_2_M ( pressure ,  m_FluidProp->Density() );
				indx = m_pcs->GetNodeValueIndex ( "HEAD" ) +timelevel;
				m_pcs->SetNodeValue ( node_Index, indx, pressure );
				break;
			case 2:
				m_pcs = PCSGet ( "LIQUID_FLOW" );
				indx = m_pcs->GetNodeValueIndex ( "PRESSURE1" ) +timelevel;

				m_pcs->SetNodeValue ( node_Index, indx, pressure );
				break;
			case 3:
				// do nothing for Richards flow
				break;
//				m_pcs = PCSGet ( "RICHARDS_FLOW" );
//				indx = m_pcs->GetNodeValueIndex ( "PRESSURE1" ) +timelevel;
//				pressure = Pressure_Bar_2_Pa ( pressure );
//				m_pcs->SetNodeValue ( node_Index, indx, pressure );
//				break;
			case 4:
				m_pcs = PCSGet ( "MULTI_PHASE_FLOW" );
				indx = m_pcs->GetNodeValueIndex ( "PRESSURE1" ) +timelevel;

				m_pcs->SetNodeValue ( node_Index, indx, pressure );
			default:
#ifdef USE_MPI_GEMS
				if ( myrank == 0 /*should be set to root*/ )
#endif
					cout<< "Error: Not implemented for the flow in GEM case!!!" <<endl;
				break;
		}
	}
	else
	{
#ifdef USE_MPI_GEMS
		if ( myrank == 0 /*should be set to root*/ )
#endif
			cout<< "Warning: No valid flow process!!" <<endl;
		return 0;
	}
	return 1;
}
double REACT_GEM::GetComponentValue_MT ( long node_Index, string m_component, int timelevel )
{
	double m_comp_value;
	m_comp_value = -1.0;
	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			if ( strcmp ( m_pcs->pcs_primary_function_name[0],m_component.c_str() ) == 0 )
			{
				m_comp_value = m_pcs->GetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( m_pcs->pcs_primary_function_name[0] ) +timelevel );
			}
		}
	}
	if ( m_comp_value != -1.0 ) {return m_comp_value;}
	else
	{
#ifdef USE_MPI_GEMS
		if ( myrank == 0 /*should be set to root*/ )
#endif
			cout<< "Error: Corresponding Component NOT FOUND!!!" <<endl;
		return m_comp_value;
	}
}

short REACT_GEM::GetDCValue_MT ( long node_Index, int timelevel, double* m_DC, double* m_DC_pts ,double* m_DC_MT_delta )
{
	string str;
	double /*DC_MT_pre,*/ DC_MT_cur;

	for ( int i=0; i < nDC ; i++ )
	{
		m_pcs = pcs_vector[i+1];// dangerous!!
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			//if ( m_pcs->m_msh->nod_vector[node_Index]->onBoundary() == false ) // do not update values for boundary node?

			str = m_pcs->pcs_primary_function_name[0];
			if ( str.compare ( "pH" ) != 0 && str.compare ( "pe" ) != 0 && str.compare ( "Eh" ) != 0 && str.compare ( "NodePorosity" ) != 0 )
			{
				// Get previous iteration mass transport concentration value
				// DC_MT_pre = m_pcs->GetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +0 );
				// Get current iteration mass transport concentration value
				DC_MT_cur = m_pcs->GetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel );

				* ( m_DC+i ) = DC_MT_cur;
			}
		}
	}

	return 1;
}
short REACT_GEM::GetSoComponentValue_MT ( long node_Index, int timelevel, double* m_Phase )
{
			string str;
			int x_Component = 0;
	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{

			x_Component = -1;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			x_Component = m_Node->Ph_name_to_xDB ( str.c_str() );//get the index of certain compound, -1: no match
			if ( x_Component > -1 )
			{
				* ( m_Phase+x_Component ) = m_pcs->GetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel );
			}
			else
			{
				//DisplayErrorMsg("Error: Corresponding Component NOT FOUND in GEM part!!");
				//return 0;
			}
		}
	}
	//DisplayErrorMsg("Error: MASS TRANSPORT NOT FOUND!!");
	return 1;
}
short REACT_GEM::SetDCValue_MT ( long node_Index, int timelevel, double* m_DC )
{
	string str;
	for ( int i=0; i < nDC ; i++ )
	{

		m_pcs = pcs_vector[i+1];

		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			str = m_pcs->pcs_primary_function_name[0];
			if ( str.compare ( "pH" ) != 0 && str.compare ( "pe" ) != 0 && str.compare ( "Eh" ) != 0 && str.compare ( "NodePorosity" ) != 0 )
			{
				if ( flag_iterative_scheme > 0 )
				{
					if ( CPGetMobil ( m_pcs->GetProcessComponentNumber() ) > 0 )
					{
						// m_pcs->eqs->b[node_Index] += m_xDC_Chem_delta[node_Index*nDC+i] / dt ;
						m_pcs->SetNodeValue ( node_Index , m_pcs->GetNodeValueIndex ( str ) +timelevel , * ( m_DC+i ) );
					}
					else
					{
						m_pcs->SetNodeValue ( node_Index , m_pcs->GetNodeValueIndex ( str ) +timelevel , * ( m_DC+i ) );
					}
				}
				else
				{
					m_pcs->SetNodeValue ( node_Index , m_pcs->GetNodeValueIndex ( str ) +timelevel , * ( m_DC+i ) );
				}
			}
		}
	}

	return 1;
}

// i_timestep 0: old timestep 1: new timestep
int REACT_GEM::SetPorosityValue_MT ( long ele_Index,  double m_porosity_Elem , int i_timestep )
{

	int idx;
	double old_porosity ;

	idx = -1;
	old_porosity = 0.0;

	CRFProcess* m_pcs = NULL;

	if ( flowflag > 0 )
	{
		GetFluidProperty_MT();
		switch ( flowflag )
		{
			case 1:
				m_pcs     = PCSGet ( "GROUNDWATER_FLOW" );
				idx       = m_pcs->GetElementValueIndex ( "POROSITY" );

				// set new porosity;
				m_pcs->SetElementValue ( ele_Index, idx+i_timestep , m_porosity_Elem );
				break;
			case 2:
				m_pcs = PCSGet ( "LIQUID_FLOW" );
				idx=m_pcs->GetElementValueIndex ( "POROSITY" );
				// always write into the new step
				m_pcs->SetElementValue ( ele_Index,idx+i_timestep,m_porosity_Elem );
				break;
			case 3:
				m_pcs = PCSGet ( "RICHARDS_FLOW" );
				idx=m_pcs->GetElementValueIndex ( "POROSITY" );
				// always write into the new step
				m_pcs->SetElementValue ( ele_Index,idx+i_timestep,m_porosity_Elem );
				break;
			case 4:   // kg44: do we have to update POROSITY_IL and POROSITY_SW?
				m_pcs = PCSGet ( "MULTI_PHASE_FLOW" );
				idx=m_pcs->GetElementValueIndex ( "POROSITY1" );
				// always write into the new step
				m_pcs->SetElementValue ( ele_Index,idx+i_timestep,m_porosity_Elem );
				break;
			default:
#ifdef USE_MPI_GEMS
				if ( myrank == 0 /*should be set to root*/ )
#endif
					cout << "Error: Not implemented for the flow in GEM case!!!" <<endl;
				break;
		}
	}
	return 1;
}


int REACT_GEM::SetSourceSink_MT ( long in, double time_step_size /*in sec*/ )
{

	if ( fabs ( m_excess_water[in]/ ( time_step_size*GetNodeAdjacentVolume ( in ) ) ) > 1e-9 ) // very small number ---better do this check after division by time_step_size and node volume! ....accuracy of 10e-9 m^3 / m^3 = 1 ml /m^3 should be enough
	{
		Water_ST_GEMS m_st;
		CRFProcess* m_pcs = NULL;
		GetFluidProperty_MT();
		switch ( flowflag )
		{
			case 1: // groundwater flow
				m_st.index_node = in ;
				m_st.water_st_value  = m_excess_water[in]  / time_step_size;
				m_st.water_st_value *= m_Node_Volume[in];  // normalize with node volume
				m_flow_pcs->Water_ST_vec.push_back ( m_st );
				return 1;
				break;
			case 2:  // liquid flow
				m_st.index_node = in ;
				m_st.water_st_value  = m_excess_water[in]  / time_step_size;
				m_st.water_st_value *= m_Node_Volume[in];  // normalize with node volume
				m_flow_pcs->Water_ST_vec.push_back ( m_st );
				return 1;
				break;
			case 3: // Richards flow
				m_st.index_node = in ;
				m_st.water_st_value  = m_excess_water[in]  / time_step_size;
				m_st.water_st_value *= m_Node_Volume[in];  // normalize with node volume
				m_flow_pcs->Water_ST_vec.push_back ( m_st );
				return 1;
				break;
			case 4: // multiphase flow...works with case 1 ...pressure saturation scheme
				m_st.index_node = in ;
				m_st.water_st_value  = m_excess_water[in]  / time_step_size;
				m_st.water_st_value *= m_Node_Volume[in];  // normalize with node volume
				m_flow_pcs->Water_ST_vec.push_back ( m_st );
				return 1;
				break;
			default:
#ifdef USE_MPI_GEMS
				if ( myrank == 0 /*should be set to root*/ )
#endif
					cout<< "Error: Not implemented for the flow in GEM case!!!"<<endl;
				break;
		}
	}
	else
	{
		return 0;
	}

}

int REACT_GEM::FindWater_xDC ( void )
{
	// initialization
	int rt = -1;
	int i;

	// loop over all the xDC names, and find the one that is water
	for ( i = 0 ; i < nDC ; i++ )
	{
		if ( dCH->ccDC[i] == 'W' )
		{
			rt = i ;
			return rt;
		}
	}
	return rt;
}

short REACT_GEM::SetSoComponentValue_MT ( long node_Index, int timelevel, double* m_Phase )
{
			string str;
			int x_Component = -1;

	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			x_Component = -1;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;

			if ( str.compare ( "pH" ) != 0 && str.compare ( "pe" ) != 0 && str.compare ( "Eh" ) != 0&& str.compare ( "NodePorosity" ) != 0 )
			{
				x_Component = m_Node->Ph_name_to_xDB ( str.c_str() );//get the index of certain compound, -1: no match
				if ( x_Component != -1 )
				{
					m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ),* ( m_Phase+x_Component ) );
				}
				else
				{
					//DisplayErrorMsg("Error: Corresponding Component NOT FOUND in MTSo part!!");
					//return 0;
				}
			}
		}
	}
	return 1;
}

short REACT_GEM::SetNodePorosityValue_MT ( long node_Index, int timelevel, double m_porosity )
{
			string str;

	for ( int i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare ( "NodePorosity" ) == 0 )
			{
				m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel, m_porosity );
			}
			if ( str.compare ( "POROSITY" ) == 0 )
			{
				m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel, m_porosity );
			}
		}
	}
	return 1;
}

short REACT_GEM::SetPHValue_MT ( long node_Index, int timelevel, double m_PH )
{
	int i;
	string str;
	for ( i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare ( "pH" ) == 0 )
			{
				m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel, m_PH );
				return 1;
			}
		}
	}
	// Not found PH:
#ifdef USE_MPI_GEMS
	if ( myrank == 0 /*should be set to root*/ )
#endif
		//	DisplayErrorMsg ( "Error: PH NOT FOUND in MT part!!" );
		return 0;
}

short REACT_GEM::SetPeValue_MT ( long node_Index, int timelevel, double m_PE )
{
	int i;
	string str;
	for ( i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare ( "pe" ) == 0 )
			{
				m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel, m_PE );
				return 1;
			}
		}
	}
	// Not found PE:
#ifdef USE_MPI_GEMS
	if ( myrank == 0 /*should be set to root*/ )
#endif
		//	DisplayErrorMsg ( "Error: PE NOT FOUND in MT part!!" );
		return 0;
}

short REACT_GEM::SetEhValue_MT ( long node_Index, int timelevel, double m_EH )
{
	int i;
	string str;
	for ( i=0; i < ( int ) pcs_vector.size() ; i++ )
	{
		m_pcs = pcs_vector[i];
		if ( m_pcs->pcs_type_name.compare ( "MASS_TRANSPORT" ) == 0 )
		{
			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			//x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( str.compare ( "Eh" ) == 0 )
			{
				m_pcs->SetNodeValue ( node_Index,m_pcs->GetNodeValueIndex ( str ) +timelevel,m_EH );
				return 1;
			}
		}
	}
	// Not found PE:
#ifdef USE_MPI_GEMS
	if ( myrank == 0 /*should be set to root*/ )
#endif
		//	DisplayErrorMsg ( "Error: Eh NOT FOUND in MT part!!" );
		return 0;
}

double REACT_GEM::Pressure_Pa_2_Bar ( double Pre_in_Pa )
{
	return Pre_in_Pa / 1.0e+5;
}

double REACT_GEM::Pressure_Bar_2_Pa ( double Pre_in_Bar )
{
	return Pre_in_Bar * 1.0e+5;
}

double REACT_GEM::Pressure_M_2_Bar ( double Pre_in_M, double flu_density )
{return Pre_in_M * 9.81 * flu_density/1.0e+5  ;}

double REACT_GEM::Pressure_Bar_2_M ( double Pre_in_Bar, double flu_density )
{return Pre_in_Bar * 1.0e5 / 9.81 / flu_density ;}

double REACT_GEM::Pressure_M_2_Pa ( double Pre_in_M, double flu_density )
{return Pre_in_M * 9.81 * flu_density  ;}

double REACT_GEM::Pressure_Pa_2_M ( double Pre_in_Pa, double flu_density )
{return Pre_in_Pa / 9.81 / flu_density ;}

double REACT_GEM::GetNodeAdjacentVolume ( long Idx_Node )
{
	double volume;
	long Idx_Ele;
	int number_of_nodes;
	volume = 0.0;
	number_of_nodes = 0;

	CNode* m_Node;
	CElem* m_Elem;

	// get the pointer to current node;
	m_Node =  m_pcs->m_msh->nod_vector[Idx_Node];

	// loop over all the elements that adjacent to this node;
	for ( int i=0 ; i < ( long ) m_Node->connected_elements.size() ; i++ )
	{
		// get the index of current element;
		Idx_Ele = m_Node->connected_elements[i];

		// get the pointer of this element;
		m_Elem = m_pcs->m_msh->ele_vector[Idx_Ele];

		// get the number of nodes in this element;
		// given argument "false" means giving node number instead of Gauss points;
		number_of_nodes = m_Elem->GetNodesNumber ( false );

		// taking part of volume from this element;
		volume += m_Elem->GetVolume() / number_of_nodes ;
	}

	return volume;
}



// i_timestep: 0: old timestep 1: new timestep
void REACT_GEM::ConvPorosityNodeValue2Elem ( int i_timestep )
{
	long i,idx_Node;
	int j, number_of_nodes;
	double pormin=2.0,pormax=0.0;
	CNode* m_Node;
	CElem* m_Elem;

	for ( i=0 ; i < nElems ; i++ )
	{
		m_Elem =  m_pcs->m_msh->ele_vector[i];

		// first set the parameters to zero;
		m_porosity_Elem[i] = 0.0;

		// then get the values from nodes
		for ( j = 0 ; j < m_Elem->GetNodesNumber ( false ) ; j++ )
		{
			idx_Node = m_Elem->GetNodeIndex ( j ); // get the connected nodes;
			m_Node = m_pcs->m_msh->nod_vector[idx_Node];
			number_of_nodes = ( int ) m_Elem->GetNodesNumber ( false );
			// m_porosity_Elem[i] += m_porosity[idx_Node] / number_of_nodes; // this is arithmetric mean
			// here we use harmonic mean, as porosity is used for permeability/diffusivity changes....flux in the element is strongly influenced by the minimum values
			// cout << " porosity " << idx_Node <<" "<<m_porosity[idx_Node] << endl;
			m_porosity_Elem[i] += 1.0/m_porosity[idx_Node] / number_of_nodes; // this is for harmonic mean
		}
//	cout << " Porosity: Element "<< i << " " <<m_porosity_Elem[i] << endl;

		m_porosity_Elem[i] = 1.0/m_porosity_Elem[i];
		if ( m_porosity_Elem[i] > 1.0 ) m_porosity_Elem[i]=1.0; // cout << "error in ConvPorosity" << endl;


		pormin=min ( pormin,m_porosity_Elem[i] );
		pormax=max ( pormax,m_porosity_Elem[i] );


		// push back porosities
		SetPorosityValue_MT ( i , m_porosity_Elem[i] , i_timestep );
	}
	cout <<"min, max porosity: "<< pormin << " " << pormax <<endl;
}

void REACT_GEM::CalcPorosity ( long in )
{
	int k; //must be int for Ph_Volume !
	m_porosity[in]=0.0;
	for ( k=0;k<dCH->nPHb;k++ )
	{
		if ( (dCH->ccPH[k] == 's') || (dCH->ccPH[k] == 'x') || (dCH->ccPH[k] == 'd')) m_porosity[in] += m_Node->Ph_Volume ( k ) ;
		// * 1.0e-6;    // transform cm3 -> m3 !!
		// second not here in the loop. move to the place where divided by m_Vs.
	}

	// normalized by m_Vs
//	m_porosity[in] = 1.0 - m_porosity[in] / ( m_Vs[in] * 1.0e-6 /*convert to cm3 here*/) ;
	m_porosity[in] = 1.0 - ( m_porosity[in] ) ; //kg44 this is the correct way to do it
//	cout <<" porosity:" << m_porosity[in] << " node: "<< in <<endl;
	// checking whether out of bounary.
	if ( m_porosity[in] >= max_possible_porosity ) m_porosity[in]=max_possible_porosity; // upper limit of porosity
	if ( m_porosity[in] <= min_possible_porosity ) m_porosity[in]=min_possible_porosity; //lower limit of porosity..

//	cout <<" porosity:" << m_porosity[in] << " node: "<< in <<" Vs "<<m_Vs[in]<<endl;

}

void REACT_GEM::CalcPorosity ( long in,double volume ) //overload variant
{
	long k;
	m_porosity[in]=0.0;
	for ( k=0;k<dCH->nPHb;k++ )
	{
		if ( (dCH->ccPH[k] == 's') || (dCH->ccPH[k] == 'x') || (dCH->ccPH[k] == 'd')) m_porosity[in] += m_Node->Ph_Volume ( k ) ;
	}
	// * 1.0e-6;    // transform cm3 -> m3 !!
	// second not here in the loop. move to the place where divided by m_Vs.


	// normalized by m_Vs
//	m_porosity[in] = 1.0 - m_porosity[in] / ( m_Vs[in] * 1.0e-6 /*convert to cm3 here*/) ;
	m_porosity[in] = 1.0 - ( m_porosity[in] ) /volume ;  //kg44 this is the correct way to to it
//	cout <<" porosity:" << m_porosity[in] << " node: "<< in <<endl;
	// checking whether out of bounary.
	if ( m_porosity[in] > max_possible_porosity ) {cout << "m_porosity[in] " << m_porosity[in]<< endl; m_porosity[in]=max_possible_porosity;  }    // upper limit of porosity
	if ( m_porosity[in] < min_possible_porosity ) {cout << "lower limit m_porosity[in] " << m_porosity[in]<< " at node "<< in << endl;m_porosity[in]=min_possible_porosity; }//lower limit of porosity..

//	cout <<" porosity:" << m_porosity[in] << " node: "<< in <<" Vs "<<m_Vs[in]<<endl;
}



void REACT_GEM::CalcExcessWater ( long in )
{
	int k,idx;
	// This works only for fully saturated conditions.
	// idea: compare overall volume with standard volume of "1"
	//       and scale water volume according to the "excess porosity"
	// kg44: added support for unsaturated flow .....now also exess gas volume is calculated

	double gas_volume,fluid_volume/*,molvolume*/;

	// get the fluid volume
	fluid_volume=0.0; gas_volume=0.0;
	for ( k=0;k<dCH->nPHb;k++ )
	{
		if ( dCH->ccPH[k] == 'a' ) fluid_volume += m_Node->Ph_Volume ( k );
		if ( dCH->ccPH[k] == 'g' ) gas_volume += m_Node->Ph_Volume ( k );

		// * 1.0e-6;    // transform cm3 -> m3 !!
	}
	// fluid_volume *=1.0e-6;
	// gas_volume *=1.0e-6;
// cout <<"calcExcessWater water volume " << fluid_volume <<" porosity "<<m_porosity[in]<<" node " << in<< endl;
	if ( ( fluid_volume <0.0 ) ) {cout <<"fluid volume negative" << fluid_volume  << " node " << in <<endl; exit ( 1 );}


	if ( ( gas_volume <0.0 ) ) {cout <<"gas volume negative" << gas_volume  << " node " << in <<endl; exit ( 1 );}
	// calculate excess water: volume of fluid phase bigger/smaller than porosity;
	// requires updated porosity (calc porosity before calculation of execc water
	CRFProcess* m_pcs = NULL;
	GetFluidProperty_MT();
	switch ( flowflag )
	{
		case 1: // groundwater flow
			m_excess_water[in] = fluid_volume- m_porosity[in];
			if ( fluid_volume > 0.0 )
			{
				for ( k=0;k<dCH->nDC;k++ )
				{
					if ( ( dCH->ccDC[k] == DC_AQ_PROTON ) || ( dCH->ccDC[k] == DC_AQ_SPECIES ) || ( dCH->ccDC[k] == DC_AQ_SOLVENT ) || ( dCH->ccDC[k] == DC_AQ_SOLVCOM ) ) m_xDC[in*nDC + k] *= m_porosity[in]/fluid_volume;
					// * 1.0e-6;    // transform cm3 -> m3 !!
				}

			}
			else {m_xDC[in*nDC + idx_water] = 0.0;}

			break;
		case 2:  // liquid flow
			m_excess_water[in] = fluid_volume- m_porosity[in];
			if ( fluid_volume > 0.0 )
			{
				m_xDC[in*nDC + idx_water] *= m_porosity[in]/fluid_volume;
			}
			else {m_xDC[in*nDC + idx_water] = 0.0;}
			break;
		case 3: // Richards flow
			m_pcs = PCSGet ( "RICHARDS_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION1" );
			m_excess_water[in] = fluid_volume- m_porosity[in]*m_pcs->GetNodeValue ( in,idx+1 );
			m_excess_gas[in] = gas_volume- m_porosity[in]* ( 1.0-m_pcs->GetNodeValue ( in,idx+1 ) );
			if ( fluid_volume > 0.0 )
			{
				m_xDC[in*nDC + idx_water] *= m_pcs->GetNodeValue ( in,idx+1 ) * m_porosity[in]/fluid_volume;
			}
			else {m_xDC[in*nDC + idx_water] = 0.0;}


			break;
		case 4: // multiphase flow...works with case 1 ...pressure saturation scheme
			m_pcs = PCSGet ( "MULTI_PHASE_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION0" );
			m_excess_water[in] = fluid_volume- m_porosity[in]*m_pcs->GetNodeValue ( in,idx+1 );
			m_excess_gas[in] = gas_volume- m_porosity[in]* ( 1.0-m_pcs->GetNodeValue ( in,idx+1 ) );
			if ( fluid_volume > 0.0 )
			{
				m_xDC[in*nDC + idx_water] *= m_pcs->GetNodeValue ( in,idx+1 ) * m_porosity[in]/fluid_volume;
			}
			else {m_xDC[in*nDC + idx_water] = 0.0;}


			break;
		default:
#ifdef USE_MPI_GEMS
			if ( myrank == 0 /*should be set to root*/ )
#endif
				cout<< "Error: Not implemented for the flow in GEM case!!!" <<endl;
			break;
	}
#ifdef USE_MPI_GEMS
//	if ( fabs ( m_excess_water_buff[in] ) >= 0.01 ) cout << "node "<< in <<" m_excess_water" << m_excess_water_buff[in] <<endl;
//	if ( fabs ( m_excess_gas_buff[in] ) >= 0.01 ) cout << "node "<< in <<" m_excess_gas" << m_excess_water_buff[in] <<endl;
#else
//	if ( fabs ( m_excess_water[in] ) >= 0.01 ) cout << "node "<< in <<" m_excess_water " << m_excess_water[in] <<endl;
//	if ( fabs ( m_excess_gas[in] ) >= 0.01 ) cout << "node "<< in <<" m_excess_gas " << m_excess_water[in] <<endl;
#endif
	// if positive, then source, otherwise sink.
	// change amount of fluid .... at the moment only water!!!!!!


	// if ( fabs ( m_excess_water[in] ) >= 1.e-10 ) cout << "node "<< in <<"m_excess_water" << m_excess_water[in] <<endl;
	// do we need to scale this accoring to the node volume?..I think so: it is done while transering this to GEMS


}

void REACT_GEM::MassToConcentration ( long l /*idx of node*/ ,int i_timestep )
{
	// converting the value from moles to the value in mol/m^3 water.
	long i,j;
	int k,idx;
	double water_volume;

// do not use water volume..use volume of fluid phase...that should be already corrected for temperature & pressure effects
// use porosity for calculation of volume only if coupling to hydrology is not implementdd, as the excess water is used to calculate "correct" concentrations otherwise mass balance for the species is wrong if coupling to hydraulics is implemented via source terms kg44 28072009 Ichanged this: concentrations are now corrected in the calc_exess_water routine as it is always run even if coupling to hydrology is not done

//	water_volume=0.0;
//	if ( flag_coupling_hydrology )
//	{
//		for ( k=0;k<dCH->nPHb;k++ )
//		{
//			if ( dCH->ccPH[k] == 'a' ) water_volume += m_Node->Ph_Volume ( k ) ;
	// * 1.0e-6;    // transform cm3 -> m3 !!
//		}
//		water_volume *= 1.e-6;
	//cout <<"masstoconc water volume " << water_volume <<" node " << l<< endl;
//		if ( ( water_volume <= min_possible_porosity ) ) { cout <<"masstoconc water volume " << water_volume << " node " << l << endl; water_volume=min_possible_porosity;}
//		if ( water_volume>= ( 1.0 ) ) { cout <<"masstoconc water volume " << water_volume << " node " << l << endl; water_volume=1.0;}
//	}
//	else
//	{
	CRFProcess* m_pcs = NULL;
	GetFluidProperty_MT();
	switch ( flowflag )
	{
		case 1: // groundwater flow
			water_volume = m_porosity[l];
			break;
		case 2:  // liquid flow
			water_volume = m_porosity[l];
			break;
		case 3: // Richards flow
			m_pcs = PCSGet ( "RICHARDS_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION1" );
			water_volume = m_porosity[l]*m_pcs->GetNodeValue ( l,idx+i_timestep );
//				cout << "water volume " << water_volume << " saturation " << m_pcs->GetNodeValue ( l,idx+0 ) << endl;

			break;
		case 4: // multiphase flow...works with case 1 ...pressure saturation scheme
			m_pcs = PCSGet ( "MULTI_PHASE_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION0" );
			water_volume = m_porosity[l]*m_pcs->GetNodeValue ( l,idx+i_timestep );
			break;
		default:
#ifdef USE_MPI_GEMS
//				if ( myrank == 0 /*should be set to root*/ )
#endif
			cout<< "Error: Not implemented for the flow in GEM case!!!" << endl;
			break;
	}
//	}
	// here we do not need to account for different flow processes .... as long as we work only with one wetting phase (water!!!) ...for liqid CO2 we have to change something ....
	//****************************************************************************************
	for ( j=0 ; j < nDC; j++ )
	{
		i = l*nDC + j;
		// a better way to do would be for all mobile species...or?
		if ( ( dCH->ccDC[j] == 'S' ) || ( dCH->ccDC[j] == 'E' ) || ( dCH->ccDC[j] == 'T' ) || ( dCH->ccDC[j] == 'L' ) )
		{
			// only for solutions species in the water. and H+ as well as electron
			m_xDC[i] /= water_volume ;
		}
		// else
		//  { cout << "Error calculating node volume " <<endl; }

		// also check if xDC values is negative
		if ( m_xDC[i] < 0.0 )
			m_xDC[i]=0.0;
		// cout << "mass2conc: l " << l << " j "<< j << " i " << i << " por " << m_porosity[l] << " m_xDC " <<  m_xDC[i] << endl;
	}
	return;
}


void REACT_GEM::ConcentrationToMass ( long l /*idx of node*/, int i_timestep )
{
	// converting the value from mol/m^3 water to moles.
	long i,j;
	double water_volume;
	int idx;
	string ErrorOut;
	// get the water volume

	//	cout <<"water volume " << water_volume << endl;
	// I think here it has to be different than in MassToConcentration module, as after hydraulics, the volume is limited to porosity
	// As we work with an unit volume of "one", it should be save to directly take porosity as volume....
	// of course we have to differenciate between the different flow models
	CRFProcess* m_pcs = NULL;
	GetFluidProperty_MT();
	switch ( flowflag )
	{
		case 1: // groundwater flow
			water_volume=m_porosity[l];  // kg44: better get porosity from other processes (in case porosity is changed by other processes)!!!!!!!!!
			break;
		case 2:  // liquid flow
			water_volume=m_porosity[l];
			break;
		case 3: // Richards flow
			m_pcs = PCSGet ( "RICHARDS_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION1" );
			water_volume=m_porosity[l]*m_pcs->GetNodeValue ( l,idx+i_timestep );
			break;
		case 4: // multiphase flow...works with case 1 ...pressure saturation scheme
			m_pcs = PCSGet ( "MULTI_PHASE_FLOW" );
			idx=m_pcs->GetNodeValueIndex ( "SATURATION0" );
			water_volume=m_porosity[l]*m_pcs->GetNodeValue ( l,idx+i_timestep );
			break;
		default:
#ifdef USE_MPI_GEMS
			if ( myrank == 0 /*should be set to root*/ )
#endif
				cout<<"Error: Not implemented for the flow in GEM case!!!"<<endl;
			break;
	}


	if ( ( water_volume < 1.e-6 ) || ( water_volume > 1.0 ) ) {cout <<"conctomass water volume " << water_volume << " at node: "<< l << endl; exit ( 1 );}

	for ( j=0 ; j < nDC; j++ )
	{
		i = l*nDC + j;

		if ( ( dCH->ccDC[j] == 'S' ) || ( dCH->ccDC[j] == 'E' ) || ( dCH->ccDC[j] == 'T' ) || ( dCH->ccDC[j] == 'L' ) )
		{
			// only for solutions species in the water, as well as H+ and electrons
			m_xDC[i] *= water_volume ;
		}

		// also check if xDC values is negative
		if ( m_xDC[i] < 0.0 )
			m_xDC[i] = fabs ( m_xDC[i] );  //kg44 test if taking absolute value is more robust than setting component to zero!!!
		// cout << "conc2mass: l " << l << " j "<< j << " i " << i << " por " << m_porosity[l] << " m_xDC " <<  m_xDC[i] << endl;
	}
}

void REACT_GEM::CopyCurXDCPre ( void )
{
	long i;
	for ( i=0 ; i < nNodes*nDC ; i++ )
	{
		m_xDC_pts[i] = m_xDC[i];
	}
}

void REACT_GEM::UpdateXDCChemDelta ( void )
{
	long i;
	for ( i=0 ; i < nNodes*nDC ; i++ )
	{
		m_xDC_Chem_delta[i] = m_xDC[i] - m_xDC_pts[i];
	}
}


// retrun the new permeability based on original permeability and old/new porosity
double REACT_GEM::KozenyCarman ( double k0, double n0, double n )
{
	double rt = 0.0;

	if ( k0 < 1.0e-20 || k0 > 1.0 || n0 <=0 || n0 >= 1 || n <=0 || n >= 1 )
		return 0.0;
	else
	{
		rt = k0 ;

		rt *=pow ( n / n0 , 3 );
	}

	return rt;
}


// retrun the new permeability based on original permeability and old/new porosity

double REACT_GEM::KozenyCarman_normalized ( double k0, double n0, double n )
{
	double rt = 0.0;

	if ( k0 < 1.0e-20 || k0 > 1.0 || n0 <=0 || n0 >= 1 || n <=0 || n >= 1 )
		return 0.0;
	else
	{
		rt = k0 ;

		rt *=pow ( n / n0 , 3 );

		rt *=pow ( ( 1 - n0 ) / ( 1 - n ) , 2 );
	}

	return rt;
}

bool GEMRead ( string base_file_name, REACT_GEM *m_GEM_p )
{
	cout << "GEMRead" << endl;
	char line[MAX_ZEILE];
	string sub_line;
	string line_string;
	ios::pos_type position;
	//========================================================================
	// file handling
	string gem_file_name;
	gem_file_name = base_file_name + GEM_FILE_EXTENSION;
	ifstream gem_file ( gem_file_name.data(),ios::in );
	if ( !gem_file.good() )
	{
		cout << "! Error in GEMRead: No GEM data !" << endl;
		return false;
	}
	gem_file.seekg ( 0L,ios::beg );
	//========================================================================
	// keyword loop
	while ( !gem_file.eof() )
	{
		gem_file.getline ( line,MAX_ZEILE );
		line_string = line;
		if ( line_string.find ( "#STOP" ) !=string::npos )
			return true;
		//----------------------------------------------------------------------
		if ( line_string.find ( "#GEM_PROPERTIES" ) !=string::npos )   // keyword found
		{
			position = m_GEM_p->Read ( &gem_file );
			gem_file.seekg ( position,ios::beg );
		} // keyword found
	} // eof
	return true;
}

ios::pos_type REACT_GEM::Read ( std::ifstream *gem_file )
{
	// Initialization----------------
	string sub_line;
	string line_string;
	string delimiter ( " " );
	bool new_keyword = false;
	string hash ( "#" );
	ios::pos_type position;
	string sub_string;
	bool new_subkeyword = false;
	string dollar ( "$" );
	string delimiter_type ( ":" );
	std::stringstream in;
	// ------------------------------

	// Loop over all the key words----------------------
	while ( !new_keyword )
	{
		new_subkeyword = false;
		position = gem_file->tellg();
		line_string = GetLineFromFile1 ( gem_file );
		if ( line_string.size() < 1 ) break;
		if ( line_string.find ( hash ) !=string::npos )
		{
			new_keyword = true;
			break;
		}
		// Key word "$GEM_INIT_FILE" .......................
		if ( line_string.find ( "$GEM_INIT_FILE" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> init_input_file_path;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$FLAG_POROSITY_CHANGE" .......................
		if ( line_string.find ( "$FLAG_POROSITY_CHANGE" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> flag_porosity_change;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$MIN_POROSITY" .............................
		if ( line_string.find ( "$MIN_POROSITY" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> min_possible_porosity;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$MAX_POROSITY" .............................
		if ( line_string.find ( "$MAX_POROSITY" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> max_possible_porosity;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$FLAG_COUPLING_HYDROLOGY" ..................
		if ( line_string.find ( "$FLAG_COUPLING_HYDROLOGY" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> flag_coupling_hydrology;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$PERMEABILITY_POROSITY_MODEL" ..............
		if ( line_string.find ( "$PERMEABILITY_POROSITY_MODEL" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> flag_permeability_porosity;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$ITERATIVE_SCHEME" .........................
		if ( line_string.find ( "$ITERATIVE_SCHEME" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> flag_iterative_scheme;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$GEM_SIMPLEX" .........................
		if ( line_string.find ( "$GEM_SMART" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> flag_gem_smart;
			in.clear();
			continue;
		}
		// ......................................................
		// ......................................................
		// Key word "$ITERATIVE_SCHEME" .........................
		if ( line_string.find ( "$TEMPERATURE_GEM" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> m_gem_temperature;
			in.clear();
			continue;
		}
		// Key word "$ITERATIVE_SCHEME" .........................
		if ( line_string.find ( "$PRESSURE_GEM" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> m_gem_pressure;
			in.clear();
			continue;
		}
		// ......................................................
		// Key word "$INDEX_SORT_MPI"   only for MPI: every M_IterDoneIndexSort the NodeIndex is sorted according to gems iterations .........................
		if ( line_string.find ( "$INDEX_SORT_MPI" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> m_IterDoneIndexSort;
			in.clear();
			continue;
		}
		// ......................................................
		// ......................................................
		// Key word "$SHUFFLE_GEMS" randomize the order of gems executions to exclude effects due to small systematic changes .........................
		if ( line_string.find ( "$SHUFFLE_GEMS" ) !=string::npos )
		{
			// subkeyword found
			in.str ( GetLineFromFile1 ( gem_file ) );
			in >> m_ShuffleGems;
			in.clear();
			continue;
		}
		// ......................................................
	}
	// End of looping over all the key words-----------
	return position;
}


// calculate the saturation index for dependent component in at pressure press and temperature temp
// test if the routine works: for all existing solids the saturation index should be  = 1 ("one")
// kg44 24.11.2008
// after GEMS update int has to be replace by long
double REACT_GEM::CalcSaturationIndex ( long j, long node,double temp, double press )
{
	double saturation;
	long ii;

// datach.h
//    *A;      // Stoichiometry matrix A containing elemental stoichiometries
	// of Dependent Components, [nIC][nDC] elements
// databr.h
//    *uIC,  // IC chemical potentials (mol/mol)[nICb]       -      -      +     +
// is m_uIC in my case??

// molar gas constant R = 8.31451070 J K-1 mol-1

//node.cpp: // Returns the (interpolated) G0 value for Tc, P from the DCH structure in J/mol
//node.cpp:  double  TNode::DC_G0_TP( const int xCH, double Tc, double P )



	saturation =0.0;
	for ( ii=0; ii<dCH->nICb; ii++ )
	{
//	   cout << ii << " " << in << " " << nodeCH_A( in, ii ) << endl;
//**************************************************************************************************************
// attention: only removed because nodeCH_a is not anymore in node.cpp		saturation += nodeCH_A ( j, ii ) *m_uIC[node*nIC+ii];
//**************************************************************************************************************
	}
//	cout << saturation << " " << m_Node->DC_G0_TP(j,tempC,press)<< endl;
	saturation -= m_Node->DC_G0 ( j,temp,press,false ) / ( 8.31451070*temp ); // get standard molar gibbs energy function with units J/mol (flag =false!)  molar gas konstant has units J/(mol K)
//	cout << saturation << endl;
	saturation = exp ( saturation );

	return saturation;
}

// calculate Reaction rates at a node in for all solids (for which kinetics is defined)!
// kg44 30.07.2009
// in: node temp: temperature press: Pressure
void REACT_GEM::CalcReactionRate ( long in, double temp, double press )
{

	int idx,i;
	long j,k,dc_counter;
	double rrn=0.0, rrb=0.0,rra=0.0, sa=0.0;
	double R=8.31451070; // molar gas konstant [J K-1 mol-1]
	double omega; // saturation index
	double aa=1.0,ab=1.0,ac=1.0;  // activity products ...species are input from material file
	double dmdt;
	double *mol_phase, *omega_phase; // this we need for the phases
	double sactivity; // dummy variable for extracting activities
	const char *species;

	CompProperties *m_cp = NULL;


// from rfmat_cp.h
//	int kinetic_model;  // only 1 = GEMS implemented right now
//        int n_activities;  // number of species for activities
//        string active_species[10];  // name for species ...maximum 10 names
//	double kinetic_parameters[32];
//	0,1,2  double E_acid,E_neutral,E_base; // activation energies
//      3-5  double k_acid, k_neutral,k_base; // dissolution/precipitation rate constants at standart konditions
//      6-11  double p1,q1,p2,q2,p3,q3; // exponents for omega
//      12,13,14  double n_1, n_2,n_3; // exponents for acidic neutral and base cases for species one
//      append for each species another set of n_1, n_2  n_3 (up to 10 sets -> up to ten species)



// loop over all phases and get rate for each phase ...
// this algorithm assumes that we have correct ordering of phases and components that belong into the phase

	omega_phase = new double [dCH->nPH]; // deleted at the end of the function
	mol_phase = new double [dCH->nPH]; // deleted at the end of the function

	dc_counter=0; // set counter to zero
	for ( k=0;k<dCH->nPHb;k++ )
	{
		m_cp = cp_vec[dc_counter];
		if ( m_cp->kinetic_model > 0 )  		// do it only if kinetic model is defined take model
		{
			mol_phase[k]=0.0;
			omega_phase[k]=0.0;

			for ( j=dc_counter;j<dc_counter+dCH->nDCinPH[k];j++ )
			{
//				omega_phase[k] += CalcSaturationIndex ( j, in,tempC,press ); // loop over all components of the phase
				omega_phase[k] += m_Node->DC_a ( j ); // loop over all components of the phase

				mol_phase[k] += m_xDC[in*nDC+j];
			}

			// cout << omega_phase[k] << " " << m_Node->Ph_SatInd ( k ) << endl; // check if the new call to saturation index gives same results


			sa=SurfaceAreaPh ( k,dc_counter ); // value for surface area in m^2...(specific surface area multiplied with volume of the phase)

			for ( i=0;i<m_cp->n_activities;i++ )
			{
				species=m_cp->active_species[i].c_str();
				idx= m_Node-> DC_name_to_xCH ( species ); // loop over all the names in the list
				if ( idx < 0 ) {cout << "CalcReactionRate: no DC-name "<< m_cp-> active_species[i] <<" found" << endl; exit ( 1 );}
				else
				{
					sactivity=m_Node->DC_a ( idx ); // extract activities (not activity coefficients!)
					// cout << "Activity " << sactivity << pow(10.0,sactivity)<< endl;
					aa *= pow ( sactivity,m_cp->kinetic_parameters[12+i] );
					ab *= pow ( sactivity,m_cp->kinetic_parameters[13+i] );
					ac *= pow ( sactivity,m_cp->kinetic_parameters[14+i] );
					//    *Y_m,     // Molalities of aqueous species and sorbates [0:Ls-1]
					//    *Y_la,    // log activity of DC in multi-component phases (mju-mji0) [0:Ls-1]
					//    *Y_w,     // Mass concentrations of DC in multi-component phases,%(ppm)[Ls]
					//    *Gamma,   // DC activity coefficients in molal or other phase-specific scale [0:L-1]
				}
			}
			//			  cout << "activities " <<aa << " " << ab << " " << ac <<" " << temp << endl;
			// terms for each case
			rra= exp ( -1.0* m_cp->kinetic_parameters[0]/R * ( 1.0/temp + 1.0/298.15 ) ) * aa * pow ( ( 1.0-pow ( omega_phase[k],m_cp->kinetic_parameters[6] ) ),m_cp->kinetic_parameters[7] );

			rrn= exp ( -1.0*m_cp->kinetic_parameters[1]/R * ( 1.0/temp + 1.0/298.15 ) ) * ab * pow ( ( 1.0-pow ( omega_phase[k],m_cp->kinetic_parameters[8] ) ),m_cp->kinetic_parameters[9] );

			rrb= exp ( -1.0*m_cp->kinetic_parameters[2]/R * ( 1.0/temp + 1.0/298.15 ) ) * ac * pow ( ( 1.0-pow ( omega_phase[k],m_cp->kinetic_parameters[10] ) ),m_cp->kinetic_parameters[11] );

			dmdt= -1.0 * sa * ( pow ( 10.0,m_cp->kinetic_parameters[3] ) * rra + pow ( 10.0,m_cp->kinetic_parameters[4] ) * rrn + pow ( 10.0,m_cp->kinetic_parameters[5] ) * rrb ); // rate is scaled to the total amount available via the surface area



// test for NaN!! ---seems necessary as sometimes rra, rrn, rrb get Inf! ---seems enough to test the upper limit---this test does not resolve the real problem ;-)...probably pow(0.0,0.0) for rra,rrn,rrb ?
			if ( ! ( dmdt<=1.0 ) && ! ( dmdt>1.0 ) )
			{
				cout << "dmdt " <<dmdt << " is NaN " << " sa "<< sa <<" rra "<< rra << " rrn "<< rrn << " rrb "<< rrb << " m_gam " << m_gam[idx]<< endl;
				dmdt= 0.0;// no change!
			}


			for ( j=dc_counter;j<dc_counter+dCH->nDCinPH[k];j++ )
			{
				if ( omega_phase[k] > 1.0001 )
				{
					m_dul[in*nDC+j]= ( mol_phase[k] + dmdt*dt ) * m_Node->DC_a ( j ) /omega_phase[k] ;
					if ( ! ( m_dul[in*nDC+j]<=1.0 ) && ! ( m_dul[in*nDC+j]>1.0 ) )
					{

						m_dul[in*nDC+j]= m_xDC[in*nDC+j];// no change!
					}
					m_dll[in*nDC+j]= m_xDC[in*nDC+j];
//					m_dll[in*nDC+j]= m_dul[in*nDC+j];

				}
				else if ( omega_phase[k] < 0.9999 )
				{
					m_dll[in*nDC+j]= ( mol_phase[k] + dmdt*dt ) * m_Node->DC_a ( j ) /omega_phase[k] ;
					if ( ! ( m_dll[in*nDC+j]<=1.0 ) && ! ( m_dll[in*nDC+j]>1.0 ) )
					{

						m_dll[in*nDC+j]= m_xDC[in*nDC+j];// no change!
					}
					m_dul[in*nDC+j]= m_xDC[in*nDC+j];
//					m_dul[in*nDC+j]= m_dll[in*nDC+j];

				}
				else
				{
					m_dul[in*nDC+j]= m_xDC[in*nDC+j];
					m_dll[in*nDC+j]= m_xDC[in*nDC+j];
				}
				// do some corrections

				if ( ( m_xDC[in*nDC+j] < 1.0e-6 ) && ( omega_phase[k] >=1.0001 ) && ( m_dul[in*nDC+j]<1.0e-6 ) ) { m_dul[in*nDC+j]=1.0e-6; m_dll[in*nDC+j]=m_dul[in*nDC+j]; }// allow some kind of precipitation...based on saturation index for component value...here we set 10-6 mol per m^3 ..which is maybe 10-10 per litre ...?
				if ( m_dll[in*nDC+j] < 0.0 ) m_dll[in*nDC+j]=0.0; // no negative masses allowed
				if ( m_dul[in*nDC+j] < 0.0 ) m_dul[in*nDC+j]=0.0; // no negative masses allowed
				if ( m_dll[in*nDC+j] > m_dul[in*nDC+j] ) m_dll[in*nDC+j]=m_dul[in*nDC+j];
			}


		} //end if for kinetic model >1
		dc_counter +=dCH->nDCinPH[k]; // this gives the number of components until here
	}


	delete [] mol_phase;
    delete [] omega_phase;
	return ;
}




// simplest case....scaling with a specific surface area per volume mineral
// phasenr: index for phase compnr: index of component which belongs to the phase and for which a specific surface area is defined
double REACT_GEM::SurfaceAreaPh ( long phasenr, long compnr )
{
	double surface_area=0.0;
	CompProperties *m_cp = NULL;

	m_cp = cp_vec[compnr]; // get the correct pointer

	surface_area=m_Node->Ph_Volume ( phasenr ) ; // now it is volume of the phase ..with respect to the unit volume in m^3

	if ( m_cp->surface_model == 1 )
	{
		surface_area *= m_cp->surface_area[0]; // multiplication with specific surface area gives area
	}
	else if ( m_cp->surface_model == 2 )
	{
		surface_area= m_cp->surface_area[0];  // constant surface area
	}
	else
	{
		surface_area=0.0;  // no kinetics...safe solution
	}

	return surface_area;
}


#ifdef USE_MPI_GEMS
void REACT_GEM::GetGEMResult_MPI ( void )
{
	// Now gather the calculated values------------------------------------------------------------------------------
	MPI_Allreduce ( m_NodeHandle_buff, m_NodeHandle, nNodes, MPI_LONG, MPI_SUM , MPI_COMM_WORLD );
	MPI_Allreduce ( m_NodeStatusCH_buff, m_NodeStatusCH, nNodes, MPI_LONG, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_IterDone_buff, m_IterDone, nNodes, MPI_LONG, MPI_SUM, MPI_COMM_WORLD );

//	MPI_Allreduce ( m_Vs_buff, m_Vs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_Ms_buff, m_Ms, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_Gs_buff, m_Gs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_Hs_buff, m_Hs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_IC_buff, m_IC, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_pH_buff, m_pH, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_pe_buff, m_pe, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_Eh_buff, m_Eh, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_porosity_buff, m_porosity, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_excess_water_buff, m_excess_water, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_excess_gas_buff, m_excess_gas, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );



	MPI_Allreduce ( m_bIC_buff, m_bIC, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_bIC_dummy_buff, m_bIC_dummy, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

	MPI_Allreduce ( m_rMB_buff, m_rMB, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_uIC_buff, m_uIC, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_xDC_buff, m_xDC, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_gam_buff, m_gam, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_dll_buff, m_dll, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_dul_buff, m_dul, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_aPH_buff, m_aPH, nNodes*nPH, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_xPH_buff, m_xPH, nNodes*nPH, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_vPS_buff, m_vPS, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_mPS_buff, m_mPS, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_bPS_buff, m_bPS, nNodes*nIC*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_xPA_buff, m_xPA, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

	MPI_Allreduce ( m_xDC_pts_buff, m_xDC_pts, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_xDC_MT_delta_buff, m_xDC_MT_delta, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
	MPI_Allreduce ( m_xDC_Chem_delta_buff, m_xDC_Chem_delta, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

	// --------------------------------------------------------------------------------------------------------------

}

void REACT_GEM::CleanMPIBuffer ( void )
{

	for ( long in = 0; in < nNodes ; in++ )
	{

		m_IterDoneCumulative[in] += m_IterDone[in];
		m_NodeHandle_buff[in] = 0;
		m_NodeStatusCH_buff[in] = 0;
		m_IterDone_buff[in] = 0;


//			m_Vs_buff[in] = 0.0;
		m_Ms_buff[in] = 0.0;
		m_Gs_buff[in] = 0.0;
		m_Hs_buff[in] = 0.0;
		m_IC_buff[in] = 0.0;
		m_pH_buff[in] = 0.0;
		m_pe_buff[in] = 0.0;
		m_Eh_buff[in] = 0.0;
		m_porosity_buff[in]= 0.0;

		m_excess_water_buff[in] = 0.0;
		m_excess_gas_buff[in] = 0.0;


		int ii;
		for ( ii = 0; ii < nIC ; ii++ )
		{

			m_bIC_buff[in*nIC + ii ] = 0.0;
			m_bIC_dummy_buff [ in*nIC + ii ] = 0.0;
			m_rMB_buff[in*nIC + ii ] = 0.0;
			m_uIC_buff[in*nIC + ii ] = 0.0;
		}

		for ( ii = 0; ii < nDC ; ii++ )
		{

			m_xDC_buff[in*nDC+ii ] = 0.0;
			m_gam_buff[in*nDC+ii ] = 0.0;
			m_dul_buff[in*nDC+ii ] = 0.0;
			m_dll_buff[in*nDC+ii ] = 0.0;
			m_xDC_pts_buff[in*nDC+ii ] = 0.0;
			m_xDC_MT_delta_buff[in*nDC+ii ] = 0.0;
			m_xDC_Chem_delta_buff[in*nDC+ii ] = 0.0;
		}

		for ( ii = 0; ii < nPH ; ii++ )
		{

			m_xPH_buff[in*nPH+ii ] = 0.0;
			m_aPH_buff[in*nPH+ii ] = 0.0;
		}

		for ( ii = 0; ii < nPS ; ii++ )
		{
			m_vPS_buff[in*nPS+ii ] = 0.0;
			m_mPS_buff[in*nPS+ii ] = 0.0;
			m_xPA_buff[in*nPS+ii ] = 0.0;
		}


		for ( ii = 0; ii < nIC ; ii++ )
		{
			for ( int jj = 0; jj < nPS ; jj++ )
			{

				m_bPS_buff[in*nIC*nPS+jj ] = 0.0;
			}
		}

	}

	for ( long in = 0; in < nElems ; in++ )
	{
		m_porosity_Elem_buff[in] = m_porosity_Elem[in];
	}

}

void REACT_GEM::CopyToMPIBuffer ( long in )
{

	m_NodeHandle_buff[in] = m_NodeHandle[in];
	m_NodeStatusCH_buff[in] = m_NodeStatusCH[in];
	m_IterDone_buff[in] = m_IterDone[in];


//			m_Vs_buff[in] = m_Vs[in];
	m_Ms_buff[in] = m_Ms[in];
	m_Gs_buff[in] = m_Gs[in];
	m_Hs_buff[in] = m_Hs[in];
	m_IC_buff[in] = m_IC[in];
	m_pH_buff[in] = m_pH[in];
	m_pe_buff[in] = m_pe[in];
	m_Eh_buff[in] = m_Eh[in];
	m_porosity_buff[in]= m_porosity[in];

	m_excess_water_buff[in] = m_excess_water[in];
	m_excess_gas_buff[in] = m_excess_gas[in];


	int ii;
	for ( ii = 0; ii < nIC ; ii++ )
	{

		m_bIC_buff[in*nIC + ii ] = m_bIC[in*nIC + ii ];
		m_bIC_dummy_buff [ in*nIC + ii ] = m_bIC_dummy[ in*nIC + ii ];
		m_rMB_buff[in*nIC + ii ] = m_rMB[in*nIC + ii ];
		m_uIC_buff[in*nIC + ii ] = m_uIC[in*nIC + ii ];
	}

	for ( ii = 0; ii < nDC ; ii++ )
	{

		m_xDC_buff[in*nDC+ii ] = m_xDC[in*nDC+ii ];
		m_gam_buff[in*nDC+ii ] = m_gam[in*nDC+ii ];
		m_dul_buff[in*nDC+ii ] = m_dul[in*nDC+ii ];
		m_dll_buff[in*nDC+ii ] = m_dll[in*nDC+ii ];
		m_xDC_pts_buff[in*nDC+ii ] = m_xDC_pts[in*nDC+ii ];
		m_xDC_MT_delta_buff[in*nDC+ii ] = m_xDC_MT_delta[in*nDC+ii ];
		m_xDC_Chem_delta_buff[in*nDC+ii ] = m_xDC_Chem_delta[in*nDC+ii ];
	}

	for ( ii = 0; ii < nPH ; ii++ )
	{

		m_xPH_buff[in*nPH+ii ] = m_xPH[in*nPH+ii ];
		m_aPH_buff[in*nPH+ii ] = m_aPH[in*nPH+ii ];
	}

	for ( ii = 0; ii < nPS ; ii++ )
	{
		m_vPS_buff[in*nPS+ii ] = m_vPS[in*nPS+ii ];
		m_mPS_buff[in*nPS+ii ] = m_mPS[in*nPS+ii ];
		m_xPA_buff[in*nPS+ii ] = m_xPA[in*nPS+ii ];
	}


	for ( ii = 0; ii < nIC ; ii++ )
	{
		for ( int jj = 0; jj < nPS ; jj++ )
		{

			m_bPS_buff[in*nIC*nPS+jj ] = m_bPS[in*nIC*nPS+jj ];
		}
	}

}

// this is a "slow" bubblesort, but it gives back the indexes of our sort
void REACT_GEM::SortIterations ( long *iterations, long *indexes, long len )
{
	// Set the index to match the current sort
	// order of the names
	for ( long i = 0; i < len; i++ )
		indexes[i] = i;

	// Bubblesort. Sort the indexes
	for ( long i = 0; i < len; i++ )
	{
		for ( long j = 0; j < len; j++ )
		{
			if ( iterations[indexes[i]] < iterations[indexes[j]] )
			{
				// swap the indexes
				long iTmp = indexes[i];
				indexes[i] = indexes[j];
				indexes[j] = iTmp;
			}
		}
	}
}
#endif  // end MPI



// this is a "shuffle" algorithm that randomizes the order of the indexes
void REACT_GEM::ShuffleIterations ( long* indexes, long len )
{
	vector<long> mydata ( indexes,indexes + len ); //constructor

	//Seeding random number generator
	std::srand ( std::time ( 0 ) );

	//assigning the numbers
	//        for(long i=0;i<len;++i)
	//        mydata[i]=indexes[i];

	//Shuffling the numbers
	std::random_shuffle ( mydata.begin(), mydata.end() ); // using build in random generator

	//Displaying the shuffled numbers
	for ( long i=0;i<len;++i )
	{
		indexes[i]=mydata[i];
		//cout << i << " " <<indexes[i];
	}

}




//taken from rf_REACT_BRNS
int REACT_GEM::IsThisPointBCIfYesStoreValue ( int index, CRFProcess* m_pcs, double& value )
{
	for ( int p=0; p< ( int ) m_pcs->bc_node_value.size(); ++p )
	{
		if ( index == m_pcs->bc_node_value[p]->msh_node_number )
		{
			value = m_pcs->bc_node_value[p]->node_value;
			return 1; // Yes, found it.
		}
	}

	return 0;
}

int REACT_GEM::WriteReloadGem()
{
	int i;

	// node porosity ...necessary for restart
// first test if m_poorosity exists (possible if no gem process exists)
	if ( !m_porosity ) return 2;
	string m_file_name = FileName +"_"+"m_porosity"+"_gem.asc";
	ofstream os ( m_file_name.c_str(), ios::trunc|ios::out );
	if ( !os.good() )
	{
		cout << "Failure to open file: "<<m_file_name << endl;
		abort();
	}
	os.precision ( 15 ); // 15 digits accuracy seems enough? more fields are filled up with random numbers!
	os.setf ( ios_base::scientific,ios_base::floatfield );

	for ( i=0; i<nNodes; i++ )
	{
		os<<m_porosity[i] <<"  ";
		os<<endl;
	}
	os.close();
	return 1;
}
int REACT_GEM::ReadReloadGem()
{
	int i;
	string m_file_name = FileName +"_"+"m_porosity"+"_gem.asc";

	ifstream is ( m_file_name.c_str(), ios::in );
	if ( !is.good() )
	{
		cout << "Failure to open file: "<<m_file_name << endl;
		abort();
	}
	//
	// node porosity ...necessary for restart

	for ( i=0; i<nNodes; i++ )
	{

		is>>m_porosity[i];
		is>>ws;;
	}
	is.close();
	return 1;
}

#endif // end of GEM_REACT
