#ifndef RF_REACT_BRNS_H
#define RF_REACT_BRNS_H

#include "Configure.h"

#include "rf_pcs.h"
#include "rfmat_cp.h"
#include "rf_mfp_new.h"
#include <time.h>

#ifdef BRNS
#ifdef WIN32
#pragma once
extern "C" __declspec( dllimport )  void brnsIsAlive();
extern "C" __declspec( dllimport )  void invokebrns(double* theCurArray, double* thePreArray, double* outputArray, int sizeOfArray, double time_step, int* boundary_flag, int* returnValue, double pos_x, double pos_y, double pos_z);
#endif
#endif

class REACT_BRNS
{
public:
    REACT_BRNS(void);
    ~REACT_BRNS(void);
    
    // flag, whether BRNS is initialized; 1-true; 0-false;
    bool init_flag;

    // Data structure storing the Concentration;
    double* cur_ts_Conc;
    double* pre_ts_Conc;
    double* m_dC_Chem_delta;
	int* boundary_flag;

    // array of return values of BRNS;
    int* rt_BRNS;
    
    #ifdef GCC
    void *hDll, *hDll_1, *hDll_2;
    typedef void (* LPFNDLLFUNC)(double*, double*, double*, int&, double&, int*, int*, double&, double&, double&);
    LPFNDLLFUNC invokebrns;
    #endif

    #ifdef USE_MPI_BRNS
    // buffer for the MPI implementation
    double* pre_ts_Conc_buf; 
    int* rt_BRNS_buf;
    #endif
    
    // pointer to the PCS Class;
    CRFProcess *m_pcs, *this_pcs;

    // pointer to the MCP Class;
    CompProperties *m_cp; 

    // number of Components passed to BRNS;
    int num_Comp;  

    // number of nodes;
    long nNodes;

    // This is just a test run of BRNS dll;
    void TestRUN(void);

    // The Run function
    void RUN(double time_step);

    // Get the number of Nodes in GeoSys;
    long GetNodesNumber(void);

    // Get the number of Components in GeoSys;
    int GetCompsNumber(void);

    // Initialize Data Structure;
    void InitBRNS(void);

    // Data transfer btw GeoSys and BRNS;
    void GSRF2Buffer( long i );
    void Buffer2GSRF( long i );


	// BC node checking
	int IsThisPointBCIfYesStoreValue(int index, CRFProcess* m_pcs, double& value);

private:
	// For measuring the time spent in BRNS calls
	double timeSpentInBrns;
	clock_t startTime;

#ifdef USE_MPI_BRNS    
    // MPI Buffer Value Manipulation
    void GetBRNSResult_MPI(void);
    void CleanMPIBuffer(void);
#endif

};

#endif
