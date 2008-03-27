//-------------------------------------
// rf_REACT_GEM.cpp
// Haibing Shao 25.03.08
// haibing.shao@ufz.de
// GEM Reaction Package
// based on the PSI node-GEM source code
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------

#include "rf_REACT_GEM.h"
#include "node.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"
#include "msh_node.h"
#include "msh_elem.h"
#ifdef _WIN32
#include "direct.h" // on win32 and win64 platform
#else
#include "unistd.h" // on unix/linux platform
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

REACT_GEM::REACT_GEM(void)
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
   initialized_flag = 0;
   heatflag = 0;
   flowflag = 0;
   flag_node_element_based = 0;//0-node based; 1-elem based;

   flag_iterative_scheme = 0;
   // flag for different iterative scheme
   // 0 - sequential non-iterative scheme
   // 1 - standard iterative scheme
   // 2 - symetric iterative scheme
   // 3 - strang splitting scheme

   mp_nodeTypes = new int;
   *(mp_nodeTypes) = 0;

   m_FluidProp = NULL;
}

REACT_GEM::~REACT_GEM(void)
{
	if ( initialized_flag > 0 )
	{
		if (flag_node_element_based == 0)
		{
			delete [] m_xDC, m_gam,  m_xPH, m_aPH, m_vPS, m_mPS, m_bPS, m_xPA, m_dul,
			 m_dll, m_uIC, m_bIC, m_bIC_dummy, m_rMB, m_xDC_pts, m_xDC_MT_delta, m_xDC_Chem_delta, m_NodeHandle;

			delete [] m_NodeStatusCH, m_IterDone, m_T, m_P, m_Vs, m_Ms, m_Gs, m_Hs, m_IC, m_pH, m_pe, m_Eh;
		}
		else
		{
			delete [] m_xDC_Elem, m_gam_Elem, m_xPH_Elem, m_aPH_Elem, m_vPS_Elem, m_mPS_Elem, m_bPS_Elem,
			  m_xPA_Elem, m_dul_Elem, m_dll_Elem, m_uIC_Elem, m_bIC_Elem, m_bIC_dummy_Elem, m_rMB_Elem, m_xDC_pts_Elem;

			delete [] m_ElemHandle, m_ElemStatusCH, m_IterDone_Elem, m_T_Elem, m_P_Elem, m_Vs_Elem, m_Ms_Elem,
			  m_Gs_Elem, m_Hs_Elem, m_IC_Elem, m_pH_Elem, m_pe_Elem, m_Eh_Elem;
		} 
		// delete MPI buffer--------
		delete [] m_NodeHandle_buff, m_NodeStatusCH_buff, m_IterDone_buff;

		delete [] m_Vs_buff, m_Ms_buff,  m_Gs_buff, m_Hs_buff, m_IC_buff, m_pH_buff, m_pe_buff, m_Eh_buff;

		delete [] m_rMB_buff, m_uIC_buff, m_xDC_buff, m_gam_buff, m_xPH_buff, m_vPS_buff, m_mPS_buff, m_bPS_buff, m_xPA_buff;
		// -------------------------
	}
    delete m_Node;
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

       heatflag = GetHeatFlag_MT();// Get heatflag
       flowflag = GetFlowType_MT();// Get flow flag
	   // Get number of Nodes
       nNodes = GetNodeNumber_MT();
       // Get number of Elems
       nElems = GetElemNumber_MT();

       // Allocating work memory for FMT part (here only chemical variables)
	   m_NodeHandle = new short [nNodes];
	   m_NodeStatusCH = new short [nNodes];
	   m_IterDone = new short [nNodes];

	   // MPI Buffer Variable---------------
	   m_NodeHandle_buff = new short[nNodes];
       m_NodeStatusCH_buff = new short[nNodes];
       m_IterDone_buff = new short[nNodes];

	   m_ElemHandle = new short [nElems];
	   m_ElemStatusCH = new short [nElems];
	   m_IterDone_Elem = new short [nElems];


	   m_T = new double [nNodes];
	   m_P = new double [nNodes];
	   m_Vs = new double [nNodes];
	   m_Ms = new double [nNodes];
	   m_Gs = new double [nNodes];
	   m_Hs = new double [nNodes];
	   m_IC = new double [nNodes];
	   m_pH = new double [nNodes];
	   m_pe = new double [nNodes];
	   m_Eh = new double [nNodes];

	   m_T_Elem = new double [nElems];
	   m_P_Elem = new double [nElems];
	   m_Vs_Elem = new double [nElems];
	   m_Ms_Elem = new double [nElems];
	   m_Gs_Elem = new double [nElems];
	   m_Hs_Elem = new double [nElems];
	   m_IC_Elem = new double [nElems];
	   m_pH_Elem = new double [nElems];
	   m_pe_Elem = new double [nElems];
	   m_Eh_Elem = new double [nElems];

       // MPI Buffer Variable---------------	   
       m_Vs_buff = new double[nNodes];
       m_Ms_buff = new double[nNodes];
       m_Gs_buff = new double[nNodes];
       m_Hs_buff = new double[nNodes];
       m_IC_buff = new double[nNodes];
       m_pH_buff = new double[nNodes];
       m_pe_buff = new double[nNodes];
       m_Eh_buff = new double[nNodes];

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


       m_rMB_buff = new double [nNodes*nIC];
       m_uIC_buff = new double [nNodes*nIC];
       m_xDC_buff = new double [nNodes*nDC];
       m_gam_buff = new double [nNodes*nDC];
       m_xPH_buff = new double [nNodes*nPH];
       m_vPS_buff = new double [nNodes*nPS];
       m_mPS_buff = new double [nNodes*nPS];
       m_bPS_buff = new double [nNodes*nIC*nPS];
       m_xPA_buff = new double [nNodes*nPS];
       // ----------------------------------

	   m_bIC_Elem = new double [nElems*nIC];
	   m_bIC_dummy_Elem = new double [nElems*nIC];
	   m_rMB_Elem = new double [nElems*nIC];
	   m_uIC_Elem = new double [nElems*nIC];
	   m_xDC_Elem = new double [nElems*nDC];
	   m_gam_Elem = new double [nElems*nDC];
	   m_dul_Elem = new double [nElems*nDC];
	   m_dll_Elem = new double [nElems*nDC];
	   m_aPH_Elem = new double [nElems*nPH];
	   m_xPH_Elem = new double [nElems*nPH];
	   m_vPS_Elem = new double [nElems*nPS];
	   m_mPS_Elem = new double [nElems*nPS];
	   m_bPS_Elem = new double [nElems*nIC*nPS];
	   m_xPA_Elem = new double [nElems*nPS];
       m_xDC_pts_Elem = new double [nElems*nDC];

       for (long in = 0; in < nNodes ; in++)
	       {
		       m_NodeHandle[in] = 0;
		       m_NodeStatusCH[in] = 0;
		       m_IterDone[in] = 0;

               m_NodeHandle_buff[in] = 0;
               m_NodeStatusCH_buff[in] = 0;
               m_IterDone_buff[in] = 0;
		       
		       m_T[in] = 0.0;
		       m_P[in] = 0.0;
		       m_Vs[in] = 0.0;
		       m_Ms[in] = 0.0;
		       m_Gs[in] = 0.0;
		       m_Hs[in] = 0.0;
		       m_IC[in] = 0.0;
		       m_pH[in] = 0.0;
		       m_pe[in] = 0.0;
		       m_Eh[in] = 0.0;

    	       m_Vs_buff[in] = 0.0;
               m_Ms_buff[in] = 0.0;
               m_Gs_buff[in] = 0.0;
               m_Hs_buff[in] = 0.0;
               m_IC_buff[in] = 0.0;
               m_pH_buff[in] = 0.0;
               m_pe_buff[in] = 0.0;
               m_Eh_buff[in] = 0.0;

		       int ii;
		       for (ii = 0; ii < nIC ; ii++)
		       {
			       *(m_bIC+in*nIC + ii) = 0.0;
			       *(m_bIC_dummy + in*nIC + ii) = 0.0;
			       *(m_rMB+in*nIC + ii) = 0.0;
			       *(m_uIC+in*nIC + ii) = 0.0;

			       *(m_rMB_buff+in*nIC + ii) = 0.0;
			       *(m_uIC_buff+in*nIC + ii) = 0.0;
		       }

		       for (ii = 0; ii < nDC ; ii++)
		       {
			       *(m_xDC+in*nDC+ii) = 0.0;
			       *(m_gam+in*nDC+ii) = 0.0;
			       *(m_dul+in*nDC+ii) = 0.0;
			       *(m_dll+in*nDC+ii) = 0.0;
                   *(m_xDC_pts+in*nDC+ii) = 0.0;
                   *(m_xDC_MT_delta+in*nDC+ii) = 0.0;
                   *(m_xDC_Chem_delta+in*nDC+ii) = 0.0;

			       *(m_xDC_buff+in*nDC+ii) = 0.0;
			       *(m_gam_buff+in*nDC+ii) = 0.0;
		       }

		       for (ii = 0; ii < nPH ; ii++)
		       {
			       *(m_aPH+in*nPH+ii) = 0.0;
			       *(m_xPH+in*nPH+ii) = 0.0;

			       *(m_xPH_buff+in*nPH+ii) = 0.0;
		       }

		       for (ii = 0; ii < nPS ; ii++)
		       {
			       *(m_vPS+in*nPS+ii) = 0.0;
			       *(m_mPS+in*nPS+ii) = 0.0;
			       *(m_xPA+in*nPS+ii) = 0.0;

			       *(m_vPS_buff+in*nPS+ii) = 0.0;
			       *(m_mPS_buff+in*nPS+ii) = 0.0;
			       *(m_xPA_buff+in*nPS+ii) = 0.0;
		       }
    		   
    	   
		       for (ii = 0; ii < nIC ; ii++)
		       {
			       for (int jj = 0; jj < nPS ; jj++)
			       {
			       *(m_bPS+in*nIC*nPS+jj) = 0.0;

			       *(m_bPS_buff+in*nIC*nPS+jj) = 0.0;
			       }
		       }
           }

	       for (long in = 0; in < nElems ; in++)
	       {
		       m_ElemHandle[in] = 0;
		       m_ElemStatusCH[in] = 0;
		       m_IterDone_Elem[in] = 0;

		       double tempT, tempP, tempVs, tempMs, tempGs, tempHs, tempIC, tempPH, tempPE, tempEh; 
		       tempT = 0; tempP = 0; tempVs = 0; tempMs = 0; tempGs = 0; tempHs = 0; tempIC = 0; tempPH = 0; tempPE = 0; tempEh = 0;

		       m_T_Elem[in] = 0.0;
		       m_P_Elem[in] = 0.0;
		       m_Vs_Elem[in] = 0.0;
		       m_Ms_Elem[in] = 0.0;
		       m_Gs_Elem[in] = 0.0;
		       m_Hs_Elem[in] = 0.0;
		       m_IC_Elem[in] = 0.0;
		       m_pH_Elem[in] = 0.0;
		       m_pe_Elem[in] = 0.0;
		       m_Eh_Elem[in] = 0.0;

		       int ii;
		       for (ii = 0; ii < nIC ; ii++)
		       {
			       *(m_bIC_Elem+in*nIC + ii) = 0.0;
			       *(m_bIC_dummy_Elem + in*nIC + ii) = 0.0;
			       *(m_rMB_Elem+in*nIC + ii) = 0.0;
			       *(m_uIC_Elem+in*nIC + ii) = 0.0;
		       }

		       for (ii = 0; ii < nDC ; ii++)
		       {
			       *(m_xDC_Elem+in*nDC+ii) = 0.0;
			       *(m_gam_Elem+in*nDC+ii) = 0.0;
			       *(m_dul_Elem+in*nDC+ii) = 0.0;
			       *(m_dll_Elem+in*nDC+ii) = 0.0;
                   *(m_xDC_pts_Elem+in*nDC+ii) = 0.0;
		       }

		       for (ii = 0; ii < nPH ; ii++)
		       {
			       *(m_aPH_Elem+in*nPH+ii) = 0.0;
			       *(m_xPH_Elem+in*nPH+ii) = 0.0;
		       }

		       for (ii = 0; ii < nPS ; ii++)
		       {
			       *(m_vPS_Elem+in*nPS+ii) = 0.0;
			       *(m_mPS_Elem+in*nPS+ii) = 0.0;
			       *(m_xPA_Elem+in*nPS+ii) = 0.0;
		       }
    		   
    	   
		       for (ii = 0; ii < nIC ; ii++)
		       {
			       for (int jj = 0; jj < nPS ; jj++)
			       {
			            *(m_bPS_Elem+in*nIC*nPS+jj) = 0.0;
			       }
		       }
           }

   return 0;//successed
   }
   else
   {
#ifdef USE_MPI_GEMS
      if ( myrank == 0 /*should be set to root*/)
#endif
      cout << "Error loading initial files to GEMS" <<endl;
	  return 1;   
   }
}

