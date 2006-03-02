// PickedProperties.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "PickedProperties.h"
#include "COGLPickingView.h"
#include "nodes.h"
#include "elements.h"
#include ".\pickedproperties.h"
#include "MainFrm.h"


// PickedProperties dialog

IMPLEMENT_DYNAMIC(PickedProperties, CDialog)
PickedProperties::PickedProperties(CWnd* pParent /*=NULL*/)
	: CDialog(PickedProperties::IDD, pParent)
{
	sizeOfWord = 500;
	widthOfCell = 80;
	PCSSwitch = 0;
	BCnSTSwitch = 0;
}

PickedProperties::~PickedProperties()
{
}

void PickedProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PICKEDLIST, m_SmallList);
}


BEGIN_MESSAGE_MAP(PickedProperties, CDialog)
    ON_BN_CLICKED(IDCLOSE, OnBnClickedClose)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PICKEDLIST, OnLvnItemchangedPickedlist)
	ON_BN_CLICKED(IDC_BOTHORONE, OnBnClickedBothorone)
	ON_BN_CLICKED(IDC_BCNST, OnBnClickedBcnst)
	ON_BN_CLICKED(IDC_NODEINDEX, OnBnClickedNodeindex)
END_MESSAGE_MAP()


// PickedProperties message handlers

BOOL PickedProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_msh = NULL;
    m_ele = NULL;
	// All the processes created so that we can display variables that we want?
	// Let's check this by comparing the size of pcs_vector
	// If the size of pcs_vector > 0, Yes. If not, No.
	PCSSwitch = (int)pcs_vector.size();
	// Let's see if bc and source stuff are done
	if (bc_group_list.size() > 0 || st_group_list.size() > 0)
		BCnSTSwitch = 1;

	// This check box is only active when PCS is created.
	if(PCSSwitch && theApp.RFINodeSwitch == 1)
		GetDlgItem(IDC_BOTHORONE)->EnableWindow();
	else
		GetDlgItem(IDC_BOTHORONE)->EnableWindow(FALSE);

	// This check box is only active when either BC or ST is done.
	if(BCnSTSwitch && theApp.RFINodeSwitch == 1)
		GetDlgItem(IDC_BCNST)->EnableWindow();
	else
		GetDlgItem(IDC_BCNST)->EnableWindow(FALSE);

	// This check box is only active when either node index is done.
	if(theApp.ElementSwitch == 1)
		GetDlgItem(IDC_NODEINDEX)->EnableWindow();
	else
		GetDlgItem(IDC_NODEINDEX)->EnableWindow(FALSE);

	if(theApp.BothPrimaryVariable == 1 && theApp.RFINodeSwitch == 1)
	{
		// Ensable the BothOrOne check box
		CheckDlgButton(IDC_BOTHORONE, 1);
	}
	else
	{
		// Disable the BothOrOne check box
		CheckDlgButton(IDC_BOTHORONE, 0);
	}

	if(theApp.BCAndST == 1 && theApp.RFINodeSwitch == 1)
	{
		// Ensable the BCnST check box
		CheckDlgButton(IDC_BCNST, 1);
	}
	else
	{
		// Disable the BCnST check box
		CheckDlgButton(IDC_BCNST, 0);
	}

	if(theApp.NodeIndexOfElementSwitch == 1 && theApp.ElementSwitch == 1)
	{
		// Ensable the BCnST check box
		CheckDlgButton(IDC_NODEINDEX, 1);
	}
	else
	{
		// Disable the BCnST check box
		CheckDlgButton(IDC_NODEINDEX, 0);
	}
	ShowThePicked();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PickedProperties::OnBnClickedClose()
{
    // TODO: Add your control notification handler code here
    // TODO: Add your control notification handler code here
	CDialog::OnDestroy();

	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	(theApp->pPickedProperty)->DestroyWindow();	
}


