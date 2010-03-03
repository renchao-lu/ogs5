#include "rf_REACT_BRNS.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"
#include <iostream>

#ifdef BRNS

#ifdef BRNS_OMP
#include <omp.h>
#define NUM_THREADS 2
#endif

#ifdef USE_MPI_BRNS
//#undef SEEK_SET
//#undef SEEK_CUR
//#undef SEEK_END
#include "mpi.h"//Parallel Computing Support
#include "par_ddc.h"
// HS 07.01.2008: Comment the following 2 lines on LiClus.
// int size;
// int myrank;

#endif

#ifdef GCC
#include <dlfcn.h>
#endif

using namespace std;

REACT_BRNS::REACT_BRNS(void)
{
    // number of Components;
    num_Comp = 0;
    
    // set initialized flag to false by default;
    init_flag = false;

	// set timer to zero
    timeSpentInBrns = 0;
    cout << "Debugging #1." << endl;
#ifdef GCC

    hDll_1 = dlopen("./libblas.so.3", RTLD_NOW);
    cout << "Debugging #2." << endl;
    hDll_2 = dlopen("./liblapack.so.3", RTLD_NOW);
    cout << "Debugging #3." << endl;
    hDll = dlopen("./brns.so", RTLD_NOW);
    cout << "Debugging #4." << endl;
    invokebrns = (LPFNDLLFUNC)dlsym(hDll, "invokebrns_");
#endif
    cout << "Debugging #5." << endl;
}

REACT_BRNS::~REACT_BRNS(void)
{
    // reclaim the memory;
    delete[] rt_BRNS;
    delete[] cur_ts_Conc;
    delete[] pre_ts_Conc;
    delete[] m_dC_Chem_delta;
    delete[] boundary_flag;

    #ifdef GCC
    dlclose(hDll);
    dlclose(hDll_1);
    dlclose(hDll_2);
    #endif

    #ifdef USE_MPI_BRNS
    delete[] rt_BRNS_buf;
    delete[] pre_ts_Conc_buf;
    #endif
}

void REACT_BRNS::TestRUN(void)
{
	cout << "Hello World!" << endl;
	cout << "Trying to call BrnsDLL ..." << endl;
	// brnsIsAlive();
	cout << "Hm, looks good, I guess." << endl;

	double* myArray = NULL;
	int sizeOfArray;
	int returnValue = -111;

	sizeOfArray=6;
	myArray = new double[sizeOfArray];

	myArray[0] = 1.2;
	myArray[1] = 3.4;
	myArray[2] = 5.6;
	myArray[3] = 7.8;
	myArray[4] = 9.10;
	myArray[5] = 11.12;

	cout << "Initial Concentrations:" << endl;
	for (int i = 0 ; i < sizeOfArray; i++){
		cout << i << ": " << myArray[i] << endl;
	}

	cout << "Calling invokebrns() ..." << endl;

	//invokebrns(myArray, sizeOfArray, &returnValue);

	cout << "Returning from invokebrns(), return code " << returnValue << endl;
	cout << "New Concentrationvector:" << endl;

	for (int i = 0 ; i < sizeOfArray; i++){
		cout << i << ": " << myArray[i] << endl;
	}
	
	delete [] myArray;
	myArray = NULL;

return;
}

long REACT_BRNS::GetNodesNumber(void)
{
	long number;
    number =0;
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
	return number;
}

int REACT_BRNS::GetCompsNumber(void)
{
    long number;
    number = 0;
    
    number = (int)cp_vec.size();

    return number;
}