short REACT_GEM::Init_RUN()
{
int StatusCheck = 0;
long in = 0;
long it_num = 0;

    if ( flag_node_element_based == 0 )
    {it_num = nNodes;}
    else
    {it_num = nElems;}
	for (in = 0; in < it_num ; in++)
	{
		// Order GEM to run
		dBR->NodeStatusCH = NEED_GEM_AIA;
		
        if ( flag_node_element_based == 0 )
        {
		    m_NodeStatusCH[in] = (short)m_Node->GEM_run(false);

		    if ( !( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_PIA ) )
		    {
			    StatusCheck = 1;
			    #ifdef USE_MPI_GEMS
			    if ( myrank == 0 /*should be set to root*/)
			    #endif
			    cout << "Error!!! Initial Running GEM on Node #" << in <<  " failed. The retruned status code is:" << m_NodeStatusCH[in] << endl;
			    return 5;
		    }

		    // Extracting chemical datat into my buffer
		    m_Node->GEM_restore_MT( m_NodeHandle[in], m_NodeStatusCH[in], m_T[in],
			    m_P[in], m_Vs[in], m_Ms[in], m_bIC+in*nIC, m_dul+in*nDC, m_dll+in*nDC, m_aPH+in*nPH );

		    m_Node->GEM_to_MT( m_NodeHandle[in], m_NodeStatusCH[in], m_IterDone[in],
			    m_Vs[in], m_Ms[in], m_Gs[in], m_Hs[in], m_IC[in], m_pH[in], m_pe[in],
		       m_Eh[in], m_rMB+in*nIC, m_uIC+in*nIC, m_xDC+in*nDC, m_gam+in*nDC,
		       m_xPH+in*nPH, m_vPS+in*nPS, m_mPS+in*nPS,
		       m_bPS+in*nIC*nPS, m_xPA+in*nPS );
        }
        else
        {
            m_ElemStatusCH[in] = (short)m_Node->GEM_run(false);

		    if ( !( m_ElemStatusCH[in] == OK_GEM_AIA || m_ElemStatusCH[in] == OK_GEM_PIA ) )
		    {
			    StatusCheck = 1;
			    #ifdef USE_MPI_GEMS
			    if ( myrank == 0 /*should be set to root*/)
			    #endif
			    cout << "Error!!! Initial Running GEM on Node #" << in <<  " failed. The retruned status code is:" << m_NodeStatusCH[in] << endl;
			    return 5;
		    }

		    // Extracting chemical datat into my buffer
		    m_Node->GEM_restore_MT( m_ElemHandle[in], m_ElemStatusCH[in], m_T_Elem[in],
			    m_P_Elem[in], m_Vs_Elem[in], m_Ms_Elem[in], m_bIC_Elem+in*nIC, m_dul_Elem+in*nDC, m_dll_Elem+in*nDC, m_aPH_Elem+in*nPH );

		    m_Node->GEM_to_MT( m_ElemHandle[in], m_ElemStatusCH[in], m_IterDone_Elem[in],
			    m_Vs_Elem[in], m_Ms_Elem[in], m_Gs_Elem[in], m_Hs_Elem[in], m_IC_Elem[in], m_pH_Elem[in], m_pe_Elem[in],
		       m_Eh_Elem[in], m_rMB_Elem+in*nIC, m_uIC_Elem+in*nIC, m_xDC_Elem+in*nDC, m_gam_Elem+in*nDC,
		       m_xPH_Elem+in*nPH, m_vPS_Elem+in*nPS, m_mPS_Elem+in*nPS,
		       m_bPS_Elem+in*nIC*nPS, m_xPA_Elem+in*nPS );
        }
		
		//GetReactInfoFromGEM(in);// HS 11.07.2007
		#ifdef USE_MPI_GEMS
		if ( myrank == 0 /*should be set to root*/)
		#endif
		cout << "Initial Running GEM on Node #" << in <<  " successful. "  << endl;
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
    char *buffer;
    int max_len=256;
    
	init_path = m_Project_path.append(init_input_file_sig);

	#ifdef _WIN32
	if (init_path.rfind("\\") == string::npos) // keep this on windows
	#else
	if ( init_path.rfind("/") == string::npos) // keep this on linux
	#endif
	{
		#ifdef _WIN32
			if( (buffer = _getcwd( NULL, 0 )) == NULL )
		#else
			if( (buffer = getcwd( NULL, 0 )) == NULL )
		#endif
            perror( "_getcwd error" );
		else
        {
		#ifdef _WIN32
			  init_path.insert( 0, "\\" ); // keep this on window
		#else
			  init_path.insert( 0, "/" ); // keep this on linux
		#endif
          init_path.insert( 0, buffer );
        }
    }

	if( m_Node->GEM_init( init_path.c_str() , mp_nodeTypes , false) )
	{
	    return 0; // error occured during reading the files
	}
	else 
	{
		return 1; // read init file successed
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
	    REACT_GEM::GetDCValue_MT(node_i, timelevel, m_xDC+node_i*nDC, m_xDC_pts+node_i*nDC, m_xDC_MT_delta+node_i*nDC);

	    // Setting Solid Phase Component // HS: Solid does not move.
	    // REACT_GEM::GetSoComponentValue_MT(node_i, timelevel, m_xPH+node_i*nPH );

	    //get PH value from MT
	    // m_pH[node_i] = REACT_GEM::GetComponentValue_MT(node_i,"pH", timelevel);

	    //get pe value from MT
	    // m_pe[node_i] = REACT_GEM::GetComponentValue_MT(node_i,"pe", timelevel);   
 	}

    if (flag_node_element_based == 1) ConvNodeValue2Elem();

	return 0;
}

short REACT_GEM::SetReactInfoBackMassTransport(int timelevel)
{

    if (flag_node_element_based == 1) ConvElemValue2Node();

	for (long in=0; in < nNodes ; in++) 
	{
           // Check if it is the boundary node
           // for the boundary nodes, the values should be fixed. 
           if ( m_pcs->m_msh->nod_vector[in]->onBoundary() == false )
           {
                // Setting Temperature // disabled by HS. temperature is NOT the output from chemistry.
		        // REACT_GEM::SetTempValue_MT(in,timelevel,m_T[in]);

		        // Setting Pressure // disabled by HS. pressure is NOT the output from chemistry.
		        // REACT_GEM::SetPressureValue_MT(in,timelevel,m_P[in]);

		        // Setting Independent Component
		        REACT_GEM::SetDCValue_MT( in , timelevel , &(m_xDC[in*nDC]));

		        // Setting Solid Phase Component
		        // REACT_GEM::SetSoComponentValue_MT( in ,timelevel ,&( m_xPH[in*nPH]));

		        // Setting PH
		        REACT_GEM::SetPHValue_MT(in,timelevel,m_pH[in]);
        		
		        // Setting pe
		        REACT_GEM::SetPeValue_MT(in,timelevel,m_pe[in]);
            }            
	}

return 0;
}

void REACT_GEM::GetReactInfoFromGEM(long in)
{
#ifdef USE_MPI_GEMS
    // Extract the result from GEM
	m_Node->GEM_to_MT( m_NodeHandle_buff[in], m_NodeStatusCH_buff[in], m_IterDone_buff[in],
		m_Vs_buff[in], m_Ms_buff[in], m_Gs_buff[in], m_Hs_buff[in], m_IC_buff[in], m_pH_buff[in], m_pe_buff[in], m_Eh_buff[in],
		m_rMB_buff+in*nIC, m_uIC_buff+in*nIC, m_xDC_buff+in*nDC, m_gam_buff+in*nDC, m_xPH_buff+in*nPH, m_vPS_buff+in*nPS, m_mPS_buff+in*nPS,
		m_bPS_buff+in*nIC*nPS, m_xPA_buff+in*nPS );
#else

	// Extracting GEMIPM output data to FMT part
    if ( flag_node_element_based == 0 )
    {
	m_Node->GEM_to_MT( m_NodeHandle[in], m_NodeStatusCH[in], m_IterDone[in],
		m_Vs[in], m_Ms[in], m_Gs[in], m_Hs[in], m_IC[in], m_pH[in], m_pe[in], m_Eh[in],
		m_rMB+in*nIC, m_uIC+in*nIC, m_xDC+in*nDC, m_gam+in*nDC, m_xPH+in*nPH, m_vPS+in*nPS, m_mPS+in*nPS,
		m_bPS+in*nIC*nPS, m_xPA+in*nPS );
    }
    else
    {
    m_Node->GEM_to_MT( m_ElemHandle[in], m_ElemStatusCH[in], m_IterDone_Elem[in],
		m_Vs_Elem[in], m_Ms_Elem[in], m_Gs_Elem[in], m_Hs_Elem[in], m_IC_Elem[in], m_pH_Elem[in], m_pe_Elem[in], m_Eh_Elem[in],
		m_rMB_Elem+in*nIC, m_uIC_Elem+in*nIC, m_xDC_Elem+in*nDC, m_gam_Elem+in*nDC, m_xPH_Elem+in*nPH, m_vPS_Elem+in*nPS, m_mPS_Elem+in*nPS,
		m_bPS_Elem+in*nIC*nPS, m_xPA_Elem+in*nPS );
    }
#endif

return;
}

void REACT_GEM::SetReactInfoBackGEM(long in)
{
	// Setting input data for GEMIPM

	// Using the overloaded version of GEM_from_MT() to load the data	// HS 10.07.2007 
    if ( flag_node_element_based == 0 )
    {
	m_Node->GEM_from_MT( m_NodeHandle[in], m_NodeStatusCH[in],
			 m_T[in], m_P[in], m_Vs[in], m_Ms[in],
			 m_bIC_dummy+in*nIC/*these values should be set to zero.*/, m_dul+in*nDC, m_dll+in*nDC, m_aPH+in*nPH  ,m_xDC+in*nDC);
    }
    else
    {
	m_Node->GEM_from_MT( m_ElemHandle[in], m_ElemStatusCH[in],
			 m_T_Elem[in], m_P_Elem[in], m_Vs_Elem[in], m_Ms_Elem[in],
			 m_bIC_dummy_Elem+in*nIC/*these values should be set to zero.*/, m_dul_Elem+in*nDC, m_dll_Elem+in*nDC, m_aPH_Elem+in*nPH  ,m_xDC_Elem+in*nDC);
    }

return;
}

short REACT_GEM::Run_MainLoop()
{
	nNodes = GetNodeNumber_MT();
    nElems = GetElemNumber_MT();
	long in,it_num;
    if ( flag_node_element_based == 0 )
    {it_num = nNodes;}
    else
    {it_num = nElems;}

#ifdef USE_MPI_GEMS
	// MPI initialization.
	// So here is going to distribute the task.
	MPI_Bcast(&nNodes, 1, MPI_LONG, 0, MPI_COMM_WORLD );
	// here "myrank" is the index of the CPU Processes, and "size" is the number of CPU Processes
	for ( in = myrank; in < it_num ; in+= mysize )
#else
    for (in = 0; in < it_num ; in++)
#endif	
    {
		//Get data
		REACT_GEM::SetReactInfoBackGEM(in);
		// Order GEM to run
		dBR->NodeStatusCH = NEED_GEM_AIA;
		
        if ( flag_node_element_based == 0 )
        {
		    m_NodeStatusCH[in] = (short)m_Node->GEM_run(false);
		    if ( !( m_NodeStatusCH[in] == OK_GEM_AIA || m_NodeStatusCH[in] == OK_GEM_PIA ) )
		    {
			    // HS: Error information should be delivered regardless of myrank
                // #ifdef USE_MPI_GEMS
      			//    if ( myrank == 0 /*should be set to root*/)
			    // #endif
			    cout << "Error: Main Loop failed when running GEM on Node #" << in << "." << endl << "Returned Error Code: " << m_NodeStatusCH[in] << endl;
			    //return 5;
		    }
		    else
		    {
                // HS: Success informaiton should be delivered regardless of myrank
		        // #ifdef USE_MPI_GEMS
      		    // if ( myrank == 0 /*should be set to root*/)
		        // #endif
		        // cout << "Main Loop Running GEM on Node #" << in <<  " successful. "  << endl;
                // Put current result into m_xDC_pts;		        
                
                // Give Databack
		        REACT_GEM::GetReactInfoFromGEM(in);
		    }
        }
        else
        {
            m_ElemStatusCH[in] = (short)m_Node->GEM_run(false);
		    if ( !( m_ElemStatusCH[in] == OK_GEM_AIA || m_ElemStatusCH[in] == OK_GEM_PIA ) )
		    {
			    #ifdef USE_MPI_GEMS
      			    if ( myrank == 0 /*should be set to root*/)
			    #endif
			    cout << "Error: Main Loop failed when running GEM on Node #" << in << "." << endl << "Returned Error Code: " << m_NodeStatusCH[in] << endl;
			    //return 5;
		    }
		    else
		    {
		        #ifdef USE_MPI_GEMS
      		    if ( myrank == 0 /*should be set to root*/)
		        #endif
		        cout << "Main Loop Running GEM on Node #" << in <<  " successful. "  << endl;
		        // Give Databack
		        REACT_GEM::GetReactInfoFromGEM(in);
		    }
        }
	}

    // For MPI scheme, gather the data here.
    #ifdef USE_MPI_GEMS
        REACT_GEM::GetGEMResult_MPI();
        REACT_GEM::CleanMPIBuffer();
    #endif	
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

void REACT_GEM::GetFluidProperty_MT(void)
{
	m_FluidProp = MFPGet("LIQUID");
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

long REACT_GEM::GetElemNumber_MT(void)
{
	long number;
	//------------read number of elems--------------
	for (int i=0; i < (int)pcs_vector.size(); i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
		{
			number = (long) m_pcs->m_msh->ele_vector.size();
			return number;
		}
	}
	//------------end of reading number of nodes---- 
	return 0;
}

double REACT_GEM::GetTempValue_MT(long node_Index, int timelevel)
{
	int indx;
	double temp;
	
	if (heatflag == 1)
	{
		m_pcs = PCSGet("HEAT_TRANSPORT");

        indx = m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
        temp = m_pcs->GetNodeValue(node_Index, indx);
        
        //sysT[i] = m_pcs->GetNodeValue(i, indx1); 
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo �C->K
        //if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo �C->K
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
	int indx;
		
	if (heatflag == 1)
	{
		m_pcs = PCSGet("HEAT_TRANSPORT");

        indx = m_pcs->GetNodeValueIndex("TEMPERATURE1")+timelevel;
        m_pcs->SetNodeValue(node_Index, indx, temp); 

		//sysT[i] = m_pcs->GetNodeValue(i, indx1); 
		//if (sysT0[i] <273.15) sysT0[i] += 273.15;  //ToDo �C->K
		//if (sysT[i] <273.15) sysT[i] += 273.15;  //ToDo �C->K
		return 1;
	}
	else
	return 0;
}

double REACT_GEM::GetPressureValue_MT(long node_Index, int timelevel)
{
  //Get pressure value 
  double pressure;
  int indx;
  pressure = 0.0;
    
  if(flowflag > 0)
  {
	GetFluidProperty_MT();
  	switch(flowflag)
	{
        case 1:
		  m_pcs = PCSGet("GROUNDWATER_FLOW");

          indx = m_pcs->GetNodeValueIndex("HEAD")+timelevel;
          pressure = m_pcs->GetNodeValue(node_Index, indx); // The unit of HEAD is meters

		  // change the pressure unit from meters of water to bar. 
		  pressure = Pressure_M_2_Bar( pressure , m_FluidProp->Density() );
		  if (pressure<1.0) pressure = 1.0;//HS 18.07.2007 do not allow pressure lower than 1 bar.
        break;
        case 2:
		  m_pcs = PCSGet("LIQUID_FLOW");
		  indx = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  pressure = m_pcs->GetNodeValue(node_Index, indx); // The unit of PRESSURE1 is Pascal.
		  // change the pressure unit from Pascal to bar. 
		  pressure = Pressure_Pa_2_Bar( pressure );
		  // if (pressure<1.0) pressure = 1.0;//HS 18.07.2007 do not allow pressure lower than 1 bar.
	    break;
        case 3:
		  m_pcs = PCSGet("RICHARDS_FLOW");
		  indx = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  pressure = m_pcs->GetNodeValue(node_Index, indx); // The unit of PRESSURE1 is Pascal.
		  // change the pressure unit from Pascal to bar. 
		  pressure = Pressure_Pa_2_Bar( pressure );
		  // if (pressure<1.0) pressure = 1.0;//HS 18.07.2007 do not allow pressure lower than 1 bar.
        break;
        case 4:
		  #ifdef USE_MPI_GEMS
		  if ( myrank == 0 /*should be set to root*/)
		  #endif
		  DisplayErrorMsg("Error: Not implemented for the flow in GEM case!!!");
		  pressure = 1.0;   //TODO  MX // HB: TODO
        break;
      }
  }
  else 
  {
	DisplayErrorMsg("Warning: No valid flow process!!");
  }
return pressure;
}

short REACT_GEM::SetPressureValue_MT(long node_Index, int timelevel, double pressure)
{
//Set pressure value 
  int indx;
  indx = 0;
  if(flowflag > 0)
  {
	switch(flowflag)
	{
        case 1:
		  m_pcs = PCSGet("GROUNDWATER_FLOW");
          pressure = Pressure_Bar_2_M( pressure ,  m_FluidProp->Density() );
         
          indx = m_pcs->GetNodeValueIndex("HEAD")+timelevel;
  		  m_pcs->SetNodeValue(node_Index, indx, pressure); 

        break;
        case 2:
		  m_pcs = PCSGet("LIQUID_FLOW");
		  indx = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  pressure = Pressure_Bar_2_Pa( pressure );
		  m_pcs->SetNodeValue(node_Index, indx, pressure); 
	break;
        case 3:
		  m_pcs = PCSGet("RICHARDS_FLOW");
		  indx = m_pcs->GetNodeValueIndex("PRESSURE1")+timelevel;
		  pressure = Pressure_Bar_2_Pa( pressure );
		  m_pcs->SetNodeValue(node_Index, indx, pressure); 
        break;
        case 4:
		  pressure = Pressure_Bar_2_Pa( pressure );
		  #ifdef USE_MPI_GEMS
      		  if ( myrank == 0 /*should be set to root*/)
		  #endif
		  DisplayErrorMsg("Error: Not implemented for the flow in GEM case!!!");
		  m_pcs->SetNodeValue(node_Index, indx, pressure); 
        break;
        }
  }
  else 
  {
	  #ifdef USE_MPI_GEMS
	  if ( myrank == 0 /*should be set to root*/)
	  #endif
	  DisplayErrorMsg("Warning: No valid flow process!!");
	  return 0;
  }
return 1;
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
			{
                m_comp_value = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0])+timelevel);
            }

		}
	}
	if ( m_comp_value != -1.0 ){return m_comp_value;}
	else
	{
	#ifdef USE_MPI_GEMS
        if ( myrank == 0 /*should be set to root*/)
	#endif
	DisplayErrorMsg("Error: Corresponding Component NOT FOUND!!!");
	return m_comp_value;
	}
}

