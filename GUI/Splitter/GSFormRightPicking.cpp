// GSFormRightPicking.cpp : implementation file
//
#include "stdafx.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSFormRightPicking.h"
#include "GeoSysTreeView.h"
#include "COGLPickingView.h"
#include "GSForm3DLeft.h"
//GEOLIB
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLIB
#include "msh_elements_rfi.h"
#include "msh_quality.h"
#include "dtmesh.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "rf_fluid_momentum.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"
#include ".\gsformrightpicking.h"

// GUI
#include "ProgressBar.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CGSFormRightPicking

IMPLEMENT_DYNCREATE(CGSFormRightPicking, CFormView)

CGSFormRightPicking::CGSFormRightPicking()
	: CFormView(CGSFormRightPicking::IDD)
{
    fem = NULL;
}

CGSFormRightPicking::~CGSFormRightPicking()
{
}

void CGSFormRightPicking::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_PID, m_PID);
}

BEGIN_MESSAGE_MAP(CGSFormRightPicking, CFormView)
    ON_BN_CLICKED(IDC_BUTTON4, OnSimulateUnderDeveloperMode)
//    ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
    ON_BN_CLICKED(IDC_PTVALUE, OnBnClickedPtvalue)
    ON_BN_CLICKED(IDC_VELOCITYVECTOR, OnBnClickedVelocityvector)
    ON_BN_CLICKED(IDC_SHOWPARTICLE, OnBnClickedShowparticle)
    ON_BN_CLICKED(IDC_INOROUT, OnBnClickedInorout)
	ON_BN_CLICKED(IDC_READPCT, OnBnClickedReadpct)
	ON_BN_CLICKED(IDC_CINELE, OnBnClickedCinele)
	ON_BN_CLICKED(IDC_ELEFROMNODE, OnBnClickedElefromnode)
	ON_BN_CLICKED(IDC_ELEFROMEDGE, OnBnClickedElefromedge)
	ON_BN_CLICKED(IDC_PICKCROSSROADS, OnBnClickedPickcrossroads)
	ON_BN_CLICKED(IDC_CROSSROADVEC, OnBnClickedCrossroadvec)
	ON_BN_CLICKED(IDC_TOXYPLANE, OnBnClickedToxyplane)
	ON_BN_CLICKED(IDC_SPARTICLE, OnBnClickedSparticle)
	ON_BN_CLICKED(IDC_NODBYPARTICLE, OnBnClickedNodbyparticle)
	ON_BN_CLICKED(IDC_PICKPLANE, OnBnClickedPickplane)
	ON_BN_CLICKED(IDC_ATBYPOLYLINE, OnBnClickedAtbypolyline)
	ON_BN_CLICKED(IDC_POUTSIDE, OnBnClickedPoutside)
	ON_BN_CLICKED(IDC_PINSIDE, OnBnClickedPinside)
	ON_BN_CLICKED(IDC_CID, OnBnClickedCid)
	ON_EN_CHANGE(IDC_PID, OnEnChangePid)
END_MESSAGE_MAP()


// CGSFormRightPicking diagnostics

#ifdef _DEBUG
void CGSFormRightPicking::AssertValid() const
{
	CFormView::AssertValid();
}

void CGSFormRightPicking::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// CGSFormRightPicking message handlers

void CGSFormRightPicking::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	// All the processes created so that we can display variables that we want?
	// Let's check this by comparing the size of pcs_vector
	// If the size of pcs_vector > 0, Yes. If not, No.
	// This check box is only active when PCS is created.
	if((int)pcs_vector.size() > 0)
		GetDlgItem(IDC_VELOCITYVECTOR)->EnableWindow();
	else
		GetDlgItem(IDC_VELOCITYVECTOR)->EnableWindow(FALSE);
}


