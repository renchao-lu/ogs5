//-------------------------------------
// rf_REACT_GEM.cpp
// Haibing Shao 23.05.07
// GEM Reaction Package
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------

#include "rf_REACT_GEM.h"
#include "node.h"

REACT_GEM::REACT_GEM(void)
{
   m_Node = new TNode();
   REACT_GEM::dbr_input_file_path = "FEM_GEM_dbr.dat";
   REACT_GEM::dch_input_file_path = "FEM_GEM_dch.dat";
   REACT_GEM::ipm_input_file_path = "FEM_GEM_ipm.dat";
   REACT_GEM::m_NodeHandle = 0;
   REACT_GEM::m_NodeStatusCH = 0;
   REACT_GEM::m_IterDone = 0;
   nIC = 0;
   nDC = 0;
   nPH = 0;
   nPS = 0;
}

REACT_GEM::~REACT_GEM(void)
{
  delete m_Node;
  free( m_xDC );
  free( m_gam );
  free( m_xPH );
  free( m_vPS );
  free( m_mPS );
  free( m_bPS );
  free( m_xPA );
  free( m_dul );
  free( m_dll );
  free( m_bIC );
  free( m_rMB );
  free( m_uIC );
}

// Initialization of the GEM TNode Class
int REACT_GEM::Init_Node(string Project_path)
{

   // Creating TNode structure accessible trough node pointer
   // Here we read the files needed as input for initializing GEMIPM2K
   // The easiest way to prepare them is to use GEMS-PSI code (GEM2MT module)
   if ( Load_IPM_File(Project_path))
   {
      cout << "Error loading IPM file to GEMS" <<endl;
	  return 1;
   };


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
   nIC = dCH->nICb;
   nDC = dCH->nDCb;
   nPH = dCH->nPHb;
   nPS = dCH->nPSb;

   // Allocating work memory for FMT part (here only chemical variables)
   m_bIC = (double*)malloc( /*nNodes**/nIC*sizeof(double) );
   m_rMB = (double*)malloc( /*nNodes**/nIC*sizeof(double) );
   m_uIC = (double*)malloc( /*nNodes**/nIC*sizeof(double) );
   m_xDC = (double*)malloc( /*nNodes**/nDC*sizeof(double) );
   m_gam = (double*)malloc( /*nNodes**/nDC*sizeof(double) );
   m_dul = (double*)malloc( /*nNodes**/nDC*sizeof(double) );
   m_dll = (double*)malloc( /*nNodes**/nDC*sizeof(double) );
   m_aPH = (double*)malloc( /*nNodes**/nPH*sizeof(double) );
   m_xPH = (double*)malloc( /*nNodes**/nPH*sizeof(double) );
   m_vPS = (double*)malloc( /*nNodes**/nPS*sizeof(double) );
   m_mPS = (double*)malloc( /*nNodes**/nPS*sizeof(double) );
   m_bPS = (double*)malloc( /*nNodes**/nIC*nPS*sizeof(double) );
   m_xPA = (double*)malloc( /*nNodes**/nPS*sizeof(double) );


}

int REACT_GEM::Init_RUN()
{
   // Getting direct access to DataCH structure in GEMIPM2K memory
   dCH = m_Node->pCSD();
   if( !dCH  )
       return 3;

   // Getting direct access to work node DATABR structure which
   // exchanges data between GEMIPM and FMT parts
   dBR = m_Node->pCNode();
   if( !dBR  )
       return 4;

	// Order GEM to run
	dBR->NodeStatusCH = NEED_GEM_AIA;
	
	m_NodeStatusCH = m_Node->GEM_run();

	if ( !( m_NodeStatusCH == OK_GEM_AIA || m_NodeStatusCH == OK_GEM_PIA ) )return 5;
    // Extracting chemical datat into FMT part
	m_Node->GEM_restore_MT( m_NodeHandle, m_NodeStatusCH, m_T,m_P, m_Vs, m_Ms, m_bIC, m_dul, m_dll, m_aPH );
    // Extracting GEMIPM output data to FMT part
    m_Node->GEM_to_MT( m_NodeHandle, m_NodeStatusCH, m_IterDone, m_Vs, m_Ms, m_Gs, m_Hs, m_IC, m_pH, m_pe, m_Eh, m_rMB, m_uIC, m_xDC, m_gam, m_xPH, m_vPS, m_mPS, m_bPS, m_xPA );

    //  Uncomment this to test variable pressures and temperatures
    //  m_T[in] += (in-1)*5;
    //  m_P[in] += (in-1)*20;
    //  Here the file output for the initial conditions can be implemented

    // Initialization of GEMIPM and chemical data kept in the FMT part
    // Can be done in a loop over boundary nodes
    //   cout << "Begin Initialiation part" << endl;

    // Read DATABR structure from text file (read boundary condition)
    //TNode::na->GEM_read_dbr( dbr_input_file_name );//HB ? What is this?

	return 0;
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


int REACT_GEM::Load_IPM_File(string m_Project_path)
{
	string ipm_path;

	//Checking absolute path
	if (ipm_input_file_path.find( m_Project_path ) != string::npos  )
	{
	  ipm_path = ipm_input_file_path;
	}
	else
	{
		ipm_path = m_Project_path.append( ipm_input_file_path );
	}

	if( m_Node->GEM_init( ipm_path.c_str() ) )
       return 1;  // error occured during reading the files
	else 
	return 0;
}