void PickedProperties::ShowThePicked()
{
	if (theApp.GLINodeSwitch == 1)
		ListGLINodeSelected();
	else if (theApp.PolylineSwitch == 1)
		ListPolylineSelected();
    else if (theApp.SurfaceSwitch == 1)
		ListSurfaceSelected();
    else if (theApp.VolumeSwitch == 1)
		ListVolumeSelected();
	else if (theApp.RFINodeSwitch == 1) 
	{
		if (PCSSwitch > 0)
		{
			// if 1, show variables solved
			// if 0, just show properties with the solved variables.
			ListRFINodeSelected(1);
		}
		else
			ListRFINodeSelected(0);
	}
	else if (theApp.ElementSwitch == 1)
	{
		if (PCSSwitch > 0)
		{
			// if 1, show variables solved
			// if 0, just show properties with the solved variables.
			ListElementSelected(1);
		}
		else
			ListElementSelected(0);
	}
}


void PickedProperties::ListGLINodeSelected()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;
    CGLPoint* thisGLIPoint = NULL;

    int numOfItems = 7;

	m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "GLIPoint No.");
	m_SmallList.InsertColumn (2, "x");
	m_SmallList.InsertColumn (3, "y");
	m_SmallList.InsertColumn (4, "z");
	m_SmallList.InsertColumn (5, "Name");
	m_SmallList.InsertColumn (6, "Radius");

    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

	// Contents selected
	for (int i = 0; i < theApp->hitsGLINodeTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);

		sprintf(tempNum[0], "%d",i+1);
		sprintf(tempNum[1], "%d", theApp->GLInodePickedTotal[i]);

		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		
		CGLPoint aPoint = pView->GetGLIPointByIndex(theApp->GLInodePickedTotal[i]);

		sprintf(tempNum[2], "%f", aPoint.x);
        sprintf(tempNum[3], "%f", aPoint.y);
        sprintf(tempNum[4], "%f", aPoint.z);

		thisGLIPoint = GEOGetPointById(theApp->GLInodePickedTotal[i]);//CC
        for(int k=0; k <= (int)thisGLIPoint->name.size(); ++k)
        {
            tempNum[5][k] = thisGLIPoint->name[k];
        }

		sprintf(tempNum[6], "%f", thisGLIPoint->epsilon);

		for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);
		}	

		// Release memory
		free(tempNum);
	}
}