void REACT_BRNS::InitBRNS(void)
{
    num_Comp = GetCompsNumber();
    nNodes = GetNodesNumber();

    // initialize the array;
    cur_ts_Conc = new double[num_Comp*nNodes];
    pre_ts_Conc = new double[num_Comp*nNodes];
    m_dC_Chem_delta = new double[num_Comp*nNodes];

    rt_BRNS = new int[nNodes];
	boundary_flag = new int[num_Comp*nNodes];

    #ifdef USE_MPI_BRNS
    pre_ts_Conc_buf = new double[num_Comp*nNodes];
    rt_BRNS_buf = new int[nNodes];
    #endif

    // Giving initial values
    for ( int i=0; i < num_Comp*nNodes ; i++ )
    {
        cur_ts_Conc[i] = 0.0;
        pre_ts_Conc[i] = 0.0;
        m_dC_Chem_delta[i] = 0.0;
    }
    for ( long k=0; k < nNodes ; k++)
    {
        rt_BRNS[k] = -111;
    }

    if ( num_Comp > 0 && nNodes > 0 )
    {init_flag = true;}
    else
    {
          #ifdef MFC
            AfxMessageBox("!!! Node number and Components Number must be bigger than zero!");
          #endif
          DisplayErrorMsg("!!! Node number and Components Number must be bigger than zero!");
          abort();
    }

	// Marking species with fixed concentrations (in boundary nodes)

	double BCValue = 0.0;
	int p, k;
	for (long i=0 ; i < nNodes ; i++ ){
		for ( k = 0 ; k < num_Comp ; k++ )
		{
			this_pcs = NULL;
			m_cp = cp_vec[k];

			// Get the pointer to the proper PCS. 
			this_pcs = PCSGet( "MASS_TRANSPORT", m_cp->compname );
			if (this_pcs)
			{
				for(p=0; p<this_pcs->pcs_number_of_primary_nvals; ++p)
				{
					// Let's print BC and ST values
					CBoundaryConditionsGroup *m_bc_group = NULL;

					m_bc_group = BCGetGroup(this_pcs->pcs_type_name,this_pcs->pcs_primary_function_name[p]);
				
					// BC printing
					if( IsThisPointBCIfYesStoreValue(i, this_pcs, BCValue) )
					{
						// If this node is on the fixed boudnary for this component
						boundary_flag[i*num_Comp+k] = 1;
						cout << "Node " << i <<", Comp " << k << ",Value " << BCValue << endl;
					}
					else
					{
						// If this node is NOT on the fixed boudnary for this component
						boundary_flag[i*num_Comp+k] = 0;
					}
				}
			}
			else  // not getting the pointer to the proper PCS. 
			{
				#ifdef MFC
				  AfxMessageBox("!!! In Data transfer for BRNS, can not find corresponding PCS!");
				#endif
				  DisplayErrorMsg("!!! In InitBRNS, can not find corresponding PCS!");
				abort();
			}
		
		}
	}
}

void REACT_BRNS::GSRF2Buffer(long i)
{
      // for this node, loop over all the chemical components and update the values
      for ( int k = 0 ; k < num_Comp ; k++ )
      {
          this_pcs = NULL;
          m_cp = cp_vec[k];

          // Get the pointer to the proper PCS. 
          this_pcs = PCSGet( "MASS_TRANSPORT", m_cp->compname );
          if (this_pcs)
          {
              // Set the Concentration of this component
              cur_ts_Conc[i*num_Comp+k] = this_pcs->GetNodeValue( i , this_pcs->GetNodeValueIndex( this_pcs->pcs_primary_function_name[0] )+ 1 );
              pre_ts_Conc[i*num_Comp+k] = this_pcs->GetNodeValue( i , this_pcs->GetNodeValueIndex( this_pcs->pcs_primary_function_name[0] )+ 0 );

          }
          else  // not getting the pointer to the proper PCS. 
          {
              #ifdef MFC
                AfxMessageBox("!!! In Data transfer for BRNS, can not find corresponding PCS!");
              #endif
                DisplayErrorMsg("!!! In Data transfer for BRNS, can not find corresponding PCS!");
              abort();
          }
      }
}

