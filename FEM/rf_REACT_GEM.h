//-------------------------------------
// rf_REACT_GEM.h
// Haibing Shao 23.05.07
// GEM Reaction Package
//-------------------------------------

#include <time.h>
#include <math.h>
#include <string.h>

#include "node.h"

class REACT_GEM
{
public:
	REACT_GEM(void);
	~REACT_GEM(void);

    
	TNode* m_Node;// Instance of TNode class
    DATACH* dCH;//pointer to DATACH
    DATABR* dBR;//pointer to DATABR


	int Init_Node(string Project_path);	// Initialization of the GEM TNode Class
    int Init_RUN();     // Run the node-GEM

   // Allocating work memory for FMT part (here only chemical variables)
   // for one node only (real FMT problems consider many nodes)
   // HB for the convenience of further development on paralell system, keep it as one node so far.
   // Names are consistent with the DataBridge structure (see "\GEM\databr.h")

   short m_NodeHandle, m_NodeStatusCH, m_IterDone;

   double m_T, m_P, m_Vs, m_Ms,
          m_Gs, m_Hs, m_IC, m_pH, m_pe,
          m_Eh;

   double *m_xDC, *m_gam, *m_xPH, *m_aPH, *m_vPS, *m_mPS,*m_bPS,
         *m_xPA, *m_dul, *m_dll, *m_bIC, *m_rMB, *m_uIC;

   // Number of ICs, DCs, Phases and Phases-solutions kept in the node
   // DATABR structure for exchange with GEMIPM
   int nIC, nDC, nPH, nPS;




private:
	string ipm_input_file_path;
	string dbr_input_file_path;
	string dch_input_file_path;
public:
	string Get_IPM_File_Path(void);
	string Get_DBR_File_Path(void);
	string Get_DCH_File_Path(void);
	int Set_IPM_FILE_PATH(string m_path);
	int Set_DBR_FILE_PATH(string m_path);
	int Set_DCH_FILE_PATH(string m_path);

	int Load_IPM_File(string m_Project_path);
};