short REACT_GEM::GetDCValue_MT(long node_Index, int timelevel, double* m_DC, double* m_DC_pts ,double* m_DC_MT_delta)
{
string str;
double DC_MT_pre, DC_MT_cur;

	for (int i=0; i < nDC ; i++)
	{
		m_pcs = pcs_vector[i+1];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{
			str = m_pcs->pcs_primary_function_name[0];
			if (str.compare("pH") != 0 && str.compare("pe") != 0 )
			{
                // Get previous iteration mass transport concentration value
                DC_MT_pre = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str)+0);
                // Get current iteration mass transport concentration value
                DC_MT_cur = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str)+timelevel);
                
                /*
                if ( flag_iterative_scheme > 0 ) // this means introduce delta_C from MT
                {
                    // Get delta_Concentration
                    *(m_DC_MT_delta+i) = DC_MT_pre - DC_MT_cur;
                    // Now introduce the change from MT into GEMS
                    *(m_DC+i) -= *(m_DC_MT_delta+i);
                }
                else // meaning non-iterative scheme
                */
                {
                    // Copy cocentration from MT to GEMS
                    *(m_DC+i) = DC_MT_cur;
                }
            }
		}

	}

	return 1;
}
short REACT_GEM::GetSoComponentValue_MT(long node_Index, int timelevel, double* m_Phase)
{
	for (int i=0; i < (int)pcs_vector.size() ; i++)
	{
		m_pcs = pcs_vector[i];
		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{	
			string str;

			int x_Component = 0;

			str = m_pcs->pcs_primary_function_name[0];//get the name of compound from MT;
			x_Component = m_Node->Ph_name_to_xDB(str.c_str());//get the index of certain compound, -1: no match
			if ( x_Component > -1)
			{
			    *(m_Phase+x_Component) = m_pcs->GetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str)+timelevel);
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
short REACT_GEM::SetDCValue_MT(long node_Index, int timelevel, double* m_DC)
{
string str;
	for (int i=0; i < nDC ; i++)
	{
		m_pcs = pcs_vector[i+1];

		if (m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0)
		{
			str = m_pcs->pcs_primary_function_name[0];
			if (str.compare("pH") != 0 && str.compare("pe") != 0 )
			{
			    if ( flag_iterative_scheme > 0 )
                {
                    if ( CPGetMobil(m_pcs->GetProcessComponentNumber()) > 0)
                    {
                        // m_pcs->eqs->b[node_Index] += m_xDC_Chem_delta[node_Index*nDC+i] / dt ;
                        m_pcs->SetNodeValue(node_Index , m_pcs->GetNodeValueIndex(str)+timelevel , *(m_DC+i));
                    }
                    else
                    {
                        m_pcs->SetNodeValue(node_Index , m_pcs->GetNodeValueIndex(str)+timelevel , *(m_DC+i));
                    }
                }
                else
                {
                    m_pcs->SetNodeValue(node_Index , m_pcs->GetNodeValueIndex(str)+timelevel , *(m_DC+i));
                }
			}
		}

	}

	return 1;
}

short REACT_GEM::SetSoComponentValue_MT(long node_Index, int timelevel, double* m_Phase)
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
                    m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str),*(m_Phase+x_Component));	
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
	    		m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str)+timelevel,m_PH);
				return 1;
			}
		}
	}
	// Not found PH: 
	#ifdef USE_MPI_GEMS
	if ( myrank == 0 /*should be set to root*/)
	#endif
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
  				m_pcs->SetNodeValue(node_Index,m_pcs->GetNodeValueIndex(str)+timelevel,m_PE);
				return 1;
			}
		}
	}
	// Not found PE: 
	#ifdef USE_MPI_GEMS
	if ( myrank == 0 /*should be set to root*/)
	#endif
	DisplayErrorMsg("Error: PE NOT FOUND in MT part!!");
	return 0;
}

