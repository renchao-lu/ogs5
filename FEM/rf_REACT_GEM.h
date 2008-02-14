//-------------------------------------
// rf_REACT_GEM.h
// Haibing Shao 23.05.07
// haibing.shao@ufz.de
// GEM Reaction Package
// based on the PSI node-GEM source code
// using the node-GEM code from Paul Sherrer Institute (PSI)
//-------------------------------------

#include <time.h>
#include <cmath>
#include <string>
#include "rf_pcs.h"
#include "rfmat_cp.h"
#include "node.h"
#include "rf_mfp_new.h"

class REACT_GEM
{
private:
	string ipm_input_file_path;
	string dbr_input_file_path;
	string dbr_bc_input_file_path;
	string dch_input_file_path;
	string init_input_file_path;
	string init_input_file_sig;

public:
	REACT_GEM(void);
	~REACT_GEM(void);

    
	TNode* m_Node; // Instance of TNode class
	// HS: 06.2007 Only set one TNode here, repeatedly use its resources for GEM calculation

	// DATABR structure for exchange with GEMIPM
	DATACH* dCH;   //pointer to DATACH
    DATABR* dBR;   //pointer to DATABR

	// Number of ICs, DCs, Phases and Phases-solutions kept in the node or elem;
	int nIC, nDC, nPH, nPS;

	// Data structure for each node to carry the chemical information (real FMT problems consider many nodes)
	// Names are consistent with the DataBridge structure (also see "\GEM\databr.h")
	short *m_NodeHandle, *m_NodeStatusCH, *m_IterDone;
    short *m_ElemHandle, *m_ElemStatusCH, *m_IterDone_Elem;

	double *m_T, *m_P, *m_Vs, *m_Ms,
          *m_Gs, *m_Hs, *m_IC, *m_pH, *m_pe,
          *m_Eh;
	double *m_T_Elem, *m_P_Elem, *m_Vs_Elem, *m_Ms_Elem,
          *m_Gs_Elem, *m_Hs_Elem, *m_IC_Elem, *m_pH_Elem, *m_pe_Elem,
          *m_Eh_Elem;

	double *m_xDC, *m_gam, *m_xPH, *m_aPH, *m_vPS, *m_mPS, *m_bPS,
         *m_xPA, *m_dul, *m_dll, *m_bIC, *m_bIC_dummy, *m_rMB, *m_uIC; 

    
	double *m_xDC_Elem, *m_gam_Elem, *m_xPH_Elem, *m_aPH_Elem, *m_vPS_Elem, *m_mPS_Elem, *m_bPS_Elem,
         *m_xPA_Elem, *m_dul_Elem, *m_dll_Elem, *m_bIC_Elem, *m_bIC_dummy_Elem, *m_rMB_Elem, *m_uIC_Elem;

    // previous time step DC values
    double *m_xDC_pts;        // previous time step Concentration;
    double *m_xDC_MT_delta;   // delta C from Mass Transport;
    double *m_xDC_Chem_delta; // delta C from Chemistry;

    double *m_xDC_pts_Elem;

	CFluidProperties* m_FluidProp;
	CRFProcess* m_pcs;//pointer to the PCS Class.		

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
	int* mp_nodeTypes;

	//---flags------
	int initialized_flag; //0 - not initialized 1 - initialized
    int flag_node_element_based; // 0 - node based; 1 - element based;
    int flag_iterative_scheme;   // 0 - sequential non-iterative scheme;
                                 // 1 - standard iterative scheme;
                                 // 2 - symetric iterative scheme;
                                 // 3 - strang splitting scheme;
	int heatflag;//0-initialized and not heat transport;1-heat_transport;
	int flowflag;//0-initialized;1-GROUNDWATER_FLOW;2-LIQUID_FLOW;3-RICHARDS_FLOW;4-FLOW;
	//--------------

	long nNodes; // number of all nodes;
    long nElems; // number of all elements;
	int GetHeatFlag_MT(void);
	int GetFlowType_MT(void);
	long GetNodeNumber_MT(void);
    long GetElemNumber_MT(void);
	void GetFluidProperty_MT(void);

	short GetReactInfoFromMassTransport(int timelevel);
    short SetReactInfoBackMassTransport(int timelevel);
    void GetReactInfoFromGEM(long in);
	void SetReactInfoBackGEM(long in);


	double GetTempValue_MT(long node_Index, int timelevel);
	double GetPressureValue_MT(long node_Index, int timelevel);
	double GetComponentValue_MT(long node_Index, string m_component, int timelevel);
	short GetDCValue_MT(long node_Index, int timelevel, double* m_DC, double* m_DC_pts, double* m_DC_MT_delta);
	short GetSoComponentValue_MT(long node_Index, int timelevel, double* m_Phase);

	short SetTempValue_MT(long node_Index, int timelevel, double temp);
	short SetPressureValue_MT(long node_Index, int timelevel, double pressure);
	short SetDCValue_MT(long node_Index, int timelevel, double* m_DC);
	short SetSoComponentValue_MT(long node_Index, int timelevel, double* m_Phase);
	short SetPHValue_MT(long node_Index, int timelevel, double m_PH);
	short SetPeValue_MT(long node_Index, int timelevel, double m_PE);

    // Copy current values into previous time step values
    void CopyCurXDCPre(void);
    void UpdateXDCChemDelta(void);

    // Element and Node value conversion
    void ConvElemValue2Node(void);
    void ConvNodeValue2Elem(void);

	// Unit conversion for pressures
	double Pressure_Pa_2_Bar(double Pre_in_Pa);
	double Pressure_Bar_2_Pa(double Pre_in_Bar);
	double Pressure_M_2_Bar(double Pre_in_M, double flu_density );
	double Pressure_Bar_2_M(double Pre_in_Bar, double flu_density );

    // Calculate the volume of the nodes;
    // given argument is the index of one particular node;
    double GetNodeAdjacentVolume(long Idx_Node);

	// MPI implementation
	void GetGEMResult_MPI(void);
    void CleanMPIBuffer(void);

	// Definition of buffer variables
	short *m_NodeHandle_buff, *m_NodeStatusCH_buff, *m_IterDone_buff;
	double *m_Vs_buff, *m_Ms_buff, *m_Gs_buff, *m_Hs_buff, *m_IC_buff, *m_pH_buff, *m_pe_buff, *m_Eh_buff;
	double *m_rMB_buff, *m_uIC_buff, *m_xDC_buff, *m_gam_buff, *m_xPH_buff, *m_vPS_buff, *m_mPS_buff, *m_bPS_buff, *m_xPA_buff;

};