void CGSFormRightPicking::OnSimulateUnderDeveloperMode()
{
	// Activate the vector button now for post processing
    GetDlgItem(IDC_VELOCITYVECTOR)->EnableWindow();
	// I'll start simulation here *argv[]
	char **argv = NULL;
	int numOfArguments = 2;
	int sizeOfWord = 100;
	argv = (char **)malloc(numOfArguments * sizeof(char *));
	// I'll assign two running arguments here.
	argv[0] = (char *)malloc(sizeOfWord * sizeof(char ));
	sprintf(argv[0],"rf4.exe");
	argv[1] = (char *)malloc(sizeOfWord * sizeof(char ));
    
	CGSProject* m_gsp = gsp_vector[0]; //TK it crash sometimes
	for(int i=0; i<= (int)m_gsp->base.size(); ++i)
		argv[1][i] = m_gsp->base[i];

	FileName = m_gsp->base;
	mainPCH ( numOfArguments, argv );

	delete m_gsp;
}

void CGSFormRightPicking::OnBnClickedPtvalue()
{
    // Let's open up a modal dialog for PT values.
    // There should picked elements before this button
    // So, this button should only be activated when elemens are selected.
    // If not, deactivated all the time.
    
    theApp.pPTValue.DoModal();
}

void CGSFormRightPicking::OnBnClickedVelocityvector()
{
    if(IsDlgButtonChecked(IDC_VELOCITYVECTOR))
		theApp.VelocitySwitch = 1;
	else 
		theApp.VelocitySwitch = 0;

    showChange();
}


void CGSFormRightPicking::showChange(void)
{
    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
	pView->Invalidate();    
}

void CGSFormRightPicking::OnBnClickedShowparticle()
{
    if(IsDlgButtonChecked(IDC_SHOWPARTICLE))
		theApp.ParticleSwitch = 1;
	else 
		theApp.ParticleSwitch = 0;

    showChange();
}

void CGSFormRightPicking::OnBnClickedInorout()
{
    int InOrOut = 100; // Just for test
    CFEMesh* m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
    //CElem* m_ele = m_msh->ele_vector[theApp.elementPickedTotal[0]];
    
    int count = 0;
    for(int i=0; i< m_msh->PT->numOfParticles; ++i)
    {
        InOrOut = m_msh->PT->IsTheParticleInThisElement(&(m_msh->PT->X[i].Now)); 

        if(InOrOut==1)
            ++count;
    }    

    int Result = 0;
    Result = count;


/*
    // Check for coordinate conversion
    double p[4];

    for(int i=0; i<4; ++i)
        p[i] = 0.0;
    
    p[0] = m_msh->PT->X[0].Now.x; p[1] = m_msh->PT->X[0].Now.y; p[2] = m_msh->PT->X[0].Now.z;

    m_ele = m_msh->ele_vector[m_msh->PT->X[0].Now.elementIndex];

    fem->computeJacobian(1);
    fem->UnitCoordinates(p);
*/
  
    m_msh->PT->InterpolateVelocityOfTheParticleByInverseDistance(&(m_msh->PT->X[0].Now));
    //double vx = m_msh->PT->X[0].Now.Vx; double vy = m_msh->PT->X[0].Now.Vy; double vz = m_msh->PT->X[0].Now.Vz;
    

    
}
void CGSFormRightPicking::OnBnClickedReadpct()
{
	CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0];  // Something must be done later on here.

	RandomWalk* RW = NULL;
	RW = m_msh->PT;
 
	static char BASED_CODE szFilter[] = "Mesh Configuration File (*.pct)|*.pct|All Files (*.*)|*.*||";
	CFileDialog pFlg(TRUE, "slt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL );

	if(pFlg.DoModal() == IDOK)
	{	
		string pct_file_name = pFlg.GetPathName();
		
		pct_file_name.erase(pct_file_name.find(".pct",0), 4);	// Getting rid of .pct
		PCTRead(pct_file_name);
	}
}