double REACT_GEM::Pressure_Pa_2_Bar(double Pre_in_Pa)
{
return Pre_in_Pa / 1e+5;
}

double REACT_GEM::Pressure_Bar_2_Pa(double Pre_in_Bar)
{
return Pre_in_Bar * 1e+5;
}

double REACT_GEM::Pressure_M_2_Bar(double Pre_in_M, double flu_density )
{return Pre_in_M * 9.8 * flu_density / 1e5 ;}

double REACT_GEM::Pressure_Bar_2_M(double Pre_in_Bar, double flu_density )
{return Pre_in_Bar * 1e5 / 9.8 / flu_density ;}

double REACT_GEM::GetNodeAdjacentVolume(long Idx_Node)
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
    for ( int i=0 ; i < (long)m_Node->connected_elements.size() ; i++ )
    {
        // get the index of current element;
        Idx_Ele = m_Node->connected_elements[i];

        // get the pointer of this element;
        m_Elem = m_pcs->m_msh->ele_vector[Idx_Ele];

        // get the number of nodes in this element;
        // given argument "false" means giving node number instead of Gauss points;
        number_of_nodes = m_Elem->GetNodesNumber(false);
        
        // taking part of volume from this element;
        volume += m_Elem->GetVolume() / number_of_nodes ;
    }