void PickedProperties::ListRFINodeSelected(int PCSSwitch)
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;

	int numOfItems = 0;
	int numOfDefaultItems = 5;
	int tempNumOfItem = 0;

	// Open the gate to processes 
	CRFProcess* m_pcs = NULL;
    m_msh = fem_msh_vector[0];

	// Let's find out how many items to show.
	if(PCSSwitch)	// if Processes created,
	{
		numOfItems = numOfDefaultItems;

		for(int i=0;i<(int)pcs_vector.size(); ++i)
		{
			m_pcs = pcs_vector[i];
			if(theApp->BothPrimaryVariable )
				numOfItems += m_pcs->pcs_number_of_primary_nvals*2;
			else
				numOfItems += m_pcs->pcs_number_of_primary_nvals;
		}
	}
	else
		numOfItems = numOfDefaultItems;	// Default

    m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "Node");
	m_SmallList.InsertColumn (2, "x");
	m_SmallList.InsertColumn (3, "y");
	m_SmallList.InsertColumn (4, "z");

	// Primary variables
	if(PCSSwitch)	// if Processes created,
	{
		int currentPosition = numOfDefaultItems;

		for(int i=0;i<(int)pcs_vector.size(); ++i)
		{
			m_pcs = pcs_vector[i];
			
			for(int j=0; j<m_pcs->pcs_number_of_primary_nvals; ++j)
			{
				string pcs_primary = m_pcs->pcs_primary_function_name[j];

				if(theApp->BothPrimaryVariable)
				{
					string Old = pcs_primary + "Old";
					m_SmallList.InsertColumn (currentPosition, Old.data() );	
					++currentPosition;
					string New = pcs_primary + "New";
					m_SmallList.InsertColumn (currentPosition, New.data() );
					++currentPosition;
				}
				else
				{
					m_SmallList.InsertColumn (currentPosition, pcs_primary.data() );
					++currentPosition;
				}
			}
		}
	}

	// BC and ST
	if(BCnSTSwitch && theApp->BCAndST == 1)
	{
		int currentPosition = tempNumOfItem = numOfItems;
		for(int i=0;i<(int)pcs_vector.size(); ++i)
		{
			m_pcs = pcs_vector[i];
			for(int j=0; j<m_pcs->pcs_number_of_primary_nvals; ++j)
			{
				string pcs_primary = m_pcs->pcs_primary_function_name[j];
				string pcs_primaryBC = "BC_"+pcs_primary;
				string pcs_primaryST = "ST_"+pcs_primary;

				m_SmallList.InsertColumn (currentPosition, pcs_primaryBC.data());
				++currentPosition;
				m_SmallList.InsertColumn (currentPosition, pcs_primaryST.data());
				++currentPosition;
			}
		}
		numOfItems = currentPosition;
	}
	
    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

	// Contents selected
	for (int i = 0; i < theApp->hitsRFINodeTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);

		sprintf(tempNum[0], "%d",i+1);
		sprintf(tempNum[1], "%d", theApp->RFInodePickedTotal[i]);
		
		// Real coordinate conversion
		sprintf(tempNum[2], "%e", m_msh->nod_vector[theApp->RFInodePickedTotal[i]]->X());
        sprintf(tempNum[3], "%e", m_msh->nod_vector[theApp->RFInodePickedTotal[i]]->Y());
        sprintf(tempNum[4], "%e", m_msh->nod_vector[theApp->RFInodePickedTotal[i]]->Z());

		if(PCSSwitch)	// if Processes created,
		{
			int currentPosition = numOfDefaultItems;
			for(int j=0;j<(int)pcs_vector.size();++j)
			{
				m_pcs = pcs_vector[j];
		
				for(int k=0; k<m_pcs->pcs_number_of_primary_nvals; ++k)
				{
					string pcs_primary = m_pcs->pcs_primary_function_name[k];
					// The following function is written for handling sting to char*
					char *VarName = string2CharArrary(pcs_primary);
				
					if(theApp->BothPrimaryVariable)
					{
                        int idxOld = m_pcs->GetNodeValueIndex(VarName);
                        int idxNew = m_pcs->GetNodeValueIndex(VarName)+1;
						sprintf(tempNum[currentPosition], "%e", m_pcs->GetNodeValue(theApp->RFInodePickedTotal[i],idxOld) );
						++currentPosition;
                        sprintf(tempNum[currentPosition], "%e", m_pcs->GetNodeValue(theApp->RFInodePickedTotal[i],idxNew) );
						++currentPosition;
					}
					else
					{
                        int idx = m_pcs->GetNodeValueIndex(VarName)+1;
                        
                        sprintf(tempNum[currentPosition], "%e", m_pcs->GetNodeValue(theApp->RFInodePickedTotal[i],idx) );
						++currentPosition;
					}
				}
			}
		}

		if(BCnSTSwitch && theApp->BCAndST == 1)
		{
			int currentPosition = tempNumOfItem;
			for(int j=0;j<(int)pcs_vector.size();++j)
			{
				m_pcs = pcs_vector[j];

				for(int k=0; k<m_pcs->pcs_number_of_primary_nvals; ++k)
				{
					// Let's print BC and ST values
					CBoundaryConditionsGroup *m_bc_group = NULL;
					CSourceTermGroup *m_st_group = NULL;

                    m_bc_group = BCGetGroup(m_pcs->pcs_type_name,m_pcs->pcs_primary_function_name[k]);
					m_st_group = STGetGroup(m_pcs->pcs_type_name,m_pcs->pcs_primary_function_name[k]);
				
					// BC printing
					double BCValue = 0.0;
					if( IsThisPointBCIfYesStoryValue(i, m_bc_group, &BCValue) )
					{
						sprintf(tempNum[currentPosition], "%e", BCValue);
						++currentPosition;
					}
					else
					{
						sprintf(tempNum[currentPosition], "Not assigned");
						++currentPosition;
					}

					// ST printing
					if (st_group_list.size() > 0)
					{
						double STValue = 0;
						if( IsThisPointSTIfYesStoryValue(i, m_st_group, &STValue) )
						{
							sprintf(tempNum[currentPosition], "%e", STValue);
							++currentPosition;
						}
						else
						{
							sprintf(tempNum[currentPosition], "Not assigned");
							++currentPosition;
						}
					}
					else
					{
						sprintf(tempNum[currentPosition], "No source term used");	
						++currentPosition;
					}
				}
			}
			numOfItems = currentPosition;
		}
	
		for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);
		}	

		free(tempNum);
	}
}