void REACT_BRNS::Buffer2GSRF(long i)
{
      // for this node, loop over all the chemical components and update the values
      for ( int k = 0 ; k < num_Comp ; k++ )
      {
          this_pcs = NULL;
          m_cp = cp_vec[k];

          // Get the pointer to the proper PCS. 
          this_pcs = PCSGet( "MASS_TRANSPORT", m_cp->compname );
          if (this_pcs)
          {
              // Set the Concentration of this component at current time step;
              this_pcs->SetNodeValue( i, this_pcs->GetNodeValueIndex(this_pcs->pcs_primary_function_name[0])+1/*1-current time step*/,pre_ts_Conc[i*num_Comp+k]);
          }
          else  // not getting the pointer to the proper PCS. 
          {
              #ifdef MFC
                AfxMessageBox("!!! In Data transfer for BRNS, can not find corresponding PCS!");
              #endif
                DisplayErrorMsg("!!! In Data transfer for BRNS, can not find corresponding PCS!");
              abort();
          }
      }
}

void REACT_BRNS::RUN(double time_step)
{
      long i;

      // Loop over all nodes to transfer data 
      for ( i=0 ; i < nNodes ; i++ )
      {
          // Get Conc Data from GSRF;
          GSRF2Buffer(i);
      }

	  double pos_x;
	  double pos_y;
	  double pos_z;

	  //------------end of reading number of nodes---- 



      // Run BRNS
#ifdef BRNS_OMP
      // do it with OpenMP
      int num_Comp_temp;double* cur_ts_Conc_temp;double* pre_ts_Conc_temp;long nNodes_temp;int* boundary_flag_temp;
      num_Comp_temp = num_Comp;
      cur_ts_Conc_temp = cur_ts_Conc;
      pre_ts_Conc_temp = pre_ts_Conc;
      boundary_flag_temp = boundary_flag;
      nNodes_temp = nNodes;
      omp_set_num_threads((int)NUM_THREADS);
      cout << "Max. thread num is:" << omp_get_max_threads() << endl;

      #pragma omp parallel for shared(nNodes_temp, num_Comp_temp, time_step, cur_ts_Conc_temp, pre_ts_Conc_temp, boundary_flag_temp) private(i,pos_x,pos_y,pos_z) 
      for ( i=0 ; i < nNodes_temp ; i++ )
      {   
	  pos_x = m_pcs->m_msh->nod_vector[i]->X();
	  pos_y = m_pcs->m_msh->nod_vector[i]->Y();
	  pos_z = m_pcs->m_msh->nod_vector[i]->Z();
          // Run BRNS;
          invokebrns( &(cur_ts_Conc_temp[i*num_Comp_temp]), &(pre_ts_Conc_temp[i*num_Comp_temp]), &(pre_ts_Conc_temp[i*num_Comp_temp]), num_Comp_temp, time_step, &(boundary_flag_temp[i*num_Comp_temp]),  &(rt_BRNS[i]), pos_x, pos_y, pos_z );
          int num = omp_get_thread_num ();
          cout << "#" << num << "thread reporting. My i is: " << i /*cur_ts_Conc_temp[i*num_Comp_temp]*/ << endl;
          // cout << endl << "Number of threads " << omp_get_num_threads ();
      }
#else
#ifdef USE_MPI_BRNS
      MPI_Bcast(&nNodes, 1, MPI_LONG, 0, MPI_COMM_WORLD );
      MPI_Bcast(&num_Comp, 1, MPI_INT, 0, MPI_COMM_WORLD );
      for ( i=myrank ; i < nNodes ; i+=mysize )
#else
      for ( i=0 ; i < nNodes ; i++ )
#endif
      {   
		  // Check out if this node is on the boundary - A fixed boundary condition
	  pos_x = m_pcs->m_msh->nod_vector[i]->X();
	  pos_y = m_pcs->m_msh->nod_vector[i]->Y();
	  pos_z = m_pcs->m_msh->nod_vector[i]->Z();
		//pos_x = 0.1;
		//pos_y = 0.1;
		//pos_z = 0.1;

          // Run BRNS;
#ifdef USE_MPI_BRNS
	// cout << " Num of components is: " << num_Comp << endl;
			if (myrank==1) startTime = clock(); // in MPI: make statistics only when running on one processor
            invokebrns( &(cur_ts_Conc[i*num_Comp]), &(pre_ts_Conc[i*num_Comp]), &(pre_ts_Conc_buf[i*num_Comp]), num_Comp, time_step, &(boundary_flag[i*num_Comp]), &(rt_BRNS[i]), pos_x, pos_y, pos_z );
			if (myrank==1) timeSpentInBrns += (double) (clock() - startTime) / CLOCKS_PER_SEC;
#else
			startTime = clock();
			cout << "debug #6." << endl;
            invokebrns( &(cur_ts_Conc[i*num_Comp]), &(pre_ts_Conc[i*num_Comp]), &(pre_ts_Conc[i*num_Comp]), num_Comp, time_step, &(boundary_flag[i*num_Comp]), &(rt_BRNS[i]), pos_x, pos_y, pos_z );
	    cout << "debug #7." << endl;
			timeSpentInBrns += (double) (clock() - startTime) / CLOCKS_PER_SEC;
#endif
      }
#endif
#ifdef USE_MPI_BRNS
      GetBRNSResult_MPI();
      CleanMPIBuffer();
	  if (myrank==1) cout << "Total time spent in BRNS (invokebrns()): " << timeSpentInBrns << "s" << endl;
#else
	  cout << "Total time spent in BRNS (invokebrns()): " << timeSpentInBrns << "s" << endl;
#endif

      // calculate dC
      for ( i=0 ; i < nNodes*num_Comp ; i++)
        m_dC_Chem_delta[i] = cur_ts_Conc[i] - pre_ts_Conc[i];
           
      // Loop over all nodes to retrieve data
      for ( i=0 ; i < nNodes ; i++ )
      {  
          // Set data back to GSRF
          Buffer2GSRF(i);
#ifndef BRNS_NO_LOG		// should be eventually changed to #ifdef BRNS_LOG
		  switch (rt_BRNS[i]) {
			case 1:
				cout << "In Node " << i << ": BRNS calculated negative concentration!" << endl;
				break;
			case 2:
				cout << "In Node " << i << ": BRNS exceeded max newton iterations!" << endl;
				break;
			case 3:
				cout << "In Node " << i << ": BRNS calculated negative concentration, " <<
					    "and exceeded max newton iterations!" << endl;
				break;
//			default:
		  }
		  if (rt_BRNS[i] == 1 || rt_BRNS[i] == 3 ){
			  cout << "Still negative concentrations after chemical simulation for:";
			  for (int j = 0; j < num_Comp; j++){
				  if (pre_ts_Conc[i*num_Comp+j] < 0.0)
					  cout << " Species " << j+1 << ": " << pre_ts_Conc[i*num_Comp+j];
			  }
			  cout << endl;
		  }
#endif
	  }

}


int REACT_BRNS::IsThisPointBCIfYesStoreValue(int index, CRFProcess* m_pcs, double& value)
{
   for(int p=0; p< (int)m_pcs->bc_node_value.size(); ++p)	
   {
     if(index == m_pcs->bc_node_value[p]->msh_node_number)
     {
         value = m_pcs->bc_node_value[p]->node_value;
         return 1; // Yes, found it.
     }
   }

        return 0;
}

#ifdef USE_MPI_BRNS
void REACT_BRNS::GetBRNSResult_MPI(void)
{
    // Retrieve the values from MPI buffer to the main memory 
    MPI_Allreduce( rt_BRNS_buf, rt_BRNS, nNodes, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce( pre_ts_Conc_buf, pre_ts_Conc, num_Comp*nNodes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

}

void REACT_BRNS::CleanMPIBuffer(void)
{
    long in;
    for ( in = 0; in < nNodes ; in++) rt_BRNS_buf[in]=0;
    for ( in = 0; in < num_Comp*nNodes ; in++) pre_ts_Conc_buf[in]=0.0;

}
#endif


#endif // end of BRNS