return volume;
}

void REACT_GEM::ConvElemValue2Node(void)
{
    long i ,idx_Elem;
    int j,k,l, number_of_conn_elems;
    
    CNode* m_Node;
    CElem* m_Elem;

    for ( i=0 ; i < nNodes ; i++)
    {
        // first set the parameters to zero;
        m_T[i] = 0.0;
        m_P[i] = 0.0;
        m_pH[i]= 0.0;
        m_pe[i]= 0.0;
        for ( k=0 ; k < nDC ; k++)
        m_xDC[i*nDC+k] = 0.0;
        for ( l=0 ; l < nPH ; l++)
        m_xPH[i*nPH+l] = 0.0;

        // then get the values from nodes
        m_Node =  m_pcs->m_msh->nod_vector[i];
        for ( j=0 ; j < (int)m_Node->connected_elements.size() ; j++ )
        {
            idx_Elem = m_Node->connected_elements[j];// get the connected element
            m_Elem = m_pcs->m_msh->ele_vector[idx_Elem];
            number_of_conn_elems = (int)m_Node->connected_elements.size();

            m_T[i] += m_T_Elem[idx_Elem] / number_of_conn_elems;
            m_P[i] += m_P_Elem[idx_Elem] / number_of_conn_elems;
            m_pH[i] += m_pH_Elem[idx_Elem] / number_of_conn_elems;
            m_pe[i] += m_pe_Elem[idx_Elem] / number_of_conn_elems;
            for ( k=0 ; k < nDC ; k++)
            m_xDC[i*nDC+k] += m_xDC_Elem[idx_Elem*nDC+k] / number_of_conn_elems;
            for ( l=0 ; l < nPH ; l++)
            m_xPH[i*nPH+l] += m_xPH_Elem[idx_Elem*nPH+l] / number_of_conn_elems;
                
        }
    }
}

