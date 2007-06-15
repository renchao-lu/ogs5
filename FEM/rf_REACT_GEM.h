//-------------------------------------
// rf_REACT_GEM.h
// Haibing Shao 23.05.07
// haibing.shao@ufz.de
// GEM Reaction Package
// based on the PSI node-GEM source code
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------
#include <time.h>
#include <math.h>
#include <string.h>

#include "node.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"
//#include "rf_REACT_GEM.h"

class REACT_GEM
{
private:
	string ipm_input_file_path;
	string dbr_input_file_path;
	string dch_input_file_path;
	string init_input_file_path;

public:
	REACT_GEM(void);
	~REACT_GEM(void);

    
	TNode* m_Node; // Instance of TNode class
	// HS: 06.2007 Only set one TNode here, repeatedly use its resources for GEM calculation

	// DATABR structure for exchange with GEMIPM
	DATACH* dCH;   //pointer to DATACH
    DATABR* dBR;   //pointer to DATABR

	// Number of ICs, DCs, Phases and Phases-solutions kept in the node
	int nIC, nDC, nPH, nPS;

	// Data structure for each node to carry the chemical information (real FMT problems consider many nodes)
	// Names are consistent with the DataBridge structure (also see "\GEM\databr.h")
	vector<short> m_NodeHandle;
	vector<short> m_NodeStatusCH, m_IterDone;

	vector<double> m_T, m_P, m_Vs, m_Ms,
          m_Gs, m_Hs, m_IC, m_pH, m_pe,
          m_Eh;

	double *m_xDC, *m_gam, *m_xPH, *m_aPH, *m_vPS, *m_mPS, *m_bPS,
         *m_xPA, *m_dul, *m_dll, *m_bIC, *m_rMB, *m_uIC;

	// functions
	short Init_Nodes(string Project_path);	// Initialization of the GEM TNode Class
	//  return: 0-ok;1-loading init file failure;3-dch problem;4-dbr problem;
    short Init_RUN();                       // Run the node-GEM	
	//  return: 0-ok;5-GEM does not converge 
	short Run_MainLoop();
	//  return: 0-ok;5-GEM does not converge 

	string Get_Init_File_Path(void);
	string Get_DCH_File_Path(void);
	string Get_IPM_File_Path(void);
	string Get_DBR_File_Path(void);

	int Set_Init_File_Path(string m_path);
	int Set_DCH_FILE_PATH(string m_path);
	int Set_IPM_FILE_PATH(string m_path);
	int Set_DBR_FILE_PATH(string m_path);


	bool Load_Init_File(string m_Project_path);

	//---flags------
	short initialized_flag; //0- not initialized 1- initialized
	short heatflag;//0-initialized and not heat transport;1-heat_transport;
	short flowflag;//0-initialized;1-GROUNDWATER_FLOW;2-LIQUID_FLOW;3-RICHARDS_FLOW;4-FLOW;
	//--------------

	CRFProcess* m_pcs;//pointer to the PCS Class.

	long nNodes; // number of all nodes
	int GetHeatFlag_MT(void);
	int GetFlowType_MT(void);
	long GetNodeNumber_MT(void);

	short GetReactInfoFromMassTransport(int timelevel);
	short SetReactInfoBackMassTransport(int timelevel);
	void GetReactInfoFromGEM(long in);
	void SetReactInfoBackGEM(long in);


	double GetTempValue_MT(long node_Index, int timelevel);
	double GetPressureValue_MT(long node_Index, int timelevel);
	double GetComponentValue_MT(long node_Index, string m_component, int timelevel);
	short GetAqComponentValue_MT(long node_Index, int timelevel, double* m_Component);
	short GetSoComponentValue_MT(long node_Index, int timelevel, double* m_Component);

	short SetTempValue_MT(long node_Index, int timelevel, double temp);
	short SetPressureValue_MT(long node_Index, int timelevel, double pressure);
	short SetAqComponentValue_MT(long node_Index, int timelevel, double* m_Component);
	short SetSoComponentValue_MT(long node_Index, int timelevel, double* m_Component);
	short SetPHValue_MT(long node_Index, int timelevel, double m_PH);
	short SetPeValue_MT(long node_Index, int timelevel, double m_PE);

};