void PickedProperties::ListElementSelected(int PCSSwitch)
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;
    static long *element_nodes;

	int numOfItems = 0;
	int numOfDefaultItems = 5;
	int MaximumNumberOfNodesInAnElement = 8;
	int numOfElementValues = 0;

	// Open the gate to processes 
	CRFProcess* m_pcs = NULL;
    m_msh = fem_msh_vector[0];

	// Let's find out how many items to show.
	if(theApp->NodeIndexOfElementSwitch == 1)	
		numOfItems = numOfDefaultItems + MaximumNumberOfNodesInAnElement;	// This is to show node indexes in an element
	else
		numOfItems = numOfDefaultItems;

	// Let's find out how many items to show.
	if(PCSSwitch)	// if Processes created,
	{
		//for(int i=0;i<(int)pcs_vector.size(); ++i)
	//	{
			//m_pcs = pcs_vector[i];
            m_pcs = PCSGet("FLUID_MOMENTUM");
			numOfElementValues = m_pcs->pcs_number_of_evals; 
			numOfItems += numOfElementValues;
	//	}

	}

    m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "Element No.");
	m_SmallList.InsertColumn (2, "x");
	m_SmallList.InsertColumn (3, "y");
	m_SmallList.InsertColumn (4, "z");

	if(theApp->NodeIndexOfElementSwitch == 1)	
	{
		m_SmallList.InsertColumn (5, "Node 1");
		m_SmallList.InsertColumn (6, "Node 2");
		m_SmallList.InsertColumn (7, "Node 3");
		m_SmallList.InsertColumn (8, "Node 4");
		m_SmallList.InsertColumn (9, "Node 5");
		m_SmallList.InsertColumn (10, "Node 6");
		m_SmallList.InsertColumn (11, "Node 7");
		m_SmallList.InsertColumn (12, "Node 8");
	}

	if(PCSSwitch)	// if Processes created,
	{
//		for(int i=0;i<(int)pcs_vector.size(); ++i)
//		{
//			m_pcs = pcs_vector[i];
            m_pcs = PCSGet("FLUID_MOMENTUM");

			for(int j=0; j < m_pcs->pcs_number_of_evals; ++j)
			{
				string pcs_eval_name = m_pcs->pcs_eval_name[j];
				
				// Let's print only the evals defined in .out file.
				m_SmallList.InsertColumn (numOfDefaultItems + j,pcs_eval_name.data() );
			}
//		}
	}

    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

    // Contents selected
	for (int i = 0; i < theApp->hitsElementTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);
	
		sprintf(tempNum[0], "%d", i+1);
		sprintf(tempNum[1], "%d", theApp->elementPickedTotal[i]);

        // Compute x, y, z of element
		double centerX=0., centerY=0., centerZ=0.;
        
        m_ele = m_msh->ele_vector[theApp->elementPickedTotal[i]];        
        int numOfNodeInElement = m_ele->GetVertexNumber();     
        
        for(int j=0;j< numOfNodeInElement;++j)
	    {
            centerX += m_msh->nod_vector[m_ele->GetNodeIndex(j)]->X();
            centerY += m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Y();
            centerZ += m_msh->nod_vector[m_ele->GetNodeIndex(j)]->Z();
	    }
	    centerX /= (double)numOfNodeInElement; centerY /= (double)numOfNodeInElement; centerZ /= (double)numOfNodeInElement;
			
		sprintf(tempNum[2], "%e", centerX);
		sprintf(tempNum[3], "%e", centerY);
		sprintf(tempNum[4], "%e", centerZ);
	
		// If node index switch is on,
		if(theApp->NodeIndexOfElementSwitch == 1)	
		{
			for(int n=0; n<numOfNodeInElement; ++n)
			{
				sprintf(tempNum[numOfItems-MaximumNumberOfNodesInAnElement+n], "%d", element_nodes[n]);
			}
			// Check if empty cells because of the number of nodes in element less than 8.
			if (numOfNodeInElement < MaximumNumberOfNodesInAnElement)
			{
				for(int n=numOfNodeInElement; n < MaximumNumberOfNodesInAnElement; ++n)
					sprintf(tempNum[numOfItems-MaximumNumberOfNodesInAnElement+n], "Not applicable");
			}
		}

		if(PCSSwitch)	// if Processes created,
		{
            // Following line can be changed later on if each developer store element value of the process.
//			for(int j=0;j<(int)pcs_vector.size();++j)
//			{
//				m_pcs = pcs_vector[j];
                m_pcs = PCSGet("FLUID_MOMENTUM");
		
				for(int k=0; k < m_pcs->pcs_number_of_evals; ++k)
				{
					string pcs_eval_name = m_pcs->pcs_eval_name[k];
					// The following function is written for handling sting to char*
					char *VarName = string2CharArrary(pcs_eval_name);

                    int idx = m_pcs->GetElementValueIndex(VarName)+1;
                    sprintf(tempNum[numOfDefaultItems+k], "%e",m_pcs->GetElementValue( theApp->elementPickedTotal[i], idx) );

					free(VarName);
				}
			//}
		}

        for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);

		}

		free(tempNum);
    }
}