void CGSFormRightPicking::OnBnClickedCinele()
{
	CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0]; 

	RandomWalk* RW = NULL;
	RW = m_msh->PT;
	int numOfElement = (int)m_msh->ele_vector.size();

	// Temp store for concentrations
	double* conc = NULL;
	conc = new double[numOfElement]();

	// Progress bar stuff
	CProgressBar bar(_T("Computing concentration in elements... Please wait."), 60, numOfElement, TRUE);

	// Loop over the elements
	for(int i=0; i< numOfElement; ++i)
	{
		int elementCount = 0;

		// Loop over the particles
		for(int j=0; j< RW->numOfParticles; ++j)
		{
			if(i == RW->X[j].Now.elementIndex )
				++elementCount;
		}

		// Store the number of particles in the element
		conc[i] = elementCount;

		// Progress a bar
		bar.StepIt();
		PeekAndPump();
	}

	// Store for the plot from the techplot.
	static char BASED_CODE szFilter[] = "Mesh Configuration File (*.tec)|*.tec|All Files (*.*)|*.*||";
	CFileDialog pFlg(FALSE, "slt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL );

	
	if(pFlg.DoModal() == IDOK)
	{	
		FILE *tec_file;
		tec_file = fopen(pFlg.GetPathName(), "w");
		
/*
		// Heading for tecplot
		fprintf(tec_file, "VARIABLES = X,Y,Z,CONCENTRATION1\n");
		fprintf(tec_file, "ZONE T=\"1.000000000000e+00s\", N=%d, E=0, F=FEPOINT, ET=QUADRILATERAL\n", numOfElement); 
*/
		
		for(int i=0; i< numOfElement; ++i)
		{
			CElem* m_ele = m_msh->ele_vector[i];	

			double* center = m_ele->GetGravityCenter();

			fprintf(tec_file, "%e %e %e %e\n", center[0], center[1], center[2], conc[i]);
		}
		
		fflush(tec_file);
		fclose(tec_file);
	}

	delete [] conc;
}


BOOL CGSFormRightPicking::PeekAndPump()
{
	static MSG msg;

	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) {
		if (!AfxGetApp()->PumpMessage()) {
			::PostQuitMessage(0);
			return FALSE;
		}	
	}

	return TRUE;
}

/******************************************************************************
*	The following function will be used to search nodes on crossroad or
	on joints.
	It is velocity obatined from FM that determines the nodes.
******************************************************************************/
void CGSFormRightPicking::OnBnClickedElefromnode()
{
	CFEMesh* m_msh = fem_msh_vector[0]; 
	
	// Let's initilize the number of nodes picked.
	theApp.hitsElementTotal = 0;

	for(int i=0; i<theApp.hitsRFINodeTotal; ++i)
	{
		CNode* thisNode = m_msh->nod_vector[theApp.RFInodePickedTotal[i]];
		int NumOfNeighborElements = (int)thisNode->connected_elements.size();
	
		for(int j=0; j<NumOfNeighborElements; ++j)
		{
			++theApp.hitsElementTotal;
			theApp.elementPickedTotal = (int *)realloc(theApp.elementPickedTotal, theApp.hitsElementTotal*sizeof(int));

			// Let's get the element one by one.
			int index = thisNode->connected_elements[j];
			theApp.elementPickedTotal[theApp.hitsElementTotal-1] = index;
		}
	}

	// Display the assoicated elements picked 
	showChange();
}

void CGSFormRightPicking::OnBnClickedElefromedge()
{
	// This function is under development.

	CFEMesh* m_msh = fem_msh_vector[0]; 
	
	// Let's initilize the number of nodes picked.
	theApp.hitsElementTotal = 0;

	if(theApp.hitsRFINodeTotal == 2)
	{
		for(int i=0; i<theApp.hitsRFINodeTotal; ++i)
		{
			CNode* thisNode = m_msh->nod_vector[theApp.RFInodePickedTotal[i]];
			int NumOfNeighborElements = (int)thisNode->connected_elements.size();    
	
			for(int j=0; j<NumOfNeighborElements; ++j)
			{
				++theApp.hitsElementTotal;
				theApp.elementPickedTotal = (int *)realloc(theApp.elementPickedTotal, theApp.hitsElementTotal*sizeof(int));

				// Let's get the element one by one.
				int index = thisNode->connected_elements[j];
				theApp.elementPickedTotal[theApp.hitsElementTotal-1] = index;
			}
		}
	}
	else
	{
		CWnd * pWnd = NULL;
		pWnd->MessageBox("Only two nodes should be selected.","Number of nodes alert.", MB_ICONINFORMATION);
	}

	// Display the assoicated elements picked 
	showChange();	
}