void REACT_GEM::ConvNodeValue2Elem(void)
{
    long i,idx_Node;
    int j,k,l, number_of_nodes;
    
    CNode* m_Node;
    CElem* m_Elem;

    for ( i=0 ; i < nElems ; i++)
    {
        m_Elem =  m_pcs->m_msh->ele_vector[i];
        
        // first set the parameters to zero;
        m_T_Elem[i] = 0.0;
        m_P_Elem[i] = 0.0;
        m_pH_Elem[i]= 0.0;
        m_pe_Elem[i]= 0.0;
        for ( k=0 ; k < nDC ; k++)
        m_xDC_Elem[i*nDC+k] = 0.0;
        for ( l=0 ; l < nPH ; l++)
        m_xPH_Elem[i*nPH+l] = 0.0;

        // then get the values from nodes
        for ( j=0 ; j < m_Elem->GetNodesNumber(false) ; j++ )
        {
            idx_Node = m_Elem->GetNodeIndex( j ); // get the connected nodes;
            m_Node = m_pcs->m_msh->nod_vector[idx_Node];
            number_of_nodes = (int)m_Elem->GetNodesNumber(false);
            
            m_T_Elem[i] += m_T[idx_Node] / number_of_nodes;
            m_P_Elem[i] += m_P[idx_Node] / number_of_nodes;
            m_pH_Elem[i] += m_pH[idx_Node] / number_of_nodes;
            m_pe_Elem[i] += m_pe[idx_Node] / number_of_nodes;
            for ( k=0 ; k < nDC ; k++)
            m_xDC_Elem[i*nDC+k] += m_xDC[idx_Node*nDC+k] / number_of_nodes;
            for ( l=0 ; l < nPH ; l++)
            m_xPH_Elem[i*nPH+l] += m_xPH[idx_Node*nPH+l] / number_of_nodes;
                
        }
    }
}
void REACT_GEM::CopyCurXDCPre(void)
{
    long i;
    for ( i=0 ; i < nNodes*nDC ; i++)
    {
        m_xDC_pts[i] = m_xDC[i];
    }
}

