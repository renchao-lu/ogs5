/**************************************************************************
FEMLib-Object: 
Task: MediumProperties
Programing:
05/2005 PCH Implementation
**************************************************************************/

#include "stdafx.h" //MFC

// C++ STL
#include <iostream>
using namespace std;

#include "rf_fluid_momentum.h"
#include "elements.h" //OK
//#include "rf_pcs.h"

/**************************************************************************
FEMLib-Method: CFluidMomentum
Task: constructor
Programing:
05/2005 PCH Implementation
last modification:
**************************************************************************/
CFluidMomentum::CFluidMomentum(void)
{
	m_pcs = NULL;
}

/**************************************************************************
FEMLib-Method: CFluidMomentum
Task: destructor
Programing:
05/2005 PCH Implementation
last modification:
**************************************************************************/
CFluidMomentum::~CFluidMomentum(void)
{
	
}

/**************************************************************************
FEMLib-Method: double Execute()
Task: compute the Darcy velocity on node
Programing:
05/2005 PCH Implementation
last modification:
**************************************************************************/
double CFluidMomentum::Execute()
{
	double pcs_error = 0.0;

	int no_processes = (int)pcs_vector.size();

	Create();

	for(int i=0; i< no_processes; ++i)
	{
		m_pcs = pcs_vector[i];
        
        // Select the mesh whose process name has "FLOW"
        if( m_pcs->pcs_type_name.find("FLOW")!=string::npos)
            m_msh = fem_msh_vector[i];
		if(m_pcs->pcs_type_name.find("FLUID_MOMENTUM")!=string::npos)
			SolveDarcyVelocityOnNode();
	}

	return pcs_error;
}

/**************************************************************************
FEMLib-Method: SolveDarcyVelocityOnNode(CRFProcess*m_pcs)
Task: compute the Darcy velocity on node
Programing:
05/2005 PCH Implementation
last modification:
**************************************************************************/
void CFluidMomentum::SolveDarcyVelocityOnNode()
{
	int nidx1;
    long i;
    CElem* elem = NULL;
	
	fem = new CFiniteElementStd(m_pcs, m_msh->GetCoordinateFlag()); 


	// Loop over three dimension to solve three velocity components
	for (int phase=0;phase<GetRFProcessNumPhases();phase++)  
	{
		for(int dimension=0; dimension < 3; ++dimension)
		{
		
			/* Initializations */
			/* System matrix */
			SetZeroLinearSolver(m_pcs->eqs);
  
            for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
            {
                elem = m_msh->ele_vector[i];
                if (elem->GetMark()) // Marked for use
                {
                    fem->ConfigElement(elem);
                    fem->Assembly(dimension);
                } 
            }

			// Solve for velocity
			ExecuteLinearSolver(m_pcs->eqs);

            /* Store solution vector in model node values table */
            nidx1 = m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[dimension])+1;
     		for(int j=0;j<m_pcs->eqs->dim;j++)
               m_pcs->SetNodeValue(m_msh->Eqs2Global_NodeIndex[j],nidx1,m_pcs->eqs->x[j]);
		}
    
        // Obtain element-based velocity
        for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
        {
            elem = m_msh->ele_vector[i];
                

            double vx = 0.0, vy = 0.0, vz = 0.0;
			int numOfNodeInElement = elem->GetVertexNumber(); 

			for(int j=0;j< numOfNodeInElement;++j)
			{
                vx += m_pcs->GetNodeValue(elem->GetNodeIndex(j), m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);  
                vy += m_pcs->GetNodeValue(elem->GetNodeIndex(j), m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);  
                vz += m_pcs->GetNodeValue(elem->GetNodeIndex(j), m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);  
			}
			vx /= (double)numOfNodeInElement; vy /= (double)numOfNodeInElement; vz /= (double)numOfNodeInElement;

/*				
			switch(phase) 
			{
				case 0:
*/
                    m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY1_X")+1, vx);
                    m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY1_Y")+1, vy);
                    m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY1_Z")+1, vz);
/*
					break;
				case 1:
					m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY2_X")+1, vx);
                    m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY2_Y")+1, vy);
                    m_pcs->SetElementValue(i, m_pcs->GetElementValueIndex("VELOCITY2_Z")+1, vz);
					break;
				default:
					cout << "Error in VELCalcElementVelocity: invalid phase number" << endl;
			}
*/
		}

	}

	// Release memroy
	delete fem;	
}

/**************************************************************************
FEMLib-Method: void Create()
Task: This only creates NUM nothing else
Programing:
06/2005 PCH Implementation
last modification:
**************************************************************************/
void CFluidMomentum::Create()
{
	// NUM_NEW
	int no_numerics = (int)num_vector.size();
	
	CNumerics* m_num_tmp = NULL;
	
	for(int i=0;i<no_numerics;i++)
	{
		m_num_tmp = num_vector[i];
		
		if(m_num_tmp->pcs_type_name.compare("FLUID_MOMENTUM") == 0 )
			m_num = m_num_tmp;
	}
	if(!m_num)
		cout << "Warning in CRFProcess::Create() - no numerical properties" << endl;
	else
	{
		pcs_nonlinear_iterations = m_num->nls_max_iterations;
		pcs_nonlinear_iteration_tolerance = m_num->nls_error_tolerance;
	}
}
void FMRead(string file_base_name)
{
    // Fluid_Momentum memory allocation is moved here. by PCH
  //  CRFProcess* m_pcs = PCSGet("FLUID_MOMENTUM");
  //  if(!m_pcs)
  //  {
  if(fem_msh_vector.size()==0) //OK
    return;
        CFEMesh* m_msh = fem_msh_vector[0];  // Something must be done later on here.
        m_msh->fm_pcs = new CFluidMomentum ();
  //  }
}

/**************************************************************************
ROCKFLOW - Funktion: DATWriteFile
Task: Write PCT file
Programing:
10/2005   PCH   Implementation
**************************************************************************/
void DATWriteHETFile(const char *file_name)
{
    FILE *tet_file = NULL;
    char tet_file_name[MAX_ZEILE];
	double* center = NULL;
    CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0];  // Something must be done later on here.
	CElem* elem = NULL;
									
	sprintf(tet_file_name,"%s.%s",file_name,"tet");
    tet_file = fopen(tet_file_name,"w+t");
	// Obtain element-based velocity
    for (int i = 0; i < (long)m_msh->ele_vector.size(); i++)
    {
		elem = m_msh->ele_vector[i];
		center = elem->GetGravityCenter();
																		  
  		fprintf(tet_file, "%17.12e %17.12e %17.12e %17.12e\n",
			center[0], center[1], center[2], elem->mat_vector(0)*1e7); 
	}

    // Let's close it, now
    fclose(tet_file);
}