void CGSFormRightPicking::OnBnClickedPickcrossroads()
{
	CFEMesh* m_msh = fem_msh_vector[0]; 
	
	// Let's initilize the number of nodes picked.
	theApp.hitsRFINodeTotal = 0;

	for(int i=0; i<(int)m_msh->nod_vector.size(); ++i) //OK
	{
		CNode* thisNode = m_msh->nod_vector[i];
		
		if(thisNode->crossroad == 1)
		{
			++theApp.hitsRFINodeTotal;
			theApp.RFInodePickedTotal = (int *)realloc(theApp.RFInodePickedTotal, theApp.hitsRFINodeTotal*sizeof(int));

			// Let's get the element one by one.
			theApp.RFInodePickedTotal[theApp.hitsRFINodeTotal-1] = i;
		}
	}

	// Display the assoicated elements picked 
	showChange();	
}

void CGSFormRightPicking::OnBnClickedCrossroadvec()
{
	// Let's select the crossroads
	OnBnClickedPickcrossroads();

	if(IsDlgButtonChecked(IDC_CROSSROADVEC))
		theApp.CrossroadSwitch = 1;
	else 
		theApp.CrossroadSwitch = 0;

    showChange();
}

void CGSFormRightPicking::OnBnClickedToxyplane()
{
	if(IsDlgButtonChecked(IDC_TOXYPLANE))
		theApp.GDebugSwitch = 1;
	else 
		theApp.GDebugSwitch = 0;

    showChange();	
}

void CGSFormRightPicking::OnBnClickedSparticle()
{
	if(IsDlgButtonChecked(IDC_SPARTICLE))
	{
		CFEMesh* m_msh = fem_msh_vector[0]; 

		theApp.hitsElementTotal = 0;
		for(int p=0; p<theApp.hitsParticleTotal; ++p)
		{
			for(int i=0; i<(int)m_msh->ele_vector.size(); ++i) //OK
			{
				if(m_msh->PT->X[theApp.ParticlePickedTotal[p]].Now.elementIndex 
					== i)
				{
					++theApp.hitsElementTotal;
					theApp.elementPickedTotal = (int *)realloc(theApp.elementPickedTotal, 
												theApp.hitsElementTotal*sizeof(int));

					// Let's get the element one by one.
					theApp.elementPickedTotal[theApp.hitsElementTotal-1] = i;
				}
			}
		}

		// Let's get rid of duplicates by doing so called compression.
		for(int j=0; j<theApp.hitsElementTotal; ++j)
		{
			for(int l=j+1; l<theApp.hitsElementTotal; ++l)
			{
				// Check two indeces are same.
				if( theApp.elementPickedTotal[j] == theApp.elementPickedTotal[l] )
				{
					// Two elements stay on the same plane
					for(int m = l; m < (theApp.hitsElementTotal - 1); ++m)
						theApp.elementPickedTotal[m] = theApp.elementPickedTotal[m+1];
					
					// delete the duplicates.
					--theApp.hitsElementTotal;
					--l;	// Very important. Huh.
				}
			} 	
		}
	}
	else 
		;

    showChange();
}