void REACT_GEM::UpdateXDCChemDelta(void)
{
    long i;
    for ( i=0 ; i < nNodes*nDC ; i++)
    {
        m_xDC_Chem_delta[i] = m_xDC[i] - m_xDC_pts[i];
    }
}

#ifdef USE_MPI_GEMS
void REACT_GEM::GetGEMResult_MPI(void)
{
	// Now gather the calculated values------------------------------------------------------------------------------
	MPI_Allreduce( m_NodeHandle_buff, m_NodeHandle, nNodes, MPI_SHORT, MPI_SUM , MPI_COMM_WORLD);
	MPI_Allreduce( m_NodeStatusCH_buff, m_NodeStatusCH, nNodes, MPI_SHORT, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_IterDone_buff, m_IterDone, nNodes, MPI_SHORT, MPI_SUM, MPI_COMM_WORLD);

	MPI_Allreduce( m_Vs_buff, m_Vs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_Ms_buff, m_Ms, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_Gs_buff, m_Gs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_Hs_buff, m_Hs, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_IC_buff, m_IC, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_pH_buff, m_pH, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_pe_buff, m_pe, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_Eh_buff, m_Eh, nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	MPI_Allreduce( m_rMB_buff, m_rMB, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_uIC_buff, m_uIC, nNodes*nIC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_xDC_buff, m_xDC, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_gam_buff, m_gam, nNodes*nDC, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_xPH_buff, m_xPH, nNodes*nPH, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_vPS_buff, m_vPS, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_mPS_buff, m_mPS, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_bPS_buff, m_bPS, nNodes*nIC*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce( m_xPA_buff, m_xPA, nNodes*nPS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    // --------------------------------------------------------------------------------------------------------------

}

void REACT_GEM::CleanMPIBuffer(void)
{
    for (long in = 0; in < nNodes ; in++)
    {
        m_NodeHandle_buff[in] = 0;
        m_NodeStatusCH_buff[in] = 0;
        m_IterDone_buff[in] = 0;

        m_Vs_buff[in] = 0.0;
        m_Ms_buff[in] = 0.0;
        m_Gs_buff[in] = 0.0;
        m_Hs_buff[in] = 0.0;
        m_IC_buff[in] = 0.0;
        m_pH_buff[in] = 0.0;
        m_pe_buff[in] = 0.0;
        m_Eh_buff[in] = 0.0;

        int ii;
        for (ii = 0; ii < nIC ; ii++)
        {
            *(m_rMB_buff+in*nIC + ii) = 0.0;
            *(m_uIC_buff+in*nIC + ii) = 0.0;
        }

        for (ii = 0; ii < nDC ; ii++)
        {
            *(m_xDC_buff+in*nDC+ii) = 0.0;
            *(m_gam_buff+in*nDC+ii) = 0.0;
        }

        for (ii = 0; ii < nPH ; ii++)
        {
            *(m_xPH_buff+in*nPH+ii) = 0.0;
        }

        for (ii = 0; ii < nPS ; ii++)
        {
            *(m_vPS_buff+in*nPS+ii) = 0.0;
            *(m_mPS_buff+in*nPS+ii) = 0.0;
            *(m_xPA_buff+in*nPS+ii) = 0.0;
        }


        for (ii = 0; ii < nIC ; ii++)
        {
            for (int jj = 0; jj < nPS ; jj++)
            {
            *(m_bPS_buff+in*nIC*nPS+jj) = 0.0;
            }
        }
    }
}
#endif