void PickedProperties::ListPolylineSelected()
{

    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;
    CGLPolyline* thisPolyline = NULL;

    int numOfItems = 3;     // This should be changed dynamically depending on the element type.

    m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "No.");
    m_SmallList.InsertColumn (2, "Name");

    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

    // Contents selected
	for (int i = 0; i < theApp->hitsPolylineTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);
	
		sprintf(tempNum[0], "%d", i+1);
        sprintf(tempNum[1], "%d", theApp->polylinePickedTotal[i]);
        thisPolyline = GEOGetPLYById(theApp->polylinePickedTotal[i]);//CC
        for(int k=0; k <= (int)thisPolyline->name.size(); ++k)
        {
            tempNum[2][k] = thisPolyline->name[k];
        }
        
        for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);
		}

		free(tempNum);
    }
}

void PickedProperties::ListSurfaceSelected()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;
    Surface* thisSurface = NULL;

    int numOfItems = 3;     // This should be changed dynamically depending on the element type.

    m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "No.");
    m_SmallList.InsertColumn (2, "Name");

    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

    // Contents selected
	for (int i = 0; i < theApp->hitsSurfaceTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);
	
		sprintf(tempNum[0], "%d", i+1);
        sprintf(tempNum[1], "%d", theApp->surfacePickedTotal[i]);

        // Get the surface by index to take care of;
        thisSurface = surface_vector[theApp->surfacePickedTotal[i]];//CC
        for(int k=0; k <= (int)thisSurface->name.size(); ++k)
        {
            tempNum[2][k] = thisSurface->name[k];
        }
        
        for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);
		}

		free(tempNum);
    }
}

void PickedProperties::ListVolumeSelected()
{
	CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	LV_ITEM lvitem;
    CGLVolume* thisVolume = NULL;

    int numOfItems = 3;     // This should be changed dynamically depending on the element type.

    m_SmallList.InsertColumn (0, "count");
	m_SmallList.InsertColumn (1, "No.");
    m_SmallList.InsertColumn (2, "Name");

    for(int i=0; i < numOfItems; ++i)
        m_SmallList.SetColumnWidth (i, widthOfCell);

    // Contents selected
	for (int i = 0; i < theApp->hitsVolumeTotal; ++i)
	{	
		// Create dynamic memory
		char** tempNum = CreateWordMemory(numOfItems);
	
		sprintf(tempNum[0], "%d", i+1);
        sprintf(tempNum[1], "%d", theApp->volumePickedTotal[i]);

        // Get the volume by index to take care of;
        thisVolume = GetVolume(theApp->volumePickedTotal[i]);
        for(int k=0; k <= (int)thisVolume->name.size(); ++k)
        {
            tempNum[2][k] = thisVolume->name[k];
        }
        
        for (int j = 0; j < numOfItems; ++j)
		{
			// Inserting the first column
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = j;
			lvitem.pszText = tempNum[j];

			if ( j == 0 )
				m_SmallList.InsertItem(&lvitem);
			else
				m_SmallList.SetItem(&lvitem);
		}

		free(tempNum);
    }
}



void PickedProperties::OnLvnItemchangedPickedlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// Getting the current list item number
	int nitem = pNMLV->iItem;
    nitem=nitem;
	

	*pResult = 0;
}
char** PickedProperties::CreateWordMemory(int numOfItems)
{
	// Create dynamic memory
	char** tempNum = NULL;
	tempNum = (char **) malloc(numOfItems * sizeof(char *));
	for(int p=0; p < numOfItems; ++p)
		tempNum[p] = (char *) malloc(sizeOfWord * sizeof(char));

	return tempNum;
}

int PickedProperties::GetNumberOfNodesInElement(int index)
{
	int numOfNodeInElement = 0;

	/*LINES = 1*/
	if (ElGetElementType(index) == 1)
	{
		numOfNodeInElement = 2;
    }
    /*RECTANGLES = 2*/ 
	else if (ElGetElementType(index) == 2)
	{
        numOfNodeInElement = 4;
    }
    /*HEXAHEDRA = 3*/ 
	else if (ElGetElementType(index) == 3)
	{
		numOfNodeInElement = 8;
    }
    /*TRIANGLES = 4*/ 
	else if (ElGetElementType(index) == 4)
	{
		numOfNodeInElement = 3;
    }
    /*TETRAHEDRAS = 5*/ 
	else if (ElGetElementType(index) == 5)
	{
		numOfNodeInElement = 4;
    }
    /*PRISMS = 6*/ 
	else if (ElGetElementType(index) == 6)
	{
		numOfNodeInElement = 6;
    }

	return numOfNodeInElement;
}

char* PickedProperties::string2CharArrary(string aString)
{
	char *charArrary = NULL;

	charArrary = (char *)malloc((aString.size()+1) * sizeof (char));
	for(int p=0; p < (int)aString.size(); ++p)
	{
		charArrary[p] = aString[p];
	}
	charArrary[p] = '\0';

	return charArrary;
}
void PickedProperties::OnBnClickedBothorone()
{
	// If checked
	if(IsDlgButtonChecked(IDC_BOTHORONE))
	{
		theApp.BothPrimaryVariable = 1;
	}
	else
	{
		theApp.BothPrimaryVariable = 0;
	}

	OnBnClickedClose();

	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
	mainframe->OnPickedProperty();
}

void PickedProperties::OnBnClickedBcnst()
{
	// If checked
	if(IsDlgButtonChecked(IDC_BCNST))
	{
		theApp.BCAndST = 1;
	}
	else
	{
		theApp.BCAndST = 0;
	}

	OnBnClickedClose();

	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
	mainframe->OnPickedProperty();
}

int PickedProperties::IsThisPointBCIfYesStoryValue(int index, CBoundaryConditionsGroup* m_bc_group, double* value)
{
	for(int p=0; p< (int)m_bc_group->group_vector.size(); ++p)		
		if(theApp.RFInodePickedTotal[index] == m_bc_group->group_vector[p]->msh_node_number)
		{
			*value = m_bc_group->group_vector[p]->node_value;
			return 1; // Yes, found it.
		}

	return 0;
}

int PickedProperties::IsThisPointSTIfYesStoryValue(int index, CSourceTermGroup* m_st_group, double* value)
{
	for(int p=0; p< (int)m_st_group->group_vector.size(); ++p)		
		if(theApp.RFInodePickedTotal[index] == m_st_group->group_vector[p]->msh_node_number)
		{
			*value = m_st_group->group_vector[p]->node_value;
			return 1; // Yes, found it.
		}

	return 0;
}


void PickedProperties::OnBnClickedNodeindex()
{
	// If checked
	if(IsDlgButtonChecked(IDC_NODEINDEX))
	{
		theApp.NodeIndexOfElementSwitch = 1;
	}
	else
	{
		theApp.NodeIndexOfElementSwitch = 0;
	}

	OnBnClickedClose();

	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
	mainframe->OnPickedProperty();

}