void CGSFormRightPicking::OnBnClickedNodbyparticle()
{
	if(IsDlgButtonChecked(IDC_NODBYPARTICLE))
	{
		CFEMesh* m_msh = fem_msh_vector[0]; 

		theApp.hitsRFINodeTotal = 0;
		for(int p=0; p<theApp.hitsParticleTotal; ++p)
		{
			for(int i=0; i<(int)m_msh->ele_vector.size(); ++i) //OK
			{
				if(m_msh->PT->X[theApp.ParticlePickedTotal[p]].Now.elementIndex 
					== m_msh->ele_vector[i]->GetIndex())
				{
					for(int j=0; j<m_msh->ele_vector[i]->GetFacesNumber(); ++j)
					{
						++theApp.hitsRFINodeTotal;
						theApp.RFInodePickedTotal = (int *)realloc(theApp.RFInodePickedTotal, 
							theApp.hitsRFINodeTotal*sizeof(int));

						// Let's get the node one by one
						theApp.RFInodePickedTotal[theApp.hitsRFINodeTotal-1] = 
							m_msh->nod_vector[m_msh->ele_vector[i]->GetNodeIndex(j)]->GetIndex();
					}
				}
			}
		}
	}
	else 
		;

    showChange();
}
#define PATCH
void CGSFormRightPicking::OnBnClickedPickplane()
{
	//double tolerance = 1e-1;
	// Mount the mesh and the picked element
	CFEMesh* m_msh = fem_msh_vector[0];
	CElem* theEle = NULL;
	if(theApp.hitsElementTotal == 1)
		theEle = m_msh->ele_vector[theApp.elementPickedTotal[0]];
	else
		;	// Please select one representive element.

#ifdef PLANE	
	// We are going to compare the norm vector of each element
	double NtheEle[3];
	for(int k=0; k<3; ++k) NtheEle[k] = theEle->getTransformTensor(k+6);
#endif

	// Initialize picking
	theApp.hitsElementTotal = 0;
	// Loop over the elements
	for(int i=0; i<(int)m_msh->ele_vector.size(); ++i) //OK
	{
		CElem* anEle = m_msh->ele_vector[i];
#ifdef PLANE
		double NanEle[3];
		for(int k=0; k<3; ++k) NanEle[k] = anEle->getTransformTensor(k+6);
		double xx = NtheEle[0] - NanEle[0], yy = NtheEle[1] - NanEle[1], zz = NtheEle[2] - NanEle[2];
		double distance = xx*xx + yy*yy + zz*zz;

		// If the normal vectors are same and the translation is the same, then pick
		if( fabs(xx) < tolerance && fabs(yy) < tolerance && fabs(zz) < tolerance &&
			fabs(theEle->GetAngle(2) - anEle->GetAngle(2)) < tolerance )
#endif			
#ifdef PATCH
		// Select by patch number
		//int patch = anEle->GetPatchIndex();
		if(anEle->GetPatchIndex() == 2)
#endif
		{
			++theApp.hitsElementTotal;
			theApp.elementPickedTotal = (int *)realloc(theApp.elementPickedTotal, 
										theApp.hitsElementTotal*sizeof(int));

			// Let's get the element one by one.
			theApp.elementPickedTotal[theApp.hitsElementTotal-1] = i;
		}
	}
}

void CGSFormRightPicking::OnBnClickedAtbypolyline()
{
	theApp.pPTValue.DoModal();	
}

void CGSFormRightPicking::OnBnClickedPoutside()
{
	// This is termperary measure only for single mesh cass
    CFEMesh* m_msh = fem_msh_vector[0];
	theApp.hitsParticleTotal = 0;
	for(int i=0; i < m_msh->PT->numOfParticles ; ++i)        
	{
		if(m_msh->PT->X[i].Now.elementIndex == -10)
		{
			++theApp.hitsParticleTotal;
			theApp.ParticlePickedTotal = (int *)realloc(theApp.ParticlePickedTotal, theApp.hitsParticleTotal*sizeof(int));
			theApp.ParticlePickedTotal[theApp.hitsParticleTotal-1] = i;
		}
	}	
}

void CGSFormRightPicking::OnBnClickedPinside()
{
	// This is termperary measure only for single mesh cass
    CFEMesh* m_msh = fem_msh_vector[0];
	theApp.hitsParticleTotal = 0;
	for(int i=0; i < m_msh->PT->numOfParticles ; ++i)        
	{
		if(m_msh->PT->X[i].Now.elementIndex != -10)
		{
			++theApp.hitsParticleTotal;
			theApp.ParticlePickedTotal = (int *)realloc(theApp.ParticlePickedTotal, theApp.hitsParticleTotal*sizeof(int));
			theApp.ParticlePickedTotal[theApp.hitsParticleTotal-1] = i;
		}
	}	
}

void CGSFormRightPicking::OnBnClickedCid()
{
	UpdateData(TRUE);

	CFEMesh* m_msh = fem_msh_vector[0]; 

	for(int p=0; p<theApp.hitsParticleTotal; ++p)
	{
		m_msh->PT->X[theApp.ParticlePickedTotal[p]].Past.identity = m_msh->PT->X[theApp.ParticlePickedTotal[p]].Now.identity = m_PID;
	}
		
}

void CGSFormRightPicking::OnEnChangePid()
{
	UpdateData(TRUE);
	theApp.PID = m_PID;
}
