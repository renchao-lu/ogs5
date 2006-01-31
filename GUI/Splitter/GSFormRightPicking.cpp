// GSFormRightPassive.cpp : implementation file
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
}

BEGIN_MESSAGE_MAP(CGSFormRightPicking, CFormView)
    ON_BN_CLICKED(IDC_BUTTON4, OnSimulateUnderDeveloperMode)
    ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
    ON_BN_CLICKED(IDC_PTVALUE, OnBnClickedPtvalue)
    ON_BN_CLICKED(IDC_VELOCITYVECTOR, OnBnClickedVelocityvector)
    ON_BN_CLICKED(IDC_SHOWPARTICLE, OnBnClickedShowparticle)
    ON_BN_CLICKED(IDC_INOROUT, OnBnClickedInorout)
	ON_BN_CLICKED(IDC_READPCT, OnBnClickedReadpct)
	ON_BN_CLICKED(IDC_CINELE, OnBnClickedCinele)
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

	mainPCH ( numOfArguments, argv );

	delete m_gsp;
}

void CGSFormRightPicking::OnBnClickedButton5()
{
    // PCH Monitoring implementation for temporary use
    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
    mainframe->OnPickedProperty();
    // PCH Ends here
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
    CElem* m_ele = m_msh->ele_vector[theApp.elementPickedTotal[0]];
    
    int count = 0;
    for(int i=0; i< m_msh->PT->numOfParticles; ++i)
    {
        InOrOut = m_msh->PT->IsTheParticleInThisElement(&(m_msh->PT->X[i].Now), m_ele); 

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
  
    m_msh->PT->InterpolateVelocityOfTheParticle(&(m_msh->PT->X[0].Now), m_ele);
    double vx = m_msh->PT->X[0].Now.Vx; double vy = m_msh->PT->X[0].Now.Vy; double vz = m_msh->PT->X[0].Now.Vz;
    

    double ok = 1.0;
    
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
		FILE *pct_file;

		pct_file = fopen(pFlg.GetPathName(), "r");

		// Later on from this line, I can put which mesh I am dealing with. 
		fscanf(pct_file,"%d", &(RW->numOfParticles));

		for(int i=0; i< RW->numOfParticles; ++i)
		{
			int index = 0; 
			double x = 0.0, y=0.0, z = 0.0;
			fscanf(pct_file, "%d %lf %lf %lf", &index, &x, &y, &z);
			RW->X[i].Now.elementIndex = index;
			RW->X[i].Now.x = x; RW->X[i].Now.y = y; RW->X[i].Now.z = z; 

			RW->X[i].Past = RW->X[i].Now;
		}

		fflush(pct_file);
		fclose(pct_file);	
	}
}

void CGSFormRightPicking::OnBnClickedCinele()
{
	CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0]; 

	RandomWalk* RW = NULL;
	RW = m_msh->PT;
	int numOfElement = m_msh->ele_vector.size();

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

